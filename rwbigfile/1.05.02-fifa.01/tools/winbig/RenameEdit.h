// (c) Electronic Arts. All Rights Reserved.
#if !defined(AFX_RENAMEEDIT_H__9E4B6C34_5613_44ED_9301_70A52426F2EA__INCLUDED_)
#define AFX_RENAMEEDIT_H__9E4B6C34_5613_44ED_9301_70A52426F2EA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// RenameEdit.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CRenameEdit window

class CRenameEdit : public CEdit
{
// Construction
public:
	CRenameEdit();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRenameEdit)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CRenameEdit();

	// Generated message map functions
protected:
	//{{AFX_MSG(CRenameEdit)
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RENAMEEDIT_H__9E4B6C34_5613_44ED_9301_70A52426F2EA__INCLUDED_)
