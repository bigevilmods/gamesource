// (c) Electronic Arts. All Rights Reserved.

/**
\file
Implements rw::core::filesys::Manager.
*/

#include "EAAssert/eaassert.h"
#include "eathread/eathread_sync.h"

#include <EAStdC/EAString.h>
#include <EAStdC/EAMemory.h>

#include "private_rwdevice.h"
#if !defined(EA_PLATFORM_AIRPLAY)
#include "private_rwnulldriver.h"
#endif
#include "private_rwprofiler.h"
#include "rw/core/filesys/driverimpl.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "platform_sdk.h"

using namespace EA::StdC;

namespace rw {
namespace core {
namespace filesys {

// singleton instance
//lint -e{956}
static DefaultProfiler sDefaultProfilerInstance;
Manager* Manager::sInstance = NULL;
EA::Allocator::ICoreAllocator*  Manager::sAllocator = NULL;
EA::Thread::IRunnable *Manager::sIdleFunction = NULL;
FileSysProfiler *Manager::sFileSysProfiler = &sDefaultProfilerInstance;
void *Manager::sFileSysProfilerContext = NULL;

const unsigned int kDefaultMaxSearchPathLength = 4096;

Options::Options(EA::Allocator::ICoreAllocator* allocator)
    : mAllocator(allocator)
{
    // increase the priority of threads created by the file system
    // to default + 2.
    mThreadParams.mnPriority += 2;
}

Options::Options(EA::Allocator::ICoreAllocator* allocator, unsigned int , unsigned int )
    : mAllocator(allocator)
{
    mThreadParams.mnPriority += 2;
}


/*!
    Use CreateInstance to create the singleton Manager class
    before using any other file system library APIs.  The
    caller passes a filesys::Options object to
    CreateInstance for any customization they wish to perform.

    After the instance has been created, the Manager's current
    working directory is set to the location of the executable
    on Windows, D:\ on Xbox 360, and host1: on PlayStation3.

    \param options options structure to control how the library is initialized
    \return the newly created manager object
*/
Manager *Manager::CreateInstance(const Options &options)
{
    //[FIFA FB][YC][Reason: Work around an issue where FIFAApp::FirstTimeBootInit() and Engine.Audio both create an instance of rwfilesystem manager.
    if(sInstance!= NULL) return sInstance;
    //END OF FIFA FB CHANGE

	if (sInstance != NULL)
	{
		return sInstance;
	}
	Manager::sAllocator = options.mAllocator;
    sInstance = new Manager(options);
    sInstance->Init();

    return sInstance;
}

#if defined(EA_PLATFORM_WINDOWS)
#include <malloc.h>
namespace
{
    class WinDefaultAllocator : public EA::Allocator::ICoreAllocator
    {
        virtual void *Alloc(size_t size, const char *, unsigned int)
        {
            const size_t default_alignment = 4;
            return _aligned_malloc(size, default_alignment);
        }

        virtual void *Alloc(size_t size, const char *, unsigned int, unsigned int alignment, unsigned int)
        {
            return _aligned_malloc(size, alignment);
        }

