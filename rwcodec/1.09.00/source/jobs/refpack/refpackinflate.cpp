// (c) Electronic Arts. All Rights Reserved.

#include "EABase/eabase.h"
#include "EAAssert/eaassert.h"
#include "jobs/refpack/refpack.h"

#ifdef EA_PLATFORM_PS3_SPU

static inline unsigned short ReadU16BigEndian(const void *data)
{
    return *(unsigned short *)data;
}

static inline unsigned ReadU32BigEndian(const void *data)
{
    return *(unsigned *)data;
}

#else

#include "EAStdC/EAEndian.h"

static inline unsigned short ReadU16BigEndian(const void *data)
{
    return EA::StdC::ReadFromBigEndianUint16(data);
}

static inline unsigned ReadU32BigEndian(const void *data)
{
    return EA::StdC::ReadFromBigEndianUint32(data);
}

#endif

namespace rw {
namespace core {
namespace codec {
namespace Refpack {

#if defined(EA_PLATFORM_PS3) && !defined(__SNC__)

static inline unsigned char *copylongn(unsigned char *d, const unsigned char *s,uint64_t run)
{
    register uint64_t si = (int64_t) s;
    register uint64_t di = (int64_t) d;
    register uint64_t temp=0;

    if (run>=4 && (di-si)>=4)
    {
        do
        {
            __asm__ ("lwz %0,0(%1)" : "=r"(temp): "r"(si));
            run -= 4;
            __asm__ ("stw %0,0(%1)" :: "r"(temp), "r"(di));
            si += 4;
            di += 4;
        } while (run>=4);
    }

    if (run)
        do
        {
            __asm__ ("lbz %0,0(%1)" :"=r"(temp):  "r"(si));
            ++si;
            __asm__ ("stb %0,0(%1)" :: "r"(temp), "r"(di));
            ++di;
        } while (--run);
    return (unsigned char *)di;
}

static inline unsigned char *copyn(unsigned char *d, const unsigned char *s, uint64_t run)
{
    register uint64_t si = (int64_t) s;
    register uint64_t di = (int64_t) d;
    register uint64_t temp=0;

    do
    {
        __asm__ ("lbz %0,0(%1)" :"=r"(temp):  "r"(si));
        ++si;
        __asm__ ("stb %0,0(%1)" :: "r"(temp), "r"(di));
        ++di;
    } while (--run);
    return (unsigned char *)di;
}

#else

static inline unsigned char *copyn(unsigned char *d, const unsigned char *s,unsigned int run)
{
    do
    {
        *d++ = *s++;
    } while (--run);

    return d;
}

#define copylongn copyn

#endif

unsigned int Inflate(void *outputBuf, const void *inputBuf)
{
    unsigned char first;
    unsigned char second;
    unsigned char third;
    unsigned char fourth;

    const unsigned char *s = static_cast<const unsigned char *>(inputBuf);
    unsigned char *d = static_cast<unsigned char *>(outputBuf);

    if (s == NULL)
    {
        return 0;
    }

    unsigned int type = *s++;
    type = (type<<8) + *s++;

    // size is the size of the decompressed data block
    unsigned int size = 0;

    if (type&0x8000)                // 4 byte size field
    {
        if (type&0x100)             // skip size
            s += 4;

        size = *s++;
        size = (size<<8) + *s++;
        size = (size<<8) + *s++;
        size = (size<<8) + *s++;
    }
    else
    {
        if (type&0x100)             // skip size
            s += 3;

        size = *s++;
        size = (size<<8) + *s++;
        size = (size<<8) + *s++;
    }

    for (;;)
    {
        unsigned char *ref;
        unsigned int run;
        first = *s;

        // assert(s < inputBuf + inputBufSize);

        if (!(first&0x80))
        {
            // short form.  The short form can have a reference
            // up to 1023 bytes, and have a 1st run up to 3 bytes,
            // and second run up to 9 bytes
            second = s[1];
            s += 2;
            run = (unsigned int)(first&3);
            if (run)
            {
                d = copyn(d,s,run);
                s += run;
            }

            ref = d-1 - (((first&0x60)<<3) + second);
            run = (unsigned int)((first&0x1c)>>2)+3;
            d = copylongn(d,ref,run);

            continue;
        }

        if (!(first&0x40))
        {
            // long form.  The long form can have a reference
            // up to 16383 bytes, and have a 1st run up to 3
            // bytes and a second run up to 66 bytes
            second = s[1];
            third = s[2];
            s += 3;
            run = (unsigned int)(second>>6);
            if (run)
            {
                d = copyn(d,s,run);
                s += run;
            }

            ref = d-1 - (((second&0x3f)<<8) + third);
            run = (unsigned int)(first&0x3f)+4;
            d = copylongn(d,ref,run);

            continue;
        }

        if (!(first&0x20))
        {
            // very long form.  The very long form can have a
            // reference up to 131071 bytes, and have a 1st run
            // up to 3 bytes, and a second run up to 1027 bytes
            second = s[1];
            third = s[2];
            fourth = s[3];
            s += 4;
            run = (unsigned int)(first&3);
            if (run)
            {
                d = copyn(d,s,run);
                s += run;
            }

            ref = d-1 - (((first&0x10)>>4<<16) +  (second<<8) + third);
            run = (unsigned int)((first&0x0c)>>2<<8) + fourth + 5;
            d = copylongn(d,ref,run);

            continue;
        }
        ++s;

        run = (unsigned int)(((first&0x1f)<<2)+4); // literal
        if (run<=112)
        {
            d = copylongn(d,s,run);
            s += run;
            continue;
        }

        run = (unsigned int)first&3;    // eof (+0..3 literal)
        if (run)
        {
            d = copyn(d,s,run);
            s += run;
        }
        break;
    }

    // Do a bit of bounds checking.  Ensure that the decompressed
    // data didn't go past outputBuf + size.  Better would be to
    // validate that the caller's buffer was large enough, but the API
    // does not include the caller's buffer sizes, just their addresses.
    EA_ASSERT_MSG(d == static_cast<unsigned char *>(outputBuf) + size,
        "Compressed source data is corrupt.  The end of the decompressed data is not in the expected location, and a buffer overrun or underrun has occurred.");

    return size;
}

unsigned int GetDecompressedSize(const void *inputBuf)
{
    unsigned int size = 0;
    const unsigned char *pData = static_cast<const unsigned char *>(inputBuf);

    // get the first two bytes of the data buffer
    unsigned short dataSignature = ReadU16BigEndian(inputBuf);

    // check for magic number; source should contain signature bits.
    if ((dataSignature & REF_SIG_MASK) == REF_SIG)
    {
        // mask off the least significant bit
        int nSizeBytes = (dataSignature & 0x8000) == 0x0 ? 3 : 4;

        // We currently do not have any 3-byte endian methods
        if (nSizeBytes == 3)
        {
            uint8_t* sizeCast = reinterpret_cast<uint8_t*>(&size);

            #if defined(EA_SYSTEM_LITTLE_ENDIAN)
                sizeCast[0] = (pData+2)[2];
                sizeCast[1] = (pData+2)[1];
                sizeCast[2] = (pData+2)[0];
            #else
                sizeCast[1] = (pData+2)[0];
                sizeCast[2] = (pData+2)[1];
                sizeCast[3] = (pData+2)[2];
            #endif
        }
        else
        {
            size = ReadU32BigEndian(pData + 2);
        }
    }

    return size;
}


}}}}
