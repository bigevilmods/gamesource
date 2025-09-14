// (c) Electronic Arts. All Rights Reserved.
// WinBigDoc.cpp : implementation of the CWinBigDoc class
//

#include "stdafx.h"
#include "WinBig.h"

#include "WinBigDoc.h"
#include "WinBigView.h"

#include "DlgPackedBigFile.h"
#include "mainfrm.h"

#include "../spit/codex.h"
#include "../spit/refcodex.h"

#include "rw/core/bigfile/fileformat.h"
#include "rw/core/bigfile/bigfile.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CWinBigApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CWinBigDoc

IMPLEMENT_DYNCREATE(CWinBigDoc, CDocument)

BEGIN_MESSAGE_MAP(CWinBigDoc, CDocument)
    //{{AFX_MSG_MAP(CWinBigDoc)
    // NOTE - the ClassWizard will add and remove mapping macros here.
    //    DO NOT EDIT what you see in these blocks of generated code!
    //}}AFX_MSG_MAP ON_COMMAND
    ON_COMMAND(ID_FILE_CLOSE, OnCloseActiveDoc)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWinBigDoc construction/destruction

CWinBigDoc::CWinBigDoc()
{
    // init defaults
    m_bigType = BIGUNKNOWN;
    m_bigSize = 0;
    m_bigHeader = NULL;
    m_fileHeaderSize = 0;
    m_directoryOffset = 0;
    m_headerSize = 0;
    m_Files = 0;
    m_dirEntrySize = 0;
    m_blnOnFirstPass = true;
}

CWinBigDoc::~CWinBigDoc()
{

    delete m_bigHeader;
    m_bigHeader = NULL;
    m_Files = 0;
}

