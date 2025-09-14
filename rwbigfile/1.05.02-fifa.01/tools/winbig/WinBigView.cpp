// (c) Electronic Arts. All Rights Reserved.
// WinBigView.cpp : implementation of the CWinBigView class
//

#include "stdafx.h"
#include "WinBig.h"


#include "afxole.h"

#include "../spit/codex.h"
#include "../spit/refcodex.h"

#include "DlgOnAction.h"
#include "SpawnExternalApp_.h"



#include "WinBigDoc.h"
#include "WinBigView.h"

extern CWinBigApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CWinBigView

IMPLEMENT_DYNCREATE(CWinBigView, CListView)

BEGIN_MESSAGE_MAP(CWinBigView, CListView)
	//{{AFX_MSG_MAP(CWinBigView)
	ON_NOTIFY_REFLECT(LVN_GETDISPINFO, OnGetdispinfo)
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnColumnclick)
	ON_WM_DESTROY()
	ON_NOTIFY_REFLECT(HDN_ITEMDBLCLICK, OnItemdblclick)
	ON_WM_LBUTTONDBLCLK()
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_NOTIFY_REFLECT(LVN_BEGINDRAG, OnBegindrag)
	ON_WM_LBUTTONDOWN()
	ON_WM_DROPFILES()
	ON_WM_CONTEXTMENU()
	ON_NOTIFY_REFLECT(LVN_BEGINLABELEDIT, OnBeginlabeledit)
	ON_NOTIFY_REFLECT(LVN_ENDLABELEDIT, OnEndlabeledit)
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnDblclk)
	ON_COMMAND(ID_CONTEXT_ONFILE_COMPRESS, OnContextOnfileCompress)
	ON_COMMAND(ID_CONTEXT_ONFILE_UNDELETE, OnContextOnfileUndelete)
	ON_COMMAND(ID_CONTEXT_ONFILE_RENAME, OnContextOnfileRename)
	ON_COMMAND(ID_EDIT_PROPERTIES, OnEditProperties)
	ON_COMMAND(ID_EDIT_SELECT_ALL, OnEditSelectAll)
	ON_COMMAND(ID_CONTEXT_ONFILE_VIEW, ViewSelectedBigFiles)
	ON_COMMAND(ID_CONTEXT_ONFILE_COPY, OnEditCopy)
	ON_COMMAND(ID_CONTEXT_ONFILE_DELETE, OnEditDelete)
	ON_COMMAND(ID_DBL_CLK, OnDblClk)
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CListView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CListView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CListView::OnFilePrintPreview)
	ON_WM_CHAR()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWinBigView construction/destruction

CWinBigView::CWinBigView()
{
	// TODO: add construction code here

	// initialize flags
	m_IsEnterKeyPressedWhileRenaming = false;
	m_blnIsLabelEditing = false;

	// Initialize allocator and scheduler.
	EA::Jobs::SetAllocator(m_mallocAllocator);
	m_scheduler = new EA::Jobs::JobScheduler();
	m_scheduler->Initialize();
	EA::Jobs::StartWithDefaultThreads(*m_scheduler);
	m_decompressor = new rw::core::codec::BufferedChunkDecompressor(&m_mallocAllocator, m_scheduler);

}

CWinBigView::~CWinBigView()
{
}

BOOL CWinBigView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	cs.style &= ~LVS_TYPEMASK;
	cs.style |= (LVS_REPORT | LVS_EDITLABELS);

	return CListView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CWinBigView drawing

void CWinBigView::OnDraw(CDC*)
{
	CWinBigDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	// TODO: add draw code for native data here
}

void CWinBigView::OnInitialUpdate()
{
	static int firstTime = TRUE;

	CListView::OnInitialUpdate();

	// this is lame... how am I supposed to do it?
	if (!firstTime) return;

	firstTime = FALSE;
	// TODO: Add your specialized code here and/or call the base class
	
	// get our listCtrl
	CListCtrl& ListCtrl = GetListCtrl();

	ListCtrl.SetExtendedStyle(LVS_EX_GRIDLINES |     /* diplay gridlines */
                              LVS_EX_FULLROWSELECT | /* select entire row */
							  LVS_EX_ONECLICKACTIVATE |
							  LVS_EDITLABELS);       /* enable editing of labels */

	ListCtrl.InsertColumn(0,_T("Name"), LVCFMT_LEFT, 100);
	ListCtrl.InsertColumn(1,_T("Type"), LVCFMT_LEFT, 100);
	ListCtrl.InsertColumn(2,_T("Size"), LVCFMT_RIGHT, 100);
	ListCtrl.InsertColumn(3,_T("Offset"), LVCFMT_RIGHT, 100);
	ListCtrl.InsertColumn(4,_T("Packed(Org Size)"), LVCFMT_LEFT, 100);
	ListCtrl.InsertColumn(5,_T("Action"), LVCFMT_LEFT, 100);

	// create a image list
	m_oSortDirections.Create(IDB_SORT_DIRECTIONS, 16, 1, RGB(255,255,255));

	// get the header control embedded in the list control
	CHeaderCtrl* pHeaderCtrl = ListCtrl.GetHeaderCtrl();

	// set the image list into the header control
	pHeaderCtrl->SetImageList(&m_oSortDirections);
}

