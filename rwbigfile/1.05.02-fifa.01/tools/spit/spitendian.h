// (c) Electronic Arts. All Rights Reserved.

#ifndef SPIT_ENDIAN_H
#define SPIT_ENDIAN_H

// The original _byteswap_ulong/short are Windows-only functions
// so they were re-implemented.

#include <stdlib.h>

#if !defined (WIN32)
static __inline unsigned short _byteswap_ushort(unsigned short arg1)
{
    return ((arg1 << 8) | (arg1 >> 8));
}

static __inline unsigned long _byteswap_ulong(unsigned long arg1)
{
    return (
        // 4th to 1st
        ( arg1 << 24) |
        // 3rd to 2nd
        ( (arg1 & 0x0000ff00) << 8) |
        // 2nd to 3rd
        ( (arg1 & 0x00ff0000) >> 8) |
        // 1st to 4th
        ( arg1 >> 24));
}
#endif

/* get big-endian */

static __inline unsigned int ggetm(const void *src, int bytes)
{
    if (bytes==1)
        return static_cast<unsigned int>(*reinterpret_cast<const unsigned char *>(src));
    else if (bytes==2)
        return _byteswap_ushort( *reinterpret_cast<const unsigned short *>(src) );
    else if (bytes==4)
        return _byteswap_ulong( *reinterpret_cast<const unsigned int *>(src) );
    else if (bytes==3)
        return (static_cast<unsigned int>((reinterpret_cast<const unsigned char *>(src))[0])<<16)
             | (static_cast<unsigned int>((reinterpret_cast<const unsigned char *>(src))[1])<<8)
             | (static_cast<unsigned int>((reinterpret_cast<const unsigned char *>(src))[2]));
    return 0;
}

/* put big-endian */

static __inline void gputm(void *dst, unsigned int data, int bytes)
{
    if (bytes==1)
    {
        (reinterpret_cast<unsigned char *>(dst))[0] = static_cast<unsigned char>(data);
    }
    else if (bytes==2)
    {
        (reinterpret_cast<unsigned char *>(dst))[0] = static_cast<unsigned char>(data>>8);
        (reinterpret_cast<unsigned char *>(dst))[1] = static_cast<unsigned char>(data);
    }
    else if (bytes==4)
    {
        (reinterpret_cast<unsigned char *>(dst))[0] = static_cast<unsigned char>(data>>24);
        (reinterpret_cast<unsigned char *>(dst))[1] = static_cast<unsigned char>(data>>16);
        (reinterpret_cast<unsigned char *>(dst))[2] = static_cast<unsigned char>(data>>8);
        (reinterpret_cast<unsigned char *>(dst))[3] = static_cast<unsigned char>(data);
    }
    else if (bytes==3)
    {
        (reinterpret_cast<unsigned char *>(dst))[0] = static_cast<unsigned char>(data>>16);
        (reinterpret_cast<unsigned char *>(dst))[1] = static_cast<unsigned char>(data>>8);
        (reinterpret_cast<unsigned char *>(dst))[2] = static_cast<unsigned char>(data);
    }
}

#endif // SPIT_ENDIAN_H
