// (c) 1996-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*      Lloyd - Lloyd Popular Histogram CRapi Module  v1.00         */
/*                                                                  */
/*                      by Frank Barchard, EAC                      */
/*                                                                  */
/*                    Main Module - Jun 23, 2003                    */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History                                        */
/* ------- ------ -- -------                                        */
/*    1.00 030623 FB based on exact                                 */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 8,32 bit                                                         */
/* Lloyd keeps list of all colours used.                            */
/* Adds the original palette with histogram of 0 to maintain        */
/* original palette if possible.                                    */
/* If there are fewer than 256 colours, the palette will be         */
/* lossless                                                         */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include "lloyd.h"
#include <string.h>
#include <stdlib.h>

typedef struct
{
    unsigned int argb;
    int count;
} XHIST;

const int HASHSIZE=50000;

typedef struct
{
    int numcolours;
    int histcolours;
    int histsize;
    int hashmin[HASHSIZE];
    int hashmax[HASHSIZE];
    XHIST *hist;
} XACT;

#if 0

// gst.cpp : Defines the entry point for the console application.
//

// 0=slow,1=faster,2=fastest
#define FASTER 1
#define DOALPHA 0
#define DUMPBLOCKS 1

// F2 5 step method
// 1. seed
// loop 2-4 no more splits
//  loop 2-3 until stable
//   2. map
//   3. center
//  4. split
// 5. save

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "crapi.h"
#include "resampleimage.h"

// *************************************
// Declarations
// *************************************

static const int MAXBLOCKPIXELS=8*2;
static const int NUMCHANNELS=4;
static const int MAXBLOCKCOLOURS=4;

struct Block
{
    int CenterColours;// normally 2 or 4, but could vary per block.  Split sets to 1
    ARGB Center[MAXBLOCKCOLOURS];
    int NewCenterColours;
    ARGB NewCenter[MAXBLOCKCOLOURS];
    double SumDiff; // total deviation for this block for all screen blocks that map to it. double for longer range than int
    int UseCount; // how many times was this block uses
    int WorstDiff;
    int WorstX;
    int WorstY;
};

static const int MAXBLOCKS=128;


struct BlockState
{                     // typical
    int BlockWidth;   // 4
    int BlockHeight;  // 2
    int DetailBpp;    // 1 or 2
    int BaseBpp;      // 4 or 8
    int BlockColours; // 2 or 4
    int ClutBpp;      // 16 or 32
    int NumBlocks;    // 64 or 128
    unsigned char *IndexMap;  // width*height/BLOCKPIXELS
    unsigned char *DetailMap; // width*height with values 0..3
    Block Blocks[MAXBLOCKS];
};

struct ShapeFile;


// *************************************
// Miscellaneous support
// *************************************

CRINSTANCE *WU_open(void)
{
    CRAPI_init();
#if FASTER>=2
    CRAPI_find("Wu555");
#elif FASTER>=1
    CRAPI_find("Wu");
#else
    CRAPI_find("Wu");
#endif
    CRINSTANCE *cx = CRAPI_open();
    return cx;
}

void WU_add(BlockState *State,CRINSTANCE *cx,const GBITMAP *image,int x,int y)
{
    GINFO ginfoblock={0,};
    memcpy(&ginfoblock,image->ginfo,sizeof(ginfoblock));
    ginfoblock.size = sizeof(ginfoblock);
    ginfoblock.width = State->BlockWidth;
    ginfoblock.height = State->BlockHeight;

    unsigned char *s = (unsigned char *)image->image + image->rowbytes*(y) + x*NUMCHANNELS;

    CRAPI_read(cx, &ginfoblock, s, image->rowbytes);
}

int WU_close(CRINSTANCE *cx,ARGB *cpal,int numcolours)
{
    int newcolours=0;
    ARGB *tpal = CRAPI_close(cx, numcolours, NULL, &newcolours);
    memcpy(cpal,tpal,numcolours*sizeof(ARGB));
    gfree(tpal);
    return newcolours;
}

