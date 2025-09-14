// (c) Electronic Arts. All Rights Reserved.

/**
\file
Private header of rw::core::filesys::Device, implements the ::detail::ListNodeS class.
*/

#ifndef RW_CORE_FILESYS_INTERNAL_DEVICE_H
#define RW_CORE_FILESYS_INTERNAL_DEVICE_H

#include "EABase/eabase.h"
#include "eathread/eathread_mutex.h"
#include "eathread/eathread_thread.h"
#include "eathread/eathread_condition.h"
#include "eathread/eathread_semaphore.h"
#include "EASTL/intrusive_list.h"
#include "rw/core/filesys/device.h"

namespace rw {
namespace core {
namespace filesys {

///! \internal
class Device : public eastl::intrusive_list_node
{
public:

    Device(DeviceDriver *driver, unsigned int flags);
    ~Device() {}

    void Unregister();

    // Is the device running?
    bool IsRunning() const { return mRunning; }

    // Return the driver object associated with this device
    DeviceDriver *GetDriver() const { return mDriver; }

    // \internal Return the thread object for this device
    EA::Thread::Thread *GetThread()
    { return &mThread; }

    // Return true if this device is virtual; false if not.
    bool IsVirtual() const { return mIsVirtual; }

    bool Start();
    void InsertOp(AsyncOp *NewOp, bool continuingOp = false);
    void ChangeOpPriority(AsyncOp *op, int newpriority);
    void RemoveOp(AsyncOp *OpToRemove);

    // Iterate over the list of known devices and find
    // the one that corresponds to the given file path.
    // If a device has not been started, this function
    // will start it before attempting to use it.
    // Also, if fullyQualifiedPathBuffer is not NULL, this
    // buffer will be filled with the fully-Qualified path
    // to the file or directory.
    // if FilePath of the form...   fullyQualifiedPathBuffer will be:
    // ${dir}                       ${CWD}/${dir}
    // /${dir}                      ${CWD device}:/${dir}
    // ${dev}:/${dir}               same as FilePath
    // \param FilePath holds the path to append or to get the device of
    // \param fullyQualifiedPathBuffer Buffer large enough to hold the complete path
    // \return Device to execute the operation on, or the NULL device
    // if all need to be checked.
    static Device *GetInstance(const char *FilePath, char *fullyQualifiedPathBuffer = NULL);

    // Wait for the device to complete the next operation.
    // The caller will still have to check if the operation
    // they were waiting for has been completed.
    // Wait probably does not do what you expect. Each device operation that
    // completes will do an up on the operation completed semaphore. IF you call
    // this function you will have one of two behaviors. EITHER
    // you will wait till the current operation is completed IF there have been
    // not previous operations that completed on this device, OR you will
    // return immediately with the current count - 1 of completed operations
    void Wait(const AsyncOp *op, const EA::Thread::ThreadTime& timeoutAbsolute);

    // Thread entry function
    static intptr_t ThreadEntry(void *pParam);

    static void *operator new(size_t size) { return RWFILESYSTEM_ALLOCATE(size); }
    static void operator delete(void *ptr) { return Manager::Free(ptr); }

    bool IsRegistered() { return (!mTerminate); }

    size_t GetIdleTime() const;

private:

    volatile bool                   mRunning;       // Device thread is running
    volatile bool                   mTerminate;     // Device thread should terminate
    bool                            mIsVirtual;     // true if this was registered as a virtual device.
    eastl::intrusive_list<AsyncOp>  mAsyncOpList;   // priority sorted list of operations waiting to be processed
    EA::Thread::Mutex               mMutex;         // To synchronize access to the device from multiple threads
    EA::Thread::Condition           mOpAdded;       // Monitor for synchronizing access to device thread
    EA::Thread::Thread              mThread;        // Device thread object.
    EA::Thread::Mutex               mOpCompletedMutex; // Mutex associated with the OpCompleted condition variable
    EA::Thread::Condition           mOpCompleted;   // Monitor for signaling that an operation has completed.
    EA::Thread::ThreadTime          mLastOpTime;    // time of last operation done on this device
    EA::Thread::Semaphore *         mThreadStartSemaphore;
    DeviceDriver *                  mDriver;        // driver object for this device
    uint64_t                        mLastReadLoc;   // location of last read done on this device

    friend class FindDeviceByName;

    Device(const Device& rhs);
    Device& operator=(const Device& rhs);

    // Search for a better operation to execute than the
    // one at the head of the pending queue
    AsyncOp *CheckForOptimalReadOp(AsyncOp *);

    void WaitWhileAsyncOpsExist();
};

}}}

#endif
