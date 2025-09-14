// (c) Electronic Arts. All Rights Reserved.
// DlgOnAction.cpp : implementation file
//

#include "stdafx.h"
#include "winbig.h"
#include "DlgOnAction.h"
#include ".\dlgonaction.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CWinBigApp theApp;

/////////////////////////////////////////////////////////////////////////////
// DlgOnAction dialog


DlgOnAction::DlgOnAction(CWnd* pParent /*=NULL*/)
	: CDialog(DlgOnAction::IDD, pParent)
{
	//{{AFX_DATA_INIT(DlgOnAction)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void DlgOnAction::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DlgOnAction)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(DlgOnAction, CDialog)
	//{{AFX_MSG_MAP(DlgOnAction)
	ON_BN_CLICKED(IDC_ON_ACTION_NEVER_AGAIN, OnOnActionNeverAgain)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// DlgOnAction message handlers

void DlgOnAction::OnOnActionNeverAgain() 
{
	// TODO: Add your control notification handler code here
	theApp.WriteProfileInt("Settings","SkipDlgOnAction",1);
}

void DlgOnAction::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	OnOK();
}
