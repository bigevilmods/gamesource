// (c) 1994-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*                        MV8 - Cobra Gimex                         */
/*                                                                  */
/*                   by FrANK G. Barchard, EAC                      */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History:                                       */
/* ------- ----   -- --------                                       */
/*    1.00 941218 FB Based on MVI and EASM player                   */
/*    1.01 941219 FB Portible, Seek backward, reduced variables     */
/*    1.02 941222 FB PSX getmask and copy entire bitmap for speed   */
/*    1.03 941225 FB Large block use 1st and 3rd row                */
/*    1.04 950128 FB CBRD header                                    */
/*    1.05 950529 FB ginstance signature                            */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 8 bit multiframe                                                 */
/* Non-Reentrant                                                    */
/* Files: mv8read.cpp mv8write.cpp mv8gimex.h                           */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Format Notes:                                                    */
/* -------------                                                    */
/* MV8 files VQ based on 4 2x2 blocks or 1 2x2 block scaled to      */
/* 4x4.  There are 256 blocks.  The file structure is:              */
/*                                                                  */
/*  [File Header]                                                   */
/*  [Audio Header 0] (optional)                                     */
/*  [Audio Buffer 0] (optional)                                     */
/*  [Audio Header 1] (optional)                                     */
/*  [Audio Buffer 1] (optional)                                     */
/*  [Movie Header + FrameTableEntry table]                          */
/*  [FrameMovie] * frames                                           */
/*                                                                  */
/*    [FrameHeader+Data]                                            */
/*    [Audio Header n] (optional)                                   */
/*    [Audio Buffer n] (optional)                                   */
/*                                                                  */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */
/*#include <string.h> */
#include "gimex.h"
#include "mv8gimex.h"

/****************************************************************/
/*  Support Functions (fast.asm)                                */
/****************************************************************/

static void ExpandB(char *s,char *d)
{
    int i;

    for (i=0; i<0x200; ++i)
    {
        *(d+0) = *(s+0);
        *(d+1) = *(s+0);
        *(d+4) = *(s+0);
        *(d+5) = *(s+0);
        *(d+2) = *(s+1);
        *(d+3) = *(s+1);
        *(d+6) = *(s+1);
        *(d+7) = *(s+1);
        *(d+8) = *(s+2);
        *(d+9) = *(s+2);
        *(d+12) = *(s+2);
        *(d+13) = *(s+2);
        *(d+10) = *(s+3);
        *(d+11) = *(s+3);
        *(d+14) = *(s+3);
        *(d+15) = *(s+3);
        s += 4;
        d += 16;
    }
}

static void NewDoFastCopy2(char *s,char *d,int rowbytes)
{
    unsigned int v1;
    unsigned int v2;

    v1 = *((unsigned int *) (s+0));
    v2 = *((unsigned int *) (s+8));

    *((unsigned int *) d) = v1;    d += rowbytes;
    *((unsigned int *) d) = v1;    d += rowbytes;

    *((unsigned int *) d) = v2;    d += rowbytes;
    *((unsigned int *) d) = v2;
}

static void NewDoFastCopy1(char *eax, char *edx, char *esi,int rowbytes)
{
    unsigned int ebx;
    unsigned char *ebxp;
    unsigned short di;

    /* Upper left block */
    /* Grab code, expand into table address */

    ebx = (unsigned int) (unsigned char) *(esi);
    ebx <<= 2;
    ebxp = eax+ebx;

    di = *(unsigned short *) ebxp;
    ebxp += 2;

    *(unsigned short *) edx = di;
    edx += rowbytes;

    di = *(unsigned short *) ebxp;
    esi += 1;

    *(unsigned short *) edx = di;
    edx -= (rowbytes-2);

    /* Upper right block */
    /* Grab code, expand into table address */

    ebx = (unsigned int) (unsigned char) *(esi);
    ebx <<= 2;
    ebxp = eax+ebx;

    di = *(unsigned short *) ebxp;
    ebxp += 2;

    *(unsigned short *) edx = di;
    edx += rowbytes;

    di = *(unsigned short *) ebxp;
    esi += 1;

    *(unsigned short *) edx = di;
    edx += (rowbytes-2);

    /* Lower left block */
    /* Grab code, expand into table address */

    ebx = (unsigned int) (unsigned char) *(esi);
    ebx <<= 2;
    ebxp = eax+ebx;

    di = *(unsigned short *) ebxp;
    ebxp += 2;

    *(unsigned short *) edx = di;
    edx += rowbytes;

    di = *(unsigned short *) ebxp;
    esi += 1;

    *(unsigned short *) edx = di;
    edx -= (rowbytes-2);

    /* Lower right block */
    /* Grab code, expand into table address */

    ebx = (unsigned int) (unsigned char) *(esi);
    ebx <<= 2;
    ebxp = eax+ebx;

    di = *(unsigned short *) ebxp;
    ebxp += 2;

    *(unsigned short *) edx = di;

    di = *(unsigned short *) ebxp;
    edx += rowbytes;
    *(unsigned short *) edx = di;
}

