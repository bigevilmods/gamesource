// (c) Electronic Arts. All Rights Reserved.

/**
\file Defines some file format information for chunked files.

Here is the file structure for the Chunk files:<br>
-FileHeader<br>
-Alignment Gap Data<br>
-Chunk1 Info("mini-header")<br>
-Chunk1 RawData<br>
-Alignment Gap Data<br>
-Chunk2 Info("mini-header")<br>
-Chunk2 RawData<br>
...<br>
-ChunkN Info("mini-header")<br>
-ChunkN RawData<br>
*/

#ifndef DETAIL_PACKINFO_H
#define DETAIL_PACKINFO_H

#include <EABase/eabase.h>

#include "rw/core/codec/chunkcompressiontypes.h"

//For memcmp
#include <string.h>

#include <EAAssert/eaassert.h>

namespace rw {
namespace core {
namespace codec {
namespace detail {

extern const char *gChunkZip;
extern const char *gChunkRefpack;
extern const char *gChunkLZX;
extern const char *gChunkLzma;

// NOTE: All of these are stored in Big Endian
struct PackInfo
{
    uint32_t CompressedDataSize; // Size of raw data
    CHUNK_COMPRESSION_TYPE CompressionType;  // Compression type used
                                             // Should really only be the one the file uses
                                             // or CHUNK_COMPRESSION_TYPE_UNCOMPRESSED
};

EA_COMPILETIME_ASSERT(sizeof(PackInfo) == 2 * sizeof(uint32_t));

}}}}

#endif

