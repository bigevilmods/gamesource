/*H************************************************************************************************/
/*!

    \File
        AptRegister.cpp

    \Description
        Simple description of main purpose of file.

    \Notes
        None.

    \Copyright
        (c) 2004 Electronic Arts Inc.

    \Version
        1.0        03/06/04 (ONA) First Version
        1.1        09/06/05 (ASK) Added code to have global AptRegister. Saves lot of memory.
        1.2        07/29/08 (RB)  Call AptHighwaterMemoryMetrics overflow callback in Create()
*/
/************************************************************************************************H*/

#include "AptRegister.h"

// psp-sn release build was giving compiler errors. so added next line. 0.17.00
#include "MainInline.h"
#include "_Apt.h"       // for gAptInitParms
#include "AptMemoryMetricsImpl.h"       // for gAptInitParms

APT_DEFINE_OBJECT_METRICS(AptRegister, AptValueNoGC);

AptRegister *  AptRegister::s_RegisterArray = NULL ;

int32_t      AptRegister::s_nMaxRegisters = 0 ;

/*F************************************************************************************************/
/*!
    \Function    AptRegister::Initialize

    \Description
        Sets up the global AptRegisters

    \Input
        None.

    \Output
        None.

    \Notes
        None.

    \Version    1.0        09/06/05 (ASK) First Version

*/
/************************************************************************************************F*/
void AptRegister::Initialize()
{
    s_nMaxRegisters = gAptInitParms.iRegArraySize ;
    if (s_RegisterArray == NULL)
    {
        // we need to allocate some AptRegister objects and initialize them
        s_RegisterArray = new AptRegister[s_nMaxRegisters];
        for (int32_t i = 0 ; i < s_nMaxRegisters; i ++)
        {
            s_RegisterArray[i].nVal = i ;
        }
    }
}

/*F************************************************************************************************/
/*!
    \Function    AptRegister::Shutdown

    \Description
        Delete the global AptRegisters

    \Input
        None.

    \Output
        None.

    \Notes
        None.

    \Version    1.0        09/06/05 (ASK) First Version

*/
/************************************************************************************************F*/
void AptRegister::Shutdown()
{
    if (s_RegisterArray != NULL)
    {
        // we need to first delete all AptRegisters and then delete array
#if (APT_DEBUG_LEVEL >= APT_DEBUG_NORMAL)
        for (int32_t i = 0; i < s_nMaxRegisters; i ++)
        {
            s_RegisterArray[i].SetDestroyedGC();
        }
#endif
        delete [] s_RegisterArray ;
        s_RegisterArray = NULL ;

    }
}


/*F************************************************************************************************/
/*!
    \Function    AptRegister::Create

    \Description
        Create a AptRegister object

    \Input      nVal              - register value

    \Output     AptRegister *       - return value

    \Notes
        Returns from the global array.

    \Version    1.0        09/06/05 (ASK) First Version

*/
/************************************************************************************************F*/
AptRegister * AptRegister::Create(int32_t nVal)
{
    if (s_RegisterArray != NULL)
    {
        if (nVal < s_nMaxRegisters)
        {
            return (&s_RegisterArray[nVal]) ;
        }
        else
        {
            AptHighwaterMemoryMetrics* pMetrics  = AptHighwaterMemoryMetricsImpl::GetMetrics(AptHighwaterMemoryMetrics::TYPE_REGISTERARRAY);
            pMetrics->mHighwaterMaxCount = INT32_MAX;
            int32_t nPolicy = AptHighwaterMemoryMetrics::sOverflowCallback->OnOverflowDetected( *pMetrics, s_RegisterArray, &nVal );
            if ( nPolicy & (int32_t)AptHighwaterMemoryMetrics::POLICY_APT_WILL_ASSERT )
            {
               APT_ASSERTM( (nVal < s_nMaxRegisters), pMetrics->mHelpfulHint );
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