BOOL CWinBigDoc::OnNewDocument()
{
    if (!CDocument::OnNewDocument())
        return FALSE;

    // TODO: add reinitialization code here
    // (SDI documents will reuse this document)
    m_bigType = BIGUNKNOWN;
    m_bigSize = 0;
    if (m_bigHeader != NULL)
    {
        delete m_bigHeader;
        m_bigHeader = NULL;
    }
    m_fileHeaderSize = 0;
    m_directoryOffset = 0;
    m_headerSize = 0;
    m_Files = 0;
    m_dirEntrySize = 0;
    m_blnOnFirstPass = true;

    // Cleanup the list (free any memory that needs freeing
    POSITION pos = GetFirstViewPosition();
    CWinBigView* pFirstView = (CWinBigView* )GetNextView( pos );
    pFirstView->CleanupList();

    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CWinBigDoc serialization
unsigned __int64 GetIntBigEndian(char *buf, int numBytes=4)
{
    unsigned __int64 result=0;

    for (int i=0; i<numBytes; i++)
    {
        result<<=8;
        result|=((unsigned char*)buf)[i];
    }

    return result;
}

char *GetFileAndExtension(char *inStr, char **ext)
{
    // NOTE will assert if string is longer than 256 chars
    static char string[256];
    int index = 0;

    do
    {
        string[index] = inStr[index];
        index++;
        // buffer is too big
        ASSERT(index < 256);

        if (inStr[index] == '.')
        {
            *ext = &string[index];
        }
    }
    while (inStr[index]);
    string[index] = 0;

    return &string[0];
}

void CWinBigDoc::Serialize(CArchive& ar)
{
    // set flag to enable deletion of items in CListCtrl when Close command
    // is invoked
    m_blnOnFirstPass = true;

    CProgressCtrl loadbar;
    CRect r;
    CMainFrame *f = (CMainFrame *)::AfxGetMainWnd();
    // if you double click on a file you won't have this as there is no window yet.
    if (f)
    {
        f->GetClientRect(r);
        loadbar.Create(WS_CHILD|WS_VISIBLE|PBS_SMOOTH, r, &f->m_wndStatusBar, 1);
        loadbar.SetRange(0,(short)r.right);
    }

    if (ar.IsStoring())
    {
        UINT totalLen=16;  // at least 16 bytes long for the main header
        UINT headerLen=16;
        UINT nFiles=0;
        int i;
        CString tempName;

        POSITION pos = GetFirstViewPosition();
        CWinBigView* pFirstView = (CWinBigView* )GetNextView( pos );
        CListCtrl& ListCtrl = pFirstView->GetListCtrl();

        // figure out the step increments (need to do this cause the StepIt
        // does it in a minimum of 1) and with a lot of files the division
        // will make it 0 cause its an int.
        double increment = (double)r.right/(double)(ListCtrl.GetItemCount()*3);
        double loadbar_position = 0.0f;

        // PASS 1
        // Get rid of all the files scheduled for deletion.
        i = 0;
        while (i < ListCtrl.GetItemCount())
        {
            LF_INFO *lfi = (LF_INFO *)ListCtrl.GetItemData(i);

            if (lfi->action == LF_DEL)
            {
                ListCtrl.DeleteItem(i);
                delete lfi;
                i = 0; // reset the looking as we just made the list smaller (and possibly re-arranged it).
            }
            else
            {
                i++;
            }
        }

        // PASS 2
        // Get all of the files (that are part of the lumpy file) as temp files
        // so we can add them that way (otherwise we would need to store the
        // whole file in memory)
        for (i = 0; i < ListCtrl.GetItemCount(); i++)
        {
            LF_INFO *lfi = (LF_INFO *)ListCtrl.GetItemData(i);

            if (lfi->action == LF_NONE || lfi->action == LF_COMPRESS_REF)
            {
                pFirstView->ExtractBigFileFile(lfi,tempName, FALSE, TRUE);
                lfi->fileName = tempName;
            }
            loadbar.SetPos((int)loadbar_position);
            loadbar_position += increment;
        }

        // PASS 3
        // Get the header details.
        for (i = 0; i < ListCtrl.GetItemCount(); i++)
        {
            LF_INFO *lfi = (LF_INFO *)ListCtrl.GetItemData(i);

            ASSERT(lfi->action != LF_DEL);
            // we want to keep the file
            nFiles++;

            // how much are we going to add to the header?
            int nameLen = lfi->name.GetLength();
            int fileHead = (4 + 4 + nameLen + 1); // offset, length, name, Null ender
            headerLen += fileHead;

            // how much will we add to the total file
            totalLen += (fileHead += GetAlignedSize(static_cast<int>(lfi->size)));
        }
        // get a new filecount.
        m_Files = nFiles;

        // PASS 4
        // Calculate offsets
        int curOffset = GetAlignedSize(headerLen);  // first file can't start before the main header.

        for (i = 0; i < ListCtrl.GetItemCount(); i++)
        {
            LF_INFO *lfi = (LF_INFO *)ListCtrl.GetItemData(i);

            lfi->offset = curOffset;
            curOffset += this->GetAlignedSize(static_cast<int>(lfi->size));
        }

        // PASS 5
        // Write the header.
#if 1
        rw::core::BigFileFormatHeader myHeader;

        myHeader.signature = BIGFILE_HEADER_SIGNATURE;
        myHeader.headerVersion = BIGFILE_HEADER_VERSION;
        myHeader.fileCount = m_Files;
        myHeader.flags = rw::core::BIGFILE_FORMAT_FLAG_SORTORDER_HASH;
        myHeader.alignment = 16;
        myHeader._reserved0 = 0;
        myHeader.baseHeaderSize = sizeof(myHeader) + m_Files * 16;
        myHeader.nameHeaderSize = 0;
        myHeader.nameLength = 0;
        myHeader.pathLength = 0;
        myHeader.numPaths = 0;
        myHeader.fileSize = 0;

#else
        UINT BIGF = 0x42494746; // special BIG ID.
        UINT motTotalLen, motFiles, motHeaderLen;
        gputm (&BIGF,BIGF,sizeof(int)); // convert to motorola
        gputm (&motTotalLen,totalLen,sizeof(int));
        gputm (&motFiles,nFiles,sizeof(int));
        gputm (&motHeaderLen,headerLen,sizeof(int));
        ar << BIGF << motTotalLen << motFiles << motHeaderLen;

        for (i = 0; i < ListCtrl.GetItemCount(); i++)
        {
            LF_INFO *lfi = (LF_INFO *)ListCtrl.GetItemData(i);

            UINT motorOffset, motorLen;
            gputm(&motorOffset,static_cast<unsigned int>(lfi->offset),sizeof(int));
            gputm(&motorLen,static_cast<unsigned int>(lfi->size),sizeof(int));
            ar << motorOffset << motorLen;
            ar.WriteString(lfi->name);
            ar << (char)0x0;  // needs to end in a NULL

            loadbar.SetPos((int)loadbar_position);
            loadbar_position += increment;
        }
#endif

        // write out the pad for the header
        for (i = 0; i < (int)(theApp.m_fileAlignment - (headerLen % theApp.m_fileAlignment)) ; i++)
        {
            ar << unsigned char(0xEA);
        }

        // PASS 6
        // write out the files...
        for (i = 0; i < ListCtrl.GetItemCount(); i++)
        {
            LF_INFO *lfi = (LF_INFO *)ListCtrl.GetItemData(i);

            ASSERT(lfi->fileName.GetLength() > 0);  // needs to be already on disk

            // its a file somewhere on disk.
            CFile fin(lfi->fileName, CFile::modeRead | CFile::typeBinary);
            UINT length = static_cast<UINT>(fin.GetLength());
            char *contents = new char[length];
            fin.Read(contents,length);
            fin.Close();

            if (lfi->action == LF_COMPRESS_REF || lfi->action == LF_ADDCOMPRESS_REF)
            {
                ASSERT(lfi->orgSize > 0); // Can't compress an already compressed file.

                // compress and write out.
                char *packed = new char[static_cast<size_t>(lfi->size)]; 
                REF_encode(packed, contents, static_cast<size_t>(lfi->orgSize), 0);
                ar.Write(packed,static_cast<size_t>(lfi->size));
                delete packed;
            }
            else
            {
                ar.Write(contents,length);
            }
            delete contents;

            // write out the pad for the alignment
            for (int j = 0; j < (int)(theApp.m_fileAlignment - (lfi->size % theApp.m_fileAlignment)); j++)			
            {
                ar << unsigned char(0xEA);
            }

            // we're done the appropriate action.
            lfi->action = LF_NONE;

            // things are now inside the big file
            lfi->fileName = "";
            loadbar.SetPos((int)loadbar_position);
            loadbar_position += increment;
        }
        // remeber the name for later.
        m_bigFile = ar.GetFile()->GetFilePath();

        // update the view?
        pFirstView->Invalidate();
    }
    else        // we are loading an archive
    {
        CFile *fin = NULL;
        m_bigHeader = NULL;

        if ( ParseHeader(ar, &fin, &m_bigHeader) == false )
        {
            // failed
            return;
        }

        // rewind the file
        fin->SeekToBegin();

        // clear the list control for the new file
        POSITION pos = GetFirstViewPosition();
        CWinBigView* pFirstView = (CWinBigView* )GetNextView( pos );
        pFirstView->CleanupList();

        if (m_bigType == BIG_TYPE_FAST)
        {
            rw::core::Big myBig(m_bigHeader);
            myBig.SetNameIndex(m_bigHeader +
                rw::core::Big::GetHeaderSize(m_bigHeader));

            for (int i = 0; i < myBig.GetCount(); ++i)
            {
                char name[256];
                char *ext;
                uint64_t compressedSize, offset, originalSize;

                myBig.FindFileByIndex(i, &offset, &originalSize, &compressedSize);
                myBig.FindNameByIndex(i, name, sizeof(name));

                // WinBig expects the original size to be zero to display the GUI
                // data correctly for uncompressed files which is different than 
                // the bigfile format where uncompressed data has a compressed size
                // of zero.
                if (compressedSize == 0 && originalSize != 0)
                {
                    compressedSize = originalSize;
                    originalSize = 0;
                }

                GetFileAndExtension(name, &ext);

                // add the item to our view
                pFirstView->AddItem(i, _T(name), _T(ext), compressedSize, offset, originalSize, LF_NONE);
            }

            // If we really need to know this, we can get it from the header
            // without having to guess.
            theApp.m_fileAlignment = 0;
        }
        else
        {
            char *curHeader = m_bigHeader + m_directoryOffset;

            //-------------------------------------------
            // go through the header and make up our list.

            // figure out the step increments (need to do this cause the StepIt does it in a minimum of 1) and 
            // with a lot of files the division will make it 0 cause its an int.
            double increment = (double)r.right/(double)m_Files;
            double loadbar_position = 0.0f;

            char entryBuf[256];

            for (int i=0; i < m_Files; i++)
            {
                // determine entry size and offset

                unsigned __int64 offset = 0;
                unsigned __int64 length = 0;

                switch ( m_bigType )
                {
                case BIGID2:    // new 16 bit format
                    {
                        BIGENTRY16 *pEntry = (BIGENTRY16 *)curHeader;
                        offset = GetIntBigEndian( pEntry->offset, 2 );
                        length = GetIntBigEndian( pEntry->length, 2 );
                    }
                    break;

                case BIG24ID:   // old 24 bit format
                case BIGID3:    // new 24 bit format
                    {
                        BIGENTRY24 *pEntry = (BIGENTRY24 *)curHeader;
                        offset = GetIntBigEndian( pEntry->offset, 3 );
                        length = GetIntBigEndian( pEntry->length, 3 );
                    }
                    break;

                case BIGID4:    // new 32 bit format
                case BIG32ID:   // old 32 bit format
                    {
                        BIGENTRY32 *pEntry = (BIGENTRY32 *)curHeader;
                        offset = GetIntBigEndian( pEntry->offset, 4 );
                        length = GetIntBigEndian( pEntry->length, 4 );
                    }
                    break;

                case BIGID5:    // new 40 bit format
                    {
                        BIGENTRY40 *pEntry = (BIGENTRY40 *)curHeader;
                        offset = GetIntBigEndian( pEntry->offset, 5 );
                        length = GetIntBigEndian( pEntry->length, 4 );
                    }
                    break;

                default:        // unsupported format
                    AfxMessageBox("Format unsupported.", MB_OK);
                    return;
                }

                // get entry name
                char *ext = NULL;
                char *name = GetFileAndExtension(curHeader+m_dirEntrySize, &ext);

                // update the curHeader
                curHeader += m_dirEntrySize + strlen(name) + 1;  // +1 for the null;

                int orgSize =0;
                if (length > 0)
                {
                    // is the file compressed?  This information is not stored in the index;
                    // you have to actually read some data (16 bytes is enough) from the file
                    // itself.
                    fin->Seek(offset, CFile::begin);
                    fin->Read(entryBuf, 16);
                    if (REF_is(entryBuf))
                    {
                        orgSize = REF_size(entryBuf);
                    }
                }

                // add the item to our view
                pFirstView->AddItem(i,_T(name),_T(ext), length, offset, orgSize, LF_NONE);

                // if there is no f, it means you double clicked a file to open it.
                if (f)
                {
                    loadbar.SetPos((int)loadbar_position);
                    loadbar_position += increment;
                }
            }

            // figure out a default alignment... may not be correct but it will be the minimum alignment that is need for this file.
            CListCtrl& ListCtrl = pFirstView->GetListCtrl();
            int guessAlign = 1024;
            int okForAll;

            do 
            {
                okForAll = TRUE;
                for (int i = 0; i < ListCtrl.GetItemCount(); i++)
                {
                    LF_INFO *lfi = (LF_INFO *)ListCtrl.GetItemData(i);
                    if (lfi->offset % guessAlign != 0)
                    {
                        // if we're not alligned try the next alignment
                        guessAlign >>= 1;
                        okForAll = FALSE;
                        i = ListCtrl.GetItemCount();
                    }

                }
            } while (!okForAll);

            theApp.m_fileAlignment = guessAlign;
        }

        pFirstView->GetListCtrl().SetColumnWidth(0, LVSCW_AUTOSIZE);

    }
}

bool CWinBigDoc::ParseHeader(CArchive &ar, CFile **ppFin, char **ppHeaderBuf)
{
    ASSERT(ppFin && ppHeaderBuf);

    //---------------------------
    // init defaults, default to 32 bit
    //---------------------------
    m_bigType = BIGUNKNOWN;
    m_bigSize = 0;
    m_bigHeader = NULL;
    m_fileHeaderSize = 0;
    m_directoryOffset = 0;
    m_headerSize = 0;
    m_Files = 0;
    m_dirEntrySize = 0;

    CFile *file = ar.GetFile();
    m_bigFile = ar.GetFile()->GetFilePath();
    m_bigSize = file->GetLength();

    //---------------------------
    // read in some data to check header info. 
    // The first 4 bytes to determine file type.
    //---------------------------

    const unsigned int headerBufSize = sizeof (rw::core::BigFileFormatHeader);
    char headerBuf[ headerBufSize ];

    if ( file->Read(headerBuf, headerBufSize) != headerBufSize )
    {
        // Failed reading the file header. Something's wrong with the file. Bail out.
        AfxMessageBox("Failed reading file header.",MB_OK);
        return false;
    }

    // JDF - if the whole file is compressed, it's not really a big
    // file any more.  It's just a regular compressed file.
    ASSERT( REF_is( headerBuf ) == FALSE );
    ASSERT( theApp.m_packFile == FALSE );

    //---------------------------
    // packed file ?
    //---------------------------

    UINT bytesRead = 0;

    //---------------------------
    // get file type
    //---------------------------
	rw::core::Big::BigfileType bigfileType = rw::core::Big::GetHeaderType(headerBuf);
    switch(bigfileType)
    {
    case rw::core::Big::BFTYPE_FAST:
        m_bigType = BIG_TYPE_FAST;
        break;

    case rw::core::Big::BFTYPE_24:
        m_bigType = BIG24ID;
        break;

    case rw::core::Big::BFTYPE_N:
        m_bigType = (BIG_FILE_TYPE)GetIntBigEndian(headerBuf, 4);
        break;

    default:
        AfxMessageBox("Unknown bigfile format.",MB_OK);
        return false;
    }

    //---------------------------
    // other types, 4 byte, big endian
    //---------------------------

    bool warning = false;
    char warningMsg[1024];
    BIGHEADER *pBigHeader = NULL;

    // assuming the new header format
    if (m_bigType == BIG_TYPE_FAST)
    {
        m_headerSize = rw::core::Big::GetHeaderSize(headerBuf) + 
            rw::core::Big::GetNameIndexSize(headerBuf);

        if (rw::core::Big::HasFAT(headerBuf))
        {
            m_headerSize += rw::core::Big::GetFATSize(headerBuf);
        }
    }
    else
    {
        m_fileHeaderSize = sizeof(BIGHEADER);
        m_directoryOffset = m_fileHeaderSize;
        pBigHeader = (BIGHEADER *)headerBuf;
        m_Files = static_cast<int>(GetIntBigEndian(pBigHeader->num));
        m_headerSize = static_cast<int>(GetIntBigEndian(pBigHeader->hlen));
    }

    switch ( m_bigType )
    {
    case BIG_TYPE_FAST:
        m_dirEntrySize = 16;
        break;

    case BIG24ID:   // old 24 bit format
        {
            m_fileHeaderSize = sizeof(BIGHEADER24);
            m_directoryOffset = sizeof(BIGHEADER24);
            m_dirEntrySize = sizeof(BIGENTRY24);

            BIGHEADER24 *pH24 = (BIGHEADER24 *)headerBuf;
            m_Files = static_cast<int>(GetIntBigEndian(pH24->num, 2));
            m_headerSize = static_cast<int>(GetIntBigEndian(pH24->hlen, 2) + sizeof(BIGHEADER24));
        }
        break;

    case BIGID1:    // new 8 bit format
        {
            sprintf_s(warningMsg, "Unsupported format: new 8 bit.\n");
            warning = true;
        }
        break;

    case BIGID2:    // new 16 bit format
        {
            m_dirEntrySize = sizeof(BIGENTRY16);
        }
        break;

    case BIGID3:    // new 24 bit format
        {
            m_dirEntrySize = sizeof(BIGENTRY24);
        }
        break;

    case BIGID4:    // new 32 bit format
    case BIG32ID:   // old 32 bit format
        {
            m_dirEntrySize = sizeof(BIGENTRY32);
        }
        break;

    case BIGID5:    // new 40 bit format
        {
            m_dirEntrySize = sizeof(BIGENTRY40);
        }
        break;

    case BIGID6:    // new 48 bit format
        {
            sprintf_s(warningMsg, "Unsupported format: new 48 bit.\n");
            warning = true;
        }
        break;

    case BIGID7:    // new 56 bit format
        {
            sprintf_s(warningMsg, "Unsupported format: new 56 bit.\n");
            warning = true;
        }
        break;

    case BIGID8:    // new 64 bit format
        {
            sprintf_s(warningMsg, "Unsupported format: new 64 bit\n");
            warning = true;
        }
        break;

    default:        // unknown type
        {
            sprintf_s(warningMsg, "Unknown big file format\n");
            warning = true;
        }
        break;
    }

    ASSERT(ppFin && ppHeaderBuf);
    if ( m_dirEntrySize > 0 )   // valid m_dirEntrySize indicating valid format
    {

        *ppHeaderBuf = new char[m_headerSize];
        file->SeekToBegin();
        bytesRead = file->Read(*ppHeaderBuf, m_headerSize);
        ASSERT( bytesRead == m_headerSize );

        *ppFin = file;

        return true;
    }
    else
    {
        if ( warning )
        {
            // warn user of unsupported formats
            AfxMessageBox(warningMsg, MB_OK);
        }

        return false;
    }
}

BOOL CWinBigDoc::AddFile(TCHAR *fullPath,TCHAR *name, TCHAR *ext,BOOL overwrite)
{
    // cannot add duplicate files
    if (this->IsDuplicateName(name))
    {
        if (overwrite)
        {
            this->DestoryDuplicateName(name);
        }
        else
        {
            return FALSE;
        }
    }

    // add them to the list?
    POSITION pos = GetFirstViewPosition();
    CWinBigView* pFirstView = (CWinBigView *)GetNextView( pos );

    pFirstView->ShowOnActionWarning();

    CFile fin(fullPath, CFile::modeRead | CFile::typeBinary);
    UINT size = static_cast<UINT>(fin.GetLength());
    // see if its a packed file?
    char buf[256];
    fin.Read(buf,256);
    int orgSize = 0;

    if (REF_is(buf))
        orgSize = REF_size(buf);
    fin.Close();

    LF_INFO *lfi = pFirstView->AddItem(m_Files,name,ext,size,0,orgSize,LF_ADD);
    if (lfi)
    {
        lfi->fileName = fullPath;
    }
    m_Files++; // we now have one more file...

    // redraw the view (to add the added flag.
    pFirstView->Invalidate();

    SetModifiedFlag(TRUE);

    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CWinBigDoc diagnostics

#ifdef _DEBUG
void CWinBigDoc::AssertValid() const
{
    CDocument::AssertValid();
}

void CWinBigDoc::Dump(CDumpContext& dc) const
{
    CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CWinBigDoc commands

BOOL CWinBigDoc::IsInList(CString newName,BOOL destroy)
{
    POSITION pos = GetFirstViewPosition();
    CWinBigView* pFirstView = (CWinBigView* )GetNextView( pos );
    CListCtrl& ListCtrl = pFirstView->GetListCtrl();

    for (int i = 0; i < ListCtrl.GetItemCount(); i++)
    {
        LF_INFO *lfi = (LF_INFO *)ListCtrl.GetItemData(i);

        if (lfi->name.CompareNoCase(newName) == 0)
        {
            // they are the same
            if (destroy)
            {
                ListCtrl.DeleteItem(i);
                delete lfi;
            }
            return TRUE;
        }
    }
    return FALSE;
}

BOOL CWinBigDoc::IsDuplicateName(CString newName)
{
    return IsInList(newName,FALSE);
}

void CWinBigDoc::DestoryDuplicateName(CString newName)
{
    IsInList(newName,TRUE);
}


// returns the size that we need for alignment
int CWinBigDoc::GetAlignedSize(int size)
{
    int alignSize = ((size / theApp.m_fileAlignment) + 1) * theApp.m_fileAlignment;
    return alignSize;
}

void CWinBigDoc::OnCloseDocument() 
{
    // TODO: Add your specialized code here and/or call the base class

    CDocument::OnCloseDocument();
}

void CWinBigDoc::OnCloseActiveDoc()
{
    BOOL blnCloseDoc = true;

    if (this->IsModified())
    {
        blnCloseDoc = CDocument::SaveModified(); 
    }

    // items in CListCtrl can only be deleted once
    if ( m_blnOnFirstPass && blnCloseDoc )
    {
        // delete user defined structures and all items in CListCtrl
        OnNewDocument();

        // set flag to disable deletion of items in CListCtrl till a new document
        // has been loaded
        m_blnOnFirstPass = false;

        // reset the number of files in the document
        m_Files = 0;

        POSITION pos = GetFirstViewPosition();
        // get a pointer to the view class (CListView)
        CWinBigView* pFirstView = (CWinBigView* )GetNextView( pos );
        // clear image in header control
        pFirstView->ClearHeaderSortImg(); 

        TRACE("Document close command selected from main menu.\n");

    }
}

static char unique_path[MAX_PATH];
CString CWinBigDoc::GetUniqueBigFileName()
{
    char drive[MAX_PATH], unique_path[MAX_PATH], fname[MAX_PATH], ext[MAX_PATH];

    // make a unique file name
    _splitpath_s(this->m_bigFile,drive,unique_path,fname,ext);

    CString strTemp(unique_path);
    char chReplacementChar = '_';
    for (int i=0; i<strTemp.GetLength(); i++)
        if ( !__iscsym(strTemp.GetAt(i)) )
            strTemp.SetAt(i, chReplacementChar);

    strTemp += fname;	
    return strTemp;
}

BOOL CWinBigDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
    return CDocument::OnOpenDocument(lpszPathName);
}
