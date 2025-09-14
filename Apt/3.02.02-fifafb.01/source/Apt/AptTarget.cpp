/*H************************************************************************************************/
/*!

    \File    Apt\AptTarget.cpp

    \Description
        Simple description of main purpose of file.

    \Notes
        None.

    \Copyright
        (c) 2005 Electronic Arts Inc.

    \Version    1.0        10/07/05 (LEO) First Version
    \Version    2.1        04/03/08 ($senzee) Using AptUpdateAutoLock (defined in _Apt.h)
                                    instead of directly locking the mutex/futex for cleanliness
                                    and to avoid requiring mutex/futex headers and other details.
    \Version    2.2        09/14/08 ($senzee) Call AptRenderList::Stop() when last AptTarget is
                                    destroyed.
    \Version    2.3        07/07/09 (RB)  Added lock in AptDestroyTargetInstance, in case it is
                                          called from non-sim thread.
    \Version    2.4        09/22/09 (MZ)  Modified AptTargetAddAlternateInput() prototype.

*/
/************************************************************************************************H*/


/*** Include files ********************************************************************************/
#include "_Apt.h"
#include "AptTarget.h"
#include "AptAnimationTarget.h"
#include "AptBCRenderTree.h"
#include "MainInline.h"
#include "AptRenderList.h"

/*** Defines **************************************************************************************/

/*** Macros ***************************************************************************************/

/*** Type Definitions *****************************************************************************/

/*** Function Prototypes **************************************************************************/

/*** Variables ************************************************************************************/

// Private variables
// Public variables
//! These are global variables used to maintain the apt targets
AptTarget *gpDefaultTarget          = NULL;
AptTarget *gpCurrentTargetSim       = NULL;
AptTarget *gpCurrentTargetRender    = NULL;
uint32_t nTargetsCreated            = 0;

/*** Private Functions ****************************************************************************/

/*** Public Functions *****************************************************************************/
//////////////////////////////////////////////////////////////////////////
// AptTarget Implementation
/*F************************************************************************************************/
/*!
    \Function    AptTarget::AptTarget

    \Description
        Initializes everyting for this target

    \Input      *aptInitParms     -

    \Output     A                 -

    \Notes
        None.

    \Version    1.0        09/01/05 (LEO) First Version

*/
/************************************************************************************************F*/
AptTarget::AptTarget(const AptInitParmsT *aptInitParms)
{
    AptUpdateAutoLock lock;

    mInitParms.CopyAptInitParms(aptInitParms);
    mpPool   = new AptAnimationTarget(&mInitParms);
    mpLoader = new AptLoader();
    mpLinker = new AptLinker();
    mpRTMgr  = new AptBCRenderTreeManager();

    mpNext = NULL;
    mpPrev = NULL;
}

/*F************************************************************************************************/
/*!
    \Function    AptTarget::AptTarget

    \Description
        Initializes everything for this target

    \Input      *aptInitParms     -

    \Output     A                 -

    \Notes
        None.

    \Version    1.0        09/01/05 (LEO) First Version

*/
/************************************************************************************************F*/
AptTarget::AptTarget(const AptTargetInitParams *aptInitParms)
{
    memcpy(&mInitParms, aptInitParms, sizeof(AptTargetInitParams));
    mpPool   = new AptAnimationTarget(&mInitParms);
    mpLoader = new AptLoader();
    mpLinker = new AptLinker();
    mpRTMgr  = new AptBCRenderTreeManager();

    mpNext = NULL;
    mpPrev = NULL;
}

/*F************************************************************************************************/
/*!
    \Function    AptTarget::~AptTarget

    \Description
        Deconstructor

    \Input
        None.

    \Output     A                 -

    \Notes
        None.

    \Version    1.0        09/01/05 (LEO) First Version

*/
/************************************************************************************************F*/
AptTarget::~AptTarget()
{
    Shutdown();
    mpPool = NULL;
    mpLoader = NULL;
    mpLinker = NULL;
    mpNext = NULL;
    mpPrev = NULL;
    mpRTMgr = NULL;
}

