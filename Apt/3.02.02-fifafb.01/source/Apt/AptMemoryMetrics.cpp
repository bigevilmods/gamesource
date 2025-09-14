/*H************************************************************************************************/
/*!

    \File    AptMemoryMetrics.cpp

    \Description
        This file contains implementation of AptMemoryMetrics class.

    \Notes
        None.

    \Copyright
        (c) 2008 Electronic Arts Inc.

    \Version    1.0        09/13/05 (ASK) First Version
    \Version    1.1        07/22/08 (RB)  EATech 114985: Added AptHighwaterMemoryMetrics

*/
/************************************************************************************************H*/


/*** Include files ********************************************************************************/

#include "Apt.h"
#include "AptDefine.h"
#include "_Apt.h"       // needed for APT_ATOMIC macros.
#include "AptMemoryMetricsImpl.h"
#include "AptAnimationTarget.h"

/*** Defines **************************************************************************************/

/*** Macros ***************************************************************************************/

/*** Type Definitions *****************************************************************************/

/// This is the default highwater memory metrics overflow callback.
/// All it does is return the 'default policy' to Apt, which typically means Apt will assert on an overflow.
/// See AptMemoryMetrics.h for more info.
class AptHighwaterMemoryMetrics_DefaultOverflowCallback : public IAptHighwaterMemoryOverflowCallback
{
public:
    int32_t OnOverflowDetected(const AptHighwaterMemoryMetrics& metrics, const void*, const void* )
    {
        return metrics.mDefaultPolicy;
    }
};

/*** Function Prototypes **************************************************************************/

/*** Variables ************************************************************************************/

// Private variables

#if defined (APT_GATHER_MEMORY_METRICS)

static AptMemoryMetrics      gAptMemoryMetrics("Apt", NULL);

AptMemoryMetrics            *AptMemoryMetrics::spHead = NULL;
IAptMemoryLimitCallback     *AptMemoryMetrics::spCallback = NULL;
bool                         AptMemoryMetrics::sInitialized = false;

#endif // APT_GATHER_MEMORY_METRICS

/// Array of highwater memory metrics
static AptHighwaterMemoryMetrics sAptHighwaterMemMetrics[ (int32_t)AptHighwaterMemoryMetrics::TYPE_MAX ];

// set the default overflow callback
static AptHighwaterMemoryMetrics_DefaultOverflowCallback defaultOverflowCallbackSingleton;
IAptHighwaterMemoryOverflowCallback* AptHighwaterMemoryMetrics::sOverflowCallback = &defaultOverflowCallbackSingleton;

/*** Private Functions ****************************************************************************/

/*** Public Functions *****************************************************************************/

//
// AptHighwaterMemoryMetrics methods
//

/******************************************************************************/
/*! AptHighwaterMemoryMetricsImpl :: InitializeItem

    \brief  Initializes one element of sAptHighwaterMemMetrics.

    \param  type                  - type of metric
    \param  initialMaxCount       - initial value of mInitialMaxCount
    \param  estimatedBytesPerItem - initial value of mEstimatedBytesPerItem
    \param  helpfulHint           - initial value of mHelpfulHint
    \param  defaultPolicy         - initial value of mDefaultPolicy
    \return                       - void.

    Called from the method AptHighwaterMemoryMetricsImpl::Initialize below.
*/
/******************************************************************************/
void AptHighwaterMemoryMetricsImpl::InitializeItem(AptHighwaterMemoryMetrics::Type type, int32_t initialMaxCount, int32_t estimatedBytesPerItem, const char * helpfulHint, int32_t defaultPolicy)
{
    AptHighwaterMemoryMetrics* pMetrics = NULL;
    pMetrics = GetMetrics( type );
    pMetrics->mType = type;
    pMetrics->mInitialMaxCount = (int32_t) initialMaxCount;
    pMetrics->mHighwaterMaxCount = 0;
    pMetrics->mEstimatedBytesPerItem = estimatedBytesPerItem;
    pMetrics->mHelpfulHint = helpfulHint;
    pMetrics->mDefaultPolicy = defaultPolicy;
}

