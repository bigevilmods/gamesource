// (c) 1994-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*                       TGV - 16 Movie Gimex                       */
/*                                                                  */
/*             by Laurent Ancessi and Frank Barchard, EAC           */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History:                                       */
/* ------- ----   -- --------                                       */
/*    1.00 940318 LA t16 Implementation write structure             */
/*    1.01 940320 LA t16 Movie Compression structure based on TGV   */
/*    1.02 940323 LA t16 Read structure based on TGV                */
/*    1.03 940330 LA t16 Implementation of the literal hight res    */
/*    1.04 940403 LA t16 Implementation of the literal low res      */
/*    1.05 940413 FB t16 Keyframe use geti/puti for PC              */
/*    1.06 950529 FB ginstance signature                            */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 16 bit                                                           */
/* Reentrant                                                        */
/* Files: t16read.c, t16gimex.h                                     */
/*        t16write.c, t16type.h                                     */
/*        refread.c, refwrite.c, refcodex.h                         */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Format Notes:                                                    */
/* -------------                                                    */
/* t16 stands for TGV 16 bits (Train a Grande Vitesse)              */
/* 16 bit, multiframe, chunk based, Intel.                          */
/* It is 4x4 motion compensation with motion vectors and literals   */
/* in tables and indexed.                                           */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include <string.h>
#include "gimex.h"
#include "t16gimex.h"

#include "codex.h"
#include "refcodex.h"
#include "refread.cpp"

/****************************************************************/
/*  Decoder Functions                                           */
/****************************************************************/

#ifdef PSX
#define ggetilong(p) \
   (((unsigned int)(*(unsigned char *) ((p)+0))<<0) \
  | ((unsigned int)(*(unsigned char *) ((p)+1))<<8) \
  | ((unsigned int)(*(unsigned char *) ((p)+2))<<16) \
  | ((unsigned int)(*(unsigned char *) ((p)+3))<<24))
#else
#ifdef MSDOS
#define ggetilong(p) *(unsigned int *) (p)
#else
#define ggetilong(p) \
   (((unsigned int)(*(unsigned char *) ((p)+0))<<0) \
  | ((unsigned int)(*(unsigned char *) ((p)+1))<<8) \
  | ((unsigned int)(*(unsigned char *) ((p)+2))<<16) \
  | ((unsigned int)(*(unsigned char *) ((p)+3))<<24))
#endif
#endif

#ifdef PSX
#define MOVLONG(d,s) *(d+0)=*(s+0);*(d+1)=*(s+1);*(d+2)=*(s+2);*(d+3)=*(s+3)
#else
#ifdef MSDOS
#define MOVLONG(d,s) *(int*)(d) = *(int*)(s)
#else
#ifdef applec
#define MOVLONG(d,s) *(int*)(d) = *(int*)(s)
#else
#define MOVLONG(d,s) *(d+0)=*(s+0);*(d+1)=*(s+1);*(d+2)=*(s+2);*(d+3)=*(s+3)
#endif
#endif
#endif

#define T16getfield(ptr,bitoff,n) ((ggetilong((ptr)+((bitoff)>>3)) >> ((bitoff)&7)) & ((1<<n)-1))

/* decode signed bits */

static void T16decodesbits(int *CPtr,char *SPtr,int NbCell,int CDepth)
{
    int  i;
    int  bitoff;
    int signbit;
    int Value;
    int NegVal;
    int  *d;

    bitoff = (NbCell-1)*CDepth;
    d = CPtr + (NbCell);

    signbit = 1<<(CDepth-1);
    NegVal = (0xFFFFFFFF<<CDepth);

    for (i=0; i<NbCell; i++)
    {
        Value = T16getfield(SPtr,bitoff,CDepth);
        bitoff -= CDepth;

        if (Value&signbit) Value |= NegVal; /* Complement to 32 */

        *--d = (int) Value;
    }
}

/* decode unsigned bits */

static void T16decodeubits(int *CPtr,char *SPtr,int NbCell,int CDepth)
{
    int  i;
    int  bitoff;
    int  *d;

    bitoff = (NbCell-1)*CDepth;
    d = CPtr + (NbCell);

    for (i=0; i<NbCell; i++)
    {
        *--d = T16getfield(SPtr,bitoff,CDepth);
        bitoff -= CDepth;
    }
}

