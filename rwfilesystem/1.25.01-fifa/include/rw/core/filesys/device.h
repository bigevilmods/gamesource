// (c) Electronic Arts. All Rights Reserved.

/**
\file
Header file of rw::core::filesys::DeviceDriver, an abstract class for defining the interface between the filesys library and a device.
*/

#ifndef RW_CORE_FILESYS_DEVICE_H
#define RW_CORE_FILESYS_DEVICE_H

#include "EABase/eabase.h"
#include "EASTL/intrusive_list.h"
#include "eathread/eathread.h"
#include "rw/core/filesys/manager.h"
#include "rw/core/filesys/async.h"

// winbase.h redefines CreateDirectory and RemoveDirectory to
// other values.  So we undefine them here so core library
// callers our CreateDirectory function.
#ifdef CreateDirectory
    #undef CreateDirectory
#endif

#ifdef RemoveDirectory
    #undef RemoveDirectory
#endif

// This disables a warning that will come up in dll-builds since this class contains a 
// templated member object that cannot be properly exported.  This would normally not
// be an issue, however there is a public method that returns a reference to this templated object.
// For now, please note that the behavior of this method in a Dll build is undefined, and
// that you should make sure that the dll is built with exactly the same compiler settings as
// your application using it.
// TODO: change the interface to this method to avoid exposing the templated object.
#if defined(EA_COMPILER_MSVC)
#pragma warning(push)
#pragma warning(disable : 4251) // disables warning: "Y::X" : class "X" needs to have dll-interface to be used by clients of class "Y"
#endif


namespace rw {
namespace core {
namespace filesys {

class Device;
class Handle;

///! Maximum length of a device name.
const unsigned int MAX_DEVICE_NAME_LENGTH = 16;

struct RWCORELIB_API DeviceRoot : public eastl::intrusive_list_node
{
    char name[MAX_DEVICE_NAME_LENGTH];
};

/*!
    \brief DeviceDriver is an abstract class that specifies the interface
    between the RwCore FileSys library and a device. A device
    driver is a service that translates library I/O requests into
    some device-specific calls and returns a result code.

    A device may be physical, as in the case of a DVD device
    driver, or it may be virtual, as in the case of a Zip file
    device.  Applications can use this interface to write
    communication drivers too.  This allows a single point-of-
    entry to the file system (AsyncOp::Open).

    To create a device driver, derive your own class from
    DeviceDriver and implement the pure virtual methods. Depending
    on the characteristics of your device, you may also implement
    the other virtual methods.

    It is not necessary for any methods in a driver to be thread-
    safe or non-blocking, although they do need to be reentrant.
    Every function is called from a high-priority device thread
    which is only active when there is a request to service.
*/
class RWCORELIB_API DeviceDriver
{
public:
    /*!
        \brief Default constructor.  After the object is instantiated,
        one or more roots must be added with AddRoot(const char *).
        \sa AddRoot
        \importlib rwfilesystem
    */
    DeviceDriver();

    /*!
        \brief Specialized constructor that takes an initial root
        directory.  This version can only be used to instantiate
        DeviceDrivers after the filesys Manager object has been
        created.  Like the default constructor, more roots may be
        added after the object is created with AddRoot.
    */
    DeviceDriver(const char *root);

    virtual ~DeviceDriver() {}

    /*!
        \brief Seek locations
        \importlib rwfilesystem
    */
    enum SeekWhence
    {
        FSEEK_BEGIN,        ///< seek from the beginning of the file
        FSEEK_CURRENT,      ///< seek from the file's current offset
        FSEEK_END           ///< seek from the file's EOF marker
    };

    /*!
        \brief The initialization function is called by the library when
        the device is used for the first time.  If your device
        needs to do anything to prepare itself before use, override the
        default Init function.

        \importlib rwfilesystem
    */
    virtual bool Init() { return true; }

    /*!
        \brief The library calls the devices' Restore method when the
        device is unregistered.  Any resources the device driver
        has allocated should be released at this time.

        \importlib rwfilesystem
    */
    virtual void Restore();

    /*!
        \brief The open function opens a file on the device

        \param name device specific file name identifying the file
                    the user would like to open.

        \param oflags a bit-wise combination of 0 or more
        AsyncOp::FOPEN_FLAGS.  A driver may respect as many of these
        flags as it wishes.  A Read-only file device driver would
        not respect a request to open a file in writable mode and
        should return NULL if such a request is made.

        \param pParentFileHandle used in virtual file
        systems.  If a virtual file contains a subfile which is
        being opened, the device driver returns the handle of the
        container file in the pParentFileHandle parameter.  To support
        subclassing a physical device which overrides the Open()
        function (for example, to insert an additional mode flag),
        the low-order bit can be set.  This will tell rwfilesystem
        to use the same handle but the parent device.  Refer to the
        DLC example to see this in action.

        \importlib rwfilesystem

        \return returns a
        handle which can be used to identify the file stream.  If
        the file cannot be found, the open function should return;
        NULL to the caller.
    */
    virtual void* Open(const char *name, unsigned int oflags, Handle **pParentFileHandle) = 0;

