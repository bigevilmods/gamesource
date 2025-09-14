// (c) Electronic Arts. All Rights Reserved.

/**
\file
Header file of rw::core::filesys::Manager.
*/

#ifndef RW_CORE_FILESYS_MANAGER_H
#define RW_CORE_FILESYS_MANAGER_H

#include "EABase/eabase.h"
#include "coreallocator/icoreallocator_interface.h"
#include "EASTL/intrusive_list.h"
#include "eathread/eathread_thread.h"
#include "eathread/eathread_futex.h"

#include "rw/core/filesys/config.h"
#include "rw/core/filesys/constants.h"
#include "rw/core/filesys/profiler.h"

/*!
    \brief Required alignment for file system buffers.

    This is a platform-specific value that is the recommended read buffer
    alignment for efficient data transfer on that platform.  If the
    file system ever allocates a buffer internally for reading data into,
    it will be aligned to this offset.
*/
#if defined(EA_PLATFORM_CAFE)
#define RW_CORE_FILESYS_READ_BUFFER_ALIGNMENT 64
#elif defined(EA_PLATFORM_PS3)
#define RW_CORE_FILESYS_READ_BUFFER_ALIGNMENT 16
#else
#define RW_CORE_FILESYS_READ_BUFFER_ALIGNMENT 4
#endif


namespace rw {
namespace core {
namespace filesys {

class AsyncOp;
class Device;
class Handle;
class DeviceDriver;

/*!
    \brief AsyncOpOpen callback signature.
    \importlib rwfilesystem
    Users can register an AsyncOpOpen callback that
    will get called when an AsyncOp::Open is called.
    \param op a reference to the calling AsyncOp that is about to schedule
            the operation in question.
    \param fileName Name of the file to be opened. This file name can be changed as long as the new filename is FILESYS_MAX_PATH characters long or shorter, including null termination.
    \param modeflags A pointer to the mode that the file will be opened with. These can be modified by the callback.
*/
typedef void (*AsyncOpOpenCallback)(AsyncOp *op, char fileName[FILESYS_MAX_PATH], unsigned int *modeflags);

/*!
    \brief The options structure is a set of options
    for the creation of the file system Manager object.

    Putting the Initialization options in a separate structure allows us
    to add more options in future revisions without requiring
    users to update their code.
    \importlib rwfilesystem
    \sa Manager
*/

struct RWCORELIB_API Options
{
    /// \brief Constructor that takes an allocator.  
    /// The two unnamed parameters are deprecated maxSearchLocations and maxSearchPathLength
    /// The default thread priority is set to default priority + 2.
    Options(EA::Allocator::ICoreAllocator* allocator);

    Options(EA::Allocator::ICoreAllocator* allocator, unsigned int, unsigned int);


    /// The allocator instance the library should use for its allocations.
    /// Nearly all of the blocks allocated by this allocator will be
    /// small.  The allocator implementation <b>must</b> be thread-safe
    EA::Allocator::ICoreAllocator*  mAllocator;

    /*! If you want to modify the file system thread creation disposition,
        you can set it here.  Note that it is also possible to modify the
        creation disposition parameters after the file system manager is
        created by using Manager::SetThreadParameters.
    */
    EA::Thread::ThreadParameters    mThreadParams;
};

/*!
    \brief Manager is the controller class for the file system library.
    It is a singleton class, and must be instantiated with
    Manager::CreateInstance before any other file system library
    calls are made.
    \importlib rwfilesystem
*/

#ifdef RWFILESYSTEM_ALLOCATE
#undef RWFILESYSTEM_ALLOCATE
#endif

#ifdef EA_DEBUG
    #define RWFILESYSTEM_STRINGIFY(x) RWFILESYSTEM_STRINGIFY_HELPER(x)
    #define RWFILESYSTEM_STRINGIFY_HELPER(x) #x
    #define RWFILESYSTEM_ALLOCATE(size) rw::core::filesys::Manager::Allocate(size, __FILE__ "(" RWFILESYSTEM_STRINGIFY(__LINE__) ")")
#else
    #define RWFILESYSTEM_ALLOCATE(size) rw::core::filesys::Manager::Allocate(size, "rw::core::filesys::Manager::Allocate")
#endif

class RWCORELIB_API Manager
{
public:
    typedef bool (*ErrorHandlerFunction)(int, unsigned int, DeviceDriver *, const char *);
    typedef bool (*ErrorInjectionFunction)(DeviceDriver *, const char *);

