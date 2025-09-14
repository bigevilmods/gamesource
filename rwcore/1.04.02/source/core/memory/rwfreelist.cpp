// (c) Electronic Arts. All Rights Reserved.

/// \file
/// A free list implementation

/*************************************************************************************************************
 Includes
 */
#include "rwsharedheaders.hpp"
#include "rw/core/memory/memoryfunctions.h"
#include "rw/core/memory/linkedlist.h"
#include "rw/core/memory/freelist.h"

namespace rw
{

/*************************************************************************************************************
 Defines
 */

/**
The default block alignment, if no alignment is specified
 */
#define rw_MALLOCALIGNMENT 8

/**
When this is defined, any free lists that are released by the client are not
freed, in case the client later wishes to have a new free list. Instead,
they are freed when the module shuts down instead.
 */
#define rw_TRACKFREELISTS

/**
When this is defined, new free lists created with FreeListInitialize are not
made with RwMalloc, but are themselves created through this single free list
of free list structures.
 */
#define rw_USEMASTERFORNEWFREELISTSx

/**
In debug mode, allocated blocks are initialized to this value.
 */
#define rw_ALLOCINITVAL  0xAA

/**
In debug mode, freed blocks are cleared to this value.
 */
#define rw_FREECLEARVAL  0xCD

/*************************************************************************************************************
 Enums
 */

/*************************************************************************************************************
 Typedef Enums
 */

/*************************************************************************************************************
 Typedef Structs + Unions
 */

/*************************************************************************************************************
 Typedef Function Pointers
 */

/*************************************************************************************************************
 Structs + Unions
 */

/*************************************************************************************************************
 Static Variables
 */

/**
This is set to TRUE when the first call is made to FreeListInitialize (or its variants).
 */

RwBool rw_IsFreeListModuleOpened = FALSE;

#if defined rw_TRACKFREELISTS

/**
This is a doubly-linked list that holds references to all the free lists currently
in use by the client. It is not used unless  rw_TRACKFREELISTS is defined.
If it is used then all remaining linked list links will be automatically released
when the free list module is shut down, as will the linked list structures
themselves.

 */
static LinkList rw_FreeListList;

#endif /* defined rw_TRACKFREELISTS */


#if defined rw_USEMASTERFORNEWFREELISTS

/**
This holds a free list of free list structures created by this module!
Since they're all the same size, they can be speedily allocated this way.
 */
static FreeList rw_masterFreeList;

/**
This is set up to be a pointer to the  rw_masterFreeList variable during
module initialization.

 */
static FreeList *rw_masterFreeListPtr = (FreeList *)0;

#endif /* def rw_USEMASTERFORNEWFREELISTS */

/*************************************************************************************************************
 Global Variables
 */

/*************************************************************************************************************
 Function prototypes
 */

/*************************************************************************************************************
 Static Functions
 */

/*************************************************************************************************************
 Internal Functions
 */

/**
Opens the free list module.  If  rw_TRACKFREELISTS is
defined, it initializes the list of free lists.  When
 rw_USEMASTERFORNEWFREELISTS is defined, a free list is created from which
future free lists will be allocated to increase free list creation speed.

\return TRUE on success, FALSE on failure.
 */
RwBool
FreeListModuleOpen(void)
{
    rwFUNCTION();
    rwASSERT(FALSE == rw_IsFreeListModuleOpened);

    rw_IsFreeListModuleOpened = TRUE;

#ifdef rw_TRACKFREELISTS
    LinkListInitialize(&rw_FreeListList);
#endif /* def rw_TRACKFREELISTS */

#ifdef rw_USEMASTERFORNEWFREELISTS
    rw_masterFreeListPtr = FreeListInitialize(sizeof(FreeList), 16,
                                              rw_MALLOCALIGNMENT, 0,
                                              &rw_masterFreeList, 0);
    if ((FreeList *)0 == rw_masterFreeListPtr)
    {
        rw_IsFreeListModuleOpened = FALSE;
        rwRETURN(FALSE);
    }

#   ifdef rw_TRACKFREELISTS

    /* Remove master free list from list of free lists so it isn't destroyed
       before the other free lists. This would be bad as all the other free
       lists are allocated in its members, so they could not be deallocated. */
    LinkListRemoveLLLink(&rw_masterFreeListPtr->link);

#   endif /* def rw_TRACKFREELISTS */

#endif /* def rw_USEMASTERFORNEWFREELISTS */

    rwRETURN(TRUE);
}

/**
Performs the opposite operation to  FreeListModuleOpen.  If a
list of free lists is being kept (due to  rw_TRACKFREELISTS being defined),
these free lists are also released.
 */
void
FreeListModuleClose(void)
{
    rwFUNCTION();
    rwASSERT(rw_IsFreeListModuleOpened);

#ifdef rw_TRACKFREELISTS
    {
        FreeList     *freeList;
        LLLink       *link;
        LLLink const *lastLink;

        /* clear up all free lists */
        link     = LinkListGetFirstLLLink(&rw_FreeListList);
        lastLink = LinkListGetTerminator(&rw_FreeListList);
        while (link != lastLink)
        {
            freeList = (FreeList *)LLLinkGetData(link);
            FreeListRelease(freeList);
            link     = LinkListGetFirstLLLink(&rw_FreeListList);
        }
    }
#endif /* rw_TRACKFREELISTS */


#ifdef rw_USEMASTERFORNEWFREELISTS

    /* destroy master free list */
    FreeListRelease(rw_masterFreeListPtr);
    rw_masterFreeListPtr = (FreeList *)0;

#endif /* rw_USEMASTERFORNEWFREELISTS */

    rw_IsFreeListModuleOpened = FALSE;
    rwRETURNVOID();
}

/**
Returns whether the free list block has no allocated entries.

\param heap        Pointer to start of allocation bitmap.
\param heapEntries Number of bytes in allocation bitmap.

\return TRUE if no allocated entry bit is found.
 */
static RwBool
rw_FreeListBlockIsEmpty(uint8_t *heap, uint32_t heapEntries)
{
    uint32_t i;
    uint32_t sum = 0;

    rwFUNCTION();

    for (i = 0; i < heapEntries; i ++)
    {
        sum += heap[i];
    }

    rwRETURN((RwBool)(0 == sum));
}

/*************************************************************************************************************
 External Functions
 */

/**
This returns the size of memory in bytes that is required for
a free list structure.  That memory can then be passed into
FreeListInitialize
 */
uint32_t
FreeListGetSize(uint32_t RWUNUSED(entrySize),
                uint32_t RWUNUSED(entriesPerBlock),
                uint32_t RWUNUSED(alignment),
                uint32_t RWUNUSED(blocks),
                uint32_t RWUNUSED(maxBlocks))
{
    rwFUNCTION();
    rwRETURN(sizeof(FreeList));
}

/**
This returns the default alignment for a free list structure
 */
uint32_t
FreeListGetAlignment(uint32_t RWUNUSED(entrySize),
                     uint32_t RWUNUSED(entriesPerBlock),
                     uint32_t RWUNUSED(alignment),
                     uint32_t RWUNUSED(blocks),
                     uint32_t RWUNUSED(maxBlocks))
{
    rwFUNCTION();
    rwRETURN(rw_MALLOCALIGNMENT);
}

/**
Creates an empty free list, using the memory given to it if a non-zero pointer
is passed in.  Some blocks may be preallocated to attempt to prevent a delay from
occurring during the later execution of client code when a new entry is
requested.

The initialized free list must be released later by  FreeListRelease.

\param entrySize       The size in bytes of a single entry.
\param entriesPerBlock The number of entries is a single block.  This must
                       be a multiple of 8.
\param alignment       The alignment which entries will have.  This must be
                       given to prevent a warning from appearing in debug
                       mode.
\param blocks          The number of blocks to preallocate.
\param freeList        Pointer to preallocated memory for new free list.  If
                       this is 0, memory will be allocated in this module.
\param hint            Not currently used.  Set to 0.

\return A pointer to the newly-created free list, or 0 on failure.

\see FreeListRelease
 */
FreeList *
FreeListInitialize(void *   ptr,
                   uint32_t entrySize,
                   uint32_t entriesPerBlock,
                   uint32_t alignment,
                   uint32_t blocks,
                   uint32_t maxBlocks,
                   uint32_t RWUNUSED(hint))
{
    uint32_t    alignedBlockSize;
    uint32_t    heapSize;
    FreeList *freeList;
    rwFUNCTION();
    rwASSERT(entrySize);
    rwASSERT(entriesPerBlock);
    rwASSERT(!(alignment & (alignment - 1))); /* Assert power of 2 */

    /* Initialise the free list module if necessary */
    if (!rw_IsFreeListModuleOpened)
    {
        rwMESSAGE(("Free List core module not opened"));
        rwRETURN((FreeList *)0);
    }

    rwASSERT(rw_IsFreeListModuleOpened);

    freeList = (FreeList *)ptr;

    /* check alignment */
    if (alignment == 0)
    {
        rwMESSAGE(("Unspecified alignment, defaulting to %d, might be wasting space", rw_MALLOCALIGNMENT));
        alignment = rw_MALLOCALIGNMENT;
    }

    /* determine whether a free list structure was passed in */
    if ((FreeList *)0 == freeList)
    {
        rwMESSAGE(("Warning, allocating memory in FreeListInitialize - "
                   "in RW4, allocated memory should be passed in through 'ptr'"));

#ifdef rw_USEMASTERFORNEWFREELISTS

        /* allocate memory for the free list structure from the free list free list */
        freeList = (FreeList *)FreeListAlloc(rw_masterFreeListPtr, 0);
#else
        /* allocate using a standard malloc */
        freeList = (FreeList *)RwMalloc(sizeof(FreeList));

#endif /* rw_USEMASTERFORNEWFREELISTS */

        if ((FreeList *)0 == freeList)
        {
            rwRETURN((FreeList *)0);
        }
        freeList->flags = 0;
    }
    else
    {
        freeList->flags = RWFREELISTFLAG_STATIC;
    }

    /* Default to freeing empty blocks */
    freeList->flags |= RWFREELISTFLAG_FREEBLOCKS;

    /* calculate the size of an aligned block */
    alignedBlockSize = RwRoundUp(entrySize, alignment);

    /* calculate the size of the heap */
    heapSize = RwRoundUp(entriesPerBlock, 0x8);
    heapSize >>= 3;

    /* setup free list */
#ifdef EA_DEBUG
    freeList->nonAlignedEntrySize = entrySize;
    RwStrcpy((RwChar *)(freeList->debugName), "----");
#endif /* EA_DEBUG */
    freeList->entrySize           = (int32_t)alignedBlockSize;
    freeList->entriesPerBlock     = (int32_t)entriesPerBlock;
    freeList->alignment           = (int32_t)alignment;
    freeList->heapSize            = heapSize;
    freeList->maxBlocks           = maxBlocks;

    /* initialize the block list */
    LinkListInitialize(&freeList->blockList);

    while (blocks > 0)
    {
        void *block;

        /* allocate memory for data block and portion of heap */
        block = RwMalloc(sizeof(LLLink) + heapSize +
                          (entriesPerBlock * alignedBlockSize) +
                          alignment - 1);
        if ((void *)0 == block)
        {
            FreeListRelease(freeList);
            rwRETURN((FreeList *)0);
        }

        /* add block to block list */
        {
            LLLink *link;

            link = (LLLink*)block;
            /* add block to block list */
            LLLinkInitialize(link);
            LinkListAddLLLink(&freeList->blockList, link);
        }

        /* setup heap */
        RwMemset((uint8_t*)block + sizeof(LLLink), 0, heapSize);
        blocks --;
    }

#ifdef rw_TRACKFREELISTS

    /* add to the list of free lists */
    LinkListAddLLLink(&rw_FreeListList, &freeList->link);

#endif /* def rw_TRACKFREELISTS */

    rwRETURN(freeList);
}

/**
Releases the block (or blocks) of memory used by the specified free list.
The memory for the free list structure itself is not freed.

\param freeList  A pointer to the free list

\return TRUE if successful or FALSE if there is an error (always TRUE
just now).

\see FreeListAlloc
\see FreeListInitialize
\see FreeListPurge
\see FreeListForAllUsed
\see FreeListPurgeAllFreeLists
 */
RwBool
FreeListRelease(FreeList *freeList)
{
    LLLink *link;
    LLLink const *lastLink;

    rwFUNCTION();

    rwASSERT(rw_IsFreeListModuleOpened);
    rwASSERT(freeList);

    /* free blocks */
    link     = LinkListGetFirstLLLink(&freeList->blockList);
    lastLink = LinkListGetTerminator(&freeList->blockList);
    while (link != lastLink)
    {
        LinkListRemoveLLLink(link);
        RwFree(link);
        link = LinkListGetFirstLLLink(&freeList->blockList);
    }

#ifdef rw_USEMASTERFORNEWFREELISTS
    /* The master free list doesn't appear on the list of free lists,
     * or in itself. */
    if (rw_masterFreeListPtr == freeList)
    {
        RwFree(freeList);
        rwRETURN(TRUE);
    }

#endif /* rw_USEMASTERFORNEWFREELISTS */


#ifdef rw_TRACKFREELISTS

    /* Remove from the list of free lists */
    LinkListRemoveLLLink(&freeList->link);

#endif /* rw_TRACKFREELISTS */

    /* free the freelist if memory was allocated by this module */
    if (!(freeList->flags & RWFREELISTFLAG_STATIC))
    {
#ifdef rw_USEMASTERFORNEWFREELISTS
        FreeListFree(rw_masterFreeListPtr, freeList);
#else
        RwFree(freeList);
#endif /* rw_USEMASTERFORNEWFREELISTS */
    }

    rwRETURN(TRUE);
}


/**
Retrieves an unused entry from the specified free list. If, prior to this
function call, the free list is empty, the whole free list block is
allocated memory. If there are no more unused entries left, a new block
is allocated, and an entry is returned from it.

In debug mode, the entry's memory will be initialized with the value
defined in rw_ALLOCINITVAL.

\param freelist  Pointer to the free list.
\param hint      Not currently used.  Set to 0.

\return A pointer to an unused entry if successful or 0
if there is an error.

\see FreeListInitialize
\see FreeListPurge
\see FreeListForAllUsed
\see FreeListRelease
\see FreeListPurgeAllFreeLists
 */
void *
FreeListAlloc(FreeList *freeList,
              uint32_t RWUNUSED(hint))
{
    uint8_t  heapElement, mask;         /* current heap element and a mask to
                                           find free element */
    void     *freeEntry = (void *)0;    /* set to a free heap entry if found */
    uint32_t i, j;                      /* loop variables */
    uint32_t heapEntries;               /* number of entries in a heap */
    uint8_t  *heap;                     /* pointer to the current heap */
    LLLink *link;                       /* used to traverse the block list */
    LLLink const *lastLink;
    uint32_t checkEntries;              /* number of entries not checked */
    rwFUNCTION();
    rwASSERT(rw_IsFreeListModuleOpened);

    /* determine the total number of heap entries in the free list */
    heapEntries  = freeList->heapSize;

    /* find a free entry on heap */
    link     = LinkListGetFirstLLLink(&freeList->blockList);
    lastLink = LinkListGetTerminator (&freeList->blockList);

    while (link != lastLink && (void *)0 == freeEntry)
    {
        heap = (uint8_t*)link + sizeof(LLLink);

        /* check the number of entries per block */
        checkEntries = (uint32_t)freeList->entriesPerBlock;

        for (i = 0; i < heapEntries; i ++)
        {
            heapElement = heap[i];

            /* check for a free entry */
            if (heapElement != 0xFF)
            {
                j = 0;
                while(j < 8 && checkEntries > 0)
                {
                    mask = (uint8_t)(0x80 >> j);
                    if ((heapElement & mask) == 0)
                    {
                        heap[i] |= mask;

                        /* align data block */
                        uint8_t *aligned  = reinterpret_cast<uint8_t*>(link) + sizeof(LLLink) + heapEntries;
                        aligned = rw::MemAlign(aligned, (uint32_t)freeList->alignment);

                        /* get the entry */
                        freeEntry = aligned + (((i << 3) + j) * freeList->entrySize);

                        break;
                    }
                    j ++;
                    checkEntries --;
                }
            }
            else
            {
                checkEntries -= 8;
            }
            if ((void *)0 != freeEntry)
                break;
        }
        /* move onto the next block */
        link = LLLinkGetNext(link);
    }

    /* make sure an entry was found */
    if ((void *)0 == freeEntry)
    {
        /* add new the data block */
        void     *dataBlock;

        dataBlock = RwMalloc(sizeof(LLLink) + heapEntries +
                      (freeList->entriesPerBlock * freeList->entrySize) +
                      freeList->alignment - 1);
        if ((void *)0 == dataBlock)
        {
            rwRETURN((void *)0);
        }

        /* prepare new heap */
        RwMemset((uint8_t*)dataBlock + sizeof(LLLink), 0, heapEntries);

        /* add block to block list */
        link = (LLLink*)dataBlock;
        LLLinkInitialize(link);
        LinkListAddLLLink(&freeList->blockList, link);

        /* allocate entry */
        *((uint8_t*)dataBlock + sizeof(LLLink)) = 0x80;

        {
            /* align data block */
            uint8_t *aligned = (uint8_t*)dataBlock + sizeof(LLLink) + heapEntries;
            aligned = rw::MemAlign(aligned, (uint32_t)freeList->alignment);

            freeEntry = aligned;
        }
    }

#ifdef EA_DEBUG
    RwMemset(freeEntry, rw_ALLOCINITVAL, (size_t)freeList->entrySize);
#endif /* EA_DEBUG */

    rwRETURN(freeEntry);
}

/**
Releases an entry that was allocated with FreeListAlloc.  In debug
mode, the entry's memory will be overwritten with the value defined
in rw_FREECLEARVAL.

\param freeList The free list that the entry was allocated from.
\param entry    The entry to be freed.

\return A pointer to the free list on success, or 0 on failure.
 */
FreeList *
FreeListFree(FreeList *freeList, void *entry)
{
    uint32_t heapEntries;
    void     *dataBlock;
    LLLink *link;
    LLLink const *lastLink;
    rwFUNCTION();
    rwASSERT(rw_IsFreeListModuleOpened);
    rwASSERT(freeList);

    heapEntries = freeList->heapSize;

    /* find the block */
    link     = LinkListGetFirstLLLink(&freeList->blockList);
    lastLink = LinkListGetTerminator(&freeList->blockList);

    while (link != lastLink)
    {
        dataBlock = (void*)((uint8_t*)link + sizeof(LLLink) + heapEntries);

        if (entry >= dataBlock &&
            entry <= (void*)((uint8_t*)dataBlock +
                (freeList->entriesPerBlock * freeList->entrySize)))
        {
            uint8_t  *heap;
            size_t entryIndex;
            size_t heapElement;
            uint8_t  mask;

            /* found the block now determine the entry index */
            entryIndex = (size_t)(((uint8_t*)entry - (uint8_t*)dataBlock) /
                            freeList->entrySize);

            /* get the heap element and the bit mask */
            heapElement = entryIndex >> 3;
            mask        = (uint8_t)(0x80 >> (entryIndex - (heapElement << 3)));

            /* get a pointer to the heap */
            heap = (uint8_t*)link + sizeof(LLLink);

#ifdef EA_DEBUG
            if ((heap[heapElement] & mask) == 0)
            {
                rwMESSAGE(("entry %p was not previously allocated", entry));
            }
            rwASSERT(heap[heapElement] & mask);

            /* stamp some entry for debug */
            RwMemset(entry, rw_FREECLEARVAL, freeList->nonAlignedEntrySize);
#endif /* EA_DEBUG */

            /* set the heap element as freed */
            heap[heapElement] &= (uint8_t)(~mask);

            /* if we're allowed to free empty blocks */
            if (freeList->flags & RWFREELISTFLAG_FREEBLOCKS)
            {
                /* if this block is now empty */
                if (rw_FreeListBlockIsEmpty(heap,heapEntries))
                {
                    LinkListRemoveLLLink(link);
                    RwFree(link);
                }
            }

            rwRETURN(freeList);
        }
        link = LLLinkGetNext(link);
    }

    /* the entry wasn't found */
    rwASSERTM(FALSE, ("Entry %p was not allocated from this FreeList", entry));

    rwRETURN((FreeList *)0);
}

/**
Releases the memory for any blocks in the specified free list that wholly
contain unused entries.  It returns the amount of memory (in bytes) recovered.

\param freeList  The free list to purge.

\return Returns the amount of memory freed.

\see FreeListAlloc
\see FreeListInitialize
\see FreeListFree
\see FreeListForAllUsed
\see FreeListRelease
\see FreeListPurgeAllFreeLists
 */
int32_t
FreeListPurge(FreeList *freeList)
{
    uint32_t heapEntries;
    LLLink *link, *nextLink;
    LLLink const *lastLink;
    uint32_t blocksFreed = 0;
    rwFUNCTION();
    rwASSERT(rw_IsFreeListModuleOpened);
    rwASSERT(freeList);

    heapEntries = freeList->heapSize;

    /* find the block */
    link     = LinkListGetFirstLLLink(&freeList->blockList);
    lastLink = LinkListGetTerminator(&freeList->blockList);

    while (link != lastLink)
    {
        uint8_t *heap;

        /* get a pointer to the heap */
        heap = (uint8_t*)link + sizeof(LLLink);

        nextLink = LLLinkGetNext(link);

        if (rw_FreeListBlockIsEmpty(heap, heapEntries))
        {
            /* if this heap is totally empty then free the block */
            LinkListRemoveLLLink(link);
            RwFree(link);
            blocksFreed ++;
        }

        link = nextLink;
    }

    rwRETURN((int32_t)(blocksFreed *
        (freeList->entrySize * freeList->entriesPerBlock + sizeof(LLLink) + heapEntries))
        );
}

/**
Applies the given callback function to all used entries in the
specified free list. The format of the callback function is:
\code
     void (*FreeListCallBack)(void *mem, void *data);
\endcode
where "data" is a pointer to user-defined data pointer to pass to the
callback function.

\param freeList    A pointer to the free list.
\param fpCallBack  A pointer to the callback to call for each used block.
\param pData       A pointer to user-supplied data to pass to the callback function.

\return A pointer to the free list if successful or 0 if there is an error.

\see FreeListAlloc
\see FreeListInitialize
\see FreeListFree
\see FreeListPurge
\see FreeListRelease
\see FreeListPurgeAllFreeLists
 */
FreeList *
FreeListForAllUsed(FreeList * freeList,
                     FreeListCallBack fpCallBack,
                     void *pData)
{
    uint32_t heapEntries;
    LLLink *link, *nextLink;
    LLLink const *lastLink;
    rwFUNCTION();
    rwASSERT(rw_IsFreeListModuleOpened);
    rwASSERT(freeList);
    rwASSERT(fpCallBack);

    heapEntries = freeList->heapSize;

    /* for each block */
    link     = LinkListGetFirstLLLink(&freeList->blockList);
    lastLink = LinkListGetTerminator(&freeList->blockList);

    while (link != lastLink)
    {
        uint8_t *heap, *heapCopy;
        uint32_t heapElement;
        uint8_t  mask;
        uint32_t i;

        /* get a pointer to the heap */
        heap = (uint8_t*)link + sizeof(LLLink);

        /**
        Copy heap elements used flags as an FreeListFree
        in the callback can mash them if it (rightly) frees the whole block,
        yet we may go on looping over them when they're dead.
        (I really don't like the idea of the callback calling FreeListFree,
        but the old implementation used to support it.)
        */
        heapCopy = (uint8_t *)RwMalloc(heapEntries);
        if (!heapCopy)
        {
            rwRETURN((FreeList *)0);
        }
        RwMemcpy(heapCopy, heap, heapEntries);

        /**
        Store pointer to next link now, as some rw code
        apparently expects to be able to remove links from the list
        in the callback function via FreeListFree (!)
        */
        nextLink = LLLinkGetNext(link);

        /* check each heap element for used entries */
        for (i = 0; i < heapEntries; i ++)
        {
            uint32_t j;

            heapElement = heapCopy[i];
            if (0 == heapElement)
            {
                continue;
            }

            /* there's a used entry in here somewhere */
            for (j=0; j<8; j++)
            {
                mask = (uint8_t)(0x80 >> j);
                if (0 == (heapElement & mask))
                {
                    continue;
                }

                /* this one's used, what's the address of the element? */

                /* align data block */
                uint8_t* aligned  = (uint8_t*)link + sizeof(LLLink) + heapEntries;
                aligned = rw::MemAlign(aligned, (uint32_t)freeList->alignment);

                /* get the entry */
                uint8_t* entry = aligned + (((i << 3) + j) * freeList->entrySize);

                /* You're welcome */
                fpCallBack((void *)entry, pData);
            }
        }

        RwFree(heapCopy);

        link = nextLink;
    }

    rwRETURN(freeList);
}

/**
Purges all free lists in the system. It returns the amount of memory freed as a result.

\return The amount of memory freed

\see FreeListInitialize
\see FreeListRelease
\see FreeListAlloc
\see FreeListFree
\see FreeListPurge
\see FreeListForAllUsed
 */
int32_t
FreeListPurgeAllFreeLists(void)
{
    int32_t  memoryFreed = 0;

    rwFUNCTION();

#if !defined rw_TRACKFREELISTS

    rwMESSAGE(("Free list purge did not happen - rw_TRACKFREELISTS was not defined during compilation"));
    rwASSERT(FALSE);

#else
    {
        LLLink *cur;
        LLLink const *end;

        cur = LinkListGetFirstLLLink(&rw_FreeListList);
        end = LinkListGetTerminator(&rw_FreeListList);
        while (cur != end)
        {
            /* Purge this list */
            FreeList *const freeList = (FreeList *const)cur;
            memoryFreed += FreeListPurge(freeList);

            /* Get next */
            cur = LLLinkGetNext(cur);
        }
    }
#endif /* !defined rw_TRACKFREELISTS */

    rwRETURN(memoryFreed);

}

} // namespace rw

