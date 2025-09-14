// (c) 1994-2007 Electronic Arts Inc.

#include <string.h>
#include "gimex.h"
#include "tm2gimex.h"

static __inline int TM2writeline(unsigned char *d,const unsigned char *s,int width,int bpp)
{
    int ok=1;
    int i;

    switch (bpp)
    {
        case 4:     /* 4 bit indexed image */
            for (i=0; i<width; ++i)
            {
                if (i&1)
                    d[i/2] |= (s[i]&15)<<4;
                else
                    d[i/2] = s[i]&15;
            }
            break;

        case 8:     /* 8 bit indexed image */
            for (i=0; i<width; ++i)
            {
                *d++ = *s++;
            }
            break;


        case 16:    /* 16 bit direct image */
            for (i=0; i<width; ++i)
            {
                int a = ((ARGB *) s)->a>>7;
                int r = ((ARGB *) s)->r>>3;
                int g = ((ARGB *) s)->g>>3;
                int b = ((ARGB *) s)->b>>3;
                *(unsigned short *) d = (unsigned short) ((a<<15)|(b<<10)|(g<<5)|(r));
                s += 4;
                d += 2;
            }
            break;

        case 24:    /* 24 bit direct image */
            for (i=0; i<width; ++i)
            {
                d[0] = ((ARGB *) s)->r;
                d[1] = ((ARGB *) s)->g;
                d[2] = ((ARGB *) s)->b;
                s += 4;
                d += 3;
            }
            break;

        case 32:    /* 32 bit direct image */
            for (i=0; i<width; ++i)
            {
                d[3] = ((ARGB *) s)->a;
                d[2] = ((ARGB *) s)->b;
                d[1] = ((ARGB *) s)->g;
                d[0] = ((ARGB *) s)->r;
                s += 4;
                d += 4;
            }
            break;
    }

    return(ok);
}

static int TIM2_BppToType(int bpp)
{
    switch (bpp)
    {
        case 4: return 4;
        case 8: return 5;
        case 16: return 1;
        case 24: return 2;
        case 32: return 3;
        default: ;
    }
    return 0;
}

/****************************************************************/
/*  Export Functions                                            */
/****************************************************************/

int GCALL TM2_wopen(GINSTANCE **gxptr, GSTREAM *g, const char *pathname, int numframes)
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

        TM2HeaderStruct tm2header;
        memset(&tm2header,0,sizeof(tm2header));

        // 00000: 54494d32 04000100 00000000 00000000  TIM2............

        gputm(&tm2header.FileId, GMAKEID('T','I','M','2'),4);    /* signature */
        TM2PUTFIELD(tm2header.FormatVersion, 4);  /* always 4 */
        TM2PUTFIELD(tm2header.FormatId, 4);       /* always 1 */
        TM2PUTFIELD(tm2header.Pictures, 1);       /* frame count */

        ok = gwrite(gx->gstream,&tm2header,16);
    }
    return(ok);
}

