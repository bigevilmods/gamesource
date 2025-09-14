/*H************************************************************************************************/
/*!

    \File
        AptNativeHash.cpp

    \Description
        This source contains the methods definition to manage the AptNativeHash.
        The AptNativeHash is an hash-table where the key are insensitive strings and the values are AptValue pointer.
        It's an open hash so during collision you can store the data on the next or previous cell without
        expanding the hash-table.
        There is a specific optimization for "prototype" and "__proto__" value, they are directly stored as pointer.

    \Notes
        None.

    \Copyright
        (c) 2005 Electronic Arts Inc.

    \Version    1.0    02/26/04 (ONA) First Version
    \Version    1.1    03/05/08 (RB)  Only define LENGTH_PROTOTYPE if EA_DEBUG is defined.
                                      Changed HashFindKey signature to match header file.
    \Version    1.2    03/05/08 (RB)  Use AptDebuggerOpaqueMessage in GenerateLocalsListKeys

*/
/************************************************************************************************H*/

#define hashmask(n) ((n) - 1)

#include "_Apt.h"
#include "AptSpriteMembersConsts.h"
#include "MainInline.h"

static const unsigned int   HASH_VALUE_PROTOTYPE  = 0x0699;
static const unsigned int   HASH_VALUE_PROTO      = 0x6BBD;

APT_DEFINE_CUSTOM_METRICS(AptHashTableMemory, NULL);
APT_DEFINE_CUSTOM_METRICS(AptHashBlocks, &AptHashTableMemory);
APT_DEFINE_OBJECT_METRICS_EXPLICIT(AptNativeHash, &AptHashTableMemory);


// fusionchange - modifying the implementation so that it lines up with the header, otherwise we get a link error
// with the snc linker.
//AptNativeHash::AptNativeHash(int nTotalSize)
AptNativeHash::AptNativeHash(const int32_t nTotalSize)
:mnTotalSize(nTotalSize),
mpData(NULL),
mp__proto__(NULL),
mpPrototype(NULL),
nEventHandlers(0)
{
    // $bug886 - DC - If mnTotalSize is not a power of two, make it one. (or bad things will happen)
    if( ( mnTotalSize & (mnTotalSize - 1) ) != 0)
    {
        // Take mnTotalSize up to the next higher power of two!!!
        int NextHighestPowerOfTwo = 1;
        while(NextHighestPowerOfTwo < mnTotalSize)
        {
            NextHighestPowerOfTwo <<= 1;
        }
        mnTotalSize = NextHighestPowerOfTwo;
    }

    //  Check some constants
#if defined(EA_DEBUG)
    const uint32_t       LENGTH_PROTOTYPE    =   9;
    const uint32_t       LENGTH_PROTO        =   9;
    APT_ASSERT(StringPool::GetString(SC___proto__)->GetLength() == LENGTH_PROTOTYPE);
    APT_ASSERT(StringPool::GetString(SC_prototype)->GetLength() == LENGTH_PROTO);
    APT_ASSERT(LENGTH_PROTOTYPE == LENGTH_PROTO);
#endif // EA_DEBUG
    APT_ASSERT(StringPool::GetString(SC___proto__)->UpdateHashValue() == HASH_VALUE_PROTO);
    APT_ASSERT(StringPool::GetString(SC_prototype)->UpdateHashValue() == HASH_VALUE_PROTOTYPE);
    APT_ASSERT( ( mnTotalSize & (mnTotalSize - 1) ) == 0 && "Total Size was not made a power of Two!!!" );
}


AptNativeHash::~AptNativeHash()
{
    if(mpData)
    {
        APT_ASSERT(false && "Apt GC Pointers were not destroyed before deletion. ");
        DestroyGCPointers();
    }
    mpData = NULL;
    mp__proto__ = NULL;
    mpPrototype = NULL;
}

/*F************************************************************************************************/
/*!
    \Function    AptNativeHash::DestroyGCPointers

    \Description
        Breaks the links we have to Garbage collected objects.

    After this the we should not have any references to Garbage collected objects any more.

    \Input
        None.

    \Output
        None.

    \Notes
        None.

    \Version    1.0        09/09/04 (DC) Bringin' it to ya
    \Version    1.1        04/11/05 (DC) Now releases memory it's immediately.

*/
/************************************************************************************************F*/
void    AptNativeHash::DestroyGCPointers()
{
    UnsetPrototype();
    Unset__Proto__();

    if (IsEmpty())
    {
        return;
    }

    nEventHandlers = 0;

    for (int i = 0; i < mnTotalSize ; i++)
    {
        if(mpData[i].mValue)
        {
            APT_DEC(mpData[i].mValue);
            mpData[i].mValue = NULL;
        }
        // considering that key is not garbage collected.
        if (mpData[i].Key.IsValid())
        {
            mpData[i].Key.Invalidate();
        }
    }

    // No need to hold on to this anymore, just give the memory away.
    APT_FREE_ARRAY_TRACKER(AptHashBlocks, mpData, AptHashItem, mnTotalSize);
    mpData = NULL;
}


