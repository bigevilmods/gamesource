// (c) Electronic Arts. All Rights Reserved.

/// \file
/// Inline implementation of deprecated endian-conversion functions

namespace rw {
namespace core {
namespace endian {

inline uint32_t ConvertLittle(const uint32_t x)
{
    rwFUNCTION();
#if (rwHOST_ENDIAN == rwBIENDIAN)
    rwRETURN(
        ((x&0xff)<<24) |
        ((x&0xff00)<<8) |
        ((x>>8)&0xff00) |
        ((x>>24)&0xff));
#else
    rwRETURN(x);
#endif
}

inline uint16_t ConvertLittle(const uint16_t x)
{
    rwFUNCTION();
#if (rwHOST_ENDIAN == rwBIENDIAN)
    rwRETURN(
        (uint16_t)
        ( ((x&0xff)<<8) |
        ((x>>8)&0xff) ));
#else
    rwRETURN(x);
#endif
}

inline int32_t ConvertLittle(const int32_t x)
{
    rwFUNCTION();
#if (rwHOST_ENDIAN == rwBIENDIAN)
    rwRETURN(
        ((x&0xff)<<24) |
        ((x&0xff00)<<8) |
        ((x>>8)&0xff00) |
        ((x>>24)&0xff));
#else
    rwRETURN(x);
#endif
}

inline int16_t ConvertLittle(const int16_t x)
{
    rwFUNCTION();
#if (rwHOST_ENDIAN == rwBIENDIAN)
    rwRETURN(
        (int16_t)
        ( ((x&0xff)<<8) |
        ((x>>8)&0xff) ));
#else
    rwRETURN(x);
#endif
}

inline uint64_t ConvertLittle(const uint64_t x)
{
    rwFUNCTION();
#if (rwHOST_ENDIAN == rwBIENDIAN)
    rwRETURN(
        ((x&0xff)<<56) |
        ((x&0xff00)<<40) |
        ((x&0xff0000)<<24) |
        ((x&0xff000000)<<8) |
        ((x>>8)&0xff000000) |
        ((x>>24)&0xff0000) |
        ((x>>40)&0xff00) |
        ((x>>56)&0xff));
#else
    rwRETURN(x);
#endif
}

inline int64_t ConvertLittle(const int64_t x)
{
    rwFUNCTION();
#if (rwHOST_ENDIAN == rwBIENDIAN)
    rwRETURN(
        ((x&0xff)<<56) |
        ((x&0xff00)<<40) |
        ((x&0xff0000)<<24) |
        ((x&0xff000000)<<8) |
        ((x>>8)&0xff000000) |
        ((x>>24)&0xff0000) |
        ((x>>40)&0xff00) |
        ((x>>56)&0xff));
#else
    rwRETURN(x);
#endif
}

inline uint32_t ConvertBig(const uint32_t x)
{
    rwFUNCTION();
#if (rwHOST_ENDIAN == rwBIENDIAN)
    rwRETURN(x);
#else
    rwRETURN(
        ((x&0xff)<<24) |
        ((x&0xff00)<<8) |
        ((x>>8)&0xff00) |
        ((x>>24)&0xff));
#endif
}

inline uint16_t ConvertBig(const uint16_t x)
{
    rwFUNCTION();
#if (rwHOST_ENDIAN == rwBIENDIAN)
    rwRETURN(x);
#else
    rwRETURN(
        (uint16_t)
        ( ((x&0xff)<<8) |
        ((x>>8)&0xff) ));
#endif
}

inline int32_t ConvertBig(const int32_t x)
{
    rwFUNCTION();
#if (rwHOST_ENDIAN == rwBIENDIAN)
    rwRETURN(x);
#else
    rwRETURN(
        ((x&0xff)<<24) |
        ((x&0xff00)<<8) |
        ((x>>8)&0xff00) |
        ((x>>24)&0xff));
#endif
}

inline int16_t ConvertBig(const int16_t x)
{
    rwFUNCTION();
#if (rwHOST_ENDIAN == rwBIENDIAN)
    rwRETURN(x);
#else
    rwRETURN(
        (int16_t)
        ( ((x&0xff)<<8) |
        ((x>>8)&0xff) ));
#endif
}

inline uint64_t ConvertBig(const uint64_t x)
{
    rwFUNCTION();
#if (rwHOST_ENDIAN == rwBIENDIAN)
    rwRETURN(x);
#else
    rwRETURN(
        ((x&0xff)<<56) |
        ((x&0xff00)<<40) |
        ((x&0xff0000)<<24) |
        ((x&0xff000000)<<8) |
        ((x>>8)&0xff000000) |
        ((x>>24)&0xff0000) |
        ((x>>40)&0xff00) |
        ((x>>56)&0xff));
#endif
}

inline int64_t ConvertBig(const int64_t x)
{
    rwFUNCTION();
#if (rwHOST_ENDIAN == rwBIENDIAN)
    rwRETURN(x);
#else
    rwRETURN(
        ((x&0xff)<<56) |
        ((x&0xff00)<<40) |
        ((x&0xff0000)<<24) |
        ((x&0xff000000)<<8) |
        ((x>>8)&0xff000000) |
        ((x>>24)&0xff0000) |
        ((x>>40)&0xff00) |
        ((x>>56)&0xff));
#endif
}

inline uint32_t make_id32(uint8_t a, uint8_t b, uint8_t c, uint8_t d)
{
    rwFUNCTION();
#if (rwHOST_ENDIAN == rwBIENDIAN)
    rwRETURN((static_cast<uint32_t>(a) << 24) | (static_cast<uint32_t>(b) << 16) | (static_cast<uint32_t>(c) << 8) | static_cast<uint32_t>(d));
#else
    rwRETURN((static_cast<uint32_t>(d) << 24) | (static_cast<uint32_t>(c) << 16) | (static_cast<uint32_t>(b) << 8) | static_cast<uint32_t>(a));
#endif
}

inline void PutBig(void* dst, uint32_t data, int32_t n)
{
    rwFUNCTION();
#if (rwHOST_ENDIAN == rwBIENDIAN)
    if (n==1)
    {
        ((uint8_t*)dst)[0] = (uint8_t)data;
    }
    else if (n==2)
    {
        ((uint16_t*)dst)[0] = (uint16_t)data;
    }
    else if (n==4)
    {
        ((uint32_t*)dst)[0] = (uint32_t)data;
    }
    else if (n==3)
    {
        ((uint8_t*)dst)[0] = (uint8_t)(data>>16);
        ((uint8_t*)dst)[1] = (uint8_t)(data>>8);
        ((uint8_t*)dst)[2] = (uint8_t)data;
    }
#else
    if (n==1)
    {
        ((uint8_t*)dst)[0] = (uint8_t)data;
    }
    else if (n==2)
    {
        ((uint8_t*)dst)[0] = (uint8_t)(data>>8);
        ((uint8_t*)dst)[1] = (uint8_t)data;
    }
    else if (n==4)
    {
        ((uint8_t*) dst)[0] = (uint8_t)(data>>24);
        ((uint8_t*) dst)[1] = (uint8_t)(data>>16);
        ((uint8_t*) dst)[2] = (uint8_t)(data>>8);
        ((uint8_t*) dst)[3] = (uint8_t)data;
    }
    else if (n==3)
    {
        ((uint8_t*)dst)[0] = (uint8_t)(data>>16);
        ((uint8_t*)dst)[1] = (uint8_t)(data>>8);
        ((uint8_t*)dst)[2] = (uint8_t)data;
    }
#endif
    rwRETURNVOID();
}

inline void PutLittle(void *dst, uint32_t data, int32_t n)
{
    rwFUNCTION();
#if (rwHOST_ENDIAN == rwBIENDIAN)
    if (n==1)
    {
        ((uint8_t*)dst)[0] = (uint8_t)data;
    }
    else if (n==2)
    {
        ((uint8_t*)dst)[0] = (uint8_t)data;
        ((uint8_t*)dst)[1] = (uint8_t)(data>>8);
    }
    else if (n==4)
    {
        ((uint8_t*)dst)[0] = (uint8_t)data;
        ((uint8_t*)dst)[1] = (uint8_t)(data>>8);
        ((uint8_t*)dst)[2] = (uint8_t)(data>>16);
        ((uint8_t*)dst)[3] = (uint8_t)(data>>24);
    }
    else if (n==3)
    {
        ((uint8_t*)dst)[0] = (uint8_t)data;
        ((uint8_t*)dst)[1] = (uint8_t)(data>>8);
        ((uint8_t*)dst)[2] = (uint8_t)(data>>16);
    }
#else
    if (n==1)
    {
        ((uint8_t*)dst)[0] = (uint8_t)data;
    }
    else if (n==2)
    {
        // ps2 cannot handle unaligned casts
        #if (defined(rwHOST_PS2))
            // this is slower, but works on ps2
            ((uint8_t*)dst)[0] = (uint8_t)data;
            ((uint8_t*)dst)[1] = (uint8_t)(data>>8);
        #else
            ((uint16_t*)dst)[0] = (uint16_t)data;
        #endif
    }
    else if (n==4)
    {
        // ps2 cannot handle unaligned casts
        #if (defined(rwHOST_PS2))
             // this is slower, but works on ps2
            ((uint8_t*)dst)[0] = (uint8_t)data;
            ((uint8_t*)dst)[1] = (uint8_t)(data>>8);
            ((uint8_t*)dst)[2] = (uint8_t)(data>>16);
            ((uint8_t*)dst)[3] = (uint8_t)(data>>24);
        #else
            ((uint32_t*)dst)[0] = (uint32_t)data;
        #endif

    }
    else if (n==3)
    {
        ((uint8_t*)dst)[0] = (uint8_t)data;
        ((uint8_t*)dst)[1] = (uint8_t)(data>>8);
        ((uint8_t*)dst)[2] = (uint8_t)(data>>16);
    }
#endif
    rwRETURNVOID();
}

inline uint32_t GetBig(const void* src, int32_t n)
{
    rwFUNCTION();
#if (rwHOST_ENDIAN == rwBIENDIAN)
    if (n==1)
    {
        rwRETURN((uint32_t)*(const uint8_t*)src);
    }
    else if (n==2)
    {
        rwRETURN((uint32_t)*(const uint16_t*)src);
    }
    else if (n==3)
    {
        rwRETURN((uint32_t)*(const uint32_t*)src>>8);
    }
    else if (n==4)
    {
        rwRETURN((uint32_t)*(const uint32_t*)src);
    }
#else
    if (n==1)
    {
        rwRETURN((uint32_t)*(const uint8_t*)src);
    }
    else if (n==2)
    {
        rwRETURN
            (((uint32_t)*(const uint8_t*)src)<<8) |
            (((uint32_t)*((const uint8_t*) src+1)));
    }
    else if (n==3)
    {
        rwRETURN
            (((uint32_t)*(const uint8_t*)src)<<16) |
            (((uint32_t)*((const uint8_t*)src+1))<<8) |
            (((uint32_t)*((const uint8_t*)src+2)));
    }
    else if (n==4)
    {
        rwRETURN
            (((uint32_t)*(const uint8_t*)src)<<24) |
            (((uint32_t)*((const uint8_t*)src+1))<<16) |
            (((uint32_t)*((const uint8_t*)src+2))<<8) |
            (((uint32_t)*((const uint8_t*)src+3)));
    }

#endif
    rwRETURN(0);
}

inline uint32_t GetLittle(const void* src, int32_t n)
{
    rwFUNCTION();
// NB: This works due to the cast for both bigendian
//     and little endian platforms, it's just slower.
//     However: PS2 in particular can't do unaligned
//     cast to short and this is required for
//     RwImageOperate.
#if (rwHOST_ENDIAN == rwBIENDIAN) || defined(rwHOST_PS2) || defined(rwHOST_PS3)
    if (n==1)
    {
        rwRETURN((uint32_t)*(const uint8_t*)src);
    }
    else if (n==2)
    {
        rwRETURN
            (((uint32_t)*((const uint8_t*)src+1))<<8) |
            (((uint32_t)*(const uint8_t*)src+0));
    }
    else if (n==3)
    {
        rwRETURN
            (((uint32_t)*((const uint8_t*)src+2))<<16) |
            (((uint32_t)*((const uint8_t*)src+1))<<8) |
            (((uint32_t)*(const uint8_t*)src+0));
    }
    else if (n==4)
    {
        rwRETURN
            (((uint32_t)*((const uint8_t*)src+3))<<24) |
            (((uint32_t)*((const uint8_t*)src+2))<<16) |
            (((uint32_t)*((const uint8_t*)src+1))<<8) |
            (((uint32_t)*(const uint8_t*)src+0));
    }
#else
    if (n==1)
    {
        rwRETURN((uint32_t)*(const uint8_t*)src);
    }
    else if (n==2)
    {
        rwRETURN((uint32_t)*(const uint16_t*)src);
    }
    else if (n==3)
    {
        rwRETURN((uint32_t)*(const uint32_t*)src&0x00ffffff);
    }
    else if (n==4)
    {
        rwRETURN((uint32_t)*(const uint32_t*)src);
    }
#endif

    rwRETURN(0);
}

inline void PutBig64(void* dst, uint64_t data, int32_t n)
{
    rwFUNCTION();
    while (n--)
    {
        ((uint8_t*)dst)[n]=(uint8_t)data;
        data>>=8;
    }
    rwRETURNVOID();
}

inline void PutLittle64(void *dst, uint64_t data, int32_t n)
{
    rwFUNCTION();
    for (int32_t i=0; i<n; i++)
    {
        ((uint8_t*)dst)[i]=(uint8_t)data;
        data>>=8;
    }
    rwRETURNVOID();
}

inline uint64_t GetBig64(const void* src, int32_t n)
{
    rwFUNCTION();
    uint64_t result=0;

    for (int32_t i=0; i<n; i++)
    {
        result<<=8;
        result|=((const uint8_t*)src)[i];
    }

    rwRETURN(result);
}

inline uint64_t GetLittle64(const void* src, int32_t n)
{
    rwFUNCTION();
    uint64_t result=0;

    while (n--)
    {
        result<<=8;
        result|=((const uint8_t*)src)[n];
    }

    rwRETURN(result);
}

}
}
}