    static Manager *CreateInstance(const Options &options);
    static Manager *GetInstance();
    static void DestroyInstance();

    static bool IsInitialized();

    static void* Allocate(size_t size, const char *fileAndLine);
    static void* Allocate(size_t size);
    static void Free(void* ptr);

    /*! \brief Set the thread parameters for file system device threads.
        Any threads that have already been created won't be affected, so
        this value should be set right after the manager is created if you
        want to change it.  The file system manager leaves this object as
        it was default-constructed, except the priority is changed to
        EA::Thread::kThreadPriorityDefault + 2, meaning file system-created
        threads will execute at higher-than-normal priority.
        \param tp New file system device threads are created with this
        disposition when they are created.
        \sa GetThreadParameters Options::mThreadParams
    */
    void SetThreadParameters(const EA::Thread::ThreadParameters &tp)
    { mThreadParams = tp; }

    /*! \brief Get the file system manager thread processor value.
        \sa SetThreadParameters
    */
    const EA::Thread::ThreadParameters &GetThreadParameters() const
    { return mThreadParams; }

    /*! \brief Get the thread for the device for the given file
        path.

        The file system thread priority can be modified using this
        thread object.

        \param FilePath Path corresponding to the device you want
        to get the thread object for.
        \return The device thread object.
    */
    EA::Thread::Thread *GetThread(const char *FilePath) const;

    /// \brief Type of operation that was executing when the error
    /// occurred.
    enum OpError
    {
        FILEERROR_SUCCESS,  ///< the operation has completed successfully
        FILEERROR_OPEN,     ///< Open operation failure
        FILEERROR_CLOSE,    ///< Close operation failure
        FILEERROR_READ,     ///< Read operation failure
        FILEERROR_WRITE,    ///< Write operation failure
        FILEERROR_SEEK,     ///< Seek operation failure
        FILEERROR_OPENDIR   ///< OpenDirectory opertation failure
    };

    /// \internal
    ErrorHandlerFunction GetErrorHandler() const
    { return mErrorHandler; }

    /*! \brief Set a file system error handler function.

        This handler is called whenever there is an error code returned
        by the platform operating system.  So it may be called for an open
        file failure, or a read failure, or a write failure.  Your handler
        function can tell the file system library to retry the operation or
        simply return failure (which usually causes the application to
        enter into an undefined state).

        The Xbox 360 TCR requires games to notify the user when a
        dirty or damaged disc cannot be read.  If set, when a call
        to a OS file system functions returns failure, the
        function pointed to by ReadFileFailCallback will be
        called.  Note that all access to the file system is
        done from a secondary thread, so if the read fails, your
        error handler function will also be called from a secondary
        thread.

        On Xbox 360, you must install your own error handler; there is
        no valid default handler.  Failing to do so is a TCR violation.

        On PlayStation3, a TRC-compliant handler is built into the
        library, so it is not necessary to create your own.  However,
        the default version simply keeps retrying the operation
        indefinitely, so the game player would not know why the system
        has hung. Informing the game player of disc read problem is
        considered good practise, even if doing so is not required by
        the PS3 TRC.  Be aware that if you do create your own handler,
        it must call cellSysutilCheckCallback, to check for the PS button
        press.

        This function is present in the Windows version strictly
        for portability purposes, although ReadFile() can fail on
        the Windows platform, there would be little action the game
        player could take to deal with the problem, and they would
        very likely be having hard disk failures during other
        operations.

        \param handler Callback function pointer notifying the caller of file
        system failures.  The handler function is passed four parameters by the
        file system library: a platform-dependent error code, the type of operation
        that was being executed when the error occurred, the device on which the
        error occurred and name of the file that was being operated on.
        To have the file system retry the operation, return true from your
        handler.  If you want the file system to abort the operation and return
        an error to the code that initiated the operation, your handler function
        should return false.  In the latter case, your application will need to
        deal with the error in some reasonable way.  If an error occurs that
        subsequently succeeds on a retry, the error handler will be called one
        time with a success notification.  On the Xbox360, the error value will
        be one of the system error codes.  On the PlayStation 3 the error value
        will be the return code from the cellFs function that was called.
    */