/*F************************************************************************************************/
/*!
    \Function    AptNativeHash::SetAt

    \Description
        Insert function's purpose here

    Insert your long description here.

    \Input      nIndex            -
    \Input      pValue            -

    \Output
        None.

    \Notes
        None.

    \Version    1.0        07/19/05 (DC) Bringin' it to ya

*/
/************************************************************************************************F*/
void        AptNativeHash::SetAt(const int32_t nIndex, AptValue * const pValue)
{
    APT_ASSERT(pValue != NULL);
    APT_ASSERT(mpData != NULL);
    AptValue *  pOldValue = mpData[nIndex].mValue;
    APT_INC(pValue);
    APT_DECSAFE(pOldValue);
    mpData[nIndex].mValue = pValue;
}

/*F************************************************************************************************/
/*!
    \Function    AptNativeHash::SetAt

    \Description
        Sets a new AptValue at the specified index (and takes care of Reference Counting).

    \Input      pItem             -
    \Input      pValue            -

    \Output
        None.

    \Notes
        None.

    \Version    1.0        07/19/05 (DC) Bringin' it to ya

*/
/************************************************************************************************F*/
void        AptNativeHash::SetAt(AptHashItem * const pItem, AptValue * const pValue)
{
    APT_ASSERT(pValue != NULL);
    AptValue *  pOldValue = pItem->mValue;
    APT_INC(pValue);
    APT_DECSAFE(pOldValue);
    pItem->mValue = pValue;
}


/*F************************************************************************************************/
/*!
    \Function    AptNativeHash::OverwriteAt

    \Description
        Sets a new AptValue at the specified index (and takes care of Reference Counting).

    \Input      nIndex            -
    \Input      pValue            -

    \Output
        None.

    \Notes
        None.

    \Version    1.0        07/19/05 (DC) Bringin' it to ya

*/
/************************************************************************************************F*/
void        AptNativeHash::OverwriteAt(const int32_t nIndex, AptValue * pValue)
{
    APT_ASSERT(pValue != NULL);
    APT_ASSERT(mpData != NULL);
	APT_ASSERT(nIndex < mnTotalSize && nIndex >= 0);

    APT_INC(pValue);
    mpData[nIndex].mValue = pValue;
}

/*F************************************************************************************************/
/*!
    \Function    AptNativeHash::OverwriteAt

    \Description
        Sets a new AptValue at the specified index (and takes care of Reference Counting).

    \Input      pHashItem         -
    \Input      pValue            -

    \Output
        None.

    \Notes
        None.

    \Version    1.0        07/19/05 (DC) Bringin' it to ya

*/
/************************************************************************************************F*/
void        AptNativeHash::OverwriteAt(AptHashItem * const pHashItem, AptValue * pValue)
{
    APT_ASSERT(pValue != NULL);
    APT_INC(pValue);
    pHashItem->mValue = pValue;
}

/*F************************************************************************************************/
/*!
    \Function    AptNativeHash::UnsetAt

    \Description
        Clears an AptValue at the specified index (and takes care of Reference Counting).

    \Input      pHashItem         -

    \Output
        None.

    \Notes
        None.

    \Version    1.0        07/19/05 (DC) Bringin' it to ya

*/
/************************************************************************************************F*/
void    AptNativeHash::UnsetAt(AptHashItem * const pHashItem)
{
    APT_ASSERT(pHashItem         != NULL);
    APT_ASSERT(pHashItem->mValue != NULL);
    APT_DEC(pHashItem->mValue);
    pHashItem->mValue = NULL;
}

/*F************************************************************************************************/
/*!
    \Function    AptNativeHash::SetIfNotExists

    \Description
        Sets a new value into the hash only if the Key Exists.

    If the key does not exist we do nothing.

    \Input      pKey              - String Key
    \Input      pValue            - Value to set if Key does not Exist

    \Output
        None.

    \Notes
        This will expand the has if needed.

    \Version    1.0        09/09/04 (DC) Bringin' it to ya

*/
/************************************************************************************************F*/
void AptNativeHash::SetIfNotExists(const AptNativeString * const pKey, AptValue * const pValue)
{
    // this can be probably be optimized to do everything in one pass
    if (Lookup(pKey) == NULL)
    {
        Set(pKey, pValue);
    }
}

/*F************************************************************************************************/
/*!
    \Function    AptNativeHash::Set__Proto__

    \Description
        sets the actionscript __proto__ member.

    \Input      pValue            -

    \Output
        None.

    \Notes
        None.

    \Version    1.0        07/19/05 (DC) Bringin' it to ya

*/
/************************************************************************************************F*/
void       AptNativeHash::Set__Proto__(AptValue * const pValue)
{
    APT_INCSAFE(pValue);
    APT_DECSAFE(mp__proto__);
    mp__proto__ = pValue;
}

