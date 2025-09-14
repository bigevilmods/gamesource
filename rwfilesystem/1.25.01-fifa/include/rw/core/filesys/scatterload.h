// (c) Electronic Arts. All Rights Reserved.

/**
\file
Header file of rw::core::filesys::ScatterLoadFile.
*/

#ifndef RW_CORE_FILESYS_SCATTERLOAD_H
#define RW_CORE_FILESYS_SCATTERLOAD_H

#include "EABase/eabase.h"
#include "eathread/eathread_semaphore.h"
#include "rw/core/filesys/manager.h"
#include "rw/core/filesys/async.h"

// This disables a warning that will come up in dll-builds since this class contains
// templated member objects that cannot be properly exported.  This should not pose a 
// problem however, because there are no public methods which allow client code to
// touch this objects.  However, as a precaution, please be sure to use the same 
// compiler settings when compiling the dll as when compiling the application that
// uses it.
// TODO: change the interface to this method to avoid exposing the templated object.
#if defined(EA_COMPILER_MSVC)
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

namespace rw {
namespace core {
namespace filesys {

class ScatterLoadFile;
struct ScatterLoadFilePrivate;

/*!
    \brief This callback function is executed when a file load completes.

    The callback is not executed for files which were not found.
    Note that callback functions should be kept fairly short.  If
    you are doing any significant processing at all, use the
    callback to signal a worker thread and allow the callback to
    return so the next load can begin as soon as possible.  The
    idea is to achieve maximum concurrency between the file system
    and the CPU processing the loaded data.  As much as possible,
    the game should try to keep both the CPU and the disc busy
    during transitions.  So while the CPU is processing a file, the
    next file is being loaded in.
*/
typedef void (ScatterLoadFileCallback)( ScatterLoadFile *file, void *context);

/*!
    \brief  A ScatterLoadFile represents a physical file to be loaded by a ScatterLoad object.
            Each file can have a callback which is executed when the file has finished loading.
    \see ScatterLoad
*/
class RWCORELIB_API ScatterLoadFile
{
public:
    ScatterLoadFile()
        : mFilePath(NULL),mCallback(NULL),mContext(NULL),
        mAddr(NULL),mFileSize(0),mDoneState(false),mModeFlags(0),
        mAlign(RW_CORE_FILESYS_READ_BUFFER_ALIGNMENT)
    {}

    /**
    \brief Constructor with file path (system allocates memory)
    \param filePath path passed to rw::core::filesys::AsyncOp::Open
    */
    explicit ScatterLoadFile(const char * filePath) :
        mFilePath(filePath),mCallback(NULL),mContext(NULL),
        mAddr(NULL),mFileSize(0),mDoneState(false),
        mModeFlags(0), mAlign(RW_CORE_FILESYS_READ_BUFFER_ALIGNMENT)
    {}

    /**
    \brief Constructor with file path and modeflags (system allocates memory)
    \param filePath path passed to rw::core::filesys::AsyncOp::Open
    \param modeflags modeflags passed to rw::core::filesys::AsyncOp::Open
    */
    ScatterLoadFile(const char * filePath, unsigned int modeflags) :
        mFilePath(filePath),mCallback(NULL),mContext(NULL),
        mAddr(NULL),mFileSize(0),mDoneState(false),
        mModeFlags(modeflags), mAlign(RW_CORE_FILESYS_READ_BUFFER_ALIGNMENT)
    {}

    /**
    \brief Constructor with file path, alignment and modeflags (system allocates memory)
    \param filePath path passed to rw::core::filesys::AsyncOp::Open
    \param bufAlign alignment to allocate file buffer on (memory allocated by ScatterLoad)
    \param modeflags modeflags passed to rw::core::filesys::AsyncOp::Open
    */
    ScatterLoadFile(const char * filePath, unsigned int bufAlign, unsigned int modeflags) :
        mFilePath(filePath),mCallback(NULL),mContext(NULL),
        mAddr(NULL),mFileSize(0),mDoneState(false),
        mModeFlags(modeflags), mAlign(bufAlign)
    {}

    /**
    \brief Constructor with file path, buffer, and buffer size
    \param filePath path passed to rw::core::filesys::AsyncOp::Open
    \param addr buffer to load the file at (pre-allocated buffer)
    \param buffersize size of buffer
    \param modeflags modeflags passed to rw::core::filesys::AsyncOp::Open
    */
    ScatterLoadFile(const char *filePath, void *addr,
        uint64_t buffersize, unsigned int modeflags) :
        mFilePath(filePath),mCallback(NULL),mContext(NULL),
        mAddr(addr),mFileSize(buffersize),mDoneState(false),
        mModeFlags(modeflags), mAlign(RW_CORE_FILESYS_READ_BUFFER_ALIGNMENT)
    {}