/****************************************************************/
/*  Engine                                                      */
/****************************************************************/

/* Decode a Cobra frame
**
** mc - pointer to current movie context struct
** fh - pointer to start of actual frame data
*/

#ifdef PSX
#define getlong(p) *p++
#define memcpy(d,s,l) blockmove(s,d,l)
#else
#ifdef MSDOS
#define getlong(p) *p++
#else
#define getlong(p) ggeti(p,4); ++p
#endif
#endif

static void DecodeCBRFrame(struct MovieContext *mc, struct FrameHeader *thefh)
{
    unsigned char *srcCB;
    unsigned char *dst;
    unsigned char *src;
    unsigned int xPos_4, yPos_4;
    unsigned int entry, mask, bits, *bit_buff;
    unsigned int qtemp;
    unsigned int CenterOffset;
    struct OldFrameHeader *fh = (struct OldFrameHeader *)thefh;

    bit_buff = (unsigned int *)((unsigned int)fh + sizeof(struct FrameHeader) + 256*4*2 + (MV8GETFIELD(fh->ofh_DCTBuffSize)));
    src = (unsigned char *)((unsigned int)fh + sizeof(struct FrameHeader) + 256*4*2 + MV8GETFIELD(fh->ofh_BitBuffSize) + MV8GETFIELD(fh->ofh_DCTBuffSize));

    mc->mc_DCB=(unsigned char *)((unsigned int)fh + sizeof(struct FrameHeader) + MV8GETFIELD(fh->ofh_DCTBuffSize));
    ExpandB((unsigned char *)((unsigned int)fh + sizeof(struct FrameHeader) + 256*4 + MV8GETFIELD(fh->ofh_DCTBuffSize)),&mc->mc_SCB[0]);

    mask = 0;

    /* Setup initial write address */
    qtemp=(mc->mc_MovieOffsetY * mc->mc_MovieWidth) + mc->mc_MovieOffsetX;

    dst=(unsigned char *) mc->framebuf + qtemp;

    CenterOffset=mc->mc_MovieWidth-mc->mc_MovieWidth;

    for (yPos_4 = 0; yPos_4 < mc->mc_MovieHeight; yPos_4 += 4)
    {
        for (xPos_4 = 0; xPos_4 < mc->mc_MovieWidth; xPos_4 += 4)
        {
            if (!mask)
            {
                mask  = 0x80000000;
                bits = getlong(bit_buff);
            }

            if (mask & bits)
            {
                srcCB = (unsigned char *)mc->mc_DCB;

                NewDoFastCopy1(srcCB,dst,src,mc->mc_MovieWidth);
                src += 4;
            }
            else
            {
                entry=(unsigned int)*src++;
                srcCB = (unsigned char *)(&mc->mc_SCB[0]) + entry*16;

                NewDoFastCopy2(srcCB,dst,mc->mc_MovieWidth);
            }
            mask >>= 1;
            dst += 4;
        }
        dst += (mc->mc_MovieWidth*3) + CenterOffset;
    }
}

/****************************************************************/
/*  Frame Decode                                                */
/****************************************************************/

static void InitMovieContext(struct MovieContext *mc)
{
    mc->mc_Future  = mc->mc_Frames[0];
    mc->mc_Past    = mc->mc_Frames[1];
    mc->mc_Decode  = mc->mc_Frames[2];
    mc->mc_Display = mc->mc_Frames[3];
    mc->mc_ValidPast = FALSE;

    /* Set up the first frame to be displayed by default. */
    /* NB: This is set to the first "future" frame because by the time */
    /*     it's actually supposed to be up on screen, it will be the current */
    /*     "past" frame. */

    mc->mc_NextDisp = mc->mc_Frames[0];

    /* Set up all of the relavent DrawEnv and DisplayEnv structures. */
}