/*F************************************************************************************************/
/*!
    \Function    AptNativeHash::Unset__Proto__

    \Description
        clears the actionscript __proto__ member.

    \Input
        None.

    \Output
        None.

    \Notes
        None.

    \Version    1.0        07/19/05 (DC) Bringin' it to ya

*/
/************************************************************************************************F*/
void       AptNativeHash::Unset__Proto__()
{
    if (mp__proto__)
    {
        APT_DEC(mp__proto__);
        mp__proto__ = NULL;
    }
}


/*F************************************************************************************************/
/*!
    \Function    AptNativeHash::SetPrototype

    \Description
        Sets the actionscript prototype member

    \Input      pValue            -

    \Output
        None.

    \Notes
        None.

    \Version    1.0        07/19/05 (DC) Bringin' it to ya

*/
/************************************************************************************************F*/
void       AptNativeHash::SetPrototype(AptValue * const pValue)
{
    APT_INCSAFE(pValue);
    APT_DECSAFE(mpPrototype);
    mpPrototype = pValue;
}

/*F************************************************************************************************/
/*!
    \Function    AptNativeHash::UnsetPrototype

    \Description
        Clears the Actionscript prototype member

    \Input
        None.

    \Output
        None.

    \Notes
        None.

    \Version    1.0        07/19/05 (DC) Bringin' it to ya

*/
/************************************************************************************************F*/
void       AptNativeHash::UnsetPrototype()
{
    if (mpPrototype)
    {
        APT_DEC(mpPrototype);
        mpPrototype = NULL;
    }
}

/*F************************************************************************************************/
/*!
    \Function    AptNativeHash::Set

    \Description
        Sets the given value to the given Key in the table.


    \Input      pKey              - String Key
    \Input      pValue            - Value to set to the Key.

    \Output
        None.

    \Notes
        This will expand the has if needed.

    \Version    1.0        09/09/04 (DC) Bringin' it to ya

*/
/************************************************************************************************F*/
void    AptNativeHash::Set(const AptNativeString * const pKey, AptValue * const pValue)
{
    if (pValue == NULL)
    {
        Unset(pKey);
        return;
    }

    if  (pKey->IsEmpty())
    {
        return;
    }

    uint32_t    h = pKey->UpdateHashValue();

    if ((HASH_VALUE_PROTOTYPE == h) && (pKey->EqualNoCase(*StringPool::GetString(SC_prototype))))
    {
        SetPrototype(pValue);
        return;
    }
    else if ((HASH_VALUE_PROTO == h) && (pKey->EqualNoCase(*StringPool::GetString(SC___proto__))))
    {
        Set__Proto__(pValue);
        return;
    }

    if (IsEmpty())
    {
        //  hash empty, allocate before add a new key / value
        FirstAllocation();
    }

    HashSet(pKey, pValue);
}
/*F************************************************************************************************/
/*!
    \Function    AptNativeHash::Unset

    \Description
        Gets rid of the key and value if found.

    makes it as if the item never existed.

    \Input      pKey              - String Key of item to remove.

    \Output
        None.

    \Notes
        None.

    \Version    1.0        09/09/04 (DC) Bringin' it to ya

*/
/************************************************************************************************F*/
void    AptNativeHash::Unset(const AptNativeString * const pKey)
{
    if  (pKey->IsEmpty())
    {
        return;
    }

    uint32_t    h = pKey->UpdateHashValue();

    if (IsEmpty() == false)
    {
        AptHashItem *   pHashItem;

        pHashItem = HashFindKey(pKey);
        if (pHashItem != NULL)
        {
            //  We unset a key / value
            //  We clear the key (it's still valid) So we know that a key was here
            //  This is actually very important because the search function stops searching at an invalid
            //  string! So if we invalidate here the search will stop early... Thankfully we will insert into
            //  locations with a cleared string, so nothing is really lost (well, after $bug884 was fixed).
            pHashItem->Key.Clear();
            UnsetAt(pHashItem);
            return;
        }
    }

    if ((HASH_VALUE_PROTOTYPE == h) && (pKey->EqualNoCase(*StringPool::GetString(SC_prototype))))
    {
        UnsetPrototype();
    }
    else if ((HASH_VALUE_PROTO == h) && (pKey->EqualNoCase(*StringPool::GetString(SC___proto__))))
    {
        Unset__Proto__();
    }
}
/*F************************************************************************************************/
/*!
    \Function    AptNativeHash::Lookup

    \Description
        Gets the value associated to the key in the hash-table.

    Insert your long description here.

    \Input      pKey              - The key to find

    \Output     AptValue *        - The value found.
                                    NULL if there is no corresponding key.

    \Notes
        None.

    \Version    1.0        09/09/04 (DC) Bringin' it to ya

*/
/************************************************************************************************F*/
AptValue *    AptNativeHash::Lookup(const AptNativeString * const pKey) const
{
    APT_ASSERT(pKey);

    AptHashItem *   pItem;

    // Calculate hash of key for the search
    uint32_t    h = pKey->UpdateHashValue();

    if (IsEmpty() == false)
    {
        pItem = HashFindKey(pKey);
        if (pItem != NULL)
        {
            return  pItem->mValue;
        }
    }

    if ((HASH_VALUE_PROTOTYPE == h) && (pKey->EqualNoCase(*StringPool::GetString(SC_prototype))))
    {
        return  mpPrototype;
    }
    else if ((HASH_VALUE_PROTO == h) && (pKey->EqualNoCase(*StringPool::GetString(SC___proto__))))
    {
        return  mp__proto__;
    }
    return  (NULL);
}

