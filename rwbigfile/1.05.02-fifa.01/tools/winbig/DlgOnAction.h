// (c) Electronic Arts. All Rights Reserved.
#if !defined(AFX_DLGONACTION_H__323BF99C_BE19_4F32_A8DA_85FC61C5D24F__INCLUDED_)
#define AFX_DLGONACTION_H__323BF99C_BE19_4F32_A8DA_85FC61C5D24F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgOnAction.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// DlgOnAction dialog

class DlgOnAction : public CDialog
{
// Construction
public:
	DlgOnAction(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(DlgOnAction)
	enum { IDD = IDD_ON_ACTION };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(DlgOnAction)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(DlgOnAction)
	afx_msg void OnOnActionNeverAgain();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGONACTION_H__323BF99C_BE19_4F32_A8DA_85FC61C5D24F__INCLUDED_)