static void DecodeFrame(struct MovieContext *mc, struct FrameHeader *thefh)
{
    unsigned int *tmpFrame;
    struct OldFrameHeader *fh = (struct OldFrameHeader *)thefh;

    switch(MV8GETFIELD(fh->ofh_Type))
    {
        case FT_INTRA:
        {
            /* Okay, swap the previous and future buffers...no sense in copying memory around */

            tmpFrame = mc->mc_Future;
            mc->mc_Future = mc->mc_Past;
            mc->mc_Past = tmpFrame;

            /* Decode the next I-Frame into the current "future" buffer */
            /* DecodeRawFrame(mc, mc->mc_Future, fh);  */

            /* Force the draw to display memory */
            mc->mc_Future=0;

            switch(MV8GETFIELD(fh->ofh_Compression))
            {
                case FC_CBR8:   DecodeCBRFrame(mc, (struct FrameHeader *)fh); break;
            }

            /* Okay, after loading a new "future" I-Frame, post the current past frame */
            /* for display, but only if it's valid (which won't be true the first time through. */

            if (mc->mc_ValidPast)
                mc->mc_NextDisp = mc->mc_Past;

            /* Okay, now it's safe to say we have a Valid Past frame (or, it will be by the time */
            /* this code gets run again.)  */

            mc->mc_ValidPast = TRUE;
        }
        break;
    }
}


static int startplay(struct MovieContext *mc,GSTREAM *MyFile)
{
    int ok=0;
    unsigned int index;
    unsigned int tempvar;

    index=0;

    InitMovieContext(mc);

    mc->AudioBuff = galloc(65536L);

    /* Allocate enough memory to hold one frame in memory */

    if (mc->myMovie=galloc(32768))
    {
        /* Open the MV8 file */
        /* is open */

        {
            gread(MyFile,&mc->fheader,sizeof(struct FileHeader));

            if (MV8GETFIELD(mc->fheader.ch_mode) & 0x01)
            {
                mc->BUFFSIZE=(MV8GETFIELD(mc->fheader.ch_samplerate) * (unsigned int)50)/MV8GETFIELD(mc->fheader.ch_fps);
                mc->MAXBUFFSIZE=mc->BUFFSIZE*4;

                /* Grab the first two audio samples */
                gread(MyFile,&mc->aheader,sizeof(struct AudioHeader));
                gread(MyFile,&mc->AudioBuff[0],mc->BUFFSIZE);
                gread(MyFile,&mc->aheader,sizeof(struct AudioHeader));
                gread(MyFile,&mc->AudioBuff[mc->BUFFSIZE],mc->BUFFSIZE);
            }

            /* Grab the movie header */
            if (gread(MyFile,&mc->movieh,sizeof(struct MovieHeader)))
            {
                /* Calculate the header length and frame mc->framecount */
                mc->mheaderlen=MV8GETFIELD(mc->movieh.mh_Atom.ah_Size) - (unsigned int)sizeof(struct MovieHeader);
                mc->framecount = (unsigned int)mc->mheaderlen / (unsigned int) sizeof(struct FrameTableEntry);

                /* Allocate memory for frame table entries */
                tempvar = (mc->mheaderlen >> 4) + 1;
                if (mc->FrameMovie=galloc(mc->mheaderlen))
                {
                    /* Read in all the frame table entries */
                    for (index=0;index<mc->framecount;++index)
                    {
                        if (!gread(MyFile,&mc->FrameMovie[index], sizeof(struct FrameTableEntry)))
                        {
                            goto ErrExit;
                        }
                    }
                    if (mc->framebuf = galloc(MV8GETFIELD(mc->movieh.mh_Width) * MV8GETFIELD(mc->movieh.mh_Height)))
                    {
                        ok = mc->framecount;
                    }
                } else
                {
                    goto ErrExit;
                }
            } else
            {
                goto ErrExit;
            }
        }
    } else
    {
        goto ErrExit;
    }

    mc->mc_MovieWidth =  MV8GETFIELD(mc->movieh.mh_Width);
    mc->mc_MovieHeight = MV8GETFIELD(mc->movieh.mh_Height);

    /* Center the movie on the screen for now */
    mc->mc_MovieOffsetX = 0;
    mc->mc_MovieOffsetY = 0;

    MV8PUTFIELD(mc->fheader.ch_mode,(MV8GETFIELD(mc->fheader.ch_mode)&255)|0x01);
    mc->currentframe = -1;
    mc->framepos = gtell(MyFile);

ErrExit:
    return(ok);
}

