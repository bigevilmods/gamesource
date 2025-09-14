// (c) Electronic Arts. All Rights Reserved.

/**
\file
Implements rw::core::Big.
*/

#include <ctype.h>
#include "EAAssert/eaassert.h"
#include "EAStdC/EAString.h"
#include "EAStdC/EAAlignment.h"
#include "EAStdC/EASprintf.h"
#include "Endian/endian.h"

#include "rw/core/bigfile/bigfile.h"
#include "rw/core/bigfile/fileformat.h"

using namespace EA::StdC;

namespace rw
{
    namespace core
    {

#ifndef MAKEID
#define MAKEID(a,b,c,d) (((static_cast<int>(a))<<24)|((static_cast<int>(b))<<16)|((static_cast<int>(c))<<8)|static_cast<int>(d))
#endif

// Big file ID's for new and old formats.
#define BIGHEAD_HASH16ID MAKEID('V','i','v','4')    // Bighead Hash16 (32-bit)
#define BIGHEAD_HASH20ID MAKEID('V','i','V','4')    // Bighead Hash20 (64-bit hash)
#define BIGNID  MAKEID('B','I','G','0')             // BigFile 8, 16, 24, 32, 40-bit
#define BIG24ID 0xc0fb                              // legacy 24-bit

// Old and new big file formats. Big file header is followed by a
// table of contents entry for each file in the big file.
// Note that all fields are stored in Big-endian format, so this
// must be taken into consideration whenever anything is read from
// the bigfile data.

struct BIGENTRY16
{
    char    offset[2];      ///< File offset relative to start of big file.
    char    length[2];      ///< Length of file (in bytes)
    char    name[1];        ///< Null-terminated name of file (variable length).
};

struct BIGENTRY24
{
    char    offset[3];      ///< File offset relative to start of big file.
    char    length[3];      ///< Length of file (in bytes)
    char    name[1];        ///< Null-terminated name of file (variable length).
};

struct BIGENTRY32
{
    char    offset[4];      ///< File offset relative to start of big file.
    char    length[4];      ///< Length of file (in bytes)
    char    name[1];        ///< Null-terminated name of file (variable length).
};

struct BIGENTRY40
{
    char    offset[5];      ///< File offset relative to start of big file.
    char    length[4];      ///< Length of file (in bytes)
    char    name[1];        ///< Null-terminated name of file (variable length).
};

class BIGENTRYHASH
{
public:
    uint32_t    GetOffset() const
    { 
        return EA::Endian::big_get(&offset, sizeof(offset)); 
    }

    uint32_t    GetzSize() const
    { 
        return EA::Endian::big_get(&zSize, sizeof(zSize)); 
    }