static void T16decodemotion(T16INFO *tgv)
{
    int     width;
    int     y,rowbytes1;
    int     rowbytes2,rowbytes3;
    int     nummotion;
    char    *dest;
    char    *destend;
    char    *source;
    int    offsettoprev;
    int     *indexptr;
    int     nextindex;

    width = tgv->frameWidth;
    nummotion = tgv->numMotion;
    indexptr = tgv->indexMatrix;
    dest = tgv->current;
    offsettoprev = tgv->previous-tgv->current;
    rowbytes1 = width*sizeof(int);
    rowbytes2 = rowbytes1*2;
    rowbytes3 = rowbytes1*3;
    for (y=0; y<tgv->frameHeight; y+=4)
    {
        destend = dest+rowbytes1;
        do
        {
            nextindex = *indexptr++;
            if (nextindex<nummotion)    /* motion */
            {
                source = dest+offsettoprev+tgv->offset[nextindex];
                MOVLONG(dest,               source);
                MOVLONG(dest+4,             source+4);
                MOVLONG(dest+8,             source+8);
                MOVLONG(dest+12,            source+12);
                MOVLONG(dest+rowbytes1,     source+rowbytes1);
                MOVLONG(dest+rowbytes1+4,   source+rowbytes1+4);
                MOVLONG(dest+rowbytes1+8,   source+rowbytes1+8);
                MOVLONG(dest+rowbytes1+12,  source+rowbytes1+12);
                MOVLONG(dest+rowbytes2,     source+rowbytes2);
                MOVLONG(dest+rowbytes2+4,   source+rowbytes2+4);
                MOVLONG(dest+rowbytes2+8,   source+rowbytes2+8);
                MOVLONG(dest+rowbytes2+12,  source+rowbytes2+12);
                MOVLONG(dest+rowbytes3,     source+rowbytes3);
                MOVLONG(dest+rowbytes3+4,   source+rowbytes3+4);
                MOVLONG(dest+rowbytes3+8,   source+rowbytes3+8);
                MOVLONG(dest+rowbytes3+12,  source+rowbytes3+12);
            }
            else /* literal */
            {
                source = tgv->literal+(nextindex-nummotion)*kSizeStoreLiteral;
                MOVLONG(dest,               source);
                MOVLONG(dest+4,             source+4);
                MOVLONG(dest+8,             source+8);
                MOVLONG(dest+12,            source+12);
                MOVLONG(dest+rowbytes1,     source+16);
                MOVLONG(dest+rowbytes1+4,   source+20);
                MOVLONG(dest+rowbytes1+8,   source+24);
                MOVLONG(dest+rowbytes1+12,  source+28);
                MOVLONG(dest+rowbytes2,     source+32);
                MOVLONG(dest+rowbytes2+4,   source+36);
                MOVLONG(dest+rowbytes2+8,   source+40);
                MOVLONG(dest+rowbytes2+12,  source+44);
                MOVLONG(dest+rowbytes3,     source+48);
                MOVLONG(dest+rowbytes3+4,   source+52);
                MOVLONG(dest+rowbytes3+8,   source+56);
                MOVLONG(dest+rowbytes3+12,  source+60);
            }
            dest += (4*sizeof(int));
        }
        while (dest<destend);
        dest += (width*sizeof(int))*3;
    }
}


/****************************************************************/
/*  Decode literal                                              */
/****************************************************************/

static void T16decodehighliteral(char* Scr,char* Dest, int numlit)
{
    int                 i,MaxIter;
    unsigned short      Rgb5;
    unsigned int       Rgb8;
    unsigned short*     Scr16;
    unsigned int*      Dest32;

    MaxIter=numlit*16;
    Scr16=(unsigned short*)Scr;
    Scr16+=MaxIter-1;
    Dest32=(unsigned int*)Dest;
    Dest32+=MaxIter-1;
    for (i=0; i<MaxIter; i++)
    {
        Rgb5=T16GETFIELD(*Scr16);
        C5to8(Rgb5,Rgb8);
        *Dest32=Rgb8;
        Scr16--;
        Dest32--;
    }
}


