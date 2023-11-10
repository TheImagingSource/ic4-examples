
// MFC-DemoApp.h : main header file for the MFC-DemoApp application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols


// CDemoApp:
// See MFC-DemoApp.cpp for the implementation of this class
//

class CDemoApp : public CWinApp
{
public:
	CDemoApp() noexcept;


// Overrides
public:
	virtual BOOL InitInstance();

// Implementation
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CDemoApp theApp;
