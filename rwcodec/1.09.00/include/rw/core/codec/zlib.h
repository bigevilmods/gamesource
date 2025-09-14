// (c) Electronic Arts. All Rights Reserved.

/// \file
/// \brief        Codec class definition.

#ifndef EA_CORE_CODEC_ZLIB_H
#define EA_CORE_CODEC_ZLIB_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "rw/core/codec/icodec.h"

// forward declare zlib stream structure
struct z_stream_s;

namespace rw
{
namespace core
{
namespace codec
{

/// ZlibDeflate implements a compressor for the zlib algorithm.
/// The level parameter can be any value from 0-9 inclusive.
class RWCODEC_API ZlibDeflate : public ICodec
{
public:
    ZlibDeflate(EA::Allocator::ICoreAllocator *allocator, unsigned int level);
    virtual ~ZlibDeflate();

    virtual void    Init(unsigned int level);
    virtual void    End();
    virtual Status  Exec(unsigned int flags);

private:

    static void *Zalloc(void *opaque, unsigned int items, unsigned int size);
    static void Zfree(void *opaque, void *address);

    z_stream_s *mStream;
};

/// ZlibInflate implements a decompressor for zlib-compressed
/// streams.  The Init level parameter is ignored in this codec.
/// Approximately 45K of temporary buffers are required to
/// decompress a stream, regardless of the size of the stream being
/// decompressed.  This codec will also decompress at about 20% of
/// the speed of refpack.
class RWCODEC_API ZlibInflate : public ICodec
{
public:
    explicit ZlibInflate(EA::Allocator::ICoreAllocator *allocator);
    virtual ~ZlibInflate();

    virtual void    Init(unsigned int level);
    virtual void    End();
    virtual Status  Exec(unsigned int flags);

private:

    static void *Zalloc(void *opaque, unsigned int items, unsigned int size);
    static void Zfree(void *opaque, void *address);

    z_stream_s *mStream;
};

}
}
}

#endif