// calculate new center for block

void CenterBlock(BlockState *State,  const GBITMAP *image, int i)
{
    CRINSTANCE *cx = WU_open();

    unsigned char *imap = State->IndexMap;
    for (int y=0; y<image->ginfo->height; y+= State->BlockHeight)
    {
        for (int x=0; x<image->ginfo->width; x+= State->BlockWidth)
        {
            // is this block of pixels mapping to our block?
            if ((int)*imap == i)
            {
                WU_add(State,cx,image,x,y);
            }
            ++imap;
        }
    }

    int newcolours = WU_close(cx,&State->Blocks[i].NewCenter[0],State->BlockColours);
    State->Blocks[i].NewCenterColours = newcolours;
}

// *************************************
// STEP 1: SEED
// 1 block of black
// for speed, consider using WU to seed with N (64?) blocks of 1 colour.
// *************************************

void GST_seed(BlockState *State, const GBITMAP *image)
{
    State->NumBlocks = 1;
    State->Blocks[0].CenterColours = 1;

    CenterBlock(State, image, 0);
}

// *************************************
// STEP 2: Map
// find closest block
// *************************************

// sum is sum of square errors
// sample count is width*height*channels
// just for diagnostics

static double psnr(double sum, double samplecount)
{
    const double MAXPSNR=128.0;
    const double MINSUM=pow(255.0/pow(10.0,MAXPSNR/20.0),2.0); // 1.0305767983978390437526664180478e-8;  // was 0.00000001

    double MSE = sum/samplecount;

    if (MSE < MINSUM)
        MSE = MINSUM;

    double psnr = -20.0 * log10(sqrt(MSE)/255.0);

    return psnr;
}



#if 0
static float RGBtoY(int R, int G, int B, int A)
{
    return (float) (( 0.299f * R + 0.587f * G + 0.114f * B ) * (A+256) / 255.0f);
}

int DiffPixels(ARGB *r, ARGB *p)
{
    float ry = RGBtoY(r->r,r->g,r->b,r->a);
    float py = RGBtoY(p->r,p->g,p->b,p->a);

    float d = ry-py;
    return (int) (d*d);
}
#endif

int DiffChannel(unsigned rc, unsigned rp)
{
    int d=(int)rc - (int) rp;
    return d*d;
}

int DiffPixels(ARGB *r, ARGB *p)
{
#if DOALPHA
    return DiffChannel(r->a,p->a)
         + DiffChannel(r->r,p->r)
         + DiffChannel(r->g,p->g)
         + DiffChannel(r->b,p->b);
#else
    return DiffChannel(r->r,p->r)
         + DiffChannel(r->g,p->g)
         + DiffChannel(r->b,p->b);
#endif
}

int ComparePixelToCenter(Block *Block, ARGB *p, unsigned char *DetailMap)
{
    int MinDiff=0x7fffffff;
    for (int i=0; i<Block->CenterColours; ++i)
    {
        int Diff = DiffPixels(Block->Center+i,p);
        if (Diff<=MinDiff)
        {
            MinDiff = Diff;
            *DetailMap = (unsigned char) i;
        }
    }
    return MinDiff;
}

// compare a block of screen pixels to a block in the block list
// improve by not being spatially sensitive... use indexed compares to a clut

int CompareBlock(Block *Block, const GBITMAP *image, unsigned char *DetailMap, int BlockX, int BlockY, int BlockWidth, int BlockHeight)
{
    int DiffSum=0;
    for (int y=0; y<BlockHeight; ++y)
    {
        unsigned char *DMap = DetailMap + (image->ginfo->width*(BlockY+y)) + BlockX;

        ARGB *s = (ARGB *)((unsigned char *)image->image + image->rowbytes*(BlockY+y) + BlockX*NUMCHANNELS);
        for (int x=0; x<BlockWidth; ++x)
        {
            int Diff = ComparePixelToCenter(Block,s,DMap);
            DiffSum += Diff;
            ++DMap;
            ++s;
        }
    }
    return DiffSum;
}

