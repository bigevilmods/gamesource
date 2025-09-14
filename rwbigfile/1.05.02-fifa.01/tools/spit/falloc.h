// (c) Electronic Arts. All Rights Reserved.

/*------------------------------------------------------------------*/
/*																	*/
/*				 FALLOC - Fast malloc replacement v0.90 			*/
/*																	*/
/*					   by FrANK G. Barchard, EAC					*/
/*																	*/
/*						 Header - Sept 09, 2003 					*/
/*																	*/
/*------------------------------------------------------------------*/

#ifndef FALLOC_H
#define FALLOC_H

#if defined(_MSC_VER)
#pragma once
#endif

#define FALLOC_VERSION 90

#define USE_ALIGNED 0

struct FAllocHunk
{
	struct FAllocHunk *mPrevHunk;
	int mSize;
};

class FAlloc
{
public:
	FAlloc(void)
	{
		mHunkSize = 1*1024*1024;
		mNextBlock = mEndHunk = 0;
		mCurrentHunk = 0;
		mCount = 0;
	}

	~FAlloc(void)
	{
		while (mCurrentHunk)
		{
			FAllocHunk *PrevHunk = mCurrentHunk->mPrevHunk;

			free(mCurrentHunk); 	// NOTE replace free() here

			mCurrentHunk = PrevHunk;
		}
		mNextBlock = mEndHunk = 0;
	}

	void *Alloc(size_t BlockSize)
	{
#if USE_ALIGNED
		BlockSize = (BlockSize+3)&~3;
#endif
		// need more memory
		if (reinterpret_cast<char *>(mNextBlock)+BlockSize > mEndHunk)
		{
			GrowHeap(BlockSize);
		}
		void *mb = mNextBlock;
		mNextBlock = reinterpret_cast<void *>(reinterpret_cast<char *>(mNextBlock)+BlockSize);
		++mCount;
		return mb;
	}

	void Free(void * /*Block*/) // provided for clean calling code
	{
		--mCount;
		return;
	}

	int GetCount(void) // provided for clean calling code
	{
		return mCount;
	}

#ifdef DEBUG
	void Dump(void)
	{
		while (mCurrentHunk)
		{
			FAllocHunk *PrevHunk = mCurrentHunk->mPrevHunk;

			printf("%08x %d\n", mCurrentHunk, mCurrentHunk->mSize); 	// NOTE replace printf() here

			mCurrentHunk = PrevHunk;
		}
	}
#endif

private:

	void GrowHeap(size_t BlockSize)
	{
		size_t HunkSize = mHunkSize;
		if ((BlockSize+sizeof(FAllocHunk)) > HunkSize)
			HunkSize = BlockSize+sizeof(FAllocHunk);

		FAllocHunk *NextHunk = reinterpret_cast<FAllocHunk *> (malloc(HunkSize)); // NOTE replace malloc() here
		NextHunk->mSize = HunkSize;
		NextHunk->mPrevHunk = mCurrentHunk;
		mCurrentHunk = NextHunk;
		mNextBlock = reinterpret_cast<void *>(reinterpret_cast<char *>(NextHunk) + sizeof(FAllocHunk)); // first allocable block
		mEndHunk = reinterpret_cast<void *>(reinterpret_cast<char *>(NextHunk) + HunkSize);  // end address of hunk
	}


	void *mNextBlock;
	void *mEndHunk;
	FAllocHunk *mCurrentHunk;
	int mHunkSize;
	int mCount;
};

#endif // FALLOC_H
