/*H************************************************************************************************/
/*!

    \File    Apt\_Apt.h

    \Description
        Simple description of main purpose of file.

    \Notes
        None.

    \Copyright
        (c) 2005 Electronic Arts Inc.

    \Version    1.0        09/01/05 (LEO) First Version
    \Version    1.1        02/19/08 (RB)  Added dummy Futex class and APT_INC_THREAD_FUTEX_H
    \Version    1.3        04/03/08 ($senzee) Added prototypes for AptUpdateLock() and
                                          AptUpdateUnlock() and a corresponding auto-lock class
                                          which is initially used in AptValueHelper.
    \Version    1.4        04/29/08 (RB)  Added PlaceObject3
    \Version    1.5        06/12/08 (JP)  Adding support for the MovieClipLoader class
    \Version    1.6        06/12/08 ($senzee)  Adding optional support for the AptUtil class
    \Version    1.7        09/22/09 (MZ)  Added struct AptAltInput definition so that it is not exposed to Apt clients

*/
/************************************************************************************************H*/

#ifndef _Apt__Apt_h
#define _Apt__Apt_h

#if defined(_MSC_VER)
#pragma once
#endif

/*** Include files ********************************************************************************/
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <time.h>

#include "Apt.h"
#include "AptDefine.h"
#include "_AptInternalDefines.h"
#include "_AptValue.h"
#include "_AptSet.h"
#include "_AptValuePtrStack.h"
#include "_AptBasePtrStack.h"
#include "_AptActions.h"
#include "AptProf.h"
#include "Aptxmlinterfaces.h"
#include "string/StringPool.h"
#include "_AptFile.h"
#include "String/StringPool.h"
#include "_AptThread.h"
#include "_AptLoad.h"
// FIFA_BEGIN | SENDBACK | mwebb | 2015 - 05 - 26 | Adding Frostbite support
#include "Module.h"
// FIFA_END

/*** Defines **************************************************************************************/
#if defined(EA_PLATFORM_WINDOWS) || defined(EA_PLATFORM_XBOX)
// warning C4514: 'blah' : unreferenced inline function has been removed
// happens lots in standard msvc headers
#pragma warning( disable : 4514 )

// unreferenced formal parameter
#pragma warning( disable : 4100 )

// conditional expression is constant (asserting not reached)
#pragma warning( disable : 4127 )

// local function being removed
#pragma warning( disable : 4505 )

// unreferenced inline function has been removed
#pragma warning( disable : 4514 )
#endif

#if defined(APT_ALTERNATE_INPUT)
/// This structure defines alternative way to provide input to Apt. Traditionally input could be key/mouse or analog stick input.
/// But with this new input mechanism, game can provide any type of input to Apt and handle that input in its action script with a special
/// type of script object which is only understood by Apt. So with following structure game can have following code
/// \verbatim
/// AptAltInput myInput;
/// myInput.sEvent = "onGameEvent1" ;
/// myInput.pValue = AptValueFactory::CreateInteger(nSomeValue);
/// AptAddAlternateInput(&myInput);
/// \endverbatim
///
/// And then later inside Flash action script it can be handled something like this
/// \verbatim
/// // To access the parameter AptAltInput.pValue that was passed to AptAddAlternateInput, the script function must define at least one parameter
/// // as Apt passes AptAltInput.pValue to the script function when it is called.
/// myMovieClip.onGameEvent1 = function(param1)
/// {
///        trace("alternate input value passed by game code is " + param1)
/// }
/// AlternateInput.addListener(myMovieClip);
/// \endverbatim
/// // And then later in action script to remove object or movieclip from alternate-input listener set
/// \verbatim
/// AlternateInput.removeListener(myMovieClip).
/// \endverbatim
#include "AptString\EAString.h"