    void            SetErrorHandler(ErrorHandlerFunction handler)
    { mErrorHandler = handler; }

    /*!
        \brief Possible flags for RegisterDevice
    */
    enum DeviceFlags
    {
        DEVICEFLAG_TYPE_PHYSICAL = 0x0,     ///< This device is a physical device.  Physical devices have a their own thread, and are meant for files directly on a file system (local or remote).
        DEVICEFLAG_TYPE_VIRTUAL  = 0x1      ///< This device is a virtual device.  Virtual devices allow you to create a new file system within another file (like a zip file).  Virtual devices do not have their own threads; operations on virtual files are executed on their parent device handle's thread.  You actually do not need to specify this flag when a device is created, but if you omit it, a thread will be created which is never used, which uses up system resources.
    };

    Device *        RegisterDevice(DeviceDriver *DeviceToAdd, unsigned int flags = DEVICEFLAG_TYPE_PHYSICAL);
    bool            UnregisterDevice(Device *DeviceToRemove);
    
    void            SetSearchPath(const char *NewSearchPath);
    size_t          GetSearchPath(char *pathBuffer, size_t pathBufferLength);

    // [DEPRECATED] - Use GetSearchPath() with NULL arguments instead.
    size_t          GetMaxSearchPathLength();

    /*!
        \brief Flags for the addition of new locations to the search path
    */
    enum SearchPathFlags
    {
        SEARCHPATH_ADD_TO_TAIL, ///< Put this search location after existing locations
        SEARCHPATH_ADD_TO_HEAD  ///< Put this search location before the existing locations
    };

    void            AddSearchLocation(const char *NewLocation, unsigned int flags = SEARCHPATH_ADD_TO_TAIL);
    void            InsertSearchLocationBefore(const char *NewLocation, const char *ExistingLocation);
    bool            RemoveSearchLocation(const char *LocationToRemove);

    /*! \brief Set an idle function to be called any time the current thread blocks

        Your idle function is called in a loop while the current thread blocks.
        This allows you to take necessary action until the current thread is
        unblocked.
    */
    static void     SetIdleFunction(EA::Thread::IRunnable *func);

    /// \internal  Idle is called in a loop by the library any time the
    /// current thread blocks on an external event.
    static void     Idle();

    /*! On non-threaded platforms, call Tick() any time you are waiting
        for a file operation to complete so the operation queue is pumped.
    */
    static void     Tick();

    /*! \brief Set a new current directory.

        The working directory is used by some file system function when their
        file path arguments are not fully-qualified.  When the
        filesystem Manager is created this points to the current directory
        on Windows, D: on Xbox 360 and host1: on PlayStation3.  If you choose to
        change this path, please observe the following restrictions:
        - the path may not have a trailing directory separator character on it.
        - the directory must exist; it will not be created for you.

        This path is only used by file system functions that are destructive in
        nature.  This includes filesys::Save, filesys::Delete, filesys::Move,
        filesys::CreateDirectory and filesys::RemoveDirectory.  If a relative
        path is passed to those functions, the current working directory is
        pre-pended to give a fully-qualified path.  The search path (see
        SetSearchPath) is not used.

        The current working directory is not used by non-destructive functions.
        These include AsyncOp::Open and DirectoryEntry::OpenDirectory.  For those
        functions, the search path is used and iterated on until a match is found
        or all the locations have been searched.  If you want your current working
        directory and search path to be consistent, it is necessary to set them
        each with the same location, for example:

        \code
        filesys::Manager::GetInstance()->SetCWD("C:\\Game");
        filesys::Manager::GetInstance()->SetSearchPath("C:\\Game");
        \endcode

        This might seem a little odd, but the current working directory and the
        search path have separate purposes - the search path is designed to
        allow an alternate location to be checked before a fallback location
        (for example you might have cached some data files to the system's hard
        disk), whereas the current working directory is designed to be used for
        destructive functions that need access to a writable device.

        Note that on Windows, this function does <b>not</b> call
        SetCurrentDirectory.

        \param dirpath The new working directory.  The path must be
        fully-qualified with a device specifier (e.g. c:\\MyGame)
        \sa GetCWD, CreateDirectory, SetSearchPath
    */
    void            SetCWD(const char *dirpath);

