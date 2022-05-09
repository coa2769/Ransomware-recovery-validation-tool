#include "stdafx.h"
#include "ClientSocket.h"
#include"TestFileFution.h"
#include"HashFuntion.h"

#include<strsafe.h>
#include<ShlObj.h>
#pragma comment(lib,"Shell32") 

////////////////////////////////////////////////////////////////////////////
CRITICAL_SECTION g_csSend;
////////////////////////////////////////////////////////////////////////////
//※※ Thread ※※
////////////////////////////////////////////////////////////////////////////
DWORD WINAPI ThreadSendHealthCheck(LPVOID pParam)
{
	printf("********** Start Health Check **********\n");

	CClientSocket* Client = (CClientSocket*)pParam;
	TCP_H Head;
	int count = 0;
	while (true)
	{
		if (Client->GetCloseHealthCheck() == TRUE)
		{
			break;
		}

		memset(&Head, 0, sizeof(Head));
		Head.type = HEALTH_CHECK;
		
		::EnterCriticalSection(&g_csSend);
		::send(Client->GetSocket(), (char*)&Head, sizeof(TCP_H), 0);
		::LeaveCriticalSection(&g_csSend);

		//밀리어초 값이 들어간다.
		//1000ms = 1s
		::Sleep(1000); 
	}

	printf("********** END Health Check ************\n");

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
//Lockup table에 넣을 Receive
//#1 접속된 것을 알려줌
void ReceiveAccpetAgent(CClientSocket* Client, TCP_H& Head)
{
	//OS를 종류를 알려준다.
	printf("Agent Connection Complete!!\n");

	//Thread생성
	DWORD dwThreadID = 0;
	HANDLE hThread = ::CreateThread(
		NULL,
		0,
		ThreadSendHealthCheck,
		(LPVOID)Client,
		0,
		&dwThreadID);
	::CloseHandle(hThread);
}

//#2 파일 받은(Ransom, recovery)
void ReceiveFileRasom(CClientSocket* Client, TCP_H& Head)
{
	printf("*********Receive Rasomware**********\n");
	//파일 받기
	TCHAR FilePath[MAX_PATH];
	memset(FilePath, 0, sizeof(TCHAR)*MAX_PATH);
	//여기서 파일 경로가 나온다.
	Client->FileRecive(Head, FilePath);
	//hash값 검사
	if (Client->FileHashCheck(Head.FileInfo.OriginalHash, FilePath) == TRUE)
	{
		//잘 받았으면 이름 저장
		Client->GetCTestFileInfo()->SetRasomwareFilePath(FilePath);
		Client->GetCTestFileInfo()->SetRansomwareFileName(Head.FileInfo.FileName);
		Client->SendSignal(FILE_RASOM);
	}
	else
	{
		Client->SendSignal(FILE_RASOM_ERROR);
	}
}

void ReceiveFileRecovery(CClientSocket* Client, TCP_H& Head)
{
	printf("*********Receive Recovery**********\n");
	//파일 받기
	TCHAR FilePath[MAX_PATH];
	memset(FilePath, 0, sizeof(TCHAR)*MAX_PATH);
	Client->FileRecive(Head, FilePath); //여기서 파일 경로가 나온다.
								//hash값 검사
	if (Client->FileHashCheck(Head.FileInfo.OriginalHash, FilePath) == TRUE)
	{
		//잘 받았으면 이름 저장
		Client->GetCTestFileInfo()->SetRecoveryFilePath(FilePath);
		Client->GetCTestFileInfo()->SetRecoveryFileName(Head.FileInfo.FileName);
		Client->SendSignal(FILE_RECOVERY);
	}
	else
	{
		Client->SendSignal(FILE_RECOVERY_ERROR);
	}
}

//#3 Test시작 신호를 받음
void ReceiveStartRasomware(CClientSocket* Client, TCP_H& Head)
{
	printf("Start Rasomware!!\n");
	Client->SetTestState(START_RASOM);
	//File 감시와 프로그램 실행(Thread)
	DWORD dwThreadID = 1;
	HANDLE hThread = ::CreateThread(
		NULL,
		0,
		MonitoringFolder,
		Client,
		0,
		&dwThreadID);
	::CloseHandle(hThread);
	//시작 신호 보내기
	Client->SendRasomStart();
}

//#4 Recovery 시작
void ReceiveStartRecovery(CClientSocket* Client, TCP_H& Head)
{
	printf("Start Recovery!!\n");
	Client->SetTestState(START_RECOVERY);
	//File 감시와 프로그램 실행(Thread)
	DWORD dwThreadID = 2;
	HANDLE hThread = ::CreateThread(
		NULL,
		0,
		MonitoringFolder,
		Client,
		0,
		&dwThreadID);
	::CloseHandle(hThread);
	//시작 신호 보내기
	Client->SendRecoveryStart();
}

//Rasomware Hash를 저장한다.
void ReceiveSaveRasomwareHash(CClientSocket* Client, TCP_H& Head)
{
	printf("Save Rasomware Hash!!\n");
	Client->GetCTestFileInfo()->HashAfterRasomware();
	Client->SendSignal(SAVE_RASOMWARE_HASH);
}

//Recovery Hash를 저장한다.
void ReceiveSaveRecoveryHash(CClientSocket* Client, TCP_H& Head)
{
	printf("Save Recovery Hash!!\n");
	Client->GetCTestFileInfo()->HashAfterRecovery();
	Client->SendSignal(SAVE_RECOVERY_HASH);
}

//결과를 전송
void ReceiveTestResult(CClientSocket* Client, TCP_H& Head)
{
	printf("Send Result!!\n");
	Client->SendTestResult(); //내부에서 for로 전송
}

/////////////////////////////////////////////////////////////////////////////
// Class함수
CClientSocket::CClientSocket(CTestFileInfo& DB)
	:m_CInfo(&DB),
	m_bCloseHealthCheck(FALSE)
{
	//다른 초기화 방법
	//https://girtowin.tistory.com/107
	//https://docs.microsoft.com/en-us/windows/desktop/sync/using-critical-section-objects
	::InitializeCriticalSection(&g_csSend);

	//함수 포인터 초기화
	for (int i = 0; i < ((int)ALL_STOP) + 1; i++)
	{
		m_FuncReceive[i] = NULL;
	}

	m_FuncReceive[ACCEPT_AGENT] = ReceiveAccpetAgent;
	m_FuncReceive[FILE_RASOM] = ReceiveFileRasom;
	m_FuncReceive[FILE_RECOVERY] = ReceiveFileRecovery;
	m_FuncReceive[START_RASOM] = ReceiveStartRasomware;
	m_FuncReceive[START_RECOVERY] = ReceiveStartRecovery;
	m_FuncReceive[SAVE_RASOMWARE_HASH] = ReceiveSaveRasomwareHash;
	m_FuncReceive[SAVE_RECOVERY_HASH] = ReceiveSaveRecoveryHash;
	m_FuncReceive[TEST_RESULT] = ReceiveTestResult;


	//1. 윈속 초기화
	if (::WSAStartup(MAKEWORD(2, 2), &m_wsa) != 0)
	{
		::WSACleanup();
		ErrorHandler("Not Initalized Winsock", false);
	}

}

CClientSocket::~CClientSocket()
{
	::DeleteCriticalSection(&g_csSend);
	ReleaseSocket();
}

//Connect 함수
void CClientSocket::Connect(char* IP, unsigned short Port)
{
	printf("*****START Connect******\n");

	//※※※※※※※※※※※※ 소켓 생성 ※※※※※※※※※※※※
	//2. 서버에 연결할 소켓 생성
	m_hSocket = ::socket(AF_INET, SOCK_STREAM, 0);
	if (m_hSocket == INVALID_SOCKET)
	{
		::WSACleanup();
		ErrorHandler("Could Not Create Socket", false);
	}

	//3. 포트 바인딩 및 연결
	SOCKADDR_IN svraddr = { 0 };
	svraddr.sin_family = AF_INET;
	svraddr.sin_port = htons(Port);
	//https://techlog.gurucat.net/317
	//https://docs.microsoft.com/en-us/windows/desktop/api/ws2tcpip/nf-ws2tcpip-inetptonw
	//inet_addr()대신 inet_pton()함수를 사용해야 한다.(WS2tcpip.h 선언)
	inet_pton(AF_INET, IP, &svraddr.sin_addr.S_un.S_addr);

	if (::connect(m_hSocket, (SOCKADDR*)&svraddr, sizeof(svraddr)) == SOCKET_ERROR)
	{
		::WSACleanup();
		ErrorHandler("Unable To Connect To Server", false);
	}

	printf("*****End Connect******\n");
}

////////////////////////////////////////////////////////////////////////////////
//Recive함수
void CClientSocket::Recive(void)
{
	printf("*****Start Recive*******\n");

	TCP_H Head;
	memset(&Head, 0, sizeof(TCP_H));
	int size = sizeof(TCP_H);
	int nRecv = 0;

	while ((nRecv = ::recv(m_hSocket, (char*)&Head, size, 0)) > 0)
	{
		if (Head.type == 0 || Head.type >= (ALL_STOP + 1))
		{
			printf("받은 크기 : %d", nRecv);
			printf("Head가 이상하다 (Head : %d) 내용 : %zd, %zd, %p)\n", Head.type, Head.TestFileCount, Head.ResultCount, m_FuncReceive[Head.type]);
		}
		//# Test 종료 신호를 받음(Test중 이건 상관없이 종료)[2]
		else if (Head.type == ALL_STOP)
		{
			//이 While문에서 나간다.
			break;
		}
		else
		{
			//Lookup Table
			m_FuncReceive[Head.type](this, Head);
		}
	}

	printf("*****End Recive(%d)*******\n", nRecv);
}

void CClientSocket::FileRecive(TCP_H& Head, TCHAR* FilePath)
{
	BYTE byBuffer[65535];
	int nRecv;
	DWORD dwTotalRecv = 0, dwRead = 0;
	HANDLE hFile;

	//파일 생성 주소 만들기
	//https://docs.microsoft.com/ko-kr/windows/desktop/api/shlobj_core/nf-shlobj_core-shgetfolderpatha
	//https://msdn.microsoft.com/ko-kr/33d92271-2865-4ebd-b96c-bf293deb4310
	if (SHGetFolderPathW(NULL, CSIDL_DESKTOP, NULL, 0, FilePath) != S_OK)
	{
		ErrorHandler("Could Not Find Folder To Save", false);
	}
	//파일 경로
	StringCchCat(FilePath, MAX_PATH, _T("\\"));
	StringCchCat(FilePath, MAX_PATH, Head.FileInfo.FileName);

	//파일 만들기
	hFile = ::CreateFileW(
		FilePath,
		GENERIC_WRITE,
		0,
		NULL,
		CREATE_ALWAYS,	//언제나 파일을 생성한다.
		0,
		NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		ErrorHandler("Failed To Create File", false);
	}

	while (dwTotalRecv < Head.FileInfo.FileSize)
	{
		if ((nRecv = ::recv(m_hSocket, (char*)byBuffer, 65535, 0)) > 0)
		{
			dwTotalRecv += nRecv;
			//서버에서 받은 크기만큼 데이터를 파일에 쓴다.
			::WriteFile(hFile, byBuffer, nRecv, &dwRead, NULL);
			printf("Receive : %d/%d\n", dwTotalRecv, Head.FileInfo.FileSize);
			fflush(stdout);
		}
		else
		{
			ErrorHandler("Error Receiving File", true);
			break;
		}
	}

	::CloseHandle(hFile);

}

BOOL CClientSocket::FileHashCheck(BYTE* OriHash, TCHAR *FilePath)
{
	BYTE Hash[20];
	if (FileHashCalution(FilePath, Hash) == FALSE)
	{
		ErrorHandler("Failed Hash Value Calculation", false);
	}
	return SameValue(OriHash, Hash);
}

//////////////////////////////////////////////////////////////////////////////////////
// Send함수 확인에 대한
void CClientSocket::SendSignal(HEAD_TYPE signal)
{
	printf("*****Start SendSignal(%d)*******\n", signal);

	TCP_H Head;
	memset(&Head, 0, sizeof(Head));
	Head.type = signal;

	::EnterCriticalSection(&g_csSend);
	::send(m_hSocket, (char*)&Head, sizeof(TCP_H), 0);
	::LeaveCriticalSection(&g_csSend);

	printf("*****End SendSignal(%d)*******\n", signal);
}

//////////////////////////////////////////////////////////////////////////////////////
//Send함수들
//#1 감염 & 복구 시작 시간을 알림
void CClientSocket::SendRasomStart(void)
{
	printf("*****Start SendRasomStart()*******\n");

	TCP_H Head;
	memset(&Head, 0, sizeof(Head));
	Head.type = START_RASOM;
	Head.TestFileCount = m_CInfo->GetTestFileCount();

	::EnterCriticalSection(&g_csSend);
	::send(m_hSocket, (char*)&Head, sizeof(TCP_H), 0);
	::LeaveCriticalSection(&g_csSend);

	printf("*****End SendRasomStart()*******\n");
}

void CClientSocket::SendRecoveryStart(void)
{
	printf("*****Start SendRecoveryStart()*******\n");

	TCP_H Head;
	memset(&Head, 0, sizeof(Head));
	Head.type = START_RECOVERY;
	Head.TestFileCount = m_CInfo->GetTestFileCount();

	::EnterCriticalSection(&g_csSend);
	::send(m_hSocket, (char*)&Head, sizeof(TCP_H), 0);
	::LeaveCriticalSection(&g_csSend);

	printf("*****End SendRecoveryStart()*******\n");
}

//#2 진행률 알림[2]
void CClientSocket::SendTestProgress(void)
{
	printf("*****Start SendTestProgress()*******\n");
	//release모드의 문제가 아니라면 삭제
	TCP_H Head;
	memset((void*)&Head, 0, sizeof(Head));
	Head.type = TEST_PROGRESS;
	Head.TestFileCount = m_CInfo->GetTestFileCount();
	Head.ResultCount = m_CInfo->GetChageTestFileCount();

	::EnterCriticalSection(&g_csSend);
	::send(m_hSocket, (char*)&Head, sizeof(TCP_H), 0);
	::LeaveCriticalSection(&g_csSend);
	
	printf("*****End SendTestProgress()*******\n");
}

//#3 결과 전송[2]
void CClientSocket::SendTestResult(void)
{
	printf("*****Start SendTestResult()*******\n");

	TCP_H Head;
	for (unsigned int i = 0; i < m_CInfo->GetTestFileCount(); i++)
	{
		memset(&Head, 0, sizeof(Head));
		Head.type = TEST_RESULT;
		Head.TestFileCount = m_CInfo->GetTestFileCount();
		Head.ResultCount = i + 1;
		m_CInfo->GetTestFileInfo(i, Head.FileInfo);

		::EnterCriticalSection(&g_csSend);
		::send(m_hSocket, (char*)&Head, sizeof(TCP_H), 0);
		::LeaveCriticalSection(&g_csSend);
	}
	//결과 전송
	printf("*****End SendTestResult()*******\n");
}

////////////////////////////////////////////////////////////////////////////////
//해제
void CClientSocket::ReleaseSocket(void)
{
	printf("*******ReleaseSocket() Start*********\n");

	m_bCloseHealthCheck = TRUE;
	//socket를 닫는다.
	if (m_hSocket)
	{
		::shutdown(m_hSocket, SD_BOTH);
		::closesocket(m_hSocket);
	}
	
	::WSACleanup();
	printf("*******ReleaseSocket() End*********\n");
}
//////////////////////////////////////////////////////////////////////////////////////
