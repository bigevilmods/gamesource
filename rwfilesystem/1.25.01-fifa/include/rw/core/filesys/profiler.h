// (c) Electronic Arts. All Rights Reserved.
/**
\file
File system events operations
*/

#ifndef RW_CORE_FILESYS_PROFILER_H
#define RW_CORE_FILESYS_PROFILER_H

#include "EABase/eabase.h"
#include "rw/core/filesys/config.h"

namespace rw {
namespace core {
namespace filesys {
    class Handle;

    /**
        \brief File Device Event Types

        The ProfilerEvent structure contains event values that
        corresponds to the event that requested the execution
        of the callback function.

        \importlib rwfilesystem
    */
    enum ProfilerCommand
    {
        // user entry points
        CREATE,
        USAGE,
        DESTROY,
        KILL,
        FILEOPEN,
        FILECLOSE,
        DROPOUT,
        GREEDYSTATECHANGE,
        REQUESTSTART,
        REQUESTEND,
        REQUESTCANCEL,
        REQUESTCOMPLETE,
        USEREVENT,

        QUEUEFILE,
        QUEUEMEM,
        CANCEL,
        GET,
        RELEASE,

        // internal tracepoints
        READCALLBACK,
        PARSECHUNKS,
        RESTARTSTREAM,
        STARTNEXTREQUEST,
        SCHEDULINGREAD,
        ADDCHUNKTOGETTABLEQUEUE,
        UNKNOWN
    };

    /**
        \brief File Device Event Types

        The ProfilerEvent structure contains event values that
        corresponds to the event that requested the execution
        of the callback function.

        \importlib rwfilesystem
    */
    enum ProfilerEvent
    {
        ProfilerEventOpen,              ///< a file was opened.  param contains the name
        ProfilerEventOpenComplete,      ///< open completed.  param contains the handle
        ProfilerEventClose,             ///< a file was closed.  param contains the handle
        ProfilerEventCloseComplete,     ///< close operation completed. param contains the handle
        ProfilerEventRead,              ///< a read was enqueued.  param is a pointer to a ProfilerEventReadStruct struct.
        ProfilerEventReadComplete,      ///< a read completed.  param is a pointer to an ProfilerEventReadCompleteStruct struct.
        ProfilerEventWrite,             ///< a write was enqueued.  param is a pointer to a ProfilerEventReadStruct struct.
        ProfilerEventWriteComplete,     ///< a write completed.  param is a pointer to an uint64_t
        ProfilerEventSeek,              ///< a seek was enqueued.  param is a pointer to a ProfilerEventSeekStruct struct.
        ProfilerEventDeviceReadStart,   ///< a read started.  param is a pointer to a ProfilerEventReadOrWriteStruct struct.
        ProfilerEventDeviceReadEnd,     ///< a read ended. param is a pointer to a ProfilerEventReadOrWriteStruct struct.
        ProfilerEventDeviceWriteStart,  ///< a write started.  param is a pointer to a ProfilerEventReadOrWriteStruct struct.
        ProfilerEventDeviceWriteEnd,    ///< a write ended. param is a pointer to a ProfilerEventReadOrWriteStruct struct.
        ProfilerEventDeviceSeekStart,   ///< a seek started.  param is a pointer to a ProfilerEventSeekStruct2 struct.
        ProfilerEventDeviceSeekEnd,     ///< a seek ended.  param is a pointer to a ProfilerEventSeekStruct2 struct.

        // RwBigFile
        ProfilerEventBigFileOpen,       ///< a big file entry was opened.  param is a pointer to a ProfilerEventBigFileStruct struct.
        ProfilerEventBigFileClose,      ///< a big file entry was closed.  param is a pointer to the devfilehandle.

        // Custom
        ProfilerEventAdditionalFileDetail, ///< additional file data information.  useful for those applications that don't use
                                          ///< vanilla rwfilesystem. param is a pointer to a ProfilerEventAdditionalFileDetailStruct
        // Streamer
        ProfilerEventStreamBufferInfo,
        ProfilerEventStreamDropOut,
        ProfilerEventStreamFileOpen,
        ProfilerEventStreamFileClose,
        ProfilerEventStreamGreedyStateChange,
        ProfilerEventStreamRequestInfo,

        // StreamTrace
        ProfilerEventStreamStateInfo,
        ProfilerEventStreamReadInfo,
        ProfilerEventStreamApiCallInfo,
        ProfilerEventStreamChunkInfo,
        ProfilerEventUnknown
    };

