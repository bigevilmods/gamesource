// (c) 1994-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*                     ANM - DPaint ANIM Gimex                      */
/*                                                                  */
/*             by FrANK G. Barchard and Darren Gyles, EAC           */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History:                                       */
/* ------- ----   -- --------                                       */
/*    1.00 940603 FB Based on LN32 module                           */
/*    1.01 940614 FB updated to Gimex 1.5                           */
/*    1.02 941010 FB isanm, read working                            */
/*    1.03 941027 FB descriptor table read, short reads ggeti 2 mac */
/*    1.04 941125 DG Added write capability.                        */
/*    1.05 941128 DG Fixed small bug, all saved anms work on IBM.   */
/*    1.06 941201 FB NULL changed to 0 for PC portibility.          */
/*    1.07 941205 FB Error checking on decompress frame.            */
/*    1.08 950102 FB Crop to 320x200 on export                      */
/*    1.09 950418 DG export dont reset anm->curlp.baseRecord        */
/*    1.10 950426 FB single palette field                           */
/*    1.11 950529 FB ginstance signature                            */
/*    1.12 970822 KO wcloseanm fix up frame count                   */
/*    1.13 000404 FB about->movie                                   */
/*    1.14 001121 FB export 0 frames                                */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 8 bit                                                            */
/* Reentrant                                                        */
/* Read only                                                        */
/* Files: anmread.c anmgimex.h                                      */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include <string.h>
#include "gimex.h"
#include "anmgimex.h"

/****************************************************************/
/*  Internal Functions                                          */
/****************************************************************/

static int ggetc(GSTREAM *g)
{
    unsigned char c[4];
    int v=-1;

    if (gread(g,(void *) &c,1))
        v = (int) (c[0]&255);
    return(v);
}

/* given a frame number return the large page number it resides in */

static unsigned short findpage(ANMINFO *anm,unsigned short framenumber)
{
    unsigned short i;

    for(i=0; i<ANMGETFIELD(anm->DAHEADER.nLps); i++)
    {
       if (ANMGETFIELD(anm->LpArray[i].baseRecord)<=framenumber
        && ANMGETFIELD(anm->LpArray[i].baseRecord)+ANMGETFIELD(anm->LpArray[i].nRecords)>framenumber)
          return(i);
    }
    return(i);
}

/* seek out and load in the large page specified */

static void loadpage(ANMINFO *anm,GSTREAM *g,unsigned short pagenumber,unsigned char *pagepointer)
{
    if (anm->curlpnum!=pagenumber)
    {
        anm->curlpnum=pagenumber;
        gseek(g, 0x0b00L+(int)pagenumber * 65536L);
        gread(g, &anm->curlp, sizeoflp_descriptor);
        ggetc(g);  /* skip empty word */
        ggetc(g);  /* skip empty word */
        gread(g,pagepointer, ANMGETFIELD(anm->curlp.nBytes)+(ANMGETFIELD(anm->curlp.nRecords)*2));
    }
}

/* This version of the decompressor is here for portability to non PC's */

static void CPlayRunSkipDump(unsigned char *srcP,unsigned char *dstP,int dstmax)
{
    signed char cnt = 0;
    unsigned short wordCnt;
    unsigned char pixel;
    unsigned char *d;
    unsigned char *dend;

    d = dstP;
    dend = dstP+dstmax;

nextOp:
        if (d>=dend)
            goto stop;
        cnt = (signed char) *srcP++;
        if (cnt > 0)
            goto dump;
        if (cnt == 0)
            goto run;
        cnt += (signed char) 0x80;
        if (cnt == 0)
            goto longOp;
                                        /* shortSkip */
        d += cnt;                   /* adding 7-bit count to 32-bit pointer */
        goto nextOp;
dump:
        do
        {
           *d++ = *srcP++;
        } while (--cnt && d<dend);

        d += cnt;
        srcP += cnt;
        goto nextOp;
run:
        wordCnt = (unsigned char)*srcP++;       /* 8-bit unsigned count */
        pixel = *srcP++;
        do
        {
           *d++ = pixel;
        } while (--wordCnt && d<dend);

        d += wordCnt;
        goto nextOp;
longOp:
        wordCnt = (unsigned short) ggeti(srcP,2);
        srcP += 2;
        if ((short)wordCnt <= 0)
           goto notLongSkip;            /* Do SIGNED test. */

        d += wordCnt;               /* longSkip. */
        goto nextOp;

notLongSkip:
        if (wordCnt == 0)
           goto stop;
        wordCnt -= 0x8000;              /* Remove sign bit. */
        if (wordCnt >= 0x4000)
           goto longRun;


        do
        {
           *d++ = *srcP++;          /* longDump. */
        } while (--wordCnt);

        d += wordCnt;
        srcP += wordCnt;
        goto nextOp;

longRun:
        wordCnt -= 0x4000;              /* Clear "longRun" bit. */
        pixel = *srcP++;
        do
        {
           *d++ = pixel;
        } while (--wordCnt && d<dend);

        d += wordCnt;
        goto nextOp;

stop:                                   /* all done */
    ;
}