/*F************************************************************************************************/
/*!
    Erases the content of the entire hash-table.

    \Input      None.

    \Output     None.

    \Notes
        None.

    \Version
        1.0        02/27/04 (ONA) First Version

*/
/************************************************************************************************F*/
void    AptNativeHash::ClearData()
{
    if (mpPrototype != NULL)
    {
        APT_DEC(mpPrototype);
        mpPrototype = NULL;
    }
    if (mp__proto__ != NULL)
    {
        APT_DEC(mp__proto__);
        mp__proto__ = NULL;
    }

    if (mpData != NULL)
    {
        for (int i = 0; i < mnTotalSize ; i++)
        {
            if (mpData[i].Key.IsValid())
            {
                if(mpData[i].mValue)
                {
                    APT_DEC(mpData[i].mValue);
                    mpData[i].mValue = NULL;
                }
                // considering that key is not garbage collected.
                mpData[i].Key.Invalidate();
            }
        }
        APT_FREE_ARRAY_TRACKER(AptHashBlocks, mpData, AptHashItem, mnTotalSize);
        mpData = NULL;
    }
    nEventHandlers = 0;
}
/*F************************************************************************************************/
/*!
Erases the content of the entire hash-table (without deleting the Memory..)

\Input      None.

\Output     None.

\Notes
None.

\Version
1.0        09/15/04 (DC) First Version

*/
/************************************************************************************************F*/
void    AptNativeHash::ClearDataNoDelete()
{
    if (mpPrototype != NULL)
    {
        APT_DEC(mpPrototype);
        mpPrototype = NULL;
    }
    if (mp__proto__ != NULL)
    {
        APT_DEC(mp__proto__);
        mp__proto__ = NULL;
    }

    if (mpData != NULL)
    {
        for (int i = 0; i < mnTotalSize ; i++)
        {
            if (mpData[i].Key.IsValid())
            {
                if(mpData[i].mValue)
                {
                    APT_DEC(mpData[i].mValue);
                    mpData[i].mValue = NULL;
                }
                // considering that key is not garbage collected.
                mpData[i].Key.Invalidate();
            }
        }
    }
    nEventHandlers = 0;
}

/*F************************************************************************************************/
/*!
    \Function    *AptNativeHash::GetFirstItem

    \Description
        returns the first item in the Hash of NULL if empty.

    This is used in "For... each" situations i.e. call GetFirstItem and loop on
    GetNextItem until item returned is NULL.

    \Input
        None.

    \Output     AptHashItem       - First ITem or NULL

    \Notes
        There is no guarantee of order of the objects returned or even that sequential calls will
        return the objects in the same order as the previous calls. This will only guarantee that
        every item will be returned before NULL is returned.

    \Version    1.0        09/09/04 (DC) Bringin' it to ya

*/
/************************************************************************************************F*/
AptHashItem *AptNativeHash::GetFirstItem()
{
    if (IsEmpty())
        return NULL ;
    for (int i=0; i< mnTotalSize ; ++i)
    {
        APT_ASSERT(mpData);
        if (mpData[i].Key.IsValid() && (mpData[i].Key.IsEmpty() == false))
            return &mpData[i];
    }
    return NULL;
}

/*F************************************************************************************************/
/*!
    \Function    *AptNativeHash::GetNextItem

    \Description
        Returns the next item in the hash table

    See AptNativeHash::GetFirstItem description for more info.

    \Input      *pItem            -

    \Output     AptHashItem       -

    \Notes
        None.

    \Version    1.0        09/09/04 (DC) Bringin' it to ya

*/
/************************************************************************************************F*/
AptHashItem *AptNativeHash::GetNextItem(AptHashItem *pItem)
{
    if (IsEmpty())
        return NULL ;

    APT_ASSERT(mpData);
    APT_ASSERT((pItem >= &mpData[0]) && (pItem < &mpData[mnTotalSize]));

    while (++pItem < &mpData[mnTotalSize])
    {
        if (pItem->Key.IsValid() && (pItem->Key.IsEmpty() == false))
            return pItem;
    }
    return NULL;
}