    uint32_t    GetSize() const
    { 
        return EA::Endian::big_get(&size, sizeof(size)); 
    }

private:
    // These should not be accessed directly because they're in big-endian format.
    // Instead, get them through the accessor functions, which handle the endian
    // conversion.
    uint32_t    offset;     // offset from start of file
    uint32_t    zSize;      // compressed file size
    uint32_t    size;       // uncompressed size (if 0, the file is not compressed)
};

// 16-byte hashed name format, created by bighead tool
// with -hash16 flag.
class BIGENTRYHASH16
{
public:
    uint32_t GetNameHash() const
    { return EA::Endian::big_endian(nameHash); }

private:
    BIGENTRYHASH entry;
    uint32_t    nameHash;   // 32-bit big-endian DJB2 hash of the input name
};

// 20-byte hashed name format, created by bighead tool
// with -hash20 flag.
struct BIGENTRYHASH20
{
public:
    uint64_t GetNameHash() const
    {
#if defined(EA_SYSTEM_LITTLE_ENDIAN)
        uint64_t nameHashAsUint64 = static_cast<uint64_t>(nameHash[1]) << 32 | nameHash[0];
#else
        uint64_t nameHashAsUint64 = static_cast<uint64_t>(nameHash[0]) << 32 | nameHash[1];
#endif

        return EA::Endian::big_endian(nameHashAsUint64);
    }
private:
    BIGENTRYHASH entry;
    uint32_t    nameHash[2];    // 64-bit big-endian DJB2 hash of the input name
};


// 16/n24/n32/40-bit big file header format

struct BigFileHeader
{
    uint32_t    type;   // BIGNID (N=2,3,4,5 or 'F') for big file.  VIV<n> for bighead
    uint32_t    len;    // Length of file in bytes.
    uint32_t    num;    // Number of files in big file.
    uint32_t    hlen;   // Header length (*including* this structure).
};

// Legacy 24-bit big file header format.
struct BigFileHeaderLegacy
{
    uint16_t    type;   // BIG24ID
    uint16_t    hlen;   // Header length (*excluding* this structure)
    uint16_t    num;    // Number of files in big file
};

/// \brief     Construct a Big object from a memory image.
///
/// It is not necessary that the entire bigfile be loaded; just the header.
/// \param bigFilePtr buffer containing the loaded bigfile (or bigfile header)
/// \param bigfileHeader buffer containing an alternate header (optional)
/// \importlib rwbigfile
Big::Big(const void *bigFilePtr, const void *bigfileHeader)
: mBigFileNameIndex(NULL)
{
    // set the member data via SetFile instead of an initializer list
    SetFile(bigFilePtr, bigfileHeader);
}

/// \brief     Construct a Big object from a memory image.
///
/// The bighead tool does not output the name index for bigfile-created 
/// archives so this constructor does not take a separate header parameter.
/// \param bigFilePtr buffer containing the loaded bigfile (or bigfile header)
/// \param flags flags specifying which load-time options to perform, such
///              as OPTION_LOAD_NAME_INDEX to load the name index.
/// \importlib rwbigfile
Big::Big(const void *bigFilePtr, unsigned int flags)
: mBigFileNameIndex(NULL)
{
    SetFile(bigFilePtr, NULL);
    EA_ASSERT(GetHeaderType(bigFilePtr) == BFTYPE_FAST);

    if (flags & OPTION_LOAD_NAME_INDEX)
    {
        const unsigned int headerSize = GetHeaderSize(bigFilePtr);
        SetNameIndex(static_cast<const char *>(bigFilePtr) + headerSize);
    }
}

/// \brief     Parses and sets up the tree-based directory structure if
//             if it exists within the bigfile metadata.
void Big::SetFAT(const void *bigFilePtr)
{
    EA_ASSERT(mBigFileHashedHeader != NULL);

    if (!HasFAT(bigFilePtr))
        return;

    const char *bigFileBytePtr = static_cast<const char *>(bigFilePtr);
    const char *directoryEntryBase = bigFileBytePtr + 
                EA::Endian::big_endian(mBigFileHashedHeader->baseHeaderSize) +
                EA::Endian::big_endian(mBigFileHashedHeader->nameHeaderSize);

    const uint32_t directoryEntrySize = EA::Endian::big_endian(
            *reinterpret_cast<const uint32_t *>(directoryEntryBase));

    mBigFileDirectoryEntries = reinterpret_cast<const BigFileDirectoryEntry *>(
            directoryEntryBase + sizeof(uint32_t));
    mFATConstituents = reinterpret_cast<const uint32_t *>(directoryEntryBase + 
            sizeof(uint32_t) + directoryEntrySize + sizeof(uint32_t));
    
    const uint32_t constituentSize = EA::Endian::big_endian(mFATConstituents[-1]);
    mFATStringTable = reinterpret_cast<const char *>(directoryEntryBase +
            sizeof(uint32_t) + directoryEntrySize + sizeof(uint32_t) +
            constituentSize + sizeof(uint32_t));
}

/// \brief     Set a new bigfile to operate on.
///
/// It is not necessary that the entire bigfile be loaded; just the header.
/// \param bigFilePtr buffer containing the loaded bigfile (or bigfile header)
/// \param bigfileHeader buffer containing an alternate header (optional)
/// \importlib rwbigfile
void Big::SetFile(const void *bigFilePtr, const void *bigfileHeader)
{
    mBigFileData = static_cast<const BigFileHeader *>(bigFilePtr);

    if (GetHeaderType(bigFilePtr) == BFTYPE_FAST)
    {
        // We have a new bigfile format
        mBigFileHashedHeader = static_cast<const BigFileFormatHeader *>(bigFilePtr);
        mBigFileHeader = NULL;

        SetFAT(bigFilePtr);
    }
    else if (bigfileHeader != NULL)
    {
        // Alternate header is being used
        if (GetHeaderType(bigfileHeader) == BFTYPE_FAST)
        {
            // Alternate header in new format
            mBigFileHashedHeader = static_cast<const BigFileFormatHeader *>(bigfileHeader);
            mBigFileHeader = NULL;

            SetFAT(bigFilePtr);
        }
        else
        {
            // Alternate header in legacy format
            mBigFileHashedHeader = NULL;
            mBigFileHeader = static_cast<const BigFileHeader *>(bigfileHeader);
        }
    }
    else
    {
        // otherwise we have a legacy header format and can overlay the hashed
        // header on top of the mBigFileData
        mBigFileHeader = mBigFileData;
        mBigFileHashedHeader = NULL;
    }

#ifdef EA_ASSERT_ENABLED

    // Lets do a bit of validation
    BigfileType bfType;

    if (mBigFileHashedHeader)
        bfType = GetHeaderType(mBigFileHashedHeader);
    else
        bfType = GetHeaderType(mBigFileHeader);

    EA_ASSERT_FORMATTED((bfType == BFTYPE_N) || (bfType == BFTYPE_24) ||
        (bfType == BFTYPE_HASH16) || (bfType == BFTYPE_HASH20) ||
        (bfType == BFTYPE_FAST),
        ("Unrecognized bigfile format 0x%08x", bfType));

    if (bfType == BFTYPE_FAST)
    {
        EA_ASSERT(mBigFileHashedHeader != NULL);
        EA_ASSERT(mBigFileHeader == NULL);

        // We have a new bigfile format
        // if the caller passed a second bigfile header, lets try to 
        // ensure there's a good chance it's for the same bigfile
        if (static_cast<const void *>(mBigFileHashedHeader) != 
            static_cast<const void *>(mBigFileData))
        {
            EA_ASSERT_MSG(mBigFileHashedHeader->fileCount == mBigFileData->num, 
                "BigFile headers do not match");
        }
    }
    else
    {
        EA_ASSERT(mBigFileHashedHeader == NULL);
        EA_ASSERT(mBigFileHeader != NULL);

        if (mBigFileHeader != mBigFileData)
        {
            // if the caller passed a second bigfile header, lets try to 
            // ensure there's a good chance it's for the same bigfile
            EA_ASSERT_FORMATTED(mBigFileHeader->num == mBigFileData->num, 
                ("BigFile headers do not match"));
        }
    }
#endif
}

/// \brief     Set a name index
///
/// \param nameIndex pointer to name index in bigfile
/// \sa FindNameByIndex
/// \importlib rwbigfile
void Big::SetNameIndex(const void *nameIndex)
{
    mBigFileNameIndex = nameIndex;
}

/*! \brief calculate a DBJ2 hash on the input string
    \param string Input string to hash.
    \param hash Starting hash value

    Reference: http://www.cs.yorku.ca/~oz/hash.html
    \return 64-bit hash of the input string, converted to big-endian format
    \sa FindFileByHash
    \importlib rwbigfile
*/
uint64_t Big::GetDJB2HashBigEndian(const char *string, uint64_t hash)
{
    const unsigned char *s = reinterpret_cast<const unsigned char *>(string);
    uint64_t c;

    while ((c = static_cast<uint64_t>(*s++)) != 0)
    {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }

    return hash;
}

/// \brief     Get the size of a big file header
///
/// \importlib rwbigfile
///
/// Returns the size in bytes of a big file header.  This information
/// is contained inthe first 16 bytes of the header, so you don't need
/// to read the entire header into memory if you just want to get the
/// complete header size.  This function is static so it can be used
/// before the bigfile object has been constructed.
/// 
/// \return Size in bytes of the bigfile header.  If it is not a
/// recognized big file type, 0 is returned.
/// 
/// \sa GetHeaderType, LocateEntry

unsigned int Big::GetHeaderSize(const void *bigFilePtr)
{
    unsigned int size = 0;

    switch (GetHeaderType(bigFilePtr))
    {
    case BFTYPE_FAST:
        size = EA::Endian::big_get(&static_cast<const BigFileFormatHeader *>(bigFilePtr)->baseHeaderSize, 4);
        break;
    case BFTYPE_N:
    case BFTYPE_HASH16:
    case BFTYPE_HASH20:
        size = EA::Endian::big_get(&static_cast<const BigFileHeader *>(bigFilePtr)->hlen, 4);
        break;

    case BFTYPE_24:
        // First two fields are *not* included in this
        // header format, so add 4 bytes for them.
        size = EA::Endian::big_get(&static_cast<const BigFileHeaderLegacy *>(bigFilePtr)->hlen, 2) + 4;
        break;

    case BFTYPE_UNKNOWN:
    default:
        break;
    }

    return size;
}

/// \brief     Get the size of a name index (only applies to BFTYPE_FAST bigfiles)
///
/// \importlib rwbigfile
///
/// \return Size in bytes of the bigfile header.  Can only be used with BFTYPE_FAST
/// bigfiles.
/// 
/// \sa GetHeaderType, LocateEntry

unsigned int Big::GetNameIndexSize(const void *bigFilePtr)
{
    EA_ASSERT(GetHeaderType(bigFilePtr) == BFTYPE_FAST);

    return EA::Endian::big_endian(static_cast<const BigFileFormatHeader *>(bigFilePtr)->nameHeaderSize);
}


unsigned int Big::GetFATSize(const void *bigFilePtr)
{
    EA_ASSERT(GetHeaderType(bigFilePtr) == BFTYPE_FAST);

    return EA::Endian::big_endian(static_cast<const BigFileFormatHeader *>(bigFilePtr)->fatSize);
}


bool Big::HasFAT(const void *bigFilePtr)
{
    const uint16_t flags = EA::Endian::big_endian(static_cast<const BigFileFormatHeader *>(bigFilePtr)->flags);
    if (flags & rw::core::BIGFILE_FORMAT_FLAG_INCLUDES_FAT)
        return true;

    return false;
}

bool Big::HasFAT() const
{
    const uint16_t flags = EA::Endian::big_endian(mBigFileHashedHeader->flags);
    if (flags & rw::core::BIGFILE_FORMAT_FLAG_INCLUDES_FAT)
        return true;

    return false;
}


const BigFileDirectoryEntry *Big::GetEntryForIndex(uint32_t index) const
{
    return mBigFileDirectoryEntries + index;
}

const uint32_t *Big::GetConstituents(const BigFileDirectoryEntry &BFD) const
{
    using namespace EA::Endian;

    const uint32_t index = big_endian(BFD.ConstituentStartIndex);
    return mFATConstituents + index; 
}


/// \brief Locate file in bigfile return size, offset, name
///
/// \importlib rwbigfile
///
/// Legacy function supporting BFTYPE_N and BFTYPE_24 formats.
/// Does not work with BFTYPE_HASH16, BFTYPE_HASH20, or BFTYPE_FAST
/// formats.
///
/// The LocateEntry function searches a big file directory for the specified
/// file and returns the offset and size of the file. If entryname is not NULL
/// then a case-insensitive search for that file name is performed, otherwise
/// the entrynum-th file in the big file is located. The returned offset is
/// from the start of the bigfile.
///
/// Note that the path directory separator character must be consistent with
/// what was used when the bigfile was produced.  i.e. If your file path is
/// \"dir\\file.txt\" but the index contains \"dir/file.txt\", the entry will
/// not be found.  glump has options to use either forward slahes or back
/// slashes for the paths, and this function takes flags that can also modify
/// the path at runtime to use consistent directory separator characters.
///
/// \return A pointer to the name of the lump file is returned, or NULL is
/// returned on failure to find the requested file.  The offset and size are
/// returned via the pointers entryoffset and entrysize.
///
/// \sa GetHeaderSize, GetCount, GetHeaderType, FindFileByName, FindFileByHash,
/// FindFileByIndex, FindNameByIndex

const char *Big::LocateEntry(const char *paramName,  ///< [IN] name of member file to search for (can be NULL)
                             int entrynum,           ///< [IN] if name is NULL, this parameter specifies the file number to return.  If name is not NULL, this parameter is ignored.
                             uint64_t *entryoffset,  ///< [OUT] pointer to integer which will receive the offset of the requested file
                             uint64_t *entrysize,    ///< [OUT] pointer to integer which will receive the size (in bytes) of the requested file.  Please note that if the file is compressed, the compressed size is returned here.  If the file is not compressed, the uncompressed size is returned.
                             void **tocptr,          ///< [IN/OUT] optional.  Can be used as a hint to LocateEntry to begin searching part way through the index rather than at the beginning.  However, if the entry being searched for is before this entry, it won't be found.
                             int *tocindex,          ///< [IN/OUT] optional.  If the tocptr parameter is not NULL, this must be set to the value returned by the previous search.
                             unsigned int flags      ///< [IN, optional].  Bit flags to control how the path directory separator characters are modified.
                             ) const
{
    // determine where to start Locate - start of file, or from last toc entry
    int i = -1;
    const unsigned char *bigentry = NULL;

    if (tocindex)   i = *tocindex;
    if (tocptr)     bigentry = (unsigned char *) *tocptr;

    if (i<=0 || entrynum<i)    // start at beginning?
    {
        i = 0;
        bigentry = ((const unsigned char *)mBigFileData + sizeof(BigFileHeader));
    }

    int version;
    int infosize = GetDebugInfo(version);

    // Locate end of header.
    const unsigned char *hend = (const unsigned char *)mBigFileData +
        GetHeaderSize(mBigFileData) - infosize;

    int sizeofoffset = 4;
    int sizeoflen = 4;

    // Intentionally using mBigFileData here, rather than mBigFileHeader.
    // The class instance could have both mBigFileData and a separate header,
    // but since the header could be hash16 format, we can't use it here.
    // mBigFileData should have the right format, and if it doesn't, there
    // is no way to return the file name
    bool early_return(false);

    switch (GetHeaderType(mBigFileData))
    {
    case BFTYPE_N:
        sizeofoffset = ((const char *)mBigFileData)[3]-'0'; // 'BIG2'..'BIG7'
        if (sizeofoffset==('F'-'0')) sizeofoffset=4;
        sizeoflen = sizeofoffset; if (sizeoflen>4) sizeoflen = 4;
        break;

    case BFTYPE_24: // legacy
        bigentry = ((const unsigned char *)mBigFileData + sizeof(BigFileHeaderLegacy));
        sizeofoffset = 3;
        sizeoflen = 3;
        break;

    case BFTYPE_FAST:
    case BFTYPE_HASH16:
    case BFTYPE_HASH20:
    case BFTYPE_UNKNOWN:
        early_return = true;
        break;
    }

    if (early_return)
    {
        EA_FAIL_MSG("Use the Big::FindFileBy... functions with hash16 headers");
        if (entryoffset) *entryoffset = 0;
        if (entrysize)   *entrysize = 0;
        return NULL;
    }

    if (paramName)
    {
        int strlen_name = static_cast<int>(Strlen(paramName));

        char convertedSlashBuf[256];
        paramName = ConvertPath(paramName, flags, convertedSlashBuf);

        while ((const unsigned char *)bigentry < hend)
        {
            const char *bigentryname = (const char *)bigentry+sizeofoffset+sizeoflen;
            int strlen_bigentryname = static_cast<int>(Strlen(bigentryname));

            if (strlen_bigentryname==strlen_name)        // prune on length of string for performance
            {
                if (!Stricmp(bigentryname, paramName))
                {
                    if (entryoffset) *entryoffset = static_cast<uint64_t>(EA::Endian::big_get64(bigentry,sizeofoffset));
                    if (entrysize)   *entrysize =   static_cast<uint64_t>(EA::Endian::big_get64(bigentry+sizeofoffset,sizeoflen));
                    if (tocptr)      *tocptr =      const_cast<void *>(static_cast<const void *>(bigentry));
                    if (tocindex)    *tocindex =    i;
                    return(bigentryname);
                }
            }
            bigentry = (const unsigned char *) bigentryname + strlen_bigentryname + 1;
            ++i;
        }
    }
    else // index search
    {
        while ((const unsigned char *)bigentry < hend)
        {
            const char *bigentryname = (const char *)bigentry+sizeofoffset+sizeoflen;
            if (i == entrynum)
            {
                if (entryoffset) *entryoffset = EA::Endian::big_get64(bigentry,sizeofoffset);
                if (entrysize)   *entrysize =   EA::Endian::big_get64(bigentry+sizeofoffset,sizeoflen);
                if (tocptr)      *tocptr =      const_cast<void *>(static_cast<const void *>(bigentry));
                if (tocindex)    *tocindex =    i;
                return(bigentryname);
            }
            bigentry = (const unsigned char *)bigentryname + Strlen(bigentryname) + 1;
            ++i;
        }
    }

    if (entryoffset) *entryoffset = 0;
    if (entrysize)   *entrysize = 0;

    return(NULL);
}

/*! \brief Search for a member file by name

    Calls GetDJB2HashBigEndian to get a hash value of the fileName,
    then calls FindFileByHash with this value.  Refer to that function
    for further details and an explanation of the parameters.

    \param fileName name of file to search for, exactly as it appears
    in the bigfile header.  It is case-sensitive, unlike legacy format
    searches
    \param flags Flags to convert the fileName to use forward or back
    slashes before the hash is computed
    \param entryFlags A pointer to a unsigned int which receives optional
    information about the file entry.
    \return true if the file was found, false if not.
    \sa FindFileByHash, GetDJB2HashBigEndian, FindFileByIndex, FindNameByIndex,
    \importlib rwbigfile
*/
bool Big::FindFileByName(const char *fileName,
                         uint64_t *entryoffset,
                         uint64_t *entrysize,
                         uint64_t *zipSize,
                         int *tocindex,
                         unsigned int flags,
                         unsigned int *entryFlags
                         ) const
{
    EA_ASSERT(fileName != NULL);

    char pathBuffer[256];
    const char *path = ConvertPath(fileName, flags, pathBuffer);

    // If the caller passed in a name, get its hash value
    uint64_t nameHash = GetDJB2HashBigEndian(path);

    return FindFileByHash(nameHash, entryoffset, entrysize, zipSize, tocindex, entryFlags);
}

/*! \brief Search for a member file by hash value

    This function searches for the given subfile in the header.  The
    header must be in BFTYPE_FAST, BFTYPE_HASH16, or BFTYPE_HASH20 format

    \param hashedName 32-bit DJB2 hash of the name to search for, in
    big-endian format.
    \param entryoffset A 64-bit buffer to receive the offset of the
    member file.  May be NULL.  If the file is not found, 0 will be
    filled in.
    \param entrysize A 64-bit buffer to receive the uncompressed size
    of the member file.  May be NULL.  If the file is not found, 0
    will be filled in.
    \param zipSize A 64-bit buffer to receive the compressed size
    of the member file.  May be NULL.  If the file is not found, 0
    will be filled in.
    \param tocindex A pointer to an integer to start the search from.
    May be NULL.  If set and non-zero, the search will start at that
    record and continue until a matching name is found or until the last
    record is checked.  Records prior to the tocindex will NOT be
    checked for a matching name.  If a match is found, tocindex is
    updated with that record index.  If no match is found, tocindex
    is set to -1.  Basically, this parameter can speed up the linear
    search by allowing the search to ignore all records, before
    tocindex.
    \param entryFlags A pointer to a unsigned int which receives the
    per-file flags in fileformat.h.  Only applicable to BFTYPE_FAST-
    format bigfiles.

    \return true if the file was found, false if not.
    \sa FindFileByName, GetDJB2HashBigEndian, FindFileByIndex, FindNameByIndex
    \importlib rwbigfile
*/
bool Big::FindFileByHash(uint64_t hashedName,
                      uint64_t *entryoffset,
                      uint64_t *entrysize,
                      uint64_t *zipSize,
                      int *tocindex,
                      unsigned int *entryFlags
                      ) const
{
    // determine where to start Locate - start of file, or from last toc entry
    int i = tocindex != NULL ? *tocindex : 0;

    const BIGENTRYHASH *toc = NULL;

    unsigned int entryFlag = UINT32_MAX;

    const char * const indexPtr = GetAddressOfHashedIndex();

    if (mBigFileHashedHeader != NULL)
    {
        EA_ASSERT_MSG(tocindex == NULL, "tocindex parameter must be NULL when using FindFileByIndex with an index sorted by hash value");

        size_t numEntries = static_cast<size_t>(GetCount());
        unsigned int indexEntrySize = 16;

        uint16_t flags = EA::Endian::big_endian(mBigFileHashedHeader->flags);

        if ((GetVersion() == 1) || ((flags & BIGFILE_FORMAT_FLAG_SORTORDER_HASH) != 0))
        {
            // Version 1 of the format only supports indexes sorted by hash value
            if ((flags & BIGFILE_FORMAT_FLAG_64BITHASH) == 0)
            {
                toc = reinterpret_cast<const BIGENTRYHASH *>(bsearch(&hashedName,
                    indexPtr, numEntries, indexEntrySize, Hash16Cmp));
            }
            else
            {
                indexEntrySize = 20;
                toc = reinterpret_cast<const BIGENTRYHASH *>(bsearch(&hashedName,
                    indexPtr, numEntries, indexEntrySize, Hash20Cmp));
            }
        }
        else
        {
            // Index sorted alphbetically or by offset.  Go through the index 

            int numEntries = GetCount();

            if ((flags & BIGFILE_FORMAT_FLAG_64BITHASH) == 0)
            {
                const BIGENTRYHASH16 *toc16 = reinterpret_cast<const BIGENTRYHASH16 *>(indexPtr);

                // Mask off the upper 32 bits
                hashedName &= 0xffffffff;

                while (i < numEntries)
                {
                    if (hashedName == toc16[i].GetNameHash())
                    {
                        // found it
                        toc = reinterpret_cast<const BIGENTRYHASH *>(toc16 + i);
                        break;
                    }
                    ++i;
                }
            }
            else    // hash20
            {
                const BIGENTRYHASH20 *toc20 = reinterpret_cast<const BIGENTRYHASH20 *>(indexPtr);

                while (i < numEntries)
                {
                    if (hashedName == toc20[i].GetNameHash())
                    {
                        // found it
                        toc = reinterpret_cast<const BIGENTRYHASH *>(toc20 + i);
                        break;
                    }
                    ++i;
                }
            }
        }

        if (toc != NULL)
        {
            // found the entry in the toc, get it's flags value
            unsigned int indexSize = AlignUp(indexEntrySize * GetCount(), 16);

            const char *entryFlagIndexBase = indexPtr + indexSize;

            size_t entryNum =
                static_cast<size_t>((reinterpret_cast<const char *>(toc) - indexPtr)) / indexEntrySize;

            entryFlag = static_cast<unsigned char>(entryFlagIndexBase[entryNum]);
        }
    }
    else
    {
        EA_ASSERT(mBigFileHeader != NULL);

        int numEntries = GetCount();
        BigfileType headerType = GetHeaderType(mBigFileHeader);
        EA_ASSERT(headerType == BFTYPE_HASH16 || headerType == BFTYPE_HASH20);

        if (headerType == BFTYPE_HASH16)
        {
            const BIGENTRYHASH16 *toc16 = reinterpret_cast<const BIGENTRYHASH16 *>(
                mBigFileHeader + 1);

            hashedName &= 0xffffffff;

            while (i < numEntries)
            {
                if (hashedName == toc16[i].GetNameHash())
                {
                    // found it
                    toc = reinterpret_cast<const BIGENTRYHASH *>(toc16 + i);
                    break;
                }
                ++i;
            }
        }
        else    // hash20
        {
            const BIGENTRYHASH20 *toc20 = reinterpret_cast<const BIGENTRYHASH20 *>(
                mBigFileHeader + 1);
            while (i < numEntries)
            {
                if (hashedName == toc20[i].GetNameHash())
                {
                    // found it
                    toc = reinterpret_cast<const BIGENTRYHASH *>(toc20 + i);
                    break;
                }
                ++i;
            }
        }
    }

    if (toc != NULL)
    {
        if (entryoffset != NULL)
        {
            *entryoffset = toc->GetOffset();
            if (mBigFileHashedHeader != NULL)
            {
                // bigfile (not bighead) must shift the offset by the alignment boundary.
                *entryoffset <<= mBigFileHashedHeader->alignment;
            }
        }

        if (entrysize != NULL)
        {
            // toc->size is 0 for uncompressed files.
            // When that is the case, use zSize instead.
            *entrysize = toc->GetSize() != 0 ? toc->GetSize() : toc->GetzSize();
        }

        if (zipSize != NULL)
        {
            *zipSize = toc->GetzSize();
        }

        if (tocindex != NULL)
        {
            *tocindex = i;
        }

        if (entryFlags != NULL)
        {
            if (entryFlag == UINT32_MAX)
            {
                // we couldn't tell what kind of compression is used, which means it's
                // a BFTYPE_HASH16 or BFTYPE_HASH20.  Therefore, if it's compressed, 
                // it'll be with refpack.
                entryFlag = BIGFILE_FORMAT_FLAG_COMPRESSION_TYPE_NONE;

                if (toc->GetSize() != 0)
                {
                    entryFlag = BIGFILE_FORMAT_FLAG_COMPRESSION_TYPE_REFPACK;
                }
            }

            *entryFlags = entryFlag;
        }
    }
    else
    {
        // not found
        if (entryoffset) *entryoffset = 0;
        if (entrysize)   *entrysize = 0;
        if (zipSize)     *zipSize = 0;
        if (tocindex)    *tocindex = -1;
        if (entryFlags)  *entryFlags = entryFlag;
    }

    return toc != NULL;
}

/*! \brief Return information about a given bigfile

    This function returns information about the ith bigfile member.  It can
    only be used with bigfiles in the BFTYPE_FAST, BFTYPE_HASH16, and
    BFTYPE_HASH20 formats.  Looksup are O(1).  Refer to FindFileByHash for a
    description of the output parameters.

    \sa FindFileByName, FindFileByHash, FindNameByIndex
    \importlib rwbigfile
*/
void Big::FindFileByIndex(int entryNum, uint64_t *entryoffset, uint64_t *entrysize, uint64_t *zipSize, unsigned int *entryFlags) const
{
    EA_ASSERT(entryNum < GetCount());
    const BIGENTRYHASH *toc = NULL;

    const char * const indexPtr = GetAddressOfHashedIndex();

    if (mBigFileHashedHeader != NULL)
    {

        if ((EA::Endian::big_endian(mBigFileHashedHeader->flags) & BIGFILE_FORMAT_FLAG_64BITHASH) == 0)
        {
            toc = reinterpret_cast<const BIGENTRYHASH *>(
                reinterpret_cast<const BIGENTRYHASH16 *>(indexPtr) + entryNum);
        }
        else    // hash20
        {
            toc = reinterpret_cast<const BIGENTRYHASH *>(
                reinterpret_cast<const BIGENTRYHASH20 *>(indexPtr) + entryNum);
        }
    }
    else
    {
        EA_ASSERT(mBigFileHeader != NULL);

        BigfileType headerType = GetHeaderType(mBigFileHeader);
        EA_ASSERT(headerType == BFTYPE_HASH16 || headerType == BFTYPE_HASH20);

        if (headerType == BFTYPE_HASH16)
        {
            const BIGENTRYHASH16 *toc16 = reinterpret_cast<const BIGENTRYHASH16 *>(
                mBigFileHeader + 1) + entryNum;

            toc = reinterpret_cast<const BIGENTRYHASH *>(toc16);
        }
        else    // hash20
        {
            const BIGENTRYHASH20 *toc20 = reinterpret_cast<const BIGENTRYHASH20 *>(
                mBigFileHeader + 1) + entryNum;

            toc = reinterpret_cast<const BIGENTRYHASH *>(toc20);
        }
    }

    if (entryoffset != NULL)
    {
        *entryoffset = toc->GetOffset();
        if (mBigFileHashedHeader != NULL)
        {
            // bigfile (not bighead) must shift the offset by the alignment boundary.
            *entryoffset <<= mBigFileHashedHeader->alignment;
        }
    }

    if (entrysize != NULL)
    {
        // toc->size is 0 for uncompressed files.
        // When that is the case, use zSize instead.
        *entrysize = toc->GetSize() != 0 ? toc->GetSize() : toc->GetzSize();
    }

    if (zipSize != NULL)
    {
        *zipSize = toc->GetzSize();
    }

    if (entryFlags != NULL)
    {
        if (mBigFileHashedHeader != NULL)
        {
            int indexEntrySize = 16;

            if ((EA::Endian::big_endian(mBigFileHashedHeader->flags) & BIGFILE_FORMAT_FLAG_64BITHASH) != 0)
            {
                indexEntrySize = 20;
            }

            int indexSize = AlignUp(indexEntrySize * GetCount(), 16);

            const char *entryFlagIndex = indexPtr + indexSize;

            *entryFlags = static_cast<unsigned char>(entryFlagIndex[entryNum]);
        }
        else
        {
            // bighead doesn't support entry flags, use UINT32_MAX to
            // indicate that (because 0 means no flags are set)
            *entryFlags = UINT32_MAX;
        }
    }
}

/*! \brief Return the name of the subfile at the given index.

    This function returns the name of the member file at the ith entry.
    If the bigfile does not have names, NULL is returned.  This function can
    only be used with the BFTYPE_FAST header type, and a name index must be
    set first with SetNameIndex.  Lookups are O(1).

    \param entryNum The index number of the entry to return.
    \param pathBuffer Pass a buffer to receive the path nd name of the subfile.
    \param pathBufSize Size in bytes of the path buffer.

    \return a pointer to the pathBuffer variable.  If entryNum is out of range,
    NULL is returned.

    \sa SetNameIndex, FindFileByName, FindFileByHash

    \importlib rwbigfile
*/
char *Big::FindNameByIndex(int entryNum, char *pathBuffer, size_t pathBufSize) const
{
    EA_ASSERT_MSG(mBigFileHashedHeader != NULL,
        "This function can only be used with new-format bigfiles.  Use LocateEntry for legacy format bigfiles.");
    EA_ASSERT_MSG(mBigFileNameIndex != NULL,
        "FindNameByIndex requires that a name index be loaded.  Set one with SetNameIndex()");

    if (entryNum >= GetCount())
    {
        // Index value out of range.
        return NULL;
    }

    const char *nameEntry = static_cast<const char *>(mBigFileNameIndex) +
        entryNum * mBigFileHashedHeader->nameLength;

    uint16_t pathIndexEntry = static_cast<uint16_t>(EA::Endian::big_get(nameEntry, 2));
    nameEntry += 2;

    const char *pathIndexBase = static_cast<const char *>(mBigFileNameIndex) +
        AlignUp(GetCount() * mBigFileHashedHeader->nameLength, 16);
    const char *pathEntry = pathIndexBase + pathIndexEntry * mBigFileHashedHeader->pathLength;

    if (*pathEntry != '.')
    {
        Snprintf(pathBuffer, pathBufSize, "%s/%s", pathEntry, nameEntry);
    }
    else
    {
        Strncpy(pathBuffer, nameEntry, pathBufSize);
    }

    return pathBuffer;
}

int Big::Hash16Cmp(const void *pkey, const void *elem)
{
    uint32_t key = static_cast<uint32_t>(*static_cast<const uint64_t *>(pkey));
    uint32_t hash = static_cast<const BIGENTRYHASH16 *>(elem)->GetNameHash();

    if (key > hash)
        return 1;
    else if (key < hash)
        return -1;

    // they're equal so a match has been found.
    return 0;
}

int Big::Hash20Cmp(const void *pkey, const void *elem)
{
    uint64_t key = *static_cast<const uint64_t *>(pkey);
    uint64_t hash = static_cast<const BIGENTRYHASH20 *>(elem)->GetNameHash();

    if (key > hash)
        return 1;
    else if (key < hash)
        return -1;

    // they're equal so a match has been found.
    return 0;
}

#ifdef EA_DEBUG
uint32_t Big::GetHashValueByIndex(int i) const
{
    if (mBigFileHashedHeader != NULL)
    {
        const char * const indexPtr = GetAddressOfHashedIndex();

        EA_ASSERT(mBigFileHeader == NULL);
        EA_ASSERT(i < GetCount());

        return (reinterpret_cast<const BIGENTRYHASH16 *>(indexPtr) + i)->GetNameHash();
    }
    else
    {
        EA_ASSERT(mBigFileHeader != NULL);
        EA_ASSERT(GetHeaderType(mBigFileHeader) == BFTYPE_HASH16);
        EA_ASSERT(i < GetCount());
    }

    return (reinterpret_cast<const BIGENTRYHASH16 *>(mBigFileHeader + 1) + i)->GetNameHash();
}
#endif

/// \brief Get the number of files in a bigfile.
///
/// \importlib rwbigfile
///
/// Returns the number of files contained in a big file.  If the pointer
/// does not point to a recognized bigfile, BFTYPE_UNKNOWN will be returned.
///
/// \return Number of files lumped together in the bigfile.  0 is returned
/// if this isn't a valid bigfile.
///
/// \sa LocateEntry, GetHeaderSize
int Big::GetCount() const
{
    unsigned int count=0;

    if (mBigFileHashedHeader != NULL)
    {
        return static_cast<int>(EA::Endian::big_endian(mBigFileHashedHeader->fileCount));
    }

    // Legacy format
    switch (GetHeaderType(mBigFileHeader))
    {
        case BFTYPE_N:
        case BFTYPE_HASH16:
        case BFTYPE_HASH20:
            count = EA::Endian::big_get(&static_cast<const BigFileHeader *>(mBigFileData)->num, 4);
            break;

        case BFTYPE_24:
            count = EA::Endian::big_get(&reinterpret_cast<const BigFileHeaderLegacy *>(mBigFileData)->num, 2);
            break;

        case BFTYPE_UNKNOWN:
        default:
            break;
    }

    return static_cast<int>(count);
}

/// \brief Returns the type of bigfile that this is.
///
/// \importlib rwbigfile
///
/// \return The type of the bigfile header.  BFTYPE_FAST, BFTYPE_HASH16is returned. 
/// If this is a valid 24-bit (legacy style) header, BFTYPE_24 is returned.
/// In all other cases, BFTYPE_N is returned.
/// \sa GetHeaderSize, GetCount
Big::BigfileType Big::GetHeaderType(const void *bigFilePtr)
{
    BigfileType type = BFTYPE_UNKNOWN;

    if(bigFilePtr == NULL)
    {
        return type;
    }

    const BigFileFormatHeader *pBig =
        static_cast<const BigFileFormatHeader *>(bigFilePtr);

    if (EA::Endian::big_endian(pBig->signature) == BIGFILE_HEADER_SIGNATURE)
    {
#if defined(EA_ASSERT_ENABLED)
        uint16_t headerVersion = EA::Endian::big_endian(pBig->headerVersion);

        // Assert that we can handle this version number
        EA_ASSERT_FORMATTED(headerVersion <= BIGFILE_HEADER_VERSION, 
            ("This bigfile header has version number %d, which is higher than this package can read (%d).  You will need to get the current release of rwbigfile to parse this bigfile",
            headerVersion, BIGFILE_HEADER_VERSION));
#endif

        return BFTYPE_FAST;
    }

    uint32_t legacyHeaderSig = EA::Endian::big_get(bigFilePtr, 4);

    // This library does not support hash8 or hash12 formats at
    // this time.  Support for these formats will be added upon
    // request by a game team.
    if (legacyHeaderSig == BIGHEAD_HASH16ID)
    {
        type = BFTYPE_HASH16;
    }
    else if (legacyHeaderSig == BIGHEAD_HASH20ID)
    {
        type = BFTYPE_HASH20;
    }
    else if ((legacyHeaderSig&0xffffff00) == (BIGNID&0xffffff00))
    {
        // new BIG*
        type = BFTYPE_N;
    }
    else if (EA::Endian::big_get(bigFilePtr, 2) == BIG24ID)
    {
        type = BFTYPE_24;
    }

    return type;
}

/// \brief Get the file format version.
///
/// \importlib rwbigfile
///
/// The GetVersion function gets the file format version number from the
/// header.  If this version is higher than what the library was built with,
/// it's likely that an updated runtime will be required to handle the
/// updated format.
///
/// If this is a legacy format bigfile, it the version of glump that was used
/// to create the bigfile is returned.  This version number does not imply
/// a change to the format, however.
///
/// \return The version of the big file creator as an integer.  0 if
/// version information is not attached.
int Big::GetVersion() const
{
    if (GetHeaderType(mBigFileData) == BFTYPE_FAST)
    {
        return EA::Endian::big_endian(mBigFileHashedHeader->headerVersion);
    }

    int infovenom(0);
    Big::GetDebugInfo(infovenom);
    return infovenom;
}

/// \brief Return a pointer to the given file
///
/// \importlib rwbigfile
///
/// Legacy function supporting BFTYPE_N and BFTYPE_24 formats.
///
/// Returns a pointer to a file in a bigfile by getting an offset 
/// from FindFileByIndex or LocateEntry, and adding the offset to
/// the mBigFileData.
///
/// \param number index of member to get pointer to
/// \return A pointer to a file is returned if the file is found in the
/// bigfile.  If the file is not found, the return value is NULL.
///
/// \sa FindFileByIndex, LocateEntry
void *Big::GetPointer(int number) const
{
    uint64_t bigoffset = 0;

    uint32_t headerType;
    if (mBigFileHashedHeader != NULL)
    {
        headerType = GetHeaderType(mBigFileHashedHeader);
    }
    else
    {
        headerType = GetHeaderType(mBigFileHeader);
    }

    if (headerType == BFTYPE_HASH16 || headerType == BFTYPE_HASH20 || headerType == BFTYPE_FAST)
    {
        FindFileByIndex(number, &bigoffset);
    }
    else
    {
        LocateEntry(NULL, number, &bigoffset, NULL, NULL, NULL);
    }

    if (bigoffset != 0)
    {
        return const_cast<char *>(reinterpret_cast<const char *>(mBigFileData)+bigoffset);
    }

    return NULL;
}

/// \brief Locate big and return pointer from a name
///
/// \importlib rwbigfile
///
/// Legacy function supporting BFTYPE_N and BFTYPE_24 formats.
/// Does not work with BFTYPE_HASH16, BFTYPE_HASH20, or BFTYPE_FAST
/// formats.
///
/// Returns a pointer to the requested file.
///
/// \param name name of subfile to search for
/// \return A pointer to the file is returned if the file is found
/// in the bigfile.  If the file was not found, NULL is returned.
///
/// \sa FindFileByName, LocateEntry, GetPointer
void *Big::Locate(const char *name) const
{
    uint64_t bigoffset = 0;
    uint32_t headerType;

    if (mBigFileHashedHeader)
        headerType = GetHeaderType(mBigFileHashedHeader);
    else
        headerType = GetHeaderType(mBigFileHeader);

    if (headerType == BFTYPE_FAST ||
        headerType == BFTYPE_HASH16 || headerType == BFTYPE_HASH20)
    {
        FindFileByName(name, &bigoffset);
    }
    else
    {
        LocateEntry(name, 0, &bigoffset, NULL, NULL, NULL);
    }

    if (bigoffset != 0)
    {
        return const_cast<char *>(reinterpret_cast<const char *>(mBigFileData) + bigoffset);
    }

    return NULL;
}

/// \brief Get name of ith file in a bigfile.
///
/// \importlib rwbigfile
///
/// Legacy function supporting BFTYPE_N and BFTYPE_24 formats.
/// Does not work with BFTYPE_HASH16, BFTYPE_HASH20, or BFTYPE_FAST
/// formats.
/// 
/// Uses LocateEntry to find the name from the index.
///
/// \param index index of member file to get name of
/// \param name buffer to copy name into.  It is the caller's
/// responsibility to ensure this buffer is large enough to hold
/// the complete name.  If not, a buffer overrun will occur.
/// \param flags This parameter is passed straight to LocateEntry.
/// Refer to that function for a description of the options.
/// \return pointer to the supplied string buffer.
///
/// \sa LocateEntry
char *Big::GetName(int index, char *name, unsigned int flags) const
{
    const char *filenameptr = LocateEntry(NULL, index, NULL, NULL, NULL, NULL, flags);

    if (filenameptr)
    {
        Strcpy(name, filenameptr);
        return(name);
    }
    else
    {
        name[0] = '\0';
        return(NULL);
    }
}

/// \brief Returns the pointer to the beginning of the bigfile data in memory.
const void *Big::GetBaseAddress() const
{
    return mBigFileData;
}

const int LUMPY_DEBUGINFO = 8;

bool inline isalpha(char c)
{ return ((c>='A') && (c<='Z')) || ((c>='a') && (c<='z')); }

bool inline isdigit(char c)
{ return (c>='0') && (c<='9'); }

// returns number of debug info bytes (0 or 8)

// done as a macro incase we want to ifdef it
int Big::GetDebugInfo(int &version) const
{
    int infosize;
    const char *venomptr=(const char *)mBigFileData+GetHeaderSize(mBigFileData)-LUMPY_DEBUGINFO;

    if (isalpha(venomptr[0]) && isdigit(venomptr[1]) && isdigit(venomptr[2]) && isdigit(venomptr[3]))
    {
        infosize=LUMPY_DEBUGINFO;
        version = (venomptr[1]-'0')*100 + (venomptr[2]-'0')*10 + (venomptr[3]-'0');
    }
    else
    {
        infosize = 0;
        version = 0;
    }

    return infosize;
}

const char *Big::ConvertPath(const char *path, unsigned int flags, char *modifiedPathBuffer) const
{
    if (flags == 0)
        return path;

    EA_ASSERT(!((flags & LOCATE_OPT_SLASH_CONVERT_FORWARD) && (flags & LOCATE_OPT_SLASH_CONVERT_BACK)));
    EA_ASSERT(!((flags & LOCATE_OPT_NAME_CONVERT_LOWERCASE) && (flags & LOCATE_OPT_NAME_CONVERT_UPPERCASE)));

    char cFind, cReplace;

    if ((flags & LOCATE_OPT_SLASH_CONVERT_FORWARD) != 0)
    {
        cFind = '\\';
        cReplace = '/';
    }
    else
    {
        cFind = '/';
        cReplace = '\\';
    }

    char *dst = modifiedPathBuffer;

    while ((*dst = *path++) != '\0')
    {
        if (*dst == cFind)
        {
            *dst = cReplace;
        }
        else
        {
            if ((flags & LOCATE_OPT_NAME_CONVERT_LOWERCASE) != 0)
            {
                *dst = static_cast<char>(tolower(*dst));
            }
            else if ((flags & LOCATE_OPT_NAME_CONVERT_UPPERCASE) != 0)
            {
                *dst = static_cast<char>(toupper(*dst));
            }
        }

        ++dst;
    }

    return modifiedPathBuffer;
}

const char *Big::GetAddressOfHashedIndex() const
{
    if (GetVersion() >= 3)
    {
        const char * const hashedHeaderAsBytePtr = reinterpret_cast<const char *>(mBigFileHashedHeader);
        size_t headerSize = ((sizeof *mBigFileHashedHeader) + 15) & ~(static_cast<size_t>(15));
        return hashedHeaderAsBytePtr + headerSize;
    }
    else
    {
        size_t headerSize = sizeof(BigFileFormatHeaderV2);
        return reinterpret_cast<const char *>(mBigFileHashedHeader) + headerSize;
    }
}



}
}
