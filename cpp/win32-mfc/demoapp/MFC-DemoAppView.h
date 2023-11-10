
// MFC-DemoAppView.h : interface of the CMFCDemoAppView class
//

#pragma once

#include <ic4/ic4.h>

class CMFCDemoAppView : public CView
{
protected: // create from serialization only
	CMFCDemoAppView() noexcept;
	DECLARE_DYNCREATE(CMFCDemoAppView)

// Attributes
public:
	CMFCDemoAppDoc* GetDocument() const;
	std::shared_ptr<ic4::Display> GetDisplay() const;

// Operations
public:

// Overrides
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnInitialUpdate();
protected:

// Implementation
public:
	virtual ~CMFCDemoAppView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSelectDevice();
	afx_msg void OnDeviceProperties();
	afx_msg void OnCaptureStillImage();
	afx_msg BOOL OnDisplaymode(UINT id);
	afx_msg void OnUpdateDisplaymode(CCmdUI* pCmdUI);

private:
	std::shared_ptr<ic4::Display> m_pDisplay;
	ic4::DisplayRenderPosition m_displayRenderPosition;
public:
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
};

#ifndef _DEBUG  // debug version in MFC-DemoAppView.cpp
inline CMFCDemoAppDoc* CMFCDemoAppView::GetDocument() const
   { return reinterpret_cast<CMFCDemoAppDoc*>(m_pDocument); }
#endif