    /*!
        \brief The close function closes a file on the device and releases
        any resources allocated to that handle.  If the device is a
        virtual device, it is recommended that it check that no
        subfiles are open inside it.

        \param h file handle to be closed

        \importlib rwfilesystem
    */
    virtual void Close(void * /*h*/) {} // deprecated
    virtual void Close(void * h, DeviceDriver * /*ddParent*/, void * /*ddFileHandle*/) { Close(h); } // to be made pure virtual

	virtual void Flush(void * /*h*/) {}

    /*!
        \brief The read function reads the given number of bytes from the
        file to the given buffer.  In the case of a virtual file,
        the parent device driver and file handle are passed in by
        the file library.  In non-virtual devices, the ddParent
        and ddFileHandle parameters can be ignored.

        \importlib rwfilesystem

        \return number of bytes read from the input stream.  If the driver
        returns UINT32_MAX, rwfilesystem will move the operation to the driver's
        ReadProcessDevice and call ExecuteProcessingFunction.  This allows the
        data to be processed on a different thread while the I/O thread
        schedules another read.  This may be a good strategy for compressed files
        if you want them to be decompressed in the driver.

        \param h valid open device handle
        \param buf buffer that will hold the read data
        \param bufsize number of bytes to read into buf
        \param ddParent virtual devices only, parent file handle
        \param ddFileHandle virtual devices only
    */
    virtual unsigned int Read(void *h, void *buf, unsigned int bufsize, DeviceDriver *ddParent, void * ddFileHandle) = 0;

    /*!
        \brief The write function writes the given number of bytes from
        the buffer to the given file stream.  It is not necessary
        for a device driver to provide an implementation for this
        method.

        \importlib rwfilesystem

        \return number of bytes written to the output stream.
    */
    virtual unsigned int Write(void *h, const void *buf, unsigned int bufsize, DeviceDriver *dd, void *dfh)
    {
        (void)h;
        (void)buf;
        (void)bufsize;
        (void)dd;
        (void)dfh;
        return 0;
    }

    /*!
        \brief The seek function changes the current file streams'
        read/write position.  On devices where seeking is not
        possible (e.g. communication streams), the driver should
        raise an abort exception unless the seek position is equal
        to the current position.

        \importlib rwfilesystem

        \return new location in bytes in the file stream
        \param h      valid open device handle
        \param offset offset in bytes to seek to
        \param whence DeviceDriver::SeekWhence value identifying the starting
                    location to seek from
        \param ddParent virtual devices only
        \param ddFileHandle virtual devices only
    */
    virtual uint64_t Seek(void *h, uint64_t offset, int whence, DeviceDriver *ddParent, void *ddFileHandle) = 0;

    /*!
        \brief GetSize returns the size of the file in bytes.

        \importlib rwfilesystem

        \return size of file in bytes
        \param h valid open device handle
    */
    virtual uint64_t GetSize(void * /*h*/) { return 0; } // deprecated
    virtual uint64_t GetSize(void *h, DeviceDriver * /*ddParent*/, void * /*ddFileHandle*/) { return GetSize(h); } // to be made pure virtual

    /*!
        \brief Resizes the file to the given size.
        Can be used to truncate or extend a file.  If the file is extended, the
        contents between the old end-of-file and the new one are
        undefined.

        \importlib rwfilesystem

        \return true on success
    */
    virtual bool Resize(void *h, uint64_t newsize)
    {
        (void)h;
        (void)newsize;
        return false;
    }

    /*!
        \brief Returns a physical offset to the file on a Read-Only File
        System.  This can be useful for optimizating the order of
        file loads at runtime by sorting loads in the order they
        are located on the disc, thereby reducing seek times.

        \importlib rwfilesystem

        \return physical offset of the file on a Read-Only file system
    */
    virtual uint64_t QueryLocation(void *)
    {
        return 0;
    }

    /*!
        \brief Returns an optimal read size for the device.
        If the library receives a request to read more data that this
        amount, it will break the reads into chunks of this size.

        This can be useful for providing timely and well-
        coordinated access to a slow device.

        \importlib rwfilesystem

        \return the optimal read size for the device
    */
    virtual unsigned int GetMaxReadSize()
    {
        return 0xffffffffu;
    }

    /*!
        \brief The delete function deletes a file on the device.
        If the file cannot be deleted, the delete function should return;
        false to the caller.

        \importlib rwfilesystem

        \return false on failure
    */
    virtual bool Delete(const char *)
    {
        return false;
    }

    /*!
        \brief The Move function moves an existing file or a directory, including
        its children.  If the file or directory is moved to the same location,
        it is renamed.

        \importlib rwfilesystem

        \return false on failure
    */
    virtual bool Move(const char *, const char *)
    {
        return false;
    }