    /*!
        \brief Gets the file path
    */
    const char* GetFilePath() const
    { return mFilePath; }

    /*!
        \brief Gets the file open mode flags
    */
    unsigned int GetFlags() const
    { return mModeFlags; }

    /*!
        \brief Gets the alignment for a dynamically-allocated buffer
    */
    unsigned int GetAlignment() const
    { return mAlign; }

    /*!
        \brief Gets the pointer to the callback
    */
    ScatterLoadFileCallback* GetCallback() const
    { return mCallback; }

    /*!
        \brief Gets the pointer to the context
    */
    void* GetContext() const
    { return mContext; }

    /*!
        \brief Gets the pointer to the specified or allocated address
    */
    void* GetAddress() const
    { return mAddr; }

    /*!
        \brief Gets the file size
    */
    uint64_t GetFileSize() const
    { return mFileSize; }

    /*!
        \brief States whether this operation has been completed or not
    */
    bool AreAllLoadsComplete() const
    { return mDoneState; }

    /*!
        \brief Sets the filename and path to the file that is to be loaded
    */
    void SetFilePath(const char* path)
    { mFilePath = path; }

    /*!
        \brief Sets the file open mode flags
    */
    void SetFlags(unsigned int flags)
    { mModeFlags = flags; }

    /*!
        \brief Sets the alignment for a dynamically-allocated buffer
    */
    void SetAlignment(unsigned int align)
    { mAlign = align; }

    /*!
        \brief Sets the callback pointer

        Each file can have a callback which is executed when the file has
        finished loading.  The callback function will be called from a
        secondary thread and hence must be reentrant.  It should
        also complete processing quickly and return because the next file
        will not start to load until the callback from the current
        one completes.  If you need to perform some non-trivial processing,
        for example data decompression, create another thread to do that
        processing and use the callback to set a condition variable on which
        that thread is waiting.  The file buffer may be freed in this callback,
        or any time thereafer.
    */
    void SetCallback(ScatterLoadFileCallback * callback)
    { mCallback = callback; }

    /*!
        \brief Sets the context pointer for the callback function
    */
    void SetContext(void* context)
    { mContext = context; }

    /*!
        \brief Sets the address for the file to be loaded at.  If you want the
        ScatterLoad object to allocate the buffer for you, leave this set to NULL.
        \param address address of memory where file should be written to
    */
    void SetAddress(void* address)
    { mAddr = address; }

    /*!
        \brief Sets the buffer size.  f you want the ScatterLoad object to
        allocate the buffer for you, leave this set to 0.
        \param fileSize size of memory buffer set with SetAddress
    */
    void SetFileSize(uint64_t fileSize)
    { mFileSize = fileSize; }

    /*!
        \brief Sets the done state
   */
    void SetDone()
    { mDoneState = true; }

private:
    const char *                mFilePath;  // Path passed to rw::core::filesys::AsyncOp::Open
    ScatterLoadFileCallback *   mCallback;  // Callback function executed when the file has finished loading
    void *                      mContext;   // Context parameter for callback function

    // Buffer to load the file at.  If this is NULL, the allocator
    // supplied to the ScatterLoadFile object will be used to
    // allocate the buffer for the file
    void *                      mAddr;

    // If mAddr is non-NULL, this is the buffer size.  When the file
    // has finished loading, this contains the number of bytes read.
    uint64_t                    mFileSize;

    bool                        mDoneState; // true if the file has finished loading; false if not
    unsigned int                mModeFlags; // mode flags passed to rw::core::filesys::AsyncOp::Open
    unsigned int                mAlign;     // Buffer alignment to load the file on.
};

/*!
\brief A ScatterLoad object is a list of one or more files to load.

Files are loaded in the order they are physically located on
the disc, not the order they're specified in the array.
*/
class RWCORELIB_API ScatterLoad
{
public:

    /*!
        \brief Possible flags for the ScatterLoad constructor
    */
    enum ScatterLoadFlags
    {
        /// When the buffers for these files are allocated, allocate an extra
        /// rw::core::filesys::ScatterLoad::OVERALLOCATION_SIZE for each one.
        /// Doing this will allow the file to be decompressed in-place, thereby
        /// decreasing the memory highwater mark.
        /// \sa rw::core::filesys::ScatterLoad::OVERALLOCATION_SIZE
        FLAG_USE_OVERALLOCATION_SIZE = 0x01,

