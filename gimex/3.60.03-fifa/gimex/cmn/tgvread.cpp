// (c) 1994-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*                         TGV - Movie Gimex                        */
/*                                                                  */
/*             by Laurent Ancessi and Frank Barchard, EAC           */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History:                                       */
/* ------- ----   -- --------                                       */
/*    1.00 941215 LA Mac TGV Movie file format read write           */
/*    1.01 950110 FB Importer portable/align TGVk & use rowbytes    */
/*    1.02 950111 FB 32 bit DrawMatrix, CopyLit, getbitfield        */
/*    1.03 950113 LA Compression of the key frame using refpack     */
/*    1.04 950115 LA Intel/Motorola independancy on key frame       */
/*    1.05 950118 LA Autobuild Key frame                            */
/*    1.06 950119 LA Optimisation of the motion vector histogram    */
/*    1.07 950119 LA tgvcom.c:functions used by tgvread & tgvwrite  */
/*    1.08 950120 LA Structure multidictionary                      */
/*    1.09 950121 LA Literal using multiple colour depth            */
/*    1.10 950123 LA Colour reduction of literal (StatColour alg.)  */
/*    1.11 950115 LA Intel/Motorola independancy on frame           */
/*    1.12 950131 FB PC importer optimizations for 4x4              */
/*    1.13 950202 LA Remove multiple references on a motion bloc    */
/*    1.14 950201 FB P5 memory corruption bug checks                */
/*    1.15 950205 FB Rewritten based on MVI importer                */
/*    1.16 950205 FB Integrated importer/exporter code, numlit      */
/*    1.17 950208 FB Exporter just tgvwrite and fix rowbyte bug     */
/*    1.18 950208 LA Importer fix one vector bug                    */
/*    1.19 950208 LA Color reduction on literal of 4 colours        */
/*    1.20 950208 LA Save previous image logical                    */
/*    1.21 950214 FB Ignore audio chunks                            */
/*    1.22 950215 FB Allow audio chunk as first chunk in file       */
/*    1.23 950215 LA Bug alignement fix                             */
/*    1.23 950217 LA Bug duplicate vectors                          */
/*    1.23 950218 LA Optimisation of GetBestDelta                   */
/*    1.24 950220 LA Color reduction optimisation                   */
/*    1.25 950221 LA Calcul of the best literal bloc                */
/*    1.26 950222 LA Speed optimisation of minor functions          */
/*    1.27 950223 FB PSX move longs a byte at a time                */
/*    1.28 950315 LA Enable optimisation dictionary                 */
/*    1.29 950320 LA Subtype is the first color in the palette      */
/*    1.30 950320 LA Fix read quality in a frame                    */
/*    1.31 950320 LA Transparent color (0) are not used             */
/*    1.32 950411 LA Fix int* alignement on SGI                     */
/*    1.33 950427 LA Fix key frame alignement                       */
/*    1.34 950529 FB ginstance signature                            */
/*    1.35 950607 FB tgvtype.h added to tgvgimex.h                  */
/*    1.36 950804 FB align keyframe with ~3                         */
/*    1.37 950901 AL Debug index frequency                          */
/*    1.38 950914 FB raw keyframes no header, set flag to 1         */
/*    1.39 951109 FB rewrote compressliteral4 for powermac          */
/*    1.40 000404 FB about->movie                                   */
/*    1.41 001205 FB TGV_is check if 2nd chunk is EOF               */
/*    1.42 030210 FB ischunkid more strict about valid chunk id     */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 8 bit                                                            */
/* Reentrant                                                        */
/* Files: tgvread.cpp, tgvwrite.cpp, tgvgimex.h                     */
/*        refread.c, refwrite.c, refcodex.h                         */
/*                                                                  */
/* SubTypes                                                         */
/*  1 strictly use keyframes when specified                         */
/*  2 dont pack keyframes                                           */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Format Notes:                                                    */
/* -------------                                                    */
/* TGV stands for:                                                  */
/*   Tile Graphics Velocity officially                              */
/*   Train a Grande Vitesse really                                  */
/*   Totally Gorgious Video unofficially                            */
/* 8 bit, multiframe, chunk based, Intel.                           */
/* 4x4 motion compensation with dictionary containing motion        */
/* vectors and literals in tables and indexed.                      */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include <string.h>
#include "gimex.h"
#include "tgvgimex.h"

