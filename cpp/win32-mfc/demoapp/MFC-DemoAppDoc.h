
// MFC-DemoAppDoc.h : interface of the CMFCDemoAppDoc class
//


#pragma once


class CMFCDemoAppDoc : public CDocument
{
protected: // create from serialization only
	CMFCDemoAppDoc() noexcept;
	DECLARE_DYNCREATE(CMFCDemoAppDoc)

// Attributes
public:

// Operations
public:
	void selectDevice();
	void showPropertyDialog();

	std::shared_ptr<ic4::ImageBuffer> captureImage(ic4::Error& err);

// Overrides
public:
	virtual BOOL OnNewDocument();

// Implementation
public:
	virtual ~CMFCDemoAppDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

private:
	ic4::Grabber m_Grabber;
	std::shared_ptr<ic4::SnapSink> m_pSink;

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()

public:	
};
