// (c) Electronic Arts. All Rights Reserved.
#if !defined(AFX_DLGPACKEDBIGFILE_H__5C36C49A_90B8_409F_AF5B_CB1DFBF7B551__INCLUDED_)
#define AFX_DLGPACKEDBIGFILE_H__5C36C49A_90B8_409F_AF5B_CB1DFBF7B551__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgPackedBigFile.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// DlgPackedBigFile dialog

class DlgPackedBigFile : public CDialog
{
// Construction
public:
	DlgPackedBigFile(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(DlgPackedBigFile)
	enum { IDD = IDD_PACKED_BIGFILE };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(DlgPackedBigFile)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(DlgPackedBigFile)
	afx_msg void OnNeverAgain();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGPACKEDBIGFILE_H__5C36C49A_90B8_409F_AF5B_CB1DFBF7B551__INCLUDED_)