/* draw the frame specified from the large page in the buffer pointed to */

static void renderframe(ANMINFO *anm,unsigned short framenumber,unsigned char *pagepointer)
{
    unsigned short offset=0;
    unsigned short i;
    unsigned short destframe;
    unsigned char *ppointer;
    unsigned char *bitmap;

    destframe = (unsigned short)(framenumber - (unsigned short)ANMGETFIELD(anm->curlp.baseRecord));

    for (i = 0; i < destframe; i++)
    {
       offset = (unsigned short)(offset + ggeti(pagepointer+i*2,2));
    }
    ppointer = pagepointer;

    ppointer += ANMGETFIELD(anm->curlp.nRecords)*2+offset;

    if (ppointer[1])
    {
       ppointer += (4 + (ggeti(&((unsigned short *)ppointer)[2],2)+1)&~1);
    }
    else
    {
       ppointer += 4;
    }

    bitmap = anm->animwin;
    CPlayRunSkipDump(ppointer, bitmap, ANMGETFIELD(anm->DAHEADER.width)*ANMGETFIELD(anm->DAHEADER.height));
}

/****************************************************************/
/*  Information Functions                                       */
/****************************************************************/

/* check for reasonable header: */
/* 'LPF ' id */

int GCALL ANM_is(GSTREAM *g)
{
    char anmbuf[4];
    int ok=0;

    gseek(g, 0);
    if (gread(g, anmbuf, 4))
    {
        if (ggetm(anmbuf,4)==GMAKEID('L','P','F',' '))
            ok = 100;    /* it's an anim */
    }
    return(ok);
}


GABOUT * GCALL ANM_about(void)
{
    GABOUT *info;

    info = (GABOUT *) galloc(sizeof(GABOUT));
    if (info)
    {
        memset(info, 0, sizeof(GABOUT));

        info->signature       = GMAKEID('.','A','N','M');
        info->size            = sizeof(GABOUT);
        info->version         = GIMEX_VERSION;
        info->canimport          = 1;    /* supports importing */
        info->canexport          = 1;    /* supports exporting */
        info->importpacked    = 1;    /* supports importing packed */
        info->exportpacked    = 1;    /* supports exporting packed */
        info->import8         = 1;    /* supports importing 8 bit indexed */
        info->export8         = 1;    /* supports exporting 8 bit indexed */
        info->import32        = 0;    /* supports importing 32 bit direct rgb */
        info->export32        = 0;    /* supports exporting 32 bit direct rgb */
        info->multiframe      = 1;    /* supports multiple frames */
        info->multifile       = 0;    /* format requires additional files or resource fork */
        info->multisize       = 0;    /* supports each frame different size */
        info->framebuffer     = 1;    /* module requires memory to buffer entire frame */
        info->external        = 0;    /* module uses external resources */
        info->usesfile        = 1;    /* module is file based */
        info->globalpalette   = 1;    /* one palette per file */
        info->maxframename    = 0;    /* max frame name character */
        info->defaultquality  = 100;  /* default pack quality */
        info->movie           = 1;    /* movie file format */
        info->mactype[0]      = GMAKEID('.','A','N','M');       /* mac file system types used */
        strcpy(info->extensions[0], ".anm");                    /* extensions */
        strcpy(info->authorstr,     "D. Gyles & F. Barchard");  /* author */
        strcpy(info->versionstr,    "1.14");                    /* version # */
        strcpy(info->shorttypestr,  "ANM");                     /* type */
        strcpy(info->wordtypestr,   "Anim");                    /* wordtype */
        strcpy(info->longtypestr,   "DPaint ANM");              /* longtype */
    }
    return(info);
}

/****************************************************************/
/*  Import Functions                                            */
/****************************************************************/

