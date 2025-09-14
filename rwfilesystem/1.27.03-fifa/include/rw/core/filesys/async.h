// (c) Electronic Arts. All Rights Reserved.

/**
\file
Header file of the rw::core::filesys::AsyncOp class.
*/

#ifndef RW_CORE_FILESYS_ASYNC_H
#define RW_CORE_FILESYS_ASYNC_H

#include "EABase/eabase.h"
#include "eathread/eathread.h"
#include "rw/core/filesys/config.h"
#include "rw/core/filesys/manager.h"

// Include this file for backward-compatibility reasons.
#include "rw/core/filesys/constants.h"
#include "EASTL/intrusive_list.h"

#ifdef _DLL
    #pragma warning(push)
    // disable warnings:
    //  - about templated class not being available to clients of this library.
    //  - about non-dll interface class used as base for dll-interface class
    #pragma warning(disable: 4251 4275)
#endif

namespace rw {
namespace core {
namespace filesys {

class AsyncOp;
class Handle;
class Device;

/*!
    \brief Completion callback signature.
    \importlib rwfilesystem
    Callers can register completion callbacks that is called when an operation
    is completed.  You can use this callback to initiate another operation
    using the same AsyncOp object or signal a caller or anything else.  Avoid
    doing expensive processing in the callback as the device execution thread
    does not begin another operation until this callback has completed.
    Also, be careful about mutating the AsyncOp object in the thread that
    initiated the operation if you use a callback because AsyncOp objects are
    not thread-safe.  For example, if you did an AsyncOp::Wait() in the
    calling thread, then started a new operation, the callback from the first
    operation may have the context from the second because the AsyncOp object
    is not copied into the local thread context.  Generally, if you are going
    to schedule back-to-back operations, they will execute more efficiently
    and predictibly if you start the second (and subsequent) one from the
    callback of the first one.
    \param op a reference to the calling AsyncOp that just finished the
    operation in question.
*/
typedef void (AsyncOpCallback)(AsyncOp *op);
/*!
    \brief Schedule a operation for the file system.

    The AsyncOp class is the at the heart of of the file system.
    This class creates file operations for another thread which
    acts on the commands.  This model allows the current thread to
    schedule an operation but not block waiting for it to complete.

    Operations supported by the File System library are Open, Close,
    Read, Write, GetStatus and GetResult.  After an operation is
    scheduled, the caller is responsible for keeping the AsyncOp
    object until the operation completes.  When an operation
    completes, a callback function may be performed.  On the
    callback, the AsyncOp object may be destroyed or a new operation
    scheduled.  This callback is performed asynchronously on the
    device thread which executed the operation.  This thread must
    not be blocked for an extended period of time.  If there is a
    significant amount of processing to be done on the callback,
    create another thread to do that, or do on the main thread
    after polling for completion.

    All operations have a priority associated with them.  When
    two or more operations are pending, the one with the higher
    priority will be executed first.  If two operations have the
    same priority, the one which was added to the queue first will
    be executed first.  This is quite powerful, as it allows
    smooth access to a slow device.
*/
class RWCORELIB_API AsyncOp : public eastl::intrusive_list_node
{
public:

    AsyncOp();
    ~AsyncOp();

    static void *operator new(size_t size, const char *, unsigned int) { return RWFILESYSTEM_ALLOCATE(size); }
    static void *operator new(size_t size) { return RWFILESYSTEM_ALLOCATE(size); }
    static void *operator new(size_t, void *ptr) { return ptr; }
    static void operator delete(void *ptr) { return Manager::Free(ptr); }

    /*!
        \brief Possible AsyncOp states
    */
    enum OpStatus
    {
        OPSTATUS_NEW       = -3, ///< no operation
        OPSTATUS_FAILED    = -2, ///< operation failed
        OPSTATUS_CANCELLED = -1, ///< operation has been cancelled
        OPSTATUS_PENDING   =  0, ///< operation is either pending or executing
        OPSTATUS_COMPLETE  =  1  ///< operation has finished
    };

