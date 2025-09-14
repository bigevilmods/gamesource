 /*!
\addtogroup g_referenceguide        Reference Guides
*/
//@{
/*!
\Module AptCore     Apt Core Library
*/
//@}


/*H************************************************************************************************/
/*!

    \File    Apt.cpp

    \Description
        This is the main Apt file that has implementation for all the Apt API
        functions that can be called from outside.

    \Notes
        None.

    \Copyright
        (c) 2005 Electronic Arts Inc.

    \Version    1.0        09/14/04 (ASK) First Version
    \Version    1.1        11/27/06 (ASK) Changed links to coreforge for external references to Apt Wiki page
    \Version    1.2        03/12/07 (RB)  Added pfnProfiler... callbacks.
    \Version    1.3        09/06/07 (RB)  Added sFreeCustomCtrlQueue and AptFreeOutstandingCustomControls.
    \Version    1.4        09/28/07 ($senzee) (fixes for lockless Apt crashes)
                                    Added high level locking to several API calls for the
                                    SIM thread only, to increase reliability when games do things
                                    like switching the thread that Apt SIM runs on, etc.  These functions
                                    are not intended to run simultaneously anyway, the mutex now guarantees
                                    it.  Previously, we used the APT_ASSERT_IS_SIMULATION_THREAD(), which of course,
                                    was turned off in release mode.  Because these SIM-side API funcs were never
                                    intended to run simultaneously, there should be no lock contention in the
                                    proper case and therefore little performance hit, if any.
                                    $TODO Because of these changes, we may be able to change the atomic int
                                    refcount in AptFile back to a regular int and remove AptLoader's MutexAptLoader.
    \Version    1.5        01/31/08 (RB) Removed unused symbols AptHasRenderData and gbInputGremlinsEnabled.
                                         Made functions without prototypes static. Don't mix int and int32_t for _AptGetAnimationAtLevel param.
    \Version    1.6        03/05/08 (RB) Removed unused symbols in _playbackSavedInputs.
    \Version    1.7        03/18/08 $senzee Added AptUpdateLock(), AptUpdateUnlock() functions to allow the sim thread to be
                                    locked.  This is necessary for the safety of external-facing APT_INC/APT_DEC macros.
                                    Also change the gAptSimThreadMutex to a futex, providing faster execution in
                                    the expected case of little contention.  $todo Change the name of gAptSimThreadMutex to
                                    gAptSimThreadLock or something that does not try to describe its implementation method.
    \Version    1.8        04/02/08 (RB)  Replaced APT_AUX_MAX_FILES_LOADED with APT_MAX_FILES_LOADED from _AptInternalDefines.h.
                                          Removed APT_SET_SIMULATION/RENDER_THREAD because they do nothing.
    \Version    1.9        04/07/08 (RB)  Handle instantiable AptExtObjects.
    \version    1.9.1      05/15/08 (ASK) Added pfnAssertFailMsg callback to AptUserFunctions.
    \Version    1.10       06/12/08 (JP)  Adding support for the MovieClipLoader class
    \version    1.11       06/13/08 (RB)  Made AptSetActionScriptProfilerThresholdMS param a float, for sub-millisecond accuracy.
    \Version    1.12       07/29/08 (RB)  Changed gpFreeTextureArray to SafeQueueFixed.
                                          Added ability to track AptHighwaterMemoryMetrics for
                                          free-texture and free-custctrl queues.
    \Version    1.13       08/14/08 ($senzee) Added support for optional AptUtil class.
    \Version    1.14       04/13/09 (RB)  On shutdown, print out number of overflow pools created.
    \Version    1.15       04/20/09 (RB)  Replaced usage of APT_DEBUGGER_CALL1 with if defined(APT_DEBUGGER_ENABLED)
    \Version    1.16       07/07/09 (RB)  Replaced sim-thread lock/unlock pairs with AptUpdateAutoLock. Removed lock from AptGetSizeOfAptValue.
    \Version    1.17       09/22/09 (MZ)  Modified AptAddAlternateInput() prototype.
    \Version    1.18       11/10/09 (RB)  Fixed bug so that saved-input files can be played when APT_USE_FILTERS==0.

*/
/************************************************************************************************H*/

/*** Include files ********************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "_Apt.h"    // DC -- Include after stdio and stdlib.
#include "AptAnimationTarget.h"
#include "Display/AptDisplayListState.h"
#include "Display/AptDisplayList.h"
#include "AptFastStack.h"
#include "AptStd/AptMath.h"
#include "AptValue/AptValueVector.h"
#include "AptObject/AptStringObject.h"
#include "AptObject/AptDate.h"
#include "AptObject/AptExternalFunction.h"
#include "AptObject/AptMathObj.h"
#include "AptObject/AptKey.h"
#include "AptObject/AptLoadVars.h"
#include "AptObject/AptSound.h"
#include "AptObject/AptTextFormat.h"
#include "AptObject/AptMouse.h"
#include "AptObject/AptError.h"
#include "AptObject/AptStage.h"
#include "AptObject/AptScriptColour.h"
#include "AptObject/AptMovieClip.h"
#include "AptObject/AptMovieClipLoader.h"
#include "AptObject/AptUtil.h"
#include "AptObject/AptXmlNode.h"
#include "AptObject/AptXml.h"
#include "AptObject/AptXmlAttributes.h"
#include "AptObject/AptGlobalObject.h"
#include "AptObject/AptGlobalExtensionObject.h"
#include "AptExtObject.h"
#include "AptFrameStack.h"
#include "MainInline.h"
#include "AptRenderItem.h"
#include "AptBCRenderTree.h"
#include "AptDebugReport.h"
#include "AptTarget.h"
#include "AptRenderList.h"
#include "AptSafeQueueFixed.h"
#include "AptRenderableCustomControl.h"
#include "AptProfile.h"
#include "SDK/AptExtObjectRegistry.h"
#include "AptMemoryMetricsImpl.h"
#include "AptDebugger/AptDebugger.h"

/*** Defines **************************************************************************************/
#if defined(EA_SYSTEM_BIG_ENDIAN)
    #define APT_SAVEINPUTS_RECORD_SIZE_SAVER       8
#else
    #define APT_SAVEINPUTS_RECORD_SIZE_SAVER       5
#endif

/*** Macros ***************************************************************************************/

/*** Type Definitions *****************************************************************************/


/*** Function Prototypes **************************************************************************/
void AptInitializeGC() ;
void CleanAllNativeFunctions();

void AptUpdateShutdown(int bQuiet) ;
void AptCommonShutdown() ;
void AptRenderShutdown() ;

void AptUpdateInitialize(const AptInitParmsT *pAptInitParms) ;
void AptCommonInitialize(const AptInitParmsT *pAptInitParms) ;
void AptRenderInitialize() ;


/*** Variables ************************************************************************************/
APT_DEFINE_CUSTOM_METRICS(AptAnonymousAllocations, NULL);
APT_DEFINE_CUSTOM_METRICS(AptMiscObjects, NULL);

APT_DEFINE_OBJECT_METRICS_EXPLICIT(AptTarget, NULL);

APT_DEFINE_OBJECT_METRICS_EXPLICIT(AptIntervalTimer, &AptMiscObjects);
APT_DEFINE_OBJECT_METRICS_EXPLICIT(TextFormat, &AptMiscObjects);
APT_DEFINE_OBJECT_METRICS_EXPLICIT(AptActionQueueC::AptActionPool, &AptMiscObjects);
APT_DEFINE_OBJECT_METRICS_EXPLICIT(AptActionQueueC, &AptMiscObjects);
APT_DEFINE_OBJECT_METRICS_EXPLICIT(AptAnimationTarget, NULL);
APT_DEFINE_OBJECT_METRICS_EXPLICIT(AptMatrix, NULL);
APT_DEFINE_OBJECT_METRICS_EXPLICIT(AptCXForm, NULL);
APT_DEFINE_OBJECT_METRICS_EXPLICIT(AptProfileSample, NULL);
APT_DEFINE_OBJECT_METRICS(AptMovieClip, AptObject);
#if defined (APT_USE_STAGE_OBJECT)
APT_DEFINE_OBJECT_METRICS(AptStage, AptObject);
#endif
APT_DEFINE_OBJECT_METRICS(AptStringObject, AptObject);
#if defined (APT_USE_DATE_OBJECT)
APT_DEFINE_OBJECT_METRICS(AptDate, AptObject);
#endif
#if defined(APT_USE_SOUND_OBJECT)
APT_DEFINE_OBJECT_METRICS(AptSound, AptObject);
#endif
#if APT_USE_MATH_OBJECT
APT_DEFINE_OBJECT_METRICS(AptMathObj, AptObject);
#endif
APT_DEFINE_OBJECT_METRICS(AptKey, AptObject);
#if defined(APT_USE_MOUSE)
APT_DEFINE_OBJECT_METRICS(AptMouse, AptObject);
#endif
#if APT_USE_LOADVARS_OBJECT
APT_DEFINE_OBJECT_METRICS(AptLoadVars, AptObject);
#endif
#if defined APT_USE_BUTTONS
APT_DEFINE_OBJECT_METRICS_EXPLICIT(ButtonHitTestRecord, &AptMiscObjects);
#endif
#if APT_USE_UTILITY
APT_DEFINE_OBJECT_METRICS(AptUtil, AptObject);
#endif
#if defined(APT_ALTERNATE_INPUT)
APT_DEFINE_OBJECT_METRICS(AptAlternateInput, AptObject);
#endif
APT_DEFINE_OBJECT_METRICS(AptError,  AptObject);
APT_DEFINE_OBJECT_METRICS(AptNone,   AptValueNoGC);
APT_DEFINE_OBJECT_METRICS(AptExtern, AptObject);

#include APT_INC_THREAD_H
#include APT_INC_THREAD_MUTEX_H
#include APT_INC_THREAD_FUTEX_H

// Private variables

APT_THREAD_NAMESPACE::Futex gAptSimThreadMutex;

#ifndef APT_SINGLE_THREADED
APT_THREAD_NAMESPACE::ThreadId gAptSimThreadId = APT_THREAD_NAMESPACE::kThreadIdInvalid;
#endif

// We want to ensure custom control zIDs are destroyed on the render thread.
// So we queue up the cust ctrl zID instead of freeing it immediately when its
// lifetime has expired.  Later, in the render thread, we dequeue and free the item.
static AptSafeQueueFixed    sFreeCustomCtrlQueue;
APT_THREAD_NAMESPACE::Futex gAptFreeCustCtrlMutex;

// Public variables

static int          gbUpdateInitialized = 0 ;
static int          gbRenderInitialized = 0;
static int          gbCommonInitialized = 0 ;

AptUserFunctions    gAptFuncs;
AptInput            gNullInput = 0;
int                 gbSavedInputsEnabled = 0;
unsigned int        gnCurTick;
bool                gbGarbageCollectThisFrame = false;
AptValueVector *    gpValuesToRelease = NULL;
AptNativeString     gstrTempString;
bool                gbBackgroundColorSet; // bg color can only be set once.

int32_t            gnCurrUpdateTick = 0 ; // DC - This is similar to gnCurTick, but we need to be able to mess with it to frame skip.
unsigned int       gnCurrRenderTick = 0 ;

// we can save the incoming parameters to AptInitialize in this as we might need them later for AptReset
AptInitParmsT       gAptInitParms;

// DC - 11/01/04 Added Global Pool Variables.
DOGMA_PoolManager       * gpNonGCPoolManager;
AptValueGC_PoolManager  * gpGCPoolManager;
AptValueGC_PoolManager* GetGCPoolManager()
{
    return gpGCPoolManager;
}

DOGMA_PoolManager* GetNonGCPoolManager()
{
    return gpNonGCPoolManager;
}

#if defined(APT_ENABLE_DEBUG_REPORT)
EA::Apt::Debug::AptReport* gpDebugReport;
#endif

void PrintAptDebugReport(int flags)
{
#if defined(APT_ENABLE_DEBUG_REPORT)
    if(gpDebugReport)
    {
        gpDebugReport->Print(flags);
    }
#endif
}

void ClearAptDebugReport()
{
#if defined(APT_ENABLE_DEBUG_REPORT)
    if(gpDebugReport)
    {
        gpDebugReport->ClearTracked();
    }
#endif
}

void EnableAptDebugTracking(bool enable)
{
#if defined(APT_ENABLE_DEBUG_REPORT)
    if(gpDebugReport)
    {
        gpDebugReport->EnableTracking(enable);
    }
#endif
}

int DebugGetNumAptValueTracked(int valueType)
{
#if defined(APT_ENABLE_DEBUG_REPORT)
    if (gpDebugReport)
    {
        return gpDebugReport->GetNumAptValues(valueType);
    }
#endif
    return 0;

}

// FIFA_BEGIN | SENDBACK | mwebb | 2015 - 05 - 26 | Adding Frostbite support
//AptUserFunctions& AptGetUserFuncs( void )
FB_APT_API AptUserFunctions& AptGetUserFuncs( void )
// FIFA_END
{
    return gAptFuncs;
}

AptGlobalExtensionObject* AptGetGlobalExtensionObject( void )
{
    return gpGlobalExtensionObject;
}


// (LEO) The gpZombieVector Vector keeps track of animations in the zombie state
// 253 Flow broken on backward : http://globaltechdocs.ea.com/MediaWiki/index.php?title=Apt:External_function_references
AptValueVector *    gpZombieVector = NULL;          // $BUG253 LEO    1/47

unsigned long gAptOptFlags = 0x00000000;
bool bDefaultMouseWheel = false;
// FIFA_BEGIN | SENDBACK | mwebb | Mar 6 2015 | change to allow PrintZombieDump to be an int
//bool bPrintZombieReferences = false;
int iPrintZombieReferences = 0;
// FIFA_END
SavedInputPlayback gSIPlayback;

AptSavedInputCheckpoints *gpSavedInputCheckpoints;

#if defined (APT_GATHER_MOVIECLIP_METRICS)
AptMovieclipInformation     gAptMovieclipInformation;
#endif

// This is the name we give to the AptFile associated with the Flash filters
// bytecode (the bytecode that contructs classes in flash.filters.*). No
// file is actually loaded-- as you'll see in AptPreloadFilterSWFFile, the
// code is just an array of bytes compiled into Apt.
static const char sFiltersPseudoFileName[] = "Apt-Flash-Native-Filters";

void AptUpdateLock();
void AptUpdateUnlock();

//-----------------------------------------------------------------------------

/// AptUpdateLock() and AptUpdateUnlock() are used by the APT_INC/APT_DEC macros to ensure that
/// external APT_INC and APT_DEC operations on AptValues are thread-safe.  The internal macros that
/// correspond to these, _APT_INC and _APT_DEC are not thread-safe, and do not use AptUpdateLock()
/// and AptUpdateUnlock().
void AptUpdateLock()
{
    gAptSimThreadMutex.Lock();
}

void AptUpdateUnlock()
{
    gAptSimThreadMutex.Unlock();
}


void AptMovieclipInformation::Reset()
{
    memset((void *)this, 0, sizeof(AptMovieclipInformation));
}


/*F************************************************************************************************/
/*!
    \Function    AptDebuggerPrint

    \Description
        Apt's debug print function

    All debug messages in Apt will be printed through this.  This was added so that we could apply a debug level
    to our messages so that we can do specific things to a message depending on the level passed to it.

    \Input      nMsgLvl           - debug level
    \Input      *szFormat         - string to print
    \Input      ...               -

    \Output
        None.

    \Notes
        None.

    \Version    1.0        12/21/04 (LEO) First Version

*/
/************************************************************************************************F*/
void AptDebuggerPrint(int32_t nMsgLvl, const char *szFormat, ...)
{
    va_list args;
    char szBuf[4096];

    va_start(args, szFormat);
    vsprintf(szBuf, szFormat, args);
    va_end(args);

#if (APT_DEBUG_LEVEL > APT_DEBUG_NONE) || defined(APT_ENABLE_ZOMBIE_OUTPUT)
    APT_PROFILE_AUXILIARY(pfnDebugPrint);
    gAptFuncs.pfnDebugPrint("%s", szBuf);
#endif
}
//#endif


/*** Private Functions ****************************************************************************/

static void _passthruToMemFree(void *p, size_t size)
{
    gAptFuncs.pfnMemFree(p);
}

/*** Public Functions *****************************************************************************/

