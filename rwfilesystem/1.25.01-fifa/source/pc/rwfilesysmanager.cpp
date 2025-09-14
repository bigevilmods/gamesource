// (c) Electronic Arts. All Rights Reserved.

/**
\file
Implements rw::core::filesys::Manager platform specific componentry.
*/

#include <new>

#include "EAAssert/eaassert.h"
#include "rw/core/filesys/manager.h"
#include "rw/core/filesys/driverimpl.h"
#include "platform_sdk.h"

#if !defined(EA_WINAPI_FAMILY_PARTITION)
    #define EA_WINAPI_FAMILY_PARTITION(partition) partition
#endif

#if !defined(EA_WINAPI_PARTITION_DESKTOP)
    #if defined(EA_PLATFORM_WINDOWS)
        #define EA_WINAPI_PARTITION_DESKTOP 1
    #else
        #define EA_WINAPI_PARTITION_DESKTOP 0
    #endif
#endif

#if defined (EA_PLATFORM_WINDOWS_PHONE)
    using namespace Windows::Storage;
#endif

namespace rw {
namespace core {
namespace filesys {    
    

#if (defined(EA_PLATFORM_WINDOWS) && !EA_WINAPI_FAMILY_PARTITION(EA_WINAPI_PARTITION_DESKTOP)) || defined(EA_PLATFORM_WINDOWS_PHONE)
    
#define METRO_INSTALL_DEVICE_NAME "ms-appx" 
#define METRO_APPDATA_DEVICE_NAME "ms-appdata" 

    static char sInstallpath[FILESYS_MAX_PATH];
    static char sLocalappstore[FILESYS_MAX_PATH];
    static char sTempfolder[FILESYS_MAX_PATH];

    static bool RemapFilename(const char* filename, char* mappedFilename, size_t mappedFilenameCap)
    {
        bool result = false;
        char devname[MAX_DEVICE_NAME_LENGTH];
        EA::StdC::Memset8(devname, 0, sizeof(devname));
            
        const char *fn = EA::StdC::Strchr(filename, ':');
        // if the name has a device identifier in it
        // (delimited by a colon), return that device.
        if (fn != 0)
        {
            size_t copySize = static_cast<unsigned int>((fn-filename)+1);
            EA::StdC::Strlcpy(devname, filename, MIN(copySize, EAArrayCount(devname)));

            if ( EA::StdC::Strcmp(devname, METRO_INSTALL_DEVICE_NAME) == 0 )
            {
                if ( EA::StdC::Strstr(filename, METRO_INSTALL_DEVICE_NAME":///") == filename
                    ||  EA::StdC::Strstr(filename, METRO_INSTALL_DEVICE_NAME":\\\\\\") == filename )
                {
                    EA::StdC::Snprintf(mappedFilename,  mappedFilenameCap, "%s/%s", sInstallpath, filename + EA::StdC::Strlen(METRO_INSTALL_DEVICE_NAME":///"));
                    result = true;
                }
                else
                {
                    // we don't support reading files from other packages at this point.
                    EA_FAIL_FORMATTED(("Ill-formed filename. "METRO_INSTALL_DEVICE_NAME": device must be of the form \""METRO_INSTALL_DEVICE_NAME":///<filepath>\".  [%s]", filename));
                }
            }
            else if ( EA::StdC::Strcmp(devname, METRO_APPDATA_DEVICE_NAME) == 0 )
            {
                // pluck out the first directory name.. this tells us which path to use
                // ms-appdata should always be followed by ":///".  
                const char* appdataTypeBegin = filename + EA::StdC::Strlen(METRO_APPDATA_DEVICE_NAME":///");
                const char* const nameEnd = filename + EA::StdC::Strlen(filename);
                if ( appdataTypeBegin < nameEnd )
                {
                    const char* typeEnd0 = strchr(appdataTypeBegin, '/');
                    const char* typeEnd1 = strchr(appdataTypeBegin, '\\');
                    const char* typeEnd = 0;
                    if ( typeEnd0 != 0 && typeEnd1 != 0 )
                    {
                        typeEnd = (typeEnd0 < typeEnd1) ? typeEnd0 : typeEnd1;
                    }
                    else if (typeEnd0 == 0) 
                    {
                        typeEnd = typeEnd1;
                    }
                    else
                    {
                        typeEnd = typeEnd0;
                    }
                    if ( typeEnd != 0 )
                    {
                        char typeName[15] = { '\0' };
                        size_t typeCopySize = static_cast<unsigned int>((typeEnd-appdataTypeBegin)+1);
                        EA::StdC::Strlcpy(typeName, appdataTypeBegin, MIN(typeCopySize, EAArrayCount(typeName)));

                        if ( EA::StdC::Strcmp( typeName, "temp" ) == 0 )
                        {
                            EA::StdC::Snprintf(mappedFilename,  mappedFilenameCap, "%s%s", sTempfolder, typeEnd);
                            result = true;
                        }
                        else if ( EA::StdC::Strcmp( typeName, "local" ) == 0 )
                        {
                            EA::StdC::Snprintf(mappedFilename,  mappedFilenameCap, "%s%s", sLocalappstore, typeEnd);
                            result = true;
                        }
                        else
                        {
                            EA_FAIL_FORMATTED(("unhandled appdata store location: %s", typeName));
                        }
                    }
                    else
                    {
                        EA_FAIL_FORMATTED(("Ill-formed filename. ms-appdata: device must be of the form \"ms-appdata:///<type>/<filepath>\".  [%s]", filename));
                    }
                }
                else
                {
                    EA_FAIL_FORMATTED(("Ill-formed filename. ms-appdata: device must be of the form \"ms-appdata:///<type>/<filepath>\".  [%s]", filename));
                }
            }
        }
        return result;
    }