    /*! \brief Return the current working directory
    */
    const char *    GetCWD() const
    { return mCurrDir; }

    #if defined EA_PLATFORM_IPHONE
    const char *    GetAppDir() const
    { return mAppDir; }
    #endif

    /*!
        \brief Possible read operation schedule policies
    */
    enum ReadSchedulePolicy
    {
        READ_SCHEDULE_POLICY_FCFS,      ///< (default) Read operations of equal priority are serviced in first-come, first-served order.
        READ_SCHEDULE_POLICY_SORTED     ///<  Read operations of equal priority are sorted by physical order instead of FCFS. This policy implements an elevator algorithm, which is usually better than other algorithms at minimizing seek times.  It will only work for read operations of equal priority, so the file(s) must be opened beforehand. Operations of other types (Open/Close) will be scheduled after any pending read operations, and all operations will continue to be sorted first by priority, then by physical order.  Multiple files may be used, but the scheduler will only work properly if the file device supports querying for a files' physical location and the files are contiguous on the disk.  Xbox 360 supports this property for files on fixed media (not the hard drive), and Windows does not allow it on any device.
    };

    void            SetReadSchedulePolicy(ReadSchedulePolicy newPolicy)
    { mReadSchedulePolicy = newPolicy; }

    ReadSchedulePolicy GetReadSchedulePolicy() const
    { return mReadSchedulePolicy; }

    void            SetEventProfiler(FileSysProfiler *, void *context = NULL);

    // Returns the current event profiler (internal function)
    FileSysProfiler *GetEventProfiler() const;

    // Returns the current event profiler context (internal function)
    void *          GetEventProfilerContext() const;

    /*!
        \brief Sets the amount of data to be read in a single call
        to the file system driver

        SetReadQuantumSize controls the number of bytes read from
        the file between queue checks during large reads.  Suppose
        a large file (say 1 MB) is being loaded into memory while
        a stream is playing.  The read is done one quantum at a
        time.  After each quantum is read, the device queue is
        checked to see if there are any higher-priority operations
        which require servicing.  If so, the current read is
        suspended and the new operation is serviced.  Larger read
        quantum values will enable faster reads.  Smaller values
        will allow smoother access to a slow device and help
        prevent stream buffer starvation.  The default value is
        256 kilobytes.  It should be a power-of-two, but that is
        not specifically required.
    */
    void            SetReadQuantumSize(unsigned int size)
    { mReadQuantumSize = size; }

    /*!
        \brief GetReadQuantumSize returns the current read quantum size.
        \return current quantum size in bytes
    */
    unsigned int    GetReadQuantumSize() const
    { return mReadQuantumSize; }

    /*!
        \brief GetAllocator returns the allocator object that the
        filesys module uses to allocate dynamic memory.
    */
    static EA::Allocator::ICoreAllocator *GetAllocator();

    // Returns the current AsyncOpOpenCallback
    AsyncOpOpenCallback GetAsyncOpOpenCallback()  const { return mAsyncOpOpenCallback; }

    // Sets the current AsyncOpOpenCallback
    void SetAsyncOpOpenCallback(AsyncOpOpenCallback callback) { mAsyncOpOpenCallback = callback; }

    //! \internal Get a pointer to the NULL device.
    Device *GetNullDevice() const
    { return mNullDevice; }

    class SearchPathInfo : public eastl::intrusive_list_node
    {
    public:
        SearchPathInfo() : mPath(NULL), mDevice(NULL), mPathLength(0) {}
        SearchPathInfo(const char* path, unsigned int len);
        ~SearchPathInfo();
        
        const char* GetPath() const { return mPath; }
        Device* GetDevice() const { return mDevice; }
        unsigned int GetPathLength() const { return mPathLength; }