/////////////////////////////////////////////////////////////////////////////
// CWinBigView printing

BOOL CWinBigView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

#define PRINT_MARGIN 5

void CWinBigView::OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo)
{
	// TODO: add extra initialization before printing
	m_fontPrinter.CreatePointFont(100, "Arial", pDC);
	pDC->SelectObject(&m_fontPrinter);
	
	TEXTMETRIC tm;
	pDC->GetTextMetrics(&tm);

	m_lineHeight = tm.tmHeight + tm.tmExternalLeading;
	m_charWidth = tm.tmAveCharWidth;

	// figure out how many lines per page
	m_linesPerPage = (pDC->GetDeviceCaps(VERTRES)/m_lineHeight) - PRINT_MARGIN;

	UINT pages = max (1, (GetDocument()->GetNumFiles() / m_linesPerPage) + 1);

	pInfo->SetMaxPage(pages);

}

void CWinBigView::OnPrint(CDC* pDC, CPrintInfo* pInfo) 
{
	// go through the whole list and print each line
	CListCtrl& ListCtrl = GetListCtrl();
	LF_INFO *lfi;
	
	int nameX = m_charWidth * 6;
	int sizeX = nameX + (m_charWidth * 20);
	int offsetX = sizeX + (m_charWidth * 15); 
	int compressX = offsetX + (m_charWidth * 15); 

	//
	// print the header
	//
	int y = (3*m_lineHeight);
	pDC->TextOut(nameX,y,this->GetDocument()->GetPathName());
	// right align the page number ( 12 chars from the right)
	UINT rightX = pDC->GetDeviceCaps(HORZRES)-(12 * m_charWidth);
	CString page;
	page.Format(_T("Page:%d"),pInfo->m_nCurPage);
	pDC->TextOut(rightX,y,page);
	y += m_lineHeight + 5;  
	pDC->MoveTo(nameX,y);	// 5 pixel fudge factor
	pDC->LineTo(rightX+10*m_charWidth,y); // wow the fudge factors
	y += m_lineHeight;
	pDC->TextOut(nameX,y,_T("Name"));
	pDC->DrawText(_T("Size"),CRect(sizeX,y,offsetX,y+m_lineHeight),DT_RIGHT);
	pDC->DrawText(_T("Offset"),CRect(offsetX,y,compressX,y+m_lineHeight),DT_RIGHT);
	pDC->TextOut(compressX + (m_charWidth * 5), y, _T("Packed (Org Size)"));
	y += m_lineHeight;

	//
	// figure out which page to print!
	//
	UINT start = (pInfo->m_nCurPage-1) * m_linesPerPage;
	int end =	min(this->GetDocument()->GetNumFiles(),(int)((pInfo->m_nCurPage) * m_linesPerPage));

	CString str;
	for (int i = start; i < end; i++)
	{
		lfi = (LF_INFO *)ListCtrl.GetItemData(i);
		UINT py = ((i - start) * m_lineHeight) + y;
		pDC->TextOut(nameX,py,lfi->name);
		str.Format("%d",lfi->size);
		pDC->DrawText(str,CRect(sizeX,py,offsetX,py+m_lineHeight),DT_RIGHT);
		str.Format("%d",lfi->offset);
		pDC->DrawText(str,CRect(offsetX,py,compressX,py+m_lineHeight),DT_RIGHT);
		if (lfi->orgSize)
		{
			str.Format("Yes (%d)",lfi->orgSize);
		}
		else 
		{
			str = "No";
		}
		pDC->TextOut(compressX+(m_charWidth * 5),py,str);
	}
}

void CWinBigView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
	m_fontPrinter.DeleteObject();
}

/////////////////////////////////////////////////////////////////////////////
// CWinBigView diagnostics

#ifdef _DEBUG
void CWinBigView::AssertValid() const
{
	CListView::AssertValid();
}

void CWinBigView::Dump(CDumpContext& dc) const
{
	CListView::Dump(dc);
}