/*F************************************************************************************************/
/*!
    \Function    AptTarget::Shutdown

    \Description
        Shutsdown the target

    \Input
        None.

    \Output
        None.

    \Notes
        None.

    \Version    1.0        09/01/05 (LEO) First Version

*/
/************************************************************************************************F*/
void AptTarget::Shutdown()
{
    AptUpdateAutoLock lock;

    AptRenderList::Stop();

    APT_SETUP_SIM_TARGET_STATES(this);
    APT_SETUP_RENDER_TARGET_STATES(this);

    bool bPrevState = gAptActionInterpreter.bShutDown;
    gAptActionInterpreter.bShutDown = true;

    if(mpPool != NULL)
    {
        mpPool->PreDestroy();       // Do this before deleting the mpLoader and mpLinker, then delete the mpPool after
        mpPool->CleanRemList();
    }

    // XXX this is actually not good, as there can be zombies across targets. But putting it in to fix
    // shutdown crashes in rw4.5 ps2 viewers which first delete target and then later call AptShutdown.
    // so here we make sure we force delete all zombies. we probably have to have separate zombie vectors
    // for every target.
    AptUpdateZombieVector(true) ;

    if (mpRTMgr != NULL)
    {
        // We are in the correct thread or we are shutting down, go ahead and clean the render manager.
        mpRTMgr->Render_Shutdown();
        delete mpRTMgr;
        mpRTMgr = NULL;
    }

    if(mpLoader != NULL)
    {
        delete mpLoader;
        mpLoader = NULL;
    }
    if(mpLinker != NULL)
    {
        delete mpLinker;
         mpLinker = NULL;
    }
    if(mpPool != NULL)
    {
        delete mpPool;
        mpPool = NULL;
    }

    gAptActionInterpreter.bShutDown = bPrevState;

    AptRenderList::Stop();
    APT_TEARDOWN_RENDER_TARGET_STATES;
    APT_TEARDOWN_SIM_TARGET_STATES;
}
/*F************************************************************************************************/
/*!
    \Function    AptTarget::Reset

    \Description
        Shuts down then resets the state

    \Input
        None.

    \Output
        None.

    \Notes
        None.

    \Version    1.0        09/01/05 (LEO) First Version

*/
/************************************************************************************************F*/
void AptTarget::Reset()
{
    Shutdown();
    AptRenderList::Restart();

    mpRTMgr  = new AptBCRenderTreeManager();

    mpPool   = new AptAnimationTarget(&mInitParms);
    mpLoader = new AptLoader();
    mpLinker = new AptLinker();
}
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// AptTarget API functions
/*F************************************************************************************************/
/*!
    \Function    *GetTarget

    \Description
        Returns the currently set target based on thread id

    \Input
        None.

    \Output     AptTarget         -

    \Notes
        None.

    \Version    1.0        10/07/05 (LEO) First Version

*/
/************************************************************************************************F*/
AptTarget *GetTarget()
{
    return gpCurrentTargetSim;
}

/*F************************************************************************************************/
/*!
    \Function    AptCreateTargetInstance

    \Description
        Creates a AptTarget and returns it as a AptTargetInstance

    \Input      *aptInitParms     -

    \Output     AptTargetInstance  -

    \Notes
        None.

    \Version    1.0        09/01/05 (LEO) First Version

*/
/************************************************************************************************F*/
AptTargetInstance AptCreateTargetInstance(const AptTargetInitParams *aptInitParms)
{
    AptUpdateAutoLock lock;

    nTargetsCreated++;
    AptTarget *pNew = new AptTarget(aptInitParms);
    AptTarget *pTmp = gpDefaultTarget;

    if(pTmp != NULL)
    {
        while(pTmp->GetNext() != NULL)
            pTmp = pTmp->GetNext();
        pTmp->SetNext(pNew);
        pNew->SetPrevious(pTmp);
    }
    else
    {
        gpDefaultTarget    = pNew; // Set as default for now
        // we have to set these in order to pre-load filters definition swf file
        gpCurrentTargetSim = pNew;
        gpCurrentTargetRender = pNew;

        AptPreloadFilterSWFFile();
    }
    return (AptTargetInstance)(pNew);
}

