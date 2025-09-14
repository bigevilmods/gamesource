// (c) Electronic Arts. All Rights Reserved.

/**
\file
Header file of rw::core::BigHandler.
*/

#ifndef RW_CORE_BIGFILE_HANDLER_H
#define RW_CORE_BIGFILE_HANDLER_H

#ifdef _MSC_VER
#pragma once
#endif

#include "EABase/eabase.h"
#include <EAThread/eathread_futex.h>
#include "rw/core/bigfile/bigfile.h"
#include "rw/core/filesys/directory.h"
#include "coreallocator/icoreallocator_interface.h"
#include "job_manager/job_scheduler.h"


#include "rw/core/codec/version.h"
#if !defined(EA_DLL) && RW_CODEC_VERSION >= RW_CODEC_CREATE_VERSION_NUMBER(1,4,0)
    #define RWBIGFILE_CHUNKED_DECOMPRESSION_SUPPORTED
#endif


namespace rw {
namespace core {

// forward-declare externally-referenced class
// so we don't need to include the header
namespace codec {
    class ICodec;
    class ChunkDecompressor;
    class ChunkReader;
}

class ChunkDecompressionClosure;

/**
\brief This class implements a file system handler for Bigfiles.

After the object has been registered with the file library, use
BigHandler::AddArchiveFromFile to register one or more bigfiles with
the device.  Files inside bigfiles can then be loaded as if they
were on the normal file system.

This class supports directory searches, but if you use that
feature, the name strings must be available, otherwise no patterns
will match.
*/
class BigHandler : public rw::core::filesys::DeviceDriver
{
    friend class ChunkDecompressionClosure;

public:
    BigHandler(EA::Allocator::ICoreAllocator* allocator);
    BigHandler(EA::Allocator::ICoreAllocator* allocator, const char *deviceName);

#ifdef RWBIGFILE_CHUNKED_DECOMPRESSION_SUPPORTED
    BigHandler(EA::Allocator::ICoreAllocator* allocator, 
            const char *deviceName, 
            EA::Jobs::JobScheduler *jobScheduler,
            uint32_t unused,
            EA::Jobs::JobAffinity jobAffinity,
            EA::Jobs::JobPriority jobPriority);
#endif

    virtual ~BigHandler();

    /*! \brief Flags for AddArchiveFromFile
    */
    enum AddOption
    {
        /// Use an alternate header instead of the one included in
        /// the bigfile.  The name should be \"<bigfile>.bh\".  If
        /// the alternate header is not available, the header inside
        /// the main bigfile will be used as a fallback.
        ADDFLAG_USE_ALTERNATE_HEADER = 0x01,

        /// Add the big file to the handler as head. The newly added 
        /// big file will be searched first.
        ADDFLAG_ADD_TO_HEAD = 0x02,

        /// When opening the bigfile (and the alternate header, if
        /// used), append the .edat extension and pass the
        /// AsyncOp::FOPEN_DRM flag when the bigfile (but not the 
        /// subfiles) are opened.  You must be using rwfilesystem 
        /// 1.07.00 or later for this flag to be respected.  If not,
        /// this flag is silently ignored.
        ADDFLAG_DRM = 0x04,

        /// This option will convert any backslashes in the path
        /// provided by the caller to forward slashes before doing
        /// the name lookup.  Use this if you built your bigfiles
        /// with the paths2 option.
        ADDFLAG_SLASH_CONVERT_FORWARD = 0x08,

        /// This option will convert any forward slashes in the path
        /// provided by the caller to backslashes before doing
        /// the name lookup.  Use this if you built your bigfiles
        /// with the paths4 option.
        ADDFLAG_SLASH_CONVERT_BACKWARD = 0x10,

        /// This option will convert names provided by the caller
        /// to lowercase before doing the name lookup.
        /// See -tolower in glump.exe or -nameLower in bigfile.exe
        ADDFLAG_NAME_CONVERT_LOWERCASE = 0x20,

        /// This option will convert names provided by the caller
        /// to uppercase before doing the name lookup.
        /// See -toupper in glump.exe or -nameUpper in bigfile.exe
        ADDFLAG_NAME_CONVERT_UPPERCASE = 0x40,

        /// Use this flag to decompress external to rwbigfile,
        /// ie. ignore BIGFILE_FORMAT_FLAG_COMPRESSION_TYPE_*.
        ADDFLAG_EXTERNAL_DECOMPRESS = 0x80,