/*F************************************************************************************************/
/*!
    \Function    AptUserFunctions::AptUserFunctions

    \Description
        Constructor that initializes all callback function pointers to zero.

    Insert your long description here.

    \Input
        None.

    \Output     A                 -

    \Notes
        None.

    \Version    1.0        04/30/04 (ASK) First Version
                            Added in 0.15.03 as a new callback function was added and
                            everybody will not be implementing it, and we do not want people to change
                            auxilliary libraries to change only for this change.
    \Version    2.0        09/20/07 (ASK) Added callback functions for UIEffects
    \Version    2.1        04/02/08 (RB)  Wrap renderflags function pointers with #if defined(APT_RENDER_FLAGS).
*/
/************************************************************************************************F*/
AptUserFunctions::AptUserFunctions()
{
    pfnMemAlloc = 0 ;
    pfnMemFree = 0 ;
    pfnMemFreeSize = 0 ;
#if APT_USE_TEMPORARY_ALLOCATORS
    pfnTempAlloc = 0;
    pfnTempFree = 0;
    pfnTempFreeSize = 0;
#endif
    pfnRenderListAlloc = 0;
    pfnRenderListFree = 0;
    pfnRenderListFreeSize = 0;
    pfnAssertFail = 0 ;
    pfnAssertFailMsg = 0 ;
    pfnSetBackgroundColour = 0 ;
    pfnDebugPrint = 0 ;
#if defined(APT_DO_PROFILE)
    pfnProfilerInit = 0 ;
    pfnProfilerShutdown = 0 ;
    pfnProfilerStartSample = 0 ;
    pfnProfilerStopSample = 0 ;
#endif
    pfnDebugAddSavedInput = 0 ;
    pfnDebugSetScreenGrabPending = 0 ;
    pfnLoadAnimation = 0 ;
    pfnFreeAnimation = 0 ;
    pfnFreeConstantTable = 0 ;
    pfnLoadAnimationCompleted = 0 ;
    pfnLoadImage = 0 ;
    pfnFreeImage = 0 ;
    pfnCommand = 0 ;
    pfnLoadVariables = 0 ;
    pfnLoadVariablesNULL = 0 ;
    pfnSetExternVariable = 0 ;
    pfnGetExternVariable = 0 ;
    pfnSendVariables = 0 ;
    pfnAllocateString = 0 ;
    pfnDeallocateString = 0 ;
    pfnDrawString = 0 ;
#if defined(APT_USE_SOUND_OBJECT)
    pfnLoadSound = 0 ;
    pfnFreeSound = 0 ;
    pfnStartSound = 0 ;
    pfnStartSoundStream = 0 ;
#endif
    pfnLoadTexture = 0 ;
    pfnFreeTexture = 0 ;
    pfnBindTexture = 0 ;
    pfnLoadRenderingUnit = 0 ;
    pfnFreeRenderingUnit = 0 ;
    pfnSetVertexMatrix = 0 ;
    pfnSetColourTransform = 0 ;
    pfnDrawRenderingUnit = 0 ;
    pfnCustomControlRender = 0 ;
    pfnOnUnload = 0 ;
    pfnPointHitTest = 0 ;
    pfnGetRealTimeClock = 0 ;
    pfnGetBytesTotal = 0 ;
    pfnGetBytesLoaded = 0 ;
    pfnUninitializedVarAccess = 0 ;
    pfnGetStageHeight = 0;
    pfnGetStageWidth = 0;
    pfnCustomControlUpdate = 0;
    pfnPlaySavedInputsDone = 0;
    pfnCustomSavedInputHandler = 0 ;
#if defined (APT_CUSTOM_CONTROL_USE_ZID)
    pfnCreateCustomControlZid = 0 ;
    pfnDestroyCustomControlZid = 0 ;
    pfnCustomControlRenderWithZid = 0 ;
#endif //APT_CUSTOM_CONTROL_USE_ZID
#if defined(APT_RENDER_FLAGS)
    pfnPushRenderFlags = 0;
    pfnPopRenderFlags = 0;
#endif // APT_RENDER_FLAGS
    pfnSyncOpenFile = 0;
    pfnSyncWriteToFile = 0;
    pfnSyncCloseFile = 0;
    pfnGetCurrentTime = 0;
    pfnHandleZombieState = 0;

    pfnCreateEffect  = 0;
    pfnDestroyEffect = 0;
    pfnPushEffect    = 0;
    pfnPopEffect     = 0;

    pfnRenderDispatch = 0;
    pfnResetTargetInstance = 0;

    pfnUpperCaseString = 0;
    pfnLowerCaseString = 0;
}


/*F************************************************************************************************/
/*!
    \Function    AptInitialize

    \Description
        Initializes all Apt Core Related items.

    \Input      *pAptInitParms    - Configuration options for Apt, Mainly related to memory stuff.

    \Output
        None.

    \Notes
        None.

    \Version    1.0        11/03/04 (DC) Bringin' it to ya

*/
/************************************************************************************************F*/
void AptInitialize(const AptInitParmsT *pAptInitParms, bool bCreateDefaultTarget)
{
    AptUpdateInitialize(pAptInitParms, bCreateDefaultTarget) ;
    AptRenderInitialize() ;

#ifdef APT_DEBUGGER_ENABLE
    AptDebugger::Initialize();
#endif

// FIFA_BEGIN | SENDBACK | clong | 2015-04-16 | The ActionScript Profiler should not always be enabled, this was added for FIFA's on screen profile results (see metricsystem package), but this should be only enabled when that on screen profiler is enabled
// This was added in FIFA 16 but we do not want it for FIFA FB
//#if APT_DO_AS_PROFILE
//	AptSetActionScriptProfilerEnabled( true );
//#endif // APT_DO_AS_PROFILE
// FIFA_END
}


/*F************************************************************************************************/
/*!
    \Function    AptUpdateInitialize

    \Description
        Update side initialize.

    Insert your long description here.

    \Input      *pAptInitParms    -

    \Output
        None.

    \Notes
        This does most of the original work of AptInitialize.

    \Version    1.0        04/25/05 (ASK) First Version
    \Version    1.1        04/02/08 (RB)  Removed APT_SET_SIMULATION_THREAD
    \Version    1.2        07/29/08 (RB)  Call AptHighwaterMemoryMetricsImpl::Initialize.
*/
/************************************************************************************************F*/
void AptUpdateInitialize(const AptInitParmsT *pAptInitParms, bool bCreateDefaultTarget)
{
    APT_ASSERT(!gbUpdateInitialized);
    AptUpdateAutoLock lock;

    AptInitParmsT defParms;
    const AptInitParmsT &initParms = (pAptInitParms) ? (*pAptInitParms) : defParms;
    gAptInitParms =  initParms ;

    AptHighwaterMemoryMetricsImpl::Initialize();

    APT_ASSERT( (initParms.iRegArraySize > 4)  && "Register Array Size must be at least 4. Flash regularly uses these");

    //  ONA:    Added string initialization to fix issue with PS2 linker
    if (gstrTempString.IsValid() == false)
    {
        gstrTempString.Validate();
    }
#if defined(APT_USE_MOUSE)
    bDefaultMouseWheel = initParms.bDefaultMouseWheelFlag;
#else
    bDefaultMouseWheel = false;
#endif
	// FIFA_BEGIN | SENDBACK | mwebb | Mar 6 2015 | change to allow PrintZombieDump to be an int
	//bPrintZombieReferences = initParms.bPrintZombieDump;
    iPrintZombieReferences = initParms.iPrintZombieDump;
	// FIFA_END
    //APT_ASSERT(gAptFuncs.pfnAssertFail);  // Issue 107825. APT_ASSERT defined in terms of pfnAssertFail  JP
    APT_ASSERT(gAptFuncs.pfnMemAlloc);
    APT_ASSERT(gAptFuncs.pfnMemFree);
    if (gAptFuncs.pfnMemFreeSize == 0)
    {
        gAptFuncs.pfnMemFreeSize = _passthruToMemFree;
    }
#if APT_USE_TEMPORARY_ALLOCATORS
    APT_ASSERT(gAptFuncs.pfnTempAlloc);
    APT_ASSERT(gAptFuncs.pfnTempFree);
    APT_ASSERT(gAptFuncs.pfnTempFreeSize);
#endif
    APT_ASSERT(gAptFuncs.pfnSetBackgroundColour);
    APT_ASSERT(gAptFuncs.pfnDebugPrint);
    APT_ASSERT(gAptFuncs.pfnDebugAddSavedInput);
    APT_ASSERT(gAptFuncs.pfnDebugSetScreenGrabPending);
    APT_ASSERT(gAptFuncs.pfnLoadAnimation);
    APT_ASSERT(gAptFuncs.pfnFreeConstantTable);
    APT_ASSERT(gAptFuncs.pfnFreeAnimation);
    APT_ASSERT(gAptFuncs.pfnCommand);
    APT_ASSERT(gAptFuncs.pfnLoadVariables);
    APT_ASSERT(gAptFuncs.pfnSetExternVariable);
    APT_ASSERT(gAptFuncs.pfnGetExternVariable);
    APT_ASSERT(gAptFuncs.pfnAllocateString);
    APT_ASSERT(gAptFuncs.pfnDeallocateString);
    APT_ASSERT(gAptFuncs.pfnDrawString);
#if defined(APT_USE_SOUND_OBJECT)
    APT_ASSERT(gAptFuncs.pfnLoadSound);
    APT_ASSERT(gAptFuncs.pfnFreeSound);
    APT_ASSERT(gAptFuncs.pfnStartSound);
    APT_ASSERT(gAptFuncs.pfnStartSoundStream);
#endif
    APT_ASSERT(gAptFuncs.pfnLoadTexture);
    APT_ASSERT(gAptFuncs.pfnFreeTexture);
    APT_ASSERT(gAptFuncs.pfnBindTexture);
    //APT_ASSERT(gAptFuncs.pfnSetVertexMatrix);    // $senzee now deprecated in favor of AptRenderInfo
    //APT_ASSERT(gAptFuncs.pfnSetColourTransform); // $senzee now deprecated in favor of AptRenderInfo
    APT_ASSERT(gAptFuncs.pfnLoadRenderingUnit);
    APT_ASSERT(gAptFuncs.pfnFreeRenderingUnit);
    APT_ASSERT(gAptFuncs.pfnDrawRenderingUnit);
    APT_ASSERT(gAptFuncs.pfnCustomControlRender);
    APT_ASSERT(gAptFuncs.pfnGetStageHeight);
    APT_ASSERT(gAptFuncs.pfnGetStageWidth);
    //gAptFuncs.pfnPointHitTest;    // do not assert on this right now as everybody might not be implementing it.

#ifndef APT_SINGLE_THREADED
    gAptSimThreadId = APT_THREAD_NAMESPACE::GetThreadId();
#endif

    // DC Moved rearranged initialization to make things more straightforward.
    AptInitializeGC();

    if (!gbCommonInitialized)
    {
        AptCommonInitialize(&gAptInitParms) ;
    }

    if(initParms.iZombieVectorSize == 0)
    {
        gpZombieVector = NULL;
    }
    else
    {
        gpZombieVector = new AptValueVector(initParms.iZombieVectorSize);             // $BUG253 LEO    2/47
    }

    // Create the manager before any CIH object might be created.
    // Initialize global variables like gpundefined var and others.
    AptValueInitialize();
    // Initialize the action interpreter variables.
    gAptActionInterpreter.initialize(initParms);

    gpSavedInputCheckpoints = new AptSavedInputCheckpoints();

    if(bCreateDefaultTarget)
    {
        // pool sizes are parameterized.
        gpDefaultTarget         = new AptTarget(&gAptInitParms);
        gpCurrentTargetSim      = gpDefaultTarget;
        gpCurrentTargetRender   = gpDefaultTarget;

        // preload the filters SWF file.
        AptPreloadFilterSWFFile();
    }
    else
    {
        gpDefaultTarget         = NULL;
        gpCurrentTargetSim      = NULL;
        gpCurrentTargetRender   = NULL;
    }

    gbUpdateInitialized = 1;

#if defined(APT_GENERATE_SIZE_CHECK_CODE)
    // Define this to generate the code below with the current sizes (then copy from TTL to the source code).
#define APT_SIZE_CHECK( _obj, oldsize) \
    APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, "APT_SIZE_CHECK( %30.30s, /*OLD_SIZE*/ 0x%4.4x ); // This warns of object sizes changing \n", #_obj, sizeof( _obj ))

#elif defined(APT_VERIFY_OBJECT_SIZES)
    // ASSERT if the sizes don't match.
#define APT_SIZE_CHECK( _obj, oldsize) \
    APT_ASSERT( sizeof(_obj) == oldsize)
#else
    // By default we ignore this.
#define APT_SIZE_CHECK( _obj, oldsize)
#endif

    // Note that all garbage collected objects are 8 bytes larger then this for GC overhead.
    APT_SIZE_CHECK(                       AptValue, /*OLD_SIZE*/ 0x0008 ); // This warns of object sizes changing
    APT_SIZE_CHECK(                         AptCIH, /*OLD_SIZE*/ 0x0060 ); // This warns of object sizes changing
    APT_SIZE_CHECK(                  AptFrameStack, /*OLD_SIZE*/ 0x0020 ); // This warns of object sizes changing
    APT_SIZE_CHECK(                      AptObject, /*OLD_SIZE*/ 0x001c ); // This warns of object sizes changing
    APT_SIZE_CHECK(                  AptNativeHash, /*OLD_SIZE*/ 0x0014 ); // This warns of object sizes changing
    APT_SIZE_CHECK(                AptNativeString, /*OLD_SIZE*/ 0x0004 ); // This warns of object sizes changing
    APT_SIZE_CHECK(                      AptString, /*OLD_SIZE*/ 0x0010 ); // This warns of object sizes changing
    APT_SIZE_CHECK(                     AptBoolean, /*OLD_SIZE*/ 0x000c ); // This warns of object sizes changing
    APT_SIZE_CHECK(              AptNativeFunction, /*OLD_SIZE*/ 0x0024 ); // This warns of object sizes changing
    APT_SIZE_CHECK(          AptScriptFunctionBase, /*OLD_SIZE*/ 0x0030 ); // This warns of object sizes changing
    APT_SIZE_CHECK(             AptScriptFunction1, /*OLD_SIZE*/ 0x0034 ); // This warns of object sizes changing
    APT_SIZE_CHECK(             AptScriptFunction2, /*OLD_SIZE*/ 0x0034 ); // This warns of object sizes changing
    APT_SIZE_CHECK( AptScriptFunctionByteCodeBlock, /*OLD_SIZE*/ 0x0044 ); // This warns of object sizes changing
    APT_SIZE_CHECK(                      AptLookup, /*OLD_SIZE*/ 0x000c ); // This warns of object sizes changing
    APT_SIZE_CHECK(                     AptInteger, /*OLD_SIZE*/ 0x000c ); // This warns of object sizes changing
    APT_SIZE_CHECK(                    AptRegister, /*OLD_SIZE*/ 0x000c ); // This warns of object sizes changing
    APT_SIZE_CHECK(                       AptFloat, /*OLD_SIZE*/ 0x000c ); // This warns of object sizes changing
    APT_SIZE_CHECK(                       AptArray, /*OLD_SIZE*/ 0x0028 ); // This warns of object sizes changing
    APT_SIZE_CHECK(                       AptSound, /*OLD_SIZE*/ 0x0028 ); // This warns of object sizes changing
    APT_SIZE_CHECK(                         AptKey, /*OLD_SIZE*/ 0x001c ); // This warns of object sizes changing
    APT_SIZE_CHECK(                     AptMathObj, /*OLD_SIZE*/ 0x001c ); // This warns of object sizes changing
    APT_SIZE_CHECK(                        AptNone, /*OLD_SIZE*/ 0x0008 ); // This warns of object sizes changing
    APT_SIZE_CHECK(                AptScriptColour, /*OLD_SIZE*/ 0x0020 ); // This warns of object sizes changing
    APT_SIZE_CHECK(                      AptObject, /*OLD_SIZE*/ 0x001c ); // This warns of object sizes changing
    APT_SIZE_CHECK(                   AptPrototype, /*OLD_SIZE*/ 0x0020 ); // This warns of object sizes changing
    APT_SIZE_CHECK(                        AptDate, /*OLD_SIZE*/ 0x0060 ); // This warns of object sizes changing
    APT_SIZE_CHECK(                      AptGlobal, /*OLD_SIZE*/ 0x001c ); // This warns of object sizes changing
    APT_SIZE_CHECK(                  AptTextFormat, /*OLD_SIZE*/ 0x0040 ); // This warns of object sizes changing
    APT_SIZE_CHECK(                   AptMovieClip, /*OLD_SIZE*/ 0x0020 ); // This warns of object sizes changing
    APT_SIZE_CHECK(                     AptXmlNode, /*OLD_SIZE*/ 0x0028 ); // This warns of object sizes changing
    APT_SIZE_CHECK(                         AptXml, /*OLD_SIZE*/ 0x0028 ); // This warns of object sizes changing
    APT_SIZE_CHECK(               AptXmlAttributes, /*OLD_SIZE*/ 0x0024 ); // This warns of object sizes changing
    APT_SIZE_CHECK(                 AptValueVector, /*OLD_SIZE*/ 0x0010 ); // This warns of object sizes changing
    APT_SIZE_CHECK(                    AptLoadVars, /*OLD_SIZE*/ 0x0024 ); // This warns of object sizes changing
    APT_SIZE_CHECK(                       AptStage, /*OLD_SIZE*/ 0x001c ); // This warns of object sizes changing
    APT_SIZE_CHECK(                       AptError, /*OLD_SIZE*/ 0x0024 ); // This warns of object sizes changing
    APT_SIZE_CHECK(                AptStringObject, /*OLD_SIZE*/ 0x0020 ); // This warns of object sizes changing
}





/*F************************************************************************************************/
/*!
    \Function    AptRenderInitialize

    \Description
        Render side initialize

    \Input
        None.

    \Output
        None.

    \Notes
        None.

    \Version    1.0        04/25/05 (ASK) First Version
    \Version    1.1        04/02/08 (RB)  Removed APT_SET_RENDER_THREAD
    \Version    1.2        07/29/08 (RB)  Changed gpFreeTextureArray to SafeQueueFixed.
*/
/************************************************************************************************F*/
void AptRenderInitialize()
{
    AptUpdateAutoLock lock;

    APT_ASSERT(!gbRenderInitialized) ;

    if (!gbCommonInitialized)
    {
        AptCommonInitialize(&gAptInitParms) ;
    }

    // Create the manager before any CIH object might be created.
    // create the Rendertree manager
    AptMath::ClipStackInit(128);
    gbRenderInitialized = 1 ;

    sFreeCustomCtrlQueue.Initialize( (uint32_t) AptRenderableCustomControl::MAX_FREE_CUSTCTRLS, &gAptFreeCustCtrlMutex, AptHighwaterMemoryMetricsImpl::GetMetrics(AptHighwaterMemoryMetrics::TYPE_FREECUSTCTRLS) );
    AptRenderableCustomControl::SetFreeQueue( &sFreeCustomCtrlQueue );

    AptRenderList::Initialize();
}


/*F************************************************************************************************/
/*!
    \Function    AptCommonInitialize

    \Description
        Common initialize function

    \Input      *pAptInitParms    - AptInitParamsT - initialization parameters.

    \Output
        None.

    \Notes
        None.

    \Version    1.0        04/25/05 (ASK) First Version

*/
/************************************************************************************************F*/
void AptCommonInitialize(const AptInitParmsT *pAptInitParms)
{
    gbCommonInitialized = 1 ;

    AptAnimationTarget::SetupStaticData(*pAptInitParms);
    AptNativeString::MemInitialize(&gAptFuncs);
    StringPool::Initialize(pAptInitParms->iStringPoolSize);
    gpValuesToRelease = new AptValueVector(pAptInitParms->iDeferedVectorSize);
}