// for the given screen block of pixels, find the best block to match to

void MapBlock(BlockState *State, const GBITMAP *image, int BlockX, int BlockY)
{
    int BestDiff=0x7fffffff;
    int BestIndex=0;
    for (int i=0; i<State->NumBlocks; ++i)
    {
        int Diff = CompareBlock(State->Blocks+i,image,State->DetailMap,BlockX,BlockY,State->BlockWidth,State->BlockHeight);
        if (Diff < BestDiff)
        {
            BestDiff = Diff;
            BestIndex = i;
        }
    }
    // once more for detail map
    CompareBlock(State->Blocks+BestIndex,image,State->DetailMap,BlockX,BlockY,State->BlockWidth,State->BlockHeight);

    // remember which block the pixels mapped to.  This is useful to the final mapping stage
    unsigned char *imap = State->IndexMap + (image->ginfo->width/State->BlockWidth)*(BlockY/State->BlockHeight) + BlockX/State->BlockWidth;
    *imap = BestIndex;

    // sum of differences will be useful to figure out which block to split
    State->Blocks[BestIndex].SumDiff += (double)BestDiff;

    // Count number of times the block is used.  Will help split
    State->Blocks[BestIndex].UseCount += 1;

    // remember the worst match block.  This is useful to figure out the seed, if we split this block.
    if (BestDiff >= State->Blocks[BestIndex].WorstDiff)
    {
        State->Blocks[BestIndex].WorstDiff = BestDiff;
        State->Blocks[BestIndex].WorstX = BlockX;
        State->Blocks[BestIndex].WorstY = BlockY;
    }
}


// for each screen block, map to the best block in the list
// return psnr as float for debug
double GST_map(BlockState *State,const GBITMAP *image)
{
    for (int i=0; i<State->NumBlocks; ++i)
    {
        State->Blocks[i].SumDiff = 0;
        State->Blocks[i].WorstDiff = 0;
        State->Blocks[i].UseCount = 0;
    }

    // find best match and update IndexMap
    for (int y=0; y<image->ginfo->height; y+= State->BlockHeight)
        for (int x=0; x<image->ginfo->width; x+= State->BlockWidth)
            MapBlock(State,image,x,y);

    // just for diagnostics, calculate a PSNR
    // this can be removed later

    // normalize sum
    double sse=0.0;
    for (int i=0; i<State->NumBlocks; ++i)
        sse += State->Blocks[i].SumDiff;

    return psnr(sse,image->ginfo->width*image->ginfo->height*3); // ignore alpha
}


// *************************************
// STEP 3: Center
// for all screen blocks that map to a block array entry, recalculate center
// uses WU
// *************************************

// calculate new center for all block

void GST_center(BlockState *State,const GBITMAP *image)
{
    for (int i=0; i<State->NumBlocks; ++i)
    {
        CenterBlock(State, image, i);
    }
}

// *************************************
// STEP 3a: Compare and Update Center
// Did the center change?  If so, we'll need to iterate more
// *************************************

// did the center change?

int GST_centercmp(BlockState *State)
{
    int diff = 0;
    for (int i=0; i<State->NumBlocks && !diff; ++i)
    {
        diff = State->Blocks[i].NewCenterColours-State->Blocks[i].CenterColours;
        if (!diff)
            diff = memcmp(State->Blocks[i].Center,State->Blocks[i].NewCenter,State->Blocks[i].CenterColours*sizeof(ARGB));
    }
    return diff;
}

// copy NewCenter to Center
void GST_centerupdate(BlockState *State)
{
    for (int i=0; i<State->NumBlocks; ++i)
    {
        State->Blocks[i].CenterColours = State->Blocks[i].NewCenterColours;
        memcpy(State->Blocks[i].Center,State->Blocks[i].NewCenter,State->Blocks[i].NewCenterColours*sizeof(ARGB));
    }
}