static int playframe(struct MovieContext *mc,GSTREAM *MyFile, int index)
{
    int ok=1;
    int done=0;
    unsigned int temploop;
    unsigned int *temppoint;

    if (index==(mc->currentframe))   /* already on current frame */
        done = 1;
    while (!done)
    {
        if (index<mc->currentframe)
        {
            mc->currentframe = -1;
            gseek(MyFile,mc->framepos);
        }

        ++mc->currentframe;             /* we are fetching this frame */

        if (gread(MyFile,(unsigned char *)mc->myMovie,sizeof(struct FrameHeader)))
        {
            if (gread(MyFile,((unsigned char *)mc->myMovie) + sizeof(struct FrameHeader),
                MV8GETFIELD(((struct OldFrameHeader *)mc->myMovie)->ofh_Atom.ah_Size)-sizeof(struct OldFrameHeader)))
            {
                if (MV8GETFIELD(mc->fheader.ch_mode) & 0x01)
                {
                    /* loop and wait for a free audio buffer. This should *NEVER* have to loop, but just incase */

                    if (!(mc->currentframe % 5))
                    {
                        /* read in the new audio sample and validate that buffer */

                        gread(MyFile,(unsigned char *)&mc->aheader,sizeof(struct AudioHeader));

                        if (!gread(MyFile,(unsigned char *)&mc->AudioBuff[0 * (unsigned int)mc->BUFFSIZE],mc->BUFFSIZE))
                        {
                            goto ErrExit;
                        }
                    }
                }

                /* Check to see if this frame has a palette attached */

                if (MV8GETFIELD(((struct OldFrameHeader *)mc->myMovie)->ofh_DCTBuffSize))
                {
                    /* Make a copy of the palette until we need it - incase of frame dropping */

                    temppoint=(unsigned int *) ((unsigned char *)mc->myMovie+(sizeof(struct OldFrameHeader)));

                    for (temploop=0;temploop<192;++temploop)
                    {
                        mc->palettebuf[temploop]=temppoint[temploop];
                    }
                }

                if (index==mc->currentframe)    /* decode it? */
                {
                    DecodeFrame(mc,(struct FrameHeader *)mc->myMovie);
                    done = 1;
                }

            }
            else
            {
                goto ErrExit;
            }
        }
        else
        {
            goto ErrExit;
        }
    }
ErrExit:
    return(ok);
}

static int endplay(struct MovieContext *mc)
{
    MV8PUTFIELD(mc->fheader.ch_mode,(MV8GETFIELD(mc->fheader.ch_mode)&255));
    if (mc->FrameMovie)     gfree(mc->FrameMovie);
    if (mc->myMovie)        gfree(mc->myMovie);
    if (mc->AudioBuff)      gfree(mc->AudioBuff);
    if (mc->framebuf)       gfree(mc->framebuf);
    return(1);
}

/****************************************************************/
/*  Information Functions                                       */
/****************************************************************/

GABOUT * GCALL MV8_about(void)
{
    GABOUT *info;

    info = (GABOUT *) galloc(sizeof(GABOUT));
    if (info)
    {
        memset(info, 0, sizeof(GABOUT));

        info->signature       = GMAKEID('C','B','R','A');
        info->size            = sizeof(GABOUT);
        info->version         = GIMEX_VERSION;
        info->canimport          = 1;    /* supports importing */
        info->canexport          = 0;    /* supports exporting */
        info->importpacked    = 1;    /* supports importing packed */
        info->exportpacked    = 0;    /* supports exporting packed */
        info->import8         = 1;    /* supports importing 8 bit indexed */
        info->export8         = 0;    /* supports exporting 8 bit indexed */
        info->import32        = 0;    /* supports importing 32 bit direct rgb */
        info->export32        = 0;    /* supports exporting 32 bit direct rgb */
        info->multiframe      = 1;    /* supports multiple frames */
        info->multifile       = 0;    /* format requires additional files or resource fork */
        info->multisize       = 0;    /* supports each frame different size */
        info->framebuffer     = 1;    /* module requires memory to buffer entire frame */
        info->external        = 0;    /* module uses external resources */
        info->usesfile        = 1;    /* module is file based */
        info->maxframename    = 0;    /* max frame name character */
        info->defaultquality  = 100;  /* default pack quality */
        info->mactype[0]      = GMAKEID('C','B','R','A');   /* mac file system types used */
        strcpy(info->extensions[0], ".mv8");                /* extensions */
        strcpy(info->authorstr,     "F Barchard");          /* author */
        strcpy(info->versionstr,    "1.05");                /* version # */
        strcpy(info->shorttypestr,  "MV8");                 /* type */
        strcpy(info->wordtypestr,   "Cobra");               /* wordtype */
        strcpy(info->longtypestr,   "Cobra MV8 Movie");     /* longtype */
    }
    return(info);
}

/* check for reasonable header: */
/* either 'CBRA', 'CBRB' or 'MooV' as intel int id */