/*F************************************************************************************************/
/*!
    \Function    AptReset

    \Description
        This function will work like soft AptShutdown. This will clear all the
        Global objects, the displaylist and make Apt like it has just started

    \Input
        None.

    \Output
        None.

    \Notes
        This function is a soft reset function. User can use it to clear everything
        no memory will be deleted execpt AptLinker and AptLoader. All the pools will
        be kept as they are, only displaylist and globals will be cleared. And pending file operations
        will also be cancelled. This will make Apt as if it is just started.
        Difference between AptShutdown and AptReset, is you do not have to call
        AptInitialize again, and all the pools are kept of same size, no allocation of pools is
        done.

    \Version    1.0        02/23/05 (ASK) First Version Added after 0.19.01

*/
/************************************************************************************************F*/
void AptReset()
{
    AptUpdateAutoLock lock;

    APT_ASSERT(GetTarget()->GetAnimationTarget());

    gAptActionInterpreter.bShutDown = true;
    //AptMath::ClipStackShutdown();
    AptScriptFunctionBase::ShutdownStaticData();
    GetTarget()->GetAnimationTarget()->PreDestroy();

    AptUpdateZombieVector(true);                         // Clean out the zombie vector $BUG253 LEO    3/47

    GetTarget()->Reset();
    delete  gpSavedInputCheckpoints;

    AptValueShutdown(true);
    CleanAllNativeFunctions();

    AptGC::CleanAll();
    AptValueShutdownRemaining();

    // this has to be done later after garbage collection
    gpValuesToRelease->ReleaseValues();

    gpDefaultTarget         = new AptTarget(&gAptInitParms);
    gpCurrentTargetSim      = gpDefaultTarget;
    gpCurrentTargetRender   = gpDefaultTarget;

    if (gAptFuncs.pfnResetTargetInstance) { gAptFuncs.pfnResetTargetInstance(gpDefaultTarget); }

    GetTarget()->GetAnimationTarget()->GetDisplayList()->pState = new AptDisplayListState();
    AptValueInitialize() ;
    AptScriptFunctionBase::InitializeStaticData(gAptInitParms);

    gpSavedInputCheckpoints = new AptSavedInputCheckpoints();

    GetTarget()->GetAnimationTarget()->Reset() ;
    gAptActionInterpreter.bShutDown = false;

    // preload the filters SWF file.
    AptPreloadFilterSWFFile();
}

/*F************************************************************************************************/
/*!
    \Function    AptShutdown

    \Description
        Clean up all Apt Core related items.

    \Input      bQuiet            -

    \Output
        None.

    \Notes
        None.

    \Version    1.0        11/03/04 (DC) Bringin' it to ya

*/
/************************************************************************************************F*/
void AptShutdown(int bQuiet)
{
#ifdef APT_DEBUGGER_ENABLE
    AptDebugger::Shutdown();
#endif
    AptUpdateShutdown(bQuiet) ;
    AptRenderShutdown() ;
    gbCommonInitialized = 0;
}



/*F************************************************************************************************/
/*!
    \Function    AptUpdateShutdown

    \Description
        Shutdown function for Update side of Apt. This could be called from separate
        thread as of render thread and it might be called before or after AptRenderShutdown.

    \Input      bQuiet            - not used right now

    \Output
        None.

    \Notes
        It calls AptCommonShutdown to complete remaining shutdown tasks if AptRenderShutdown is
        already been called.


    \Version    1.0        04/25/05 (ASK) First Version

*/
/************************************************************************************************F*/
void AptUpdateShutdown(int bQuiet)
{
    AptUpdateAutoLock lock;

    AptRenderList::Stop();

    APT_ASSERT(gbUpdateInitialized);
    gAptActionInterpreter.bShutDown = true;
    gbUpdateInitialized = 0 ;

    //  Before Garbage Collection, we clean all the native functions
    CleanAllNativeFunctions();

    gpCurrentTargetSim      = gpDefaultTarget;
    gpCurrentTargetRender   = gpDefaultTarget;

    if(gpDefaultTarget)
    {
        APT_SETUP_SIM_TARGET_STATES(gpDefaultTarget);
        APT_SETUP_RENDER_TARGET_STATES(gpDefaultTarget);
        gpDefaultTarget->GetAnimationTarget()->PreDestroy();
        gpDefaultTarget->GetAnimationTarget()->CleanRemList();
        APT_TEARDOWN_RENDER_TARGET_STATES;
        APT_TEARDOWN_SIM_TARGET_STATES;
    }

    AptValueShutdown(bQuiet);

    AptUpdateZombieVector(true);                         // Clean out the zombie vector $BUG253 LEO    3/47

    delete  gpSavedInputCheckpoints;
    gpSavedInputCheckpoints = NULL;

    AptGC::CleanAll();
    //AptValueShutdownRemaining();

    // this has to be done later after garbage collection
    gpValuesToRelease->ReleaseValues();

    if(gpZombieVector != NULL)
    {
        delete  gpZombieVector;                              // $BUG253 LEO    4/47
    }
    gpZombieVector = NULL;

    gAptActionInterpreter.shutdown();

    APT_PROFILE_Shutdown();

    if (gbRenderInitialized == 0)
    {
        // that means render has already been shutdown and now update is last to shutdown.
        // so clean up remaining things.
        AptCommonShutdown() ;
    }

#ifndef APT_SINGLE_THREADED
    gAptSimThreadId = APT_THREAD_NAMESPACE::kThreadIdInvalid;
#endif
}


static void AptFreeOutstandingCustomControls()
{
    if ( gAptFuncs.pfnDestroyCustomControlZid  )
    {
        sFreeCustomCtrlQueue.DequeueAndFreeAll( gAptFuncs.pfnDestroyCustomControlZid );
    }
}


/*F************************************************************************************************/
/*!
    \Function    AptRenderShutdown

    \Description
        Shutdown function for Render side of Apt. This could be called from separate thread as of
        update thread and it might be called before or after AptUpdateShutdown.

    \Input
        None.

    \Output
        None.

    \Notes
        It calls AptCommonShutdown to complete remaining shutdown tasks if AptUpdateShutdown is
        already been called.

    \Version    1.0        04/25/05 (ASK) First Version
    \Version    1.1        07/29/08 (RB)  Changed gpFreeTextureArray to SafeQueueFixed.
*/
/************************************************************************************************F*/
void AptRenderShutdown()
{
    AptUpdateAutoLock lock;

    AptRenderList::Stop();

    APT_ASSERT(gbRenderInitialized);
    gbRenderInitialized = 0 ;
    AptMath::ClipStackShutdown();

    if (gbUpdateInitialized == 0)
    {
        // this means that update has already shutdown and now render is last one to shutdown.
        // so cleanup things which are common.
        AptCommonShutdown() ;
    }

    AptFreeOutstandingCustomControls();
    AptRenderableCustomControl::SetFreeQueue( NULL );
    sFreeCustomCtrlQueue.Uninitialize();
}


/*F************************************************************************************************/
/*!
    \Function    AptCommonShutdown

    \Description
        This function does common tasks of shutdown, which needs to be done at end.

    \Input
        None.

    \Output
        None.

    \Notes
        This gets called either from AptUpdateshutdown or AptRendershutdown which ever is being called last.

    \Version    1.0        04/25/05 (ASK) First Version

*/
/************************************************************************************************F*/
void AptCommonShutdown()
{
    gbRenderInitialized = 0 ;

    AptAnimationTarget::CleanupStaticData();

    if(gpDefaultTarget)
    {
        delete  gpDefaultTarget;
    }

    gpDefaultTarget = NULL;
    gpCurrentTargetSim = NULL;
    gpCurrentTargetRender = NULL;
    APT_ASSERT(nTargetsCreated == 0 && "Forgot to remove some AptTargetInstance's")

    AptRenderList::Shutdown();

#if defined(APT_RI_LIST_VERIFY)
    AptRenderItem::CleanUp();
    APT_ASSERT(AptRenderItem::spHeadItem == NULL);
#endif

    AptValueShutdownRemaining();
    gstrTempString.Clear();         //  Clear the temporary string

    // $bug758 - DC - 11/12/04 - Must ensure Free List pools are emptied at shutdown!
    StringPool::Teardown();

    //  We need to destruct gpValuesToRelease after StringPool::Teardown()
    //  Because the clamped strings will be destructed here and added in the gpValuesToRelease vector
    gpValuesToRelease->ReleaseValues();
    delete  gpValuesToRelease;
    gpValuesToRelease = NULL;

    AptInteger::ClearPool();
    AptFloat::ClearPool();
    StringPool::ClearTemporaryPool();

    AptNativeString::MemUninitialize();
    gbCommonInitialized = 0;
}



//-------------------------------------------------------------
#if defined(APT_ALLOCATION_TRACKING)
// DC - Added Tracking Callbacks to print out data about an allocated memory chuck. Feel free to add to these.
void PrintNonGCTrackedAllocation(TrackedAllocation *pTracker)
{
    APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, "  ptr:%p Size:0x%6.6X File:%s Line:%d\n", pTracker->pAllocated, pTracker->nAllocSize, pTracker->szFileName, pTracker->nLineNumber);
}
void PrintGCTrackedAllocation(TrackedAllocation *pTracker)
{
    APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, "  AptValueGC:%p VFT:%d Size:0x%6.6X File:%s Line:%d\n", pTracker->pAllocated, ((AptValueGC*)pTracker->pAllocated)->getVtblIndex(), pTracker->nAllocSize, pTracker->szFileName, pTracker->nLineNumber);
}
#endif


/*F************************************************************************************************/
/*!
    \Function    AptAllocatorInitialize

    \Description
        Sets up the Allocator used by Apt and it's support logic.

    This MUST be donw prior to AptInitialize.

    \Input      nGCMainPoolSize   -  Size of Main Pool For garbage collected objects
    \Input      nGCMainPoolSize   -  Size of Overflow Pool For garbage collected objects
    \Input      nGCMainPoolSize   -  Size of Main Pool For non-garbage collected objects
    \Input      nGCMainPoolSize   -  Size of overflow Pool For non-garbage collected objects

    \Output
        None.

    \Notes
        None.

    \Version    1.0        11/03/04 (DC) Bringin' it to ya

*/
/************************************************************************************************F*/
void AptAllocatorInitialize(size_t nGCMainPoolSize,    size_t nGCOverflowPoolSize,
                            size_t nNonGCMainPoolSize, size_t nNonGCOverflowPoolSize)
{
    AptUpdateAutoLock lock;

    APT_ASSERT(gbUpdateInitialized == 0 && "Apt Allocator must be initialized before Apt Core!" );
    AptValueGC_PoolManager::StaticInitialize();
    gpNonGCPoolManager = new DOGMA_PoolManager(     nNonGCMainPoolSize, nNonGCOverflowPoolSize, 4, 256, 0,false,0, true);
    gpGCPoolManager    = new AptValueGC_PoolManager(static_cast<uint32_t>(nGCMainPoolSize), static_cast<uint32_t>(nGCOverflowPoolSize));

#if defined(APT_ENABLE_DEBUG_REPORT)
    gpDebugReport      = new EA::Apt::Debug::AptReport(gpNonGCPoolManager, gpGCPoolManager);
    AptCIH::SetReporter(gpDebugReport);
    AptValue::SetReporter(gpDebugReport);
    EAStringC::SetReporter(gpDebugReport);
#endif

#if defined(APT_ALLOCATION_TRACKING)
    gpNonGCPoolManager->mnTrackers = 0x2000;
    gpNonGCPoolManager->mnTrackThisSize = gpNonGCPoolManager->TRACK_SIZE_ALL;
    gpGCPoolManager->mnTrackers = 0x2000;
    gpGCPoolManager->mnTrackThisSize = gpNonGCPoolManager->TRACK_SIZE_ALL;
#endif

#if defined (APT_GATHER_MEMORY_METRICS)
    AptMemoryMetrics::ResetAll();
    AptMemoryMetrics::Initialize();
#endif

    APT_PROFILE_Init();
}


/*F************************************************************************************************/
/*!
    \Function    AptAllocatorShutdown

    \Description
        Cleans up the allocator and releases all resources.

    This will also report any more allocated objects that are still there. If allocation tracking is
    on it will print out a report on the tracked objects that remain in memory.

    \Input
        None.

    \Output
        None.

    \Notes
        I apologize in advance for all the #defines... basically this is just trying to provide
        users with information about how much memory was used during the run. If
        DOGMA_EXTRA_MEMORY_COUNTERS is defined there is more information to provide to the user.
        If DO_COVERAGE is defined (define for the Apt test suite executions, not for game teams)
        then we don't want to print out anything that changes from build to build like total memory
        usage and especially not pointer values, doing so would cause our tests to fail for stupid
        reasons. We do however want our tests to fail if memory was not cleaned properly.

    \Version    1.0        11/03/04 (DC) Bringin' it to ya
    \Version    1.1        08/07/08 (ASK) Split mnBytesAllocated into 2 parts mnBytesAllocatedInDogma and mnBytesAllocatedOutsideDogma
                                        and mnItemsFreed, mnBytesInFreeLists is tracked only for DOGMA_EXTRA_MEMORY_COUNTERS

*/
/************************************************************************************************F*/
void AptAllocatorShutdown()
{
    AptUpdateAutoLock lock;

    APT_ASSERT(gbUpdateInitialized == 0 && "Apt Allocator must be shutdown after Apt Core!" );

    // Don't print this crap in Coverage Builds, unless there are leaks.
#if defined(DO_COVERAGE)
    if(gpNonGCPoolManager->mnItemsAllocated != 0)
#endif
    {
        APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, "--+-Apt NonGC Items Pool Shutdown\n");

        #if !defined(DO_COVERAGE) && defined (DOGMA_EXTRA_MEMORY_COUNTERS)
        {
            // Even if there are leaks, don't display mutable data in coverage build.
            APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, "  +- Items In Free Lists:0x%X\n", gpNonGCPoolManager->mnItemsFreed);
        }
        #endif

#if defined(APT_ALLOCATION_TRACKING)
        if(gpNonGCPoolManager->mpTrackers != NULL)
        {
            for(uint32_t i = 0; i < gpNonGCPoolManager->mnTrackers; i++)
            {
                if(gpNonGCPoolManager->mpTrackers[i].pAllocated != NULL)
                {
                    PrintNonGCTrackedAllocation(&gpNonGCPoolManager->mpTrackers[i]);
                }
            }
        }

        if(gpGCPoolManager->mpTrackers != NULL)
        {
            for(uint32_t i = 0; i < gpGCPoolManager->mnTrackers; i++)
            {
                if(gpGCPoolManager->mpTrackers[i].pAllocated != NULL)
                {
                    PrintGCTrackedAllocation(&gpGCPoolManager->mpTrackers[i]);
                }
            }
        }
        APT_ASSERT(gpNonGCPoolManager->mnBytesAllocatedInDogma == 0 && "MEMORY LEAK!!! Apt is shutting down with bytes allocated in Dogma!!!");
#endif

        if (gpNonGCPoolManager->mnBytesAllocatedInDogma != 0)
        {
            APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, "  +- Bytes Allocated    :0x%X   <-- !Warning! Shutting down with bytes allocated!!!\n", gpNonGCPoolManager->mnBytesAllocatedInDogma);
        }

        if(gpNonGCPoolManager->mnBytesAllocatedOutsideDogma != 0 && gpNonGCPoolManager->GetFirstOutsideAllocation() != NULL)
            {
                void * pCurrent = gpNonGCPoolManager->GetFirstOutsideAllocation();
                int    count = 1;
                while (pCurrent != NULL) {

                    #if !defined(DO_COVERAGE)
                    {
                        APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, "  +- Apt Non-GC Still Allocated! p:0x%p\n", pCurrent);
                    }
                    #else
                    {
                        // Don't print pointers in coverage build.
                        APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, "  +- Apt Non-GC Still Allocated! p:0x%x\n", -1);
                    }
                    #endif

                    pCurrent = gpNonGCPoolManager->GetNextOutsideAllocation(pCurrent);
                    count++;
                }
            }

        #if defined(DOGMA_EXTRA_MEMORY_COUNTERS)
        {
            // Give more detailed results if this is defined.
            if( gpNonGCPoolManager->mnBytesAllocatedInDogma != 0)
            {
                APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, "  +- Bytes Allocated Non-GC in DOGMA  :0x%X\n", gpNonGCPoolManager->mnBytesAllocatedInDogma);
            }

            if( gpNonGCPoolManager->mnBytesAllocatedOutsideDogma != 0)
            {
                APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, "  +- Bytes Allocated Non-GC Outside DOGMA   :0x%X\n", gpNonGCPoolManager->mnBytesAllocatedOutsideDogma);
            }

            #if !defined(DO_COVERAGE)
            {
                // Even if there are leaks, don't display mutable data in coverage build.
                APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, "  +- Bytes In Free Lists:0x%X\n", gpNonGCPoolManager->mnBytesInFreeLists);
            }
            #endif

        }
        #endif

        #if !defined(DO_COVERAGE)
        {
            // Even if there are leaks, don't display mutable data in coverage build.
            APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, "  +- Total Bytes Used   :0x%X\n", gpNonGCPoolManager->GetTotalBytesUsed() );
            APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, "  +- Number of Overflow Pools Created: %d\n", gpNonGCPoolManager->GetNumOverflowPools() );
        }
        #endif
    }

#if defined(DO_COVERAGE)
    if( gpGCPoolManager->mnItemsAllocated != 0)
#endif
    {
        APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, "--+-Apt Garbage Collected Items Pool Shutdown\n");
        #if !defined(DO_COVERAGE)  && defined (DOGMA_EXTRA_MEMORY_COUNTERS)
        {
            // Even if there are leaks, don't display mutable data in coverage build.
            APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, "  +- Items In Free Lists:0x%X\n", gpGCPoolManager->mnItemsFreed);
        }
        #endif

        if (gpGCPoolManager->mnBytesAllocatedInDogma != 0)
        {
            APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, "  +- Bytes Allocated    :0x%X   <-- !Warning! Shutting down with bytes allocated!!!\n", gpGCPoolManager->mnBytesAllocatedInDogma);
        }

        if(gpGCPoolManager->mnBytesAllocatedOutsideDogma != 0 && gpGCPoolManager->GetFirstOutsideAllocation() != NULL)
        {
            AptValue * pCurrent = gpGCPoolManager->GetFirstAptValue();
            int    count = 1;
            while (pCurrent != NULL) {

                #if !defined(DO_COVERAGE)
                {
                    APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, "  +- AptValue Still Allocated! this:0x%p Type:0x%X Size:0x%X\n", pCurrent, pCurrent->getVtblIndex(), AptGetSizeOfAptValue(pCurrent));
                }
                #else
                {
                    // Don't print pointers in coverage build.
                    APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, "  +- AptValue Still Allocated! this:0x%x Type:0x%X Size:0x%X\n", -1, pCurrent->getVtblIndex(), AptGetSizeOfAptValue(pCurrent));
                }
                #endif

                pCurrent = gpGCPoolManager->GetNextAptValue(pCurrent);
                count++;
            }
            APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, "  +- Total Items Allocated    :0x%X   <-- !Warning! Shutting down with items allocated!!!\n", count);
        }

        #if defined(DOGMA_EXTRA_MEMORY_COUNTERS)
        {
            // Give more detailed results if this is defined.
            if(gpGCPoolManager->mnBytesAllocatedInDogma != 0)
            {
                APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, "  +- Bytes Allocated    :0x%X\n", gpGCPoolManager->mnBytesAllocatedInDogma);
            }

            if( gpGCPoolManager->mnBytesAllocatedOutsideDogma != 0)
            {
                APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, "  +- Bytes Allocated    :0x%X\n", gpGCPoolManager->mnBytesAllocatedOutsideDogma);
            }

            #if !defined(DO_COVERAGE)
            {
                // Even if there are leaks, don't display mutable data in coverage build.
                APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, "  +- Bytes In Free Lists:0x%X\n", gpGCPoolManager->mnBytesInFreeLists);
            }
            #endif
        }
        #endif

        #if !defined(DO_COVERAGE)
        {
            // Even if there are leaks, don't display mutable data in coverage build.
            APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, "  +- Total Bytes Used   :0x%X\n", gpGCPoolManager->GetTotalBytesUsed() );
            APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, "  +- Number of Overflow Pools Created: %d\n", gpGCPoolManager->GetNumOverflowPools() );
        }
        #endif
    }

