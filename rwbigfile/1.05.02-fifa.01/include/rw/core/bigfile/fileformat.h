// (c) Electronic Arts. All Rights Reserved.

#if !defined(RW_CORE_BIGFILE_FORMAT_H)
#define RW_CORE_BIGFILE_FORMAT_H

#if defined _MSC_VER
    #pragma once
#endif

#include <EABase/EABase.h>

namespace rw {
namespace core {

#ifndef MAKEID
#define MAKEID(a,b,c,d) (((static_cast<int>(a))<<24)|((static_cast<int>(b))<<16)|((static_cast<int>(c))<<8)|static_cast<int>(d))
#endif

// Bigfile header signature
#define BIGFILE_HEADER_SIGNATURE MAKEID(0, 0, 'E', 'B')

// Bigfile file format version number (incremented any time the file format
// is altered for any reason)
#define BIGFILE_HEADER_VERSION 0x0003

// for bigfiles that require an 8-byte name hash to avoid collisions.
const int BIGFILE_FORMAT_FLAG_64BITHASH =   0x0001;

const unsigned int BIGFILE_FAT_DIRECTORY = 0;
const unsigned int BIGFILE_FAT_FILE = 1;

// index sort order
const unsigned int BIGFILE_FORMAT_FLAG_SORTORDER_HASH =             0x0010;
const unsigned int BIGFILE_FORMAT_FLAG_SORTORDER_ALPHABETICAL =     0x0020;
const unsigned int BIGFILE_FORMAT_FLAG_INCLUDES_FAT =               0x0040;
const unsigned int BIGFILE_FORMAT_FLAG_SORTORDER_OFFSET =           0x0080;

// flags values
const unsigned char BIGFILE_FORMAT_FLAG_COMPRESSION_TYPE_NONE = 0;
const unsigned char BIGFILE_FORMAT_FLAG_COMPRESSION_TYPE_REFPACK = 1;
const unsigned char BIGFILE_FORMAT_FLAG_COMPRESSION_TYPE_REFPACK_CHUNKED = 2;
const unsigned char BIGFILE_FORMAT_FLAG_COMPRESSION_TYPE_ZLIB_CHUNKED = 3;
const unsigned char BIGFILE_FORMAT_FLAG_COMPRESSION_TYPE_LZX = 4;

/*
// Other things to consider for the file header
// option for whether in-place decompression is supported
// file alignment field

// not currently supported by either the tool or the runtime
// const unsigned int BIGFILE_FORMAT_FLAG_SORTORDER_NAME =     0x0400;
// const unsigned int BIGFILE_FORMAT_FLAG_SORTORDER_SIZE =     0x0800;
*/

// Header format.  All fields are stored in big-endian format.  This structure
// should be allocated on an 8-byte boundary because there is a 64-bit field
// in it.
struct BigFileFormatHeaderV2
{
    uint16_t signature;         // "EB" (Electronic Arts Bigfile)
    uint16_t headerVersion;     // version number
    uint32_t fileCount;         // number of files in this archive
    uint16_t flags;             // bitflags
    uint8_t alignment;          // power of two on which the files are aligned (default = 4, 16 bytes)
    uint8_t _reserved0;         // not used
    uint32_t baseHeaderSize;    // file header + hash index
    uint32_t nameHeaderSize;    // size of the names & paths, in bytes
    uint8_t nameLength;         // length of each name entry
    uint8_t pathLength;         // length of each path entry
    uint16_t numPaths;          // total number of unique paths
    uint64_t fileSize;          // total size of the bigfile
};

struct BigFileFormatHeader
{
    uint16_t signature;         // "EB" (Electronic Arts Bigfile)
    uint16_t headerVersion;     // version number
    uint32_t fileCount;         // number of files in this archive
    uint16_t flags;             // bitflags
    uint8_t alignment;          // power of two on which the files are aligned (default = 4, 16 bytes)
    uint8_t _reserved0;         // not used
    uint32_t baseHeaderSize;    // file header + hash index
    uint32_t nameHeaderSize;    // size of the names & paths, in bytes
    uint8_t nameLength;         // length of each name entry
    uint8_t pathLength;         // length of each path entry
    uint16_t numPaths;          // total number of unique paths
    uint64_t fileSize;          // total size of the bigfile
    uint32_t fatSize;           // total size of the bigfile tree structure

    // following this header is the index, each entry being 16 or 20 bytes:
    // It looks like this:
    //  uint32_t offset;    // offset divided by the alignment to the start of the file
    //  uint32_t zSize;     // compressed size
    //  uint32_t size;      // uncompressed size
    //  uint32_t hash;      // 32/64-bit DJB2 hash of the original name

    // following the index is a flags index, each entry being 8-bits.  As of
    // version 1, these flags indicate what kind of compression was used when
    // the bigfile was built.  This index is padded so its size is a multiple
    // of 16.

    // following that index is the list of NULL-terminated file names, sorted
    // in the same order as the index.  They are always present in the bigfile,
    // for debug purposes or in case the files need to be extracted later on,
    // but the runtime code will usually ignore them.

    // following the names index is the paths index (each name has a path
    // associated with it), and more than one name can use the same path.
};


#ifdef EA_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 4201)
#endif

// If the bigfile has the FAT included, there will be three components
// following the path list before the data. 
//
// NOTE: Each section is prepended with the byte size of the section 
// represented by a 4-byte unsigned integer. 
//
// NOTE: All values are *big endian*.
//
// The first is the array of directory entries. These entries represent
// items within our bigfile file system, either directories or files. 
// Each item has an index into a string table that contains its name.
// This is different than the other storage of the file names or path
// names because the other file names are prepended with the path index
// and the path names are not split. Since the FAT is an optional 
// convenience it isn't meant to be the most space optimal format possible.
//
// Also stored in the directory entry are a flags field, indicating whether
// or not the entry is a file or a directory. The containing node index
// is a back-reference to the directory entry that holds the current item.
//
// For files, the file index field holds the index into the, well, main 
// bigfile index for the file information for ease of looking up the actual
// file details.
//
// For directories we store the number of items that live within this directory
// and an index into the master constituent list where we can look up the
// directory entry indices of the contained elements. 
//
// Following these entries is the array of constituents. These are the indices
// that describe what entries constitute a directory.
//
// After the constituent table is the string table. This is a list of unique
// names for files and directories that is indexed in by the directory entry
// elements.

struct BigFileDirectoryEntry
{
    uint32_t NameIndex;
    uint32_t EntryType;
    uint32_t ContainingNodeIndex;
    union
    {
        struct
        {
            uint32_t FileIndex;
            uint32_t Unused;
        };
        struct
        {
            uint32_t NumConstituents;
            uint32_t ConstituentStartIndex;
        };
    };
};

#ifdef EA_COMPILER_MSVC
#pragma warning(pop)
#endif

}}      // rw::core namespace

#endif  // include guard