struct AptAltInput
{
    APT_NEW_DELETE_OPERATORS
    APT_DECLARE_OBJECT_METRICS
    /// Name of the event to be passed to Apt. Movieclip or object that is added to alternate-input listener set by AlternateInput.addListener()
    /// must have a function with same name
    AptNativeString     sEvent ;
    /// Pointer to AptValue type object that is later passed as parameter to alternate input handler function in action script.
    AptValue            *pValue ;

    AptAltInput():pValue(0) {}
};
#endif

#if defined(APT_USE_MOUSE)
#define CLIP_EVENT_ARRAY_SIZE   17
#else
#define CLIP_EVENT_ARRAY_SIZE   6
#endif

// AptUpdateLock() and AptUpdateUnlock() are used by the APT_INC/APT_DEC macros to ensure that
// external APT_INC and APT_DEC operations on AptValues are thread-safe.  The internal macros that
// correspond to these, _APT_INC and _APT_DEC are not thread-safe, and do not use AptUpdateLock()
// and AptUpdateUnlock().
// FIFA_BEGIN | SENDBACK | mwebb | 2015 - 05 - 26 | Adding Frostbite support
//void AptUpdateLock();
FB_APT_API void AptUpdateLock();
// FIFA_END

// AptUpdateLock() and AptUpdateUnlock() are used by the APT_INC/APT_DEC macros to ensure that
// external APT_INC and APT_DEC operations on AptValues are thread-safe.  The internal macros that
// correspond to these, _APT_INC and _APT_DEC are not thread-safe, and do not use AptUpdateLock()
// and AptUpdateUnlock().
// FIFA_BEGIN | SENDBACK | mwebb | 2015 - 05 - 26 | Adding Frostbite support
//void AptUpdateUnlock();
FB_APT_API void AptUpdateUnlock();
// FIFA_END

// AptUpdateAutoLock class is provided as a convenience - an autolock class for the AptUpdateLock()
// and AptUpdateUnlock() functions
class AptUpdateAutoLock
{
public:
    inline AptUpdateAutoLock()
    {
        AptUpdateLock();
    }
    inline ~AptUpdateAutoLock()
    {
        AptUpdateUnlock();
    }
};

// DC - Added #defines to do atomic operations (equate to native operations in non-decoupled mode)
// NOTICE! These will not do pointer arithmetic. These macros implicitly cast all values to uint32_t's
// I.e. (uint32_t*)x + 4 is not equal to (uint32_t)x + 4. This does the latter in all cases.

// Note that you can't directly typecast an lvalue on most compilers (as per ANSI spec) thus this
// takes the address of the ref and assigns it using a pointer operation.
#define APT_READ32(ref)                                            (ref)
#define APT_WRITE32(ref, value)                                    (*(uint32_t*)&ref) =  ((uint32_t)value)
#define APT_ADD32(ref, value)                                      (*(uint32_t*)&ref) += ((uint32_t)value)
#define APT_SUB32(ref, value)                                      (*(uint32_t*)&ref) -= ((uint32_t)value)
#define APT_INC32(ref)                                             (++(*(uint32_t*)&ref))
#define APT_DEC32(ref)                                             (--(*(uint32_t*)&ref))
#define APT_CONDITIONAL_EXCHANGE32(ref, compareValue, newvalue)    if ((ref) == (compareValue)) (ref) = (newvalue);
#define APT_READ_PTR(ref)                                          (ref)
#define APT_WRITE_PTR(ref, value)                                  (*(intptr_t*)&ref) =  ((intptr_t)value)
#define APT_ADD_PTR(ref, value)                                    (*(intptr_t*)&ref) += ((intptr_t)value)
#define APT_SUB_PTR(ref, value)                                    (*(intptr_t*)&ref) -= ((intptr_t)value)
#define APT_INC_PTR(ref)                                           (++(*(intptr_t*)&ref))
#define APT_DEC_PTR(ref)                                           (--(*(intptr_t*)&ref))
#define APT_CONDITIONAL_EXCHANGE_PTR(ref, compareValue, newvalue)  if((ref) == (compareValue)) (ref) = (newvalue);

