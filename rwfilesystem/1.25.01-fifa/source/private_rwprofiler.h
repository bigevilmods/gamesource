// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_CORE_FILESYS_RWDEFAULTPROFILER_H
#define RW_CORE_FILESYS_RWDEFAULTPROFILER_H

#include "rw/core/filesys/profiler.h"

namespace rw {
namespace core {
namespace filesys {

    class DefaultProfiler : public FileSysProfiler
    {
    public:
        DefaultProfiler();
        virtual ~DefaultProfiler();

        virtual void Callback(ProfilerEvent event, const char *devicename, void *param, void *context);
    };

    void LogGenericEvent(ProfilerEvent event, const char *deviceName, void *data);
    void LogOpenCompleteEvent(const char *deviceName, intptr_t handle, const char *filePath, void *deviceFileHandle);
    void LogReadCompleteEvent(ProfilerEvent event, const char *deviceName, uint64_t size, intptr_t handle, void *buffer);

    // TODO: rename this to LogDeviceReadOrWrite
    void LogReadOrWriteEvent(ProfilerEvent event, 
            const char *deviceName, 
            intptr_t handle, 
            uint64_t size, 
            uint64_t location, 
            void *buffer, 
            int priority);

    // TODO: rename this to something different, say LogReadOrWriteCall
    void LogReadEvent(ProfilerEvent event, const char *deviceName, intptr_t handle, uint64_t offset, uint64_t nbytes, const void *raddr, int priority);
    void LogSeekEvent(const char *deviceName, intptr_t handle, int whence, uint64_t prevOffset, uint64_t newOffset);
    void LogSeekEvent2(ProfilerEvent event, const char *deviceName, intptr_t handle, uint64_t offset, void *buffer);

    void LogStreamCreate(void *streamHandle, int requests, int buffersize);
    void LogStreamDestroy(void *streamHandle);
    void LogStreamQueueFile(void *streamHandle, int requestId, const char *name);
    void LogStreamQueueMem(void *streamHandle, int requestId, void *addr);
    void LogStreamCancel(void *streamHandle, int requestId);
    void LogStreamGet(void *streamHandle, int requestId, void *chunk, void *pData, unsigned chunkSize);
    void LogStreamRelease(void *streamHandle, int requestId, void *chunk, void *pData, unsigned chunkSize);

    void LogStreamBufferOperation(ProfilerCommand command, void *streamHandle, int requestId, int size, Handle *fileHandle, int state, bool greedyState);
    
    void LogStreamChunkInfoEvent(void *streamHandle, int requestId, void *chunkAddr, void *pData, unsigned size, int endOfStream);
    void LogStreamDropOutEvent(void *streamHandle, Handle *fileHandle, float percentage);
    void LogStreamFileCloseEvent(void *streamHandle, int requestId, Handle *fileHandle);
    void LogStreamFileOpenEvent(void *streamHandle, int requestId, Handle *fileHandle, int64_t fileOffset, const char *name);
    void LogStreamGreedyStateChangeEvent(void *streamHandle, int greedyLevel, bool greedyState);
    void LogStreamReadInfoEvent(void *streamHandle, Handle *fileHandle, unsigned char *addr, int64_t offset);
    
    void LogStreamRequestOperation(ProfilerCommand command, void *streamHandle, int requestId, Handle *handle, int reason, const char *name);
    
    void LogStreamStateInfoEvent(ProfilerCommand command,
            void *streamHandle, 
            unsigned char *bufferStart,
            int state,
            unsigned char *dataStart,
            unsigned char *dataTail,
            unsigned char *dataEnd,
            Handle *handle,
            int64_t offset);


}}}

#endif

