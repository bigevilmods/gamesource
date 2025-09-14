// (c) 2003-2007 Electronic Arts Inc.

#ifndef _TWIZZLE_H
#define _TWIZZLE_H

namespace Gst
{
    
int BaseOffset(int x, int y, int width, int blockwidth, int blockheight);
int Detail1Offset(int x, int y, int width);
int Detail2Offset(int x, int y, int width);

void TwiddleBase(unsigned char *source, unsigned char *dest, int width, int height, int blockwidth, int blockheight);
void TwizzleDetail1(unsigned char *source, unsigned char *dest, int width, int height);
void TwizzleDetail2(unsigned char *source, unsigned char *dest, int width, int height);

void UntwiddleBase(unsigned char *source, unsigned char *dest, int width, int height, int blockwidth, int blockheight);
void UntwizzleDetail1(unsigned char *source, unsigned char *dest, int width, int height);
void UntwizzleDetail2(unsigned char *source, unsigned char *dest, int width, int height);

static inline unsigned char Get1Bit(unsigned char *buff, int offset)
{
    int index = offset >> 3;
    int bit = offset & 0x07;
    return(static_cast<unsigned char>((buff[index]>>bit) & 1));
}

static inline unsigned char Get2Bits(unsigned char *buff, int offset)
{
    int index = offset >> 3;
    int bit = offset & 0x07;
    return(static_cast<unsigned char>((buff[index]>>bit) & 3));
}

static inline void Set1Bit(unsigned char *buff, int offset, unsigned char val)
{
    int index = offset >> 3;
    int bit = offset & 0x07;
    buff[index] = static_cast<unsigned char>((buff[index] & ~(1 << bit)) | (val << bit));
}

static inline void Set2Bits(unsigned char *buff, int offset, unsigned char val)
{
    int index = offset >> 3;
    int bit = offset & 0x07;
    buff[index] = static_cast<unsigned char>((buff[index] & ~(3 << bit)) | (val << bit));
}

} // namespace GST

#endif
