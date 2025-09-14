// (c) Electronic Arts. All Rights Reserved.
// DlgProperties.cpp : implementation file
//

#include "stdafx.h"
#include "winbig.h"
#include "DlgProperties.h"
#include ".\dlgproperties.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// DlgProperties dialog


DlgProperties::DlgProperties(CWnd* pParent /*=NULL*/)
	: CDialog(DlgProperties::IDD, pParent)
{
	//{{AFX_DATA_INIT(DlgProperties)
	m_fileAlignment = 0;
	m_packFile = FALSE;
	//}}AFX_DATA_INIT
}


void DlgProperties::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DlgProperties)
	DDX_Text(pDX, IDC_FILE_ALIGNMENT, m_fileAlignment);
	DDX_Check(pDX, IDC_PACK_FILE, m_packFile);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(DlgProperties, CDialog)
	//{{AFX_MSG_MAP(DlgProperties)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
	ON_EN_CHANGE(IDC_FILE_ALIGNMENT, OnEnChangeFileAlignment)
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// DlgProperties message handlers

void DlgProperties::OnEnChangeFileAlignment()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}

void DlgProperties::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	OnCancel();
}