CWinBigDoc* CWinBigView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CWinBigDoc)));
	return (CWinBigDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CWinBigView message handlers

char* CWinBigView::FormatWithCommas(char *buf, __int64 num)
{
	if (num == 0)
	{
		*buf = '0';
		*(++buf) = 0;
		return --buf;
	}

	int i=0;
	char temp_buf[256]; // hope we don't get a number bigger than this!
	int cur_buf=0;

	while (num > 0)
	{
		unsigned char d = static_cast<unsigned char>(num % 10);
		d += '0'; // make it a char
		temp_buf[cur_buf] = d;
		cur_buf++;
		i++;
		num /= 10;

		if (i % 3 == 0 && num > 0)
		{
			// time to add a ,
			temp_buf[cur_buf] = ',';
			cur_buf++;
		}
		temp_buf[cur_buf] = 0;
	}
	// reverse the string
	cur_buf--;  // make this one smaller to properly account for number of chars in the string (0 based)
	for (int x=0; x <= cur_buf; x++)
	{
		buf[x] = temp_buf[cur_buf-x];
	}
	buf[cur_buf+1] = 0;
	
	return buf;
}

void CWinBigView::OnGetdispinfo(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
	CString string;
	LF_INFO *lfi = (LF_INFO *)pDispInfo->item.lParam;
	char buf[256];

	if (pDispInfo->item.mask & LVIF_TEXT)
	{
		// asking for the text that goes into a column (different text for differet columns)
		switch(pDispInfo->item.iSubItem)
		{
		case 0: // name
			::lstrcpy(pDispInfo->item.pszText, lfi->name);
			break;
		
		case 1: // type
			::lstrcpy(pDispInfo->item.pszText, lfi->type);
			break;
		case 2: // size
			::lstrcpy(pDispInfo->item.pszText, FormatWithCommas(buf, lfi->size));
			break;
		case 3: // offset
			::lstrcpy(pDispInfo->item.pszText, FormatWithCommas(buf, lfi->offset));
			break;
		
		case 4: // orgSize
			if (lfi->orgSize)
			{
				string.Format("Yes (%s)",FormatWithCommas(buf,lfi->orgSize));
			}
			else 
			{
				string = "No";
			}
			::lstrcpy(pDispInfo->item.pszText, string);
			break;

		case 5: // status
			string = "";
			if (lfi->action == LF_ADDCOMPRESS_REF)
			{
				string = "Add Ref Compressed";
			}
			else if (lfi->action == LF_COMPRESS_REF)
			{
				string = "Compress (Ref)";
			}
			else if (lfi->action == LF_ADD)
			{
				string = "Add";
			}
			else if (lfi->action == LF_DEL)
			{
				string = "Delete";
			}
			::lstrcpy(pDispInfo->item.pszText, string);
			break;

		default:
			// invalid case
			ASSERT(0);
			break;
		}
	}
	else
	{
		// it should always be text!
		ASSERT(0);
	}
	*pResult = 0;
}

static BOOL ascending = TRUE;

void CWinBigView::OnColumnclick(NMHDR* pNMHDR, LRESULT* pResult) 
{
	static int old_column = -1;
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	// figure our what order to sort it in.
	if (old_column != pNMListView->iSubItem)
	{
		ascending = TRUE;
		old_column = pNMListView->iSubItem;
	}
	else
	{
		ascending = !ascending;
	}

	// TODO: Add your control notification handler code here
	
	// want to sort the columns
	GetListCtrl().SortItems(CompareFunc,pNMListView->iSubItem);

	*pResult = 0;
	GetDocument()->SetModifiedFlag(TRUE);

	// get a pointer to the header control
	CHeaderCtrl* pHeaderCtrl = GetListCtrl().GetHeaderCtrl();
	// validate pointer obtained
	ASSERT_VALID(pHeaderCtrl);
	// get total columns in header
	int intTotalColumns = GetListCtrl().GetHeaderCtrl()->GetItemCount();
	// determine which column in header has been clicked
	int intColumnIdxClicked = pNMListView->iSubItem;

	for (int i=0; i<intTotalColumns; i++)
	{
		// get existing structure of header's column
		pHeaderCtrl->GetItem(i, &headerItem);
		headerItem.fmt |= HDF_IMAGE;
		headerItem.mask |= HDI_IMAGE;

		if (i == intColumnIdxClicked)
		{
			if (ascending)
				// assign ascending bitmap
				headerItem.iImage = 0;
			else
				// assign descending bitmap
				headerItem.iImage = 1;
		}
		else
		{
			// assign "cover-up" bitmap
			headerItem.iImage = 2;
		}

		// set changes to header's column
		pHeaderCtrl->SetItem(i, &headerItem);
	}

	pHeaderCtrl->Invalidate();
}

int CALLBACK CWinBigView::CompareFunc(LPARAM lp1, LPARAM lp2, LPARAM lps)
{
	__int64 nResult=0;
	LF_INFO *lf1 = (LF_INFO *)lp1;
	LF_INFO *lf2 = (LF_INFO *)lp2;

	// lParamSort contains a pointer to the list view control.
	// The lParam of an item is just its index.

	switch(lps)
	{
	case 0:
		nResult = lstrcmp(lf1->name,lf2->name);
		break;
	case 1:
		nResult = lstrcmp(lf1->type,lf2->type);
		break;
	case 2:
		nResult = lf1->size - lf2->size;
		break;
	case 3:
		nResult = lf1->offset - lf2->offset;
		break;
	case 4:
		nResult = lf1->orgSize - lf2->orgSize;
		break;
	case 5:
		nResult = lf1->action - lf2->action;
		break;
	}
	
	if (!ascending)
	{
		nResult *= -1;
	}
	return static_cast<int>(nResult);
}

LF_INFO *CWinBigView::AddItem(int index, CString name, CString ext, unsigned __int64 size, unsigned __int64 offset, unsigned __int64 orgSize, LF_ACTION action)
{
	LF_INFO *lfi;

	try 
	{
		lfi = new LF_INFO;
	}
	catch (CMemoryException *e)
	{	
		e->Delete();
		return FALSE;
	}

	lfi->name = name;
	lfi->type = ext;
	
	lfi->size = size;
	lfi->offset = offset;
	lfi->orgSize = orgSize;
	lfi->action = action;
	lfi->fileName = "";
	// now add the item to the list view
	LV_ITEM lvi;

	// get our listCtrl
	CListCtrl& ListCtrl = GetListCtrl();


	lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
	lvi.iItem = index;
	lvi.iSubItem = 0;
	lvi.pszText = LPSTR_TEXTCALLBACK;
	lvi.iImage = 0;
	lvi.lParam = (LPARAM)lfi;

	if (ListCtrl.InsertItem(&lvi) == -1)
	{
		// couldn't add item?
		ASSERT(0);
		return NULL;
	}

	return lfi;
}


void CWinBigView::CleanupList()
{
	// get our listCtrl
	CListCtrl& ListCtrl = GetListCtrl();
	
	for (int i = 0; i < ListCtrl.GetItemCount(); i++)
	{
		delete (LF_INFO *)ListCtrl.GetItemData(i);
	}

	ListCtrl.DeleteAllItems();
}

BOOL CWinBigView::DestroyWindow() 
{
	return CListView::DestroyWindow();
}

void CWinBigView::OnDestroy() 
{
	// delete all our memory when destroying the view
	CleanupList();

	// delete all the temp files we viewed
	POSITION pos = m_tempFiles.GetHeadPosition();
	while (pos)
	{
		DeleteFile(m_tempFiles.GetNext(pos));
	}

	// Free the decompressor and scheduler.
	delete m_scheduler;
	delete m_decompressor;

	CListView::OnDestroy();
}

void CWinBigView::OnItemdblclick(NMHDR*, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	
	*pResult = 0;
}

void CWinBigView::ViewSelectedBigFiles()
{
	// TODO: Add your control notification handler code here
	
	CListCtrl& listCtrl = GetListCtrl();
	int item = -1;

	if (listCtrl.GetSelectedCount() > 5)
	{
		if (IDOK != AfxMessageBox(_T("You are trying to view a lot of items, are you sure?"),MB_OKCANCEL))
		{
			return;
		}
	}

	CString tempName;
	// try and open the files.
	for (UINT i = 0; i < listCtrl.GetSelectedCount(); i++)
	{
		item = listCtrl.GetNextItem(item, LVNI_SELECTED);
		ASSERT(item != -1);
		//listCtrl.Update(item); 
		LF_INFO *lfi = (LF_INFO *)listCtrl.GetItemData(item);
		
		ExtractBigFileFile(lfi, tempName, TRUE, TRUE);

		// start the proper program 
		HINSTANCE  ret = ShellExecute(m_hWnd, "open", tempName, NULL, NULL, SW_SHOWNORMAL);
		if ((int)ret < 32)
		{ // shell execute operation failed at this point ...

			CSpawnExternalApp_ oSpawnExternalApp;
			oSpawnExternalApp.SetFileNameToDisplay(lfi->name);
			// display a dialog for user confirmation to view file using notepad
			if (oSpawnExternalApp.DoModal() == IDOK)
			{
				CString str = "";

				// display the wait cursor
				CWaitCursor oWaitCursor;

				// execute notepad application
				ShellExecute(this->m_hWnd,
					         "open",
							 "notepad.exe",
							 tempName,
							 "",
							 SW_SHOW );

				// restore cursor to original state
				oWaitCursor.Restore();
			}
		}
	}
}

void CWinBigView::OnDblclk(NMHDR*, LRESULT*)
{
	ViewSelectedBigFiles();
}

// this is dumb but I'm not sure how to vector both to the same command.
void CWinBigView::OnDblClk()
{
	ViewSelectedBigFiles();
}

void CWinBigView::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	
	CListView::OnLButtonDblClk(nFlags, point);
}

