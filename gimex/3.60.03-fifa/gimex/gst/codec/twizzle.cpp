// (c) 2003-2007 Electronic Arts Inc.

namespace Gst
{

static inline unsigned char Get1Bit(const unsigned char *buff, int offset)
{
    int index = offset >> 3;
    int bit = offset & 0x07;
    return(static_cast<unsigned char>((buff[index]>>bit) & 1));
}

static inline unsigned char Get2Bits(const unsigned char *buff, int offset)
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


/*
;
; Swizzling notes:
;
; The formulas used by the hardware to store a pixel within a page are
; given below. In these formulas, xn and yn represent the nth bit of x and y
; and the symbol + represents bit concatenation.
;
; offset32 = x5+y4+x4+y3+x3+y2+y1+x2+x1+y0+x0+00000
; offset16 = y5+x5+y4+x4+y3+y2+y1+x2+x1+y0+x0+x3+0000
; offset8 =  x6+y5+x5+y4+x4+y3+y2+(x2^y1^y2)+x1+y0+x0+x3+y1+000
; offset4 =  y6+x6+y5+x5+y4+y3+y2+(x2^y1^y2)+x1+y0+x0+x4+x3+y1+00
;
; We can use these formulas to convert the x,y coordinate of a pixel in
; a 4 or 8 bit unswizzled texture to the x,y,z coordinate of the same pixel
; in a 16 or 32 bit swizzled texture (z represents the position of the
; unswizzled pixel inside the swizzled pixel). If we assume that the page
; layout is the same in both the swizzled and unswizzled texture, then we
; get the following conversion for both 4->16 and 8->32 bit swizzling:
;
; newx = ...+x7+x6+x5+x4+(x2^y1^y2)+x1+x0
; newy = ...+y6+y5+y4+y3+y2+y0
; newz = x3+y1
;
*/


//---------------------------------------------------------------------------
//
// Computes the offset (in bits) of a texel in a twiddled GST base
// texture given the x and y coordinates of the same texel in a linear
// GST base texture.
//
// width is the width of the base texture and blockwidth and blockheight
// are the parameters used to compress the texture.
//
//---------------------------------------------------------------------------

int BaseOffset(int x, int y, int width, int blockwidth, int blockheight)
{
    x *= blockwidth;
    y *= blockheight;
    int x1 = ((x&~0x0f)>>1) + ((x&0x07)^((y&0x02)<<1)^(y&0x04));
    int y1 = ((y&~0x03)>>1) + (y&0x01);
    int z1 = ((x&0x08)>>2) + ((y&0x02)>>1);
    x1 /= blockwidth;
    y1 /= blockheight;
    return(32*(y1*width/2+x1)+8*z1);
}

//---------------------------------------------------------------------------
//
// Computes the offset (in bits) of a texel in a twizzled GST 1 bit detail
// texture given the x and y coordinates of the same texel in a linear
// GST 1 bit detail texture.
//
// width is the width of the detail texture.
//
//---------------------------------------------------------------------------

int Detail1Offset(int x, int y, int width)
{
    int x1 = ((x&~0x0f)>>1) + ((x&0x07)^((y&0x02)<<1)^(y&0x04));
    int y1 = ((y&~0x03)>>1) + (y&0x01);
    int z1 = ((x&0x08)>>2) + ((y&0x02)>>1);
    int x2 = ((x1&~0x0f)>>1) + ((x1&0x07)^((y1&0x02)<<1)^(y1&0x04));
    int y2 = ((y1&~0x03)>>1) + (y1&0x01);
    int z2 = ((x1&0x08)>>2) + ((y1&0x02)>>1);
    return(16*(y2*width/4+x2)+4*z2+z1);
}

//---------------------------------------------------------------------------
//
// Computes the offset (in bits) of a texel in a twizzled GST 2 bit detail
// texture given the x and y coordinates of the same texel in a linear
// GST 2 bit detail texture.
//
// width is the width of the detail texture.
//
//---------------------------------------------------------------------------

int Detail2Offset(int x, int y, int width)
{
    int x1 = ((x&~0x0f)>>1) + ((x&0x07)^((y&0x02)<<1)^(y&0x04));
    int y1 = ((y&~0x03)>>1) + (y&0x01);
    int z1 = ((x&0x08)>>2) + ((y&0x02)>>1);
    int x2 = ((x1&~0x0f)>>1) + ((x1&0x07)^((y1&0x02)<<1)^(y1&0x04));
    int y2 = ((y1&~0x03)>>1) + (y1&0x01);
    int z2 = ((x1&0x08)>>2) + ((y1&0x02)>>1);
    return(32*(y2*width/4+x2)+8*z2+2*z1);
}


//---------------------------------------------------------------------------
//
// Twiddle a linear GST base texture so that it can be used on the PS2.
//
// width, height, blockwidth and blockheight are the parameters of the
// compressed texture, so the source base texture is a width/blockwidth by
// height/blockheight array of bytes and the destination base texture is
// a width/(2*blockwidth) by height/(2*blockheight) array of 32 bit pixels.
//
//---------------------------------------------------------------------------

void TwiddleBase(const unsigned char *source, unsigned char *dest, int width, int height,
                 int blockwidth, int blockheight)
{
    int basewidth = width/blockwidth;
    int baseheight = height/blockheight;
    for (int y = 0; y < baseheight; ++y)
    {
        for (int x = 0; x < basewidth; ++x)
        {
            unsigned char data = *source++;
            int offset = BaseOffset(x, y, basewidth, blockwidth, blockheight);
            dest[offset>>3] = data;
        }
    }
}

//---------------------------------------------------------------------------
//
// Twizzle a linear GST 1 bit detail texture so that it can be used on the PS2.
//
// width and height are the parameters of the compressed texture, so the source
// detail texture is a width/8 by height array of bytes and the destination
// detail texture is a width/2 by height/2 array of 4 bit pixels, which must
// be loaded as a width/4 by height/4 array of 16 bit pixels.
//
//---------------------------------------------------------------------------

void TwizzleDetail1(const unsigned char *source, unsigned char *dest, int width, int height)
{
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; x += 8)
        {
            unsigned char data = *source++;
            for (int xfine = 0; xfine < 8; ++xfine)
            {
                int offset = Detail1Offset(x+xfine, y, width);
                Set1Bit(dest, offset, data&1);
                data >>= 1;
            }
        }
    }
}