/*F************************************************************************************************/
/*!
    \Function    AptChangeTargetInstance

    \Description
        Changes the current sim and render targets to the one passed in

    \Input      Target            -

    \Output
        None.

    \Notes
        None.

    \Version    1.0        09/01/05 (LEO) First Version

*/
/************************************************************************************************F*/
void AptChangeTargetInstance(AptTargetInstance Target)
{
    APT_ASSERT((AptTarget *)Target != NULL);
    gpCurrentTargetSim    = (AptTarget *)Target;
    gpCurrentTargetRender = (AptTarget *)Target;
}


/*F************************************************************************************************/
/*!
    \Function    AptChangeSimTargetInstance

    \Description
        Changes the current sim target to the one passed in

    \Input      Target            -

    \Output
        None.

    \Notes
        None.

    \Version    1.0        09/01/05 (LEO) First Version

*/
/************************************************************************************************F*/
void AptChangeSimTargetInstance(AptTargetInstance Target)
{
    APT_ASSERT((AptTarget *)Target != NULL);
    gpCurrentTargetSim      = (AptTarget *)Target;
}


/*F************************************************************************************************/
/*!
    \Function    AptChangeRenderTargetInstance

    \Description
        Changes the current render target to the one passed in

    \Input      Target            -

    \Output
        None.

    \Notes
        None.

    \Version    1.0        09/01/05 (LEO) First Version

*/
/************************************************************************************************F*/
void AptChangeRenderTargetInstance(AptTargetInstance Target)
{
    APT_ASSERT((AptTarget *)Target != NULL);
    gpCurrentTargetRender   = (AptTarget *)Target;
}


/*F************************************************************************************************/
/*!
    \Function    AptDestroyTargetInstance

    \Description
        Destroys an instance of a AptTarget

    \Input      Target            -

    \Output
        None.

    \Notes
        None.

    \Version    1.0        09/01/05 (LEO) First Version
    \Version    1.1        01/03/07 (JP)  Updates global pointers if needed
    \Version    1.2        07/07/09 (RB)  Put lock around "delete pTmp"

*/
/************************************************************************************************F*/
void AptDestroyTargetInstance(AptTargetInstance Target)
{
    nTargetsCreated--;
    AptTarget *pTmp = (AptTarget *)Target;
    AptTarget *pNext = pTmp->GetNext();
    AptTarget *pPrev = pTmp->GetPrevious();

    if(pTmp == gpCurrentTargetSim)
    {
        if (!pNext)
        {
            AptRenderList::Stop();
        }
        gpCurrentTargetSim = pNext;
    }
    if(pTmp == gpCurrentTargetRender)
    {
        gpCurrentTargetRender = pNext;
    }
    if(pTmp == gpDefaultTarget)
    {
        gpDefaultTarget = pNext;
    }
    if(pPrev != NULL)
    {
        pPrev->SetNext(pNext);
    }
    if(pNext != NULL)
    {
        pNext->SetPrevious(pPrev);
    }

    {
        // protect Dogma pools in case we're being called from non-sim thread.
        AptUpdateAutoLock lock;
        delete pTmp;
}
}

/*F************************************************************************************************/
/*!
    \Function    AptResetTargetInstance

    \Description
        Resets the target instance

    \Input      Target            -

    \Output
        None.

    \Notes
        None.

    \Version    1.0        09/01/05 (LEO) First Version

*/
/************************************************************************************************F*/
void AptResetTargetInstance(AptTargetInstance Target)
{
    APT_SETUP_SIM_TARGET_STATES(Target);
    ((AptTarget *)Target)->Reset();
    APT_TEARDOWN_SIM_TARGET_STATES;
}

