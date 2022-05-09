#pragma once
#include<map>
#include<vector>
#include<string>

//종료 event
extern HANDLE g_hEventEndMonitoring;


//※※ 폴더 감시 함수(thread가 된다.)
//폴더 헨들 : https://docs.microsoft.com/en-us/windows/desktop/api/fileapi/nf-fileapi-findfirstchangenotificationa
//예제 : https://docs.microsoft.com/ko-kr/windows/desktop/FileIO/obtaining-directory-change-notifications
//대기함수에 대해 : https://docs.microsoft.com/ko-kr/windows/desktop/Sync/wait-functions
//이런 방법도 있다.
//https://www.benjaminlog.com/entry/ReadDirectoryChangesW
DWORD WINAPI MonitoringFolder(LPVOID pParam);
//DWORD WINAPI ThreadSaveHash(LPVOID pParam);

class CTestFileInfo
{
private:
	//hash_map -> 이제는 hash_map 대신 unordered_map가 사용된다.
	//C++언어 스펙이 바뀌면서 이렇게 되었는데 이런 부분은 OS버전에 상관 없는지 고민이다.
	//http://www.hanbit.co.kr/channel/category/category_view.html?cms_code=CMS4230438179
	//일단 map을 사용한다.
	
	//※ Test폴더의 파일들 구성
	std::vector<FILE_INFO*>				m_vecTestFile;
	std::map<std::wstring, FILE_INFO*>	m_mapTestFile;
	std::map<std::wstring, _FILETIME>	m_mapFileLastWrite; //파일 변경 시간 Accet는 window vista부터 적용이 안된다.  

	//<감시 폴더>
	//C:\Users\cC_er\Documents

	TCHAR m_FolderPath[MAX_PATH];	//테스트할 폴더
	size_t m_nChangeFileCount = 0;		//변경된 파일 개수
	
	TCHAR m_RasomwareName[MAX_PATH];
	TCHAR m_RecoveryName[MAX_PATH];
	TCHAR m_RansomwarePath[MAX_PATH];
	TCHAR m_RecoveryPath[MAX_PATH];
public:
	//함수
	CTestFileInfo();
	~CTestFileInfo();
	TCHAR* GetFolderPath(void) { return m_FolderPath; }
	TCHAR* GetRasomwarePath(void) { return m_RansomwarePath; }
	TCHAR* GetRasomwareName(void) { return m_RasomwareName; }
	TCHAR* GetRecoveryPath(void) { return m_RecoveryPath; }
	TCHAR* GetRecoveryName(void) { return m_RecoveryName; }
	size_t GetTestFileCount(void) { return m_vecTestFile.size(); }
	size_t GetChageTestFileCount(void) { return m_nChangeFileCount; }

	void GetTestFileInfo(int index, FILE_INFO& info)
	{
		FILE_INFO *temp = m_vecTestFile[index];
		memcpy_s(info.FileName, sizeof(wchar_t) * (MAX_PATH),temp->FileName, sizeof(wchar_t) * (MAX_PATH));
		memcpy_s(info.OriginalHash, sizeof(unsigned char) * 20, temp->OriginalHash, sizeof(unsigned char) * 20);
		memcpy_s(info.InfectionHash, sizeof(unsigned char) * 20, temp->InfectionHash, sizeof(unsigned char) * 20);
		memcpy_s(info.RecoveryHash, sizeof(unsigned char) * 20, temp->RecoveryHash, sizeof(unsigned char) * 20);
	}


	void SetRasomwareFilePath(TCHAR* path)
	{
		memcpy_s(m_RansomwarePath, sizeof(TCHAR) * MAX_PATH, path, sizeof(TCHAR) * MAX_PATH);
	}
	void SetRecoveryFilePath(TCHAR* path)
	{
		memcpy_s(m_RecoveryPath, sizeof(TCHAR) * MAX_PATH, path, sizeof(TCHAR) * MAX_PATH);

	}
	void SetRansomwareFileName(TCHAR* name)
	{
		memcpy_s(m_RasomwareName,sizeof(TCHAR) * MAX_PATH, name, sizeof(TCHAR) * MAX_PATH);
	}

	void SetRecoveryFileName(TCHAR* name)
	{
		memcpy_s(m_RecoveryName, sizeof(TCHAR) * MAX_PATH, name, sizeof(TCHAR) * MAX_PATH);
	}

	//파일 이름 파싱
	void ParsingFileName(TCHAR* ori, TCHAR* Name);

	//1. 특정 폴더 안의 파일 정보를 가져온다.
	BOOL FindFilesInAFolder(HANDLE& hFind, WIN32_FIND_DATA& fileData);
	//2. 관리할 파일들 이름에 따라 저장
	void InitTestFolderAndFile(void);
	//3. 모든 정보를 해제
	void ReleaseTestFolderAndFile(void);
	//4. 파일이 몇개가 변경되었는가? (MAC시간으로 판별)
	//http://www.tipssoft.com/bulletin/board.php?bo_table=FAQ&wr_id=14
	void FindfilesChange(void);
	//파일 시간 모두 업데이트
	void InitFileMACtime(void);
	//5. 파일 Hash값 정보 저장
	//void HashOriginal(void);
	void HashAfterRasomware(void);
	void HashAfterRecovery(void);

};