// *************************************
// STEP 4: Split
// find best block to split
// create a new block with an initial seed
// *************************************

void DumpBlocks(BlockState *State)
{
    for (int i=0; i<State->NumBlocks; ++i)
    {
        printf("%d: %d %d %.0f %d %d %d %d\n", i,
            State->Blocks[i].CenterColours,// normally 2 or 4, but could vary per block.  Split sets to 1
            State->Blocks[i].NewCenterColours,
            State->Blocks[i].SumDiff, // total deviation for this block for all screen blocks that map to it. double for longer range than int
            State->Blocks[i].UseCount, // how many times was this block uses
            State->Blocks[i].WorstDiff,
            State->Blocks[i].WorstX,
            State->Blocks[i].WorstY);
    }
};

bool GST_split(BlockState *State,const GBITMAP *image)
{
    double MostDiff=0.0;
    int MostIndex=0;
    bool FoundSplit=false;

    // remove unused blocks
    int j=0;
    for (int i=0; i<State->NumBlocks; ++i)
    {
        memcpy(&State->Blocks[j],&State->Blocks[i],sizeof(State->Blocks[j]));
        if (State->Blocks[j].UseCount>0)
        {
            ++j;
        }
        else
        {
            printf("warning - unused block %d removed.\n",i);
        }
    }
    State->NumBlocks = j;

    // find the block to split
    for (int i=0; i<State->NumBlocks; ++i)
    {
        if (State->Blocks[i].SumDiff > MostDiff && State->Blocks[i].UseCount>1)
        {
            MostDiff = State->Blocks[i].SumDiff;
            MostIndex = i;
            FoundSplit = true;
        }
    }

    // seed new block with the worst matching source pixels
    if (FoundSplit)
    {
        int NumBlocks = State->NumBlocks;
        // put new block first, as we know it will get used, but the old block may remap elsewhere
        memcpy(&State->Blocks[NumBlocks],&State->Blocks[MostIndex],sizeof(State->Blocks[NumBlocks]));

        // update map
        int x = State->Blocks[MostIndex].WorstX;
        int y = State->Blocks[MostIndex].WorstY;
        unsigned char *imap = State->IndexMap + (image->ginfo->width/State->BlockWidth)*(y/State->BlockHeight) + x/State->BlockWidth;
        *imap = NumBlocks;

        State->NumBlocks++;
        CenterBlock(State, image, NumBlocks);   // update center for both parts of the split
        CenterBlock(State, image, MostIndex);
        GST_centerupdate(State);
    }
    return FoundSplit;
}

// *************************************
// STEP 5: Save
// *************************************


enum GstFlagMasks
{
    ELEMENT_FLAG_NEW_FORMAT   = 0x00000001,  //!< bit specifing whether this element is in the new shape format.  Real 5 shapes will have this disabled.
    ELEMENT_FLAG_COMPRESSED   = 0x00000002,  //!< determines whether or not this  shape element is compressed (specifically refpacked)
    ELEMENT_FLAG_OWNED        = 0x00000004,  //!< determines whether or not this shape element owns the class memory (ShapeElement::Destroy will free the memory if it is owned)
    ELEMENT_FLAG_OWNED_DATA   = 0x00000008,   //!< determines whether or not this shape element owns the data (ShapeElement::Destroy will free the data memory if it is owned)

    TEXTURE_FLAG_CUBE_MAP     = 0x00000100, //!< This is a cube map
    TEXTURE_FLAG_DOT3         = 0x00000200, //!< This is a dot3 bump map
    TEXTURE_FLAG_EMBM         = 0x00000400, //!< This isan environmental bump map
    TEXTURE_FLAG_HALF_ALPHA   = 0x00000800, //!< This texture uses halved alpha values (applies to PS2 only)
    TEXTURE_FLAG_TRANSPARENT  = 0x00001000, //!< Denotes that, if alpha is present, each texel is either fully opaque or fully transparent.

