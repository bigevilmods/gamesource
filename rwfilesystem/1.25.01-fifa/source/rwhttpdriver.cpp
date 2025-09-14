// (c) Electronic Arts. All Rights Reserved.

#include "EABase/eabase.h"
#include "EAAssert/eaassert.h"
#include "EAStdC/EASprintf.h"
#include "EAStdC/EAString.h"
#include "EAStdC/EAMemory.h"
#include "EASTL/intrusive_list.h"
#include "eathread/eathread_semaphore.h"
#include "eathread/eathread_mutex.h"
#include "rw/core/filesys/device.h"
#include "rw/core/filesys/httpdriver.h"

#include "protohttp.h"
#include "protostream.h"
#include "dirtylib.h"
#include "dirtyvers.h"
#include "netconn.h"



#if defined(EA_PLATFORM_XBOXONE) && defined(DO_PROFILE_PIX)
#include "pix.h"
#define DS_MARKER_UNICODE_HELPER(x) L ## x
#define DS_MARKER_BEGIN(x) PIXBeginEvent(1, DS_MARKER_UNICODE_HELPER(x))
#define DS_MARKER_END() PIXEndEvent()
#else
#define DS_MARKER_BEGIN(x)
#define DS_MARKER_END()

#endif

#ifndef PROTOSTREAM_MINBUFFER
    #define PROTOSTREAM_MINBUFFER 4*1024
#endif

#define HTTP_DEFINE_CLASS_NEW_AND_DELETE(name, flags)                                                   \
    void* operator new    (size_t size) { return Manager::GetAllocator()->Alloc(size, name, flags); }   \
    void* operator new [] (size_t size) { return Manager::GetAllocator()->Alloc(size, name, flags); }   \
    void* operator new    (size_t, void* ptr) { return ptr; }                                           \
    void operator delete    (void* ptr) { if (ptr) Manager::GetAllocator()->Free(ptr); }                \
    void operator delete [] (void* ptr) { if (ptr) Manager::GetAllocator()->Free(ptr); }

namespace rw {
namespace core {
namespace filesys {

const uint32_t MAX_APPENDED_HEADER_LENGTH = 255;
const uint32_t MAX_HEADER_LENGTH = 1024;
const uint32_t MAX_ATTEMPTS = 8;
const uint32_t DOWNLOAD_STATE_REF_POOL_INDEX = 0;
const uint32_t UPLOAD_STATE_REF_POOL_INDEX = 1;
const int32_t  HEAD_REQUEST_BUFFER_SIZE = 4*1024;

extern "C" void *DirtyMemAlloc(int32_t iSize, int32_t iMemModule, int32_t iMemGroup, void *pMemGroupUserData);
extern "C" void DirtyMemFree(void *pMem, int32_t iMemModule, int32_t iMemGroup, void *pMemGroupUserData);

void QualifyUrl(char* qualifiedUrl, const char* url, const char* serverString)
{
    size_t hostLen = EA::StdC::Strlen(serverString);
	EA_ASSERT_MSG(hostLen + EA::StdC::Strlen(url) < (size_t)FILESYS_MAX_PATH, "url length is too large");

    // Path is relative to the serverString hostname, so we fully qualify it internally
    if( EA::StdC::Strstr(url, "http:/") == url && url[6] != '/')
    {
    	url += 6;
        EA::StdC::Snprintf(qualifiedUrl, FILESYS_MAX_PATH, "%s%s%s\0",
        	serverString,
        	serverString[hostLen-1] == '/' ? "" : "/",
        	url );
    }
    else
    {
        // Use the fully qualified URL which was passed in
        EA::StdC::Strlcpy(qualifiedUrl, url, FILESYS_MAX_PATH);
    }
}

// This is the default directory handler if none is supplied to by the user after creating a HttpDeviceDriver
static class NullDirectoryHandler : public IHttpDirectoryHandler
{
public:
    void* FindBegin(const char *filePattern, DeviceDriver::FindData *findData) { EA_FAIL_MESSAGE("Performing directory access with the default NullDirectoryHandler"); return NULL; }
    bool  FindNext(void *h, DeviceDriver::FindData *findData) { EA_FAIL_MESSAGE("Performing directory access with the default NullDirectoryHandler"); return false; }
    bool  FindEnd(void *h) { EA_FAIL_MESSAGE("Performing directory access with the default NullDirectoryHandler"); return false; }
    bool  DirectoryCreate(const char *directory) { EA_FAIL_MESSAGE("Performing directory access with the default NullDirectoryHandler"); return false; }
    bool  DirectoryRemove(const char *directory) { EA_FAIL_MESSAGE("Performing directory access with the default NullDirectoryHandler"); return false; }
} sNullDirectoryHandler;

//----------------------------------------------------------------------
// This is the object that is returned to the user whenever a Http file handle
// is requested.
struct HttpHandle
{
    HttpHandle(const char* serverString, const char* url, int32_t bufferSize)
        : mCurrentPosition(0)
        , mFileSize(0)
    {
        QualifyUrl(mFileUrl, url, serverString);
    }