/*** Macros ***************************************************************************************/


template < class T >
static inline T* AptResolve( T* ptr, void * pBase )
{
    if ( ptr != 0)
    {
        return (T*) ( (uintptr_t)ptr + (uintptr_t)pBase );
    }
    return ptr;
}

template < class T >
static inline T* AptUnResolve( T* ptr, void * pBase )
{
    if ( ptr != 0)
    {
        return (T*) ( (uintptr_t)ptr - (uintptr_t)pBase );
    }
    return ptr;
}

#define APT_RESOLVE(_)      { _ = AptResolve(_, pBase); }    /* DC - Removed this: APT_ASSERT((uintptr_t)_ < 0xfffff);  */
#define APT_UNRESOLVE(_)    { _ = AptUnResolve(_, pBase); }  /* DC - Removed this: APT_ASSERT((uintptr_t)_ == 0x0000 || (uintptr_t)_ > 0xfffff); */

/*** Type Definitions *****************************************************************************/
struct AptCharacter;
class AptCharacterInst;
class AptNativeHash;
class AptAnimationTarget;
struct AptLoader;
struct AptLinker;
class AptBCRenderTreeManager;
struct AptFilter;

enum AptPlaceObjectFlags
{
    AptPlaceObjectFlag_None          = 0x00,
    AptPlaceObjectFlag_Move          = 0x01, // this place moves an existing object
    AptPlaceObjectFlag_Character     = 0x02, // there is a character tag (if no tag, must be a move)
    AptPlaceObjectFlag_Matrix        = 0x04, // there is a matrix (matrix)
    AptPlaceObjectFlag_CXForm        = 0x08, // there is a color transform (cxform with alpha)
    AptPlaceObjectFlag_Ratio         = 0x10, // there is a blend ratio (word)
    AptPlaceObjectFlag_Name          = 0x20, // there is an object name (string)
    AptPlaceObjectFlag_DefineClip    = 0x40, // this shape should open or close a clipping bracket (character != 0 to open, character == 0 to close)
    AptPlaceObjectFlag_Actions       = 0x80, // has actions associated with it
};

/// These values match what is output by swfc
enum AptControlType
{
    AptControlType_DoAction = 1,
    AptControlType_FrameLabel = 2,
    AptControlType_PlaceObject2 = 3,
    AptControlType_RemoveObject2 = 4,
    AptControlType_BackgroundColour = 5,
    AptControlType_StartSound = 6,
    AptControlType_StartSoundStream = 7,
    AptControlType_DoInitAction = 8,
    AptControlType_PlaceObject3 = 9,
};

struct AptControlDoAction
{
    AptActionBlock actions;
};

struct AptControlDoInitAction
{
    int nSpriteID;
    AptActionBlock actions;
};

struct AptControlFrameLabel
{
    char *szLabel;
};

struct AptFloatArrayCXForm
{
    float scale[4];
    float translate[4];
};

struct AptUint32CXForm
{
    uint32_t nScale;
    uint32_t nBias;
};

struct AptControlPlaceObject2
{
    AptPlaceObjectFlags eFlags;     // could be char
    int nDepth;                     // could be short
    int nCharacterID;               // could be short
    AptMatrix matrix;
    AptUint32CXForm ncxform;
    float fRatio;                   // for morphs, could be short
    char *szName;                   // ptr
    int nClipDepth;                 // could be short
    AptEventActionSet *pActions;    // ptr
};

struct AptControlPlaceObject3
{
    AptPlaceObjectFlags eFlags;     // could be char
    int nDepth;                     // could be short
    int nCharacterID;               // could be short
    AptMatrix matrix;
    AptUint32CXForm ncxform;
    float fRatio;                   // for morphs, could be short
    char *szName;                   // ptr
    int nClipDepth;                 // could be short
    AptEventActionSet *pActions;    // ptr
    int32_t    nBlendMode;
    uint32_t   nNumFilters;
    AptFilter **ppFilters;
};

