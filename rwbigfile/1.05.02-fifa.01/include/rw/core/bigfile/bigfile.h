// (c) Electronic Arts. All Rights Reserved.

/**
\file
Header file of rw::core::Big.
*/

#ifndef RW_CORE_BIGFILE_H
#define RW_CORE_BIGFILE_H

#if defined _MSC_VER
    #pragma once
#endif

#include "EABase/eabase.h"

namespace rw {
namespace core {

// forward declaration of internal type
struct BigFileHeader;
struct BigFileFormatHeader;
struct BigFileDirectoryEntry;
/**
\brief Big class encapsulates the bigfile format and provides a way of
    accessing the member files.
*/
class Big
{
public:

    /**
    \brief Flags to pass during object construction.
    */
    enum Options
    {
        /*! This option is passed to the constructor and when done so will
            load the name index automatically.
        */
        OPTION_LOAD_NAME_INDEX = 1
    };

    /**
    \brief This package (tools and libraries) support these types of bigfiles.
    */
    enum BigfileType
    {
        /*! Preferred format.  32-byte header, fixed-size index entries with
            hashed names for fast searching, per-file flags and optional name
            strings.  Compression formats supported: refpack, chunked refpack,
            and chunked zlib.

            Created by the bigfile tool.
        */
        BFTYPE_FAST,

        /*! Legacy format.  Each index entry has an N-byte-sized offset and
            size field, where N can be 2, 3, 4, or 5.  Supports bigfiles up to
            2^40 bytes in size.  Can compress with refpack.  Created by the
            glump tool.
        */
        BFTYPE_N,

        /*! Legacy format.  Each index entry has a 24-bit offset and size
            field.  The maximum size total size of a bigfile with this type of
            header is 16MB.  Created by the glump tool with the -24 option.
        */
        BFTYPE_24,

        /*! Legacy format.  Hash16 format (offset/zSize/size/32-bit hash for
            name)  Created by the bighead tool with the -hash16 option.
        */
        BFTYPE_HASH16,

        /*! Legacy format.  Hash20 format (offset/zSize/size/64-bit hash for
            name)  Created by the bighead tool with the -hash20 option.
        */
        BFTYPE_HASH20,

        BFTYPE_UNKNOWN  ///< This is not a recognized type of bigfile.
    };

    /**
    \brief flags for Big::Locate
    */
    enum LocateOptions
    {
        /** Do not modify the file path
        */
        LOCATE_OPT_NONE = 0x0,

        /** All backslashes will be converted to forward slashes (glump -paths2)
        */
        LOCATE_OPT_SLASH_CONVERT_FORWARD = 0x1,

        /** All forward slashes will be converted to back slashes (glump -paths4)
        */
        LOCATE_OPT_SLASH_CONVERT_BACK = 0x2,

        /** Name will be converted to lower case (glump -tolower, bigfile -nameLower)
        */
        LOCATE_OPT_NAME_CONVERT_LOWERCASE = 0x4,

        /** Name will be converted to upper case (glump -toupper, bigfile -nameUpper)
        */
        LOCATE_OPT_NAME_CONVERT_UPPERCASE = 0x8

    };

    /*! Default hash seed value for GetDJB2HashBigEndian.  This should
        match the seed value in the bighead tool.
    */
    static const uint64_t DEFAULT_HASH_SEED = 5381;

    Big(const void *bigFilePtr, const void *bigfileHeader = NULL);
    Big(const void *bigFilePtr, unsigned int flags);

    // Functions that support both bigfile format headers
    static unsigned int GetHeaderSize(const void *bigfileptr);
    static unsigned int GetNameIndexSize(const void *bigFilePtr);
    static BigfileType GetHeaderType(const void *bigFilePtr);

    static unsigned int GetFATSize(const void *bigFilePtr);
    static bool HasFAT(const void *bigFilePtr);
    bool HasFAT() const;
    const BigFileDirectoryEntry *GetEntryForIndex(uint32_t index) const;
    const uint32_t *GetConstituents(const BigFileDirectoryEntry &BFD) const;
    const char *GetFATStringForIndex(uint32_t index) const
    { 
        return mFATStringTable + index; 
    }

    void SetFile(const void *bigFilePtr, const void *bigfileHeader = NULL);
    void SetNameIndex(const void *nameIndex);

    // Functions to support the BFTYPE_FAST, BFTYPE_HASH16, and BFTYPE_HASH20
    // formats, where the name strings are hashed to fixed-sized entries.
    static uint64_t GetDJB2HashBigEndian(const char *string,
        uint64_t seed = DEFAULT_HASH_SEED);

    // Given a name, return its offset, size, compressed size and compression
    // format
    bool FindFileByName(const char *name, uint64_t *entryOffset=NULL,
        uint64_t *entrySize=NULL, uint64_t *zSize=NULL, int *tocIndex=NULL,
        unsigned int flags = 0, unsigned int *entryFlags = NULL) const;

    // Given a hash value, return its offset, size, compressed size and
    // compression format
    bool FindFileByHash(uint64_t hashedName, uint64_t *entryOffset=NULL,
        uint64_t *entrySize=NULL, uint64_t *zSize=NULL, int *tocIndex=NULL,
        unsigned int *entryFlags = NULL) const;

    // Given an index number, return its offset, size, compressed size, and
    // compression format.  Note that the index is sorted by hash value, and
    // hence the order in which the files are stored in the index may change
    // as new files are added.
    void FindFileByIndex(int index, uint64_t *entryOffset=NULL,
        uint64_t *entrySize=NULL, uint64_t *zSize=NULL, unsigned int *entryFlags=NULL) const;

    // Given an index number, return its name.  Only works with BFTYPE_FAST
    // bigfiles, and only if a name index has been set (see SetNameIndex)
    char *FindNameByIndex(int entryNum, char *name, size_t nameLen) const;

    // Legacy functions that exist to support BFTYPE_N, and BFTYPE_24 formats.
    // These functions may be removed in a future version of this package.
    const char *LocateEntry(const char *name, int entryNum, uint64_t *entryOffset,
        uint64_t *entrySize, void **tocPtr=NULL, int *tocIndex=NULL, unsigned int flags = 0) const;

    // All these functions are simple wrappers for the LocateEntry function
    int GetCount() const;
    int GetVersion() const;
    void *GetPointer(int number) const;
    void *Locate(const char *name) const;
    char *GetName(int number, char *name, unsigned int flags = 0) const;
    const void *GetBaseAddress() const;

#ifdef EA_DEBUG
    uint32_t GetHashValueByIndex(int i) const;
#endif

private:

    static int Hash16Cmp(const void *, const void *);
    static int Hash20Cmp(const void *, const void *);

    int GetDebugInfo(int &version) const;
    void SetFAT(const void *bigFilePtr);
    const char *ConvertPath(const char *path, unsigned int flags, char *modifiedPathBuffer) const;
    const char *GetAddressOfHashedIndex() const;

    const BigFileFormatHeader *mBigFileHashedHeader;    // BFTYPE_FAST/BFTYPE_HASH16/BFTYPE_HASH20 formats
    const void *mBigFileNameIndex;                      // BFTYPE_FAST format, to record the location of the optional name index
    const BigFileHeader *mBigFileData;                  // BFTYPE_N, BFTYPE_24 formats
    const BigFileHeader *mBigFileHeader;                // BFTYPE_N, BFTYPE_24 formats

    const BigFileDirectoryEntry *mBigFileDirectoryEntries;
    const uint32_t *mFATConstituents;
    const char *mFATStringTable;
};

}}

#endif
