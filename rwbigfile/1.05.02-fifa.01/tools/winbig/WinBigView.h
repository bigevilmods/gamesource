// (c) Electronic Arts. All Rights Reserved.
// WinBigView.h : interface of the CWinBigView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_WINBIGVIEW_H__D9673B67_3562_443B_99B8_545241B45420__INCLUDED_)
#define AFX_WINBIGVIEW_H__D9673B67_3562_443B_99B8_545241B45420__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "RenameEdit.h"

enum LF_ACTION
{
	LF_NONE,
	LF_ADD,
	LF_ADDCOMPRESS_REF,
	LF_COMPRESS_REF,
	LF_DEL,
};

typedef struct tagLF_INFO
{
	CString name;
	CString type;
	unsigned __int64 size;
	unsigned __int64 offset;
	unsigned __int64 orgSize;	// if its > 0 then the file is packed (this is its original size).
	LF_ACTION action;   // what action to take when saving the file.
	CString	fileName;	// path to the file on disk (if we added/compressed a file)
} LF_INFO;

class CWinBigView : public CListView
{
public:
	void CleanupList();

	LF_INFO *AddItem(int index, CString name, CString ext, unsigned __int64 size, unsigned __int64 offset, unsigned __int64 orgSize, LF_ACTION action);

	// takes a lfi node and returns the tempName that was created (real name is the name as it appears in WinBig).
	void ExtractBigFileFile(LF_INFO *lfi, CString &tempName, BOOL uncompress, BOOL uniqueName);

	afx_msg void ViewSelectedBigFiles();

	HANDLE ExtractSelectedFilesToOLE(TCHAR *hDropFileNames, int bufSize);

	// clear image in header control
	void ClearHeaderSortImg();

	static char* FormatWithCommas(char *buf, __int64 num);

	// list of temp files to delete.
	CStringList m_tempFiles; 

protected:
	CFont	m_fontPrinter;
	UINT	m_linesPerPage;
	UINT	m_lineHeight;
	UINT	m_charWidth;


protected: // create from serialization only
	CWinBigView();
	DECLARE_DYNCREATE(CWinBigView)

// Attributes
private:
	BOOL m_IsEnterKeyPressedWhileRenaming;
	BOOL m_blnIsLabelEditing;
	CImageList m_oSortDirections;
	CRenameEdit m_oRenameEdit;
	CEdit* m_pEditBox;
	HDITEM headerItem;
	MallocAllocator m_mallocAllocator;
	EA::Jobs::JobScheduler* m_scheduler;
	rw::core::codec::BufferedChunkDecompressor* m_decompressor;


public:
	CWinBigDoc* GetDocument();

// Operations
public:
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWinBigView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual DROPEFFECT  OnDropEx(COleDataObject* pDataObject, DROPEFFECT dropEffect, DROPEFFECT dropList, CPoint point);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL DestroyWindow();
	virtual DROPEFFECT OnDragOver(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	virtual DROPEFFECT OnDragEnter(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void OnInitialUpdate(); // called first time after construct
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnPrint(CDC* pDC, CPrintInfo* pInfo);
	//}}AFX_VIRTUAL

// Implementation
private:
	static int CALLBACK CompareFunc(LPARAM lp1, LPARAM lp2, LPARAM lps);
public:
	void ShowOnActionWarning();
	virtual ~CWinBigView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif


protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CWinBigView)
	//afx_msg void OnHandleKeyDown(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnGetdispinfo(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnColumnclick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDestroy();
	afx_msg void OnItemdblclick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblclk(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnEditCopy();
	afx_msg void OnBegindrag(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnDropFiles(HDROP hDropInfo);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnBeginlabeledit(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEndlabeledit(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnContextOnfileCompress();
	afx_msg void OnContextOnfileUndelete();
	afx_msg void OnEditDelete();
	afx_msg void OnContextOnfileRename();
	afx_msg void OnEditProperties();
	afx_msg void OnEditSelectAll();
	afx_msg void OnDblClk();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
};

#ifndef _DEBUG  // debug version in WinBigView.cpp
inline CWinBigDoc* CWinBigView::GetDocument()
   { return (CWinBigDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WINBIGVIEW_H__D9673B67_3562_443B_99B8_545241B45420__INCLUDED_)