        /// This flag tells the handler to load the name index.  The
        /// use-case for loading the name index is to do wild-card
        /// searches.
        ADDFLAG_LOAD_NAME_INDEX = 0x100,

        /// Add the big file to the handler as a permanent head.
        /// The newly added big file will always be searched first.
        /// Example use would be for a patch.big file where patched
        /// data should always take precedence over other files.
        ADDFLAG_ADD_PERMANENT_HEAD = 0x200,
        
        // The big file needs to be loaded using FOPEN_SDATA , see async.h
        ADDFLAG_FILE_USES_SDATA = 0x400,

    };

    /*! \brief Flags for Open.
    */
    enum OpenFlags
    {
        /// Use in-place decompression, if the output buffer is large
        /// enough.  This feature avoids the need for a temporary buffer
        /// to hold the compressed data.  Instead, the compressed data
        /// is read into the tail end of the output buffer, and
        /// decompressed to the start.  Requires that the buffer be a
        /// few bytes larger than the decompressed file size.  If this
        /// flag is not specified, or the buffer is not large enough to
        /// hold the file, the decompression will be done via streaming,
        /// which is not as fast and requires temporary buffers.
        FOPEN_DECOMPRESS_IN_PLACE = 0x10000,

        /*! If the path specifies a specific bigfile, (e.g.
            \"myfile.big|file1.xml\", 
        */
        FOPEN_SEARCH_OTHER_BIGFILES = 0x20000
    };

    void *              AddArchiveFromFile(const char *filename, unsigned int flags = 0);
     void *                   AddArchiveFromMemory(const char *filename, char* bigdata, char* bighead = NULL, unsigned int flags = 0);
    void                RemoveArchive(void *archive);
    void *              GetHeader(void *archive);
    unsigned int        GetBigFileCount() const
    {
        return mBigfileList.GetCount();
    }

    ///FindEntry searches the big file list for file name and optionally fills params from Big::FindFileByName or Big::LocateEntry.
    /// returns the entry's offset if found, otherwise zero.
    /// entryArchive is the return value from AddArchiveFromFile, also the parameter to RemoveArchive.
    uint64_t            FindEntry(const char* name, 
        void** entryArchive = NULL,
        uint64_t* entryOffset = NULL,
        uint64_t* entrySize = NULL,
        uint64_t* zSize = NULL,
        int* tocIndex = NULL,
        unsigned int *entryFlags = NULL);
    bool                EntryExists(const char *name) { return(FindEntry(name) > 0); }

#ifdef EA_DEBUG

    /// \brief Pass this flag if you want the Hash16CollisionCheck to
    /// throw an assert if it detects any collisions.
    static const unsigned int HASH_COLLISION_CHECK_ASSERT = 0x01;

    /*! \brief Scan any registered bigfiles that have Hash16
        headers looking for any collisions.

        This function is NOT thread-safe.  Do not call it from more
        than one thread at the same time.

        \param flags - 0 or HASH_COLLISION_CHECK_ASSERT if you want
        the function to throw an assert if a collision is found.
        \return true - No collisions found.  false - one or more
        names collided
    */
    bool                Hash16CollisionCheck(unsigned int flags = 0);
    static int          uint_cmp(const void *, const void *);
#endif

private:

    // Device Driver implementation functions
    virtual void *      Open(const char *name, unsigned int oflags, rw::core::filesys::Handle **pParentFileHandle);
    virtual void        Close(void *);
    virtual unsigned int Read(void *, void *, unsigned int, rw::core::filesys::DeviceDriver *, void *);
    virtual uint64_t    Seek(void *, uint64_t, int, rw::core::filesys::DeviceDriver *, void *);
    virtual uint64_t    GetSize(void *);
    virtual uint64_t    QueryLocation(void *h);
    virtual void *      FindBegin(const char *, rw::core::filesys::DeviceDriver::FindData *);
    virtual bool        FindNext(void *, rw::core::filesys::DeviceDriver::FindData *);
    virtual bool        FindEnd(void *);

    bool FindNextWithFAT(void *, rw::core::filesys::DeviceDriver::FindData *);

    virtual void *GetReadProcessingContext()
    { return pContext; }

    virtual int         ExecuteProcessingFunction(void *context, unsigned int *numRead);
    virtual rw::core::filesys::Device *ReadProcessDevice();

    // supporting functions, not part of the driver API.
    static void         CleanPath(char *buffer);
    static bool         SearchStringMatches(const char *str, const char *searchString );

