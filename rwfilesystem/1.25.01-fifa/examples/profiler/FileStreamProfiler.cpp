// (c) Electronic Arts. All Rights Reserved.

#include "FileStreamProfiler.h"

#include <rw/core/filesys/async.h>

#include <EAStdC/EAMemory.h>
#include <EAStdC/EAString.h>
#include <EAStdC/EASprintf.h>
#include <EAStdC/EAEndian.h>

#include <EAAssert/eaassert.h>

using namespace rw::core;

namespace fpa
{

struct FpaFileHeaderStruct
{
    // All fields are written out in big-endian format
    char signature[4];      // "EASP" (Electronic Arts Stream Profile)
    char version[4];        // Data version number
    char flags[4];          // Flags
    char entrySize[2];      // Size of an entry member
    char _reserved[2];      // Pad the structure out to 16 bytes
};

const uint32_t STRM_PROFILER_SIGNATURE = 'E' << 24 | 'A' << 16 | 'S' << 8 | 'P';
const uint32_t FPA_HEADER_VERSION = 2002000;    // 2.02.00
const int FPA_FLAG_BIG_ENDIAN = 0x01;

unsigned int numOfDataLoss = 0;

FileStreamProfiler::FileStreamProfiler(EA::Allocator::ICoreAllocator *allocator, size_t bufSize)
    : mInitialized( false ), mActive( false ), mWritePending( false ),
    mEntryOffSet(0), mWriteBufIndex(0), mBufSize(bufSize),
    mEntryCapacity((bufSize/2)/sizeof(EventEntry)), mAllocator(allocator)
{
    // Use the file system allocator if one wasn't supplied
    if (mAllocator == NULL)
        mAllocator = rw::core::filesys::Manager::GetAllocator();
}

FileStreamProfiler::~FileStreamProfiler()
{
}

void FileStreamProfiler::Initialize()
{
    EA_ASSERT_MSG(mInitialized == false, "Error: file stream profiler has already been initialized!");

    mEventBuffer = static_cast< EventEntry * >(mAllocator->Alloc(mBufSize,
        "FileStreamProfiler", EA::Allocator::MEM_PERM));
    EA::StdC::Memset8(mEventBuffer, 0, mBufSize);
    mWriteBuf[0] = mEventBuffer;
    mWriteBuf[1] = mEventBuffer + mEntryCapacity;

    char dataOutputFileName[256];

    for (int i = 0; i < 10000; ++i)
    {
        const char *PROFILE_STREAM_DATA_FILE = "StreamProfile%04d.spd";
        EA::StdC::Sprintf(dataOutputFileName, PROFILE_STREAM_DATA_FILE, i);
        if (!rw::core::filesys::Exists(dataOutputFileName))
            break;
    }

    mWriterOp.Open( dataOutputFileName, filesys::AsyncOp::FOPEN_CREATE | filesys::AsyncOp::FOPEN_READWRITE );
    mHandle = mWriterOp.GetResultHandle();
    EA_ASSERT(mHandle != NULL);

    FpaFileHeaderStruct fpaFileHeader;

    EA::StdC::Memclear(&fpaFileHeader, sizeof(fpaFileHeader));

    uint32_t flags = 0;

#if defined(EA_SYSTEM_BIG_ENDIAN)
    flags |= FPA_FLAG_BIG_ENDIAN;
#endif

    EA::StdC::WriteToBigEndian(&fpaFileHeader.signature, STRM_PROFILER_SIGNATURE);
    EA::StdC::WriteToBigEndian(&fpaFileHeader.version, FPA_HEADER_VERSION);
    EA::StdC::WriteToBigEndian(&fpaFileHeader.flags, flags);
    EA::StdC::WriteToBigEndian(&fpaFileHeader.entrySize, (uint16_t)sizeof(EventEntry));

    // write out this structure at the start of the profile data file
    mWriterOp.Write( mHandle, &fpaFileHeader, 0, sizeof(fpaFileHeader) );
    mWriterOp.Wait();

    mActive = true;
    mInitialized = true;
}

void FileStreamProfiler::Uninitialize()
{
    EA_ASSERT_MSG(mInitialized == true, "Error: can not call 'Uninitialize\\(\\)' before calling 'Initialize\\(\\)'");

    while(mWriterOp.GetStatus() == rw::core::filesys::AsyncOp::OPSTATUS_PENDING)
    {
        EA::Thread::ThreadSleep(1);
    }

    DumpBufferToFile(mWriteBuf[mWriteBufIndex], mEntryOffSet);
    mWriterOp.Wait();

    mAllocator->Free(mEventBuffer);
    mEventBuffer = NULL;
    mWriteBuf[0] = mWriteBuf[1] = NULL;

    mAllocator = 0;

    mWriterOp.Close(mHandle);
    mWriterOp.Wait();
    mHandle = 0;
    mActive = false;
    mInitialized = false;
}

void FileStreamProfiler::DumpBufferToFile(EventEntry * buffer, unsigned int count)
{
    unsigned int fileSize = static_cast<unsigned int>( GetSize( mHandle ) );

    // we want this to be quickly so the buffer is available again when the other side
    // is filled.
    mWriterOp.Write( mHandle, buffer, fileSize, count * sizeof(EventEntry), 0, 0, INT_MAX );
}

FileStreamProfiler::EventEntry * FileStreamProfiler::GetNextEventEntry()
{
    mMutex.Lock();

    if(mWritePending)
    {
        if(mWriterOp.GetStatus() == rw::core::filesys::AsyncOp::OPSTATUS_COMPLETE)
            mWritePending = false;
    }

    EventEntry * ret;
    bool dumpBuffer = false;

    if((mEntryCapacity - mEntryOffSet) < 1)
    {
        // if there is not enough space
        if(mWritePending)
        {
            numOfDataLoss++;
            if(numOfDataLoss <= mEntryCapacity)
            {
                ret = mWriteBuf[mWriteBufIndex] + mEntryOffSet - numOfDataLoss; // overwrite the last data entry
                EA::StdC::Memset8(ret, 0, sizeof(EventEntry));
                ret->ues.tick = static_cast< unsigned int >(EA_THREADTIME_AS_INT64(EA::Thread::GetThreadTime()));
                ret->ues.handle = 0;
                ret->ues.command = rw::core::filesys::USEREVENT;
                EA::StdC::Strncpy(ret->ues.acronym, "DLS", sizeof(ret->ues.acronym));
                EA::StdC::Strncpy(ret->ues.desc, "Data loss!", sizeof(ret->ues.desc));
            }
            mMutex.Unlock();
            return NULL;
        }
        else
        {
            dumpBuffer = true;
            mWriteBufIndex ^= 1;    // flip to filling the other half of the buffer
            mEntryOffSet = 0;
            numOfDataLoss = 0;
        }
    }

    ret = mWriteBuf[mWriteBufIndex] + mEntryOffSet;
    ++mEntryOffSet;

    mMutex.Unlock();

    if(dumpBuffer)
    {
        mWritePending = true;
        DumpBufferToFile(mWriteBuf[mWriteBufIndex^1], static_cast<unsigned int>(mEntryCapacity));
    }

    EA::StdC::Memset8(ret, 0, sizeof(EventEntry));

    return ret;
}

void FileStreamProfiler::Enable()
{
    EA_ASSERT_MSG(mInitialized == true, "Error: file stream profiler hasn't been initialized!");
    mActive = true;
}

void FileStreamProfiler::Disable()
{
    EA_ASSERT_MSG(mInitialized == true, "Error: file stream profiler hasn't been initialized!");
    mActive = false;
}

void FileStreamProfiler::Callback( rw::core::filesys::ProfilerEvent type, const char * handle, void *param, void * context )
{
    (void)context;
    (void)handle;

    if( mActive)
    {
        EA_ASSERT_MSG(mInitialized == true, "Error: file stream profiler hasn't been initialized!");
        switch( type )
        {
        case rw::core::filesys::ProfilerEventStreamBufferInfo:
            {
                rw::core::filesys::ProfilerEventStreamBufferInfoStruct * streamBufferInfoEvent = static_cast< rw::core::filesys::ProfilerEventStreamBufferInfoStruct * >(param);
                
                EventEntry * bufferInfoEvent = GetNextEventEntry();
                if(bufferInfoEvent != 0)
                {
                    bufferInfoEvent->bis.tick = static_cast< unsigned int >(EA_THREADTIME_AS_INT64(EA::Thread::GetThreadTime()));
                    bufferInfoEvent->bis.shandle = streamBufferInfoEvent->shandle;
                    bufferInfoEvent->bis.command = streamBufferInfoEvent->command;
                    bufferInfoEvent->bis.amount = streamBufferInfoEvent->amount;
                    bufferInfoEvent->bis.fhandle = streamBufferInfoEvent->fhandle;
                    bufferInfoEvent->bis.state = streamBufferInfoEvent->state;
                    bufferInfoEvent->bis.requestid = streamBufferInfoEvent->requestid;
                    bufferInfoEvent->bis.greedystate = streamBufferInfoEvent->greedystate;
                    if(streamBufferInfoEvent->command == rw::core::filesys::USAGE)
                        bufferInfoEvent->bis.fhandle = streamBufferInfoEvent->fhandle;
                }
            }
            break;
        case rw::core::filesys::ProfilerEventStreamFileOpen:
            {
                rw::core::filesys::ProfilerEventStreamFileOpenStruct * streamFileOpenEvent = static_cast< rw::core::filesys::ProfilerEventStreamFileOpenStruct * >(param);

                EventEntry * fileOpenEvent = GetNextEventEntry();
                if(fileOpenEvent != 0)
                {
                    fileOpenEvent->fos.tick = static_cast< unsigned int >(EA_THREADTIME_AS_INT64(EA::Thread::GetThreadTime()));
                    fileOpenEvent->fos.shandle = streamFileOpenEvent->shandle;
                    fileOpenEvent->fos.command = streamFileOpenEvent->command;
                    fileOpenEvent->fos.fhandle = streamFileOpenEvent->fhandle;
                    fileOpenEvent->fos.foffset = streamFileOpenEvent->foffset;
                    fileOpenEvent->fos.requestid = streamFileOpenEvent->requestid;
                    EA::StdC::Strcpy(fileOpenEvent->fos.fname, streamFileOpenEvent->fname);
                }
            }
            break;
        case rw::core::filesys::ProfilerEventStreamFileClose:
            {
                rw::core::filesys::ProfilerEventStreamFileCloseStruct * streamFileCloseEvent = static_cast< rw::core::filesys::ProfilerEventStreamFileCloseStruct * >(param);

                EventEntry * fileCloseEvent = GetNextEventEntry();
                if(fileCloseEvent != 0)
                {
                    fileCloseEvent->fcs.tick = static_cast< unsigned int >(EA_THREADTIME_AS_INT64(EA::Thread::GetThreadTime()));
                    fileCloseEvent->fcs.shandle = streamFileCloseEvent->shandle;
                    fileCloseEvent->fcs.command = streamFileCloseEvent->command;
                    fileCloseEvent->fcs.fhandle = streamFileCloseEvent->fhandle;
                    fileCloseEvent->fcs.requestid = streamFileCloseEvent->requestid;
                }
            }
            break;
        case rw::core::filesys::ProfilerEventStreamDropOut:
            {
                rw::core::filesys::ProfilerEventStreamDropOutStruct * streamDropOutEvent = static_cast< rw::core::filesys::ProfilerEventStreamDropOutStruct * >(param);

                EventEntry * fileDropOutEvent = GetNextEventEntry();
                if(fileDropOutEvent != 0)
                {
                    fileDropOutEvent->dos.tick = static_cast< unsigned int >(EA_THREADTIME_AS_INT64(EA::Thread::GetThreadTime()));
                    fileDropOutEvent->dos.shandle = streamDropOutEvent->shandle;
                    fileDropOutEvent->dos.command = streamDropOutEvent->command;
                    fileDropOutEvent->dos.fhandle = streamDropOutEvent->fhandle;
                    fileDropOutEvent->dos.percentage = streamDropOutEvent->percentage;
                }
            }
            break;
        case rw::core::filesys::ProfilerEventStreamGreedyStateChange:
            {
                rw::core::filesys::ProfilerEventStreamGreedyStateChangeStruct * streamGreedyStateChangeEvent = static_cast< rw::core::filesys::ProfilerEventStreamGreedyStateChangeStruct * >(param);
                EventEntry * greedyStateChangeEvent = GetNextEventEntry();
                if(greedyStateChangeEvent != 0)
                {
                    greedyStateChangeEvent->gscs.tick = static_cast< unsigned int >(EA_THREADTIME_AS_INT64(EA::Thread::GetThreadTime()));
                    greedyStateChangeEvent->gscs.shandle = streamGreedyStateChangeEvent->shandle;
                    greedyStateChangeEvent->gscs.command = streamGreedyStateChangeEvent->command;
                    greedyStateChangeEvent->gscs.greedystate = streamGreedyStateChangeEvent->greedystate;
                    greedyStateChangeEvent->gscs.greedylevel = streamGreedyStateChangeEvent->greedylevel;
                }
            }
            break;
        case rw::core::filesys::ProfilerEventStreamRequestInfo:
            {
                rw::core::filesys::ProfilerEventStreamRequestInfoStruct * streamRequestInfoEvent = static_cast< rw::core::filesys::ProfilerEventStreamRequestInfoStruct * >(param);
                EventEntry * requestInfoEvent = GetNextEventEntry();
                if(requestInfoEvent != 0)
                {
                    requestInfoEvent->ris.tick = static_cast< unsigned int >(EA_THREADTIME_AS_INT64(EA::Thread::GetThreadTime()));
                    requestInfoEvent->ris.shandle = streamRequestInfoEvent->shandle;
                    requestInfoEvent->ris.command = streamRequestInfoEvent->command;
                    requestInfoEvent->ris.requestid = streamRequestInfoEvent->requestid;
                    requestInfoEvent->ris.reason = streamRequestInfoEvent->reason;
                    requestInfoEvent->ris.fhandle = streamRequestInfoEvent->fhandle;
                    EA::StdC::Strcpy(requestInfoEvent->ris.fname, streamRequestInfoEvent->fname);
                }
            }
            break;
#if defined(RW_CORE_FILESYS_ENABLE_STREAMTRACE)
        case rw::core::filesys::ProfilerEventStreamStateInfo:
            {
                rw::core::filesys::ProfilerEventStreamStateInfoStruct * ssi = static_cast< rw::core::filesys::ProfilerEventStreamStateInfoStruct * >(param);
                EventEntry * stateInfoEvent = GetNextEventEntry();
                stateInfoEvent->s.tick = static_cast< unsigned int >(EA_THREADTIME_AS_INT64(EA::Thread::GetThreadTime()));
                stateInfoEvent->s.command = ssi->command;
                stateInfoEvent->s.streamhandle = ssi->streamhandle;
                stateInfoEvent->s.bufferstart = ssi->bufferstart;
                stateInfoEvent->s.state = ssi->state;
                stateInfoEvent->s.datastart = ssi->datastart;
                stateInfoEvent->s.datatail = ssi->datatail;
                stateInfoEvent->s.dataend = ssi->dataend;
                stateInfoEvent->s.fhandle = ssi->fhandle;
                stateInfoEvent->s.foffset = ssi->foffset;
            }
            break;
        case rw::core::filesys::ProfilerEventStreamReadInfo:
            {
                rw::core::filesys::ProfilerEventStreamReadInfoStruct * ris = static_cast< rw::core::filesys::ProfilerEventStreamReadInfoStruct * >(param);
                EventEntry * readInfoEvent = GetNextEventEntry();
                readInfoEvent->r.tick = static_cast< unsigned int >(EA_THREADTIME_AS_INT64(EA::Thread::GetThreadTime()));
                readInfoEvent->r.command = ris->command;
                readInfoEvent->r.streamhandle = ris->streamhandle;
                readInfoEvent->r.fhandle = ris->fhandle;
                readInfoEvent->r.addr = ris->addr;
                readInfoEvent->r.ofs = ris->ofs;
            }
            break;
        case rw::core::filesys::ProfilerEventStreamApiCallInfo:
            {
                rw::core::filesys::ProfilerEventStreamApiCallInfoStruct * acis = static_cast< rw::core::filesys::ProfilerEventStreamApiCallInfoStruct * >(param);
                EventEntry * apiInfoEvent = GetNextEventEntry();
                apiInfoEvent->a.tick = static_cast< unsigned int >(EA_THREADTIME_AS_INT64(EA::Thread::GetThreadTime()));
                apiInfoEvent->a.command = acis->command;
                apiInfoEvent->a.streamhandle = acis->streamhandle;
                if(acis->command == rw::core::filesys::GET || acis->command == rw::core::filesys::RELEASE)
                {
                    apiInfoEvent->a.chunk = acis->chunk;
                    apiInfoEvent->a.pData = acis->pData;
                    apiInfoEvent->a.chunksize = acis->chunksize;
                    apiInfoEvent->a.requestid = acis->requestid;
                }
                else if(acis->command == rw::core::filesys::CREATE)
                {
                    apiInfoEvent->a.requests = acis->requests;
                    apiInfoEvent->a.buffersize = acis->buffersize;
                }
                else if(acis->command == rw::core::filesys::QUEUEFILE)
                {
                    apiInfoEvent->a.requestid = acis->requestid;
                    EA::StdC::Strncpy(apiInfoEvent->a.fname, acis->fname, sizeof(acis->fname));
                }
                else if(acis->command == rw::core::filesys::QUEUEMEM)
                {
                    apiInfoEvent->a.addr = acis->addr;
                    apiInfoEvent->a.requestid = acis->requestid;
                }
                else if(acis->command == rw::core::filesys::CANCEL)
                {
                    apiInfoEvent->a.requestid = acis->requestid;
                }
                else if(acis->command == rw::core::filesys::DESTROY || acis->command == rw::core::filesys::KILL) {}
            }
            break;
        case rw::core::filesys::ProfilerEventStreamChunkInfo:
            {
                rw::core::filesys::ProfilerEventStreamChunkInfoStruct * cis = static_cast< rw::core::filesys::ProfilerEventStreamChunkInfoStruct * >(param);
                EventEntry * chunkInfoEvent = GetNextEventEntry();
                chunkInfoEvent->c.tick = static_cast< unsigned int >(EA_THREADTIME_AS_INT64(EA::Thread::GetThreadTime()));
                chunkInfoEvent->c.command = cis->command;
                chunkInfoEvent->c.streamhandle = cis->streamhandle;
                chunkInfoEvent->c.chunkaddr = cis->chunkaddr;
                chunkInfoEvent->c.pData = cis->pData;
                chunkInfoEvent->c.size = cis->size;
                chunkInfoEvent->c.requestid = cis->requestid;
                chunkInfoEvent->c.endofstream = cis->endofstream;
            }
            break;
#endif
        case rw::core::filesys::ProfilerEventUnknown:
            break;
        default:
            break;
        }
    }
}

void FileStreamProfiler::AddUserEvent(const char * acronym, const char * description)
{
    EA_ASSERT_MSG(mInitialized == true, "Error: file stream profiler hasn't been initialized!");

    if( mActive)
    {
        EventEntry * userEvent = GetNextEventEntry();
	if(userEvent != 0)
	{
		userEvent->ues.tick = static_cast< unsigned int >(EA_THREADTIME_AS_INT64(EA::Thread::GetThreadTime()));
		userEvent->ues.handle = 0;
		userEvent->ues.command = rw::core::filesys::USEREVENT;
		EA::StdC::Strncpy(userEvent->ues.acronym, acronym, sizeof(userEvent->ues.acronym));
		EA::StdC::Strncpy(userEvent->ues.desc, description, sizeof(userEvent->ues.desc));
	}
    }
}

}
