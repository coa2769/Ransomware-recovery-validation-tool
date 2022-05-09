
// stdafx.cpp : 표준 포함 파일만 들어 있는 소스 파일입니다.
// Manager_v1.0.pch는 미리 컴파일된 헤더가 됩니다.
// stdafx.obj에는 미리 컴파일된 형식 정보가 포함됩니다.

#include "stdafx.h"
#include"Manager_v1.0.h"

void ErrorHandler(LPCTSTR lpszText, BOOL exit)
{
	//Error 처리
	AfxMessageBox(lpszText);
	if (exit == TRUE)
	{
		theApp.m_pMainWnd->PostMessage(WM_CLOSE);
	}
}
