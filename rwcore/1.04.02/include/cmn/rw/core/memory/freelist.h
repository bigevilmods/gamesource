// (c) Electronic Arts. All Rights Reserved.

/// \file
/// Header file for free list support
#ifndef RW_FREELIST_H
#define RW_FREELIST_H
#include <rw/core/base/ostypes.h>
#include <rw/core/debug/debug.h>
#include <rw/core/string.h>
#include <rw/core/memory/linkedlist.h>


/*************************************************************************************************************
 Includes
 */

/*************************************************************************************************************
 Defines
 */

namespace rw
{

/**
This flag, which can be set/cleared through a call to FreeListSetFlags,
causes blocks that have no used entries to be freed immediately.  New
freelists have this flag set by default.

\see FreeList
\see FreeListGetFlags
\see FreeListSetFlags

\showinitializer
 */
#define RWFREELISTFLAG_FREEBLOCKS 1

/**
This flag is set for a free list internally. If memory is passed into the
free list module to make a new free list from, this flag is set. This is
to ensure that the free list module does not release this memory when
 FreeListRelease is called.

\see FreeList
\see FreeListGetFlags

\showinitializer
 */
#define RWFREELISTFLAG_STATIC     2


/*************************************************************************************************************
 Enums
 */

/*************************************************************************************************************
 Typedef Enums
 */

/*************************************************************************************************************
 Typedef Structs + Unions
 */

/**
A free list structure
 */
struct FreeList;

/*************************************************************************************************************
 Typedef Function Pointers
 */

/**
The callback function used on each free list entry by  FreeListForAllUsed.
 */
typedef void (*FreeListCallBack)(void *entry, void *pData);

/*************************************************************************************************************
 Structs + Unions
 */

/**
Represents a free list.  Free lists allow same-sized blocks of memory (entries)
to be allocated and deallocated at a higher speed than usual. When used with a
specific number of entries, the allocation/deallocation speed is suitable for
use in high-speed loops.

\member link                Used to link all the free lists into a list of free lists.
\member nonAlignedEntrySize Size of a single entry.
\member entrySize           Size of a single entry, rounded up to match alignment.
\member entriesPerBlock     The number of entries in one block.
\member maxBlocks           The maximum number of blocks that can be allocated.  Set to
                            0 for 'no maximum'.
\member alignment           The alignment of each entry in the block.
\member hint                Not currently used.
\member heapSize            The size in bytes of the bitmap of free entries.
\member blockList           Holds a double linked list with each block appended to the
                            linked list elements.
 */
struct FreeList
{
    LLLink      link;

#ifdef EA_DEBUG
    uint32_t    nonAlignedEntrySize;
    uint8_t     debugName[5];
#endif /* def EA_DEBUG */

    int32_t     entrySize;
    int32_t     entriesPerBlock;
    uint32_t    maxBlocks;
    int32_t     alignment;
    uint32_t    hint;
    uint32_t    heapSize;
    LinkList    blockList;

    uint32_t    flags;
};

/*************************************************************************************************************
 Global Variables
 */
extern RwBool rw_IsFreeListModuleOpened;

/*************************************************************************************************************
 Internal Functions Prototypes
 */

extern RwBool
FreeListModuleOpen(void);

extern void
FreeListModuleClose(void);

/*************************************************************************************************************
 External Functions Prototypes
 */

extern uint32_t
FreeListGetSize(uint32_t entrySize,
                uint32_t entriesPerBlock,
                uint32_t alignment,
                uint32_t blocks,
                uint32_t maxBlocks);

extern uint32_t
FreeListGetAlignment(uint32_t entrySize,
                     uint32_t entriesPerBlock,
                     uint32_t alignment,
                     uint32_t blocks,
                     uint32_t maxBlocks);

extern FreeList *
FreeListInitialize(void *   ptr,
                   uint32_t entrySize,
                   uint32_t entriesPerBlock,
                   uint32_t alignment,
                   uint32_t blocks,
                   uint32_t maxBlocks,
                   uint32_t hint);

extern RwBool
FreeListRelease(FreeList *freeList);

extern void *
FreeListAlloc(FreeList *freeList, uint32_t hint);

extern FreeList *
FreeListFree(FreeList *freeList, void *entry);

extern int32_t
FreeListPurge(FreeList *freeList);

extern FreeList *
FreeListForAllUsed(FreeList *freeList,
                   FreeListCallBack fpCallBack,
                   void *pData);

extern int32_t
FreeListPurgeAllFreeLists(void);

/**
Sets the four character debug string that is contained within the free list
structure in debug mode. This function is defined to nothing when not in
debug mode. The string that is set must be exactly four characters long.

\param freeList     The free list for which to set the four character string
\param fourCharName The four character string.

\see FreeListGetDebugName
 */
inline void
FreeListSetDebugName(FreeList *RWUNUSED_RELEASE(freeList), const RwChar *RWUNUSED_RELEASE(fourCharName))
{
    rwFUNCTION();

    rwASSERT(freeList);
    rwASSERT(4 == RwStrlen(fourCharName));

#ifdef EA_DEBUG
    RwStrcpy((RwChar *)(freeList->debugName), fourCharName);
#endif
    rwRETURNVOID();
}

/**
Returns a four character string that is associated with the free list
structure passed in. This function only works correctly if EA_DEBUG is defined.

If this function is called without EA_DEBUG defined, "N/A " will be returned.

\param freeList The free list structure to return the string from.

\return A constant pointer to the four character debug string.

\see FreeListSetDebugName
 */
inline const RwChar *
FreeListGetDebugName(const FreeList *RWUNUSED_RELEASE(freeList))
{
    rwFUNCTION();

    rwASSERT(freeList);

#ifdef EA_DEBUG
    rwRETURN((const RwChar *)freeList->debugName);
#else
    rwRETURN((const RwChar *)"N/A ");
#endif
}

/**
Sets the allocation mode of a freelist.

Currently the only read/write flag available is  RWFREELISTFLAG_FREEBLOCKS.
This flag is set by default and causes free blocks to be deallocated from the
heap as soon as a free list entry is freed.  If this flag is cleared all
empty free list blocks will remain.

\param freeList Pointer to the free list to change the flags on.
\param flags    The new flags for the free list.

\see FreeListGetFlags
\see FreeListAlloc
\see FreeListFree
 */
inline void
FreeListSetFlags(FreeList *freeList, uint32_t flags)
{
    rwFUNCTION();
    rwASSERT(rw_IsFreeListModuleOpened);
    rwASSERT(freeList);

    /* save allocation bit if set */
    freeList->flags = (flags & ~RWFREELISTFLAG_STATIC) |
                      (freeList->flags & RWFREELISTFLAG_STATIC);

    rwRETURNVOID();
}

/**
Gets the allocation mode of a free list. For more information see
 FreeListSetFlags.  In addition, the RWFREELISTFLAG_STATIC flag
can be read from the returned flags.

\param freeList Pointer to the free list to retrieve flags from.

\return Currently set flags for the specified free list.

\see FreeListSetFlags
 */
inline uint32_t
FreeListGetFlags(FreeList *freeList)
{
    rwFUNCTION();
    rwASSERT(rw_IsFreeListModuleOpened);
    rwASSERT(freeList);

    rwRETURN(freeList->flags);
}

} // namespace rw

#endif /* RW_FREELIST_H */
