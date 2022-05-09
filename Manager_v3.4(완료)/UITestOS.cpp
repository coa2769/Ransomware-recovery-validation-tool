// UITestOS.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "Manager_v1.0.h"
#include "UITestOS.h"

#include"GradientButton.h"
#include"ServerSocket.h"
#include"TestOSInfo.h"
#include"VBoxController.h"
#include"MainFrm.h"

/////////////////////////////////////////////////////////////////////////////////////
void ChangeStateTextReady(CUITestOS* UI)
{
	UI->SetStateText(_T("준비중"));
}

void ChangeStateTextActive(CUITestOS* UI)
{
	UI->SetStateText(_T(""));
}

void ChangeStateTextTest(CUITestOS* UI)
{
	UI->SetStateText(_T("테스트중"));
}

void ChangeStateTextTestComplete(CUITestOS* UI)
{
	UI->SetStateText(_T("결과 저장중"));
}

void ChangeStateTextClose(CUITestOS* UI)
{
	UI->SetStateText(_T("에이전트 종료중"));
}

void ChangeStateTextShowReport(CUITestOS* UI)
{
	UI->SetStateText(_T("보고서 출력"));
}

void ChangeStateTextDisable(CUITestOS* UI)
{
	UI->SetStateText(_T("비활성화"));
}

void ChangeStateTextError(CUITestOS* UI)
{
	UI->SetStateText(_T("ERROR 종료"));
}
/////////////////////////////////////////////////////////////////////////////////////
// CUITestOS
IMPLEMENT_DYNAMIC(CUITestOS, CWnd)

CUITestOS::CUITestOS()
{
	m_FuncStateText[STATE_READY] = ChangeStateTextReady;
	m_FuncStateText[STATE_ACTIVE] = ChangeStateTextActive;
	m_FuncStateText[STATE_TEST] = ChangeStateTextTest;
	m_FuncStateText[STATE_TEST_COMPLETE] = ChangeStateTextTestComplete;
	m_FuncStateText[STATE_CLOSE] = ChangeStateTextClose;
	m_FuncStateText[STATE_SHOW_REPORT] = ChangeStateTextShowReport;
	m_FuncStateText[STATE_DISABLE] = ChangeStateTextDisable;
	m_FuncStateText[STATE_ERROR] = ChangeStateTextError;
}

CUITestOS::~CUITestOS()
{
}


BEGIN_MESSAGE_MAP(CUITestOS, CWnd)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_BN_CLICKED(BTN_ID_START, &CUITestOS::OnBnClickedButtonStart)
END_MESSAGE_MAP()



// CUITestOS 메시지 처리기입니다.

BOOL CUITestOS::CreateUITestOS(CWnd* CWndParent, TEST_OS os, int x, int y, int ID)
{
	const int Obj_dis = 10;
	const DWORD dwStyle = WS_CHILD | WS_VISIBLE;
	m_os = os;
	if (os == WINDOW_7) m_CStrOSName.Format(_T(" 7"));
	else if (os == WINDOW_8) m_CStrOSName.Format(_T(" 8"));
	else if (os == WINDOW_10) m_CStrOSName.Format(_T("10"));

	return Create(NULL, _T(""), dwStyle, CRect(x, y, x + VERSION_UI_WIDTH + LOG_UI_WIDTH, y + TEST_OS_HEIGHT), CWndParent, ID);
}

int CUITestOS::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	//rect
	m_DisabledRect = new CGradientRect;
	m_DisabledRect->SetGradientRect(this, CRect(0, 0, VERSION_UI_WIDTH, TEST_OS_HEIGHT), RGB(103, 107, 108), RGB(157, 163, 165), LinearGradientMode::Vertical, FALSE);
	
	m_ActivationRect = new CGradientRect;
	m_ActivationRect->SetGradientRect(this, CRect(0, 0, VERSION_UI_WIDTH, TEST_OS_HEIGHT), RGB(43, 197, 90), RGB(22, 160, 63), LinearGradientMode::Vertical, FALSE);
	
	m_LogRect = new CGradientRect;
	m_LogRect->SetGradientRect(this, CRect(VERSION_UI_WIDTH + 10, 0, VERSION_UI_WIDTH + LOG_UI_WIDTH, TEST_OS_HEIGHT), RGB(61, 67, 67), RGB(51, 55, 56), LinearGradientMode::Vertical, FALSE);

	const DWORD dwStyle = WS_CHILD | WS_VISIBLE | BS_OWNERDRAW;
	const int obj_dis = 20;

	//button
	CRect* rect = m_LogRect->GetRect();
	m_StartButton = new CGradientButton;
	m_StartButton->Create(NULL, dwStyle, CRect(CPoint(VERSION_UI_WIDTH + 10 + obj_dis, obj_dis), CSize(BTN_WIDTH, BTN_HEIGHT)), this, BTN_ID_START);
	m_StartButton->SetNormalGradient(_T("시  작"), RGB(105, 110, 113), RGB(90, 94, 97), LinearGradientMode::Vertical);
	m_StartButton->SetOverGradient(_T("시  작"), RGB(227, 229, 229), RGB(141, 144, 146), LinearGradientMode::ForwardDiagonal);
	m_StartButton->SetSelectGradient(_T("시  작"), RGB(103, 107, 108), RGB(157, 163, 165), LinearGradientMode::Vertical);
	//버튼 숨김
	if (theApp.m_mapTestOsInfo[m_os]->getState() == STATE_DISABLE)
	{
		DrawDisable();
	}
	
	return 0;
}