    struct ProfilerEventStreamStateInfoStruct
    {
        ProfilerCommand command;
        unsigned int tick;
        void * streamhandle;
        unsigned char * bufferstart;    // true start of circular buffer
        int             state;          // stream state
        unsigned char * datastart;      // first chunk in buffer
        unsigned char * datatail;       // start of unparsed chunks
        unsigned char * dataend;        // destination of next read
        Handle *        fhandle;        // handle of current file
        int64_t         foffset;        // offset of next read
    };

    struct ProfilerEventStreamReadInfoStruct
    {
        ProfilerCommand command;
        unsigned int tick;
        void * streamhandle;
        Handle *        fhandle;
        unsigned char * addr;
        int64_t         ofs;
    };

    struct ProfilerEventStreamApiCallInfoStruct
    {
        ProfilerCommand command;
        unsigned int tick;
        void * streamhandle;

        // Create parameters
        int requests;
        int buffersize;

        // destroy & kill
        // just need the handle

        // Queue commands
        char fname[32];
        void *addr;
        int offset;

        // get/release
        void *chunk;
        void *pData;            // chunk data pointer
        unsigned int chunksize; // chunk size
        int requestid;

        // gettable, bufferusage
        int amount;             // how much data is available or how much buffer is being used
    };

    struct ProfilerEventStreamChunkInfoStruct
    {
        ProfilerCommand command;
        unsigned int tick;
        void * streamhandle;
        void *chunkaddr;
        void *pData;
        unsigned int size;
        int requestid;
        int endofstream;
    };

    struct ProfilerEventStreamBufferInfoStruct
    {
        ProfilerCommand command;
        unsigned int tick;
        void * shandle;
        int amount;
        Handle * fhandle;
        int state;
        int requestid;
        bool greedystate;
    };

    struct ProfilerEventStreamDropOutStruct
    {
        ProfilerCommand command;
        unsigned int tick;
        void * shandle;
        Handle * fhandle;
        float percentage;
    };

    struct ProfilerEventStreamFileOpenStruct
    {
        ProfilerCommand command;
        unsigned int tick;
        void * shandle;
        Handle * fhandle;
        int64_t foffset;
        int requestid;
        char fname[32];
    };

    struct ProfilerEventStreamFileCloseStruct
    {
        ProfilerCommand command;
        unsigned int tick;
        void * shandle;
        Handle * fhandle;
        int requestid;
    };

    struct ProfilerEventStreamGreedyStateChangeStruct
    {
        ProfilerCommand command;
        unsigned int tick;
        void * shandle;
        int greedylevel;
        bool greedystate;
    };

    struct ProfilerEventStreamRequestInfoStruct
    {
        ProfilerCommand command;
        unsigned int tick;
        void * shandle;
        int requestid;
        int reason;
        Handle * fhandle;
        char fname[32];
    };

    /**
        \brief This information contains the async OpenComplete
        information when an OpenComplete is issued as the
        event within the callback function set by
        rw::core::filesys::Manager::SetEventProfiler

        \importlib rwfilesystem
    */
    struct ProfilerEventOpenCompleteStruct
    {
        // Has to be here for backward compatability
        intptr_t handle;            ///< handle (rwfilesystem native)

        // New data
        const char *filePath;
        void *devFileHandle;

        ProfilerEventOpenCompleteStruct( intptr_t _handle, const char *_filePath, void *_devFileHandle )
            : handle(_handle), filePath(_filePath), devFileHandle( _devFileHandle )
        {}
    };


    /**
        \brief This information contains the async read
        information when a read is issued as the
        event within the callback function set by
        rw::core::filesys::Manager::SetEventProfiler

        \importlib rwfilesystem
    */
    struct ProfilerEventReadStruct
    {
        intptr_t handle;      ///< handle
        uint64_t ofs;         ///< offset at which the read was requested
        uint64_t nbytes;      ///< number of bytes requested
        const void *raddr;    ///< address to which the data will be copied
        int priority;         ///< priority of the read

        ProfilerEventReadStruct(intptr_t _handle, uint64_t _ofs, uint64_t _nbytes, const void *_raddr, int _priority)
            : handle(_handle), ofs(_ofs), nbytes(_nbytes), raddr(_raddr), priority(_priority)
        {}
    };

    /**
        \brief This information contains the async ReadComplete
        information when an ReadComplete is issued as the
        event within the callback function set by
        rw::core::filesys::Manager::SetEventProfiler

        \importlib rwfilesystem
    */
    struct ProfilerEventReadCompleteStruct
    {
        // Has to be here for backward compatability
        uint64_t size;

        // New data
        intptr_t handle;            ///< handle (rwfilesystem native)
        void     *buffer;            ///< buffer

