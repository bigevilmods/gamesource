// (c) Electronic Arts. All Rights Reserved.

#include "EAAssert/eaassert.h"

#include "private_rwhandle.h"
#include "private_rwdevice.h"
#include "rw/core/filesys/device.h"
#include "rw/core/filesys/driverimpl.h"
#include "rw/core/filesys/directory.h"
#include "rw/core/filesys/path.h"

#include <string.h>
#include <ctype.h>
#include <stdio.h>

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

namespace rw {
namespace core {
namespace filesys {

//////////////////////////////////////////////////////////////////////////////
// Driver helper functions                                                  //
//////////////////////////////////////////////////////////////////////////////

#define NUMBEROFMSG 32
static OSMessageQueue  ReadFileThreadMsgQ;
static OSMessage       ReadFileThreadMsgData[NUMBEROFMSG];

#define AlignDownP2(MemBase,Align) ((MemBase)&(~((Align)-1)))
#define AlignUpP2(MemBase,Align) (((MemBase)+(Align)-1)&(~((Align)-1)))

#define ModP2(MemBase,Align) ((MemBase)&((Align)-1))

// defines must be 64 or more and power of 2
#define BUFFERSIZE 64
#define BUFFERSIZEDIV2 (BUFFERSIZE/2)
#define MYBIGBUFFERSIZE (1024*32)
#define MYBIGBUFFERSIZEDIV2 (MYBIGBUFFERSIZE/2)
#define CDMINALIGN 32

/*! \internal
*/
typedef enum
{
    START,
    SMALL_FILE,
    ALIGN_THE_START,
    ALIGN_READ,
    NONALIGN_READ,
    DONE
} FileReadStateEnum;

struct ReadStatus
{
    FileReadStateEnum CurState;
    s32 MemBase;
    s32 FileBase;
    s32 Size;
    // ***********************************************************************
    // POS:  BS     0         SA                                     EA     E
    //       |      |---------|--------------------------------------|------|
    // ***********************************************************************
    int MBS;// = AlignDownP2(MemBase,BUFFERSIZEDIV2);
    int MSA;// = AlignUpP2(MemBase,BUFFERSIZEDIV2);
    int ME;//  = MemBase+Size;
    int MEA;// = AlignDownP2(ME,BUFFERSIZEDIV2);

    int FBS;// = AlignDownP2(FileBase,BUFFERSIZEDIV2);
    int FSA;// = AlignUpP2(FileBase,BUFFERSIZEDIV2);
    int FE;//  = FileBase+Size;
    int FEA;// = AlignDownP2(FE,BUFFERSIZEDIV2);

    int SD;
    int TA;
    int ret;

    volatile void *MemPointer;
    volatile s32 parmFileBase;
    volatile s32 parmSize;

