// MainFrm.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "Manager_v1.0.h"
#include "MainFrm.h"
#include "UITestOS.h"
#include "TestOSInfo.h"
#include"ServerSocket.h"
#include"VBoxController.h"

void DrawClose(CDC& dc, CRect& rect, bool isActive)
{
	CPen NewPen, *oldPen;
	if (isActive == true)
	{
		NewPen.CreatePen(PS_SOLID, 2, RGB(255, 255, 255));
		oldPen = dc.SelectObject(&NewPen);
	}
	else
	{
		NewPen.CreatePen(PS_SOLID, 2, RGB(100, 100, 100));
		oldPen = dc.SelectObject(&NewPen);
	}

	dc.MoveTo(rect.left, rect.top);
	dc.LineTo(rect.right, rect.bottom);
	dc.MoveTo(rect.left, rect.bottom);
	dc.LineTo(rect.right, rect.top);

	dc.SelectObject(oldPen);
}

void DrawMaximize(CDC& dc, CRect& rect, bool isActive)
{
	CPen NewPen, *oldPen;
	if (isActive == true)
	{
		NewPen.CreatePen(PS_SOLID, 2, RGB(255, 255, 255));
		oldPen = dc.SelectObject(&NewPen);
	}
	else
	{
		NewPen.CreatePen(PS_SOLID, 2, RGB(100, 100, 100));
		oldPen = dc.SelectObject(&NewPen);
	}

	dc.MoveTo(rect.left, rect.top);
	dc.LineTo(rect.right, rect.top);
	dc.LineTo(rect.right, rect.bottom);
	dc.LineTo(rect.left, rect.bottom);
	dc.LineTo(rect.left, rect.top);

	dc.SelectObject(oldPen);
}

void DrawMinimize(CDC& dc, CRect& rect, bool isActive)
{
	CPen NewPen, *oldPen;
	if (isActive == true)
	{
		NewPen.CreatePen(PS_SOLID, 2, RGB(255, 255, 255));
		oldPen = dc.SelectObject(&NewPen);
	}
	else
	{
		NewPen.CreatePen(PS_SOLID, 2, RGB(100, 100, 100));
		oldPen = dc.SelectObject(&NewPen);
	}

	dc.MoveTo(rect.left, rect.top + rect.Width() / 2);
	dc.LineTo(rect.right, rect.top + rect.Width() / 2);

	dc.SelectObject(oldPen);
}

// CMainFrm

IMPLEMENT_DYNAMIC(CMainFrm, CWnd)

CMainFrm::CMainFrm()
	:m_CRansomwarePath(_T("")),
	m_CRansomwareName(_T("")),
	m_CRecoveryPath(_T("")),
	m_CRecoveryName(_T(""))
{

}

CMainFrm::~CMainFrm()
{
}

//CEdit 관련 URL
//https://blog.naver.com/proonan29/130072752557
//통지 메시지
//https://anster.tistory.com/11

BEGIN_MESSAGE_MAP(CMainFrm, CWnd)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()
	ON_WM_PAINT()
	ON_WM_DROPFILES()
	ON_BN_CLICKED(SYSBTN_ID_CLOSE, &CMainFrm::OnBnClickedButtonClose)
	ON_BN_CLICKED(SYSBTN_ID_MINIMIZE, &CMainFrm::OnBnClickedButtonMinimize)
	ON_WM_DESTROY()
END_MESSAGE_MAP()



// CMainFrm 메시지 처리기입니다.

BOOL CMainFrm::CreateMainWindow()
{
	HCURSOR hCur = theApp.LoadStandardCursor(IDC_ARROW);
	LPCTSTR lpszClassName = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_SAVEBITS | CS_DBLCLKS, hCur);

	return CreateEx(WS_EX_APPWINDOW, lpszClassName, _T(""), WS_VISIBLE | WS_POPUP, CRect(100, 100, 1300, 900), NULL, 0, NULL);
}