        ProfilerEventReadCompleteStruct( uint64_t _size, intptr_t _handle, void *_buffer )
            : size(_size), handle(_handle), buffer(_buffer)
        {}
    };

    /**
        \brief This information contains the seek
        information when a seek is issued as the
        event within the callback function set by
        rw::core::filesys::Manager::SetEventProfiler

        \importlib rwfilesystem
    */
    struct ProfilerEventSeekStruct
    {
        intptr_t handle;      ///< handle
        int whence;           ///< seek origin
        uint64_t  prevOffset; ///< offset before seek started
        uint64_t  newOffset;  ///< offset after seek operation finished

        ProfilerEventSeekStruct(intptr_t _handle, int _whence, uint64_t  _prevOffset, uint64_t  _newOffset)
            : handle(_handle), whence(_whence), prevOffset(_prevOffset), newOffset(_newOffset)
        {}
    };

    /**
        \brief This information contains the seek
        information when a seek is issued as the
        event within the callback function set by
        rw::core::filesys::Manager::SetEventProfiler

        \importlib rwfilesystem
    */
    struct ProfilerEventSeekStruct2
    {
        intptr_t handle;      ///< handle
        uint64_t  offset;     ///< offset
        void *buffer;         ///< buffer

        ProfilerEventSeekStruct2(intptr_t _handle, uint64_t  _offset, void *_buffer )
            : handle(_handle), offset(_offset), buffer(_buffer)
        {}
    };


    /**
        \brief This information contains the read/write
        information when a read/write is issued at the
        start/end of the event within the callback
        function set by
        rw::core::filesys::Manager::SetEventProfiler

        \importlib rwfilesystem
    */
    struct ProfilerEventReadOrWriteStruct
    {
        intptr_t  handle;       ///< handle
        uint64_t  size;         ///< size of the data
        uint64_t  location;     ///< location of the data on disk (XBOX only)
        void      *buffer;      ///< buffer
        int       priority;     ///< priority

        ProfilerEventReadOrWriteStruct( intptr_t _handle, uint64_t _size, uint64_t _location, void *_buffer, int _priority )
            : handle(_handle), size(_size), location( _location ), buffer(_buffer), priority(_priority)
        {}
    };


    /**
        \brief This information contains the bigfile
        information when a read/write is issued of the
        event within the callback function set by
        rw::core::filesys::Manager::SetEventProfiler

        \importlib rwfilesystem
    */
    struct ProfilerEventBigFileOpenStruct
    {
        const char *bigfileName;        ///< name of the BIG file
        const char *fileName;           ///< name of the (small) file
        void       *voidPtr;            ///<

        ProfilerEventBigFileOpenStruct( const char * _bigfileName, const char *_fileName, void *_voidPtr )
            : bigfileName(_bigfileName ), fileName(_fileName), voidPtr(_voidPtr)
        {}
    };


    /**
        \brief This information contains the async read
        information when a read is issued as the
        event within the callback function set by
        rw::core::filesys::Manager::SetEventProfiler

        \importlib rwfilesystem
    */
    struct ProfilerEventAdditionalFileDetailStruct
    {
        intptr_t    handle;   ///< handle
        const void  *buffer;  ///< buffer
        const char* fileName; ///< alternate file name
        uint64_t    fileId;   ///< fileId
        uint32_t    fileType; ///< file Type
        ProfilerEventAdditionalFileDetailStruct(intptr_t _handle, const void *_buffer, const char *_fileName, uint64_t _fileId, uint32_t _fileType)
            : handle(_handle), buffer(_buffer), fileName(_fileName), fileId(_fileId), fileType(_fileType)
        {}
    };


    /**
        \brief To set up a callback for file system profiler events, subclass
        the FileSysProfiler class and implement the Callback function.

        \importlib rwfilesystem

        \sa rw::core::fileSys::Manager::SetEventProfiler
    */
    class RWCORELIB_API FileSysProfiler
    {
    public:
        /**
            This function is called each time an open, close, read, or write
            is scheduled or completes.  Implement this function in your subclass.

            \importlib rwfilesystem
        */
        virtual void Callback(
            ProfilerEvent event,    ///< the type of event
            const char *devicename, ///< device on which the event happened.  Each device can only schedule one event at a time, and a new event cannot begin until the current one completes.  However, if you have multiple devices, each device may enter the callback function concurrently.
            void *param,            ///< parameter may be NULL, or it may contain additional information about the event (the name of the file being opened, for example).
            void *context           ///< user context parameter passed to rw:core::fileSys::Manager::SetEventProfiler
            ) = 0;

        // default constructor & destructor
        FileSysProfiler() {}
        virtual ~FileSysProfiler() {}
    };
}}}

#endif