    /*!
        \brief The absolute minimum priority
    */
    static const int PRIORITY_MIN = -2147483647;

    /*!
        \brief Bitwise combinations of flags which can be passed to AsyncOp::Open.
    */
    enum OpenFlags
    {
        /*! open the file in read-only mode */
        FOPEN_READONLY      = 0x00,

        /*! open the file in read-write mode.  This flag must be used when a
            new file is created or opened with truncation.
        */
        FOPEN_READWRITE     = 0x01,

        /*! create a new file if it does not already exist. If the file already
            exists, the open will fail unless the FOPEN_OVERWRITE and
            FOPEN_READWRITE are also passed, in which case the existing file
            will be cleared to zero bytes.
        */
        FOPEN_CREATE        = 0x02,

        /*! truncate the file if it exists.  FOPEN_READWRITE must also be passed
            or the open will fail.  If the file does not exist, the open will
            fail unless the FOPEN_CREATE flag is also passed.
        */
        FOPEN_OVERWRITE     = 0x04,

        /*! hint to file system that the file will be read sequentially.
            Correct operation is still guaranteed even if the file is
            not read sequentially, but performance may be reduced slightly.
        */
        FOPEN_SEQUENTIAL    = 0x08,

        /*! Maps to FILE_FLAG_NO_BUFFERING on Microsoft-based systems. Refer
            to documentation for CreateFile for information on restrictions
            (and advantages) this flag implies.
            This flag can also be used on Kettle to enforce the 
            SCE_KERNEL_O_FSYNC flag (no write caching)
         */
        FOPEN_NOBUFFERING   = 0x10,

        /*! If the entire file will be read in a single Read, or if done in
            multiple reads, the parts which were previously read will not be
            modified until the entire file has been read. Use of this flag
            allows the file system to avoid the need to allocate a
            temporary sliding window buffer when decompressing refpack-
            compressed files. If the file is not compressed, this flag is
            ignored.
        */
        FOPEN_READWHOLEFILE = 0x20,

        /*! Disused legacy option.  May be removed in a future version of this
            library.
        */
        FOPEN_DRM           = 0x40,

        /*! This flag is used on the PS3 to tell the device driver that the
            file being opened in a script file and must be opened with
            cellFsSdataOpen as required by TRC 072.  The string ".sdat" will
            be appended to the file name by the PS3 driver.  If this flag is
            not specified, the usual cellFsOpen function will be used.  On
            non-PS3 platforms, this flag is ignored.
        */
        FOPEN_SDATA         = 0x80
    };

    void Open(
        const char *FilePath,
        unsigned int modeflags = FOPEN_READONLY,
        AsyncOpCallback *OpDoneCallback = NULL,
        void *context = NULL,
        int priority = 0);

    void Close(
        Handle *FileHandle,
        AsyncOpCallback *OpDoneCallback = NULL,
        void *context = NULL,
        int priority = 0);

    void Read(
        Handle *FileHandle,
        void * buffer,
        uint64_t offset,
        uint64_t bytes,
        AsyncOpCallback *OpDoneCallback =  NULL,
        void *context = NULL,
        int priority = 0);

    void Write(
        Handle *FileHandle,
        const void *buffer,
        uint64_t offset,
        uint64_t bytes,
        AsyncOpCallback *OpDoneCallback = NULL,
        void *context = NULL,
        int priority = 0);

    void Flush(
        Handle *FileHandle,
        AsyncOpCallback *OpDoneCallback = NULL,
        void *context = NULL,
        int priority = 0);

    void        SetPriority(int priority);

    /*!
        GetPriority returns the operation's current priority.
        \importlib rwfilesystem
        \see SetPriority
    */
    int         GetPriority() const
    {
        return mPriority;
    }

    void        Cancel();

