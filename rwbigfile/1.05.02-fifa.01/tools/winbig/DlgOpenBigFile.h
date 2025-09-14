// (c) Electronic Arts. All Rights Reserved.
#if !defined(AFX_DLGOPENBIGFILE_H__CADF4F95_8D5B_43AB_905E_4FBF7E712F6F__INCLUDED_)
#define AFX_DLGOPENBIGFILE_H__CADF4F95_8D5B_43AB_905E_4FBF7E712F6F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgOpenBigFile.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// DlgOpenBigFile dialog

class DlgOpenBigFile : public CDialog
{
// Construction
public:
	DlgOpenBigFile(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(DlgOpenBigFile)
	enum { IDD = IDD_OPEN_BIGFILE };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(DlgOpenBigFile)
	public:
	virtual int DoModal();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// TOM's own
public:
	void SetBigFileName(CString name);
private:
	CString m_bigFileName;

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(DlgOpenBigFile)
	virtual BOOL OnInitDialog();
	afx_msg void OnOpenBigOpen();
	afx_msg void OnOpenBigAdd();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGOPENBIGFILE_H__CADF4F95_8D5B_43AB_905E_4FBF7E712F6F__INCLUDED_)
