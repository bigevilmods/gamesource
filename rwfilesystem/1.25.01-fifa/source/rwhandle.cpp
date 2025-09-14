// (c) Electronic Arts. All Rights Reserved.

/**
\file
Implements rw::core::filesys::Handle.
*/

#include "EAAssert/eaassert.h"
#include "EAStdC/EAString.h"
#include "EAStdC/EASprintf.h"
#include "private_rwhandle.h"
#include "private_rwdevice.h"
namespace rw {
namespace core {
namespace filesys {

// Same as iFileSysOpenHandle in realcore 6.  Except that since
// this is a constructor, it can't return a failed value.  We are
// forbidden from using C++ exception handling in runtime game
// code so the only  way to find out if the file was not opened is
// to check the value of this object's mFileHandle member.  If
// it's NULL, the file failed to open and the Handle object should
// be deleted.  If it's not NULL, the open succeeded.  We are
// relying on the user's allocator to be able to allocate and
// deallocate a Handle object very quickly.  (That should be the
// case; this object is small).
Handle::Handle(const char *filename, unsigned int modeflags, Device *device) :
    mpParentFile(NULL), mDevFileHandle(NULL),
    mDevice(device), mDriver(device->GetDriver()),
    mLength(0), mOffset(0)
{
    mDeviceName[0] = '\0';

    // skip over any .\ or ./ at the start of the path.
    if (*filename == '.' && ((*(filename+1) == '/') || (*(filename+1) == '\\')))
    {
        filename += 2;
    }

    Handle *ParentFileHandle = NULL;
    Manager *pMgr = Manager::GetInstance();

#if defined(EA_PLATFORM_AIRPLAY)
#else
    if (device == pMgr->GetNullDevice())
#endif
    {
        // lock the search path futex so that it cannot be modified in
        // another thread while we're iterating over it here.
        EA::Thread::AutoFutex lock(pMgr->GetSearchPathFutex());

        const Manager::SearchPathList& list = pMgr->GetSearchPathList();

        // search for the file on all devices
        for (Manager::SearchPathList::const_iterator info = list.begin(); info != list.end(); ++info)
        {
            char fullname[FILESYS_MAX_PATH];
            const char *sp = info->GetPath();
            device = info->GetDevice();

#ifdef EA_PLATFORM_WINDOWS
            if (device == pMgr->GetNullDevice())
            {
                // this is the NULL device.  Assert that the file path
                // is UNC
                EA_ASSERT_MSG((*sp == '\\') && (*(sp+1) == '\\'),
                    "unknown device in search path");

                EA_ASSERT_MSG(device->mpNext != NULL,
                    "No device exists that can execute this operation.");

                // go to the next registered device.
                device = static_cast<Device *>(device->mpNext);
            }
#else
            if (device == pMgr->GetNullDevice()) continue;
#endif

            // start the device if it isn't already started.  This might be necessary
            // if the user has registered the device but not used it yet.
            if (!device->IsRunning())
            {
                bool deviceStarted = device->Start();
                EA_ASSERT(deviceStarted);
                (void)deviceStarted;
            }
            EA_ASSERT_MSG((EA::StdC::Strlen(filename) < static_cast<size_t>(FILESYS_MAX_PATH)),"Error, the given filename is too long. The filename will be truncated to FILESYS_MAX_PATH");
            size_t result;
            // drivers are responsible for converting directory separator characters to
            // the correct character (backslash on Windows/Xbox, slash otherwise)
            if ((*filename == '\\') || (*filename == '/'))
            {
                // The file name begins with a directory separator character;
                // so search this device from the root.
                result = EA::StdC::Strlcpy(fullname, filename, static_cast<size_t>(FILESYS_MAX_PATH));
            }
            else
            {
                // the file path is relative; append the name to this search location
                int r = EA::StdC::Snprintf(fullname, FILESYS_MAX_PATH, "%s/%s", sp, filename);
                EA_ASSERT_MSG(r >= 0,"Error, an encoding failure occurred");
                if(r >= 0)
                {
                    result = static_cast<size_t>(r);
                }
                else
                {
                    result = static_cast<size_t>(FILESYS_MAX_PATH);
                }


            }
            if(result < static_cast<size_t>(FILESYS_MAX_PATH))
            {
                mDevFileHandle = device->GetDriver()->Open(fullname, modeflags, &ParentFileHandle);
            }
            

            // this file has been found at this location.
            if (mDevFileHandle != 0)
            {
                GetRootFromPath(mDeviceName, fullname);
                break;
            }
        }
    }
#if !defined(EA_PLATFORM_AIRPLAY)
    else
    {
        // The file name has a device specifier on it, so only check that device
        mDevFileHandle = device->GetDriver()->Open(filename,
            modeflags, &ParentFileHandle);
    }
#endif

    if (mDevFileHandle != NULL)
    {
        if (ParentFileHandle != NULL)
        {
            // If the driver set the ParentFileHandle, meaning that this file is
            // contained in another file, we have to remember the parent handle
            // so the virtual device knows the physical file to read from.
            mpParentFile = ParentFileHandle;

            // Also, update the handle's device thread
            // to point to the parents' device thread.
            // Note that the mDriver member stays with
            // the virtual device.
            mDevice = mpParentFile->mDevice;
        }
        else
        {
            mpParentFile = this;
            mDevice = device;
        }

        // update the driver for this file handle
        mDriver = device->GetDriver();
        mLength = mDriver->GetSize(mDevFileHandle, mDevice->GetDriver(), mpParentFile->mDevFileHandle);
    }
}

Handle::~Handle()
{
    // TODO: scan all the open files and see if any
    // of them are inside this one.  If so, abort.
    if (mDevFileHandle != NULL)
    {
        mDriver->Close(mDevFileHandle, mDevice->GetDriver(), mpParentFile->mDevFileHandle);
    }

    mpParentFile = NULL;
    mDevFileHandle = NULL;
    mDevice = NULL;
    mDriver = NULL;
}


void Handle::GetRootFromPath(char *root, const char *path)
{
    
#if defined(EA_PLATFORM_CAFE)
    const char *deviceEnd = EA::StdC::Strchr(path+1, '/');
    if (deviceEnd != NULL)
        --deviceEnd;
#else
	const char *deviceEnd = EA::StdC::Strchr(path, ':');
#endif

#if defined EA_PLATFORM_APPLE || defined EA_PLATFORM_UNIX || defined EA_PLATFORM_AIRPLAY || defined EA_PLATFORM_PLAYBOOK || defined(EA_PLATFORM_PS4)
    if (!deviceEnd)
    {
        root[0] = '/';
        root[1] = '\0';
        return;
    }
#endif


#if defined(EA_PLATFORM_WINDOWS)
    // handle UNC paths
    if (EA::StdC::Strncmp(path, "\\\\", 2) == 0)
    {
        deviceEnd = EA::StdC::Strchr(&path[2], '\\');
        if (deviceEnd)
        {
            --deviceEnd;
            EA_ASSERT(deviceEnd != NULL);
        }
    }
#endif

#if defined(EA_PLATFORM_PS3)

    if (!deviceEnd)
    {
        deviceEnd = EA::StdC::Strchr(path + 1, '/');
        EA_ASSERT(deviceEnd != NULL);
        --deviceEnd;
    }

#else

    EA_ASSERT(deviceEnd != NULL);

#endif

    size_t nchars = static_cast<size_t>(deviceEnd + 1 - path);
    // limit the number of characters in the device name to
    // MAX_DEVICE_NAME_LENGTH.
    if (nchars > MAX_DEVICE_NAME_LENGTH-1)
        nchars = MAX_DEVICE_NAME_LENGTH-1;
    EA_ASSERT_MSG((EA::StdC::Strlen(path) < static_cast<size_t>(FILESYS_MAX_PATH)),"Error, the given path is too long. The path will be truncated to the MAX_DEVICE_NAME_LENGTH");
    size_t result = EA::StdC::Strlcpy(root, path, nchars);
	if(result >= static_cast<size_t>(FILESYS_MAX_PATH))
	{
		root[0]='\0';
	}
	else
	{
    	root[nchars] = '\0';
	}
}

}}}