//---------------------------------------------------------------------------
//
// Twizzle a linear GST 2 bit detail texture so that it can be used on the PS2.
//
// width and height are the parameters of the compressed texture, so the source
// detail texture is a width/4 by height array of bytes and the destination
// detail texture is a width/2 by height/2 array of 8 bit pixels, which must
// be loaded as a width/4 by height/4 array of 32 bit pixels.
//
//---------------------------------------------------------------------------

void TwizzleDetail2(const unsigned char *source, unsigned char *dest, int width, int height)
{
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; x += 4)
        {
            unsigned char data = *source++;
            for (int xfine = 0; xfine < 4; ++xfine)
            {
                int offset = Detail2Offset(x+xfine, y, width);
                Set2Bits(dest, offset, data&3);
                data >>= 2;
            }
        }
    }
}

//---------------------------------------------------------------------------
//
// Untwiddle a PS2 GST base texture to make it linear.
//
//---------------------------------------------------------------------------

void UntwiddleBase(const unsigned char *source, unsigned char *dest, int width, int height,
                   int blockwidth, int blockheight)
{
    int basewidth = width/blockwidth;
    int baseheight = height/blockheight;
    for (int y = 0; y < baseheight; ++y)
    {
        for (int x = 0; x < basewidth; ++x)
        {
            int offset = BaseOffset(x, y, basewidth, blockwidth, blockheight);
            unsigned char data = source[offset>>3];
            *dest++ = data;
        }
    }
}

//---------------------------------------------------------------------------
//
// Untwizzle a PS2 GST 1 bit detail texture to make it linear.
//
//---------------------------------------------------------------------------

void UntwizzleDetail1(const unsigned char *source, unsigned char *dest, int width, int height)
{
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; x += 8)
        {
            unsigned char data = 0;
            for (int xfine = 7; xfine >= 0; --xfine)
            {
                int offset = Detail1Offset(x+xfine, y, width);
                data = (data << 1) + Get1Bit(source, offset);
            }
            *dest++ = data;
        }
    }
}

//---------------------------------------------------------------------------
//
// Untwizzle a PS2 GST 2 bit detail texture to make it linear.
//
//---------------------------------------------------------------------------

void UntwizzleDetail2(const unsigned char *source, unsigned char *dest, int width, int height)
{
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; x += 4)
        {
            unsigned char data = 0;
            for (int xfine = 3; xfine >= 0; --xfine)
            {
                int offset = Detail2Offset(x+xfine, y, width);
                data = (data << 2) + Get2Bits(source, offset);
            }
            *dest++ = data;
        }
    }
}

} // end of Gst namespace
