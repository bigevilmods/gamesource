// (c) Electronic Arts. All Rights Reserved.

/**
\file
Implementation of rw::core::codec::RefpackInflate.
*/

#include "rw/core/codec/icodec.h"
#include "rw/core/codec/refpack.h"
#include "jobs/refpack/refpack.h"
#include <stdlib.h>

namespace rw {
namespace core {
namespace codec {

//lint -e{1927} symbol 'x' did not appear in the constructor initializer list.
RefpackInflate::RefpackInflate(EA::Allocator::ICoreAllocator* allocator)
    : ICodec(allocator, 0)
{
    RefpackInflate::Init(0);
}

RefpackInflate::~RefpackInflate()
{
    RefpackInflate::End();
}

void RefpackInflate::Init(unsigned int)
{
    if (mReady)
        return;

    ClearValues();

    mState = GET_TYPE;
    mNextState = GET_TYPE;
    mBytesNeeded = 2;
    mOriginalLength = 0;
    mRunLength = 0;
    mControl = 0;
    mRefPtr = NULL;
    mRefWindowBase = NULL;
    mRefWindowPtr = NULL;
    mNumBytesDecompressed = 0;
    mWindowSize = 0;
    mStatus = ICodec::STREAM_OK;
    mReady = true;
}

void RefpackInflate::End()
{
    if (mReady)
    {
        if (mRefWindowBase != NULL)
        {
            mAllocator->Free(mRefWindowBase);
            mRefWindowBase = NULL;
        }

        mReady = false;
    }
}

ICodec::Status RefpackInflate::Exec(unsigned int)
{
    EA_ASSERT_FORMATTED(mReady,
        ("Error - codec not initialized - called Init first"));

    mNextIn = static_cast<const unsigned char *>(mInputBuffer);
    mNextOut = static_cast<unsigned char *>(mOutputBuffer);

    mStatus = Decompress();

    mInputBuffer = static_cast<const void *>(mNextIn);
    mOutputBuffer = static_cast<unsigned char *>(mNextOut);

    return mStatus;
}

#define NEEDBYTE {if(mInputBytesAvailable==0) return mStatus;}
#define NEXTBYTE (mInputBytesAvailable--,*mNextIn++)

ICodec::Status RefpackInflate::Decompress()
{
    if (mStatus != ICodec::STREAM_OK)
    {
        return mStatus;
    }

    if (mState == GET_TYPE)
    {
        // if this is the first call to Decompress, we can check whether we can
        // use a much faster implementation of the decompressor
        const unsigned int MIN_BYTES_REQUIRED = 6;
        if (mInputBytesAvailable >= MIN_BYTES_REQUIRED)
        {
            // There are enough bytes in the input stream to determine the necessary
            // size of the decompressed buffer
            codec::RefpackBuf refpackBuf;

            unsigned int decompressedSize = static_cast<unsigned int>(
                refpackBuf.GetDecompressedSize(mInputBuffer));

            if ((decompressedSize != 0) && (decompressedSize <= mOutputBytesAvailable))
            {
                // the output buffer is large enough to hold all the uncompressed data,
                // so we can do the decompression in a much faster way.
                mOutputBytes = refpackBuf.Decompress(mOutputBuffer, mInputBuffer);
                mOutputBytesAvailable -= static_cast<int32_t>(mOutputBytes);
                mNextOut += mOutputBytes;

                // we cannot tell how much input data was consumed,
                // so those values are not updated.

                // The decompression has finished, so return STREAM_END.
                return STREAM_END;
            }
        }
    }

    for (;;)
    {
        switch(mState)
        {
        case GET_TYPE:
            while (mBytesNeeded > 0)
            {
                NEEDBYTE;

                //lint -e{960}
                mControl = (mControl<<8) + NEXTBYTE;
                --mBytesNeeded;
            }

            if ((mControl & REF_SIG_MASK) != REF_SIG)
            {
                mStatus = STREAM_ERROR;
                return mStatus;
            }

            if ((mControl & REF_HEADER_FLAG_SIZE32) != 0)
                mBytesNeeded = 4;
            else
                mBytesNeeded = 3;

            mWindowSize = 1 + (((mControl & REF_HEADER_FLAG_SHORT_REFS) != 0) ?
                MAX_LOOKBACK_SHORT : MAX_LOOKBACK_LONG);

            mRefWindowBase = static_cast<unsigned char *>(
                mAllocator->Alloc(static_cast<unsigned int>(mWindowSize),
                "rw::core::codec::RefpackInflate::Decompress::mRefWindowBase",
                EA::Allocator::MEM_TEMP));

            mRefWindowPtr = mRefWindowBase;

            mState = GET_LENGTH;

            //lint -fallthrough
        case GET_LENGTH:
            while (mBytesNeeded > 0)
            {
                NEEDBYTE;
                //lint -e{701, 960}
                mOriginalLength = (mOriginalLength<<8) + NEXTBYTE;
                --mBytesNeeded;
            }

            mState = GET_CONTROL_BYTE;

            //lint -fallthrough
        case GET_CONTROL_BYTE:
            if (mNumBytesDecompressed == mOriginalLength)
            {
                mState = SETUPFLUSH;
                break;
            }
            NEEDBYTE;
            //lint -e{960}
            mControl = NEXTBYTE;
            mState = TEST_CONTROL_BYTE;

            //lint -fallthrough
        case TEST_CONTROL_BYTE:
            if ((mControl&0x80) == 0)
            {
                mBytesNeeded = 1;
                mState = GET_CONTROL_BYTES;
                mNextState = GET_SHORT_RUN_BYTES;
            }
            else if ((mControl&0x40) == 0)
            {
                mBytesNeeded = 2;
                mState = GET_CONTROL_BYTES;
                mNextState = GET_LONG_RUN_BYTES;
            }
            else if ((mControl&0x20) == 0)
            {
                mBytesNeeded = 3;
                mState = GET_CONTROL_BYTES;
                mNextState = GET_VERY_LONG_RUN_BYTES;
            }
            else
            {
                mRunLength = ((mControl&0x1f)<<2)+4;    // literal
                if (mRunLength <= 112)
                {
                    mState = COPY_LITERAL;
                    mNextState = GET_CONTROL_BYTE;
                }
                else
                {
                    mRunLength = mControl&3;            // EOF (+0..3 literal)
                    mState = COPY_LITERAL;
                    mNextState = GET_CONTROL_BYTE;
                }
            }
            break;

        case GET_CONTROL_BYTES:
            while(mBytesNeeded > 0)
            {
                NEEDBYTE;
                //lint -e{960}
                mControl = (mControl<<8) + NEXTBYTE;
                --mBytesNeeded;
            }
            mState = mNextState;
            break;

        case GET_SHORT_RUN_BYTES:
            mRunLength = (mControl & 0x300) >> 8;
            if (mRunLength > 0)
            {
                mState = COPY_LITERAL;
                mNextState = GET_SHORT_REF_BYTES;
            }
            else
            {
                mState = GET_SHORT_REF_BYTES;
            }
            break;

        case GET_LONG_RUN_BYTES:
            mRunLength = (mControl&0xff00) >> 14;
            if (mRunLength > 0)
            {
                mState = COPY_LITERAL;
                mNextState = GET_LONG_REF_BYTES;
            }
            else
            {
                mState = GET_LONG_REF_BYTES;
            }
            break;

        case GET_VERY_LONG_RUN_BYTES:
            mRunLength = (mControl&0x3000000)>>24;
            if (mRunLength > 0)
            {
                mState = COPY_LITERAL;
                mNextState = GET_VERY_LONG_REF_BYTES;
            }
            else
            {
                mState = GET_VERY_LONG_REF_BYTES;
            }
            break;

        case GET_SHORT_REF_BYTES:
            //lint -e{834, 961}
            mRefPtr = mRefWindowPtr - 1 - (((mControl&0x6000)>>5) + (mControl&0xff));
            if (mRefPtr < mRefWindowBase)
                mRefPtr += mWindowSize;
            mRunLength = ((mControl&0x1c00)>>10)+3;
            mState = COPY_REFERENCE;
            break;

        case GET_LONG_REF_BYTES:
            //lint -e{834, 961}
            mRefPtr = mRefWindowPtr - 1 - (mControl&0x3fff);
            if (mRefPtr < mRefWindowBase)
                mRefPtr += mWindowSize;
            mRunLength = ((mControl&0x3f0000)>>16)+4;
            mState = COPY_REFERENCE;
            break;

        case GET_VERY_LONG_REF_BYTES:
            //lint -e{834, 961}
            mRefPtr = mRefWindowPtr - 1 - (((mControl&0x10000000)>>12) + ((mControl&0xffff00)>>8));
            if (mRefPtr < mRefWindowBase)
                mRefPtr += mWindowSize;

            #if defined EA_DEBUG
                // If the reference pointer is STILL behind the mRefWindowBase start address,
                // the reference jumped over too many bytes and we'll have to abort.  The
                // data stream needs to be recompressed without any very long (over 16K-1
                // bytes) references.
                if (mRefPtr < mRefWindowBase)
                {
                    // cannot deal with references over 16K in length.
                    return STREAM_ERROR;
                }
            #endif

            mRunLength = ((mControl&0x0c000000)>>18) + (mControl&0xff) + 5;
            mState = COPY_REFERENCE;
            break;

        case COPY_LITERAL:
            while(mRunLength > 0)
            {
                if (mNumBytesDecompressed < mOriginalLength)
                {
                    NEEDBYTE;
                }

                if (mNumBytesDecompressed >= mWindowSize)
                {
                    if (mOutputBytesAvailable != 0)
                    {
                        // The mRefWindowBase buffer is full, so move the tail end of the
                        // mRefWindowBase buffer to the users' output buffer.
                        const unsigned char *tail = mRefWindowPtr;
                        if (tail == (mRefWindowBase + mWindowSize))
                        {
                            tail = mRefWindowBase;
                            mRefWindowPtr = mRefWindowBase;
                        }

                        *mNextOut++ = *tail;
                        ++mOutputBytes;
                        --mOutputBytesAvailable;
                    }
                    else
                    {
                        // out of room in the decompress buffer; return.
                        return ICodec::STREAM_OK;
                    }
                }

                if (mNumBytesDecompressed < mOriginalLength)
                {
                    //lint -e{960} comma operator used outside of 'for' expression
                    *mRefWindowPtr++ = NEXTBYTE;
                    ++mNumBytesDecompressed;
                    --mRunLength;
                }
            }
            mState = mNextState;
            break;

        case COPY_REFERENCE:
            while(mRunLength > 0)
            {
                if (mNumBytesDecompressed >= mWindowSize)
                {
                    if (mOutputBytesAvailable != 0)
                    {
                        // The mRefWindowBase buffer is full, so move the tail end of the
                        // mRefWindowBase buffer to the users' output buffer.
                        const unsigned char *tail = mRefWindowPtr;
                        if (tail == (mRefWindowBase + mWindowSize))
                        {
                            tail = mRefWindowBase;
                            mRefWindowPtr = mRefWindowBase;
                        }

                        *mNextOut++ = *tail;
                        ++mOutputBytes;
                        --mOutputBytesAvailable;
                    }
                    else
                    {
                        // out of room in the decompress buffer; return.
                        return ICodec::STREAM_OK;
                    }
                }

                if (mRefPtr == (mRefWindowBase + mWindowSize))
                    mRefPtr -= mWindowSize;

                *mRefWindowPtr++ = *mRefPtr++;
                ++mNumBytesDecompressed;
                --mRunLength;
            }
            mState = GET_CONTROL_BYTE;
            break;

        case SETUPFLUSH:
            // we have completed the decompression into the mRefWindowBase
            // buffer, but now we'll need to flush the mRefWindowBase buffer to
            // the users' output buffer.

            if (mNumBytesDecompressed < mWindowSize)
            {
                // If the number of bytes decompressed is less than the size
                // of the sliding window, we'll need to move the window pointer
                // back to the window base address.  If there were more bytes
                // than the sliding window size, mRefWindowPtr is already at the
                // first byte to be placed into the user output buffer.
                mRefWindowPtr = mRefWindowBase;
            }
            mNumBytesDecompressed -= static_cast<int64_t>(mOutputBytes);
            mState = FLUSHWINDOW;
            break;

        case FLUSHWINDOW:
            while(mOutputBytesAvailable > 0)
            {
                if ((mRefWindowPtr - mRefWindowBase) >= mWindowSize)
                    mRefWindowPtr = mRefWindowBase;

                *mNextOut++ = *mRefWindowPtr++;
                ++mOutputBytes;
                --mOutputBytesAvailable;

                // decrement mNumBytesDecompressed and terminate the loop when it reaches 0.
                --mNumBytesDecompressed;
                if (mNumBytesDecompressed == 0)
                    return STREAM_END;
            }

            return ICodec::STREAM_OK;
        }
    }
}

}}}
