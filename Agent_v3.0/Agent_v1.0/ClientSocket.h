#pragma once

class CTestFileInfo;

class CClientSocket
{
private:
	WSADATA m_wsa = { 0 };
	SOCKET m_hSocket;
	HEAD_TYPE m_TestState;
	CTestFileInfo* m_CInfo;
	BOOL m_bCloseHealthCheck;	//Health Check Thread를 나올 때

	void(*m_FuncReceive[((int)ALL_STOP) + 1])(CClientSocket* Client, TCP_H& Head);
public:
	explicit CClientSocket(CTestFileInfo& DB);
	~CClientSocket();

	//get 함수
	SOCKET GetSocket(void) { return m_hSocket; }
	HEAD_TYPE GetTestState(void) { return m_TestState; }
	BOOL GetCloseHealthCheck(void) { return m_bCloseHealthCheck; }
	CTestFileInfo* GetCTestFileInfo(void) { return m_CInfo; }
	
	//set 함수
	void SetTestState(HEAD_TYPE type) { m_TestState = type; }

	//Connet 함수
	void Connect(char* IP, unsigned short Port);

	//Recive(한 함수에)
	void Recive(void);
	void FileRecive(TCP_H& Head, TCHAR* FilePath);
	BOOL FileHashCheck(BYTE* OriHash, TCHAR *FilePath);

	// Send함수
	void SendSignal(HEAD_TYPE signal);
	//#1 감염 & 복구 시작 시간을 알림
	void SendRasomStart(void);
	void SendRecoveryStart(void);
	//#2 진행률 알림
	void SendTestProgress(void);
	//#3 결과 전송
	void SendTestResult(void);

	//해제
	void ReleaseSocket(void);

};