void CWinBigView::OnEditCopy()
{
	// TODO: Add your command handler code here
	// put a list of the files selected on the clipboard and a list of the names in text format!
	CListCtrl& listCtrl = GetListCtrl();
	UINT nSelections = listCtrl.GetSelectedCount();

	// if nothing is selected don't bother.
	if (nSelections < 1)
	{
		return;
	}

	// this will hold our list of HDROP formatted names (it has to end in a double NULL)
	UINT bufferLength = MAX_PATH*nSelections;
	TCHAR *fileNames = new TCHAR[bufferLength];
	CStringList tempFilesList;

	HANDLE hData = ExtractSelectedFilesToOLE(fileNames,bufferLength);

	COleDataSource *ods = new COleDataSource;
	ods->CacheGlobalData(CF_HDROP,hData);
	ods->SetClipboard();

	// also put the text onto the clipboard so you can paste it into word/etc
}

// this function will set up the clipboard/drag&drop files
// returns tempName which it the uncompressed version of the file.
void CWinBigView::ExtractBigFileFile(LF_INFO *lfi, CString &tempName, BOOL uncompress, BOOL uniqueName)
{
	CString inFile = GetDocument()->m_bigFile;

	// see if its already been extracted out?
	if (lfi->fileName.GetLength() > 0)
	{
		if (lfi->orgSize == 0 || !uncompress)
		{
			// we have our file already.
			tempName = lfi->fileName;
			return;
		}
		else
		{
			// look in the file on disk (its compressed so uncompress it to the proper directory.
			inFile = lfi->fileName;
		}
	}
	
	__int64 size;
	// tricky if we marked something for compression still use the
    // orgSize to extract (its not compressed until a save happens)
	if (lfi->action == LF_COMPRESS_REF || lfi->action == LF_ADDCOMPRESS_REF)
	{
		size = lfi->orgSize;
		uncompress = FALSE;
	}
	else
	{
		size = lfi->size;
	}

	// copy the file out of the big file
	char *inBuf = static_cast<char *>(_aligned_malloc(static_cast<size_t>(size), 16));
	char *unpackBuf = NULL;
	CFile fin(inFile,CFile::modeRead | CFile::typeBinary);
	fin.Seek(lfi->offset,CFile::begin);
	fin.Read(inBuf,static_cast<UINT>(size));
	fin.Close();

	// write the file to a temporary file.
	// we need to do this in a directory of the big file name, so that 
	// if you open two different big files with the same info they don't conflict.
	TCHAR tempPath[256];
	::GetTempPath(256,&tempPath[0]);

	if (uniqueName)
	{
		// replace \ and / with underscores

		TCHAR tempLfi[256];
		TCHAR *fileName = tempLfi;
		int i;
		char chReplacementChar = '_';
		for (i=0; lfi->name[i]; ++i)
		{
			fileName[i] = lfi->name[i];

			if ( !__iscsym(fileName[i]) && (fileName[i] != '.') ) 	
				fileName[i] = chReplacementChar;
		
		}
		fileName[i] = (TCHAR) 0;

		//tempName.Format("%s%s_%s",tempPath,this->GetDocument()->GetUniqueBigFileName(),tempLfi);
		tempName = "c:\\";
		tempName += tempLfi;
	}
	else
	{
		tempName.Format("%s%s",tempPath,lfi->name);
	}

	CFile fout(tempName,CFile::modeCreate | CFile::modeWrite | CFile::typeBinary);
	// lfi->orgSize can be > 0 even if lfi->size = 0 !!!
	if ( (lfi->orgSize && uncompress) && (lfi->size != 0) )
	{
		// only unpack packed files that have been saved.
		/*
		* BufferedChunkDecompressor can only use an output buffer with aligned memory & size
		*/
		size_t newOrgSize = (static_cast<size_t>(lfi->orgSize) + 15) & (~15U);
		unpackBuf = static_cast<char*>(_aligned_malloc(newOrgSize, 16));

		// Create a buffered chunk decompressor in case we need to decode.
		
        if (REF_is(inBuf))
        {
            REF_decode(unpackBuf, inBuf, 0);
        }
		// Chunk ref
		else if (rw::core::codec::ChunkReader::IsChunkPacked(inBuf))
		{
			m_decompressor->Decompress(unpackBuf, newOrgSize, inBuf, static_cast<size_t>(size));
		}
        else
        {
            ASSERT(0);
        }
		fout.Write(unpackBuf,static_cast<UINT>(lfi->orgSize));
	}
	else
	{
		fout.Write(inBuf,static_cast<UINT>(size));
	}
	fout.Close();

	// add the file to the list of deletable files (these get tossed at the end)
	m_tempFiles.AddTail(tempName);

	// cleanup 
	_aligned_free(inBuf);
	_aligned_free(unpackBuf);; // if we didn't unpack this will be NULL so its still Ok.
}