    TEXTURE_FLAG_OPAQUE       = 0x00002000, //!< Denotes that the entire image is fully opaque (i.e. alpha is at the maximum level).
    TEXTURE_FLAG_SWIZZLED     = 0x00004000, //!< This texture is swizzled
    TEXTURE_FLAG_TRANSPOSED   = 0x00008000, //!< The texture data is transposed
    TEXTURE_FLAG_NUM_MIP_MAPS = 0x00F00000  //!< Number of mipmaps beyond LOD 0
};


struct ShapeFile
{
    unsigned int mSignature;  // ShpS
    unsigned int mSize;       // sizeof the shape, including this header
};

struct Shape
{
    unsigned int mType         :8;  // TexelType
    unsigned int mFlags        :24; // combined flag bits
    int mNextOffset;   // next ShapeElement in the list
    int mDataOffset;   // offset to data
    int mDataSize;     // size of the data in bytes
    int mShapeX;
    int mShapeY;
    int mWidth;
    int mHeight;
};


// 00yyyxxx -> yyy0xxx0
unsigned char base_remap(unsigned char c,int DetailBpp=2)
{
    if (DetailBpp==1)
        return c<<1;

    // else 2
    return ((c&0x07)<<1) | ((c&0x38)<<2);
}

unsigned int GST_ShapeType(int BlockWidth=4,int BlockHeight=2,int DetailBpp=2)
{
    unsigned int t=0x80;
    if (BlockHeight==1) t = 0x88;
    else if (BlockWidth==2) t = 0x80;
    else if (BlockWidth==8) t = 0x84;
    else if (BlockHeight==4) t = 0x86;
    else t = 0x82;
    if (DetailBpp==2)
        ++t;
    return t;
}

; // GST4 4x2x2