    /*!
        \brief Returns the ReadProcessing function context.  A derived device
        driver may choose to implement this function if data processing is to
        occur on a separate device from where the read was performed.
        This function will be called by rwfilesystem immediately following the
        return of UINT32_MAX.  This context pointer is intended to be used to
        pass any parameters that are needed by the ExecuteProcessingFunction.

        \importlib rwfilesystem
    */
    virtual void *GetReadProcessingContext()
    {
        return NULL;
    }

    /*!
        \brief Execute read processing.  This function is called from the
        device specified by ReadProcessDevice.  It should be used to perform
        expensive processing on the data, such as decompression.  While
        such processing can be done in the driver Read function, I/O will
        be blocked until the processing is complete.  By moving the processing
        to this function, the I/O thread can start another read while
        data that is already read in is processed.

        \param pContext This context pointer is the same one that was
        returned by GetReadProcessingContext.
        \param nread Return the number of bytes read to rwfilesystem through
        this parameter.

        \return At this time, this function should always return 0.  In a
        future release, the return value may indicate an error or some
        further action to be taken.
    */
    virtual int ExecuteProcessingFunction(void *pContext, unsigned int *nread)
    {
        (void)pContext;
        (void)nread;

        return 0;
    }

    /*!
        \brief Returns a device on which read processing can occur.  Implement
        this function to allow rwfilesystem to schedule expensive tasks that
        would otherwise block the I/O thread.

        \return A device to run the data decode function on.
    */
    virtual Device *ReadProcessDevice()
    {
        return NULL;
    }

    /*!
        \brief Structure used to describe the files found on the device.
        The driver should fill in this structure when its
        FindFirstFile/FindNextFile functions are called.
        \importlib rwfilesystem
    */
    struct FindData
    {
        unsigned int mFlags;        ///< Platform-dependent bitflags.
        uint64_t     mCreationTime;     ///< The time the file was created.
        uint64_t     mAccessTime;       ///< The time the file was last accessed.
        uint64_t     mModificationTime;     ///< The last modification time of the file.
        uint64_t     mSize;     ///< The size in bytes of the file.
        char         mName[FILESYS_MAX_PATH];       ///< The name of the file or directory.
    };

    /*!
        \brief This function searches a directory for a file or subdirectory whose name
        matches the specified file name. It should return a handle to use for
        future GetNextFile calls.

        \importlib rwfilesystem

        \return handle for use in future FindNext calls
    */
    virtual void* FindBegin(const char *, FindData *)
    {
        return NULL;
    }

    /*!
        \brief This function continues a file search from a previous call to the
        GetFirstFile function.

        \importlib rwfilesystem

        \return false on failure to find any more files
    */
    virtual bool FindNext(void *, FindData *)
    {
        return false;
    }

    /*!
        \brief This function closes the specified search handle. The GetFirstFile and
        GetNextFile functions use the search handle to locate files with names
        that match a specified name.

        \importlib rwfilesystem

        \return false on failure
    */
    virtual bool FindEnd(void *)
    {
        return false;
    }

    /*!
        \brief The DirectoryCreate function creates a new directory on the device.

        \importlib rwfilesystem

        \return false on failure
    */
    virtual bool DirectoryCreate(const char *)
    {
        return false;
    }

    /*!
        \brief The DirectoryRemove function deletes a directory on the device.

        \importlib rwfilesystem

        \return false on failure
    */
    virtual bool DirectoryRemove(const char *)
    {
        return false;
    }

    /*!
        \brief Returns the name of the file for the given handle.  Drivers
        may implement this function to return the name of the file for the
        given handle.  It is up to the driver to store that name internally.

        \importlib rwfilesystem

        \param handle This is the handle returned by your open function.
        \param name The buffer to copy the name into.
        \param nameBufferSize The size of the name buffer.  Do not copy
        more characters than this into the name buffer.

        \return True if the handle is valid and a name returned.  False
        if the handle is invalid or the driver does not store the file names.
    */
    virtual bool GetNameFromHandle(void *handle, char *name, size_t nameBufferSize)
    {
        (void)handle;
        (void)name;
        (void)nameBufferSize;

        return false;
    }

    /*!
        \brief Returns the name of the device as a string.
        Your driver does not need to implement this function, it is just for
        the library's use.

        \importlib rwfilesystem

        \return name of the device as a string
    */
    const char* GetName() const;

    const eastl::intrusive_list<DeviceRoot> &GetRootList() const
    {
        return mRootList;
    }

    /*!
        \brief Add a new root to this device.  Because this allocates
        memory, it must be done after the filesys Manager object is
        created.
    */
    void AddRoot(const char *root);

protected:
    eastl::intrusive_list<DeviceRoot> mRootList;
};

}}}

#if defined(EA_COMPILER_MSVC)
#pragma warning(pop)
#endif

#endif
