//////////////////////////////////////////////////////////
// Copyright (c) Electronic Arts Inc. All rights reserved.
//////////////////////////////////////////////////////////

/**
\file
Implements rw::core::filesys::DeviceDriver.
*/

#include "EABase/eabase.h"
#include "EAAssert/eaassert.h"
#include "EAStdC/EAString.h"
#include "eathread/eathread_sync.h"
#include "eathread/eathread_thread.h"
#include "eathread/eathread_semaphore.h"
#include "rw/core/filesys/constants.h"

#include "private_rwdevice.h"
#include "private_rwhandle.h"
#include "rw/core/filesys/path.h"
#include "platform_sdk.h"

using namespace EA::StdC;
using namespace EA::Thread;

namespace rw {
namespace core {
namespace filesys {

DeviceDriver::DeviceDriver()
{
}

DeviceDriver::DeviceDriver(const char *root)
{
    AddRoot(root);
}

void DeviceDriver::Restore()
{
    EA::Allocator::ICoreAllocator *allocator = Manager::GetAllocator();

    if (allocator != NULL)
    {
        // if the allocator IS NULL already (meaning the manager has
        // gone out of scope), we don't need to take further action.
        // Other than the root list, there are no other resources to
        // release.
        eastl::intrusive_list<DeviceRoot>::iterator drItr = mRootList.begin();

        while (drItr != mRootList.end())
        {
            void *block = &*drItr;
            drItr = mRootList.erase(drItr);
            allocator->Free(block);
        }
    }
}

const char* DeviceDriver::GetName() const
{
    if (!mRootList.empty())
        return mRootList.front().name;

    return NULL;
}

void DeviceDriver::AddRoot(const char *name)
{
    EA_ASSERT(name != NULL);
    EA_ASSERT_MSG(EA::StdC::Strlen(name) < MAX_DEVICE_NAME_LENGTH,
        "This device name is too long.  The maximum device name is 14 characters, plus a colon");
    DeviceRoot *dr = static_cast<DeviceRoot *>(
        RWFILESYSTEM_ALLOCATE(sizeof(DeviceRoot)));
    dr->mpNext = dr->mpPrev = NULL;
    EA_STRLCPY_VERIFY(dr->name, name, EAArrayCount(dr->name));
    mRootList.push_back(*dr);
}

Device::Device(DeviceDriver *driver, unsigned int flags) :
    mRunning(false),
    mTerminate(false),
    mIsVirtual((flags & Manager::DEVICEFLAG_TYPE_VIRTUAL) == Manager::DEVICEFLAG_TYPE_VIRTUAL),
    mAsyncOpList(),
    mMutex(),
    mOpAdded(),
    mThread(),
    mOpCompletedMutex(),
    mOpCompleted(),
    mLastOpTime(),
    mDriver(driver),
    mLastReadLoc(0)
{
    mpNext = NULL;
    mpPrev = NULL;
#if defined(EA_PLATFORM_PS3)
    // On the PS3, we start the device when it is created.  This is necessary
    // because if the device is started in the cellGameDataCreate2 callback,
    // the system hangs when we call cellDiscGameGetBootDiscInfo in the driver
    // Init function.  In addition, we can assume that pretty much any title
    // will want to use all the physical devices that the library knows about
    // (gDeviceDriverImpl and gHddDriverImpl) so they'll eventually be started
    // anyway.  The other platforms have library devices registered that may
    // never be accessed (PC has a device per logical drive letter, Xenon has
    // a device for the HDD), so we don't want to start these devices until
    // they are actually used.
    bool deviceStarted = Start();
    EA_ASSERT(deviceStarted);
    (void)deviceStarted;

#else

    // Virtual device need to be started right away.  We never schedule
    // operations on them, so we can't rely on the lazy initialization scheme
    // that we use for non-virtual devices.
    if (mIsVirtual)
    {
        bool deviceStarted = Start();
        EA_ASSERT(deviceStarted);
        (void)deviceStarted;
    }

#endif

}

void Device::Unregister()
{
    if (mRunning)
    {
        WaitWhileAsyncOpsExist();

        // Tell the device thread to shut down.
        if (!mIsVirtual)
        {
            mMutex.Lock();

            mTerminate = true;

            EAWriteBarrier();

            mOpAdded.Signal();

            mMutex.Unlock();

            // Block until the device thread has terminated
            while (mRunning)
            {
                EA::Thread::ThreadSleep(1);
            }
        }

        if (mDriver)
            mDriver->Restore();
    }

    mDriver = NULL;
}

void Device::WaitWhileAsyncOpsExist()
{
    for (;;)
    {
        mMutex.Lock();
        bool listEmpty = mAsyncOpList.empty();
        mMutex.Unlock();

        if (listEmpty)
        {
            break;
        }

        EA::Thread::ThreadSleep(1);
    }
}

bool Device::Start()
{
    // Initialize the driver object, failing if it does not initialize successfully.
    if (!mDriver->Init())
        return false;

    if (mIsVirtual)
    {
        // If the device is virtual, we will not create a
        // thread or thread synchronization objects for it.
        // Just mark the device as running.
        mRunning = true;
    }
    else
    {
        EA::Thread::Semaphore semaphore;
        mThreadStartSemaphore = &semaphore;

        EA::Thread::ThreadParameters threadParams =
            Manager::GetInstance()->GetThreadParameters();

		char szThreadName[32]; // declare this here because we want it to still be valid inside mThread.Begin

        if (threadParams.mpName == NULL || threadParams.mpName[0] == '\0')
        {
            // use the device name for the name of this thread if the caller
            // didn't specify one.
			EA::StdC::Snprintf(szThreadName, sizeof(szThreadName), "FileDevice driver %s", mDriver->GetName());
            threadParams.mpName = szThreadName;
        }

#if EA_DEBUG
        EA::Thread::ThreadId id =
#endif
        mThread.Begin(ThreadEntry, this, &threadParams);

#if EA_DEBUG
        EA_ASSERT(id != EA::Thread::kThreadIdInvalid);
#endif

        semaphore.Wait();

        // Assert that the device service thread is running.  If you run
        // out of handles or memory (more likely on a console than a PC)
        // the thread will fail to start, leading to a bug-hunt to find
        // out why.  This assert should catch that condition.
        EA_ASSERT(mThread.GetStatus() == EA::Thread::Thread::kStatusRunning);
    }

    return mRunning;
}

void Device::InsertOp(AsyncOp *Op, bool continuingOp)
{
    // operations cannot be inserted into a virtual devices' queue
    EA_ASSERT_MSG(!mIsVirtual, "Illegal virtual device call");

    EAReadBarrier();
    // Cancelled operations are not added to device queues.
    if (Op->mCancelled)
    {
        Op->mStatus = AsyncOp::OPSTATUS_CANCELLED;
        EAWriteBarrier();
        mOpCompletedMutex.Lock();
        mOpCompleted.Signal(true);
        mOpCompletedMutex.Unlock();
        return;
    }

    // fb - Mutex needs to wrap Start to avoid reentrancy issue if two
    // threads access the device for the first time at the same time.
    // Start() can be quite slow on some devices as it mounts the device

    mMutex.Lock();

    if (!mRunning)
    {
        if(!Start())
        {
            Op->mStatus = AsyncOp::OPSTATUS_FAILED;
            mMutex.Unlock();
            return;
        }
    }

    uint64_t opLocation = 0;
    if (Op->mIsRead)
    {
        // So we don't have to recompute this every time
        // through the list iteration, we compute this once
        // before the iteration starts, even though it may
        // not be needed.
        opLocation = Op->mFileHandle->mDriver->QueryLocation(
            Op->mFileHandle->mDevFileHandle) + Op->mOffset;
    }

    if(Op->mTrackDeviceIdleTime)
    {
        mLastOpTime = GetThreadTime();
    }

    eastl::intrusive_list<AsyncOp>::iterator Itr = mAsyncOpList.begin();
    const int opPriority = Op->GetPriority();

    // find the correct place for this operation in the device queue.
    for (eastl::intrusive_list<AsyncOp>::iterator End = mAsyncOpList.end(); Itr != End; ++Itr)
    {
        const int currentOpPriority = Itr->GetPriority();

        if (opPriority > currentOpPriority)
        {
            // if the new operation's priority is higher than this
            // operation's priority, stop the iteration and insert the
            // new operation after the previous one (before this one)
            break;
        }
        else if (opPriority == currentOpPriority)
        {
            if ((Manager::GetInstance()->GetReadSchedulePolicy() == Manager::READ_SCHEDULE_POLICY_SORTED) &&
                (Op->mIsRead && Itr->mIsRead))
            {
                // In this case, the operations are of equal priority,
                // the scheduling policy is to sort reads according to
                // physical location, and this operation and the
                // previous one are both reads
                uint64_t thisOpLocation = Itr->mFileHandle->mDriver->QueryLocation(
                    Itr->mFileHandle->mDevFileHandle) + Itr->mOffset;

                if (opLocation < thisOpLocation)
                {
                    // The new operation's location is lower than the previous
                    // operation's loctation, so we should insert this operation
                    // after it.
                    break;
                }
            }

            if (continuingOp)
            {
                // This an internally-rescheduled operation.  Execute it
                // ahead of enqueued operations at the same priority.
                break;
            }
        }
    }

    mAsyncOpList.insert(Itr, *Op);

    mMutex.Unlock();
    mOpAdded.Signal();
}

void Device::ChangeOpPriority(AsyncOp *op, int newpriority)
{
    // virtual device do not have a queue; it is not possible to
    // change any priorities of operations on them.
    EA_ASSERT_MSG(!mIsVirtual, "Illegal virtual device call");
    mMutex.Lock();

    eastl::intrusive_list<AsyncOp>::iterator it = mAsyncOpList.locate(*op);

    if (it != mAsyncOpList.end())
    {
        mAsyncOpList.erase(it);
        op->mPriority = newpriority;
        InsertOp(op);
    }

    mMutex.Unlock();
}

// Remove an operation from the pending queue.
void Device::RemoveOp(AsyncOp *op)
{
    // virtual devices do have a queue
    EA_ASSERT_MSG(!mIsVirtual, "Illegal virtual device call");

    // Set the mCancelled to true before we go through the async op list below.
    // If a device is already operating on the op it may not be in the queue so
    // this ensures the op is retired.
    op->mCancelled = true;
    EAWriteBarrier();

    bool foundOp = false;

    {
        AutoMutex autoMutex(mMutex);

        eastl::intrusive_list<AsyncOp>::iterator it = mAsyncOpList.locate(*op);

        if (it != mAsyncOpList.end())
        {
            foundOp = true;
            // Since the op is removed from the queue retire the op
            // in case anyone is Wait()ing on it.
            op->mStatus = AsyncOp::OPSTATUS_CANCELLED;
            EAWriteBarrier();
            mAsyncOpList.erase(it);
        }
    }

    if (foundOp)
        op->mCallbackFunc(op);
	else if (op->mIsRead)
	{
		mDriver->CancelReadOp(op->mFileHandle->mDevFileHandle);
	}
}


// This function returns the device for a given path.  The
// returned device may be virtual or physical.
// \param FilePath The path the user passed to the library at the
// entry point.  May be relative, absolute, or fully-qualified.
// FilePath capacity must be FILESYS_MAX_PATH
// \param fullyQualifiedPathBuffer An optional buffer which receives
// the fully-qualified path to the file.  If the FilePath isn't
// already fully-qualified (which it won't be most of the time),
// the Manager's Current Working Directory (CWD) is used to figure
// out a fully-qualified path.  Cannot alias FilePath.
// fullyQualifiedPathBuffer capacity must be FILESYS_MAX_PATH
// \return device to execute the operation on.  If we aren't sure
// what device will be used, the nulldevice is used, which will
// subsequently cause each device to be checked.
Device *Device::GetInstance(const char *FilePath, char *fullyQualifiedPathBuffer)
{
    Manager *manager = Manager::GetInstance();

    EA_ASSERT_MSG(manager != 0,
        "FileSys library must be initalized with rw::core::filesys::Manager::CreateInstance before any file operations are done.");

    EA_ASSERT(FilePath != fullyQualifiedPathBuffer);

    const char *localPathBuffer = fullyQualifiedPathBuffer;

    if (localPathBuffer != NULL)
    {
        bool bPathIsRelative;

        bPathIsRelative = !PathIsRooted(FilePath);

        // the caller supplied a buffer for the fully-qualified name
        // so, we may need to prepend the CWD or the CWD device to
        // the FilePath, if it's not already fully-qualified
        if (bPathIsRelative)
        {
            // no device was specified in the path,
            // so prepend the CWD path

            EA_STRLCPY_VERIFY(fullyQualifiedPathBuffer, manager->GetCWD(), static_cast<size_t>(FILESYS_MAX_PATH));
            // skip over any .\ or ./ at the start of the path.
            if (*FilePath == '.' && ((*(FilePath+1) == '/') || (*(FilePath+1) == '\\')))
            {
                FilePath += 2;
            }

            if ((*FilePath != '/') && (*FilePath != '\\'))
            {
                // FilePath is relative to CWD, so prepend the CWD.
                // Insert a dirsep character
                EA_STRLCAT_VERIFY(fullyQualifiedPathBuffer, "/", static_cast<size_t>(FILESYS_MAX_PATH));
            }
            else
            {
                char *path = EA::StdC::Strnchr(fullyQualifiedPathBuffer, ':', static_cast<size_t>(FILESYS_MAX_PATH));

                if (path != NULL)
                    // FilePath starts with a directory separator character,
                    // so only use the CWD device instead of the full path
                    *(path+1) = '\0';
            }

            EA_STRLCAT_VERIFY(fullyQualifiedPathBuffer, FilePath, static_cast<size_t>(FILESYS_MAX_PATH));
        }
        else
        {
            // the source path is already fully-qualified, so just copy
            // it to the destination buffer
            EA_STRLCPY_VERIFY(fullyQualifiedPathBuffer, FilePath, static_cast<size_t>(FILESYS_MAX_PATH));
        }
    }
    else
    {
        // just use the FilePath string as-is; do not prepend a device or CWD
        // to it.  This code path is taken from functions such as open or
        // exists,
        localPathBuffer = FilePath;
    }

    char devname[MAX_DEVICE_NAME_LENGTH];
    memset(devname, 0, sizeof(devname));

    const char *fn = EA::StdC::Strnchr(localPathBuffer, ':', static_cast<size_t>(FILESYS_MAX_PATH));

    Device *dev = NULL;

#if defined(EA_PLATFORM_PS3)
    if (fn != NULL)
    {
        // check for a path on /app_home that also contains a Win32 device
        // specifier (e.g. "/app_home/C:\\dir1\file1.dat")
        if (EA::StdC::Strncmp(localPathBuffer, "/app_home", 9) == 0)
        {
            fn = NULL;
        }
    }

    if (fn == NULL)
    {
        // PS3 devices aren't delimited by a colon.  If the path starts
        // with a slash (not a backslash), then we should assume it could
        // be a device
        if (localPathBuffer[0] == '/')
        {
            fn = localPathBuffer+1;

            // Find the first directory separator
            // character after the leading '/'.
            while ((*fn != NULL) && (*fn != '/') && (*fn != '\\'))
            {
                ++fn;
            }

            if (*fn == NULL)
            {
                // the path only contains a device name
                EA_STRLCPY_VERIFY(devname, localPathBuffer, EAArrayCount(devname));
                fn = NULL;
            }
            else if (*(fn+1) == '.')
            {
                // the path ends with a "/.", so treat that as if no
                // extension is present, and NULL out the file name portion of
                // the path.
                const unsigned int devnameSize = static_cast<unsigned int>(fn - localPathBuffer);
                EA::StdC::Strncpy(devname, localPathBuffer, devnameSize);
                devname[EAArrayCount(devname)-1] = '\0';
                fn = NULL;
            }
            else
            {
                --fn;
            }
        }
    }
#endif

    // if the name has a device identifier in it
    // (delimited by a colon), return that device.
    if (fn != 0)
    {
        const unsigned int devnameSize = static_cast<unsigned int>(fn - localPathBuffer + 1); //+1 for colon
        EA::StdC::Strncpy(devname, localPathBuffer, devnameSize);
        devname[EAArrayCount(devname)-1] = '\0';
    }

    if (devname[0] != '\0')
    {
        // lock the manager futex to so the access to the device list
        // is thread-safe
        EA::Thread::AutoFutex lock(manager->GetDeviceListFutex());

        Manager::DeviceList &deviceList = manager->mDeviceList;
        for (Manager::DeviceList::iterator I = deviceList.begin(), E = deviceList.end();
                (I != E) && (dev == NULL);
                ++I)
        {
            if (I->mDriver == 0)
            {
                // This happens if the device was terminated.
                continue;
            }

            const eastl::intrusive_list<DeviceRoot> &deviceRootList = I->mDriver->GetRootList();
            for (eastl::intrusive_list<DeviceRoot>::const_iterator DRI = deviceRootList.begin(),
                    DRE = deviceRootList.end(); DRI != DRE; ++DRI)
            {
                if (Stricmp(DRI->name, devname) == 0)
                {
                    dev = &*I;
                    if (!dev->IsRegistered())
                        continue;
                    break;
                }
            }
        }

        // It is an unrecoverable error if we ever get here - it means that
        // the user is trying to open a file on a device which has no driver, which
        // will lead to a crash at some future point.  Hopefully they're not going
        // to use the release libraries too much at first.
        //lint -e{666, 1025, 1703}  expression with side effects passed to repeated parameter 1
        EA_ASSERT_FORMATTED(dev != NULL,
            ("A device with name %s is not registered with the File System Manager.  Install a driver for this device with rw::core::filesys::RegisterDevice.", devname));
    }
    else
    {
        // otherwise we don't know what device we're going to be on, so
        // use the null Pseudo device (so all devices will be checked)

        if (localPathBuffer[0]=='/' || localPathBuffer[0]=='\\' || 
            (localPathBuffer[0] == '.' && localPathBuffer[1] == '/') )
        {
            // lock the manager futex to so the access to the device list
            // is thread-safe
            EA::Thread::AutoFutex lock(manager->GetDeviceListFutex());

            Manager::DeviceList &deviceList = manager->mDeviceList;
            for (Manager::DeviceList::iterator I = deviceList.begin(), E = deviceList.end(); 
                    (I != E) && (I->mDriver != 0) && (dev == NULL);
                    ++I)
            {
                const eastl::intrusive_list<DeviceRoot> &deviceRootList = I->mDriver->GetRootList();
                for (eastl::intrusive_list<DeviceRoot>::const_iterator DRI = deviceRootList.begin(),
                        DRE = deviceRootList.end(); DRI != DRE; ++DRI)
                {
                    if (&*I != manager->GetNullDevice())
                    {
                        dev = &*I;
                        break;
                    }
                }
            }
        }
        else
        {
            dev = manager->GetNullDevice();
        }
    }

    return dev;

}
// Wait probably does not do what you expect. Each device operation that
// completes will do an up on the operation completed semaphore. IF you call
// this function you will have one of two behaviours. EITHER
// you will wait till the current operation is completed IF there have been
// not previous operations that completed on this device, OR you will
// return immediatly with the current count - 1 of completed operations
void Device::Wait(const AsyncOp *op, const EA::Thread::ThreadTime& timeoutAbsolute)
{
    // it's illegal to try to wait for an operation on a virtual
    // device.
    EA_ASSERT_MSG(!mIsVirtual, "Illegal virtual device call");

    mOpCompletedMutex.Lock();
    EAReadBarrier();
    while ((op->mStatus == AsyncOp::OPSTATUS_PENDING) &&
        timeoutAbsolute > EA::Thread::GetThreadTime())
    {
        mOpCompleted.Wait(&mOpCompletedMutex, timeoutAbsolute);
        EAReadBarrier();
    }
    mOpCompletedMutex.Unlock();

    if (timeoutAbsolute == EA::Thread::kTimeoutNone)
    {
        EA_ASSERT(op->mStatus != AsyncOp::OPSTATUS_PENDING);
    }
}

// This is the entry point for the device processor thread.
intptr_t Device::ThreadEntry(void *Param)
{
    Device *pDev = static_cast<Device *>(Param);

    pDev->mRunning = true;

    pDev->mThreadStartSemaphore->Post();
    pDev->mThreadStartSemaphore = NULL;

    // Virtual devices do not have threads.
    EA_ASSERT_MSG(!pDev->IsVirtual(), "Illegal virtual device call");

    pDev->mMutex.Lock();

    do
    {
        if (!pDev->mAsyncOpList.empty())
        {
            // Get the next job off the Pending list.
            AsyncOp *op = &pDev->mAsyncOpList.front();

            pDev->mAsyncOpList.pop_front();

            // See if there's a better operation to execute
            op = pDev->CheckForOptimalReadOp(op);

            // There is a job to do, so execute it.
            pDev->mMutex.Unlock();

            // Execute the command.
            AsyncOp::OpStatus newstatus = op->mExecFunc(op);

            // Large read operations need to be done in pieces,
            // so see if the DoRead function actually completed
            // the request.
            if (newstatus != AsyncOp::OPSTATUS_PENDING)
            {
                pDev->mOpCompletedMutex.Lock();

                AsyncOpCallback *pCallback = op->mCallbackFunc;
                Device *opDevice = op->mDevice;

                // putting a compiler memory barrier here eliminates the (remote)
                // possibility of the compiler (or even the hardware) re-ordering
                // these two instructions.  If these instructions are executed in
                // the opposite order they're written here, a very rare but
                // potentially major problem will be introduced because the operation
                // can be destroyed any time after mStatus is modified.
                EACompilerMemoryBarrier();

                op->mStatus = newstatus;

                // Force the new mStatus value is to be flushed out to main (i.e.
                // non-cached) memory.  If this isn't done, other threads might see
                // the previous value of mStatus and not register the operation as
                // complete when they should.
                EAWriteBarrier();
                
                // Set the last operation time to be right before the callback
                // is called.
                pDev->mLastOpTime = GetThreadTime();
                
                // At this point, we can no longer assume that op is valid.
                // From the moment the status is changed to something other
                // than pending, the operation object may be destroyed.  It
                // is still safe to call this callback function however because
                // if the caller was waiting on the callback, the op object
                // must still be valid at the time the callback is called.
                // If not, the default function does nothing.

                // The operation is complete; execute its callback.
                pCallback(op);

                // Any threads blocking in Device::Wait will wake up and
                // see if the operation they were waiting on has completed.
                // Threads waiting on other operations will just go back to
                // waiting on their operation.
                pDev->mOpCompleted.Signal(true);
                pDev->mOpCompletedMutex.Unlock();

                if (opDevice != pDev)
                {
                    // In most cases, this device will be the same as the
                    // operation device.  But, in the case where a read was
                    // processed on a processing thread, the above callback
                    // will not wake up the waiting thread.  So, we have to
                    // signal the device on which the read was originally
                    // started.
                    opDevice->mOpCompletedMutex.Lock();
                    opDevice->mOpCompleted.Signal(true);
                    opDevice->mOpCompletedMutex.Unlock();
                }
            }

            pDev->mMutex.Lock();
        }
        else
        {
            // Nothing to do; wait for a command to be
            // entered on the device operation queue
            EAReadBarrier();
            if (pDev->mTerminate)
                break;

            const EA::Thread::Condition::Result result = pDev->mOpAdded.Wait(&pDev->mMutex);
            if(result != EA::Thread::Condition::kResultOK)
                break;
        }
    } while (!pDev->mTerminate);

    pDev->mMutex.Unlock();

    pDev->mRunning = false;

    EAWriteBarrier();

    return 0;
}

// although the pending queue is sorted, we might
// be better off choosing an operation which is not
// at the head of the list, so that reads are done
// in physical sort order.
// This is a private function to this class, and the
// device mutex must be locked when this function is
// called.
AsyncOp *Device::CheckForOptimalReadOp(AsyncOp *op)
{
    if (op->mIsRead &&
        (Manager::GetInstance()->GetReadSchedulePolicy() ==
        Manager::READ_SCHEDULE_POLICY_SORTED))
    {
        uint64_t opLocation = op->mFileHandle->mDriver->QueryLocation(
            op->mFileHandle->mDevFileHandle) + op->mOffset;

        if (mLastReadLoc != 0)
        {
            // If opLocation is higher than the last read location,
            // we should just use it.  If not, we will search the
            // op list for the first read operation that is ahead
            // of the last read location.  If one can't be found,
            // we'll keep this operation.
            if (opLocation < mLastReadLoc)
            {
                // It might be better to not take the first read op
                // in the list.  Instead, search for the first one
                // that is located after pDev->mLastReadLoc.
                eastl::intrusive_list<AsyncOp>::iterator Itr, End;
                uint64_t thisOpLocation = 0;
                bool bFoundABetterOp = false;

                for (Itr = mAsyncOpList.begin(), End = mAsyncOpList.end(); Itr != End ; ++Itr)
                {
                    if (!Itr->mIsRead || Itr->mPriority != op->mPriority)
                    {
                        // stop the iteration when we:
                        // - have searched the entire list; OR
                        // - find an operation that isn't a read; OR
                        // - find an operation at a different priority.
                        break;
                    }

                    thisOpLocation = Itr->mFileHandle->mDriver->QueryLocation(Itr->mFileHandle->mDevFileHandle) + Itr->mOffset;

                    if (mLastReadLoc < thisOpLocation)
                    {
                        // We have found a read operation which is after
                        // the last one.  Since the list is sorted in
                        // physical order, this is the best operation to
                        // execute
                        bFoundABetterOp = true;
                        break;
                    }
                }

                if (bFoundABetterOp)
                {
                    // the first one on the list is not the best choice;
                    // substitute another.  First push the previous op
                    // back onto the head of the list, then remove this
                    // operation from the middle
                    mAsyncOpList.push_front(*op);
                    op = &*Itr;
                    opLocation = thisOpLocation;
                    mAsyncOpList.remove(*op);
                }
            }
        }

        // store the last read location so we can choose
        // the right read op next time.
        mLastReadLoc = opLocation;
    }

    return op;
}

size_t Device::GetIdleTime() const
{
    ThreadTime currTime = GetThreadTime();
    uint64_t temp = (uint64_t)EA_THREADTIME_AS_INT64(currTime - mLastOpTime);
    return (size_t)temp;
}

}}}