int CMainFrm::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rect;
	int Obj_dis = 10; //배경과 배경컨테이너 사이 간격 
	const DWORD dwStyle = WS_CHILD | WS_VISIBLE | BS_OWNERDRAW;

	//Base
	m_CaptionRect = CRect(0, 0, 1200, 50);
	m_btnClose.Create(NULL, dwStyle, CRect(CPoint(WINDOW_WIDTH - 50, 0), CSize(50, 50)), this, SYSBTN_ID_CLOSE);
	m_btnClose.SetButtonInfo(RGB(27, 28, 30), RGB(222, 12, 12), RGB(238, 129, 129), 150, true, DrawClose);
	m_btnMaximize.Create(NULL, dwStyle, CRect(CPoint(WINDOW_WIDTH - 100, 0), CSize(50, 50)), this, SYSBTN_ID_MAXIMIZE);
	m_btnMaximize.SetButtonInfo(RGB(27, 28, 30), RGB(192, 192, 192), RGB(192, 192, 192), 150, false, DrawMaximize);
	m_btnMinimize.Create(NULL, dwStyle, CRect(CPoint(WINDOW_WIDTH - 150, 0), CSize(50, 50)), this, SYSBTN_ID_MINIMIZE);
	m_btnMinimize.SetButtonInfo(RGB(27, 28, 30), RGB(192, 192, 192), RGB(192, 192, 192), 150, true, DrawMinimize);

	m_RectBk.SetGradientRect(this, CRect(CPoint(1,1), CSize(WINDOW_WIDTH, WINDOW_HEIGHT)), RGB(27, 28, 30), RGB(0, 0, 0), LinearGradientMode::Vertical, FALSE);
	rect = CRect(Obj_dis, m_CaptionRect.Height(), WINDOW_WIDTH - Obj_dis, WINDOW_HEIGHT - Obj_dis);
	m_RectBkContainer.SetGradientRect(this, rect, RGB(46, 47, 51), RGB(30, 31, 33), LinearGradientMode::Vertical, TRUE);

	//기능 UI
	Obj_dis = 30;
	//rasom path
	rect = CRect(CPoint(rect.left + Obj_dis, m_CaptionRect.Height() + Obj_dis), CSize(FILE_DROP_WIDTH, 25));
	m_EditRansom.Create(ES_AUTOHSCROLL | ES_READONLY | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER, rect, this, EDIT_ID_RANSOM);
	//rasom input
	rect = CRect(CPoint(rect.left, rect.bottom + 5), CSize(FILE_DROP_WIDTH, FILE_DROP_HEIGHT));
	m_RectRansomInput.SetGradientRect(this, rect, RGB(105, 110, 113), RGB(90, 94, 97), LinearGradientMode::Vertical, TRUE);
	m_RectRansomDisabledInput.SetGradientRect(this, rect, RGB(103, 107, 108), RGB(157, 163, 165), LinearGradientMode::ForwardDiagonal, TRUE);
	m_CImageRansom.Load(_T("res\\lock(2).png"));
	//recovery path
	rect = CRect(CPoint(rect.left, rect.bottom + Obj_dis), CSize(FILE_DROP_WIDTH, 25));
	m_EditRecovery.Create(ES_AUTOHSCROLL | ES_READONLY | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER, rect, this, EDIT_ID_RANSOM);
	//recovery inpu
	rect = CRect(CPoint(rect.left, rect.bottom + 5), CSize(FILE_DROP_WIDTH, FILE_DROP_HEIGHT));
	m_RectRecoveryInput.SetGradientRect(this, rect, RGB(105, 110, 113), RGB(90, 94, 97), LinearGradientMode::Vertical, TRUE);
	m_RectRecoveryDisabledInput.SetGradientRect(this, rect, RGB(103, 107, 108), RGB(157, 163, 165), LinearGradientMode::ForwardDiagonal, TRUE);
	m_CImageRecovery.Load(_T("res\\tools(2).png"));

	//UI
	m_CUIWIndow[(int)WINDOW_7] = new CUITestOS;
	m_CUIWIndow[(int)WINDOW_7]->CreateUITestOS(this, WINDOW_7, rect.right + Obj_dis, m_CaptionRect.bottom + Obj_dis, UI_ID_WINDOW7);
	
	m_CUIWIndow[(int)WINDOW_8] = new CUITestOS;
	m_CUIWIndow[(int)WINDOW_8]->CreateUITestOS(this, WINDOW_8, rect.right + Obj_dis, m_CaptionRect.bottom + TEST_OS_HEIGHT + Obj_dis + 20, UI_ID_WINDOW7);

	m_CUIWIndow[(int)WINDOW_10] = new CUITestOS;
	m_CUIWIndow[(int)WINDOW_10]->CreateUITestOS(this, WINDOW_10, rect.right + Obj_dis, m_CaptionRect.bottom + (TEST_OS_HEIGHT * 2) + Obj_dis + 40, UI_ID_WINDOW7);

	//File Drag
	DragAcceptFiles();

	return 0;
}


BOOL CMainFrm::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
}


void CMainFrm::OnLButtonDown(UINT nFlags, CPoint point)
{
	//※ 캡션 Drag & Drop
	if (m_CaptionRect.PtInRect(point))
	{
		PostMessage(WM_NCLBUTTONDOWN, HTCAPTION, MAKELPARAM(point.x, point.y));
	}

	CWnd::OnLButtonDown(nFlags, point);
}

void CMainFrm::DrawDropFileRect(void)
{
	InvalidateRect(m_RectRansomDisabledInput.GetRect());
	InvalidateRect(m_RectRecoveryDisabledInput.GetRect());
}

