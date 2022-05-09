#include"stdafx.h"
#include"HashFuntion.h"


BOOL SameValue(BYTE* fir, BYTE* scn)
{
	bool is = TRUE;
	for (int i = 0; i < 20; i++)
	{
		if (fir[i] != scn[i])
		{
			is = FALSE;
			break;
		}
	}
	return is;
}

/**
*@ brief 파일 내용의 Hash값을 구하는 
*@ param pFilePath	파일 주소를 입력받는다.
*@ param Hash		Hash값을 저장할 
*/
BOOL FileHashCalution(TCHAR *pFilePath, BYTE *Hash)
{
	//파일 열기
	//CreateFile() : 최소 지원 WindowXP, 파일 핸들을 할당하는 함수
	//https://docs.microsoft.com/en-us/windows/desktop/api/fileapi/nf-fileapi-createfilea
	//HANDLE CreateFileA(
	//	 LPCSTR lpFileName,									//파일 경로
	//	 DWORD dwDesiredAccess,								//파일의 액세스 타입
	//	 DWORD dwShareMode,									//파일의 공유 모드
	//	 LPSECURITY_ATTRIBUTES lpSecurityAttributes,		//보안에 관련된 구조체
	//	 DWORD dwCreationDisposition,						//파일을 어떤 방식으로 열것인가?
	//	 DWORD dwFlagsAndAttributes,						//파일에 대한 속성
	//	 HANDLE hTemplateFile								//파일의 추가 속성, 잘 사용되지 않음
	//);

	HANDLE hFile;
	if ((hFile = CreateFile(pFilePath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE)
	{
		printf("ERROR : Failed To Open File\n");
		return FALSE;
	}

	//파일 길이 알아오기
	//GetFileSize() : 최소 지원 WindowXP, 파일의 사이즈를 가져온다. 4GB이상은 인지하지 못한다.
	//에이전트가 32bit이고 큰 파일에 대한 처리를 지원하지 않을 것이다.
	DWORD nLength = 0;
	DWORD nResult = 0;
	if ((nLength = GetFileSize(hFile, NULL)) == INVALID_FILE_SIZE)
	{
		CloseHandle(hFile);
		printf("ERROR : Not Know The File Size\n");
		return FALSE;
	}

	BYTE *pBuffer = new BYTE[nLength];
	//파일 읽어오기
	if (ReadFile(hFile, pBuffer, sizeof(BYTE) * nLength, &nResult, NULL) == FALSE)
	{
		CloseHandle(hFile);
		printf("ERRPR : Failure File Read\n");
		return FALSE;
	}

	//winCrypto api
	HCRYPTPROV hProv;
	HCRYPTHASH hHash;
	DWORD dwHash = 0;

	//SHA-1 최대 160bit
	if (CryptAcquireContextW(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT) == FALSE)
	{
		CloseHandle(hFile);
		printf("ERROR : Failed To Create Object <CryptAcquire>\n");
		return FALSE;
	}

	if (CryptCreateHash(hProv, CALG_SHA1, 0, 0, &hHash) == FALSE)
	{
		CloseHandle(hFile);
		printf("ERROR : Failed To Create Object <Hash>\n");
		return FALSE;
	}

	if (CryptHashData(hHash, pBuffer, nLength, 0) == FALSE)
	{
		CloseHandle(hFile);
		printf("ERROR : failed Hash Calculation\n");
		return FALSE;
	}

	//최대 20byte, 파일의 크기가 작다면 그 값이 20byte보다 작을 수 있다.
	dwHash = 20;
	if (CryptGetHashParam(hHash, HP_HASHVAL, Hash, &dwHash, 0) == FALSE)
	{
		CloseHandle(hFile);
		printf("ERROR : Failed To Get Value\n");
		return FALSE;
	}

	if (hHash) CryptDestroyHash(hHash);
	if (hProv) CryptReleaseContext(hProv, 0);

	//파일 닫기
	CloseHandle(hFile);

	return TRUE;
}