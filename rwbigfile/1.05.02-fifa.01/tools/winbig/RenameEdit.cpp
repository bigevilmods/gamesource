// (c) Electronic Arts. All Rights Reserved.
// RenameEdit.cpp : implementation file
//

#include "stdafx.h"
#include "WinBig.h"
#include "RenameEdit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CRenameEdit

CRenameEdit::CRenameEdit()
{
}

CRenameEdit::~CRenameEdit()
{
}


BEGIN_MESSAGE_MAP(CRenameEdit, CEdit)
	//{{AFX_MSG_MAP(CRenameEdit)
	ON_WM_CHAR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRenameEdit message handlers


void CRenameEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// TODO: Add your message handler code here and/or call default
	
	CEdit::OnChar(nChar, nRepCnt, nFlags);
}
