// DrawButton.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "Manager_v1.0.h"
#include "DrawButton.h"


// CDrawButton

IMPLEMENT_DYNAMIC(CDrawButton, CButton)

CDrawButton::CDrawButton()
{

}

CDrawButton::~CDrawButton()
{
}


BEGIN_MESSAGE_MAP(CDrawButton, CButton)
	ON_WM_DESTROY()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSELEAVE()
END_MESSAGE_MAP()




// CDrawButton 메시지 처리기입니다.

void CDrawButton::CreateBtnBitmap(CBitmap& bmp, COLORREF color, void(*Fun)(CDC& dc, CRect& rect, bool isActive))
{
	CRect ClientRect;
	GetClientRect(ClientRect);

	CDC *pDC = GetDC();
	CDC memDC;
	memDC.CreateCompatibleDC(pDC);

	bmp.DeleteObject();
	bmp.CreateCompatibleBitmap(pDC, ClientRect.Width(), ClientRect.Height());
	CBitmap *oldBmp = memDC.SelectObject(&bmp);

	memDC.FillSolidRect(&ClientRect, color);


	CRect temp = CRect(CPoint(ClientRect.Width() / 3, ClientRect.Height() / 3), CSize(ClientRect.Width() / 3, ClientRect.Height() / 3));
	
	Fun(memDC, temp, m_isActive);

	memDC.SelectObject(oldBmp);
	memDC.DeleteDC();
	ReleaseDC(pDC);
}

void CDrawButton::SetButtonInfo(COLORREF Normal, COLORREF Over, COLORREF Select, BYTE Alpha, bool isActive, void(*Fun)(CDC& dc, CRect& rect, bool isActive))
{
	m_isActive = isActive;
	m_bf.BlendOp = AC_SRC_OVER;
	m_bf.BlendFlags = 0;
	m_bf.SourceConstantAlpha = Alpha;
	m_bf.AlphaFormat = 0;

	CreateBtnBitmap(m_bmpNormal, Normal, Fun);
	CreateBtnBitmap(m_bmpOver, Over, Fun);
	CreateBtnBitmap(m_bmpSelect, Select, Fun);
	CreateBtnBitmap(m_bmpDisabled, Normal, Fun);
}


void CDrawButton::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	CDC *pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
	CDC memDC;
	memDC.CreateCompatibleDC(pDC);

	CRect ClientRect(lpDrawItemStruct->rcItem);
	if (m_isActive == true)
	{
		if (lpDrawItemStruct->itemState & ODS_SELECTED)
		{
			CDC AlphaDC;
			AlphaDC.CreateCompatibleDC(pDC);
			AlphaDC.SelectObject(m_bmpSelect);
			CBitmap memBmp;
			memBmp.CreateCompatibleBitmap(pDC, ClientRect.Width(), ClientRect.Height());
			memDC.SelectObject(memBmp);
			memDC.AlphaBlend(0, 0, ClientRect.Width(), ClientRect.Height(),
				&AlphaDC, 0, 0, ClientRect.Width(), ClientRect.Height(), m_bf);
		}
		else
		{
			if (m_bOver)
				memDC.SelectObject(m_bmpOver);
			else
				memDC.SelectObject(m_bmpNormal);

		}
	}
	else
	{
		memDC.SelectObject(m_bmpDisabled);
	}

	pDC->BitBlt(0, 0, ClientRect.Width(), ClientRect.Height(), &memDC, 0, 0, SRCCOPY);
}


void CDrawButton::OnDestroy()
{
	CButton::OnDestroy();

	m_bmpNormal.DeleteObject();
	m_bmpOver.DeleteObject();
	m_bmpSelect.DeleteObject();
	m_bmpDisabled.DeleteObject();
}


void CDrawButton::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_isActive == true && m_bOver == false)
	{
		m_bOver = true;

		TRACKMOUSEEVENT tme = { 0 };
		tme.cbSize = sizeof(tme);
		tme.hwndTrack = m_hWnd;
		tme.dwFlags = TME_LEAVE;
		tme.dwHoverTime = 1;
		TrackMouseEvent(&tme);

		RedrawWindow();
	}

	CButton::OnMouseMove(nFlags, point);
}


void CDrawButton::OnMouseLeave()
{
	if (m_isActive == true)
	{
		m_bOver = false;
		RedrawWindow();
	}

	CButton::OnMouseLeave();
}