struct AptControlRemoveObject2
{
    int nDepth;
};

struct AptControlBackgroundColour
{
    unsigned int nColour;
};

struct AptControlSound
{
#if defined(APT_USE_SOUND_OBJECT)
    int nID;
#endif
};

struct AptControl
{
    AptControlType eType;
    union
    {
        AptControlDoAction action;
        AptControlDoInitAction initAction;
        AptControlFrameLabel frameLabel;
        AptControlPlaceObject2 placeObject2;
        AptControlPlaceObject3 placeObject3;
        AptControlRemoveObject2 removeObject2;
        AptControlBackgroundColour backgroundColour;
#if defined(APT_USE_SOUND_OBJECT)
        AptControlSound startSound;
        AptControlSound startSoundStream;
#endif
    };
};

struct AptFrame
{
    int nControls;
    AptControl **apControls;
};

struct AptCharacterButtonRecord
{
#if defined APT_USE_BUTTONS
    AptCharacterButtonRecordState eStates;
    AptCharacter *pCharacter;
    int nLayer;
    AptMatrix matrix;
    AptFloatArrayCXForm cxform;
#endif
};

struct AptCharacterButtonSound
{
#if defined APT_USE_BUTTONS
    AptCharacter *pOverUpToIdle;
    AptCharacter *pIdleToOverUp;
    AptCharacter *pOverUpToOverDown;
    AptCharacter *pOverDownToOverUp;
#endif
};

struct AptCharacterGlyphEntry
{
    short nIndex;
    short nAdvance;
};

struct AptCharacterStaticTextRecords
{
    int nFontID;
    AptFloatArrayCXForm cxform;
    float fXOffset;
    float fYOffset;
    float fScale;
    int nGlyphs;
    AptCharacterGlyphEntry *aGlyphs;
};

struct AptImport
{
    char *szFile;
    char *szName;
    int nID;
    AptFilePtr file;
};

struct AptExport
{
    char *szName;
    int nID;
};

struct SavedInputPlayback
{
    unsigned char *pSavedInputs;
    unsigned char *pCurSavedInput;
    int nInputFileSize;
    int nLoadedAnims;
    unsigned int nCurTick;
};

struct ClipEventType
{
    int         nFlag;
    StringCode  eName;
};

class AptNone : public AptValueNoGC
{
public:
    APT_VALUE_NEW_DELETE_OPERATORS
        // DC - Added Metric defines
    APT_DECLARE_OBJECT_METRICS

    AptNone() : AptValueNoGC(AptVFT_None)
    {
        setIsDefined(0);
        setRefCount(MAX_REFCOUNT);
    }

    // This Optimizes global objects by doing nothing on Add/Release
#if defined(APT_INC_DEC_MESSAGES)
    virtual void    AddRef(const char * szFuncName, const char * szFileName, int nLineNumber)  { }   //  Do nothing...
    virtual void    Release(const char * szFuncName, const char * szFileName, int nLineNumber) { }   //  Do nothing...
#else
    virtual void    AddRef()  { }   //  Do nothing...
    virtual void    Release() { }   //  Do nothing...
#endif

protected:
    APT_INLINE
        virtual ~AptNone()
    {
        //  Do nothing...
    }
};

class AptExtern : public AptValueNoGC
{
public:
    APT_VALUE_NEW_DELETE_OPERATORS
        // DC - Added Metric defines
    APT_DECLARE_OBJECT_METRICS

    AptExtern() : AptValueNoGC(AptVFT_Extern)
    {
        setGCRoot(1);
        setRefCount(MAX_REFCOUNT);
    }

    virtual bool        objectMemberSet(AptValue * const pContext, const AptNativeString * const pName, AptValue * const pValue);