/*F************************************************************************************************/
/*!
    Allocates the buffer of the hash-table.

    \Input      None.

    \Output     None.

    \Notes
        Usually this buffer is allocated just before we set the first value.

    \Version
        1.0        02/27/04 (ONA) First Version

*/
/************************************************************************************************F*/
void    AptNativeHash::FirstAllocation()
{
    APT_ASSERT(IsEmpty());
    mpData = APT_MALLOC_ARRAY_TRACKER(AptHashBlocks, AptHashItem, mnTotalSize);
    APT_ASSERT(mpData != NULL ); //&& "Allocation Returned NULL!!!!");
    //lint --e(668) PcLint warning Warning 668: Possibly passing a null pointer to function 'memset(void *, int, unsigned int)', arg. no. 1 [Reference: file C:\MyWork\Perforce\mullet\sw10\code\mainline\ion\Apt\2.06.03-ion_dev\source\Apt\AptNativeHash.cpp: line 772]
    memset(mpData, 0, mnTotalSize * sizeof(AptHashItem));
}

/*F************************************************************************************************/
/*!
    Expands the hash-table, usually after several collisions.

    \Input      None.

    \Output     None.

    \Notes
        None.

    \Version
        1.0        02/26/04 (ONA) First Version

*/
/************************************************************************************************F*/
void    AptNativeHash::Expand()
{
    #if (APT_DEBUG_LEVEL >= APT_DEBUG_MAXIMUM)
        APT_ASSERT(CheckConsistency()); // Very time consuming, only do on maximum testing.
    #endif

    //  First create a bigger AptNativeHash
    AptNativeHash       TempHash(mnTotalSize * 2);
    TempHash.FirstAllocation();             //  We will add items, don't forget to allocate ;)
    AptNativeString *   pKey;
    int32_t             i;

    //  Insert the elements in the larger hash-table
    //  There should not be a lot of collision
    //  Even if this happen, there will be another recursive expansion
    for(i = 0 ; i < mnTotalSize ; ++i)
    {
        APT_ASSERT(mpData);
        pKey = &mpData[i].Key;
        if (pKey->IsValid() == false)
        {
            //  No key to add
            continue;
        }
        if (pKey->IsEmpty())
        {
            //  There is no interest to copy erased key state as we start a new hash
            continue;
        }

        AptValue *      pValue = mpData[i].mValue;
        TempHash.HashSet(pKey, pValue);
    }

    //  We swap the buffer and the size
    AptHashItem *   pOldItems = mpData;
    mpData          = TempHash.mpData;
    TempHash.mpData = pOldItems;
    int32_t         iTemp;                     // ONA 2004/4/9: It's possible that we grow several times instead only one
    iTemp               = TempHash.mnTotalSize; //  So we take the TempHash size to be sure that we have the right size
    TempHash.mnTotalSize = mnTotalSize;
    mnTotalSize = iTemp;

    #if (APT_DEBUG_LEVEL >= APT_DEBUG_MAXIMUM)
        APT_ASSERT(CheckConsistency()); // Very time consuming, only do on maximum testing.
    #endif

    //  The temporary hash is destroyed at the end of the method, so the old buffer is released
    TempHash.DestroyGCPointers();
}

/*F************************************************************************************************/
/*!
    Checks that the hash-table is consistent with its content.

    \Input      None.

    \Output     bool              -

    \Notes
        This function is slow and should be called only for debugging purpose.
        It is only present if APT_DEBUG_LEVEL >= APT_DEBUG_MAXIMUM

    \Version
        1.0        02/26/04 (ONA) First Version

*/
/************************************************************************************************F*/
#if APT_DEBUG_LEVEL >= APT_DEBUG_MAXIMUM

bool    AptNativeHash::CheckConsistency() const
{
    if (IsEmpty())
    {
        //  Empty nothing to scan
        return  (true);
    }

    int32_t         i, j;
    AptHashItem *   pItem;
    AptHashItem *   pItem2;
    //  Check that __proto__ and prototype don't exist in the table
    if( (SlowFindKey(StringPool::GetString(SC___proto__)) != NULL) ||
        (SlowFindKey(StringPool::GetString(SC_prototype)) != NULL) )
    {
        return (false);
    }

    //  Third, we check that there is only one entry of each key
    for(i = 0 ; i < mnTotalSize ; ++i)
    {
        pItem = &mpData[i];
        if (pItem->Key.IsValid() == false)
        {
            continue;
        }
        if (pItem->Key.IsEmpty())
        {
            continue;
        }

        if (Lookup(&pItem->Key) == NULL)
        {
            //  We didn't found the key!
            return  (false);
        }

        for(j = i + 1 ; j < mnTotalSize ; ++j)
        {
            pItem2 = &mpData[j];
            if (pItem2->Key.IsValid() == false)
            {
                //  Last key for a vector
                break;
            }
            if (pItem2->Key.IsEmpty())
            {
                continue;
            }
            if (pItem2->Key.EqualNoCaseHash(pItem->Key))
            {
                //  Found the same key
                return  (false);
            }
        }
    }

    //  We could add more test like if the h probing is correct and so on

    //  No error found
    return  (true);
}
#endif

