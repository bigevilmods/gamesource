// (c) Electronic Arts. All Rights Reserved.

/**
\file
Implementation of rw::core::codec::ICodec.
*/

#include "rw/core/codec/icodec.h"

namespace rw {
namespace core {
namespace codec {

ICodec::ICodec(EA::Allocator::ICoreAllocator* allocator, unsigned int level)
: mAllocator(allocator), mReady(false), mLevel(level), mInputBytesAvailable(0),
mOutputBytesAvailable(0), mOutputBytes(0), mInputBuffer(NULL),
mOutputBuffer(NULL)
{}

ICodec::~ICodec()
{
    mInputBuffer = NULL;
    mOutputBuffer = NULL;
}

void ICodec::SetInputBuffer(const void *in, size_t InputBufSize)
{
    mInputBuffer = in;
    mInputBytesAvailable = InputBufSize;
}

void ICodec::SetOutputBuffer(void *out, size_t OuputBufSize)
{
    mOutputBuffer = out;
    mOutputBytesAvailable = OuputBufSize;
}

void ICodec::Reset()
{
    End();
    Init(mLevel);
}

// Clear all the base class values to 0
void ICodec::ClearValues()
{
    mInputBytesAvailable = 0;
    mOutputBytesAvailable = 0;
    mOutputBytes = 0;

    mInputBuffer = NULL;
    mOutputBuffer = NULL;
}



}}}
