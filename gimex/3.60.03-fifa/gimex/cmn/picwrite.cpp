// (c) 1994-2007 Electronic Arts Inc.

#include <string.h>
#include "gimex.h"
#include "picgimex.h"


/****************************************************************/
/*  Internal Structures                                         */
/****************************************************************/

typedef struct _PICTXRECT
{
    char left[2];
    char top[2];
    char right[2];
    char bottom[2];
} PICTX_RECT;

typedef struct _PICTXHDR1
{
    char       FileSize1[2];
    PICTX_RECT Frame;
    char       VerOpcode[2];
    char       VerNum[2];
} PICTXHDR1;

/* Header 2 for Version 2 only */

typedef struct _PICTXHDR2
{
    char       HdrOpcode[2];       /* Version 2 Header opcode */
    char       HdrOpcodeData[2];
    char       Reserved1[2];
    char       HorzPixPerInch[4];
    char       VertPixPerInch[4];
    PICTX_RECT Frame;
    char       Reserved2[4];
} PICTXHDR2;

typedef struct _PICTXPIXMAP
{
    char       BaseAddr[4];
    char       RowBytes[2];
    PICTX_RECT Bounds;
    char       Version[2];
    char       PkngType[2];
    char       PkdSize[4];
    char       SrcXRes[4]; /* fixed point */
    char       SrcYRes[4]; /* fixed point */
    char       PixType[2];
    char       PixSize[2];
    char       NumPlanes[2];
    char       CompSize[2];
    char       pmTable[2];
    char       pmReserved[2];
} PICTX_PIXMAP;

/****************************************************************/
/*  Export Functions                                            */
/****************************************************************/

int GCALL PIC_wopen(GINSTANCE **gxptr, GSTREAM *g, const char *pathname, int numframes)
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

int GCALL PIC_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes)
{
    int ok;
    int x;
    int y;
    int width;
    int height;
    int bpp;
    int commentlen;
    int pictrowbytes;
    int flen;
    char buf[PICTHEADERSIZE];
    char *s;
    char *d;
    char *linebuf;
    int idpi;
    PICTXHDR1 pichdr1;
    PICTXHDR2 pichdr2;
    PICTX_PIXMAP pixmap;

    width = ginfo->width;
    height = ginfo->height;
    bpp = ginfo->originalbpp;

    pictrowbytes = ((width+3)&~3)*4;
    commentlen = 0;
    if (ginfo->comment[0])
        commentlen = ((strlen(ginfo->comment)+1+3)&~3)+6;

    /* 512 bytes unused */
    memset(buf,0,PICTHEADERSIZE);
    ok = gwrite(gx->gstream, buf, PICTHEADERSIZE);

    idpi = (int)(ginfo->dpi*65536.0f);
    if (!idpi)
        idpi = 72<<16;

    /* pict1 header */
    flen = PICTHEADERSIZE+14+26+12+commentlen+2+42+8+8+8+2+pictrowbytes*height+2;
    PICPUTFIELD(pichdr1.FileSize1, (flen-PICTHEADERSIZE)&65535);
    PICPUTFIELD(pichdr1.Frame.left, 0);
    PICPUTFIELD(pichdr1.Frame.top, 0);
    PICPUTFIELD(pichdr1.Frame.right, height);
    PICPUTFIELD(pichdr1.Frame.bottom, width);
    PICPUTFIELD(pichdr1.VerOpcode, 0x0011);
    PICPUTFIELD(pichdr1.VerNum, 0x02ff);
    ok = gwrite(gx->gstream, (char *) &pichdr1, 14);

    /* pict2 header */
    PICPUTFIELD(pichdr2.HdrOpcode, PICT_VERS2_HDR_OPCODE);
    PICPUTFIELD(pichdr2.HdrOpcodeData, (unsigned int) -2);
    PICPUTFIELD(pichdr2.Reserved1, 0);
    PICPUTFIELD(pichdr2.HorzPixPerInch, idpi);
    PICPUTFIELD(pichdr2.VertPixPerInch, idpi);
    memcpy(&pichdr2.Frame,&pichdr1.Frame,8);
    PICPUTFIELD(pichdr2.Reserved2, 0);
    ok = gwrite(gx->gstream, (char *) &pichdr2, 26);

    /* clip region */
    gputm(buf+0,PICTOP_CLIPREGION,2);
    gputm(buf+2,12-2,2);
    memcpy(buf+4,&pichdr1.Frame,8);
    ok = gwrite(gx->gstream, buf, 12);

    /* optional comment */

    if (commentlen)
    {
        gputm(buf+0,PICTOP_LONGCOMMENT,2);
        gputm(buf+2,0,2);
        gputm(buf+4,commentlen-6,2);
        strcpy(buf+6,ginfo->comment);
        ok = gwrite(gx->gstream, buf, commentlen);
    }

    /* direct bits rect */
    gputm(buf,PICTOP_DIRECTBITSRECT,2);
    ok = gwrite(gx->gstream, buf, 2);
    PICPUTFIELD(pixmap.BaseAddr,255);
    PICPUTFIELD(pixmap.RowBytes,pictrowbytes|PICT_USEPIXMAP);
    memcpy(&pixmap.Bounds,&pichdr1.Frame,8);
    PICPUTFIELD(pixmap.Version,0);
    PICPUTFIELD(pixmap.PkngType,1); /* uncompressed */
    PICPUTFIELD(pixmap.PkdSize,0);
    PICPUTFIELD(pixmap.SrcXRes,0x00480000);
    PICPUTFIELD(pixmap.SrcYRes,0x00480000);
    PICPUTFIELD(pixmap.PixType,0x0010);
    PICPUTFIELD(pixmap.PixSize,32);
    PICPUTFIELD(pixmap.NumPlanes,4);
    PICPUTFIELD(pixmap.CompSize,8);
    PICPUTFIELD(pixmap.pmTable,0);
    PICPUTFIELD(pixmap.pmReserved,0);
    ok = gwrite(gx->gstream, (char *) &pixmap, 42);

    /* palette goop */
    memset(buf,0,8);
    ok = gwrite(gx->gstream, buf, 8);

    memcpy(&pixmap.Bounds,&pichdr1.Frame,8);

    ok = gwrite(gx->gstream, &pixmap.Bounds, 8);    /* source rect */
    ok = gwrite(gx->gstream, &pixmap.Bounds, 8);    /* dest rect */

    gputm(buf,0x0040,2);             /* umode */
    ok = gwrite(gx->gstream, buf, 2);

    linebuf = (char *) galloc(pictrowbytes);
    if (linebuf)
    {
        memset(linebuf,0,pictrowbytes);
        for (y=0; y<height && ok; ++y)
        {
            d = linebuf;
            s = source+y*rowbytes;

            for (x=0; x<width; ++x)
            {
                d[0] = ((ARGB *)s)->a;
                d[1] = ((ARGB *)s)->r;
                d[2] = ((ARGB *)s)->g;
                d[3] = ((ARGB *)s)->b;
                s += 4;
                d += 4;
            }

            ok = gwrite(gx->gstream,linebuf,pictrowbytes);
        }
        gfree(linebuf);
    }

    gputm(buf,PICTOP_ENDOFPICTURE,2);
    ok = gwrite(gx->gstream, buf, 2);

    if (bpp<=8) /* we wrote out garbage, but indicate failure */
        ok = 0;

    return(ok);
}

int GCALL PIC_wclose(GINSTANCE *gx)
{
    int ok=0;

    if (gx)
        ok = gfree((void *) gx);

    return(ok);
}


