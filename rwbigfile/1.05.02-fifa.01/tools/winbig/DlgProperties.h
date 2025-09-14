// (c) Electronic Arts. All Rights Reserved.
#if !defined(AFX_DLGPROPERTIES_H__39E1DAA6_EB97_4A1A_B570_31CE259FC60D__INCLUDED_)
#define AFX_DLGPROPERTIES_H__39E1DAA6_EB97_4A1A_B570_31CE259FC60D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgProperties.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// DlgProperties dialog

class DlgProperties : public CDialog
{
// Construction
public:
	DlgProperties(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(DlgProperties)
	enum { IDD = IDD_PROPERTIES };
	UINT	m_fileAlignment;
	BOOL	m_packFile;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(DlgProperties)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(DlgProperties)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnEnChangeFileAlignment();
	afx_msg void OnBnClickedCancel();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGPROPERTIES_H__39E1DAA6_EB97_4A1A_B570_31CE259FC60D__INCLUDED_)