int GCALL TM2_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes)
{
    int bpp;
    int bpl;
   // int x;
    int y;
    int ok=0;
    int i;
    TIM2_PICTUREHEADER tm2info;

    bpp = ginfo->originalbpp;
         if (bpp<=4)  bpp=4;
    else if (bpp<=8)  bpp=8;
    else if (ginfo->alphabits<=1
          && ginfo->redbits  <=5
          && ginfo->greenbits<=5
          && ginfo->bluebits <=5) bpp=16;
    else if (ginfo->alphabits==0) bpp=24;
    else                          bpp=32;

    bpl = ginfo->width*bpp/8;

    int colours=0;
    if (bpp<=8)
    {
        colours = (ginfo->numcolours+31)&~31;
    }

// john.tm2:
// 00000: 54494d32 04000100 00000000 00000000  TIM2............
// 00010: 30000200 00000000 00000200 30000000  0...........0...
// 00020: 00010001 00010001 00002020 02000000  ..........  ....
// 00030: 60020000 00000000 00000000 00000000  `...............
// 00040: 33fb33fb 13fb13fb 33fb33fb 33fb33fb  3.3.....3.3.3.3.
// 00050: 33fb33fb 33fb34fb 34fb34fb 34fb34fb  3.3.3.4.4.4.4.4.

    /* TIM2 PICTURE HEADER structure 48 bytes */

    TM2PUTFIELD(tm2info.TotalSize,colours*4+bpl*ginfo->height+sizeof(tm2info));   // Total size in bytes of all picture data
    TM2PUTFIELD(tm2info.ClutSize,colours*4);             // Size in bytes of CLUT data part
    TM2PUTFIELD(tm2info.ImageSize,bpl*ginfo->height);   // Size in bytes of image data part
    TM2PUTFIELD(tm2info.HeaderSize,sizeof(tm2info));    // Size in bytes of info part
    TM2PUTFIELD(tm2info.ClutColors,colours);    // Total number of colors in CLUT data part
    TM2PUTFIELD(tm2info.PictFormat,0);          // Picture format ID (always 0)
    TM2PUTFIELD(tm2info.MipMapTextures,0);                      // Number of MIPMAP textures
    TM2PUTFIELD(tm2info.ClutType,colours?TIM2_BppToType(32):0); // CLUT data part type
    TM2PUTFIELD(tm2info.ImageType,TIM2_BppToType(bpp));         // Image data part type
    TM2PUTFIELD(tm2info.ImageWidth,ginfo->width);               // Picture width
    TM2PUTFIELD(tm2info.ImageHeight,ginfo->height);             // Picture height
    TM2PUTFIELD(tm2info.GsTex0,0);      // GS TEX0 register data
    TM2PUTFIELD(tm2info.GsTex1,0);      // GS TEX1 register data
    TM2PUTFIELD(tm2info.GsRegs,0);      // GS TEXA, FBA, PABE register data
    TM2PUTFIELD(tm2info.GsTexClut,0);   // GS TEXCLUT register data

    ok = gwrite(gx->gstream,&tm2info,sizeof(tm2info)/*48*/);

    unsigned char *linebuf = (unsigned char *) galloc(bpl);
    if (linebuf)
    {
        ok = 1;
        for (y=0; y<ginfo->height && ok; ++y)
        {
            unsigned char *s = (unsigned char *) (source+rowbytes*y);

            TM2writeline(linebuf,(const unsigned char *)s,ginfo->width,bpp);

            ok = gwrite(gx->gstream,linebuf,bpl);

#if 0
            if (bpp==8)
            {
                ok = gwrite(gx->gstream,(char *)s,bpl);
            }
            else
            {
                for (x=0; x<ginfo->width; ++x)
                {
                    lineptr[0] = s->r;
                    lineptr[1] = s->g;
                    lineptr[2] = s->b;
                    lineptr[3] = s->a;
                    ++s;
                    lineptr += 4;
                }
            }
#endif
        }

        gfree(linebuf);
    }

    if (colours)
    {
        unsigned char palbuf[256*4];

        memset(&palbuf,0,sizeof(palbuf));

        for (i=0; i<colours; ++i)
        {
            int j = i;
            if (bpp==8 && (((j&0x18)==0x08) || ((j&0x18)==0x10)))  // swap 8..15 range with 16..23
                j ^= 0x18;

            palbuf[j*4+0] = ginfo->colourtbl[i].r;
            palbuf[j*4+1] = ginfo->colourtbl[i].g;
            palbuf[j*4+2] = ginfo->colourtbl[i].b;
            palbuf[j*4+3] = ginfo->colourtbl[i].a;
        }
        ok = gwrite(gx->gstream, (char *) palbuf, colours*4);
    }

    ++gx->framenum;

    return(ok);
}

int GCALL TM2_wclose(GINSTANCE *gx)
{
    int ok=0;

    if (gx)
    {
        TM2HeaderStruct tm2header;
        memset(&tm2header,0,sizeof(tm2header));

        // 00000: 54494d32 04000100 00000000 00000000  TIM2............

        gputm(&tm2header.FileId, GMAKEID('T','I','M','2'),4);    /* signature */
        TM2PUTFIELD(tm2header.FormatVersion, 4);  /* always 4 */
        TM2PUTFIELD(tm2header.FormatId, 4);       /* always 1 */
        TM2PUTFIELD(tm2header.Pictures, gx->framenum);       /* frame count */

        gseek(gx->gstream,0);
        ok = gwrite(gx->gstream,&tm2header,16);

        ok = gfree((void *) gx);
    }

    return(ok);
}