#if defined(APT_ALLOCATION_TRACKING)
    // One final assert if there are outstanding allocations, after all the above output
    if (gpNonGCPoolManager->mnBytesAllocatedInDogma != 0)
    {
        APT_ASSERTM(false, "AptAllocatorShutdown :: Non-garbage-collected memory (DOGMA) has been leaked.");
    }
#endif

#if defined(APT_ENABLE_DEBUG_REPORT)
    AptCIH::SetReporter(NULL);
    AptValue::SetReporter(NULL);
    EAStringC::SetReporter(NULL);

    delete gpDebugReport;
    gpDebugReport = 0;
#endif

    delete gpNonGCPoolManager;
    delete gpGCPoolManager;

    gpNonGCPoolManager = 0;
    gpGCPoolManager = 0;
}

// registers all the callback functions for garbage collected classes.
void AptInitializeGC()
{
    AptGC::Initialize();
}


void    CleanAllNativeFunctions()
{
    AptArray::CleanNativeFunctions();
#if defined (APT_USE_DATE_OBJECT)
    AptDate::CleanNativeFunctions();
#endif
#if APT_USE_MATH_OBJECT
    AptMathObj::CleanNativeFunctions();
#endif
    AptKey::CleanNativeFunctions();
#if defined(APT_USE_MOUSE)
    AptMouse::CleanNativeFunctions();
#endif
#if APT_USE_LOADVARS_OBJECT
    AptLoadVars::CleanNativeFunctions();
#endif
#if defined(APT_USE_SOUND_OBJECT)
    AptSound::CleanNativeFunctions();
#endif
#if APT_USE_SCRIPTCOLOUR_OBJECT
    AptScriptColour::CleanNativeFunctions();
#endif
    AptXml::CleanNativeFunctions();
    AptXmlNode::CleanNativeFunctions();
    AptCIH::CleanNativeFunctions();
    AptString::CleanNativeFunctions();
    AptError::CleanNativeFunctions();         // (DC) Added for Flash Player 7 / AS 2.0 support. (Release 17.0)
    AptMovieClipLoader::CleanNativeFunctions();
#if defined (APT_USE_STAGE_OBJECT)
    AptStage::CleanNativeFunctions() ;
#endif
#if APT_USE_UTILITY
    AptUtil::CleanNativeFunctions();
#endif
#if defined (APT_ALTERNATE_INPUT)
    AptAlternateInput::CleanNativeFunctions() ;
#endif
}

void AptCompleteAnimationAsyncLoad(AptFilePtr file, void *pData, void *pConstTable, void *pUserData)
{
    AptUpdateAutoLock lock;

    GetTargetSim()->GetLoader()->CompleteAnimationLoad(file, pData, pConstTable, pUserData);
}

void AptCompleteImageAsyncLoad(AptFilePtr file, AptAssetTexture texture, int width, int height, void* userData)
{
    AptUpdateAutoLock lock;

    GetTargetSim()->GetLoader()->CompleteImageLoad(file, texture, width, height, userData);
}

AptCIH *_AptGetAnimationAtLevel(int32_t nLevel)
{
    AptUpdateAutoLock lock;

    if (GetTargetSim() == NULL || (!GetTargetSim()->GetAnimationTarget() || !GetTargetSim()->GetAnimationTarget()->GetDisplayList()->pState))
    {
        return NULL;
    }

    AptCIH *pCur = GetTargetSim()->GetAnimationTarget()->GetDisplayList()->pState->GetFirstItem();

    while (pCur)
    {
        if (pCur->GetDepth() == nLevel)
        {
            return pCur;
        }
        pCur = pCur->GetDisplayListNext();
    }

    AptCIH *pEmpty = new AptCIH(NULL, NULL);

    pEmpty->SetDepth(nLevel);
    pEmpty->setGCRoot(1);
    GetTargetSim()->GetAnimationTarget()->GetDisplayList()->pState->insert(nLevel, pEmpty);

    return pEmpty;
 }

void AptGetAnimationSize(int *pnWidth, int *pnHeight)
{
    AptUpdateAutoLock lock;

    APT_ASSERT(gbUpdateInitialized);
    if (!GetTarget()->GetAnimationTarget() ||
        !GetTarget()->GetAnimationTarget()->GetDisplayList()->pState)
    {
        return ;
    }

    AptCIH *pNext = GetTarget()->GetAnimationTarget()->GetDisplayList()->pState->GetFirstItem();

    if ( pNext != NULL && pNext->GetCharacterInst() && pNext->IsAnimationInst())
    {
        AptCharacterAnimationInst *pSprInst = pNext->GetAnimationInst();

        if (pnWidth)
            *pnWidth = pSprInst->GetCharacterConst()->animation.nWidth;

        if (pnHeight)
            *pnHeight = pSprInst->GetCharacterConst()->animation.nHeight;
    }
    else
    {
        if (pnWidth)
            *pnWidth = 0;
        if (pnHeight)
            *pnHeight = 0;
    }
}


void AptSetValidFocusButton()
{
#if defined APT_USE_BUTTONS
    AptUpdateAutoLock lock;

    GetTargetSim()->GetAnimationTarget()->SetValidFocusButton();
#else
    APT_ASSERT(0 && "DEFINE APT_USE_BUTTONS TO USE THIS");
#endif
}

bool _AptValidate(void)
{
    return true;
/*  GetTarget()->GetAnimationTarget()->GetDisplayList()->validate(0);
    for (int i = 0; i < GetTarget()->GetAnimationTarget()->mBILCount; i++)
    {
//      APT_ASSERT(GetTarget()->GetAnimationTarget()->aButtonInstanceList[i].pCIH->IsButtonInst());
    }
    return true;*/
}

void AptLoadAnimation(const char *szBaseName, const char *szTarget)
{
    AptUpdateAutoLock lock;

    APT_ASSERT(gbUpdateInitialized);

    AptNativeString sTarget(szTarget);
    APT_ASSERT(sTarget != AptNativeString("#import") && "Please use AptPreloadFile instead of a target of '#import' now.");

    // Each Animation can only have one background color. This value is reset
    // every time the game (or viewer) loads a new animation.
    gbBackgroundColorSet = false;

    // Animations always end in ".swf" so make sure the filename ends in .swf.
    AptNativeString fileName(szBaseName);
    if (!fileName.EndWithIgnoreCase(".swf"))
    {
        fileName += ".swf";
    }

    GetTargetSim()->GetLinker()->Load(fileName, sTarget);
}

AptFilePtr AptPreloadAnimation(const char *szBaseName)
{
    gAptSimThreadMutex.Lock();

    // Animations always end in ".swf" so make sure the filename ends in .swf.
    AptNativeString fileName(szBaseName);
    if (!fileName.EndWithIgnoreCase(".swf"))
    {
        fileName += ".swf";
    }

    AptFilePtr fp = GetTargetSim()->GetLoader()->Load(fileName, AptFileType_Animation);
    gAptSimThreadMutex.Unlock();
    return fp;
}

void AptAddCustomSavedInput(void * pBuffer, unsigned int nSize)
{
    AptUpdateAutoLock lock;

    if (gbSavedInputsEnabled)
    {
        AptSavedInputRecordCustom inputRecord;
        inputRecord.nTick      = gnCurTick;
        inputRecord.nInputType = INPUT_CUSTOM;
        APT_ASSERT(nSize <= 0xFFFF && "Saved Custom Event handler cannot handle a buffer of this size!");
        inputRecord.nInputBufferSize = static_cast<uint16_t>(nSize);

        gAptFuncs.pfnDebugAddSavedInput(&inputRecord, sizeof(AptSavedInputRecordCustom) );
        gAptFuncs.pfnDebugAddSavedInput( (AptSavedInputRecord*) pBuffer, nSize);
    }
}

void AptCancelPreloadedAnimation(const char *szBaseName)
{
    AptUpdateAutoLock lock;

    GetTargetSim()->GetLoader()->CancelPreloadedAnimation(AptNativeString(szBaseName));
}

bool AptIsFileLoaded(AptFilePtr file)
{
    gAptSimThreadMutex.Lock();
    bool bResult = file.pData != NULL && file->GetState() == AptFile::Resolved;
    gAptSimThreadMutex.Unlock();
    return bResult;
}

/*F************************************************************************************************/
/*!
    \Function    _addScreenGrabToSavedInputs

    \Description
        Saves off the screen state record for the saved inputs file

    Insert your long description here.

    \Input
        None.

    \Output
        None.

    \Notes
        None.

    \Version    1.0        03/22/05 (LEO) First Version
                1.1        03/22/05 (LEO) Added Big Endian fix

*/
/************************************************************************************************F*/
static void _addScreenGrabToSavedInputs()
{
    if (gbSavedInputsEnabled)
    {
        char szBuf[16];
        sprintf(szBuf, "%06d", gnCurTick);
        gAptFuncs.pfnDebugSetScreenGrabPending(szBuf);

        AptSavedInputRecordInput inputRecord;
        inputRecord.nTick = gnCurTick;
        inputRecord.nInput = SET_SCREENGRAB_INPUT();
#if defined(EA_SYSTEM_BIG_ENDIAN)
        inputRecord.nInput = ((inputRecord.nInput << 24) & 0xff000000);
#endif
        gAptFuncs.pfnDebugAddSavedInput(&inputRecord, APT_SAVEINPUTS_RECORD_SIZE_SAVER);
    }
}

// (ASK) - after 0.19.01 -added new parameter/s to do selective update/tick of animation levels. This will only work at highest root levels.
static int _tick(unsigned int nDeltaTime, AptAnimLevelE eAnimLevels)
{
    APT_PROFILE_BEGIN( APT_TICK, "Apt::_tick" );
    {
        APT_PROFILE(_tick) ;
    int bAdvancedFrame = 0;

    AptAnimationTarget * pAnimTarget = GetTargetSim()->GetAnimationTarget();
    AptCIH * pRootItem = GetTargetSim()->GetAnimationTarget()->GetDisplayList()->pState->GetFirstItem();

    if (!pRootItem->IsAnimationInst())
        {
            APT_PROFILE_END( APT_TICK );
        return bAdvancedFrame;
        }

    AptCharacterAnimationInst *pAnimInst = pRootItem->GetAnimationInst();

    nDeltaTime += pAnimInst->mnLeftoverTime;
    unsigned int nMillisecondsPerFrame = pAnimInst->GetCharacterConst()->animation.nMillisecondsPerFrame;

    while (nDeltaTime >= nMillisecondsPerFrame)
    {
        APT_ASSERT(gAptActionInterpreter.stack.GetSize() == 0);

        pAnimTarget->TickIntervalTimers(nMillisecondsPerFrame);
        pAnimTarget->GetDisplayList()->tick(eAnimLevels, true);       // pass in the animLevels to do selective processing
        pAnimTarget->RunActions();
        pAnimTarget->ProcessInputs();

        GetTargetSim()->GetLinker()->Update();

        APT_ASSERT(gAptActionInterpreter.stack.GetSize() == 0);

        nDeltaTime      -= nMillisecondsPerFrame;
        gnCurTick       += nMillisecondsPerFrame;

        bAdvancedFrame = 1;

        if (!pAnimTarget->GetDisplayList()->pState->GetFirstItem()->IsAnimationInst())
        {
                APT_PROFILE_END( APT_TICK );
            return bAdvancedFrame;
        }

        if (gbSavedInputsEnabled) break;
    }

    // must check again, because it could have been unloaded
    if (GetTargetSim()->GetAnimationTarget()->GetDisplayList()->pState->GetFirstItem()->IsAnimationInst())
    {
        AptCharacterAnimationInst *pAnimInst = GetTargetSim()->GetAnimationTarget()->GetDisplayList()->pState->GetFirstItem()->GetAnimationInst();
        pAnimInst->mnLeftoverTime = nDeltaTime;
    }

#if defined(APT_USE_BUTTONS)
    GetTargetSim()->GetAnimationTarget()->ClearBIL();
    //(DUG) EATech 103381: Hit region ignores offset within movie clip
    //The AppendButtonToBIL() call in AptRenderItem.cpp was incorrectly placed and hence the precision to sense mouse over button was way off
    uint32_t (*pOldButtonValue) (AptCIH * pCIH, void * pVoid) = AptCIH::sCIHButtonProcessCb;
    AptCIH::sCIHButtonProcessCb = AptCIH::ProcessButtonState;
#endif

    uint32_t (*pOldValue1) (AptCIH * pCIH, void * pVoid) = AptCIH::sCIHProcessCb1;
    uint32_t (*pOldValue2) (AptCIH * pCIH, void * pVoid) = AptCIH::sCIHProcessCb2;
    AptCIH::sCIHProcessCb1 = AptCIH::ProcessCustomControls;
    AptCIH::sCIHProcessCb2 = AptCIH::ProcessMaskMatricies;
	// FIFA_BEGIN | SENDBACK | gmicle | 2018-10-23 | https://jira.frostbite.ea.com/browse/FB-108141 - Move performance markers from inside recursive functions to around their entry points, so that they work with Frostbite profiling tools (Delta Viewer).
	APT_PROFILE_BEGIN(APT_DL_GENERALISED_PROCESS, "AptDisplayList::GeneralisedProcess");
    AptDisplayList::GeneralisedProcess(GetTargetSim()->GetAnimationTarget()->GetDisplayList(), NULL, eAnimLevels, true) ;
	APT_PROFILE_END(APT_DL_GENERALISED_PROCESS);
	// FIFA_END
	AptCIH::sCIHProcessCb1 = pOldValue1 ;
    AptCIH::sCIHProcessCb2 = pOldValue2 ;

#if defined(APT_USE_BUTTONS)
    AptCIH::sCIHButtonProcessCb = pOldButtonValue;
#endif

        APT_PROFILE_END( APT_TICK );
    return bAdvancedFrame;
    }
}

