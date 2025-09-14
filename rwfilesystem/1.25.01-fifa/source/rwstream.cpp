// (c) Electronic Arts. All Rights Reserved.

/**
\file
Implements rw::core::filesys::Stream.
*/

#include "EAAssert/eaassert.h"

#include "rw/core/filesys/stream.h"
#include "rw/core/filesys/manager.h"
#include "rw/core/filesys/async.h"
#include "private_rwprofiler.h"
#include "private_rwhandle.h"
#include "eathread/eathread_futex.h"
#include "EAStdC/EAAlignment.h"
#include "EASTL/intrusive_list.h"
#include "private_rwhandle.h"

#include <stdio.h>
#include <string.h>

using namespace EA::StdC;

namespace rw {
namespace core {
namespace filesys {

// Signature used to validate stream buffer.
#ifdef EA_SYSTEM_LITTLE_ENDIAN
    const unsigned int STREAM_ID = ((static_cast<uint32_t>('M') << 24) | (static_cast<uint32_t>('R') << 16) | (static_cast<uint32_t>('T') << 8) | static_cast<uint32_t>('S'));
#else
    const unsigned int STREAM_ID = ((static_cast<uint32_t>('S') << 24) | (static_cast<uint32_t>('T') << 16) | (static_cast<uint32_t>('R') << 8) | static_cast<uint32_t>('M'));
#endif

#if defined(EA_COMPILER_SN)
// ignore unused variables in release mode
#pragma diag_suppress=178,552
#endif

// Limits on stream creation parameters.
#define MAX_REQUESTS 256

enum ReadType
{
    FILEREAD,       ///<  read chunks from file
    MEMREAD         ///<  read chunks from memory
};

enum ChunkStatus
{
    GETTABLE,       ///<  chunks are gettable when they're first put on the chunk queue
    RECEIVED,       ///<  received chunks are under user control
    RELEASED        ///<  released chunks are ready to go back on the free queue
};

struct ChunkStruct : public eastl::intrusive_list_node
{
    ChunkStruct(Stream::RequestId id, int tap, unsigned int size, void *pData) :
        status(GETTABLE), tapnum(tap)
    {
        mpNext = NULL;
        mpPrev = NULL;

        ci.reqId = id;
        ci.size = size;
        ci.pData = pData;
    }

    ~ChunkStruct() {}

    static void *operator new(size_t size) { return RWFILESYSTEM_ALLOCATE(size); }
    static void operator delete(void *ptr) { return Manager::Free(ptr); }

    Stream::ChunkInfo ci;
    ChunkStatus status;         ///<  GETTABLE/RECEIVED/RELEASED
    int tapnum;                 ///<  tap this chunk belongs to
};

struct TapStruct : public eastl::intrusive_list_node
{
    TapStruct(Stream *strm, int num) :
        stream(strm), tapnum(num+1),
        gettable(0), gettablechunk(NULL)
    {
        mpNext = NULL;
        mpPrev = NULL;
    }

    ~TapStruct()
    {
        stream = NULL;
        return;
    }

    static void *operator new(size_t size) { return RWFILESYSTEM_ALLOCATE(size); }
    static void operator delete(void *ptr) { return Manager::Free(ptr); }

    Stream *stream;             ///<  pointer back to stream header
    int     tapnum;             ///<  tap number of this tap (1 origin)
    int     gettable;           ///<  amount of gettable data at this tap
    eastl::intrusive_list<ChunkStruct>::iterator gettablechunk; ///<  pointer to first gettable chunk on this tap
};

struct FilterStruct : public eastl::intrusive_list_node
{
    FilterStruct(Stream::FILTERFUNC *_func, void *_context, int tap) :
        func(_func), context(_context), tapnum(tap)
    {
        mpNext = NULL;
        mpPrev = NULL;
    }

    ~FilterStruct()
    {
        context = NULL;
    }

    static void *operator new(size_t size) { return RWFILESYSTEM_ALLOCATE(size); }
    static void operator delete(void *ptr) { return Manager::Free(ptr); }

    Stream::FILTERFUNC *func;   ///<  function to call
    void *              context;///<  context parameter to pass to func.
    int                 tapnum; ///<  destination tap number (1 origin)
};

struct RequestStruct
{
    Stream::RequestId   id;                 ///<  request ID
    Stream::RequestState state;             ///<  request state
    RequestStruct *     prev;               ///<  previous request in queue
    RequestStruct *     next;               ///<  next request in queue
    ReadType            type;               ///<  type of request
    char                fname[FILESYS_MAX_PATH]; ///<  file name for file request (mutually exclusive with handle)
    filesys::Handle *   fhandle;            ///<  handle of file to read from (mutually exclusive with fname)
    unsigned char *     address;            ///<  address for memory request
    int64_t             parm;               ///<  request parameter (length for memory streams, file offset for file streams)
    unsigned char *     datastart;          ///<  first chunk of request in buffer
    bool                endoffile;          ///<  has the end-of-file for this request been reached?
    Stream::PARSEFUNC * parsefunc;          ///<  parsing function for this request
    void *              parsecontext;       ///<  parsing context parameter for this request
    int                 gettable;           ///<  amount of gettable data for this request
    uint32_t            flags;              ///<  what to do with unparsed data when the end-of-stream is found
    bool                ended;              ///<  has this request been ended already?
    Stream::ErrorState  error;
};

// Request ID counter. Used to generate a unique request ID for each request.
// A request ID contains the index of the request in the lower 8 bits and
// an integer value in the upper 24 bits. By incrementing this integer each
// time a request in generated, a unique identifier is created. Since the
// variable is global, identifiers will not be duplicated in different
// streams, making it impossible to confuse them.
static int requestidcounter;

///<  mask off all but the least significant bits of Stream::BUFFERALIGN.
const int DATAALIGNMASK = Stream::BUFFERALIGN-1;

enum StreamInternalFlags
{
    /// Flag indicating that the file handle belongs
    /// to the caller, not to the stream object.
    STREAM_FLAG_USER_FILE_HANDLE = 0x01,

    /// Indicates that the streamer was in the parse-enabled
    /// state if the amount of gettable data is between the
    /// stop and start triggers.  (If the amount is less than
    /// stop, the parser will definitely be stopped, and if
    /// it is more than start, it will definitely be enabled.
    STREAM_FLAG_PARSE_TRIGGER_PARSING = 0x02,