ShapeFile * GST_Save(BlockState *State,GBITMAP *image, int *sizeofOutput)
{

    // *************************************
    // preamble - allocate buffers
    // *************************************

    int BlockWidth  = State->BlockWidth;
    int BlockHeight = State->BlockHeight;
    int BaseBpp     = State->BaseBpp;
    int DetailBpp   = State->DetailBpp;
    int ClutBpp     = State->ClutBpp;

    int width=image->ginfo->width;
    int height=image->ginfo->height;

    int sizeofShapeFileHeader = sizeof(ShapeFile);
    int sizeofBaseTextureData   = width*height/(BlockWidth*BlockHeight)*BaseBpp/8;
    int sizeofDetailTextureData = width*height*DetailBpp/8;
    int sizeofClutData = 256 * ClutBpp/8;
    int sizeofShape = sizeof(ShapeFile)
                    + sizeof(Shape) + sizeofBaseTextureData + sizeofDetailTextureData
                    + sizeof(Shape) + sizeofClutData;

    ShapeFile *output = (ShapeFile *) galloc(sizeofShape);
    memset(output,0,sizeofShape);
    *sizeofOutput = sizeofShape;

    output->mSignature = 'FfsS';
    output->mSize = sizeofShape;

    Shape *shp  = (Shape *)((char *) output + sizeof(ShapeFile));
    Shape *clut = (Shape *)((char *) output + sizeof(ShapeFile) + sizeof(Shape) + sizeofBaseTextureData + sizeofDetailTextureData);

    shp->mType = GST_ShapeType(BlockWidth,BlockHeight,DetailBpp); // GST4 4x2x2
    shp->mFlags = ELEMENT_FLAG_NEW_FORMAT;
    shp->mDataOffset = sizeof(Shape);
    shp->mDataSize = sizeofBaseTextureData + sizeofDetailTextureData;
    shp->mNextOffset = sizeof(Shape) + sizeofBaseTextureData + sizeofDetailTextureData;
    shp->mWidth = width;
    shp->mHeight = height;
    shp->mShapeX = BlockWidth; // for simplicity, we'll put block size here too.  May not be in the final codec
    shp->mShapeY = BlockHeight;

    clut->mType = 0x21; // 32 bpp SSH
    clut->mFlags = ELEMENT_FLAG_NEW_FORMAT;
    clut->mDataOffset = sizeof(Shape);
    clut->mDataSize = sizeofClutData;
    clut->mNextOffset = 0;  // TBD EAGL element?
    clut->mWidth = 256;
    clut->mHeight = 1;
    clut->mShapeX = 0;
    clut->mShapeY = 0;

    int DetailColours=1<<DetailBpp; // 2 or 4
    int BaseColours=1<<BaseBpp; // 256
    int MaxBlockColours=BaseColours/DetailColours; // 128 or 64

    // copy base texture
    unsigned char *basedata = (unsigned char *)shp+sizeof(Shape);
    for (int i=0; i<sizeofBaseTextureData; ++i)
        basedata[i] = base_remap(State->IndexMap[i],DetailBpp);

    // pack detail into 1 or 2 bits
    unsigned char *s=State->DetailMap;
    unsigned char *d=basedata+sizeofBaseTextureData;
    if (DetailBpp==2)
    {
        for (int i=0; i<image->ginfo->width*image->ginfo->height; i+=4)
        {
            *d = s[0]
              | (s[1]<<2)
              | (s[2]<<4)
              | (s[3]<<6);
            ++d;
            s+=4;
        }
    }
    else // (DetailBpp==1)
    {
        for (int i=0; i<image->ginfo->width*image->ginfo->height; i+=8)
        {
            *d = s[0]
              | (s[1]<<1)
              | (s[2]<<2)
              | (s[3]<<3)
              | (s[4]<<4)
              | (s[5]<<5)
              | (s[6]<<6)
              | (s[7]<<7);
            ++d;
            s+=8;
        }
    }

    // copy clut
    unsigned char *clutdata=(unsigned char *)clut+sizeof(Shape);
    for (int i=0; i<MaxBlockColours; ++i)
    {
        int pi = base_remap(i,DetailBpp);
        for (int j=0; j<DetailColours; ++j)
        {
            int pj = pi+(j&1)+((j&2)<<3); // now 0..255
            clutdata[pj*4+0] = State->Blocks[i].Center[j].r;
            clutdata[pj*4+1] = State->Blocks[i].Center[j].g;
            clutdata[pj*4+2] = State->Blocks[i].Center[j].b;
            clutdata[pj*4+3] = State->Blocks[i].Center[j].a;
        }
    }

    return output;
}


// *************************************
// Compress
// main compression entry point that creates a compressed shape
// *************************************

