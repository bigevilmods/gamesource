// (c) Electronic Arts. All Rights Reserved.

#include "EABase/eabase.h"
#include "EAAssert/eaassert.h"
#include "detail/decompressors.h"

namespace EA { namespace Allocator { class ICoreAllocator; } }

#if defined(EA_PLATFORM_XENON) || defined(RW_CORE_CODEC_LZXCOMPRESSION)

#if defined(EA_PLATFORM_XENON)
#ifndef NOD3D
#define NOD3D
#endif

#ifndef NONET
#define NONET
#endif

#include <xtl.h>

#else

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>

#endif

#include <xcompress.h>


//---------------------------------------------------------------------------
namespace rw { namespace core { namespace codec { namespace detail {

void DecompressLZX(uintptr_t outputBuffer, uintptr_t outputBufferSize, uintptr_t inputBuffer, uintptr_t inputBufferSize)
{
    void *outBuf = reinterpret_cast<void *>(outputBuffer);
    size_t outBufSize = static_cast<size_t>(outputBufferSize);
    const void *inBuf = reinterpret_cast<const void *>(inputBuffer);
    size_t inputSize = static_cast<size_t>(inputBufferSize);

    EA_ASSERT(inputSize != 0);

    HRESULT hr;
    XMEMDECOMPRESSION_CONTEXT cmpContext = NULL;

    hr = XMemCreateDecompressionContext(XMEMCODEC_LZX, NULL, 0, &cmpContext);
    EA_ASSERT(hr == S_OK);

    SIZE_T sz = outBufSize;
    hr = XMemDecompress(cmpContext, outBuf, &sz, inBuf, inputSize);
    EA_ASSERT(hr == S_OK);
    (void)hr;

    XMemDestroyDecompressionContext(cmpContext);
}

}}}}

#else

namespace rw { namespace core { namespace codec { namespace detail {

#ifdef EA_PLATFORM_PS3_SPU
void Decompress(void *, size_t, const void *, size_t)
#else
void DecompressLZX(uintptr_t, uintptr_t, uintptr_t, uintptr_t)
#endif
{
    EA_FAIL_MSG("LZX is not supported on non-Xbox 360 platforms!");
}

}}}}
#endif