/*F************************************************************************************************/
/*!
\Function    _playbackSavedInputs

\Description
Insert function's purpose here

Insert your long description here.

\Input      nDeltaTime        -
\Input      AptAnimLevel_ALL  -

\Output
None.

\Notes
None.

\Version    1.0        03/22/05 (LEO) First Version
1.1                 (ASK) - after 0.19.01 -added new parameter/s to do selective update/tick of animation levels. This will only work at highest root levels.
1.2        03/22/05 (LEO) added Big Endian support
            1.3        03/05/08 (RB)  Removed unused symbols.
*/
/************************************************************************************************F*/
static void _playbackSavedInputs(unsigned int nDeltaTime, AptAnimLevelE eAnimLevels = AptAnimLevel_ALL)
{
    unsigned int nTargetTime = gSIPlayback.nCurTick + nDeltaTime;
    unsigned int nTick;

    for (;gSIPlayback.pSavedInputs;)
    {
        APT_ASSERT(gSIPlayback.nCurTick <= nTargetTime);
        if (gSIPlayback.nCurTick == nTargetTime)
            break;

        // make sure all the necessary animations are loaded before continuing
        if (gpSavedInputCheckpoints->CanContinueSavedInputs())
        {
            while ((nTick = GET_UNALIGNED_U32(gSIPlayback.pCurSavedInput)) <= gSIPlayback.nCurTick)
            {
                APT_ASSERT(nTick == gSIPlayback.nCurTick);
                gSIPlayback.pCurSavedInput += sizeof(unsigned int);

                switch(GET_TYPE_INPUT(gSIPlayback.pCurSavedInput))
                {
                case INPUT_MOUSE:
                case INPUT_KEYBOARD:
                    {
                        unsigned int nInput = GET_UNALIGNED_U32( gSIPlayback.pCurSavedInput );
                        gSIPlayback.pCurSavedInput += 4;
#if defined(EA_SYSTEM_BIG_ENDIAN)
                        // Re-encode input
                        char * buf = (char *)&nInput;
                        for (int i=0; i<4; i+=2) {
                            buf[i]   ^= buf[i+1];
                            buf[i+1] ^= buf[i];
                            buf[i]   ^= buf[i+1];
                        }
                        short * buf2 = (short *)buf;
                        for (int i=0; i<2; i+=2) {
                            buf2[i]   ^= buf2[i+1];
                            buf2[i+1] ^= buf2[i];
                            buf2[i]   ^= buf2[i+1];
                        }
#endif
                        //gAptFuncs.pfnDebugPrint("**-** %d Input: %x\n",nTick, nInput);
                        unsigned int nKeyCode = 0;
                        GET_ACTION_TYPE(nInput, nKeyCode);
                        switch(nKeyCode)
                        {
                            case AptInputType_LeftAnalogStick:
                            case AptInputType_RightAnalogStick:
                                GetTargetSim()->GetAnimationTarget()->AddAnalogInput(*((AptAnalogStickInfo *)gSIPlayback.pCurSavedInput));
                                gSIPlayback.pCurSavedInput += sizeof(AptAnalogStickInfo);
                                break;
                            default:
                                GetTargetSim()->GetAnimationTarget()->AddInput(nInput);
                                break;
                        }
                        break;
                     }
                case INPUT_CHECKPOINT:
                    {
                        APT_ASSERT(INPUT_IS_CHECKPOINT(gSIPlayback.pCurSavedInput));
                        char *szName = (char *)++gSIPlayback.pCurSavedInput;
                        gSIPlayback.pCurSavedInput += strlen(szName) + 1;

                        //gAptFuncs.pfnDebugPrint("**-** %d Checkpoint: %s\n",nTick, szName);

#if defined(EA_SYSTEM_BIG_ENDIAN)
                        // Byte Align pCurSavedInputs
                        uintptr_t nTmp = (uintptr_t)gSIPlayback.pCurSavedInput;
                        if(nTmp & 0x03)
                        {
                            nTmp += 4;
                            nTmp &= ~0x03;
                        }
                        gSIPlayback.pCurSavedInput = (unsigned char *)nTmp;
#endif

                        AptNativeString fileName(szName);
#if (!APT_USE_FILTERS)
                        // special case: if the saved-input file wants us to set a checkpoint for
                        // the Flash filters bytecode and wait for it to be loaded, we can't--
                        // because it never will be loaded if APT_USE_FILTERS==0.
                        if ( fileName == sFiltersPseudoFileName )
                        {
                            break;
                        }
#endif
                        gpSavedInputCheckpoints->Checkpoint(AptNativeString(szName));
                        break;
                    }
                case INPUT_OTHER:
                    {
                        switch(GET_OTHER_INPUT_TYPE(gSIPlayback.pCurSavedInput))
                        {
                        case INPUT_SCREENGRAB:
                            {
                                //gAptFuncs.pfnDebugPrint("**-** %d ScreenGrab\n",nTick);

#if defined(EA_SYSTEM_BIG_ENDIAN)
                                gSIPlayback.pCurSavedInput += 4;
#else
                                gSIPlayback.pCurSavedInput++;
#endif
                                if (!gbSavedInputsEnabled) // if we're saving inputs, we want to do this at the normal saved input time
                                {
                                    char szBuf[16];
                                    sprintf(szBuf, "%06d", gSIPlayback.nCurTick);
                                    gAptFuncs.pfnDebugSetScreenGrabPending(szBuf);
                                }
                                break;
                            }
                        case INPUT_CUSTOM:
                            {
                                //gAptFuncs.pfnDebugPrint("**-** %d Custom\n",nTick);
                                gSIPlayback.pCurSavedInput += 2;
                                uint32_t nCustomMessageBytes = GET_UNALIGNED_U16( gSIPlayback.pCurSavedInput );
                                gSIPlayback.pCurSavedInput += 2;

                                APT_ASSERT(gAptFuncs.pfnCustomSavedInputHandler && "Custom Input handler found but not handler present!");

                                if(gAptFuncs.pfnCustomSavedInputHandler)
                                {
                                    gAptFuncs.pfnCustomSavedInputHandler(gSIPlayback.pCurSavedInput, nCustomMessageBytes);
                                }

                                gSIPlayback.pCurSavedInput += nCustomMessageBytes;
                                break;
                            }
                        case INPUT_TRIGGER:
                            {
                                // added this extra type for trigger analog data. This is a special case because, for trigger button we just have normal
                                // keyup/key down messages, but now we also support analog values for triggers on xenon. These analog values are
                                // stored in static array called AptAnimationTarget::sgATriggers[AptInputController_NumControllers]
                                // We save those values to saved input file and replay them. There will be a corresponding AddInput for keyup/keydown
                                // along with these AddAnalogInput call. But it might be before this record or after this record, depending on how
                                // user has passed to Apt originally.
                                AptSavedInputRecordTriggers *pTriggerInfo = (AptSavedInputRecordTriggers *) ( gSIPlayback.pCurSavedInput - sizeof(unsigned int) );
                                GetTargetSim()->GetAnimationTarget()->AddAnalogInput(pTriggerInfo->mAnalogTriggerInfo);
                                gSIPlayback.pCurSavedInput += ( sizeof(AptSavedInputRecordTriggers) - sizeof(unsigned int) );
                                break;
                            }
                        default:
                            APT_ASSERT(false && "Unknown Saved Input Type Reached!!!");
                        }
                        break;
                    }
                default:
                    // do nothing
                    APT_ASSERT(false && "Unknown Saved Input Type Reached!!!")
                        break;
                }

                APT_ASSERT((gSIPlayback.pCurSavedInput - gSIPlayback.pSavedInputs) <= gSIPlayback.nInputFileSize);
            }
        }
        else
        {
            // $HACK!!! We need to get this outta here with a revamp of the saved inputs. The problem is that interval timers are not tracked as it stands
            // so timing issues can (and did) arise.
            // If we couldn't tick because files weren't loaded, at least make sure we have played the interval timer (in case that is what loaded the file we are waiting on)
            nTick = GET_UNALIGNED_U32(gSIPlayback.pCurSavedInput);

            int timeToSkip = nTick - gSIPlayback.nCurTick;
            timeToSkip--;

            if(timeToSkip > 0)
            {
                GetTargetSim()->GetAnimationTarget()->TickIntervalTimers(16);
            }
        }

        if ((gSIPlayback.pCurSavedInput - gSIPlayback.pSavedInputs) >= gSIPlayback.nInputFileSize)
        {
            APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, "Playback of inputs completed.\n");
            gSIPlayback.pSavedInputs = NULL; // we aren't responsible for this memory
            gSIPlayback.pCurSavedInput = NULL;

            if(gAptFuncs.pfnPlaySavedInputsDone != NULL)
            {
                gAptFuncs.pfnPlaySavedInputsDone(true, NULL);
            }

            if (gbSavedInputsEnabled)
            {
                APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, "  Turning off saved inputs too.\n");
                gbSavedInputsEnabled = 0;
            }
            break;
        }

        // make sure all the necessary animations are loaded before continuing

        int bCorrectAnimsLoaded = gpSavedInputCheckpoints->CanContinueSavedInputs();

        if (bCorrectAnimsLoaded
            && GetTargetSim()->GetAnimationTarget()->GetDisplayList()->pState->GetFirstItem()
            && GetTargetSim()->GetAnimationTarget()->GetDisplayList()->pState->GetFirstItem()->IsAnimationInst())
        {
            // DC - Shouldn't be queuing frames in playback, but passing in an arbitrary number so that it has one.
            if (_tick(1, eAnimLevels))
            {
                _addScreenGrabToSavedInputs();
            }
            gSIPlayback.nCurTick++;
        }
        else
        {
            GetTargetSim()->GetLinker()->Update();
            break;
        }
    }
}


// (ASK) - after 0.19.01 -added new parameter/s to do selective update/tick of animation levels. This will only work at highest root levels.
static void _AptInternalUpdate(unsigned int nDeltaTime, AptAnimLevelE eAnimLevels)
{
#if defined (APT_DO_AS_PROFILE)
    // Start action script function profiling for the frame
    gAptActionInterpreter.mFunctionProfiler.LogFrameStart();
#endif

    APT_ASSERT(gbUpdateInitialized);

    // Grab a reference to all loaded files before we start doing the update
    // This is so that if a block of code, or a user callback unloads the animation
    // we're currently executing, we don't be freeing the data we're using
    // Don't make it a Vector, as it previously was, otherwise we're thrashing dynamic memory
    // while doing an Update.
    // The sole purpose of this is to increment the reference counts until the end of the function.
    AptFilePtr aLoadedFiles[APT_MAX_FILES_LOADED];
    GetTargetSim()->GetLoader()->GetFileVector(aLoadedFiles, APT_ARRAYSIZE(aLoadedFiles));

    if (gSIPlayback.pSavedInputs)
    {
        _playbackSavedInputs(nDeltaTime, eAnimLevels);

    }

    else
    {
        if (GetTargetSim()->GetAnimationTarget()->GetDisplayList()->pState->GetFirstItem() != NULL && GetTargetSim()->GetAnimationTarget()->GetDisplayList()->pState->GetFirstItem()->IsAnimationInst())
        {
            if (_tick(nDeltaTime, eAnimLevels))
            {
                _addScreenGrabToSavedInputs();
            }
        }
        else
        {
            GetTargetSim()->GetLinker()->Update();

            // If there isn't anything to do, inc the tick. This gets around timing issues in loading the first swf and
            // it loading other files. It is a Hack for now until we get time to investigate a better solution. It should
            // pose no real danger.
            gnCurTick++;

            // discard all inputs if we didn't have anywhere to send them.
            // this is slightly wrong, because we shouldn't discard inputs until the next "frame", but since nothings loaded we
            // can't tell when the next frame is. If the user is always calling AptUpdate/AptRender, and just leaving an empty
            // movie always loaded, and passing inputs to Apt, then they expect that input buffers shouldn't overflow
            // (reasonably), so we do this here.
            GetTargetSim()->GetAnimationTarget()->SetQueuedInputsSize(0);
        }
    }

    //  Try to release all the registered objects
    gpValuesToRelease->ReleaseValues();

    //  If needed garbage collect
    if (gbGarbageCollectThisFrame)
    {
        AptGC::CleanUnreachable();
        gbGarbageCollectThisFrame = false;
    }

    APT_ASSERT(gAptActionInterpreter.stack.GetSize() == 0);

#if defined (APT_DO_AS_PROFILE)
    // End action script function profiling for the frame
    gAptActionInterpreter.mFunctionProfiler.LogFrameEnd();
#endif
}

extern void AptDecoupleTreeTraversal(const AptCIH * pCur, int32_t nCurrRenderTick, AptRenderingContext * pRenderingContext, AptMaskRenderOperation eMaskOperation, int nMaskDepth, AptAnimLevelE eAnimLevels, bool bUseDepthCompare);

// (ASK) - after 0.19.01 -added new parameter/s to do selective display of animation levels. This will only work at highest root levels.
static void _AptInternalRender(AptAnimLevelE eAnimLevels)
{
    APT_ASSERT(gbRenderInitialized);

    AptRenderItem::suCurrentRenderAnimLevel = 0;

    if (!GetTargetRender()->GetAnimationTarget()) return;

    AptFilePtr aLoadedFiles[APT_MAX_FILES_LOADED];
    if (GetTargetRender()->GetLoader())
        GetTargetRender()->GetLoader()->GetFileVector(aLoadedFiles, APT_ARRAYSIZE(aLoadedFiles));

    // push unit
    AptMath::ClipStackPushUnit();

    // $fr840 - DC - start render at mask level 0.
    {
        const AptCIH * pCur = GetTargetRender()->GetAnimationTarget()->GetDisplayList()->pState->GetFirstItem();
		// FIFA_BEGIN | SENDBACK | gmicle | 2018-10-23 | https://jira.frostbite.ea.com/browse/FB-108141 - Move performance markers from inside recursive functions to around their entry points, so that they work with Frostbite profiling tools (Delta Viewer).
		APT_PROFILE_BEGIN(APT_DECOUPLE_TREE_TRAVERSAL, "Apt::AptDecoupleTreeTraversal");
        AptDecoupleTreeTraversal(pCur, gnCurrRenderTick, gpRenderingContext, AptMaskRenderOperation_None, 0, eAnimLevels, true);
		APT_PROFILE_END(APT_DECOUPLE_TREE_TRAVERSAL);
		// FIFA_END
	}

    // pop unit
    AptMath::ClipStackPop();

    APT_ASSERT(AptMath::ClipStackIsEmpty());
}

// (ASK) - after 0.19.01 -added new parameter/s to do selective update/tick of animation levels. This will only work at highest root levels.
void AptUpdate(unsigned int nDeltaTime, AptAnimLevelE eAnimLevels)
{
    APT_PROFILE_BEGIN( APT_UPDATE, "Apt::AptUpdate" );
    AptUpdateAutoLock lock;

#ifndef APT_SINGLE_THREADED
    gAptSimThreadId = APT_THREAD_NAMESPACE::GetThreadId();
#endif

    // use fast stack, if possible
    if (AptOptIsEnabled(APT_OPT_SPRSTACK))
    {
        AptFastStack::Begin(2,16*1024);
    }

    // run the update
    _AptInternalUpdate( nDeltaTime
                        ,eAnimLevels );

    if (AptFastStack::IsActive())
    {
        // shutdown fast stack
        APT_ASSERT(AptOptIsEnabled(APT_OPT_SPRSTACK));
        AptFastStack::End();
    }

    APT_PROFILE_END( APT_UPDATE );
}

void AptUpdateRender(AptAnimLevelE eAnimLevels)
{
	APT_PROFILE_BEGIN(APT_UPDATE, "Apt::AptUpdateRender");
	AptUpdateAutoLock lock;

#ifndef APT_SINGLE_THREADED
	// $senzee-lockless write to render list
	if (AptRenderList::Open())
	{
		_AptInternalRender(eAnimLevels);
		AptRenderList::Close();
	}
#endif

	APT_PROFILE_END(APT_UPDATE);
}

// (ASK) - after 0.19.01 -added new parameter/s to do selective display of animation levels. This will only work at highest root levels.
void AptRender(uint32_t nDeltaTimeMS, AptAnimLevelE eAnimLevels)
{
    APT_PROFILE_BEGIN( APT_RENDER, "Apt::AptRender" );
    APT_PROFILE(AptRender);
#ifndef APT_SINGLE_THREADED
    AptRenderList::StartRender();
    AptRenderList::Render(eAnimLevels);
    AptFreeOutstandingCustomControls();
    AptRenderList::EndRender();
#else
    _AptInternalRender(eAnimLevels);
    AptFreeOutstandingCustomControls();
#endif
    APT_PROFILE_END( APT_RENDER );
}

void AptSetMousePosition(int nX, int nY)
{
#if defined(APT_USE_MOUSE)
    if (!gbUpdateInitialized)
    {
        APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, "WARNING: trying to set mouse position when Apt not initalized\n");
        return;
    }

    if (gSIPlayback.pSavedInputs)
        return;

    AptUpdateAutoLock lock;
    if (GetTargetSim()->GetAnimationTarget())
        GetTargetSim()->GetAnimationTarget()->AddInput(nX, nY);
#else
    APT_ASSERT(false && "MOUSE NOT SUPPORTED, define APT_USE_MOUSE to use the mouse");
#endif
}

#if defined(APT_ALTERNATE_INPUT)
void AptAddAlternateInput(const char *szEvent, AptValue *pValue)
{
    if (!gbUpdateInitialized)
    {
        APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, "WARNING: trying to add input when Apt not initalized\n");
        return;
    }

    AptUpdateAutoLock lock;
    if (GetTargetSim()->GetAnimationTarget())
    {
        AptAltInput input;
        input.sEvent = szEvent;
        input.pValue = pValue;
        GetTargetSim()->GetAnimationTarget()->AddAlternateInput(&input);
    }
}
#endif

void AptAddToInputQueue(AptInputType eInput, AptInputState eState, AptInputController eController)
{
    if (!gbUpdateInitialized)
    {
        APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, "WARNING: trying to add input when Apt not initalized\n");
        return;
    }

    if (gSIPlayback.pSavedInputs)
        return;

    AptUpdateAutoLock lock;
    if (GetTargetSim()->GetAnimationTarget())
    {
        GetTargetSim()->GetAnimationTarget()->AddInput(eInput, eState, eController);
    }
}

// added in 0.13.00
void AptAddToInputAnalogQueue(AptAnalogStickInfo pAnalogInput)
{
    if (!gbUpdateInitialized)
    {
        APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, "WARNING: trying to add input when Apt not initalized\n");
        return;
    }

    if (gSIPlayback.pSavedInputs)
        return;

    AptUpdateAutoLock lock;
    if (GetTargetSim()->GetAnimationTarget())
        GetTargetSim()->GetAnimationTarget()->AddAnalogInput(pAnalogInput);
}

void AptAddToInputGestureQueue(AptGestureInfo pGestureInput)
{
    if (!gbUpdateInitialized)
    {
        APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, "WARNING: trying to add input when Apt not initalized\n");
        return;
    }

    if (gSIPlayback.pSavedInputs)
        return;

    AptUpdateAutoLock lock;
    if (GetTargetSim()->GetAnimationTarget())
        GetTargetSim()->GetAnimationTarget()->AddGestureInput(pGestureInput);
}

void AptSetInternalVariable(const char *szName, const char *szValue)
{
    AptUpdateAutoLock lock;

    AptString *pStr = AptString::Create();
    APT_INC(pStr);
    pStr->cpy(szValue);
    AptNativeString strName(szName);
    gAptActionInterpreter.setVariable(_AptGetAnimationAtLevel(0), 0, &strName, pStr);
    APT_DEC(pStr);
}

void AptGetInternalVariable(const char *szName, char *szValue)
{
    AptUpdateAutoLock lock;

    AptNativeString strName(szName);
    AptValue *pVal = gAptActionInterpreter.getVariable(_AptGetAnimationAtLevel(0), 0, &strName);
    APT_INC(pVal);
    pVal->toString(szValue);
    APT_DEC(pVal);
}

void AptGetInternalVariable(const AptNativeString* szName, char *szValue)
{
    AptUpdateAutoLock lock;

    AptValue *pVal = gAptActionInterpreter.getVariable(_AptGetAnimationAtLevel(0), 0, szName);
    APT_INC(pVal);
    pVal->toString(szValue);
    APT_DEC(pVal);
}