ShapeFile * GSTCompressImage(GBITMAP *image, int *sizeofOutput,
                int BlockWidth=4,int BlockHeight=2,int DetailBpp=2,int BaseBpp=8,int ClutBpp=32)
{
#if FASTER>=2
    int MaxBlocks = 8;
    const int MAXMAPS=1;
    const int MAXMAPSLAST=2;
    const int SETTLINGFRAMES=1;
#elif FASTER>=1
    int MaxBlocks = 1<<(BaseBpp-DetailBpp); // 64 typical
    const int MAXMAPS=1;
    const int MAXMAPSLAST=8;
    const int SETTLINGFRAMES=4;
#else
    int MaxBlocks = 1<<(BaseBpp-DetailBpp); // 64 typical
    const int MAXMAPS=4;
    const int MAXMAPSLAST=32;
    const int SETTLINGFRAMES=8;  // number of initial blocks splits to do with lloyd allowed to settle
#endif

    BlockState State={0,};
    State.BlockWidth  = BlockWidth;
    State.BlockHeight = BlockHeight;
    State.DetailBpp   = DetailBpp;
    State.BaseBpp     = BaseBpp;
    State.ClutBpp     = ClutBpp;
    State.BlockColours = 1<<DetailBpp;
    State.NumBlocks = 1;

    int SizeOfIndexMap = (image->ginfo->width/BlockWidth) * (image->ginfo->height/BlockHeight);
    State.IndexMap  = (unsigned char *) galloc(SizeOfIndexMap);
    memset(State.IndexMap,0,SizeOfIndexMap);

    int SizeOfDetailMap = image->ginfo->width * image->ginfo->height;
    State.DetailMap = (unsigned char *) galloc(SizeOfDetailMap);
    memset(State.DetailMap,0,SizeOfDetailMap);

    // *************************************
    // STEP 1: SEED
    // *************************************

    GST_seed(&State,image);

    // Iterate map & split until no more splits

    double psnr;
    bool Split=false;
    int SplitLoopCount=0;
    do
    {

        bool MoreMap=true;
        int MapCount=0;
        int diff=0;

        do
        {
            // *************************************
            // STEP 2: MAP
            // find closest blocks
            // *************************************

            psnr = GST_map(&State,image);
            printf("psnr %2.2f count %d, blocks %d\n",psnr,MapCount,State.NumBlocks);

            // *************************************
            // STEP 3: Center
            // update center points based on new mapping
            // *************************************

            GST_center(&State,image);

            diff = GST_centercmp(&State);
            if (diff)
            {
                GST_centerupdate(&State);
            }

            ++MapCount;

            // on last iteration, try hard to settle on no difference
        } while (diff && (MapCount<MAXMAPS || (MapCount<MAXMAPSLAST && (State.NumBlocks<=SETTLINGFRAMES || State.NumBlocks>=MaxBlocks))));

        // *************************************
        // STEP 4: Split
        // find best block to split
        // create a new block with an initial seed
        // *************************************

        Split=false;
        if (State.NumBlocks<MaxBlocks && SplitLoopCount<MaxBlocks)
            Split = GST_split(&State,image);
        ++SplitLoopCount;

    } while (Split);

    psnr = GST_map(&State,image); // needed for detail map
    printf("final psnr %2.2f blocks %d\n",psnr,State.NumBlocks);

#if DUMPBLOCKS
    DumpBlocks(&State);
#endif


    // *************************************
    // STEP 5: Save
    // map to final palette, detail, base and save output
    // *************************************

    ShapeFile *output = GST_Save(&State,image,sizeofOutput);

    gfree(State.DetailMap);
    gfree(State.IndexMap);

    return output;
}





#endif

static int hashfunction(unsigned int p)
{
    return (((p>>24)+100)+(p>>16)+(p>>8)+p) % HASHSIZE;
}

static void xaddhist(XACT *xact,unsigned int p, int newcount)
{
    int i;

    // find colour and count

    int hash=hashfunction(p);
    int hmin=xact->hashmin[hash];
    int hmax=xact->hashmax[hash];

    for (i=hmax; i>=hmin; --i)
    {
        if (xact->hist[i].argb==p)
        {
            xact->hist[i].count += newcount;
            return;
        }
    }

    // adding new colour, realloc histogram if full

    i = xact->histcolours;
    if (i<hmin)
        xact->hashmin[hash] = i;
    if (i>hmax)
        xact->hashmax[hash] = i;

    if (i>=xact->histsize)
    {
        XHIST *hist;

        xact->histsize += 16384;
        hist = (XHIST *) galloc(xact->histsize*sizeof(XHIST));
        if (xact->hist)
        {
            memcpy(hist,xact->hist,i*sizeof(XHIST));
            gfree(xact->hist);
        }
        xact->hist = hist;
    }

    // add new colour

    xact->hist[i].argb = p;
    xact->hist[i].count = newcount;
    xact->histcolours = i+1;
}

static int xhistcmp(const void *h1, const void *h2)
{
    return((*(XHIST **)h2)->count - (*(XHIST **)h1)->count);
}

/****************************************************************/
/* CRapi functions                                              */
/****************************************************************/