    char Data[MYBIGBUFFERSIZE] __attribute__ ((aligned (32)));
};

static ReadStatus gCurRead;

static void AsyncDVDCallback(s32 Result, DVDFileInfo* FileInfo);

static void QEndOp()
{
    gCurRead.CurState = DONE;
    AsyncDVDCallback(0,0);
}

static int AsyncDVDRead(DVDFileInfo* FileInfo)
{
    //************************************************************************
    // Do copy and update read state from last file read
    //************************************************************************

    // the number of bytes from the read buffer that we actually keep and copy
    // into out data buffer; This can be later than the previous nBtesToRead
    // value due to alignment of the read
    unsigned int nBytesToKeep = 0;
    if( gCurRead.CurState == ALIGN_THE_START )
    {
        nBytesToKeep = static_cast<unsigned int>(gCurRead.TA);
        // copy enough to align the beginning of the next read but do not read
        // more than the file size
        memcpy((char*)gCurRead.MemBase, gCurRead.Data+gCurRead.SD, nBytesToKeep);
        gCurRead.FileBase += nBytesToKeep;
        gCurRead.MemBase += nBytesToKeep;

        // Parts File copied so far ******************************************
        // POS:  MBS    0         MSA                               MEA    ME
        //       |      |---------|----------------------------------|------|
        //              **********
        // *******************************************************************
    }
    else if( gCurRead.CurState == ALIGN_READ )
    {
        nBytesToKeep = static_cast<unsigned int>(gCurRead.MEA-gCurRead.MemBase);
        // SSX bandage
        DCInvalidateRange((void*)gCurRead.MemBase, nBytesToKeep);
        gCurRead.FileBase += nBytesToKeep;
        gCurRead.MemBase += nBytesToKeep;
        // Parts File copied so far ******************************************
        // POS:  MBS    0         MSA                               MEA    ME
        //       |      |---------|----------------------------------|------|
        //              *********************************************
        // *******************************************************************
    }
    else if( gCurRead.CurState == NONALIGN_READ )
    {
        nBytesToKeep = static_cast<unsigned int>(gCurRead.ME - gCurRead.MemBase);
        nBytesToKeep = MIN(nBytesToKeep,MYBIGBUFFERSIZEDIV2);
        memcpy((char*)gCurRead.MemBase,gCurRead.Data,nBytesToKeep);
        gCurRead.FileBase += nBytesToKeep;
        gCurRead.MemBase  += nBytesToKeep;
        // Parts File copied so far ******************************************
        // POS:  MBS    0         MSA                                    MEA    ME
        //       |      |---------|--------------------------------------|------|
        //              **********? copies MYBIGBUFFERSIZEDIV2 per-loop ?
        // *******************************************************************
    }
    else if( gCurRead.CurState == SMALL_FILE )
    {
        //FBS--FileBase-MSA
        //0----SD-------31---------64
        //iiiiirrrrrrriiiiiiiiiiiiii
        // or
        //0----SD-------31---------64
        //iiiiirrrrrrrrrrrrrrrriiiii
        nBytesToKeep = static_cast<unsigned int>(gCurRead.Size);
        // copy the whole file because it is really small
        memcpy((char*)gCurRead.MemBase,gCurRead.Data+gCurRead.SD,nBytesToKeep);
        gCurRead.CurState = SMALL_FILE;
        QEndOp(); // This tells filesys layer I am done
        return static_cast<int>(nBytesToKeep);
    }

    //************************************************************************
    // Start the next read if necessary
    //************************************************************************

    // number of bytes to read from the dvd; this value can put us past the
    // end of the file due to alignment considerations
    int nBytesToRead = 0;

    // CD read is aligned and is larger 32 bytes
    if( gCurRead.MemBase == gCurRead.MSA && gCurRead.MemBase < gCurRead.MEA )
    {
        // Start this Read ***************************************************
        // POS:  MBS    0         MSA                               MEA    ME
        //       |      |---------|---------------------------------|------|
        //              **********RRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRR
        // *******************************************************************

        //Yeah we can
        // tell the call back how to copy the data out after the read
        gCurRead.CurState = ALIGN_READ;
        nBytesToRead = gCurRead.MEA-gCurRead.MemBase;
        gCurRead.ret = DVDReadAsync(FileInfo, (char*)gCurRead.MemBase, nBytesToRead, gCurRead.FileBase, AsyncDVDCallback);
        return static_cast<int>(nBytesToKeep);
    }
    else if( gCurRead.ME > gCurRead.MemBase )  // the CD read is not aligned
    {
        // Start this Read ***************************************************
        // POS:  MBS    0         MSA                               MEA    ME
        //       |      |---------|---------------------------------|------|
        //              **********R?????????????????????????????????
        // *******************************************************************

        // need to loop in this state once per MYBIGBUFFERSIZEDIV2 of data
        //0-----32
        //rrrrrrr
        //or
        //0-----32
        //rrrrrii
        // I can read a little beyond the end of the file but not too far
        nBytesToRead = AlignUpP2((gCurRead.ME-gCurRead.MemBase),CDMINALIGN);
        nBytesToRead = MIN(nBytesToRead,MYBIGBUFFERSIZEDIV2);

        // tell the callback how to copy the data out after the read
        gCurRead.CurState = NONALIGN_READ;
        gCurRead.ret += DVDReadAsync(FileInfo, gCurRead.Data, nBytesToRead,
            (s32)gCurRead.FileBase, AsyncDVDCallback);
        return static_cast<int>(nBytesToKeep);
    }

    //************************************************************************
    // No more data to read
    //************************************************************************
    QEndOp(); // tell filesys layer I am done

    return static_cast<int>(nBytesToKeep);
}

static void StartNonAlignedAsyncRead(DVDFileInfo *FileInfo,void *MemPointer,s32
                                    FileBase,s32 Size)
{
    if(Size==0)
    {
        QEndOp(); // This tells filesys layer I am done
        return;
    }

    gCurRead.Size = Size;
    gCurRead.FileBase = FileBase;
    gCurRead.MemBase = (s32)MemPointer;

    // ***********************************************************************
    // POS:  BS     0         SA                                     EA     E
    //       |      |---------|--------------------------------------|------|
    // ***********************************************************************
    gCurRead.MBS = AlignDownP2(gCurRead.MemBase,BUFFERSIZEDIV2);
    gCurRead.MSA = AlignUpP2(gCurRead.MemBase,BUFFERSIZEDIV2);
    gCurRead.ME  = gCurRead.MemBase+gCurRead.Size;
    gCurRead.MEA = AlignDownP2(gCurRead.ME,BUFFERSIZEDIV2);

    gCurRead.FBS = AlignDownP2(gCurRead.FileBase,BUFFERSIZEDIV2);
    gCurRead.FSA = AlignUpP2(gCurRead.FileBase,BUFFERSIZEDIV2);
    gCurRead.FE  = gCurRead.FileBase+gCurRead.Size;
    gCurRead.FEA = AlignDownP2(gCurRead.FE,BUFFERSIZEDIV2);

    gCurRead.ret=0;

    // piture not to scale :)
    //0----SD-------31----ED----64
    //     |---TA---|-TAE-|
    gCurRead.SD=gCurRead.FileBase-gCurRead.FBS;
    gCurRead.TA=BUFFERSIZEDIV2-gCurRead.SD;

    int readSize;
    if( gCurRead.Size < BUFFERSIZEDIV2 ) // The file is smaller then BUFFERSIZEDIV2
    {
        //FBS--FileBase-MSA
        //0----SD-------31---------64
        //iiiiirrrrrrriiiiiiiiiiiiii
        // or
        //0----SD-------31---------64
        //iiiiirrrrrrrrrrrrrrrriiiii
        gCurRead.CurState = SMALL_FILE;// tell the call back how to copy out the data
        int sizealigned = gCurRead.FE-gCurRead.FBS;
        readSize = AlignUpP2(sizealigned,CDMINALIGN);
        gCurRead.ret = DVDReadAsync(FileInfo, gCurRead.Data, readSize,
            (s32)gCurRead.FBS, AsyncDVDCallback);
        // off then end of the file max CDMINALIGN bytes
    }
    else if( gCurRead.SD != 0 ) // The CD read is not aligned
    {
        //FBS--FileBase-MSA
        //0----SD-------31---------64
        //iiiiirrrrrrrrrrriiiiiiiiiii

        // Read these bytes **************************************************
        // POS:  BS     0         SA                                 EA     E
        //       |      |---------|----------------------------------|------|
        //              RRRRRRRRRR
        // *******************************************************************

        // tell the callback how to copy out the data
        gCurRead.CurState = ALIGN_THE_START;
        readSize = AlignUpP2(gCurRead.TA,CDMINALIGN);
        gCurRead.ret += DVDReadAsync( FileInfo, gCurRead.Data, readSize,
            (s32)gCurRead.FBS ,AsyncDVDCallback );
    }
    else if( gCurRead.MemBase == gCurRead.MSA && gCurRead.MemBase < gCurRead.MEA )
    {
        // CD read is aligned and is larger 32 bytes
        // Read these bytes **************************************************
        // POS:  BS     0         SA                                 EA     E
        //       |      |---------|----------------------------------|------|
        //              RRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRR
        // *******************************************************************
        // tell thecall back how to copy out the data (it will really only
        // update pointer in this state)
        gCurRead.CurState = ALIGN_READ;
        readSize = gCurRead.MEA-gCurRead.MemBase;
        gCurRead.ret = DVDReadAsync( FileInfo, (char*)gCurRead.MemBase,
            readSize, gCurRead.FileBase,AsyncDVDCallback );
    }
    else if(gCurRead.ME > gCurRead.MemBase) // File is not aligned in memory
    {
        // Read these bytes **************************************************
        // POS:  BS     0         SA                                 EA     E
        //       |      |---------|----------------------------------|------|
        //              RRRRRRRRRRRRR? will read upto EA in many passes
        // *******************************************************************

        //0-----32
        //rrrrrrr
        //or
        //0-----32
        //rrrrrii
        // can't read very far off then end of the file
        readSize = AlignUpP2((gCurRead.ME-gCurRead.MemBase),CDMINALIGN);
        readSize = MIN(readSize,MYBIGBUFFERSIZEDIV2);
        // tell the call back how to copy out the data
        gCurRead.CurState = NONALIGN_READ;
        gCurRead.ret += DVDReadAsync(FileInfo, gCurRead.Data, readSize,
            (s32)gCurRead.FileBase,AsyncDVDCallback);
    }
}

/*! \internal
*/
enum
{
    START_READ_MSG,
    CONT_READ_MSG
};

static void AsyncDVDCallback(s32, DVDFileInfo*)
{
    OSSendMessage(
        &ReadFileThreadMsgQ,
        (void*)CONT_READ_MSG,
        OS_MESSAGE_BLOCK);
}

class DvdFileHandle
{
public:
    static void *operator new(size_t size)
    {
        return RWFILESYSTEM_ALLOCATE(size);
    }

