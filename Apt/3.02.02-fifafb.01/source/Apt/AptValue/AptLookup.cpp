/*H************************************************************************************************/
/*!

    \File
        AptLookup.cpp

    \Description
        Simple description of main purpose of file.

    \Notes
        None.

    \Copyright
        (c) 2004 Electronic Arts Inc.

    \Version
        1.0        03/06/04 (ONA) First Version
        1.1        09/06/05 (ASK) Added code to have global AptLookups. Saves lot of memory.
        1.2        07/29/08 (RB)  Call AptHighwaterMemoryMetrics overflow callback in Create()
*/
/************************************************************************************************H*/

#include "AptLookup.h"

// psp-sn release build was giving compiler errors. so added next line. 0.17.00
#include "MainInline.h"
#include "_Apt.h"       // for gAptInitParms
#include "AptMemoryMetricsImpl.h"

APT_DEFINE_OBJECT_METRICS(AptLookup, AptValueNoGC);

AptLookup * AptLookup::s_LookupArray = NULL ;

int32_t      AptLookup::s_nMaxLookups = 0 ;


/*F************************************************************************************************/
/*!
\Function    AptLookup::Initialize

\Description
Sets up the global true false AptBooleans

\Input
None.

\Output
None.

\Notes
None.

\Version    1.0        09/06/05 (ASK) First Version

*/
/************************************************************************************************F*/
void AptLookup::Initialize()
{
    s_nMaxLookups = gAptInitParms.iLookupArraySize ;
    if (s_LookupArray == NULL)
    {
        // we need to allocate some AptLookup objects and initialize them
        s_LookupArray = new AptLookup[s_nMaxLookups];
        for (int32_t i = 0 ; i < s_nMaxLookups; i ++)
        {
            s_LookupArray[i].nLookup = i ;
        }
    }
}

/*F************************************************************************************************/
/*!
\Function    AptLookup::Shutdown

\Description
Delete the global true false AptBooleans

\Input
None.

\Output
None.

\Notes
None.

\Version    1.0        09/06/05 (ASK) First Version

*/
/************************************************************************************************F*/
void AptLookup::Shutdown()
{
    if (s_LookupArray != NULL)
    {
        // we need to first delete all Lookups and then delete array
#if (APT_DEBUG_LEVEL >= APT_DEBUG_NORMAL)
        for (int32_t i = 0; i < s_nMaxLookups; i ++)
        {
            s_LookupArray[i].SetDestroyedGC();
        }
#endif
        delete [] s_LookupArray ;
        s_LookupArray = NULL ;
    }
}


/*F************************************************************************************************/
/*!
    \Function    AptLookup::Create

    \Description
        Create a AptLookup object

    \Input      nVal              - lookup value

    \Output     AptLookup *       - return value

    \Notes
        Returns from the global array.

    \Version    1.0        09/06/05 (ASK) First Version

*/
/************************************************************************************************F*/
AptLookup * AptLookup::Create(int32_t nVal)
{
    if (s_LookupArray != NULL)
    {
        APT_ASSERT(nVal < s_nMaxLookups && "Apt- Please increase the iLookupArraySize member in AptInitParmsT while calling initializing Apt") ;
        if (nVal < s_nMaxLookups)
        {
            return (&s_LookupArray[nVal]) ;
        }
        else
        {
            AptHighwaterMemoryMetrics* pMetrics  = AptHighwaterMemoryMetricsImpl::GetMetrics(AptHighwaterMemoryMetrics::TYPE_LOOKUPARRAY);
            pMetrics->mHighwaterMaxCount = INT32_MAX;
            int32_t nPolicy = AptHighwaterMemoryMetrics::sOverflowCallback->OnOverflowDetected( *pMetrics, s_LookupArray, &nVal );
            if ( nPolicy & (int32_t)AptHighwaterMemoryMetrics::POLICY_APT_WILL_ASSERT )
            {
               APT_ASSERTM( (nVal < s_nMaxLookups), pMetrics->mHelpfulHint );
            }
            return NULL ;
        }
    }
    else
    {
        return NULL ;
    }
}

/*H*************************************************************************************************
    EOF
*************************************************************************************************H*/

