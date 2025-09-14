// (c) Electronic Arts. All Rights Reserved.

/**
\file
Streamed file interface specification.
*/

#ifndef RW_CORE_FILESYS_STREAM_H
#define RW_CORE_FILESYS_STREAM_H

#include "EABase/eabase.h"
#include "EASTL/intrusive_list.h"
#include "eathread/eathread_futex.h"

#include "rw/core/filesys/config.h"
#include "rw/core/filesys/async.h"

// This disables a warning that will come up in dll-builds since this class contains
// templated member objects that cannot be properly exported.  This should not pose a 
// problem however, because there are no public methods which allow client code to
// touch these objects.  However, as a precaution, please be sure to use the same 
// compiler settings when compiling the dll as when compiling the application that
// uses it.
// TODO: change the interface to this method to avoid exposing the templated objects.
#if defined(EA_COMPILER_MSVC)
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

namespace rw {
namespace core {
namespace filesys {

// forward declarations
struct FilterStruct;
struct RequestStruct;
struct TapStruct;
struct ChunkStruct;

/*!
  \brief The Stream class is used for files which are too large or do
  not need to be in memory at the same time.  Instead, as much
  of the file as possible is read in, and then parsed with a
  user-defined callback function.  The user then calls
  Stream::GetChunk to obtain chunks of data.
*/
class RWCORELIB_API Stream
{
public:

    enum {
        BUFFERALIGN = 16    ///< BUFFERALIGN Platform-dependent optimal data alignment value. Misaligned reads will succeed, but may have performance penalties since the CPU may have to move some data with DMA.
    };

    /*!
      \brief Current state of a stream
        \importlib rwfilesystem
    */
    enum State
    {
        STATE_IDLE=0,       ///< nothing to read, no requests pending
        STATE_RUNNING,      ///< streamer is reading into stream buffer
        STATE_STOPPED       ///< request queued but stream buffer is full or there are no more chunk structures
    };

    /*!
      \brief Current state of a specific stream request
        \importlib rwfilesystem
    */
    enum RequestState
    {
        REQUESTSTATE_FREE=0,        ///< =0    request has been freed (or never existed)
        REQUESTSTATE_PENDING,       ///< request waiting to execute
        REQUESTSTATE_EXECUTING,     ///< request is executing
        REQUESTSTATE_COMPLETED,     ///< request has completed reading
        REQUESTSTATE_CANCELLED      ///< request was cancelled by user
    };

    enum ErrorState
    {
        ERRORSTATE_NONE=0,          ///< No errors were encountered
        ERRORSTATE_READ_FAILED      ///< Internal Async Op has failed to read
    };

    /*!
      \brief Default stream priority levels
        \importlib rwfilesystem
    */
    enum PriorityLevel
    {
        STREAM_PRIORITYLOW = -1000,     ///< default normal priority
        STREAM_PRIORITYHIGH = 1000      ///< default greedy priority
    };

    enum RequestFlags
    {
        /// default behavior for this stream.  Use on QueueRequest only, not
        /// Create()
        REQUESTFLAG_DEFAULT = 0x00,

        /// unparsed data at the end of the request (i.e. data that was read
        /// from the file but that the parser function has not identified as
        /// part of a complete chunk when the end-of-file is found) is made
        /// into a chunk and appended to the chunk list as the last chunk in
        /// the request.  This is the normal behavior.
        REQUESTFLAG_RETURN_UNPARSED_DATA = 0x01,

        /// Unparsed data remaining in the stream buffer when the end-of-file
        /// is found will be discarded.  This may be useful if your parser can
        /// detect when the stream has ended and the return of a partial
        /// chunk could lead to undefined behavior in your application.  An
        /// incomplete chunk can be returned when the end-of-file is found
        /// prematurely, which happens when an unreliable connection is
        /// terminated unexpectedly.  You can detect that the stream is done
        /// by using the IsEndOfStream() call or when GetState() returns
        /// STATE_IDLE.
        REQUESTFLAG_IGNORE_UNPARSED_DATA = 0x02
    };

    /*!
      \brief Handle to a queued stream request
        \importlib rwfilesystem
    */
    typedef int RequestId;

    /*!
      \brief Information about a chunk.
      \importlib rwfilesystem
    */
    struct ChunkInfo
    {
        RequestId reqId;                    ///< Request ID of the request this chunk belongs to.
        unsigned int size;                  ///< size of the chunk in bytes
        void *pData;                        ///< pointer to data in the circular buffer
    };

    /*!
      \brief Possible PARSEFUNC return values.  Your parser must
      return one of these values.
      \importlib rwfilesystem
    */
    enum ParseReturnValue
    {
        PARSE_RETURN_MORE_DATA_NEEDED,      ///< not enough data for another complete chunk, or the data is not recognized by this filter
        PARSE_RETURN_CHUNK,                 ///< got a complete chunk, continue parsing
        PARSE_RETURN_CHUNK_END_REQ          ///< got a complete chunk, but stop parsing because the end of the request has been reached
    };

