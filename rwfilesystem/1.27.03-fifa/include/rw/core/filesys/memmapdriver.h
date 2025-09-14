// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_CORE_FILESYS_MEM_MAP_DRIVER_H
#define RW_CORE_FILESYS_MEM_MAP_DRIVER_H

#include "rw/core/filesys/config.h"
#include "rw/core/filesys/manager.h"
#include "rw/core/filesys/device.h"
#include "rw/core/filesys/async.h"

namespace EA { namespace Allocator { class ICoreAllocator; } }

namespace rw {
namespace core {
namespace filesys {

class MemMapDeviceDriverImpl;

/*!
 * \brief The Memory Mapped Device Driver class
 *
 * The Memory Mapped device driver is a RAM disk interface for the rwfilesystem
 * library. It is able to work with any memory that an object that implements
 * the MemMapDeviceDriver::IStorageTransfer interface is able to transfer between.
*/
class RWCORELIB_API MemMapDeviceDriver : public rw::core::filesys::DeviceDriver
{
public:

    /*!
     * \brief The interface used by MemMapDeviceDriver to facilitate data transfer in memory.
     *
     * An implementation of the IStorageTransfer interface is required for using the
     * MemMapDeviceDriver. This may be as simple as using a memcpy or something more
     * elaborate. There are four things to keep in mind when implementing this interface:
     * first, the transfer blocks will not overlap; second, it will occasionally do transfers
     * within the allocated foreign pool; third, all transfers will be the size of the pages
     * specified in the options structure; four, if data transfers are asynchronous then
     * Transfer must block until they are done. For example, if an implementation of IStorageTransfer
     * is used to move data between the PlayStation 3's RSX local memory and main memory,
     * it must also be able to transfer data from one location in RSX local memory to another
     * location in RSX local memory.
     */
    struct IStorageTransfer
    {
        virtual void Transfer( void* dst, const void* src, size_t size ) = 0;
        virtual ~IStorageTransfer() {}
    };

    /*!
     * \brief The configuration options for the MemMapDeviceDriver class.
     */
    struct Options
    {
        /*!
         * The constructor for the MemMapDeviceDriver::Options structure
         *
         * \param pa An object that implements the EA::Allocator::ICoreAllocator interface
         *           which is used for managing allocations within the pool of memory that
         *           the MemMapDeviceDriver operates in
         * \param ist An object that implements the MemMapDeviceDriver::IStorageTransfer
         *            interface. This is used for transferring data between sections of
         *            memory.
         * \param sizeOfPages The size of the pages to use. These must be a power of two and
         *                    it is recommended that pages be at least 4096 bytes in size.
         */
        Options( EA::Allocator::ICoreAllocator& pa, IStorageTransfer& ist, int sizeOfPages )
            : poolAllocator( pa ), storageTransfer( ist ), pageSize( sizeOfPages )
        {}

        EA::Allocator::ICoreAllocator& poolAllocator;
        IStorageTransfer& storageTransfer;
        const int pageSize;

    private:
        Options& operator=( const Options& );
        Options( const Options& );
    };

    MemMapDeviceDriver( const Options& ops );
    virtual ~MemMapDeviceDriver();

    /*!
     * \brief Releases unused memory, if possible.
     *
     * If memory is unused and can be de-allocated (for example, if a file has
     * been deleted on the device), then calling this method will release it
     * back to the allocator for other uses.
     *
     * \return The number of bytes freed
     */
    unsigned int ReleaseMemory();

    /*!
     * \brief Initiates a format of the device and frees all file resources.
     *
     * This will delete all the files on the device and free any memory held
     * by the driver, save for the internal scratch buffer.
     *
     * \return True on success, false on failure
     */
    bool Format();

    void* operator new(size_t size)
    {
        return Manager::GetAllocator()->Alloc(size, "MemMapDeviceDriver", EA::Allocator::MEM_PERM);
    }

    void* operator new [](size_t size)
    {
        return Manager::GetAllocator()->Alloc(size, "MemMapDeviceDriver", EA::Allocator::MEM_PERM);
    }

    void* operator new(size_t, void* ptr)
    {
        return ptr;
    }

	void operator delete(void*, void *)
	{
	}

    void operator delete(void* ptr)
    {
        if (ptr)
            Manager::GetAllocator()->Free(ptr);
    }

    void operator delete [](void* ptr)
    {
        if (ptr)
            Manager::GetAllocator()->Free(ptr);
    }

private:
    virtual void* Open( const char *name, unsigned int oFlags, rw::core::filesys::Handle **pParentFileHandle );
    virtual void Close( void* h, DeviceDriver *ddParent, void *ddFileHandle );
    using        DeviceDriver::Close;

    virtual unsigned int Read( void *h, void *buf, unsigned int bufSize, DeviceDriver *ddParent, void * ddFileHandle );
    virtual unsigned int Write( void *h, const void * buf, unsigned int bufSize, DeviceDriver * ddParent, void * ddFileHandle );

    virtual uint64_t Seek( void *h, uint64_t offset, int whence, DeviceDriver *ddParent, void *ddFileHandle );
    virtual uint64_t GetSize( void *h, DeviceDriver *ddParent, void *ddFileHandle );
    using            DeviceDriver::GetSize;

    virtual bool Move( const char* from, const char* to );
    virtual bool Delete( const char* name );

    virtual bool DirectoryCreate( const char * ) { return true; }

    MemMapDeviceDriverImpl* mImpl;

    // this class is non-copyable
    MemMapDeviceDriver( const MemMapDeviceDriver& );
    MemMapDeviceDriver& operator=( const MemMapDeviceDriver& );
};

}}}

#endif
