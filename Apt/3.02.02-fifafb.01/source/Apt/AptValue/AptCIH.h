/*H************************************************************************************************/
/*!

    \File
        AptCIH.h

    \Description

        Basically CIH objects are a window into the character inst for actionscript. Thus
        the CIH object is becoming a bridge from actionscript to the data in the inst object.
        at the moment this is not wholly enforced, but for future additions we are trying to
        go in this direction.


        Here's an overview of the system and how the objects interact.




        +--------+
        |        |              Apt CIH: Handle used by actionscript. A window into the inst data.
        | AptCIH |                  In the future all actionscript releated code should go here.
        |        |
        +---++---+
            ||
            ||
            \/
        +------------------+
        |                  |    Apt Character Instance: An instance of a Character contains all of the
        | AptCharacterInst |        data that is needed to maintain and render the characters. To make
        |                  |        things consistent, this should not care about actionscript, only
        +---++-------------+        about the character instance data, and how it is stored and modified.
            ||
            ||
            \/
        +---------------+
        |               |       Apt Render Item: Character inst data required to render the object.
        | AptRenderItem |           The character instance hides which data is actually stored where.
        |               |           Basically anything the renderer needs is actually hidden in this object.
        +---++----------+           Render items are designed to be very light, and functions that need
            ||
            ||
            \/
        +---------------+
        |               |       Apt Character: This is typically a library item. A prototype in essence.
        | AptCharacter  |           The character can be instantiated multiple places on a screen, each
        |               |           with their own instance data that contains the information about how
        +---------------+           the character is used in each case.



    \Notes
        None.

    \Copyright
        (c) 2004 Electronic Arts Inc.

    \Version 1.0   03/07/04 (ONA) First Version
    \Version 1.1   04.01/05 (ASK) Removed some functions from header file and moved them to cpp file.
                                  Also removed some member variables as those are being moved to RenderItem.
    \Version 2.0   04/13/05 (DC)  Major changes made! All members are now hidden in accessors. Moved
                                  Actionscript Native Functions to here from character inst object.
                                  started const correctness in the file (i.e. made every function
                                  const correct to my knowledge). Organized functions in header.
                                  moved all non-trivial implementations to .inl and .cpp files
                                  respectively. added comments by the keg.
    \Version 2.1   08/8/06 (ASK)  Added AptCIHNativeFunctionHelper class for easier debugging.
                                  Only gets defined if APT_USE_CIH_NATIVEFUNC_HELPER is defined.
    \Version 2.2   09/20/07 (ASK)  Added 2 new member variable bit fields to denote if CIH has blendModes or filters
    \Version 2.3   06/09/09 (DG)  FogBug 1233: AptNativeHash change from Tiger10:
                                  2 methods added to AptCIHNone class:
                                    virtual AptNativeHash * GetNativeHashVirtual()
                                    virtual bool            ContainsNativeHashVirtual() const
    \Version 2.4   07/10/09 (DG)  Make GetFirstChild inline
*/
/************************************************************************************************H*/

#ifndef __APTCIH_H__
#define __APTCIH_H__

/*** Include files ********************************************************************************/
#include "AptCharacterInst.h"       // Defines the AptCharacter inst that we include as a member of this object.

/*** Defines **************************************************************************************/
#define APT_USE_CIH_NATIVEFUNC_HELPER   1
// Created new #defines only for AptCIH NativeFunctions.
#if !defined (APT_USE_CIH_NATIVEFUNC_HELPER)
// if #define is not defined then use the same old macros
#define APT_CIH_NATIVE_MEMBER_FUNCTION(_class_,_name_) NATIVE_MEMBER_FUNCTION(_class_,_name_)
#define APT_CIH_NATIVE_MEMBER_FUNCTION_DISPATCH(_name_) NATIVE_MEMBER_FUNCTION_DISPATCH(_name_)
#define APT_CIH_NATIVE_MEMBER_FUNCTION_DESTROY(_name_) NATIVE_MEMBER_FUNCTION_DESTROY(_name_)