static void decodelowliteral(unsigned short* Scr,unsigned int* Dest)
{
    unsigned int*  PtrMatrix;
    unsigned int   Matrix;
    unsigned short  Rgb5;

    Scr[0]=T16GETFIELD(Scr[0]);
    Scr[1]=T16GETFIELD(Scr[1]);
    Scr[2]=T16GETFIELD(Scr[2]);
    Scr[3]=T16GETFIELD(Scr[3]);
    PtrMatrix=(unsigned int*)&Scr[4];
    Matrix=T16GETFIELD(*PtrMatrix);
    Rgb5=Scr[(Matrix>>30)&3];C5to8(Rgb5,Dest[0]);
    Rgb5=Scr[(Matrix>>28)&3];C5to8(Rgb5,Dest[1]);
    Rgb5=Scr[(Matrix>>26)&3];C5to8(Rgb5,Dest[2]);
    Rgb5=Scr[(Matrix>>24)&3];C5to8(Rgb5,Dest[3]);
    Rgb5=Scr[(Matrix>>22)&3];C5to8(Rgb5,Dest[4]);
    Rgb5=Scr[(Matrix>>20)&3];C5to8(Rgb5,Dest[5]);
    Rgb5=Scr[(Matrix>>18)&3];C5to8(Rgb5,Dest[6]);
    Rgb5=Scr[(Matrix>>16)&3];C5to8(Rgb5,Dest[7]);
    Rgb5=Scr[(Matrix>>14)&3];C5to8(Rgb5,Dest[8]);
    Rgb5=Scr[(Matrix>>12)&3];C5to8(Rgb5,Dest[9]);
    Rgb5=Scr[(Matrix>>10)&3];C5to8(Rgb5,Dest[10]);
    Rgb5=Scr[(Matrix>>8)&3];C5to8(Rgb5,Dest[11]);
    Rgb5=Scr[(Matrix>>6)&3];C5to8(Rgb5,Dest[12]);
    Rgb5=Scr[(Matrix>>4)&3];C5to8(Rgb5,Dest[13]);
    Rgb5=Scr[(Matrix>>2)&3];C5to8(Rgb5,Dest[14]);
    Rgb5=Scr[(Matrix>>0)&3];C5to8(Rgb5,Dest[15]);
}


static void T16decodelowliteral(char* Scr,char* Dest, int numlit)
{
    int     i;

    Scr+=numlit*kSizeLowLiteral;
    Dest+=numlit*kSizeStoreLiteral;
    for (i=0; i<numlit; i++)
    {
        Scr-=kSizeLowLiteral;
        Dest-=kSizeStoreLiteral;
        decodelowliteral((unsigned short*)Scr,(unsigned int*)Dest);
    }
}


/****************************************************************/
/*  Read TGV 16 bits frame                                      */
/****************************************************************/

