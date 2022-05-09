#pragma once


// CDrawButton



class CDrawButton : public CButton
{
	DECLARE_DYNAMIC(CDrawButton)

private:
	bool m_bOver = false;
	bool m_isActive = true;
	BLENDFUNCTION m_bf;

	CBitmap m_bmpNormal;
	CBitmap m_bmpOver;
	CBitmap m_bmpSelect;
	CBitmap m_bmpDisabled;

	void CreateBtnBitmap(CBitmap& bmp, COLORREF color, void(*Fun)(CDC& dc, CRect& rect, bool isActive));
public:
	CDrawButton();
	virtual ~CDrawButton();
	
	void SetButtonInfo(COLORREF Normal, COLORREF Over, COLORREF Select, BYTE Alpha, bool isActive, void(*Fun)(CDC& dc, CRect& rect, bool isActive));
	//BYTE는 0(투명) ~ 255(반투명) 까지
	//키워드 MFC 알파블렌딩
	//http://blog.naver.com/PostView.nhn?blogId=reverse_ing&logNo=60137695747
	//http://egloos.zum.com/EireneHue/v/969409

protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg void OnDestroy();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();
};


