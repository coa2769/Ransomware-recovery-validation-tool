#pragma once
#include<wincrypt.h>
#pragma comment(lib,"Advapi32.lib")

//https://docs.microsoft.com/ko-kr/dotnet/standard/security/ensuring-data-integrity-with-hash-codes
//https://ratsgo.github.io/data%20structure&algorithm/2017/10/25/hash/
BOOL SameValue(BYTE* fir, BYTE* scn);
BOOL FileHashCalution(TCHAR *pFilePath, BYTE *Hash);