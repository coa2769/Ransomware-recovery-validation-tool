#pragma once
#include<wincrypt.h>
#pragma comment(lib,"Advapi32.lib")

//https://ratsgo.github.io/data%20structure&algorithm/2017/10/25/hash/
BOOL SameValue(BYTE* Fir, BYTE* Scn);
BOOL FileHashCalution(TCHAR *FilePath, BYTE *Hash);