/******************************************************************************/
/*! AptHighwaterMemoryMetricsImpl :: Initialize

    \brief  Sets up sAptHighwaterMemMetrics, a static array of metrics data

    \param          - none.
    \return         - void.

    Gets called by AptAllocatorInitialize
*/
/******************************************************************************/
void AptHighwaterMemoryMetricsImpl::Initialize()
{
    // Some of the sizes passed to InitializeItem below will be different depending
    // on which #defines are set.
    int32_t nNumListenerSets = 1;
#if defined APT_USE_MOUSE
    ++nNumListenerSets;
#endif
#if defined APT_ALTERNATE_INPUT
    ++nNumListenerSets;
#endif

    static const int32_t SIZEOF_APTALTINPUT =
#if defined(APT_ALTERNATE_INPUT)
        sizeof(AptAltInput);
#else
        0;
#endif

    // Some helpful hints passed to sOverflowCallback
    static const char HINT_BUTTONSET[] = "Please increase AptInitParmsT::iButtonSetSize, or reduce number of Button objects that receive input during a frame";
    static const char HINT_INPUTSET[] = "Please increase AptInitParmsT::iInputSetSize, or reduce number of controller input events during a frame";
    static const char HINT_LISTENERSET[] = "Please increase AptInitParmsT::iListenerSetSize, or reduce number of key/mouse listeners";
    static const char HINT_ACTIONPOOL[] = "Please increase AptInitParmsT::iActionPoolSize";
    static const char HINT_INTERVALS[] = "Please increase AptInitParmsT::iMaxIntervalFunctions, or reduce number of interval timers, otherwise setInterval will return 'undefined'";
    static const char HINT_BUTTON_INSTANCES[] = "Please increase AptInitParmsT::iMaxButtonInstances, or reduce the number of new buttons created during a frame";
    static const char HINT_MOVIECLIP_INSTANCES[] = "Please increase AptInitParmsT::iMaxNewMovieClipsPerFrame, or reduce the number of new movieclips created during a frame";
    static const char HINT_QUEUED_INPUTS[] = "Please increase AptInitParmsT::iMaxQueuedInputs, or reduce number of controller input events during a frame";
    static const char HINT_DATASTACK_DEPTH[] = "Please increase AptInitParmsT::iStackSize"; // highwater only tracked in debug; overflow always called; will assert
    static const char HINT_CALLSTACK_DEPTH[] = "Please increase AptInitParmsT::iCallStackDepth"; // highwater only tracked in debug; overflow always called; will assert
    static const char HINT_REGISTERARRAY[] = "Please increase AptInitParmsT::iRegArraySize";
    static const char HINT_LOOKUPARRAY[] = "Please increase AptInitParmsT::iLookupArraySize";
    static const char HINT_ZOMBIEVECTOR[] = "Please increase AptInitParmsT::iZombieVectorSize";
    static const char HINT_FREECUSTCTRLS[] = "In AptRenderableCustomControl.h, MAX_FREE_CUSTCTRLS has been exceeded, and either the render thread is blocked, or too many custom controls have been created";
    static const char HINT_INTERVAL_PARAMS[] = "In AptAnimationTarget.h, please set a higher value for MAX_INTERVAL_FUNC_PARAMS";

    // get default Apt init params
    const AptInitParmsT& initParms = gAptInitParms;

    // initialize the static array sAptHighwaterMemMetrics
    InitializeItem( TYPE_BUTTONSET, (int32_t) initParms.iButtonSetSize, (int32_t) sizeof(AptCIH *), HINT_BUTTONSET );
    InitializeItem( TYPE_INPUTSET, (int32_t) initParms.iInputSetSize, (int32_t) sizeof(AptCIH*), HINT_INPUTSET );
    InitializeItem( TYPE_LISTENERSET, (int32_t) initParms.iListenerSetSize, nNumListenerSets * (int32_t) sizeof(AptValue*), HINT_LISTENERSET );
    InitializeItem( TYPE_ACTIONPOOL, (int32_t) initParms.iActionPoolSize, (int32_t) sizeof(AptActionQueueC::AptActionPool), HINT_ACTIONPOOL );
    InitializeItem( TYPE_INTERVALS, (int32_t) initParms.iMaxIntervalFunctions, (int32_t) sizeof(AptIntervalTimer), HINT_INTERVALS );
    InitializeItem( TYPE_BUTTON_INSTANCES, (int32_t) initParms.iMaxButtonInstances, (int32_t) sizeof(ButtonHitTestRecord), HINT_BUTTON_INSTANCES );
    InitializeItem( TYPE_MOVIECLIP_INSTANCES, (int32_t) initParms.iMaxNewMovieClipsPerFrame, 2 * (int32_t) sizeof(AptCIH*), HINT_MOVIECLIP_INSTANCES );
    InitializeItem( TYPE_QUEUED_INPUTS, (int32_t) initParms.iMaxQueuedInputs, (int32_t) (SIZEOF_APTALTINPUT + sizeof(AptInput)), HINT_QUEUED_INPUTS );
    InitializeItem( TYPE_DATASTACK_DEPTH, (int32_t) initParms.iStackSize, (int32_t) sizeof(AptValue*), HINT_DATASTACK_DEPTH );
    InitializeItem( TYPE_CALLSTACK_DEPTH, (int32_t) initParms.iCallStackDepth, 4 * (int32_t) sizeof(AptValue*), HINT_CALLSTACK_DEPTH );
    InitializeItem( TYPE_REGISTERARRAY, (int32_t) initParms.iRegArraySize, (int32_t) sizeof(AptValue*), HINT_REGISTERARRAY );
    InitializeItem( TYPE_LOOKUPARRAY, (int32_t) initParms.iLookupArraySize, (int32_t) sizeof(AptLookup), HINT_LOOKUPARRAY );
    InitializeItem( TYPE_ZOMBIEVECTOR, (int32_t) initParms.iZombieVectorSize, (int32_t) sizeof(AptValue*), HINT_ZOMBIEVECTOR );

    InitializeItem( TYPE_FREECUSTCTRLS, AptRenderableCustomControl::MAX_FREE_CUSTCTRLS, (int32_t) sizeof(void*), HINT_FREECUSTCTRLS );
    InitializeItem( TYPE_INTERVAL_PARAMS, AptIntervalTimer::MAX_INTERVAL_FUNC_PARAMS, (int32_t) sizeof(AptValue*), HINT_INTERVAL_PARAMS );
}


