// (c) Electronic Arts. All Rights Reserved.
#include "rw/core/codec/lz4codec.h"

#include <lz4/lz4.h>
#include <lz4/lz4hc.h>
#include <lz4/xxhash.h>

#include <EAStdC/EAMemory.h>
#include <EAStdC/EAEndian.h>

namespace rw
{
namespace core
{
namespace codec
{

static const uint32_t LZ4S_MAGICNUMBER = 0x184D2204;

#if defined(min)
#undef min
#endif

template <typename T> T min(const T a, const T b)
{
    return a <= b ? a : b;
}

void Buffer::Free()
{
    mAllocator->Free(mBegin);
    mBegin = 0;
    mCurrent = 0;
    mEnd = 0;
    mAllocEnd = 0;
}

void Buffer::Reset(const size_t size)
{
    if (static_cast<size_t>(mAllocEnd - mBegin) < size)
    {
        mAllocator->Free(mBegin);
        mBegin = static_cast<char*>(mAllocator->Alloc(size, "Lz4Codec Buffer", 0));
        mAllocEnd = mBegin + size;
    }
    mCurrent = mBegin;
    mEnd = mBegin + size;
}

size_t ConsumeBuffer::Consume(const void*& in, size_t& inAvailable)
{
    const size_t toConsume = min(static_cast<size_t>(mBuffer.End() - mBuffer.Current()), inAvailable);
    EA::StdC::Memcpy(mBuffer.Current(), in, toConsume);
    mBuffer.Advance(toConsume);
    in = static_cast<const char*>(in) + toConsume;
    inAvailable -= toConsume;
    return toConsume;
}

size_t ProduceBuffer::Produce(void*& out, size_t& outAvailable)
{
    const size_t toProduce = min(static_cast<size_t>(mBuffer.End() - mBuffer.Current()), outAvailable);
    EA::StdC::Memcpy(out, mBuffer.Current(), toProduce);
    mBuffer.Advance(toProduce);
    out = static_cast<char*>(out) + toProduce;
    outAvailable -= toProduce;
    return toProduce;
}

bool Lz4Header::Valid() const
{
    const int reserved0 = (flg >> 1) & 0x1; // Flag bit 1
    const int reserved1 = (bd >> 7) & 0x1;  // Block Descriptor bit 7
    const int reserved2 = bd & 0x7;         // Block Descriptor bits 0-3
    const unsigned int expectedChecksum = (XXH32(&flg, 2, 0) >> 8) & 0xff; // header checksum is byte 2 of hash of flg and bd
    return VersionNumber() == 1 && reserved0 == 0 && reserved1 == 0 && reserved2 == 0 && expectedChecksum == hc;
}

void Lz4Header::CalculateHeaderChecksum()
{
    // header checksum is byte 2 of xxh32 hash of flg and bd
    hc = (XXH32(&flg, 2, 0) >> 8) & 0xff;
}

Lz4Decompress::Lz4Decompress(EA::Allocator::ICoreAllocator* allocator) 
    : ICodec(allocator, 0),
      mConsume(allocator),
      mProduce(allocator)
{
    Init(0);
}

Lz4Decompress::~Lz4Decompress()
{
    End();
}

void Lz4Decompress::Init(unsigned int)
{
    ClearValues();

    mMagicNumber = 0;
    mCurrentBlockUncompressed = false;
    mState = HEADER;
    mStreamChecksumState = nullptr;
    // Prepare consume buffer to read 4 byte magic number + 3 byte header
    static_assert(sizeof(Lz4Header) == 3, "Lz4Header should be 3 bytes, no padding.");
    static const int magicPlusHeaderBytes = sizeof(mMagicNumber) + sizeof(Lz4Header);
    mConsume.Reset(magicPlusHeaderBytes);
    mReady = true;
}

void Lz4Decompress::End()
{
    mConsume.Free();
    mProduce.Free();

    if (mStreamChecksumState != nullptr)
    {
        XXH_errorcode e = XXH32_freeState(mStreamChecksumState);
		EA_ASSERT(e == XXH_OK);
        mStreamChecksumState = nullptr;
    }

    mReady = false;
}

ICodec::Status Lz4Decompress::Exec(unsigned int flags)
{
    EA_ASSERT_MSG(mReady, "Error - codec not initialized - called Init first\n");

    switch (mState)
    {
    case HEADER:
    {
        mConsume.Consume(mInputBuffer, mInputBytesAvailable);
        if (mConsume.Filled())
        {
            mMagicNumber = EA::StdC::ReadFromLittleEndianUint32(mConsume.Data());
            if (mMagicNumber != LZ4S_MAGICNUMBER)
            {
                return STREAM_ERROR;
            }
            EA::StdC::Memcpy(&mHeader, mConsume.Data() + sizeof(mMagicNumber), sizeof(mHeader));
            if (!mHeader.Valid())
            {
                return STREAM_ERROR;
            }
            // We only handle default streaming format
            if (!mHeader.BlockIndependence() || mHeader.BlockChecksum() || mHeader.StreamSize()
                || !mHeader.StreamChecksum() || mHeader.PresetDictionary())
            {
                return STREAM_ERROR;
            }
            mStreamChecksumState = XXH32_createState();
			XXH_errorcode e = XXH32_reset(mStreamChecksumState, 0);
			EA_ASSERT(e == XXH_OK);

            // Read block size for first block next
            mConsume.Reset(sizeof(uint32_t));
            mState = BLOCKSIZE;
        }
        break;
    }
    case BLOCKSIZE:
    {
        mConsume.Consume(mInputBuffer, mInputBytesAvailable);
        if (mConsume.Filled())
        {
            // Read block size - high bit set indicates uncompressed block.
            uint32_t blockSize = EA::StdC::ReadFromLittleEndianUint32(mConsume.Data());
            mCurrentBlockUncompressed = ((blockSize >> 31) & 0x1) == 1;
            blockSize &= 0x7fffffff;
            if (blockSize == 0)
            {
                // prepare to read stream checksum
                mConsume.Reset(sizeof(uint32_t));
                mState = STREAMCHECKSUM;
            }
            else
            {
                // prepare to read next block
                mConsume.Reset(blockSize);
                mState = BLOCK;
            }
        }
        break;
    }
    case DRAIN:
    {
        mOutputBytes += mProduce.Produce(mOutputBuffer, mOutputBytesAvailable);
        if (mProduce.Drained())
        {
            // prepare to read size of next block
            mConsume.Reset(sizeof(uint32_t));
            mState = BLOCKSIZE;
        }
        break;
    }
    case BLOCK:
    {
        mConsume.Consume(mInputBuffer, mInputBytesAvailable);
        if (mConsume.Filled())
        {
            // Decompress
            mProduce.Reset(mHeader.BlockMaximumSize());

            if (!mCurrentBlockUncompressed)
            {
                mProduce.Fill(static_cast<size_t>(LZ4_decompress_safe(mConsume.Data(), mProduce.Data(),
                                                                      static_cast<int>(mConsume.Size()),
                                                                      static_cast<int>(mProduce.Capacity()))));
            }
            else
            {
                EA::StdC::Memcpy(mProduce.Data(), mConsume.Data(), mConsume.Size());
                mProduce.Fill(mConsume.Size());
            }
            // Update stream checksum
            XXH_errorcode e = XXH32_update(mStreamChecksumState, mProduce.Data(), static_cast<int>(mProduce.Size()));
			EA_ASSERT(e == XXH_OK);

            // prepare to drain produce buffer to output
            mState = DRAIN;
        }
        break;
    }
    case STREAMCHECKSUM:
    {
        mConsume.Consume(mInputBuffer, mInputBytesAvailable);
        if (mConsume.Filled())
        {
            // Check checksum
            const uint32_t expectedStreamChecksum = EA::StdC::ReadFromLittleEndianUint32(mConsume.Data());
            const uint32_t calculatedStreamChecksum = XXH32_digest(mStreamChecksumState);
			XXH_errorcode e = XXH32_freeState(mStreamChecksumState);
			EA_ASSERT(e == XXH_OK);
			mStreamChecksumState = nullptr;
            if (calculatedStreamChecksum != expectedStreamChecksum)
            {
                return STREAM_ERROR;
            }
            return STREAM_END;
        }
        break;
    }
    }

    return STREAM_OK;
}

Lz4Compress::Lz4Compress(EA::Allocator::ICoreAllocator* allocator, unsigned int level)
    : ICodec(allocator, level),
      mConsume(allocator),
      mProduce(allocator)
{
    Init(level);
}

Lz4Compress::~Lz4Compress()
{
    End();
}

void Lz4Compress::Init(unsigned int level)
{
    ClearValues();

    // Initially supporting a fairly restricted set of options, don't assume code can handle other option combinations
    // without checking! In particular we don't currently handle independent blocks, block checksums, stream size or
    // preset dictionaries.
    mHeader.SetVersionNumber(1);
    mHeader.SetBlockIndependence(true);
    mHeader.SetBlockChecksum(false);
    mHeader.SetStreamSize(false);
    mHeader.SetStreamChecksum(true);
    mHeader.SetPresetDictionary(false);
    mHeader.SetBlockMaximumSize(Lz4Header::BMS_64KB); // MNTODO: Make this configurable?
    mHeader.CalculateHeaderChecksum();

    mStreamChecksumState = XXH32_createState();
	XXH_errorcode e = XXH32_reset(mStreamChecksumState, 0);
	EA_ASSERT(e == XXH_OK);

    // Set up buffer to consume input blocks
    mConsume.Reset(mHeader.BlockMaximumSize()); // read maximum of BlockMaximumSize bytes from input

    // Set up buffer to produce output blocks
    mProduce.Reset(sizeof(uint32_t) + mHeader.BlockMaximumSize()); // 4 byte block size + up to BlockMaximumSize data

    // Initialize produce buffer with LZ4S magic number and header
    static_assert(sizeof(Lz4Header) == 3, "Lz4Header should be 3 bytes, no padding.");
    EA::StdC::WriteToLittleEndian(&LZ4S_MAGICNUMBER, mProduce.Data(), sizeof(LZ4S_MAGICNUMBER));
    EA::StdC::Memcpy(mProduce.Data() + sizeof(LZ4S_MAGICNUMBER), &mHeader, sizeof(Lz4Header));
    mProduce.Fill(sizeof(LZ4S_MAGICNUMBER) + sizeof(Lz4Header));
    mState = HEADER;

    switch (level)
    {
    case 0:
    case 1:
    case 2: mCompressionLevel = 0; break;
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9: mCompressionLevel = 1; break;
    default: mCompressionLevel = 0; break;
    }

    mReady = true;
}

void Lz4Compress::End()
{
    mConsume.Free();
    mProduce.Free();

    if (mStreamChecksumState != nullptr)
    {
		XXH_errorcode e = XXH32_freeState(mStreamChecksumState);
		EA_ASSERT(e == XXH_OK);
        mStreamChecksumState = nullptr;
    }

    mReady = false;
}

ICodec::Status Lz4Compress::Exec(unsigned int flags)
{
    EA_ASSERT_MSG(mReady, "Error - codec not initialized - called Init first\n");

    const bool finish = (flags & ICodec::FINISH) == ICodec::FINISH;

    do
    {
        switch (mState)
        {
        case HEADER:
        {
            mOutputBytes += mProduce.Produce(mOutputBuffer, mOutputBytesAvailable);
            if (mProduce.Drained())
            {
                // If FINISH flag is set, consume smaller of available bytes and maximum block size.
                // Else, just consume maximum block size (more input will be made available).
                const uint32_t toConsume = static_cast<uint32_t>(
                    finish ? min(mInputBytesAvailable, static_cast<size_t>(mHeader.BlockMaximumSize()))
                           : mHeader.BlockMaximumSize());
                mConsume.Reset(toConsume);
                mState = BLOCK;
            }
            break;
        }
        case BLOCK:
        {
            mConsume.Consume(mInputBuffer, mInputBytesAvailable);
            if (mConsume.Filled())
            {
                // Update stream checksum
                XXH_errorcode e = XXH32_update(mStreamChecksumState, mConsume.Data(), static_cast<int>(mConsume.Size()));
				EA_ASSERT(e == XXH_OK);

                mProduce.Reset(sizeof(uint32_t) + mHeader.BlockMaximumSize());
                int (*compressionFunction)(const char*, char*, int, int)
                    = mCompressionLevel == 0 ? LZ4_compress_limitedOutput : LZ4_compressHC_limitedOutput;
                int outSize = compressionFunction(mConsume.Data(), mProduce.Data() + sizeof(uint32_t),
                                                  static_cast<int>(mConsume.Size()),
                                                  static_cast<int>(mConsume.Size()) - 1);
                // If compression succeeded, we want to output compressed block, otherwise copy uncompressed data
                if (outSize > 0)
                {
                    const uint32_t blockSize = static_cast<uint32_t>(outSize);
                    EA::StdC::WriteToLittleEndian(mProduce.Data(), blockSize);
                }
                else
                {
                    EA::StdC::Memcpy(mProduce.Data() + sizeof(uint32_t), mConsume.Data(), mConsume.Size());
                    outSize = static_cast<int>(mConsume.Size());
                    // Set high bit of block size to indicate uncompressed block follows
                    const uint32_t blockSize = static_cast<uint32_t>(outSize) & 0x80000000u;
                    EA::StdC::WriteToLittleEndian(mProduce.Data(), blockSize);
                }
                mProduce.Fill(sizeof(uint32_t) + outSize);

                mState = DRAIN;
            }
            break;
        }
        case DRAIN:
        {
            mOutputBytes += mProduce.Produce(mOutputBuffer, mOutputBytesAvailable);
            if (mProduce.Drained())
            {
                const uint32_t toConsume = static_cast<uint32_t>(
                    finish ? min(mInputBytesAvailable, static_cast<size_t>(mHeader.BlockMaximumSize()))
                            : mHeader.BlockMaximumSize());
                mConsume.Reset(toConsume);
                mState = BLOCK;

                if (mConsume.Size() == 0)
                {
                    // Reached end of stream, write 0 block size to indicate stream end and then write stream checksum.
                    const uint32_t blockSize = 0;
                    const uint32_t streamChecksum = static_cast<uint32_t>(XXH32_digest(mStreamChecksumState));
					XXH_errorcode e = XXH32_freeState(mStreamChecksumState);
					EA_ASSERT(e == XXH_OK);
                    mStreamChecksumState = nullptr;

                    mProduce.Reset(sizeof(blockSize) + sizeof(streamChecksum));
                    EA::StdC::WriteToLittleEndian(mProduce.Data(), blockSize);
                    EA::StdC::WriteToLittleEndian(mProduce.Data() + sizeof(blockSize), streamChecksum);
                    
                    mState = STREAMCHECKSUM;
                }
            }
            else
            {
                // Need to return here in case we're trying to finish but don't have enough output space
                return STREAM_OK;
            }
            break;
        }
        case STREAMCHECKSUM:
        {
            mOutputBytes += mProduce.Produce(mOutputBuffer, mOutputBytesAvailable);
            if (mProduce.Drained())
            {
                return STREAM_END;
            }
            else
            {
                // Need to return here in case we're trying to finish but don't have enough output space
                return STREAM_OK;
            }
        }
        }
    } while (finish);

    return STREAM_OK;
}
}
}
}
