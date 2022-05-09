#include"stdafx.h"
#include"HashFuntion.h"


BOOL SameValue(BYTE* Fir, BYTE* Scn)
{
	bool is = TRUE;
	for (int i = 0; i < 20; i++)
	{
		if (Fir[i] != Scn[i])
		{
			is = FALSE;
			break;
		}
	}
	return is;
}

BOOL FileHashCalution(TCHAR *FilePath, BYTE *Hash)
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

	HANDLE FileHandle;
	if ((FileHandle = CreateFile(FilePath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE)
	{
		ErrorHandler("Failed To Open File", true);
		return FALSE;
	}

	//파일 길이 알아오기
	//GetFileSize() : 최소 지원 WindowXP, 파일의 사이즈를 가져온다. 4GB이상은 인지하지 못한다.
	//에이전트가 32bit이고 큰 파일에 대한 처리를 지원하지 않을 것이다.
	DWORD Length = 0;
	DWORD Result = 0;
	if ((Length = GetFileSize(FileHandle, NULL)) == INVALID_FILE_SIZE)
	{
		CloseHandle(FileHandle);
		ErrorHandler("Not Know The File Size", true);
		return FALSE;
	}

	BYTE *pBuffer = new BYTE[Length];
	//파일 읽어오기
	if (ReadFile(FileHandle, pBuffer, sizeof(BYTE) * Length, &Result, NULL) == FALSE)
	{
		CloseHandle(FileHandle);
		ErrorHandler("Failure File Read", true);
		return FALSE;
	}

	//winCrypto api
	HCRYPTPROV ProvHandle;
	HCRYPTHASH HashHandle;
	DWORD HashLength = 0;

	//SHA-1 최대 160bit
	if (CryptAcquireContextW(&ProvHandle, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT) == FALSE)
	{
		CloseHandle(FileHandle);
		ErrorHandler("Failed To Create Object <CryptAcquire>", true);
		return FALSE;
	}

	if (CryptCreateHash(ProvHandle, CALG_SHA1, 0, 0, &HashHandle) == FALSE)
	{
		CloseHandle(FileHandle);
		ErrorHandler("Failed To Create Object <Hash>", true);
		return FALSE;
	}

	if (CryptHashData(HashHandle, pBuffer, Length, 0) == FALSE)
	{
		CloseHandle(FileHandle);
		ErrorHandler("failed Hash Calculation", true);
		return FALSE;
	}

	//최대 20byte, 파일의 크기가 작다면 그 값이 20byte보다 작을 수 있다.
	HashLength = 20;
	if (CryptGetHashParam(HashHandle, HP_HASHVAL, Hash, &HashLength, 0) == FALSE)
	{
		CloseHandle(FileHandle);
		ErrorHandler("Failed To Get Value",true);
		return FALSE;
	}

	if (HashHandle) CryptDestroyHash(HashHandle);
	if (ProvHandle) CryptReleaseContext(ProvHandle, 0);

	//파일 닫기
	CloseHandle(FileHandle);

	return TRUE;
}