void CMainFrm::OnPaint()
{
	CPaintDC dc(this); // device context for painting
					   // TODO: 여기에 메시지 처리기 코드를 추가합니다.
					   // 그리기 메시지에 대해서는 CWnd::OnPaint()을(를) 호출하지 마십시오.
	CDC memDC;
	memDC.CreateCompatibleDC(&dc);
	CBitmap memBmp, *pOldBmp;
	
	memBmp.CreateCompatibleBitmap(&dc, WINDOW_WIDTH, WINDOW_HEIGHT);
	pOldBmp = memDC.SelectObject(&memBmp);
	
	//Base
	memDC.Rectangle(CRect(0,0,WINDOW_WIDTH,WINDOW_HEIGHT));
	m_RectBk.DrawRect(&memDC);
	m_RectBkContainer.DrawRect(&memDC);
	
	//file & path
	if (PossibleFileInput() == TRUE)
	{
		m_RectRansomInput.DrawRect(&memDC);
		m_RectRecoveryInput.DrawRect(&memDC);
	}
	else
	{
		m_RectRansomDisabledInput.DrawRect(&memDC);
		m_RectRecoveryDisabledInput.DrawRect(&memDC);
	}

	m_CImageRansom.TransparentBlt(memDC, m_RectRansomInput.GetRect()->left + 72,
		m_RectRansomInput.GetRect()->top + 25, 175, 250, RGB(0, 0, 0));
	m_CImageRecovery.TransparentBlt(memDC, m_RectRecoveryInput.GetRect()->left + 25,
		m_RectRecoveryInput.GetRect()->top + 30, 250, 240, RGB(0, 0, 0));
	//
	dc.BitBlt(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, &memDC, 0, 0, SRCCOPY);
	memDC.SelectObject(pOldBmp);

}


void CMainFrm::OnDropFiles(HDROP hDropInfo)
{
	if (hDropInfo != NULL && PossibleFileInput() == TRUE)
	{
		int file_Path_size = 0;
		TCHAR *file_Path = NULL;

		file_Path_size = DragQueryFile(hDropInfo, 0, NULL, 0);
		file_Path = new TCHAR[sizeof(TCHAR)*(file_Path_size + 1)];
		DragQueryFile(hDropInfo, 0, file_Path, file_Path_size + 1);

		POINT pt;
		GetCursorPos(&pt);
		ScreenToClient(&pt);

		if (m_RectRansomInput.PtInRect(pt) == TRUE)
		{
			m_CRansomwarePath = file_Path;
			m_CRansomwareName = file_Path;
			theApp.m_CParser.FileNameParsing(m_CRansomwareName);
			m_EditRansom.SetWindowTextW(m_CRansomwareName);
		}
		else if (m_RectRecoveryInput.PtInRect(pt) == TRUE)
		{
			m_CRecoveryPath = file_Path;
			m_CRecoveryName = file_Path;
			theApp.m_CParser.FileNameParsing(m_CRecoveryName);
			m_EditRecovery.SetWindowTextW(m_CRecoveryName);
		}

		delete file_Path;
		
		//모든 Control Thread가 닫혀 있어야 가능
		TEST_OS_STATE state;
		for (int i = 0; i < (WINDOW_10 + 1); i++)
		{
			state = theApp.m_mapTestOsInfo[(TEST_OS)i]->getState();
			
			if (state == STATE_SHOW_REPORT)
			{
				m_CUIWIndow[(TEST_OS)i]->DrawActive();
			}
		}
	
	}

	CWnd::OnDropFiles(hDropInfo);
}


void CMainFrm::OnBnClickedButtonMinimize()
{
	//창 최소화
	PostMessage(WM_SYSCOMMAND, SC_MINIMIZE);
}


void CMainFrm::OnBnClickedButtonClose()
{
	//1. 통신을 모두 끈는다.
	theApp.m_CServer->Release();

	//2. Ready Thread에게 Exit이벤트를 준다.
	theApp.m_ExitEvent.SetEvent();
	::Sleep(1000);

	//3. 모든 VM종료
	theApp.m_CVBoxController->Release();

	//창 닫기
	((CMainFrm*)theApp.m_pMainWnd)->PostMessage(WM_SYSCOMMAND, SC_CLOSE);


}

void CMainFrm::OnDestroy()
{
	CWnd::OnDestroy();

	//4. UI delete
	for (int i = 0; i < (WINDOW_10 + 1); i++)
	{
		if (m_CUIWIndow[i] != NULL)
		{
			delete m_CUIWIndow[i];
			m_CUIWIndow[i] = NULL;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////
BOOL CMainFrm::PossibleFileInput(void)
{
	BOOL is = FALSE;

	TEST_OS_STATE state[3];
	state[0] = theApp.m_mapTestOsInfo[WINDOW_7]->getState();
	state[1] = theApp.m_mapTestOsInfo[WINDOW_8]->getState();
	state[2] = theApp.m_mapTestOsInfo[WINDOW_10]->getState();

	if ((state[0] == STATE_ACTIVE || state[0] == STATE_DISABLE || state[0] == STATE_SHOW_REPORT) &&
		(state[1] == STATE_ACTIVE || state[1] == STATE_DISABLE || state[1] == STATE_SHOW_REPORT) &&
		(state[2] == STATE_ACTIVE || state[2] == STATE_DISABLE || state[2] == STATE_SHOW_REPORT))
	{
		is = TRUE;
	}
	
	return is;
}
