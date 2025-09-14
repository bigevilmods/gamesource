// (c) Electronic Arts. All Rights Reserved.
// SpawnExternalApp_.cpp : implementation file
//

#include "stdafx.h"
#include "WinBig.h"
#include "SpawnExternalApp_.h"
#include ".\spawnexternalapp_.h"


// CSpawnExternalApp_ dialog

IMPLEMENT_DYNAMIC(CSpawnExternalApp_, CDialog)
CSpawnExternalApp_::CSpawnExternalApp_(CWnd* pParent /*=NULL*/)
	: CDialog(CSpawnExternalApp_::IDD, pParent)
	, m_strFileNameToDisplay(_T(""))
{
	m_strFileNameToDisplay = "";
}

CSpawnExternalApp_::~CSpawnExternalApp_()
{
}

void CSpawnExternalApp_::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CSpawnExternalApp_, CDialog)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
	ON_WM_KEYDOWN()
	ON_WM_CHAR()
END_MESSAGE_MAP()


// CSpawnExternalApp_ message handlers

void CSpawnExternalApp_::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	OnOK();
}

void CSpawnExternalApp_::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	OnCancel();
}

void CSpawnExternalApp_::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: Add your message handler code here and/or call default

	CDialog::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CSpawnExternalApp_::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: Add your message handler code here and/or call default

	CDialog::OnChar(nChar, nRepCnt, nFlags);
}

BOOL CSpawnExternalApp_::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class

	static int i = 0;
	TRACE("Key pressed on SpawnExternalApp dialog - %d\n", ++i);

	if ((pMsg->message == WM_KEYDOWN) && (pMsg->wParam == 'X'))
	{
		TRACE("WM_CLOSE sent\n");
		SendMessage(WM_CLOSE, 0, 0);
	}

	return CDialog::PreTranslateMessage(pMsg);
}

void CSpawnExternalApp_::SetFileNameToDisplay(CString strFileName)
{
	m_strFileNameToDisplay = strFileName;
}

BOOL CSpawnExternalApp_::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here

	// get a pointer to the CStatic control displaying the filename
	CStatic* oStcFileNameToDisplay = (CStatic*) GetDlgItem(IDC_FILE_WITH_NO_VIEWER);
	// check validity of pointer obtained
	ASSERT_VALID(oStcFileNameToDisplay);
	// setup the string
	CString strTemp = "\" " + m_strFileNameToDisplay + " \"";
	// set filename to be displayed
	oStcFileNameToDisplay->SetWindowText(strTemp);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
