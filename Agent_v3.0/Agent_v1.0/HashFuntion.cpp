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
	//���� ����
	//CreateFile() : �ּ� ���� WindowXP, ���� �ڵ��� �Ҵ��ϴ� �Լ�
	//https://docs.microsoft.com/en-us/windows/desktop/api/fileapi/nf-fileapi-createfilea
	//HANDLE CreateFileA(
	//	 LPCSTR lpFileName,									//���� ���
	//	 DWORD dwDesiredAccess,								//������ �׼��� Ÿ��
	//	 DWORD dwShareMode,									//������ ���� ���
	//	 LPSECURITY_ATTRIBUTES lpSecurityAttributes,		//���ȿ� ���õ� ����ü
	//	 DWORD dwCreationDisposition,						//������ � ������� �����ΰ�?
	//	 DWORD dwFlagsAndAttributes,						//���Ͽ� ���� �Ӽ�
	//	 HANDLE hTemplateFile								//������ �߰� �Ӽ�, �� ������ ����
	//);

	HANDLE FileHandle;
	if ((FileHandle = CreateFile(FilePath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE)
	{
		ErrorHandler("Failed To Open File", true);
		return FALSE;
	}

	//���� ���� �˾ƿ���
	//GetFileSize() : �ּ� ���� WindowXP, ������ ����� �����´�. 4GB�̻��� �������� ���Ѵ�.
	//������Ʈ�� 32bit�̰� ū ���Ͽ� ���� ó���� �������� ���� ���̴�.
	DWORD Length = 0;
	DWORD Result = 0;
	if ((Length = GetFileSize(FileHandle, NULL)) == INVALID_FILE_SIZE)
	{
		CloseHandle(FileHandle);
		ErrorHandler("Not Know The File Size", true);
		return FALSE;
	}

	BYTE *pBuffer = new BYTE[Length];
	//���� �о����
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

	//SHA-1 �ִ� 160bit
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

	//�ִ� 20byte, ������ ũ�Ⱑ �۴ٸ� �� ���� 20byte���� ���� �� �ִ�.
	HashLength = 20;
	if (CryptGetHashParam(HashHandle, HP_HASHVAL, Hash, &HashLength, 0) == FALSE)
	{
		CloseHandle(FileHandle);
		ErrorHandler("Failed To Get Value",true);
		return FALSE;
	}

	if (HashHandle) CryptDestroyHash(HashHandle);
	if (ProvHandle) CryptReleaseContext(ProvHandle, 0);

	//���� �ݱ�
	CloseHandle(FileHandle);

	return TRUE;
}