    class WinRTURIDeviceDriver : public DeviceDriver
    {
    public:
        

        struct DirectoryIteratorHandle
        {
            DirectoryIteratorHandle() : handle(NULL), driver(NULL) {}
            static void *operator new(size_t size)
            {
                return RWFILESYSTEM_ALLOCATE(size);
            }

            static void operator delete(void *p)
            {
                return Manager::Free(p);
            }

            void* handle;
            DeviceDriver* driver;
        };

        WinRTURIDeviceDriver() 
        {
            const char* const METRO_APP_APPDATASTORE = METRO_INSTALL_DEVICE_NAME":";
            const char* const METRO_APP_PACKAGEAPPSTORE = METRO_APPDATA_DEVICE_NAME":";
            AddRoot(METRO_APP_APPDATASTORE);
            AddRoot(METRO_APP_PACKAGEAPPSTORE);
        }

        virtual ~WinRTURIDeviceDriver() {};

        virtual void *          Open(const char *name, unsigned int oflags, Handle **ppdfh)
        {
            char mappedFilename[FILESYS_MAX_PATH] = { '\0' };
            bool mapped = RemapFilename(name, mappedFilename, EAArrayCount(mappedFilename));
            if ( !mapped )
            {
                EA_FAIL_FORMATTED(("failed to remap file: %s", name));
                return NULL;
            }


            if ( mappedFilename[0] != '\0' )
            {
                Device* mappedDevice = Device::GetInstance(mappedFilename);
                    
                Handle* parentHandle = new Handle(mappedFilename, oflags, mappedDevice);
                if ( parentHandle->mDevFileHandle == NULL )
                {
                    delete parentHandle;
                    parentHandle = NULL;
                }

                *ppdfh = parentHandle;

                return parentHandle;
            }
            else
            {
                EA_FAIL_FORMATTED(("File mapped to an empty string.  this shouldn't happen: %s", name));
                return NULL;
            }
        }

        virtual void            Close(void * h)
        {
            Handle* parentHandle = static_cast<Handle*>(h);
            delete parentHandle;
        }

