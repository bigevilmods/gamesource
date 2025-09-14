// (c) Electronic Arts. All Rights Reserved.
// WinBig.cpp : Defines the class behaviors for the application.
//

#include "afxole.h"
#include "stdafx.h"
#include "WinBig.h"

#include "MainFrm.h"
#include "WinBigDoc.h"
#include "WinBigView.h"
#include "dlgwishlist.h"
#include "dlgProperties.h"

#include <afxadv.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CWinBigApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CWinBigApp

BEGIN_MESSAGE_MAP(CWinBigApp, CWinApp)
	//{{AFX_MSG_MAP(CWinBigApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	ON_COMMAND(ID_APP_WISHLIST, OnAppWishlist)
	ON_COMMAND(ID_EDIT_PROPERTIES, OnEditProperties)
	//}}AFX_MSG_MAP
	// Standard file based document commands
	ON_UPDATE_COMMAND_UI(ID_FILE_MRU_FILE1, OnUpdateRecentFileMenu)
	ON_COMMAND_EX_RANGE(ID_FILE_MRU_FILE1, ID_FILE_MRU_FILE16, OnOpenRecentFile)
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)

END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWinBigApp construction

CWinBigApp::CWinBigApp()
{
	// Place all significant initialization in InitInstance
	m_version = "1.34";
	m_fileAlignment = 64;
	m_packFile = FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CWinBigApp object

CWinBigApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CWinBigApp initialization

BOOL CWinBigApp::InitInstance()
{
	if (!AfxOleInit())
	{
		::AfxMessageBox(_T("AfxOLEInit failed!  No idea why?"));
		return FALSE;
	}

	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
//	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
//	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	// Change the registry key under which our settings are stored.
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization.
	SetRegistryKey(_T("EAC Tools"));

	LoadStdProfileSettings();  // Load standard INI file options (including MRU)

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.

	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CWinBigDoc),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CWinBigView));
	AddDocTemplate(pDocTemplate);

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// Dispatch commands specified on the command line
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	// The one and only window has been initialized, so show and update it.
	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();

	m_pMainWnd->DragAcceptFiles(TRUE);

	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	CString	m_historyText;
	CString	m_version;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	afx_msg void OnUpdateHistory();
	afx_msg void OnSendtommail();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	m_historyText = _T("");
	m_version = theApp.m_version;
	//}}AFX_DATA_INIT
	
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	DDX_Text(pDX, IDC_HISTORY, m_historyText);
	DDX_Text(pDX, IDC_VERSION, m_version);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
	ON_EN_UPDATE(IDC_HISTORY, OnUpdateHistory)
	ON_BN_CLICKED(IDC_SENDTOMMAIL, OnSendtommail)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
END_MESSAGE_MAP()

// App command to run the dialog
void CWinBigApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.m_historyText.Format( 
"\
Feb 14, 2005        Version 1.34\r\n\
\t-Fixed drag and drop crash \r\n\
Nov 12, 2004        Version 1.33\r\n\
\t-Double click open files that contain slashes (subdirectories) and/or colons \r\n\
\t-Increased refpack buffer to avoid crash on files that grow in size\r\n\
\t-Use release codex library\r\n\
\t-Default alignment 64\r\n\
June 9, 2003        Version 1.32\r\n\
\t-Removed support for Bzip2 codec\r\n\
April 1, 2003       Version 1.31\r\n\
\t-The WinBig tool is now supported by the Real Core group of the EAC\r\n\
\t Tools & Libraries department.  Contact helpdesk-tools@ea.com for support.\r\n\
\t-Added support for bzip2 compression.\r\n\
\t-Disabled the auto-notification for newer versions of the tool.\r\n\
Sept 12, 2001		Version 1.30\r\n\
\t-[Enter] opens files.\r\n\
\t-Now shows number of selected files.\r\n\
\t-Dragging a .big, .abg, .viv file will prompt for open/add.\r\n\
\t-Allow replacement of already existing files\r\n\
\t-Progress bars on load/save\r\n\
June 6, 2001		Version 1.22\r\n\
\t-Ctrl+A now works.\r\n\
May  3, 2001		Version 1.21\r\n\
\t- More unique name so two samed named lumpy files can be opened\r\n\
\t- Fixed dragging out filename creation\r\n\
Apr 22, 2001		Version 1.2\r\n\
\t- Default alignment is set to an educated guess\r\n\
\t- Same names files inside of two different big files no longer conflict\r\n\
\t  in the temp directory.\r\n\
\t- Changing properties counts as a change now :)\r\n\
Dec 30, 2000		Version 1.1\r\n\
\t- Auto-notification if the version changes (and you have an R: drive) :)\r\n\
\t- Fixed a small memory leak\r\n\
\t- Nice number formatting (comma seperated)\r\n\
Nov  6, 2000		Version 1.0\r\n\
\tInitial Release by Tom Niwinski, EAC Funatic.\r\n\
  ");

	aboutDlg.DoModal();
}

/////////////////////////////////////////////////////////////////////////////
// CWinBigApp message handlers


void CAboutDlg::OnUpdateHistory() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function to send the EM_SETEVENTMASK message to the control
	// with the ENM_UPDATE flag ORed into the lParam mask.
	
	// TODO: Add your control notification handler code here
	
}

void CAboutDlg::OnSendtommail() 
{
	CString title;
	title.Format("mailto:helpdesk-tools@ea.com?subject=WinBIG (Ver %s): Hey why doesn't WinBIG do...",theApp.m_version);
	// TODO: Add your control notification handler code here
	ShellExecute(m_hWnd,NULL,title,NULL,NULL,SW_SHOWNORMAL); 
}

void CWinBigApp::OnAppWishlist() 
{
	DlgWishList dlg;
	dlg.DoModal();
}

void CWinBigApp::OnEditProperties() 
{
	DlgProperties dlg;
	dlg.m_fileAlignment = m_fileAlignment;
	dlg.m_packFile = m_packFile;

	if (dlg.DoModal() == IDOK)
	{
		// make sure that the alignment is a power of two.
		this->m_fileAlignment = dlg.m_fileAlignment;
		this->m_packFile = dlg.m_packFile;
	}
}

BOOL CWinBigApp::OnOpenRecentFile(UINT nID)
{
	int nIndex = nID - ID_FILE_MRU_FILE1;

	CString strMRU = (*m_pRecentFileList)[nIndex];

	this->OpenDocumentFile(strMRU); 

	return true;
}

void CAboutDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	OnOK();
}