HANDLE CWinBigView::ExtractSelectedFilesToOLE(TCHAR *hDropFileNames, int bufSize)
{
	CListCtrl& listCtrl = GetListCtrl();

	int item = -1;
	UINT nSelections = listCtrl.GetSelectedCount();

	// if nothing is selected don't bother.
	if (nSelections < 1)
	{
		return NULL;
	}

	// this will hold our list of HDROP formatted names (it has to end in a double NULL)
	TCHAR *fileNamePtr = hDropFileNames;
	CString tempName;
	CString realName;

	// figure out which files are selected
	for (UINT i = 0; i < nSelections; i++)
	{
		item = listCtrl.GetNextItem(item, LVNI_SELECTED);
		ASSERT(item != -1);
		LF_INFO *lfi = (LF_INFO *)listCtrl.GetItemData(item);

		ExtractBigFileFile(lfi, tempName, TRUE, TRUE);
		
		// add the file to our list of files (kinda crude way of doing it)
		UINT len = tempName.GetLength();
		memcpy(fileNamePtr,tempName,len);
		fileNamePtr[len] = 0;
		fileNamePtr += (len + 1);
		// YES its very primitive way of checking.
		ASSERT (((int)fileNamePtr - (int)hDropFileNames) < bufSize);  // make sure we don't overrun the buffer. :)
	}
	fileNamePtr[0] = 0;  // double NULL for the HDROP format
		
	// make an HDROP struct
	int size = sizeof(DROPFILES)+bufSize;
	HANDLE hData = ::GlobalAlloc(GHND,size);
	LPDROPFILES pDF = (LPDROPFILES)::GlobalLock(hData);
	pDF->pFiles = sizeof(DROPFILES);
	#ifdef UNICODE
		ASSERT(0);  // things go WAY bad in UNICODE
	#endif

	pDF->fWide = FALSE;
	LPBYTE pData = (LPBYTE) pDF + sizeof(DROPFILES);
	::CopyMemory(pData,hDropFileNames,bufSize);
	::GlobalUnlock(hData);

	return hData;
}

