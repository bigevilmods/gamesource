// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_CORE_CODEC_DETAIL_CHUNKPACKHEADER_H
#define RW_CORE_CODEC_DETAIL_CHUNKPACKHEADER_H

#include "EABase/eabase.h"
#include "rw/core/codec/chunkcompressiontypes.h"

namespace rw {
namespace core {
namespace codec {
namespace detail {

struct ChunkPackHeader
{
    static const uint32_t CHUNK_FILE_VERSION_NUMBER = 2;

    char ID[8];
    uint32_t VersionNum;
    uint32_t UncompressedLength;
    uint32_t BlockSize;
    uint32_t NumSegments;
    uint32_t Alignment;

    CHUNK_COMPRESSION_TYPE GetType() const;
    void SetType(CHUNK_COMPRESSION_TYPE compressionType);
};

}}}}

#endif

