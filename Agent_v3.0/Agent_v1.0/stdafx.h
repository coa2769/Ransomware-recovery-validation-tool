// stdafx.h : 자주 사용하지만 자주 변경되지는 않는
// 표준 시스템 포함 파일 또는 프로젝트 관련 포함 파일이
// 들어 있는 포함 파일입니다.
//

#pragma once

#include "targetver.h"

#include <tchar.h>
#include"..\..\Common\Common.h"

#include<WinSock2.h>
#include<WS2tcpip.h>
#pragma comment(lib,"ws2_32")
void ErrorHandler(char *str, bool isErrorCode);
