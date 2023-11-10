
// MFC-DemoAppDoc.cpp : implementation of the CMFCDemoAppDoc class
//

#include "pch.h"
#include "framework.h"

#include "MFC-DemoApp.h"
#include "MFC-DemoAppDoc.h"
#include "MFC-DemoAppView.h"

#include "StringUtil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CMFCDemoAppDoc

IMPLEMENT_DYNCREATE(CMFCDemoAppDoc, CDocument)

BEGIN_MESSAGE_MAP(CMFCDemoAppDoc, CDocument)
END_MESSAGE_MAP()


// CMFCDemoAppDoc construction/destruction

CMFCDemoAppDoc::CMFCDemoAppDoc() noexcept
{
	// TODO: add one-time construction code here

}

CMFCDemoAppDoc::~CMFCDemoAppDoc()
{
}

BOOL CMFCDemoAppDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	return TRUE;
}

// CMFCDemoAppDoc diagnostics

#ifdef _DEBUG
void CMFCDemoAppDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CMFCDemoAppDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CMFCDemoAppDoc commands

void CMFCDemoAppDoc::selectDevice()
{
	m_Grabber.streamStop();

	ic4gui::showDeviceDialog(AfxGetMainWnd()->GetSafeHwnd(), m_Grabber);

	m_pSink = ic4::SnapSink::create();

	auto pos = GetFirstViewPosition();
	auto* view = GetNextView(pos);
	auto display = dynamic_cast<CMFCDemoAppView*>(view)->GetDisplay();

	m_Grabber.streamSetup(m_pSink, display);

	auto name = m_Grabber.deviceInfo().modelName();
	std::wstring wname = Win32StringUtil::wstringFromUtf8(name);

	SetTitle(wname.c_str());
}

void CMFCDemoAppDoc::showPropertyDialog()
{
	ic4gui::PropertyDialogOptions options = {
		ic4gui::PropertyDialogFlags::AllowStreamRestart
	};
	ic4gui::showDevicePropertyDialog(AfxGetMainWnd()->GetSafeHwnd(), m_Grabber, options);
}

std::shared_ptr<ic4::ImageBuffer> CMFCDemoAppDoc::captureImage(ic4::Error& err)
{
	return m_pSink->snapSingle(4000, err);
}