    static void operator delete(void *p)
    {
        return Manager::Free(p);
    }

    DVDFileInfo fileInfo;
    int         offset;
    int         size;

    char        name[FILESYS_MAX_PATH];
};

// Specific to path iteration, holds the file pattern,
// the device handle, and the path
class DvdFindHandle
{
public:
    void *operator new(size_t size)
    {
        return RWFILESYSTEM_ALLOCATE(size);
    }

    void operator delete(void *p)
    {
        return Manager::Free(p);
    }

    explicit DvdFindHandle(DVDDir dir, char * filePattern, char * directory)
        : mDir(dir)
    {
        if(filePattern != NULL)
        {
            strcpy(mFilePattern,filePattern);
        }
        else
        {
            mFilePattern[0] = '\0';
        }

        if(directory != NULL)
        {
            strcpy(mDirectory,directory);
        }
        else
        {
            mDirectory[0] = '\0';
        }
    }

    DVDDir *GetDir()                    { return &mDir; }
    const char * GetFilePattern() const { return mFilePattern; }
    const char * GetDirectory()   const { return mDirectory; }

private:
    DVDDir mDir;
    char mFilePattern[128];
    char mDirectory[FILESYS_MAX_PATH];
};

//////////////////////////////////////////////////////////////////////////////
// Driver functions                                                         //
//////////////////////////////////////////////////////////////////////////////

bool WiiDvdFileDeviceDriver::Init()
{
    // prep the message queue
    OSInitMessageQueue(
        &ReadFileThreadMsgQ,
        ReadFileThreadMsgData,
        NUMBEROFMSG);
    DVDInit();
    return true;
}

void *WiiDvdFileDeviceDriver::Open(const char *filename, unsigned int, Handle **)
{
    char newname[256];
    DvdFileHandle* dvd_fh = new DvdFileHandle();

    // Since the driver is responsible for converting directory separators, let's convert those
    //   backslashes to forward slashes
    char * name = newname;
    const char * namesrc = filename;
    while (*namesrc != 0)
    {
        if (*namesrc == '\\')
        {
            *name++ = '/';
        }
        else
        {
            *name++ = *namesrc;
        }
        namesrc++;
    }
    *name = 0;
    name = newname;

    memcpy(dvd_fh->name, name, FILESYS_MAX_PATH);

    // If the name includes a device specifier, skip over it
    if (strncmp(name, "dvd:", 4) == 0)
        name += 4;

    // skip over the leading / character
    ++name;

    int32_t ret=0;
    s32 entryNum = DVDConvertPathToEntrynum(name);
    if( entryNum != -1 )
    {
        ret = DVDFastOpen(entryNum, &dvd_fh->fileInfo);
    }

    if( ret == 0)
    {
        // There are no error codes for DVDOpen, as I believe the only case where
        // control will return to the user is file not found.
        Manager::GetInstance()->GetErrorHandler()(
            static_cast<int>(DVDGetFileInfoStatus(&dvd_fh->fileInfo)), 
            Manager::FILEERROR_OPEN, this, name);

        // Not found; release the file handle
        delete dvd_fh;
        return NULL;
    }
    else
    {
        dvd_fh->size = (int)DVDGetLength(&dvd_fh->fileInfo);
        return dvd_fh;
    }
}

void WiiDvdFileDeviceDriver::Close(void *h)
{
    DvdFileHandle *dvd_fh = reinterpret_cast<DvdFileHandle *>(h);
    DVDClose(&dvd_fh->fileInfo);
    delete dvd_fh;
}

unsigned int WiiDvdFileDeviceDriver::Read(void *h, void *buf, unsigned int bufsize,
                                          DeviceDriver *, void *)
{
    DvdFileHandle *dvd_fh = reinterpret_cast<DvdFileHandle *>(h);
    OSMessage msg;

    if ((int)bufsize > dvd_fh->size - dvd_fh->offset)
    {
        if (dvd_fh->offset >= dvd_fh->size)
        {
#ifdef EA_DEBUG
//            PRINT_string(
//                "Warning: WiiDvdFileDeviceDriver::Read - Starting offset is greater than or equal to the length of the file.\n");
#endif
            bufsize = 0;
        }
        else
        {
            bufsize = static_cast<unsigned int>(dvd_fh->size - dvd_fh->offset);
        }
    }

    StartNonAlignedAsyncRead((DVDFileInfo *)&dvd_fh->fileInfo, buf,
        dvd_fh->offset, static_cast<long>(bufsize));

    OSReceiveMessage(
        &ReadFileThreadMsgQ,
        &msg,
        OS_MESSAGE_BLOCK);

    EA_ASSERT_MSG((int)msg == CONT_READ_MSG,
        ("msg is other than CONT_READ_MSG\n"));

    int nBytesKept = 0;
    while(gCurRead.CurState!=DONE)
    {
        nBytesKept += AsyncDVDRead((DVDFileInfo *)&dvd_fh->fileInfo);

        OSReceiveMessage(
            &ReadFileThreadMsgQ,
            &msg,
            OS_MESSAGE_BLOCK);

        EA_ASSERT_MSG((int)msg == CONT_READ_MSG,
            ("msg is other that CONT_READ_MSG\n"));
    }

    while(1)
    {
        s32 Err = DVDGetDriveStatus();
        if(
            DVD_STATE_END == Err ||
            DVD_STATE_PAUSING == Err ||
            DVD_STATE_BUSY == Err
            )
        {
            break;
        }
        else if (
            DVD_STATE_FATAL_ERROR == Err ||
            DVD_STATE_NO_DISK == Err ||
            DVD_STATE_WRONG_DISK == Err
            )
        {
            if (!Manager::GetInstance()->GetErrorHandler()(
                static_cast<int>(Err), 
                Manager::FILEERROR_OPEN, this, dvd_fh->name))
            {
                break;
            }
        }

        EA::Thread::ThreadSleep(100);
    }

    dvd_fh->offset += nBytesKept;
    return static_cast<unsigned int>(nBytesKept);
}

uint64_t WiiDvdFileDeviceDriver::Seek(void *h, uint64_t offset, int whence,
                                      DeviceDriver *, void *)
{
    DvdFileHandle *dvd_fh = reinterpret_cast<DvdFileHandle *>(h);

    // the driver only needs to handle seeks from the origin becuase the caller
    // (the Async module) keeps track of the file position
    EA_ASSERT(whence == FSEEK_BEGIN);
    (void)whence;

    dvd_fh->offset = static_cast<int>(offset);

    return offset;
}

uint64_t WiiDvdFileDeviceDriver::GetSize(void *h)
{
    return reinterpret_cast<DvdFileHandle *>(h)->size;
}

#define ISJUNK(V) (*(V) == '/' || *(V) == '\\' || *(V) == ':')

void*   WiiDvdFileDeviceDriver::FindBegin(const char *filePattern, FindData *findData)
{
    char dirName[FILESYS_MAX_PATH];
    const char *fp = filePattern;

    const char *devName = GetName();
    const size_t devNameLen = strlen(devName);

    // Skip over the device name, if present
    if (!strncmp(filePattern, devName, devNameLen))
    {
        fp += devNameLen;
    }

    memset(dirName, 'a', sizeof(dirName));
    strcpy(dirName, fp);

    {
        // convert any backslashes to forward slashes
        char *c = dirName;
        while (*c != '\0')
        {
            if (*c == '\\')
                *c = '/';
            c++;
        }
    }

    unsigned long len = strlen(dirName);
    char *pattern = NULL;
    char *patend  =  dirName + len;
    char *end     =  patend;
    DVDDir dir;

    // search for a file pattern, we need just
    // the directory part.
    while(end && end > dirName && !ISJUNK(end))
    {
        end--;
    }

    // We have a file pattern
    if(end && end != patend && (*end == '/' || *end == '\\'))
    {
        pattern   = (end+1);
        *end      = '\0';
    }

    int result = DVDOpenDir(dirName, &dir);

    if (result != 0)
    {
        DvdFindHandle *fileHandle = new DvdFindHandle(dir, pattern, dirName);

        if(!FindNext(fileHandle,findData))
        {
            FindEnd(fileHandle);
            fileHandle = NULL;
            return NULL;
        }

        return fileHandle;
    }

    return NULL;
}

bool    WiiDvdFileDeviceDriver::FindNext(void *h, FindData *findData)
{
    DvdFindHandle *fh = static_cast<DvdFindHandle *>(h);

    const char * pattern = fh->GetFilePattern();
    bool  noPattern      = pattern[0] == '\0';

    DVDDirEntry dirEntry;
    int result = DVDReadDir(fh->GetDir(), &dirEntry);

    while (result != 0)
    {
        if (noPattern || MatchesWildcardPattern(dirEntry.name, pattern))
        {
            strcpy(findData->mName, dirEntry.name);

            // On the Wii, all files are read-only.
            findData->mFlags = DirectoryEntry::FLAG_READONLY;

            if (dirEntry.isDir)
            {
                findData->mFlags |= DirectoryEntry::FLAG_DIRECTORY;
            }
            else
            {
                DVDFileInfo fileInfo;
                if (DVDFastOpen(static_cast<s32>(dirEntry.entryNum), &fileInfo))
                {
                    findData->mSize = DVDGetLength(&fileInfo);

                    DVDClose(&fileInfo);
                }
            }

            findData->mCreationTime = 0;
            findData->mAccessTime = 0;
            findData->mModificationTime = 0;

            // Set Creation, Access, Modification time to 0.
            return true;
        }
        result = DVDReadDir(fh->GetDir(), &dirEntry);
    }

    // no more matching entries in this directory.
    return false;
}

bool    WiiDvdFileDeviceDriver::FindEnd(void *h)
{
    DvdFindHandle *fh = static_cast<DvdFindHandle *>(h);
    DVDCloseDir(fh->GetDir());
    delete fh;

    return true;
}

bool    WiiDvdFileDeviceDriver::GetNameFromHandle(void *h, char *name, size_t nameBufferSize)
{
    DvdFileHandle *dvd_fh = static_cast<DvdFileHandle *>(h);

    EA_ASSERT(dvd_fh != NULL);

    strncpy(name, dvd_fh->name, nameBufferSize);

    name[nameBufferSize-1] = '\0';

    return true;
}

WiiDvdFileDeviceDriver gDeviceDriverImpl;

}
}
}
