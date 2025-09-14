// (c) Electronic Arts. All Rights Reserved.
// DlgWishList.cpp : implementation file
//

#include "stdafx.h"
#include "winbig.h"
#include "DlgWishList.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CWinBigApp theApp;

/////////////////////////////////////////////////////////////////////////////
// DlgWishList dialog


DlgWishList::DlgWishList(CWnd* pParent /*=NULL*/)
	: CDialog(DlgWishList::IDD, pParent)
{
	//{{AFX_DATA_INIT(DlgWishList)
	m_wishlistText = _T("");
	//}}AFX_DATA_INIT

	m_wishlistText.Format( 
"\
These will be done in semi-random order when I have free time.\r\n\
Requests will increase priorities of items. \r\n\
If you know an easy way to do any of the below let me know.\r\n\
\r\n\
-Spawn Notepad if there is no viewer associated with a view file\r\n\
-On error bring the app to the foreground (esp when dragging files)\r\n\
-Keep window positions/column widths\r\n\
-Auto register .VIV and .BIG files\r\n\
-Copy filenames onto clipboard as Text also (so you can paste as text)\r\n\
-Allow paste, not just drag in.\r\n\
-Show icons by the filenames\r\n\
-Aborting long load procedures\r\n\
-Proper types support (show the type not just the extension)\r\n\
-Edited labels won't resize properly\r\n\
-Auto compress a huffed file on save.\r\n\
-24 bit BIG file support (old style)\r\n\
-40 bit BIG file support (new style)\r\n\
-Sorting arrows on column titles\r\n\
-Showing sort icons in the header rows\r\n\
\r\n\
\r\n\
");
}


void DlgWishList::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DlgWishList)
	DDX_Text(pDX, IDC_WISHLIST_TEXT, m_wishlistText);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(DlgWishList, CDialog)
	//{{AFX_MSG_MAP(DlgWishList)
	ON_BN_CLICKED(IDC_SENDTOMMAIL, OnSendtommail)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// DlgWishList message handlers

void DlgWishList::OnSendtommail()
{
	CString title;
	title.Format("mailto:ignitionsupport@ea.com?subject=WinBIG(Ver %s): Request",
        theApp.m_version);
	ShellExecute(m_hWnd,NULL,title,NULL,NULL,SW_SHOWNORMAL);
}
