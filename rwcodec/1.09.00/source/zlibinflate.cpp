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
ZlibInflate::ZlibInflate(EA::Allocator::ICoreAllocator *allocator)
    : ICodec(allocator, 0)
{
    ZlibInflate::Init(0);
}

ZlibInflate::~ZlibInflate()
{
    ZlibInflate::End();
}

//lint -e{818}
void *ZlibInflate::Zalloc(void *opaque, unsigned int items, unsigned int size)
{
    return static_cast<ZlibInflate *>(opaque)->mAllocator->Alloc(
        items * size, "zlib temp buffer", EA::Allocator::MEM_TEMP);
}

//lint -e{818}
void ZlibInflate::Zfree(void *opaque, void *address)
{
    static_cast<ZlibInflate *>(opaque)->mAllocator->Free(address);
}

void ZlibInflate::Init(unsigned int)
{
    if (mReady)
        return;

    ClearValues();

    mStream = static_cast<z_stream_s *>(mAllocator->Alloc(sizeof(*mStream),
        "rw::core::codec::ZlibInflate::mStream", EA::Allocator::MEM_TEMP));

    memset(mStream, 0, sizeof(*mStream));

    mStream->opaque = this;
    mStream->zalloc = Zalloc;
    mStream->zfree = Zfree;
    inflateInit(mStream);
    mReady = true;
}

void ZlibInflate::End()
{
    if (mReady)
    {
        inflateEnd(mStream);
        mAllocator->Free(mStream);
        mStream = NULL;
        mReady = false;
    }
}

ICodec::Status ZlibInflate::Exec(unsigned int flags)
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

    int zRet = inflate(mStream, zFlag);

    // This is to catch an error where it wants one more byte
    // But we do not think it actually uses it(thought it does read it from memory)
    // We believe this stems from setting the window size to -MAX_WBITS(the important part is that it is negative)
    // This enables an undocumented feature that removes a footer and header on the zlib file
    // We use said undocumented feature to stay compatible with sony_edge zlib, which makes use of it
    // In what seems like a small number of cases, we will get a Z_BUF_ERROR in which one more byte
    // will fix this issue
    // We got the idea to add one byte from: http://java.sun.com/javase/6/docs/api/java/util/zip/Inflater.html
    if(zRet == Z_BUF_ERROR && zFlag == Z_FINISH && !mStream->avail_in )
    {
        // We give it one more byte
        mStream->avail_in = 1;
        // And do not let it output anymore

        // We save the avail_out to restore it momentarily, hopefully this is OK
        // I'll assume this is not threaded or anything for now
        uInt availOut = mStream->avail_out;
        mStream->avail_out = 0;

        zRet = inflate(mStream, zFlag);
        mStream->avail_out = availOut;
    }

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