/******************************************************************************/
/*! AptHighwaterMemoryMetricsImpl :: GetMetrics

    \brief  Gets metrics

    \param  type    - type to get
    \return         - Pointer (non-const) to metrics record

    Only called by Apt internally
*/
/******************************************************************************/
AptHighwaterMemoryMetrics * AptHighwaterMemoryMetricsImpl::GetMetrics(AptHighwaterMemoryMetrics::Type type)
{
    return &sAptHighwaterMemMetrics[ (int32_t)type ];
}


/******************************************************************************/
/*! AptHighwaterMemoryMetrics :: GetMetrics

    \brief  Gets metrics.

    \param  type    - type to get
    \return         - const pointer to metrics record

    Called by user from outside of Apt
*/
/******************************************************************************/
const AptHighwaterMemoryMetrics * AptHighwaterMemoryMetrics::GetMetrics(AptHighwaterMemoryMetrics::Type type)
{
    return &sAptHighwaterMemMetrics[ (int32_t)type ];
}


/******************************************************************************/
/*! AptHighwaterMemoryMetrics :: CompareAptInitParams

    \brief  Compares user's current Apt Init Params with the gathered highwater values

    \param  userParams     - User's Apt Init Params that were passed to AptInitialize
    \param  returnedParams - New, suggested Apt Init Params. NOTE: Not guaranteed to be optimal!
    \return                - true if params need to change; false if everythings OK for now

    Convenience function. Called by user from outside of Apt.
*/
/******************************************************************************/
bool AptHighwaterMemoryMetrics::CompareAptInitParams(const AptInitParmsT* userParams, AptInitParmsT * returnedParams)
{
    if ( !userParams )
        userParams = &gAptInitParms;
    *returnedParams = *userParams;
    // NOTE: the amount by which we increase the values below are just a best-guess, not a guarantee
    if ( sAptHighwaterMemMetrics[ TYPE_BUTTONSET ].mHighwaterMaxCount >= returnedParams->iButtonSetSize )
        returnedParams->iButtonSetSize += 16;
    if ( sAptHighwaterMemMetrics[ TYPE_INPUTSET ].mHighwaterMaxCount >= returnedParams->iInputSetSize )
        returnedParams->iInputSetSize += 16;
    if ( sAptHighwaterMemMetrics[ TYPE_LISTENERSET ].mHighwaterMaxCount >= returnedParams->iListenerSetSize )
        returnedParams->iListenerSetSize += 16;
    if ( sAptHighwaterMemMetrics[ TYPE_ACTIONPOOL ].mHighwaterMaxCount >= returnedParams->iActionPoolSize )
        returnedParams->iActionPoolSize += 32;
    if ( sAptHighwaterMemMetrics[ TYPE_INTERVALS ].mHighwaterMaxCount >= returnedParams->iMaxIntervalFunctions )
        returnedParams->iMaxIntervalFunctions += 8;
    if ( sAptHighwaterMemMetrics[ TYPE_BUTTON_INSTANCES ].mHighwaterMaxCount >= returnedParams->iMaxButtonInstances )
        returnedParams->iMaxButtonInstances += 16;
    if ( sAptHighwaterMemMetrics[ TYPE_MOVIECLIP_INSTANCES ].mHighwaterMaxCount >= returnedParams->iMaxNewMovieClipsPerFrame )
        returnedParams->iMaxNewMovieClipsPerFrame += 64;
    if ( sAptHighwaterMemMetrics[ TYPE_QUEUED_INPUTS ].mHighwaterMaxCount >= returnedParams->iMaxQueuedInputs )
        returnedParams->iMaxQueuedInputs += 8;
    if ( sAptHighwaterMemMetrics[ TYPE_DATASTACK_DEPTH ].mHighwaterMaxCount >= returnedParams->iStackSize )
        returnedParams->iStackSize += 32;
    if ( sAptHighwaterMemMetrics[ TYPE_CALLSTACK_DEPTH ].mHighwaterMaxCount >= returnedParams->iCallStackDepth )
        returnedParams->iCallStackDepth += 32;
    if ( sAptHighwaterMemMetrics[ TYPE_REGISTERARRAY ].mHighwaterMaxCount >= returnedParams->iRegArraySize )
        returnedParams->iRegArraySize += 16;
    if ( sAptHighwaterMemMetrics[ TYPE_LOOKUPARRAY ].mHighwaterMaxCount >= returnedParams->iLookupArraySize )
        returnedParams->iLookupArraySize += 16;
    if ( sAptHighwaterMemMetrics[ TYPE_ZOMBIEVECTOR ].mHighwaterMaxCount >= returnedParams->iZombieVectorSize )
        returnedParams->iZombieVectorSize += 8;

    for (int32_t i=0; i<(int32_t)AptHighwaterMemoryMetrics::TYPE_MAX; ++i )
    {
        if ( sAptHighwaterMemMetrics[i].mHighwaterMaxCount >= sAptHighwaterMemMetrics[i].mInitialMaxCount )
        {
            return true;
        }
    }
    return false;
}


