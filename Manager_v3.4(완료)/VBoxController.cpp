#include "stdafx.h"
#include "VBoxController.h"
#include<io.h>
#include<iostream>	
using namespace std;

#include"Manager_v1.0.h"
#include"TestOSInfo.h"

CVBoxController::CVBoxController()
{
}


CVBoxController::~CVBoxController()
{
}

FILE* CVBoxController::RunShellCommand(const TCHAR* command)
{
	BOOL success = FALSE;

	//1. 파이프 생성
	//1-1. 보안 속성 설정(파이프 핸들 상속 여부)
	//https://wonjayk.tistory.com/271
	SECURITY_ATTRIBUTES securityAttr = { 0 };
	securityAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
	securityAttr.bInheritHandle = TRUE;			//상속가능한 Handle이라는 뜻.
	securityAttr.lpSecurityDescriptor = NULL;	//접근 권한

	/*=======================================================*
	*                                                        *
	*  부모 자식 프로세스간 연결되는 파이프 구조               *
	*                                                        *
	*  Parent                                         Child  *
	*  +------+                                    +------+  *
	*  |  hParentWriteToChild  ---> hChildReadFromParent  |  *
	*  |      |                                    |      |  *
	*  |  hParentReadFromChild <--- hChildWriteToParent   |  *
	*  +------+                                    +------+  *
	*                                                        *
	*========================================================*/

	//1-2. 단방향 파이프 생성
	//https://artisticbit.tistory.com/entry/08%EC%9E%A5-%ED%94%84%EB%A1%9C%EC%84%B8%EC%8A%A4%EA%B0%84-%ED%86%B5%EC%8B%A0IPC-2-%E2%91%A1%ED%8C%8C%EC%9D%B4%ED%94%84-%EB%B0%A9%EC%8B%9D%EC%9D%98-IPC
	HANDLE hParentReadFromChild = NULL;
	HANDLE hChildWriteToParent = NULL;
	success = CreatePipe(&hParentReadFromChild, &hChildWriteToParent, &securityAttr, 0);
	if (success == FALSE)
	{
		printf("ERROR : 파이프 생성 실패\n");
		return NULL;
	}

	//1-3. 핸들의 특정 속성을 설정합니다.
	//자식 프로세스에게 개체 핸들을 상속할 수 있다.
	//https://docs.microsoft.com/ko-kr/windows/desktop/api/handleapi/nf-handleapi-sethandleinformation
	success = SetHandleInformation(hParentReadFromChild, HANDLE_FLAG_INHERIT, 0);
	if (success == FALSE)
	{
		printf("ERROR : 핸들 속성 변경에 실패\n");
		return NULL;
	}

	//2. 자식 프로세스 생성
	//2-1. 자식 프로세스의 표준 입력, 출력, 에러출력 핸들 설정
	STARTUPINFO startupInfo = { 0 };
	startupInfo.cb = sizeof(STARTUPINFO);
	startupInfo.hStdError = hChildWriteToParent;
	startupInfo.hStdOutput = hChildWriteToParent;
	startupInfo.hStdInput = hChildWriteToParent;
	startupInfo.dwFlags |= STARTF_USESTDHANDLES;

	PROCESS_INFORMATION processInfo = { 0 };

	//2-2. 자식 프로세스 생성
	success = CreateProcess(
		NULL,
		(LPTSTR)command,
		NULL,
		NULL,
		TRUE,
		DETACHED_PROCESS,
		NULL,
		NULL,
		&startupInfo,
		&processInfo);
	//자식 프로세스의console을 생성하지 않는 방법
	//DWORD dwCreationFlags		DETACHED_PROCESS이 flag를 준다.

	CloseHandle(hChildWriteToParent);

	if (success == FALSE)
	{
		printf("ERROR : 프로세스 실행 실패\n");
		return NULL;
	}

	//3. 핸들 반환
	//https://docs.microsoft.com/ko-kr/cpp/c-runtime-library/reference/open-osfhandle?view=vs-2017
	//파이프 핸들 -> 파일 핸들
	const int fd = _open_osfhandle((intptr_t)hParentReadFromChild, 0);
	return _wfdopen(fd, _T("r"));

	//CloseHandle(hParentReadFromChild);
}