    /*!
      \brief Filter function signature.

      The parse function will be called once for each filter
      that has been installed with Stream::SetFilter.  The filter
      function should check if the data is recognised and return;
      a chunk size if a complete chunk is available.  If the filter
      does recognize the chunk but not enough data is available,
      the parser should return PARSE_RETURN_MORE_DATA_NEEDED.
      \importlib rwfilesystem
      \param pData - pointer to data to parse
      \param filtercontext - context pointer passed to Stream::SetFilter when the filter was added.
    */
    typedef bool (FILTERFUNC)(void *pData, void *filtercontext);

    /*!
      \brief Parse function signature.

      Your parse function is called whenever the streamer has
      more data available for parsing.  The parse function should
      examine the data and return one of the ParseReturnValue
      values to the streamer.  If there are multiple filters installed,
      the parse function will be called once for each filter until
      either there are no more filters or a chunk is returned.
      If a chunk is found, set its size in the blocksize parameter
      before returning.

      \importlib rwfilesystem

      \param pData - (input) pointer to data to be parsed
      \param datasize - (input) size in bytes of data block
      \param reqid - (input) request ID which this data block belongs to
      \param context - (input) request context pointer from Stream::QueueFile or Stream::QueueMem
      \param filterfunc - (input) filter function (may be NULL).  The parser is called once for each filter until a chunk is returned.
      \param filtercontext - (input) filter context pointer
      \param blocksize - (output) if a complete chunk is available, set its size here.
    */
    typedef ParseReturnValue (PARSEFUNC)( void *pData,
        unsigned int datasize, RequestId reqid, void *context,
        FILTERFUNC *filterfunc, void *filtercontext,
        unsigned int *blocksize);

    // Stream creation, setup and destruction.
    static Stream *     Create(int requests, void *buffer, int bufsize, int readblocksize=0, PARSEFUNC *default_parse_func=NULL, void *context=NULL, uint32_t flags = REQUESTFLAG_RETURN_UNPARSED_DATA);
    void                SetFilter(int tapnum, FILTERFUNC *filterfunc, void *context);
    void                SetFilter(FILTERFUNC *filterfunc, void *context = NULL);
    void                Destroy(bool allowBlocking = true);
    void                Delete();
    void                CloseFileHandle();
    void                CloseFileHandleAsync();
    bool                IsUsingDeviceDriver(const DeviceDriver *driver);

    // I/O coordination settings.
    void                SetPriority(int prioritylow, int priorityhigh);
    void                SetGreedyLevel(int greedylevel);
    void                SetGreedyState();
    void                SetNonGreedyState();
    void                SetForceStarvation(bool enable);
    void                SetStarvationThreshold(unsigned int threshold);
    void                SetParserThresholdValues(int startThreshold, int stopThreshold);
    void                SetBufferLowCallback(void (*cbFunc)(Stream *, void *), void *context, int threshold);

    // Conversions between tap numbers and tap handles.
    Stream *            CreateTap();
    int                 TapNum() const;

    // Request queuing and cancellation.
    RequestId           QueueFile(const Handle *, int64_t offset=0, PARSEFUNC *parse_func=NULL, void *parsecontext=NULL, uint32_t flags = REQUESTFLAG_DEFAULT);
    RequestId           QueueFile(const char *fname, int64_t offset=0, PARSEFUNC *parse_func=NULL, void *parsecontext=NULL, uint32_t flags = REQUESTFLAG_DEFAULT);
    RequestId           Requeue(int64_t offset=0, PARSEFUNC *parse_func=NULL, void *parsecontext=NULL, uint32_t flags = REQUESTFLAG_DEFAULT);
    RequestId           QueueMem(void *address, unsigned int length, PARSEFUNC *parse_func=NULL, void *parsecontext=NULL);
    void                CancelRequest(RequestId requestid);
    void                Kill();

    // Chunk get and release.
    ChunkInfo *         GetChunk();
    void                ReleaseChunk(ChunkInfo *chunk);

    // Status information.
    int                 Gettable() const;
    int                 Gettable(RequestId requestid) const;
    State               GetState() const;
    bool                IsEndOfStream() const;
    int                 BufferSize() const;
    int                 BufferUsage() const;
    RequestId           TapRequestId() const;
    RequestState        GetRequestState(RequestId requestid) const;
    uint64_t            GetFileSize() const;
    bool                IsUsingDevice(const DeviceDriver *pDeviceDriver) const;
    ErrorState          GetErrorState(RequestId requestid);
    bool                IsFileHandleClosed() const;
    bool                IsOpComplete() const;

private:

