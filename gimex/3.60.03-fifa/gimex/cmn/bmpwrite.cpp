// (c) 1994-2007 Electronic Arts Inc.

#include <string.h>

#include "gimex.h"
#include "bmpgimex.h"

/****************************************************************/
/*  Export Functions                                            */
/****************************************************************/

#define BMPS(X) ggetm(s+(X),1)

static int BMPrlecount(void *p, int maxcount)
{
    int ct=0;

    if (maxcount > 255)
        maxcount = 255;

    while (ct<maxcount && ((unsigned char *)p)[0]==((unsigned char *)p)[ct])
        ++ct;
    return(ct);
}

static int BMPwriteline(void *src, void *dest, int count, int bpp, int subtype, int pack, int alphabits, int redbits, int greenbits, int bluebits)
{
    unsigned char *s=(unsigned char *) src;
    unsigned char *d=(unsigned char *) dest;
    int x;

    if (bpp==15)
        bpp=16;
    if (!pack)          /* uncompressed images */
    {
        for (x=0; x<count; ++x)
        {
            if ( (bpp == 16) || (bpp == 32) )
            {
                unsigned int aval;
                unsigned int rval;
                unsigned int gval;
                unsigned int bval;
                unsigned int val;

                aval = (((ARGB *) s)->a * ((1<<alphabits) - 1) + 128) / 255;
                rval = (((ARGB *) s)->r * ((1<<redbits)   - 1) + 128) / 255;
                gval = (((ARGB *) s)->g * ((1<<greenbits) - 1) + 128) / 255;
                bval = (((ARGB *) s)->b * ((1<<bluebits)  - 1) + 128) / 255;

                val = aval << (32-alphabits) |
                      rval << (32-alphabits-redbits) |
                      gval << (32-alphabits-redbits-greenbits) |
                      bval << (32-alphabits-redbits-greenbits-bluebits);


                if (bpp == 32)                  /* 32 bpp */
                {
                    d[0] = ((unsigned char*)(&val))[0];
                    d[1] = ((unsigned char*)(&val))[1];
                    d[2] = ((unsigned char*)(&val))[2];
                    d[3] = ((unsigned char*)(&val))[3];
                    d += 4;
                }
                else                            /* 16 bpp */
                {
                    d[0] = ((unsigned char*)(&val))[2];
                    d[1] = ((unsigned char*)(&val))[3];
                    d += 2;
                }
                s += 4;
            }
            else if (bpp==24)
            {
                d[0] = ((ARGB *) s)->b;
                d[1] = ((ARGB *) s)->g;
                d[2] = ((ARGB *) s)->r;
                d += 3;
                s += 4;
            }
            else if (bpp==8)
            {
                *d++ = *s++;
            }
            else if (bpp==4)
            {
                if (x&1)
                {
                    *d |= (*s&15);
                    ++d;
                }
                else
                {
                    *d = (unsigned char)((*s&15)<<4);
                    if (x==count-1)
                        ++d;
                }
                ++s;
            }
            else if (bpp==1)
            {
                if ((x&7)==0)
                    *d = 0;

                *d |= (*s&1)<<(7-(x&7));

                if ((x&7)==7 || x==count-1)
                {
                    ++d;
                }
                ++s;
            }
        }
        while ((long long)d&3)
            *d++ = 0;
    }
    else                /* rle 8 */
    {
        int ct;
        int ct2;

        while (count > 0)
        {

            /* not in a literal... if either current or next run will be 3,
              encode current as run.
              End of line must be encoded with run, since literal cant do 1 or 2 */

            ct  = BMPrlecount(s, count);
            ct2 = BMPrlecount(s+ct, count-ct); // next count (could be 0)

            if (count<3 || (ct+ct2>=4))
            {
                if (count<3)
                {
                    ct = 1;
                    if (count==2 && (BMPS(0) == BMPS(1)))
                        ct = 2;
                }

                *d++ = (unsigned char) ct;
                *d++ = *s;

                s += ct;
                count -= ct;
            }
            else
            {
                int ct3;

                // literals of odd length are padded out to even,
                // so its only worthwhile to do even lengths
                // if in literal, and next run is a literal too, then run
                // must be 5 or more to be worth while
                // if next run, then current run must be 3 or more.

                for (ct=4; ct<count && ct<254; ct+=2)
                {
                    ct2 = BMPrlecount(s+ct, count-ct);        // rle from here
                    ct3 = BMPrlecount(s+ct2, count-(ct+ct2)); // next rle after that

                    if ((ct2>=3 && ct3>=2) || (ct2>=5))
                        break;
                }
                if (ct>=(count-2))
                    ct = count;
                if (ct>254)
                    ct = 254;
                if (ct>=3)
                {
                    /* write out count */
                    *d++ = 0;
                    *d++ = (unsigned char) ct;
                    count -= ct;

                    /* copy string of pixels */

                    while (ct--)
                    {
                        *d++ = *s++;
                    }
                    if (((long long)d)&1)
                    {
                        *d++ = 0;
                    }
                }
            }
        }

        *d++ = 0;
        *d++ = 0;
    }
    return(d-(unsigned char *)dest);
}