    OpStatus    GetStatus(const EA::Thread::ThreadTime& timeoutAbsolute = EA::Thread::kTimeoutImmediate);
    OpStatus    GetStatusImmediate();
    OpStatus    GetStatusWait();

    Handle *    GetResultHandle() const;

    uint64_t    GetResultSize() const;

    /*!
        \brief Get the context pointer associated with the operation
    */
    void *      GetContext() const
    {
        return mContext;
    }

    /*!
        \brief Blocks the calling thread until the scheduled operation is complete.
        \importlib rwfilesystem
    */
    void        Wait()
    {
        GetStatus(EA::Thread::kTimeoutNone);
    }

    bool GetTrackDeviceIdleTime() const { return mTrackDeviceIdleTime; }
    void SetTrackDeviceIdleTime(bool enabled) { mTrackDeviceIdleTime = enabled; }

    explicit AsyncOp(const AsyncOp & other);
    AsyncOp & operator=(const AsyncOp & other);

private:
    static OpStatus DoOpen(AsyncOp *op);
    static OpStatus DoClose(AsyncOp *op);
	static OpStatus DoFlush(AsyncOp *op);
    static OpStatus DoRead(AsyncOp *op);
    static OpStatus ProcessRead(AsyncOp *op);
    static OpStatus DoWrite(AsyncOp *op);

    static unsigned int GetNumberOfBytesToRead(AsyncOp *op);

    // member variables.
    OpStatus volatile   mStatus;
    bool volatile       mCancelled;             // Flag used to signify when we want to cancel an AsyncOp
    bool                mIsRead;                // Is this a read operation?  This is used by the device object if the schedule policy is READ
    volatile int        mPriority;              // Operation priority.  Higher values indicate operations which should be performed first.
    Handle *            mFileHandle;            // File Handle associated with this operation
    void *              mContext;               // User context pointer for the callback
    AsyncOpCallback *   mCallbackFunc;          // User callback function to execute when the operation completes.
    unsigned int        mFlags;                 // File flags
    uint64_t            mOffset;                // File Offset
    uint64_t            mAmount;                // amount of data to read/write
    void *              mData;                  // Data buffer
    uint64_t            mTotalBytes;            // Total number of bytes read on a split read.
    Device *            mDevice;                // Device this operation is attached to
    OpStatus (*mExecFunc)(AsyncOp *);           // Internal callback function for threaded operation.
    void *              mReadProcessContext;    // for passing context information to a parallel processing function
    Device *            mReadProcessDevice;     // device to execute the read processing function on
    bool                mTrackDeviceIdleTime;   // Tracks the device's idle time for this op
    char                mFilename[FILESYS_MAX_PATH];
    // Device class needs read access to mExecFunc, mCallbackFunc,
    // write access to mStatus
    friend class Device;

    // Needs to access mStatus
    friend OpStatus InsertOpWithStatusUpdate(AsyncOp* op);
};

RWCORELIB_API bool            Exists(const char *FilePath);
RWCORELIB_API uint64_t        GetSize(const char *FilePath);
RWCORELIB_API bool            GetLocationAndSize(const char *FilePath, uint64_t *location, uint64_t *size);
RWCORELIB_API uint64_t        GetSize(const Handle *FileHandle);
RWCORELIB_API bool            Resize(Handle *FileHandle, uint64_t size);
RWCORELIB_API uint64_t        GetLocation(const Handle *FileHandle);
RWCORELIB_API bool            GetNameFromHandle(const Handle *FileHandle, char *name, size_t nameBufferSize);
RWCORELIB_API DeviceDriver *  GetDriver(const Handle *FileHandle);

#if defined(EA_PLATFORM_PS3)
RWCORELIB_API char *ConvertHostNameToNativeName(char *HostNameBuf, const char *RwFileName);
#endif

}}}

#ifdef _DLL
    #pragma warning(pop)
#endif

#endif