BOOL CUITestOS::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
}
/////////////////////////////////////////////////////////////////////////////////////
//Draw Funtion
void CUITestOS::SetChangeStateText(void)
{
	TEST_OS_STATE state = theApp.m_mapTestOsInfo[m_os]->getState();
	m_FuncStateText[state](this);
}

void CUITestOS::DrawDisable(void)
{
	m_StartButton->ShowWindow(SW_HIDE);
	
	theApp.m_mapTestOsInfo[m_os]->SetState(STATE_DISABLE);

	SetChangeStateText();
	SetLogText(_T("GuestOS가 없습니다."));
}

void CUITestOS::DrawActive(void)
{
	theApp.m_mapTestOsInfo[m_os]->SetState(STATE_ACTIVE);
	theApp.m_mapTestOsInfo[m_os]->InitResult();

	SetChangeStateText();
	SetProgressText(0, -1);
	SetLogText(_T(""));
	m_StartButton->ShowWindow(SW_SHOW);
	InvalidateRect(NULL);
}

void CUITestOS::DrawStateText(CDC* pDC)
{
	LOGFONT lf;
	CFont smallFont, bigFont;
	::ZeroMemory(&lf, sizeof(lf));
	lf.lfWeight = FW_BOLD;
	lf.lfHeight = 80;
	bigFont.CreateFontIndirectW(&lf);
	CFont *pOldFont = pDC->SelectObject(&bigFont);
	CRect *pRect = m_LogRect->GetRect();
	pDC->TextOutW(pRect->left + 10, pRect->top + 10, m_CStrState);
	
	pDC->SetTextColor(RGB(0, 0, 0));
	//작은 글씨
	lf.lfHeight = 30;
	smallFont.CreateFontIndirectW(&lf);
	pDC->SelectObject(&smallFont);
	pDC->TextOutW(pRect->left + 10, pRect->top + 100, m_CStrLog);
	pDC->TextOutW(pRect->left + 10, pRect->top + 130, m_CStrProgress);

	pDC->SelectObject(pOldFont);
}

void CUITestOS::DrawReportCount(CDC* pDC)
{
	LOGFONT lf;
	CFont smallFont, bigFont;
	::ZeroMemory(&lf, sizeof(lf));
	lf.lfWeight = FW_BOLD;
	//작은 글씨
	lf.lfHeight = 25;
	bigFont.CreateFontIndirectW(&lf);
	CFont *pOldFont = pDC->SelectObject(&bigFont);
	CRect *pRect = m_LogRect->GetRect();
	
	pDC->SetTextColor(RGB(192, 192, 192));

	CPoint pos(pRect->left + LOG_UI_WIDTH / 2 + 40, pRect->top + 20);
	CString str;

	CTestOSInfo * Info = theApp.m_mapTestOsInfo[m_os];

	str.Format(_T("감염시작 : %hd시 %hd분 %hd초"),
		Info->getTestTime().InfecionStart.wHour, Info->getTestTime().InfecionStart.wMinute, Info->getTestTime().InfecionStart.wSecond);
	pDC->TextOutW(pos.x, pos.y, str);

	str.Format(_T("감염종료 : %hd시 %hd분 %hd초"),
		Info->getTestTime().InfectionEnd.wHour, Info->getTestTime().InfectionEnd.wMinute, Info->getTestTime().InfectionEnd.wSecond);
	pDC->TextOutW(pos.x, pos.y + 30, str);
	
	str.Format(_T("복구시작 : %hd시 %hd분 %hd초"),
		Info->getTestTime().RecoveryStart.wHour, Info->getTestTime().RecoveryStart.wMinute, Info->getTestTime().RecoveryStart.wSecond);
	pDC->TextOutW(pos.x, pos.y + 60, str);
	
	str.Format(_T("복구종료 : %hd시 %hd분 %hd초"),
		Info->getTestTime().RecoveryEnd.wHour, Info->getTestTime().RecoveryEnd.wMinute, Info->getTestTime().RecoveryEnd.wSecond);
	pDC->TextOutW(pos.x, pos.y + 90, str);

	str.Format(_T("복구/총 : %d / %d"),Info->GetRecoveryFileCount(),Info->GetResultCount());
	pDC->TextOutW(pos.x, pos.y + 120, str);

}