        virtual unsigned int    Read(void * h, void *buf, unsigned int bufsize, DeviceDriver *dd, void *dfh)
        {
            Handle* handle = static_cast<Handle*>(h);
            Handle *parent = handle->mpParentFile;
            return dd->Read(dfh, buf, bufsize, parent->mDevice->GetDriver(), parent->mDevFileHandle);
        }

        virtual unsigned int    Write(void * h, const void *buf, unsigned int bufsize, DeviceDriver *dd, void *dfh)
        {
            Handle* handle = static_cast<Handle*>(h);
            Handle *parent = handle->mpParentFile;
            return dd->Write(dfh, buf, bufsize, parent->mDevice->GetDriver(), parent->mDevFileHandle);
        }

        virtual uint64_t        Seek(void * h, uint64_t offset, int whence, DeviceDriver *dd, void *dfh)
        {
            Handle* handle = static_cast<Handle*>(h);
            Handle *parent = handle->mpParentFile;
            return dd->Seek(dfh, offset, whence, parent->mDevice->GetDriver(), parent->mDevFileHandle);
        }

        virtual uint64_t        GetSize(void * h)
        {
            Handle* parentHandle = static_cast<Handle*>(h);
            return parentHandle->mDriver->GetSize(parentHandle->mDevFileHandle);
        }

        virtual bool            Resize(void * h, uint64_t newsize)
        {
            Handle* parentHandle = static_cast<Handle*>(h);
            return parentHandle->mDriver->Resize(parentHandle->mDevFileHandle, newsize);
        }

        virtual bool            Delete(const char *filePath)
        {
            char mappedFilename[FILESYS_MAX_PATH] = { '\0' };
            bool mapped = RemapFilename(filePath, mappedFilename, EAArrayCount(mappedFilename));
            if ( !mapped )
            {
                EA_FAIL_FORMATTED(("failed to remap file: %s", filePath));
                return false;
            }

            return Device::GetInstance(mappedFilename)->GetDriver()->Delete(mappedFilename);
        }

        virtual bool            Move(const char *currentName, const char *newName)
        {
            char mappedCurrentName[FILESYS_MAX_PATH] = { '\0' };
            bool mapped = RemapFilename(currentName, mappedCurrentName, EAArrayCount(mappedCurrentName));
            if ( !mapped )
            {
                EA_FAIL_FORMATTED(("failed to move file: %s ==> %s", currentName, newName));
                return false;
            }
            
            char mappedNewName[FILESYS_MAX_PATH] = { '\0' };
            mapped = RemapFilename(newName, mappedNewName, EAArrayCount(mappedNewName));
            if ( mapped )
            {
                // if we mapped successfully, then use the mapped name for the newName.
                // otherwise, it's not not a name we know how to map, so just pass through
                newName = mappedNewName;
            }

            return Device::GetInstance(mappedCurrentName)->GetDriver()->Move(mappedCurrentName, newName);
        }

        virtual void *          FindBegin(const char *filePattern, FindData *findData)
        {
            char mappedFilename[FILESYS_MAX_PATH] = { '\0' };
            bool mapped = RemapFilename(filePattern, mappedFilename, EAArrayCount(mappedFilename));
            if ( !mapped )
            {
                EA_FAIL_FORMATTED(("failed to remap directory: %s", filePattern));
                return false;
            }
            
            DirectoryIteratorHandle* handle = NULL;

            DeviceDriver* driver = Device::GetInstance(mappedFilename)->GetDriver();
            void* mappedhandle = driver->FindBegin(mappedFilename, findData);
            if ( mappedhandle != NULL )
            {
                handle = new DirectoryIteratorHandle();
                handle->handle = mappedhandle;
                handle->driver = driver;
            }

            return handle;
        }

