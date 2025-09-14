// (c) Electronic Arts. All Rights Reserved.

#include "rw/core/filesys/manager.h"
#include "private_rwprofiler.h"
#include "EAStdC/EAString.h"
#include "EAAssert/eaassert.h"

namespace rw {
namespace core {
namespace filesys {

    DefaultProfiler::DefaultProfiler()
    {
    }


    DefaultProfiler::~DefaultProfiler()
    {
    }


    void DefaultProfiler::Callback(ProfilerEvent event, const char *name, void *param, void *context)
    {
        (void)event;
        (void)name;
        (void)param;
        (void)context;
    }


    static inline void SendEvent(ProfilerEvent event, const char *name, void *param)
    {
        Manager *manager = Manager::GetInstance();
        FileSysProfiler *profiler = manager->GetEventProfiler();
        void *profilerContext = manager->GetEventProfilerContext();
        profiler->Callback(event, name, param, profilerContext);
    }


    void LogGenericEvent(ProfilerEvent event, const char *deviceName, void *data)
    {
        SendEvent(event, deviceName, data);
    }


    void LogOpenCompleteEvent(const char *deviceName, intptr_t handle, const char *filePath, void *deviceFileHandle)
    {
        ProfilerEventOpenCompleteStruct P(handle, filePath, deviceFileHandle);

        SendEvent(ProfilerEventOpenComplete, deviceName, &P);
    }


    void LogReadCompleteEvent(ProfilerEvent event, const char *deviceName, uint64_t size, intptr_t handle, void *buffer)
    {
        ProfilerEventReadCompleteStruct P(size, handle, buffer);

        SendEvent(event, deviceName, &P);
    }


    void LogReadOrWriteEvent(ProfilerEvent event, 
            const char *deviceName, 
            intptr_t handle, 
            uint64_t size, 
            uint64_t location, 
            void *buffer, 
            int priority)
    {
        ProfilerEventReadOrWriteStruct P(handle, size, location, buffer, priority);

        SendEvent(event, deviceName, &P);
    }


    void LogReadEvent(ProfilerEvent event, const char *deviceName, intptr_t handle, uint64_t offset, uint64_t nbytes, const void *raddr, int priority)
    {
        ProfilerEventReadStruct P(handle, offset, nbytes, raddr, priority);

        SendEvent(event, deviceName, &P);
    }


    void LogSeekEvent(const char *deviceName, intptr_t handle, int whence, uint64_t prevOffset, uint64_t newOffset)
    {
        ProfilerEventSeekStruct P(handle, whence, prevOffset, newOffset);

        SendEvent(ProfilerEventSeek, deviceName, &P);
    }


    void LogSeekEvent2(ProfilerEvent event, const char *deviceName, intptr_t handle, uint64_t offset, void *buffer)
    {
        ProfilerEventSeekStruct2 P(handle, offset, buffer);

        SendEvent(event, deviceName, &P);
    }


    void LogStreamCreate(void *streamHandle, int requests, int bufferSize)
    {
        ProfilerEventStreamApiCallInfoStruct P = {};
        P.streamhandle = streamHandle;
        P.requests = requests;
        P.buffersize = bufferSize;
        P.command = CREATE;

        SendEvent(ProfilerEventStreamApiCallInfo, "", &P);
    }


    void LogStreamDestroy(void *streamHandle)
    {
        ProfilerEventStreamApiCallInfoStruct P = {};
        P.streamhandle = streamHandle;
        P.command = DESTROY;

        SendEvent(ProfilerEventStreamApiCallInfo, "", &P);
    }


    void LogStreamQueueFile(void *streamHandle, int requestId, const char *name)
    {
        ProfilerEventStreamApiCallInfoStruct P = {};
        P.streamhandle = streamHandle;
        P.requestid = requestId;
        P.command = QUEUEFILE;

        EA_STRLCPY_VERIFY(P.fname, name, EAArrayCount(P.fname));

        SendEvent(ProfilerEventStreamApiCallInfo, "", &P);
    }


    void LogStreamQueueMem(void *streamHandle, int requestId, void *addr)
    {
        ProfilerEventStreamApiCallInfoStruct P = {};
        P.streamhandle = streamHandle;
        P.requestid = requestId;
        P.addr = addr;
        P.command = QUEUEMEM;

        SendEvent(ProfilerEventStreamApiCallInfo, "", &P);
    }


    void LogStreamCancel(void *streamHandle, int requestId)
    {
        ProfilerEventStreamApiCallInfoStruct P = {};
        P.streamhandle = streamHandle;
        P.requestid = requestId;
        P.command = CANCEL;

        SendEvent(ProfilerEventStreamApiCallInfo, "", &P);
    }


    void LogStreamGet(void *streamHandle, int requestId, void *chunk, void *pData, unsigned chunkSize)
    {
        ProfilerEventStreamApiCallInfoStruct P = {};
        P.streamhandle = streamHandle;
        P.requestid = requestId;
        P.chunk = chunk;
        P.pData = pData;
        P.chunksize = chunkSize;
        P.command = GET;

        SendEvent(ProfilerEventStreamApiCallInfo, "", &P);
    }


