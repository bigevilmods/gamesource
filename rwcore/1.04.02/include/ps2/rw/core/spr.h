// (c) Electronic Arts. All Rights Reserved.
/* RWPUBLIC */
/*************************************************************************************************************

 File: rwspr.hpp

 Purpose: PS2 Scratchpad handling

 */

#ifndef RW_SPR_HPP
#define RW_SPR_HPP

// ***********************************************************************************************************
// Includes

/* RWOLDPUBLIC */

// ***********************************************************************************************************
// Defines

namespace rw
{

// ***********************************************************************************************************
// Forward Declarations


// ***********************************************************************************************************
// Typedefs


// ***********************************************************************************************************
// Enums + Consts


// ***********************************************************************************************************
// Global Variables


// ***********************************************************************************************************
// External Functions Prototypes


// ***********************************************************************************************************
// Structs + Unions + Classes

// ***********************************************************************************************************
//                                                SprAllocation Definition
// ***********************************************************************************************************

/*
Allocator for scratchpad memory. MASKTYPE must be an unsigned integer type (8, 16, 32 or 64bit).
*/
template<typename MASKTYPE>
class SprAllocation
{
public:

    void *GetHighAddress() const;
    void *GetLowAddress() const;
    void  Initialize(uint32_t size);
    void  Release();

private:

    static const uint32_t BLOCKSIZE      = 2048 / sizeof(MASKTYPE);
    static const uint32_t NUMBLOCKS      = 8 * sizeof(MASKTYPE);
    static const uint32_t SPRBASEADDRESS = 0x70000000;

    void     *mHighAddress;
    void     *mLowAddress;
    MASKTYPE  mMask;

    static MASKTYPE mAllocated;
};


/*
*/
template<typename MASKTYPE>
inline void *SprAllocation<MASKTYPE>::GetHighAddress() const
{
    rwFUNCTION();
    rwRETURN(mHighAddress);
}


/*
*/
template<typename MASKTYPE>
inline void *SprAllocation<MASKTYPE>::GetLowAddress() const
{
    rwFUNCTION();
    rwRETURN(mLowAddress);
}


/*
*/
template<typename MASKTYPE>
inline void SprAllocation<MASKTYPE>::Initialize(uint32_t size)
{
    rwFUNCTION();
    // Calculate number of blocks to allocate
    uint32_t numBlocks = (size + BLOCKSIZE - 1u) / BLOCKSIZE;
    rwENFORCE(numBlocks <= NUMBLOCKS, ("Attempted allocation too large (%dbytes)", size));

    if (NUMBLOCKS == numBlocks)
    {
        rwENFORCE(0 == mAllocated, ("Failed allocation (%dbytes)", size));
        mMask = ~static_cast<MASKTYPE>(0);
        mLowAddress = reinterpret_cast<void *>(SPRBASEADDRESS);
        mHighAddress = reinterpret_cast<void *>(SPRBASEADDRESS + BLOCKSIZE * NUMBLOCKS);
        mAllocated = mMask;
    }
    else
    {
        // Calculate block mask
        MASKTYPE blockMask = ~static_cast<MASKTYPE>(0) >> (NUMBLOCKS - numBlocks);
        uint32_t firstBlock = 0;
        mMask = blockMask;

        // Search for free space
        while (0 != (mMask & mAllocated))
        {
            rwENFORCE(firstBlock + numBlocks <= NUMBLOCKS, ("Failed allocation (%dbytes)", size));
            mMask <<= 1;
            ++firstBlock;
        }

        // Allocate blocks
        mLowAddress = reinterpret_cast<void *>(SPRBASEADDRESS + BLOCKSIZE * firstBlock);
        mHighAddress = reinterpret_cast<void *>(SPRBASEADDRESS + BLOCKSIZE * (firstBlock + numBlocks));
        mAllocated |= mMask;
    }
    rwRETURNVOID();
}


/*
*/
template<typename MASKTYPE>
inline void SprAllocation<MASKTYPE>::Release()
{
    rwFUNCTION();
    // Free allocated blocks
    rwENFORCE(mMask == (mMask & mAllocated), ("Attempted to free unallocated blocks."));
    mAllocated = mAllocated & ~mMask;
    rwRETURNVOID();
}


typedef SprAllocation<uint64_t> DefaultSprAllocation;

} //namespace rw

/* RWOLDPUBLICEND */

#endif // RW_SPR_HPP
/* RWPUBLICEND */