        virtual bool            FindNext(void *h, FindData *findData)
        {
            DirectoryIteratorHandle* handle = static_cast<DirectoryIteratorHandle*>(h);
            return handle->driver->FindNext(handle->handle, findData);
        }
        
        virtual bool            FindEnd(void *h)
        {
            DirectoryIteratorHandle* handle = static_cast<DirectoryIteratorHandle*>(h);
            
            bool result = handle->driver->FindEnd(handle->handle);
            delete handle;
            return result;
        }

        virtual bool            DirectoryCreate(const char *directory)
        {
            char mappedFilename[FILESYS_MAX_PATH] = { '\0' };
            bool mapped = RemapFilename(directory, mappedFilename, EAArrayCount(mappedFilename));
            if ( !mapped )
            {
                EA_FAIL_FORMATTED(("failed to remap directory: %s", directory));
                return false;
            }
            
            return Device::GetInstance(mappedFilename)->GetDriver()->DirectoryCreate(mappedFilename);
        }

        virtual bool            DirectoryRemove(const char *directory)
        {
            char mappedFilename[FILESYS_MAX_PATH] = { '\0' };
            bool mapped = RemapFilename(directory, mappedFilename, EAArrayCount(mappedFilename));
            if ( !mapped )
            {
                EA_FAIL_FORMATTED(("failed to remap directory: %s", directory));
                return false;
            }
            
            return Device::GetInstance(mappedFilename)->GetDriver()->DirectoryRemove(mappedFilename);
        }

        virtual bool            GetNameFromHandle(void *handle, char *name, size_t nameBufferSize)
        {
            Handle* parentHandle = static_cast<Handle*>(handle);
            return parentHandle->mDriver->GetNameFromHandle(parentHandle->mDevFileHandle, name, nameBufferSize);
        }

      
    };
    static WinRTURIDeviceDriver* sWinrtURIDriver = NULL;
#endif


    // On Windows, we treat each logical drive a separate
    // physical drive.  Even though more than one logical
    // drive could map to the same physical device, the
    // O/S will take care of the scheduling for us.  The
    // PC never has poor load times due to disc access times,
    // and we don't want to throttle the system by using a
    // single queue for ever device in the system, so the
    // easiest thing to do is just use a single root on each
    // device.
    static PCFileDeviceDriver *libdevice[26];



#ifdef EA_PLATFORM_WINDOWS_PHONE
    // String conversion functions required on WinPRT
    // Move to a better place (possibly EAStdC).
    template<typename T, typename U>
    T string_cast(const U);

    template<>
    std::string string_cast<std::string, Platform::String^>(Platform::String^ src)
    {
        std::wstring wstr(src->Data());
        std::string res;
        res.resize(EA::StdC::Strlcpy(static_cast<decltype(&res[0])>(nullptr), wstr.c_str(), 0) + 1);
        EA::StdC::Strlcpy(&res[0], wstr.c_str(), res.size());
        return res;
    }

    template<>
    Platform::String^ string_cast<Platform::String^, const char*>(const char* src)
    {
        std::wstring wstr;
        wstr.resize(EA::StdC::Strlcpy(static_cast<decltype(&wstr[0])>(nullptr), src, 0) + 1);
        EA::StdC::Strlcpy(&wstr[0], src, wstr.size());
        return ref new Platform::String(wstr.c_str());
    }
#endif