/******************************************************************************/
/*! AptHighwaterMemoryMetrics :: GetTypeAsString

    \brief  Converts Type enum to a string

    \param  type           - Type to convert
    \return                - friendly string name
*/
/******************************************************************************/
const char* AptHighwaterMemoryMetrics::GetTypeAsString( AptHighwaterMemoryMetrics::Type type )
{
    static const char BUTTONSET[] = "BUTTONSET";
    static const char INPUTSET[] = "INPUTSET";
    static const char LISTENERSET[] = "LISTENERSET";
    static const char ACTIONPOOL[] = "ACTIONPOOL";
    static const char INTERVALS[] = "INTERVALS";
    static const char BUTTON_INSTANCES[] = "BUTTON_INSTANCES";
    static const char MOVIECLIP_INSTANCES[] = "MOVIECLIP_INSTANCES";
    static const char QUEUED_INPUTS[] = "QUEUED_INPUTS";
    static const char DATASTACK_DEPTH[] = "DATASTACK_DEPTH";
    static const char CALLSTACK_DEPTH[] = "CALLSTACK_DEPTH";
    static const char REGISTERARRAY[] = "REGISTERARRAY";
    static const char LOOKUPARRAY[] = "LOOKUPARRAY";
    static const char ZOMBIEVECTOR[] = "ZOMBIEVECTOR";
    static const char FREECUSTCTRLS[] = "FREECUSTCTRLS";
    static const char INTERVAL_PARAMS[] = "INTERVAL_PARAMS";


    const char * szName = NULL;
    switch(type)
    {
        case AptHighwaterMemoryMetrics::TYPE_BUTTONSET: szName = BUTTONSET; break;
        case AptHighwaterMemoryMetrics::TYPE_INPUTSET: szName = INPUTSET; break;
        case AptHighwaterMemoryMetrics::TYPE_LISTENERSET: szName = LISTENERSET; break;
        case AptHighwaterMemoryMetrics::TYPE_ACTIONPOOL: szName = ACTIONPOOL; break;
        case AptHighwaterMemoryMetrics::TYPE_INTERVALS: szName = INTERVALS; break;
        case AptHighwaterMemoryMetrics::TYPE_BUTTON_INSTANCES: szName = BUTTON_INSTANCES; break;
        case AptHighwaterMemoryMetrics::TYPE_MOVIECLIP_INSTANCES: szName = MOVIECLIP_INSTANCES; break;
        case AptHighwaterMemoryMetrics::TYPE_QUEUED_INPUTS: szName = QUEUED_INPUTS; break;
        case AptHighwaterMemoryMetrics::TYPE_DATASTACK_DEPTH: szName = DATASTACK_DEPTH; break;
        case AptHighwaterMemoryMetrics::TYPE_CALLSTACK_DEPTH: szName = CALLSTACK_DEPTH; break;
        case AptHighwaterMemoryMetrics::TYPE_REGISTERARRAY: szName = REGISTERARRAY; break;
        case AptHighwaterMemoryMetrics::TYPE_LOOKUPARRAY: szName = LOOKUPARRAY; break;
        case AptHighwaterMemoryMetrics::TYPE_ZOMBIEVECTOR: szName = ZOMBIEVECTOR; break;
        case AptHighwaterMemoryMetrics::TYPE_FREECUSTCTRLS: szName = FREECUSTCTRLS; break;
        case AptHighwaterMemoryMetrics::TYPE_INTERVAL_PARAMS: szName = INTERVAL_PARAMS; break;
        default: break;
   }
    return szName;
}