static int TGVdecodeframe(char *chunk, T16INFO *tgv)
{
    int             ok=1;
    int             i;
    int             width;
    int             height;
    int             nummotion;
    int             numhighliteral;
    int             numlowliteral;
    int             indexbits;
    int             numliteral;
    int             numindex;
    int            decodelen;
    char            *s;
    char            *tempframe;
    int             *rowtbl;
    T16FrameStruct  *tgvframe;

    tgvframe = (T16FrameStruct *) chunk;
    width  = tgv->frameWidth;
    height = tgv->frameHeight;

    /* make current previous, previous current */

    tempframe       = tgv->previous;
    tgv->previous   = tgv->current;
    tgv->current    = tempframe;

    /* read frame header */

    nummotion       = tgv->numMotion        = T16GETFIELD(tgvframe->numMotion);
    numhighliteral  = tgv->numHightLiteral  = T16GETFIELD(tgvframe->numHighLiteral);
    numlowliteral   = tgv->numLowLiteral    = T16GETFIELD(tgvframe->numLowLiteral);
    indexbits       = tgv->indexBits        = T16GETFIELD(tgvframe->indexBits);

    /* reallocate motion and literal blocks */

    if (nummotion)
    {
        if (nummotion>tgv->maxMotion)
        {
            if (tgv->motion)
            {
                gfree((void *)tgv->motion);
                tgv->motion = (T16MOTION *) 0;
                tgv->maxMotion = 0;
            }
            if (tgv->offset)
            {
                gfree((void *)tgv->offset);
                tgv->offset = (int *) 0;
            }
        }
        if (!tgv->motion)
        {
            tgv->motion = (T16MOTION *) galloc(nummotion*sizeof(T16MOTION));
            tgv->maxMotion = nummotion;
        }
        if (!tgv->offset)
        {
            tgv->offset = (int *) galloc(nummotion*sizeof(int));
            tgv->maxMotion = nummotion;
        }
        if (!tgv->motion) ok=0;
        if (!tgv->offset) ok=0;
    }

    numliteral = numhighliteral+numlowliteral;
    if (numliteral)
    {
        if (numliteral>tgv->maxLiteral)
        {
            if (tgv->literal)
            {
                gfree((void *)tgv->literal);
                tgv->literal = 0;
            }
        }
        if (!tgv->literal)
        {
            tgv->literal = (char*)galloc(numliteral*kSizeStoreLiteral);
            tgv->maxLiteral = numliteral;
        }
        if (!tgv->literal) ok=0;
    }

    if (ok)
    {
        /* decode motion vectors */

        decodelen = ((nummotion*MOTIONBITS*2+31)&~31)>>3;
        s = chunk+sizeof(T16FrameStruct);
        T16decodesbits(&tgv->motion[0].x,s,nummotion*2,MOTIONBITS);
        rowtbl = tgv->rowtbl+height;
        for (i=0; i<nummotion; ++i)
        {
            tgv->offset[i] = rowtbl[tgv->motion[i].y]+(tgv->motion[i].x<<2);
        }
        s += decodelen;

        /* decode high literal */

        decodelen = numhighliteral*kSizeHighLiteral;
        if (decodelen) T16decodehighliteral(s,tgv->literal,numhighliteral);
        s += decodelen;

        /* decode low literal */

        decodelen = numlowliteral*kSizeLowLiteral;
        if (decodelen) T16decodelowliteral(s,tgv->literal+numhighliteral*kSizeStoreLiteral,numlowliteral);
        s += decodelen;

        /* decode index matrix */

        numindex = width*height>>4;
        decodelen = ((numindex*indexbits+31)&~31)>>3;
        if (decodelen) T16decodeubits(tgv->indexMatrix,s,numindex,indexbits);
        s += decodelen;

        /* decode the motion frame */

        T16decodemotion(tgv);
    }
    tgv->packed = 1; /* Type of packing: 1=motion */
    return(ok);
}


/****************************************************************/
/*  Read TGV 16 bits key frame                                  */
/****************************************************************/

static int TGVdecodekeyframe(char *chunk, T16INFO *tgv)
{
    GPOS            framesize;
    int             i,ok=0;
    int             width,height;
    T16KeyStruct    *tgvkey;
    int             x,y,buffersize;
    unsigned short* savebuffer16;
    unsigned short* buffer16;
    unsigned int*  buffer32;
    unsigned short  Rgb5;
    unsigned int   Rgb8;

    /* reallocate frames */

    tgvkey = (T16KeyStruct *) chunk;
    if (tgv->current)
    {
        gfree(tgv->current);
        tgv->current = 0;
    }
    if (tgv->previous)
    {
        gfree(tgv->previous);
        tgv->previous = 0;
    }
    if (tgv->rowtbl)
    {
        gfree(tgv->rowtbl);
        tgv->rowtbl = 0;
    }
    if (tgv->indexMatrix)
    {
        gfree(tgv->indexMatrix);
        tgv->indexMatrix = 0;
    }
    width = tgv->frameWidth = T16GETFIELD(tgvkey->frameWidth);
    height = tgv->frameHeight = T16GETFIELD(tgvkey->frameHeight);
    framesize = width*height*sizeof(unsigned int);
    tgv->current = (char*)galloc(framesize);
    tgv->previous = (char*)galloc(framesize);
    tgv->rowtbl = (int*)galloc(height*2*sizeof(int));
    tgv->indexMatrix = (int*)galloc(framesize/16*sizeof(int));

    /* decode the current frames */

    if (tgv->current && tgv->previous)
    {
        for (i=0; i<height*2; ++i) tgv->rowtbl[i] = (i-height)*(width*sizeof(int));
        tgv->packed = 0;                                    /* Type of packing: 0=key */
        buffersize = width*height*sizeof(unsigned short);   /* x*y in 16 bits */
        buffer16=(unsigned short*)galloc(buffersize);       /* The buffer to decode the pixmap in 16 bits */
        if (buffer16)
        {
            savebuffer16=buffer16;
            REF_decode(chunk+sizeof(T16KeyStruct),(int *)0,(char*)buffer16);
            buffer32=(unsigned int*)tgv->current;
            for (y=0; y<height; ++y)
            {
                for (x=0; x<width; ++x)
                {
                    Rgb5=T16GETFIELD(*buffer16);
                    buffer16++;
                    C5to8(Rgb5,Rgb8);
                    *buffer32++=Rgb8;
                }
            }
            gfree((char*)savebuffer16);
            ok = 1;
        }
    }
    return(ok);
}


