// (c) 2007 Electronic Arts Inc.

#ifndef __JLZWRITE
#define __JLZWRITE 1

/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                           JLZCompress                                   //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#include "codex.h"
#include "jlzcodex.h"

//
// Utility Functions
//
static int32 Compare(uint8 *p1, uint8 *p2, int32 MaxLen)
{
	int32 Result = 0;

#if defined( PLAT_PC )
	while( (Result+3) < MaxLen && *(int32 *)(p1+Result) == *(int32 *)(p2+Result) )
		Result += 4;
#endif

	while( Result < MaxLen && p1[Result] == p2[Result] )
		Result++;

	return Result;
}

#define PLAT_PIPELINE_TOOL

//
// Hash Table
//
class JLZHash
{
public:
	int32	Offset;
	uint8	*pData;
	JLZHash *pPrev, *pNext;
};

class JLZHashPool
{
private:
	JLZHash *pPool;		// Pool of hashes
	JLZHash **pHead;	// Array of hash lists
	int32 PoolSize;		// Number of hashes in the pool / size of the compression window

public:
	JLZHashPool(int32 WindowSize);
	~JLZHashPool();

	void Update(uint8 *pData, int32 Offset);	// Both removes an existing and adds a new hash
	JLZHash *FindList(uint8 *pData);
};

JLZHashPool::JLZHashPool(int32 WindowSize)
{
	PoolSize = WindowSize;
#ifdef PLAT_PIPELINE_TOOL
	pPool = new JLZHash[PoolSize];
	pHead = new JLZHash*[JLZ_HTSize];
#else
	pPool = new("JLZHash") JLZHash[PoolSize];
	pHead = new("JLZHash") JLZHash*[JLZ_HTSize];
#endif

	bMemSet(pPool, 0, sizeof(JLZHash) * PoolSize);
	bMemSet(pHead, 0, sizeof(JLZHash *) * JLZ_HTSize);
}

JLZHashPool::~JLZHashPool()
{
	delete [] pPool;
	delete [] pHead;
}

static int32 GetTableHash(uint8 *pData)
{
	uint32 Value = ((int32)pData[2] << 8) ^ ((int32)pData[1] << 4) ^ pData[0];

	return (Value * 40543) % JLZ_HTSize;
}

void JLZHashPool::Update(uint8 *pData, int32 Offset)
{
	int32 WindowPos = Offset % PoolSize;
	JLZHash *pHash = pPool + WindowPos;
	int32 TableNum;

	// Remove the previous previous entry at this window position from the lists
	if(pHash->pData)
	{
		if(pHash->pNext) pHash->pNext->pPrev = pHash->pPrev;
		if(pHash->pPrev) pHash->pPrev->pNext = pHash->pNext;

		TableNum = GetTableHash(pHash->pData);
		if(pHead[TableNum] == pHash) pHead[TableNum] = pHash->pNext;
	}

	// Add the new entry to the head of the correct list
	pHash->pData = pData;
	pHash->Offset = Offset;
	TableNum = GetTableHash(pData);
	pHash->pPrev = 0;
	pHash->pNext = pHead[TableNum];
	pHead[TableNum] = pHash;
	if(pHash->pNext) pHash->pNext->pPrev = pHash;
}

JLZHash *JLZHashPool::FindList(uint8 *pData)
{
	return pHead[ GetTableHash(pData) ];
}