//
// The legacy memory metrics tracking code for Apt class-specific new/delete operators
//

#if defined (APT_GATHER_MEMORY_METRICS)

//! Gets the first AptClassMemoryMetrics instance. Useful for walking and printing (or looking for something funky)
void AptMemoryMetrics::ResetAll()
{
    // Reset the chain (but don't disturb the chain).
    AptMemoryMetrics *pTemp = GetFirstInstance();
    while(pTemp)
    {
        pTemp->Reset();
        pTemp = GetSiblingInstance(pTemp);
    }
}

//! Gets the first AptClassMemoryMetrics instance. Useful for walking and printing (or looking for something funcky)
AptMemoryMetrics *AptMemoryMetrics::GetFirstInstance()
{
    return spHead;
}
//! Gets the next AptClassMemoryMetrics instance
AptMemoryMetrics *AptMemoryMetrics::GetSiblingInstance(AptMemoryMetrics *pCurrent)
{
    return pCurrent->mpNextSibling;
}

//! Gets the next AptClassMemoryMetrics instance
AptMemoryMetrics *AptMemoryMetrics::GetChildInstance(AptMemoryMetrics *pCurrent)
{
    return pCurrent->mpFirstChild;
}

//! Sets the callback for memory limit callback. It's functions are called when the configured limits are exceeded
void                   AptMemoryMetrics::SetLimitCallback( const IAptMemoryLimitCallback * pCallback)
{
#if (EA_PLATFORM_PTR_SIZE == 4)
	APT_WRITE32(spCallback, pCallback);
#else
    APT_WRITE_PTR(spCallback, pCallback);
#endif
}

//! Sets the callback for memory limit callback. It's functions are called when the configured limits are exceeded
void                   AptMemoryMetrics::Initialize()
{
    if(!sInitialized)
    {
        gAptMemoryMetrics.mpParent = NULL;
        AptMemoryMetrics *pTemp = GetFirstInstance();
        while(pTemp)
        {
            AptMemoryMetrics * pParent = pTemp->mpParent;
            AptMemoryMetrics * pNext = GetSiblingInstance(pTemp);

            if(pTemp != &gAptMemoryMetrics)
            {
                APT_ASSERT(pParent != NULL);
                pTemp->mpNextSibling  = pParent->mpFirstChild;
                pParent->mpFirstChild = pTemp;
            }

            pTemp = pNext;
        }
        gAptMemoryMetrics.mpNextSibling = NULL;
        spHead = &gAptMemoryMetrics;
        sInitialized = true;
    }
}

//-----------------------------------------------------------------------------------------------------------------
// Instance functions.