/****************************************************************/
/*  Chunk Parse Functions                                       */
/****************************************************************/

#define TGVisalphanum(c) (c>=' ' && c<='z')

static int T16ischunkid(char *id)
{
    if (TGVisalphanum(id[0]))
    {
        if (TGVisalphanum(id[1]))
        {
            if (TGVisalphanum(id[2]))
            {
                if (TGVisalphanum(id[3])) return(1);
            }
        }
    }
    return(0);
}


static int TGVread(GSTREAM *g, char *buf, int len, int doit)
{
    int ok = 0;

    switch (doit)
    {
        default:
        case CHUNKLOAD:ok = gread(g,buf,len);break;
        case CHUNKSKIP:ok = gseek(g,gtell(g)+len);break;
    }
    return(ok);
}


/****************************************************************/
/*  Read TGV 16 bits                                            */
/****************************************************************/

static char *T16getchunk(GSTREAM *g, T16INFO *tgv, int doit)
{
    int     ok=0;
    int    len,filelen,curpos;
    char    buf[16];
    char    *chunk=0;

    curpos = gtell(g);
    filelen = glen(g);
    if (curpos<filelen) ok = gread(g,buf,8);
    if (ok) /* if chunk, just load it */
    {
        len = ggeti(buf+4,4);
        ok = 0;
        if (T16ischunkid(buf) && (len+curpos)<=filelen)
        {
            chunk = (char*)galloc(len);
            if (chunk)
            {
                memcpy(chunk,buf,8);
                ok = TGVread(g,chunk+8,len-8,doit);
                if (!ok)
                {
                    gfree(chunk);
                    chunk = 0;
                }
            }
            tgv->chunkSize = len;
        }
    }
    return(chunk);
}


static int TGVseek(GINSTANCE *gx, T16INFO *tgv, int framenum, int doit)
{
    int     frame;
    int     ok=1;
    int    keyoff;
    char    *chunk;

    frame = gx->framenum;
    while (frame!=framenum && ok)
    {
        if (framenum<frame && framenum<tgv->keyframe)   /* seek backward beyond keyframe */
        {
            tgv->keyframe = 0;
            tgv->keyoffset = 0;
        }
        if (framenum<frame && tgv->keyframe<frame)      /* seek backward to keyframe */
        {
            gseek(gx->gstream,tgv->keyoffset);
            frame = tgv->keyframe-1;                    /* as if decoded the frame previous to key */
        }
        if (framenum>(frame+1) && tgv->keyframe>(frame+1) && framenum>=(tgv->keyframe))
        {                                               /* seek forward to keyframe */
            gseek(gx->gstream,tgv->keyoffset);
            frame = tgv->keyframe-1;
        }
        ok = 0;
        keyoff = gtell(gx->gstream);
        chunk = T16getchunk(gx->gstream, tgv, doit);
        if (chunk)
        {
            if (doit!=CHUNKLOAD) ok = 1;
            switch (ggeti(chunk,4))
            {
                case T16K:      /* key frame chunks */
                    ++frame;
                    if (doit==CHUNKLOAD) ok = TGVdecodekeyframe(chunk, tgv);
                    tgv->keyoffset = keyoff;
                    tgv->keyframe  = frame;
                    break;
                case T16F:      /* frame chunks */
                    ++frame;
                    if (doit==CHUNKLOAD) ok = TGVdecodeframe(chunk, tgv);
                    break;
                default:        /* audio chunks */
                    ok = 1;
                    break;
            }
            gfree(chunk);
        }
    }
    gx->framenum = frame;
    return(ok);
}


