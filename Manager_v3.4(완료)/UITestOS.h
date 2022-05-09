#pragma once

#define VERSION_UI_WIDTH	210
#define LOG_UI_WIDTH		580
#define TEST_OS_HEIGHT		215

#define BTN_WIDTH 210
#define BTN_HEIGHT 100


class CGradientButton;
class CGradientRect;

class CUITestOS : public CWnd
{
	DECLARE_DYNAMIC(CUITestOS)

public:
	CUITestOS();
	virtual ~CUITestOS();

private:
	CGradientButton* m_StartButton;
	CGradientRect* m_LogRect;
	CGradientRect* m_DisabledRect;
	CGradientRect* m_ActivationRect;
	
	TEST_OS m_os;
	CString m_CStrOSName;
	CString m_CStrState;
	CString m_CStrLog;
	CString m_CStrProgress;

	void(*m_FuncStateText[8])(CUITestOS* UI);
protected:
	DECLARE_MESSAGE_MAP()

public:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();

	//Create function
	BOOL CreateUITestOS(CWnd* CWndParent, TEST_OS os, int x, int y, int ID);

	//Button
	void OnBnClickedButtonStart();
	
	TEST_OS GetOS(void) { return m_os; }

	//Progress
	void SetProgressText(size_t max, size_t current)
	{
		if (current == SIZE_MAX)
		{
			m_CStrProgress = _T("");
		}
		else
		{
			m_CStrProgress.Format(_T("ม๘วเทü : %zd / %zd"), current, max);
		}
		InvalidateRect(NULL);
	}
	void SetStateText(TCHAR* text)
	{
		m_CStrState = text;
	}
	void SetLogText(TCHAR* log)
	{
		m_CStrLog = log;
	}

	void SetChangeStateText(void);
	void DrawStateText(CDC* pDC);
	void DrawReportCount(CDC* pDC);
	void DrawDisable(void);
	void DrawActive(void);


};