    // InitDeviceList populates the libdevice array by using the information
    // returned by GetLogicalDrives to figure out what drive letters are
    // mapped on this particular system.  Only drives with letters attached
    // to them are supported by the EACore file system.
    void Manager::PlatformInit()
    {
        memset(libdevice, 0, sizeof(libdevice));
            
        #if EA_WINAPI_FAMILY_PARTITION(EA_WINAPI_PARTITION_DESKTOP)

            DWORD ret = GetCurrentDirectory(sizeof(mCurrDir), mCurrDir);
            (void)ret;
                
            EA_ASSERT_MSG(ret != 0, 
                "The current directory path is longer than 256 characters. "
                "Either make FILESYS_MAX_PATH longer, or shorten the current working directory path.");


            DWORD DriveMask = GetLogicalDrives();
            
        #elif (defined(EA_PLATFORM_WINDOWS) && !EA_WINAPI_FAMILY_PARTITION(EA_WINAPI_PARTITION_DESKTOP)) || defined(EA_PLATFORM_WINDOWS_PHONE)
            Platform::String^ installpath = Windows::ApplicationModel::Package::Current->InstalledLocation->Path;
            EA::StdC::Strlcpy(sInstallpath, installpath->Data(), sizeof(sInstallpath));
            #if defined(EA_PLATFORM_WINDOWS_PHONE)
                Platform::String^ tempfolder = Windows::Storage::ApplicationData::Current->LocalFolder->Path;
            #else
            Platform::String^ tempfolder = Windows::Storage::ApplicationData::Current->TemporaryFolder->Path;
            #endif
            EA::StdC::Strlcpy(sTempfolder, tempfolder->Data(), sizeof(sTempfolder));
            Platform::String^ localappstore = Windows::Storage::ApplicationData::Current->LocalFolder->Path;
            EA::StdC::Strlcpy(sLocalappstore, localappstore->Data(), sizeof(sLocalappstore));

            sWinrtURIDriver = new (RWFILESYSTEM_ALLOCATE(sizeof WinRTURIDeviceDriver)) WinRTURIDeviceDriver();
                        
            EA::StdC::Strlcpy(mCurrDir, sInstallpath, EAArrayCount(mCurrDir));

            char installDriveLetter = sInstallpath[0];
            char tempDriveLetter = sTempfolder[0];
            char localStoreDriveLetter = sLocalappstore[0];
                        
            RegisterDevice(sWinrtURIDriver, DEVICEFLAG_TYPE_VIRTUAL);
            DWORD DriveMask = 
                  (1 << (installDriveLetter - 'A')) 
                | (1 << (tempDriveLetter - 'A')) 
                | (1 << (localStoreDriveLetter - 'A'));
          
        #else
            const char *PLATFORM_DEFAULT_CWD = "G:";
            strcpy(mCurrDir, PLATFORM_DEFAULT_CWD);

            // Add drive C, D, E , G and T to the mask
            DWORD DriveMask = 
                  (1 << ('C' - 'A')) 
                | (1 << ('D' - 'A')) 
                | (1 << ('E' - 'A'))
                | (1 << ('G' - 'A'))
                | (1 << ('T' - 'A'));
        #endif

        char DriveString[4];
        strcpy(DriveString, "A:");

        for (int i = 0; i < 26; ++i)
        {
            if ((DriveMask & (1 << i)) == (1u << i))
            {
                DriveString[0] = 'A' + static_cast<char>(i);
                libdevice[i] = new (RWFILESYSTEM_ALLOCATE(sizeof PCFileDeviceDriver))
                    PCFileDeviceDriver;
                libdevice[i]->AddRoot(DriveString);
                RegisterDevice(libdevice[i]);
            }
        }
    }


    void Manager::PlatformTeardown()
    {
        // release the library physical devices
        for (int i = 0; i < 26; ++i)
        {
            if (libdevice[i] != NULL)
            {
                // this was allocated with the library's allocator;
                // cannot call operator delete on it.
                libdevice[i]->Restore();
                Free(libdevice[i]);
            }
        }

        memset(libdevice, 0, sizeof(libdevice));
        
        #if (defined(EA_PLATFORM_WINDOWS) && !EA_WINAPI_FAMILY_PARTITION(EA_WINAPI_PARTITION_DESKTOP)) || defined(EA_PLATFORM_WINDOWS_PHONE)
            // this was allocated with the library's allocator;
            // cannot call operator delete on it.
            sWinrtURIDriver->Restore();
            Free(sWinrtURIDriver);
        #endif
    }

}}}