    ~HttpHandle()
    {
    }

    HTTP_DEFINE_CLASS_NEW_AND_DELETE("HttpHandle", EA::Allocator::MEM_TEMP);

    uint64_t       mCurrentPosition;
    uint64_t       mFileSize;
    char           mFileUrl[FILESYS_MAX_PATH];

private:
    HttpHandle& operator=( const HttpHandle& h );
    HttpHandle( const HttpHandle& h );
};


struct HttpRef : eastl::intrusive_list_node
{
    ProtoHttpRefT* mRef;
    HttpRef() : mRef(NULL) {};
    HttpRef(ProtoHttpRefT* ref) : mRef(ref) {};
};

//----------------------------------------------------------------------
class HttpDeviceDriverImpl
{
public:

    enum
    {
        PROTOHTTP_SUCCESS        =  0,
        PROTOHTTP_STATUS_PENDING =  0,
        PROTOHTTP_ERROR          = -1
    };

    HttpDeviceDriverImpl(const HttpDeviceDriver::Options& ops)
        : mRefPoolSemaphore(&ops.mSemaphoreParams)
    {
        mHttpBufferSize = ops.mHttpBufferSize <= 0 ? DEFAULT_BUFFER_SIZE : ops.mHttpBufferSize;
        mHttpStatePoolSize = ops.mHttpStatePoolSize <= 0 ? DEFAULT_POOL_SIZE : ops.mHttpStatePoolSize;
        mTimeout = ops.mTimeout;
		EA::StdC::Memset8(mServerString, 0, sizeof(mServerString));

        if(ops.mServerString != NULL)
        {
            EA::StdC::Strlcpy(mServerString, ops.mServerString, sizeof(mServerString));
        }

        for(int32_t i = 0; i < mHttpStatePoolSize; ++i)
        {
            mRefPool[i] = NULL;
        }

        mRefFreeList.clear();
    }

    ~HttpDeviceDriverImpl()
    {
    }

    HTTP_DEFINE_CLASS_NEW_AND_DELETE("HttpDeviceDriverImpl", EA::Allocator::MEM_PERM);

    bool                           Init();
    void                           Shutdown();
    bool                           FileExists(HttpHandle* handle);

    // pImpl DeviceDriver functions
    HttpHandle*                    Open(const char *name);
    void                           Close(HttpHandle* handle);
    uint32_t                       Download(HttpHandle* pHandle, void* pData, uint32_t bufSize);
    uint32_t                       Upload(HttpHandle* pHandle, const char* pData, uint32_t bufSize);
    uint64_t                       GetSize(HttpHandle* handle);
    bool                           DeleteFile(HttpHandle* handle);

    char                           mServerString[FILESYS_MAX_PATH];

private:
    HttpRef*                       AcquireStateHandle();
    void                           ReleaseStateHandle(HttpRef* ref);

    int32_t                        mHttpBufferSize;
    int32_t                        mHttpStatePoolSize;
    int32_t                        mTimeout;
    
    EA::Thread::Mutex              mFreeListMutex;
    EA::Thread::Semaphore          mRefPoolSemaphore;
    HttpRef                        mRefPool[DEFAULT_POOL_SIZE];
    eastl::intrusive_list<HttpRef> mRefFreeList;