//VBoxManager가 있는가? 확인 후 path저장
void CVBoxController::Initialize(void)
{
	//VBoxManage 주소
	CString Findfile;
	TCHAR path[MAX_PATH] = { 0 };
	ExpandEnvironmentStrings(_T("%ProgramW6432%"), path, sizeof(TCHAR)*MAX_PATH);
	Findfile += path;
	Findfile += _T("\\Oracle\\VirtualBox\\VBoxManage.exe");

	//파일 있는지 확인
	//exists 존재하다.
	const int existsFile = _taccess(Findfile, 0x00);
	if (existsFile != 0)
	{
		printf("ERROR : VBoxManage가 없습니다.\n");
		ErrorHandler(_T("ERROR : VBoxManage가 없습니다. Virtualbox가 설치 됐는지 확인해 주세요.\n"), TRUE);
	}

	m_CVBoxManagerPath = _T("\"");
	m_CVBoxManagerPath += Findfile;
	m_CVBoxManagerPath += _T("\"");

}

//fget
//https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/fgets-fgetws?view=vs-2017
//readfile함수
//https://docs.microsoft.com/en-us/windows/desktop/api/fileapi/nf-fileapi-readfile
//https://blog.naver.com/power2845/50144349557
//그 외
//https://wwwi.tistory.com/218
//CString 파싱에 쓰일 함수
//https://shaeod.tistory.com/331

//list검사 & 각 TestOSInfo에 UUID정보 갱신
void CVBoxController::CheckVMList()
{
	//명령어 구성
	CString command;
	command += m_CVBoxManagerPath;
	command += _T(" list -s vms");

	//출력
	_tprintf(_T("%s\n"), command.GetBuffer(0));
	//명령어 실행
	FILE* fp = RunShellCommand(command);
	
	//결과 출력 & 비교
	BOOL success = FALSE;
	char buffer[1024] = { 0 };
	int index = 0;
	CString UUID;
	CString Name;
	while (fgets(buffer, sizeof(char) * 1024, fp) != NULL)
	{
		printf("%s\n", buffer);

		UUID = buffer;
		index = UUID.Find('{');
		Name = UUID.Left(index - 1);
		UUID = UUID.Mid(index);
		index = UUID.Find('}');
		UUID = UUID.Left(index + 1);

		if (Name == _T("\"TestWin7\""))
		{
			theApp.m_mapTestOsInfo[WINDOW_7]->SetUUID(UUID);
			theApp.m_mapTestOsInfo[WINDOW_7]->SetState(STATE_ACTIVE);
		}
		else if (Name == _T("\"TestWin81\""))
		{
			theApp.m_mapTestOsInfo[WINDOW_8]->SetUUID(UUID);
			theApp.m_mapTestOsInfo[WINDOW_8]->SetState(STATE_ACTIVE);
		}
		else if (Name == _T("\"TestWin10\""))
		{
			theApp.m_mapTestOsInfo[WINDOW_10]->SetUUID(UUID);
			theApp.m_mapTestOsInfo[WINDOW_10]->SetState(STATE_ACTIVE);
		}
	}
	
	
	fclose(fp);
	cout << endl;
}

//실행 list검사
BOOL CVBoxController::CheckBVMExecution(CString& UUID)
{
	//명령어 구성
	CString command;
	command += m_CVBoxManagerPath;
	command += _T(" list runningvms");

	//출력
	_tprintf(_T("%s\n"), command.GetBuffer(0));

	//명령어 실행
	FILE* fp = RunShellCommand(command);

	//결과 출력 & 비교
	BOOL success = FALSE;
	char buffer[1024] = { 0 };
	int index = 0;
	CString CStrUUID;

	while (fgets(buffer, sizeof(char) * 1024, fp) != NULL)
	{
		printf("%s\n", buffer);

		CStrUUID = buffer;
		index = CStrUUID.Find('{');
		CStrUUID = CStrUUID.Mid(index);
		index = CStrUUID.Find('}');
		CStrUUID = CStrUUID.Left(index + 1);
		if (CStrUUID == UUID)
		{
			success = TRUE;
			break;
		}
	}

	cout << endl;

	fclose(fp);
	return success;

}

