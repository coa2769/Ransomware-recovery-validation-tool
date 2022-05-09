#pragma once
////////////////////////////////////////////////////////////////////////////////////////////
//※※ CEvent ※※
extern CEvent g_CEventRecv[WINDOW_10 + 1];
extern CEvent g_CEventConnet[WINDOW_10 + 1];

UINT ThreadCommunication(LPVOID pParam);

class CServerSocket
{
private:
	WSADATA				m_wsa;
	SOCKET				m_hSocket;

	CMap<TEST_OS, TEST_OS, SOCKET, SOCKET>	m_mapClnSocket;
	void InitAcceptSocket(SOCKET& hSocket, unsigned short Port, BOOL isHealth);

public:
	CServerSocket();
	~CServerSocket();

	//Lookup table
	void(*m_FuncReceive[(int)(ALL_STOP + 1)])(TEST_OS os, TCP_H& Head);

	SOCKET getSocket(void) { return m_hSocket; }
	SOCKET getClnSocket(TEST_OS os) { return m_mapClnSocket[os]; }

	//client추가(내부에서 OS를 나눈다.)
	TEST_OS AddCln(SOCKADDR_IN& clnaddr, SOCKET& hSocekt);
	//client삭제
	void RemoveClient(TEST_OS os) { m_mapClnSocket.RemoveKey(os); }
	//Error로 인한 종료
	void TermicationSession(TEST_OS os, LPCTSTR lpszText);

	//접속 함수
	void AcceptFuntion(void);
	//send 함수
	void SendSignal(TEST_OS os, HEAD_TYPE signal);
	void SendRansomware(TEST_OS os); 
	void SendRecovery(TEST_OS os);	  
	
	void Release(void);
};

