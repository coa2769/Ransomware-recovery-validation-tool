// Agent_v1.0.cpp : �ܼ� ���� ���α׷��� ���� �������� �����մϴ�.
//

#include "stdafx.h"
#include"TestFileFution.h"
#include"ClientSocket.h"
#include<WinInet.h>
#pragma comment(lib, "WinInet.lib")

/*
Agent�� ������ ��Ŷ�� TCP_H��ŭ�� ũ��� ���� ������
���� ������ 0���� ä���� ��Ŷ�� ���� ���۵ȴ�.
�ΰ����� ����Ǵµ� Release��嶧���̰ų�
�ʹ� ������ ���� �Ͼ�� ��Ŷ ���� �������� �����ȴ�.

�ϴ� Release���� �ƴϴ�. volatile�ϸ� �׷� ������ ���� �Ͼ�� ������
����ȭ�� ���� �ʾƼ� �����ϴ� ���� ������� �ϴ�.
*/

//!! console���� �����ڵ�� �ѱ��� ��� �Ϸ���
//_wsetlocale(LC_ALL, _T("korean"));
//_tprintf(_T("_tprintf()�Լ��̴�.\n"));
//�̿� ���� ó���� ���־�� �Ѵ�.

CTestFileInfo g_TestFileInfo;
CClientSocket g_Socket(g_TestFileInfo);

//������Ʈ ���� code
BOOL CtrlHandler(DWORD fdwCtrlType)
{
	//��� ���� �� ����� �̺�Ʈ
	//CTRL_C_EVENT
	//CTRL_BREAK_EVENT
	//CTRL_CLOSE_EVENT		//X�ڸ� �����ų� �۾������ڿ��� �����⸦ ������ ���
	//CTRL_LOGOFF_EVENT		//����ڰ� �α׿��� �� ���
	//CTRL_SHUTDOWN_EVENT	//��ǻ�͸� ���� �������� �ǰ�

	//�� ���ῡ ���� ��� �����ؾ��ϴ� ���� ���⼭ �ذ�
	_tprintf(_T("## Close Agnet ##\n"));

	//���� ���� Thread ����
	if (g_hEventEndMonitoring != 0)
	{
		SetEvent(g_hEventEndMonitoring);
	}
	
	//Health Check�� Thread ����
	g_Socket.ReleaseSocket();
	//���� ����
	g_TestFileInfo.ReleaseTestFolderAndFile();

	Sleep(200); //���� ��ٷ��� ������


	return TRUE;
}


int main()
{
	//���� �� �Ҹ� �Լ� ���
	BOOL Success = SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlHandler, TRUE);
	//Test�� ���ϵ� 
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

	//conect�� ����
	g_Socket.Connect("192.168.56.1", 25000);
	//recive
	g_Socket.Recive();
	g_TestFileInfo.ReleaseTestFolderAndFile();

    return 0;
}