//VM실행
void CVBoxController::ExevuteVM(CString& UUID)
{
	//명령어 구성
	CString command;
	command += m_CVBoxManagerPath;
	command += _T(" startvm ");
	command += UUID;
	command += _T(" --type headless");

	//출력
	_tprintf(_T("%s\n"), command.GetBuffer(0));

	//명령어 실행
	FILE* fp = RunShellCommand(command);

	char buffer[1024] = { 0 };
	while (fgets(buffer, sizeof(char) * 1024, fp) != NULL)
	{
		printf("%s\n", buffer);
	}

}

void CVBoxController::QuitVM(CString& UUID)
{
	//명령어 구성
	CString command;
	command += m_CVBoxManagerPath;
	command += _T(" controlvm ");
	command += UUID;
	command += _T(" poweroff");

	//출력
	_tprintf(_T("%s\n"), command.GetBuffer(0));

	//명령어 실행
	FILE* fp = RunShellCommand(command);

	char buffer[1024] = { 0 };
	while (fgets(buffer, sizeof(char) * 1024, fp) != NULL)
	{
		printf("%s\n", buffer);
	}
}

//스냅샷 찍기
void CVBoxController::TakeSnapshot(CString& UUID)
{
	//명령어 구성
	CString command;
	command += m_CVBoxManagerPath;
	command += _T(" snapshot ");
	command += UUID;
	command += _T(" take \"first\"");

	//출력
	_tprintf(_T("%s\n"), command.GetBuffer(0));

	//명령어 실행
	FILE* fp = RunShellCommand(command);

	char buffer[1024] = { 0 };
	while (fgets(buffer, sizeof(char) * 1024, fp) != NULL)
	{
		printf("%s\n", buffer);
	}

}

//스냅샷 복구
void CVBoxController::RestoreSnampshot(CString& UUID)
{
	//명령어 구성
	CString command;
	command += m_CVBoxManagerPath;
	command += _T(" snapshot ");
	command += UUID;
	command += _T(" restorecurrent");

	//출력
	_tprintf(_T("%s\n"), command.GetBuffer(0));

	//명령어 실행
	FILE* fp = RunShellCommand(command);

	char buffer[1024] = { 0 };
	while (fgets(buffer, sizeof(char) * 1024, fp) != NULL)
	{
		printf("%s\n", buffer);
	}
}

//스냅샷이 있는가?
BOOL CVBoxController::HaveSnapshot(CString& UUID)
{
	//명령어 구성
	CString command;
	command += m_CVBoxManagerPath;
	command += _T(" snapshot ");
	command += UUID;
	command += _T(" list");

	//출력
	_tprintf(_T("%s\n"), command.GetBuffer(0));

	//명령어 실행
	FILE* fp = RunShellCommand(command);
	
	size_t count = 0;
	unsigned char buffer[1024 + 1] = { 0 };
	while (!feof(fp))
	{
		count = fread(buffer, 1, sizeof(char) * 1024, fp);
		buffer[count] = '\0';
		cout << buffer;
	}

	return TRUE;
}

//해제
void CVBoxController::Release(void)
{
	//명령어 구성
	CString command;
	command += m_CVBoxManagerPath;
	command += _T(" list runningvms");

	//출력
	_tprintf(_T("%s\n"), command.GetBuffer(0));

	//명령어 실행
	FILE* fp = RunShellCommand(command);

	//결과 출력 & 비교
	BOOL success = FALSE;
	char buffer[1024] = { 0 };
	int index = 0;
	int count = 0;
	CString CStrUUID[WINDOW_10 + 1];
	CStrUUID[0] = _T("");
	CStrUUID[1] = _T("");
	CStrUUID[2] = _T("");

	while (fgets(buffer, sizeof(char) * 1024, fp) != NULL)
	{
		printf("%s\n", buffer);

		CStrUUID[count] = buffer;
		index = CStrUUID[count].Find('{');
		CStrUUID[count] = CStrUUID[count].Mid(index);
		index = CStrUUID[count].Find('}');
		CStrUUID[count] = CStrUUID[count].Left(index + 1);

		count++;
	}

	cout << endl;

	fclose(fp);

	for (int i = 0; i < (WINDOW_10 + 1); i++)
	{
		if (CStrUUID[i] != _T(""))
		{
			this->QuitVM(CStrUUID[i]);
			this->RestoreSnampshot(CStrUUID[i]);
		}
	}
}