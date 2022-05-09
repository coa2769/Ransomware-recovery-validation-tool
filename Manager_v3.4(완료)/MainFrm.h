#pragma once

#include"GradientButton.h"
#include"DrawButton.h"
// CMainFrm

#define WINDOW_WIDTH  1200
#define WINDOW_HEIGHT 800

#define FILE_DROP_WIDTH 300
#define FILE_DROP_HEIGHT 300

class CUITestOS;

class CMainFrm : public CWnd
{
	DECLARE_DYNAMIC(CMainFrm)

public:
	CMainFrm();
	virtual ~CMainFrm();
	BOOL CreateMainWindow(); //등록 함수

	CString m_CRansomwarePath;
	CString m_CRansomwareName;
	CString m_CRecoveryPath;
	CString m_CRecoveryName;

private:
	CRect m_CaptionRect;
	CRect m_FrameRect;

	CGradientRect m_RectBk;
	CGradientRect m_RectBkContainer;

	//http://www.tipssoft.com/bulletin/board.php?bo_table=FAQ&wr_id=299
	CEdit m_EditRansom;
	CEdit m_EditRecovery;
	CGradientRect m_RectRansomInput;
	CGradientRect m_RectRansomDisabledInput;
	CGradientRect m_RectRecoveryInput;
	CGradientRect m_RectRecoveryDisabledInput;
	CImage m_CImageRansom;
	CImage m_CImageRecovery;

	CDrawButton m_btnClose;
	CDrawButton m_btnMaximize;
	CDrawButton m_btnMinimize;


	BOOL PossibleFileInput(void);
protected:
	DECLARE_MESSAGE_MAP()
public:
	CUITestOS* m_CUIWIndow[(int)WINDOW_10 + 1];

	void OnBnClickedButtonMinimize();
	void OnBnClickedButtonClose();

	void DrawDropFileRect(void);

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnPaint();
	afx_msg void OnDropFiles(HDROP hDropInfo);
	afx_msg void OnDestroy();
};