void CWinBigView::OnBegindrag(NMHDR*, LRESULT*) 
{
	// TODO: Add your control notification handler code here

	CListCtrl& listCtrl = GetListCtrl();
	UINT nSelections = listCtrl.GetSelectedCount();

	// if nothing is selected don't bother.
	if (nSelections < 1)
	{
		return;
	}

	// this will hold our list of HDROP formatted names (it has to end in a double NULL)
	UINT bufferLength = MAX_PATH*nSelections;
	TCHAR *fileNames = new TCHAR[bufferLength];

	HANDLE hData = ExtractSelectedFilesToOLE(fileNames,bufferLength);

	COleDataSource ods;
	ods.CacheGlobalData(CF_HDROP,hData);

	// cleanup
	// remove the files from the temp directory and free the filenames
	delete fileNames;
}

void CWinBigView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	CListView::OnLButtonDown(nFlags, point);
}

DROPEFFECT CWinBigView::OnDragOver(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point) 
{
	CListView::OnDragOver(pDataObject, dwKeyState, point);
	// TODO: Add your specialized code here and/or call the base class

	return DROPEFFECT_COPY;
}

DROPEFFECT CWinBigView::OnDragEnter(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point) 
{
	// TODO: Add your specialized code here and/or call the base class
	
	CListView::OnDragEnter(pDataObject, dwKeyState, point);
	return DROPEFFECT_COPY;

}

DROPEFFECT  CWinBigView::OnDropEx(COleDataObject* pDataObject, DROPEFFECT dropEffect, DROPEFFECT dropList, CPoint point) 
{
	// TODO: Add your specialized code here and/or call the base class
	
	return CListView::OnDropEx(pDataObject, dropEffect, dropList, point);
}

void CWinBigView::OnDropFiles(HDROP hDropInfo) 
{
	// TODO: Add your message handler code here and/or call default
	
	CListView::OnDropFiles(hDropInfo);
}

void CWinBigView::OnContextMenu(CWnd*, CPoint point) 
{
	// TODO: Add your message handler code here
	
	// bring up a menu if there is an item selected
	CListCtrl& listCtrl = GetListCtrl();
	int item = -1;
	int nSelections = listCtrl.GetSelectedCount();

	ScreenToClient(&point);
	CRect rect;

	// TODO make the context menu. Is this the right way of doing it?
	CMenu menu;
	menu.LoadMenu(IDR_CONTEXT_ONFILE);
	CMenu *pContext = menu.GetSubMenu(0);

	//
	// what items do we need to disable?
	//
	pContext->EnableMenuItem(ID_CONTEXT_ONFILE_RENAME,MF_GRAYED);
	pContext->EnableMenuItem(ID_CONTEXT_ONFILE_COPY,MF_ENABLED);  // always
	pContext->EnableMenuItem(ID_CONTEXT_ONFILE_VIEW,MF_ENABLED);  // always
	pContext->EnableMenuItem(ID_CONTEXT_ONFILE_COMPRESS,MF_GRAYED);
	pContext->EnableMenuItem(ID_CONTEXT_ONFILE_DELETE,MF_GRAYED);
	pContext->EnableMenuItem(ID_CONTEXT_ONFILE_UNDELETE,MF_GRAYED);

	if (nSelections == 1)
	{
		// can only rename if one is selected.
		pContext->EnableMenuItem(ID_CONTEXT_ONFILE_RENAME,MF_ENABLED);
	}

	int i;
	for (i = 0; i < nSelections; i++)
	{
		item = listCtrl.GetNextItem(item, LVNI_SELECTED);
		ASSERT(item != -1);
		LF_INFO *lfi = (LF_INFO *)listCtrl.GetItemData(item);
		
/*
		// file of 0 byte (lfi->size = 0) should not be able to be compressed !
		if  ( (lfi->orgSize == 0) && (lfi->size != 0) )
*/

		if (lfi->orgSize == 0)
		{
			// need at least one uncompressed file to uncompress
			pContext->EnableMenuItem(ID_CONTEXT_ONFILE_COMPRESS,MF_ENABLED);
		}
		if (lfi->action == LF_DEL)
		{
			// need on deleted to enable undeletion
			pContext->EnableMenuItem(ID_CONTEXT_ONFILE_UNDELETE,MF_ENABLED);
		}
		else
		{
			// need on deleted to enable undeletion
			pContext->EnableMenuItem(ID_CONTEXT_ONFILE_DELETE,MF_ENABLED);
		}
	}
	item = -1;
	// figure out if we are over a selected file?
	for (i = 0; i < nSelections; i++)
	{
		item = listCtrl.GetNextItem(item, LVNI_SELECTED);
		listCtrl.GetItemRect(item,&rect,LVIR_BOUNDS);
		if (rect.PtInRect(point))
		{
			// yup we've right clicked on an item bring up the context menu.
			ClientToScreen(&point);
			pContext->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON, point.x, point.y, this);
		}
	}
}

void CWinBigView::OnBeginlabeledit(NMHDR*, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here

	m_blnIsLabelEditing = true;
	m_oRenameEdit.SubclassWindow(GetListCtrl().GetEditControl()->GetSafeHwnd()); 

	// accept changes to be made on the label
	*pResult = 0;
}

