// (c) 1994-2007 Electronic Arts Inc.

#include <string.h>
#include "gimex.h"
#include "lbmgimex.h"

/****************************************************************/
/*  Internal Functions                                          */
/****************************************************************/

#define GOFFSETRED      (&((ARGB*)0)->r - (unsigned char*)(0))
#define GOFFSETGREEN    (&((ARGB*)0)->g - (unsigned char*)(0))
#define GOFFSETBLUE     (&((ARGB*)0)->b - (unsigned char*)(0))
#define GOFFSETALPHA    (&((ARGB*)0)->a - (unsigned char*)(0))

static int constrainbpp(int bpp)
{
    if (bpp>8) bpp=24;
    return(bpp);
}

static void MakeCMAP(const GINFO *ginfo, unsigned char *SrcPtr, int cmapsize)
{
    int i;
    int colours=1<<ginfo->originalbpp;

    gputm(SrcPtr + 0, CMAP, 4);         /* chunk id */
    gputm(SrcPtr + 4, cmapsize-8, 4);   /* chunk size */

    for (i=0; i<colours; ++i)
    {
        *(SrcPtr+8 +(i*3)) = ginfo->colourtbl[i].r;
        *(SrcPtr+9 +(i*3)) = ginfo->colourtbl[i].g;
        *(SrcPtr+10+(i*3)) = ginfo->colourtbl[i].b;
    }
}

static void MakeCRNG(const GINFO *ginfo, unsigned char *SrcPtr)
{
    int i;
    int active;
    GINFOOLDSHP *ginfoshp=(GINFOOLDSHP *)ginfo;

    for (i=0; i<ginfoshp->numanims; ++i)
    {
        active = 0;
        if (ginfoshp->palanim[i].dir<0) active = 2;

        gputm(SrcPtr+i*16+0, CRNG, 4); /* chunk id */
        gputm(SrcPtr+i*16+4, 8, 4);    /* chunk size */
        gputm(SrcPtr+i*16+8, 0, 2);    /* pad */
        gputm(SrcPtr+i*16+10, ginfoshp->palanim[i].speed*273, 2); /* rate */
        gputm(SrcPtr+i*16+12, active, 2);                         /* active (direction) */
        gputm(SrcPtr+i*16+14, ginfoshp->palanim[i].start, 1);
        gputm(SrcPtr+i*16+15, ginfoshp->palanim[i].end  , 1);
    }
}


static void MakeGRAB(const GINFO *ginfo, unsigned char *SrcPtr)
{
    gputm(SrcPtr+0, GRAB, 4); /* chunk id */
    gputm(SrcPtr+4, 4, 4);    /* chunk size */
    gputm(SrcPtr+8, ginfo->centerx, 2);
    gputm(SrcPtr+10,ginfo->centery, 2);
}

static void MakeANNO(const GINFO *ginfo, unsigned char *SrcPtr, int annosize)
{
    gputm(SrcPtr+0, ANNO, 4);        /* chunk id */
    gputm(SrcPtr+4, annosize-8, 4);  /* chunk size */
    memcpy(SrcPtr+8,ginfo->comment, annosize-8);
}



static unsigned char *PutDump(unsigned char *dest, int nn, int *putSize, unsigned char *buf)
{
    int i;

    PByte((char)(nn-1));
    for (i = 0; i < nn; ++i)
    {
        PByte(buf[i]);
    }

    return(dest);
}

static unsigned char *PutRun(unsigned char *dest, int nn, int cc, int *putSize)
{
    PByte((char)(-(nn-1)));
    PByte((char)(cc));

    return(dest);
}

static int LBMPackBits(unsigned char **pSource, unsigned char **pDest, int rowSize)
{
    unsigned char buf[256];
    int putSize;
    unsigned char *source, *dest;
    char c, lastc = '\0';
    int mode = DUMP;
    int nbuf = 0;           /* number of chars in buffer */
    int rstart = 0 ;        /* buffer index current run stats */

    source = *pSource;
    dest = *pDest;
    putSize = 0;
    buf[0] = lastc = c = GByte(); /* so have valid lastc */
    nbuf = 1;
    rowSize--;              /* since one byte eaten */

    for (; rowSize; --rowSize)
    {
        buf[nbuf++] = c = GByte();
        switch(mode)
        {
            case DUMP:
                /* if the buffer is full, write the length
                byte, then the data */
                if (nbuf>MAXDAT)
                {
                    OutDump(nbuf-1);
                    buf[0] = c;
                    nbuf = 1;
                    rstart = 0;
                    break;
                }
                if (c==lastc)
                {
                    if (nbuf-rstart >= MINRUN)
                    {
                        if (rstart > 0)
                        {
                            OutDump(rstart);
                        }
                        mode = RUN;
                    }
                    else if (rstart==0)
                    {
                        mode = RUN; /* no dump in progress */
                    }
                }
                else
                {
                    rstart = nbuf-1;   /* first of run */
                }
                break;

            case RUN:
                if ((c != lastc) || (nbuf-rstart > MAXRUN))
                {
                    /* output run */
                    OutRun(nbuf-1-rstart, lastc);
                    buf[0] = c;
                    nbuf = 1;
                    rstart = 0;
                    mode = DUMP;
                }
                break;
        }
        lastc = c;
    }
    switch(mode)
    {
        case DUMP:
            OutDump(nbuf);
            break;
        case RUN:
            OutRun(nbuf-rstart, lastc);
            break;
    }

    *pSource = source;
    *pDest = dest;

    return(putSize);
}