// $senzee Factored this out to allow calling a member function on pThis
static void AptCallMemberFunctionInternal(const char *szName, char *szReturnValue, AptValue *pThis, int nNumParams, AptValue **apArguments)
{
    if (!pThis)
    {
        pThis = _AptGetAnimationAtLevel(0);
    }
    APT_ASSERT(pThis);

    // BTurkelson 03/2010 - Only do anything if the object whose member function we're calling is actually there.
    if(pThis->getIsDefined())
    {
        for (int i = nNumParams - 1; i >= 0; i--)
        {
            gAptActionInterpreter.stackPush(apArguments[i]);
        }

        AptNativeString strName(szName);
        AptValue *pFuncValue = gAptActionInterpreter.getVariable(pThis, 0, &strName, 1);

#if APT_PROF_ENABLED && APT_PROFILE_ACTIONSCRIPT_ENABLED
        APT_PROF_ENTER((char8_t *)szName);
#endif
        // $bug779 - MCB - Save function state information before execution.
        void* pFuncState = AptScriptFunctionBase::PushStaticData();

#ifdef APT_DEBUGGER_ENABLE
        bool isScriptActionFlag = AptDebugger::GetInstance()->PushCallStack(pFuncValue, szName, reinterpret_cast<unsigned char* const>(AptDebugger::INVALID_OFFSET), pThis);
#endif

        // Fix for one of the "Super Bugs" from BTurkelson
        // it was pulling member var values from pThis's prototype
        // instead of pThis itself
        gAptActionInterpreter.withStack.push(pThis);

        // See comment about FindSuperImplementor in  AptActionInterpreter::_FunctionAptActionCallMethod.
        // This really should be refactored. AptCallMemberFunctionInternal should behave more like _FunctionAptActionCallMethod.
        AptValue* pOverrideSuper = NULL;
        if ( !strName.Equal( *StringPool::GetString(SC_super) ) )
        {
            // search up the __proto__ chain until we find a prototype that implements strName.
             AptValue* pProto = AptActionInterpreter::FindSuperImplementor( pThis, &strName );
             if ( pProto )
             {
                // pProto implements the given method.  Set pOverrideSuper
                // to the superclass of pProto.
                pOverrideSuper = AptActionInterpreter::GetNextProto( pProto );
             }
        }
        gAptActionInterpreter.callFunction(pThis, pFuncValue, nNumParams, NULL, pOverrideSuper);

        // Fix for one of the "Super Bugs" from BTurkelson
        // it was pulling member var values from pThis's prototype
        // instead of pThis itself
        gAptActionInterpreter.withStack.pop();

#ifdef APT_DEBUGGER_ENABLE
        if (isScriptActionFlag)
        {
            AptDebugger::GetInstance()->PopCallStack(pFuncValue);
        }
#endif

        // $bug779 - MCB - And aaawaaay we go.
        AptScriptFunctionBase::PopStaticData(pFuncState);

#if APT_PROF_ENABLED && APT_PROFILE_ACTIONSCRIPT_ENABLED
        APT_PROF_LEAVE((char8_t *)szName);
#endif

        if (szReturnValue)
        {
            AptValue *pVal = gAptActionInterpreter.stackAt(0);
            pVal->toString(szReturnValue);
        }
        gAptActionInterpreter.stackPop();
}
}

static void AptCallFunctionInternal(const char *szName, char *szReturnValue, const char *szThisObject, int nNumParams, AptValue **apArguments)
{
    AptValue *pThis = 0;

    if (szThisObject != 0)
    {
        AptNativeString strObject(szThisObject);
        pThis = gAptActionInterpreter.getVariable(_AptGetAnimationAtLevel(0), 0, &strObject, 1);
    }

    AptCallMemberFunctionInternal(szName, szReturnValue, pThis, nNumParams, apArguments);
}

void AptCallFunctionV(const char *szName, char *szReturnValue, const char *szThisObject, int nNumParams, va_list &varargs)
{
    AptUpdateAutoLock lock;

    AptValue *apStrings[32];
    APT_ASSERT(nNumParams >= 0 && nNumParams < APT_ARRAYSIZE(apStrings));

    for (int i = 0; i < nNumParams; i++)
    {
        //lint --e(119) PcLint warning Error 119: Too many arguments (1) for prototype 'AptString::Create(void)' (on ps3)
        apStrings[i] = AptString::Create(va_arg(varargs, char *));
    }

    AptCallFunctionInternal(szName, szReturnValue, szThisObject, nNumParams, apStrings);
}

void AptCallFunction(const char *szName, char *szReturnValue, const char *szThisObject, int nNumParams, ...)
{
    AptUpdateAutoLock lock;

    va_list varargs;
    va_start( varargs, nNumParams );
    AptCallFunctionV(szName, szReturnValue, szThisObject, nNumParams, varargs);
    va_end(varargs);
}


void AptCallFunctionVOpti(const char *szName, char *szReturnValue, const char *szThisObject, int nNumParams, va_list &varargs)
{
    AptUpdateAutoLock lock;

    AptValue *apValues[32];
    APT_ASSERT(nNumParams >= 0 && nNumParams < APT_ARRAYSIZE(apValues));

    for (int i = 0; i < nNumParams; i++)
    {
        //lint --e(64) PcLint warning Error 64: Type mismatch (assignment) (AptValue * = int)
        apValues[i] = va_arg(varargs, AptValue *);
    }

    AptCallFunctionInternal(szName, szReturnValue, szThisObject, nNumParams, apValues);
}

void AptCallFunctionOpti(const char *szName, char *szReturnValue, const char *szThisObject, int nNumParams, ...)
{
    AptUpdateAutoLock lock;

    va_list varargs;
    va_start( varargs, nNumParams );
    AptCallFunctionVOpti(szName, szReturnValue, szThisObject, nNumParams, varargs);
    va_end(varargs);
}

void AptCallFunctionAOpti(const char *szName, char *szReturnValue, const char *szThisObject, int nNumParams, AptValue **apValues)
{
    AptUpdateAutoLock lock;
    AptCallFunctionInternal(szName, szReturnValue, szThisObject, nNumParams, apValues);
}

void AptCallMemberFunctionV(const char *szName, char *szReturnValue, AptValue *pThis, int nNumParams, va_list &varargs)
{
    AptUpdateAutoLock lock;

    AptValue *apValues[32];
    APT_ASSERT(nNumParams >= 0 && nNumParams < APT_ARRAYSIZE(apValues));

    for (int i = 0; i < nNumParams; i++)
    {
        //lint --e(64) PcLint warning Error 64: Type mismatch (assignment) (AptValue * = int)
        apValues[i] = va_arg(varargs, AptValue *);
    }

    AptCallMemberFunctionInternal(szName, szReturnValue, pThis, nNumParams, apValues);
}

void AptCallMemberFunction(const char *szName, char *szReturnValue, AptValue *pThis, int nNumParams, ...)
{
    AptAssertIsSimulationThread();

    va_list varargs;
    va_start( varargs, nNumParams );
    AptCallMemberFunctionV(szName, szReturnValue, pThis, nNumParams, varargs);
    va_end(varargs);
}


/*F************************************************************************************************/
/*!
    AptCallFunctionObjectInternal

    Calls the given actionscript function without scope.  Pops the return value off the stack
    and returns it.

    \Input      pFuncValue - pointer to the function object as an AptValue
                numParams - how many params are in values
                values - array of AptValues to use as parameters to the function

    \Output     Value returned by the function. Please APT_DEC it when you are done with it!

    \Notes
        None.

*/
/************************************************************************************************F*/
static AptValue * AptCallFunctionObjectInternal( AptValue *pFuncValue,
                                           int numParams,
                                           AptValue **values )
{
    for (int i = numParams - 1; i >= 0; i--)
    {
        gAptActionInterpreter.stackPush(values[i]);
    }

#if APT_PROF_ENABLED && APT_PROFILE_ACTIONSCRIPT_ENABLED
    char8_t * profilingName = AptGetFunctionProfilingName(pFuncValue, "AptCallFunctionObjectInternal");
    APT_PROF_ENTER(profilingName);
#endif

    void* pFuncState = AptScriptFunctionBase::PushStaticData();
    gAptActionInterpreter.callFunction(gpUndefinedValue, pFuncValue, numParams, NULL, NULL);
    AptScriptFunctionBase::PopStaticData(pFuncState);

#if APT_PROF_ENABLED && APT_PROFILE_ACTIONSCRIPT_ENABLED
    APT_PROF_LEAVE((char8_t *)profilingName);
#endif

    return gAptActionInterpreter.stackGetPop();
}

/*F************************************************************************************************/
/*!
    AptCallFunctionObjectV

    Calls the given actionscript function without scope.  Pops the return value off the stack
    and returns it.

    \Input      pFuncValue - pointer to the function object as an AptValue
                nNumParams - how many param you are sending to the function
                varargs - the params to give the function (should all be pointers to AptValues)

    \Output     Value returned by the function. Please APT_DEC it when you are done with it!

    \Notes
        None.

*/
/************************************************************************************************F*/
static AptValue * AptCallFunctionObjectV(AptValue *pFuncValue, int nNumParams, va_list &varargs)
{
    AptUpdateAutoLock lock;

    AptValue *apValues[32];
    APT_ASSERT(nNumParams >= 0 && nNumParams < APT_ARRAYSIZE(apValues));

    for (int i = 0; i < nNumParams; i++)
    {
        apValues[i] = va_arg(varargs, AptValue *);
    }

    return AptCallFunctionObjectInternal(pFuncValue, nNumParams, apValues);
}

/*F************************************************************************************************/
/*!
    AptCallFunctionObject

    Calls the given actionscript function without scope.  Pops the return value off the stack
    and returns it.

    \Input      pFuncValue - pointer to the function object as an AptValue
                nNumParams - how many param you are sending to the function
                ... - the params to give the function (should all be pointers to AptValues)

    \Output     Value returned by the function. Please APT_DEC it when you are done with it!

    \Notes
        None.

*/
/************************************************************************************************F*/
AptValue * AptCallFunctionObject(AptValue *pFuncValue, int nNumParams, ...)
{
    AptUpdateAutoLock lock;

    va_list varargs;
    va_start( varargs, nNumParams );
    AptValue * retVal = AptCallFunctionObjectV(pFuncValue, nNumParams, varargs);
    va_end(varargs);
    return retVal;
}

/******************************************************************************/
/*!
    \brief  Call a function object you already have a hold on

    \param  funcValue Function to call
    \param  numParams Number of parameters being passed
    \param  params    Array of those parameters

    \return           - Return value of the function
*/
/******************************************************************************/
AptValue* AptCallFunctionObjectA(AptValue * funcValue, int numParams, AptValue * * params)
{
    AptUpdateAutoLock lock;
    return AptCallFunctionObjectInternal(funcValue, numParams, params);
}

bool AptSetInputRoot(const char *szRoot)
{
    AptUpdateAutoLock lock;

    // if szRoot == NULL, means want to disable the input masking
    if(szRoot == 0)
    {
        GetTargetSim()->GetAnimationTarget()->SetInputMask(0);
        return true;
    }

    // try to find the CIH which szRoot is specifying
    // if can't find just object, just report operation fail
    AptValue *      pTarget;
    AptNativeString strRoot(szRoot);
    pTarget = gAptActionInterpreter.getVariable(_AptGetAnimationAtLevel(0), NULL, &strRoot);
    if (pTarget == 0 || !(pTarget->isCIH()))
    {
        return false;
    }

    AptCIH* pCIH = 0;
    pCIH =pTarget->c_cih();

    // set the mask
    GetTargetSim()->GetAnimationTarget()->SetInputMask(pCIH);

    return true;
}


int AptDebugGetCurrentFrame(int nLevel)
{

    AptCIH *pCIH = _AptGetAnimationAtLevel(static_cast<int32_t>(nLevel));
    if (pCIH && pCIH->IsAnimationInst())
    {
        return pCIH->GetAnimationInst()->mnFrame;
        //if (pCIH->IsAnimationInst())
        //{
        //    return pCIH->GetAnimationInst()->mnFrame;
        //}
        //else
        //    return 0 ;
    }
    else
    {
        return -1;
    }
}

int AptDebugGetNumFrames(int nLevel)
{

    AptCIH *pCIH = _AptGetAnimationAtLevel(static_cast<int32_t>(nLevel));
    if (pCIH)
    {
        return pCIH->GetAnimationInst()->GetCharacterConst()->sprite.movie.nFrames;
    }
    else
    {
        return 0;
    }
}

int AptDebugIsPlaying(int nLevel)
{

    AptCIH *pCIH = _AptGetAnimationAtLevel(static_cast<int32_t>(nLevel));
    if (pCIH && pCIH->IsAnimationInst())
    {
        return pCIH->GetIsPlaying();
    }
    else
    {
        return -1;
    }
}


void AptDebugEnableSavedInputs(int bEnabled)
{


    gbSavedInputsEnabled = bEnabled;
    gnCurTick = 0;
}

void AptDebugPlaySavedInputs(AptSavedInputRecord *aSavedInputs, int nSavedInputsFileSize)
{
// commented because threadid is not yet set. It gets set from AptInitialize.
//
    gSIPlayback.pSavedInputs = (unsigned char *)aSavedInputs;
    gSIPlayback.pCurSavedInput = gSIPlayback.pSavedInputs;
    gSIPlayback.nInputFileSize = nSavedInputsFileSize;
    gSIPlayback.nCurTick = 0;
}

void AptDebugStopSavedInputs(void)
{
    APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, "Playing of saved inputs stopped before completion\n");
    gSIPlayback.pSavedInputs = NULL;
    gbSavedInputsEnabled = 0;
    gAptFuncs.pfnPlaySavedInputsDone(true, "Playback Stopped.");
}


/*F*************************************************************************************************

    Apt_atoff

    Description:

    Inputs:
        const char *    _s              -

    Outputs:
        int                             -

    Notes:
        None.

    History:
        Ver.    Description
        1.0     06/17/00 (RJS) Ported from LLAPI so that we have don't do double conversions on the ps2.

*************************************************************************************************F*/

#define __SPEED__
float Apt_atoff(const char *_s)
{
#if defined(EA_PLATFORM_PLAYSTATION2)    || defined(EA_PLATFORM_PSP)
    float val, holder;
    int floatsign, expsign, exp, i;
    const char *s = _s;
#ifdef __SPEED__
    static const float pow10[] =
        {1.0e-1, 1.0e-2, 1.0e-3, 1.0e-4, 1.0e-5, 1.0e-6, 1.0e-7, 1.0e-8, 1.0e-9,
         1.0e-10, 1.0e-11, 1.0e-12, 1.0e-13, 1.0e-14, 1.0e-15, 1.0e-16, 1.0e-17};
    const float *dp;
#endif // __SPEED__

    // scan past blanks
    while (isspace(*s))
    {
        ++s;
    }

    floatsign = 1;  // assume positive

    if ((i = *s) == '+')
    {
        ++s;
    }
    else if (i == '-')
    {
        ++s;
        floatsign = 0;
    }

    for (val = 0 ; isdigit(i = *s) ; ++s)
    {
        val = val * 10 + (i - '0');
    }

    if (*s == '.')
    {
        ++s;
#ifdef __SPEED__
        for (dp = pow10, holder = *dp; isdigit(i = *s) && dp < &pow10[sizeof pow10 / sizeof pow10[0]]; ++s, holder = *++dp)
#else
        for (holder = 0.1f ; isdigit(i = *s) ; ++s, holder *= 0.1f)
#endif // __SPEED__
        {

            val = val + holder * (i - '0');
        }
    }

    while (isdigit(*s)) // scan past insignificant digits
    {
        ++s;
    }

    if (tolower(*s) == 'e')
    {
        expsign = 1;
        ++s;
        if (!isdigit(i = *s))
        {
            if (i == '-')
            {
                expsign = -1;
            }

            ++s;
        }

        for (exp = 0; isdigit(i = *s) ; ++s )
        {
            exp = 10 * exp + (i - '0');
        }

        if (expsign == -1)
        {
            exp = exp - (2 * exp);
        }

        while (exp < 0)
        {
            val=val / 10;
            ++exp;
        }

        while (exp > 0)
        {
            val=val * 10;
            --exp;
        }
    }

    return floatsign ? val : -val;
#else
    return (float) (atof(_s));
#endif
}



void AptDeallocAllAssetString()
{

    if(GetTargetSim()->GetAnimationTarget())
    {
        GetTargetSim()->GetAnimationTarget()->GetDisplayList()->DeallocAssetStringRecursive();
    }
}


void AptPrintAllDynamicText(AptAnimLevelE eAnimLevels)
{

    if(GetTargetSim()->GetAnimationTarget())
    {
        uint32_t (*pOldValue) (AptCIH * pCIH, void * pVoid) = AptCIH::sCIHProcessCb;
        bool bOldEarlyReturn = AptCIH::bEarlyReturn;
        AptCIH::bEarlyReturn = false;
        AptCIH::sCIHProcessCb = AptCIH::ProcessTextInstPrint ;
		// FIFA_BEGIN | SENDBACK | gmicle | 2018-10-23 | https://jira.frostbite.ea.com/browse/FB-108141 - Move performance markers from inside recursive functions to around their entry points, so that they work with Frostbite profiling tools (Delta Viewer).
		APT_PROFILE_BEGIN(APT_DL_GENERALISED_PROCESS, "AptDisplayList::GeneralisedProcess");
        AptDisplayList::GeneralisedProcess(GetTargetSim()->GetAnimationTarget()->GetDisplayList(), NULL, eAnimLevels, true) ;
		APT_PROFILE_END(APT_DL_GENERALISED_PROCESS);
		// FIFA_END
		AptCIH::sCIHProcessCb = pOldValue ;
        AptCIH::bEarlyReturn = bOldEarlyReturn ;
    }
}


void AptPrintMovieclipTree(AptAnimLevelE eAnimLevels)
{

    if(GetTargetSim()->GetAnimationTarget())
    {
        APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, "<?xml version=\"1.0\" encoding=\"utf-8\" ?>\n");
#if defined (APT_GATHER_MOVIECLIP_METRICS)
        APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, "<Apt_MovieclipTree Animations=\"%d\" Movieclips=\"%d\" DynamicText=\"%d\" Shapes=\"%d\" CustomControls=\"%d\" StaticText=\"%d\" Buttons=\"%d\">\n",
            gAptMovieclipInformation.nAnimations,
            gAptMovieclipInformation.nMovieClips,
            gAptMovieclipInformation.nDynamicText,
            gAptMovieclipInformation.nShapes,
            gAptMovieclipInformation.nCustomControls,
            gAptMovieclipInformation.nStaticText,
            gAptMovieclipInformation.nButtons);
#else
        APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, "<Apt_MovieclipTree warning=\"Please define APT_GATHER_MOVIECLIP_METRICS in AptDefine.h to get correct number of movieclips printed here, tree below is good though\">\n");
#endif
        APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, "<Message Note=\"Please look in AptCharacter.h file to see the enums for movieclip types\"/>\n");
        uint32_t (*pOldValue) (AptCIH * pCIH, void * pVoid) = AptCIH::sCIHProcessCb;
        bool bOldEarlyReturn = AptCIH::bEarlyReturn;
        AptCIH::bEarlyReturn = false;
        AptCIH::sCIHProcessCb = AptCIH::PrintMovieclipTree ;
		// FIFA_BEGIN | SENDBACK | gmicle | 2018-10-23 | https://jira.frostbite.ea.com/browse/FB-108141 - Move performance markers from inside recursive functions to around their entry points, so that they work with Frostbite profiling tools (Delta Viewer).
		APT_PROFILE_BEGIN(APT_DL_GENERALISED_PROCESS, "AptDisplayList::GeneralisedProcess");
        AptDisplayList::GeneralisedProcess(GetTargetSim()->GetAnimationTarget()->GetDisplayList(), NULL, eAnimLevels, true) ;
		APT_PROFILE_END(APT_DL_GENERALISED_PROCESS);
		// FIFA_END
		AptCIH::sCIHProcessCb = pOldValue ;
        AptCIH::bEarlyReturn = bOldEarlyReturn ;
        APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, "</TreeDepth0>\n");
        APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, "</Apt_MovieclipTree>\n");
    }
}