CRABOUT *GCALL LLOYD_about(void)
{
    CRABOUT *about = (CRABOUT *)galloc(sizeof(CRABOUT));
    if (about)
    {
        memset(about, 0, sizeof(CRABOUT));

        about->signature =          GMAKEID('L','O','Y','D');
        about->size =               sizeof(CRABOUT);
        about->version =            CRAPI_VERSION;
        about->inversetable =       0;
        about->respectalpha =       1;
        strcpy(about->authorstr,    "Frank Barchard");
        strcpy(about->versionstr,   "1.00");
        strcpy(about->namestr,      "Lloyd");
        strcpy(about->longnamestr,  "Lloyd Histogram");
    }
    return(about);
}

CRINSTANCE *GCALL LLOYD_open(void)
{
    CRINSTANCE *crx=0;

    crx = (CRINSTANCE *)galloc(sizeof(CRINSTANCE));
    if (crx)
    {
        memset(crx, 0, sizeof(CRINSTANCE));
        XACT *xref = (XACT *) galloc(sizeof(XACT));
        if (xref)
        {
            memset(xref, 0, sizeof(XACT));
            crx->signature  = GMAKEID('L','O','Y','D');
            crx->size       = sizeof(sizeof(CRINSTANCE));
            crx->cref       = (void *)xref;
            for (int i=0; i<HASHSIZE; ++i)
            {
                xref->hashmax[i] = -1;
                xref->hashmin[i] = 0x7fffffff;
            }
        }
        else
        {
            gfree(crx);
            crx = 0;
        }
    }
    return(crx);
}


int GCALL LLOYD_read(CRINSTANCE *crx, const GINFO *ginfo, const void *source, int rowbytes)
{
    int i;
    int x;
    int y;
    int ok=0;
    unsigned char *s;
    XACT *xact = (XACT *)(crx->cref);

    if (xact)
    {
        for (i=0; i<256; ++i)
            xaddhist(xact,(*(unsigned int *)&ginfo->colourtbl[i]),0);

        for (y=0; y<ginfo->height; ++y)
        {
            s = (unsigned char *)source + y*rowbytes;
            if (ginfo->bpp==32)
            {
                for (x=0; x<ginfo->width; ++x)
                {
                    xaddhist(xact,(*(unsigned int *) s),1);
                    s += 4;
                }
            }
            else
            {
                for (x=0; x<ginfo->width; ++x)
                {
                    xaddhist(xact,(*(unsigned int *)&ginfo->colourtbl[*s]),1);
                    s += 1;
                }
            }
        }

        ok = 1;
    }
    return(ok);
}

ARGB *GCALL LLOYD_close(CRINSTANCE *crx, int numcolours, CRINVERSETABLE **inversetable, int *returncolours)
{
    int i;
    int j;
    int ok=0;
    int realcolours=0;
    unsigned int *colourtbl=0;
    XHIST **histindex;
    XACT *xact = (XACT *)(crx->cref);

    if (xact)
    {
        if (xact->hist)
        {
            histindex = (XHIST **) galloc(xact->histcolours*sizeof(XHIST *));
            if (histindex)
            {
                for (i=0; i<xact->histcolours; ++i)
                    histindex[i] = xact->hist+i;

                qsort(histindex,xact->histcolours,sizeof(XHIST *),xhistcmp);

                realcolours = gmin(numcolours,xact->histcolours);

                for (i=0; i<realcolours; ++i)
                    histindex[i]->count = -histindex[i]->count;

                colourtbl = (unsigned int *) galloc(numcolours*sizeof(ARGB));
                if (colourtbl)
                {
                    memset(colourtbl,0,numcolours*sizeof(ARGB));
                    for (i=0,j=0; i<xact->histcolours; ++i)
                        if (xact->hist[i].count<0)
                            colourtbl[j++] = xact->hist[i].argb;

                    ok = 1;
                }
                gfree(histindex);
            }
        }
        if (xact->hist) gfree(xact->hist);
        if (xact)       gfree(xact);
        if (crx)        gfree(crx);
    }
    if (returncolours)
        *returncolours = realcolours;
    return((ARGB *)colourtbl);
}