        virtual void Free(void *block, size_t)
        {
            _aligned_free(block);
        }
    };
}
#endif  // defined(EA_PLATFORM_WINDOWS)

/*!
    GetInstance returns the singleton file system manager
    object.

    \return Will assert if CreateInstance has not been called,
    otherwise will return the filesys Manager object
*/
Manager *Manager::GetInstance()
{
#if defined(EA_PLATFORM_WINDOWS)

    // permit lazy initialization in Windows builds, to avoid having each DLL
    // need to initialize the file system manager explicitly.  Still not reference-
    // counted, so the caller will still need to call DestroyInstance before
    // program termination if they want to completely release all resources
    // before exiting.
    if (sInstance == NULL)
    {
        static WinDefaultAllocator gWinDefaultAllocator;
        const Options options(&gWinDefaultAllocator);
        filesys::Manager::CreateInstance(options);
    }

#endif

    return sInstance;
}

/*!
    DestroyInstance shuts down the file system and
    releases any resources owned by the library.
*/
void Manager::DestroyInstance()
{
    EA_ASSERT_MSG(sInstance != NULL, "instance does not exist, call Manager::CreateInstance() first.");
    sInstance->SetEventProfiler(NULL, NULL);

    delete sInstance;
    sInstance = NULL;
    sAllocator = NULL;
}


/*!
    Returns whether or not the manager has been initialized
*/
bool  Manager::IsInitialized()
{ 
    return rw::core::filesys::Manager::sInstance != NULL; 
}


void* Manager::Allocate(size_t size, const char *fileAndLine)
{
    return sAllocator->Alloc(
        size,
        fileAndLine,
        EA::Allocator::MEM_TEMP,
#ifdef EA_PLATFORM_PS3
        RW_CORE_FILESYS_READ_BUFFER_ALIGNMENT
#else
        EA_ALIGN_OF(void *)
#endif
       );
}

void* Manager::Allocate(size_t size)
{
    EA_FAIL_MSG("Please use the RWFILESYSTEM_ALLOCATE(x) macro instead of calling this function directly.");
    return RWFILESYSTEM_ALLOCATE(size);
}

void Manager::Free(void* ptr)
{
    return sAllocator->Free(ptr);
}

EA::Thread::Thread *Manager::GetThread(const char *filePath) const
{
    Device *dev = Device::GetInstance(filePath);

    if (dev == NULL)
        return NULL;

    return dev->GetThread();
}

/*!
    RegisterDevice registers a new device type with the filesys
    library.  Each device must have a corresponding driver which
    provides the services for opening and closing files, seeking,
    and reading data,  Note that this device will not be used
    unless a file path is passed to AsyncOp::Open with this device
    specified or it is added to the search locations.

    Devices may be phyisical or virtual.  A physical device is one
    that reads actual file system files.  Most devices are physical
    devices.  Virtual devices are for "files within a file" cases,
    where the physical file contains one or more subfiles.  These subfiles
    can be referred to by client code as if they were physical files.

    Each physical device uses its own thread, which means stack space must
    be reserved for each device.

    Virtual devices do not have their own threads; operations on
    virtual devices are executed on the device thread where the parent
    file handle is opened.  To specify that a device is virtual, pass the
    DEVICEFLAG_TYPE_VIRTUAL flag to this function.  Not passing this flag
    will have no effect on the system operation, other than some
    unnecessary resources being used.

    When the library is initialized, some devices are preregistered.
    On Windows, this includes any mapped drives and on the Xenon, the
    D: drive is registered.

    \return A pointer to the new device.  This pointer can be used to
    unregister the device later on; it has no other use.

    \param drv instance of new device to register.  The user is responsible for destroying this object after it has been Unregistered
    \param flags 0 or one of the DeviceFlags options.

    \sa UnregisterDevice, Manager::CreateInstance,
    Manager::SetSearchPath, Manager::AddSearchLocation,
*/
Device *Manager::RegisterDevice(DeviceDriver *drv, unsigned int flags)
{
    EA::Thread::AutoFutex lock(mDeviceListFutex);

    Device *dev = new Device(drv, flags);

    // TODO: assert that a device with this name is not
    // already on the list.

    // put the device on the bottom of the list.
    Manager::GetInstance()->mDeviceList.push_back(*dev);

    return dev;
}

/*!
    UnregisterDevice is used to remove a device driver from
    the system.  You might want to do this if you add a driver
    which is only needed for a short period of time and you want
    the resources released.

    Be careful not to remove a device on which there are open
    file handles or pending operations.  If you do, you might get
    a resource leak, program hang, or other undefined behaviour.

    If your driver wants to self-terminate (for example, if an
    unreliable communications device disconnects), rather than
    calling UnregisterDevice, the driver should begin returning 0
    from all Read and Write operations and NULL from Open
    requests.  You might want to remove the device from the search
    path too (refer to Manager::RemoveSearchLocation) which should
    help reduce the number of Open requests to the driver.  After
    all the file handles have been closed, the device can be
    safely removed with UnregisterDevice.

    \return true if the device was found and removed.  False if
    device was not found.

    \param dev Device to unregister (value returned from RegisterDevice).  There must be no open handles on this device when it is unregistered.

    \sa RegisterDevice, Manager::DestroyInstance
*/
bool Manager::UnregisterDevice(Device *dev)
{
    /*
    TODO:
    Have three separate device lists. One for active devices,
    one for devices that were flagged to be terminated,
    and one for devices that were terminated.
    This way we won't have to block until the device finishes
    waiting for any active AsyncOp's to be finished.
    */
    EA::Thread::AutoFutex lock(mDeviceListFutex);

    eastl::intrusive_list<Device>::iterator it =
        Manager::GetInstance()->mDeviceList.locate(*dev);

    if (it == Manager::GetInstance()->mDeviceList.end())
        return false;
    
    dev->Unregister();

    // Remove from search path info.
    RemoveDeviceFromSearchPath(dev);

    return true;
}

Manager::SearchPathInfo::SearchPathInfo( const char* path, unsigned int len )
{
    EA_ASSERT(path && len);
    mPath = static_cast<char*>(RWFILESYSTEM_ALLOCATE(len + 1));
    Memcpy(mPath, path, len);
    mPath[len] = '\0';
    mDevice = Device::GetInstance(mPath);
    EA_ASSERT(mDevice != NULL);
    mPathLength = len;
}

Manager::SearchPathInfo::~SearchPathInfo()
{
    if (mPath)
        Free(mPath);
    mPath = NULL;
    mDevice = NULL;
    mPathLength = 0;
}

void * Manager::SearchPathInfo::operator new(size_t size)
{
    EA_ASSERT(size == sizeof(SearchPathInfo));
    return RWFILESYSTEM_ALLOCATE(size);
}

void Manager::SearchPathInfo::operator delete(void *p)
{
    Free(p);
}

bool Manager::SearchPathInfo::operator==( const SearchPathInfo& rhs ) const
{
    if (mPath == NULL)
        return rhs.mPath == NULL;
    else
        return (Strcmp(mPath, rhs.mPath) == 0);
}

/*!
    This function clears and free all of the search path locations
*/
void Manager::InternalClearSearchPath()
{
    EA::Thread::AutoFutex lock(mSearchPathFutex);
    EA_ASSERT(mSearchPathList.validate());
    for (SearchPathList::iterator info = mSearchPathList.begin(); info != mSearchPathList.end(); )
    {
        SearchPathInfo* ptr = info.mpNode;
        info = mSearchPathList.erase(info);
        delete ptr;
    }
}

/*!
    This function does exactly what the public SetSearchPath does
    except that it does not assert when validation conditions
    are not met.
*/
bool Manager::InternalSetSearchPath(const char *path)
{
    using namespace EA::StdC;

    EA_ASSERT(path != NULL);
    InternalClearSearchPath();

    EA::Thread::AutoFutex lock(mSearchPathFutex);

    const char *prevSearchPathPtr = path;
    const char *searchPathPtr = Strchr(path, ';');

    while (searchPathPtr != NULL)
    {
        unsigned int len = static_cast<unsigned int>(searchPathPtr - prevSearchPathPtr);
        if (!InternalInsertSearchLocation(prevSearchPathPtr, len, mSearchPathList.end()))
            return false;
        prevSearchPathPtr = searchPathPtr + 1;
        searchPathPtr = Strchr(prevSearchPathPtr, ';');
    }

    return InternalInsertSearchLocation(prevSearchPathPtr, static_cast<unsigned int>(Strlen(prevSearchPathPtr)), mSearchPathList.end());
}

/*! 
    This function adds a new location into the search path before the location pointed to by the iterator.
    If you pass in the end iterator, it will append the location to the end of the searchpath.
    If you pass in the begin iterator, it will prepend the location to the start of the searchpath.
*/
bool Manager::InternalInsertSearchLocation( const char * loc, unsigned int locLen, SearchPathList::iterator pos)
{
    EA_ASSERT(loc && locLen);
           
    // Remove any trailing / or \ from a non-root path
    if ((locLen > 1) && ((loc[locLen - 1] == '\\') || (loc[locLen - 1] == '/')))
    {
        locLen--;
    }


    SearchPathInfo* info = new SearchPathInfo(loc, locLen);
    EA_ASSERT(!mSearchPathList.contains(*info));

#if defined(EA_ASSERT_ENABLED)
    bool isPathFullyQualified = info->GetDevice() != mNullDevice;

#if defined(EA_PLATFORM_WINDOWS)
    if (!isPathFullyQualified)
    {
        // check for a UNC path
        isPathFullyQualified = (locLen > 2) && ((loc[0] == '\\') && (loc[1] == '\\'));
    }
#endif

#if defined EA_PLATFORM_UNIX || defined EA_PLATFORM_APPLE || defined EA_PLATFORM_PLAYBOOK || defined EA_PLATFORM_PS4
    if (!isPathFullyQualified)
        isPathFullyQualified = (loc[0] == '/') || (loc[0] == '\\') || (loc[0] == '.');
#endif

#if !defined(EA_PLATFORM_AIRPLAY)
    // If Device::GetInstance returns the null device, it means the
    // path is not fully qualified.  However, search paths must be
    // fully-qualified, so return false to indicate a problem.
    if(!isPathFullyQualified)
    {
        delete info;
        return false;
    }
#endif
#endif  // EA_ASSERT_ENABLED

    mSearchPathList.insert(pos, *info);

    return true;
}


/*!
    This function sets a new search path for non-qualified files.
    Search locations are delimited by a semicolon.  Each search
    location must be fully qualified with a device name and full
    path.  Directories can use backslashes or forward slashes.

    The maximum number of search locations and the maximum total
    length of the search path are specified when the file system
    Manager is created, and cannot be changed later.

    If you don't want to specify the whole path, you can use
    AddSearchLocation to append or prepend a location
    to the existing search path.  This would be a common thing
    to do when a new device is registered with the system.

    Be sure that no other threads call AsyncOp::Open with
    non-qualified paths while this function is executing.

    The first location in the search path MUST have device-name
    qualifier in it.  It cannot be a relative path.

    \param path new path to search

    \sa AddSearchLocation
    \sa RemoveSearchLocation
*/
void Manager::SetSearchPath(const char *path)
{
    bool validationPassed = InternalSetSearchPath(path);
    EA_ASSERT_MSG(validationPassed, "Error - rw::core::filesys::Manager::SetSearchPath - Search locations must be fully-qualified.");
    (void)validationPassed;
}

/*! Return the currently set search path. If the path buffer is set to NULL 
    or the specified path buffer size is not large enough this method will
    not copy the path but return the current size of the search path,
    including null terminator.

    \param pathBuffer The buffer in which to copy the current search path.
    \param pathBufferSize The size, in bytes, of the path buffer.
    \return The total size of the search path, including null terminator.
*/
size_t Manager::GetSearchPath(char *pathBuffer, size_t pathBufferSize)
{
    using namespace EA::StdC;

    EA::Thread::AutoFutex lock(mSearchPathFutex);

    // Calculate the size buffer we will need
    unsigned int size = 0;
    for(SearchPathList::iterator info = mSearchPathList.begin(); info != mSearchPathList.end(); ++info)
        size += (info->GetPathLength() + 1);

    if (!pathBuffer || (size > pathBufferSize))
        return size;

    char* ptr = pathBuffer;

    for(SearchPathList::iterator info = mSearchPathList.begin(); info != mSearchPathList.end(); ++info)
    {
        unsigned int len = info->GetPathLength();
        Memcpy(ptr, info->GetPath(), len);
        ptr += len;
        *ptr++ = ';';
    }
    
    // Remove the final ';' if anything was added to the buffer.
    if (ptr > pathBuffer)
        --ptr;

    // Make sure the pathBuffer is null-terminated. 
    *ptr ='\0';

    return static_cast<size_t>(ptr - pathBuffer);
}


/*! [DEPRECATED] 
    Search path length is now unbounded, so call GetSearchPath() function twice - the first call will return exact length required.
*/
size_t Manager::GetMaxSearchPathLength()
{
    EA::Thread::AutoFutex lock(mSearchPathFutex);
    unsigned int size = 0;
    for(SearchPathList::iterator info = mSearchPathList.begin(); info != mSearchPathList.end(); ++info)
        size += (info->GetPathLength() + 1);
    if (size < kDefaultMaxSearchPathLength)
        size = kDefaultMaxSearchPathLength;

    return static_cast<size_t>(size); 
}

/*!
    AddSearchLocation adds the a search location to the
    list of places which are searched for non-qualified file
    paths (file names without a device).

    Be sure that no other threads call AsyncOp::Open with
    non-qualified names while this function is executing.

    The first location in the search path MUST have device-name
    qualifier in it.  It cannot be a relative path.

    \param NewLocation new location to search
    \param flags One of SEARCHPATH_ADD_TO_TAIL or SEARCHPATH_ADD_TO_HEAD

    \sa SetSearchPath, RemoveSearchLocation, InsertSearchLocationBefore
*/
void Manager::AddSearchLocation(const char *NewLocation,unsigned int flags)
{
    using namespace EA::StdC;
    EA_ASSERT(NewLocation);

    EA::Thread::AutoFutex lock(mSearchPathFutex);

    unsigned int len = static_cast<unsigned int>(Strlen(NewLocation));
    bool validationPassed = len ? InternalInsertSearchLocation(NewLocation, len, (flags == SEARCHPATH_ADD_TO_TAIL) ? mSearchPathList.end() : mSearchPathList.begin()) : false;
    EA_ASSERT_MSG(validationPassed, "Error - rw::core::filesys::Manager::AddSearchLocation - Search locations must be fully-qualified.");
    (void)validationPassed;
}

struct PathEquals
{
    const char* path;
    unsigned int len;
    PathEquals(const char* p, unsigned int l) : path(p), len(l) {}
    bool operator ()(const Manager::SearchPathInfo& rhs) const
    {
        return (len == rhs.GetPathLength()) && (Memcmp(rhs.GetPath(), path, len) == 0);
    }
};


/*!
    This function removes the search location from the list of
    places which are searched for non-qualified file paths (file
    names without a device).

    Be sure that no other threads call AsyncOp::Open with
    non-qualified names while this function is executing.

    If the first location is removed, be sure the one following
    is qualified with a device name.  It cannot be a relative path.

    \return True if the search location was successfully removed.
    False if the search location was not found.

    \param pszLoc location to remove
    \sa SetSearchPath, AddSearchLocation, InsertSearchLocationBefore
*/
bool Manager::RemoveSearchLocation(const char *path)
{
    EA_ASSERT(path);
    unsigned int len = static_cast<unsigned int>(Strlen(path));
    EA_ASSERT(len);

    EA::Thread::AutoFutex lock(mSearchPathFutex);

#if defined EA_ASSERT_ENABLED
    //lint -e{961} operator precedence
    const char *co = path + len - 1;

    // check that the user didn't supply a directory separator character.
    EA_ASSERT_MSG((*co != '/') && (*co != '\\'),
        "Do not put a trailing dirsep character on the path");
#endif

    SearchPathList::iterator found = eastl::find_if(mSearchPathList.begin(), mSearchPathList.end(), PathEquals(path, len));
    if (found != mSearchPathList.end())
    {
        SearchPathInfo* info = found.mpNode;
        mSearchPathList.erase(found);
        delete info;
        return true;
    }

    return false;
}


/*!
    This function inserts a new search location just ahead of
    an existing search path.

    \return True if the new search location was successfully inserted
    ahead of an existing search location.
    False if the existing search location was not found.

    \param NewLocation The new search location to insert
    \param ExistingLocation The existing search location to use as insert point
    \sa SetSearchPath, AddSearchLocation
*/

void Manager::InsertSearchLocationBefore( const char *NewLocation, const char *ExistingLocation )
{
    EA_ASSERT(NewLocation && ExistingLocation);

    EA::Thread::AutoFutex lock(mSearchPathFutex);

    SearchPathList::iterator found = eastl::find_if(mSearchPathList.begin(), mSearchPathList.end(),
            PathEquals(ExistingLocation, static_cast<unsigned int>(Strlen(ExistingLocation))));

    bool validationPassed = (found != mSearchPathList.end())
        ? InternalInsertSearchLocation(NewLocation, static_cast<unsigned int>(Strlen(NewLocation)), found)
        : false;

    EA_ASSERT_MSG(validationPassed, "Error - rw::core::filesys::Manager::SetSearchPath - Search locations must be fully-qualified.");
    (void)validationPassed;
}


// This method had to be moved here to placate dll builds
void Manager::SetIdleFunction(EA::Thread::IRunnable *func)
{ 
    sIdleFunction = func;
}


// This method had to be moved inline to placate dll builds
void Manager::Idle()
{
    if (sIdleFunction != NULL) 
    {
        sIdleFunction->Run();
    }
    
    Tick();
}


// Give each registered device some time to execute its operations on
// the Airplay platform.  On all other platforms, Manager::Tick is a
// null function.
void Manager::Tick()
{
#if defined(EA_PLATFORM_AIRPLAY)
    Manager *manager = Manager::GetInstance();

    DeviceList &deviceList = manager->mDeviceList;
    for (DeviceList::iterator I = deviceList.begin(), E = deviceList.end(); I != E; ++I)
    {
        I->ThreadEntry(&*I);
    }
#endif
}

void Manager::SetCWD(const char *dirpath)
{
#ifdef EA_PLATFORM_PS3
    EA_ASSERT_MSG(strchr(dirpath, ':') != NULL || dirpath[0] == '/',
        "The path must include a device specifier or must be an absolute path");
#else
    EA_ASSERT_MSG(strchr(dirpath, ':') != NULL || dirpath[0] == '/' || dirpath[0] == '.',
        "The path must include a device specifier");
#endif

#if defined(EA_DEBUG) && EA_WINAPI_FAMILY_PARTITION(EA_WINAPI_PARTITION_DESKTOP)

    // validate that the given directory exists.
    char curDir[1024];
    if (GetCurrentDirectory(sizeof(curDir), curDir) < sizeof(curDir))
    {
        BOOL success = SetCurrentDirectory(dirpath);
        EA_ASSERT_MSG(success, "The directory does not exist");
        (void)success;
        SetCurrentDirectory(curDir);
    }
#endif

    strncpy(mCurrDir, dirpath, sizeof(mCurrDir));
}


// Returns the current event profiler (internal function)
FileSysProfiler * Manager::GetEventProfiler() const
{
    return sFileSysProfiler;
}

// Returns the current event profiler context (internal function)
void * Manager::GetEventProfilerContext() const
{ 
    return sFileSysProfilerContext;
}


/// \brief Set a file system profiler function.
///
/// This profiler allows the application to see what the file system is
/// doing on its device thread(s).  Each time an open, close, read, or write
/// operation is added to the queue, and each time one of those operations
/// completes, the callback function is called.  Your applications sets up
/// a handler for these events and records them to a memory buffer, along
/// with a timestamp for each event.  When the run is complete, the memory
/// buffer is dumped for later analysis.  This information can be used to
/// improve the layout of the files on the disc to reduce seek times and
/// thereby improve the overall efficiency of the disc.  To use this feature,
/// the RW Core library must be compiled in debug mode.
void Manager::SetEventProfiler(
    FileSysProfiler *pFileSysProfiler,  //!< FileSysProfiler instance
    void *context                       ///< context parameter for the file system profiler
   )
{
    sFileSysProfiler = pFileSysProfiler ? pFileSysProfiler : &sDefaultProfilerInstance;
    sFileSysProfilerContext = context;
}


Manager::Manager(const Options &options) :
    mDeviceList(),
    mThreadParams(options.mThreadParams),
    //lint -e{961} operator precedence
    mReadQuantumSize(UINT32_MAX),
    mNullDevice(NULL),
    mReadSchedulePolicy(READ_SCHEDULE_POLICY_FCFS),
    mErrorHandler(DefaultErrorHandler),
    mAsyncOpOpenCallback(NULL)
{
    mCurrDir[0] = '\0';
}

// This method had to be moved here in order to placate dll builds
EA::Allocator::ICoreAllocator* Manager::GetAllocator()
{ 
    return sAllocator;
}


Manager::~Manager()
{
    InternalClearSearchPath();
    mDeviceListFutex.Lock();
    DeviceList::iterator dev = mDeviceList.begin();
    // Unregister all the devices.
    for (; dev != mDeviceList.end(); dev++)
    {
        UnregisterDevice(dev.mpNode);
    }

    for (dev = mDeviceList.begin(); dev != mDeviceList.end(); )
    {
        EAReadBarrier();
        Device *device = dev.mpNode;

        if (!device->IsVirtual())
        {
            // Only wait for non-virtual devices.
            while (device->IsRunning())
                EA::Thread::ThreadSleep(1);
        }

        dev = mDeviceList.erase(dev);
        delete device;
    }
    mDeviceListFutex.Unlock();

#if !defined(EA_PLATFORM_AIRPLAY)
    // Release the null root list.
    NullFileDriverInst.Restore();
    mNullDevice = NULL;
#endif

    PlatformTeardown();

    mCurrDir[0] = '\0';
}

void *Manager::operator new(size_t size)
{
    EA_ASSERT(sizeof(Manager) == size);

    return RWFILESYSTEM_ALLOCATE(size);
}

void Manager::operator delete(void *p)
{
    Free(p);
}


void Manager::Init()
{
#if !defined(EA_PLATFORM_AIRPLAY)
    // Add the Null Device first.
    NullFileDriverInst.AddRoot("null:");
    mNullDevice = sInstance->RegisterDevice(&NullFileDriverInst);
#endif

    PlatformInit();

    SetSearchPath(mCurrDir);
}

void Manager::RemoveDeviceFromSearchPath(const Device *device)
{
    EA::Thread::AutoFutex lock(GetSearchPathFutex());

    // Find the device index
    for (SearchPathList::iterator info = mSearchPathList.begin(); info != mSearchPathList.end(); ++info)
    {
        Device *deviceToCheck = info->GetDevice();

        if (device == deviceToCheck)
        {
            mSearchPathList.erase(info);
            delete info.mpNode;
            break;
        }
    }
}

bool Manager::DefaultErrorHandler(int, unsigned int, DeviceDriver *, const char *)
{
    // retry the operation
    return true;
}


bool (*Manager::sFileSysErrorInjectionFunc)(DeviceDriver *, const char *) = DefaultErrorInjectionFunction;

/*! Set an artificial error injection function
    If you want to simulate a read error, you can use this function to tell
    the rwfilesystem library that a read error occurred to check your error
    handler implementation.  The real O/S read function will not be called,
    instead, an error is simulated.

    Note that functionality is not implemented on the Wii because the Wii has
    no error handler and thus the error injection would not serve any useful 
    purpose.

    \param func This is handler that accepts a device driver and file name.
    If you want the read to proceed normally, your handler should return true.
    If you want to simulate an error, your handler should return false.
*/
void Manager::SetErrorInjectionFunction(ErrorInjectionFunction func)
{
    if (func == NULL)
    {
        func = DefaultErrorInjectionFunction;
    }

    sFileSysErrorInjectionFunc = func;
}

/// \brief Get the current error injection function.
Manager::ErrorInjectionFunction Manager::GetErrorInjectionFunction() const
{
    return sFileSysErrorInjectionFunc;
}

/// \internal Default error injection function is to proceed with the read as normal.
bool Manager::DefaultErrorInjectionFunction(DeviceDriver *, const char *)
{
    // default behavior is to succeed
    return true;
}

}}}