        static void *operator new(size_t size);
        static void operator delete(void *p);

        bool operator==(const SearchPathInfo& rhs) const;

    private:
        char *mPath;
        Device *mDevice;
        unsigned int mPathLength;
    };
    typedef eastl::intrusive_list<SearchPathInfo> SearchPathList;

    const SearchPathList& GetSearchPathList() const 
    { return mSearchPathList; }

    //! \internal Get the device list futex.  Note that getting it does not lock it.
    EA::Thread::Futex &GetSearchPathFutex()
    { return mSearchPathFutex; }

    //! \internal Get the device list futex.  Note that getting it does not lock it.
    EA::Thread::Futex &GetDeviceListFutex()
    { return mDeviceListFutex; }

    void SetErrorInjectionFunction(ErrorInjectionFunction errorInjectionFunction);
    ErrorInjectionFunction GetErrorInjectionFunction() const;

private:

    // Prevent default generation of these functions by not defining them
    explicit Manager(const Options &options);
    ~Manager();

    static void *operator new(size_t size);
    static void operator delete(void *p);

    // This class is not copyable
    Manager(const Manager &);
    Manager& operator=(const Manager &);

    // singleton manager instance
    static Manager* sInstance;
    static EA::Allocator::ICoreAllocator* sAllocator;
    static EA::Thread::IRunnable *sIdleFunction;

    #if defined(_MSC_VER)
        #pragma warning(push)
        #pragma warning(disable: 4251)  // class 'type' needs to have dll-interface to be used by clients of class 'type2'
    #endif

    // Parsed search path
    SearchPathList                  mSearchPathList;
    EA::Thread::Futex               mSearchPathFutex;

    // Device list
    friend class Device;
    typedef eastl::intrusive_list<Device> DeviceList;
    DeviceList                      mDeviceList;
    EA::Thread::Futex               mDeviceListFutex;

    #if defined(_MSC_VER)
        #pragma warning(pop)
    #endif

    // Thread parameters for newly-created file system threads.
    EA::Thread::ThreadParameters    mThreadParams;

    // The read quantum size is the number of bytes which are read
    // before the file system queue is checked for higher-priority
    // operations.  Larger values will normally read more quickly
    // with less CPU overhead.  Smaller values will allow smoother
    // coordination to the device with smaller buffers.  Should be
    // a power of two for efficiency reasons.
    unsigned int                    mReadQuantumSize;

    // For Open operations where the device is not specified in the file path.
    Device *                        mNullDevice;

    // The CurrDir device is used when some functions (e.g. OpenDirectory)
    // need a device to operate on because the path is not fully-qualified.
    char                            mCurrDir[FILESYS_MAX_PATH];

    #if defined EA_PLATFORM_IPHONE
    char                            mAppDir[FILESYS_MAX_PATH];
    #endif

    ReadSchedulePolicy              mReadSchedulePolicy;

    // if a file system function fails, this function will
    // be called by the library so the game can take some appropriate
    // action (such as informing the user his disc can't
    // be read and it might be dirty or damaged)
    bool                          (*mErrorHandler)(int, unsigned int, DeviceDriver *, const char *);

    /// internal Simple error handler simply returns true (retry the operation)
    static bool                     DefaultErrorHandler(int, unsigned int, DeviceDriver *, const char *);

    static FileSysProfiler *        sFileSysProfiler;
    static void *                   sFileSysProfilerContext;

    static ErrorInjectionFunction   sFileSysErrorInjectionFunc;
    static bool                     DefaultErrorInjectionFunction(DeviceDriver *, const char *);

    // The AsyncOpOpenCallback that can be called from AsyncOp::Open
    AsyncOpOpenCallback             mAsyncOpOpenCallback;

    void Init();
    void RemoveDeviceFromSearchPath(const Device *device);
    void InternalClearSearchPath();
    bool InternalSetSearchPath(const char *path);
    bool InternalInsertSearchLocation( const char * path, unsigned int pathLen, SearchPathList::iterator pos);
    void PlatformInit();
    void PlatformTeardown();

};

}}}

#endif