//! Get the total number of instances of this class allocated since AptInitialize
void    AptMemoryMetrics::Reset()
{
    // Reset the counters and limits, but don't mess with the class name of pNext.
    APT_WRITE32(muTotalAllocationCount,          0);
    APT_WRITE32(muCurrentAllocationCount,        0);
    APT_WRITE32(muBytesCurrentlyAllocated,       0);
    APT_WRITE32(muMaxBytesAllocatedAtAnyOneTime, 0);
    APT_WRITE32(muMemoryLimit,                   0xFFFFFFFF);
    if(mpNextSibling)
        mpNextSibling->Reset();
    if(mpFirstChild)
        mpFirstChild->Reset();
}
//! Get the total number of instances of this class allocated since AptInitialize
uint32_t    AptMemoryMetrics::GetTotalAllocationCount()
{
    return APT_READ32(muTotalAllocationCount);
}
//! Get the Number of items currently allocated
uint32_t    AptMemoryMetrics::GetCurrentAllocationCount()
{
    return APT_READ32(muCurrentAllocationCount);
}
//! Get the number of bytes this class currently takes up in memory
size_t      AptMemoryMetrics::GetBytesCurrentlyAllocated()
{
    return APT_READ32(muBytesCurrentlyAllocated);
}
//! Get the maximum count of these objects allocated at any one instant.
size_t      AptMemoryMetrics::GetMaxBytesAllocatedAtAnyOneTime()
{
    return APT_READ32(muMaxBytesAllocatedAtAnyOneTime);
}
//! Get the name of the class this metric is for
const char *AptMemoryMetrics::GetClassName()
{
    return mszName;
}
//! Calls IAptMemoryLimitCallback::ClassMemoryLimitExceeded() when the passed limit is exceeded.
void        AptMemoryMetrics::SetClassMemoryLimit(uint32_t uMaxBytes)
{
    APT_WRITE32(muMemoryLimit, uMaxBytes);
}
//! Constructs the object and inserts it into the static list of all metric objects.
AptMemoryMetrics::AptMemoryMetrics(const char * szClassName, AptMemoryMetrics * pParent):
muTotalAllocationCount(0),
muCurrentAllocationCount(0),
muBytesCurrentlyAllocated(0),
muMaxBytesAllocatedAtAnyOneTime(0),
muMemoryLimit(0xFFFFFFFF),
mszName(szClassName),
mpParent(pParent),
mpNextSibling(NULL),
mpFirstChild(NULL)
{
    APT_THREADHELPER_LOCK(gMemoryMetricsSpinLock);

    if(pParent == NULL)
    {
        mpParent = &gAptMemoryMetrics;
    }

    APT_ASSERT(mpParent != NULL);

    // This ensures only one thread gets in, basically only the one that actually sets the value initializes.
    AptMemoryMetrics * pHead = (AptMemoryMetrics *)APT_READ32(spHead);
    if(pHead == NULL)
    {
#if (EA_PLATFORM_PTR_SIZE == 4)
        APT_WRITE32(spHead, this);
#else
		APT_WRITE_PTR(spHead, this);
#endif
    }
    else
    {
#if (EA_PLATFORM_PTR_SIZE == 4)
        APT_WRITE32(mpNextSibling, pHead);
        APT_WRITE32(spHead, this);
#else
		APT_WRITE_PTR(mpNextSibling, pHead);
		APT_WRITE_PTR(spHead, this);
#endif
    }
    APT_THREADHELPER_UNLOCK(gMemoryMetricsSpinLock);
}

//! modifies internal members
void AptMemoryMetrics::OnMemoryAllocated(size_t nObjectSize)
{
    APT_INC32(muTotalAllocationCount);
    APT_INC32(muCurrentAllocationCount);

    uint32_t newCurrent = APT_ADD32(muBytesCurrentlyAllocated,        nObjectSize);
    uint32_t maxCurrent = APT_READ32(muMaxBytesAllocatedAtAnyOneTime);
    if(newCurrent > maxCurrent)
    {
        // Only exchange if it hasn't been updated since we read it. (prevent cross thread clobbers)
        // We take on a small risk of having a slightly wrong number here, but it will correct itself over time.
        APT_CONDITIONAL_EXCHANGE32(muMaxBytesAllocatedAtAnyOneTime,  maxCurrent, newCurrent);

        APT_THREADHELPER_LOCK(gMemoryMetricsSpinLock);
        // No need to check for limit exceeding unless we break our present MAx :)
        if(newCurrent > APT_READ32(muMemoryLimit))
        {
            spCallback->ClassMemoryLimitExceeded(this);
        }
        APT_THREADHELPER_UNLOCK(gMemoryMetricsSpinLock);
    }

    if(mpParent)
    {
        mpParent->OnMemoryAllocated(nObjectSize);
    }
}

//! modifies internal members
void AptMemoryMetrics::OnMemoryDeallocated(size_t nObjectSize)
{
    if(mpParent)
    {
        mpParent->OnMemoryDeallocated(nObjectSize);
    }
    APT_DEC32(muCurrentAllocationCount);
    APT_SUB32(muBytesCurrentlyAllocated,        nObjectSize);
}
#endif // #if defined(APT_GATHER_MEMORY_METRICS)
