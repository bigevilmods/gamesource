// (c) Electronic Arts. All Rights Reserved.
// WinBig.h : main header file for the WINBIG application
//

#if !defined(AFX_WINBIG_H__310430BF_8239_4E41_9B42_E4A2D956B7ED__INCLUDED_)
#define AFX_WINBIG_H__310430BF_8239_4E41_9B42_E4A2D956B7ED__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CWinBigApp:
// See WinBig.cpp for the implementation of this class
//

class CWinBigApp : public CWinApp
{
public:
	BOOL m_packFile;
	UINT m_fileAlignment;
	CString m_version;
	CWinBigApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWinBigApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL
	virtual BOOL OnOpenRecentFile(UINT nID);
	

// Implementation
	//{{AFX_MSG(CWinBigApp)
	afx_msg void OnAppAbout();
	afx_msg void OnAppWishlist();
	afx_msg void OnEditProperties();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WINBIG_H__310430BF_8239_4E41_9B42_E4A2D956B7ED__INCLUDED_)