int GCALL ANM_open(GINSTANCE **gxptr, GSTREAM *g, const char *pathname,bool framecountflag)
{
    GINSTANCE *gx;
    ANMINFO *anm;
    int  ok=0;
    int i;

    gx = (GINSTANCE *) galloc(sizeof(GINSTANCE));
    if (gx)
    {
        memset(gx, 0, sizeof(GINSTANCE));
        *gxptr = gx;
        gx->signature=GMAKEID('.','A','N','M');
        gx->size=sizeof(GINSTANCE);
        gx->gstream = g;
        gx->framenum = -1;  /* keep track of which frame were pointing at */

        gseek(gx->gstream,(GPOS)0);

        gx->gref = galloc(sizeof(ANMINFO));
        if (gx->gref)
        {
            anm = (ANMINFO *) gx->gref;
            memset((void *) anm,0,sizeof(ANMINFO));

            anm->curlpnum=0xFFFF;       /* initialize to an invalid Large page number */

            anm->thepage = (unsigned char *) galloc(65536L);

            /* read the anim file header */
            gread(g,&anm->DAHEADER,sizeof(lpfileheader));

            /* If the last record is the last-to-first delta, we ignore it */
            if (ANMGETFIELD(anm->DAHEADER.hasLastDelta) == 1)
               ANMPUTFIELD(anm->DAHEADER.nRecords, ANMGETFIELD(anm->DAHEADER.nRecords) -1);

            gseek(g, 128L+gtell(g));     /* skip color cycling structures */

            /* read in the color palette */
            for(i=0; i<256; ++i)
            {
                anm->animpalette[i].b = (unsigned char)ggetc(g);      /* read blue */
                anm->animpalette[i].g = (unsigned char)ggetc(g);      /* read green */
                anm->animpalette[i].r = (unsigned char)ggetc(g);      /* read red */
                anm->animpalette[i].a = (unsigned char)255;
                ggetc(g);                          /* skip over the extra pad byte */
            }


            /* read in large page descriptors */

            for (i=0; i<256; ++i)
                gread(g,&anm->LpArray[i],sizeoflp_descriptor);

/* this may be unportable descriptors are 6 bytes. do loop to be sure */
/*          gread(g,anm->LpArray,sizeoflp_descriptor*256L); */

            /* the file pointer now points to the first large page structure */

            anm->animwin = (unsigned char *) galloc(ANMGETFIELD(anm->DAHEADER.width) * ANMGETFIELD(anm->DAHEADER.height));
            if (anm->animwin)
                ok = gx->frames = ANMGETFIELD(anm->DAHEADER.nRecords);
        }
    }
    return(ok);
}



GINFO * GCALL ANM_info(GINSTANCE *gx, int framenum)
{
    GINFO *ginfo = NULL;
    ANMINFO *anm = NULL;
    int width    = 0;
    int height   = 0;

    anm = (ANMINFO *) gx->gref;
    if (anm)
    {
        width = ANMGETFIELD(anm->DAHEADER.width);
        height = ANMGETFIELD(anm->DAHEADER.height);

        if (framenum < gx->frames)
            ginfo = (GINFO *) galloc(sizeof(GINFO));

        if (ginfo)
        {
            memset(ginfo,0,sizeof(GINFO));

            /* template for ginfo */

            ginfo->signature        = GMAKEID('.','A','N','M');   /* Gimex type */
            ginfo->size             = sizeof(GINFO);            /* Length of GINFO */
            ginfo->version          = GIMEX_VERSION;
            ginfo->framenum         = framenum;                 /* Current frame */
            ginfo->width            = width;                    /* Width of bitmap in pixels */
            ginfo->height           = height;                   /* Height of bitmap in pixels */
            ginfo->subtype          = 0;                        /* Internal format sub-type 0=chunk, 1=nonchunk */
            ginfo->packed           = 1;                        /* Type of packing on original image. 0=key, 1=motion */
            ginfo->quality          = 100;                      /* Quality of lossy packing 0..100 */
            ginfo->alphabits        = 0;    /* Number of bits in alpha channel */
            ginfo->redbits          = 8;    /* Number of bits in red channel */
            ginfo->greenbits        = 8;    /* Number of bits in grreen channel */
            ginfo->bluebits         = 8;    /* Number of bits in blue channel */
            ginfo->centerx          = 0;    /* Center Point relative to upper left corner */
            ginfo->centery          = 0;
            ginfo->defaultx         = 0;    /* Default Coordinate Point */
            ginfo->defaulty         = 0;
            ginfo->numhotspots      = 0;    /* Number of Hot Spots defined */
            ginfo->bpp              = 8;    /* Bits per pixel (8 or 32) */
            /*ginfo->indexed          = 1;    \* 0 no, 1 yes (indexed meaning it uses a palette) *\ */
            ginfo->originalbpp      = 8;    /* Bits per pixel in original image (1 to 32) */
            /*ginfo->originalindexed  = 1;    \* 0 no, 1 yes (indexed meaning it used a palette) *\ */
            ginfo->numcolours  = 256;                                /* Number of colours in original indexed palette */

            memcpy(ginfo->colourtbl,anm->animpalette,256*4);
        }
    }
    return(ginfo);
}

int GCALL ANM_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes)
{
    ANMINFO *anm;
    int y;
    int bpl;
    unsigned char *d;
    unsigned char *s;
    int ok=1;

    anm = (ANMINFO *) gx->gref;

    loadpage(anm,gx->gstream,findpage(anm,(unsigned short)(ginfo->framenum)), anm->thepage);
    renderframe(anm, (unsigned short)(ginfo->framenum), anm->thepage);

    bpl = ginfo->width;
    for (y=0; y<ginfo->height; ++y)
    {
        d = (unsigned char *) dest + (rowbytes * y);
        s = anm->animwin + (y * bpl);

        memcpy(d,s,bpl);
    }

    return(ok);
}

int GCALL ANM_close(GINSTANCE *gx)
{
    ANMINFO *anm;
    int  ok=0;

    if (gx)
    {
        anm = (ANMINFO *) gx->gref;
        if (anm)
        {
            if (anm->thepage) gfree((void *)anm->thepage);
            if (anm->animwin) gfree((void *)anm->animwin);
            gfree((void *)anm);
        }
        ok = gfree((void *) gx);
    }
    return(ok);
}