    // Use the Stream::Create function to create a stream
    Stream(int requests, void *buffer, int bufsize,
        int readblocksize, Stream::PARSEFUNC *parse, void *parsecontext,
        uint32_t flags);

    // Use Stream::Destroy to destroy a stream.
    ~Stream();

    // this class is non-copyable
    Stream(const Stream &);
    Stream& operator=(const Stream& );

    static void *operator new(size_t size) { return RWFILESYSTEM_ALLOCATE(size); }
    static void *operator new(size_t, void *ptr) { return ptr; }
    static void operator delete(void *ptr) { return Manager::Free(ptr); }

    enum ParseStopReason
    {
        CONTINUE,       ///< the parser should continue (not return)
        NEED_MORE_DATA, ///< not enough data to form another complete chunk
        END_CHUNK_FOUND ///< the end chunk has been found
    };

    Stream *GetStreamHeaderFromHandle(const Stream *handle) const;
    void decbufferusage(int amount);
    RequestStruct *getfreerequest();
    void queuerequest(RequestStruct *req);
    RequestStruct *locaterequest(Stream::RequestId requestid);
    void freerequest(RequestStruct *req);
    ParseStopReason parsechunks();
    static void opencallback(AsyncOp *op);
    static void closecallback_filehandleasync(AsyncOp *op);
    static void closecallback(AsyncOp *op);
    static void filereadcallback(AsyncOp *op);
    void readcallback();

    void startnextrequest(int priority);
    void restartstream(int priority);

    RequestId QueueFile(const char *fname, Handle *, int64_t offset, PARSEFUNC *parse_func, void *parsecontext, uint32_t flags);

    unsigned int            id;                         ///< stream buffer signature
    unsigned int            flags;                      ///< general-purpose flags
    EA::Thread::Futex       futex;
    RequestStruct *         request;                    ///< pointer to request array
    int                     requests;                   ///< size of request array
    eastl::intrusive_list<FilterStruct> filter;         ///< list of filters
    eastl::intrusive_list<TapStruct> tap;               ///< list of taps
    eastl::intrusive_list<ChunkStruct> chunk;           ///< list of active chunks

    unsigned char *         actualbufferstart;          ///< true start of circular buffer
    unsigned char *         bufferstart;                ///< start of circular buffer (aligned)
    unsigned char *         bufferend;                  ///< end of circular buffer + 1

    volatile Stream::State  state;                      ///< stream state
    int                     prioritylow;                ///< low file system priority level
    int                     priorityhigh;               ///< high file system priority level
    int                     greedylevel;                ///< usage level for greedy state change
    bool                    greedystate;                ///< greedy mode state (0=off, 1=on)
    bool                    forcestarvation;            ///< starve mode state 
    bool                    isfilehandleclosed;         ///< has the file handle been closed. 
    int                     starvationthreshold;        ///< causes the streamer to fill until this size is reached (bytes)
    int                     bufferusage;                ///< circular buffer usage level
    unsigned char *         datastart;                  ///< first chunk in buffer
    unsigned char *         datatail;                   ///< start of unparsed chunks
    unsigned char *         dataend;                    ///< destination of next read
    RequestStruct *         firstreq;                   ///< first request in queue
    RequestStruct *         curreq;                     ///< request currently being processed
    RequestStruct *         lastreq;                    ///< last request in queue
    RequestStruct *         freereq;                    ///< first free request

    Stream::PARSEFUNC *     parsefunc;
    void *                  parsecontext;
    char                    fname[FILESYS_MAX_PATH]; ///< name of current file
    Handle *                fhandle;            ///< handle of current file
    int64_t                 foffset;            ///< offset of next read
    AsyncOp                 mAsyncOp;           ///< file operation
    int64_t                 readsize;           ///< requested read size
    int                     readblocksize;      ///< read block size calculated at creation time
    uint64_t                filesize;           ///< size of the file.
    uint32_t                requestflags;       ///< default request flag

    int                     startParseThreshold;///< amount of parseable data (dataend-datatail) that must be available before parsing begins
    int                     stopParseThreshold; ///< when the amount of parseable data (dataend-datatail) falls below this amount, parsing is halted until it rises above startParseThreshold;

    /// Function called when the buffer gets low.
    void                  (*mLowBufferWarningCallback)(Stream *, void *);

    /// Context pointer for the low buffer warning callback
    void                   *mLowBufferWarningCallbackContext;

    /// Percentage full the buffer must fall below to trigger the warning callback.
    int                     mLowBufferWarningThreshold;
};

}
}
}

#if defined(EA_COMPILER_MSVC)
#pragma warning(pop)
#endif

#endif
/* RWPUBLICEND */