int GCALL MV8_is(GSTREAM *g)
{
    unsigned char mv8header[4];
    int ok=0;

    gseek(g,(GPOS)0);
    if (gread(g,mv8header,4))
    {
        if (ggetm(mv8header,4)==GMAKEID('A','R','B','C')
         || ggetm(mv8header,4)==GMAKEID('B','R','B','C')
         || ggetm(mv8header,4)==GMAKEID('D','R','B','C')
         || ggetm(mv8header,4)==GMAKEID('V','o','o','M'))
            ok = 100;
    }
    return(ok);
}


/****************************************************************/
/*  Import Functions                                            */
/****************************************************************/

int GCALL MV8_open(GINSTANCE **gxptr, GSTREAM *g, const char *pathname,bool framecountflag)
{
    GINSTANCE *gx;
    struct MovieContext *mc;
    int  ok=0;

    gx = (GINSTANCE *) galloc(sizeof(GINSTANCE));
    if (gx)
    {
        memset(gx, 0, sizeof(GINSTANCE));
        *gxptr = gx;
        gx->gstream = g;
        gx->framenum = -1;  /* keep track of which frame were pointing at */

        gseek(gx->gstream,(GPOS)0);

        gx->gref = galloc(sizeof(struct MovieContext));
        if (gx->gref)
        {
            mc = (struct MovieContext *) gx->gref;
            memset((void *) mc,0,sizeof(struct MovieContext));

            ok = gx->frames = startplay(mc,g);
            gx->signature=GMAKEID('C','B','R','A');
            gx->size=sizeof(GINSTANCE);
        }
    }
    return(ok);
}

GINFO * GCALL MV8_info(GINSTANCE *gx, int framenum)
{
    GINFO *ginfo=0;
    struct MovieContext *mc;
    int  i;
    int  ok;

    mc = (struct MovieContext *) gx->gref;

    ok = playframe(mc, gx->gstream, framenum);

    if (ok)
    {
        ginfo = (GINFO *) galloc(sizeof(GINFO));
        if (ginfo)
        {
            memset(ginfo,0,sizeof(GINFO));

            /* template for ginfo */

            ginfo->signature        = GMAKEID('C','B','R','A');  /* Gimex type */
            ginfo->size             = sizeof(GINFO);           /* Length of GINFO */
            ginfo->version          = GIMEX_VERSION;
            ginfo->framenum         = framenum;                /* Current frame */
            ginfo->width            = mc->mc_MovieWidth;       /* Width of bitmap in pixels */
            ginfo->height           = mc->mc_MovieHeight;      /* Height of bitmap in pixels */
            ginfo->subtype          = 0;                       /* Internal format sub-type 0=chunk, 1=nonchunk */
            ginfo->packed           = 1;                       /* Type of packing on original image. 0=key, 1=motion,2=refkey,3=refmotion */
            ginfo->quality = 100;               /* key frame */

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
            ginfo->numcolours  = 256;  /* Number of colours in original indexed palette */

            for (i=0; i<256; ++i)
            {
                ginfo->colourtbl[i].a = 255;
                ginfo->colourtbl[i].r = ((*(((char *) &mc->palettebuf)+i*3+0)&63)*255+32)/63;
                ginfo->colourtbl[i].g = ((*(((char *) &mc->palettebuf)+i*3+1)&63)*255+32)/63;
                ginfo->colourtbl[i].b = ((*(((char *) &mc->palettebuf)+i*3+2)&63)*255+32)/63;
            }
        }
    }
    return(ginfo);
}

int GCALL MV8_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes)
{
    int  y;
    int  ok;
    char *d;
    char *s;
    struct MovieContext *mc;

    /* seek to frame and skip header if there is one. */

    mc = (struct MovieContext *) gx->gref;
    ok = playframe(mc, gx->gstream, ginfo->framenum);

    if (ok)
    {
        if (ginfo->width==rowbytes)
        {
            memcpy(dest,mc->framebuf,ginfo->width*ginfo->height);
        }
        else
        {
            for (y=0; y<ginfo->height; ++y)
            {
                d = dest+y*rowbytes;
                s = mc->framebuf+y*ginfo->width;

                memcpy(d,s,ginfo->width);
            }
        }
    }
    return(ok);
}

int GCALL MV8_close(GINSTANCE *gx)
{
    struct MovieContext *mc;
    int ok=0;

    if (gx)
    {
        if (gx->gref)
        {
            mc = (struct MovieContext *) gx->gref;
            endplay(mc);
            gfree((void *)gx->gref);
            gx->gref = 0;
        }
        ok = gfree((void *) gx);
    }

    return(ok);
}


