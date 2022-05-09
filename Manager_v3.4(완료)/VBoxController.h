#pragma once
class CVBoxController
{
private:
	FILE* RunShellCommand(const TCHAR* command);

	CString m_CVBoxManagerPath;
public:
	CVBoxController();
	~CVBoxController();

	//VBoxManager가 있는가? 확인 후 path저장
	void Initialize(void);

	//list검사 & 각 TestOSInfo에 UUID정보 갱신
	void CheckVMList();
	//실행 list검사
	BOOL CheckBVMExecution(CString& UUID);
	//VM실행
	void ExevuteVM(CString& UUID);
	//VM종료
	void QuitVM(CString& UUID);
	//스냅샷이 있는가?
	BOOL HaveSnapshot(CString& UUID);
	//스냅샷 찍기
	void TakeSnapshot(CString& UUID);
	//스냅샷 복구
	void RestoreSnampshot(CString& UUID);

	void Release(void);
};

