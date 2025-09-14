// (c) Electronic Arts. All Rights Reserved.
/**
\file
File Copier, for copying files from one place to another while allowing to abort the copy.
*/

#ifndef RW_CORE_FILESYS_FILECOPIER_H
#define RW_CORE_FILESYS_FILECOPIER_H

#include "EABase/eabase.h"
#include "rw/core/filesys/config.h"
#include "eathread/eathread_thread.h"
#include "eathread/eathread_semaphore.h"
#include "coreallocator/icoreallocator_interface.h"

namespace rw {
namespace core {
namespace filesys {

class Device;

/**
    \brief The set of errors that must be handled by the error callback.
*/
enum FileCopierError
{
    /// Could not open the src filename
    FileCopierError_MissingFile,

    /// The HDD is full, cannot copy any more data
    FileCopierError_NotEnoughSpace,

    /// The file name is invalid, most likely because it is too long
    FileCopierError_InvalidFileName,
};


/**
    \brief The error handler that is called in case of an error
*/
typedef void (*FileCopierErrorCallback)(const char *srcFilename, const char *dstFilename, FileCopierError error );


/**
    \brief The test method that is called in which the user should determine whether or not to copy the file.
            If this function returns true the copier will copy the file (that also implies overwriting it even
            though it may exist. If this function returns false the copier will not copy the file.
*/
typedef bool (*FileCopierTestCallback)(const char *srcFilename, const char *dstFilename);


/**
    \brief This method is called after each block of data is copied.
            The currentBytesCopied contains the amount of bytes written in this block.
            The totalBytesCopied contains the amount of bytes written for this Copy operation (since you called Copy)
*/
typedef void (*FileCopierUpdateCallback)(const char *srcFilename, const char *dstFilename, uint64_t currentBytesCopied, uint64_t totalBytesCopied );


/**
    \brief The method that is called when a file is successfully copied.
*/
typedef void (*FileCopierFileCallback)(const char *srcFilename, const char *dstFilename);


/**
    \brief The method that is called when the current Copy operation (since you called Copy) is completed.
*/
typedef void (*FileCopierDoneCallback)(void);


/**
    \brief The FileCopierOptions allows you to configure the FileCopier upon creation.
*/
struct RWCORELIB_API FileCopierOptions
{
    FileCopierOptions()
        : srcIdleTime(0)
        , dstIdleTime(0)
        , copyPriority( -256 )
        , threadPriority( EA::Thread::kThreadPriorityMin )
        , bufferSize( 64 * 1024 )
        , errorCallback( ErrorDefaultCallback )
        , testCallback( TestDefaultCallback )
        , updateCallback( UpdateDefaultCallback )
        , fileCallback( FileDefaultCallback )
        , doneCallback( DoneDefaultCallback )
    {
    }

    unsigned int srcIdleTime;                   ///< The time the src device has to be idle in (ms) before reading from it
    unsigned int dstIdleTime;                   ///< The time the dst device has to be idle in (ms) before writing to it
    int copyPriority;                           ///< The RwFileSystem priority of async ops
    int threadPriority;                         ///< The priority of the background copy thread
    size_t bufferSize;                          ///< The buffer size for the copy op, this is the amount of data read / written in a single block
    FileCopierErrorCallback errorCallback;      ///< \see FileCopierErrorCallback
    FileCopierTestCallback testCallback;        ///< \see FileCopierTestCallback
    FileCopierUpdateCallback updateCallback;    ///< \see FileCopierUpdateCallback
    FileCopierFileCallback fileCallback;        ///< \see FileCopierFileCallback
    FileCopierDoneCallback doneCallback;        ///< \see FileCopierDoneCallback

    // Default error callbacks
    static void ErrorDefaultCallback(const char *srcFilename, const char *dstFilename, rw::core::filesys::FileCopierError error );
    static bool TestDefaultCallback(const char *srcFilename, const char *dstFilename);
    static void UpdateDefaultCallback(const char *srcFilename, const char *dstFilename, uint64_t currentBytesCopied, uint64_t totalBytesCopied );
    static void FileDefaultCallback(const char *srcFilename, const char *dstFilename);
    static void DoneDefaultCallback();
};

/**
    \brief The FileCopier allows you to copy files from source to destination.

    It also allows you to interrupt the file copy, because the actual copy
    process is running in a thread. The FileCopier is used by the incremental
    installer part of RwFileSystem to copy the files to the console's harddisk.

    \importlib rwfilesystem
*/
class RWCORELIB_API FileCopier
{
public:

    static void *operator new(size_t size);
    static void *operator new(size_t, void *ptr) { return ptr; }
    static void operator delete(void *p);

    FileCopier(const FileCopierOptions &options);
    ~FileCopier();

    /**
        \brief Start a copy operation.

        This method will return false when a copy operation is already in progress.
        The copy operation will be started on another thread and this method will
        return immediately.

        \param srcFilenames Similar to the argv argument to the main function,
        this is a NULL-terminated list of files to copy to the dstPath directory.
        Cannot be NULL.  The array doesn't have a size because it's very easy
        to get the size field of sync with the actual number of entries in the
        array.  By NULL-terminating the array, the list can expand or contract
        without having to keep the size correct.
        \param srcPath Base directory of where the source files are to located.
        May be NULL.
        \param dstPath Location where the files are to be copied into, typically
        this is a cache drive.  Cannot be NULL.
        \sa Wait, Stop, Start
    */
    void Copy( const char **srcFilenames, const char *srcPath, const char *dstPath );

    /**
        \brief Block the current thread until the copy operation is done.
        \sa Copy, Stop, Start
    */
    void Wait();

    /**
        \brief Restart the copying.
        \sa Stop
    */
    void Start();

    /**
        \brief Pause the copy thread.  The copy will be suspended after the current
        operation completes.  Resume the copy with Start.
        \sa Start
    */
    void Stop();

    /**
        \brief Terminates the copy thread.  waitForEnd will wait in this function
        until the copy thread is terminated.
        \sa
    */
    void Terminate(bool waitForEnd);

    /**
        \brief Determines if the thread is currently terminating.
    */
    bool IsTerminating() const { return mThreadIsTerminating; }

    /**
        \brief Return the total bytes copied since the last copy operation.

        Note that this number is cleared for each copy operation.
    */
    uint64_t GetCopiedBytes() const { return mCopiedBytes; }

    static void CreateTempFilename(const char *srcFilename, char *dstFilename, size_t dstFilenameSize);

private:

    // Non-copyable
    FileCopier( const FileCopier & );
    const FileCopier& operator=( const FileCopier & );

    static intptr_t ThreadEntry( void *param );
    void Execute();
    void ThreadTerminate(bool waitForEnd = true);

    void WaitForDeviceIdle(const Device * device, unsigned int idleMilliSeconds) const;
    bool Copy( const char *srcFilename, const char *dstFilename );
    inline void BufferSwap()
    { void *tmp = mWriteBuffer; mWriteBuffer = mReadBuffer; mReadBuffer = tmp; }

    FileCopierOptions mOptions;
    void *mReadBuffer;
    void *mWriteBuffer;
    uint64_t mCopiedBytes;
    EA::Thread::Thread mThread;
    EA::Thread::Semaphore mThreadPauseSemaphore;
    EA::Thread::Semaphore mComplete;
    volatile bool mThreadPause;
    volatile bool mThreadTerminate;
    volatile bool mThreadIsTerminating;

    const char **mSrcFilenames;
    const char *mSrcPath;
    const char *mDstPath;
    bool mDoneWithThis;
};


}}} // namespace

#endif