/*F************************************************************************************************/
/*!
    Sets the key / value in the internal hash-table.

    \Input      pKey              - Key to set.
    \Input      pValue            - Value to set.

    \Output     None.

    \Notes
        This is an internal method.

    \Version
        1.0        02/26/04 (ONA) First Version

*/
/************************************************************************************************F*/
void    AptNativeHash::HashSet(const AptNativeString * const pKey, AptValue * const pValue)
{
    APT_PROFILE_LOOKUP(AptNativeHash_HashSet);
    APT_ASSERT(pKey);
    APT_ASSERT(mpData);
    #if (APT_DEBUG_LEVEL >= APT_DEBUG_MAXIMUM)
        APT_ASSERT(CheckConsistency()); // Very time consuming, only do on maximum testing.
    #endif

    int         nBoundMin, nBoundMax;
    int         nFirstFit = -1;
    uint32_t    h = pKey->GetHashValue() & hashmask(mnTotalSize);

    //  First we search for the best hash-value
    if (mpData[h].Key.IsValid() == false)
    {
        //  First time we add a key here, we can add it without testing everything else
        mpData[h].Key.Validate(*pKey); //  Validate the string with the new copy
        OverwriteAt(h, pValue);
        return;
    }

    if (mpData[h].Key.IsEmpty())
    {
        //  A key was erased here, this place is the best for an insertion
        nFirstFit = h;
    }
    else
    {
        //  There is already a key, we need to check if it's the same key
        if (mpData[h].Key.EqualNoCaseHash(*pKey))
        {
            // Same key, replace key + value
            SetAt(h, pValue);
            return;
        }
    }

    //  We need now to find on the sides

    // ONA 2004/2/26: We use nSize to limit the bounds
    //  We limit the bound on the hash-table so we reduce the probability of cache-miss
    //  While giving a big range for the search
    nBoundMin = h - NUM_PROBES;
    if (nBoundMin < 0)
    {
        nBoundMin = 0;
        nBoundMax = 2 * NUM_PROBES;
        if (nBoundMax >= mnTotalSize)
        {
            nBoundMax = mnTotalSize - 1;
        }
    }
    else
    {
        nBoundMax = h + NUM_PROBES;
        if (nBoundMax > mnTotalSize - 1)
        {
            nBoundMax = mnTotalSize - 1;
            nBoundMin = nBoundMax - (2 * NUM_PROBES);
            if (nBoundMin < 0)
            {
                nBoundMin = 0;
            }
        }
    }
    APT_ASSERT(nBoundMin >= 0);
    APT_ASSERT(nBoundMax < mnTotalSize);
    APT_ASSERT(nBoundMin < nBoundMax);

    //  Now do the same research after
    int32_t i, nCurrentH;

    nCurrentH = h;
    i = nBoundMax - h;
    while(i--)
    {
        ++nCurrentH;
        if (mpData[nCurrentH].Key.IsValid() == false)
        {
            //  Even if it's not the best place we can insert the key here
            mpData[nCurrentH].Key.Validate(*pKey); //  Validate the string with the new copy
            OverwriteAt(nCurrentH, pValue);
            return;
        }
        if (mpData[nCurrentH].Key.IsEmpty())
        {
            // $bug884 - DC - want to set if it was not previously set. (logic was backwards), This cause
            // items to never back fill cleared spots in the hash (waste!!!)
            if (nFirstFit == -1)
            {
                //  Set the best fit, if not already set
                nFirstFit = nCurrentH;
            }
        }
        else
        {
            if (mpData[nCurrentH].Key.EqualNoCaseHash(*pKey))
            {
                // Same key, replace key + value
                SetAt(nCurrentH, pValue);
                return;
            }
        }
    }

    //  Still not, try before
    nCurrentH = h;
    i = h - nBoundMin;
    while(i--)
    {
        --nCurrentH;
        if (mpData[nCurrentH].Key.IsValid() == false)
        {
            //  Even if it is not the best place we can insert the key here
            mpData[nCurrentH].Key.Validate(*pKey); //  Validate the string with the new copy
            OverwriteAt(nCurrentH, pValue);
            return;
        }
        if (mpData[nCurrentH].Key.IsEmpty())
        {
            // $bug884 - DC - want to set if it was not previously set. (logic was backwards), This cause
            // items to never back fill cleared spots in the hash (waste!!!)
            if (nFirstFit == -1)
            {
                //  Set the best fit, if not already set
                nFirstFit = nCurrentH;
            }
        }
        else
        {
            if (mpData[nCurrentH].Key.EqualNoCaseHash(*pKey))
            {
                // Same key, replace key + value
                SetAt(nCurrentH, pValue);
                return;
            }
        }
    }

    //  We need to insert the key as it is not there
    #if (APT_DEBUG_LEVEL >= APT_DEBUG_MAXIMUM)
        APT_ASSERT(SlowFindKey(pKey) == NULL); // Very time consuming, only do on maximum testing.
    #endif

    if (nFirstFit == -1)
    {
        //  We didn't find enough room for the insertion, expand it
        Expand();
        //  Call recursively this set until there is enough room
        HashSet(pKey, pValue);
        return;
    }

    //  We found a place to put it
    mpData[nFirstFit].Key   = *pKey;
    OverwriteAt(nFirstFit, pValue);
}

