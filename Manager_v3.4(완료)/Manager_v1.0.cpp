
// Manager_v1.0.cpp : 응용 프로그램에 대한 클래스 동작을 정의합니다.
//

#include "stdafx.h"
#include "afxwinappex.h"
#include "afxdialogex.h"

#include "Manager_v1.0.h"

#include "MainFrm.h"
#include"ServerSocket.h"
#include"TestOSInfo.h"
#include"VBoxController.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
/////////////////////////////////////////////////////////////////////////////
// $ : Info에 대한 Lock이다.
// $@ : Socket에 대한 Lock이다.
/////////////////////////////////////////////////////////////////////////////

// CManager_v10App

BEGIN_MESSAGE_MAP(CManager_v10App, CWinApp)
	ON_COMMAND(ID_APP_ABOUT, &CManager_v10App::OnAppAbout)
END_MESSAGE_MAP()


// CManager_v10App 생성

CManager_v10App::CManager_v10App()
{
	// 다시 시작 관리자 지원
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;
#ifdef _MANAGED
	// 응용 프로그램을 공용 언어 런타임 지원을 사용하여 빌드한 경우(/clr):
	//     1) 이 추가 설정은 다시 시작 관리자 지원이 제대로 작동하는 데 필요합니다.
	//     2) 프로젝트에서 빌드하려면 System.Windows.Forms에 대한 참조를 추가해야 합니다.
	System::Windows::Forms::Application::SetUnhandledExceptionMode(System::Windows::Forms::UnhandledExceptionMode::ThrowException);
#endif

	// TODO: 아래 응용 프로그램 ID 문자열을 고유 ID 문자열로 바꾸십시오(권장).
	// 문자열에 대한 서식: CompanyName.ProductName.SubProduct.VersionInformation
	SetAppID(_T("Manager_v1.0.AppID.NoVersion"));

	// TODO: 여기에 생성 코드를 추가합니다.
	// InitInstance에 모든 중요한 초기화 작업을 배치합니다.
}

// 유일한 CManager_v10App 개체입니다.

CManager_v10App theApp;


// CManager_v10App 초기화


BOOL CManager_v10App::InitInstance()
{
	// 응용 프로그램 매니페스트가 ComCtl32.dll 버전 6 이상을 사용하여 비주얼 스타일을
	// 사용하도록 지정하는 경우, Windows XP 상에서 반드시 InitCommonControlsEx()가 필요합니다. 
	// InitCommonControlsEx()를 사용하지 않으면 창을 만들 수 없습니다.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// 응용 프로그램에서 사용할 모든 공용 컨트롤 클래스를 포함하도록
	// 이 항목을 설정하십시오.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();


	// OLE 라이브러리를 초기화합니다.
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}

	AfxEnableControlContainer();

	EnableTaskbarInteraction(FALSE);

	// RichEdit 컨트롤을 사용하려면  AfxInitRichEdit2()가 있어야 합니다.	
	// AfxInitRichEdit2();

	// 표준 초기화
	// 이들 기능을 사용하지 않고 최종 실행 파일의 크기를 줄이려면
	// 아래에서 필요 없는 특정 초기화
	// 루틴을 제거해야 합니다.
	// 해당 설정이 저장된 레지스트리 키를 변경하십시오.
	// TODO: 이 문자열을 회사 또는 조직의 이름과 같은
	// 적절한 내용으로 수정해야 합니다.
	SetRegistryKey(_T("로컬 응용 프로그램 마법사에서 생성된 응용 프로그램"));
	
	//※ 0. TestOS 정보를 넣어둘 class 생성
	m_mapTestOsInfo.SetAt(WINDOW_7, new CTestOSInfo(WINDOW_7));
	m_mapTestOsInfo.SetAt(WINDOW_8, new CTestOSInfo(WINDOW_8));
	m_mapTestOsInfo.SetAt(WINDOW_10, new CTestOSInfo(WINDOW_10));

	//※ 1. 목록 list검사 UUID추가
	m_CVBoxController = new CVBoxController;
	m_CVBoxController->Initialize();
	m_CVBoxController->CheckVMList();

	//※ 1.1 소켓의 연결을 받는 Thread
	m_CServer = new CServerSocket;
	m_CServer->AcceptFuntion();
	

	// 주 창을 만들기 위해 이 코드에서는 새 프레임 창 개체를
	// 만든 다음 이를 응용 프로그램의 주 창 개체로 설정합니다.
	CMainFrm* pFrame = new CMainFrm;
	if (!pFrame->CreateMainWindow())
		return FALSE;

	m_pMainWnd = pFrame;
	pFrame->ShowWindow(SW_SHOW);
	pFrame->UpdateWindow();

	return TRUE;
}

int CManager_v10App::ExitInstance()
{
	//TODO: 추가한 추가 리소스를 처리합니다.
	delete m_mapTestOsInfo[WINDOW_7];
	delete m_mapTestOsInfo[WINDOW_8];
	delete m_mapTestOsInfo[WINDOW_10];

	m_mapTestOsInfo.RemoveAll();

	delete m_CServer;
	delete m_CVBoxController;

	AfxOleTerm(FALSE);

	return CWinApp::ExitInstance();
}

// CManager_v10App 메시지 처리기


// 응용 프로그램 정보에 사용되는 CAboutDlg 대화 상자입니다.

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

// 구현입니다.
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

// 대화 상자를 실행하기 위한 응용 프로그램 명령입니다.
void CManager_v10App::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

// CManager_v10App 메시지 처리기



