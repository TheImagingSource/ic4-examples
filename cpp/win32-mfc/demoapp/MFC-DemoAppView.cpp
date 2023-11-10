
// MFC-DemoAppView.cpp : implementation of the CMFCDemoAppView class
//

#include "pch.h"
#include "framework.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "MFC-DemoApp.h"
#endif

#include "MFC-DemoAppDoc.h"
#include "MFC-DemoAppView.h"

#include "StringUtil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMFCDemoAppView

IMPLEMENT_DYNCREATE(CMFCDemoAppView, CView)

BEGIN_MESSAGE_MAP(CMFCDemoAppView, CView)
	ON_COMMAND(ID_DEVICE_SELECT, &CMFCDemoAppView::OnSelectDevice)
	ON_COMMAND(ID_DEVICE_PROPERTIES, &CMFCDemoAppView::OnDeviceProperties)
	ON_COMMAND(ID_CAPTURE_STILLIMAGE, &CMFCDemoAppView::OnCaptureStillImage)
	ON_COMMAND_EX(ID_DISPLAYMODE_TOPLEFT, &CMFCDemoAppView::OnDisplaymode)
	ON_COMMAND_EX(ID_DISPLAYMODE_CENTER, &CMFCDemoAppView::OnDisplaymode)
	ON_COMMAND_EX(ID_DISPLAYMODE_STRETCHTOPLEFT, &CMFCDemoAppView::OnDisplaymode)
	ON_COMMAND_EX(ID_DISPLAYMODE_STRETCHCENTER, &CMFCDemoAppView::OnDisplaymode)
	ON_UPDATE_COMMAND_UI(ID_DISPLAYMODE_TOPLEFT, &CMFCDemoAppView::OnUpdateDisplaymode)
	ON_UPDATE_COMMAND_UI(ID_DISPLAYMODE_CENTER, &CMFCDemoAppView::OnUpdateDisplaymode)
	ON_UPDATE_COMMAND_UI(ID_DISPLAYMODE_STRETCHTOPLEFT, &CMFCDemoAppView::OnUpdateDisplaymode)
	ON_UPDATE_COMMAND_UI(ID_DISPLAYMODE_STRETCHCENTER, &CMFCDemoAppView::OnUpdateDisplaymode)
END_MESSAGE_MAP()

// CMFCDemoAppView construction/destruction

CMFCDemoAppView::CMFCDemoAppView() noexcept
{
	// TODO: add construction code here

}

CMFCDemoAppView::~CMFCDemoAppView()
{
}

BOOL CMFCDemoAppView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

// CMFCDemoAppView drawing

void CMFCDemoAppView::OnDraw(CDC* /*pDC*/)
{
	CMFCDemoAppDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: add draw code for native data here
}

void CMFCDemoAppView::OnInitialUpdate()
{
	GetDocument()->selectDevice();
}

// CMFCDemoAppView diagnostics

#ifdef _DEBUG
void CMFCDemoAppView::AssertValid() const
{
	CView::AssertValid();
}

void CMFCDemoAppView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CMFCDemoAppDoc* CMFCDemoAppView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CMFCDemoAppDoc)));
	return (CMFCDemoAppDoc*)m_pDocument;
}
#endif //_DEBUG

std::shared_ptr<ic4::Display> CMFCDemoAppView::GetDisplay() const
{
	return m_pDisplay;
}

// CMFCDemoAppView message handlers


void CMFCDemoAppView::OnSelectDevice()
{
	GetDocument()->selectDevice();
}


void CMFCDemoAppView::OnDeviceProperties()
{
	GetDocument()->showPropertyDialog();
}

void CMFCDemoAppView::OnCaptureStillImage()
{
	ic4::Error err;
	auto image = GetDocument()->captureImage(err);
	if(err.isError())
	{
		::AfxMessageBox(Win32StringUtil::wstringFromUtf8(err.message()).c_str());
	}

	static TCHAR szFilter[] =
		_T("Bitmap Files (*.bmp)|*.bmp|")
		_T("TIFF Files (*.tif, *.tiff)|*.tif;*.tiff|")
		_T("JPEG Files (*.jpg, *.jpeg)|*.jpg;*.jpeg|")
		_T("PNG Files (*.png)|*.png|");

	CFileDialog dlg(FALSE, CString(".bmp"), NULL, 0, szFilter);

	if (dlg.DoModal() == IDOK)
	{
		auto ext = dlg.GetFileExt().MakeLower();
		if (ext == "bmp")
		{
			ic4::imageBufferSaveAsBitmap(*image, dlg.GetPathName(), {}, err);
		}
		else if (ext == "png")
		{
			ic4::imageBufferSaveAsPng(*image, dlg.GetPathName(), {}, err);
		}
		else if (ext == "jpg" || ext == "jpeg")
		{
			ic4::imageBufferSaveAsJpeg(*image, dlg.GetPathName(), {}, err);
		}
		else if (ext == "tif" || ext == "tiff")
		{
			ic4::imageBufferSaveAsTiff(*image, dlg.GetPathName(), {}, err);
		}

		if (err.isError())
		{
			::AfxMessageBox(Win32StringUtil::wstringFromUtf8(err.message()).c_str());
		}
	}
}

BOOL CMFCDemoAppView::OnDisplaymode(UINT id)
{
	switch (id)
	{
	case ID_DISPLAYMODE_TOPLEFT:
		m_displayRenderPosition = ic4::DisplayRenderPosition::TopLeft;
		break;
	case ID_DISPLAYMODE_CENTER:
		m_displayRenderPosition = ic4::DisplayRenderPosition::Center;
		break;
	case ID_DISPLAYMODE_STRETCHTOPLEFT:
		m_displayRenderPosition = ic4::DisplayRenderPosition::StretchTopLeft;
		break;
	case ID_DISPLAYMODE_STRETCHCENTER:
		m_displayRenderPosition = ic4::DisplayRenderPosition::StretchCenter;
		break;
	default:
		return FALSE;
	}

	m_pDisplay->setRenderPosition(m_displayRenderPosition);
	return TRUE;
}

void CMFCDemoAppView::OnUpdateDisplaymode(CCmdUI* pCmdUI)
{
	switch (pCmdUI->m_nID)
	{
	case ID_DISPLAYMODE_TOPLEFT:
		pCmdUI->SetCheck(m_displayRenderPosition == ic4::DisplayRenderPosition::TopLeft ? BST_CHECKED : BST_UNCHECKED);
		break;
	case ID_DISPLAYMODE_CENTER:
		pCmdUI->SetCheck(m_displayRenderPosition == ic4::DisplayRenderPosition::Center ? BST_CHECKED : BST_UNCHECKED);
		break;
	case ID_DISPLAYMODE_STRETCHTOPLEFT:
		pCmdUI->SetCheck(m_displayRenderPosition == ic4::DisplayRenderPosition::StretchTopLeft ? BST_CHECKED : BST_UNCHECKED);
		break;
	case ID_DISPLAYMODE_STRETCHCENTER:
		pCmdUI->SetCheck(m_displayRenderPosition == ic4::DisplayRenderPosition::StretchCenter ? BST_CHECKED : BST_UNCHECKED);
		break;
	}
}


BOOL CMFCDemoAppView::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext)
{
	if (!CView::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext))
	{
		return FALSE;
	}

	m_pDisplay = ic4::Display::create(ic4::DisplayType::Default, m_hWnd);
	m_displayRenderPosition = ic4::DisplayRenderPosition::StretchCenter;
	m_pDisplay->setRenderPosition(m_displayRenderPosition);

	return TRUE;
	
}
