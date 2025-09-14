// (c) Electronic Arts. All Rights Reserved.
// DlgPackedBigFile.cpp : implementation file
//

#include "stdafx.h"
#include "winbig.h"
#include "DlgPackedBigFile.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CWinBigApp theApp;

/////////////////////////////////////////////////////////////////////////////
// DlgPackedBigFile dialog


DlgPackedBigFile::DlgPackedBigFile(CWnd* pParent /*=NULL*/)
	: CDialog(DlgPackedBigFile::IDD, pParent)
{
	//{{AFX_DATA_INIT(DlgPackedBigFile)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

}


void DlgPackedBigFile::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DlgPackedBigFile)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(DlgPackedBigFile, CDialog)
	//{{AFX_MSG_MAP(DlgPackedBigFile)
	ON_BN_CLICKED(IDC_NEVER_AGAIN, OnNeverAgain)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// DlgPackedBigFile message handlers

void DlgPackedBigFile::OnNeverAgain() 
{
	// TODO: Add your control notification handler code here
	theApp.WriteProfileInt("Settings","SkipDlgPackedBigFile",1);
}
