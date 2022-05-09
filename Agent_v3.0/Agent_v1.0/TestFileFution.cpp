#include"stdafx.h"
#include"TestFileFution.h"
#include<ShlObj.h>
#include<strsafe.h>
#pragma comment(lib,"User32.lib")

#include"HashFuntion.h"
#include"ProcessControl.h"
#include"ClientSocket.h"

/////////////////////////////////////////////////////////////////////////////
//※※ Thread ※※
/////////////////////////////////////////////////////////////////////////////
HANDLE g_hEventEndMonitoring; //실험을 중간에 종료시키기 위한 Event
/////////////////////////////////////////////////////////////////////////////
DWORD WINAPI MonitoringFolder(LPVOID pParam)
{
	printf("************* MonitoringFolder() **************\n");
	//LPVOID == void* 여기에 void* 배열의 주소를 넣었다.
	//한 마디로 LPVOID = void**
	//(void**)[0] = void* <-근데 [0]과 [1]이 각각 CTestFileInfo*와 CClientSocekt*이다.
	CClientSocket* Client = (CClientSocket*)pParam;
	CTestFileInfo*	CTFInfo = Client->GetCTestFileInfo(); //※ 주소 값이 잘 넘어오나 확인

	CProcessControl CProcControl;
	HANDLE			hFind;
	
	//printf("Create a Stop Event\n");
	g_hEventEndMonitoring = CreateEvent(NULL, TRUE, FALSE, _T("EndMonitoring"));

	//파일 MAC time 초기화
	CTFInfo->InitFileMACtime();

	//감시하는 폴더 설정
	//FILE_NOTIFY_CHANGE_LAST_WRITE
	//이 flag는 파일을 열어서 편집할때, 파일을 닫을 때 반응한다.
	printf("C:\\Users\\cC_er\\Documents Start Monitoring\n");
	if ((hFind = FindFirstChangeNotification(CTFInfo->GetFolderPath(), FALSE, FILE_NOTIFY_CHANGE_LAST_WRITE)) == INVALID_HANDLE_VALUE)
	{
		ErrorHandler("FindFirstChangeNotification() Fail!!", true);
	}

	if (hFind == NULL)
	{
		ErrorHandler("The Handle Value is NULL.", true);
	}

	//프로그램 시작
	if (Client->GetTestState() == START_RASOM)
	{
		printf("!!RUN Rasomware!!\n");
		CProcControl.RunChildProcess(CTFInfo->GetRasomwarePath());
	}
	else if (Client->GetTestState() == START_RECOVERY)
	{
		printf("!!RUN Recovery!!\n");
		CProcControl.RunChildProcess(CTFInfo->GetRecoveryPath());
	}

	
	//처음 5초 동안 기다려 본다.(이때 동안 변화가 없으면 프로세스 종료)
	DWORD dwWaitExit = WaitForSingleObject(g_hEventEndMonitoring, 3000);

	//감시 시작
	while (true)
	{
		if (dwWaitExit == WAIT_OBJECT_0)
		{
			break;
		}

		printf("Let's Check.......\n\n");

		//몇개의 파일에 변화가 있었는 가?
		//FILE_NOTIFY_CHANGE_LAST_WRITE가 파일을 저장할 때 엑세스 할 때 모두 반응 하므로 필요
		CTFInfo->FindfilesChange();

		//변화한 파일 개수 전송
		Client->SendTestProgress();
		printf("=====(%zd / %zd)\n", CTFInfo->GetChageTestFileCount(), CTFInfo->GetTestFileCount());

		if (CTFInfo->GetChageTestFileCount() == 0)
		{
			printf("No Run Rasomware\n");
			Client->SendTestProgress();
			break;
		}
		else if (CTFInfo->GetChageTestFileCount() == CTFInfo->GetTestFileCount())
		{
			int count = 0;

			while (WaitForSingleObject(hFind, 0) == WAIT_OBJECT_0)
			{
				count++;
				if (FindNextChangeNotification(hFind) == FALSE)
				{
					ErrorHandler("FindNextChangeNotification() Fail !!", true);
					break;
				}
			}

			if (count == 0)
			{
				printf("ALL Changed\n");
				break;
			}
		}

		//0.01초
		dwWaitExit = WaitForSingleObject(g_hEventEndMonitoring, 500);
	}

	//실행 중인 프로그램 종료
	printf("!! Exit The Executable Program !!\n");
	if (Client->GetTestState() == START_RASOM)
	{
		CProcControl.TerminateChildProcess(CTFInfo->GetRasomwareName());
	}
	else if (Client->GetTestState() == START_RECOVERY)
	{
		CProcControl.TerminateChildProcess(CTFInfo->GetRecoveryName());
	}

	printf("Close Folder Detection Handle\n");
	//받은 handl 닫기
	FindCloseChangeNotification(hFind);
	//생성한 Evnet
	CloseHandle(g_hEventEndMonitoring);
	g_hEventEndMonitoring = 0;


	if (CTFInfo->GetChageTestFileCount() == CTFInfo->GetTestFileCount())
	{
		//실험 완료한 시간 전송
		if (Client->GetTestState() == START_RASOM)
		{
			printf("!! Send Infection End Time !!\n");
			Client->SendSignal(END_RASOM);
		}
		else if (Client->GetTestState() == START_RECOVERY)
		{
			printf("!! Send Recovery End Time !!\n");
			Client->SendSignal(END_RECOVERY);
		}
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////


CTestFileInfo::CTestFileInfo()
	:m_nChangeFileCount(0)
{
	//CSIDL_PERSONAL  = CSIDL_MYDOCUMENTS
	if (SHGetFolderPathW(NULL, CSIDL_MYDOCUMENTS, NULL, 0, m_FolderPath) != S_OK)
	{
		ErrorHandler("Not Found Folder To Be Monitored", false);
	}

	g_hEventEndMonitoring = 0;
}

CTestFileInfo::~CTestFileInfo()
{
	ReleaseTestFolderAndFile();
}

//파일 이름 파싱
void CTestFileInfo::ParsingFileName(TCHAR* ori, TCHAR* Name)
{
	//int size;
	TCHAR* temp;
	StringCchCopy(Name, sizeof(TCHAR)*MAX_PATH, ori);

	while (true)
	{
		temp = _tcsrchr(Name, _T('.'));
		if (temp == NULL)break;

		(*temp) = _T('\0');
	}
	
}

//HANDLE& hFind		: 처음에만 INVALID_HANDLE_VALUE 값을 넣은 handl을 넣어주고 다음 파일을 읽을 때는 함수 내부에서
//					  사용된 hFind를 그대로 매개변수로 넣어준다.
//WIN32_FIND_DATA& fileData : 원하는 경로의 폴더안의 파일의 데이터를 읽어온다.
//BOOL 모두 다 읽었거나 실패하면 FALSE이 반환된다.
BOOL CTestFileInfo::FindFilesInAFolder(HANDLE& hFind, WIN32_FIND_DATA& fileData)
{
	TCHAR szDir[MAX_PATH]; //실제 쓰일 경로
	size_t length_of_arg;
	BOOL isComplete = TRUE;

	//모든 file을 나타내기 위해 \*을 붙이고 \n을 붙여야하므로 3칸이 남겨 있어야한다.
	//문자열이 지정된 길이를 초과하는지 여부 판별
	//#TRUE
	StringCchLength(m_FolderPath, MAX_PATH, &length_of_arg);

	if (length_of_arg > (MAX_PATH - 3))
	{
		ErrorHandler("폴더 경로가 너무 길다.", false);
	}
	StringCchCopy(szDir, MAX_PATH, m_FolderPath);
	StringCchCat(szDir, MAX_PATH, _T("\\*"));

	//검색한 파일의 첫번째를 WIN32_FIND_DATA에 정보를 담아온다.
	if (hFind == INVALID_HANDLE_VALUE)
	{
		hFind = FindFirstFile(szDir, &fileData);

		if (hFind == INVALID_HANDLE_VALUE)
		{
			ErrorHandler("폴더의 handl을 가져오기를 실패했습니다.", false);
		}
	}
	else
	{
		isComplete = FindNextFile(hFind, &fileData);

		//이 파일이면 다음으로 넘어간다.
		if (_tcscmp(fileData.cFileName, _T("desktop.ini")) == 0)
		{
			isComplete = FindNextFile(hFind, &fileData);
		}
		
		if (isComplete == FALSE)
		{
			if (GetLastError() == ERROR_NO_MORE_FILES)
			{
				printf("No More File\n");
			}
			FindClose(hFind);
		}

	}

	return isComplete;
}


void CTestFileInfo::InitTestFolderAndFile(void)
{
	WIN32_FIND_DATA ffd;
	HANDLE hFind = INVALID_HANDLE_VALUE; //handle초기화
	TCHAR szDir[MAX_PATH];
	TCHAR FileName[MAX_PATH];

	_tprintf(_T("<Target Directory is %s >\n"), m_FolderPath);

	while (FindFilesInAFolder(hFind, ffd) != FALSE)
	{
		if ((ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0x00000000)
		{
			//이 안에서 file list를 만들면 됨
			FILE_INFO *temp = new FILE_INFO;
			//Hash 계산
			StringCchCopy(temp->FileName, MAX_PATH, ffd.cFileName);
			StringCchCopy(szDir, MAX_PATH, m_FolderPath);
			StringCchCat(szDir, MAX_PATH, _T("\\"));
			StringCchCat(szDir, MAX_PATH, temp->FileName);
			FileHashCalution(szDir, temp->OriginalHash);

			//파일 이름
			ParsingFileName(ffd.cFileName, FileName);
			m_mapTestFile.insert(std::pair<std::wstring, FILE_INFO*>(FileName, temp));
			m_vecTestFile.push_back(temp);

			_tprintf(_T("File : %s \n"), ffd.cFileName);
		}
	}
}

void CTestFileInfo::ReleaseTestFolderAndFile(void)
{
	for (unsigned int i = 0; i < m_vecTestFile.size(); i++)
	{
		delete m_vecTestFile[i];
	}
	m_vecTestFile.clear();
	m_mapTestFile.clear();
	m_mapFileLastWrite.clear();
}


//4. 파일들의 Mac시간을 가져와서 변화 감지
void CTestFileInfo::FindfilesChange(void)
{
	_tprintf(_T("## Monitoring File Last Write Time"));
	HANDLE hFind = INVALID_HANDLE_VALUE; //handle초기화
	WIN32_FIND_DATA ffd;
	TCHAR FileName[MAX_PATH];

	m_nChangeFileCount = 0;
	while (FindFilesInAFolder(hFind, ffd) != FALSE)
	{
		if ((ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0x00000000)
		{
			ParsingFileName(ffd.cFileName, FileName);
			if ((m_mapFileLastWrite[FileName].dwHighDateTime != ffd.ftLastWriteTime.dwHighDateTime) ||
				(m_mapFileLastWrite[FileName].dwLowDateTime != ffd.ftLastWriteTime.dwLowDateTime))
			{
				m_nChangeFileCount++;
			}
		}
	}
}

//파일 시간 모두 업데이트
void CTestFileInfo::InitFileMACtime(void)
{
	if (m_mapFileLastWrite.empty() == false)
	{
		m_mapFileLastWrite.clear();
	}
	m_nChangeFileCount = 0;

	WIN32_FIND_DATA ffd;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	TCHAR FileName[MAX_PATH];

	while (FindFilesInAFolder(hFind, ffd) != FALSE)
	{
		if ((ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0x00000000)
		{
			ParsingFileName(ffd.cFileName, FileName);
			m_mapFileLastWrite.insert(std::pair<std::wstring, _FILETIME>(FileName, ffd.ftLastWriteTime));
		}
	}
}

void CTestFileInfo::HashAfterRasomware(void)
{
	WIN32_FIND_DATA ffd;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	TCHAR FileName[MAX_PATH];
	TCHAR szDir[MAX_PATH];

	while (FindFilesInAFolder(hFind, ffd) != FALSE)
	{
		if ((ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0x00000000)
		{
			StringCchCopy(szDir, MAX_PATH, m_FolderPath);
			StringCchCat(szDir, MAX_PATH, _T("\\"));
			StringCchCat(szDir, MAX_PATH, ffd.cFileName);
			ParsingFileName(ffd.cFileName, FileName);
			FileHashCalution(szDir, m_mapTestFile[FileName]->InfectionHash);
		}
	}
}

void CTestFileInfo::HashAfterRecovery(void)
{
	WIN32_FIND_DATA ffd;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	TCHAR FileName[MAX_PATH];
	TCHAR szDir[MAX_PATH];

	while (FindFilesInAFolder(hFind, ffd) != FALSE)
	{
		if ((ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0x00000000)
		{
			StringCchCopy(szDir, MAX_PATH, m_FolderPath);
			StringCchCat(szDir, MAX_PATH, _T("\\"));
			StringCchCat(szDir, MAX_PATH, ffd.cFileName);
			ParsingFileName(ffd.cFileName, FileName);
			FileHashCalution(szDir, m_mapTestFile[FileName]->RecoveryHash);
		}
	}
	
}