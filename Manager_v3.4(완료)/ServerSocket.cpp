#include "stdafx.h"
#include "ServerSocket.h"

#include"Manager_v1.0.h"
#include"MainFrm.h"
#include"UITestOS.h"
#include"TestOSInfo.h"
#include"HashFuntion.h"
#include"VBoxController.h"

#include<WinSock2.h>
#include<ws2tcpip.h>
#pragma comment(lib,"ws2_32.lib")

#include<MSWSock.h>
#pragma comment(lib,"Mswsock")

////////////////////////////////////////////////////////////////////////////////////////////
//※※ CEvent ※※
CEvent g_CEventRecvEnd[WINDOW_10 + 1];
CEvent g_CEventConnet[WINDOW_10 + 1];
////////////////////////////////////////////////////////////////////////////////////////////
//※※ CCriticalSection ※※
CCriticalSection g_csUseClientSocek;
/////////////////////////////////////////////////////////////////////////////////////////////
UINT ThreadCommunication(LPVOID pParam)
{
	TEST_OS OS = *((TEST_OS*)pParam);
	CUITestOS* UI = ((CMainFrm*)theApp.m_pMainWnd)->m_CUIWIndow[OS];
	CTestOSInfo* Info = theApp.m_mapTestOsInfo[OS];

	//1. VM실행
	Info->SetState(STATE_READY);
	UI->SetChangeStateText();
	UI->SetLogText(_T("에이전트 접속을 기다립니다."));
	((CMainFrm*)theApp.m_pMainWnd)->DrawDropFileRect();
	UI->InvalidateRect(NULL);
	
	CString UUID = Info->GetUUID();
	theApp.m_CVBoxController->ExevuteVM(UUID);

	Info->InitResult();

	//2. 에이전트의 Accept를 기다리고 UI에 띄운다.
	HANDLE hArr[2];
	hArr[0] = g_CEventConnet[OS];
	hArr[1] = theApp.m_ExitEvent;
	DWORD dwWaitState;
	//1000 = 1초, 120 X 1000(2분 동안만 기다린다.)
	dwWaitState = WaitForMultipleObjects(2, hArr, FALSE, 120000);

	g_CEventConnet[OS].ResetEvent();

	//3.1. 에이전트 접속
	if (dwWaitState == WAIT_OBJECT_0)
	{		
		SOCKET	hClient = theApp.m_CServer->getClnSocket(OS);
		TCP_H Head;
		memset(&Head, 0, sizeof(TCP_H));
		printf("%d OS recv진입\n", OS);

		//Recive 받음
		//키워드 : recv() 대기시간
		//https://kldp.org/node/113143
		//https://docs.microsoft.com/en-us/windows/desktop/api/winsock/nf-winsock-setsockopt
		//또 다른 방법이 소켓 옵션에서 RCVTIMEO, 즉, receive time out 값을 설정하는 방법이다.
		//간단하게 소켓 생성 이후 소켓에 아래와 같이 설정함으로써 일정시간동안 recv 함수에서 대기가 발생하는 경우 타임아웃으로 처리할 수 있다.
		printf("socket을 설정하자.\n");
		DWORD	nTime = 5000;	//5000 = 5초 동안 오지 않으면 에이전트의 종료로 인식
		if (setsockopt(hClient, SOL_SOCKET, SO_RCVTIMEO, (char*)&nTime, sizeof(DWORD)) == SOCKET_ERROR)
		{
			OutputDebugString(_T("Health Check Socket의 설정을 실패했습니다."));
			shutdown(hClient, SD_BOTH);
			closesocket(hClient);
			return 0;
		}

		printf("Ransomware를 전송합니다.\n");
		UI->SetLogText(_T("Ransomware를 전송합니다."));
		UI->InvalidateRect(NULL);
		theApp.m_CServer->SendRansomware(OS);

		//3.1.1. 에이전트 Recevie 시작
		//https://m.blog.naver.com/PostView.nhn?blogId=skywood1&logNo=100128756613&proxyReferer=https%3A%2F%2Fwww.google.com%2F
		int nRecevie;
		while ((nRecevie = ::recv(hClient, (char*)&Head, sizeof(TCP_H), 0)) > 0)
		{
			if (Head.type == 0 || Head.type >= ALL_STOP)
			{
				printf("받은 크기 : %d", nRecevie);
				printf("Head가 이상하다 (Head : %d) 내용 : %zd, %zd, %p)\n",Head.type, Head.TestFileCount, Head.ResultCount, theApp.m_CServer->m_FuncReceive[Head.type]);
			}
			else
			{
				theApp.m_CServer->m_FuncReceive[Head.type](OS, Head);
			}
		}
		//DWORD dwError = WSAGetLastError();
		printf("## recv 빠져 나왔습니다.(%d)\n", OS);

		//3.1.2. 잘못된 상황에서 종료
		TEST_OS_STATE state = Info->getState();
		if (state != STATE_CLOSE)
		{
			Info->SetState(STATE_ERROR);
			UI->SetChangeStateText();
			UI->InvalidateRect(NULL);
			ErrorHandler(_T("에이전트와의 연결이 끊겼습니다."), FALSE);
		}
		
		//3.1.3. Socket을 종료 시킴
		g_csUseClientSocek.Lock();
		theApp.m_CServer->RemoveClient(OS);
		g_csUseClientSocek.Unlock();
		::shutdown(hClient, SD_BOTH);
		::closesocket(hClient);

	}
	//3.2. Geust OS 종료
	else if (dwWaitState == WAIT_TIMEOUT)
	{
		ErrorHandler(_T("에이전트의 Connect가 없습니다."), FALSE);

		theApp.m_mapTestOsInfo[OS]->SetState(STATE_ERROR);
		UI->SetChangeStateText();
		UI->SetLogText(_T("Connect이 없습니다."));
		UI->InvalidateRect(NULL);
		::Sleep(10);
	}

	//3.3. vm실행 중이다.
	if (theApp.m_CVBoxController->CheckBVMExecution(UUID) == TRUE)
	{
		UI->SetLogText(_T("GuestOS를 종료 중 입니다."));
		theApp.m_CVBoxController->QuitVM(UUID);
		theApp.m_CVBoxController->RestoreSnampshot(UUID);
		::Sleep(100);
	}

	//3.4. 상태가 바뀐다.
	if (Info->getState() == STATE_CLOSE)
	{
		Info->SetState(STATE_SHOW_REPORT);
		UI->SetChangeStateText();
		UI->SetLogText(_T("실험 결과 입니다."));
		UI->SetProgressText(0, SIZE_MAX);
		UI->InvalidateRect(NULL);
	}
	else if (Info->getState() == STATE_ERROR)
	{
		UI->DrawActive();
	}

	((CMainFrm*)theApp.m_pMainWnd)->DrawDropFileRect();
	//socket종료 됨
	g_CEventRecvEnd[OS].SetEvent();
	
	return 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
//Thread Accept 함수
UINT ThreadAccept(LPVOID pParam)
{
	SOCKADDR_IN clientaddr = { 0 };
	int nAddrLen = sizeof(clientaddr);

	SOCKET hClient = { 0 };
	CServerSocket *Server = theApp.m_CServer;

	//나중에 삭제
	//int temp = 0;
	while ((hClient = ::accept(Server->getSocket(), (SOCKADDR*)&clientaddr, &nAddrLen)) != INVALID_SOCKET)
	{
		TEST_OS os = Server->AddCln(clientaddr, hClient);
		
		theApp.m_mapTestOsInfo[os]->SetState(STATE_TEST);
		printf("%d accept 테스트 상태로 변경\n", os);
		Server->SendSignal(os, ACCEPT_AGENT);

		//Thread시작
		//LPVOID = Long Point void
		//예전에 사용하던 형식 LP. 그냥 32bit 포인터라고 생각하면 된다.
		//typedef UINT_PTR        SOCKET;
		//UINT_PTR은 32bit & 64bit 상호 호환되는 주소값을 제공하기 위한 변수 타입이다.
		//지금은 32bit이므로 LPVOID에 그대로 대입가능하지만 64bit면 ERROR가 날 수 있다.

		g_CEventRecvEnd[os].ResetEvent();
		g_CEventConnet[os].SetEvent();

	}

	OutputDebugString(_T("Communitication Accept Thread가 종료됩니다."));

	return 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
//#1 랜섬 전달 완료
void RecevieFileRasome(TEST_OS os, TCP_H& Head)
{
	CUITestOS* UI = ((CMainFrm*)theApp.m_pMainWnd)->m_CUIWIndow[os];

	printf("## 랜섬 파일 전송 성공\n");
	UI->SetLogText(_T("복구 도구를 전송합니다."));
	UI->InvalidateRect(NULL);
	::Sleep(10);
	theApp.m_CServer->SendRecovery(os);

}
//#1-1 랜섬 전달 ERROR
void RecevieErrorFileRasome(TEST_OS os, TCP_H& Head)
{
	printf("ERROR : 랜섬 파일 전송 실패\n");
	//Error Box
	theApp.m_CServer->TermicationSession(os, _T("랜섬파일 전송 실패"));
}
//#2 복구도구 전달 완료
void RecevieRecovery(TEST_OS os, TCP_H& Head)
{
	printf("## 복구도구 파일 전송 성공\n");
	theApp.m_CServer->SendSignal(os, START_RASOM);
}
//#2-2 복구도구 전달 ERROR
void RecevieErrorFileRecovery(TEST_OS os, TCP_H& Head)
{
	printf("ERROR : 복구도구 파일 전송 실패\n");
	//Error Box
	theApp.m_CServer->TermicationSession(os, _T("복구도구 파일 전송 실패"));

}
//#3 테스트 진행 정도
void RecevieProgress(TEST_OS os, TCP_H& Head)
{
	if (Head.ResultCount == 0)
	{
		printf("ERROR : 전송한 프로그램이 실행하지 않는다.\n");
		//Error Box
		theApp.m_CServer->TermicationSession(os, _T("전송한 프로그램이 작동하지 않습니다."));
	}
	else
	{
		//printf("## Progress %d / %d\n",Head.ResultCount, Head.TestFileCount);
		CUITestOS* UI = ((CMainFrm*)theApp.m_pMainWnd)->m_CUIWIndow[os];
		//UITestOS에 %가 바뀐다.
		UI->SetProgressText(Head.TestFileCount, Head.ResultCount);
	}
}
//#3-1 감염 시작
void RecevieStartRasom(TEST_OS os, TCP_H& Head)
{
	printf("## 감염을 시작 합니다.(%d)\n", os);

	CTestOSInfo *Info = theApp.m_mapTestOsInfo[os];
	CUITestOS* UI = ((CMainFrm*)theApp.m_pMainWnd)->m_CUIWIndow[os];

	//시간 기록
	Info->SetInfectionStart();
	//상태 변화
	Info->SetState(STATE_TEST);

	//UI 변화
	UI->SetChangeStateText();
	UI->SetLogText(_T("감염중 입니다."));
	UI->SetProgressText(Head.TestFileCount, 0);
	UI->InvalidateRect(NULL);
	::Sleep(10);

}
//#3-2 감염 끝
void RecevieEndRasom(TEST_OS os, TCP_H& Head)
{
	printf("## 감염이 끝났습니다.(%d)\n", os);

	CTestOSInfo *Info = theApp.m_mapTestOsInfo[os];
	CUITestOS* UI = ((CMainFrm*)theApp.m_pMainWnd)->m_CUIWIndow[os];

	//시간 기록
	Info->SetInfectionEnd();

	//UI변화
	UI->SetLogText(_T("감염끝. 결과를 저장중 입니다."));
	UI->InvalidateRect(NULL);
	::Sleep(10);
	//hash값 저장
	theApp.m_CServer->SendSignal(os, SAVE_RASOMWARE_HASH);

}
//#3-3 복구 시작
void RecevieStartRecovery(TEST_OS os, TCP_H& Head)
{
	printf("## 복구를 시작 합니다.(%d)\n", os);

	CTestOSInfo *Info = theApp.m_mapTestOsInfo[os];
	CUITestOS* UI = ((CMainFrm*)theApp.m_pMainWnd)->m_CUIWIndow[os];
	//시간 기록
	Info->SetRecoveryStart();

	//UI 변화
	UI->SetLogText(_T("복구중 입니다."));
	UI->SetProgressText(Head.TestFileCount, 0);
	UI->InvalidateRect(NULL);
	::Sleep(10);
}
//#3-4 복구 끝
void RecevieEndRecovery(TEST_OS os, TCP_H& Head)
{
	printf("## 복구가 끝났습니다.(%d)\n", os);

	CTestOSInfo *Info = theApp.m_mapTestOsInfo[os];
	CUITestOS* UI = ((CMainFrm*)theApp.m_pMainWnd)->m_CUIWIndow[os];

	//시간 기록
	Info->SetRecoveryEnd();

	//UI 변화
	UI->SetLogText(_T("복구끝. 결과를 저장중 입니다."));
	UI->InvalidateRect(NULL);
	::Sleep(10);
	//Hash를 저장 해라
	theApp.m_CServer->SendSignal(os, SAVE_RECOVERY_HASH);

}
//#4 결과를 저장했다.
void RecevieSaveRasomwareHash(TEST_OS os, TCP_H& Head)
{
	printf("## rasomware 결과 저장(%d)\n", os);

	//
	theApp.m_CServer->SendSignal(os, START_RECOVERY);
}
void RecevieSaveRecoveryHash(TEST_OS os, TCP_H& Head)
{
	printf("## recovery 결과 저장(%d)\n", os);

	//결과 전송
	theApp.m_CServer->SendSignal(os, TEST_RESULT);
}
//#5 결과를 받습니다.
void RecevieTestResult(TEST_OS os, TCP_H& Head)
{
	CTestOSInfo *Info = theApp.m_mapTestOsInfo[os];
	CUITestOS* UI = ((CMainFrm*)theApp.m_pMainWnd)->m_CUIWIndow[os];

	//결과를 저장한다.
	Info->AddResult(Head.FileInfo);

	UI->SetProgressText(Head.TestFileCount, Head.ResultCount);
	UI->InvalidateRect(NULL);
	::Sleep(10);

	if (Head.ResultCount == 1)
	{
		printf("## 결과를 전송 받는다(%d)\n", os);

		//상태 변화
		Info->SetState(STATE_TEST_COMPLETE);

		//UI 변화
		UI->SetChangeStateText();
		UI->SetLogText(_T("결과 전송 받는 중 입니다."));
		UI->InvalidateRect(NULL);
		::Sleep(10);
	}
	//모두 받음
	else if (Head.TestFileCount == Head.ResultCount)
	{
		printf("## 결과를 전송 끝(%d)\n", os);

		//복구된 파일 계산
		Info->CountRecoveryFile();

		//UI변경
		UI->SetLogText(_T("보고서 저장중 입니다."));
		UI->InvalidateRect(NULL);
		::Sleep(10);

		//보고서 저장
		Info->ExportData();
		Info->ReleaseResult();

		//상태 변화
		Info->SetState(STATE_CLOSE);

		UI->SetChangeStateText();
		UI->SetLogText(_T("에이전트 종료 중"));
		UI->InvalidateRect(NULL);
		::Sleep(10);
		//에이전트 종료와 vm종료, 스냅샷 복구(Thread)
		//일단 에이전트 종료, 나중에 변경된다.
		theApp.m_CServer->SendSignal(os, ALL_STOP);
	}

}
//#5 Health Check
void RecevieHealthCheck(TEST_OS os, TCP_H& Head)
{
	//printf("Health Check!!(%d)\n", os);
}

////////////////////////////////////////////////////////////////////////////////////////////
//class 함수
CServerSocket::CServerSocket()
	:m_hSocket(0)
	//m_ConnectEnd(FALSE)
{
	g_CEventRecvEnd[WINDOW_7].SetEvent();
	g_CEventRecvEnd[WINDOW_8].SetEvent();
	g_CEventRecvEnd[WINDOW_10].SetEvent();

	//
	for (int i = 0; i < (ALL_STOP + 1); i++)
	{
		m_FuncReceive[i] = NULL;
	}
	//
	m_FuncReceive[FILE_RASOM] = RecevieFileRasome;
	m_FuncReceive[FILE_RASOM_ERROR] = RecevieErrorFileRasome;
	m_FuncReceive[FILE_RECOVERY] = RecevieRecovery;
	m_FuncReceive[FILE_RECOVERY_ERROR] = RecevieErrorFileRecovery;
	m_FuncReceive[SAVE_RASOMWARE_HASH] = RecevieSaveRasomwareHash;
	m_FuncReceive[SAVE_RECOVERY_HASH] = RecevieSaveRecoveryHash;
	m_FuncReceive[START_RASOM] = RecevieStartRasom;
	m_FuncReceive[END_RASOM] = RecevieEndRasom;
	m_FuncReceive[START_RECOVERY] = RecevieStartRecovery;
	m_FuncReceive[END_RECOVERY] = RecevieEndRecovery;
	m_FuncReceive[TEST_PROGRESS] = RecevieProgress;
	m_FuncReceive[TEST_RESULT] = RecevieTestResult;
	m_FuncReceive[HEALTH_CHECK] = RecevieHealthCheck;

	//1. 윈속 초기화
	m_wsa = { 0 };
	if (::WSAStartup(MAKEWORD(2, 2), &m_wsa) != 0)
		OutputDebugString(_T("윈속을 초기화 할 수 없습니다."));
}


CServerSocket::~CServerSocket()
{
}

void CServerSocket::InitAcceptSocket(SOCKET& hSocket, unsigned short Port, BOOL isHealth)
{
	//2. 접속대기 소켓 생성
	hSocket = ::socket(AF_INET, SOCK_STREAM, 0);
	if (hSocket == INVALID_SOCKET)
	{
		OutputDebugString(_T("접속대기 소켓을 생성할 수 없습니다."));
		//ErrorBox
		ErrorHandler(_T("접속대기 소켓을 생성할 수 없습니다."), TRUE);
	}
	//3. 포트 바인딩
	SOCKADDR_IN svraddr = { 0 };
	svraddr.sin_family = AF_INET;
	svraddr.sin_port = htons(Port);
	svraddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	if (::bind(hSocket, (SOCKADDR*)&svraddr, sizeof(svraddr)) == SOCKET_ERROR)
	{
		OutputDebugString(_T("소켓에 IP주소와 포트를 바인드 할 수 없습니다."));
		//ErrorBox
		ErrorHandler(_T("소켓에 IP주소와 포트를 바인드 할 수 없습니다."), TRUE);
	}
	//4. 접속대기 상태로 전환
	if (::listen(hSocket, SOMAXCONN) == SOCKET_ERROR)
	{
		OutputDebugString(_T("리슨 상태로 전환할 수 없습니다."));
		//ErrorBox
		ErrorHandler(_T("리슨 상태로 전환할 수 없습니다."), TRUE);
	}

}

//접속 함수
void CServerSocket::AcceptFuntion(void)
{
	this->InitAcceptSocket(m_hSocket, 25000, FALSE);
	CWinThread *pThread = AfxBeginThread(ThreadAccept, NULL);
}

//Add Client
TEST_OS CServerSocket::AddCln(SOCKADDR_IN& clnaddr, SOCKET& hSocekt)
{
	TEST_OS os;
	char clnIP[254];
	memset(clnIP, 0, sizeof(char) * 254);
	inet_ntop(AF_INET, &clnaddr.sin_addr, clnIP, sizeof(char) * 254);

	if (strcmp(clnIP, "192.168.56.101") == 0)
	{
		os = WINDOW_7;
		printf("window7 접속\n");
	}
	else if (strcmp(clnIP, "192.168.56.102") == 0)
	{
		os = WINDOW_8;
		printf("window8 접속\n");
	}
	else if (strcmp(clnIP, "192.168.56.103") == 0)
	{
		os = WINDOW_10;
		printf("window10 접속\n");
	}

	//$@
	g_csUseClientSocek.Lock();
	m_mapClnSocket.SetAt(os, hSocekt);
	g_csUseClientSocek.Unlock();

	return os;
}

//send 함수
void CServerSocket::SendSignal(TEST_OS os, HEAD_TYPE signal)
{
	TCP_H Head;
	memset(&Head, 0, sizeof(TCP_H));
	int size = sizeof(TCP_H);
	Head.type = signal;

	//$@
	g_csUseClientSocek.Lock();
	::send(m_mapClnSocket[os], (char*)&Head, size, 0);
	g_csUseClientSocek.Unlock();
}

void CServerSocket::SendRansomware(TEST_OS os)
{
	TCP_H Head;
	memset(&Head, 0, sizeof(TCP_H));
	Head.type = FILE_RASOM;
	memcpy(Head.FileInfo.FileName, ((CMainFrm*)theApp.m_pMainWnd)->m_CRansomwareName, sizeof(TCHAR) * MAX_PATH);
	
	//$@
	g_csUseClientSocek.Lock();

	if (FileHashCalution(((CMainFrm*)theApp.m_pMainWnd)->m_CRansomwarePath.GetBuffer(0), Head.FileInfo.OriginalHash) == FALSE)
	{
		OutputDebugString(_T("Ransomware 파일의 Hash값을 받지 못했습니다."));
	}

	//파일 열기
	HANDLE hFile = ::CreateFile(((CMainFrm*)theApp.m_pMainWnd)->m_CRansomwarePath,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_FLAG_SEQUENTIAL_SCAN,
		NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		OutputDebugString(_T("Ransomware 파일 Open실패"));
		//Error code 넣기
		TermicationSession(os, _T("랜섬웨어 파일 open실패"));
		return;
	}

	Head.FileInfo.FileSize = ::GetFileSize(hFile, NULL);
	
	TRANSMIT_FILE_BUFFERS tfb = { 0 };
	tfb.Head = &Head;
	tfb.HeadLength = sizeof(TCP_H);

	//파일 송신
	if (::TransmitFile(
		m_mapClnSocket[os],	//파일을 전송할 소켓 핸들.
		hFile,		//전송할 파일 핸들
		0,			//전송할 크기, 0이면 전체.
		65535,		//한 번에 전송할 버퍼 크기,MAX_PACKET_SIZE
		NULL,		//비동기 입/출력에 대한 OVERLAPPED구조체
		&tfb,		//파일 전송에 앞서 먼저 전송할 데이터
		0			//기타 옵션
		) == FALSE)
	{
		OutputDebugString(_T("파일 전송에 실패했습니다."));
		//ERROR code 넣기
		TermicationSession(os,_T("랜섬웨어 파일 전송 실패"));
	}
	CloseHandle(hFile);
	g_csUseClientSocek.Unlock();
}

void CServerSocket::SendRecovery(TEST_OS os)
{
	TCP_H Head;
	memset(&Head, 0, sizeof(TCP_H));
	Head.type = FILE_RECOVERY;
	
	memcpy(Head.FileInfo.FileName, ((CMainFrm*)theApp.m_pMainWnd)->m_CRecoveryName, sizeof(TCHAR) * MAX_PATH);
	
	//$@
	g_csUseClientSocek.Lock();
	if (FileHashCalution(((CMainFrm*)theApp.m_pMainWnd)->m_CRecoveryPath.GetBuffer(0), Head.FileInfo.OriginalHash) == FALSE)
	{
		OutputDebugString(_T("복구도구 파일의 Hash값을 받지 못했습니다."));
	}

	//파일 열기
	HANDLE hFile = ::CreateFile(((CMainFrm*)theApp.m_pMainWnd)->m_CRecoveryPath,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_FLAG_SEQUENTIAL_SCAN,
		NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		OutputDebugString(_T("복구도구 파일 Open실패"));
		//Error code 넣기
		TermicationSession(os, _T("복구도구 파일 Open실패"));
		return;
	}

	Head.FileInfo.FileSize = ::GetFileSize(hFile, NULL);

	TRANSMIT_FILE_BUFFERS tfb = { 0 };
	tfb.Head = &Head;
	tfb.HeadLength = sizeof(TCP_H);

	//파일 송신
	if (::TransmitFile(
		m_mapClnSocket[os],	//파일을 전송할 소켓 핸들.
		hFile,		//전송할 파일 핸들
		0,			//전송할 크기, 0이면 전체.
		65535,		//한 번에 전송할 버퍼 크기,MAX_PACKET_SIZE
		NULL,		//비동기 입/출력에 대한 OVERLAPPED구조체
		&tfb,		//파일 전송에 앞서 먼저 전송할 데이터
		0			//기타 옵션
	) == FALSE)
	{
		OutputDebugString(_T("파일 전송에 실패했습니다."));
		//Error code 넣기
		TermicationSession(os, _T("파일 전송에 실패했습니다."));
	}

	CloseHandle(hFile);
	g_csUseClientSocek.Unlock();

}

//Error로 인한 종료
void CServerSocket::TermicationSession(TEST_OS os, LPCTSTR lpszText)
{
	//Error code 넣기
	ErrorHandler(lpszText, FALSE);
	SendSignal(os, ALL_STOP);
}

void CServerSocket::Release()
{
	//에이전트와의 연결 종료
	BOOL bFind = FALSE;
	SOCKET hSocket;

	for (int i = 0; i < (WINDOW_10 + 1); i++)
	{
		g_csUseClientSocek.Lock();
		bFind = m_mapClnSocket.Lookup((TEST_OS)i, hSocket);
		g_csUseClientSocek.Unlock();

		if (bFind == TRUE)
		{
			theApp.m_mapTestOsInfo[(TEST_OS)i]->SetState(STATE_CLOSE);
			SendSignal((TEST_OS)i, ALL_STOP);
		}
	}

	//모두 종료되길 기다린다.
	HANDLE hArr[3];
	hArr[0] = g_CEventRecvEnd[WINDOW_7];
	hArr[1] = g_CEventRecvEnd[WINDOW_8];
	hArr[2] = g_CEventRecvEnd[WINDOW_10];

	WaitForMultipleObjects(3, hArr, TRUE, INFINITE);

	//Accept 종료
	::shutdown(m_hSocket, SD_BOTH);

	POSITION pos;
	TEST_OS os;

	//!!communication socket
	//$@
	g_csUseClientSocek.Lock();
	pos = m_mapClnSocket.GetStartPosition();
	while (pos != NULL)
	{
		m_mapClnSocket.GetNextAssoc(pos, os, hSocket);
		//소켓 해제
		closesocket(hSocket);
	}
	//map 전체 삭제
	m_mapClnSocket.RemoveAll();
	g_csUseClientSocek.Unlock();

	OutputDebugString(_T("모든 클라이언트 연결을 종료\n"));

	::Sleep(100);
	if (m_hSocket != 0) ::closesocket(m_hSocket);
	
	//윈속 해제
	::WSACleanup();

}