/*F************************************************************************************************/
/*!
    Finds a key/value in the internal hash-table.

    \Input      pKey              - Key to find.

    \Output     AptHashItem *     - Returns the AptHashItem for the Key.
                                    NULL if the key is not found.

    \Notes
        This is an internal method.

    \Version
        1.0        02/26/04 (ONA) First Version
        1.1        03/05/08 (RB)  Made pKey a const * const pointer to match header file.
*/
/************************************************************************************************F*/
AptHashItem *    AptNativeHash::HashFindKey(const AptNativeString * const pKey) const
{
    APT_PROFILE_LOOKUP(AptNativeHash_HashFindKey);
    APT_ASSERT(pKey != NULL);
    APT_ASSERT(mpData);

    int         nBoundMin, nBoundMax;
    uint32_t    h = pKey->GetHashValue() & hashmask(mnTotalSize);

    //  First we search for the best hash-value
    if (mpData[h].Key.IsValid() == false)
    {
        //  There was never a key here, therefore the key we are looking for is not here
        return  (NULL);
    }

    if (mpData[h].Key.IsEmpty())
    {
        //  A key was erased here, we need to continue the search
    }
    else
    {
        //  There is already a key, we need to check if it's the same key
        if (mpData[h].Key.EqualNoCaseHash(*pKey))
        {
            //  Got it!
            return  (&mpData[h]);
        }
    }

    //  We need now to find on the sides

    // ONA 2004/2/26: We use nSize to limit the bounds
    //  We limit the bound on the hash-table so we reduce the probability of cache-miss
    //  While given a big range for the search
    nBoundMin = h - NUM_PROBES;
    if (nBoundMin < 0)
    {
        nBoundMin = 0;
        nBoundMax = 2 * NUM_PROBES;
        if (nBoundMax >= mnTotalSize)
        {
            nBoundMax = mnTotalSize - 1;
        }
    }
    else
    {
        nBoundMax = h + NUM_PROBES;
        if (nBoundMax > mnTotalSize - 1)
        {
            nBoundMax = mnTotalSize - 1;
            nBoundMin = nBoundMax - (2 * NUM_PROBES);
            if (nBoundMin < 0)
            {
                nBoundMin = 0;
            }
        }
    }
    APT_ASSERT(nBoundMin >= 0);
    APT_ASSERT(nBoundMax < mnTotalSize);
    APT_ASSERT(nBoundMin < nBoundMax);

    //  Now do the same research after
    int32_t i, nCurrentH;

    nCurrentH = h;
    i = nBoundMax - h;
    while(i--)
    {
        ++nCurrentH;
        if (mpData[nCurrentH].Key.IsValid() == false)
        {
            //  Never a key here, don't need to continue
            return  (NULL);
        }
        if (mpData[nCurrentH].Key.IsEmpty())
        {
            //  Need to continue
        }
        else
        {
            if (mpData[nCurrentH].Key.EqualNoCaseHash(*pKey))
            {
                return  (&mpData[nCurrentH]);
            }
        }
    }

    //  Still not, try before
    nCurrentH = h;
    i = h - nBoundMin;
    while(i--)
    {
        --nCurrentH;
        if (mpData[nCurrentH].Key.IsValid() == false)
        {
            //  Never a key here, don't need to continue
            return  (NULL);
        }
        if (mpData[nCurrentH].Key.IsEmpty())
        {
            //  Need to continue
        }
        else
        {
            if (mpData[nCurrentH].Key.EqualNoCaseHash(*pKey))
            {
                return  (&mpData[nCurrentH]);
            }
        }
    }

    return  (NULL);
}

