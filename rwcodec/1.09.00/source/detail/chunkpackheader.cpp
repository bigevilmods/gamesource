// (c) Electronic Arts. All Rights Reserved.

#include <string.h>

#include "EAAssert/eaassert.h"
#include "EABase/eabase.h"
#include "rw/core/codec/detail/chunkpackheader.h"

namespace rw {
namespace core {
namespace codec {
namespace detail {

static const char *gChunkZip = "chunkzip";
static const char *gChunkRefpack = "chunkref";
static const char *gChunkLZX = "chunklzx";
static const char *gChunkLzma = "chunlzma";
static const char *gChunkLz4 = "chunklz4";
static const char *gChunkUncompressed = "chunkunc";

CHUNK_COMPRESSION_TYPE ChunkPackHeader::GetType() const
{
    if (memcmp(ID, gChunkUncompressed, 8) == 0)
    {
        return CHUNK_COMPRESSION_TYPE_UNCOMPRESSED;
    }
    if (memcmp(ID, gChunkZip, 8) == 0)
    {
        return CHUNK_COMPRESSION_TYPE_ZLIB;
    }
    else if (memcmp(ID, gChunkRefpack, 8) == 0)
    {
        return CHUNK_COMPRESSION_TYPE_REFPACK;
    }
    else if (memcmp(ID, gChunkLZX, 8) == 0)
    {
        return CHUNK_COMPRESSION_TYPE_LZX;
    }
    else if (memcmp(ID, gChunkLzma, 8) == 0)
    {
        return CHUNK_COMPRESSION_TYPE_LZMA;
    }
    else if (memcmp(ID, gChunkLz4, 8) == 0)
    {
        return CHUNK_COMPRESSION_TYPE_LZ4;
    }
    else
    {
        return CHUNK_COMPRESSION_TYPE_UNKNOWN;
    }
}

void ChunkPackHeader::SetType(CHUNK_COMPRESSION_TYPE compressionType)
{
    switch (compressionType)
    {
        case CHUNK_COMPRESSION_TYPE_ZLIB:
            memcpy(ID, gChunkZip, 8);
            break;
        case CHUNK_COMPRESSION_TYPE_REFPACK:
            memcpy(ID, gChunkRefpack, 8);
            break;
        case CHUNK_COMPRESSION_TYPE_LZX:
            memcpy(ID, gChunkLZX, 8);
            break;
        case CHUNK_COMPRESSION_TYPE_LZMA:
            memcpy(ID, gChunkLzma, 8);
            break;
        case CHUNK_COMPRESSION_TYPE_LZ4:
            memcpy(ID, gChunkLz4, 8);
            break;
        case CHUNK_COMPRESSION_TYPE_UNCOMPRESSED:
            memcpy(ID, gChunkUncompressed, 8);
            break;
        default:
            EA_FAIL_MSG("Unknown chunk compression type");
            break;
    }
}

}}}}