    // brief Class implementation functions
    class BigFileStruct
    {
    public:
        BigFileStruct(rw::core::filesys::Handle* handle, void *header,
                      char *name, uint64_t location, unsigned int flags, char *bighead, char *bigdata);

        void *operator new(size_t /*size*/, void *p) { return p;}   // placement new
        void operator delete(void * /*ptr*/) {}                     // delete operator does NOT free the memory

        BigFileStruct *next;                // next bigfile in the chain
        rw::core::filesys::Handle *hFile;   // physical file handle
        void       *header;                 // bigfile header
        Big         mBig;
        char       *name;                   // name of container file
        uint64_t    location;               // physical location of the file on the device
        int         headerformat;           // header format
        unsigned int flags;                 // flags for this bigfile
        char*       bighead;                // big file header
        char*       bigdata;                // big file data
    };

    struct BigFileAsyncReadProcessContext
    {
        void *dstBuf;
        const void *srcBuf;
    };

    // subfile structure
    class MemberFileStruct
    {
    public:
        MemberFileStruct() 
            : pBigfile(NULL), 
              signature(0),
              modeflags(0),
              fileflags(0),
              inf(NULL),
              tmpZbuf(0),
              baseoffset(0),
              curroffset(0),
              length(0),
              zLength(0),
              chunkDecompressionClosure(NULL)
        {}

        MemberFileStruct(BigFileStruct *bf, unsigned int modeflags, unsigned int fileflags, uint64_t baseoffset, uint64_t length, uint64_t zLength);
        void *operator new(size_t, void *p) { return p; }
        void operator delete(void *) {}

        BigFileStruct * pBigfile;   // pointer to container bigfile
        int             signature;  // debug signature check
        unsigned int    modeflags;  // open mode flags
        unsigned int    fileflags;  // format flags (currently used to indicate compression format)
        rw::core::codec::ICodec *inf; // for streamed codecs, this is the decompressor.
        void *          tmpZbuf;    // for streamed codecs, this buffer holds the compressed data
        uint64_t        baseoffset; // offset to start of member data
        uint64_t        curroffset; // current file offset from beginning of subfile.
                                    // If the file is compressed, this is number of
                                    // bytes read from the disk, it is not the
                                    // number bytes that have been decompressed.
        uint64_t        length;     // size of member file
        uint64_t        zLength;    // compressed size of member file; 0 = uncompressed
        ChunkDecompressionClosure *chunkDecompressionClosure;
    };

    class BigFileList
    {
    public:
        BigFileList() : head(NULL), permanenthead(NULL), tail(NULL), nNodes(0) {}

        BigFileStruct * GetHead() const;
        unsigned int    GetCount() const { return nNodes; }
        void            PushHead(BigFileStruct *node);
        void               PushPermanentHead(BigFileStruct *node);
        void            PushTail(BigFileStruct *node);
        bool            Remove(BigFileStruct *node);

    private:
        BigFileStruct *head;
        BigFileStruct *permanenthead;
        BigFileStruct *tail;
        unsigned int nNodes;
    };

    struct FindStruct
    {
        FindStruct(const char *pattern, BigFileStruct *);

        void *operator new(size_t, void *p) { return p; }

        // bigfile containing a match for the given pattern
        BigFileStruct *pBigFile;

        // last match in the current bigfile; GetNextFile starts looking at one
        // past this location the next time it's called.
        int     fileIndex;

        // last LocateEntry index values
        int     tocIdx;
        void *  tocPtr;
        uint32_t fatDirectoryEntry;

        // pattern to match, passed to OpenDirectory by caller
        char    Pattern[filesys::FILESYS_MAX_PATH];
        // searches through the FAT trim down the pattern to remove
        // any directory prefixes. This pattern must be stored separate
        // from the "full" pattern so that when we cascade from one
        // bigfile to the next we can refer to the original pattern
        char    TrimmedPattern[filesys::FILESYS_MAX_PATH];
    };

    BigFileAsyncReadProcessContext *pContext;   // stores a pointer to the compressed data buffer
    BigFileList mBigfileList;   // list of registered bigfiles
    EA::Allocator::ICoreAllocator *mAllocator;
    rw::core::codec::ChunkDecompressor *mChunkDecompressor;
    BigHandler(const BigHandler& rhs);               // copy constructor
    BigHandler& operator=(const BigHandler& rhs);    // assignment operator
    EA::Thread::Futex        mBigFileListMutex;
};

}}

#endif