/*F************************************************************************************************/
/*!
    \Function    AptSetDefaultTargetInstance

    \Description
        Sets the current target to the defualt

    \Input
        None.

    \Output
        None.

    \Notes
        None.

    \Version    1.0        09/01/05 (LEO) First Version

*/
/************************************************************************************************F*/
void AptSetDefaultTargetInstance()
{
    gpCurrentTargetSim    = gpDefaultTarget;
    gpCurrentTargetRender = gpDefaultTarget;
}
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//Core Apt AptTarget API functions
void AptTargetCompleteAnimationAsyncLoad(AptTargetInstance Target, AptFilePtr file, void *pData, void *pConstTable, void *pUserData)
{
    APT_SETUP_SIM_TARGET_STATES(Target);
    AptCompleteAnimationAsyncLoad(file, pData, pConstTable, pUserData);
    APT_TEARDOWN_SIM_TARGET_STATES;
}

void AptTargetCompleteImageAsyncLoad(AptTargetInstance Target, AptFilePtr file, AptAssetTexture texture, int width, int height, void* userData)
{
    APT_SETUP_SIM_TARGET_STATES(Target);
    AptCompleteImageAsyncLoad(file, texture, width, height, userData);
    APT_TEARDOWN_SIM_TARGET_STATES;
}

void AptTargetGetAnimationSize(AptTargetInstance Target, int *pnWidth, int *pnHeight)
{
    APT_SETUP_SIM_TARGET_STATES(Target);
    AptGetAnimationSize(pnWidth, pnHeight);
    APT_TEARDOWN_SIM_TARGET_STATES;
}

void AptTargetLoadAnimation(AptTargetInstance Target, const char *szBaseName, const char *szTarget)
{
    APT_SETUP_SIM_TARGET_STATES(Target);
    AptLoadAnimation(szBaseName, szTarget);
    APT_TEARDOWN_SIM_TARGET_STATES;
}

AptFilePtr AptTargetPreloadAnimation(AptTargetInstance Target, const char *szBaseName)
{
    APT_SETUP_SIM_TARGET_STATES(Target);
    AptFilePtr pTmp = AptPreloadAnimation(szBaseName);
    APT_TEARDOWN_SIM_TARGET_STATES;
    return pTmp;
}

void AptTargetCancelPreloadedAnimation(AptTargetInstance Target, const char *szBaseName)
{
    APT_SETUP_SIM_TARGET_STATES(Target);
    AptCancelPreloadedAnimation(szBaseName);
    APT_TEARDOWN_SIM_TARGET_STATES;
}

bool AptTargetIsFileLoaded(AptTargetInstance Target, AptFilePtr file)
{
    APT_SETUP_SIM_TARGET_STATES(Target);
    bool bRet = AptIsFileLoaded(file);
    APT_TEARDOWN_SIM_TARGET_STATES;
    return bRet;
}

void AptUpdateTarget(AptTargetInstance Target, unsigned int nDeltaTimeMS, AptAnimLevelE eAnimLevels)
{
    APT_SETUP_SIM_TARGET_STATES(Target);
    AptUpdate(nDeltaTimeMS, eAnimLevels);
    APT_TEARDOWN_SIM_TARGET_STATES;
}

void AptUpdateRenderTarget(AptTargetInstance Target, AptAnimLevelE eAnimLevels)
{
	APT_SETUP_SIM_TARGET_STATES(Target);
	AptUpdateRender(eAnimLevels);
	APT_TEARDOWN_SIM_TARGET_STATES;
}

void AptRenderTarget(AptTargetInstance Target, uint32_t nDeltaTimeMS, AptAnimLevelE eAnimLevels)
{
    APT_SETUP_RENDER_TARGET_STATES(Target);
    AptRender(nDeltaTimeMS, eAnimLevels);
    APT_TEARDOWN_RENDER_TARGET_STATES;
}

