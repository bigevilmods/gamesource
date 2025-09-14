// (c) 2007 - 2009 Electronic Arts

#include <stdlib.h>

#include <coreallocator/icoreallocator_interface.h>

#include <rw/core/filesys/manager.h>
#include <rw/core/filesys/stream.h>

#include "FileStreamProfiler.h"

using namespace rw::core;

class AllocatorImpl : public EA::Allocator::ICoreAllocator
{
public:
    virtual void *Alloc(size_t size, const char *, unsigned int)
    {
        return malloc(size);
    }

    virtual void *Alloc(size_t size, const char *, unsigned int, unsigned int, unsigned int)
    {
        return Alloc(size, NULL, 0);
    }

    virtual void Free(void *block, size_t)
    {
        free(block);
    }
};

AllocatorImpl gAllocator;

// For EAStdC
void *operator new[](size_t size, const char *, int, unsigned int, const char *, int)
{
    return gAllocator.Alloc( size, NULL, 0 );;
}

EA_ALIGNED(char, gStrmBuf1[100*1024], 16);
EA_ALIGNED(char, gStrmBuf2[100*1024], 16);

int main()
{
    filesys::Options opts(&gAllocator);

    filesys::Manager::CreateInstance(opts);

    fpa::FileStreamProfiler *fileProfilerHandler = new fpa::FileStreamProfiler(&gAllocator, 10*1024);

    fileProfilerHandler->Initialize();

    filesys::Manager::GetInstance()->SetEventProfiler(fileProfilerHandler);

    filesys::Stream *strm1 = filesys::Stream::Create(2, gStrmBuf1, sizeof(gStrmBuf1));
    filesys::Stream *strm2 = filesys::Stream::Create(2, gStrmBuf2, sizeof(gStrmBuf2));

    strm1->QueueFile("menu1.asf");
    strm2->QueueFile("menu2.asf");

    bool stop = false;
    while(!stop)
    {
        bool running = false;

        filesys::Stream::ChunkInfo *chunk1 = strm1->GetChunk();
        if (chunk1 != 0)
        {
            EA::Thread::ThreadSleep(10);
            strm1->ReleaseChunk(chunk1);
        }
        if(! strm1->IsEndOfStream() || strm1->BufferUsage() != 0 )
            running = true;

        filesys::Stream::ChunkInfo *chunk2 = strm2->GetChunk();
        if (chunk2 != 0)
        {
            EA::Thread::ThreadSleep(10);
            strm2->ReleaseChunk(chunk2);
        }
        if(! strm2->IsEndOfStream() || strm2->BufferUsage() != 0 )
            running = true;

        EA::Thread::ThreadSleep(50);
        if(!running)
            stop = true;
    }

    strm1->CloseFileHandle();
    strm2->CloseFileHandle();

    strm1->Destroy();
    strm2->Destroy();

    fileProfilerHandler->Uninitialize();

    delete fileProfilerHandler;

    filesys::Manager::DestroyInstance();

    return 0;
}