void CUITestOS::OnPaint()
{
	CPaintDC dc(this); // device context for painting
					   // TODO: 여기에 메시지 처리기 코드를 추가합니다.
					   // 그리기 메시지에 대해서는 CWnd::OnPaint()을(를) 호출하지 마십시오.
	CDC memDC;
	memDC.CreateCompatibleDC(&dc);
	CBitmap memBmp, *pOldBmp;

	CRect clnRect;
	GetClientRect(&clnRect);
	memBmp.CreateCompatibleBitmap(&dc, clnRect.Width(), clnRect.Height());
	pOldBmp = memDC.SelectObject(&memBmp);

	m_LogRect->DrawRect(&memDC);

	TEST_OS_STATE state = theApp.m_mapTestOsInfo[m_os]->getState();

	if (state == STATE_DISABLE)
	{
		m_DisabledRect->DrawRect(&memDC);
	}
	else
	{
		m_ActivationRect->DrawRect(&memDC);
	}

	//window 글자
	//https://blog.naver.com/iamcloud/50146656660
	LOGFONT lf;
	CFont smallFont, bigFont;
	::ZeroMemory(&lf, sizeof(lf));
	lf.lfWeight = FW_BOLD;
	//작은 글씨
	lf.lfHeight = 50;
	smallFont.CreateFontIndirectW(&lf);
	CFont *pOldFont = memDC.SelectObject(&smallFont);
	memDC.SetBkMode(TRANSPARENT);
	memDC.SetTextColor(RGB(255, 255, 255));
	memDC.TextOutW(10, 10, _T("Windows"));
	//큰 글씨
	lf.lfHeight = 100;
	bigFont.CreateFontIndirectW(&lf);
	memDC.SelectObject(&bigFont);
	memDC.TextOutW(50, 80, m_CStrOSName);
	memDC.SelectObject(pOldFont);

	DrawStateText(&memDC);

	state = theApp.m_mapTestOsInfo[m_os]->getState();

	if (state == STATE_TEST || state == STATE_TEST_COMPLETE || state == STATE_SHOW_REPORT)
	{
		DrawReportCount(&memDC);
	}

	dc.BitBlt(0, 0, clnRect.Width(), clnRect.Height(), &memDC, 0, 0, SRCCOPY);
	memDC.SelectObject(pOldBmp);
}

///////////////////////////////////////////////////////////////////////////////////
void CUITestOS::OnBnClickedButtonStart()
{
	if (((CMainFrm*)theApp.m_pMainWnd)->m_CRansomwarePath != _T("") &&
		((CMainFrm*)theApp.m_pMainWnd)->m_CRecoveryPath != _T(""))
	{
		//State를 준비중으로 변경
		m_StartButton->ShowWindow(SW_HIDE);
		//Test를 준비하기 위한 Thread
		CWinThread* pThread = AfxBeginThread(ThreadCommunication, (LPVOID)&(this->m_os));
	}
	else
	{
		ErrorHandler(_T("전송 파일 입력이 완료되지 않았습니다."), FALSE);
	}

}

//Recvie의 종류에 따른 UI변화
void CUITestOS::OnDestroy()
{
	CWnd::OnDestroy();

	if (m_StartButton != NULL)
	{
		delete m_StartButton;
		m_StartButton = NULL;
	}
	if (m_LogRect != NULL)
	{
		delete m_LogRect;
		m_LogRect = NULL;
	}
	if (m_DisabledRect != NULL)
	{
		delete m_DisabledRect;
		m_DisabledRect = NULL;
	}
	if (m_ActivationRect != NULL)
	{
		delete m_ActivationRect;
		m_ActivationRect = NULL;
	}
}	

