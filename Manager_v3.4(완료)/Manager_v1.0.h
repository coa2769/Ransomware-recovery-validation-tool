
// Manager_v1.0.h : Manager_v1.0 응용 프로그램에 대한 주 헤더 파일
//
#pragma once

#ifndef __AFXWIN_H__
	#error "PCH에 대해 이 파일을 포함하기 전에 'stdafx.h'를 포함합니다."
#endif

#include "resource.h"       // 주 기호입니다.

#include"TextParsing.h"

// CManager_v10App:
// 이 클래스의 구현에 대해서는 Manager_v1.0.cpp을 참조하십시오.
//

/**
*@ mainpage 랜섬웨어와 복구도구를 입력 받고 제공하는 Geust OS 전송과 실행하여 결과를 얻는 프로그램
*@ se
*/

class CServerSocket;
class CTestOSInfo;
class CVBoxController;


class CManager_v10App : public CWinApp
{
public:
	CManager_v10App();


// 재정의입니다.
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// 구현입니다.

public:
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
	
public:
	CTextParsing m_CParser;

	CServerSocket *m_CServer;
	CVBoxController *m_CVBoxController;

	CMap<TEST_OS, TEST_OS, CTestOSInfo*, CTestOSInfo*> m_mapTestOsInfo;
	CEvent m_ExitEvent;

};

extern CManager_v10App theApp;