    void LogStreamRelease(void *streamHandle, int requestId, void *chunk, void *pData, unsigned chunkSize)
    {
        ProfilerEventStreamApiCallInfoStruct P = {};
        P.streamhandle = streamHandle;
        P.requestid = requestId;
        P.chunk = chunk;
        P.pData = pData;
        P.chunksize = chunkSize;
        P.command = RELEASE;

        SendEvent(ProfilerEventStreamApiCallInfo, "", &P);        
    }


    void LogStreamBufferOperation(ProfilerCommand command, void *streamHandle, int requestId, int size, Handle *fileHandle, int state, bool greedyState)
    {
        ProfilerEventStreamBufferInfoStruct P = {};
        P.shandle = streamHandle;
        P.requestid = requestId;
        P.amount = size;
        P.fhandle = fileHandle;
        P.state = state;
        P.greedystate = greedyState;
        P.command = command;

        SendEvent(ProfilerEventStreamBufferInfo, "", &P);
    }

   
    void LogStreamChunkInfoEvent(void *streamHandle, int requestId, void *chunkAddr, void *pData, unsigned size, int endOfStream)
    {
        ProfilerEventStreamChunkInfoStruct P = {};
        P.streamhandle = streamHandle;
        P.requestid = requestId;
        P.chunkaddr = chunkAddr;
        P.pData = pData;
        P.size = size;
        P.endofstream = endOfStream;
        P.command = ADDCHUNKTOGETTABLEQUEUE;

        SendEvent(ProfilerEventStreamChunkInfo, "", &P);
    }


    void LogStreamDropOutEvent(void *streamHandle, Handle *fileHandle, float percentage)
    {
        ProfilerEventStreamDropOutStruct P = {};
        P.shandle = streamHandle;
        P.fhandle = fileHandle;
        P.percentage = percentage;
        P.command = DROPOUT;

        SendEvent(ProfilerEventStreamDropOut, "", &P);
    }


    void LogStreamFileCloseEvent(void *streamHandle, int requestId, Handle *fileHandle)
    {
        ProfilerEventStreamFileCloseStruct P = {};
        P.shandle = streamHandle;
        P.requestid = requestId;
        P.fhandle = fileHandle;
        P.command = FILECLOSE;

        SendEvent(ProfilerEventStreamFileClose, "", &P);
    }


    void LogStreamFileOpenEvent(void *streamHandle, int requestId, Handle *fileHandle, int64_t fileOffset, const char *name)
    {
        ProfilerEventStreamFileOpenStruct P = {};
        P.shandle = streamHandle;
        P.requestid = requestId;
        P.fhandle = fileHandle;
        P.foffset = fileOffset;
        P.command = FILEOPEN;

        EA_STRLCPY_VERIFY(P.fname, name, EAArrayCount(P.fname));

        SendEvent(ProfilerEventStreamFileOpen, "", &P);
    }


    void LogStreamGreedyStateChangeEvent(void *streamHandle, int greedyLevel, bool greedyState)
    {
        ProfilerEventStreamGreedyStateChangeStruct P = {};
        P.shandle = streamHandle;
        P.greedylevel = greedyLevel;
        P.greedystate = greedyState;
        P.command = GREEDYSTATECHANGE;

        SendEvent(ProfilerEventStreamGreedyStateChange, "", &P);
    }


    void LogStreamReadInfoEvent(void *streamHandle, Handle *fileHandle, unsigned char *addr, int64_t offset)
    {
        ProfilerEventStreamReadInfoStruct P = {};
        P.streamhandle = streamHandle;
        P.fhandle = fileHandle;
        P.addr = addr;
        P.ofs = offset;
        P.command = SCHEDULINGREAD;

        SendEvent(ProfilerEventStreamReadInfo, "", &P);
    }

    
    void LogStreamRequestOperation(ProfilerCommand command, void *streamHandle, int requestId, Handle *handle, int reason, const char *name)
    {
        ProfilerEventStreamRequestInfoStruct P = {};
        P.shandle = streamHandle;
        P.requestid = requestId;
        P.fhandle = handle;
        P.reason = reason;
        P.command = command;

        EA_STRLCPY_VERIFY(P.fname, name, EAArrayCount(P.fname));
        
        SendEvent(ProfilerEventStreamRequestInfo, "", &P);
    }

    
    void LogStreamStateInfoEvent(ProfilerCommand command,
            void *streamHandle, 
            unsigned char *bufferStart,
            int state,
            unsigned char *dataStart,
            unsigned char *dataTail,
            unsigned char *dataEnd,
            Handle *handle,
            int64_t offset)
    {
        ProfilerEventStreamStateInfoStruct P = {};
        P.streamhandle = streamHandle;
        P.bufferstart = bufferStart;
        P.state = state;
        P.datastart = dataStart;
        P.datatail = dataTail;
        P.dataend = dataEnd;
        P.fhandle = handle;
        P.foffset = offset;
        P.command = command;

        SendEvent(ProfilerEventStreamStateInfo, "", &P);
    }


}}}