// FIFA_BEGIN | DIVERGENCE | nmeyer | 2020-02-03 | Add support for dumping display lists. Useful for debugging and tracking memory leaks in the UI.
void AptDumpDisplayList( const char* szFileName )
{
	// Grab the root display object and print out its tree to the specified file
	if(GetTargetSim()->GetAnimationTarget())
    {
		GetTargetSim()->GetAnimationTarget()->GetDisplayList()->dumpDisplayList(szFileName);
	}
}
// FIFA_END


// This is provided only in debug mode right now, just to keep static size small, but can be turned on in release mode.
void AptPrintLoadedSWFFiles(bool bPrintImports, bool bPrintExports)
{
#if defined (EA_DEBUG) || defined (APT_PRINT_LOADED_SWFS)

    if(GetTargetSim()->GetLinker())
    {
        APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, "<?xml version=\"1.0\" encoding=\"utf-8\" ?>\n");
        APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, "<Apt_SWF_Loaded>\n");
        APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, "<Message Note=\"FileState enums are 0:Invalid, 1:Queued, 2:WaitingForData, 3:WaitingForImports, 4:Resolved, 5:Zombie, 6:Unloaded\"/>\n");
        GetTargetSim()->GetLinker()->PrintLoadedFiles(bPrintImports, bPrintExports);
        APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, "</Apt_SWF_Loaded>\n");
    }
#endif
}



// this will change the bSkipTraceBytecodes variable from AptActionInterpreter
// Look in Apt.h for more help.
void AptSetSkipTracesFlag(bool bSkipTraces)
{
    gAptActionInterpreter.mbSkipTraceBytecodes = bSkipTraces;
}


IAptXmlImpl * gpAptXmlImpl = NULL ;

void AptSetXMLImplementor(IAptXmlImpl * pIAptXmlImpl)
{

    if (pIAptXmlImpl)
    {
        gpAptXmlImpl = pIAptXmlImpl ;
    }
}

// given by maxis guys. use of this function can be at following places.
// -At the first line of AptAux..._LoadAnimation()
// -Each of our flash movies has an Init() function that is always the first function called when the movie is loaded.
// -At the end of this function there is a call to "extern._r".  This _r message tells us the movie is "ready".  When the
//  engine side recieves this ready message, this function is called.
// -We also added an "extern._f" message that flash could call when it wanted to flush the input queue.
// Basically we call this function when the load of a movie starts, and when we consider the load of a movie to be finished.

void AptFlushInputQueue(void)
{

    APT_ASSERT(gbUpdateInitialized);

    if (gSIPlayback.pSavedInputs)
        return;

    if (!GetTargetSim()->GetAnimationTarget())
        return;

    GetTargetSim()->GetAnimationTarget()->SetQueuedInputsSize(0);
#if defined(APT_ALTERNATE_INPUT)
    GetTargetSim()->GetAnimationTarget()->SetAltQueuedInputsSize(0);
#endif
}

// FR #360 Mouse Over Button detection - Scott Bowen not tested fully
bool AptIsMouseOverButton(void)
{

    #if defined(APT_USE_MOUSE) && defined(APT_USE_BUTTONS)
    return GetTargetSim()->GetAnimationTarget()->GetFocusButton() != 0;
    #else
    APT_ASSERT(false && "MOUSE NOT SUPPORTED, define APT_USE_MOUSE to use the mouse");
    return false;
    #endif
}

/*F************************************************************************************************/
/*!
    \Function    AptRegisterExtension

    \Description
        Registers an Apt C++ extension object

    \Input      *pExtObject       - a pointer to the object

    \Output
        None.

    \Notes
        None.

    \Version    1.0        04/03/04 (JSH) First Version
    \Version    1.1        05/09/05 (DC)  Now ThreadSafe
    \Version    1.2        02/05/08 (RB)  Handle instantiable AptExtObjects. Most of the work is
                                          actually performed by AptExtObjectRegistry.

*/
/************************************************************************************************F*/
void AptRegisterExtension(AptExtObject *pExtObject)
{
    AptUpdateAutoLock lock;

    APT_ASSERT(pExtObject != NULL);

    pExtObject->Initialize();

    AptExtObjectRegistry::Register(pExtObject);
}

/*F************************************************************************************************/
/*!
\Function    AptUnRegisterExtension

\Description
UnRegisters an Apt C++ extension object

\Input      *pExtObject       - a pointer to the object

\Output
None.

\Notes
None.

\Version    1.0        03/03/05 (DUG) First Version
    \Version    1.1        04/15/08 (RB)  Handle instantiable extension objects. Unregister by
                                          pointer.
*/
/************************************************************************************************F*/
void AptUnRegisterExtension(AptExtObject *pExtObject)
{
    AptUpdateAutoLock lock;

    APT_ASSERT(pExtObject != NULL);

    AptExtObjectRegistry::UnRegister(pExtObject->GetName());
}

void AptStackPush(AptValue * const pValue)
{
    gAptActionInterpreter.stackPush(pValue);
}

void AptStackPop(const int32_t nItems)
{
    gAptActionInterpreter.stackPop(nItems);
}

/*F************************************************************************************************/
/*!
\Function    AptUnRegisterExtension

\Description
UnRegisters an Apt C++ extension object

\Input      *pExtObject       - a pointer to the object

\Output
None.

\Notes
None.

\Version    1.0        03/03/05 (DUG) First Version
    \Version    1.1        04/15/08 (RB)  Handle instantiable extension objects. Unregister by
                                          name.

*/
/************************************************************************************************F*/
void AptUnRegisterExtension(const char *pExtObjectName)
{
    AptUpdateAutoLock lock;

    APT_ASSERT(pExtObjectName != NULL);

    AptExtObjectRegistry::UnRegister(pExtObjectName);
}


/*F************************************************************************************************/
/*!
    \Function    AptBreakAfterAssert

    \Description
        This functionality was removed. Please don't call it :)

    \Input      bBreakAfterAssert -

    \Output
        None.

    \Notes
        None.

    \Version    1.0        07/18/05 (DC) Bringin' it to ya

*/
/************************************************************************************************F*/
void AptBreakAfterAssert(int bBreakAfterAssert)
{
    if(bBreakAfterAssert != 0)
    {
        APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, "AptBreakAfterAssert function is deprecated break in Aux library Assert Fail call if desired.\n");
    }
}

/*F************************************************************************************************/
/*!
    Sets a Garbage Collection to be called at the end of the AptUpdate. We only set a flag here.

    \Input      None.

    \Output     None.

    \Notes
        This is not a full Garbage Collection, only the cyclic and/or leaked object will be deleted.

    \Version
        1.0        03/23/04 (ONA) First Version

*/
/************************************************************************************************F*/
void    AptPartialGarbageCollection()
{

    gbGarbageCollectThisFrame = true;
}

/*F************************************************************************************************/
/************************************************************************************************F*/
//(rrv) AS2
static int32_t nSwfVersion = 0;
//
void    AptSetSwfVersion(int32_t p_version)
{
    nSwfVersion = p_version;
}
//
/*F************************************************************************************************/
/************************************************************************************************F*/
int32_t     AptGetSwfVersion()
{
    return nSwfVersion;
}

/*F************************************************************************************************/
/*!
    \Function    AptCheckZombieVector

    \Description
        When called, this function will output the current status of the zombie vector.

    Insert your long description here.

    \Input
        None.

    \Output
        None.

    \Notes
        None.

    \Version    1.0        08/04/04 (LEO) First Version     $BUG253 LEO    5/47

*/
/************************************************************************************************F*/
void AptDebugPrintZombieVector()
{
#if defined(APT_ENABLE_ZOMBIE_OUTPUT)
    if(gpZombieVector == NULL)
    {
        return;
    }
    AptCIH *pCIH;
    int nNumVals = gpZombieVector->GetNumValues() - 1;
    APT_DEBUGPRINT(APT_DEBUG_MSG_DEBUG_LVL, "ZOMBIE VECTOR HAS %d ITEMS IN IT\n", nNumVals + 1);
    for(int i = nNumVals; i >= 0; i--)
    {
        pCIH = gpZombieVector->GetAt(i)->c_cih();
        AptCharacterAnimationInst *pAnimation = pCIH->GetAnimationInst();
        APT_DEBUGPRINT(APT_DEBUG_MSG_DEBUG_LVL, "%d :: ZOMBIE SPRITE[name %s  file = %s.swf] HAS %d EXTERNAL REFERENCES\n",
            i + 1,
            pCIH->GetInstanceName().c_str(),
            pAnimation->mpFile.Get()->GetName().c_str(),
            pCIH->GetZombieCount());
    }
#endif
}