#include "codex.h"
#include "refcodex.h"

#define CHUNKLOAD  0
#define CHUNKSKIP  1

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

#define TGVgetfield(ptr,bitoff,n) ((ggetilong((ptr)+((bitoff)>>3)) >> ((bitoff)&7)) & ((1<<n)-1))

/* decode signed bits */

static void TGVdecodesbits(int *CPtr,char *SPtr,int NbCell,int CDepth)
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
        Value = TGVgetfield(SPtr,bitoff,CDepth);
        bitoff -= CDepth;

        if (Value&signbit) Value |= NegVal; /* Complement to 32 */

        *--d = (int) Value;
    }
}

/* decode unsigned bits */

static void TGVdecodeubits(int *CPtr,char *SPtr,int NbCell,int CDepth)
{
    int  i;
    int  bitoff;
    int  *d;

    bitoff = (NbCell-1)*CDepth;
    d = CPtr + (NbCell);

    for (i=0; i<NbCell; i++)
    {
        *--d = TGVgetfield(SPtr,bitoff,CDepth);
        bitoff -= CDepth;
    }
}

static void TGVdecodeliteral4(unsigned char *s, unsigned char *d, int numlit)
{
    unsigned int   matrix;
    int i;

    for (i=0; i<numlit; ++i)
    {
        matrix=ggetilong(s+4);

        d[0]=  s[(matrix>>30)&3];
        d[1]=  s[(matrix>>28)&3];
        d[2]=  s[(matrix>>26)&3];
        d[3]=  s[(matrix>>24)&3];

        d[4]=  s[(matrix>>22)&3];
        d[5]=  s[(matrix>>20)&3];
        d[6]=  s[(matrix>>18)&3];
        d[7]=  s[(matrix>>16)&3];

        d[8]=  s[(matrix>>14)&3];
        d[9]=  s[(matrix>>12)&3];
        d[10]= s[(matrix>>10)&3];
        d[11]= s[(matrix>> 8)&3];

        d[12]= s[(matrix>> 6)&3];
        d[13]= s[(matrix>> 4)&3];
        d[14]= s[(matrix>> 2)&3];
        d[15]= s[(matrix>> 0)&3];
        s += 8;
        d += 16;
    }
}

static int TGVdecodemotion(TGVINFO *tgv)
{
    int  width        = 0;
    int  y            = 0;
    int  nummotion    = 0;
    char *dest        = 0;
    char *destend     = 0;
    char *source      = 0;
    int offsettoprev  = 0;
    int  *indexptr    = 0;
    int  nextindex    = 0;
    int  *checktgv    = 0;
    int  i,totalindex = 0;

    width =     tgv->frameWidth;
    nummotion = tgv->numMotion;
    indexptr =  tgv->indexMatrix;
    dest =      tgv->current;
    offsettoprev = tgv->previous-tgv->current;

    if (TGVDEBUG)
    {
        totalindex=tgv->numMotion+tgv->numLiteral16+tgv->numLiteral4;
        checktgv=(int*) galloc(totalindex*sizeof(int));
        memset(checktgv, 0, totalindex*sizeof(int));
    }

    for (y=0; y<tgv->frameHeight; y+=4)
    {
        destend = dest+width;
        do
        {
            nextindex = *indexptr++;

            if (TGVDEBUG)
            {
                if ((nextindex>=totalindex) || (nextindex<0)) return(0);
                checktgv[nextindex]++;
            }

            if (nextindex<nummotion)    /* motion */
            {
                source = dest+offsettoprev+tgv->offset[nextindex];
                MOVLONG(dest,         source);
                MOVLONG(dest+width,   source+width);
                MOVLONG(dest+width*2, source+width*2);
                MOVLONG(dest+width*3, source+width*3);
            }
            else /* literal */
            {
                source = tgv->literal+(nextindex-nummotion)*16;
                *(int *)(dest)         = *(int *) (source);
                *(int *)(dest+width)   = *(int *) (source+4);
                *(int *)(dest+width*2) = *(int *) (source+8);
                *(int *)(dest+width*3) = *(int *) (source+12);
            }

            dest += 4;
        }
        while (dest<destend);

        dest += width*3;
    }

    if (TGVDEBUG)
    {
        for (i=0; i<totalindex; i++) if (!checktgv[i]) return(0);
        gfree(checktgv);
    }

    return(1);
}