int32 JLZCompress(uint8 *pSrc, int32 Size, uint8 *pDest)
{
	JLZHashPool	Pool(WINDOWSIZE);
	int32 Pos = 0;
	uint8 *pOut = pDest;
	JLZHash *pHash, *pLongRun;
	int32 LongRun, MaxLen, Run, RunOffset;
	int32 OrigSize = Size;
	int32 Iteration = 0;
	uint16 Control, RunType;
	uint8 *pControl, *pRunType;

	// Header
	LZHeader *header = (LZHeader *)pOut;

	JLZPUTFIELD(header->ID,                 JLZCOMP_ID);
	JLZPUTFIELD(header->Version, 	        JLZCOMP_VERSION);
	JLZPUTFIELD(header->HeaderSize,         sizeof(LZHeader));
	JLZPUTFIELD(header->Flags, 			    0);
	JLZPUTFIELD(header->UncompressedSize,   Size);
	JLZPUTFIELD(header->CompressedSize,     0);

	pOut += sizeof(LZHeader);

	// Initialize everything
	int previous_percent = 0;
	Control = 0xff00;	// When the upper byte is clear,
	RunType = 0xff00;	// The value needs to be written / updated
	pControl = pOut;
	pRunType = pOut + 1;
	pOut += 2;

	while( Size >= 0 )
	{
		if(Size >= NEAR_RUNMAX)		MaxLen = NEAR_RUNMAX;
		else						MaxLen = Size;

		// Search for the best int32 run of the current char
		pHash = Pool.FindList(pSrc);
		LongRun = 2;	// Run must be > LongRun to become a match
		pLongRun = 0;

		while(pHash && LongRun < NEAR_RUNMAX)
		{
			Run = Compare(pHash->pData, pSrc, MaxLen);
			if(Run > LongRun)
			{
				RunOffset = Pos - pHash->Offset;
				if((Run > FAR_RUNMAX && (RunOffset < NEAR_OFFMAX || LongRun <= FAR_RUNMAX)) || (Run <= FAR_RUNMAX))
				{
					LongRun = Run;
					pLongRun = pHash;
				}
			}
			pHash = pHash->pNext;
		}

		// If we found a match, output it
		if(LongRun > 2)
		{
			Control >>= 1;								// Leave the high bit set
			RunOffset = (Pos - pLongRun->Offset) - 1;

			// Do we need a far window offset for it?
			if(RunOffset >= NEAR_OFFMAX)
			{
				if(LongRun > FAR_RUNMAX) LongRun = FAR_RUNMAX;			// Make sure the run fits the length

				// Long offset / Short run (1-4096 offset / 3 - 18 len)
				RunType = (RunType >> 1) & 0xff7f;		// Clear the high bit
				pOut[0] = (((RunOffset - NEAR_OFFMAX) >> 8) << FAR_RUNBITS) | (LongRun - 3);
				pOut[1] = (RunOffset - NEAR_OFFMAX) & 0xff;
				pOut += 2;
			}
			else	// Small window offset  (Near run)
			{
				// Short offset / Long run (1-256 offset / 3 - 258 len)
				RunType >>= 1;							// Leave the high bit set
				pOut[0] = (((LongRun-3) >> 8) << NEAR_OFFBITS) | (uint8)RunOffset;
				pOut[1] = (LongRun-3) & 0xff;
				pOut += 2;
			}
			Size -= LongRun;
			do {
				Pool.Update( pSrc, Pos );
				pSrc++;
				Pos++;
			} while(--LongRun);
		}
		else
		{
			// Output a literal
			Pool.Update( pSrc, Pos );
			*pOut++ = *pSrc++;
			Control = (Control >> 1) & 0xff7f;			// Clear the high bit
			Pos++;
			Size--;
		}

		// If the control byte is full, output it
		if(Control < 0x100)
		{
			*pControl = Control & 0xff;
			Control = 0xff00;
			pControl = pOut++;
		}

		// If the runtype byte is full, output it
		if(RunType < 0x100)
		{
			*pRunType = RunType & 0xff;
			RunType = 0xff00;
			pRunType = pOut++;
		}

		if( (++Iteration & 0x1fff) == 0)
		{
			int current_percent = 10 - (Size * 10 / OrigSize);

			if( current_percent != previous_percent )
			{
				//bPrintf("%c", '0' + (current_percent - 1));

				previous_percent = current_percent;
			}
			//bPrintf("\r%4.1f%%", 100.f - (float)Size / (float)OrigSize * 100.f);
		}
	}

	//bPrintf("\n");

	while(Control & 0xff00)
		Control >>= 1;
	*pControl = (Control & 0xff);

	while(RunType & 0xff00)
		RunType >>= 1;
	*pRunType = (RunType & 0xff);


	// Patch Header

	JLZPUTFIELD(header->CompressedSize,   pOut - pDest);

	return (int)(pOut - pDest);
}

/****************************************************************/
/*  Encode Function                                             */
/****************************************************************/

// codex API

int GCALL JLZ_encode(void *compresseddata, const void *source, int sourcesize, int *)
{
    int plen = JLZCompress((unsigned char *)source, sourcesize, (unsigned char *)compresseddata);

    return(plen);
}

#endif

