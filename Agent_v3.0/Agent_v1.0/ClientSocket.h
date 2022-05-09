#pragma once

class CTestFileInfo;

class CClientSocket
{
private:
	WSADATA m_wsa = { 0 };
	SOCKET m_hSocket;
	HEAD_TYPE m_TestState;
	CTestFileInfo* m_CInfo;
	BOOL m_bCloseHealthCheck;	//Health Check Thread�� ���� ��

	void(*m_FuncReceive[((int)ALL_STOP) + 1])(CClientSocket* Client, TCP_H& Head);
public:
	explicit CClientSocket(CTestFileInfo& DB);
	~CClientSocket();

	//get �Լ�
	SOCKET GetSocket(void) { return m_hSocket; }
	HEAD_TYPE GetTestState(void) { return m_TestState; }
	BOOL GetCloseHealthCheck(void) { return m_bCloseHealthCheck; }
	CTestFileInfo* GetCTestFileInfo(void) { return m_CInfo; }
	
	//set �Լ�
	void SetTestState(HEAD_TYPE type) { m_TestState = type; }

	//Connet �Լ�
	void Connect(char* IP, unsigned short Port);

	//Recive(�� �Լ���)
	void Recive(void);
	void FileRecive(TCP_H& Head, TCHAR* FilePath);
	BOOL FileHashCheck(BYTE* OriHash, TCHAR *FilePath);

	// Send�Լ�
	void SendSignal(HEAD_TYPE signal);
	//#1 ���� & ���� ���� �ð��� �˸�
	void SendRasomStart(void);
	void SendRecoveryStart(void);
	//#2 ����� �˸�
	void SendTestProgress(void);
	//#3 ��� ����
	void SendTestResult(void);

	//����
	void ReleaseSocket(void);

};