    /// Indicates that the streamer's buffer was at one point
    /// at least half full.  If the stream buffer should ever
    /// drop below the mLowBufferWarningThreshold when this flag
    /// is set, the LowBufferWarning callback is triggered and
    /// this flag is cleared.  Once the buffer tops up again,
    /// this flag is set, so another low buffer warning can be
    /// triggered should it become necessary.
    STREAM_FLAG_HALF_FULL = 0x04
};

// this is the default parsing function used if the user never
// passes one in.
static Stream::ParseReturnValue stream_default_parser(void *,
    unsigned int datasize,
    Stream::RequestId,
    void *,
    Stream::FILTERFUNC *,
    void *,
    unsigned int *blocksize)
{
    const unsigned int DEFAULT_DATA_SIZE = 2*1024;

    if (datasize >= DEFAULT_DATA_SIZE)
    {
        *blocksize = DEFAULT_DATA_SIZE;
        return Stream::PARSE_RETURN_CHUNK;
    }

    return Stream::PARSE_RETURN_MORE_DATA_NEEDED;
}

// Validates the specified stream handle and locates the associated
// stream header and tap structure. Returns an error code or zero
// if the handle is valid.
Stream *Stream::GetStreamHeaderFromHandle(const Stream *tapstruct) const
{
    EA_ASSERT(tapstruct != 0);

    //lint -e{826} suspicious pointer-to-pointer conversion
    Stream *strm = reinterpret_cast<const TapStruct *>(tapstruct)->stream;
    EA_ASSERT(strm->id == STREAM_ID);

    return strm;
}
// Determines whether a given pointer is in-between a start pointer and
// an end pointer in a circular buffer, given that the interval may
// wrap. Note that the end pointer is not part of the interval and that
// startptr == endptr represents an empty interval (no in-between).
static bool inbetween(
    const unsigned char *startptr,
    const unsigned char *endptr,
    const unsigned char *ptr)
{
    if (startptr <= endptr)
        return (ptr >= startptr) && (ptr < endptr);
    else
        return (ptr >= startptr) || (ptr < endptr);
}

// Decrements a stream's buffer usage value by a specified amount and
// determines if the value has crossed the greedy threshold. If so, greedy
// mode is enabled and any pending file operation has its priority level
// boosted to high.

void Stream::decbufferusage(int amount)
{
    int             oldbufferusage;
    int             bufferusage;
    int             greedylevel;

    this->futex.Lock();
    oldbufferusage = this->bufferusage;
    bufferusage = oldbufferusage - amount;
    this->bufferusage = bufferusage;

    if (curreq->type == FILEREAD)
        LogStreamBufferOperation(USAGE, this, curreq->id, bufferusage, fhandle, static_cast<int>(state), greedystate);

    if ((this->state == STATE_RUNNING) &&
        (this->curreq->state == REQUESTSTATE_EXECUTING) &&
        ((this->flags & STREAM_FLAG_HALF_FULL) == STREAM_FLAG_HALF_FULL))
    {
        int strmBufSize = static_cast<int>(this->bufferend - this->actualbufferstart);
        int percentFull = (oldbufferusage * 100) / strmBufSize;

        if (percentFull < this->mLowBufferWarningThreshold)
        {
            this->flags &= ~STREAM_FLAG_HALF_FULL;

            if (this->mLowBufferWarningCallback != NULL)
            {
                this->mLowBufferWarningCallback(reinterpret_cast<Stream *>
                    (&this->tap.front()), this->mLowBufferWarningCallbackContext);
            }

            if (curreq->type == FILEREAD)
                LogStreamDropOutEvent(this, fhandle, static_cast<float>(percentFull) / 100.0f);
        }
    }
    this->futex.Unlock();

    greedylevel = this->greedylevel;
    if ((oldbufferusage >= greedylevel) && (bufferusage < greedylevel))
    {
        bool currentGreedyState = this->greedystate;

        this->greedystate = 1;

        if (!currentGreedyState)
            LogStreamGreedyStateChangeEvent(this, greedylevel, greedystate);

        if (this->state == Stream::STATE_RUNNING)
            this->mAsyncOp.SetPriority(this->priorityhigh);
    }
}

// Get a free request. The request is removed from the free list and
// a unique request ID is generated. Returns NULL if no free request
// is available.

// Note that we do not allow the request ID counter to become zero
// because request ID zero has a reserved meaning (no request).

RequestStruct *Stream::getfreerequest()
{
    RequestStruct   *req;

    this->futex.Lock();

    if (this->freereq == NULL)
    {
        req = NULL;
    }
    else
    {
        req = this->freereq;
        this->freereq = req->next;

        requestidcounter += 256;
        if (requestidcounter == 0) requestidcounter = 256;
        req->id = (req->id & 0xff) | requestidcounter;
    }

    this->futex.Unlock();

    return req;
}

// Inserts the specified request at the end of the request queue and
// places it in the pending state.

void Stream::queuerequest(RequestStruct *req)
{
    req->state = Stream::REQUESTSTATE_PENDING;
    req->next = NULL;

    this->futex.Lock();

    if (this->lastreq == NULL)  // queue empty
    {
        req->prev = NULL;
        this->firstreq = req;
        this->curreq = req;
        this->lastreq = req;
    }
    else
    {
        req->prev = this->lastreq;
        this->lastreq->next = req;
        this->lastreq = req;
    }

    this->futex.Unlock();
}

// Locate a request in the request queue, given its request ID.
// Returns NULL if the request is not in the queue.

RequestStruct *Stream::locaterequest(Stream::RequestId requestid)
{
    RequestStruct   *req;
    int             index;

    // Extract the index from the request ID to locate the request.
    index = requestid & 0xff;
    if (index >= this->requests) return NULL;
    req = &this->request[index];

    // Make sure the request ID matches and the request is not
    // in the free list.
    if (requestid != req->id) return NULL;
    if (req->state == Stream::REQUESTSTATE_FREE) return NULL;

    return req;
}

// Removes a request from the request queue and places it in the free list.

// Note that the structures are not locked: it is assumed that the caller
// will performed the lock and unlock operations.

void Stream::freerequest(RequestStruct *req)
{
    EA_ASSERT(req->state != REQUESTSTATE_FREE);

    if (req == this->firstreq) this->firstreq = req->next;
    else req->prev->next = req->next;

    if (req == this->lastreq) this->lastreq = req->prev;
    else req->next->prev = req->prev;

    if (req == this->curreq)
    {
        if (req->next == NULL) this->curreq = req->prev;
        else this->curreq = req->next;
    }

    req->state = Stream::REQUESTSTATE_FREE;
    req->next = this->freereq;
    this->freereq = req;
}

// Parses as many chunks in the tail end of the data buffer as possible.
// If it runs out of chunk structures or cannot parse another chunk, it
// stops at that point.  After it returns, the next time parsechunks is
// called, it will continue parsing from the current tail pointer.

// This function runs on the both the file callback thread and on
// the thread where chunks are released.  If the user cancels
// the current request, the request state will spontaneously change to
// canceled and this function must ensure that the tail pointer stops
// advancing immediately.

Stream::ParseStopReason Stream::parsechunks()
{
    RequestStruct * req;
    TapStruct *     tap;
    ChunkStruct *   chunk;
    unsigned int        chunksize;
    int         tapnum = 1;
    int         oldbufferusage;
    int         bufferusage;
    int         greedylevel;
    Stream::ParseReturnValue ret = Stream::PARSE_RETURN_MORE_DATA_NEEDED;
    ParseStopReason stopreason = CONTINUE;

    LogStreamStateInfoEvent(PARSECHUNKS, this, bufferstart, state, datastart, datatail, dataend, fhandle, foffset);

    // if we have more parseable data than the parseStartTrigger
    // threshold, set the parse flag
    if (this->dataend - this->datatail >= this->startParseThreshold)
        this->flags |= STREAM_FLAG_PARSE_TRIGGER_PARSING;

    // if the parse flag is not set at this point, or datatail is
    // ahead of dataend don't do any parsing.
    if ((this->datatail >= this->dataend) ||
        ((this->flags & STREAM_FLAG_PARSE_TRIGGER_PARSING) == 0))
    {
        stopreason = NEED_MORE_DATA;
    }

    req = this->curreq;

    // Loop until the tail pointer catches up to the end pointer
    // or the user cancels the request
    while ((stopreason == CONTINUE) &&
        (req->state != Stream::REQUESTSTATE_CANCELLED))
    {
        chunksize = 0;
        eastl::intrusive_list<FilterStruct>::iterator filterItr;
        for (filterItr = this->filter.begin(); filterItr != this->filter.end(); ++filterItr)
        {
            ret = req->parsefunc(this->datatail,
                static_cast<unsigned int>(this->dataend - this->datatail),
                req->id, req->parsecontext, filterItr->func,
                filterItr->context, &chunksize);

            if (ret != Stream::PARSE_RETURN_MORE_DATA_NEEDED)
            {
                if (ret == Stream::PARSE_RETURN_CHUNK)
                {
                    // make sure the returned chunk size is reasonable.
                    EA_ASSERT_MSG(chunksize != 0,
                        "Error - returned chunk size is 0.  Must be at least 1 byte.");
                }

                EA_ASSERT_MSG(chunksize <= static_cast<unsigned int>(this->dataend - this->datatail),
                    ("Error - returned chunk size is larger than the amount of parsable data."));

                tapnum = filterItr->tapnum;
                break;  // for loop.
            }
        }

        // if there wasn't enough data to get the next
        // complete chunk then break out of the loop.
        if (ret == Stream::PARSE_RETURN_MORE_DATA_NEEDED)
        {
            if (this->curreq->endoffile)
            {
                if ((this->curreq->flags & REQUESTFLAG_IGNORE_UNPARSED_DATA) != 0)
                {
                    // If the user wants to ignore unparsed data at the end
                    // of the stream, stop parsing now.
                    return END_CHUNK_FOUND;
                }

                // if the users' parser does not or cannot detect the end-of-stream
                // condition, and the end of the file has been reached, all the remaining
                // data will be put into a single chunk and output on tap 1.
                tapnum = 1;
                chunksize = static_cast<unsigned int>(this->dataend - this->datatail);
                ret = Stream::PARSE_RETURN_CHUNK_END_REQ;

                // If the chunksize is 0, return at this point (do not add
                // 0-sized chunks to the stream queue).
                if (chunksize == 0)
                    return END_CHUNK_FOUND;
            }
            else
            {
                // even if bufferusage is 0 here, it could be that the just-
                // released chunk will allow the tail to be moved around to
                // the start of the buffer and the streamer can continue.
                stopreason = NEED_MORE_DATA;
                break;
            }
        }

        this->futex.Lock();

        // Make the chunk gettable (unless request canceled).
        // If nothing is currently gettable on this tap,
        // the get pointer must be set.
        if (req->state == Stream::REQUESTSTATE_CANCELLED)
        {
            // Exit if canceled.
            this->futex.Unlock();
            stopreason = NEED_MORE_DATA;
            break;
        }

        // In files where the chunk size is larger than the read block size
        // (as is sometimes the case with movie files), the stream buffer
        // needs to be larger than 3x the chunk size, otherwise if a large
        // chunk is read near the end of the stream buffer, we may not be
        // able to move it to the start of the buffer once space
        // is available.  Check for that condition here.
        EA_ASSERT_FORMATTED(chunksize <=
            static_cast<unsigned int>(this->bufferend - this->actualbufferstart - this->readblocksize - 256) / 2,
            ("A chunk of size %d is too large for a stream buffer of size %d.  Refer to the documentation for Stream::Create guidelines on how to make the stream buffer large enough.",
            chunksize, this->bufferend - this->actualbufferstart));

        chunk = new ChunkStruct(req->id, tapnum, chunksize, this->datatail);
        this->chunk.push_back(*chunk);

        eastl::intrusive_list<TapStruct>::iterator tapItr = this->tap.begin();

        for (int i = 0; i < tapnum-1; ++i, ++tapItr)
        {
            EA_ASSERT(tapItr != this->tap.end());
        }
        tap = tapItr.mpNode;
        tap->gettable += static_cast<int>(chunksize);
        req->gettable += static_cast<int>(chunksize);
        EA_ASSERT(tap->gettable >= 0);
        EA_ASSERT(req->gettable >= 0);

        if (tap->gettable == static_cast<int>(chunksize))
        {
            tap->gettablechunk =
                eastl::intrusive_list<ChunkStruct>::iterator(chunk);
        }

#if defined(EA_ASSERT_ENABLED)
        {
            int tmp_gettable(0);
            eastl::intrusive_list<ChunkStruct>::iterator
                cs = tap->gettablechunk;

            if (cs != this->chunk.end())
            {
                EA_ASSERT(cs->status == GETTABLE);
            }

            while (cs != this->chunk.end())
            {
                if (cs->status == GETTABLE && cs->tapnum == tap->tapnum)
                {
                    tmp_gettable += cs->ci.size;
                }
                ++cs;
            }

            EA_ASSERT(tap->gettable == tmp_gettable);
        }
#endif

        this->datatail += chunksize;

        int endOfStream = (ret == Stream::PARSE_RETURN_CHUNK_END_REQ);
        LogStreamChunkInfoEvent(this, chunk->ci.reqId, chunk, chunk->ci.pData, chunk->ci.size, endOfStream);

        // Increment the buffer usage value and switch off greedy
        // mode if the greedy threshold was crossed.
        oldbufferusage = this->bufferusage;
        bufferusage = oldbufferusage + static_cast<int>(chunksize);
        this->bufferusage = bufferusage;
        greedylevel = this->greedylevel;
        if ((oldbufferusage < greedylevel) && (bufferusage >= greedylevel)
            && this->greedystate)
        {
            this->greedystate = 0;
            LogStreamGreedyStateChangeEvent(this, greedylevel, greedystate);
        }

        // mark the chunk as gettable so Stream::GetChunk can return it.
        chunk->status = GETTABLE;

        // if we have less parseable data than the stopParseThreshold
        // threshold, clear the parse flag and stop the parsing
        if (this->dataend - this->datatail < this->stopParseThreshold)
        {
            stopreason = NEED_MORE_DATA;
            this->flags &= ~STREAM_FLAG_PARSE_TRIGGER_PARSING;
        }

        if (curreq->type == FILEREAD)
        {
            int actualbuffersize = static_cast<int>(this->bufferend - this->actualbufferstart);
            int halffullmark = actualbuffersize / 2;
            if (bufferusage >= halffullmark)
                flags |= STREAM_FLAG_HALF_FULL;
            
            LogStreamBufferOperation(USAGE, this, curreq->id, bufferusage, fhandle, static_cast<int>(state), greedystate);
        }

        this->futex.Unlock();

        if (ret == Stream::PARSE_RETURN_CHUNK_END_REQ)
            stopreason = END_CHUNK_FOUND;

        #ifdef EA_DEBUG
            else if (this->datatail > this->dataend)
            {
                EA_FAIL_MSG("this->datatail has somehow gotten ahead of this->dataend in the parser");
            }
        #endif
    }

    return stopreason;
}

// this was where the parser was located
// File open callback function. This function is called by the async file
// module when the FILESYS_open() operation issued by function startnextrequest()
// or by function closecallback() is done.

//lint -e{818}  op cannot be const
void Stream::opencallback(AsyncOp *op)
{
    Stream *strm = static_cast<Stream *>(op->GetContext());

    strm->fhandle = op->GetResultHandle();

    if (strm->fhandle == 0)
    {
        // the file was not found.  Move on to the next request.
        strm->futex.Lock();

        // invalidate the file name so that if the next file happens to
        // have the same (meaning it won't be found either), startnextrequest
        // will still attempt to open it and we'll get back here.
        strm->fname[0] = '\0';

        // Free the request.  This is necessary because otherwise you
        // may run out of request structures and yet have no active streams
        strm->freerequest(strm->curreq);

        strm->futex.Unlock();

        // go on to the next request
        if (strm->greedystate) strm->startnextrequest(strm->priorityhigh);
        else strm->startnextrequest(strm->prioritylow);
    }
    else
    {
        LogStreamFileOpenEvent(strm, strm->curreq->id, strm->fhandle, strm->foffset, strm->fname);

        strm->flags &= ~STREAM_FLAG_USER_FILE_HANDLE;
        strm->filesize = GetSize(strm->fhandle);

        // File handle is valid, so start reading the new request
        strm->restartstream(strm->priorityhigh);
    }
}

// File close callback function. This function is called by the async file
// module when the FILESYS_close() operation issued by function startnextrequest()
// is done.

//lint -e{818}  op cannot be const
void Stream::closecallback(AsyncOp *op)
{
    Stream    *strm = static_cast<Stream *>(op->GetContext());

    if (strm->curreq->fhandle == NULL)
    {
        // Open the new stream file.
        strm->mAsyncOp.Open(strm->fname, AsyncOp::FOPEN_READONLY,
            &opencallback, strm, strm->priorityhigh);
    }
    else
    {
        strm->flags |= STREAM_FLAG_USER_FILE_HANDLE;
        strm->fhandle = strm->curreq->fhandle;
        strm->filesize = GetSize(strm->fhandle);

        // File handle is valid, so start reading the new request
        strm->restartstream(strm->priorityhigh);
    }
}

void Stream::readcallback()
{
    RequestStruct   *req;
    int64_t          bytesread;
    int64_t          skipbytes(0);

    req = this->curreq;

    LogStreamStateInfoEvent(READCALLBACK, this, bufferstart, state, datastart, datatail, dataend, fhandle, foffset);

    // Determine bytes read and detect end of file.
    if (req->type == MEMREAD)
    {
        bytesread = this->readsize;
        req->endoffile = ((this->foffset + bytesread) >= req->parm);
    }
    else
    {
        bytesread = static_cast<int>(this->mAsyncOp.GetResultSize());
        req->endoffile = (bytesread < this->readsize);
        skipbytes = this->foffset & (RW_CORE_FILESYS_READ_BUFFER_ALIGNMENT-1);
    }

    // Advance pointers.
    this->foffset += (bytesread-skipbytes);
    this->dataend += bytesread;
    this->datatail += skipbytes;

    // Parse chunks in the tail end of the data buffer.
    ParseStopReason stopreason = parsechunks();

    // If the current request was canceled, proceed to the next request.
    if (req->state == Stream::REQUESTSTATE_CANCELLED)
    {
        startnextrequest(this->priorityhigh);
        return;
    }

    if ((stopreason == END_CHUNK_FOUND) || req->endoffile)
    {
        // If an end of file condition was detected, mark the request as
        // completed and proceed to the next request.
        // Don't mark a canceled request as completed. We have to be
        // careful because the cancel can occur at any time.

        // Any unparsed data at the end of the current request is ignored

        this->futex.Lock();
        if (req->state != Stream::REQUESTSTATE_CANCELLED)
        {
            req->state = Stream::REQUESTSTATE_COMPLETED;

            int reason = 0;
            if(req->endoffile)
                reason = -1;
            else if(stopreason == END_CHUNK_FOUND)
                reason = END_CHUNK_FOUND;

            if (req->type == FILEREAD)
                LogStreamRequestOperation(REQUESTCOMPLETE, this, req->id, fhandle, reason, req->fname); 
        }
        this->futex.Unlock();

        startnextrequest(this->priorityhigh);
    }
    else if (stopreason == NEED_MORE_DATA)
    {
        // Continue reading the current request.
        restartstream(this->priorityhigh-1);
    }
#if defined(EA_DEBUG)
    else
    {
        EA_FAIL_MSG("illegal value from parsechunks");
    }
#endif
}


Stream::ErrorState Stream::GetErrorState(RequestId requestid)
{
    Stream *strm = GetStreamHeaderFromHandle(this);

    const RequestStruct *req = strm->locaterequest(requestid);

    if (req == NULL)
        return ERRORSTATE_NONE;

    return req->error;
}


// File read callback function. This function is called by the async file
// module when the FILESYS_read() operation issued by function restartstream()
// is done. It is also called directly by restartstream() when a memory
// read is executed.

//lint -e{818}  op cannot be const
void Stream::filereadcallback(AsyncOp *op)
{
    static_cast<Stream *>(op->GetContext())->readcallback();
}

// Starts executing the next pending request in the request queue. If no
// request is pending, places the stream in the idle state and exits.

void Stream::startnextrequest(int priority)
{
    RequestStruct   *req = NULL;
    bool nopendingrequest;

    this->futex.Lock();

    LogStreamStateInfoEvent(STARTNEXTREQUEST, this, bufferstart, state, datastart, datatail, dataend, fhandle, foffset);

    if (this->curreq == NULL)
        nopendingrequest = 1;
    else if (this->curreq->state == Stream::REQUESTSTATE_PENDING)
    {
        nopendingrequest = 0;

        if (curreq->type == FILEREAD)
            LogStreamRequestOperation(REQUESTSTART, this, curreq->id, curreq->fhandle, 0, curreq->fname);
    }
    else if (this->curreq->next == NULL)
    {
        nopendingrequest = 1;
        this->curreq->ended = true;

        if (curreq->type == FILEREAD)
            LogStreamRequestOperation(REQUESTEND, this, curreq->id, curreq->fhandle, 0, curreq->fname);
    }
    else
    {
        if (!curreq->ended)
            LogStreamRequestOperation(REQUESTEND, this, curreq->id, fhandle, 0, curreq->fname);

        if (curreq->next->type == FILEREAD)
            LogStreamRequestOperation(REQUESTSTART, this, curreq->next->id, curreq->next->fhandle, 0, curreq->next->fname);

        this->curreq = this->curreq->next;
        nopendingrequest = 0;
    }

    if (nopendingrequest)
        this->state = Stream::STATE_IDLE;
    else
    {
        // round up datatail to the next alignment boundary
        // TODO: check that this doesn't cause datatail to go past datastart,
        // which might cause problems later on.
        this->datatail = AlignUp(this->datatail, Stream::BUFFERALIGN);
        req = this->curreq;
        req->datastart = this->datatail;
        req->state = Stream::REQUESTSTATE_EXECUTING;
        req->endoffile = 0;
    }

    this->futex.Unlock();

    if (nopendingrequest) return;

    EA_ASSERT_MSG(req != NULL,
        "Internal Streamer Error - req is NULL, but nopendingrequest is false");

    // Delete any junk from the previous request
    // in the tail end of the data buffer.
    this->dataend = this->datatail;

    if (req->type == MEMREAD)
    {
        this->foffset = 0;

        //lint -e{571}
        this->filesize = static_cast<uint64_t>(req->parm);
    }
    else
    {
        this->foffset = req->parm;

        // If the file name has changed, close the old file and open
        // the new one before reading (runs on the callback thread).

        if (req->fhandle != NULL)
        {
            // NULL out the stream file name
            this->fname[0] = '\0';

            // we are queuing a request in which the caller owns the handle
            if ((this->fhandle == NULL) ||
                (this->flags & STREAM_FLAG_USER_FILE_HANDLE) == STREAM_FLAG_USER_FILE_HANDLE)
            {
                // We don't need to close the existing handle, (if any)
                this->flags |= STREAM_FLAG_USER_FILE_HANDLE;
                this->fhandle = req->fhandle;
                this->filesize = GetSize(this->fhandle);
            }
            else if (this->fhandle != NULL)
            {
                LogStreamFileCloseEvent(this, req->id, fhandle);

                this->mAsyncOp.Close(this->fhandle, closecallback, this, priority);
                return;
            }
        }
        else
        {
            // the new request may need to be opened first.
            if (strcmp(req->fname, this->fname) != 0)
            {
                strcpy(this->fname, req->fname);

                if ((this->fhandle == NULL) ||
                   ((this->flags & STREAM_FLAG_USER_FILE_HANDLE) == STREAM_FLAG_USER_FILE_HANDLE))
                {
                    // nothing to close, skip directly to open
                    this->mAsyncOp.Open(this->fname, AsyncOp::FOPEN_READONLY,
                        opencallback, this, priority);
                }
                else
                {
                    LogStreamFileCloseEvent(this, req->id, fhandle);

                    // The streamer owns this file handle, so it must be closed before
                    // this request's file can be opened.
                    this->mAsyncOp.Close(this->fhandle, closecallback, this, priority);
                }

                return;
            }
        }
    }

    // Start reading the new request.
    restartstream(priority);
}

// TODO: replace this with rw::core::stdc::AlignDown after the
// trailing edge of rwstdc is at 1.03.00 or later.
#define ROUND_DOWN(v, a) ((v)&~(a-1))

// Attempts to read some more data into the stream buffer. The streamer
// essentially runs in a big loop which starts when this function issues
// an async read. The read callback function then processes the data and
// calls this function again. This continues until the buffer fills up
// and the streamer stops, or the request completes. When a request
// completes, the streamer either starts the next request or goes idle
// if none is pending. A stopped stream is restarted when a chunk is
// released. An idle stream is restarted when a new request is queued.
//  \importlib rwfilesystem


void Stream::restartstream(int priority)
{
    RequestStruct *req;
//    ChunkStruct *chunk;
    const unsigned char *reqend;
    int largestread;
    int tailsize;

    LogStreamStateInfoEvent(RESTARTSTREAM, this, bufferstart, state, datastart, datatail, dataend, fhandle, foffset);

    // Advance the start pointer past any released chunks to free-up space
    // in the circular buffer.

    this->futex.Lock();

    eastl::intrusive_list<ChunkStruct>::iterator chunk =
        this->chunk.begin();

    while (chunk != this->chunk.end())
    {
        if ((chunk->status == GETTABLE) || (chunk->status == RECEIVED))
        {
            this->datastart = static_cast<unsigned char *>(chunk->ci.pData);
            break;
        }

        eastl::intrusive_list<ChunkStruct>::iterator ret =
            this->chunk.erase(chunk);

        if (this->chunk.empty())
        {
            // If this is the last chunk in the list, update the datastart pointer
            unsigned char *start = static_cast<unsigned char *>(chunk->ci.pData) + chunk->ci.size;

            //lint -e{923}
            this->datastart = reinterpret_cast<unsigned char *>(reinterpret_cast<uintptr_t>(start) & ~DATAALIGNMASK);
        }

        delete chunk.mpNode;

        chunk = ret;
    }

    // Free any requests that are now entirely behind the start pointer.
    for (;;)
    {
        req = this->firstreq;
        if (req->next == NULL) break;
        if (req->next->state == Stream::REQUESTSTATE_PENDING) break;
        reqend = req->next->datastart;
        if (inbetween(this->datastart, this->dataend, reqend-1)) break;
        freerequest(req);
    }

    // Determine the largest possible read into the circular buffer.
    if (this->datastart > this->dataend)
    {
        // Data wraps: we can read to datastart-1.
        largestread = static_cast<int>(
            (this->datastart - this->dataend) - (Stream::BUFFERALIGN + 1));
    }
    else
    {
        // Data does not wrap: we can read to the end of the buffer
        largestread = static_cast<int>(
            (this->bufferend - this->dataend) - Stream::BUFFERALIGN);

        // If we can't read a full block, attempt to move the tail end
        // of the buffer to the start of the buffer.
        if (largestread < this->readblocksize)
        {
            // If there isn't enough room, stop the stream.
            tailsize = static_cast<int>(this->dataend - this->datatail);

            if (((this->datastart - this->bufferstart) - (Stream::BUFFERALIGN + 1)) >= tailsize)
            {
                // Move the tail back to the start of the buffer.

                // set bufferstart so that the next read will be aligned to a
                // Stream::BUFFERALIGN offset
                if (((tailsize % Stream::BUFFERALIGN) == 0) || (this->curreq->type == MEMREAD))
                {
                    this->bufferstart = this->actualbufferstart;
                }
                else
                {
                    //lint -e{961} operator precedence
                    this->bufferstart = \
                        this->actualbufferstart + Stream::BUFFERALIGN - (tailsize % Stream::BUFFERALIGN);
                }

                memcpy(this->bufferstart, this->datatail,
                    static_cast<unsigned int>(tailsize));

                this->datatail = this->bufferstart;
                this->dataend = this->datatail + tailsize;

                // Now recompute the largest read.
                largestread = static_cast<int>(
                    (this->datastart - this->dataend) - (Stream::BUFFERALIGN + 1));
            }
        }
    }

    // 'forcestarvation' mode explicitly stops the streamer
    // when the 'starvationthreshold' is exceeded.  This allows
    // users to control the amount of data loaded.
    // 
    if (this->forcestarvation)
    {        
        if (this->bufferusage > starvationthreshold)
        {
            this->state = Stream::STATE_STOPPED;
            this->futex.Unlock();
            return;
        }
    }

    // If we can't read a full block, stop the stream.
    if (largestread < this->readblocksize)
    {
        // if there is no more data to be released, and we still don't have
        // room to read more data, we're deadlocked.  Alert the user with a
        // nice abort message.  The casual reader might think it odd that
        // this situation could occur: how can the stream buffer be empty and
        // there is still not enough room to read another block?  Well, it's
        // because the next read must to dataend, and it could be that there
        // is some room at the start of the buffer, but if dataend is pointing
        // near the end of the stream buffer, there wouldn't be enough room
        // to read another block.  How about moving the tail back around to
        // the start of the stream buffer?  We already tried that above and
        // found that the tail size (all the unparsed data) wouldn't fit in
        // the space available between the start of the stream buffer and
        // datatail (the start of the unparsed data).  So we're stuck.  Can't
        // read to dataend and can't move datatail (and dataend) back to the
        // start of the buffer.  The only thing we can do is tell the user
        // to increase stream buffer size.
        //
        // To prevent this situation, the stream buffer size needs to be a
        // minimum of:
        // 2 * (largest chunk in the stream) +
        // 1 * (read block size) +
        // 256 bytes (for alignment purposes)

        // As assert in parsechunks should catch this situation before the
        // too-large chunk is added to the list, but just in case it doesn't,
        // we'll leave this assert in here.

		if (!this->bufferusage)
		{
			char buf[1024];
			char *cur = buf;
			const char *end = buf + sizeof(buf);
			EA::StdC::Snprintf(cur, end - cur, "(curreq = %s", this->curreq->fname);
			cur += EA::StdC::Strlen(cur);
			req = this->firstreq;
			while (req != NULL)
			{
				EA::StdC::Snprintf(cur, end - cur, "; %s", req->fname);
				cur += EA::StdC::Strlen(cur);
				req = req->next;
			}
			EA::StdC::Snprintf(cur, end - cur, ")");

			EA_ASSERT_FORMATTED(false,
				("The streamer is deadlocked.  There is no more data waiting to be released and there is still not enough room to read in another block of data to the end of the previously-read data.  Make the stream buffer larger. %s", buf));
		}

        this->state = Stream::STATE_STOPPED;
        this->futex.Unlock();
        return;
    }

    this->futex.Unlock();

    req = this->curreq;

    if (req->type == MEMREAD)
    {
        // For memory reads, read as much as possible (without going past the
        // end of the source) and explicitly call the read callback function.
        // The calls will recurse until the buffer fills up or the request
        // completes.

        if ((this->foffset + largestread) > req->parm)
            this->readsize = req->parm - this->foffset;
        else this->readsize = largestread;

        memcpy(this->dataend, req->address,
            static_cast<unsigned int>(this->readsize));
        req->address += this->readsize;

        readcallback();
    }
    else
    {
        // For file reads, read one block at a time.

        this->readsize = this->readblocksize;

        LogStreamReadInfoEvent(this, fhandle, dataend, foffset);

        EA_ASSERT_MSG((reinterpret_cast<intptr_t>(this->dataend) % Stream::BUFFERALIGN) == 0,
            "Misaligned read");

        this->mAsyncOp.Read(this->fhandle, this->dataend,
            ROUND_DOWN(static_cast<uint64_t>(this->foffset), RW_CORE_FILESYS_READ_BUFFER_ALIGNMENT),
            static_cast<uint64_t>(this->readsize), filereadcallback,
            this, priority);        
        if (this->mAsyncOp.GetStatus() == rw::core::filesys::AsyncOp::OPSTATUS_FAILED)    
        {
            this->futex.Lock();
            req->error = ERRORSTATE_READ_FAILED;
            this->state = Stream::STATE_STOPPED;
            this->futex.Unlock();
        }        
    }
}

// Stream class constructor
//lint -e{26, 64, 910, 1927} symbol 'x' did not appear in the constuctor initializer list
Stream::Stream(int numrequests, void *buffer, int bufsize,
    int read_size, Stream::PARSEFUNC *parse, void *_parsecontext, uint32_t _requestflags) :
    id(STREAM_ID), flags(0), futex(), request(), requests(numrequests), filter(), tap(), chunk(), mAsyncOp(),
    requestflags(_requestflags), startParseThreshold(0), stopParseThreshold(0), mLowBufferWarningCallback(NULL),
    mLowBufferWarningCallbackContext(NULL), mLowBufferWarningThreshold(0)
{
    request = static_cast<RequestStruct *>(
        RWFILESYSTEM_ALLOCATE(static_cast<unsigned int>(requests) * sizeof(RequestStruct)));

    actualbufferstart = static_cast<unsigned char *>(buffer);
    bufferstart = actualbufferstart;
    bufferend = bufferstart + bufsize;

    state = Stream::STATE_IDLE;
    prioritylow = Stream::STREAM_PRIORITYLOW;
    priorityhigh = Stream::STREAM_PRIORITYHIGH;
    greedylevel = bufsize / 3;
    greedystate = 0;
    forcestarvation = false;
    isfilehandleclosed = false;
    bufferusage = 0;
    datastart = bufferstart;
    datatail = bufferstart;
    dataend = bufferstart;
    firstreq = NULL;
    curreq = NULL;
    lastreq = NULL;
    freereq = &request[0];

    parsefunc = (parse != NULL) ? parse : stream_default_parser;
    parsecontext = _parsecontext;

    memset(fname, 0, FILESYS_MAX_PATH);
    fhandle = NULL;

    readblocksize = read_size;

    if (readblocksize == 0)
    {
        // calculate a reasonable read block size by dividing the
        // buffer into 6 approximately equal parts.
        // set the readblocksize based on how large the buffer is.
        // There must be room for a minimum of 3 read blocks, and
        // we want the readblocksize to be an even multiple of 2K,
        // so divide the buffer size by 6 and round up to the next
        // 2K boundary.
        //
        // Generally, larger reads are better, resulting in fewer
        // seeks and better overall transfer rate.  This
        // consideration leads us to choose the smallest number of
        // blocks possible (3).  However, a larger block size also
        // causes more wasted space in the stream buffer because there
        // must be at least readblocksize bytes of contiguous free
        // space in the stream buffer before a new read can be started.
        // This consideration suggests we use a small read block size
        // and therefore the largest possible value for the number of
        // read blocks.  We have chosen to use 6 blocks as a reasonable
        // trade-off between these two competing objectives.
        readblocksize = ((bufsize / 6) + 2047) & ~2047;
    }

    // Give the starvation threshold a default value.
    starvationthreshold = readblocksize * 2;

    // Make sure there is room for at least three blocks
    EA_ASSERT_MSG((bufsize / readblocksize) >= 3, "Buffer is too small");
    EA_ASSERT_MSG(bufsize >= starvationthreshold, "Buffer is too small");

    for (int i = 0; i < requests; ++i)
    {
        RequestStruct *req = &request[i];
        req->id = i;
        req->state = Stream::REQUESTSTATE_FREE;
        req->next = &request[i+1];
        req->gettable = 0;
    }
    request[requests-1].next = NULL;

    // make dummy tap & filter structures.  If the
    // user sets a new filter, it'll replace this dummy
    // one.  Any filters set after the first one will
    // be appended to the filter list.
    tap.push_back(*(new TapStruct(this, 0)));
    filter.push_back(*(new FilterStruct(NULL, NULL, 1)));
}

Stream::~Stream()
{
    // delete all chunkstructs owned by the stream
    eastl::intrusive_list<ChunkStruct>::iterator cs = chunk.begin();

    while (cs != chunk.end())
    {
        EA_ASSERT_MSG(cs->status == RELEASED, "Internal error");
        ChunkStruct *node = cs.mpNode;
        cs = chunk.erase(cs);
        delete node;
    }

    eastl::intrusive_list<FilterStruct>::iterator fs = filter.begin();

    while (fs != filter.end())
    {
        FilterStruct *node = fs.mpNode;
        fs = filter.erase(fs);
        delete node;
    }

    eastl::intrusive_list<TapStruct>::iterator ts = tap.begin();

    while (ts != tap.end())
    {
        TapStruct *node = ts.mpNode;
        ts = tap.erase(ts);
        delete node;
    }

    // Free the request array.
    Manager::Free(request);

    request = NULL;
    actualbufferstart = NULL;
    bufferstart = NULL;
    bufferend = NULL;
    datastart = NULL;
    datatail = NULL;
    dataend = NULL;
    firstreq = NULL;
    curreq = NULL;
    lastreq = NULL;
    freereq = NULL;
    parsecontext = NULL;
    fhandle = NULL;
}

/*!
    Creates a new stream using the specified buffer.

    When choosing a stream buffer size, there are several factors to consider.
    Larger stream buffers will need to fill less frequently (and therefore
    cause fewer seeks) than small stream buffers for the same consumption rate.  The
    smallest possible stream buffer plus 6 KB.  However, a stream buffer of
    this size will waste about 1/3 of its buffer and impose extra overhead
    on the file system because reads will be done in 2K blocks.  If your
    stream chunks are much smaller than the read block size, which is
    usually the case for audio streams, you should add the size of an average
    chunk to the stream buffer.  This will gain a lot of efficiency for the
    amount of memory required.  Generally, the most efficient size for a
    stream buffer is an integral multiple of the read block size + the average
    chunk size in the file being streamed.  However, if you have chunks which
    are significantly larger than the read block size, you will have to make
    your stream buffer larger.  This situation occurs commonly in movies, where
    some frames are many times larger than the read block size.  To avoid a
    potential deadlock condition, the minimum stream buffer size is 2 x the
    largest chunk size plus 1 x the read block size + 256 bytes for alignment
    purposes.

    Parameter requests specifies the maximum number of streamer requests that
    can be queued at one time. A request is not freed until all of its chunks
    as well as chunks from previous requests have been released from the stream
    buffer. Note that the request is not freed instantly and that the last
    request is not freed until a new request is queued. For this reason, a
    minimum of two requests is enforced when the stream is created. Finally,
    a pending request is always freed immediately when cancelled.

    When a block of data is read into the circular buffer, it is parsed by a
    user-defined parsing function.  The parsing function examines the data
    and returns a chunk size to the streamer if a complete chunk is available.
    The parsing function may also detect that the End Of Stream has been
    reached.

    When the stream is no longer required, it must be destroyed using
    Stream::Destroy before its storage can be released.

    \param requests      max. number of requests
    \param buffer        circular buffer location
    \param bufsize       size of circular buffer in bytes.
    \param readblocksize size of a read block (if 0, will be calculated based on the buffer size)
    \param parsefunc     parsing function (if NULL a default parser is used, which will create chunks which are 2K in size)
    \param parsecontext  parse function context parameter
    \param requestflags  one of the RequestFlags values (but not REQUESTFLAG_DEFAULT).  This value is used on individual requests another value is passed to QueueFile.

    \importlib rwfilesystem

    \return Returns the handle of the first output tap.
    \sa Stream::CreateTap, Stream::SetFilter(FILTERFUNC *, void *), Stream::Destroy
*/
Stream *Stream::Create(
    int requests,
    void *buffer,
    int bufsize,
    int readblocksize,
    PARSEFUNC *parsefunc,
    void *parsecontext,
    uint32_t requestflags
    )
{
    //lint -e{923}  cast from pointer to int
    EA_ASSERT_MESSAGE((reinterpret_cast<intptr_t>(buffer) & (Stream::BUFFERALIGN-1)) == 0,
        "Stream buffer must be aligned to Stream::BUFFERALIGN bytes");
    EA_ASSERT_MESSAGE(bufsize >= (6*1024),
        "Stream buffer must be at least 6K in size");
    EA_ASSERT_MESSAGE((2 <= requests) && (requests < MAX_REQUESTS),
        "Number of requests must be between 2 and 255");

    // Must use REQUESTFLAG_RETURN_UNPARSED_DATA or REQUESTFLAG_IGNORE_UNPARSED_DATA
    // at this point.  REQUESTFLAG_DEFAULT is only valid when a request is enqueued.
    EA_ASSERT(requestflags != REQUESTFLAG_DEFAULT);

#if defined(EA_DEBUG)
    memset(buffer, 0x55, static_cast<unsigned int>(bufsize));
#endif

    Stream *strm = new Stream(requests, buffer, bufsize,
        readblocksize, parsefunc, parsecontext, requestflags);

    // align bufferstart to Stream::BUFFERALIGN bytes

    LogStreamCreate(strm, requests, bufsize);
    LogStreamBufferOperation(CREATE, strm, 0, bufsize, 0, strm->state, strm->greedystate);

    // What the heck is going on here?  The design of the streamer calls
    // for multiple entry points into the object (refer to CreateTap).  Reads
    // may be initiated from any tap so every tap that belongs to a stream
    // must have a pointer to it.  Therefore what we are returning is an
    // object that contains a pointer to the actual stream object.  If we
    // wanted to make the streamer truly object-oriented in the future,
    // we need to have GetChunk and many other stream functions take an
    // index into what tap they are to use in addition to the stream object.
    return reinterpret_cast<Stream *>(&strm->tap.front());
}

/*! \deprecated This function has been Use Stream::SetFilter(FILTERFUNC *, void *)
    instead
*/
void Stream::SetFilter(
    int tapnum,
    FILTERFUNC *func,
    void *context
    )
{
    Stream *strm = GetStreamHeaderFromHandle(this);

    EA_ASSERT_MSG((1 <= tapnum) && (tapnum <= static_cast<int>(strm->tap.size())),
        "invalid tap number");
    EA_ASSERT_MSG(strm->state == Stream::STATE_IDLE,
        "stream must be idle to set a filter");

    if ((&strm->filter.front())->func == NULL)
    {
        // if the first filter function is NULL, replace it with
        // this new one.
        FilterStruct &filt = strm->filter.front();
        filt.func = func;
        filt.context = context;
        filt.tapnum = tapnum;
    }
    else
    {
        strm->filter.push_back(*new FilterStruct(func, context, tapnum));
    }
}

/*!
    Sets which chunks a filter will capture and which output tap will receive
    the captured chunks.  The stream parsing function is called for each
    filter which has been set.  The first filter which recognizes the data
    should return that a complete chunk has been found.  That chunk is
    then directed to the specified output tap. Note that both filter numbers
    and tap numbers start at one. The last filter in the stream must always be
    set to null so that it captures all remaining chunks.  Filters can only
    be set while the stream is idle.
    \param func    function to call for this filter type
    \param context context parameter passed through to func
    \importlib rwfilesystem
    \sa Stream::Create, Stream::CreateTap
*/
void Stream::SetFilter(
    FILTERFUNC *func,
    void *context
    )
{
    Stream *strm = GetStreamHeaderFromHandle(this);

    EA_ASSERT_MSG(strm->state == Stream::STATE_IDLE,
        "stream must be idle to set a filter");

    int tapnum = reinterpret_cast<TapStruct *>(this)->tapnum;

    if (strm->filter.front().func == NULL)
    {
        // if the first filter function is NULL, replace it with
        // this new one.
        strm->filter.front().func = func;
        strm->filter.front().context = context;
        strm->filter.front().tapnum = tapnum;
    }
    else
    {
        strm->filter.push_back(*new FilterStruct(func, context, tapnum));
    }
}

/*!
    Destroys a stream so that its storage can be reclaimed. It is safe
    to destroy a stream at any time since the routine will first kill the
    stream and then wait for it to go idle before returning.
    \importlib rwfilesystem
    \sa Stream::Create, Stream::Kill
*/
void Stream::Destroy(bool allowBlocking)
{
    Stream *strm = GetStreamHeaderFromHandle(this);

    LogStreamDestroy(strm);

    Kill();

    // Block until the stream is stopped or idle
    while (strm->state == STATE_RUNNING)
    {
        Manager::Idle();
        EA::Thread::ThreadSleep();
    }

	if ( allowBlocking )
	{
		strm->id = 0;
	}

    if ((strm->fhandle != 0) && ((strm->flags & STREAM_FLAG_USER_FILE_HANDLE) == 0))
    {
        // Do a blocking close here.  We cannot return from Destroy
        // until all resources allocated by the stream have been
        // released.
        strm->mAsyncOp.Close(strm->fhandle);
        while (allowBlocking && strm->mAsyncOp.GetStatus(EA::Thread::kTimeoutImmediate) ==
            AsyncOp::OPSTATUS_PENDING)
        {
            Manager::Idle();
            EA::Thread::ThreadSleep(1);
        }
    }

    LogStreamBufferOperation(DESTROY, strm, 0, 0, 0, static_cast<int>(strm->state), strm->greedystate);

	if ( allowBlocking )
	{
		delete strm;
	}
}

void Stream::Delete()
{
	Stream *strm = GetStreamHeaderFromHandle(this);
	strm->id = 0;
	delete strm;
}

/*!
    Close the stream handle.  The streamer keeps its last file handle open
    until a request is queued on another file or the stream object is
    destroyed.  Use this function to force the streamer to close its file
    handle before one of those events occurs.

    This function can only be called when the streamer is idle; calling it
    while the stream is still reading will cause undefined behavior.  Use
    Stream::GetState to determine when the streamer is idle.

    \importlib rwfilesystem

    \sa Stream::Destroy, Stream::GetState
*/
void Stream::CloseFileHandle()
{
    Stream *strm = GetStreamHeaderFromHandle(this);

    EA_ASSERT_MSG(strm->state == Stream::STATE_IDLE,
        "stream must be idle before the handle is closed");

    if ((strm->fhandle != 0) && ((strm->flags & STREAM_FLAG_USER_FILE_HANDLE) == 0))
    {
        strm->mAsyncOp.Close(strm->fhandle);
        strm->mAsyncOp.Wait();
        LogStreamFileCloseEvent(strm, 0, strm->fhandle);

        strm->fhandle = NULL;

        // Remove the existing file name so that if the next request has
        // the same name as this request, the file will be re-opened in
        // startnextrequest.
        strm->fname[0] = '\0';
    }
}

/*!
    Async file handle close callback.  This sets the streamer internal state required to close the file handle.
*/
void Stream::closecallback_filehandleasync(AsyncOp *op)
{
    Stream* strm = static_cast<Stream *>(op->GetContext());    

    LogStreamFileCloseEvent(strm, 0, strm->fhandle);

    strm->futex.Lock();
    strm->fhandle = NULL;
    strm->fname[0] = '\0';
    EAWriteBarrier();  // ensure the write to the flag isn't reordered with any of the stream object status updates.
    strm->isfilehandleclosed = true;    
    strm->futex.Unlock();
}

/*!
    User level non-blocking function to close the streamer file handle.
*/
void Stream::CloseFileHandleAsync()
{
    Stream *strm = GetStreamHeaderFromHandle(this);

    EA_ASSERT_MSG(strm->state == Stream::STATE_IDLE,
        "stream must be idle before the handle is closed");

    if ((strm->fhandle != 0) && ((strm->flags & STREAM_FLAG_USER_FILE_HANDLE) == 0))
    {
        strm->mAsyncOp.Close(strm->fhandle, closecallback_filehandleasync, strm->mAsyncOp.GetContext());
    }
}

/*!
    Non-blocking user function to poll if the file handle has been closed on one of rwfilesystems worker threads (device thread).
*/
bool Stream::IsFileHandleClosed() const
{
    Stream *strm = GetStreamHeaderFromHandle(this);
    return strm->isfilehandleclosed;

}

/*!
    Query the stream to see if it's using the specified driver. As some
    platforms place limits on the number of drivers in play, the Xbox
    360 for example limits the system to 5 DLC drivers, it is handy to
    be able to query streams to see if they are using a particular driver.
    This makes it easier for you to find out if a certain stream needs to
    be shut down before mounting a new DLC device.
*/
bool Stream::IsUsingDeviceDriver(const DeviceDriver *driver)
{
    const Stream *strm = GetStreamHeaderFromHandle(this);
    return ((strm->fhandle != 0) && (strm->fhandle->mDriver == driver));
}

/*!
    Sets the priority levels used to issue file operations for the specified
    stream. When the stream is first started, it queues itself up using the
    low priority level. Once it starts running, it requeues itself using the
    high priority level until it is topped up. When enough data has been
    released for the stream to start up again, it will requeue itself at
    the low priority level and repeat this cycle. This allows the stream to
    maintain control of the file device while it is reading but gives other
    clients of the file system (including other streams) a chance to execute.

    If the priority levels are not set, the default values represented by
    STREAM_PRIORITYLOW and STREAM_PRIORITHIGH will be used.

    \importlib rwfilesystem

    \param prioritylow low priority level
    \param priorityhigh high priority level
    \sa Stream::SetGreedyLevel, Stream::SetGreedyState
*/
void Stream::SetPriority(int prioritylow,int priorityhigh)
{
    Stream *strm = GetStreamHeaderFromHandle(this);

    EA_ASSERT_MSG(prioritylow <= priorityhigh,
        "prioritylow is a higher priority than priorityhigh.");

    strm->prioritylow = prioritylow;
    strm->priorityhigh = priorityhigh;
}

/*!
    Sets the buffer usage level at which the stream switches to and from
    greedy mode. When a stream is in greedy mode, it will requeue itself
    using the high priority level instead of the low priority level (refer
    to Stream::SetPriority for information on priority levels). The stream
    is switched to greedy mode when the buffer usage level crosses the
    threshold going down and it switches out of greedy mode when the buffer
    usage level crosses the threshold going up. If a stream request is
    pending at low priority when the stream switches to greedy mode, the
    request will be boosted to the high priority level. This mechanism
    allows a stream to restart itself when other file I/O is occurring on
    the same device and the stream is about to starve.

    When a stream is first created, the greedy level is set to zero and
    the stream is not in greedy mode. Since a threshold level of zero
    will never be crossed, the stream will not enter greedy mode on its
    own. This can be useful when explicit control of the greedy state is
    desired using function Stream::SetGreedyState. Note that when the
    greedy level is set, it may cause the greedy state to change, depending
    on whether or not the new level crosses the current buffer usage.

    When multiple streams sharing the same device have greedy levels set,
    it is important that all the streams have different high priority
    levels to prevent thrashing when more than one stream enters greedy
    mode at the same time.
    \param greedylevel threshold level for mode switch (bytes)
    \importlib rwfilesystem
    \sa Stream::SetPriority, Stream::SetGreedyState, Stream::BufferUsage
*/
void Stream::SetGreedyLevel(int greedylevel)
{
    Stream *strm = GetStreamHeaderFromHandle(this);

    int oldgreedylevel = strm->greedylevel;
    strm->greedylevel = greedylevel;
    int bufferusage = strm->bufferusage;
    bool oldside = (bufferusage < oldgreedylevel);
    bool newside = (bufferusage < greedylevel);

    //lint -e{731} boolean argument to equal/not equal
    if (oldside != newside)
    {
        if (newside)
        {
            SetGreedyState();
        }
        else
        {
            SetNonGreedyState();
        }
    }
}

/*!
    Allows the state of a stream's greedy mode to be set explicitly. This is
    normally done only when automatic control using Stream::SetGreedyLevel
    is not sufficient.
    \importlib rwfilesystem
    \sa Stream::SetGreedyLevel, Stream::SetNonGreedyState
*/
void Stream::SetGreedyState()
{
    Stream *strm = GetStreamHeaderFromHandle(this);

    if (!strm->greedystate)
    {
        strm->greedystate = 1;

        LogStreamGreedyStateChangeEvent(strm, strm->greedylevel, strm->greedystate);
    }

    // Boost any pending request to the high priority level
    // when greedy mode is enabled.
    if (strm->state == STATE_RUNNING)
        strm->mAsyncOp.SetPriority(strm->priorityhigh);
}

/*!
    Allows the state of a stream's greedy mode to be set explicitly. This is
    normally done only when automatic control using Stream::SetGreedyLevel
    is not sufficient.
    \importlib rwfilesystem
    \sa Stream::SetGreedyState, Stream::SetGreedyLevel
*/
void Stream::SetNonGreedyState()
{
    Stream *strm = GetStreamHeaderFromHandle(this);

    if (strm->greedystate)
    {
        strm->greedystate = 0;

        LogStreamGreedyStateChangeEvent(strm, strm->greedylevel, strm->greedystate);
    }
}

/*!
    Allows explicit external control to force a stream to starve. This is 
    done when modifying fileop priorities/greedystates is insufficient. This mode works
    in conjunction with modifications made to restartStream that all read
    requests of n - readblocksizes.
    \importlib rwfilesystem
    \sa Stream::SetForceStarvation
*/
void Stream::SetForceStarvation(bool enable)
{
    Stream *strm = GetStreamHeaderFromHandle(this);

    strm->forcestarvation = enable;
}

/*!
    Allows the user to set the amount of data in bytes the streamer will
    copy into its internal buffer that is will hand off to the user.
*/
void Stream::SetStarvationThreshold(unsigned int threshold)
{
    Stream *strm = GetStreamHeaderFromHandle(this);

    strm->starvationthreshold = threshold;
    EA_ASSERT_MSG((strm->bufferend - strm->bufferstart) >= strm->starvationthreshold, "Starvation threshold is too large.");
}

/*!
    This function allows better control of when the stream parser function
    is called.  Parsing begins when there are at least startThreshold bytes
    available for parsing.  It stops when there are fewer than stopThreshold
    bytes.  When the amount of buffered data is between the two values, it
    keeps the last state, sort of like a Schmitt Trigger in electronics.

    This is useful when data is being streamed over a network - it allows
    the parsing to be more spread out instead of bunched up when a new
    packet of data arrives.

    \param startThreshold The amount of parsable data that must be available
    before parsing begins.  Parsing ceases when there is not enough data
    available make another chunk, or an end chunk is found, or when the
    amount of parseable data falls below the stopThreshold amount.
    \param stopThreshold When the amount of gettable data falls below this
    amount, parsing ceases, even if more chunks could be parsed.  Parsing
    resumes only when the amount of parseable data rises above startThreshold
    bytes.
    \importlib rwfilesystem
*/
void Stream::SetParserThresholdValues(int startThreshold, int stopThreshold)
{
    Stream *strm = GetStreamHeaderFromHandle(this);

    strm->startParseThreshold = startThreshold;
    strm->stopParseThreshold = stopThreshold;

    // if there is less data available than the stop
    // threshold, disable parsing
    if (strm->dataend - strm->datatail < stopThreshold)
        strm->flags &= ~STREAM_FLAG_PARSE_TRIGGER_PARSING;
}

/*!
    Set a callback function which is executed if the stream buffer gets too low

    \param cbFunc This is a function pointer which takes a stream handle and a
    void context.  It is called whenever the stream buffer gets too low.
    \param context This context pointer is passed to your low warning callback
    function.
    \param theshold A number between 0 and 49 which indicates the percentage
    full you want the buffer to be when the callback function is triggered.  A
    value of 0 means the buffer must be completely dry before the callback is
    triggered.  A value of 10 is recommended.
*/
void Stream::SetBufferLowCallback(void (*cbFunc)(Stream *, void *), void *context, int threshold)
{
    Stream *strm = GetStreamHeaderFromHandle(this);

    EA_ASSERT(0 <= threshold && threshold < 50);

    strm->mLowBufferWarningCallback = cbFunc;
    strm->mLowBufferWarningCallbackContext = context;
    strm->mLowBufferWarningThreshold = threshold;
}

/*!
    Creates a new stream object that uses the parent stream's buffer.
    This function is meant to retrieve the second and subsequent tap handles
    using the first tap handle returned when a multi-tap stream is created.
    Note that output taps are numbered starting at one.  Do not destroy secondary
    taps directly.  They are destroyed when the primary stream object is
    destroyed.
    \importlib rwfilesystem
    \return Returns the handle of the specified output tap.
    \sa Stream::Create, Stream::SetFilter(FILTERFUNC *, void *), Stream::Destroy
*/
Stream *Stream::CreateTap()
{
    Stream *strm = GetStreamHeaderFromHandle(this);

    TapStruct *tap = new TapStruct(strm, static_cast<int>(strm->tap.size()));

    strm->tap.push_back(*tap);

    return reinterpret_cast<Stream *>(tap);
}

/*!
    \deprecated This function is deprecated.  Use
    Stream::SetFilter(FILTERFUNC *, void *) instead.
*/
int Stream::TapNum() const
{
    return reinterpret_cast<const TapStruct *>(this)->tapnum;
}

/*!
    Queues a request to read chunks from a the same file as the last request
    in the queue, or, if there is no executing request, the same as the last
    one that executed.  This can be useful if you need to play a looped
    sample from the same file but don't want to have to open the file manually
    or keep the name string in memory.

    \param offset       offset to start reading from in file
    \param parse        parse function for this request (if 0, the stream parse function is used)
    \param parsecontext context parameter for parse function
    \param flags        One of the RequestFlags flags.  If the default value is used, the flags passed when the stream was created are used.
    \importlib rwfilesystem
    \return Returns a request ID that uniquely identifies the queued request
    or zero if no request is available.
    \sa Stream::QueueFile, Stream::CancelRequest, Stream::GetRequestState
*/

Stream::RequestId Stream::Requeue(
    int64_t offset,
    PARSEFUNC *parse,
    void *parsecontext,
    uint32_t flags
    )
{
    Stream *strm = GetStreamHeaderFromHandle(this);
    char fileName[FILESYS_MAX_PATH] = { '\0', };
    Handle *hFile = NULL;

    // Get the file name or from the last request in the list
    strm->futex.Lock();
    if (strm->state == STATE_IDLE)
    {
        // the streamer is idle.  Use whatever information was in the last
        // request
        if (*strm->fname != '\0')
        {
            // streamer owns the handle, use the file name
            strcpy(fileName, strm->fname);
        }
        else
        {
            // streamer does not own the handle, use the handle
            hFile = strm->fhandle;
        }
    }
    else
    {
        // streamer has a pending request; use the information from the last
        // queued request instead of the current one
        RequestStruct *req = strm->lastreq;
        if (*req->fname != '\0')
        {
            strcpy(fileName, req->fname);
        }
        else
        {
            hFile = req->fhandle;
        }
    }
    strm->futex.Unlock();

    return QueueFile(fileName, hFile, offset, parse, parsecontext, flags);
}

/*!
    Queues a request to read chunks from a specified file. The request will
    start executing as soon as the previously queued request terminates. A file
    read request terminates when the end of file is reached or a chunk matching
    the end chunk ID is read. Note that the end chunk is always placed in the
    stream buffer. The state of a request can be obtained using function
    Stream::GetRequestState.

    For efficiency, if the file name exactly matches the name in the previous
    file read request, the file is not closed and re-opened.

    If the file is not found, the streamer frees the request and moves onto
    the next one in the queue.  No error is returned and no exception is thrown.
    Clients can detect that a request was skipped by recording the returned
    value and comparing it with the reqId member in ChunkInfo structs.

    \param fname        name of file to read from
    \param offset       offset to start reading from in file
    \param parse        parse function for this request (if 0, the stream parse function is used)
    \param parsecontext context parameter for parse function
    \param flags        One of the RequestFlags flags.  If the default value is used, the flags passed when the stream was created are used.
    \importlib rwfilesystem
    \return Returns a request ID that uniquely identifies the queued request
    or zero if no request is available.
    \sa Stream::Requeue, Stream::QueueMem, Stream::CancelRequest, Stream::GetRequestState
*/
Stream::RequestId Stream::QueueFile(
    const char *fname,
    int64_t offset,
    PARSEFUNC *parse,
    void *parsecontext,
    uint32_t flags
    )
{
    return QueueFile(fname, NULL, offset, parse, parsecontext, flags);
}

/*!
    Queues a request to read chunks from a specified file handle. The request will
    start executing as soon as the previously queued request terminates. A file
    read request terminates when the end of file is reached or a chunk matching
    the end chunk ID is read. Note that the end chunk is always placed in the
    stream buffer. The state of a request can be obtained using function
    Stream::GetRequestState.

    In this version of QueueFile, the caller is responsible for opening and
    closing the file handle; the streamer will not do it for you.  The file handle
    must be kept open until the request has completed.

    \param handle       handle to read from
    \param offset       offset to start reading from in file
    \param parse        parse function for this request (if 0, the stream parse function is used)
    \param parsecontext context parameter for parse function
    \param flags        One of the RequestFlags flags.  If the default value is used, the flags passed when the stream was created are used.
    \importlib rwfilesystem
    \return Returns a request ID that uniquely identifies the queued request
    or zero if no request is available.
    \sa Stream::Requeue, Stream::QueueMem, Stream::CancelRequest, Stream::GetRequestState
*/
Stream::RequestId Stream::QueueFile(
    const Handle *handle,
    int64_t offset,
    PARSEFUNC *parse,
    void *parsecontext,
    uint32_t flags
    )
{
    // cast away the constness of the handle so the member can be shared with a
    // non-const file handle that is owned by the stream.  We promise not to
    // modify the handle if it was passed in through this API.
    return QueueFile("", const_cast<Handle *>(handle), offset, parse, parsecontext, flags);
}

// private helper function.  The other two functions simply call
// into this one.
Stream::RequestId Stream::QueueFile(
    const char *fname,
    Handle *fhandle,
    int64_t offset,
    PARSEFUNC *parse,
    void *parsecontext,
    uint32_t flags
    )
{
    Stream *strm = GetStreamHeaderFromHandle(this);

    // Locate a free request.
    RequestStruct *req = strm->getfreerequest();
    if (req == NULL) return 0;

    LogStreamQueueFile(strm, req->id, fname);

    // Fill it in and queue it.
    req->type = FILEREAD;
    strncpy(req->fname, fname, FILESYS_MAX_PATH-1);
    req->fname[FILESYS_MAX_PATH-1] = 0;
    req->fhandle = fhandle;
    req->parm = offset;
    req->parsefunc = (parse != NULL) ? parse : strm->parsefunc;
    req->parsecontext = (parsecontext != NULL) ? parsecontext : strm->parsecontext;
    req->gettable = 0;
    req->flags = flags == REQUESTFLAG_DEFAULT ? strm->requestflags : flags;
    req->ended = false;
    req->error = ERRORSTATE_NONE;
    strm->queuerequest(req);

    // If the stream is idle, restart it at the appropriate priority level.
    strm->futex.Lock();
    State streamstate = strm->state;
    if (streamstate == STATE_IDLE)
        strm->state = STATE_RUNNING;
    strm->futex.Unlock();

    if (streamstate == STATE_IDLE)
    {
        if (strm->greedystate) strm->startnextrequest(strm->priorityhigh);
        else strm->startnextrequest(strm->prioritylow);
    }

    return req->id;
}

/*!
    Queues a request to read chunks from memory. The request will start
    executing as soon as the previously queued request terminates. A memory
    read request terminates when the maximum length is reached or a chunk
    matching the end chunk ID is read. Note that the end chunk is always
    placed in the stream buffer. The state of a request can be obtained using
    function Stream::GetRequestState.

    It is important to properly set the length parameter to prevent the
    streamer from reading past the end of the source buffer, possibly
    causing an access violation.

    \param address      address to start reading from
    \param length       length of buffer
    \param parse        parse function for this request (if 0, the stream parse function is used)
    \param parsecontext context parameter for parse function

    \importlib rwfilesystem

    \return Returns a request ID that uniquely identifies the queued request
    or zero if no request is available.
    \sa Stream::QueueFile, Stream::CancelRequest, Stream::GetRequestState
*/
Stream::RequestId Stream::QueueMem(
    void *address,
    unsigned int length,
    PARSEFUNC *parse,
    void *parsecontext
    )
{
    Stream *strm = GetStreamHeaderFromHandle(this);

    // Locate a free request.
    RequestStruct *req = strm->getfreerequest();
    if (req == NULL) return 0;

    EA_ASSERT_MSG(length != 0,
        "Must pass a valid stream length");

    LogStreamQueueMem(strm, req->id, address);

    // Fill in the request and queue it.
    req->type = MEMREAD;
    req->address = static_cast<unsigned char *>(address);
    req->parm = static_cast<int>(length);
    req->parsefunc = (parse != NULL) ? parse : strm->parsefunc;
    req->parsecontext = (parsecontext != NULL) ? parsecontext : strm->parsecontext;
    req->error = ERRORSTATE_NONE;
    strm->queuerequest(req);

    // If the stream is idle, restart it.
    // The priority level is irrelevant for memory streams.
    strm->futex.Lock();
    State streamstate = strm->state;
    if (streamstate == STATE_IDLE)
        strm->state = STATE_RUNNING;
    strm->futex.Unlock();
    if (streamstate == STATE_IDLE) strm->startnextrequest(0);

    return req->id;
}

/*!
    Cancels a previously queued request. If the request is still pending,
    it is simply removed from the queue and freed. If the request is completed
    or executing, it is marked as canceled and any chunks from that request
    that have not been received are released. Received chunks must be released
    as usual. If the executing request is cancelled, it stops reading
    immediately and the next pending request is started (for this reason,
    it makes sense to cancel several requests in the reverse order they
    were queued).

    A cancelled request is not freed until all of its chunks as well as chunks
    from previous requests have been released. Cancelling a canceled request
    does not have any effect. Cancelling a freed request (or any other
    unrecognized request) also has no effect.
    \param requestid ID of the request to cancel
    \importlib rwfilesystem
    \sa Stream::QueueFile, Stream::QueueMem, Stream::GetRequestState
*/
void Stream::CancelRequest(RequestId requestid)
{
    Stream *strm = GetStreamHeaderFromHandle(this);
    TapStruct *tap = NULL;

    LogStreamCancel(strm, requestid);

    strm->futex.Lock();

    RequestStruct *req = strm->locaterequest(requestid);

    if ((req == NULL) || (req->state == REQUESTSTATE_CANCELLED))
    {
        // Nothing to do.
    }
    else if (req->state == REQUESTSTATE_PENDING)
    {
        // Free the request and exit.
        strm->freerequest(req);
    }
    else  // completed or executing
    {
        // Place the request in the cancelled state.

        req->state = REQUESTSTATE_CANCELLED;

        if (req->type == FILEREAD)
            LogStreamRequestOperation(REQUESTCANCEL, strm, req->id, strm->fhandle, 0, req->fname);

        // go through the chunk queue and release any chunks for this request
        bool ChunkGotten = false;

        eastl::intrusive_list<ChunkStruct>::iterator chunk =
            strm->chunk.begin();

        while (chunk != strm->chunk.end())
        {
            if ((chunk->ci.reqId == req->id) &&
                (chunk->status == GETTABLE))
            {
                ChunkGotten = true;

                eastl::intrusive_list<TapStruct>::iterator tapItr =
                    strm->tap.begin();

                for (int i = 0; i < chunk->tapnum-1; ++i, ++tapItr)
                {
                    EA_ASSERT(tapItr != strm->tap.end());
                }

                tap = tapItr.mpNode;

                EA_ASSERT_MSG(tap != NULL, "Illegal tap index number");
                tap->gettable -= static_cast<int>(chunk->ci.size);
                EA_ASSERT(tap->gettable >= 0);
                strm->decbufferusage(static_cast<int>(chunk->ci.size));
                chunk->status = RELEASED;
            }

            ++chunk;
        }

        if (ChunkGotten)
        {
            // If one or more chunks were taken off the get queue,
            // set each taps gettable chunk pointer to point to the next
            // gettable chunk
            eastl::intrusive_list<TapStruct>::iterator TapItr;
            for (TapItr = strm->tap.begin(); TapItr != strm->tap.end(); ++TapItr)
            {
                if (TapItr->gettable > 0)
                {
                    if (TapItr->gettablechunk->status != GETTABLE)
                    {
                        int chunktap = TapItr->tapnum;
                        eastl::intrusive_list<ChunkStruct>::iterator
                            nextchunk = strm->chunk.begin();

                        // Iterate over the chunk queue until the next chunk is
                        // going to this tap and is gettable.
                        while ((nextchunk->tapnum != chunktap) ||
                            (nextchunk->status != GETTABLE))
                        {
                            //lint -e{1939} down cast detected
                            ++nextchunk;
                            EA_ASSERT_MSG(nextchunk != strm->chunk.end(),
                                "internal streamer error");
                        }

                        TapItr->gettablechunk = nextchunk;
                    }
                }
                else
                {
                    // There is no data at this tap.
                    TapItr->gettablechunk = strm->chunk.end();
                }
            }
        }
    }

#if defined(EA_ASSERT_ENABLED)
    if (tap != NULL)
    {
        int tmp_gettable(0);
        eastl::intrusive_list<ChunkStruct>::iterator cs =
            tap->gettablechunk;

        if (cs != strm->chunk.end())
        {
            EA_ASSERT(cs->status == GETTABLE);
        }

        while (cs != strm->chunk.end())
        {
            if (cs->status == GETTABLE)
            {
                tmp_gettable += cs->ci.size;
            }
            ++cs;
        }

        EA_ASSERT(tap->gettable == tmp_gettable);
    }
#endif

    strm->futex.Unlock();
}

/*!
    Cancels all requests in the stream and releases all chunks, including
    those that have been received but not released. This essentially restores
    the stream to the state it was in after it was created and initialized.

    Note that Stream::Kill returns immediately, so the stream may continue
    running for a short time before going idle. New requests can be queued
    during this interval.
    \importlib rwfilesystem
    \sa CancelRequest, Destroy
*/
void Stream::Kill()
{
    volatile Stream *strm = GetStreamHeaderFromHandle(this);

    LogStreamRequestOperation(KILL, const_cast<Stream *>(strm), 0, strm->fhandle, 0, const_cast<const char *>(strm->fname));

    const RequestStruct *req = strm->lastreq;
    if (req == NULL) return;

    // Remove all pending requests and cancel the executing request.
    // When this is done, all remaining requests will be completed
    // or canceled.
    while ((req->state == REQUESTSTATE_PENDING) ||
        (req->state == REQUESTSTATE_EXECUTING))
    {
        CancelRequest(req->id);
        req = strm->lastreq;
    }

    // Return all requests except the current one (which the streamer may
    // still be processing) to the free list. Make sure the current request
    // is in the canceled state.
    while (strm->firstreq != strm->curreq)
        const_cast<Stream *>(strm)->freerequest(strm->firstreq);

    strm->curreq->state = REQUESTSTATE_CANCELLED;

    // Release all chunks.

    eastl::intrusive_list<TapStruct>::iterator TapItr =
        const_cast<Stream *>(strm)->tap.begin();

    while (TapItr != const_cast<Stream *>(strm)->tap.end())
    {
        TapItr->gettable = 0;
        ++TapItr;
    }

    const_cast<Stream *>(strm)->decbufferusage(strm->bufferusage);

    const_cast<Stream *>(strm)->futex.Lock();

    eastl::intrusive_list<ChunkStruct>::iterator chunkItr = const_cast<Stream *>(strm)->chunk.begin();

    while (chunkItr != const_cast<Stream *>(strm)->chunk.end())
    {
        // Mark every chunk as released.
        chunkItr->status = RELEASED;

        ++chunkItr;
    }

    // A stopped stream must go idle.
    if (strm->state == STATE_STOPPED)
        strm->state = STATE_IDLE;

    // clear the half-full flag
    strm->flags &= ~STREAM_FLAG_HALF_FULL;

    // Cancel the associated async op as well.
    const_cast<Stream *>(strm)->mAsyncOp.Cancel();
    const_cast<Stream *>(strm)->futex.Unlock();
}

/*!
    Obtains a pointer to the next chunk in the stream buffer that has been
    directed to the specified output tap by the filtering process. When the
    chunk is no longer needed, it must be released using Stream::ReleaseChunk
    so that the space can be reused for reading new chunks. Note that the
    space can not be reused until all chunks that were read before the current
    chunk, including chunks from other taps, have also been released.

    The amount of data currently available at a specified tap can be
    determined using function Stream::Gettable. It is also possible to
    determine which request the next get will come from using function
    Stream::TapRequestId.
    \importlib rwfilesystem
    \return Returns a pointer to the next available chunk from the specified
    tap.  If no data is currently available at that tap, returns NULL.
    \sa ReleaseChunk, Gettable, TapRequestId
*/
Stream::ChunkInfo *Stream::GetChunk()
{
    TapStruct       *tap = reinterpret_cast<TapStruct *>(this);

    Stream *strm = GetStreamHeaderFromHandle(this);

    strm->futex.Lock();
    if (tap->gettable == 0)
    {
        strm->futex.Unlock();
        return NULL;
    }

    eastl::intrusive_list<ChunkStruct>::iterator chunk = tap->gettablechunk;
    EA_ASSERT_MSG(chunk != strm->chunk.end(),
        "Stream claims there is gettable data, but the chunkqueue is empty");

    // mark the the chunk as being user-controlled.
    chunk->status = RECEIVED;

    int chunksize = static_cast<int>(chunk->ci.size);
    Stream::ChunkInfo *ci = &chunk->ci;

    // Update the amount of gettable data at this tap.
    int gettable = tap->gettable - chunksize;
    tap->gettable = gettable;
    EA_ASSERT(tap->gettable >= 0);

    // modify the amount of data available for this request
    strm->request[chunk->ci.reqId & 0xff].gettable -= chunksize;
    EA_ASSERT(strm->request[chunk->ci.reqId & 0xff].gettable >= 0);

    // If there is still some gettable data, find it.
    if (gettable > 0)
    {
        int chunktap = tap->tapnum;
        //lint -e{1939} down cast detected
        ++chunk;
        EA_ASSERT_MSG(chunk != strm->chunk.end(), "internal streamer error");
        while((chunk->tapnum != chunktap) || (chunk->status != GETTABLE))
        {
            //lint -e{1939} down cast detected
            ++chunk;
            EA_ASSERT_MSG(chunk != strm->chunk.end(), "internal streamer error");
        }

        tap->gettablechunk = chunk;
    }
    else
    {
        tap->gettablechunk = strm->chunk.end();

        // If there is no more gettable data, clear the half-full flag
        // so we don't get the low-buffer warning.  We only want that
        // when we've released chunks and the streamer is not refilling
        // as fast as the data is being consumed.
        strm->flags &= ~STREAM_FLAG_HALF_FULL;
    }

    LogStreamGet(strm, chunk->ci.reqId, chunk.mpNode, chunk->ci.pData, chunk->ci.size);

#if defined(EA_ASSERT_ENABLED)
    {
        int tmp_gettable(0);
        chunk = tap->gettablechunk;

        if (chunk != strm->chunk.end())
        {
            EA_ASSERT(chunk->status == GETTABLE);
        }

        while (chunk != strm->chunk.end())
        {
            if (chunk->status == GETTABLE && chunk->tapnum == tap->tapnum)
            {
                tmp_gettable += chunk->ci.size;
            }
            ++chunk;
        }

        EA_ASSERT(tap->gettable == tmp_gettable);
    }
#endif

    // Note that we return an internal reference here.  When we want to go
    // back to a ChunkStruct, we'll need to make sure the pointers line up
    strm->futex.Unlock();
    return ci;
}

/*!
    Releases the specified chunk, obtained with Stream::GetChunk, so that the
    space can be reused for reading more chunks. Note that the space can
    not be reused until all chunks that were read before the current chunk,
    including chunks from other taps, have also been released.
    \param _chunk pointer to chunk to release
    \importlib rwfilesystem
    \sa GetChunk
*/
void Stream::ReleaseChunk(ChunkInfo *_chunk)
{
    State           streamstate;

    // This crazy pointer arithmetic is required because the
    // ChunkStruct starts two pointers before the ChunkInfo the
    // user has a handle on.  So we subtract 2 * size(void *) from
    // the ChunkInfo to get back to the ChunkStruct.
    ChunkStruct     *chunk = reinterpret_cast<ChunkStruct *>(
        reinterpret_cast<char *>(_chunk) - 2 * sizeof(void *));

    Stream *strm = GetStreamHeaderFromHandle(this);

    LogStreamRelease(strm, chunk->ci.reqId, chunk, chunk->ci.pData, chunk->ci.size);

    EA_ASSERT_MSG((strm->actualbufferstart <= chunk->ci.pData) &&
        (chunk->ci.pData < strm->bufferend),
        ("chunk does not belong to specified stream"));
    EA_ASSERT_MSG(chunk->status == RECEIVED,
        ("chunk has already been released"));

    strm->futex.Lock();

    // Mark the chunk as released. The streamer will free-up the space.
    chunk->status = RELEASED;

    // Update buffer usage now so we can detect if the stream is
    // running low and kick it into a higher priority level.
    strm->decbufferusage(static_cast<int>(chunk->ci.size));

    // If the stream is stopped, restart it at the appropriate priority level.
    streamstate = strm->state;
    if (streamstate == STATE_STOPPED)
        strm->state = STATE_RUNNING;
    strm->futex.Unlock();

    if (streamstate == STATE_STOPPED)
    {
        if (strm->greedystate) strm->restartstream(strm->priorityhigh);
        else strm->restartstream(strm->prioritylow);
    }
}

/*!
    Determines the amount of data gettable from a specified output tap.
    This amount increases as the stream is read and decreases as chunks
    are received.
    \importlib rwfilesystem

    \return Amount of data gettable from the specified output tap in bytes.
    \sa GetChunk
*/
int Stream::Gettable() const
{
    return reinterpret_cast<const TapStruct *>(this)->gettable;
}

/*!
    Determines the amount of data gettable from a specified request id.
    This amount increases as the stream is read and decreases as chunks
    are received.
    \importlib rwfilesystem

    \return Amount of data gettable from the specified request id in bytes,
    otherwise, return zero if the specified request id does not exist

    \sa GetChunk
*/
int Stream::Gettable(RequestId requestid) const
{
    Stream *strm = GetStreamHeaderFromHandle(this);
    RequestStruct *req = strm->locaterequest(requestid);

    if (req != NULL)
        return req->gettable;
    else
        return 0;
}

/*!
    Determines the current state of the specified stream. When there is
    nothing to read, the stream is idle. As soon as a request is queued,
    the stream enters the running state. If the stream buffer fills up,
    the stream enters the stopped state until enough data is released
    to resume reading.
    \importlib rwfilesystem

    \return One of the Stream::State values.
    \sa Create, State, IsEndOfStream
*/
Stream::State Stream::GetState() const
{
    return GetStreamHeaderFromHandle(this)->state;
}

/*!
    Determines if the end of stream has been reached at the specified output
    tap. The end of stream has been reached if the stream is idle and there
    is no more data gettable at the tap. Note that there may still be data
    gettable at other taps in the stream.
    \importlib rwfilesystem

    \return Returns true if the end of stream has been reached for the
    specified tap.
    \sa Gettable, GetState
*/
bool Stream::IsEndOfStream() const
{
    const TapStruct *tap = reinterpret_cast<const TapStruct *>(this);
    const Stream *strm = GetStreamHeaderFromHandle(this);

    return (strm->state == STATE_IDLE) && (tap->gettable == 0);
}

/*!
    Determines the size of the circular buffer used to hold chunk data for
    the specified stream. This value does not include the overhead allocated
    for storing state information in the buffer used to create the stream.
    \importlib rwfilesystem

    \return Returns the stream data buffer size in bytes.
    \sa Create
*/
int Stream::BufferSize() const
{
    const Stream *strm = GetStreamHeaderFromHandle(this);

    return static_cast<int>(strm->bufferend - strm->actualbufferstart);
}

/*!
    Determines how much of a stream's data buffer is currently in use.
    Stream data buffer usage is defined as the difference between the amount
    of data read and the amount of data released. Note that there may be
    sections of the data buffer that are not in use located between sections
    that are in use (in particular, when completed requests are canceled).
    These unused sections are not available for reading new data because
    of the circular nature of the data buffer.
    \importlib rwfilesystem

    \return Returns the stream data buffer usage in bytes.
    \sa BufferSize
*/
int Stream::BufferUsage() const
{
    return GetStreamHeaderFromHandle(this)->bufferusage;
}

/*!
    Determines the request from which the next chunk will come
    from when a GetChunk is performed on the specified output tap.

    Be aware that this function must traverse the request queue to
    locate the request. While this is efficient, it is not free.
    \importlib rwfilesystem

    \return  Returns the request ID of the request that will supply
    the next chunk at the specified output tap or zero if no data
    is gettable at that tap.
    \sa GetChunk
*/
Stream::RequestId Stream::TapRequestId() const
{
    const TapStruct *tap = reinterpret_cast<const TapStruct *>(this);
    Stream *strm = GetStreamHeaderFromHandle(this);

    if (tap->gettable == 0)
        return 0;

    strm->futex.Lock();

    const unsigned char *getptr = static_cast<unsigned char *>(tap->gettablechunk->ci.pData);
    const RequestStruct *req = strm->firstreq;
    const unsigned char *reqstart = strm->datastart;
    for (;;)
    {
        if (req->next == NULL) break;
        if (req->next->state == REQUESTSTATE_PENDING) break;
        const unsigned char *reqend = req->next->datastart;
        if (inbetween(reqstart, reqend, getptr)) break;
        reqstart = reqend;
        req = req->next;
    }

    strm->futex.Unlock();

    return req->id;
}

/*!
    Determines the current state of a specified request. When a request is
    first queued, it is in the pending state. As soon as the streamer starts
    executing the request, it moves to the executing state. When the streamer
    finishes executing the request, it moves to the completed state. If an
    executing or completed request is canceled, it is placed in the canceled
    state. Canceled and completed requests remain in the request queue until
    all of their chunks as well as chunks from previous requests have been
    released, at which point the request is freed. Pending requests are freed
    immediately when canceled. If a request cannot be found in the request
    queue, it is assumed to have been freed and is reported in the free state.
    \importlib rwfilesystem

    \return One of the Stream::RequestState values.
    \sa QueueFile, QueueMem, CancelRequest
*/
Stream::RequestState Stream::GetRequestState(RequestId requestid) const
{
    Stream *strm = GetStreamHeaderFromHandle(this);

    const RequestStruct *req = strm->locaterequest(requestid);

    if (req == NULL)
        return REQUESTSTATE_FREE;

    return req->state;
}

/*!
	Check if the last requested op status is complete
*/
bool Stream::IsOpComplete() const
{
	bool isComplete = false;
	Stream *strm = GetStreamHeaderFromHandle(this);

	isComplete = strm->mAsyncOp.GetStatus(EA::Thread::kTimeoutImmediate) !=	AsyncOp::OPSTATUS_PENDING;
	return isComplete;
}

/*!
    Returns the current request's file size (in bytes)
    \importlib rwfilesystem

    \return Size of the active request file (in bytes)
    QueueFile, QueueMem
*/
uint64_t Stream::GetFileSize() const
{
    return GetStreamHeaderFromHandle(this)->filesize;
}

/*!
    Returns whether the indicated driver is in use.
    \importlib rwfilesystem

    \return Whether the stream is using the indicated driver.
*/
bool Stream::IsUsingDevice(const DeviceDriver *pDeviceDriver) const
{
    bool usingDevice = false;
    const Stream *strm = GetStreamHeaderFromHandle(this);

    if ((strm->fhandle != 0) && (strm->fhandle->mDriver == pDeviceDriver))
        usingDevice = true;

    return usingDevice;
}

}}}
