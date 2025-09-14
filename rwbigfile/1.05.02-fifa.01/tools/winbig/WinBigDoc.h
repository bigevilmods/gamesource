// (c) Electronic Arts. All Rights Reserved.
// WinBigDoc.h : interface of the CWinBigDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_WINBIGDOC_H__5B9E3EA7_F8B5_4EE1_81CF_C1DBD06B132D__INCLUDED_)
#define AFX_WINBIGDOC_H__5B9E3EA7_F8B5_4EE1_81CF_C1DBD06B132D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000



#ifndef MAKEID
#define MAKEID(a,b,c,d) (((int)(a)<<24)|((int)(b)<<16)|((int)(c)<<8)|(int)(d))
#endif


/*
* Copied from tool-bigfile.cpp
* Simple malloc allocator
*/

class MallocAllocator : public EA::Allocator::ICoreAllocator
{
    virtual void *Alloc(size_t size, const char *, unsigned int, unsigned int alignment, unsigned int)
    {
        return _aligned_malloc(size, alignment);
    }

    virtual void *Alloc(size_t size, const char *, unsigned int)
    {
        return Alloc(size, NULL, 0, 16, 0);
    }

    virtual void Free(void *ptr, size_t)
    {
        _aligned_free(ptr);
    }
};

enum BIG_FILE_TYPE
{
    BIGUNKNOWN = 0,
    BIG_TYPE_FAST = 1,                      // hashed type, fast
    BIG24ID = 0xC0FB,                       // old 24 bit
    BIG32ID = MAKEID('B','I','G','F'),      // old 32 bit
    BIGNID  = MAKEID('B','I','G','0'),      // generic, where N = # of bytes
    BIGID1  = MAKEID('B','I','G','1'),      // 8 bit
    BIGID2  = MAKEID('B','I','G','2'),      // 16 bit
    BIGID3  = MAKEID('B','I','G','3'),      // 24 bit
    BIGID4  = MAKEID('B','I','G','4'),      // 32 bit
    BIGID5  = MAKEID('B','I','G','5'),      // 40 bit
    BIGID6  = MAKEID('B','I','G','6'),      // 48 bit
    BIGID7  = MAKEID('B','I','G','7'),      // 56 bit
    BIGID8  = MAKEID('B','I','G','8')       // 64 bit
};

struct BIGHEADER            // new header for all formats
{
    char    type[4];        /* 'BIG4'/'BIGF' for big file. */
    char    len[4];         /* Length of file in bytes. */
    char    num[4];         /* Number of files in big file. */
    char    hlen[4];        /* Header length (including this structure). */
};

struct BIGHEADER24          // old 24 bit header
{
    char    type[2];        /* 0xc0fb for 24-bit big file. */
    char    hlen[2];        /* Header length (excluding this structure). */
    char    num[2];         /* Number of files in big file. */
};

struct BIGENTRY16
{
    char    offset[2];      /* File offset relative to start of big file. */
    char    length[2];      /* Length of file. */
//    char    name[1];        /* Null-terminated name of file (variable length). */
};

struct BIGENTRY24 
{
    char    offset[3];      /* File offset relative to start of big file. */
    char    length[3];      /* Length of file. */
//    char    name[1];        /* Null-terminated name of file (variable length). */
};

struct BIGENTRY32
{
    char    offset[4];      /* File offset relative to start of big file. */
    char    length[4];      /* Length of file. */
//    char    name[1];        /* Null-terminated name of file (variable length). */
};

struct BIGENTRY40
{
    char    offset[5];      /* File offset relative to start of big file. */
    char    length[4];      /* Length of file. */
//    char    name[1];        /* Null-terminated name of file (variable length). */
};

class CWinBigDoc : public CDocument
{
public:
	CString		m_bigFile;	// pointer to the big file

private:
	bool            ParseHeader( CArchive &ar, CFile **pFile, char **pHeader );
    BIG_FILE_TYPE   m_bigType;
	unsigned __int64 m_bigSize;		    // size of the big file
	char		    *m_bigHeader;	    // pointer to the header
    UINT            m_fileHeaderSize;   // size the first header block
    UINT            m_directoryOffset;  // size of file directory (or toc = table of conntents)
	UINT		    m_headerSize;	    // = m_fileHeaderSize + m_directorySize
	int			    m_Files;		    // number of files in the big file. (including deleted files)
    int             m_dirEntrySize;
	BOOL            m_blnOnFirstPass;
	CString			m_strBuffer;
	
protected: // create from serialization only
	CWinBigDoc();
	DECLARE_DYNCREATE(CWinBigDoc)

// Attributes
public:

// Operations
public:
	int GetNumFiles() { return m_Files;};
	BOOL AddFile(TCHAR *fullPath,TCHAR *name, TCHAR *ext, BOOL destroy);
	BOOL IsInList(CString newName,BOOL destroy);
	BOOL IsDuplicateName(CString newName);
	void DestoryDuplicateName(CString newName);

	int GetAlignedSize(int size);
	CString GetUniqueBigFileName();
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWinBigDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	virtual void OnCloseDocument();
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CWinBigDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CWinBigDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	void OnCloseActiveDoc();
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WINBIGDOC_H__5B9E3EA7_F8B5_4EE1_81CF_C1DBD06B132D__INCLUDED_)
