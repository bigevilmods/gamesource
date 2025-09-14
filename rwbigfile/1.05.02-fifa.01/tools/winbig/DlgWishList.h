// (c) Electronic Arts. All Rights Reserved.
#if !defined(AFX_DLGWISHLIST_H__4E0E4F01_B8FC_4AFF_8B9A_3BCAABF8D0D0__INCLUDED_)
#define AFX_DLGWISHLIST_H__4E0E4F01_B8FC_4AFF_8B9A_3BCAABF8D0D0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgWishList.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// DlgWishList dialog

class DlgWishList : public CDialog
{
// Construction
public:
	DlgWishList(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(DlgWishList)
	enum { IDD = IDD_DIALOG_WISHLIST };
	CString	m_wishlistText;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(DlgWishList)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(DlgWishList)
	afx_msg void OnSendtommail();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGWISHLIST_H__4E0E4F01_B8FC_4AFF_8B9A_3BCAABF8D0D0__INCLUDED_)