    HttpDeviceDriverImpl& operator=(const HttpDeviceDriverImpl&);
    HttpDeviceDriverImpl(const HttpDeviceDriverImpl&);
};

bool HttpDeviceDriverImpl::Init()
{
    for(int32_t i = 0; i < mHttpStatePoolSize; ++i)
    {
        if(i == static_cast<int>(DOWNLOAD_STATE_REF_POOL_INDEX) || i == static_cast<int>(UPLOAD_STATE_REF_POOL_INDEX))
        {
            mRefPool[i] = HttpRef(ProtoHttpCreate(mHttpBufferSize));
        }
        else
        {
            mRefPool[i] = HttpRef(ProtoHttpCreate(HEAD_REQUEST_BUFFER_SIZE));
        }
        ProtoHttpControl(mRefPool[i].mRef, 'keep', 1, 0 , NULL);
        ProtoHttpControl(mRefPool[i].mRef, 'spam', 0, 0 , NULL);
        ProtoHttpControl(mRefPool[i].mRef, 'time', mTimeout, 0 , NULL);
    }

    // we reserve the first two connections for uploading and downloading
    for(int32_t i = 2; i < mHttpStatePoolSize; ++i)
    {
        mRefFreeList.push_back(mRefPool[i]);
    }
	
    return true;
}

void HttpDeviceDriverImpl::Shutdown()
{
    for(int32_t i = 0; i < mHttpStatePoolSize; ++i)
    {
        ProtoHttpRefT*& pRef = mRefPool[i].mRef;
        if(pRef)
        {
            ProtoHttpAbort(pRef);
            ProtoHttpDestroy(pRef);
            pRef = NULL;
        }
    }
    mRefFreeList.clear();
}

HttpRef* HttpDeviceDriverImpl::AcquireStateHandle()
{
    mRefPoolSemaphore.Wait();
    EA_ASSERT(mRefFreeList.size() > 0);
    HttpRef* ref;
    {
        EA::Thread::AutoMutex mutex(mFreeListMutex);
        ref = &mRefFreeList.front();
        mRefFreeList.pop_front();
    }

    return ref;
}

void HttpDeviceDriverImpl::ReleaseStateHandle(HttpRef* ref)
{
    {
        EA::Thread::AutoMutex mutex(mFreeListMutex);
        mRefFreeList.push_front(*ref);
    }
    mRefPoolSemaphore.Post();
}

//HttpDeviceDriverImpl
bool HttpDeviceDriverImpl::FileExists(HttpHandle* pHandle)
{
    if(pHandle == NULL)
        return false;

    int32_t code = 0;
    int32_t httpResult = 0;
    bool returnResult = false;
    HttpRef* ref = AcquireStateHandle();
    ProtoHttpRefT* state = ref->mRef;
    uint32_t attempt = 0;

    // Forces the header to be cleared
    ProtoHttpControl(state, 'apnd', 0, 0, NULL);

    while(attempt++ < MAX_ATTEMPTS)
    {
        httpResult = ProtoHttpGet(state, pHandle->mFileUrl, true);

        if(httpResult < PROTOHTTP_SUCCESS)
        {
            returnResult = false;
            break;
        }

        while((httpResult = ProtoHttpStatus(state, 'done', NULL, 0)) == PROTOHTTP_STATUS_PENDING && ProtoHttpStatus(state, 'time', NULL, 0) == 0)
        {
            ProtoHttpUpdate(state);
            EA::Thread::ThreadSleep(1);
        }

        if(httpResult == PROTOHTTP_ERROR)
		{

            ProtoHttpAbort(state);
			continue;
		}

        code = ProtoHttpStatus(state, 'code', NULL, 0);

        // No content is ok because it just means the file is empty
        if(code != PROTOHTTP_RESPONSE_SUCCESSFUL && code != PROTOHTTP_RESPONSE_NOCONTENT)
        {
            returnResult = false;
            ProtoHttpAbort(state);
            break;
        }
        else
        {
            returnResult = true;
            break;
        }
    }

    ReleaseStateHandle(ref);

    return returnResult;
}

uint64_t HttpDeviceDriverImpl::GetSize(HttpHandle* pHandle)
{
    if(pHandle == NULL)
        return 0;

    // Return whatever size we found when we opened the file. We can't get around
    // the problem of a different user changing the same remote resource simultaneously, so we stick with
    // what we know.
    if(pHandle->mFileSize != 0)
        return pHandle->mFileSize;

    const char* url = pHandle->mFileUrl;
    int32_t result = 0;
    uint64_t returnSize = 0;
    char headerBuf[MAX_HEADER_LENGTH];
    char fileSizeBuf[64];
    int32_t code = 0;

    uint32_t attempt = 0;
    HttpRef* ref = AcquireStateHandle();
    ProtoHttpRefT* state = ref->mRef;

    // Forces the header to be cleared
    ProtoHttpControl(state, 'apnd', 0, 0, NULL);

    while(attempt++ < MAX_ATTEMPTS)
    {
        result = ProtoHttpGet(state, url, true);

        if(result < PROTOHTTP_SUCCESS)
        {
            returnSize = 0;
            break;
        }
        
        //if we haven't received the code yet so we wait for it.
        while((result = ProtoHttpStatus(state, 'done', NULL, 0)) == PROTOHTTP_STATUS_PENDING && ProtoHttpStatus(state, 'time', NULL, 0) == 0)
        {
            ProtoHttpUpdate(state);
            EA::Thread::ThreadSleep(1);
        }

        if(result == PROTOHTTP_ERROR)
		{
            // let's try again
            ProtoHttpAbort(state);
			continue;
		}

        code = ProtoHttpStatus(state, 'code', NULL, 0);

        if(code != PROTOHTTP_RESPONSE_SUCCESSFUL)
        {
            returnSize = 0;
            break;
        }
        
        result = ProtoHttpStatus(state, 'htxt', headerBuf, MAX_HEADER_LENGTH);

        // Did we fail to get the header
        if(result < 0)
        {
            returnSize = 0;
            break;
        }

        result = ProtoHttpGetHeaderValue(NULL, headerBuf, "content-length", fileSizeBuf, sizeof(fileSizeBuf), NULL);

        if(result < 0)
        {
            returnSize = 0;
            break;
        }

        returnSize = EA::StdC::AtoU64(fileSizeBuf);
        pHandle->mFileSize = returnSize;
        break;
    }

    ReleaseStateHandle(ref);

    return returnSize;
}

uint32_t HttpDeviceDriverImpl::Upload(HttpHandle* pHandle, const char* pData, uint32_t dataSize)
{
    if(pHandle == NULL || pData == NULL)
        return 0;

    uint32_t dataSent = 0;
    int32_t sentBlock = 0;
    ProtoHttpRefT* state = mRefPool[UPLOAD_STATE_REF_POOL_INDEX].mRef;
    char8_t range[MAX_APPENDED_HEADER_LENGTH];

    EA::StdC::Snprintf(range, MAX_APPENDED_HEADER_LENGTH, "content-range: bytes %" PRIu64 "-%" PRIu64 "/", pHandle->mCurrentPosition, pHandle->mCurrentPosition+dataSize);
    ProtoHttpControl(state, 'apnd', 0, 0, range);

    //Initiates a PUT request to the server.
    sentBlock = ProtoHttpPost(state, pHandle->mFileUrl, pData, dataSize, true /*DoPut*/);
    ProtoHttpUpdate(state);
    if(sentBlock < 0)
    {
        ProtoHttpControl(state, 'apnd', 0, 0, NULL);
        ProtoHttpAbort(state);
        return 0;
    }
    else if(sentBlock == 0 && dataSize == 0)
    {
        while(ProtoHttpStatus(state, 'done', NULL, 0) <= PROTOHTTP_STATUS_PENDING  && ProtoHttpStatus(state, 'time', NULL, 0) == 0)
        {
            ProtoHttpUpdate(state);
            EA::Thread::ThreadSleep(1);
        }

        return 0;
    }

    dataSent += sentBlock;
    pHandle->mCurrentPosition += dataSent;

    do
    {
        sentBlock = ProtoHttpSend(state, pData, dataSize-dataSent);

        if(sentBlock >= PROTOHTTP_SUCCESS)
        {
            dataSent += sentBlock;
            pHandle->mCurrentPosition += sentBlock;
            pHandle->mFileSize = pHandle->mCurrentPosition > pHandle->mFileSize ? pHandle->mCurrentPosition : pHandle->mFileSize;
        }
        else
        {
            break;
        }

        ProtoHttpUpdate(state);
    }while((dataSent < dataSize || ProtoHttpStatus(state, 'done', NULL, 0) <= PROTOHTTP_STATUS_PENDING));

    // We failed to download everything we wanted after re-trying
    if(dataSent < dataSize)
    {
        Manager::GetInstance()->GetErrorHandler()(static_cast<int>(Manager::FILEERROR_WRITE), Manager::FILEERROR_WRITE, reinterpret_cast<rw::core::filesys::DeviceDriver*>(this), NULL);
    }

    // Forces the header to be cleared
    ProtoHttpControl(state, 'apnd', 0, 0, NULL);

    return dataSent;
}

uint32_t HttpDeviceDriverImpl::Download(HttpHandle* pHandle, void* pData, uint32_t bufSize)
{
    if(pHandle == NULL || pHandle->mFileSize == 0)
		return 0;

    int32_t result = 0;
    int32_t count = 0;
    uint32_t dataRecv = 0;
    char8_t range[MAX_APPENDED_HEADER_LENGTH];
    const char* pUrl = pHandle->mFileUrl;
    ProtoHttpRefT* state = mRefPool[DOWNLOAD_STATE_REF_POOL_INDEX].mRef;
    uint32_t attempt = 0;

	uint64_t endPos = pHandle->mCurrentPosition + bufSize;

	if(endPos > pHandle->mFileSize)
    {
		endPos = pHandle->mFileSize;
    }

	// We have to be clear about the end byte here, since we can't trust the server to properly correct an erroneously specified end byte for our chunk.
	EA::StdC::Snprintf(range, MAX_APPENDED_HEADER_LENGTH, "range: bytes=%" PRIu64 "-%" PRIu64, pHandle->mCurrentPosition, endPos-1);
    ProtoHttpControl(state, 'apnd', 0, 0, range);

    result = ProtoHttpGet(state, pUrl, false);

    // With the new DirtySDK we need to wait for the request to finish due to the underlying ProtoHttp implementation relying on MS
    // hat type(^) objects issuing callbacks to update the ProtoHTTPRef object's status otherwise we will prematurely abort the connection
    // kill the object and create a new object and connection request
    // On older versions of DirtySDK we won't have to wait for a callback, so waiting here will cause a hang since the GET request
    // will never finish since we won't be calling ProtoHttpRecv()
    #if defined EA_PLATFORM_XBOXONE && DIRTYVERS >= 1303010100
        while(ProtoHttpStatus(state, 'done', NULL, 0) <= PROTOHTTP_STATUS_PENDING  && ProtoHttpStatus(state, 'time', NULL, 0) == 0)
        {
            ProtoHttpUpdate(state);
            EA::Thread::ThreadSleep(1);
        }
    #endif

    
    if(result == PROTOHTTP_SUCCESS)
    {
        do
        {
			DS_MARKER_BEGIN("ProtoHttpUpdate");
            ProtoHttpUpdate(state);
			DS_MARKER_END();
            count = ProtoHttpRecv(state, (static_cast<char*>(pData) + dataRecv), 1, bufSize - dataRecv);

            if(count > 0)
            {
                pHandle->mCurrentPosition += count;
                dataRecv += count;
            }
            else if(count == PROTOHTTP_RECVFAIL)
            {
                // Retry getting data in place until we reach MAX_ATTEMPTS
                // at which point we return as much data as we've received so far
                attempt++;
                EA::StdC::Snprintf(range, MAX_APPENDED_HEADER_LENGTH, "range: bytes=%" PRIu64 "-%" PRIu64, pHandle->mCurrentPosition, endPos-1);
                ProtoHttpControl(state, 'apnd', 0, 0, range);
                result = ProtoHttpGet(state, pUrl, false);
            }

        }while(dataRecv < bufSize && count != PROTOHTTP_RECVDONE && attempt < MAX_ATTEMPTS);

        EA_ASSERT(dataRecv <= bufSize);
    }

    // If we request more than we can actually use, the connection will remain open (due to keep-alive) in between calls to download, waiting for more data to be fetched.
	EA_ASSERT(ProtoHttpStatus(state, 'done', NULL, 0) > 0);

    // We failed to download everything we wanted after re-trying
    if(attempt >= MAX_ATTEMPTS && dataRecv < bufSize)
    {
        Manager::GetInstance()->GetErrorHandler()(static_cast<int>(Manager::FILEERROR_READ), Manager::FILEERROR_READ, reinterpret_cast<rw::core::filesys::DeviceDriver*>(this), NULL);
    }

    // Forces the header to be cleared
    ProtoHttpControl(state, 'apnd', 0, 0, NULL);

    return dataRecv;
}

bool HttpDeviceDriverImpl::DeleteFile(HttpHandle* pHandle)
{
    if(pHandle == NULL)
        return 0;

    int32_t code = 0;
    int32_t httpResult;
    bool    deleteSuccess = false;

    uint32_t attempt = 0;
    HttpRef* ref = AcquireStateHandle();
    ProtoHttpRefT* state = ref->mRef;

    // Forces the header to be cleared
    ProtoHttpControl(state, 'apnd', 0, 0, NULL);

    while(attempt++ < MAX_ATTEMPTS)
    {
        httpResult = ProtoHttpDelete(state, pHandle->mFileUrl);

        if(httpResult == PROTOHTTP_SUCCESS)
        {
            // Get Status to see if the delete was successful
            while((httpResult = ProtoHttpStatus(state, 'done', NULL, 0)) == PROTOHTTP_STATUS_PENDING && ProtoHttpStatus(state, 'time', NULL, 0) == 0)
            {
                ProtoHttpUpdate(state);
                EA::Thread::ThreadSleep(1);
            }

            if(httpResult == PROTOHTTP_ERROR)
            {
                ProtoHttpAbort(state);
                continue;
            }

            code = ProtoHttpStatus(state, 'code', NULL, 0);

            switch(code)
            {
                // Any of these codes tell us the file has been deleted
                case 200: // OK
                case 202: // Accepted
                case 204: // No Content
                {
                    deleteSuccess = true;
                    goto CleanUp;
                }
                default:
                {
                    ProtoHttpAbort(state);
                    deleteSuccess = false;
                    goto CleanUp;
                }
            }// switch
        } // if
    } // while

CleanUp:
    ReleaseStateHandle(ref);

    return deleteSuccess;
}

HttpHandle* HttpDeviceDriverImpl::Open(const char *name)
{
    HttpHandle* handle = new HttpHandle(mServerString, name, mHttpBufferSize);

    return handle;
}

void HttpDeviceDriverImpl::Close(HttpHandle* handle)
{
    if(handle)
    {
        delete handle;
    }
}

//HttpDeviceDriver
HttpDeviceDriver::HttpDeviceDriver( const Options& ops ) : DeviceDriver("http:")
{
    mImpl = new HttpDeviceDriverImpl( ops );
    mDirectoryHandler = &sNullDirectoryHandler;
    bInitialized = false;
    mBufferSize = ops.mHttpBufferSize;
    mTimeout = ops.mTimeout;
    mIsDirtySockStartedHere = false;
}

HttpDeviceDriver::~HttpDeviceDriver()
{
    delete mImpl;
    mImpl = NULL;
}

bool HttpDeviceDriver::Init()
{
    if(!bInitialized)
    {
		bool isStarted = (NetConnStatus('open', 0, NULL, 0) != 0);            
		if ( !isStarted )
		{
			// Note by Arpit Baldeva: This assert is to safeguard against the accidental DirtySock/network layer start 
			// up of the application. Even though the transport handler here has the capability to turn on the DirtySock 
			// network layer (and later shut it down), this is usually not what a game team wants. It is safer to place 
			// a NetConnStartup("") somewhere in your application start up code and call NetConnShutdown(0) during 
			// shutdown to avoid this assert.
			EA_ASSERT_MSG(false,"rwfilesystem warning: Starting the network layer/DirtySock inside rwfilesystem HttpDriver since the application did not start it yet.\n");
			
			mIsDirtySockStartedHere = true;
			NetConnStartup(""); 
		}
		
        bInitialized = mImpl->Init();
    }

    return bInitialized;
}

void HttpDeviceDriver::Shutdown()
{
    if(bInitialized)
    {
        mImpl->Shutdown();
        
        if(mIsDirtySockStartedHere)
        {
            NetConnShutdown(0);
            mIsDirtySockStartedHere = false;
        }
    }
    bInitialized = false;
}

void* HttpDeviceDriver::Open( const char *name, unsigned int openFlags, rw::core::filesys::Handle**)
{
    HttpHandle* handle = mImpl->Open(name);
    bool createNewFile = false;
    bool fileExists = mImpl->FileExists(handle);

    if ((openFlags & AsyncOp::FOPEN_CREATE) == AsyncOp::FOPEN_CREATE)
    {
        if ((openFlags & AsyncOp::FOPEN_OVERWRITE) == AsyncOp::FOPEN_OVERWRITE)
        {
            // blow the existing file away and create a new one
            if(fileExists)
            {
                mImpl->DeleteFile(handle);
            }

            createNewFile = true;
        }
        else // FOPEN_OVERWRITE flag is not set
        {
            // an existing file cannot be opened with just the FOPEN_CREATE flag
            if(!fileExists)
            {
                createNewFile = true;
            }
        }
    }
    else    // FOPEN_CREATE flag is not set
    {
        if ((openFlags & AsyncOp::FOPEN_OVERWRITE) == AsyncOp::FOPEN_OVERWRITE)
        {

            if ((openFlags & AsyncOp::FOPEN_READWRITE) == AsyncOp::FOPEN_READWRITE)
            {
                // clear the contents of this file
                mImpl->DeleteFile(handle);
                createNewFile = true;
            }

            // else do nothing, we'll close our handle at the bottom if the file doesn't exist
        }
        else    // FOPEN_OVERWRITE flag is not set
        {
            // do nothing.  If the file already exists, we return it,
            // otherwise we return NULL after cleaning up our handles
        }
    }

    if(createNewFile)
    {
        // Create empty file
        mImpl->Upload(handle, "", 0);

        // Ensure our upload successful before we return the handle to the user
        if(mImpl->FileExists(handle))
        {
            return handle;
        }
    }
    else if(fileExists)
    {
        return handle;
    }

    // Reporting errors on file open can be tricky because people often use 
    // Open to check if a file exists.
    Manager::GetInstance()->GetErrorHandler()(static_cast<int>(Manager::FILEERROR_OPEN), Manager::FILEERROR_OPEN, this, NULL);

    mImpl->Close(handle);
    return NULL;
}

void HttpDeviceDriver::Close( void* h)
{
    EA_ASSERT(h != NULL);
    HttpHandle* httpHandle = static_cast<HttpHandle*>(h);

    mImpl->Close(httpHandle);
}


bool HttpDeviceDriver::Delete( const char* name )
{
    EA_ASSERT(name != NULL);
    HttpHandle* handle = mImpl->Open(name);
    bool result = mImpl->DeleteFile(handle);
    mImpl->Close(handle);

    return result;
}

bool HttpDeviceDriver::Exists(const char* name )
{
    EA_ASSERT(name != NULL);
    HttpHandle* handle = mImpl->Open(name);
    bool result = mImpl->FileExists(handle);
    mImpl->Close(handle);

    return result;
}

uint64_t HttpDeviceDriver::GetSize(void* h)
{
    EA_ASSERT(h != NULL);
    HttpHandle* httpHandle = static_cast<HttpHandle*>(h);

    uint64_t fileSize = mImpl->GetSize(httpHandle);

    httpHandle->mFileSize = fileSize;
    return httpHandle->mFileSize;
}

unsigned int HttpDeviceDriver::Read( void *h, void *buf, unsigned int bufSize, DeviceDriver * driver, void * context )
{
    EA_ASSERT(h != NULL);
    HttpHandle* httpHandle = static_cast<HttpHandle*>(h);

    EA_ASSERT(bufSize <= 0x7FFFFFFF);

    return mImpl->Download(httpHandle, buf, bufSize);
}

unsigned int HttpDeviceDriver::Write( void *h, const void * buf, unsigned int bufSize, DeviceDriver * ddParent, void * ddFileHandle )
{
    EA_ASSERT(h != NULL);
    HttpHandle* httpHandle = static_cast<HttpHandle*>(h);

    return mImpl->Upload(httpHandle, static_cast<const char*>(buf), bufSize);
}

uint64_t HttpDeviceDriver::Seek(void *h, uint64_t offset, int whence, DeviceDriver *, void *)
{
    EA_ASSERT(h != NULL);
    HttpHandle* httpHandle = static_cast<HttpHandle*>(h);

    switch(whence)
    {
        case DeviceDriver::FSEEK_CURRENT:
            httpHandle->mCurrentPosition += offset;
            break;

        case DeviceDriver::FSEEK_END:
            httpHandle->mCurrentPosition = static_cast<int64_t>(httpHandle->mFileSize - offset) < 0 ? 0 : httpHandle->mFileSize - offset;
            break;

        case DeviceDriver::FSEEK_BEGIN:
        default:
            httpHandle->mCurrentPosition = offset;
            break;
    }

    return httpHandle->mCurrentPosition;
}

bool HttpDeviceDriver::Move(const char* src, const char* dst)
{
    const int32_t bufferSize = 4096;
    char buffer[bufferSize] = {'\0'};

    bool success = false;
    int32_t count = 0;
    int32_t uploaded = 0;
    uint64_t fileSize = 0;
    uint64_t downloaded = 0;

    if(src == NULL || dst == NULL)
    {
        return success;
    }

    HttpHandle* srcHandle = static_cast<HttpHandle*>(Open(src, AsyncOp::FOPEN_READONLY, NULL));
    HttpHandle* dstHandle = static_cast<HttpHandle*>(Open(dst, AsyncOp::FOPEN_CREATE | AsyncOp::FOPEN_OVERWRITE, NULL));

    if(srcHandle == NULL || dstHandle == NULL)
    {
        goto CleanUp;
    }

    fileSize = mImpl->GetSize(srcHandle);

    do
    {
        // download and upload chunks until the entire file is finished
        count = mImpl->Download(srcHandle, buffer, bufferSize);

        if(count < 0)
        {
            goto CleanUp;
        }

        uploaded = mImpl->Upload(dstHandle, buffer, count);

        if(uploaded != count)
        {
            goto CleanUp;
        }

        downloaded += count;
    }while(downloaded < fileSize);

    success = true;

CleanUp:

    if(success)
    {
        mImpl->DeleteFile(srcHandle);
    }
    else
    {
        mImpl->DeleteFile(dstHandle);
    }

    // Clean-up handles
    mImpl->Close(srcHandle);
    mImpl->Close(dstHandle);

    return success;
}

unsigned int HttpDeviceDriver::GetMaxReadSize()
{
    return mBufferSize;
}

// Directory Functions
void HttpDeviceDriver::SetDirectoryHandler(IHttpDirectoryHandler* pHandler)
{
    if(pHandler)
    {
        mDirectoryHandler = pHandler;
    }
    else
    {
        mDirectoryHandler = &sNullDirectoryHandler;
    }
}

void* HttpDeviceDriver::FindBegin(const char *filePattern, DeviceDriver::FindData *findData)
{
    EA_ASSERT(mDirectoryHandler);
    return mDirectoryHandler->FindBegin(filePattern, findData);
}

bool  HttpDeviceDriver::FindNext(void *h, DeviceDriver::FindData *findData)
{
    EA_ASSERT(mDirectoryHandler);
    return mDirectoryHandler->FindNext(h, findData);
}

bool  HttpDeviceDriver::FindEnd(void *h)
{
    EA_ASSERT(mDirectoryHandler);
    return mDirectoryHandler->FindEnd(h);
}
bool  HttpDeviceDriver::DirectoryCreate(const char *directory)
{
    EA_ASSERT(mDirectoryHandler);
    return mDirectoryHandler->DirectoryCreate(directory);
}
bool  HttpDeviceDriver::DirectoryRemove(const char *directory)
{
    EA_ASSERT(mDirectoryHandler);
    return mDirectoryHandler->DirectoryRemove(directory);
}

void* HttpDeviceDriver::operator new(size_t size)
{
    return Manager::GetAllocator()->Alloc(size, "HttpDeviceDriver", EA::Allocator::MEM_PERM, RW_CORE_FILESYS_READ_BUFFER_ALIGNMENT);
}

void* HttpDeviceDriver::operator new [](size_t size)
{
    return Manager::GetAllocator()->Alloc(size, "HttpDeviceDriver", EA::Allocator::MEM_PERM, RW_CORE_FILESYS_READ_BUFFER_ALIGNMENT);
}

void* HttpDeviceDriver::operator new(size_t, void* ptr)
{
    return ptr;
}

void HttpDeviceDriver::operator delete(void*, void *)
{
}

void HttpDeviceDriver::operator delete(void* ptr)
{
    if (ptr)
        Manager::GetAllocator()->Free(ptr);
}

void HttpDeviceDriver::operator delete [](void* ptr)
{
    if (ptr)
        Manager::GetAllocator()->Free(ptr);
}


}}} // namespaces rw/core/filesys
