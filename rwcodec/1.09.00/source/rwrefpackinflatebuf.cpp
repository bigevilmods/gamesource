// (c) Electronic Arts. All Rights Reserved.

/**
\file
Implementation of rw::core::codec::RefpackBuf.
*/

#include "rw/core/codec/icodec.h"
#include "rw/core/codec/refpack.h"
#include "jobs/refpack/refpack.h"

namespace rw {
namespace core {
namespace codec {

unsigned int RefpackBuf::Decompress(void *outputBuf, const void *inputBuf)
{
    return Refpack::Inflate(outputBuf, inputBuf);
}

unsigned int RefpackBuf::GetDecompressedSize(const void *inputBuf)
{
    return Refpack::GetDecompressedSize(inputBuf);
}

}}}

