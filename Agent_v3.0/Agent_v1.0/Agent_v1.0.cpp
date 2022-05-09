// Agent_v1.0.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include"TestFileFution.h"
#include"ClientSocket.h"
#include<WinInet.h>
#pragma comment(lib, "WinInet.lib")

/*
Agent가 보내는 패킷중 TCP_H만큼의 크기는 잡혀 있지만
안의 내용이 0으로 채워진 패킷이 가끔 전송된다.
두가지가 예상되는데 Release모드때문이거나
너무 빠르고 자주 일어나는 패킷 전송 때문으로 생각된다.

일단 Release모드는 아니다. volatile하면 그런 전송이 자주 일어나지 않지만
최적화를 하지 않아서 전송하는 텀이 길어진듯 하다.
*/

//!! console에서 유니코드로 한글을 출력 하려면
//_wsetlocale(LC_ALL, _T("korean"));
//_tprintf(_T("_tprintf()함수이다.\n"));
//이와 같은 처리를 해주어야 한다.

CTestFileInfo g_TestFileInfo;
CClientSocket g_Socket(g_TestFileInfo);

//에이전트 종료 code
BOOL CtrlHandler(DWORD fdwCtrlType)
{
	//모드 닫힐 때 생기는 이벤트
	//CTRL_C_EVENT
	//CTRL_BREAK_EVENT
	//CTRL_CLOSE_EVENT		//X자를 누르거나 작업관리자에서 끝내기를 선택한 경우
	//CTRL_LOGOFF_EVENT		//사용자가 로그오프 한 경우
	//CTRL_SHUTDOWN_EVENT	//컴퓨터를 끌때 정상종료 되게

	//※ 종료에 의해 모두 해제해야하는 모든걸 여기서 해결
	_tprintf(_T("## Close Agnet ##\n"));

	//폴더 감시 Thread 종료
	if (g_hEventEndMonitoring != 0)
	{
		SetEvent(g_hEventEndMonitoring);
	}
	
	//Health Check인 Thread 종료
	g_Socket.ReleaseSocket();
	//파일 해제
	g_TestFileInfo.ReleaseTestFolderAndFile();

	Sleep(200); //좀더 기다려야 할지도


	return TRUE;
}


int main()
{
	//종료 때 불릴 함수 등록
	BOOL Success = SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlHandler, TRUE);
	//Test할 파일들 
	g_TestFileInfo.InitTestFolderAndFile();
	DWORD Flag = 0;

	TCHAR Name[256];
	while (true)
	{
		if (::InternetGetConnectedStateEx(&Flag, Name, 256, 0))
		{
			printf("Connet\n");
			break;
		}
		else
		{
			printf("No Connet\n");
		}

		Sleep(1000);
	}

	//conect를 연결
	g_Socket.Connect("192.168.56.1", 25000);
	//recive
	g_Socket.Recive();
	g_TestFileInfo.ReleaseTestFolderAndFile();

    return 0;
}

