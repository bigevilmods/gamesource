// (c) Electronic Arts. All Rights Reserved.

/**
\file
Implements rw::core::filesys::DefaultStream for non-PC platforms. 
*/

#include "rw/core/filesys/directory.h"
#include "rw/core/filesys/path.h"
#include "rw/core/filesys/ifilestream.h"
#include "EAAssert/eaassert.h"

#ifdef CreateDirectory
#undef CreateDirectory
#endif

namespace rw {
namespace core {
namespace filesys {

DefaultFileStream::DefaultFileStream()
    : mFileHandle(NULL)
    , mPos(0)
{}

int DefaultFileStream::Open(const char * filename,unsigned int modeflags)
{
    // Ensure file stream is closed
    EA_ASSERT(mFileHandle == NULL);

    mPos = 0;

    // Ensure the directory exists if the user desires to create the file.
    // This behavior mimics what the old PC-only default file stream
    // implementation used to do.
    if (modeflags & AsyncOp::FOPEN_CREATE)
    {
        char directoryPath[FILESYS_MAX_PATH] = {};
        GetPathDirectory(filename, directoryPath, sizeof directoryPath);

        if (!CreateDirectory(directoryPath, CREATEDIRECTORY_FLAG_RECURSIVE))
            return -1;
    }

    mFileOp.Open(filename, modeflags);
    filesys::AsyncOp::OpStatus status = mFileOp.GetStatus(EA::Thread::kTimeoutNone);
    mFileHandle = mFileOp.GetResultHandle();

    int rc = (mFileHandle != NULL && status == filesys::AsyncOp::OPSTATUS_COMPLETE) ? 0 : -1;

    return rc;
}

void DefaultFileStream::Close()
{
    if(mFileHandle != NULL)
    {
        mFileOp.Close(mFileHandle);
        mFileOp.GetStatus(EA::Thread::kTimeoutNone);
        mPos        = 0;
        mFileHandle = NULL;
    }
}

DefaultFileStream::~DefaultFileStream()
{
    if(mFileHandle)
    {
        this->Close();
    }
}

uint64_t DefaultFileStream::Read(void * buffer, size_t count)
{
    EA_ASSERT(mFileHandle != NULL);

    mFileOp.Read(mFileHandle,buffer,mPos,count);
    // update file pointer with read bytes
    uint64_t sizeRead = mFileOp.GetResultSize();

    mPos += sizeRead;
    return sizeRead;
}

uint64_t DefaultFileStream::Write(const void * buffer, size_t count)
{
    EA_ASSERT(mFileHandle != NULL);
    mFileOp.Write(mFileHandle,buffer,mPos,count);

    mFileOp.GetStatus(EA::Thread::kTimeoutNone);

    // update file pointer with bytes written
    uint64_t sizeWritten = mFileOp.GetResultSize();

    mPos  += sizeWritten;
    return sizeWritten;
}

uint64_t DefaultFileStream::Tell()
{
    EA_ASSERT(mFileHandle != NULL);
    return mPos;
}

int DefaultFileStream::Seek(int64_t offset, IFileStream::SeekOrigin origin)
{
    EA_ASSERT(mFileHandle != NULL);

    switch(origin)
    {
    case IFileStream::SEEKORIGIN_CURRENT:
        mPos += offset;
        break;

    case IFileStream::SEEKORIGIN_BEGINNING:
        EA_ASSERT_MSG(offset >= 0, "Cannot seek to a negative position from the beginning");
        mPos = static_cast<uint64_t>(offset);
        break;

    case IFileStream::SEEKORIGIN_END:
        mPos = GetSize(mFileHandle) + offset;
        break;

    default:
        return -1;
    }

    return 0;
}

uint64_t DefaultFileStream::Size()
{
    EA_ASSERT(mFileHandle != NULL);
    return GetSize(mFileHandle);
}

}}}


