#pragma once

 

class CProcessControl
{
private:
	DWORD m_dwRunProcessID;

public:
	CProcessControl();
	~CProcessControl();


	//프로세스 실행
	//https://docs.microsoft.com/en-us/windows/desktop/api/processthreadsapi/nf-processthreadsapi-createprocessa
	void RunChildProcess(TCHAR* filePath);

	//프로세스가 실행 중인가?
	//https://docs.microsoft.com/en-us/windows/desktop/api/tlhelp32/nf-tlhelp32-createtoolhelp32snapshot
	//https://docs.microsoft.com/ko-kr/windows/desktop/api/tlhelp32/ns-tlhelp32-tagprocessentry32
	//https://docs.microsoft.com/ko-kr/windows/desktop/ToolHelp/traversing-the-module-list
	//https://docs.microsoft.com/ko-kr/windows/desktop/api/tlhelp32/ns-tlhelp32-tagmoduleentry32

	//https://hazeyun.tistory.com/52
	//http://www.borlandforum.com/impboard/impboard.dll?action=read&db=bcb_qna&no=72267
	BOOL IsProcessRunning(TCHAR* fileName);

	//프로그램 종료
	//https://docs.microsoft.com/ko-kr/windows/desktop/api/processthreadsapi/nf-processthreadsapi-terminateprocess
	//이런 방법도 있다.
	//https://artwook.tistory.com/123
	//http://www.digipine.com/index.php?mid=windowsmfc&document_srl=341
	void TerminateChildProcess(TCHAR* fileName);
};