// (c) Electronic Arts. All Rights Reserved.

/**
\file
Implements rw::core::fileys::AsyncOp.
*/

#include "private_rwdevice.h"
#include "private_rwhandle.h"
#include "private_rwprofiler.h"
#include <string.h>

#include "EAAssert/eaassert.h"
#include "eathread/eathread_sync.h"
#include "EAStdC/EAMemory.h"

using namespace EA::StdC;

namespace rw {
namespace core {
namespace filesys {

// If the caller did not request that a callback function be
// called when an operation completes, this function is called.
static void DefaultAsyncOpCallback(AsyncOp *)
{}

// Helper function that makes sure mStatus passes the write barrier
AsyncOp::OpStatus InsertOpWithStatusUpdate(AsyncOp *op);
AsyncOp::OpStatus InsertOpWithStatusUpdate(AsyncOp *op)
{
    AsyncOp::OpStatus status;
    // Make sure the device exists.
    if (op->mDevice->IsRegistered())
    {
        status = AsyncOp::OPSTATUS_PENDING;
        op->mStatus = status;
        op->mCancelled = false;
        EAWriteBarrier();
        op->mDevice->InsertOp(op);
    }
    else
    {
        status = AsyncOp::OPSTATUS_FAILED;
        op->mStatus = status;
        EAWriteBarrier();
    }

    return status;
}

AsyncOp::AsyncOp() :
    mStatus(OPSTATUS_NEW), mCancelled(false), mIsRead(false),
    mPriority(0), mFileHandle(NULL), mContext(NULL),
    mCallbackFunc(DefaultAsyncOpCallback), mFlags(0), mOffset(0),
    mAmount(0), mData(NULL), mTotalBytes(0), mDevice(NULL),
    mExecFunc(NULL), mReadProcessContext(NULL),
    mReadProcessDevice(NULL), mTrackDeviceIdleTime(true)
{
    mpNext = NULL;
    mpPrev = NULL;

    Memclear(mFilename, sizeof mFilename);

    // no other members are initialzed.  Each operation sets them
    // when the operation is scheduled.
}

AsyncOp::~AsyncOp()
{
    EAReadBarrier();    // force mStatus to be pulled from main memory
    EA_ASSERT_MSG(mStatus != OPSTATUS_PENDING,
        "This instance has an unfinished operation associated with it.  "
        "Cancel the operation or wait for it to complete before destroying the AsyncOp object.");
    mFileHandle = NULL;
    mContext = NULL;
    mData = NULL;
    mDevice = NULL;
}

/*!
    \brief schedules an asynchronous operation to open a file.

    A file path is of the format \<device name\>:/\<dir\>/\<name\>,
    where \<device\> is the name of a device which has been
    registered with the file system library, \<dir\> is one or
    more directories, and \<name\> is the name of the file.
    The \<device\> and \<dir\> parts of the path name are optional.

    We refer to a path that has a device and full directory
    path to the file as fully-qualified.  Non-qualified file
    names, such as "myfile.txt", are treated specially by
    the file system library.  The library iterates through
    each location in the search path, appending the file
    name to each location.  The file will be opened on the first
    device to return a non-zero value.  For file paths that
    begin with a directory separator character, such as "/myfile.txt",
    each device in the search path will be checked, but the file
    path will be appended directly to the root of the device.  For
    example, if you had two locations on the C: drive, such as
    c:\\dir1 and c:\\dir2, the file system will try to open
    c:\\e yfile.txt twice.

    When the operation completes, use GetResultHandle to obtain
    the Handle to the newly opened file.

    The size of the opened file may be obtained by using the
    GetSize(const Handle *) function.

    BEGIN Xbox360 only

    The devices recognized by the library are D: (the DVD),
    CACHE:, and NET:.

    Before you can use the cache: drive, you must mount it with
    XMountUtilityDrive.  For further information, look up that
    function in the Xbox 360 documentation.

    The NET: device is used with remote file sharing, which allows
    a program running on Xbox360 devkit to reference a file on a
    remote PC (similar to PS3's host file system.)  For further
    information on setting this up, please refer to Development
    Environment|Tools|Console|Remote File Sharing in the Xbox360
    documentation.

    END Xbox360 only

    BEGIN Windows specific

    If your game uses non-ANSIC code pages, you can still use rwfilesystem.
    Recompile the library with /DRWFILESYSTEM_CODEPAGE=your_code_page.  You
    can then pass multi-byte character strings to the library which will be
    converted back to your program's code page inside the library.

    END Windows specific

    \importlib rwfilesystem

    \param FilePath path to file.  Can be absolute or relative. However, relative paths with a parent directory (i.e. "../") are only supported on Windows. UNC paths are supported.
    \param modeflags 0 or a combination of OpenFlags flags
    \param OpDoneCallback function to execute when the operation completes
    \param context pointer for callback function
    \param priority priority of the operation.

    \see AsyncOp::GetResultHandle
    \see AsyncOp::RegisterDevice
    \see Manager::SetSearchPath
    \see AsyncOp::Exists
    \see AsyncOp::GetSize(const char *)
    \see AsyncOp::GetSize(const Handle *)
*/
void AsyncOp::Open(
    const char *FilePath,
    unsigned int modeflags,
    AsyncOpCallback *OpDoneCallback,
    void *context,
    int priority)
{
    EAReadBarrier();    // force mStatus to be pulled from main memory

    EA_ASSERT_MSG(mStatus != OPSTATUS_PENDING,
        "This instance already has an unfinished operation associated with it.");

    mStatus = OPSTATUS_PENDING;
    EAWriteBarrier();

    mIsRead = false;
    mPriority = priority;
    mFileHandle = NULL;
    mContext = context;
    mFlags = modeflags;
    mAmount = 0;
    mTotalBytes = 0;

    mDevice = Device::GetInstance(FilePath);

    size_t nameLength = strlen(FilePath) + 1;
    memcpy(mFilename, FilePath, nameLength);
    mCallbackFunc = (OpDoneCallback != NULL) ? OpDoneCallback : DefaultAsyncOpCallback;
    mExecFunc = DoOpen;
    mReadProcessContext = NULL;
    mReadProcessDevice = NULL;

    Manager *manager = Manager::GetInstance();
    AsyncOpOpenCallback callback = manager->GetAsyncOpOpenCallback();
    if (callback)
        callback(this, mFilename, &mFlags);

    // NOTE: Do not move this upwards as the async op open callback may change the filename.
    LogGenericEvent(ProfilerEventOpen, mDevice->GetDriver()->GetName(), mFilename);

    if (mDevice->IsVirtual())
    {
        // Virtual devices don't have a running thread.  So they
        // are inserted on the NULL device thread instead.  Once
        // they are opened, further operations (e.g. reads) will
        // be done on the parent devices' thread.
        manager->GetNullDevice()->InsertOp(this);
    }
    else
    {
        InsertOpWithStatusUpdate(this);
    }
}

AsyncOp::OpStatus AsyncOp::DoOpen(AsyncOp *op)
{
    OpStatus result = OPSTATUS_PENDING;

    const char *devName;

    op->mFileHandle = new Handle(op->mFilename, op->mFlags, op->mDevice);

    // Handle's constructor can potentially use a different device than given
    op->mDevice = op->mFileHandle->mDevice;

    // See if the open succeeded.  If we were permitted to use
    // exception handling, we'd put a try/catch block around
    // the statement above.
    if (op->mFileHandle->mDevFileHandle != NULL)
    {
        result = OPSTATUS_COMPLETE;

        // Use the correct device name, now that's it's known.
        devName = op->mFileHandle->GetDeviceName();
    }
    else
    {
        delete op->mFileHandle;
        op->mFileHandle = NULL;
        result = OPSTATUS_FAILED;
        devName = op->mDevice->GetDriver()->GetName();
    }

    LogOpenCompleteEvent(devName, 
            reinterpret_cast<intptr_t>(op->mFileHandle), 
            op->mFilename,
            op->mFileHandle ? op->mFileHandle->mDevFileHandle : NULL );

    return result;
}

/*!
    \brief Close an open file handle

    Close schedules an asynchronous operation to close an
    open file.  Be sure that there are no other threads currently
    doing read or write operations on the file.

    \importlib rwfilesystem

    \param FileHandle handle of file to close
    \param OpDoneCallback function to call when the operation has been completed
    \param context context value for callback function
    \param priority operation priority

    \see AsyncOp::Open
    \see AsyncOp::Read
    \see AsyncOp::Write
*/
void AsyncOp::Close(
    Handle *FileHandle,
    AsyncOpCallback *OpDoneCallback,
    void *context,
    int priority)
{
    LogGenericEvent(ProfilerEventClose, FileHandle->GetDeviceName(), &FileHandle);

    EAReadBarrier();    // force mStatus to be pulled from main memory

    EA_ASSERT_MSG(mStatus != OPSTATUS_PENDING,
        "This instance already has an unfinished operation associated with it.");

    mIsRead = false;
    mPriority = priority;
    mFileHandle = FileHandle;
    mContext = context;
    mCallbackFunc = (OpDoneCallback != NULL) ?
        OpDoneCallback : DefaultAsyncOpCallback;

    mFlags = 0;
    mOffset = 0;
    mAmount = 0;
    mData = NULL;
    mTotalBytes = 0;
    mDevice = FileHandle->mDevice;
    mExecFunc = DoClose;
    mReadProcessContext = NULL;
    mReadProcessDevice = NULL;

    OpStatus insertStatus = InsertOpWithStatusUpdate(this);
    // Close the handle manually if the insert operation failed.
    if (insertStatus == OPSTATUS_FAILED)
    {
        delete mFileHandle;
        mFileHandle = NULL;
    }
}

AsyncOp::OpStatus AsyncOp::DoClose(AsyncOp *op)
{
    // Here because we still need the file handle
    LogGenericEvent(ProfilerEventCloseComplete, op->mFileHandle->GetDeviceName(), op->mFileHandle);

    // As with open, it'd be nice to do a try/catch around this and set
    // mStatus to OPSTATUS_FAILED if the driver throws an exception.
    delete op->mFileHandle;
    op->mFileHandle = NULL;
    Memclear(op->mFilename, sizeof op->mFilename);

    return OPSTATUS_COMPLETE;
}

/*!
    \brief Flush an open file handle

    Flush schedules an asynchronous operation to flush an
    open file.

    \importlib rwfilesystem

    \param FileHandle handle of file to close
    \param OpDoneCallback function to call when the operation has been completed
    \param context context value for callback function
    \param priority operation priority

    \see AsyncOp::Open
    \see AsyncOp::Read
    \see AsyncOp::Write
*/

void AsyncOp::Flush(
	Handle *FileHandle,
	AsyncOpCallback *OpDoneCallback,
	void *context,
	int priority)
{
    EAReadBarrier();    // force mStatus to be pulled from main memory

    EA_ASSERT_MSG(mStatus != OPSTATUS_PENDING,
        "This instance already has an unfinished operation associated with it.");

    mIsRead = false;
    mPriority = priority;
    mFileHandle = FileHandle;
    mContext = context;
    mCallbackFunc = (OpDoneCallback != NULL) ?
        OpDoneCallback : DefaultAsyncOpCallback;

    mFlags = 0;
    mOffset = 0;
    mAmount = 0;
    mData = NULL;
    mTotalBytes = 0;
    mDevice = FileHandle->mDevice;
    mExecFunc = DoFlush;
    mReadProcessContext = NULL;
    mReadProcessDevice = NULL;

    InsertOpWithStatusUpdate(this);
}

AsyncOp::OpStatus AsyncOp::DoFlush(AsyncOp *op)
{
    OpStatus result = OPSTATUS_PENDING;
    Handle *handle = op->mFileHandle;
    DeviceDriver *driver = handle->mDriver;

	driver->Flush(handle->mDevFileHandle);

    return OPSTATUS_COMPLETE;
}

/*!
    Read schedules an asynchronous operation to read some data
    from an open file into memory.

    If the number of bytes to read is larger than the read quantum
    size or the device's Optimal Read size, the read will be done
    in several smaller chunks.  The callback will only be called
    when the number of bytes requested has been read in, or the
    end-of-file is reached.

    When the operation is complete, use GetResultSize to determine
    the number of bytes actually read.

    \importlib rwfilesystem

    \param FileHandle handle of file to read from
    \param buffer memory buffer to place data
    \param offset offset at which to begin reading data
    \param bytes number of bytes to read (make sure the buffer is large enough to hold this much data, even if reading that much would read past the end of file)
    \param OpDoneCallback function to call when the operation has been completed
    \param context context value for callback function
    \param priority operation priority

    \see Manager::SetReadQuantumSize
    \see AsyncOp::Open
    \see AsyncOp::Close
    \see AsyncOp::GetResultSize
    \see AsyncOp::GetSize
*/
void AsyncOp::Read(
    Handle *FileHandle,
    void * buffer,
    uint64_t offset,
    uint64_t bytes,
    AsyncOpCallback *OpDoneCallback,
    void *context,
    int priority)
{
    LogReadEvent(ProfilerEventRead, 
            FileHandle->GetDeviceName(), 
            reinterpret_cast<intptr_t>(FileHandle), 
            offset, 
            bytes, 
            buffer,
            priority);

    EAReadBarrier();    // force mStatus to be pulled from main memory
    EA_ASSERT_MSG(mStatus != OPSTATUS_PENDING,
        "This instance already has an unfinished operation associated with it.");

    mIsRead = true;
    mDevice = FileHandle->mDevice;
    mPriority = priority;
    mFileHandle = FileHandle;
    mOffset = offset;
    mData = buffer;
    mAmount = bytes;
    mTotalBytes = 0;
    mContext = context;
    mCallbackFunc = (OpDoneCallback != NULL) ? OpDoneCallback :
        DefaultAsyncOpCallback;
    mExecFunc = DoRead;
    mReadProcessContext = NULL;
    mReadProcessDevice = NULL;

    InsertOpWithStatusUpdate(this);
}

unsigned int AsyncOp::GetNumberOfBytesToRead(AsyncOp *op)
{
    // The number of bytes which are requested is the smallest of
    // the Devices' Maximum read block size, the user-settable
    // read quantum size, or the number of bytes remaining in the
    // read.
    if (!op->mFileHandle->mDevice->IsRegistered())
    {
        return 0;
    }

    unsigned int maxDeviceReadSize = op->mFileHandle->mDriver->GetMaxReadSize();
    unsigned int readQuantum = Manager::GetInstance()->GetReadQuantumSize();

    unsigned int NumBytesToRead = static_cast<unsigned int>((op->mAmount < maxDeviceReadSize) ? op->mAmount : maxDeviceReadSize);
    NumBytesToRead = (NumBytesToRead < readQuantum) ? NumBytesToRead : readQuantum;

    return NumBytesToRead;
}

AsyncOp::OpStatus AsyncOp::DoRead(AsyncOp *op)
{
    OpStatus result = OPSTATUS_PENDING;
    Handle *handle = op->mFileHandle;
    Handle *parent = handle->mpParentFile;
    DeviceDriver *driver = handle->mDriver;
    void *buffer = static_cast<void*>(static_cast<char *>(op->mData) - op->mTotalBytes);

    LogSeekEvent2(ProfilerEventDeviceSeekStart, 
            handle->GetDeviceName(), 
            reinterpret_cast<intptr_t>(handle), 
            parent->mOffset,
            buffer);

    // seek to the correct location (driver should ignore if the position is the same)
    parent->mOffset = driver->Seek(handle->mDevFileHandle, op->mOffset,
        DeviceDriver::FSEEK_BEGIN,
        parent->mDevice->GetDriver(),
        parent->mDevFileHandle);

    LogSeekEvent2(ProfilerEventDeviceSeekEnd, 
            handle->GetDeviceName(), 
            reinterpret_cast<intptr_t>(handle), 
            parent->mOffset,
            buffer);

    unsigned int NumBytesToRead = GetNumberOfBytesToRead(op);

    LogReadOrWriteEvent(ProfilerEventDeviceReadStart, 
            handle->GetDeviceName(), 
            reinterpret_cast<intptr_t>(handle),
            op->mAmount,
            driver->QueryLocation(reinterpret_cast<void*>(handle->mDevFileHandle)),
            buffer,
            op->mPriority);

    // amount is the returned number of bytes read, so make sure to set it here.
    unsigned int nread(0);

    if (NumBytesToRead != 0)
    {
        nread = driver->Read(
            handle->mDevFileHandle,
            op->mData, NumBytesToRead,
            parent->mDevice->GetDriver(),
            parent->mDevFileHandle);
    }

    LogReadOrWriteEvent(ProfilerEventDeviceReadEnd, 
            handle->GetDeviceName(), 
            reinterpret_cast<intptr_t>(handle),
            op->mAmount,
            driver->QueryLocation(reinterpret_cast<void*>(handle->mDevFileHandle)),
            buffer,
            op->mPriority);

    if (nread == (unsigned int)-1)
    {
        // Further processing is required that doesn't involve I/O.
        // Do it on another device so this device can start another
        // read.
        op->mExecFunc = ProcessRead;
        op->mReadProcessContext = driver->GetReadProcessingContext();
        op->mReadProcessDevice = driver->ReadProcessDevice();
        EA_ASSERT(op->mReadProcessDevice != NULL);
        op->mReadProcessDevice->InsertOp(op, false);
    }
    else
    {
        op->mAmount -= nread;        // number of bytes left to read
        op->mOffset += nread;        // read offset
        op->mTotalBytes += nread;    // total number of bytes read
        op->mData = static_cast<char *>(op->mData) + nread;   // location to read to

        // if there is still room in the buffer and we were given the
        // number of bytes we asked for, continue the read.  If the amount
        // of data read is is less than NumBytesToRead, we've reached the
        // end of the file and we should stop the operation.
        if ((op->mAmount > 0) && (nread == NumBytesToRead))
        {
            op->mDevice->InsertOp(op, true);
            result = op->mStatus;
        }
        else
        {
            result = OPSTATUS_COMPLETE;
        }

        if (result == OPSTATUS_COMPLETE)
            LogReadCompleteEvent(ProfilerEventReadComplete, 
                    handle->GetDeviceName(), 
                    op->mAmount, 
                    reinterpret_cast<intptr_t>(handle), 
                    buffer);
    }

    return result;
}

AsyncOp::OpStatus AsyncOp::ProcessRead(AsyncOp *op)
{
    unsigned int nread;

    int status = op->mFileHandle->mDriver->ExecuteProcessingFunction(
        op->mReadProcessContext, &nread);

    EA_ASSERT_MSG(status == 0, "This library version does not support return values other than 0.");

    (void)status;

    op->mAmount -= nread;
    op->mOffset += nread;            // read offset
    op->mTotalBytes += nread;       // total number of bytes read
    op->mData = static_cast<char *>(op->mData) + nread;   // location to read to

    // if there is still room in the buffer and we were given the
    // number of bytes we asked for, continue the read.  If the amount
    // of data read is is less than NumBytesToRead, we've reached the
    // end of the file and we should stop the operation.
    if (op->mAmount > 0)
    {
        if (nread == GetNumberOfBytesToRead(op))
        {
            op->mExecFunc = DoRead;
            op->mReadProcessContext = NULL;
            op->mReadProcessDevice = NULL;

            if (op->mDevice->IsRegistered())
            {
                op->mDevice->InsertOp(op, true);
                return OPSTATUS_PENDING;
            }
            else
            {
                return OPSTATUS_FAILED;
            }
        }
    }

    void *buffer = static_cast<void*>(static_cast<char *>(op->mData) - op->mTotalBytes);
    LogReadCompleteEvent(ProfilerEventReadComplete, 
            op->mFileHandle->GetDeviceName(), 
            op->mAmount, 
            reinterpret_cast<intptr_t>(op->mFileHandle), 
            buffer);

    return OPSTATUS_COMPLETE;
}

/*!
    Write schedules an asynchronous operation to write some data
    from memory to an open file.

    When the operation is complete, use GetResultSize to determine
    the number of bytes actually written.

    \importlib rwfilesystem

    \param FileHandle handle of file to write to
    \param buffer memory buffer to write from
    \param offset offset at which to begin writing data.  Note: if this is past the current EOF marker, the data between the previous EOF and this offset will be filled with an undefined value)
    \param bytes number of bytes to write
    \param OpDoneCallback function to call when the operation has been completed
    \param context context value for callback function
    \param priority operation priority

    \see AsyncOp::Read
    \see AsyncOp::Open
    \see AsyncOp::Close
    \see AsyncOp::GetResultSize
*/
void AsyncOp::Write(
    Handle *FileHandle,
    const void *buffer,
    uint64_t offset,
    uint64_t bytes,
    AsyncOpCallback *OpDoneCallback,
    void *context,
    int priority)
{
    LogReadEvent(ProfilerEventWrite, 
            FileHandle->GetDeviceName(), 
            reinterpret_cast<intptr_t>(FileHandle), 
            offset, 
            bytes, 
            buffer,
            priority);

    EAReadBarrier();    // force mStatus to be pulled from main memory

    EA_ASSERT_MSG(mStatus != OPSTATUS_PENDING,
        "This instance already has an unfinished operation associated with it.");

    mIsRead = false;
    mDevice = FileHandle->mDevice;
    mPriority = priority;
    mFileHandle = FileHandle;
    mOffset = offset;
    mData = const_cast<void *>(buffer);
    mAmount = bytes;
    mTotalBytes = 0;
    mContext = context;
    mCallbackFunc = (OpDoneCallback != NULL) ? OpDoneCallback :
        DefaultAsyncOpCallback;
    mExecFunc = DoWrite;
    mReadProcessContext = NULL;
    mReadProcessDevice = NULL;

    InsertOpWithStatusUpdate(this);
}

AsyncOp::OpStatus AsyncOp::DoWrite(AsyncOp *op)
{
    OpStatus result = OPSTATUS_PENDING;
    Handle *handle = op->mFileHandle;
    Handle *parent = handle->mpParentFile;
    DeviceDriver *driver = handle->mDriver;

    // seek to the correct location (driver should
    // ignore if the position is the same)
    parent->mOffset = driver->Seek(handle->mDevFileHandle, op->mOffset,
        DeviceDriver::FSEEK_BEGIN,
        parent->mDevice->GetDriver(),
        parent->mDevFileHandle);

    unsigned int NumBytesToWrite = static_cast<unsigned int>(op->mAmount);
    void *buffer = static_cast<void*>(static_cast<char *>(op->mData) - op->mTotalBytes);

    LogReadOrWriteEvent(ProfilerEventDeviceWriteStart,
            handle->GetDeviceName(),
            reinterpret_cast<intptr_t>(op->mFileHandle),
            op->mAmount,
            driver->QueryLocation(reinterpret_cast<void*>(handle->mDevFileHandle)),
            buffer,
            op->mPriority);

    // amount is the returned number of bytes read, so make sure to set it here.
    op->mAmount = driver->Write(handle->mDevFileHandle,
        op->mData, NumBytesToWrite, parent->mDevice->GetDriver(),
        parent->mDevFileHandle);

    LogReadOrWriteEvent(ProfilerEventDeviceWriteEnd,
            handle->GetDeviceName(),
            reinterpret_cast<intptr_t>(op->mFileHandle),
            op->mAmount,
            driver->QueryLocation(reinterpret_cast<void*>(handle->mDevFileHandle)),
            buffer,
            op->mPriority);

    // Update the offset value.  TODO: Does this even matter?
    parent->mOffset += op->mAmount;

    // Update function result value
    op->mTotalBytes += op->mAmount;

    result = OPSTATUS_COMPLETE;

    if (result == OPSTATUS_COMPLETE)
        LogReadCompleteEvent(ProfilerEventWriteComplete,
                handle->GetDeviceName(),
                op->mAmount,
                reinterpret_cast<intptr_t>(op->mFileHandle),
                buffer);

    return result;
}

/*!
    SetPriority allows the priority of a scheduled operation to be
    changed.  This is normally used to boost the priority of an
    operation that has been pending for a long time whose urgency
    has increased.  If the operation is no longer pending, this
    function is ignored.

    This function is NOT re-entrant. DO NOT CALL FROM MULTIPLE
    THREADS without synchronization or it will deadlock.

    \importlib rwfilesystem

    \param priority new priority for this operation

    \see AsyncOp::GetPriority
*/
void AsyncOp::SetPriority(int priority)
{
    EA_ASSERT_MSG(mDevice != NULL,
        "No device is associated with this operation.");

    if (mDevice->IsRegistered() && mPriority != priority)
    {
        // If the device is virtual, it will actually be on the NULL
        // devices' queue.  The only time this can happen is if the
        // operation is an open and the file path specified a virtual
        // device name.  Any other operation would be on the parent
        // files' physical device queue
        Device *device = !mDevice->IsVirtual() ?
            mDevice : Manager::GetInstance()->GetNullDevice();

        device->ChangeOpPriority(this, priority);
    }
}

/*!
    Cancels a pending operation.  If the operation is
    already completed or executing, the cancel is ignored.

    \importlib rwfilesystem

*/
void AsyncOp::Cancel()
{
    EAReadBarrier();    // force mStatus to be pulled from main memory

    if (mDevice != NULL && mDevice->IsRegistered() && mStatus == OPSTATUS_PENDING)
    {
        // If the device is virtual, it will actually be on the NULL
        // devices' queue.  The only time this can happen is if the
        // operation is an open and the file path specified a virtual
        // device name.  Any other operation would be on the parent
        // files' physical device queue
        Device *device = !mDevice->IsVirtual() ?
            mDevice : Manager::GetInstance()->GetNullDevice();

        device->RemoveOp(this);
    }
}

/*!
    GetStatus returns the current status of the operation.  It
    takes an optional timeout parameter in case you want to block
    the current thread until the operation completes.

    A note about using blocking behaviour - try to structure your
    loads so that the CPU(s) is(are) always busy doing something
    other than busywaiting for a File I/O request to complete.  If
    you really can't do anything else, at least compress the file
    so there is less data to load because the data can be
    decompressed much faster by the CPU than it can be read of the
    disc.

    This function is NOT re-entrant. DO NOT CALL FROM MULTIPLE
    THREADS without synchronization or it will deadlock.

    \importlib rwfilesystem

    \param timeoutAbsolute timeout for blocking behaviour

    \return OpStatus state value
*/
AsyncOp::OpStatus AsyncOp::GetStatus(const EA::Thread::ThreadTime& timeoutAbsolute)
{
    EAReadBarrier();    // force mStatus to be pulled from main memory

    if (!(timeoutAbsolute == EA::Thread::kTimeoutImmediate))
    {
        // Check that an operation has actually been started.
        EA_ASSERT_MSG(mStatus != OPSTATUS_NEW, "No operation started yet");
        EA_ASSERT_MSG(mDevice != NULL, "No device is associated with this operation\n");

        // If the device is virtual, it will actually be on the NULL
        // devices' queue.  The only time this can happen is if the
        // operation is an open and the file path specified a virtual
        // device name.  Any other operation would be on the parent
        // files' physical device queue
        if (mDevice->IsRegistered())
        {
            Device *device = !mDevice->IsVirtual() ?
                mDevice : Manager::GetInstance()->GetNullDevice();

            device->Wait(this, timeoutAbsolute);
        }
        else
        {
            mStatus = OPSTATUS_FAILED;
            EAWriteBarrier();
        }
    }

    EAReadBarrier();    // force mStatus to be pulled from main memory
    return mStatus;
}

AsyncOp::OpStatus AsyncOp::GetStatusImmediate()
{
    return GetStatus(EA::Thread::kTimeoutImmediate);
}

AsyncOp::OpStatus AsyncOp::GetStatusWait()
{
    return GetStatus(EA::Thread::kTimeoutNone);
}

/*!
    Get the result of an Open function.  The result remains
    valid until the next operation is started.  This function
    blocks until the operation completes.  Use
    AsyncOp::GetStatus if you want to poll for operation
    completion.

    This function is NOT re-entrant. DO NOT CALL FROM MULTIPLE
    THREADS without synchronization or it will deadlock.

    \importlib rwfilesystem

    \return A handle to the file, if the file was found.  This must
    be closed with AsyncOp::Close when it is no longer needed.  If
    the file could not be opened, NULL is returned.

    \see AsyncOp::Open
    \see AsyncOp::GetStatus
    \see AsyncOp::GetResultSize
*/
Handle *    AsyncOp::GetResultHandle() const
{
    EA_ASSERT_MSG(mDevice != NULL, "Internal FileSys Error");

    if (mDevice->IsRegistered())
    {
        Device *device = !mDevice->IsVirtual() ?
            mDevice : Manager::GetInstance()->GetNullDevice();

        device->Wait(this, EA::Thread::kTimeoutNone);
    }

    return mFileHandle;
}

/*!
    Get the result of an operation that returns a size.  The
    result remains valid until the next operation is started.
    This function blocks until the operation completes.  Use
    AsyncOp::GetStatus if you want to poll for operation
    completion.

    This function is NOT re-entrant. DO NOT CALL FROM MULTIPLE
    THREADS without synchronization or it will deadlock.

    \importlib rwfilesystem

    \return The actual number of bytes read or written.  This
    may be fewer than were asked for, if the End-Of-File was
    found or if the disk was full.

    \see AsyncOp::Read
    \see AsyncOp::Write
    \see AsyncOp::GetStatus
    \see AsyncOp::GetResultHandle
*/
uint64_t    AsyncOp::GetResultSize() const
{
    if (mDevice->IsRegistered())
    {
        Device *device = !mDevice->IsVirtual() ?
            mDevice : Manager::GetInstance()->GetNullDevice();

        device->Wait(this, EA::Thread::kTimeoutNone);
    }

    return mTotalBytes;
}

AsyncOp::AsyncOp(const AsyncOp & other)
{
    if (this != &other)
    {
        *this = other;
    }
}

AsyncOp & AsyncOp::operator=(const AsyncOp & other)
{
    if (this != &other)
    {
        mStatus = other.mStatus;
        mIsRead = other.mIsRead;
        mPriority = other.mPriority;
        mFileHandle = other.mFileHandle;
        mContext = other.mContext;
        mCallbackFunc = other.mCallbackFunc;
        mFlags = other.mFlags;
        mOffset = other.mOffset;
        mData = other.mData;
        mTotalBytes = other.mTotalBytes;
        mDevice = other.mDevice;
        mExecFunc = other.mExecFunc;
        mReadProcessContext = other.mReadProcessContext;
        mReadProcessDevice = other.mReadProcessDevice;
        mTrackDeviceIdleTime = other.mTrackDeviceIdleTime;
        memcpy(mFilename, other.mFilename, FILESYS_MAX_PATH);
    }
    return *this;
}

}}}