/*F************************************************************************************************/
/*!
    Search a key without using the hash value.

    \Input      pKey              - The key to find.

    \Output     AptHashItem *     - The AptHashItem pointer.
                                    NULL if key is not found.

    \Notes
        This is an internal method. This method should be used only for debugging purpose.
        This is only present for APT_DEBUG_LEVEL >= APT_DEBUG_MAXIMUM

    \Version
        1.0        02/26/04 (ONA) First Version

*/
/************************************************************************************************F*/
#if APT_DEBUG_LEVEL >= APT_DEBUG_MAXIMUM
AptHashItem *    AptNativeHash::SlowFindKey(const AptNativeString * const pKey) const
{
    int32_t         i;
    AptHashItem *   pItem;
    for(i = 0 ; i < mnTotalSize ; ++i)
    {
        pItem = &mpData[i];
        if (pItem->Key.IsValid() == false)
        {
            continue;
        }
        if (pItem->Key.IsEmpty())
        {
            continue;
        }
        if (pItem->Key.EqualNoCaseHash(*pKey))
        {
            //  We found the item
            return  (pItem);
        }
    }
    return  (NULL);
}
#endif

/*F************************************************************************************************/
/*!
    \Function    AptNativeHash::UpdateObjectMethods

    \Description
        Updates the event flags for a given AptValue

    If any events are added or removed from this AptValue, we update the event flags here

    \Input      *pContext         -
    \Input      pVar              -
    \Input      bRemove           -

    \Output
        None.

    \Notes
        None.

    \Version    1.0        10/14/04 (LEO) First Version

*/
/************************************************************************************************F*/
void AptNativeHash::UpdateObjectMethods(AptValue *pContext, const AptNativeString * pVar, int bRemove)
{
    if(pContext->isCIH()) return;

    SpriteMembers *pProp = SpriteMembersIndex::in_word_set(pVar->c_str(), pVar->Size());
    if (pProp)
    {
        if (pProp->nIndex < 200) return;
        int nTableIndex = pProp->nIndex - AptSpriteMethod_onData;
        APT_ASSERT(nTableIndex >= 0 && nTableIndex < APT_ARRAYSIZE(aSpriteGperfToActionFlag));
        int nFlag = aSpriteGperfToActionFlag[nTableIndex];
        //APT_ASSERT(nFlag != -1);
        if(nFlag == -1)
        {
            return;
        }
        if (bRemove)
        {
            RemoveEventHandler(nFlag);
        }
        else
        {
            SetEventHandler(nFlag);
        }
    }
}

/*F************************************************************************************************/
/*!
    \Function    AptNativeHash::RegisterReferences

    \Description
        Registers all of the references using the APT_REGISTER_REFERENCE macro. This provides
        a flexible method of using this to do a number of interesting tasks....

    Insert your long description here.

    \Input
        None.

    \Output
        None.

    \Notes
        None.

    \Version    1.0        09/09/04 (DC) Bringin' it to ya
                2.0        09/09/04 (LEO) Bringin it back with style yo!

*/
/************************************************************************************************F*/
#if defined(APT_XML_MEMORY_DUMP_SUPPORTED)
void AptNativeHash::RegisterReferences(const AptValue * pFromRef, char * pFromRefC)
{
    APT_REGISTER_REFERENCE_FROM_SAFE(pFromRef, mp__proto__, *StringPool::GetString(SC___proto__), APT_REFREG_IS_APTVALUE);
    APT_REGISTER_REFERENCE_FROM_SAFE(pFromRef, mpPrototype, *StringPool::GetString(SC_prototype), APT_REFREG_IS_APTVALUE);

    if (IsEmpty())
    {
        return;
    }

    for(int i = 0; i < mnTotalSize; i++)
    {
        APT_ASSERT(mpData);
        if(mpData[i].mValue != NULL)
        {
            AptNativeString sTemp = pFromRefC + mpData[i].Key;
            APT_REGISTER_REFERENCE_FROM_SAFE(pFromRef,  mpData[i].mValue, sTemp.c_str(), APT_REFREG_IS_APTVALUE);

        }
    }

    return;
}
#endif
void AptNativeHash::RegisterReferences(const AptValue * pFromRef)
{
    APT_REGISTER_REFERENCE_FROM_SAFE(pFromRef, mp__proto__, *StringPool::GetString(SC___proto__), APT_REFREG_IS_APTVALUE);
    APT_REGISTER_REFERENCE_FROM_SAFE(pFromRef, mpPrototype, *StringPool::GetString(SC_prototype), APT_REFREG_IS_APTVALUE);

    if (IsEmpty())
    {
        return;
    }

    for(int i = 0; i < mnTotalSize; i++)
    {
        APT_ASSERT(mpData);
        APT_REGISTER_REFERENCE_FROM_SAFE(pFromRef,  mpData[i].mValue, mpData[i].Key.c_str(), APT_REFREG_IS_APTVALUE);
    }

    return;
}

/*H*************************************************************************************************
    EOF
*************************************************************************************************H*/
