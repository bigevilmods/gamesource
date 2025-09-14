// (c) Electronic Arts. All Rights Reserved.
// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "WinBig.h"
#include "winbigdoc.h"
#include "MainFrm.h"
#include "WinBigView.h"
#include "dlgOpenBigFile.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CWinBigApp theApp;


/////////////////////////////////////////////////////////////////////////////
// CMainFrame
static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_STATUSBAR_NUM_OF_FILES,           // Number of files
//	ID_SEPARATOR,           // Was it compressed
//	ID_INDICATOR_CAPS,
//	ID_INDICATOR_NUM,
//	ID_INDICATOR_SCRL,
};

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_WM_DROPFILES()
	//}}AFX_MSG_MAP

	// WinBig's special updates
	// TOM TODO:  this is not right it gets updated way too often
	ON_UPDATE_COMMAND_UI(ID_STATUSBAR_NUM_OF_FILES, OnUpdateNumOfFiles)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here

}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	// add the number of files status column
	m_wndStatusBar.SetPaneInfo(1, ID_STATUSBAR_NUM_OF_FILES, SBPS_NORMAL, 140);

	// TODO: Delete these three lines if you don't want the toolbar to
	//  be dockable
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);

	return 0;
}

void CMainFrame::OnUpdateNumOfFiles(CCmdUI *pCmdUI)
{
	TRACE("CMainFrame::OnUpdateNumOfFiles(CCmdUI *pCmdUI) called.\n");

	// this is lame as this function gets called really often now!
	CString s;

	// figure out how many are selected.
	POSITION pos = ((CWinBigDoc *)GetActiveDocument())->GetFirstViewPosition();
	CWinBigView* pFirstView = (CWinBigView*)((CWinBigDoc *)GetActiveDocument())->GetNextView( pos );
	int selectedCount = pFirstView->GetListCtrl().GetSelectedCount();

	// ........................................................................

	unsigned __int64 intTotalSizeSelected = 0;
	// when document is initially loaded or user close a document
	if ( selectedCount == 0 )
	{
		for (int i=0; i<((CWinBigDoc*) GetActiveDocument())->GetNumFiles(); i++)
		{
			LF_INFO *lfi = (LF_INFO *)pFirstView->GetListCtrl().GetItemData(i);
			intTotalSizeSelected += lfi->size; 
		}
	}
	// when user select an item or multiple items
	else
	{
		pos = pFirstView->GetListCtrl().GetFirstSelectedItemPosition(); 
		while (pos)
		{
			int intSelectedItem = pFirstView->GetListCtrl().GetNextSelectedItem(pos);
			LF_INFO *lfi = (LF_INFO *)pFirstView->GetListCtrl().GetItemData(intSelectedItem);
			intTotalSizeSelected += lfi->size; 
		}
	}

	// ........................................................................

	CString strTemp = "";
	char buf[256];
	
	// enchance the string for total files size for easy reading off the status bar
	strTemp.Format("%s", CWinBigView::FormatWithCommas(buf, intTotalSizeSelected));
	CString strTotalSizeSelected;
	// format the string for total files size
	strTotalSizeSelected = "    " + strTemp + " Bytes";
	
	// ........................................................................

	if (selectedCount > 0)
	{
		s.Format(_T(" %d File(s) Selected"),selectedCount);
	}
	else
	{
		s.Format(_T(" %d File(s)"),((CWinBigDoc *)GetActiveDocument())->GetNumFiles());
	}

	// ........................................................................

	s += strTotalSizeSelected;
	int intIndex = m_wndStatusBar.CommandToIndex(ID_STATUSBAR_NUM_OF_FILES);
	CWindowDC dc(&m_wndStatusBar);
	CSize sizeText = dc.GetTextExtent(s);
	// shorten the length of the status pane
	sizeText.cx = static_cast<LONG> (0.77 * sizeText.cx);
	m_wndStatusBar.SetPaneInfo(intIndex, ID_STATUSBAR_NUM_OF_FILES, SBPS_NORMAL, sizeText.cx);

	// ........................................................................

	pCmdUI->SetText(s);	
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers



void CMainFrame::OnDropFiles(HDROP hDropInfo) 
{
	// check what kind of file we dragged on, if its registered with WinBig then open it, otherwise 
	// add it to current lumpy file.

	TRACE("CMainFrame::OnDropFiles(HDROP hDropInfo) called.\n");
	
	TCHAR szFile[MAX_PATH];

	if (hDropInfo)
	{
		int nCount = ::DragQueryFile(hDropInfo, (UINT)-1, NULL, 0);

		if (nCount == 1)
		{
			//TODO.
			// is it a big type file?  If so bring up a dialog and see if we want to add or open
			::DragQueryFile(hDropInfo, 0, szFile, MAX_PATH);

			// get extension, etc...
			char drive[_MAX_DRIVE];
			char dir[_MAX_DIR];
			char fname[_MAX_FNAME];
			char ext[_MAX_EXT];
			_splitpath(szFile,drive,dir,fname,ext);

			char filename[MAX_PATH];
			sprintf(filename,"%s%s",fname,ext);

			// ideally this would be an option in the properties setting.
			if ((_stricmp(ext,".BIG") == 0) ||
				(_stricmp(ext,".VIV") == 0) ||
				(_stricmp(ext,".ABG") == 0))
			{
				DlgOpenBigFile dlg;
				dlg.SetBigFileName(filename);
				int status = dlg.DoModal();

				if (status == 2)
				{
					// open the big file.
					theApp.OpenDocumentFile(szFile);
					return;
				}
				else if (status == 3)
				{
					// add the big file, just drop on through
					;
				}
				else 
				{
					// cancel or abort or error
					return;
				}
			}
		}

		for (int i = 0; i < nCount; i++)
		{
			::DragQueryFile(hDropInfo, i, szFile, MAX_PATH);

			// get extension, etc...
		    char drive[_MAX_DRIVE];
		    char dir[_MAX_DIR];
		    char fname[_MAX_FNAME];
		    char ext[_MAX_EXT];

			_splitpath(szFile,drive,dir,fname,ext);
			sprintf(fname,"%s%s",fname,ext);

			if (!((CWinBigDoc* )GetActiveDocument())->AddFile(szFile,fname,ext,FALSE))
			{
				// bring up a yes no dialog

				CString error;
				error.Format("File %s already exists in this archive\nDo you want to replace it?",fname);
				if (::AfxMessageBox(error, MB_YESNO | MB_APPLMODAL | MB_ICONEXCLAMATION) == IDYES)
				{
					((CWinBigDoc* )GetActiveDocument())->AddFile(szFile,fname,ext,TRUE);
				}
			}
		}

		// figure out how many are selected.
		POSITION pos = ((CWinBigDoc *)GetActiveDocument())->GetFirstViewPosition();
		CWinBigView* pFirstView = (CWinBigView*)((CWinBigDoc *)GetActiveDocument())->GetNextView( pos );
		// get the embedded list control in the list view
		CListCtrl& pListCtrl = pFirstView->GetListCtrl();
		// set minimum width for column 0
		pListCtrl.SetColumnWidth(0, LVSCW_AUTOSIZE); 

	}
	return;
	CFrameWnd::OnDropFiles(hDropInfo);
}
