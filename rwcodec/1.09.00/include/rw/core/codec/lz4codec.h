// (c) Electronic Arts. All Rights Reserved.

/// \file
/// \brief        Codec class definition.

#ifndef EA_CORE_CODEC_LZ4CODEC_H
#define EA_CORE_CODEC_LZ4CODEC_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "rw/core/codec/icodec.h"

typedef struct XXH32_state_s XXH32_state_t;

namespace rw
{
namespace core
{
namespace codec
{

// Buffer, ConsumeBuffer and ProduceBuffer are helper classes to manage input / output state across multiple invocations
// of Exec(). Because of the block based format used by LZ4 streams, we cannot produce output until a full block of
// input has been received.
class RWCODEC_API Buffer
{
public:
    explicit Buffer(EA::Allocator::ICoreAllocator* allocator)
            : mAllocator(allocator), mBegin(0), mCurrent(0), mEnd(0), mAllocEnd(0)
    {
    }
    ~Buffer() { Free(); }
    void Free();
    void Reset(size_t size);
    char* Begin() const { return mBegin; }
    char* Current() const { return mCurrent; }
    void Advance(size_t n) { mCurrent += n; }
    char* End() const { return mEnd; }
    size_t Size() const { return static_cast<size_t>(mEnd - mBegin); }
    size_t Capacity() const { return static_cast<size_t>(mAllocEnd - mBegin); }
    void Fill(size_t n) { mEnd = mBegin + n; }

private:
    EA::Allocator::ICoreAllocator* mAllocator;
    char* mBegin;
    char* mCurrent;
    char* mEnd;
    char* mAllocEnd;
};

class RWCODEC_API ConsumeBuffer
{
public:
    explicit ConsumeBuffer(EA::Allocator::ICoreAllocator* allocator) : mBuffer(allocator) {}
    void Free() { mBuffer.Free(); }
    void Reset(size_t bytesToConsume) { mBuffer.Reset(bytesToConsume); }
    bool Filled() const { return mBuffer.Current() == mBuffer.End(); }
    size_t Consume(const void*& in, size_t& inAvailable);
    const char* Data() const { return mBuffer.Begin(); }
    size_t Size() const { return mBuffer.Size(); }

private:
    Buffer mBuffer;
};

class RWCODEC_API ProduceBuffer
{
public:
    explicit ProduceBuffer(EA::Allocator::ICoreAllocator* allocator) : mBuffer(allocator) {}
    void Free() { mBuffer.Free(); }
    void Reset(size_t bytesToProduce) { mBuffer.Reset(bytesToProduce); }
    bool Drained() const { return mBuffer.Current() == mBuffer.End(); }
    size_t Produce(void*& out, size_t& outAvailable);
    char* Data() const { return mBuffer.Begin(); }
    size_t Capacity() const { return mBuffer.Capacity(); }
    void Fill(size_t bytes) { mBuffer.Fill(bytes); }
    size_t Size() const { return mBuffer.Size(); }

private:
    Buffer mBuffer;
};

// LZ4 Streaming Format Stream Descriptor.
// See http://fastcompression.blogspot.fr/2013/03/a-streaming-format-for-lz4.html
struct RWCODEC_API Lz4Header
{
    enum BlockMaximumSizeValue
    {
        BMS_64KB = 4,
        BMS_256KB = 5,
        BMS_1MB = 6,
        BMS_4MB = 7
    };

    Lz4Header() : flg(0), bd(0), hc(0)
    {
    }