void AptTargetSetMousePosition(AptTargetInstance Target, int nX, int nY)
{
    APT_SETUP_SIM_TARGET_STATES(Target);
    AptSetMousePosition(nX, nY);
    APT_TEARDOWN_SIM_TARGET_STATES;
}

void AptTargetAddToInputQueue(AptTargetInstance Target, AptInputType eInput, AptInputState eState, AptInputController eController)
{
    APT_SETUP_SIM_TARGET_STATES(Target);
    AptAddToInputQueue(eInput, eState, eController);
    APT_TEARDOWN_SIM_TARGET_STATES;
}

void AptTargetAddToInputAnalogQueue(AptTargetInstance Target, AptAnalogStickInfo pAnalogInput)
{
    APT_SETUP_SIM_TARGET_STATES(Target);
    AptAddToInputAnalogQueue(pAnalogInput);
    APT_TEARDOWN_SIM_TARGET_STATES;
}

void AptTargetAddToInputGestureQueue(AptTargetInstance Target, AptGestureInfo pGestureInput)
{
    APT_SETUP_SIM_TARGET_STATES(Target);
    AptAddToInputGestureQueue(pGestureInput);
    APT_TEARDOWN_SIM_TARGET_STATES;
}

#if defined(APT_ALTERNATE_INPUT)
void AptTargetAddAlternateInput(AptTargetInstance Target, const char *szEvent, AptValue *pValue)
{
    APT_SETUP_SIM_TARGET_STATES(Target);
    AptAddAlternateInput(szEvent, pValue);
    APT_TEARDOWN_SIM_TARGET_STATES;
}
#endif

void AptTargetSetInternalVariable(AptTargetInstance Target, const char *szName, const char *szValue)
{
    APT_SETUP_SIM_TARGET_STATES(Target);
    AptSetInternalVariable(szName, szValue);
    APT_TEARDOWN_SIM_TARGET_STATES;
}

void AptTargetGetInternalVariable(AptTargetInstance Target, const char *szName, char *szValue)
{
    APT_SETUP_SIM_TARGET_STATES(Target);
    AptGetInternalVariable(szName, szValue);
    APT_TEARDOWN_SIM_TARGET_STATES;
}

void AptTargetCallFunction(AptTargetInstance Target,const char *szName, char *szReturnValue, const char *szThisObject, int nNumParams, ...)
{
    APT_SETUP_SIM_TARGET_STATES(Target);

    va_list varargs;
    va_start( varargs, nNumParams );
    AptCallFunctionV(szName, szReturnValue, szThisObject, nNumParams, varargs);
    va_end(varargs);
    APT_TEARDOWN_SIM_TARGET_STATES;
}

void AptTargetCallFunctionOpti(AptTargetInstance Target,const char *szName, char *szReturnValue, const char *szThisObject, int nNumParams, ...)
{
    APT_SETUP_SIM_TARGET_STATES(Target);

    va_list varargs;
    va_start( varargs, nNumParams );
    AptCallFunctionVOpti(szName, szReturnValue, szThisObject, nNumParams, varargs);
    va_end(varargs);
    APT_TEARDOWN_SIM_TARGET_STATES;
}

void AptTargetCallFunctionV(AptTargetInstance Target,const char *szName, char *szReturnValue, const char *szThisObject, int nNumParams, va_list &varargs)
{
    APT_SETUP_SIM_TARGET_STATES(Target);

    AptCallFunctionV(szName, szReturnValue, szThisObject, nNumParams, varargs);
    APT_TEARDOWN_SIM_TARGET_STATES;
}

void AptTargetCallFunctionVOpti(AptTargetInstance Target,const char *szName, char *szReturnValue, const char *szThisObject, int nNumParams, va_list &varargs)
{
    APT_SETUP_SIM_TARGET_STATES(Target);

    AptCallFunctionVOpti(szName, szReturnValue, szThisObject, nNumParams, varargs);
    APT_TEARDOWN_SIM_TARGET_STATES;
}
