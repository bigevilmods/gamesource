// (c) 2004-2007 Electronic Arts Inc.
// gstencode.cpp - functions to encode a gst clut from an image (8 or 32 bpp)

#define GSTDUMPBLOCKS 0
#define GSTDEBUG 0
#define GSTPARTIALSEEDREMAP 1

// F2 5 step method
// 1. seed - pq6
// while need more blocks
//   while lloyd changing 
//      2. map 
//      3. center
//   4. split
// 5. save

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "gst.h"
#include "gimex.h" // galloc
#include "quantize.h"
#include "helper.h"

namespace Gst
{

// *************************************
// Declarations
// *************************************
		
static const int GSTNUMCHANNELS=4;
static const int MAXBLOCKCOLOURS=4;

struct Block
{
	int CenterColours;// normally 2 or 4, but could vary per block.  Split sets to 1
	GstArgb Center[MAXBLOCKCOLOURS];
	int NewCenterColours;
	GstArgb NewCenter[MAXBLOCKCOLOURS];
	double SumDiff; // total deviation for this block for all screen blocks that map to it. double for longer range than int
	int UseCount; // how many times was this block uses
	int WorstDiff;
	int WorstX;
	int WorstY;
};

static const int MAXBLOCKS=128;

// squares table for L2 mapping.  Also used by gstmap.cpp
unsigned int GSTsquaretbl[512];


struct BlockState
{	
	// image reference				
	const unsigned char *image;
	int rowbytes;
	int width;
	int height;
	int bpp;		  // original image bpp (8 or 32)
	const GstArgb *clut;	  // original image clut

