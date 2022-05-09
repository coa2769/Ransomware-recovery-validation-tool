#include"stdafx.h"
#include"TestFileFution.h"
#include<ShlObj.h>
#include<strsafe.h>
#pragma comment(lib,"User32.lib")

#include"HashFuntion.h"
#include"ProcessControl.h"
#include"ClientSocket.h"

/////////////////////////////////////////////////////////////////////////////
//�ء� Thread �ء�
/////////////////////////////////////////////////////////////////////////////
HANDLE g_hEventEndMonitoring; //������ �߰��� �����Ű�� ���� Event
/////////////////////////////////////////////////////////////////////////////
DWORD WINAPI MonitoringFolder(LPVOID pParam)
{
	printf("************* MonitoringFolder() **************\n");
	//LPVOID == void* ���⿡ void* �迭�� �ּҸ� �־���.
	//�� ����� LPVOID = void**
	//(void**)[0] = void* <-�ٵ� [0]�� [1]�� ���� CTestFileInfo*�� CClientSocekt*�̴�.
	CClientSocket* Client = (CClientSocket*)pParam;
	CTestFileInfo*	CTFInfo = Client->GetCTestFileInfo(); //�� �ּ� ���� �� �Ѿ���� Ȯ��

	CProcessControl CProcControl;
	HANDLE			hFind;
	
	//printf("Create a Stop Event\n");
	g_hEventEndMonitoring = CreateEvent(NULL, TRUE, FALSE, _T("EndMonitoring"));

	//���� MAC time �ʱ�ȭ
	CTFInfo->InitFileMACtime();

	//�����ϴ� ���� ����
	//FILE_NOTIFY_CHANGE_LAST_WRITE
	//�� flag�� ������ ��� �����Ҷ�, ������ ���� �� �����Ѵ�.
	printf("C:\\Users\\cC_er\\Documents Start Monitoring\n");
	if ((hFind = FindFirstChangeNotification(CTFInfo->GetFolderPath(), FALSE, FILE_NOTIFY_CHANGE_LAST_WRITE)) == INVALID_HANDLE_VALUE)
	{
		ErrorHandler("FindFirstChangeNotification() Fail!!", true);
	}

	if (hFind == NULL)
	{
		ErrorHandler("The Handle Value is NULL.", true);
	}

	//���α׷� ����
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

	
	//ó�� 5�� ���� ��ٷ� ����.(�̶� ���� ��ȭ�� ������ ���μ��� ����)
	DWORD dwWaitExit = WaitForSingleObject(g_hEventEndMonitoring, 3000);

	//���� ����
	while (true)
	{
		if (dwWaitExit == WAIT_OBJECT_0)
		{
			break;
		}

		printf("Let's Check.......\n\n");

		//��� ���Ͽ� ��ȭ�� �־��� ��?
		//FILE_NOTIFY_CHANGE_LAST_WRITE�� ������ ������ �� ������ �� �� ��� ���� �ϹǷ� �ʿ�
		CTFInfo->FindfilesChange();

		//��ȭ�� ���� ���� ����
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

		//0.01��
		dwWaitExit = WaitForSingleObject(g_hEventEndMonitoring, 500);
	}

	//���� ���� ���α׷� ����
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
	//���� handl �ݱ�
	FindCloseChangeNotification(hFind);
	//������ Evnet
	CloseHandle(g_hEventEndMonitoring);
	g_hEventEndMonitoring = 0;


	if (CTFInfo->GetChageTestFileCount() == CTFInfo->GetTestFileCount())
	{
		//���� �Ϸ��� �ð� ����
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

//���� �̸� �Ľ�
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

//HANDLE& hFind		: ó������ INVALID_HANDLE_VALUE ���� ���� handl�� �־��ְ� ���� ������ ���� ���� �Լ� ���ο���
//					  ���� hFind�� �״�� �Ű������� �־��ش�.
//WIN32_FIND_DATA& fileData : ���ϴ� ����� �������� ������ �����͸� �о�´�.
//BOOL ��� �� �о��ų� �����ϸ� FALSE�� ��ȯ�ȴ�.
BOOL CTestFileInfo::FindFilesInAFolder(HANDLE& hFind, WIN32_FIND_DATA& fileData)
{
	TCHAR szDir[MAX_PATH]; //���� ���� ���
	size_t length_of_arg;
	BOOL isComplete = TRUE;

	//��� file�� ��Ÿ���� ���� \*�� ���̰� \n�� �ٿ����ϹǷ� 3ĭ�� ���� �־���Ѵ�.
	//���ڿ��� ������ ���̸� �ʰ��ϴ��� ���� �Ǻ�
	//#TRUE
	StringCchLength(m_FolderPath, MAX_PATH, &length_of_arg);

	if (length_of_arg > (MAX_PATH - 3))
	{
		ErrorHandler("���� ��ΰ� �ʹ� ���.", false);
	}
	StringCchCopy(szDir, MAX_PATH, m_FolderPath);
	StringCchCat(szDir, MAX_PATH, _T("\\*"));

	//�˻��� ������ ù��°�� WIN32_FIND_DATA�� ������ ��ƿ´�.
	if (hFind == INVALID_HANDLE_VALUE)
	{
		hFind = FindFirstFile(szDir, &fileData);

		if (hFind == INVALID_HANDLE_VALUE)
		{
			ErrorHandler("������ handl�� �������⸦ �����߽��ϴ�.", false);
		}
	}
	else
	{
		isComplete = FindNextFile(hFind, &fileData);

		//�� �����̸� �������� �Ѿ��.
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
	HANDLE hFind = INVALID_HANDLE_VALUE; //handle�ʱ�ȭ
	TCHAR szDir[MAX_PATH];
	TCHAR FileName[MAX_PATH];

	_tprintf(_T("<Target Directory is %s >\n"), m_FolderPath);

	while (FindFilesInAFolder(hFind, ffd) != FALSE)
	{
		if ((ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0x00000000)
		{
			//�� �ȿ��� file list�� ����� ��
			FILE_INFO *temp = new FILE_INFO;
			//Hash ���
			StringCchCopy(temp->FileName, MAX_PATH, ffd.cFileName);
			StringCchCopy(szDir, MAX_PATH, m_FolderPath);
			StringCchCat(szDir, MAX_PATH, _T("\\"));
			StringCchCat(szDir, MAX_PATH, temp->FileName);
			FileHashCalution(szDir, temp->OriginalHash);

			//���� �̸�
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


//4. ���ϵ��� Mac�ð��� �����ͼ� ��ȭ ����
void CTestFileInfo::FindfilesChange(void)
{
	_tprintf(_T("## Monitoring File Last Write Time"));
	HANDLE hFind = INVALID_HANDLE_VALUE; //handle�ʱ�ȭ
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

//���� �ð� ��� ������Ʈ
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