    // This Optimizes global objects by doing nothing on Add/Release
#if defined(APT_INC_DEC_MESSAGES)
    virtual void    AddRef(const char * szFuncName, const char * szFileName, int nLineNumber)  { }   //  Do nothing...
    virtual void    Release(const char * szFuncName, const char * szFileName, int nLineNumber) { }   //  Do nothing...
#else
    virtual void    AddRef()  { }   //  Do nothing...
    virtual void    Release() { }   //  Do nothing...
#endif

    APT_INLINE
        virtual ~AptExtern()
    {
        //  Do nothing...
    }
};

/*** Variables ************************************************************************************/
extern SavedInputPlayback       gSIPlayback;
extern int                      gbSavedInputsEnabled;
extern unsigned int             gnCurTick;
extern AptRenderingContext *    gpRenderingContext;
extern AptExtern *              gpExternValue;
extern AptNativeHash *          gpObjRegistrationHash;
extern AptNativeFunction *      gpObjRegistrationFunc;
extern bool                     gbGarbageCollectThisFrame;
extern AptNativeString          gstrTempString;
extern bool                     gbBackgroundColorSet;   // Background color can only be set once.
extern IAptXmlImpl *            gpAptXmlImpl;
extern AptGlobal *              gpGlobalGlobalObject;

extern int32_t                  gnCurrUpdateTick;
extern unsigned int             gnCurrRenderTick;
//extern int32_t                  gnUnresolvingFile; // defined in AptLoad.cpp for debugging.

extern AptValueVector *         gpZombieVector;         // (LEO) Zombie Vector to hold all animations in the zombie state   $BUG256 LEO    18
extern AptInitParmsT            gAptInitParms;

extern ClipEventType            _aClipEvents[];

extern AptTarget *gpDefaultTarget;
extern AptTarget *gpCurrentTargetSim;
extern AptTarget *gpCurrentTargetRender;

#if !defined(APT_ALTERNATE_INPUT)
#define APT_VALUESIZES_ARRAYSIZE    42
#else
#define APT_VALUESIZES_ARRAYSIZE    43
#endif
extern const uint8_t AptValueSizesByVType[APT_VALUESIZES_ARRAYSIZE]; // Tell the compiler that no really I want this exported... stupid compiler...

#if defined (APT_GATHER_MOVIECLIP_METRICS)
extern AptMovieclipInformation gAptMovieclipInformation;
#endif

/*** Functions ************************************************************************************/

/*! \Returns the currently set sim target */
APT_INLINE AptTarget *GetTargetSim();

/*! \Returns the currently set render target */
APT_INLINE AptTarget *GetTargetRender();

/*! \Returns the currently set target based on thread id*/
AptTarget *GetTarget();

float       Apt_atoff(const char* _s);
uint32_t    AptRand(void);

#if defined(APT_USE_MOUSE)
bool        _AptGetMousePosition(int32_t *pX, int32_t *pY);
#endif
AptCIH *    _AptGetAnimationAtLevel(int32_t nLevel);
void        AptUpdateZombieVector(bool bClean = false);        // $BUG256 LEO    19

bool        _AptValidate(void);

// DC - Added utility Function to return info on a given AptValue
uint32_t    AptGetSizeOfAptValue(const AptValue * pValue);
const char *AptGetTypeOfAptValue(const AptValue * pValue);

extern int32_t  AptGetSwfVersion();
extern void     AptSetSwfVersion(int32_t p_version);
extern void     AptRegisterGlobalReferences();
extern void     AptPreloadFilterSWFFile();

// we define all the filter classes with their bodies in a as file and create a swf file that has all those classes.
// pointers to .Apt and .const files for filter definition swf file.
extern unsigned char g_arrFiltersAptFile[];
extern unsigned char g_arrFiltersConstFile[];
#define APT_FILTERS_FILE_USERDATA 0x12345679        // make it odd so the possibility of real world pointer having this value is very rare.

#endif // _Apt__Apt_h