/****************************************************************/
/*  Information Functions                                       */
/****************************************************************/

GABOUT * GCALL T16_about(void)
{
    GABOUT *info;

    info=(GABOUT *) galloc(sizeof(GABOUT));
    if (info)
    {
        memset(info, 0, sizeof(GABOUT));
        info->signature     = T16SIGN;
        info->size          = sizeof(GABOUT);
        info->version       = GIMEX_VERSION;
        info->canimport        = 1;    /* supports importing */
        info->canexport        = 1;    /* supports exporting */
        info->importpacked  = 1;    /* supports importing packed */
        info->exportpacked  = 1;    /* supports exporting packed */
        info->import8       = 0;    /* supports importing 8 bit indexed */
        info->export8       = 0;    /* supports exporting 8 bit indexed */
        info->import32      = 1;    /* supports importing 32 bit direct rgb */
        info->export32      = 1;    /* supports exporting 32 bit direct rgb */
        info->multiframe    = 1;    /* supports multiple frames */
        info->multifile     = 0;    /* format requires additional files or resource fork */
        info->multisize     = 1;    /* supports each frame different size */
        info->framebuffer   = 1;    /* module requires memory to buffer entire frame */
        info->external      = 0;    /* module uses external resources */
        info->usesfile      = 1;    /* module is file based */
        info->maxframename  = 0;    /* max frame name character */
        info->defaultquality= 0; 100;  /* default pack quality */
        info->mactype[0]    = T16SIGN;                          /* mac file system types used */
        strcpy(info->extensions[0], ".T16");                    /* extensions */
        strcpy(info->authorstr,     "L.Ancessi & F.Barchard");  /* author */
        strcpy(info->versionstr,    "1.06");                    /* version # */
        strcpy(info->shorttypestr,  "T16");                     /* type */
        strcpy(info->wordtypestr,   "T16");                     /* wordtype */
        strcpy(info->longtypestr,   "T16 Movie");               /* longtype */
    }
    return(info);
}


int GCALL T16_is(GSTREAM *g)
{
    unsigned char   T16header[4];
    int             ok=0;
    char*           chunk;

    gseek(g,(GPOS)0);
    if (gread(g,T16header,4))
    {
        if (ggeti(T16header,4)==T16K) ok=100;   /* The first chunk is a key frame */
    }
    return(ok);
}


/****************************************************************/
/*  Import Functions                                            */
/****************************************************************/

int GCALL T16_open(GINSTANCE** gxptr,GSTREAM* g,const char* pathname,bool framecountflag)
{
    GINSTANCE   *gx;
    T16INFO     *tgv;
    int         ok=0;

    gx = (GINSTANCE *) galloc(sizeof(GINSTANCE));
    if (gx)
    {
        memset(gx, 0, sizeof(GINSTANCE));
        *gxptr = gx;
        gx->signature=T16SIGN;
        gx->size=sizeof(GINSTANCE);
        gx->gstream = g;
        gx->framenum = -1;                              /* keep track of which frame were pointing at */
        gseek(g,(GPOS)0);
        gx->gref = (char*)galloc(sizeof(T16INFO));
        if (gx->gref)
        {
            tgv = (T16INFO *) gx->gref;
            memset((void *) tgv,0,sizeof(T16INFO));
            gx->frames = 1000000L;
            TGVseek(gx, tgv, 1000000L, CHUNKSKIP);      /* note return 0 is ok this time */
            ok = gx->frames = (gx->framenum+1);
            gseek(g,(GPOS)0);
            gx->framenum = -1;
        }
    }
    return(ok);
}


