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
//�ء� Thread �ء�
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

		//�и����� ���� ����.
		//1000ms = 1s
		::Sleep(1000); 
	}

	printf("********** END Health Check ************\n");

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
//Lockup table�� ���� Receive
//#1 ���ӵ� ���� �˷���
void ReceiveAccpetAgent(CClientSocket* Client, TCP_H& Head)
{
	//OS�� ������ �˷��ش�.
	printf("Agent Connection Complete!!\n");

	//Thread����
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

//#2 ���� ����(Ransom, recovery)
void ReceiveFileRasom(CClientSocket* Client, TCP_H& Head)
{
	printf("*********Receive Rasomware**********\n");
	//���� �ޱ�
	TCHAR FilePath[MAX_PATH];
	memset(FilePath, 0, sizeof(TCHAR)*MAX_PATH);
	//���⼭ ���� ��ΰ� ���´�.
	Client->FileRecive(Head, FilePath);
	//hash�� �˻�
	if (Client->FileHashCheck(Head.FileInfo.OriginalHash, FilePath) == TRUE)
	{
		//�� �޾����� �̸� ����
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
	//���� �ޱ�
	TCHAR FilePath[MAX_PATH];
	memset(FilePath, 0, sizeof(TCHAR)*MAX_PATH);
	Client->FileRecive(Head, FilePath); //���⼭ ���� ��ΰ� ���´�.
								//hash�� �˻�
	if (Client->FileHashCheck(Head.FileInfo.OriginalHash, FilePath) == TRUE)
	{
		//�� �޾����� �̸� ����
		Client->GetCTestFileInfo()->SetRecoveryFilePath(FilePath);
		Client->GetCTestFileInfo()->SetRecoveryFileName(Head.FileInfo.FileName);
		Client->SendSignal(FILE_RECOVERY);
	}
	else
	{
		Client->SendSignal(FILE_RECOVERY_ERROR);
	}
}

//#3 Test���� ��ȣ�� ����
void ReceiveStartRasomware(CClientSocket* Client, TCP_H& Head)
{
	printf("Start Rasomware!!\n");
	Client->SetTestState(START_RASOM);
	//File ���ÿ� ���α׷� ����(Thread)
	DWORD dwThreadID = 1;
	HANDLE hThread = ::CreateThread(
		NULL,
		0,
		MonitoringFolder,
		Client,
		0,
		&dwThreadID);
	::CloseHandle(hThread);
	//���� ��ȣ ������
	Client->SendRasomStart();
}

//#4 Recovery ����
void ReceiveStartRecovery(CClientSocket* Client, TCP_H& Head)
{
	printf("Start Recovery!!\n");
	Client->SetTestState(START_RECOVERY);
	//File ���ÿ� ���α׷� ����(Thread)
	DWORD dwThreadID = 2;
	HANDLE hThread = ::CreateThread(
		NULL,
		0,
		MonitoringFolder,
		Client,
		0,
		&dwThreadID);
	::CloseHandle(hThread);
	//���� ��ȣ ������
	Client->SendRecoveryStart();
}

//Rasomware Hash�� �����Ѵ�.
void ReceiveSaveRasomwareHash(CClientSocket* Client, TCP_H& Head)
{
	printf("Save Rasomware Hash!!\n");
	Client->GetCTestFileInfo()->HashAfterRasomware();
	Client->SendSignal(SAVE_RASOMWARE_HASH);
}

//Recovery Hash�� �����Ѵ�.
void ReceiveSaveRecoveryHash(CClientSocket* Client, TCP_H& Head)
{
	printf("Save Recovery Hash!!\n");
	Client->GetCTestFileInfo()->HashAfterRecovery();
	Client->SendSignal(SAVE_RECOVERY_HASH);
}

//����� ����
void ReceiveTestResult(CClientSocket* Client, TCP_H& Head)
{
	printf("Send Result!!\n");
	Client->SendTestResult(); //���ο��� for�� ����
}

/////////////////////////////////////////////////////////////////////////////
// Class�Լ�
CClientSocket::CClientSocket(CTestFileInfo& DB)
	:m_CInfo(&DB),
	m_bCloseHealthCheck(FALSE)
{
	//�ٸ� �ʱ�ȭ ���
	//https://girtowin.tistory.com/107
	//https://docs.microsoft.com/en-us/windows/desktop/sync/using-critical-section-objects
	::InitializeCriticalSection(&g_csSend);

	//�Լ� ������ �ʱ�ȭ
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


	//1. ���� �ʱ�ȭ
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

//Connect �Լ�
void CClientSocket::Connect(char* IP, unsigned short Port)
{
	printf("*****START Connect******\n");

	//�ءءءءءءءءءءء� ���� ���� �ءءءءءءءءءءء�
	//2. ������ ������ ���� ����
	m_hSocket = ::socket(AF_INET, SOCK_STREAM, 0);
	if (m_hSocket == INVALID_SOCKET)
	{
		::WSACleanup();
		ErrorHandler("Could Not Create Socket", false);
	}

	//3. ��Ʈ ���ε� �� ����
	SOCKADDR_IN svraddr = { 0 };
	svraddr.sin_family = AF_INET;
	svraddr.sin_port = htons(Port);
	//https://techlog.gurucat.net/317
	//https://docs.microsoft.com/en-us/windows/desktop/api/ws2tcpip/nf-ws2tcpip-inetptonw
	//inet_addr()��� inet_pton()�Լ��� ����ؾ� �Ѵ�.(WS2tcpip.h ����)
	inet_pton(AF_INET, IP, &svraddr.sin_addr.S_un.S_addr);

	if (::connect(m_hSocket, (SOCKADDR*)&svraddr, sizeof(svraddr)) == SOCKET_ERROR)
	{
		::WSACleanup();
		ErrorHandler("Unable To Connect To Server", false);
	}

	printf("*****End Connect******\n");
}

////////////////////////////////////////////////////////////////////////////////
//Recive�Լ�
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
			printf("���� ũ�� : %d", nRecv);
			printf("Head�� �̻��ϴ� (Head : %d) ���� : %zd, %zd, %p)\n", Head.type, Head.TestFileCount, Head.ResultCount, m_FuncReceive[Head.type]);
		}
		//# Test ���� ��ȣ�� ����(Test�� �̰� ������� ����)[2]
		else if (Head.type == ALL_STOP)
		{
			//�� While������ ������.
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

	//���� ���� �ּ� �����
	//https://docs.microsoft.com/ko-kr/windows/desktop/api/shlobj_core/nf-shlobj_core-shgetfolderpatha
	//https://msdn.microsoft.com/ko-kr/33d92271-2865-4ebd-b96c-bf293deb4310
	if (SHGetFolderPathW(NULL, CSIDL_DESKTOP, NULL, 0, FilePath) != S_OK)
	{
		ErrorHandler("Could Not Find Folder To Save", false);
	}
	//���� ���
	StringCchCat(FilePath, MAX_PATH, _T("\\"));
	StringCchCat(FilePath, MAX_PATH, Head.FileInfo.FileName);

	//���� �����
	hFile = ::CreateFileW(
		FilePath,
		GENERIC_WRITE,
		0,
		NULL,
		CREATE_ALWAYS,	//������ ������ �����Ѵ�.
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
			//�������� ���� ũ�⸸ŭ �����͸� ���Ͽ� ����.
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
// Send�Լ� Ȯ�ο� ����
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
//Send�Լ���
//#1 ���� & ���� ���� �ð��� �˸�
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

//#2 ����� �˸�[2]
void CClientSocket::SendTestProgress(void)
{
	printf("*****Start SendTestProgress()*******\n");
	//release����� ������ �ƴ϶�� ����
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

//#3 ��� ����[2]
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
	//��� ����
	printf("*****End SendTestResult()*******\n");
}

////////////////////////////////////////////////////////////////////////////////
//����
void CClientSocket::ReleaseSocket(void)
{
	printf("*******ReleaseSocket() Start*********\n");

	m_bCloseHealthCheck = TRUE;
	//socket�� �ݴ´�.
	if (m_hSocket)
	{
		::shutdown(m_hSocket, SD_BOTH);
		::closesocket(m_hSocket);
	}
	
	::WSACleanup();
	printf("*******ReleaseSocket() End*********\n");
}
//////////////////////////////////////////////////////////////////////////////////////