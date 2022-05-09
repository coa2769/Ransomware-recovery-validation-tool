#include"stdafx.h"
#include"ProcessControl.h"
#include<TlHelp32.h>
#include<string>

CProcessControl::CProcessControl()
	:m_dwRunProcessID(0)
{
}
CProcessControl::~CProcessControl()
{
}


void CProcessControl::RunChildProcess(TCHAR* filePath)
{
	BOOL bSuccess;

	STARTUPINFO si = { 0 };
	si.cb = sizeof(STARTUPINFO);
	PROCESS_INFORMATION pi = { 0 };
	
	//ERROR 193 : CreateProcess는 exe파일만을 실행한다. txt, png등 파일을 실행 하려면 Shell함수를 써야한다.
	//우리가 보통 txt, png등의 파일을 더블 클릭 할 때 그 파일을 읽어들일 수 있는 실행 프로그램을 registry에서 찾는다.
	//Shell은 이걸 자동으로 해결해주고 CreateProcess는 파일을 읽어들 수 있는 실행 프로그램에 command인자를 넣어주면 가능하다.
	//ERROR 740 : admin권한이 필요한 애플리케이션을 시작.
	bSuccess = CreateProcess(
		filePath,
		NULL,
		NULL,
		NULL,
		FALSE,
		0,
		NULL,
		NULL,
		&si,
		&pi);

	if (bSuccess == FALSE)
	{
		ErrorHandler("프로세스 실행 실패", true);
	}

	m_dwRunProcessID = pi.dwProcessId;
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
}

//프로세스가 실행 중인가?
BOOL CProcessControl::IsProcessRunning(TCHAR* fileName)
{
	BOOL bNext;
	BOOL bIs = FALSE;

	//지금의 프로세스 스냅샷을 가져온다.
	//TH32CS_SNAPPROCESS : 스냅샷의 시스템에 있는 모든 프로세스를 가져온다.
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	
	std::wstring process;
	char buf[MAX_PATH] = { 0 };
	PROCESSENTRY32 ppe;
	ppe.dwSize = sizeof(PROCESSENTRY32);

	//첫 프로세스 정보를 가져온다.
	bNext = Process32First(hSnapshot, &ppe);

	while (bNext)
	{
		if (_tcscmp(ppe.szExeFile, fileName) == 0)
		{
			bIs = TRUE;
			break;
		}
		bNext = Process32Next(hSnapshot, &ppe);
	}

	CloseHandle(hSnapshot);
	return bIs;
}

void CProcessControl::TerminateChildProcess(TCHAR* fileName)
{
	//실행중이면 종료
	if (IsProcessRunning(fileName) == TRUE)
	{
		DWORD dwDesiredAccess = PROCESS_TERMINATE;
		BOOL bInHeritHandle = FALSE;
		//보안 프로그램에 의해 차단되어 있을 수 있다.
		HANDLE hProcess = OpenProcess(dwDesiredAccess, bInHeritHandle, m_dwRunProcessID);
		if (hProcess == NULL)
			return;

		BOOL result = TerminateProcess(hProcess, 0);

		if (result == FALSE)
		{
			_tprintf(_T("TerminateProcess() 실패"));
		}

		WaitForSingleObject(hProcess, INFINITE);
		m_dwRunProcessID = 0;

		CloseHandle(hProcess);
	}
}