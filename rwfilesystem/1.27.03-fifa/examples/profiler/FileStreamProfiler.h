// (c) 2007 - 2009 Electronic Arts

#ifndef FILE_PROFILE_HANDLER_H
#define FILE_PROFILE_HANDLER_H 1

#include <EABase/eabase.h>
#include <eathread/eathread_mutex.h>
#include <rw/core/filesys/profiler.h>
#include <rw/core/filesys/async.h>
#include <coreallocator/icoreallocator_interface.h>

#if !defined(EA_FILESYSTEM_PROFILING)
#error EA_FILESYSTEM_PROFILING must be defined
#endif

/*!
    \brief A File Stream Profiler for the file system.

    This profiler can be used by applications to collect file system streamer 
    profile data. The data will be written to a file.
*/

// Define your own stream profile data file name here.
//#if defined EA_PLATFORM_PS3
//#define PROFILE_STREAM_DATA_FILE "/app_home/my_stream_data.SPD"
//#elif defined EA_PLATFORM_XENON
//#define PROFILE_STREAM_DATA_FILE "D:\\my_stream_data.SPD"
//#else
//#define PROFILE_STREAM_DATA_FILE "my_stream_data.SPD"
//#endif 

namespace fpa
{

    const size_t DEFAULT_PROFILE_BUFFER_SIZE = 100*1024;

class FileStreamProfiler : public rw::core::filesys::FileSysProfiler
{
public:

    struct ProfilerUserEventStruct
    {
        rw::core::filesys::ProfilerCommand command;
        unsigned int tick;
        void * handle; // dummy handle
        char acronym[4];
        char desc[32];
    };

    union EventEntry
    {
        rw::core::filesys::ProfilerEventStreamBufferInfoStruct bis;
        rw::core::filesys::ProfilerEventStreamFileOpenStruct fos;
        rw::core::filesys::ProfilerEventStreamFileCloseStruct fcs;
        rw::core::filesys::ProfilerEventStreamDropOutStruct dos;
        rw::core::filesys::ProfilerEventStreamGreedyStateChangeStruct gscs;
        rw::core::filesys::ProfilerEventStreamRequestInfoStruct ris;
        ProfilerUserEventStruct ues;
#ifdef RW_CORE_FILESYS_ENABLE_STREAMTRACE
        rw::core::filesys::ProfilerEventStreamApiCallInfoStruct a;
        rw::core::filesys::ProfilerEventStreamStateInfoStruct s;
        rw::core::filesys::ProfilerEventStreamReadInfoStruct r;
        rw::core::filesys::ProfilerEventStreamChunkInfoStruct c;
#endif
    };

    FileStreamProfiler(EA::Allocator::ICoreAllocator *allocator = NULL,
        size_t bufSize = DEFAULT_PROFILE_BUFFER_SIZE);
    ~FileStreamProfiler();

    void Initialize();

    void Uninitialize();

    /*!
        \brief Enable recording profile information.

        (By default the File Stream Profiler
        is enabled.) You can use this to temporarily enable/disable the
        profiler logging.
        \see Disable
    */
    void Enable();


    /*!
        \brief Disable recording profile information.

        (By default the File Stream Profiler
        is enabled.) You can use this to temporarily enable/disable the
        profiler logging.
        \see Enable
    */
    void Disable();


    /*!
        \brief The callback that will be called by the RwFileSystem streamer.

        This callback will be called by the RwFileSystem streamer for each profile event
        that it recognizes.
    */
    void Callback( rw::core::filesys::ProfilerEvent type, const char * handle, void *param, void *context );

    /*!
        \brief The callback that will be called by external users of this profiler

        Users can call this callback to insert user profiling events.
    */
    void AddUserEvent(const char * acronym, const char * description);

private:

    EventEntry * GetNextEventEntry();
    void DumpBufferToFile(EventEntry * buffer, unsigned int count);

    bool mInitialized;
    bool mActive;
    bool mWritePending;
    unsigned int mEntryOffSet;
    int mWriteBufIndex;
    size_t mBufSize;
    size_t mEntryCapacity;

    EA::Thread::Mutex mMutex;
    EA::Allocator::ICoreAllocator* mAllocator;
    EventEntry *mEventBuffer;
    EventEntry *mWriteBuf[2];
    rw::core::filesys::AsyncOp mWriterOp;
    rw::core::filesys::Handle* mHandle;

    // non-copyable/assignable
    FileStreamProfiler(const FileStreamProfiler&);
    FileStreamProfiler& operator=(const FileStreamProfiler&);
};

}
#endif  // include guard