    bool Valid() const;
    int VersionNumber() const
    {
        return (flg >> 6) & 0x3; // Flag bits 6 and 7
    }
    void SetVersionNumber(int ver)
    {
        flg &= ~(0x3u << 6); // Clear flag bits 6 and 7
        flg |= ((ver & 0x3) << 6); // Set flag bits 6 and 7
    }
    int BlockIndependence() const
    {
        return (flg >> 5) & 0x1; // Flag bit 5
    }
    void SetBlockIndependence(bool blockIndependence)
    {
        const int bi = blockIndependence ? 1 : 0;
        flg &= ~(0x1u << 5); // Clear flag bit 5
        flg |= bi << 5; // Set flag bit 5
    }
    int BlockChecksum() const
    {
        return (flg >> 4) & 0x1; // Flag bit 4
    }
    void SetBlockChecksum(bool blockChecksum)
    {
        const int bc = blockChecksum ? 1 : 0;
        flg &= ~(0x1u << 4); // Clear flag bit 4
        flg |= bc << 4; // Set flag bit 4
    }
    int StreamSize() const
    {
        return (flg >> 3) & 0x1; // Flag bit 3
    }
    void SetStreamSize(bool streamSize)
    {
        const int ss = streamSize ? 1 : 0;
        flg &= ~(0x1u << 3); // Clear flag bit 3
        flg |= ss << 3; // Set flag bit 3
    }
    int StreamChecksum() const
    {
        return (flg >> 2) & 0x1; // Flag bit 2
    }
    void SetStreamChecksum(bool streamChecksum)
    {
        const int sc = streamChecksum ? 1 : 0;
        flg &= ~(0x1u << 2); // Clear flag bit 2
        flg |= sc << 2; // Set flag bit 2
    }
    int PresetDictionary() const
    {
        return flg & 0x1; // Flag bit 0
    }
    void SetPresetDictionary(bool presetDictionary) 
    {
        const int pd = presetDictionary ? 1 : 0;
        flg &= ~0x1u; // Clear flag bit 1
        flg |= pd; // Set flag bit 1
    }
    size_t BlockMaximumSize() const
    {
        int blockDescriptorMaxSizeBits = (bd >> 4) & 0x7;   // Block Descriptor bits 4, 5, 6
        return static_cast<size_t>(1ull << ((2 * blockDescriptorMaxSizeBits) + 8)); // Convert to size in bytes
    }
    void SetBlockMaximumSize(BlockMaximumSizeValue blockMaximumSize)
    {
        bd &= ~(0x7 << 4); // Clear block descriptor bits 4, 5, 6;
        bd |= blockMaximumSize << 4; // Set block descriptor bits 4, 5, 6
    }

    void CalculateHeaderChecksum();

    unsigned char flg; // Flags
    unsigned char bd;  // Block Descriptor
    unsigned char hc;  // Header checksum
};

/// Lz4Decompress implements a decompressor for the lz4 algorithm.
class RWCODEC_API Lz4Decompress : public ICodec
{
public:
    Lz4Decompress(EA::Allocator::ICoreAllocator* allocator);
    virtual ~Lz4Decompress();

    virtual void Init(unsigned int level);
    virtual void End();
    virtual Status Exec(unsigned int flags);

private:
    enum State
    {
        HEADER,
        BLOCKSIZE,
        BLOCK,
        DRAIN,
        STREAMCHECKSUM
    };

    uint32_t mMagicNumber;
    Lz4Header mHeader;
    bool mCurrentBlockUncompressed;

    ConsumeBuffer mConsume;
    ProduceBuffer mProduce;

    State mState;

	XXH32_state_t* mStreamChecksumState;
};

/// Lz4Compress implements a compressor for the lz4 algorithm
/// The level parameter can be any value from 0-9 inclusive.
class RWCODEC_API Lz4Compress : public ICodec
{
public:
    Lz4Compress(EA::Allocator::ICoreAllocator* allocator, unsigned int level);
    virtual ~Lz4Compress();

    virtual void Init(unsigned int level);
    virtual void End();
    virtual Status Exec(unsigned int flags);

private:
    enum State
    {
        HEADER,
        BLOCK,
        DRAIN,
        STREAMCHECKSUM,
    };

    Lz4Header mHeader;
    int mCompressionLevel;

    ConsumeBuffer mConsume;
    ProduceBuffer mProduce;

    State mState;

    XXH32_state_t* mStreamChecksumState;
};

}
}
}

#endif // EA_CORE_CODEC_LZ4CODEC_H
