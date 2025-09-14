// (c) Electronic Arts. All Rights Reserved.
// DlgOpenBigFile.cpp : implementation file
//

#include "stdafx.h"
#include "winbig.h"
#include "DlgOpenBigFile.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// DlgOpenBigFile dialog


DlgOpenBigFile::DlgOpenBigFile(CWnd* pParent /*=NULL*/)
	: CDialog(DlgOpenBigFile::IDD, pParent)
{
	//{{AFX_DATA_INIT(DlgOpenBigFile)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void DlgOpenBigFile::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DlgOpenBigFile)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(DlgOpenBigFile, CDialog)
	//{{AFX_MSG_MAP(DlgOpenBigFile)
	ON_BN_CLICKED(IDC_OPEN_BIG_OPEN, OnOpenBigOpen)
	ON_BN_CLICKED(IDC_OPEN_BIG_ADD, OnOpenBigAdd)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// DlgOpenBigFile message handlers

BOOL DlgOpenBigFile::PreCreateWindow(CREATESTRUCT& cs) 
{
	// TODO: Add your specialized code here and/or call the base class

	return CDialog::PreCreateWindow(cs);
}

int DlgOpenBigFile::DoModal() 
{
	// TODO: Add your specialized code here and/or call the base class
	
	return CDialog::DoModal();
}

BOOL DlgOpenBigFile::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	CString buf;
	buf = "Do you want to ADD or OPEN " + this->m_bigFileName + "?";
	this->SetDlgItemText(IDC_OPEN_BIG_TEXT,buf);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void DlgOpenBigFile::SetBigFileName(CString name)
{
	m_bigFileName = name;
}

void DlgOpenBigFile::OnOpenBigOpen() 
{
	this->EndDialog(2);
}

void DlgOpenBigFile::OnOpenBigAdd() 
{
	this->EndDialog(3);
}