        /// This flag tells the loader to sort the loads by offset, rather than
        /// performing them in the order they are passed in.  If several files are
        /// being loaded, sorting them into the same order that they are
        /// physically located on the media can help to reduce seek times.  This
        /// flag may be beneficial when you know the files to be loaded are all
        /// coming from a device which has an open function that does not touch
        /// the disk (such as a bigfile).  In such cases, the sorting is very
        /// cheap compared to the cost of the back-and-forth seeking that occurs
        /// when reads are not done in physical order.  But if the files are being
        /// loaded from the the media file system, this flag will significantly
        /// increase the startup time and has no benefit since neither the Xbox 360
        /// nor the PS3 currently support a mechanism for sorting reads by physical
        /// location.
        FLAG_SORT_FILES_BY_LOCATION = 0x02
    };

    /*!
        \brief The number of extra bytes that are allocated for each file by the
        ScatterLoader when the
        rw::core::filesys::ScatterLoad::FLAG_USE_OVERALLOCATION_SIZE flag is set.

        This is user-configurable parameter, although it is a compile-time constant.
        A default value of 16 bytes has been chosen because that is almost always
        enough to allow the file to be decompressed in-place.  If you use the glump to
        compress your data, you should use the -failoverlap\<n\> option to ensure
        this over allocation size is adequate for all files.
    */
    static const int OVERALLOCATION_SIZE  = 16;

    ScatterLoad(ScatterLoadFile *filelist, unsigned int listsize,
        unsigned int flags = 0, EA::Allocator::ICoreAllocator *pAllocator = NULL,
        unsigned int maxOps = 1);
    ~ScatterLoad();

    static void *operator new(size_t size) { return RWFILESYSTEM_ALLOCATE(size); }
    static void *operator new(size_t, void *ptr) { return ptr; } 
    static void operator delete(void *ptr) { return Manager::Free(ptr); }

    /*!
        Possible ScatterLoadFile states
    */
    enum OpStatus
    {
        OPSTATUS_PENDING,   ///< operation is either pending or executing
        OPSTATUS_COMPLETE   ///< operation has finished
    };

    OpStatus GetStatus(const EA::Thread::ThreadTime& timeoutAbsolute = EA::Thread::kTimeoutImmediate);

private:

    static void OpDoneCallback(AsyncOp *op);

    // Possible loading states
    enum ScatterLoaderState
    {
        ALS_OPEN,
        ALS_READ,
        ALS_CLOSE
    };

    // This structure contains the information for one operation
    // Many operations can be executed concurrently.
    struct OpInfo
    {
        AsyncOp         mOp;            // operation
        Handle *        mFileHandle;    // handle for this file
        ScatterLoadFilePrivate *mFile;  // which file is being operated on
        ScatterLoad *   mLoader;        // pointer to loader so the next operation can be started
        ScatterLoaderState mState;      // state of this operation

        // constructor
        OpInfo(ScatterLoadFilePrivate *file, ScatterLoad *loader) : mOp(),
            mFileHandle(NULL), mFile(file), mLoader(loader), mState(ALS_OPEN) {}

        // default destructor is fine
        ~OpInfo() {}

        // in-place new & delete
        void *operator new(size_t, void *ptr) { return ptr; }
        void operator delete(void *, void *) {}
        void operator delete(void *) {}
    };

    void StartNextFile(OpInfo *);

    bool                mCompleteFlag;  // true when all files have been loaded (or not found)
    unsigned int        mNumFiles;      // total number of files be loaded
    unsigned int        mFlags;         // flags passed to ctor
    OpInfo *            mOpArray;       // array of operations
    unsigned int        mNumOpsUsed;    // number of operations actually used
    EA::Thread::AtomicUint32 mCurr;     // index of the current file being loaded
    ScatterLoadFilePrivate *mAipList;   // list of files which are to be loaded.
    EA::Thread::Semaphore   mComplete;  // semaphore for signalling the completion of all loads
    EA::Allocator::ICoreAllocator *mAllocator;  // allocator for allocating file buffers

    // Objects of this type cannot be copied or assigned.
    ScatterLoad(const ScatterLoad & );
    ScatterLoad& operator=(const ScatterLoad &);
};

}}}

#if defined(EA_COMPILER_MSVC)
#pragma warning(pop)
#endif

#endif