					  // typical values
	int BlockWidth;   // 4
	int BlockHeight;  // 2
	int DetailBpp;	  // 1 or 2
	int BaseBpp;	  // 4 or 8
	int BlockColours; // 2 or 4
	int NumBlocks;    // 64 or 128
	int Quality;	  // 100 slow, 75 normal, 50 fast, 0 super fast
	int HasAlpha;     // 0=no, 1=yes
	unsigned char *IndexMap;  // width*height/BLOCKPIXELS
	unsigned char *DetailMap; // width*height with values 0..3
	unsigned char *PrevIndexMap;  // previous iteration map, so we can center just blocks necessary
	Block Blocks[MAXBLOCKS];
};

// *************************************
// Miscellaneous support
// *************************************

typedef struct
{
    int signature;          /* crapi module signature */
    int size;               /* size of the CRINSTANCE structure */
    int frames;             /* number of frames processed so far */
    void *cref;            /* reference to additional memory for module */
} CRINSTANCE;

struct PQ
{
    int numcolours;
	int maxcolours;
    InputVector *vect;
};


/****************************************************************/
/* CRapi functions                                              */
/****************************************************************/

CRINSTANCE * GSTPQ_open(void)
{
    CRINSTANCE *crx=0;

    crx = (CRINSTANCE *)galloc(sizeof(CRINSTANCE));
    if (crx)
    {
        memset(crx, 0, sizeof(CRINSTANCE));
        PQ *xref = (PQ *) galloc(sizeof(PQ));
        if (xref)
        {
            memset(xref, 0, sizeof(PQ));
            crx->signature  = GMAKEID('P','A','T','Q');
            crx->size       = sizeof(sizeof(CRINSTANCE));
            crx->cref       = (void *)xref;
        }
        else
        {
            gfree(crx);
            crx = 0;
        }
    }
    return(crx);
}


int  GSTPQ_read(CRINSTANCE *crx, const GINFO *ginfo, const void *source, int rowbytes)
{
    int x;
    int y;
    int ok=0;
    PQ *pq = (PQ *)(crx->cref);

    if (pq)
    {
		// move image into InputVector

		int newcolours=ginfo->width*ginfo->height;
		InputVector *v;
		if (newcolours+pq->numcolours > pq->maxcolours)
		{
			const int ADDCOLOURS=8192;
			int maxcolours=gmax(newcolours,ADDCOLOURS);
			InputVector *newVect = (InputVector *)galloc((pq->maxcolours+maxcolours) * sizeof(InputVector));
			v = newVect;

			if (pq->vect)
			{
				memcpy(newVect,pq->vect,pq->numcolours*sizeof(InputVector));
				gfree(pq->vect);
				v = newVect+pq->numcolours;
				newcolours += pq->numcolours;
				maxcolours += pq->maxcolours;
			}
			pq->vect = newVect;
			pq->numcolours = newcolours;
			pq->maxcolours = maxcolours;
		}
		else
		{
			v = pq->vect+pq->numcolours;
			pq->numcolours += newcolours;
		}

//        for (i=0; i<256; ++i)
//            xaddvect(PQ,(*(unsigned int *)&ginfo->colourtbl[i]),0);

        for (y=0; y<ginfo->height; ++y)
        {
            unsigned char *s = (unsigned char *)source + y*rowbytes;
            if (ginfo->bpp==32)
            {
                for (x=0; x<ginfo->width; ++x)
                {
					const GstArgb *p = (const GstArgb *) s;
		            v->val[0] = p->r;
					v->val[1] = p->g;
					v->val[2] = p->b;
					v->val[3] = p->a;
					++v;
                    s += 4;
                }
            }
            else
            {
                for (x=0; x<ginfo->width; ++x)
                {
					const GstArgb *p = (const GstArgb *) &ginfo->colourtbl[*s];
		            v->val[0] = p->r;
					v->val[1] = p->g;
					v->val[2] = p->b;
					v->val[3] = p->a;
					++v;
                    s += 1;
                }
            }
        }

        ok = 1;
    }
    return(ok);
}

GstArgb * GSTPQ_close(CRINSTANCE *crx, int numcolours, void **inversetable, int *returncolours)
{
    int ok=0;
    int realcolours=0;
    GstArgb *colourtbl=NULL;
    PQ *pq = (PQ *)(crx->cref);

    if (pq)
    {
	    Cluster *clust = (Cluster*) galloc(numcolours*sizeof(Cluster));

	    int clusters = Cluster4DVectors(pq->vect, pq->numcolours, clust, numcolours);

        realcolours = gmin(numcolours,clusters);

        colourtbl = (GstArgb *) galloc(numcolours*sizeof(GstArgb));
        if (colourtbl)
		{
			memset(colourtbl,0,numcolours*sizeof(GstArgb));
			
			for (int i=0; i<realcolours; ++i)
			{
				colourtbl[i].r = clust[i].val[0];
				colourtbl[i].g = clust[i].val[1];
				colourtbl[i].b = clust[i].val[2];
				colourtbl[i].a = clust[i].val[3];
            }
            ok = 1;
        }

		gfree(clust);
        if (pq->vect) gfree(pq->vect);
        if (pq)       gfree(pq);
        if (crx)      gfree(crx);
    }
    if (returncolours)
        *returncolours = realcolours;
    return((GstArgb *)colourtbl);
}

CRINSTANCE *WU_open(void)
{
	CRINSTANCE *cx = GSTPQ_open();
	return cx;
}

void WU_add(BlockState *State,CRINSTANCE *cx,int x,int y, int BlockWidth, int BlockHeight)
{
	GINFO ginfoblock;

	ginfoblock.alphabits = 6;
	ginfoblock.redbits = 6;
	ginfoblock.greenbits = 6;
	ginfoblock.bluebits = 6;

	ginfoblock.bpp = 32;
	ginfoblock.originalbpp = 32;
	ginfoblock.size = sizeof(GINFO);
	ginfoblock.width  = BlockWidth;
	ginfoblock.height = BlockHeight;

	unsigned char *s = (unsigned char *)State->image + State->rowbytes*(y) + x*GSTNUMCHANNELS;

	GSTPQ_read(cx, &ginfoblock, s, State->rowbytes);
}

int WU_close(CRINSTANCE *cx,GstArgb *cpal,int numcolours)
{
	int newcolours=0;
	GstArgb *tpal = GSTPQ_close(cx, numcolours, NULL, &newcolours);
	memcpy(cpal,tpal,numcolours*sizeof(GstArgb));
	gfree(tpal);
	return newcolours;
}



// calculate new center for block

void CenterBlock(BlockState *State,  int i)
{
	//int Precision = 3;
	//if (State->Blocks[i].UseCount)
	//{
	//	int PixelErr = (int)(State->Blocks[i].SumDiff/State->Blocks[i].UseCount)/(State->BlockHeight*State->BlockWidth);

	//	if (State->Blocks[i].SumDiff<=6.0) // 4 is 1 error on 4 channels/pixels.  guaranteed that exact can map that
	//		Precision = 0;
	//	else if (PixelErr<=2*2)
	//		Precision = 1;
	//	else if (PixelErr<=8*8)
	//		Precision = 2;
	//	else
	//		Precision = 3;
	//}
	
//#if	GSTDUMPBLOCKS
//	printf("%d",Precision);
//#endif

	CRINSTANCE *cx = WU_open();

	unsigned char *imap = State->IndexMap;
	for (int y=0; y<State->height; y+= State->BlockHeight)
	{
		for (int x=0; x<State->width; x+= State->BlockWidth)
		{
			// is this block of pixels mapping to our block?
			if ((int)*imap == i)
			{
				WU_add(State,cx,x,y,State->BlockWidth,State->BlockHeight);
			}
			++imap;
		}
	}

	int newcolours = WU_close(cx,&State->Blocks[i].NewCenter[0],State->BlockColours);
	if (newcolours>1)
	{
		// Wu Da sometimes duplicates the last colour?!
		if (*(int *)&State->Blocks[i].NewCenter[newcolours-1] == *(int *)&State->Blocks[i].NewCenter[newcolours-2])
		{
			--newcolours;
#if GSTDEBUG
			printf("WARNING removing duplicate colour\n");
#endif
		}
	}
	State->Blocks[i].NewCenterColours = newcolours;
}

// copy NewCenter to Center
void GST_centerupdate(BlockState *State)
{
	for (int i=0; i<State->NumBlocks; ++i)
	{
		State->Blocks[i].CenterColours = State->Blocks[i].NewCenterColours;
		memcpy(State->Blocks[i].Center,State->Blocks[i].NewCenter,State->Blocks[i].NewCenterColours*sizeof(GstArgb));
	}
}

// *************************************
// STEP 1: SEED
// use colour reduction to seed some of the blocks.  75% of blocks seems to work best
// for 8 bpp input, use the original palette so that gst should be lossless, if it was a gst palette
// *************************************


void SetBlockVector(const GstArgb *pixel, InputVector *v, int BlockWidth, int BlockHeight, int RowBytes)
{
    float imin = 255.0f;
    float imax = 0.0f;
    float amin = 255.0f;
    float amax = 0.0f;
    float sumcx = 0.0f;
    float sumcy = 0.0f;
    float sumw = 1e-30f; // prevents division by zero

    const GstArgb *p = pixel;
    for (int y = 0; y < BlockHeight; ++y)
    {
        for (int x = 0; x < BlockWidth; ++x)
        {
            float r = p->r;
            float g = p->g;
            float b = p->b;
            float a = p->a;

            float i = 0.3333f*(r+g+b); // intensity
            float cx = r - 0.5f*(g+b); // colour x component
            float cy = 0.866f*(g-b); // colour y component
            float w = (2.0f/(170.0f*85.0f))*i*(255.0f-i); // colour weight (parabola)

            if (i < imin) imin = i;
            if (i > imax) imax = i;
            if (a < amin) amin = a;
            if (a > amax) amax = a;
            sumcx += cx;
            sumcy += cy;
            sumw += w;
            ++p;
        }
        p  = (const GstArgb *) ((const char *) p + RowBytes - (BlockWidth*sizeof(GstArgb)));
    }
	const float ChromaWeight=1.0f;
	const float LumaWeight=0.5f;
	const float AlphaWeight=0.5f;

    v->val[0] = static_cast<unsigned char>(IntRound(ChromaWeight*sumcx/sumw)+128);
    v->val[1] = static_cast<unsigned char>(IntRound(ChromaWeight*sumcy/sumw)+128);
    v->val[2] = static_cast<unsigned char>(IntRound(LumaWeight*imin));
    v->val[3] = static_cast<unsigned char>(IntRound(LumaWeight*imax));
    v->val[4] = static_cast<unsigned char>(IntRound(AlphaWeight*amin));
    v->val[5] = static_cast<unsigned char>(IntRound(AlphaWeight*amax));
}

/*--------------------------------------------------------------------------/
;
; Given a 6D vector value, finds the closest match in the specified Cluster
; array. The function returns the index of the nearest entry in the array.
;
/--------------------------------------------------------------------------*/

#if GSTPARTIALSEEDREMAP
int FindNearestCluster(unsigned char val[6], Cluster *clust, int NumClusters)
{
    int minpos = 0;
    int minerr = 999999999;

    for (int i = 0; i < NumClusters; ++i)
    {
        int d0 = val[0] - clust[i].val[0];
        int d1 = val[1] - clust[i].val[1];
        int d2 = val[2] - clust[i].val[2];
        int d3 = val[3] - clust[i].val[3];
        int d4 = val[4] - clust[i].val[4];
        int d5 = val[5] - clust[i].val[5];
        int err = d0*d0 + d1*d1 + d2*d2 + d3*d3 + d4*d4 + d5*d5;
        if (err < minerr)
        {
            minpos = i;
            minerr = err;
        }
    }

    return(minpos);
}
#endif

void GST_seed(BlockState *State, int MaxBlocks)
{
	// initialize L2 difference table

    for (int i=0; i<512; ++i)
		GSTsquaretbl[i] = (i-256)*(i-256);

	if (State->bpp<=8)
	{
		const GstArgb *SeedPalette = State->clut;

		int FullMaxBlocks = 1<<(State->BaseBpp-State->DetailBpp);
		State->NumBlocks = FullMaxBlocks;
		for (int i=0; i<FullMaxBlocks; ++i)
		{
			for (int j=0; j<State->BlockColours; ++j)
			{
				State->Blocks[i].Center[j] = SeedPalette[j];
				State->Blocks[i].CenterColours = State->BlockColours;
				State->Blocks[i].UseCount = 1;
			}
		}
	}		
	else
	{
		int x;
		int y;

		// move image into InputVector

		int NumBlocks=State->width*State->height/(State->BlockWidth*State->BlockHeight);
		
		InputVector *vect = (InputVector *)galloc(NumBlocks * sizeof(InputVector));

		InputVector *v = vect;

		int id=0;
		int MinAlpha=255;
		int MaxAlpha=0;
		for (y=0; y<State->height; y+=State->BlockHeight)
		{
			//if (State->bpp==32)
			{
				const GstArgb *s = (const GstArgb *)((unsigned char *)State->image + y*State->rowbytes);

				for (x=0; x<State->width; x+=State->BlockWidth)
				{
					v->id = id;
					v->cluster = 0;
					++id;
					SetBlockVector(s, v, State->BlockWidth, State->BlockHeight, State->rowbytes);
					MinAlpha = IntMin(v->val[4],MinAlpha);
					MaxAlpha = IntMax(v->val[5],MaxAlpha);
					s += State->BlockWidth;
					++v;
				}
			}
			//else
			//{
			//	for (x=0; x<State->width; ++x)
			//	{
			//		const GstArgb *p = (const GstArgb *) &State->colourtbl[*s];
			//		v->val[0] = p->r;
			//		v->val[1] = p->g;
			//		v->val[2] = p->b;
			//		v->val[3] = p->a;
			//		++v;
			//		s += 1;
			//	}
			//}
		}
		State->HasAlpha = true;
		if (MinAlpha==MaxAlpha)
			State->HasAlpha = false;

		// the more that is seeded, the faster GST encodes, but quality is slightly worse
		int NumColours = MaxBlocks;		// seed full cluster list for speed
		if (State->Quality>=75)	// mostly seeded
			NumColours = MaxBlocks-16;
		else if (State->Quality>=50)		// empty seed... slower
			NumColours = MaxBlocks-8;

		Cluster *clust = (Cluster*) galloc(NumColours*sizeof(Cluster));

		int clusters = Cluster6DVectors(vect, NumBlocks, clust, NumColours);
#if GSTDEBUG
	printf("clusters %d\n",clusters);
#endif
		int ActualClusters = gmin(NumColours,clusters);
#if GSTDEBUG
	printf("ActualClusters %d\n",ActualClusters);
#endif
		const float outlierThreshold=2.0f;

        float sumError = 0.0f;
        for (int i = 0; i < ActualClusters; ++i) sumError += clust[i].error;
        float threshold = outlierThreshold*outlierThreshold * sumError/NumBlocks;

        for (int i = 0; i < NumBlocks; ++i)
        {
            InputVector *v = &vect[i];
            Cluster *c = &clust[v->cluster];

            int d0 = v->val[0] - c->val[0];
            int d1 = v->val[1] - c->val[1];
            int d2 = v->val[2] - c->val[2];
            int d3 = v->val[3] - c->val[3];
            int d4 = v->val[4] - c->val[4];
            int d5 = v->val[5] - c->val[5];
			int err = d0*d0+d1*d1+d2*d2+d3*d3+d4*d4+d5*d5;
            if (float(err) < threshold)
            {
                int cluster = vect[i].cluster;
				State->IndexMap[vect[i].id] = (unsigned char) cluster;
				State->PrevIndexMap[vect[i].id] = (unsigned char) cluster;
            }
            else
            {
#if GSTPARTIALSEEDREMAP                
				int cluster = FindNearestCluster(v->val, clust, ActualClusters);
#else
				int cluster = 255; // not normally cluster id, which range from 0..127
#endif
				State->IndexMap[vect[i].id] = (unsigned char) cluster;
				State->PrevIndexMap[vect[i].id] = (unsigned char) cluster;
            }
        }

		gfree(clust);
		gfree(vect);

		State->NumBlocks = ActualClusters;
		for (int i=0; i<ActualClusters; ++i)
		{
			State->Blocks[i].CenterColours = 4;
			State->Blocks[i].NewCenterColours = 4;
			State->Blocks[i].UseCount = 1;

			CenterBlock(State, i);
		}
		GST_centerupdate(State);

#if GSTDEBUG
	printf("State->NumBlocks %d\n",State->NumBlocks);
#endif

	}
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

int ComparePixelToCenterAlpha(Block *Block, GstArgb *p, unsigned char *DetailMap)
{
	int MinDiff=512*512*4; // set realistically to avoid overflow in accumulation by caller
	for (int i=0; i<Block->CenterColours; ++i)
	{
		int Diff = GSTsquaretbl[(int)Block->Center[i].r - (int)p->r + 256]
			     + GSTsquaretbl[(int)Block->Center[i].g - (int)p->g + 256]
				 + GSTsquaretbl[(int)Block->Center[i].b - (int)p->b + 256]
				 + GSTsquaretbl[(int)Block->Center[i].a - (int)p->a + 256];
		if (Diff<=MinDiff)
		{
			MinDiff = Diff;
			*DetailMap = (unsigned char) i;
		}
	}
	return MinDiff;
}



int ComparePixelToCenter(Block *Block, GstArgb *p, unsigned char *DetailMap)
{
	int MinDiff=512*512*3; // set realistically to avoid overflow in accumulation by caller
	for (int i=0; i<Block->CenterColours; ++i)
	{
		int Diff = GSTsquaretbl[(int)Block->Center[i].r - (int)p->r + 256]
			     + GSTsquaretbl[(int)Block->Center[i].g - (int)p->g + 256]
				 + GSTsquaretbl[(int)Block->Center[i].b - (int)p->b + 256];
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

int CompareBlock(BlockState *State, Block *Block, unsigned char *DetailMap, int BlockX, int BlockY, int BlockWidth, int BlockHeight)
{
	int DiffSum=0;
	
	unsigned char *DMap = DetailMap + (State->width*(BlockY)) + BlockX;
	GstArgb *s = (GstArgb *)((unsigned char *)State->image + State->rowbytes*(BlockY) + BlockX*GSTNUMCHANNELS);

	if (State->HasAlpha)
	{
		for (int y=0; y<BlockHeight; ++y)
		{
			for (int x=0; x<BlockWidth; ++x)
			{
				int Diff = ComparePixelToCenterAlpha(Block,s+x,DMap+x);
				DiffSum += Diff;
			}
			s = (GstArgb *)((unsigned char *)s + State->rowbytes);
			DMap += State->width;
		}
	}
	else
	{
		for (int y=0; y<BlockHeight; ++y)
		{
			for (int x=0; x<BlockWidth; ++x)
			{
				int Diff = ComparePixelToCenter(Block,s+x,DMap+x);
				DiffSum += Diff;
			}
			s = (GstArgb *)((unsigned char *)s + State->rowbytes);
			DMap += State->width;
		}
	}
	return DiffSum;
}

// for the given screen block of pixels, find the best block to match to

void MapBlock(BlockState *State, int BlockX, int BlockY)
{
	int BestDiff=0x7fffffff;
	int BestIndex=0;
	for (int i=0; i<State->NumBlocks; ++i)
	{
		int Diff = CompareBlock(State,State->Blocks+i,State->DetailMap,BlockX,BlockY,State->BlockWidth,State->BlockHeight);
		if (Diff < BestDiff)
		{
			BestDiff = Diff;
			BestIndex = i;
		}
	}
	// once more for detail map
	CompareBlock(State,State->Blocks+BestIndex,State->DetailMap,BlockX,BlockY,State->BlockWidth,State->BlockHeight);

	// remember which block the pixels mapped to.  This is useful to the final mapping stage
	unsigned char *imap = State->IndexMap + (State->width/State->BlockWidth)*(BlockY/State->BlockHeight) + BlockX/State->BlockWidth;
	*imap = (unsigned char) BestIndex;

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
// return psnr as float for GSTDEBUG
double GST_map(BlockState *State)
{
	for (int i=0; i<State->NumBlocks; ++i)
	{
		State->Blocks[i].SumDiff = 0;
		State->Blocks[i].WorstDiff = 0;
		State->Blocks[i].UseCount = 0;
	}

	// find best match and update IndexMap
	for (int y=0; y<State->height; y+= State->BlockHeight)
		for (int x=0; x<State->width; x+= State->BlockWidth)
			MapBlock(State,x,y);

	// just for diagnostics, calculate a PSNR
	// this can be removed later

	// normalize sum
	double sse=0.0;
	for (int i=0; i<State->NumBlocks; ++i)
	{
		sse += State->Blocks[i].SumDiff;
#if GSTDEBUG
		printf("State->Blocks[%d].SumDiff %.2f  used %d\n",i,State->Blocks[i].SumDiff,State->Blocks[i].UseCount);
#endif
	}

	return psnr(sse,State->width*State->height*3); // ignore alpha
}


// *************************************
// STEP 3: Center
// for all screen blocks that map to a block array entry, recalculate center
// uses WU
// *************************************

bool IndexSame(BlockState *State,int SizeOfIndexMap, int i)
{
	unsigned char *imap = State->IndexMap;
	unsigned char *pmap = State->PrevIndexMap;
	
	for (int x=0; x<SizeOfIndexMap; ++x)
	{
		if ((*imap==(unsigned char)i || *pmap==(unsigned char)i) && (*imap != *pmap))
		{
			return false;
		}
		++imap;
		++pmap;
	}
	return true;
}

// calculate new center for all block

void GST_center(BlockState *State,int SizeOfIndexMap)
{
	for (int i=0; i<State->NumBlocks; ++i)
	{
		if (!IndexSame(State, SizeOfIndexMap ,i))	// only recenter blocks that changed
			CenterBlock(State, i);
//		else
//			printf("same %d\n",i);
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
			diff = memcmp(State->Blocks[i].Center,State->Blocks[i].NewCenter,State->Blocks[i].CenterColours*sizeof(GstArgb));
	}
	return diff;
}

// *************************************
// STEP 4: Split
// find best block to split
// create a new block with an initial seed
// *************************************

#if	GSTDUMPBLOCKS

void GSTDumpBlocks(BlockState *State)
{
	for (int i=0; i<State->NumBlocks; ++i)
	{
		printf("%d: %d %d %.0f %d %d %d %d ", i,
			State->Blocks[i].CenterColours,// normally 2 or 4, but could vary per block.  Split sets to 1
			State->Blocks[i].NewCenterColours,
			State->Blocks[i].SumDiff, // total deviation for this block for all screen blocks that map to it. double for longer range than int
			State->Blocks[i].UseCount, // how many times was this block uses
			State->Blocks[i].WorstDiff,
			State->Blocks[i].WorstX,
			State->Blocks[i].WorstY);
		for (int j=0; j<State->Blocks[i].CenterColours; ++j)
			printf("%08lx ", *(int*)&State->Blocks[i].Center[j]);
		printf("\n");
	}
};

#endif

bool GST_merge(BlockState *State, int IndexCount)
{
	bool FoundMerge=false;
	unsigned char xlat[256];

	// remove unused blocks
	int j=0;
	for (int i=0; i<State->NumBlocks; ++i)
	{
		memcpy(&State->Blocks[j],&State->Blocks[i],sizeof(State->Blocks[j]));
		xlat[i] = (unsigned char)j;
		if (State->Blocks[j].UseCount>0)
		{
			++j;
		}
		else
		{
#if GSTDEBUG
			printf("warning - unused block %d removed.\n",i);
#endif
			FoundMerge = true;
		}
	}
	State->NumBlocks = j;

	if (FoundMerge)
	{
		// clean up unused blocks for safety
		while (j<State->NumBlocks)
		{
			memset(&State->Blocks[j],0,sizeof(State->Blocks[j]));
			++j;
		}

		unsigned char *imap = State->IndexMap;

		for (int i=0; i<IndexCount; ++i)
			imap[i] = xlat[imap[i]];
	}

	return FoundMerge;
}

// ideas
// split along axis - ie intensity (a+r+g+b)
// split all blocks


bool GST_split(BlockState *State,int MaxBlocks)
{
	bool FoundSplits=false;
	bool MoreSplits=true;
	int numsplits = 16;

#if	GSTDUMPBLOCKS
	// to help show where centering stops and split centering begins
	printf(" ");
#endif
	while (MoreSplits)
	{
		MoreSplits = false;
		bool FoundSplit = false;
		double MostDiff=0.0;
		int MostIndex=0;

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
			unsigned char *imap = State->IndexMap + (State->width/State->BlockWidth)*(y/State->BlockHeight) + x/State->BlockWidth;
			*imap = (unsigned char) NumBlocks;

			State->Blocks[NumBlocks].UseCount = 1;	// set to 1 to prevent further immediate split and avoid merge
			State->Blocks[NumBlocks].SumDiff = State->Blocks[NumBlocks].WorstDiff;
			CenterBlock(State, NumBlocks);	// update center for both parts of the split

#if 0
			if (State->Blocks[NumBlocks].NewCenterColours>=4)
			{
				State->Blocks[NumBlocks].SumDiff = 8.0; // redo with something better
				CenterBlock(State, NumBlocks);	// update center for both parts of the split
			}
#endif
			CenterBlock(State, MostIndex);
			State->Blocks[NumBlocks].UseCount = 1;	// set to 1 to prevent further immediate split and avoid merge
			State->Blocks[MostIndex].UseCount = 1;
			
			if (memcmp(State->Blocks[MostIndex].NewCenter,State->Blocks[NumBlocks].NewCenter,sizeof(State->Blocks[NumBlocks].NewCenter)))
			{
				State->NumBlocks++;
				GST_centerupdate(State);
				FoundSplits = true;
				if (State->NumBlocks < MaxBlocks)
					MoreSplits = true;
			}
#if GSTDEBUG
			else
			{
				printf("warning: split degenerated\n");
			}
#endif
		}
		if (--numsplits<=0)
			MoreSplits = false;
	}
	return FoundSplits;
}


//	// note will want to remove this in the future to test 8 bit direct reencoding
//
//	if (info->bpp <= 8)
//	{
////		printf("Bumping image from 8 to 32 bit\n");
//		void *image32 = galloc(info->mWidth*info->mHeight*sizeof(GstArgb));
//		GstArgb *d = (GstArgb *) image32;
//		
//		for (int y=0; y<image->ginfo->height; ++y)
//		{
//			unsigned char *s = (unsigned char *)image->image + y*image->rowbytes;
//			for (int x=0; x<image->ginfo->width; ++x)
//			{
//				*d++ = image->ginfo->colourtbl[*s++];
//			}
//		}
//		gfree(image->image);
//		image->image = image32;
//		image->rowbytes = image->ginfo->width * sizeof(GstArgb);
//		image->ginfo->bpp = 32;
//		image->ginfo->originalbpp = 32;
//	}

// *************************************
// Compress
// main compression entry point that creates a compressed shape
// *************************************

static bool GSTEncodePaletteInternal(GstArgb *outclut, int *numclutentriesout, // output clut parameters
    const void *image, int rowbytes, int width, int height, int bpp, const GstArgb *inclut, // input image
	int BlockWidth=4,int BlockHeight=2,int DetailBpp=2,int Quality=75)  // settings
{
	int MaxBlocks = 32;
	int MAXMAPS=1;
	int MAXMAPSLAST=2;
	int MaxMerges=4;
	const int BaseBpp = 8;

	MaxBlocks = 1<<(BaseBpp-DetailBpp); // 64 typical
	if (Quality>=100)	// slow
	{
		MAXMAPS=1;
		MAXMAPSLAST=12;
	}
	else if (Quality>=75)	// normal
	{
		MAXMAPS=1;
		MAXMAPSLAST=2;
	}
	else if (Quality>=50)	// fast
	{
		MAXMAPS=1;
		MAXMAPSLAST=1;
	}
	else if (Quality>=25)	// faster
	{
		MAXMAPS=1;
		MAXMAPSLAST=1;
	}
	else				 	// super fast
	{
		MAXMAPS=0;
		MAXMAPSLAST=0;
	}
	BlockState State={0,};
	State.BlockWidth  = BlockWidth;
	State.BlockHeight = BlockHeight;
	State.DetailBpp   = DetailBpp;
	State.BaseBpp     = BaseBpp;
	State.BlockColours = 1<<DetailBpp;	// 2 or 4
	State.Quality    = Quality;		// 0=low, 50 lower, 75 normal, 100 best
	State.NumBlocks = 1;
	
	State.image = (unsigned char *)image;
	State.rowbytes = rowbytes;
	State.width = width;
	State.height = height;
	State.bpp = bpp;
	State.clut = inclut;

	int SizeOfIndexMap = (State.width/BlockWidth) * (State.height/BlockHeight);
	State.IndexMap  = (unsigned char *) galloc(SizeOfIndexMap);
	memset(State.IndexMap,0,SizeOfIndexMap);
	State.PrevIndexMap  = (unsigned char *) galloc(SizeOfIndexMap);
	memset(State.PrevIndexMap,0,SizeOfIndexMap);

	int SizeOfDetailMap = State.width * State.height;
	State.DetailMap = (unsigned char *) galloc(SizeOfDetailMap);
	memset(State.DetailMap,0,SizeOfDetailMap);

	// *************************************
	// STEP 1: SEED
	// *************************************
		
	GST_seed(&State,MaxBlocks);
	//GST_center(&State,SizeOfIndexMap);

	// Iterate map & split until no more splits

	double psnr;
	bool Split=false;
	int SplitLoopCount=0;
	int MergeCount=0;
	do
	{

		int MapCount=0;

		while (MapCount<MAXMAPS || (MapCount<MAXMAPSLAST && State.NumBlocks>=MaxBlocks))
		{
			// *************************************
			// STEP 2: MAP
			// find closest blocks
			// *************************************

			psnr = GST_map(&State);
#if GSTDEBUG
			printf(" psnr %2.2f count %d, blocks %d\n",psnr,MapCount,State.NumBlocks);
#endif
			Split = GST_merge(&State,SizeOfIndexMap);
			if (Split)
				++MergeCount;

			// *************************************
			// STEP 3: Center
			// update center points based on new mapping
			// *************************************

			int diff = 0;
			if (memcmp(State.PrevIndexMap,State.IndexMap,SizeOfIndexMap))
			{
				GST_center(&State,SizeOfIndexMap);

				diff = GST_centercmp(&State);
				if (diff)
				{
					GST_centerupdate(&State);
				}
				memcpy(State.PrevIndexMap,State.IndexMap,SizeOfIndexMap);
			}
			if (!diff) break;

			++MapCount;
		}
			// on last iteration, try hard to settle on no difference

		// *************************************
		// STEP 4: Split
		// find best block to split
		// create a new block with an initial seed
		// *************************************
	
		Split=false;

		if (SplitLoopCount<MaxBlocks && MergeCount<MaxMerges)
		{
			if (State.NumBlocks<MaxBlocks)
				Split = GST_split(&State,MaxBlocks);

#if	GSTDUMPBLOCKS>=2
			GSTDumpBlocks(&State);
#endif
		}
		++SplitLoopCount;

	} while (Split);

#if	GSTDUMPBLOCKS
	GSTDumpBlocks(&State);
#endif

	gfree(State.DetailMap);
	gfree(State.IndexMap);
	gfree(State.PrevIndexMap);

	// copy clut to return it to the caller

	*numclutentriesout = State.NumBlocks * State.BlockColours;
	GstArgb *clut = outclut;
	for (int i=0; i<State.NumBlocks; ++i)
	{
		for (int j=0; j<State.BlockColours; ++j)
		{
			clut->a = State.Blocks[i].Center[j].a;
			clut->r = State.Blocks[i].Center[j].r;
			clut->g = State.Blocks[i].Center[j].g;
			clut->b = State.Blocks[i].Center[j].b;
			++clut;
		}
	}
#if GSTDEBUG
	printf("end State.NumBlocks %d\n",State.NumBlocks);
	printf("end State.BlockColours %d\n",State.BlockColours);
	printf("end *numclutentriesout %d\n",*numclutentriesout);
#endif
	return true;
}

////////////////////////////////////////////////////////
// PUBLIC API
////////////////////////////////////////////////////////


// Calculate gst palette.  This is the main work in gst encoding
bool GSTEncodePalette(GstImageInfo *info, GstType GstType, GstArgb *outclut, int *numclutentriesout, int Quality)
{
	GstImageInfo gstinfo = *info;

	int BlockWidth=4, BlockHeight=2, DetailBpp=2;
	GetTypeInfo(GstType,&BlockWidth,&BlockHeight,&DetailBpp);


	// note will want to remove this in the future to test 8 bit direct reencoding

	void *image32 = NULL;
	
	if (gstinfo.mBpp <= 8)
	{
#if	GSTDUMPBLOCKS
		printf("Bumping image from 8 to 32 bit\n");
#endif
		image32 = galloc(gstinfo.mWidth*gstinfo.mHeight*sizeof(GstArgb));
		GstArgb *d = (GstArgb *) image32;
		
		for (int y=0; y<gstinfo.mHeight; ++y)
		{
			unsigned char *s = (unsigned char *)gstinfo.mImage + y*gstinfo.mRowBytes;
			for (int x=0; x<gstinfo.mWidth; ++x)
			{
				*d++ = gstinfo.mClut[*s++];
			}
		}
		gstinfo.mImage = image32;
		gstinfo.mRowBytes = gstinfo.mWidth * sizeof(GstArgb);
//		gstinfo.mBpp = 32;
	}

	bool ok = GSTEncodePaletteInternal(outclut, numclutentriesout,
		gstinfo.mImage, gstinfo.mRowBytes, gstinfo.mWidth, gstinfo.mHeight, gstinfo.mBpp, gstinfo.mClut,
		BlockWidth,BlockHeight,DetailBpp,
		Quality);

	if (image32)
		gfree(image32);
	return ok;
}

} // end of Gst namespace