#else
// otherwise use the AptCIHNativeFunctionHelper class to define all the native functions in AptCIH
#define APT_CIH_NATIVE_MEMBER_FUNCTION(_class_,_name_) NATIVE_MEMBER_FUNCTION(AptCIHNativeFunctionHelper,_name_)
#define APT_CIH_NATIVE_MEMBER_FUNCTION_DISPATCH(_name_) \
    if (!AptCIHNativeFunctionHelper::psMethod_##_name_) { \
    AptCIHNativeFunctionHelper::psMethod_##_name_ = new AptNativeFunction(AptCIHNativeFunctionHelper::sMethod_##_name_); \
    AptCIHNativeFunctionHelper::psMethod_##_name_->setGCRoot(1); \
    APT_INC(AptCIHNativeFunctionHelper::psMethod_##_name_); \
    } \
    return AptCIHNativeFunctionHelper::psMethod_##_name_;

#define APT_CIH_NATIVE_MEMBER_FUNCTION_DESTROY(_name_) \
    if (AptCIHNativeFunctionHelper::psMethod_##_name_) { APT_DEC(AptCIHNativeFunctionHelper::psMethod_##_name_); \
    AptCIHNativeFunctionHelper::psMethod_##_name_ = 0; }
#endif

/*** Macros ***************************************************************************************/
/// The CIH pointer to the mask is stored in the hash table since it is infrequently used / accessed.
#define APTCIH_MASK_HASHNAME    ("#!MASKMASTER!#")
#define APTCIH_MASKEDITEM_HASHNAME    ("#!MASKSLAVE!#")

/*** Type Definitions *****************************************************************************/

#if defined APT_USE_CIH_NATIVEFUNC_HELPER
// This class is used to move the static Nativefunctions from AptCIH class to this helper class.
// When we are debugging in debugger and expand any AptCIH, first almost 27 entries are for NativeFunctions
// which are never required to be debugged/looked at and we do not need them in the debugger, it becomes very hard
// to find the actual AptCIH member variables, so created this helper class which will hold all the NativeFunctions for AptCIH.
// if the APT_USE_CIH_NATIVEFUNC_HELPER is not defined everything will work as old way and AptCIH will hold all the
// Native functions.
class AptCIHNativeFunctionHelper
{
public:
    AptCIHNativeFunctionHelper() {} ;

    NATIVE_MEMBER_FUNCTION_DECL(gotoAndStop);
    NATIVE_MEMBER_FUNCTION_DECL(gotoAndPlay);
    NATIVE_MEMBER_FUNCTION_DECL(prevFrame);
    NATIVE_MEMBER_FUNCTION_DECL(nextFrame);
    NATIVE_MEMBER_FUNCTION_DECL(stop);
    NATIVE_MEMBER_FUNCTION_DECL(play);
    NATIVE_MEMBER_FUNCTION_DECL(loadVariables);
    NATIVE_MEMBER_FUNCTION_DECL(attachMovie);
    NATIVE_MEMBER_FUNCTION_DECL(loadMovie);                     // ### New feature
    NATIVE_MEMBER_FUNCTION_DECL(unloadMovie);                   // ### New feature
    NATIVE_MEMBER_FUNCTION_DECL(loadMovieNum);                  // ### New feature
    NATIVE_MEMBER_FUNCTION_DECL(unloadMovieNum);                // ### New feature
    NATIVE_MEMBER_FUNCTION_DECL(duplicateMovieClip);
    NATIVE_MEMBER_FUNCTION_DECL(removeMovieClip);
    NATIVE_MEMBER_FUNCTION_DECL(createTextField);
    NATIVE_MEMBER_FUNCTION_DECL(removeTextField);
    NATIVE_MEMBER_FUNCTION_DECL(getDepth);
    NATIVE_MEMBER_FUNCTION_DECL(getInstanceAtDepth);            // ### New feature    
    NATIVE_MEMBER_FUNCTION_DECL(getBounds);
    NATIVE_MEMBER_FUNCTION_DECL(hitTest);
    NATIVE_MEMBER_FUNCTION_DECL(createEmptyMovieClip);
    NATIVE_MEMBER_FUNCTION_DECL(getNewTextFormat);
    NATIVE_MEMBER_FUNCTION_DECL(getTextFormat);
    NATIVE_MEMBER_FUNCTION_DECL(setTextFormat);
    NATIVE_MEMBER_FUNCTION_DECL(getBytesTotal);
    NATIVE_MEMBER_FUNCTION_DECL(getBytesLoaded);
    NATIVE_MEMBER_FUNCTION_DECL(swapDepths);
    NATIVE_MEMBER_FUNCTION_DECL(setMask);
    NATIVE_MEMBER_FUNCTION_DECL(startDrag);
    NATIVE_MEMBER_FUNCTION_DECL(localToGlobal);
    NATIVE_MEMBER_FUNCTION_DECL(globalToLocal);

};
#endif


#if defined(APT_ENABLE_DEBUG_REPORT)
namespace EA { namespace Apt { namespace Debug { class AptReport; } } }
#endif

class AptCIH : public AptValueGC
{
public:
    APT_VALUE_GC_NEW_DELETE_OPERATORS
    // DC - Added Metric defines
    APT_DECLARE_OBJECT_METRICS

    enum AptCIHState
    {
        AptCIHState_Normal  = 0,     // Normal state of an AptCIH
        AptCIHState_Zombie  = 1,     // The AptCIH is being held onto so the file isn't unloaded
        AptCIHState_Deleted = 2,     // The APtCIH would have been zombied, but the ZombieVector was full
        AptCIHState_Unloaded= 3
    };

    //################################################################################################################
    // Public constructor
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    //################################################################################################################

    explicit AptCIH( AptCharacter *pCharacter, AptCIH *_pParent) ;

    //################################################################################################################
    // Actionable CIH functions. These perform operations (not just get or set).
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    //################################################################################################################
    void            Remove(const bool bDestroyGC);
    void            ClearCIH(const bool bDestroyGC);
    void            ForceCleanNativeHash();
    void            jumpToFrame(int nTargetFrame);
    uint32_t        tick();
    bool            queueClipEvents(int nEventFlags, AptInput input = gNullInput, int bQueuedClipEvent = true); //bQueuedClipEvent = false
    void            AssociateInstToClass() ;    // this associates any class related to this particular character
    void            link(void *pUserData);
    void            EnsureStringAllocated(AptCIH *pParent) ;
    void            DeallocAssetStringRecursive();

    static uint32_t     GeneralisedProcess(AptCIH * pCIH, void * pVoid) ;
    static uint32_t     (*sCIHProcessCb) (AptCIH * pCIH, void * pVoid) ;
    static uint32_t     (*sCIHProcessCb1) (AptCIH * pCIH, void * pVoid) ;
    static uint32_t     (*sCIHProcessCb2) (AptCIH * pCIH, void * pVoid) ;

    static uint32_t     ProcessTextInst(AptCIH * pCIH, void * pVoid) ;
    static uint32_t     ProcessCustomControls(AptCIH * pCIH, void * pVoid) ;
    static uint32_t     ProcessMaskMatricies(AptCIH *pCIH, void *pVoid);
    // added following members for printing debugging information
    static uint32_t     ProcessTextInstPrint(AptCIH * pCIH, void * pVoid);
    static uint32_t     PrintMovieclipTree(AptCIH * pCIH, void * pVoid);
    static bool         bEarlyReturn;       // this is used inside GeneralisedProcess to decide if we want to early return for optimization
    static int32_t      nTreeDepth;         // this is used inside GeneralisedProcess to find at what tree depth we are. this is especially
                                            // used in debugging/printing functions.

#if defined APT_USE_BUTTONS
    static uint32_t     (*sCIHButtonProcessCb) (AptCIH * pCIH, void * pVoid);
    static uint32_t     ProcessButtonState(AptCIH *pCIH, void *pVoid);
    void                gotoState(AptCharacterButtonRecordState nNewState);
#endif
    void                ClearDepends();
    void                ResetInitActions();

    //################################################################################################################
    // AptValue Overrides.  No point in making them inline... they are virtual, duh.
    // ~~~~~~~~~~~~~~~~~~~
    //################################################################################################################
    virtual void            SetHasClass(int bHasClass);
    virtual bool            GetHasClass() const ;
    virtual void            DestroyGCPointers();
    virtual void            RegisterReferences();
    virtual void            PreDestroy();
    virtual AptNativeHash * GetNativeHashVirtual();
    virtual bool            ContainsNativeHashVirtual() const;
    virtual AptValue *      objectMemberLookup(AptValue * const pContext, const AptNativeString * const pName) const;
    virtual bool            objectMemberSet(AptValue * const pContext, const AptNativeString * const pName, AptValue * const pValue);

// DC - Added Increment / Decrement messages. Turn them on with APT_INC_DEC_MESSAGES
#if defined(APT_INC_DEC_MESSAGES)

    virtual void            Release(const char * szFuncName, const char * szFileName, int nLineNumber);
#else

    virtual void            Release();
#endif

    //################################################################################################################
    // Getter and setter functions for internal values. Mainly inline'd.
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    //################################################################################################################

    ///! returns the instance name of the CIH object.
    APT_INLINE const AptNativeString & GetInstanceName() const
    {
        return mMyName;
    }

    ///! Sets a new instance name for the object
    APT_INLINE void SetInstanceName(const AptNativeString & newName)
    {
        mMyName = newName;
    }

    ///! Setting to true signals this object as having been touched by actionscript.
    ///! This is very important knowledge when merging frame actions during a goto and X.
    APT_INLINE void SetASChanged(int bASChanged)
    {
        mbASChange = (bASChanged) ? 1 : 0;
    }

    ///! Setting to true signals this object as having been touched by actionscript.
    ///! This is very important knowledge when merging frame actions during a goto and X.
    APT_INLINE bool  GetASChanged() const
    {
        return mbASChange != 0;
    }

    ///! Just a flag that is set to true when in a constructor.
    ///! This is needed for .super stuff
    APT_INLINE bool IsInCtor() const
    {
        return mbInCtor ;
    }

    ///! Just a flag that is set to true when in a constructor.
    ///! This is needed for .super stuff
    APT_INLINE void SetInCtor(uint32_t bInCtor)
    {
        mbInCtor = bInCtor;
    }

    ///! Sets the status of this object as a zombie
    APT_INLINE void SetCIHState(AptCIHState eState)        // $BUG253 LEO    25/47
    {
        mbCIHState = (0x3 & (int)eState);
    }

    ///! Gets the status of this object as a zombie
    APT_INLINE AptCIHState GetCIHState() const               // $BUG253 LEO    26/47
    {
        return static_cast<AptCIHState>(mbCIHState);
    }

    ///! Increments the zombie count.
    APT_INLINE void IncZombieCount()
    {
        APT_ASSERT(mnZombieCounter < 65536);            // We only allot 16 bits for this counter.      // $BUG253 LEO    27/47
        mnZombieCounter = mnZombieCounter + 1;
    }

    void        DecZombieCount(); // Not suitable for inlining.

    ///! returns the zombie count, which is modified with inc's and dec's.
    APT_INLINE int  GetZombieCount()
    {
        return mnZombieCounter;
    }

    APT_INLINE void                 SetEventHandler(int nEvent);
    APT_INLINE void                 RemoveEventHandler(int nEvent);

    int32_t                         HasEvent(int32_t nEvent);
    int32_t                         HasClipEvent(int32_t nEvent);
    int32_t                         HasEventMember(int32_t nEvent);
    APT_INLINE int                  HasMouseEvent();                   // Checks if this sprite has any AptEventActionFlag_MouseEvents events
    int                             FindAndSetEvents();


    APT_INLINE bool                             IsSpriteInst()      const;
    APT_INLINE bool                             IsSpriteInstBase()  const;
    APT_INLINE bool                             IsButtonInst()      const;
    APT_INLINE bool                             IsShapeInst()       const;
    APT_INLINE bool                             IsDynamicTextInst() const;
    APT_INLINE bool                             IsStaticTextInst()  const;
    APT_INLINE bool                             IsMorphInst()       const;
    APT_INLINE bool                             IsAnimationInst()   const;
    APT_INLINE bool                             IsLevelInst()       const;      // Level is a placeholder object, returns true when Update item is NULL
    APT_INLINE bool                             IsImageInst() const;
    APT_INLINE bool                             IsCustomControlInst() const;

    APT_INLINE bool                             IsNone() const;

    APT_INLINE const AptCharacterInst          *GetCharacterInst()     const;
    APT_INLINE const AptCharacterSpriteInst    *GetSpriteInst()        const;
    APT_INLINE const AptCharacterSpriteInstBase*GetSpriteInstBase()    const;
    APT_INLINE const AptCharacterTextInst      *GetDynamicTextInst()   const;
    APT_INLINE const AptCharacterStaticTextInst*GetStaticTextInst()    const;
    APT_INLINE const AptCharacterMorphInst     *GetMorphInst()         const;
    APT_INLINE const AptCharacterButtonInst    *GetButtonInst()        const;
    APT_INLINE const AptCharacterAnimationInst *GetAnimationInst()     const;
    APT_INLINE const AptCharacterShapeInst     *GetShapeInst()         const;
	// FIFA_BEGIN | DIVERGENCE | nmeyer | 2020-02-03 | Add support for returning image instances.
	APT_INLINE const AptCharacterImageInst     *GetImageInst()         const;
	// FIFA_END

    APT_INLINE AptCharacterInst                *GetCharacterInst() ;
    APT_INLINE AptCharacterSpriteInst          *GetSpriteInst() ;
    APT_INLINE AptCharacterSpriteInstBase      *GetSpriteInstBase() ;
    APT_INLINE AptCharacterTextInst            *GetDynamicTextInst() ;
    APT_INLINE AptCharacterStaticTextInst      *GetStaticTextInst() ;
    APT_INLINE AptCharacterMorphInst           *GetMorphInst() ;
    APT_INLINE AptCharacterButtonInst          *GetButtonInst() ;
    APT_INLINE AptCharacterAnimationInst       *GetAnimationInst() ;
    APT_INLINE AptCharacterShapeInst           *GetShapeInst() ;
	// FIFA_BEGIN | DIVERGENCE | nmeyer | 2020-02-03 | Add support for returning image instances.
	APT_INLINE AptCharacterImageInst           *GetImageInst() ;
	// FIFA_END


    APT_INLINE AptMatrix                       *GetPositionMatrixWritable() ;
    APT_INLINE const AptMatrix                 *GetPositionMatrixConst() const;
    APT_INLINE AptCXForm                       *GetColorMatrixWritable() ;
    APT_INLINE const AptCXForm                 *GetColorMatrixConst() const;

    void                                       SetProceduralProperty(AptProceduralProperty eProperty, float fValue, bool bASFlag = false);
    float                                      GetProceduralProperty(AptProceduralProperty eProperty) const;
    bool                                       GetVisible() const;
    void                                       SetVisible(bool bVisible);

    void                                       GetBoundingRect(AptRenderingContext *pRenderingContext, const AptMatrix * pCurrentTransform, AptRect *pRect) const;
    void                                       GetBoundingRect(AptRect *pRect) const;
    void                                       GetGlobalBoundingRect(AptRect *pRect) const;
    void                                       GetGlobalTranslation(float *pfX, float *pfY) const;

    APT_INLINE AptNativeHash *                 GetNativeHash() const;
    void                                       SetMask(AptCIH * pMask);
    AptCIH *                                   GetMask() const;
    APT_INLINE bool                            IsMask() const ;
    APT_INLINE bool                            HasMask() const ;

    APT_INLINE void                            SetIsMask(bool bNew, AptMatrix *pMatrix);
    APT_INLINE void                            SetHasMask(bool bNew, AptRenderItem *pMask);

    APT_INLINE int32_t                         GetDepth() const;
    APT_INLINE void                            SetDepth(uint32_t nDepth) ;
    void                                       SetCharacterInst(AptCharacterInst * pData, bool bMoveDataFromCurrent);

    // functions for UIEffects
    APT_INLINE bool                             GetHasBlendMode()   const;
    APT_INLINE void                             SetHasBlendMode(uint32_t nBlendMode);
    APT_INLINE bool                             GetHasFilterEffects()   const;
    APT_INLINE void                             SetHasFilterEffects(uint32_t nUIEffect);
    APT_INLINE bool                             GetHasUIEffects() const;

    ///! returns the next item in the display list
    APT_INLINE AptCIH                          *GetDisplayListNext() const
    {
        return mpNext;
    }
    ///! returns the previous item in the display list
    APT_INLINE AptCIH                          *GetDisplayListPrevious() const
    {
        return mpPrev;
    }

    APT_INLINE void SetInRemList(bool bInRemList)
    {
        APT_ASSERT(mpPrev == NULL && mpNext == NULL);
        mbInRemList = bInRemList;
    }

    APT_INLINE bool GetInRemList() const
    {
        return mbInRemList;
    }


    ///! returns the parent of the this.
    APT_INLINE AptCIH                          *GetDisplayListParent() const
    {
        return mpParent;
    }

    void                            ReplaceChild( AptCIH *pNew,  AptCIH *pOld);
    void                            ReplaceZombieChild( AptCIH *pNew,  AptCIH *pOld);
    AptCIH*                         InsertChild(  AptCIH *pSourceCIH, AptCharacter *pNew,  int     nDepth, AptNativeString *sInstanceName, AptValue * pInitObject);
    void                            RemoveChild(  AptCIH *pToRemove );
    void                            SwapChildrenDepths( AptCIH *p0, AptCIH *p1);
    AptDisplayListState            *GetDisplayListState();

    APT_INLINE void                 SetIsInserted     ( );
    APT_INLINE void                 SetDisplayListNext(     AptCIH* pNew);
    APT_INLINE void                 SetDisplayListPrevious( AptCIH* pNew);
    APT_INLINE void                 SetDisplayListParent(   AptCIH* pNew);


    ///! Gets what frame this was created on.
    ///! This is needed for merging states and frame actions in goto and XXX calls.
    APT_INLINE int                  GetCreatedOnFrame() const
    {
        return mnCreatedOnFrame;
    }

    ///! Sets what frame this was created on.
    ///! This is needed for merging states and frame actions in goto and XXX calls.
    APT_INLINE void                 SetCreatedOnFrame(int newValue)
    {
        mnCreatedOnFrame = newValue;
    }


    //################################################################################################################
    // Utility / conversion Functions. These are calculated values, not directly stored values.
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    //################################################################################################################

    static float        GetCosAngle(const AptMatrix * const matrix);
    static float        GetVectorLength(const AptMatrix * const matrix);
    int                 GetParentCount()                                const;    // Returns the depth based on the parent chain
    int                 GetDepthOfParentAt(int nLvl)                    const;    // Returns the level of the parent at the given depth
    bool                CheckIfHigher(const AptCIH *pTmp)               const;    // Checks if the passed in sprite is at a higher level
    bool                IsVisible()                                     const;    // Looks up the parent chain to see if this guy is visible
    bool                IsParent(AptCIH *pParentTmp)                    const;    // Checks if passed AptCIH is in parent chain
    bool                HasRenderData()                                 const;    // Checks if the CIH has any render/displayable data.
    AptCIH *            GetRootAnimation();                                       // Returns the level AptCIH this is in. Not const becuase it could return this object
    void                GetMovieclipInfo(AptMovieclipInformation * pMCInfo, bool bRecurse = true) const;
    APT_INLINE bool     GetIsPlaying() const;
    void                SetIsPlaying(bool bFlag);
    void                SetDirtyState(bool bFlag, bool bTraverse);                // Sets the dirty state
    APT_INLINE bool     GetDirtyState()                                 const;    // Returns the dirty state

    APT_INLINE void     SetSkipEval(bool bSkipEval)
    {
        mbSkipEval = (bSkipEval ? 1 : 0);
    }

    APT_INLINE bool     IsSkipEval() const
    {
        return (mbSkipEval == 1);
    }

    APT_INLINE const AptCIH *GetFirstChild() const;
    bool                IsFirstChild() const;
    bool                IsDisplayListHead() const;
    void                MultParentMatrix(const AptCIH *pCur, AptMatrix &cur) const;

    //################################################################################################################
    // Debugger functions.
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    //################################################################################################################

#if defined(APT_ENABLE_DEBUG_REPORT)
    static void SetReporter(EA::Apt::Debug::AptReport* report) { mReport = report; }
    APT_INLINE const AptNativeString & GetTrackLabel() const
    {
        return mLabel;
    }
#endif

#if !defined APT_USE_CIH_NATIVEFUNC_HELPER
    //################################################################################################################
    // Native Functions (Actionsript accessable functions)
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    //################################################################################################################
    NATIVE_MEMBER_FUNCTION_DECL(gotoAndStop);
    NATIVE_MEMBER_FUNCTION_DECL(gotoAndPlay);
    NATIVE_MEMBER_FUNCTION_DECL(prevFrame);
    NATIVE_MEMBER_FUNCTION_DECL(nextFrame);
    NATIVE_MEMBER_FUNCTION_DECL(stop);
    NATIVE_MEMBER_FUNCTION_DECL(play);
    NATIVE_MEMBER_FUNCTION_DECL(loadVariables);
    NATIVE_MEMBER_FUNCTION_DECL(attachMovie);
    NATIVE_MEMBER_FUNCTION_DECL(loadMovie);                     // ### New feature
    NATIVE_MEMBER_FUNCTION_DECL(unloadMovie);                   // ### New feature
    NATIVE_MEMBER_FUNCTION_DECL(loadMovieNum);                  // ### New feature
    NATIVE_MEMBER_FUNCTION_DECL(unloadMovieNum);                // ### New feature
    NATIVE_MEMBER_FUNCTION_DECL(duplicateMovieClip);
    NATIVE_MEMBER_FUNCTION_DECL(removeMovieClip);
    NATIVE_MEMBER_FUNCTION_DECL(createTextField);
    NATIVE_MEMBER_FUNCTION_DECL(removeTextField);
    NATIVE_MEMBER_FUNCTION_DECL(getDepth);
    NATIVE_MEMBER_FUNCTION_DECL(getInstanceAtDepth);            // ### New feature  
    NATIVE_MEMBER_FUNCTION_DECL(getBounds);
    NATIVE_MEMBER_FUNCTION_DECL(hitTest);
    NATIVE_MEMBER_FUNCTION_DECL(createEmptyMovieClip);
    NATIVE_MEMBER_FUNCTION_DECL(getNewTextFormat);
    NATIVE_MEMBER_FUNCTION_DECL(getTextFormat);
    NATIVE_MEMBER_FUNCTION_DECL(setTextFormat);
    NATIVE_MEMBER_FUNCTION_DECL(getBytesTotal);
    NATIVE_MEMBER_FUNCTION_DECL(getBytesLoaded);
    NATIVE_MEMBER_FUNCTION_DECL(swapDepths);
    NATIVE_MEMBER_FUNCTION_DECL(setMask);
    NATIVE_MEMBER_FUNCTION_DECL(startDrag);
    NATIVE_MEMBER_FUNCTION_DECL(localToGlobal);
    NATIVE_MEMBER_FUNCTION_DECL(globalToLocal);
#else
    static AptCIHNativeFunctionHelper  sCIHNativeHelper;
#endif

    static void CleanNativeFunctions();


    // TODO: Should move this into private at some time...
    static int              sEmptyAssetString;

protected:

    virtual ~AptCIH();

public:
    static AptValue *_gotoAndX(AptValue *pThis, int nParams, int bPlay);
private:
    static uint32_t GetBlendMode(AptValue * const pObject);

    AptCIH();               //  Can't use this constructor


    AptNativeString         mMyName;

#if defined(APT_ENABLE_DEBUG_REPORT)
    AptNativeString         mLabel;
#endif

    // Note that There are too many bits here to fit into one 32 bt value. We should try to remedy that.

    uint32_t                mbASChange:1;       // Flag to know if SetProceduralProperty was called in ActionScript (bug258)
    uint32_t                mbCIHState:2;       // Flag to indicate if this AptCIH is in a zombie state (0 = no, 1 = yes, 2 = zombie vector was full)     $BUG253 LEO    31/47
    uint32_t                mbHasClass:1;       // ### LEO Flag added to indicate class hierarchy
    uint32_t                mbInCtor:1 ;        // added for super problem, especially for AS2.0
    uint32_t                mbInRemList:1 ;     // added to quickly see if item is in Rem list
    uint32_t                mbDirty:1;          // Flag to determine if we need to tick down
    uint32_t                mbHasBlendMode:1;   // Flag to determine if CIH has UIEffect - blendmode associated with it.
    uint32_t                mbHasFilters  :1;   // Flag to determine if CIH has UIEffect - filters associated with it.

    uint32_t                mbSkipEval:1;       // Flag to determine if we need to skip evaluation of this node

    // $senzee-lockless the following are kept for binary compatibility with decoupled
    uint32_t                mbGPDirty:1;        // Flag to determine if we need to generalized process tick down // $senzee-SDL won't be used
    uint32_t                mbChildGPDirty:1;   // Flag to determine if we need to generalized process tick down // $senzee-SDL won't be used
    // $senzee-lockless end

    // changing it to int as we need to set mnCreatedOnFrame to -1
    int32_t                mnZombieCounter:16;  // $BUG253 LEO    30/47
    int32_t                mnCreatedOnFrame:14; // (LEO) $BUG670    3/5

    // Display list tree management objects.
    AptCIH *                mpPrev;
    AptCIH *                mpNext;
    AptCIH *                mpParent;

    // The character inst has the render and update items, and abstracts from the caller what info is stored where.
    AptCharacterInst       *mpCharacterInst;

    float * fRot; // This can probably be removed at a later date.

#if defined(APT_ENABLE_DEBUG_REPORT)
    static EA::Apt::Debug::AptReport* mReport;
#endif

    // Friends... Right now DisplayListState is given access to AptCIH goodies. Probably not the best, but whatever.
    friend struct AptDisplayListState;
};

//! use this for declaring the new instances array in AptAnimationPoolData
typedef AptCIH* AptCIHPtr;

// (LEO) This class was created to act as a replacement AptCIH for any AptCIH's that are being deleted
// and that are leaving references behind.
class AptCIHNone : public AptCIH
{
public:
    AptCIHNone() : AptCIH(NULL,NULL)
    {
        // Overwrite the value type.
        mValueBitfield.meValueType = AptVFT_CIHNone;
        setIsDefined(1);
        setRefCount(MAX_REFCOUNT);
    }

    virtual AptNativeHash * GetNativeHashVirtual() { APT_ASSERTM(false, "Trying to get a NativeHash for a deleted CIH (no class)"); return NULL; }
    virtual bool            ContainsNativeHashVirtual() const { /*If they're just checking, we don't need to assert.*/ return false; }

    // This Optimizes global objects by doing nothing on Add/Release
#if defined(APT_INC_DEC_MESSAGES)
    virtual void    AddRef(const char * szFuncName, const char * szFileName, int nLineNumber)  { }   //  Do nothing...
    virtual void    Release(const char * szFuncName, const char * szFileName, int nLineNumber) { }   //  Do nothing...
#else
    virtual void    AddRef()  { }   //  Do nothing...
    virtual void    Release() { }   //  Do nothing...
#endif

protected:

    virtual ~AptCIHNone()
    {
        // Do Nothing
    }
};

// This is a helper class that creates static AptCharacters for dynamically created Text and Movie instances.
class AptCharacterHelper
{
protected:
APT_ACCESS_INTERNAL:
    static void Initialize();
    static void Shutdown();
    static AptCharacter *GetAptTextCharacter();
    static AptCharacter *GetAptMovieCharacter();
private:
    //  The user can't call new and delete, need to call Create() and Destroy()
    APT_NEW_DELETE_OPERATORS
    APT_DECLARE_OBJECT_METRICS

    AptCharacterHelper();
    ~AptCharacterHelper();

    static void CreateTextCharacterInst();
    static void CreateMovieCharacterInst();

    static AptCharacter *s_pDynamicText;
    static AptCharacter *s_pDynamicMovie;
};

/*** Variables ************************************************************************************/

/*** Functions ************************************************************************************/

#endif  //  __APTCIH_H__

/*H*************************************************************************************************
    EOF
*************************************************************************************************H*/

