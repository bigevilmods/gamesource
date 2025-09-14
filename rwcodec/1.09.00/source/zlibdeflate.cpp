// (c) Electronic Arts. All Rights Reserved.

/// \file
/// \brief        Codec class definition.

#include "rw/core/codec/icodec.h"
#include "rw/core/codec/zlib.h"

#include <EAAssert/eaassert.h>
#include <coreallocator/icoreallocator_interface.h>
#include <xceedzip/zlib.h>

#include <string.h>

namespace rw
{
namespace core
{
namespace codec
{

// Constructor simply calls the base class constructor.  Not needed, but
// let's be explicit
ZlibDeflate::ZlibDeflate(EA::Allocator::ICoreAllocator *allocator, unsigned int level)
    : ICodec(allocator, level)
{
    ZlibDeflate::Init(level);
}

ZlibDeflate::~ZlibDeflate()
{
    ZlibDeflate::End();
}

//lint -e{818}
void *ZlibDeflate::Zalloc(void *opaque, unsigned int items, unsigned int size)
{
    return static_cast<ZlibDeflate *>(opaque)->mAllocator->Alloc(
        items * size, "zlib temp buffer", EA::Allocator::MEM_TEMP);
}

//lint -e{818}
void ZlibDeflate::Zfree(void *opaque, void *address)
{
    static_cast<ZlibDeflate *>(opaque)->mAllocator->Free(address);
}

void ZlibDeflate::Init(unsigned int level)
{
    if (mReady)
    {
        EA_ASSERT_MSG(level == mLevel,
            "To re-initialize with a different compression level, change the compression level, first call End");
        return;
    }

    ClearValues();

    mStream = static_cast<z_stream_s *>(mAllocator->Alloc(sizeof(*mStream),
        "rw::core::codec::ZlibDeflate::mStream", EA::Allocator::MEM_TEMP));

    memset(mStream, 0, sizeof(*mStream));

    mLevel = level;
    mStream->opaque = this;
    mStream->zalloc = Zalloc;
    mStream->zfree = Zfree;
    deflateInit(mStream, static_cast<int>(mLevel));
    mReady = true;
}

void ZlibDeflate::End()
{
    if (mReady)
    {
        deflateEnd(mStream);
        mAllocator->Free(mStream);
        mStream = NULL;
        mReady = false;
    }
}

ICodec::Status ZlibDeflate::Exec(unsigned int flags)
{
    EA_ASSERT_MSG(mReady,
        "Error - codec not initialized - called Init first\n");

    mStream->next_in = const_cast<Bytef *>(static_cast<const Bytef *>(mInputBuffer));
    mStream->avail_in = static_cast<uInt>(mInputBytesAvailable);
    mStream->next_out = static_cast<Bytef *>(mOutputBuffer);
    mStream->avail_out = static_cast<uInt>(mOutputBytesAvailable);

    int zFlag = Z_NO_FLUSH;

    switch(flags)
    {
    case ICodec::NO_FLUSH:
        // use default
        break;

    case ICodec::FLUSH:
        zFlag = Z_SYNC_FLUSH;
        break;

    case ICodec::FINISH:
        zFlag = Z_FINISH;
        break;

    default:
        EA_FAIL_MSG("Invalid flags parameter");
        break;
    }

    int zRet = deflate(mStream, zFlag);

    ICodec::Status ret = ICodec::STREAM_OK;

    mInputBuffer = mStream->next_in;
    mInputBytesAvailable = mStream->avail_in;
    mOutputBuffer = mStream->next_out;
    mOutputBytesAvailable = mStream->avail_out;
    mOutputBytes = mStream->total_out;

    switch(zRet)
    {
    case Z_OK:
        break;

    case Z_STREAM_END:
        ret = codec::ICodec::STREAM_END;
        break;

    default:
        ret = codec::ICodec::STREAM_ERROR;
        break;
    }

    return ret;
}


}
}
}