/* decode the frame field we are currently pointing at */
/* store results in tgv structure */

static int TGVdecodeframe(char *chunk, TGVINFO *tgv)
{
    int  ok=1;
    int  i;
    int  width;
    int  height;
    int  nummotion;
    int  numliteral16;
    int  numliteral4;
    int  indexbits;
    int  numliteral;
    int  numindex;
    int decodelen;
    char *s;
    char *tempframe;
    int  *rowtbl;
    TGVFrameStruct *tgvframe;

    tgvframe = (TGVFrameStruct *) chunk;
    width  = tgv->frameWidth;
    height = tgv->frameHeight;

    /* make current previous, previous current */

    tempframe       = tgv->previous;
    tgv->previous   = tgv->current;
    tgv->current    = tempframe;

    /* read frame header */

    nummotion     = tgv->numMotion    = TGVGETFIELD(tgvframe->numMotion);
    numliteral16  = tgv->numLiteral16 = TGVGETFIELD(tgvframe->numLiteral16);
    numliteral4   = tgv->numLiteral4  = TGVGETFIELD(tgvframe->numLiteral4);
    indexbits     = tgv->indexBits    = TGVGETFIELD(tgvframe->indexBits);

    /* reallocate motion and literal blocks */

    if (nummotion)
    {
        if (nummotion>tgv->maxMotion)
        {
            if (tgv->motion)
            {
                gfree((void *)tgv->motion);
                tgv->motion = (TGVMOTION *) 0;
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
            tgv->motion = (TGVMOTION *) galloc(nummotion*sizeof(TGVMOTION));
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

    numliteral = numliteral16+numliteral4;
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
            tgv->literal = (char*) galloc(numliteral*4*4);
            tgv->maxLiteral = numliteral;
        }
        if (!tgv->literal) ok=0;
    }

    if (ok)
    {
        /* decode motion vectors */

        decodelen = ((nummotion*MOTIONBITS*2+31)&~31)>>3;
        s = chunk+sizeof(TGVFrameStruct);
        TGVdecodesbits(&tgv->motion[0].x,s,nummotion*2,MOTIONBITS);
        rowtbl = tgv->rowtbl+height;
        for (i=0; i<nummotion; ++i)
            tgv->offset[i] = rowtbl[tgv->motion[i].y]+tgv->motion[i].x;
        s += decodelen;

        /* decode literal16 */

        decodelen = numliteral16*4*4;
        if (decodelen)
            memcpy(tgv->literal,s,decodelen);
        s += decodelen;

        /* decode literal4 */

        decodelen = numliteral4*8;
        if (decodelen)
            TGVdecodeliteral4((unsigned char*)s,(unsigned char*)(tgv->literal+numliteral16*4*4),numliteral4);
        s += decodelen;

        /* decode index matrix */

        numindex = width*height>>4;
        decodelen = ((numindex*indexbits+31)&~31)>>3;
        if (decodelen)
            TGVdecodeubits(tgv->indexMatrix,s,numindex,indexbits);
        s += decodelen;

        /* decode the motion frame */

        ok=TGVdecodemotion(tgv);
    }
    tgv->packed = 1; /* Type of packing: 1=motion */
    return(ok);
}

/* key frame includes header information and palette and is
   responsible for resizing frame buffers
*/

static int TGVdecodekeyframe(char *chunk, TGVINFO *tgv)
{
    int  i;
    int  ok=0;
    int  colours;
    int  colourstart;
    GPOS framesize;
    int  rowoff;
    int  width;
    int  height;
    int  flags;
    TGVKeyStruct *tgvkey;

    tgvkey = (TGVKeyStruct *) chunk;

    width =       tgv->frameWidth  = TGVGETFIELD(tgvkey->frameWidth );
    height =      tgv->frameHeight = TGVGETFIELD(tgvkey->frameHeight);
    colourstart = tgv->firstColour = TGVGETFIELD(tgvkey->firstColour);
    colours =     tgv->colourCount = TGVGETFIELD(tgvkey->colourCount);
    flags                          = TGVGETFIELD(tgvkey->flags);

    for (i=0; i<colours; ++i)
    {
        tgv->colourtbl[i+colourstart].a = (i==255)?0:255;
        tgv->colourtbl[i+colourstart].r = tgvkey->colourtbl[i][0];
        tgv->colourtbl[i+colourstart].g = tgvkey->colourtbl[i][1];
        tgv->colourtbl[i+colourstart].b = tgvkey->colourtbl[i][2];
    }

    framesize = (width*height+3)&~3;

    /* reallocate frames */

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

    tgv->current =     (char*) galloc((int)framesize);
    tgv->previous =    (char*) galloc((int)framesize);
    tgv->rowtbl   =    (int*) galloc(height*2*sizeof(int));
    tgv->indexMatrix = (int*) galloc((int)framesize/16*sizeof(int));

    if (tgv->current && tgv->previous && tgv->rowtbl && tgv->indexMatrix)
    {
        rowoff = -height*width;
        for (i=0; i<height*2; ++i)
        {
            tgv->rowtbl[i] = rowoff;
            rowoff += width;
        }
        tgv->packed = 0; /* Type of packing: 0=key */

        if (flags&1)
            memcpy(tgv->current,chunk+20+colours*3,(unsigned int)framesize);
        else
            REF_decode(tgv->current,chunk+20+colours*3, (int *)0);
        ok = 1;
    }

    return(ok);
}

/****************************************************************/
/*  Chunk Parse Functions                                       */
/****************************************************************/

static int TGVisalphanum(char c)
{
	if ((c>='A' && c<='Z')
	 || (c>='a' && c<='z')
	 || (c>='0' && c<='0'))
	{
		return 1;
	}
	return 0;
}


static int TGVischunkid(char *id)
{
    if (TGVisalphanum(id[0])
     && TGVisalphanum(id[1])
     && TGVisalphanum(id[2])
     && TGVisalphanum(id[3]))
        return(1);
    return(0);
}

static int TGVread(GSTREAM *g, char *buf, int len, int doit)
{
    int ok = 0;

    switch (doit)
    {
        default:
        case CHUNKLOAD:
            ok = gread(g,buf,len);
            break;

        case CHUNKSKIP:
            ok = gseek(g,gtell(g)+len);
            break;
    }
    return(ok);
}

/****************************************************************/
/*  Get Chunk                                                   */
/*  - loads next chunk into memory.  no parsing if just a chunk */
/*  - if not a chunk, convert into chunk                        */
/****************************************************************/

/* get a chunk.  just loads the chunk if chunk based file. */
/* non-chunk based 'chunks' are converted into chunks */

static char *TGVgetchunk(GSTREAM *g, TGVINFO *tgv, int doit)
{
    int  ok=0;
    int len;
    int filelen;
    int curpos;
    char buf[16];
    char *chunk=0;

    curpos =  (int)gtell(g);
    filelen = (int)glen(g);
    if (curpos<filelen)
        ok = gread(g,buf,8);
    if (ok)
    {
        /* if chunk, just load it */

        len = ggeti(buf+4,4);
        ok = 0;
        if (TGVischunkid(buf) && (len+curpos)<=filelen)
        {
            chunk = (char*) galloc(len);
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

static int TGVseek(GINSTANCE *gx, TGVINFO *tgv, int framenum, int doit)
{
    int  frame;
    int  ok=1;
    int keyoff;
    char *chunk;

    frame = gx->framenum;

    while (frame!=framenum && ok)
    {
        /* seek backward beyond keyframe */

        if (framenum<frame && framenum<tgv->keyframe)
        {
            tgv->keyframe = 0;
            tgv->keyoffset = 0;
        }

        /* seek backward to keyframe */

        if (framenum<frame && tgv->keyframe<frame)
        {
            gseek(gx->gstream,tgv->keyoffset);
            frame = tgv->keyframe-1;           /* as if decoded the frame previous to key */
        }

        /* seek forward to keyframe */

        if (framenum>(frame+1) && tgv->keyframe>(frame+1) && framenum>=(tgv->keyframe))
        {
            gseek(gx->gstream,tgv->keyoffset);
            frame = tgv->keyframe-1;
        }

        ok = 0;
        keyoff = (int)gtell(gx->gstream);
        chunk = TGVgetchunk(gx->gstream, tgv, doit);
        if (chunk)
        {
            if (doit!=CHUNKLOAD)
                ok = 1;

            switch (ggeti(chunk,4))
            {
                case GMAKEID('T','G','V','k'):
                    ++frame;
                    if (doit==CHUNKLOAD)
                    {
                        ok = TGVdecodekeyframe(chunk, tgv);
                    }
                    tgv->keyoffset = keyoff;
                    tgv->keyframe  = frame;
                    break;

                case GMAKEID('T','G','V','f'):
                    ++frame;
                    if (doit==CHUNKLOAD)
                    {
                        ok = TGVdecodeframe(chunk, tgv);
                    }
                    break;

                default: /* audio chunks */
                    ok = 1;
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

GABOUT * GCALL TGV_about(void)
{
    GABOUT *info;

    info = (GABOUT *) galloc(sizeof(GABOUT));
    if (info)
    {
        memset(info, 0, sizeof(GABOUT));

        info->signature       = GMAKEID('T','G','V','i');
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
        info->multisize       = 1;    /* supports each frame different size */
        info->framebuffer     = 1;    /* module requires memory to buffer entire frame */
        info->external        = 0;    /* module uses external resources */
        info->usesfile        = 1;    /* module is file based */
        info->maxframename    = 0;    /* max frame name character */
        info->defaultquality  = 18;   /* default pack quality */
        info->movie           = 1;    /* movie file format */
        info->mactype[0]      = GMAKEID('T','G','V','i');       /* mac file system types used */
        strcpy(info->extensions[0], ".tgv");                    /* extensions */
        strcpy(info->authorstr,     "L.Ancessi & F.Barchard");  /* author */
        strcpy(info->versionstr,    "1.42");                    /* version # */
        strcpy(info->shorttypestr,  "TGV");                     /* type */
        strcpy(info->wordtypestr,   "TGV");                     /* wordtype */
        strcpy(info->longtypestr,   "TGV Movie");               /* longtype */
    }
    return(info);
}

/* check for reasonable header: */
/* 'TGVk' intel chunk as either first or second chunk (audio) */

int GCALL TGV_is(GSTREAM *g)
{
    unsigned char tgvheader[8];
    int ok=0;
    int chunklen;
    char *chunk;
    TGVINFO tgv;

    gseek(g,(GPOS)0);
	memset(tgvheader,0,sizeof(tgvheader));
    if (gread(g,tgvheader,8))
    {
        if (ggeti(tgvheader,4)==GMAKEID('T','G','V','k'))
            ok = 100;
        else
        {
            chunklen = ggeti(tgvheader+4,4);
            if (chunklen>8 && chunklen<65536)
            {
                if (TGVischunkid((char*)tgvheader))
                {
                    gseek(g,(GPOS)0);
                    chunk = TGVgetchunk(g, &tgv, 0);
                    if (chunk)
                    {
                        if (gtell(g)<glen(g))
                        {
                            if (gread(g,tgvheader,4))
                            {
                                if (ggeti(tgvheader,4)==GMAKEID('T','G','V','k'))
                                    ok = 100;
                            }
                        }
                        gfree(chunk);
                    }
                }
            }
        }
    }
    return(ok);
}


/****************************************************************/
/*  Import Functions                                            */
/****************************************************************/

/* opentgv seeks to last frame to determine number of frames */

int GCALL TGV_open(GINSTANCE **gxptr, GSTREAM *g, const char *pathname,bool framecountflag)
{
    GINSTANCE *gx;
    TGVINFO *tgv;
    int  ok=0;

    gx = (GINSTANCE *) galloc(sizeof(GINSTANCE));
    if (gx)
    {
        memset(gx, 0, sizeof(GINSTANCE));
        *gxptr = gx;
        gx->signature=GMAKEID('T','G','V','i');
        gx->size=sizeof(GINSTANCE);
        gx->gstream = g;
        gx->framenum = -1;  /* keep track of which frame were pointing at */

        gseek(g,(GPOS)0);

        gx->gref = (char*) galloc(sizeof(TGVINFO));
        if (gx->gref)
        {
            tgv = (TGVINFO *) gx->gref;
            memset((void *) tgv,0,sizeof(TGVINFO));

            gx->frames = 1000000L;
            TGVseek(gx, tgv, 1000000L, CHUNKSKIP);  /* note return 0 is ok this time */

            ok = gx->frames = (gx->framenum+1);

            gseek(g,(GPOS)0);
            gx->framenum = -1;
        }
    }
    return(ok);
}

GINFO * GCALL TGV_info(GINSTANCE *gx, int framenum)
{
    GINFO *ginfo=0;
    TGVINFO *tgv;
    int  ok;

    tgv = (TGVINFO *) gx->gref;

    ok = TGVseek(gx, tgv, framenum, CHUNKLOAD);

    if (ok)
    {
        ginfo = (GINFO *) galloc(sizeof(GINFO));
        if (ginfo)
        {
            memset(ginfo,0,sizeof(GINFO));

            /* template for ginfo */

            ginfo->signature        = GMAKEID('T','G','V','i'); /* Gimex type */
            ginfo->size             = sizeof(GINFO);            /* Length of GINFO */
            ginfo->version          = GIMEX_VERSION;
            ginfo->framenum         = framenum;                 /* Current frame */
            ginfo->width            = tgv->frameWidth;          /* Width of bitmap in pixels */
            ginfo->height           = tgv->frameHeight;         /* Height of bitmap in pixels */
            ginfo->subtype          = 0;                        /* Internal format */
            ginfo->packed           = tgv->packed;              /* Type of packing on original image. 0=key, 1=motion */

            if (!tgv->packed)
                ginfo->quality = 100;               /* key frame */
            else
            {
                /* perform reverse of datarate limit calculation used by tgvwrite */
                ginfo->quality = tgv->chunkSize * 100 / (ginfo->width * ginfo->height + 10);
                if (ginfo->quality > 99) ginfo->quality = 99;
            }

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
            ginfo->originalbpp      = 8;    /* Bits per pixel in original image (1 to 32) */
            ginfo->numcolours  = tgv->firstColour+tgv->colourCount;  /* Number of colours in original indexed palette */

            memcpy(ginfo->colourtbl,tgv->colourtbl,256*4);
        }
    }
    return(ginfo);
}

int GCALL TGV_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes)
{
    int  bpl;
    int  y;
    int  ok;
    char *d;
    char *s;
    TGVINFO *tgv;

    /* seek to frame and skip header if there is one. */

    tgv = (TGVINFO *) gx->gref;
    ok = TGVseek(gx, tgv, ginfo->framenum, CHUNKLOAD);

    if (ok)
    {
        bpl = tgv->frameWidth;
        d = dest;
        s = tgv->current;
        for (y=0; y<ginfo->height; ++y)
        {
            memcpy(d,s,bpl);
            s += bpl;
            d += rowbytes;
        }
    }
    return(ok);
}

int GCALL TGV_close(GINSTANCE *gx)
{
    TGVINFO *tgv;
    int ok=0;

    if (gx)
    {
        if (gx->gref)
        {
            tgv = (TGVINFO *) gx->gref;
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