int GCALL BMP_wopen(GINSTANCE **gxptr, GSTREAM *g, const char *pathname, int numframes)
{
    GINSTANCE *gx;
    int ok=0;

    gx = (GINSTANCE *) galloc(sizeof(GINSTANCE));
    if (gx)
    {
        memset(gx, 0, sizeof(GINSTANCE));
        gx->gstream = g;
        gx->framenum = 0;
        *gxptr = gx;
        ok = 1;
    }
    return(ok);
}

int GCALL BMP_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes)
{
    char *linebuf;
    int width;
    int height;
    int bpp;
    int bpl;
    int subtype;
    int y;
    int ok;
    int i;
    int packed;
    int biCompression;
    int headersize;
    int ntheadersize;
    int palettesize;
    int bitmapsize;
    char *s;
    BMPHeaderStruct bmpheader;
    unsigned char palbuf[256*4];
    int redbits;
    int greenbits;
    int bluebits;
    int alphabits;

    int colours = gmin(ginfo->numcolours,256);
    width = ginfo->width;
    height = ginfo->height;

    bpp = ginfo->originalbpp;
/*    if (ginfo->bpp <= 8)
        bpp = gmin(ginfo->originalbpp, 8);
    else
        bpp = ginfo->redbits + ginfo->greenbits + ginfo->bluebits + ginfo->alphabits;
*/
    subtype = ginfo->subtype;

         if (bpp<=1)  bpp=1;
    else if (bpp<=4)  bpp=4;
    else if (bpp<=8)  bpp=8;
    else if (bpp<=16) bpp=16;
    else if (bpp<=24) bpp=24;
    else              bpp=32;
    if (bpp>8) colours = 0;

    /* Init header info */
    headersize = 40;
    ntheadersize = 0;
    packed = 0;
    biCompression = 0;  /* RGB */
    memset(&bmpheader,0,sizeof(bmpheader));

    alphabits = ginfo->alphabits;
    redbits   = ginfo->redbits;
    greenbits = ginfo->greenbits;
    bluebits  = ginfo->bluebits;

    if (subtype == BMPWIN95)               /* Win 95 */
    {
        headersize = 108;

        if (ginfo->alphabits == 0)                  /* No alpha */
        {
            if ( (bpp == 16) &&
                 (ginfo->redbits <= 5) &&
                 (ginfo->bluebits <= 5) &&
                 (ginfo->greenbits <= 6) )
            {

                biCompression = 3;  /* Bitfields */

                if ( ginfo->greenbits <= 5 )        /* 0555 */
                {
                    redbits = greenbits = bluebits = 5; alphabits = 1;
                    BMPPUTFIELD(bmpheader.bmi.win.biRedMask,  0x7c00);  /* Mask identifying bits of red channel */
                    BMPPUTFIELD(bmpheader.bmi.win.biGreenMask,0x03e0);  /* Mask identifying bits of green channel */
                    BMPPUTFIELD(bmpheader.bmi.win.biBlueMask, 0x001f);  /* Mask identifying bits of blue channel */
                    BMPPUTFIELD(bmpheader.bmi.win.biAlphaMask,0x0000);  /* Mask identifying bits of alpha channel */
                }
                else                                /* 0565 */
                {
                    redbits = bluebits = 5; greenbits = 6; alphabits = 0;
                    BMPPUTFIELD(bmpheader.bmi.win.biRedMask,  0xf800);  /* Mask identifying bits of red channel */
                    BMPPUTFIELD(bmpheader.bmi.win.biGreenMask,0x07e0);  /* Mask identifying bits of green channel */
                    BMPPUTFIELD(bmpheader.bmi.win.biBlueMask, 0x001f);  /* Mask identifying bits of blue channel */
                    BMPPUTFIELD(bmpheader.bmi.win.biAlphaMask,0x0000);  /* Mask identifying bits of alpha channel */
                }
            }
            else if (bpp == 24)                     /* 0888 */
            {
                redbits = bluebits = greenbits = 8; alphabits = 0;

                /* Fill in the channel mask anyway */
                BMPPUTFIELD(bmpheader.bmi.win.biRedMask,  0x00ff0000);  /* Mask identifying bits of red channel */
                BMPPUTFIELD(bmpheader.bmi.win.biGreenMask,0x0000ff00);  /* Mask identifying bits of green channel */
                BMPPUTFIELD(bmpheader.bmi.win.biBlueMask, 0x000000ff);  /* Mask identifying bits of blue channel */
                BMPPUTFIELD(bmpheader.bmi.win.biAlphaMask,0x00000000);  /* Mask identifying bits of alpha channel */
            }
        }
        /* Any alpha means automatically round to 32bpp */
        else if (bpp > 8)                          /* 8888 */
        {
            bpp = 32;
            redbits = greenbits = bluebits = alphabits = 8;

            /* Fill in the channel mask anyway */
            BMPPUTFIELD(bmpheader.bmi.win.biRedMask,  0x00ff0000);  /* Mask identifying bits of red channel */
            BMPPUTFIELD(bmpheader.bmi.win.biGreenMask,0x0000ff00);  /* Mask identifying bits of green channel */
            BMPPUTFIELD(bmpheader.bmi.win.biBlueMask, 0x000000ff);  /* Mask identifying bits of blue channel */
            BMPPUTFIELD(bmpheader.bmi.win.biAlphaMask,0xff000000);  /* Mask identifying bits of alpha channel */
        }

        BMPPUTFIELD(bmpheader.bmi.win.biCSType,1);              /* Colour space type (1=normal) */
        BMPPUTFIELD(bmpheader.bmi.win.biRedX,0);                /* X coordinate of red endpoint */
        BMPPUTFIELD(bmpheader.bmi.win.biRedY,0);                /* Y coordinate of red endpoint */
        BMPPUTFIELD(bmpheader.bmi.win.biRedZ,0);                /* Z coordinate of red endpoint */
        BMPPUTFIELD(bmpheader.bmi.win.biGreenX,0);              /* X coordinate of green endpoint */
        BMPPUTFIELD(bmpheader.bmi.win.biGreenY,0);              /* Y coordinate of green endpoint */
        BMPPUTFIELD(bmpheader.bmi.win.biGreenZ,0);              /* Z coordinate of green endpoint */
        BMPPUTFIELD(bmpheader.bmi.win.biBlueX,0);               /* X coordinate of blue endpoint */
        BMPPUTFIELD(bmpheader.bmi.win.biBlueY,0);               /* Y coordinate of blue endpoint */
        BMPPUTFIELD(bmpheader.bmi.win.biBlueZ,0);               /* Z coordinate of blue endpoint */
        BMPPUTFIELD(bmpheader.bmi.win.biGammaRed,0);            /* Gamma red coordinate scale value */
        BMPPUTFIELD(bmpheader.bmi.win.biGammaGreen,0);          /* Gamma green coordinate scale value */
        BMPPUTFIELD(bmpheader.bmi.win.biGammaBlue,0);           /* Gamma blue coordinate scale value */
    }
    else if (subtype == BMPWINNT)          /* Win NT */
    {
        int redmask;
        int bluemask;
        int greenmask;
        int alphamask;

        biCompression = 3;  /* Bitfields */
        ntheadersize = 12;

        if (bpp == 16)
        {
            if (ginfo->alphabits + ginfo->redbits + ginfo->greenbits + ginfo->bluebits > 16)
            {
                bpp = 32;
            }
        }

        alphabits = gmin(ginfo->alphabits, 8);
        redbits   = gmin(ginfo->redbits, 8);
        greenbits = gmin(ginfo->greenbits, 8);
        bluebits  = gmin(ginfo->bluebits, 8);

        alphamask = ((1<<alphabits) - 1) << (redbits+greenbits+bluebits);
        redmask   = ((1<<redbits) - 1)   << (greenbits+bluebits);
        greenmask = ((1<<greenbits) - 1) << (bluebits);
        bluemask  = (1<<bluebits) - 1;

        BMPPUTFIELD(bmpheader.bmi.win.biAlphaMask,alphamask);   /* Mask identifying bits of alpha channel */
        BMPPUTFIELD(bmpheader.bmi.win.biRedMask,  redmask);     /* Mask identifying bits of red channel */
        BMPPUTFIELD(bmpheader.bmi.win.biGreenMask,greenmask);   /* Mask identifying bits of green channel */
        BMPPUTFIELD(bmpheader.bmi.win.biBlueMask, bluemask);    /* Mask identifying bits of blue channel */
    }
    else                                /* Win 3.1 */
    {
        if ( bpp == 16 )                /* 555 */
        {
            if (subtype == BMPWIN31a)   /* 565 */
            {
                redbits = bluebits = 5; greenbits = 6; alphabits = 0;
            }
            else
            {
                redbits = greenbits = bluebits = 5; alphabits = 1;
            }
        }
        else if ( bpp == 32 )            /* 888 */
        {
            redbits = greenbits = bluebits = alphabits = 8;
        }
        else if ( bpp == 8 )             /* 555 */
        {
            if (ginfo->packed)
            {
                packed = 1;
                biCompression = 1;  /* 8 bit RLE */
            }
        }
    }


    bpl = ((width*bpp+31)&~31)/8;
    palettesize = 4 * colours;
    bitmapsize = 0;  //  bpl * height;

    // compress once to get framesize.  Done twice to avoid extra buffering awhile allowing sequential writes so that this module can be used for clipboard
    const int MAXLINEBUF = (bpl+4)*3+32;
    linebuf = (char *) galloc(MAXLINEBUF); // bpl can change by up to 2 during the write line so we need to allocate more space here.
    if (linebuf)
    {
        for (y=0; y<height; ++y)
        {
            s = (source+(height-y-1)*rowbytes);
            bpl = BMPwriteline(s, linebuf, width, bpp, subtype, packed, alphabits, redbits, greenbits, bluebits);
            bitmapsize += bpl;
        }
    }

    bmpheader.bfType[0] = 'B';         /* The bytes "BM" (for bitmap) */
    bmpheader.bfType[1] = 'M';
    BMPPUTFIELD(bmpheader.bfSize, 14+headersize+ntheadersize+palettesize+bitmapsize);   /* Total size of the file */
    BMPPUTFIELD(bmpheader.bfReserved1, 0);
    BMPPUTFIELD(bmpheader.bfReserved2, 0);
    BMPPUTFIELD(bmpheader.bfOffbits, 14+headersize+ntheadersize+palettesize);/* Offset to the bits from beginning of the file */
    BMPPUTFIELD(bmpheader.bmi.win.biSize, headersize);          /* Size of the BITMAPINFOHEADER structure in bytes */
    BMPPUTFIELD(bmpheader.bmi.win.biWidth, width);              /* Width of the bitmap in pixels */
    BMPPUTFIELD(bmpheader.bmi.win.biHeight, height);            /* Height of the bitmap in pixels */
    BMPPUTFIELD(bmpheader.bmi.win.biPlanes, 1);                 /* Number of image planes */
    BMPPUTFIELD(bmpheader.bmi.win.biBitCount, bpp);             /* Colour bits per pixel */
    BMPPUTFIELD(bmpheader.bmi.win.biCompression, biCompression); /* Compression scheme (0 or 3 for none) */
    BMPPUTFIELD(bmpheader.bmi.win.biSizeImage, bitmapsize);     /* Size of bitmap bits in bytes (only required if compression is used) */
    BMPPUTFIELD(bmpheader.bmi.win.biXPelsPerMeter, (int)(ginfo->dpi*39.370079f+0.5f)); /* Horizontal resolution in pixels per meter */
    BMPPUTFIELD(bmpheader.bmi.win.biYPelsPerMeter, (int)(ginfo->dpi*39.370079f+0.5f)); /* Vertical resolution in pixels per meter */
    BMPPUTFIELD(bmpheader.bmi.win.biClrUsed, colours);          /* Number of colours used in image */
    BMPPUTFIELD(bmpheader.bmi.win.biClrImportant, 0);           /* Number of important colours used in image */

    ok = gwrite(gx->gstream, (char *) &bmpheader, 14+headersize+ntheadersize);

    if (bpp<=8 && colours)
    {

        for (i=0; i<colours; ++i)
        {
            palbuf[i*4+0] = ginfo->colourtbl[i].b;
            palbuf[i*4+1] = ginfo->colourtbl[i].g;
            palbuf[i*4+2] = ginfo->colourtbl[i].r;


            // changes to support alpha channel
            if(ginfo->alphabits == 0)           // if user is taking out alpha(ie. no alphabits)
                palbuf[i*4+3] = 0xFF;           // set to fully opaque
            else
                palbuf[i*4+3] = ginfo->colourtbl[i].a;

        }




        ok = gwrite(gx->gstream, (char *) palbuf, (int)colours*4);
    }

    if (linebuf)
    {
        memset(linebuf,0,MAXLINEBUF);
        for (y=0; y<height && ok; ++y)
        {
            s = (source+(height-y-1)*rowbytes);

            bpl = BMPwriteline(s, linebuf, width, bpp, subtype, packed, alphabits, redbits, greenbits, bluebits);
            ok = gwrite(gx->gstream,linebuf,bpl);

        }

        gfree(linebuf);
    }

    return(ok);
}

int GCALL BMP_wclose(GINSTANCE *gx)
{
    int ok=0;

    if (gx)
        ok = gfree((void *) gx);

    return(ok);
}