void CWinBigView::OnEndlabeledit(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
	// TODO: Add your control notification handler code here

	if (((NMLVDISPINFO *)pNMHDR)->item.pszText)
	{
		LF_INFO *lfi = (LF_INFO *)pDispInfo->item.lParam;
		lfi->name = ((NMLVDISPINFO *)pNMHDR)->item.pszText;
		// need to get a new size of the item

		CListCtrl& listCtrl = GetListCtrl();
		int item = -1;

		item = listCtrl.GetNextItem(item, LVNI_SELECTED);
		ASSERT(item != -1);
		listCtrl.Update(item);
	}

	m_blnIsLabelEditing = false;
	m_oRenameEdit.UnsubclassWindow();

	//((NMLVDISPINFO *)pNMHDR)->item
	*pResult = 0;
	GetDocument()->SetModifiedFlag(TRUE);

/*
	// get our listCtrl
	CListCtrl& ListCtrl = GetListCtrl();

	ListCtrl.SetColumnWidth(0, LVSCW_AUTOSIZE);
	ListCtrl.Invalidate();
*/
}

void CWinBigView::OnContextOnfileCompress() 
{
    // This displays the dialog box about the queuing of an action.  Nothing is
    // done until we actually save the file to the disk.
	ShowOnActionWarning();
	
	// need to compress the selected files we will compress them and store them on disk
	// until the file is saved.
	// bring up a menu if there is an item selected
	CListCtrl& listCtrl = GetListCtrl();
	int item = -1;
	int nSelections = listCtrl.GetSelectedCount();

	// figure out which files are selected
	for (int i = 0; i < nSelections; i++)
	{
		item = listCtrl.GetNextItem(item, LVNI_SELECTED);
		ASSERT(item != -1);
		LF_INFO *lfi = (LF_INFO *)listCtrl.GetItemData(item);

		// is the file uncompressed?
/*
		// file of 0 byte is not compressed (lfi->size = 0) !
		if ((lfi->orgSize == 0) && (lfi->size != 0) )
*/

		if (lfi->orgSize == 0)
		{
			CString tempName;

			// Need to figure out the final sizes.  This could be slow but there isn't too many nice ways around it
			ExtractBigFileFile(lfi,tempName, FALSE, TRUE);

			CFile fin(tempName, CFile::modeRead | CFile::typeBinary);
			UINT length = static_cast<UINT>(fin.GetLength());
			char *contents = new char[length];
			fin.Read(contents, length);
			fin.Close();

			// compress and write out.
			// lfi->size is the the worst case.
            // Actually, it isn't.  The compressed file may actually be larger.  Therefore
            // we'll add a couple K extra to the packed buffer, "just to be sure".
            char *packed = new char[(int)(lfi->size*1.2f)+65536];
			lfi->orgSize = lfi->size;
			lfi->size = REF_encode(packed, contents, static_cast<size_t>(lfi->size));
			delete packed;
			delete contents;

			// generate a temp file with the compressed file in it.
			if (lfi->action == LF_ADD)
			{
				lfi->action = LF_ADDCOMPRESS_REF;
			}
			else
			{
				lfi->action = LF_COMPRESS_REF;
			}
		}
	}

    this->Invalidate();  // redraw the label
	GetDocument()->SetModifiedFlag(TRUE);
}

/*
// same as CWinBigView::OnContextOnfileCompress execpt that it uses BZIP2 compression instead of
// RefPack.
void CWinBigView::OnBzip2Compress()
{
    // This displays the dialog box about the queuing of an action.  Nothing is
    // done until we actually save the file to the disk.
	ShowOnActionWarning();
	
	// need to compress the selected files we will compress them and store them on disk
	// until the file is saved.
	// bring up a menu if there is an item selected
	CListCtrl& listCtrl = GetListCtrl();
	int item = -1;
	int nSelections = listCtrl.GetSelectedCount();

	// figure out which files are selected
	for (int i = 0; i < nSelections; i++)
	{
		item = listCtrl.GetNextItem(item, LVNI_SELECTED);
		ASSERT(item != -1);
		LF_INFO *lfi = (LF_INFO *)listCtrl.GetItemData(item);

		// is the file uncompressed?
		if (lfi->orgSize == 0)
		{
			CString tempName;

			// Need to figure out the final sizes.  This could be slow but there isn't too many nice ways around it
			ExtractBigFileFile(lfi,tempName, FALSE, TRUE);

			CFile fin(tempName, CFile::modeRead | CFile::typeBinary);
			UINT length = static_cast<UINT>(fin.GetLength());
			char *contents = new char[length];
			fin.Read(contents, length);
			fin.Close();

			// compress and write out.
			// lfi->size is the the worst case.
            // Actually, it isn't.  The compressed file may actually be larger.  Therefore
            // we'll add a couple K extra to the packed buffer, "just to be sure".
			char *packed = new char[lfi->size + 2048];
			lfi->orgSize = lfi->size;
			lfi->size = BZIP2_encode(packed,contents,lfi->size);
			delete packed;
			delete contents;

			// generate a temp file with the compressed file in it.
			if (lfi->action == LF_ADD)
			{
				lfi->action = LF_ADDCOMPRESS_BZ2;
			}
			else
			{
				lfi->action = LF_COMPRESS_BZ2;
			}
		}
	}

    this->Invalidate();  // redraw the label
	GetDocument()->SetModifiedFlag(TRUE);
}
*/

