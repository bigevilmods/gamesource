#ifndef __aptset_h_
#define __aptset_h_

#include "AptMemoryMetrics.h"

template <class T>
class AptValueSet
{
public:
    uint16_t mnElements;
    uint16_t mnMaxElements;
    uint16_t mnHighwaterNumElements;
    T *maElements;
    AptHighwaterMemoryMetrics* mpMemMetrics;

    AptValueSet(int nMaxElements, AptHighwaterMemoryMetrics* pMetrics)
    :   mnElements(0),
        mnMaxElements( static_cast<uint16_t>(nMaxElements) ),
        mnHighwaterNumElements(0),
        maElements(NULL),
        mpMemMetrics(pMetrics)
    {
        APT_ASSERT(nMaxElements <= 0xffff);
        if(mnMaxElements > 0)
        {
            maElements = APT_MALLOC_ARRAY(T, mnMaxElements);
            clear();
        }
    }

    ~AptValueSet()
    {
        int nCount = mnElements;
        for (int i = 0; i < mnMaxElements; i++)
        {
            if (maElements[i])
            {
                APT_DEC(maElements[i]);
                nCount--;
                if (nCount == 0) break;
            }
        }
        if(maElements != 0)
            APT_FREE_ARRAY(maElements, T, mnMaxElements);
    }

    void clear() { mnElements = 0; mnHighwaterNumElements = 0; memset(maElements, 0, sizeof(T)*mnMaxElements); }

    int capacity() { return mnHighwaterNumElements; }

    void add(T element)
    {
        APT_ASSERT(mnElements < mnMaxElements);
        int i;
        // don't add the same thing 2x
#if defined(EA_DEBUG)
        for (i= 0; i<mnMaxElements; i++)
        {
            APT_ASSERT(maElements[i] != element);
        }
#endif // EA_DEBUG
        mnElements++;
        if(mnElements >= mnHighwaterNumElements)
        {
            mnHighwaterNumElements = mnElements+1;
        }

#if defined(EA_DEBUG)
        if ( mpMemMetrics && ( ((int32_t)mnElements) > mpMemMetrics->mHighwaterMaxCount ) )
        {
            mpMemMetrics->mHighwaterMaxCount = (int32_t)mnElements;
        }
#endif // EA_DEBUG

        // Find an empty spot
        for (i = mnElements; maElements[i] != NULL; i++)
        {
            // changed i = 0 to i = -1. Because in case where 0 th element is empty and we are
            // looping thru, then we are omitting 0 the element.
            if(i >= mnMaxElements)
            {
                i = -1;

                // rather than potentially looping forever, let's check for an overflow
                if ( (mnElements >= mnMaxElements) && mpMemMetrics)
                {
#if !defined(EA_DEBUG)
                    mpMemMetrics->mHighwaterMaxCount = INT32_MAX;
#endif // EA_DEBUG
                    int32_t nPolicy = AptHighwaterMemoryMetrics::sOverflowCallback->OnOverflowDetected( *mpMemMetrics, maElements, element );
                    if ( nPolicy & (int32_t)AptHighwaterMemoryMetrics::POLICY_APT_WILL_ASSERT )
                    {
                        APT_ASSERTM( (mnElements < mnMaxElements), mpMemMetrics->mHelpfulHint );
                    }
                    return; // bail out
                }
            }
        }
        maElements[i] = element;
        APT_INC(element);
    }

    int remove(T element)
    {
        //$fr826 - MCB - If there isn't anything in the array, don't search, return now.
        if (mnElements == 0)
        {
            return 0;
        }

        APT_ASSERT(mnElements < mnMaxElements);
        int i;
        for (i = 0; (i < mnMaxElements) && (maElements[i] != element); i++)
        {
        }

        // Make sure we found the dude
        if (i < mnMaxElements)
        {
            mnElements--;
            APT_DEC(maElements[i]);
            maElements[i] = 0;
            return 1;
        }

        return 0;
    }

    bool has(T element)
    {
        for (int i= 0; i<mnMaxElements; i++)
        {
            if(maElements[i] == element)
                return true;
        }
        return false;
    }
};

#endif