GINFO * GCALL T16_info(GINSTANCE *gx, int framenum)
{
    GINFO       *ginfo=0;
    T16INFO     *tgv;
    int         ok;

    tgv = (T16INFO *) gx->gref;
    ok = TGVseek(gx, tgv, framenum, CHUNKLOAD);
    if (ok)
    {
        ginfo = (GINFO *) galloc(sizeof(GINFO));
        if (ginfo)
        {
            memset(ginfo,0,sizeof(GINFO));
            ginfo->signature        = T16SIGN;              /* Gimex type */
            ginfo->size             = sizeof(GINFO);        /* Length of GINFO */
            ginfo->version          = GIMEX_VERSION;
            ginfo->framenum         = framenum;             /* Current frame */
            ginfo->width            = tgv->frameWidth;      /* Width of bitmap in pixels */
            ginfo->height           = tgv->frameHeight;     /* Height of bitmap in pixels */
            ginfo->subtype          = 0;                    /* Internal format */
            ginfo->packed           = tgv->packed;          /* Type of packing on original image. 0=key, 1=motion */
            if (!tgv->packed) ginfo->quality = 100;         /* key frame */
            else                                            /* perform reverse of datarate limit calculation used by tgvwrite */
            {
                ginfo->quality=tgv->chunkSize*100/(ginfo->width*ginfo->height*sizeof(unsigned short));
                if (ginfo->quality > 99) ginfo->quality = 99;
            }
            ginfo->alphabits        = 0;    /* Number of bits in alpha channel */
            ginfo->redbits          = 5;    /* Number of bits in red channel */
            ginfo->greenbits        = 5;    /* Number of bits in grreen channel */
            ginfo->bluebits         = 5;    /* Number of bits in blue channel */
            ginfo->centerx          = 0;    /* Center Point relative to upper left corner */
            ginfo->centery          = 0;
            ginfo->defaultx         = 0;    /* Default Coordinate Point */
            ginfo->defaulty         = 0;
            ginfo->numhotspots      = 0;    /* Number of Hot Spots defined */
            ginfo->bpp              = 32;   /* Bits per pixel (8 or 32) */
            /*ginfo->indexed          = 0;  \* 0 no, 1 yes (indexed meaning it uses a palette) *\ */
            ginfo->originalbpp      = 16;   /* Bits per pixel in original image (1 to 32) */
            /*ginfo->originalindexed  = 0;  \* 0 no, 1 yes (indexed meaning it used a palette) *\ */
            ginfo->numcolours  = 0; /* Number of colours in original indexed palette */
        }
    }
    return(ginfo);
}


int GCALL T16_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes)
{
    int         bpl,minbpl,y,ok;
    char        *d;
    char        *s;
    T16INFO     *tgv;

    tgv = (T16INFO *) gx->gref; /* seek to frame and skip header if there is one. */
    ok = TGVseek(gx, tgv, ginfo->framenum, CHUNKLOAD);
    if (ok)
    {
        bpl = tgv->frameWidth*sizeof(unsigned int);
        if (bpl<rowbytes) minbpl=bpl;
        else minbpl=rowbytes;
        d = dest;
        s = tgv->current;
        for (y=0; y<ginfo->height; ++y)
        {
            memcpy(d,s,minbpl);
            s += bpl;
            d += rowbytes;
        }
    }
    return(ok);
}


int GCALL T16_close(GINSTANCE *gx)
{
    T16INFO     *tgv;
    int         ok=0;

    if (gx)
    {
        if (gx->gref)
        {
            tgv = (T16INFO *) gx->gref;
            if (tgv->literal)     gfree(tgv->literal);
            if (tgv->motion)      gfree(tgv->motion);
            if (tgv->offset)      gfree(tgv->offset);
            if (tgv->current)     gfree(tgv->current);
            if (tgv->previous)    gfree(tgv->previous);
            if (tgv->rowtbl)      gfree(tgv->rowtbl);
            if (tgv->indexMatrix) gfree(tgv->indexMatrix);
            gfree((void *)gx->gref);
            gx->gref = 0;
        }
        ok = gfree((void *) gx);
    }
    return(ok);
}
