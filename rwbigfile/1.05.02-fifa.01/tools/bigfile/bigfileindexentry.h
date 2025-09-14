// (c) Electronic Arts. All Rights Reserved.

#ifndef BIGFILE_BIGFILE_H
#define BIGFILE_BIGFILE_H

#include "EABase/eabase.h"

struct BigFileIndexEntry
{
    BigFileIndexEntry() {}  // default constructor required for eastl list
    BigFileIndexEntry(const char *sourcePath, const char *destinationPath);

    uint64_t CalculateHash(const char *string, uint64_t hash = 5381);

    char srcpath[256];  // path to the file
    char savepath[256]; // path written into the bigfile index

    char name[256];     // file name only (no path). This has to be 256-ish
                        // bytes because we assert below that the name 
                        // cannot be greater than 254. However, its previous
                        // value of 128 is far too small.
    int nameLen;        // length of the name

    char path[256];     // file path only (no name)
    int pathLen;        // length of the path

    uint16_t pathIndex; // pointer to path in separated index
    uint32_t fileSize;  // uncompressed file size
    uint32_t zSize;     // compressed file size (0 if file is uncompressed)
    uint64_t hash;      // 64-bit DJB2 hash of the file name
    uint64_t offset;    // offset from beginning of the file
    unsigned char flags;// flags & options
    uint32_t index;	

    uint32_t mergeBigfileIndex; // If the current operation is merge, this field points to which bigfile this file resides in.
};


#endif
