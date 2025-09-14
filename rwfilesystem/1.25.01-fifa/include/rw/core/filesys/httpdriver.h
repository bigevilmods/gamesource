// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_CORE_FILESYS_HTTP_DEVICE_DRIVER_H
#define RW_CORE_FILESYS_HTTP_DEVICE_DRIVER_H

#include "rw/core/filesys/config.h"
#include "rw/core/filesys/device.h"
#include "rw/core/filesys/ihttpdirectoryhandler.h"

namespace rw {
namespace core {
namespace filesys {

// It is recommended by DirtySDK that the internal buffer size be in the range of 4-8k, as smaller sizes can impact 
// performance and larger sizes improve performance on a scale of diminishing returns. The created buffer size is 
// clamped to a minimum of 4k by ProtoHttp. However experimenting has shown sizes of up to 64 - 128K to provide 
// better reliability when streaming lags are more noticable, such as with video
const int32_t DEFAULT_BUFFER_SIZE = 8*1024;

// From DirtySDK
//! maximum number of protohttp refs that can be allocated
const int32_t DEFAULT_POOL_SIZE = 8;
const int32_t MINIMUM_POOL_SIZE = 3;

const int32_t DEFAULT_TIMEOUT = 10000; // In milliseconds

class HttpDeviceDriverImpl;

enum HttpOpError
{
    HTTPERROR_SUCCESS = 0xFEED0FED,
    HTTPERROR_TIMEOUT = 0xFEEDDEAD
};

/*!
 * \brief The HTTP Driver class
 *
*/
class RWCORELIB_API HttpDeviceDriver : public DeviceDriver
{
public:
    struct Options
    {
        /*!
         * The constructor for the HttpDeviceDriver::Options structure
         *
         * \param timeout    Sets ProtoHttp client timeout in milliseconds
         */
        Options(int32_t httpBufferSize, int32_t timeout = DEFAULT_TIMEOUT, int32_t poolSize = DEFAULT_POOL_SIZE, const char* serverName = NULL)
            : mTimeout( timeout )
            , mHttpBufferSize( httpBufferSize)
            , mHttpStatePoolSize(poolSize < MINIMUM_POOL_SIZE ? MINIMUM_POOL_SIZE : poolSize)
            , mServerString(serverName)
            , mSemaphoreParams(mHttpStatePoolSize, true, "HttpRefPoolSemaphore")
        {}

        Options()
            : mTimeout(DEFAULT_TIMEOUT)
            , mHttpBufferSize(DEFAULT_BUFFER_SIZE)
            , mHttpStatePoolSize(DEFAULT_POOL_SIZE)
            , mServerString(NULL)
            , mSemaphoreParams(mHttpStatePoolSize, true, "HttpRefPoolSemaphore")
        {}

        int32_t                         mTimeout;
        int32_t                         mHttpBufferSize;
        int32_t                         mHttpStatePoolSize;
        const char*                     mServerString;
        EA::Thread::SemaphoreParameters mSemaphoreParams;
        
    private:
        Options& operator=( const Options& );
        Options( const Options& );
    };

    HttpDeviceDriver(const Options& ops);
    virtual ~HttpDeviceDriver();

    void* operator new(size_t size);
    void* operator new [](size_t size);
    void* operator new(size_t, void* ptr);
    void operator delete(void*, void *);
    void operator delete(void* ptr);
    void operator delete [](void* ptr);

    virtual bool           Init();
    virtual void           Shutdown();

    void                   SetDirectoryHandler(IHttpDirectoryHandler* pHandler);

private:

    virtual void*          Open(const char *name, unsigned int oFlags, rw::core::filesys::Handle **pParentFileHandle);
    virtual void           Close(void* h);
    virtual void           Close(void* h, DeviceDriver *ddParent, void *ddFileHandle) { Close(h); }
    virtual unsigned int   Read(void *h, void *buf, unsigned int bufSize, DeviceDriver *ddParent, void * ddFileHandle);
    virtual unsigned int   Write(void *h, const void * buf, unsigned int bufSize, DeviceDriver * ddParent, void * ddFileHandle);
    virtual bool           Delete(const char* name);
    virtual uint64_t       Seek(void *h, uint64_t offset, int whence, DeviceDriver *ddParent, void *ddFileHandle);
    virtual uint64_t       GetSize(void *h);
    virtual uint64_t       GetSize(void *h, DeviceDriver *ddParent, void *ddFileHandle) { return GetSize(h); }
    virtual bool           Exists(const char* name);
    virtual bool           Move(const char* from, const char* to);
    virtual unsigned int   GetMaxReadSize();

    // Directory Functions
    virtual void*          FindBegin(const char *filePattern, DeviceDriver::FindData *findData);
    virtual bool           FindNext(void *h, DeviceDriver::FindData *findData);
    virtual bool           FindEnd(void *h);
    virtual bool           DirectoryCreate(const char *directory);
    virtual bool           DirectoryRemove(const char *directory);

    HttpDeviceDriverImpl*  mImpl;
    IHttpDirectoryHandler* mDirectoryHandler;

    bool                   bInitialized;
    int32_t                mTimeout;
    int32_t                mBufferSize;
    bool                   mIsDirtySockStartedHere;    

    // This class is non-copyable
    HttpDeviceDriver( const HttpDeviceDriver& );
    HttpDeviceDriver& operator=( const HttpDeviceDriver& );
};

}}}

#endif