/*F************************************************************************************************/
/*!
    \Function    AptUpdateZombieVector

    \Description
        This function checks the zombie vector for any zombie sprites ready to die

    Insert your long description here.

    \Input
        None.

    \Output
        None.

    \Notes
        None.

    \Version    1.0        07/30/04 (LEO) First Version         $BUG253 LEO    6/47
        -253 Flow broken on backward : http://globaltechdocs.ea.com/MediaWiki/index.php?title=Apt:External_function_references
                1.1        02/07/05 (LEO) -$bug815: Index out of bounds assert due to empty Zombie vector.

*/
/************************************************************************************************F*/
void AptUpdateZombieVector(bool bClean)
{
    if(gpZombieVector == NULL)
    {
        return;
    }

    AptUpdateAutoLock lock;

    AptCIH *pCIH;
    int nNumVals = gpZombieVector->GetNumValues() - 1;
    for(int i = nNumVals; i >= 0; i--)
    {
        //Check if previous iteration freed
        //elements from zombie vector.
        if (gpZombieVector->GetNumValues()<=i)        // $bug815: Index out of bounds assert due to empty Zombie vector.
        {
            break;
        }
        pCIH = gpZombieVector->GetAt(i)->c_cih(true);
        if(pCIH->GetCIHState() == AptCIH::AptCIHState_Zombie && (bClean || pCIH->GetZombieCount() == 0))
        {
            gpZombieVector->RemoveAt(i);
            if (pCIH->GetCharacterInst() == NULL)
            {
                // that means that the CIH is cleared out already.
                continue;
            }
            AptCharacterAnimationInst *pAnimation = pCIH->GetAnimationInst();
            pCIH->SetCIHState(AptCIH::AptCIHState_Normal);
            pAnimation->mpFile.Get()->setState(AptFile::Resolved);
// FIFA_BEGIN | SENDBACK | mwebb | Mar 6 2015 | change to allow PrintZombieDump to be an int
//#if defined(APT_ENABLE_ZOMBIE_OUTPUT)
//			if (bPrintZombieReferences)
#if defined(APT_XML_MEMORY_DUMP_SUPPORTED)
			if(iPrintZombieReferences > 0)
// FIFA_END
			{
				if(bClean == false && pAnimation->mpFile.Get()->mRefCount > 2)
				{
					// Something else is holding onto the animation's file, so only kill the zombied sprite.  Cases for this would be that the zombied animation was reloaded.
	#if defined(DO_COVERAGE)
					APT_DEBUGPRINT(APT_DEBUG_MSG_WARNING_LVL, "ZOMBIE CLEARED [name = %s  file = %s.swf] NO LONGER HAS EXTERNAL FUNCTION REFERENCES\n", 
	#else
					APT_DEBUGPRINT(APT_DEBUG_MSG_WARNING_LVL, "ZOMBIE CLEARED [%x  name = %s  file = %s.swf] NO LONGER HAS EXTERNAL FUNCTION REFERENCES\n", 
						pCIH,
	#endif
						pCIH->GetInstanceName().c_str(),
						pAnimation->mpFile.Get()->GetName().c_str());
					if(gAptFuncs.pfnHandleZombieState != NULL)
						gAptFuncs.pfnHandleZombieState(false, true, pCIH->GetInstanceName().c_str(), pAnimation->mpFile.Get()->GetName().c_str());
					pAnimation->mpFile = AptFilePtr(0);
				}
				else
				{
					// Here, the zombied animation and file can be release.
	#if defined(DO_COVERAGE)
					APT_DEBUGPRINT(APT_DEBUG_MSG_WARNING_LVL, "ZOMBIE CLEARED [name = %s  file = %s.swf] NO LONGER HAS EXTERNAL FUNCTION REFERENCES\n",
	#else
					APT_DEBUGPRINT(APT_DEBUG_MSG_WARNING_LVL, "ZOMBIE CLEARED [%x  name = %s  file = %s.swf] NO LONGER HAS EXTERNAL FUNCTION REFERENCES\n",
						pCIH,
	#endif
						pCIH->GetInstanceName().c_str(),
						pAnimation->mpFile.Get()->GetName().c_str());

                if(gAptFuncs.pfnHandleZombieState != NULL)
                    gAptFuncs.pfnHandleZombieState(false, true, pCIH->GetInstanceName().c_str(), pAnimation->mpFile.Get()->GetName().c_str());
                bool bLastRef = false;

                if(pAnimation->mpFile.Get()->mRefCount == 1)
                {
                    bLastRef = true;
                }
                pAnimation->mpFile = AptFilePtr(0);
                if(bLastRef)        // If this is the last reference, we have to null out the mpCharacter pointer
                {
                    pAnimation->GetRenderItemWritable()->SetCharacter(NULL);
                }
            }
            }
#endif
            pCIH->setGCRoot(0);

            pCIH->DestroyGCPointers();

            AptPartialGarbageCollection();              // TO allow a clean removal of this zombie sprite, we need the garbage collecter to do it.
        }
    }
}

/*F************************************************************************************************/
/*!
    \Function    AptRemoveCIHFromZombieVector

    \Description
        This function removes a given CIH from the zombie vector

    \Input
        pTarget           - A pointer to the CIH to potentially remove

    \Return
        True if the CIH was found and removed, false otherwise
*/
/************************************************************************************************F*/
bool AptRemoveCIHFromZombieVector(AptCIH* pTarget)
{
    AptUpdateAutoLock lock;
    if(gpZombieVector != NULL)
    {
        for ( int i = gpZombieVector->GetNumValues() - 1; i >= 0; i-- )
        {
            if (pTarget == gpZombieVector->GetAt(i)->c_cih(true))
            {
                gpZombieVector->RemoveAt(i);
                return true;
            }
        }
    }
    return false;
}

/*F************************************************************************************************/
/*!
    \Function    AptGetMouseOverSpriteName

    \Description
        Returns the instance name of the button or movieclip under the current mouse position

    Insert your long description here.

    \Input      *szName           - Is set to the movieclip's or button's name

    \Output
        None.

    \Notes
        - (LEO) $FR630 : Need to determine the name of the button that the mouse is over

    \Version    1.0        08/24/04 (LEO) First Version

*/
/************************************************************************************************F*/
void AptGetMouseOverSpriteName(char *szName)
{

#if defined(APT_USE_MOUSE)
#if defined APT_USE_BUTTONS
    if(GetTargetSim()->GetAnimationTarget()->GetFocusButton() != NULL)                // If this is not NULL then we have a button inst under the mouse cursor
    {
        GetTargetSim()->GetAnimationTarget()->GetFocusButton()->toString(szName);
    }
    else
#endif
    if(GetTargetSim()->GetAnimationTarget()->GetTopMostSprite() && GetTargetSim()->GetAnimationTarget()->GetTopMostSprite()->isCIH() && !GetTargetSim()->GetAnimationTarget()->GetTopMostSprite()->c_cih()->GetInstanceName().IsEmpty())    // Else there might be a moveclip inst
    {
        GetTargetSim()->GetAnimationTarget()->GetTopMostSprite()->toString(szName);
    }
    else                                            // Else we have nothing that handles mouse input
    {
        strcpy(szName, "");
    }
#else
    APT_ASSERT(false && "MOUSE NOT SUPPORTED, define APT_USE_MOUSE to use the mouse");
    strcpy(szName, "");
#endif
}

/*F************************************************************************************************/
/*!
    \Function    AptRegisterGlobalReferences

    \Description
        This calls RegisterReferences on all global objects that are not based off of AptValue.


    \Input
        None.

    \Output
        None.

    \Notes
        None.

    \Version    1.0        09/09/04 (DC) Bringin' it to ya

*/
/************************************************************************************************F*/
void AptRegisterGlobalReferences()
{

    AptTarget *pTmp = gpDefaultTarget;
    while(pTmp)
    {
        pTmp->GetAnimationTarget()->RegisterReferences();
        pTmp = pTmp->GetNext();
    }

    // gpObjRegistrationHash only exists if _gObjRegistrationFunc is called.
    if(gpObjRegistrationHash)
    {
#if defined(APT_XML_MEMORY_DUMP_SUPPORTED)
        gpObjRegistrationHash->RegisterReferences(NULL, "Object.registerClass::");
#else
        gpObjRegistrationHash->RegisterReferences(NULL);
#endif
    }
}

/*F************************************************************************************************/
/*!
    \Function    AptPreloadFilterSWFFile

    \Description
        Loads filter swf file that is used for defining the Flash filter classes.

    \Input
        None.

    \Output
        None.

    \Notes
        This has to be called immediately after a new AptTarget is created as it has to be first
        file loaded to have all filter definitions loaded for access.


    \Version    1.0        10/26/07 (ASK) First Version
    \Version    1.1        09/23/09 (MZ)  Conditionally exclude code in AptPreloadFilterSWFFile()
                                          based on APT_USE_FILTERS macro.
*/
/************************************************************************************************F*/
void AptPreloadFilterSWFFile()
{
#if APT_USE_FILTERS
    static const char szLevel[] = "_level24";

    // preload this filter swf file, immediately after apt initialization (AptTarget creation) and also
    // resolve this file so on next AptUpdate the actionscript in its first frame to define all the
    // filter classes gets executed, and we have these filter classes ready even before first SWF file
    // is loaded by aptaux
    AptNativeString sFileName( sFiltersPseudoFileName );
    // this will add filter definition file to linker and loader queues
    gpDefaultTarget->GetLinker()->Load(sFileName + ".swf", szLevel);
    AptFilePtr fp = gpDefaultTarget->GetLoader()->findFile( sFileName );
    fp->setState(AptFile::WaitingForData);
    // immediately call this to let Apt loader/linker logic know that the .Apt and
    // .const files for filters are ready for processing.
    // this will resolve/parse the .apt file and it will be ready for execution on the
    // next AptUpdate call for AptLinker->Update function.


    AptCompleteAnimationAsyncLoad(fp, g_arrFiltersAptFile, g_arrFiltersConstFile, (void*)APT_FILTERS_FILE_USERDATA);
#endif
}

/*F************************************************************************************************/
/*!
    \Function    AptGetMovieclipInfo

    \Description
        This function will go through the main displaylist and all other recursive
        displaylists and count the various number of movieclips.


    \Input      pMCInfo           - pointer to AptMovieclipInformation structure.
                    This will actually act a in-out parameter.

    \Output
        None.

    \Notes
        Each recursive call on display list will add to the counters in
        the pMCInfo structure.
        pMCInfo in in-out parameter.

    \Version    1.0        09/14/04 (ASK) First Version

*/
/************************************************************************************************F*/
void AptGetMovieclipInfo(AptMovieclipInformation * pMCInfo, bool bGenerateNew)
{
    AptUpdateAutoLock lock;

    APT_ASSERT(pMCInfo) ;

    if (bGenerateNew)
    {
        AptTarget *pTmp = gpDefaultTarget;
        while(pTmp)
        {
            pTmp->GetAnimationTarget()->GetDisplayList()->getState()->GetMovieclipInfo(pMCInfo);
            pTmp = pTmp->GetNext();
        }
    }
    else
    {
        // copy already calculated gAptMovieclipInformation into pMCInfo
#if defined (APT_GATHER_MOVIECLIP_METRICS)
        *pMCInfo = gAptMovieclipInformation;
#else
        APT_DEBUGPRINT(APT_DEBUG_MSG_WARNING_LVL, "Please define APT_GATHER_MOVIECLIP_METRICS in AptDEfine.h to get correct values, or pass true to this function\n");
#endif
    }
}


#if defined(APT_XML_MEMORY_DUMP_SUPPORTED)
    const char * gVFTNameStrings[] =
    {
        "xxx",
        "StringValue",
        "Property",
        "None",
        "Register",
        "Boolean",
        "Float",
        "Integer",
        "Lookup",
        "NativeFunction",
        "FrameStack",
        "Extern",
        "CharacterInstHandle",
        "Sound",
        "Array",
        "Math",
        "Key",
        "Global",
        "ScriptColour",
        "Object",
        "Prototype",
        "Date",
        "MovieClip",
        "Mouse",
        "XmlNode",
        "Xml",
        "XmlAttributes",
        "LoadVars",
        "TextFormat",
        "Extension",
        "GlobalExtension",
        "Stage",
        "Error",
        "StringObject",
        "ScriptFunction1",
        "ScriptFunction2",
        "ScriptFunctionByteCodeBlock",
        "CIHNone",
        "MovieClipLoader"
    };
#endif

/*F************************************************************************************************/
/*!
\Function    *AptGetTypeOfAptValue

\Description
Returns the Type as a string..

\Input      pValue            -

\Output     const char  -  Type String

\Notes
None.

\Version    1.0        11/03/04 (DC) Bringin' it to ya

*/
/************************************************************************************************F*/
const char *AptGetTypeOfAptValue(const AptValue * pValue)
{
    AptUpdateAutoLock lock;

    const char *pVal = 0;

#if defined(APT_XML_MEMORY_DUMP_SUPPORTED)
    AptVirtualFunctionTable_Indices eType = pValue->getVtblIndex();

    if(eType < AptVFT_NumVFTs)
    {
        pVal = gVFTNameStrings[pValue->getVtblIndex()];
    }
    else
    {
        APT_ASSERT(eType < AptVFT_NumVFTs);
        pVal = "INVALID";
    }
#else
    static char tempString[8];
    sprintf(tempString, "0x%X",pValue->getVtblIndex());
    pVal = tempString;
#endif

    return pVal;
}

const uint8_t AptValueSizesByVType[] =
{
    0,  //
    sizeof(AptNativeString                 ),  //AptVFT_StringValue
    0,                                         //AptVFT_Property
    sizeof(AptNone                         ),  //AptVFT_None
    sizeof(AptRegister                     ),  //AptVFT_Register
    sizeof(AptBoolean                      ),  //AptVFT_Boolean
    sizeof(AptFloat                        ),  //AptVFT_Float
    sizeof(AptInteger                      ),  //AptVFT_Integer
    sizeof(AptLookup                       ),  //AptVFT_Lookup
    sizeof(AptNativeFunction               ),  //AptVFT_NativeFunction
    sizeof(AptFrameStack                   ),  //AptVFT_FrameStack
    sizeof(AptExtern                       ),  //AptVFT_Extern
    sizeof(AptCIH                          ),  //AptVFT_CharacterInstHandle
    sizeof(AptSound                        ),  //AptVFT_Sound
    sizeof(AptArray                        ),  //AptVFT_Array
    sizeof(AptMathObj                      ),  //AptVFT_Math
    sizeof(AptKey                          ),  //AptVFT_Key
    sizeof(AptGlobal                       ),  //AptVFT_Global
    sizeof(AptScriptColour                 ),  //AptVFT_ScriptColour
    sizeof(AptObject                       ),  //AptVFT_Object
    sizeof(AptPrototype                    ),  //AptVFT_Prototype
    sizeof(AptDate                         ),  //AptVFT_Date
    sizeof(AptMovieClip                    ),  //AptVFT_MovieClip
    sizeof(AptMouse                        ),  //AptVFT_Mouse
    sizeof(AptXmlNode                      ),  //AptVFT_XmlNode
    sizeof(AptXml                          ),  //AptVFT_Xml
    sizeof(AptXmlAttributes                ),  //AptVFT_XmlAttributes
    sizeof(AptLoadVars                     ),  //AptVFT_LoadVars
    sizeof(AptTextFormat                   ),  //AptVFT_TextFormat
    sizeof(AptExtObject                    ),  //AptVFT_Extension
    sizeof(AptGlobalExtensionObject        ),  //AptVFT_GlobalExtension
    sizeof(AptStage                        ),  //AptVFT_Stage
    sizeof(AptError                        ),  //AptVFT_Error
    sizeof(AptStringObject                 ),  //AptVFT_StringObject
    sizeof(AptScriptFunction1              ),  //AptVFT_ScriptFunction1
    sizeof(AptScriptFunction2              ),  //AptVFT_ScriptFunction2
    sizeof(AptScriptFunctionByteCodeBlock  ),  //AptVFT_ScriptFunctionByteCodeBlock
    sizeof(AptCIHNone                      ),  //AptVFT_CIHNone
    sizeof(AptMovieClipLoader              ),  //AptVFT_MovieClipLoader
    sizeof(AptUtil                         ),  //AptVFT_AptUtil
    0,                                         //AptVFT_AptExternalFunction, size found elsewhere
#if defined(APT_ALTERNATE_INPUT)
    sizeof(AptAlternateInput               ),  //AptVFT_AltInput
#endif
    0
};

/*F************************************************************************************************/
/*!
\Function    AptGetSizeOfAptValue

    \Brief       Returns the size of a given Apt Value derivative.

    \Input      pValue   - AptValue of which to get size
    \Output     uint32_t - Size of given AptValue, in bytes

    \Notes      We expect AptExtObjects to be extended externally and as such provide a
                mechanism for obtaining the size of the extended object.

\Version    1.0        11/03/04 (DC) Bringin' it to ya
    \Version    1.1        07/07/09 (RB)  Removed sim-thread lock because it's not necessary:
                                          sizes of built-in AptValues are constant, and size of
                                          an AptExtObject is set once on construction.

*/
/************************************************************************************************F*/
uint32_t AptGetSizeOfAptValue(const AptValue * pValue)
{
    AptVirtualFunctionTable_Indices eType = pValue->getVtblIndex();
    APT_ASSERT(eType < AptVFT_NumVFTs);
    APT_ASSERT(eType >= AptVFT_xxx );
    uint32_t size = 0;

    switch (eType)
    {
        case AptVFT_Extension:
            size = (static_cast<const AptExtObject*>(pValue))->GetSize();
            break;

        case AptVFT_ExternalFunction:
            size = (static_cast<const AptExternalFunction*>(pValue))->GetClassSize();
            break;

        default:
        size = AptValueSizesByVType[eType];
            break;
    }

    return size;
}



/*F************************************************************************************************/
/*!
\Function    *AptNonGCAllocSaveSize

\Description
Insert function's purpose here

Insert your long description here.

\Input      nSize             -
\Input      *szName           -
\Input      nLine             -

\Output     void*             -

\Notes
None.

\Version    1.0        11/08/04 (DC) Bringin' it to ya

*/
/************************************************************************************************F*/
#if defined(APT_ALLOCATION_TRACKING)
void *AptNonGCAllocSaveSize(size_t size, const char *szName, int nLine)
#else
void *AptNonGCAllocSaveSize(size_t size)
#endif
{
    uint32_t * x = (uint32_t*)APT_MALLOC_BLOCK( (size+4) );
    x[0]=static_cast<uint32_t>(size);
    return &x[1];
}

/*F************************************************************************************************/
/*!
    \Function    *AptNonGCFreeSavedSize

    \Description
        Insert function's purpose here

    Insert your long description here.

    \Input      void*           -

    \Output
        None.

    \Notes
        None.

    \Version    1.0        11/08/04 (DC) Bringin' it to ya

*/
/************************************************************************************************F*/
void AptNonGCFreeSavedSize(void* p)
{
    uint32_t * x = (uint32_t*)p;
    APT_FREE_BLOCK( (x-1),(*(x-1))+4);
}

/*F************************************************************************************************/
/*!
    \Function    AptSetSimulationThread

    \Description
        Designate the current thread as the simulation thread. This is only used for
        AptAssertIsSimulationThread.

    \Input
        None.

    \Output
        None.

    \Notes
        None.

    \Version    1.0
*/
/************************************************************************************************F*/
void AptSetSimulationThread()
{
#ifndef APT_SINGLE_THREADED
    gAptSimThreadId = APT_THREAD_NAMESPACE::GetThreadId();
#endif
}

/*F************************************************************************************************/
/*!
    \Function    AptAssertIsSimulationThread

    \Description
        Checks if the current thread running is the Simulation thread, is not it will assert.

    \Input
        None.

    \Output
        None.

    \Notes
        None.

    \Version    1.0        05/16/05 (LEO) First Version

*/
/************************************************************************************************F*/
void AptAssertIsSimulationThread()
{
#ifndef APT_SINGLE_THREADED
    APT_ASSERT(AptIsSimulationThread());
#endif
}

/*F************************************************************************************************/
/*!
    \Description Checks if the current thread running is the Simulation thread
*/
/************************************************************************************************F*/
bool AptIsSimulationThread()
{
    return gAptSimThreadMutex.HasLock() || gAptSimThreadId == APT_THREAD_NAMESPACE::GetThreadId();
}

/*F************************************************************************************************/
/*!
    \Function    AptAssertIsRenderThread

    \Description
        Checks if the current thread running is the Render thread, if not it will assert.

    \Input
        None.

    \Output
        None.

    \Notes
        None.

    \Version    1.0        05/16/05 (LEO) First Version

*/
/************************************************************************************************F*/
void AptAssertIsRenderThread()
{
}


/******************************************************************************/
/*! AptSetActionScriptProfilerEnabled

    \brief  Set the action script profiling state.

    \param  enabled - true to enable profiling, false to disable.
    \return         - void.
*/
/******************************************************************************/
void AptSetActionScriptProfilerEnabled(bool8_t enabled)
{
#if defined (APT_DO_AS_PROFILE)
    if (enabled)
    {
        // if you want to use Actionscript profiling then pfnGetCurrentTime callback has to be implemented.
        APT_ASSERT(gAptFuncs.pfnGetCurrentTime);
        gAptActionInterpreter.mFunctionProfiler.Start();
    }
    else
    {
        gAptActionInterpreter.mFunctionProfiler.Stop();
    }
#endif
}

/******************************************************************************/
/*! AptGetActionScriptProfilerEnabled

    \brief  Get the action script profiling state.

    \param              - none.
    \return             - true if profiling is enabled, false if disabled
*/
/******************************************************************************/
bool8_t AptGetActionScriptProfilerEnabled()
{
#if defined (APT_DO_AS_PROFILE)
    return gAptActionInterpreter.mFunctionProfiler.IsProfiling();
#else
    return false;
#endif
}

/******************************************************************************/
/*! AptActionScriptProfilerStandaloneBlockStart

    \brief  Starts profiling arbitrary ActionScript until told otherwise. This
            will start the frame at this point (regardless of truth) and set things
            up so that we can profile single lines of actionscript, if needed.

    \param  none
    \return             - void
*/
/******************************************************************************/
void AptActionScriptProfilerStandaloneBlockStart()
{
    // Recommendation: In the future, if this works one as expected, this
    // should get shifted into AptUtil as a way to log a function call.
    // profileBlockStart and profileBlockEnd do not work as well if profiling
    // has not been running before they are called.
#if defined (APT_DO_AS_PROFILE)
    gAptActionInterpreter.mFunctionProfiler.LogFrameStart();
    gAptActionInterpreter.mFunctionProfiler.LogFunctionStart("__AptActionScriptProfilerFrameStartFunction__", false );
    gAptActionInterpreter.mFunctionProfiler.LogBlockStart("__AptActionScriptProfilerFrameStartBlock__");

#endif
}

/******************************************************************************/
/*! AptActionScriptProfilerStandaloneBlockEnd

    \brief  Stops arbitrary profiling. End point of AptActionScriptProfilerStandaloneBlockStart

    \param  none
    \return             - void
*/
/******************************************************************************/
void AptActionScriptProfilerStandaloneBlockEnd()
{
#if defined (APT_DO_AS_PROFILE)
    gAptActionInterpreter.mFunctionProfiler.LogBlockEnd();
    gAptActionInterpreter.mFunctionProfiler.LogFunctionEnd("__AptActionScriptProfilerFrameStartFunction__");
    gAptActionInterpreter.mFunctionProfiler.LogFrameEnd();
#endif
}

/******************************************************************************/
/*! AptSetActionScriptProfilerThresholdMS

    \brief  Set the action script profiling threshold time.

    \param  threshold   - profiling threshold time in milliseconds
    \return             - void.
*/
/******************************************************************************/
void AptSetActionScriptProfilerThresholdMS(float threshold)
{
#if defined (APT_DO_AS_PROFILE)
    gAptActionInterpreter.mFunctionProfiler.SetThreshold(static_cast<uint32_t>(threshold * 1000));
#endif
}

/******************************************************************************/
/*! AptGetActionScriptProfilerThresholdMS

    \brief  Get the action script profiling threshold time.

    \param              - none.
    \return             - profiling threshold time in milliseconds
*/
/******************************************************************************/
float AptGetActionScriptProfilerThresholdMS()
{
#if defined (APT_DO_AS_PROFILE)
    return gAptActionInterpreter.mFunctionProfiler.GetThreshold() / 1000.0f;
#else
    return 0;
#endif
}

/******************************************************************************/
/*! AptSetActionScriptProfilerBufferSizeKB

    \brief  Set the action script profiling buffer size.

    \param  bufferSize  - profiling buffer size in kilobytes
    \return             - void.
*/
/******************************************************************************/
void AptSetActionScriptProfilerBufferSizeKB(uint32_t bufferSize)
{
#if defined (APT_DO_AS_PROFILE)
    gAptActionInterpreter.mFunctionProfiler.SetBufferSize(bufferSize * 1024);
#endif
}

/******************************************************************************/
/*! AptGetActionScriptProfilerBufferSizeKB

    \brief  Get the action script profiling buffer size.

    \param              - none.
    \return             - profiling buffer size in kilobytes
*/
/******************************************************************************/
uint32_t AptGetActionScriptProfilerBufferSizeKB()
{
#if defined (APT_DO_AS_PROFILE)
    return gAptActionInterpreter.mFunctionProfiler.GetBufferSize() / 1024;
#else
    return 0;
#endif
}



/*F************************************************************************************************/
/*!
    AptGetFunctionDebugName

    Get a nice name to label the function with while debugging

    \Input      pFunctionValue - pointer to the function object as an AptValue
                defaultName - name to fall back on if we fail to find a better one

    \Output     Name used to label the function for debugging

*/
/************************************************************************************************F*/
const char8_t * AptGetFunctionDebugName(AptValue * pFunctionValue, const char8_t * defaultName)
{
#if defined (APT_USE_DEBUG_NAMES)
    // If it is a function, we hope to have stored its name at some point
    // in the past.  If so, we did it by throwing a name into the Function's HashTable
    if (pFunctionValue->isScriptFunction())
    {
        AptNativeHash* pFuncHash = pFunctionValue->GetNativeHashVirtual();
        AptValue* currentName = pFuncHash->Lookup(StringPool::GetString(SC__debugName));
        if (NULL != currentName && currentName->isString())
        {
            return currentName->c_string()->GetInternalString()->c_str();
        }
    }
#endif
    return defaultName;
}

/******************************************************************************/
/*! 
    \brief  Sets the path to use for the action script profiler log.

    \param              - path of to use for the log file.
*/
/******************************************************************************/
void AptSetActionScriptProfilerLogPath(const char* path)
{
#if defined (APT_DO_AS_PROFILE)
    gAptActionInterpreter.mFunctionProfiler.SetLogFilePath(path);
#endif
}


/************************************************************************************************F*/
bool AptMetricsEnabled()
{
#if APT_ENABLE_METRICS 
	return true;
#else
	return false;
#endif // APT_ENABLE_METRICS
}

/************************************************************************************************F*/
bool AptASProfilerEnabled()
{
#if APT_DO_AS_PROFILE 
	return true;
#else
	return false;
#endif // APT_DO_AS_PROFILE
}

/************************************************************************************************F*/
void AptSetActionASProfilerHookFn( void (*ActionScriptEvent) ( int , const char * ) )
{
#if APT_DO_AS_PROFILE 
	gAptFuncs.pfnActionScriptProfileEvent = ActionScriptEvent;
#endif // APT_DO_AS_PROFILE 
}