void CWinBigView::OnEditDelete() 
{
	ShowOnActionWarning();

	// mark all of the selected files for deletion.
	// NOTE: This overrides any of the other flags.
	CListCtrl& listCtrl = GetListCtrl();
	int item = -1;
	int nSelections = listCtrl.GetSelectedCount();

		// figure out which files are selected
	for (int i = 0; i < nSelections; i++)
	{
		item = listCtrl.GetNextItem(item, LVNI_SELECTED);
		ASSERT(item != -1);
		LF_INFO *lfi = (LF_INFO *)listCtrl.GetItemData(item);

		if (lfi->action != LF_NONE)
		{
			// there are other commands pending... we should do something?
		}
		lfi->action = LF_DEL;
	}
	this->Invalidate();  // redraw the label
	GetDocument()->SetModifiedFlag(TRUE);
}

void CWinBigView::OnContextOnfileUndelete() 
{
	// undo the deletion, this is just setting the status back to NONE.
	CListCtrl& listCtrl = GetListCtrl();
	int item = -1;
	int nSelections = listCtrl.GetSelectedCount();

	// figure out which files are selected
	for (int i = 0; i < nSelections; i++)
	{
		item = listCtrl.GetNextItem(item, LVNI_SELECTED);
		ASSERT(item != -1);
		LF_INFO *lfi = (LF_INFO *)listCtrl.GetItemData(item);

		if (lfi->action == LF_DEL)
		{
			// keep the old actions of files.  TODO compression doesn't really work.
			if (lfi->fileName.GetLength() > 0)
			{
				lfi->action = LF_ADD;
			}
			else
			{
				lfi->action = LF_NONE;
			}
		}
	}
	this->Invalidate();  // redraw the label
	GetDocument()->SetModifiedFlag(TRUE);
}



void CWinBigView::OnContextOnfileRename() 
{
	// TODO: Add your command handler code here
	CListCtrl& listCtrl = GetListCtrl();
	int item = -1;
	int nSelections = listCtrl.GetSelectedCount();

	if (nSelections == 1)
	{
		item = listCtrl.GetNextItem(item, LVNI_SELECTED);
		ASSERT(item != -1);
		// start in-place label editing ...
	}
}

void CWinBigView::ShowOnActionWarning()
{
	// should we bring up the dialog?
	if (!theApp.GetProfileInt("Settings","SkipDlgOnAction",0))
	{
		DlgOnAction dlg;
		if (dlg.DoModal() != IDOK)
		{
			return;
		}
	}
}

void CWinBigView::OnEditProperties() 
{
	// TODO: Add your command handler code here
	theApp.OnEditProperties();
	GetDocument()->SetModifiedFlag(TRUE);
}

void CWinBigView::OnEditSelectAll() 
{

	CListCtrl& listCtrl = GetListCtrl();
	for (int i=0; i < listCtrl.GetItemCount(); i++)
	{
	   listCtrl.SetItem(i, 0, LVIF_STATE, NULL, 0, LVIS_SELECTED, LVIS_SELECTED, 0);
	}

}

BOOL CWinBigView::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class

	if ((pMsg->message == WM_KEYDOWN) && (pMsg->wParam == VK_DELETE))
	{
		if (!m_blnIsLabelEditing)
		{
			CListCtrl& listCtrl = GetListCtrl();
			int item = -1;
			// get selected item
			item = listCtrl.GetNextItem(item, LVNI_SELECTED);
		
			if (item != -1)
			{
				// mark selected items for deletion
				OnEditDelete();
/*
				LF_INFO *lfi = (LF_INFO *)listCtrl.GetItemData(item);
				lfi->action = LF_DEL; // set to deletion mode
				this->Invalidate();  // redraw the label
				GetDocument()->SetModifiedFlag(TRUE);
*/
			}
			else{}
				// consume the "Delete" keypress
				//return true;
		}
	}

	// ........................................................................

	if ((pMsg->message == WM_KEYDOWN) && (pMsg->wParam == VK_RETURN))
	{
		// set flag to prevent the viewing of selected file when "Enter"
		// key is pressed during file renaming
		m_IsEnterKeyPressedWhileRenaming = true;	
		static int i = 0;
		TRACE("Enter key pressed during file renaming ! %d\n", ++i);
		CListCtrl* pListCtrl = &GetListCtrl();
		// send enter key event to list control during file renaming
		if (m_blnIsLabelEditing)
			return pListCtrl->PreTranslateInput(pMsg); 
	}

	// else proceed to handle the keypress
	return CListView::PreTranslateMessage(pMsg);
}

void CWinBigView::ClearHeaderSortImg()
{
	// get a pointer to the header control
	CHeaderCtrl* pHeaderCtrl = GetListCtrl().GetHeaderCtrl();
	// validate pointer obtained
	ASSERT_VALID(pHeaderCtrl);
	// get total columns in header
	int intTotalColumns = GetListCtrl().GetHeaderCtrl()->GetItemCount();
//	HDITEM headerItem;

	for (int i=0; i<intTotalColumns; i++)
	{
		// get existing structure of header's column
		pHeaderCtrl->GetItem(i, &headerItem);
		headerItem.fmt |= HDF_IMAGE;
		headerItem.mask |= HDI_IMAGE;

		// assign "cover-up" bitmap
		headerItem.iImage = 2;
	
		// set changes to header's column
		pHeaderCtrl->SetItem(i, &headerItem);
	}

	pHeaderCtrl->Invalidate(); 
}

void CWinBigView::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: Add your message handler code here and/or call default

	CListView::OnChar(nChar, nRepCnt, nFlags);
}