static void MakePlaneRow(unsigned char *source, unsigned char *dest, int rowbytes, int plane, int planes)
{
    int i, bn;
    int sourcemod=1;
    int bitsperchannel;
    int channeltbl[4];

    channeltbl[0] =      GOFFSETRED;
    channeltbl[1] =      GOFFSETGREEN;
    channeltbl[2] =      GOFFSETBLUE;
    channeltbl[3] =      GOFFSETALPHA;

    if (planes>8)
    {
        bitsperchannel = gmin(8,planes/3);
        source += channeltbl[plane/bitsperchannel];
        plane %= bitsperchannel;
        sourcemod = 4;
    }


    for (i=0,bn=7; i<rowbytes; ++i)
    {
        *dest |= ((((*source) >> plane) & 1) << bn);
        source+=sourcemod;
        bn--;
        if (bn < 0)
        {
            bn = 7;
            ++dest;
        }
    }
}

/****************************************************************/
/*  Export Functions                                            */
/****************************************************************/

int GCALL LBM_wopen(GINSTANCE **gxptr, GSTREAM *g, const char *pathname, int numframes)
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

int GCALL LBM_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes)
{
    int BytesPerPlaneRow;
    int plane, row;
    char FormPtr[12];
    unsigned char *CmapPtr=0;
    unsigned char *CrngPtr;
    unsigned char *GrabPtr;
    unsigned char *AnnoPtr;
    unsigned char *BmapPtr;
    int count, tcount;
    unsigned char *planePtr, *SrcPtr;
    unsigned char *StartDstPtr, *DstPtr;
    LBMHeaderStruct Hdr;
    unsigned char *workptr;
    int bmhsize;
    int cmapsize=0;
    int crngsize;
    int grabsize;
    int annosize;
    int ok=0;
    unsigned char *IFFWorkPtr;
        int ValidColorRange; /* non zero if color range is valid */

    IFFWorkPtr = (unsigned char *) galloc(MAXROWBYTES);
    if (IFFWorkPtr)
    {
        /* work buffer to pack into (entire frame) */

        workptr = (unsigned char *) galloc(((((((ginfo->width*constrainbpp(ginfo->originalbpp))+15)&~15) / 8) * ginfo->height) + 8)*5/4+8192);
        if (workptr)
        {
            /* set up the BMHD */

            memset(&Hdr, 0, sizeof(LBMHeaderStruct));
            LBMPUTFIELD(Hdr.w, ginfo->width);
            LBMPUTFIELD(Hdr.h, ginfo->height);
            LBMPUTFIELD(Hdr.x, ginfo->defaultx);
            LBMPUTFIELD(Hdr.y, ginfo->defaulty);
            LBMPUTFIELD(Hdr.nPlanes, constrainbpp(ginfo->originalbpp));
            LBMPUTFIELD(Hdr.compression, 1);  /* always packed */
            LBMPUTFIELD(Hdr.xAspect, 0x5);
            LBMPUTFIELD(Hdr.yAspect, 0x6);
            LBMPUTFIELD(Hdr.pageWidth, ginfo->width);
            LBMPUTFIELD(Hdr.pageHeight, ginfo->height);


            bmhsize = sizeof(LBMHeaderStruct) + 8;
            BmapPtr = (unsigned char *) galloc(bmhsize);
            if (BmapPtr)
            {
                memcpy(BmapPtr+ 8, &Hdr, sizeof(LBMHeaderStruct));
                gputm(BmapPtr + 0, BMHD, 4);
                gputm(BmapPtr + 4, sizeof(LBMHeaderStruct), 4);

                /* pack it and save it, one line at a time */

                BytesPerPlaneRow = ((ginfo->width+15)&~15)/8;
                SrcPtr = (unsigned char *)source;
                count = 0L;
                StartDstPtr = workptr;
                DstPtr = StartDstPtr + 8;

                for (row = 0; row < ginfo->height; ++row)
                {
                    for (plane = 0; plane < constrainbpp(ginfo->originalbpp); ++plane)
                    {
                        /* convert byte per pixel format to bit-plane format */

                        memset(IFFWorkPtr, 0, MAXROWBYTES);
                        MakePlaneRow(SrcPtr, IFFWorkPtr, ginfo->width, plane, constrainbpp(ginfo->originalbpp));

                        /* pack a row */
                        planePtr = IFFWorkPtr;
                        count += LBMPackBits(&planePtr, &DstPtr, BytesPerPlaneRow);
                        planePtr = IFFWorkPtr;

                    }
                    SrcPtr += rowbytes;
                }
                if (count&1)
                {
                    count++;
                    *(unsigned char *)DstPtr++ = 0;
                }
                gputm(StartDstPtr + 0, BODY, 4);
                gputm(StartDstPtr + 4, count, 4);


                /* colour table */

                if (ginfo->originalbpp <= 8)
                {
                    cmapsize = (((1 << ginfo->originalbpp) * 3) + 8+1) & ~1;
                    CmapPtr = (unsigned char *) galloc(cmapsize);
                    if (CmapPtr)
                    {
                        MakeCMAP(ginfo, (unsigned char *) CmapPtr, cmapsize);
                    }
                }

                /* colour range */

                ValidColorRange = 1; /* assume true unless proven otherwise */
                if (ginfo->size != sizeof(GINFOOLDSHP))
                    ValidColorRange = 0;
                if (( ginfo->signature != GMAKEID('I','L','B','M')) &&
                    ((ginfo->signature&~255) != GMAKEID('S','H','P',0  )))
                ValidColorRange = 0;

                crngsize = 0;
                CrngPtr  = 0;

                if (ValidColorRange)
                {
                    GINFOOLDSHP *ginfoshp = (GINFOOLDSHP *)ginfo;

                    crngsize = ginfoshp->numanims*16;
                    if (crngsize > 0)
                    {
                        CrngPtr = (unsigned char *) galloc(crngsize);
                        if (CrngPtr)
                        {
                            MakeCRNG(ginfo, (unsigned char *) CrngPtr);
                        }
                    }
                }

                /* center point */

                grabsize = 0;
                GrabPtr = 0;
                if (ginfo->centerx || ginfo->centery)
                {
                    grabsize = 8+4;
                    GrabPtr = (unsigned char *) galloc(grabsize);
                    if (GrabPtr)
                    {
                        MakeGRAB(ginfo, (unsigned char *) GrabPtr);
                    }
                }

                /* comment field */

                annosize = strlen(ginfo->comment);
                AnnoPtr = 0;
                if (annosize)
                {
                    annosize = (8+annosize+1+1)&~1;
                    AnnoPtr = (unsigned char *) galloc(annosize);
                    if (AnnoPtr)
                    {
                        MakeANNO(ginfo, (unsigned char *) AnnoPtr, annosize);
                    }
                }

                gputm(FormPtr + 0, FORM  , 4);
                tcount = 4 + bmhsize + cmapsize + crngsize + grabsize + annosize + count + 8;

                gputm(FormPtr + 4, tcount, 4);
                gputm(FormPtr + 8, ILBM  , 4);

                gwrite(gx->gstream, FormPtr, 12);
                gwrite(gx->gstream, BmapPtr, bmhsize);
                if (CmapPtr) gwrite(gx->gstream, CmapPtr, cmapsize);
                if (CrngPtr) gwrite(gx->gstream, CrngPtr, crngsize);
                if (GrabPtr) gwrite(gx->gstream, GrabPtr, grabsize);
                if (AnnoPtr) gwrite(gx->gstream, AnnoPtr, annosize);
                gwrite(gx->gstream, StartDstPtr, count + 8L);

                if (CmapPtr) gfree(CmapPtr);
                if (CrngPtr) gfree(CrngPtr);
                if (GrabPtr) gfree(GrabPtr);
                if (AnnoPtr) gfree(AnnoPtr);
                ok = 1;

                gfree(BmapPtr);
            }
            gfree(workptr);
        }

        gfree(IFFWorkPtr);
    }
    return(ok);
}

int GCALL LBM_wclose(GINSTANCE *gx)
{
    int ok=0;

    if (gx)
        ok = gfree((void *) gx);

    return(ok);
}

#undef GOFFSETRED
#undef GOFFSETGREEN
#undef GOFFSETBLUE
#undef GOFFSETALPHA

