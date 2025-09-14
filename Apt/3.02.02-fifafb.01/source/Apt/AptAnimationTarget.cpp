/*H************************************************************************************************/
/*!

    \File    Apt\AptAnimationTarget.cpp

    \Description
        Implementations of functions for AptAnimationTarget

    \Notes
        None.

    \Copyright
        (c) 2005 Electronic Arts Inc.

    \Version    1.0        08/30/05 (LEO) First Version
    \Version    2.0        01/08/07 (JP)  Added support for analog triggers (issue 106856)
    \Version    2.1        12/05/07 (ASK) Added optimizations in CleanRemList function.
                                          Also added lot of function headers.
    \Version    2.2        07/29/08 (RB)  EATech 114985: Added ability to track AptHighwaterMemoryMetrics
                                          for mInputSet, mListenerSet, etc.
    \Version    2.3        01/22/09 (RB)  EATech 115181: Reverse order of button events in DoButtonActions
    \Version    2.4        01/22/09 (RB)  EATech 115120: Added support for hit-testing masked movieclips and buttons.
    \Version    2.5        02/04/09 (RB)  Prevent extra onRollOver / onRollOut mouse events from being fired.
    \Version    2.6        03/23/09 (RB)  Call TickNewInsts after running function actions in runActions.
    \Version    2.7        04/30/09 (RB)  Added information in the ActionScript callstack for MovieClip events:
                                          the SWF filename and the event description (such as "<onEnterFrame>").
    \Version    2.8        07/08/09 (RB)  Don't call ForceDelete in CleanRemList when refcount != 1 in case object is
                                          referred to by something in gpValuesToRelease. Prevents a crash, but may
                                          cause a rare, slow leak until AptShutdown.
    \Version    2.9        07/10/09 (RB)  Don't loop thru sapDelayedReleaseList in AddToRemList
*/
/************************************************************************************************H*/


/*** Include files ********************************************************************************/
#include "AptAnimationTarget.h"
#include "AptRenderingContext.h"
#include "AptAnimationTargetButtonFunction.h"
#include "MainInline.h"

#include "AptBCRenderTree.h"
#include "AptMemoryMetricsImpl.h"
#include "AptCallStack.h"
#include "AptDebugger/AptDebugger.h"

#include "AptProfile.h"

/*** Defines **************************************************************************************/
#define APT_SAVEINPUTS_RECORD_SIZE       8

/*** Macros ***************************************************************************************/

/*** Type Definitions *****************************************************************************/
struct ListenerEventType
{
    int32_t     nFlag;
    StringCode  eName;
};


static ListenerEventType _aListenerEvents[] =
{
    { AptEventActionFlag_KeyUp,     SC_onKeyUp      },
    { AptEventActionFlag_KeyDown,   SC_onKeyDown    },
    { AptEventActionFlag_MouseDown, SC_onMouseDown  },
    { AptEventActionFlag_MouseUp,   SC_onMouseUp    },
    { AptEventActionFlag_MouseMove, SC_onMouseMove  },
    { AptEventActionFlag_Wheel,     SC_onMouseWheel }
};

/*** Function Prototypes **************************************************************************/

/*** Variables ************************************************************************************/
#if defined(APT_ALTERNATE_INPUT)
APT_DEFINE_OBJECT_METRICS_EXPLICIT(AptAltInput, NULL);
#endif
// Private variables

AptAnalogStickInfo AptAnimationTarget::sgAStickLeft[AptInputController_NumControllers];
AptAnalogStickInfo AptAnimationTarget::sgAStickRight[AptInputController_NumControllers];
AptAnalogStickInfo AptAnimationTarget::sgATriggers[AptInputController_NumControllers];  // Analog triggers data JP (issue 106856)

int32_t     AptAnimationTarget::snXMousePos = 0;
int32_t     AptAnimationTarget::snYMousePos = 0;

#if defined APT_USE_BUTTONS
int32_t     AptAnimationTarget::snBILCount = 0;              // count of the used elements in buttonInstanceList
ButtonHitTestRecord *AptAnimationTarget::saButtonInstanceList = NULL;  // content of the array will be clean and refilled every frame, these are now allocated at runtime in constructor
int32_t     AptAnimationTarget::siButtonInstanceListSize = 0;
#endif

int32_t     AptAnimationTarget::siMaxNewMovieClips = 0;             // Although the vectors will grow to any size, the max count will still assert to keep our memory in track.
int32_t     AptAnimationTarget::siNewMovieClipsGrowSize = 1024;
AptAnimationCihList AptAnimationTarget::sapNewInsts;                // Vector to track newly constructed CIH objects.
int32_t     AptAnimationTarget::snNewInsts = 0;
AptAnimationCihList AptAnimationTarget::sapDelayedReleaseList;      // Vector to track CIH objects to be deleted on the next frame update.
int32_t     AptAnimationTarget::snDelayedReleaseCount = 0;
int32_t     AptAnimationTarget::snDelayedReleaseCountHW = 0;

// Public variables

extern AptCallStack *gAptOptCallStack;

/*** Private Functions ****************************************************************************/

namespace {

#if defined APT_USE_MOUSE
    /*F************************************************************************************************/
    /*!
        \Function    IsPointInRect

        \Description Finds if a screen point is inside a transformed rectangle

        \Input      rect              -   Original rectangle
        \Input      mat               -   Transformation matrix
        \Input      x                 -   x coordinate of the screen point
        \Input      y                 -   y coordinate of the screen point

        \Output     bool              -   True if the point is inside the rectangle, false if it isn't

        \Notes      None.

        \Version    1.0        07/01/08 (JP)  Encapsulating this method
    */
    /************************************************************************************************F*/

    static bool IsPointInRect(AptRect &rect, const AptMatrix &mat, int32_t x, int32_t y)
    {
         float l = rect.fLeft;
         float r = rect.fRight;
         float t = rect.fTop;
         float b = rect.fBottom;
         float x0 = mat.a * l + mat.b * t + mat.tx;
         float y0 = mat.c * l + mat.d * t + mat.ty;
         float x1 = mat.a * r + mat.b * b + mat.tx;
         float y1 = mat.c * r + mat.d * b + mat.ty;
         return (x > x0 && x < x1 && y > y0 && y < y1);
    }

    /*F************************************************************************************************/
    /*!
        \Function    IsPointInCIH

        \Description Finds if a screen point is inside an AptCIH

        \Input      pCIH              -   Tested AptCIH
        \Input      x                 -   x coordinate of the screen point
        \Input      y                 -   y coordinate of the screen point

        \Output     bool              -   True if the point is in the pCIH area, false otherwise

        \Notes      Created to reduce the hit area of maskes buttons
                    By now, we are only testing the bounding rectangle of the AptCIH
                    So if the mask is irregular the test is not accurate

        \Version    1.0        07/01/08 (JP)  First version
    */
    /************************************************************************************************F*/

    static bool IsPointInCIH(AptCIH *cih, int32_t x, int32_t y)
    {
        AptRect rect;
        cih->GetGlobalBoundingRect(&rect);
        return (x > rect.fLeft && x < rect.fRight && y > rect.fTop && y < rect.fBottom);
    }

    /*F************************************************************************************************/
    /*!
        \Function    PointHits

        \Description
            Does a point hit test for the passed in sprite

        \Input      cih               - pointer to movieclip
        \Input      x, y              - point to hit-test against

        \Output     bool              - true if hit point is inside bounding box of movieclip.

        \Notes
            None.

        \Version    1.0        09/01/05 (LEO) First Version
        \Version    1.1        01/22/09 (RB)  Refactored into non-member function; hit-test against mask.
                                              Added x,y params and removed unused 'input' param.

    */
    /************************************************************************************************F*/
    static bool PointHits( AptCIH* cih, int32_t x, int32_t y )
    {
        bool hits = IsPointInCIH(cih, x, y);
        if (hits)
        {
            AptCIH *mask = cih->GetMask();
            if ( mask )
            {
                // hit-test against mask
                hits = IsPointInCIH(mask, x, y);
            }

            // hit-test any other masks in mask layer
            for(AptCIH *prev = cih->GetDisplayListPrevious(); hits && prev; prev = prev->GetDisplayListPrevious())
            {
                if(prev->GetCharacterInst()->GetRenderItem()->GetClipDepth() >= 0)  // It's a clipper
                {
                    hits =  IsPointInCIH(prev, x, y);
                }
            }
        }
        return hits;
    }
#endif //APT_USE_MOUSE

    /*F************************************************************************************************/
    /*!
        \Function    _GetMovieClipEventFunctionDescription

        \Description
            Tries to lookup a friendly name for a given AptActionType. Used for gAptOptCallStack
            in AptAnimationTarget::RunActions().

        \Input      cur               - contains the AptActionType to lookup
        \Output     char8_t*          - name of movieclip event; will be simply "<movieclipevent>"
                                        unless EA_DEBUG or APT_DO_AS_PROFILE is defined.

        \Version    1.0        04/30/09 (RB)  First Version
    */
    /************************************************************************************************F*/
    inline static const char8_t* _GetMovieClipEventFunctionDescription( const AptActionQueueC::AptActionPool* cur )
    {
        static const char8_t DEFAULT_DESC[] =       "<movieclipevent>";
#if defined (EA_DEBUG) || defined (APT_DO_AS_PROFILE)
        static const char8_t LOAD_DESC[] =          "<onLoad>";
        static const char8_t ENTERFRAME_DESC[] =    "<onEnterFrame>";
        static const char8_t UNLOAD_DESC[] =        "<onUnload>";
        static const char8_t MOUSEMOVE_DESC[] =     "<onMouseMove>";
        static const char8_t MOUSEDOWN_DESC[] =     "<onMouseDown>";
        static const char8_t MOUSEUP_DESC[] =       "<onMouseUp>";
        static const char8_t KEYDOWN_DESC[] =       "<onKeyDown>";
        static const char8_t KEYUP_DESC[] =         "<onKeyUp>";
        static const char8_t DATA_DESC[] =          "<onData>";
        static const char8_t INITIALIZE_DESC[] =    "<onInitialize>";
        static const char8_t PRESS_DESC[] =         "<onPress>";
        static const char8_t RELEASE_DESC[] =       "<onRelease>";
        static const char8_t RELEASEOUTSIDE_DESC[] ="<onReleaseOutside>";
        static const char8_t ROLLOVER_DESC[] =      "<onRollOver>";
        static const char8_t ROLLOUT_DESC[] =       "<onRollOut>";
        static const char8_t DRAGOVER_DESC[] =      "<onDragOver>";
        static const char8_t DRAGOUT_DESC[] =       "<onDragOut>";
        static const char8_t KEYPRESS_DESC[] =      "<onButtonKeyPress>";
        static const char8_t MOUSEWHEEL_DESC[] =    "<onMouseWheel>";

        switch( cur->eAptActionType )
        {
        case AptActionType_OnLoad:     return LOAD_DESC;
        case AptActionType_EnterFrame: return ENTERFRAME_DESC;
        case AptActionType_Unload:     return UNLOAD_DESC;
        case AptActionType_MouseMove:  return MOUSEMOVE_DESC;
        case AptActionType_MouseDown:  return MOUSEDOWN_DESC;
        case AptActionType_MouseUp:    return MOUSEUP_DESC;
        case AptActionType_KeyDown:    return KEYDOWN_DESC;
        case AptActionType_KeyUp:      return KEYUP_DESC;
        case AptActionType_Data:       return DATA_DESC;
        case AptActionType_Initialize: return INITIALIZE_DESC;
        case AptActionType_Press:      return PRESS_DESC;
        case AptActionType_Release:    return RELEASE_DESC;
        case AptActionType_ReleaseOutside: return RELEASEOUTSIDE_DESC;
        case AptActionType_RollOver:   return ROLLOVER_DESC;
        case AptActionType_RollOut:    return ROLLOUT_DESC;
        case AptActionType_DragOver:   return DRAGOVER_DESC;
        case AptActionType_DragOut:    return DRAGOUT_DESC;
        case AptActionType_KeyPress:   return KEYPRESS_DESC;
        case AptActionType_Wheel:      return MOUSEWHEEL_DESC;
        default:
            break;
        }
#endif // defined (EA_DEBUG) || defined (APT_DO_AS_PROFILE)
        return DEFAULT_DESC;
    }
}

/*** Private Methods ******************************************************************************/

/*F************************************************************************************************/
/*!
    \Function    AptAnimationTarget::_addListenerToQueue

    \Description
        Adds the event handler to action queue.

    This gets called from the event handling functions of Apt especially for listener objects.

    \Input      pValue            - AptValue that gets added to listener queue
    \Input      nEventFlags       - Event flags
    \Input      input             - Actual AptInput that came in

    \Output
        None.

    \Notes
        It basically goes thru the parent hierarchy chain looking for event handler and then
        queues it to action queue.

    \Version    1.0        06/16/04 (ASK) First Version
    \Version    2.0        07/21/04 (LEO) F#563 Mouse Drag and Mouse Wheel support request.
    \Version    2.1        09/15/04 (DC)  Made Script Function Type agnostic.

*/
/************************************************************************************************F*/
void AptAnimationTarget::AddListenerToQueue(AptValue * pValue, int32_t nEventFlags, AptInput input)
{
    APT_PROFILE_INPUT(_addListenerToQueue) ;
    // (LEO) jump out early if the sprite doesn't handle the event.
    APT_ASSERT(pValue->ContainsNativeHashVirtual());
    if(pValue->isCIH() && !pValue->c_cih()->HasEvent(nEventFlags))
    {
        return;
    }
    // Added extra check for mp__proto__
    AptNativeHash * pHash = pValue->GetNativeHashVirtual() ;
    if(!pValue->isCIH() && !pHash->HasEventHandler(nEventFlags) &&
        ( pHash->Get__Proto__() && !pHash->Get__Proto__()->GetNativeHashVirtual()->HasEventHandler(nEventFlags))) // $bug771 - MCB - Check __proto__ for event flag as well
    {
        return;
    }

    for (int32_t i = 0; i < APT_ARRAYSIZE(_aListenerEvents); i++)
    {
        if (_aListenerEvents[i].nFlag & nEventFlags)
        {
            AptValue *pValue1 = pValue->findChild(StringPool::GetString(_aListenerEvents[i].eName), NULL) ;
            if (pValue1 && pValue1->getIsDefined() && pValue1->isScriptFunction())
            {
                // DC 18.00 - Changed to use AptScriptFunctionBase (Reduces duplication of code).
                // This also is much more correct from a scoping perspective because the function
                // prototype and such is copied as well (it wasn't before).
                AptScriptFunctionBase * pFuncBase = pValue1->c_scriptfunction() ;
                if (pFuncBase->mpCIH != pValue)
                {
                    //$bug734 - MCB - set mpParentAnim to the CIH that created the script function, so the correct hash table is used
                    //                to find the script function.
                    // TODO: find another way to fix this...
                    AptCIH * pTempCIH = pFuncBase->mpCIH;
                    //pFuncBase = pFuncBase->Duplicate( pFuncBase->mpFuncAnim, (AptCIH*) pValue);
                    pFuncBase = pFuncBase->Duplicate((AptCIH*) pValue);
                    pFuncBase->mpParentAnim->DecZombieCount();          // (LEO) We also need to dec the Zombie counter!
                    APT_DEC(pFuncBase->mpParentAnim);
                    pFuncBase->mpParentAnim = (AptCIH *) pTempCIH;      // (LEO) DOING THIS IS REALLY REALLY BAD, WE SHOULD NEVER CHAGNE THE mpParentAnim POINTER SINCE IT IS USED TO MAINTAIN THE ZOMBIE REFERENCES!!!!
                    pFuncBase->mpParentAnim->IncZombieCount();          // (LEO) Now we need to inc the new parents zombie counter
                    APT_INC(pFuncBase->mpParentAnim);

                    pFuncBase->setGCRoot(1);
                }

                AddFunctionBack((AptCIH *)pValue,
                    pFuncBase,
                    pFuncBase->GetNumArguments(),
                    ACTION_PROF_CALL_PARAM(StringPool::GetString(_aClipEvents[i].eName)->GetBuffer())
                    ACTION_TYPE_CALL_PARAM(_aClipEvents[i].nFlag)
                    input);
            }
        }
    }
}

/*F************************************************************************************************/
/*!
    \Function    AptAnimationTarget::ProcessListenerEvents

    \Description
        Executes input for all items in the mListenerSet and mMouseListenerSet

    Runs through all items in the listener set and handles the passed in input on them

    \Input      eType             - type of input event
    \Input      eState            - state of the input
    \Input      input             - actual input value

    \Output
        None.

    \Notes
        None.

    \Version    1.0        09/10/04 (LEO) First Version

*/
/************************************************************************************************F*/
void AptAnimationTarget::ProcessListenerEvents(AptInputType eType, AptInputState eState, AptInput input, AptInputController eController)
{
    APT_PROFILE_INPUT(_processListenerEvents);
    APT_ASSERT(eState == AptInputState_Pressed || eState == AptInputState_Released || eState == AptInputState_WheelUp || eState == AptInputState_WheelDown || eState == AptInputState_MouseMove);
    AptValue *pValue = NULL;
    int32_t nCount = 0;

#if defined(APT_USE_MOUSE)
    if(eController == AptInputController_Mouse && ((eType == AptInputType_MouseButton0 && (eState == AptInputState_Pressed || eState == AptInputState_Released)) || eState == AptInputState_WheelDown || eState == AptInputState_WheelUp))
    {
        nCount = 0;
        for (int32_t i = 0; i < mMouseListenerSet.capacity(); i++)
        {
            if(nCount == mMouseListenerSet.mnElements)
            {
                break;
            }
            if (mMouseListenerSet.maElements[i] == NULL)
            {
                continue;
            }
            pValue = mMouseListenerSet.maElements[i];

            switch (eState)
            {
            case AptInputState_Pressed:
                APT_ASSERT(eType == AptInputType_MouseButton0)
                    AddListenerToQueue(pValue, AptEventActionFlag_MouseDown, input);
                break;
            case AptInputState_Released:
                APT_ASSERT(eType == AptInputType_MouseButton0)
                    AddListenerToQueue(pValue, AptEventActionFlag_MouseUp, input) ;
                break;
            case AptInputState_WheelUp:                             // F#563    8
            case AptInputState_WheelDown:
                if(pValue->isCIH() && pValue->c_cih()->IsDynamicTextInst())        // (LEO) added support for TextMouseWheel   4/
                {
                    if(mpTopMostSprite == pValue)
                    {
                        int32_t nTmp = eType;
                        if(eState == AptInputState_WheelDown)
                        {
                            nTmp *= -1;
                        }
                        AptValue *nPos = AptInteger::Create(nTmp);
                        AptNativeString sScroll = "scroll";
                        pValue->c_cih()->objectMemberSet(pValue, &sScroll, nPos);
                    }
                }
                else
                {
                    AddListenerToQueue(pValue, AptEventActionFlag_Wheel, input);
                }
                break;
            default:
                APT_ASSERT(NOT_REACHED);
                break;
            }
            nCount++;
        }
    }
    else
#endif
    {
        if(eState == AptInputState_Pressed || eState == AptInputState_Released)
        {
            // ok now queue up the clipevent actions that need keydown/up events
            for (int32_t i = 0; i < mListenerSet.capacity(); i++)
            {
                if(nCount == mListenerSet.mnElements)
                {
                    break;
                }
                if (mListenerSet.maElements[i] == NULL)
                {
                    continue;
                }
                pValue = mListenerSet.maElements[i];

                switch (eState)
                {
                case AptInputState_Pressed:
                    AddListenerToQueue(pValue, AptEventActionFlag_KeyDown, input) ;
                    break;
                case AptInputState_Released:
                    AddListenerToQueue(pValue, AptEventActionFlag_KeyUp, input) ;
                    break;
                default:
                    break;
                }
                nCount++;
            }
        }
    }
}

/*F************************************************************************************************/
/*!
    \Function    AptAnimationTarget::ProcessInputSet

    \Description
        Handles input from user

    This function runs through all the currently listening sprites in the inputSet and handles the passed in
    input for each sprite

    \Input      eType             - type of input
    \Input      eState            - the state of the input
    \Input      input             - the actual input
    \Input      bCheckTop         - flag

    \Output
        None.

    \Notes
        None.

    \Version    1.0        09/09/04 (LEO) First Version

*/
/************************************************************************************************F*/
void AptAnimationTarget::ProcessInputSet(AptInputType eType, AptInputState eState, AptInput input, AptInputController eController, bool bCheckTop)
{

    AptCIH *pSprInst = NULL;
    bool bQueuedKeyPress = false;
#if defined(APT_USE_MOUSE)
    AptValue *pCurrentTopSprite = gpUndefinedValue;
    bool bPressRelease = false;
    bool bPress = false;
#endif
    int32_t nCount = 0;

    for (int32_t i = 0; i < mInputSet.capacity(); i++)
    {
        if(nCount == mInputSet.mnElements)
        {
            break;
        }
        if (mInputSet.maElements[i] == NULL)
        {
            continue;
        }
        pSprInst = mInputSet.maElements[i];

        if(IsInputMasked(pSprInst))
        {
            continue;
        }
        APT_ASSERT(mInputSet.maElements[i]->isCIH());

        #if defined(APT_USE_MOUSE)
        if(bCheckTop)
        {
            // (LEO) Here we get the top most sprite over the current mouse position, we only do this once, bCehckTop should be false everytime except the first time.
            if(pSprInst->IsVisible() && pSprInst->HasMouseEvent() && PointHits(pSprInst, snXMousePos, snYMousePos))  // (LEO) added support for TextField MouseWheel  2/
            {
                if(pCurrentTopSprite->isUndefined() || (!pSprInst->IsParent(pCurrentTopSprite->c_cih()) && pSprInst->CheckIfHigher(pCurrentTopSprite->c_cih())))
                {
                    pCurrentTopSprite = pSprInst;
                }
            }
        }

        if(eController == AptInputController_Mouse)
        {
            // Now we handle the input event
            switch (eState)
            {
            case AptInputState_Pressed:
                APT_ASSERT(eType == AptInputType_MouseButton0);
                {APT_PROFILE_INPUT(_processKeyboardInput_Mousedown) ;
                pSprInst->queueClipEvents(AptEventActionFlag_MouseDown, input);
                bPress = true;
                }
                break;
            case AptInputState_Released:
                APT_ASSERT(eType == AptInputType_MouseButton0);
                {APT_PROFILE_INPUT(_processKeyboardInput_MouseUp) ;
                pSprInst->queueClipEvents(AptEventActionFlag_MouseUp, input);
                bPressRelease = true;
                }
                break;
            case AptInputState_WheelUp:         // (LEO) 07/21/04 these two can pass here since _ProcessMouseListenerEvents will take care of these events
            case AptInputState_WheelDown:
                break;
            case AptInputState_MouseMove:
                if(!pSprInst->IsDynamicTextInst())                                             // (LEO) skip this if the pSprInst is a textfield   1/
                {
                    pSprInst->queueClipEvents(AptEventActionFlag_MouseMove, input);
                }
                break;
            default:
                APT_ASSERT(NOT_REACHED);
                break;
            }
        }
        else
        #endif
        {
            // Now we handle the input event
            switch (eState)
            {
            case AptInputState_Pressed:
                if(INPUT_IS_ANALOG(&input))
                {
                    APT_ASSERT((eType==AptInputType_LeftAnalogStick) || (eType==AptInputType_RightAnalogStick));
                    pSprInst->queueClipEvents(AptEventActionFlag_KeyDown, input, false);
                }
                else if(INPUT_IS_GESTURE(&input))
                {
                    APT_PROFILE_INPUT(_processKeyboardInput_keyDown) ;
                    pSprInst->queueClipEvents(AptEventActionFlag_KeyDown, input, false);
                }
                else if(INPUT_IS_KEY(&input))
                {
                    APT_PROFILE_INPUT(_processKeyboardInput_keyDown) ;
                    pSprInst->queueClipEvents(AptEventActionFlag_KeyDown, input, false);
                    if (!bQueuedKeyPress)
                    {
                        bQueuedKeyPress = pSprInst->queueClipEvents(AptEventActionFlag_KeyPress, input);
                    }
                }
                break;
            case AptInputState_Released:
                if(INPUT_IS_KEY(&input))
                {
                    {
                        APT_PROFILE_INPUT(_processKeyboardInput_keyUp) ;
                        pSprInst->queueClipEvents(AptEventActionFlag_KeyUp, input, false);
                    }
                }
                break;
            default:
                APT_ASSERT(NOT_REACHED);
                break;
            }
        }
        nCount++;
    }

#if defined APT_USE_BUTTONS
    // button keypresses
    if (eState == AptInputState_Pressed && mButtonSet.mnElements != 0 && bQueuedKeyPress == false)        // NO idea what this does, come back and comment later
    {
        nCount = 0;
        for (int32_t i = 0; i < mButtonSet.capacity(); i++)
        {
            if(nCount == mButtonSet.mnElements)
            {
                break;
            }
            if (mButtonSet.maElements[i] == NULL)
            {
                continue;
            }
            AptCIH *pInst = mButtonSet.maElements[i];

            if(IsInputMasked(pInst))
            {
                continue;
            }

            APT_ASSERT(pInst->IsButtonInst());

            const AptCharacter *pChar = pInst->GetCharacterInst()->GetCharacterConst();
            for (int32_t j = 0; j < pChar->button.nActionConditions; j++)
            {
                if (pChar->button.aActionConditions[j].nConditions & AptActionConditionFlag_KeyPress)
                {
                    int32_t nKey = (pChar->button.aActionConditions[j].nConditions & AptActionConditionFlag_KeyPress) >> 9;
                    if (eType == nKey)
                    {
                        AddActionBack(&pChar->button.aActionConditions[j].actions, pInst->GetDisplayListParent(),
                            ACTION_PROF_CALL_PARAM("ButtonKeyPress")
                            ACTION_TYPE_CALL_PARAM(AptActionType_ButtonActions)
                            gNullInput);
                        return;
                    }
                }
            }
            nCount++;
        }
    }
#endif

#if defined(APT_USE_MOUSE)
    if(bCheckTop)
    {
        mpTopMostSprite = pCurrentTopSprite;         // (LEO) keep track of the highest sprite       F#563   7
    }
    ProcessMouseState(bPress, bPressRelease, input);
#if defined APT_USE_BUTTONS
    if(eController == AptInputController_Mouse)
    {
        ProcessButtonState();
    }
#endif
#endif
}

/*F************************************************************************************************/
/*!
    \Function    AptAnimationTarget::ProcessAptInput

    \Description
        Gets called for every input in the input queue and processes them

    This function will take each input passed in and shoot them to the listener sets that would react to any inputs

    \Input      input             - actual Apt Input
    \Input      bCheck            - passed on to ProcessInputSet

    \Output
        None.

    \Notes
        None.

    \Version    1.0        09/10/04 (LEO) First Version

*/
/************************************************************************************************F*/
void AptAnimationTarget::ProcessAptInput(AptInput input, bool bCheck)
{
    APT_PROFILE_INPUT(_processInput);
    APT_ASSERT(!INPUT_IS_KEY(&input) || !INPUT_IS_MOUSE(&input) || !INPUT_IS_ANALOG(&input) || !INPUT_IS_GESTURE(&input));

    AptInputType eType;
    AptInputState eState;
    AptInputController eController;
#if defined APT_USE_BUTTONS
    AptCIH *pNewButton = NULL;
#endif

#if defined(APT_USE_MOUSE)
    if(INPUT_IS_MOUSE(&input))
    {
        eType = AptInputType_MouseMovement;
        eState = AptInputState_MouseMove;
        eController = AptInputController_Mouse;
        GET_MOUSE_INPUT(input, snXMousePos, snYMousePos);
    }
    else
#endif
    {
        GET_INPUT(input, eType, eState, eController);
    }

    ProcessInputSet(eType, eState, input, eController, bCheck);
    ProcessListenerEvents(eType, eState, input, eController);
#if defined APT_USE_BUTTONS
    if (!HandleFocusButton(eType, eState, &pNewButton))
    {
        HandleAutoNav(pNewButton, eType, eState);
    }
#endif
}

#if defined APT_USE_BUTTONS
/*F************************************************************************************************/
/*!
    \Function    AptAnimationTarget::ValidateBIL

    \Description
        Validates the saButtonInstanceList in debug mode only

    \Input
        None.

    \Output     bool              -

    \Notes
        None.

    \Version    1.0        09/01/05 (LEO) First Version

*/
/************************************************************************************************F*/
bool AptAnimationTarget::ValidateBIL()
{
#if defined(EA_DEBUG)
    for (int32_t i = snBILCount-1;i>=0 ;i--)
    {
        APT_ASSERT(saButtonInstanceList[i].pCIH != NULL);

        if(saButtonInstanceList[i].pCIH->isUndefined())
        {
            continue;
            // this situation will happen when the animation is still being initializing
            // and a mouse event come in
        }
        APT_ASSERT(saButtonInstanceList[i].pCIH->IsButtonInst());
    }
#endif
    return true;
}
#endif

/*F************************************************************************************************/
/*!
    \Function    AptAnimationTarget::ProcessInputs

    \Description
        Main function to process inputs in Apt

    \Input
        None.

    \Output
        None.

    \Notes
        None.

    \Version    1.0        09/01/05 (LEO) First Version

*/
/************************************************************************************************F*/
void AptAnimationTarget::ProcessInputs()
{
    APT_PROFILE_BEGIN( APT_PROCESS_INPUT, "AptAnimationTarget::ProcessInputs" );

    APT_PROFILE(processInputs) ;
    int32_t i;

    //// (LEO) 06.03.04 FR217.  This is the StartDrag/StopDrag feature.  If the mpDragMC is set, then move it based on the mouse postion.
#if defined(APT_USE_MOUSE)
    if(mpDragMC->isUndefined() == false)
    {
        DragMovieClip();
    }
#endif
    for (i = 0; i < mnQueuedInputs; i++)
    {
        APT_PROFILE_INPUT(calling_processInputs) ;
        ProcessAptInput(maQueuedInputs[i], i == 0 ? true : false);
    }

    mnQueuedInputs = 0;

#if defined(APT_ALTERNATE_INPUT)
    ProcessAlternateEvents();
    mnQueuedAltInputs = 0;
#endif

    APT_PROFILE_END( APT_PROCESS_INPUT );
}



// Begin Private Button Functions
#if defined APT_USE_BUTTONS
/*F************************************************************************************************/
/*!
    \Function    AptAnimationTarget::ActionConditionFlagToActionEventFlags

    \Description
        Converts the condition to the proper button flag

    \Input      condition         -

    \Output     int32_t           -

    \Notes
        None.

    \Version    1.0        09/01/05 (LEO) First Version

*/
/************************************************************************************************F*/
int32_t AptAnimationTarget::ActionConditionFlagToActionEventFlags(int32_t condition)
{
    int flags = 0;
    if(condition & AptActionConditionFlag_OverDownToOverUp)
    {
        flags |= AptEventActionFlag_Release;
    }
    if(condition & AptActionConditionFlag_OverUpToOverDown)
    {
        flags |= AptEventActionFlag_Press;
    }
    if(condition & AptActionConditionFlag_OutDownToIdle)
    {
        flags |= AptEventActionFlag_ReleaseOutside;
    }
    if(condition & AptActionConditionFlag_IdleToOverUp)
    {
        flags |= AptEventActionFlag_RollOver;
    }
    if(condition & AptActionConditionFlag_OverUpToIdle)
    {
        flags |= AptEventActionFlag_RollOut;
    }
    if(condition & AptActionConditionFlag_OutDownToOverDown)
    {
        flags |= AptEventActionFlag_DragOver;
    }
    if(condition & AptActionConditionFlag_OverDownToOutDown)
    {
        flags |= AptEventActionFlag_DragOut;
    }
    return flags;
}

/*F************************************************************************************************/
/*!
    \Function    AptAnimationTarget::ProcessButtonState

    \Description
        Updates the current button state

    This function updates the current state of buttons in the flash movie

    \Output
        None.

    \Notes
        -This was moved to a separate function

    \Version    1.0        09/10/04 (LEO) First Version
    \Version    1.1        02/04/09 (RB)  Prevent extra onRollOver / onRollOut events from being fired.

*/
/************************************************************************************************F*/
void AptAnimationTarget::ProcessButtonState()
{
    AptCIH *pNewButton = GetButton(snXMousePos, snYMousePos);
    if(IsInputMasked(pNewButton))
    {
        pNewButton = 0;
    }

    APT_INCSAFE(pNewButton);

    // Move the focus to the new button
    if (mbButtonDown)
    {
        if (mpFocusButton && !mpFocusButton->isUndefined())
        {
            if (pNewButton != mpFocusButton)
            {
                if (mpFocusButton->GetButtonInst()->mnState == AptCharacterButtonRecordState_Down)
                {
                    mpFocusButton->gotoState(AptCharacterButtonRecordState_Over);
                    DoButtonActions(mpFocusButton, AptActionConditionFlag_OverDownToOutDown);
                }
            }
            else
            {
                if (mpFocusButton->GetButtonInst()->mnState == AptCharacterButtonRecordState_Over)
                {
                    mpFocusButton->gotoState(AptCharacterButtonRecordState_Down);
                    DoButtonActions(mpFocusButton, AptActionConditionFlag_OutDownToOverDown);
                }
            }
        }
        else
        {
            if (pNewButton && !pNewButton->isUndefined())
            {
                //              mpFocusButton->gotoState(AptCharacterButtonRecordState_Over);
                DoButtonActions(pNewButton, AptActionConditionFlag_OutDownToOverDown);
            }
        }
    }
    else if (pNewButton != mpFocusButton)
    {
        if (mpFocusButton && !mpFocusButton->isUndefined())
        {
            if ( mpFocusButton->GetButtonInst()->mnState != AptCharacterButtonRecordState_Up )
            {
                // Don�t set the state unless it�s different.
                // Prevents extra onRollOver / onRollOut events from being fired.
                mpFocusButton->gotoState(AptCharacterButtonRecordState_Up);
                DoButtonActions(mpFocusButton, AptActionConditionFlag_OverUpToIdle);
            }
        }

        APT_DECSAFE(mpFocusButton);
        mpFocusButton = pNewButton;
        APT_INCSAFE(mpFocusButton);

        if (pNewButton && !pNewButton->isUndefined())
        {
            mpFocusButton->gotoState(AptCharacterButtonRecordState_Over);
            DoButtonActions(pNewButton, AptActionConditionFlag_IdleToOverUp);
        }
    }

    APT_DECSAFE(pNewButton)
}

/*F************************************************************************************************/
/*!
    \Function    AptAnimationTarget::HandleFocusButton

    \Description
        This function handles the button logic for the currently focused button

    \Input      eType             -
    \Input      eState            -
    \Input      **ppNewButton     -

    \Output     bool              -

    \Notes
        None.

    \Version    1.0        09/01/05 (LEO) First Version

*/
/************************************************************************************************F*/
bool AptAnimationTarget::HandleFocusButton(AptInputType eType, AptInputState eState, AptCIH **ppNewButton)
{
    APT_ASSERT(ppNewButton);
    *ppNewButton = NULL;

    // now try and handle aptmovie overrides
    if (mpFocusButton == NULL)
    {
        return false;
    }
    if (eState != AptInputState_Pressed)
    {
        return false;
    }

    const AptNativeString * pStrText;

    switch (eType)
    {
    case    AptInputType_Up:
        pStrText = StringPool::GetString(SC__up);
        break;
    case    AptInputType_Down:
        pStrText = StringPool::GetString(SC__down);
        break;
    case    AptInputType_Left:
        pStrText = StringPool::GetString(SC__left);
        break;
    case    AptInputType_Right:
        pStrText = StringPool::GetString(SC__right);
        break;
    default:
        //  Key not handled
        return false;
    }

    AptCIH *pParent = mpFocusButton->GetDisplayListParent();
    AptValue *pValue = pParent->GetCharacterInst()->GetNativeHash()->Lookup(pStrText);
    if (pValue == NULL)
    {
        return false;
    }
    if (pValue->isString())
    {
        AptNativeString sVar;
        AptValue *pObject;

        gAptActionInterpreter.getContext(pParent, NULL, pValue->c_string()->GetInternalString(), &pObject, sVar);
        if (pObject)
        {
            AptCIH *pInst = pObject->c_cih();
            if (sVar.Size() == 0 && (pInst->IsButtonInst() || pInst->IsSpriteInstBase()))
            {
                *ppNewButton = pInst;
            }
            else if (sVar.Size() > 0 && pInst->IsSpriteInstBase())
            {
                AptValue *pValue2 = pInst->GetCharacterInst()->GetNativeHash()->Lookup(&sVar);

                // use function as atom
                pValue = pValue2 ? pValue2 : pValue;
            }
        }
    }
    if (pValue->isScriptFunction())
    {
        // DC -- 18.00 - Simplified Logic significantly.
        //void * pSavedValue   = gAptActionInterpreter.PrepareForExecution("_handleFocusButton");
        APT_DEFINE_ACTION_SETUP(pParent, pValue, pStrText->GetBuffer(), AptActionType_ButtonActions);
        void * pSavedValue   = gAptActionInterpreter.PrepareForExecution(&oActionSetup);

        gAptActionInterpreter.callFunction(pParent, pValue, 0);
        //gAptActionInterpreter.CleanupAfterExecution("_handleFocusButton", pSavedValue);
        gAptActionInterpreter.CleanupAfterExecution(pSavedValue, &oActionSetup);  //oActionSetup is defined by the macro

        AptValue *pReturnedValue = gAptActionInterpreter.stackAt(0);
        // if the return value is a pcharacter, try and use it as new focus
        if (pReturnedValue->isCIH())
        {
            APT_INC(pReturnedValue);
            gAptActionInterpreter.stackPop();
            pValue = pReturnedValue;
        }
        else
        {
            gAptActionInterpreter.stackPop();
            return true;
        }
    }
    else
    {
        APT_INC(pValue);
    }

    if (pValue->c_cih()->IsButtonInst() || pValue->c_cih()->IsSpriteInstBase())
    {
        if(!IsInputMasked(pValue->c_cih()))
            *ppNewButton = pValue->c_cih();
    }
    APT_DEC(pValue);
    return false;
}

/*F************************************************************************************************/
/*!
    \Function    AptAnimationTarget::HandleAutoNav

    \Description
        Insert function's purpose here


    \Input      *pNewButton       -
    \Input      eType             -
    \Input      eState            -

    \Output
        None.

    \Notes
        None.

    \Version    1.0        09/10/04 (LEO) First Version
    \Version    1.1        02/04/09 (RB)  Prevent extra onRollOver / onRollOut events from being fired.

*/
/************************************************************************************************F*/
void AptAnimationTarget::HandleAutoNav(AptCIH *pNewButton, AptInputType eType, AptInputState eState)
{
    // This is a set, so we need to iterate through the whole thing
    switch (eType)
    {
    case AptInputType_Up:
    case AptInputType_Down:
    case AptInputType_Left:
    case AptInputType_Right:
        // Only move if the padbutton isn't down
        // (note that normally, this isn't an issue in videogames because an option is selected on press, not release like a mouse)
        if (!mbButtonDown && eState == AptInputState_Pressed)
        {
            // make sure we have a valid focus button
            SetValidFocusButton();

            if (mpFocusButton == NULL)
                break;

            // if we have already found a new button just use that one
            if (pNewButton == NULL)
            {
                pNewButton = GetBestButton(eType, mpFocusButton->GetDisplayListParent(), mpFocusButton);        // GetBestButton defined in AptAnimationTargetButtonFunction.h
            }
            // if the new button we found is actually a sprite, find an appropriate button within the sprite
            else if (pNewButton->IsSpriteInstBase())
            {
                pNewButton = GetBestButton(eType, pNewButton, NULL);                                            // GetBestButton defined in AptAnimationTargetButtonFunction.h
            }

            // Move the focus to the new button
            if (pNewButton)
            {
                mpFocusButton->gotoState(AptCharacterButtonRecordState_Up);
                pNewButton->gotoState(AptCharacterButtonRecordState_Over);
                // Run event action code
                DoButtonActions(mpFocusButton, AptActionConditionFlag_OverUpToIdle);
                DoButtonActions(pNewButton, AptActionConditionFlag_IdleToOverUp);

                APT_DECSAFE(mpFocusButton);
                mpFocusButton = pNewButton;
                APT_INCSAFE(mpFocusButton);
            }
        }
        break;
#if defined(APT_USE_MOUSE)
        // Press/release - change state, do actions and stuff
    case AptInputType_MouseButton0:
        if (mpFocusButton == NULL)
        {
            if (eState == AptInputState_Released)
            {
                mbButtonDown = false;
                if (mpFocusButton)
                {
                    mpFocusButton->gotoState(AptCharacterButtonRecordState_Over);
                    DoButtonActions(mpFocusButton, AptActionConditionFlag_IdleToOverUp);
                }
            }
            else
            {
                mbButtonDown = true;
            }
            break;
        }
        if (!mbButtonDown && eState == AptInputState_Pressed)
        {
            mbButtonDown = true;
            mpFocusButton->gotoState(AptCharacterButtonRecordState_Down);
            DoButtonActions(mpFocusButton, AptActionConditionFlag_OverUpToOverDown);
        }
        if (mbButtonDown && eState == AptInputState_Released)
        {
            mbButtonDown = false;
            if (mpFocusButton->GetButtonInst()->mnState == AptCharacterButtonRecordState_Over)
            {
                mpFocusButton->gotoState(AptCharacterButtonRecordState_Up);
                DoButtonActions(mpFocusButton, AptActionConditionFlag_OutDownToIdle);

                if(mpDragMC == mpFocusButton->GetDisplayListParent())
                {
                    mpFocusButton->gotoState(AptCharacterButtonRecordState_Over);
                    DoButtonActions(mpFocusButton, AptActionConditionFlag_OverDownToOverUp);
                }
            }
            else
            {
                mpFocusButton->gotoState(AptCharacterButtonRecordState_Over);
                DoButtonActions(mpFocusButton, AptActionConditionFlag_OverDownToOverUp);
            }
        }
        break;
#endif
    default:
        break;
    }
}

/*F************************************************************************************************/
/*!
    \Function    AptAnimationTarget::DoButtonActions

    \Description
        Handles the actions for a button transition

    \Input      *pInst            -
    \Input      nTransition       -

    \Output
        None.

    \Notes
        None.

    \Version    1.0        09/01/05 (LEO) First Version
    \Version    1.1        01/22/09 (RB)  EATech 115181: Reverse order of button events
*/
/************************************************************************************************F*/
void AptAnimationTarget::DoButtonActions(AptCIH *pInst, int nTransition)
{
    APT_ASSERT(pInst);
    APT_ASSERT(pInst->IsButtonInst());

    const AptCharacter *pChar = pInst->GetCharacterInst()->GetCharacterConst();

    int32_t i;
    for (i = 0; i < pChar->button.nActionConditions; i++)
    {
        if (pChar->button.aActionConditions[i].nConditions & nTransition)
        {
            APT_ASSERT(pInst->GetDisplayListParent());
#if APT_PROF_ENABLED && APT_PROFILE_ACTIONSCRIPT_ENABLED
            int32_t iConditions = APT_ARRAYSIZE(_aConditionNames);
            AptNativeString strEvent;
            while (iConditions--)
            {
                if (_aConditionNames[iConditions].nFlag == nTransition)
                {
                    break;
                }
            }
            APT_ASSERT(iConditions >= 0);
#endif
            AddActionBack(&pChar->button.aActionConditions[i].actions, pInst->GetDisplayListParent(),
                ACTION_PROF_CALL_PARAM(_aConditionNames[iConditions].strName)
                ACTION_TYPE_CALL_PARAM(AptActionType_ButtonActions)
                gNullInput
                );
        }
    }
    int32_t nEventFlag = ActionConditionFlagToActionEventFlags(nTransition);
    if(pInst->GetNativeHashVirtual()->HasEventHandler(nEventFlag))
    {
        for (i = 0; i < APT_ARRAYSIZE(_aInputFlags); i++)
        {
            if (nEventFlag & _aInputFlags[i].nFlag)
            {
                // (LEO) buttons now have a hash
                AptValue *pValue = pInst->GetNativeHash()->Lookup(StringPool::GetString(_aInputFlags[i].eName));
                if (pValue)
                {
                    AddFunctionBack(pInst, pValue, 0,
                        ACTION_PROF_CALL_PARAM(StringPool::GetString(_aInputFlags[i].eName)->GetBuffer())
                        ACTION_TYPE_CALL_PARAM(_aClipEvents[i].nFlag)
                        SET_KEYBOARD_INPUT(AptInputType_MouseButton0, _aInputFlags[i].nInputState, AptInputController_Mouse));
                }
            }
        }

    }

#if defined(APT_USE_SOUND_OBJECT)
    if (pChar->button.pButtonSound)
    {
        switch (nTransition)
        {
        case AptActionConditionFlag_OverUpToIdle:
            if (pChar->button.pButtonSound->pOverUpToIdle)
            {
                APT_PROFILE_AUXILIARY(pfnStartSound);
                gAptFuncs.pfnStartSound(pChar->button.pButtonSound->pOverUpToIdle->sound.zID, NULL);
                //gAptFuncs.pfnStartSound(pChar->button.pButtonSound->pOverUpToIdle->sound.zID);
            }
            break;
        case AptActionConditionFlag_IdleToOverUp:
            if (pChar->button.pButtonSound->pIdleToOverUp)
            {
                APT_PROFILE_AUXILIARY(pfnStartSound);
                gAptFuncs.pfnStartSound(pChar->button.pButtonSound->pIdleToOverUp->sound.zID, NULL);
                //gAptFuncs.pfnStartSound(pChar->button.pButtonSound->pIdleToOverUp->sound.zID);
            }
            break;
        case AptActionConditionFlag_OverUpToOverDown:
            if (pChar->button.pButtonSound->pOverUpToOverDown)
            {
                APT_PROFILE_AUXILIARY(pfnStartSound);
                gAptFuncs.pfnStartSound(pChar->button.pButtonSound->pOverUpToOverDown->sound.zID, NULL);
                //gAptFuncs.pfnStartSound(pChar->button.pButtonSound->pOverUpToOverDown->sound.zID);
            }
            break;
        case AptActionConditionFlag_OverDownToOverUp:
            if (pChar->button.pButtonSound->pOverDownToOverUp)
            {
                APT_PROFILE_AUXILIARY(pfnStartSound);
                gAptFuncs.pfnStartSound(pChar->button.pButtonSound->pOverDownToOverUp->sound.zID, NULL);
                //gAptFuncs.pfnStartSound(pChar->button.pButtonSound->pOverDownToOverUp->sound.zID);
            }
            break;
        }
    }
#endif
    // removing this runactions from here - 1.01.00 ASK
    // because of new CleanUpRemList it might create problem if the run actions deletes
    // any movieclip which is accessed from any of these functions.
//    RunActions();
}

/*F************************************************************************************************/
/*!
    \Function    AptAnimationTarget::IsPointInButtonHitTestRegion

    \Description
        Does a point hit test for buttons

    \Input      button            -
    \Input      *pMatrix          -
    \Input      nX                -
    \Input      nY                -

    \Output     bool              -

    \Notes
        None.

    \Version    1.0        09/01/05 (LEO) First Version

*/
/************************************************************************************************F*/
bool AptAnimationTarget::IsPointInButtonHitTestRegion(const AptCharacterButton * button,AptMatrix *pMatrix,int32_t nX, int32_t nY)
{
    int32_t nTriangle;
    float fX = (float)nX;
    float fY = (float)nY;

    // reject by bounding rect first
    float fRect[4];
    float fRectTransformed[4];
    fRect[0] = button->mHitTestBoundingRect.fLeft;
    fRect[1] = button->mHitTestBoundingRect.fTop;
    fRect[2] = button->mHitTestBoundingRect.fRight;
    fRect[3] = button->mHitTestBoundingRect.fBottom;

    // TODO(LJS)
    // ideally we should build an inverse matrix of pMatrix and transform the x/y into local space
    // instead of transform boundbox and all the vertex into world space
    // Or take Paul's suggestion, avoid triangle totally

    // transform the bounding rect into worldspace
    MatrixVecMult(&fRect[0], pMatrix, &fRectTransformed[0]);
    MatrixVecMult(&fRect[2], pMatrix, &fRectTransformed[2]);

    if (fRectTransformed[0] > fRectTransformed[2])
    {
        float t = fRectTransformed[2];
        fRectTransformed[2] = fRectTransformed[0];
        fRectTransformed[0] = t;
    }
    if (fRectTransformed[1] > fRectTransformed[3])
    {
        float t = fRectTransformed[3];
        fRectTransformed[3] = fRectTransformed[1];
        fRectTransformed[1] = t;
    }

    // do a fast reject
    if(fX < fRectTransformed[0]||
        fX > fRectTransformed[2] ||
        fY < fRectTransformed[1] ||
        fY > fRectTransformed[3]
        )
        {
            // return false;    //### the fast reject will break on certain cases, do nothing for now...fixed in 0.7.11
        }

        // for each triangle
        for (nTriangle=0; nTriangle<button->mHitTestTriangles; nTriangle++)
        {
            float *p = button->mHitTestVertexTable;
            short* idx = button->mHitTestIndexTable + nTriangle * 3;

            // transform teh 3 vertex into world space
            float v[6];
            MatrixVecMult(&p[*idx * 2], pMatrix, &v[0]);
            MatrixVecMult(&p[*(idx+1) * 2], pMatrix, &v[2]);
            MatrixVecMult(&p[*(idx+2) * 2], pMatrix, &v[4]);

            // test against x/y
            if (PointInTri(v, fX, fY))
            {
                return true;
            }
        }
        return false;
}

/*F************************************************************************************************/
/*!
    \Function    *AptAnimationTarget::GetButton

    \Description
        Returns the button under the passed IN x y values

    \Input      nX                - X coordinate of mouse cursor
    \Input      nY                - Y coordinate of mouse cursor

    \Output     AptCIH  - Button that is under the point (nX, nY) NULL if none

    \Notes
        None.

    \Version    1.0        09/01/05 (LEO) First Version
    \Version    1.1        01/22/09 (RB)  EATech 115120: Adding support for masked buttons.
                                          Refactored saButtonInstanceList[i].pCIH to cih.
                                          Refactored aButtonInstanceList[i].matrix to instanceMatrix
*/
/************************************************************************************************F*/
AptCIH *AptAnimationTarget::GetButton(int nX, int nY)
{
#if defined APT_USE_MOUSE
    // should test in the reverse order to the drawing order
    for (int32_t i = snBILCount-1; i>=0 ; i--)
    {
        AptCIH *cih = saButtonInstanceList[i].pCIH;
        APT_ASSERT(cih != NULL);
        AptMatrix *instanceMatrix = &saButtonInstanceList[i].matrix;

        if(cih->isUndefined())
        {
            continue;
            // this situation will happen when the animation is still being initializing
            // and a mouse event come in
        }
        APT_ASSERT(cih->IsButtonInst());

        const AptCharacterButton *pButtonChar = &cih->GetCharacterInst()->GetCharacterConst()->button;
        APT_ASSERT(cih->GetCharacterInst()->GetCharacterConst()->eType == AptCharacterType_Button);

        bool hitsButton = false;

        for (int32_t nRecord=0; nRecord < pButtonChar->nButtonRecords && !hitsButton; nRecord++)
        {
            AptCharacterButtonRecord &record = pButtonChar->aButtonRecords[nRecord];
            if (!(record.eStates & AptCharacterButtonRecordState_HitTest))
            {
                continue;
            }
            AptMatrix mat = record.matrix;
            AptRenderingContext::multMatrix(instanceMatrix, &mat, &mat);

            AptCharacter *pChar = record.pCharacter;
            switch(pChar->eType)
            {
                case AptCharacterType_Shape:
                    hitsButton = IsPointInButtonHitTestRegion(pButtonChar, &mat, nX, nY);
                    break;
                case AptCharacterType_Text:
                    hitsButton = IsPointInRect(pChar->text.rBounds, mat, nX, nY);
                    break;
                default:
                    APT_DEBUGPRINT(APT_DEBUG_MSG_DEBUG_LVL, "not hit testing: unhandled type\n");
                    break;
            }
        }
        // We know that the mouse cursor hits the button. Now we need to know if it hits also the mask if any
        if(hitsButton)
        {
            // Get top-level assigned mask, if any
            AptCIH *parent  = cih;
            AptCIH *mask = NULL;
            while(parent && mask == NULL)
            {
                mask = parent->GetMask();
                parent = (AptCIH *)parent->GetDisplayListParent();
            }

            if(mask)
            {
                // hit-test against mask
                hitsButton =  IsPointInCIH(mask,  nX, nY);
            }

            // hit-test any other masks in mask layer
            for(AptCIH *prev = cih->GetDisplayListPrevious(); hitsButton && prev; prev = prev->GetDisplayListPrevious())
            {
                if(prev->GetCharacterInst()->GetRenderItem()->GetClipDepth() >= 0)  // It's a clipper
                {
                    hitsButton =  IsPointInCIH(prev, nX, nY);
                }
            }
            if(hitsButton)
            {
                return cih;
            }
        }
    } // for i
#endif
    return NULL;
}
#endif
// End Private Button Functions

// Begin Private Mouse Functions
#if defined APT_USE_MOUSE
/*F************************************************************************************************/
/*!
    \Function    AptAnimationTarget::ProcessMouseState

    \Description
        Updates the currently mouse interacted sprite

    This function will update the currently mouse interacted sprite by sending it the correct event based
    on the current mouse position and button press.

    \Input      bPress            -
    \Input      bPressRelease     -
    \Input      input             -

    \Output
        None.

    \Notes
        None.

    \Version    1.0        09/10/04 (LEO) First Version

*/
/************************************************************************************************F*/
void AptAnimationTarget::ProcessMouseState(bool bPress, bool bPressRelease, AptInput input)
{

    if(bPressRelease)
    {
        // (LEO) We have a onRelease event, so we need to see if the it's a release outside, or simply just a release event.
        // This sprite was previously pressed, set the release event to either release, or releaseOutside
        if(!mpOnPress->isUndefined())
        {
            if(!mpDragMC->isUndefined())
            {
                if(mpDragMC == mpOnPress)
                    mpOnPress->c_cih()->queueClipEvents(AptEventActionFlag_Release, input);
                else
                    mpOnPress->c_cih()->queueClipEvents(AptEventActionFlag_ReleaseOutside, input);
            }
            else if(PointHits(mpOnPress->c_cih(), snXMousePos, snYMousePos) && mpTopMostSprite == mpOnPress)
            {
                mpOnPress->c_cih()->queueClipEvents(AptEventActionFlag_Release, input);
            }
            else
            {
                mpOnPress->c_cih()->queueClipEvents(AptEventActionFlag_ReleaseOutside, input);
            }
            mpOnPress = gpUndefinedValue;
        }
    }
    else if (bPress)
    {
        // (LEO) We have a onPress event, send the event to the top most sprite
        if(mpTopMostSprite->isUndefined() == false)
        {
            // We now have to send a onPress event for this sprite
            mpTopMostSprite->c_cih()->queueClipEvents(AptEventActionFlag_Press, input);
            mpOnPress = mpTopMostSprite;
        }
    }
    else if(!mpOnPress->isUndefined())
    {
        // (LEO) If the current mpOnPress sprite is defined, check it for a drag over or out event    F#563   7
        bool bHitTest = PointHits(mpOnPress->c_cih(), snXMousePos, snYMousePos);
        if(!mpOnRollOver->isUndefined() && !bHitTest)
        {
            mpOnPress->c_cih()->queueClipEvents(AptEventActionFlag_DragOut, input);
            mpOnRollOver = gpUndefinedValue;
        }
        else if(mpOnRollOver->isUndefined() && bHitTest)
        {
            mpOnPress->c_cih()->queueClipEvents(AptEventActionFlag_DragOver, input);
            mpOnRollOver = mpOnPress;
        }
    }
    else
    {
        if(!mpTopMostSprite->isUndefined() && mpTopMostSprite != mpOnRollOver)
        {
            if(!mpOnRollOver->isUndefined() &&  mpOnRollOver != mpOnPress)
            {
                // The last rolledOn sprite has a sprite on top; send it the rollOut event before sending the new rolledOn sprite its event
                mpOnRollOver->c_cih()->queueClipEvents(AptEventActionFlag_RollOut, input);
            }
            mpOnRollOver = mpTopMostSprite;
            mpOnRollOver->c_cih()->queueClipEvents(AptEventActionFlag_RollOver, input);
        }
        else if(!mpOnRollOver->isUndefined() && mpTopMostSprite != mpOnRollOver &&  mpOnPress->isUndefined() &&
                !PointHits(mpOnRollOver->c_cih(), snXMousePos, snYMousePos))
        {
            mpOnRollOver->c_cih()->queueClipEvents(AptEventActionFlag_RollOut, input);
            mpOnRollOver = gpUndefinedValue;
        }
    }
}
#endif
// End Private Mouse Functions

/*** Public Functions *****************************************************************************/
/*F************************************************************************************************/
/*!
    \Function    AptAnimationTarget::AptAnimationTarget

    \Description
        Constructor

    this is new constructor written that initializes all the pools at runtime
    instead of setting them at compile time.
    This gives the user, ability to change the pool sizes at runtime, before initializing Apt.

    \Input      &aptInitParms     -

    \Output

    \Notes
        None.

    \Version    1.0         (ASK) First Version
        (ASK) changed to use pfnMemAlloc in cases where pool of pointers
        is created. and Added NEW_DELETE to AptActionPool, ButtonHitTestRecord
        so new can be use directly. This is basically done so that all allocations
        will go thru pfnMemAlloc.
    \Version   1.1           (ASK) added mAltListenerSet, maQueuedAltInputs for alternate input
                                 handling
    \Version   1.2    11/21/07 (JP) Initialing the drag pos matrix

*/
/************************************************************************************************F*/
AptAnimationTarget::AptAnimationTarget(const AptTargetInitParams *aptInitParms)
    :
        miMaxIntervalTimers(aptInitParms->iMaxIntervalFunctions),
        miMaxQueuedInputs(aptInitParms->iMaxQueuedInputs),
        mListenerSet(aptInitParms->iListenerSetSize, AptHighwaterMemoryMetricsImpl::GetMetrics(AptHighwaterMemoryMetrics::TYPE_LISTENERSET) ),
        mInputSet(aptInitParms->iInputSetSize, AptHighwaterMemoryMetricsImpl::GetMetrics(AptHighwaterMemoryMetrics::TYPE_INPUTSET)),
        mDisplayList()
        #if defined APT_USE_BUTTONS
        ,mButtonSet(aptInitParms->iButtonSetSize, AptHighwaterMemoryMetricsImpl::GetMetrics(AptHighwaterMemoryMetrics::TYPE_BUTTONSET) )
        #endif
        #if defined(APT_USE_MOUSE)
        ,mMouseListenerSet(aptInitParms->iListenerSetSize, AptHighwaterMemoryMetricsImpl::GetMetrics(AptHighwaterMemoryMetrics::TYPE_LISTENERSET) )
        #endif
        #if defined(APT_ALTERNATE_INPUT)
        ,mAltListenerSet(aptInitParms->iListenerSetSize, AptHighwaterMemoryMetricsImpl::GetMetrics(AptHighwaterMemoryMetrics::TYPE_LISTENERSET))
        #endif
{
    APT_ASSERT(gAptFuncs.pfnMemAlloc);

    mDragPos.a = mDragPos.b =  mDragPos.c =  mDragPos.d =  mDragPos.tx =  mDragPos.ty = 0;

    mpAptActionPool = new AptActionQueueC(aptInitParms->iActionPoolSize, AptHighwaterMemoryMetricsImpl::GetMetrics(AptHighwaterMemoryMetrics::TYPE_ACTIONPOOL));

    maIntervalTimers = new AptIntervalTimer[miMaxIntervalTimers];
    APT_ASSERT(maIntervalTimers != 0);

    APT_ASSERT(miMaxQueuedInputs != 0);
    maQueuedInputs = APT_MALLOC_ARRAY(AptInput, miMaxQueuedInputs);
    APT_ASSERT(maQueuedInputs != NULL);

    mnQueuedInputs = 0;

    mpInputMask = NULL;
    mpDragMC = gpUndefinedValue;
    mpOnPress = gpUndefinedValue;
    mpOnRollOver = gpUndefinedValue;
    mpTopMostSprite = gpUndefinedValue;

#if defined APT_USE_BUTTONS
    mpFocusButton = NULL;
    mbButtonDown = false;
    snBILCount = 0;
#endif

#if defined(APT_ALTERNATE_INPUT)
    maQueuedAltInputs = new AptAltInput[miMaxQueuedInputs] ;
    mnQueuedAltInputs = 0;
#endif
}

void AptAnimationTarget::SetupStaticData(const AptInitParmsT &aptInitParms)
{
#if defined APT_USE_BUTTONS
    siButtonInstanceListSize = aptInitParms.iMaxButtonInstances;
    if(siButtonInstanceListSize == 0)
        saButtonInstanceList = NULL;
    else
        saButtonInstanceList = new ButtonHitTestRecord[siButtonInstanceListSize];
#endif

    siMaxNewMovieClips = aptInitParms.iMaxNewMovieClipsPerFrame;
    APT_ASSERT(siMaxNewMovieClips != 0);

	// Set initial vector sizes to the assumed max capacity.
	sapNewInsts.set_capacity(aptInitParms.iMaxNewMovieClipsPerFrame);
	sapNewInsts.resize(sapNewInsts.capacity());
	
	sapDelayedReleaseList.set_capacity(aptInitParms.iMaxNewMovieClipsPerFrame);
	sapDelayedReleaseList.resize(sapDelayedReleaseList.capacity());

    snNewInsts = 0;
    snDelayedReleaseCount = 0;
}


void AptAnimationTarget::CleanupStaticData()
{
	sapNewInsts.set_capacity(0);
	sapDelayedReleaseList.set_capacity(0);


    #if defined APT_USE_BUTTONS
    if(saButtonInstanceList != NULL)
        delete [] saButtonInstanceList;
    #endif

}
/*F************************************************************************************************/
/*!
    \Function    AptAnimationTarget::AddToRemList

    \Description
        Adds an item into the apDelayedReleaseList list

    This function adds an item into the apDelayedReleaseList list, which is the list that
    keeps track of the delayed deleted AptCIH's.

    \Input      pItem             - AptCIH to be added to apDelayedReleaseList

    \Output
        None.

    \Notes
        None.

    \Version    1.0        04/07/05 (LEO) First Version
    \Version    1.1        05/19/05 (DC)  Now uses the InRemList functions in AptCIH.
    \Version    1.2        07/10/09 (RB)  Don't loop thru sapDelayedReleaseList
*/
/************************************************************************************************F*/
void AptAnimationTarget::AddToRemList(AptCIH *pItem)
{
    if(pItem->GetInRemList())   // if it is already in the rem list, return.
        return;

	// Grow the delayed release list if we're at capacity
	if (sapDelayedReleaseList.capacity() == snDelayedReleaseCount)
	{
		sapDelayedReleaseList.set_capacity(sapDelayedReleaseList.capacity() + siNewMovieClipsGrowSize);
		sapDelayedReleaseList.resize(sapDelayedReleaseList.capacity());
	}

    int32_t i = snDelayedReleaseCount;
    APT_ASSERT(sapDelayedReleaseList[i] == NULL);

    APT_INC(pItem);
    pItem->SetInRemList(true);
    sapDelayedReleaseList[i] = pItem;

    if(i == snDelayedReleaseCount)
        snDelayedReleaseCount++;

	if (snDelayedReleaseCount > snDelayedReleaseCountHW)
	{
		snDelayedReleaseCountHW = snDelayedReleaseCount;
	}
}

/*F************************************************************************************************/
/*!
    \Function    AptAnimationTarget::CleanRemList

    \Description
        This function cleans out the apDelayedReleaseList

    \Input
        None.

    \Output
        None.

    \Notes
        None.

    \Version    1.0        04/05/05 (LEO) First Version
    \Version    1.1        05/19/05 (DC)  Now uses the InRemList functions in AptCIH.
    \Version    1.2        05/19/07 (ASK) Added optimization to get all GC objects in a
                           linear array to avoid millions of calls to GetNextAptValue() for
                           every ReplaceReferences() call.
    \Version    2.0        12/05/07 (ASK) Replaced call to ReplaceReferences by RemoveReferences which goes
                           through all the GC objects only ones and bulk-replace all the AptCIH in
                           sapDelayedReleaseList which are GC-marked for identification.
                           This gives tremendous performance improvement for the operation of
                           AptAnimationTarget::CleanRemList() especially when screens are
                           shutting down and lot of AptCIHs are getting deleted but do not
                           get deleted thru natural reference counting mechanism and still
                           all references to it need to be replaced everywhere.
                           This optimization was added with help of Toan Pham from NCAA09.
    \Version    2.1        07/08/09 (RB)  Don't call ForceDelete when refcount != 1 in case object is referred to by
                           something in gpValuesToRelease. Prevents a crash, but may cause a rare, slow
                           leak until AptShutdown.
*/
/************************************************************************************************F*/
//#include <xtl.h>
//#include <ppcintrinsics.h>

// Added reference function RemoveReferences that gets called from AptAnimationTarget::CleanRemList
extern void RemoveReferences(AptValue ** pAllocatedValues, int32_t nItemsInArray);

void AptAnimationTarget::CleanRemList()
{
    //uint64_t startTime3 = 0;
    //uint64_t startTime4 = 0;
    int32_t nActualItemsToBeReplaced = 0;
    if (snDelayedReleaseCount > 0)
    {
        //QueryPerformanceCounter((union _LARGE_INTEGER*)&startTime3);
    }

    for(int32_t i = 0; i < snDelayedReleaseCount; i++)
    {
        AptCIH * pObj = sapDelayedReleaseList[i];
        APT_ASSERT(pObj->GetInRemList());
        sapDelayedReleaseList[i] = NULL;

        if(pObj != NULL)
        {
            // we will use the GCMark to mark that an item needs to be deleted
            APT_ASSERT(pObj->getGCMark() == false);

            if(!pObj->isCIH(true))
                continue;
            if(pObj->c_cih(true)->GetCIHState() == AptCIH::AptCIHState_Zombie ||  pObj->isNone())
            {
                continue;
            }
            if(pObj->getRefCount() > 1)
            {
                pObj->PreDestroy();
                pObj->DestroyGCPointers();

                if(!pObj->isCIH(true))
                {
                    continue;
                }
                if(pObj->getRefCount() == 1)
                {
                    APT_DEC(pObj);          // All done, it can go away now.
                }
                else
                {
                    sapDelayedReleaseList[nActualItemsToBeReplaced++] = pObj;
                    pObj->setGCMark(true);      // mark object so that we can delete all of these with one pass through the AptValues
                }
            }
            else if (pObj->getRefCount() == 1)
            {
                APT_DEC(pObj);
            }
            else
            {
                APT_ASSERT(0 && "Reference counting issue encountered");
            }
        }
    }

    // update size of the sapDelayedReleaseList
    snDelayedReleaseCount = nActualItemsToBeReplaced;

    AptValue ** pAllocatedValues = NULL;
    int32_t nPrintCycles = 0;
    if (nActualItemsToBeReplaced > 0)
    {
        pAllocatedValues = gpGCPoolManager->GetAllAllocatedAptValues();
        int32_t nItemsInArray = (int32_t) gpGCPoolManager->mnItemsAllocated ;

        // delete references
        RemoveReferences(pAllocatedValues, nItemsInArray);

        // clean up
        for (int32_t j=0; j < nActualItemsToBeReplaced; j++)
        {
            AptCIH * pObj = sapDelayedReleaseList[j];
            sapDelayedReleaseList[j] = NULL;

            if (pObj)
            {
                pObj->setGCMark(false);         // unmark the object

                // Note, we're not calling pObj->ForceDelete() when the refcount != 1, because the
                // object might be referenced by something in gpValuesToRelease. In that case (which
                // should be very rare in practice) it would eventually be cleaned up during Apt shutdown.
                APT_DEC(pObj);

                nPrintCycles++;
            }
        }

        gpGCPoolManager->ReleaseAllocatedAptValuesArray(pAllocatedValues, nItemsInArray);
    }

    if (snDelayedReleaseCount > 0 && nPrintCycles > 0 )
    {
        //QueryPerformanceCounter((union _LARGE_INTEGER*)&startTime4);
        //printf("CleanRemList took - %I64u, %d \n", startTime4 - startTime3, nPrintCycles);
    }


    snDelayedReleaseCount = 0;
}



/*F************************************************************************************************/
/*!
    \Function    AptAnimationTarget::~AptAnimationTarget

    \Description
        Destructor

    new destructor now only has to delete the pools and PreDestroy will do the work of
    clearing contents of all the pools.

    \Input
        None.

    \Output

    \Notes
        None.

    \Version    1.0        (ASK) First Version
                    4/22/04 (ASK) changed to use pfnMemFreeSize for pools of pointers and also made sure
                    that the other deletes will go through pfnFreeSize
    \Version   1.1    12/6/05  (ASK) added maQueuedAltInputs for alternate input handling
*/
/************************************************************************************************F*/
AptAnimationTarget::~AptAnimationTarget()
{
    // free the new inst memory
    APT_FREE_ARRAY(maQueuedInputs, AptInput, miMaxQueuedInputs) ;
    delete [] maIntervalTimers;
    delete mpAptActionPool;

#if defined(APT_ALTERNATE_INPUT)
    delete [] maQueuedAltInputs ;
    maQueuedAltInputs = NULL;
#endif

    mpInputMask = NULL;
    maQueuedInputs = NULL;
    mpOnPress = NULL;
    mpOnRollOver = NULL;
    mpTopMostSprite = NULL;
    mpDragMC = NULL;
}

/*F************************************************************************************************/
/*!
    \Function    AptAnimationTarget::PreDestroy

    \Description
        PreDestroy function thats first cleans all the pools

    \Input
    None.

    \Output

    \Notes
    None.

    \Version    1.0        First Version
                    This is basically doing same work as old destructor used to do.
                    But now as pool sizes are set at runtime, and pools are allocated at runtime and not at compile time
                    there has to be this predestroy function that will clear the contents of the pools.
                    Destructor will only remove those pools from memory.
    \Version   1.1    12/6/05  (ASK) added mAltListenerSet code for alternate input handling
*/
/************************************************************************************************F*/
void AptAnimationTarget::PreDestroy(void)
{
#if defined APT_USE_BUTTONS
    int32_t nCount = mButtonSet.mnElements;

    // XXX TODO this loop can be removed because it should be done in the dtor?
    for (int32_t i = 0; i < mButtonSet.capacity(); i++)
    {
        if (mButtonSet.maElements[i])
        {
            mButtonSet.remove(mButtonSet.maElements[i]);
            nCount--;
            if (nCount == 0) break;
        }
    }
#endif

    // clear the interval timers
    for (int32_t i = 0; i < GetMaxIntervalTimers(); i++)
    {
        if (maIntervalTimers[i].bValid == false)
        {
            continue;
        }
        APT_DEC(maIntervalTimers[i].pCBFunction);
        APT_DECSAFE(maIntervalTimers[i].pContext);
        int32_t j;
        for(j = 0 ; j < maIntervalTimers[i].pParams.size() ; ++j)
        {
            maIntervalTimers[i].CleanParams();
        }
        maIntervalTimers[i].bValid = false;
    }

    // clear  listener events.
    // instead of calling remove on those elements just doing same functionality here.
    for (int32_t i = 0; i < mListenerSet.capacity(); i++)
    {
        if (mListenerSet.maElements[i] == NULL)
            continue;
        APT_DECSAFE(mListenerSet.maElements[i]);
        mListenerSet.maElements[i] = 0;
    }

#if defined(APT_USE_MOUSE)
    // do not Skip this code if the mouse flag is false
    // as we are adding mouselisteners to this set irresepective of bUseMouse.
    for (int32_t i = 0; i < mMouseListenerSet.capacity(); i++)
    {
        if (mMouseListenerSet.maElements[i] == NULL)
            continue;
        APT_DECSAFE(mMouseListenerSet.maElements[i]);
        mMouseListenerSet.maElements[i] = 0;
    }
#endif

#if defined(APT_ALTERNATE_INPUT)
    for (int32_t i = 0; i < mAltListenerSet.capacity(); i++)
    {
        if (mAltListenerSet.maElements[i] == NULL)
            continue;
        APT_DECSAFE(mAltListenerSet.maElements[i]);
        mAltListenerSet.maElements[i] = 0;
    }
#endif

#if defined APT_USE_BUTTONS
    ClearBIL();
#endif
    mDisplayList.PreDestroy() ;
}

/*F************************************************************************************************/
/*!
    \Function    AptAnimationTarget::Reset

    \Description
    This function will reset all the members variables of AnimationPoolData

    \Input
    None.

    \Output
    None.

    \Notes
    This was added to be called from AptReset. AptReset first calls AnimationPoolData::PreDestroy
    and then it calls this function to reset remaining values.

    \Version    1.0        02/23/05 (ASK) First Version
    \Version    1.1        12/6/05  (ASK) added mnQueuedAltInputs code for alternate input handling
*/
/************************************************************************************************F*/
void AptAnimationTarget::Reset()
{
    mnQueuedInputs = 0;
#if defined APT_USE_BUTTONS
    mpFocusButton = NULL;
    mbButtonDown = false;
    snBILCount = 0;
#endif
    snXMousePos = 0;
    snYMousePos = 0;

    mpInputMask = NULL;
    mpDragMC = gpUndefinedValue;
    mpOnPress = gpUndefinedValue;
    mpOnRollOver = gpUndefinedValue;
    mpTopMostSprite = gpUndefinedValue;

#if defined(APT_ALTERNATE_INPUT)
    mnQueuedAltInputs = 0;
#endif
}

/*F************************************************************************************************/
/*!
\Function    AptAnimationTarget::tickIntervalTimers

\Description
Decrements counters on all timer functions runs the Actionscript for the ones that are
ready to go.

\Input      nMilliseconds     - time since last call.

\Output
None.

\Notes
None.

\Version    1.0        09/15/04 (DC) Added Comment. Also made Script Function Type Agnostic.
\Version    1.1        11/21/07 (JP) Checking that pFuncValue and pContext are not NULL

*/
/************************************************************************************************F*/
void AptAnimationTarget::TickIntervalTimers(int32_t nMilliseconds)
{
    APT_PROFILE_BEGIN( APT_TICK_INTERVAL_TIMERS, "Apt::TickIntervalTimers" );

    APT_PROFILE(AptAnimationTarget_tickIntervalTimers);
    for (int32_t i = 0; i < GetMaxIntervalTimers(); ++i)
    {
        if (maIntervalTimers[i].bValid == false)
        {
            continue;
        }

        maIntervalTimers[i].fCurTime -= nMilliseconds;

        if (maIntervalTimers[i].fCurTime < 0)
        {
            AptValue * pFuncValue = maIntervalTimers[i].pCBFunction;
            AptCIH   * pScriptCIH = (pFuncValue && pFuncValue->isScriptFunction()) ? pFuncValue->c_scriptfunction()->mpCIH : NULL;

            if (pFuncValue && (pFuncValue->isNativeFunction() || pFuncValue->isExternalFunction() || (pScriptCIH && !pScriptCIH->isUndefined() && !pScriptCIH->IsLevelInst())))
            {
                // DC 18.00 - Changed to use AptScriptFunctionBase (Reduces duplication of code).
                AptValue * pContext  = maIntervalTimers[i].pContext != gpUndefinedValue ? maIntervalTimers[i].pContext : pScriptCIH;
                int32_t nParams          = maIntervalTimers[i].pParams.size();

                APT_INCSAFE(pContext);

                //void * pSavedValue   = gAptActionInterpreter.PrepareForExecution("tickIntervalTimers");
                APT_DEFINE_ACTION_SETUP(pContext, pFuncValue, "AptTimerFunc", AptActionType_Timer);

                #if defined (EA_DEBUG)
                    APT_PROFILE_BEGIN( APT_TICK_INTERVAL_TIMERS_ACTION, oActionSetup.m_strFunction );
                #else
                    APT_PROFILE_BEGIN( APT_TICK_INTERVAL_TIMERS_ACTION, "gAptActionInterpreter.PrepareForExecution" );
                #endif

                void * pSavedValue   = gAptActionInterpreter.PrepareForExecution(&oActionSetup);

                APT_PROFILE_END( APT_TICK_INTERVAL_TIMERS_ACTION );

                // HERE WE NEED TO PUSH ALL maIntervalTimers[i].pParams onto stack and replace the -1 with the num of params
                for(int32_t j = 0; j < nParams; j++)
                {
                    gAptActionInterpreter.stackPush(maIntervalTimers[i].pParams.at(j));
                }

                // (PDS) moved following line above the function call for bug #1008
                maIntervalTimers[i].fCurTime += maIntervalTimers[i].fInterval;

                #if APT_PROF_ENABLED && APT_PROFILE_ACTIONSCRIPT_ENABLED
                APT_PROF_ENTER((char8_t *)(maIntervalTimers[i].strFunction ? maIntervalTimers[i].strFunction : "AsIntervalFnc"));
                #endif

#ifdef APT_DEBUGGER_ENABLE
                if (pScriptCIH)
                {
                    AptDebugger::GetInstance()->PushCallStack(pFuncValue, "AptTimerFunc", reinterpret_cast<unsigned char* const>(AptDebugger::INVALID_OFFSET), pScriptCIH);
                }
#endif

                APT_PROFILE_BEGIN( APT_TICK_INTERVAL_TIMERS_ACTION, "gAptActionInterpreter.callFunction" );
                gAptActionInterpreter.callFunction(pContext, pFuncValue, nParams);
                APT_PROFILE_END( APT_TICK_INTERVAL_TIMERS_ACTION );

#ifdef APT_DEBUGGER_ENABLE
                if (pScriptCIH)
                {
                    AptDebugger::GetInstance()->PopCallStack(pFuncValue);
                }
#endif

                #if APT_PROF_ENABLED && APT_PROFILE_ACTIONSCRIPT_ENABLED
                APT_PROF_LEAVE((char8_t *)(maIntervalTimers[i].strFunction ? maIntervalTimers[i].strFunction : "AsIntervalFnc"));
                #endif

                APT_PROFILE_BEGIN( APT_TICK_INTERVAL_TIMERS_ACTION, "gAptActionInterpreter.CleanupAfterExecution" );

                //gAptActionInterpreter.CleanupAfterExecution("tickIntervalTimers", pSavedValue);
                gAptActionInterpreter.CleanupAfterExecution(pSavedValue, &oActionSetup);

                APT_PROFILE_END( APT_TICK_INTERVAL_TIMERS_ACTION );

                // Should only have one Item left on the stack.
                APT_ASSERT(gAptActionInterpreter.stack.GetSize() == 1);

                // Either way we Get clear out the stack elements.
                gAptActionInterpreter.stack.Pop( gAptActionInterpreter.stack.GetSize() );

                // (PDS) moved following line above the function call for bug #1008
                //maIntervalTimers[i].fCurTime += maIntervalTimers[i].fInterval;
                APT_DECSAFE(pContext);
            }
            else
            {
                // ONA 2004/3/23: The function can't be handled by the tickIntervalTimers
                //  We need to remove it from the list
                APT_DEC(maIntervalTimers[i].pCBFunction);
                APT_DECSAFE(maIntervalTimers[i].pContext);
                maIntervalTimers[i].CleanParams();
                maIntervalTimers[i].bValid = false;
            }
        }

    }

    APT_PROFILE_END( APT_TICK_INTERVAL_TIMERS );
}

/*F************************************************************************************************/
/*!
    \Function    AptAnimationTarget::TickNewInsts

    \Description
        Ticks the display list of all the new movieclips that where added to this list

    \Input
        None.

    \Output
        None.

    \Notes
        None.

    \Version    1.0        09/01/05 (LEO) First Version

*/
/************************************************************************************************F*/
void AptAnimationTarget::TickNewInsts(void)
{
    for (int32_t i = 0; i < snNewInsts; i++)
    {
        if(sapNewInsts[i] == NULL)
        {
            continue;
        }
        #if defined APT_USE_BUTTONS
        if (sapNewInsts[i]->IsButtonInst())
        {
            AptCharacterButtonInst *pButtonInst = sapNewInsts[i]->GetButtonInst();
            if (pButtonInst->mnState == AptCharacterButtonRecordState_None)
            {
                sapNewInsts[i]->gotoState(AptCharacterButtonRecordState_Up);
            }
        }
        else
        #endif
        if (sapNewInsts[i]->IsSpriteInst())
        {
            AptCharacterSpriteInst *pSprInst = sapNewInsts[i]->GetSpriteInst();
            if (pSprInst->mnFrame == -1)
            {
                sapNewInsts[i]->tick();
            }
        }
        APT_DECSAFE(sapNewInsts[i]);
        sapNewInsts[i] = NULL;
    }
    snNewInsts = 0;
}

/*F************************************************************************************************/
/*!
    \Function    AptAnimationTarget::runActions

    \Description
        Runs the actionscipt actions added to action queue by various functions.

    \Input
        None.

    \Output
        None.

    \Notes
        This gets called by core loop inside _tick() function

    \Version    1.0        03/16/05 (LEO) First Version
    \Version    1.1        03/16/05 (LEO) $bug874 Null check required in AptAnimationTarget::runActions.
    \Version    1.2        12/06/05 (ASK) Added extra type of function AAT_ALT_INP_FUNCTION which will be used
                                          for executing alternate input type functions.
    \Version    1.3        11/21/07 (JP)  Checking that action.pCIH is not NULL
    \Version    1.4        03/23/09 (RB)  Call TickNewInsts for case AAT_FUNCTION and AAT_ALT_INP_FUNCTION.
                                          See testcase gotoAndStop-TickNewInsts.
    \Version    1.5        04/30/09 (RB)  Added SWF filename in ActionScript callstack for AAT_FUNCTION events.
                                          When EA_DEBUG or APT_DO_AS_PROFILE is defined, a function description
                                          will also appear on the callstack.
*/
/************************************************************************************************F*/
#if defined (APT_SPREAD_ACTIONS_MULTI_FRAMES)
// when this #define is turned on, it tries to spread the actions executed over multiple frames to improve
// on spike frames. We tried testing it in Game NCAA 08 but it was not that helpful.
// But still keeping that code in, so we can try it out later if needed.
int32_t s_nMaxActionsToExecute = 10;
#endif
void AptAnimationTarget::RunActions()
{
    APT_PROFILE_BEGIN( APT_RUN_ACTIONS, "AptAnimationTarget::RunActions" );

    APT_PROFILE(run_actions);
    CleanRemList();

    AptActionQueueC *pCurrentPool = GetActionPool();
    AptActionQueueC::AptActionPool *pCur = pCurrentPool->GetFirstItem();

#if defined (APT_SPREAD_ACTIONS_MULTI_FRAMES)
    int32_t nActionsExecuted = 0;
    int32_t nQueueSize = pCurrentPool->GetDequeSize();
#endif

    // Runs all the actions in the display list action pool.
    {APT_PROFILE(AptAnimationTarget_runActions_loop);
    while (!pCurrentPool->IsLastItem(pCur))
    {
        //AptActionQueueC::AptActionPool *pOrigEnd = pCurrentPool->GetLastItem();
        pCurrentPool->SetCurItem(pCur); // $bug846 - MCB - Save off current pointer so we don't remove it from the action pool underneath ourselves.
        if(pCur->eActionType == AptActionQueueC::AAT_ACTION)
        {
            APT_PROFILE(AptAnimationTarget_runActions_runStream);
            // the classic action
            gAptActionInterpreter.input = pCur->input;

            // Here we check if the nFrame is negative.  If so, we need to check if this action is running in the correct frame num for this sprite inst.
            if (pCur->action.pCIH && !pCur->action.pCIH->isUndefined() &&
                !pCur->action.pCIH->IsLevelInst() &&
                pCur->action.pCIH->GetCIHState() != AptCIH::AptCIHState_Unloaded &&
                pCur->action.pCIH->IsSkipEval() == false &&
                !(pCur->action.nFrame < 0 && (pCur->action.pCIH->GetCharacterInst() != NULL && (pCur->action.nFrame * -1) != pCur->action.pCIH->GetSpriteInstBase()->mnFrame)))   // (LEO) $bug874 Null check required in AptAnimationTarget::runActions.
            {
                APT_DEFINE_ACTION_SETUP(pCur->action.pCIH, NULL, "AptRun-Actions", pCur->eAptActionType);

                #if defined (EA_DEBUG)
                    APT_PROFILE_BEGIN( APT_RUN_ACTIONS_ACTION, oActionSetup.m_strFunction );
                #else
                    APT_PROFILE_BEGIN( APT_RUN_ACTIONS_ACTION, "gAptActionInterpreter.PrepareForExecution" );
                #endif

                void * pSavedValue   = gAptActionInterpreter.PrepareForExecution(&oActionSetup);

                APT_PROFILE_END( APT_RUN_ACTIONS_ACTION );

                AptCharacterInst * pCharacterInst = (pCur->action.pCIH && !pCur->action.pCIH->isNone()) ? pCur->action.pCIH->GetRootAnimation()->GetAnimationInst() : NULL ;

#ifdef APT_DEBUGGER_ENABLE
                AptDebugger::GetInstance()->PushCallStack(pCur->action.pCIH, "Frame time", pCur->action.pBlock->aActionStream, pCur->action.pCIH);
#endif

                APT_PROFILE_BEGIN( APT_RUN_ACTIONS_ACTION, "gAptActionInterpreter.runStream" );
                gAptActionInterpreter.runStream(pCur->action.pBlock->aActionStream, pCur->action.pCIH, -1, pCharacterInst);
                APT_PROFILE_END( APT_RUN_ACTIONS_ACTION );

                APT_PROFILE_BEGIN( APT_RUN_ACTIONS_ACTION, "gAptActionInterpreter.CleanupAfterExecution" );
                gAptActionInterpreter.CleanupAfterExecution(pSavedValue, &oActionSetup);
                APT_PROFILE_END( APT_RUN_ACTIONS_ACTION );

#ifdef APT_DEBUGGER_ENABLE
                AptDebugger::GetInstance()->PopCallStack(pCur->action.pCIH);
#endif

                APT_ASSERT(_AptValidate());
                TickNewInsts();
            }
#if defined (APT_SPREAD_ACTIONS_MULTI_FRAMES)
            nActionsExecuted++;
#endif
        }
        else if(pCur->eActionType == AptActionQueueC::AAT_FUNCTION)
        {
            APT_PROFILE(AptAnimationTarget_runActions_callFunction);
            // queued function
            gAptActionInterpreter.input = pCur->input;
            gAptActionInterpreter.thisStack.push(pCur->function.pContext);

            APT_DEFINE_ACTION_SETUP(pCur->function.pContext, pCur->function.pFuncDef, "AptRun-Functions", pCur->eAptActionType);

            #if defined (EA_DEBUG)
                APT_PROFILE_BEGIN( APT_RUN_ACTIONS_ACTION, oActionSetup.m_strFunction );
            #else
                APT_PROFILE_BEGIN( APT_RUN_ACTIONS_ACTION, "gAptActionInterpreter.PrepareForExecution" );
            #endif

            void * pSavedValue   = gAptActionInterpreter.PrepareForExecution(&oActionSetup);

            APT_PROFILE_END( APT_RUN_ACTIONS_ACTION );

            #if defined(APT_USE_MOUSE)          // All mouse events are queued as keyboard events, except mouse movements
            if(pCur->input && INPUT_IS_KEY(&pCur->input))
            {
                AptInputController eController;
                GET_ACTION_CONTROLLER(pCur->input, eController);
                if(eController == AptInputController_Mouse)                         // F#563    5
                {
                    // (LEO) only do this if we have input and the input is from the mouse wheel
                    int32_t eType;
                    AptInputState eState;
                    GET_INPUT(pCur->input, eType, eState, eController);
                    if(pCur->function.nParams > 0 && (eState & (AptInputState_WheelDown | AptInputState_WheelUp)))
                    {
                        if(pCur->function.nParams > 1)
                        {
                            gAptActionInterpreter.stackPush(mpTopMostSprite);                // If we have more than one param, push the top most sprite on top
                        }

                        if(eState == AptInputState_WheelDown)                                       // No push the mouse wheel data, up or down
                        {
                            gAptActionInterpreter.stackPush(AptInteger::Create(-eType));
                        }
                        else
                        {
                            gAptActionInterpreter.stackPush(AptInteger::Create(eType));
                        }
                    }
                }
            }
            #endif

            #if APT_PROF_ENABLED && APT_PROFILE_ACTIONSCRIPT_ENABLED
            APT_PROF_ENTER((char8_t *)pCur->strFunction);
            #endif

            {
                // The function description is more accurate when EA_DEBUG or APT_DO_AS_PROFILE
                // is defined. Otherwise the description is simply "<movieclipevent>".
                const char8_t* functionDescription = _GetMovieClipEventFunctionDescription(pCur);

#ifdef APT_DEBUGGER_ENABLE
                bool isScriptActionFlag = AptDebugger::GetInstance()->PushCallStack(pCur->function.pFuncDef, functionDescription, reinterpret_cast<unsigned char* const>(AptDebugger::INVALID_OFFSET), pCur->function.pContext);
#endif

                gAptOptCallStack->Push(functionDescription, NULL, NULL, NULL );

                // WithStack needs a reference to the object so if we call super.onLoad (or any super stuff, really), we get the
                // right prototype context.  Otherwise we're going to call the same onLoad function twice. Colin C. 8/28/12
                gAptActionInterpreter.withStack.push(pCur->function.pContext);

                APT_PROFILE_BEGIN( APT_RUN_ACTIONS_ACTION, "gAptActionInterpreter.callFunction" );
                gAptActionInterpreter.callFunction(
                    pCur->function.pContext,
                    pCur->function.pFuncDef,
                    pCur->function.nParams,
                    NULL,
                    NULL,
                    true);
                APT_PROFILE_END( APT_RUN_ACTIONS_ACTION );

                gAptActionInterpreter.withStack.pop();

                gAptOptCallStack->Pop();

#ifdef APT_DEBUGGER_ENABLE
            if(isScriptActionFlag)
            {
                AptDebugger::GetInstance()->PopCallStack(pCur->function.pFuncDef);
            }
#endif
            }

            #if APT_PROF_ENABLED && APT_PROFILE_ACTIONSCRIPT_ENABLED
            APT_PROF_LEAVE((char8_t *)pCur->strFunction);
            #endif

            APT_PROFILE_BEGIN( APT_RUN_ACTIONS_ACTION, "gAptActionInterpreter.CleanupAfterExecution" );
            gAptActionInterpreter.CleanupAfterExecution(pSavedValue, &oActionSetup);
            APT_PROFILE_END( APT_RUN_ACTIONS_ACTION );

            gAptActionInterpreter.thisStack.pop();
            gAptActionInterpreter.stack.Pop();
            TickNewInsts();
#if defined (APT_SPREAD_ACTIONS_MULTI_FRAMES)
            nActionsExecuted++;
#endif
        }
#if defined(APT_ALTERNATE_INPUT)
        else if(pCur->eActionType == AptActionQueueC::AAT_ALT_INP_FUNCTION)
        {
            APT_PROFILE(AptAnimationTarget_runActions_callFunction);
            // queued function
            //gAptActionInterpreter.input = pCur->input;
            gAptActionInterpreter.thisStack.push(pCur->function.pContext);

            APT_DEFINE_ACTION_SETUP(pCur->function.pContext, pCur->function.pFuncDef, "AptRun-Functions-AltInput", pCur->eAptActionType);
            void * pSavedValue   = gAptActionInterpreter.PrepareForExecution(&oActionSetup);
#if APT_PROF_ENABLED && APT_PROFILE_ACTIONSCRIPT_ENABLED
            APT_PROF_ENTER((char8_t *)pCur->strFunction);
#endif
            // push the passed in AptValue
            gAptActionInterpreter.stackPush(pCur->mAltInputValue ? pCur->mAltInputValue : gpUndefinedValue);
            gAptActionInterpreter.callFunction(pCur->function.pContext, pCur->function.pFuncDef, 1);
#if APT_PROF_ENABLED && APT_PROFILE_ACTIONSCRIPT_ENABLED
            APT_PROF_LEAVE((char8_t *)pCur->strFunction);
#endif
            gAptActionInterpreter.CleanupAfterExecution(pSavedValue, &oActionSetup);

            gAptActionInterpreter.thisStack.pop();
            gAptActionInterpreter.stack.Pop();
            TickNewInsts();
        }
#endif
        else
        {
#if !defined (APT_SPREAD_ACTIONS_MULTI_FRAMES)
            APT_ASSERT(0);//unknown action/function type
#endif
        }
        if(gAptActionInterpreter.stack.GetSize() > 0)
        {
            /// In the case of a pre-mature return we need to pop off the item here
            gAptActionInterpreter.stackPop();
            APT_ASSERT(gAptActionInterpreter.stack.GetSize() == 0);
        }
        #if defined(EA_DEBUG)
        APT_ASSERT(gAptActionInterpreter.debugCallStack.GetSize() == 0);
        #endif

        if (pCurrentPool->IsLastItemOrBeyond(pCur))
        {
            // Check pCur is within the action array boundaries, if not break.
            break;
        }

        // if the end of the queue is modified while we iterating through it, correct here
        // $bug 854 Crash in RunActions - commenting following code.
        //if (pOrigEnd > pCurrentPool->GetLastItem())
        //{
        // pCur -= pOrigEnd - pCurrentPool->GetLastItem();
        //}
#if defined (APT_SPREAD_ACTIONS_MULTI_FRAMES)
        pCurrentPool->ClearAction(pCur);
#endif
        pCur = pCurrentPool->GetNextItem(pCur);
#if defined (APT_SPREAD_ACTIONS_MULTI_FRAMES)
        if (nActionsExecuted >= s_nMaxActionsToExecute)
        {
            // just to print how many items we delayed this frame.
            int32_t nItemsExecuted = (pCur - pCurrentPool->GetFirstItem());
            printf("========Executed only %d action items out of %d\n", nItemsExecuted, nQueueSize);
            pCurrentPool->SetAsStart(pCur);
            break;
        }
#endif
    }
    }
    #if defined APT_USE_BUTTONS
    APT_ASSERT(ValidateBIL());
    #endif
    {
        APT_PROFILE(AptAnimationTarget_runActions_end);
        TickNewInsts();
#if defined (APT_SPREAD_ACTIONS_MULTI_FRAMES)
        if (nTemp1 < s_nMaxActionsToExecute)
        {
            pCurrentPool->ResetPool();
        }
#else
        pCurrentPool->ClearActions();
#endif
        CleanRemList();
    }

    APT_PROFILE_END( APT_RUN_ACTIONS );
}


#if defined(APT_ALTERNATE_INPUT)
/*F************************************************************************************************/
/*!
    \Function    AptAnimationTarget::AddAlternateInput

    \Description
    Adds the AptAltInput object to queue of AptAltInput

    \Input      pInput  - pointer to AptAltInput object

    \Output
        None.

    \Notes
        None.

    \Version    1.0        12/6/05 (LEO) First Version

*/
/************************************************************************************************F*/
void AptAnimationTarget::AddAlternateInput(AptAltInput *pInput)
{
    if(mnQueuedAltInputs >= miMaxQueuedInputs)
    {
        AptHighwaterMemoryMetrics* pMetrics  = AptHighwaterMemoryMetricsImpl::GetMetrics(AptHighwaterMemoryMetrics::TYPE_QUEUED_INPUTS );
        pMetrics->mHighwaterMaxCount = INT32_MAX;
        int32_t nPolicy = AptHighwaterMemoryMetrics::sOverflowCallback->OnOverflowDetected( *pMetrics, maQueuedAltInputs, pInput );
        if ( nPolicy & (int32_t)AptHighwaterMemoryMetrics::POLICY_APT_WILL_ASSERT )
        {
            APT_ASSERTM( (mnQueuedAltInputs < miMaxQueuedInputs), pMetrics->mHelpfulHint );
        }

        return;
    }

    maQueuedAltInputs[mnQueuedAltInputs].sEvent = pInput->sEvent ;
    maQueuedAltInputs[mnQueuedAltInputs].pValue = pInput->pValue;
    APT_INCSAFE(maQueuedAltInputs[mnQueuedAltInputs].pValue);
    mnQueuedAltInputs++;
}

/*F************************************************************************************************/
/*!
    \Function    AptAnimationTarget::ProcessAlternateEvents

    \Description
        Executes input for all items in the mAltListenerSet

    Runs through all items in the listener set and handles the passed in input on them

    \Input
        None

    \Output
        None.

    \Notes
        None.

    \Version    1.0        12/6/5 (LEO) First Version

*/
/************************************************************************************************F*/
void AptAnimationTarget::ProcessAlternateEvents()
{
    AptValue *pValue = NULL;
    int32_t nCount = 0;

    for (int32_t i = 0; i < mnQueuedAltInputs; i++)
    {
        nCount = 0;
        for (int32_t j = 0; j < mAltListenerSet.capacity(); j++)
        {
            if(nCount == mAltListenerSet.mnElements)
            {
                break;
            }
            if(mAltListenerSet.maElements[j] == NULL)
            {
                continue;
            }
            pValue = mAltListenerSet.maElements[j];

            AptNativeHash *pHash = pValue->GetNativeHashVirtual();
            if(pHash != NULL)
            {
                //AptValue *pFunc = pHash->Lookup(&(maQueuedAltInputs[i].sEvent));
                AptValue *pFunc = pValue->findChild(&maQueuedAltInputs[i].sEvent, NULL);
                if(pFunc != NULL && pFunc != gpUndefinedValue)
                {
                    GetTargetSim()->GetAnimationTarget()->AddAltInputFunctionBack((AptCIH *)pValue, pFunc, 0,
                            ACTION_PROF_CALL_PARAM(maQueuedAltInputs[i].sEvent.c_str())
                            ACTION_TYPE_CALL_PARAM(0x0)
                            maQueuedAltInputs[i].pValue);
                }
            }

            nCount++;
        }

        APT_DECSAFE(maQueuedAltInputs[i].pValue);
    }
}


/*F************************************************************************************************/
/*!
    \Function    AptActionQueueC::AddAltInputFunctionBack

    \Description
        Adds an item to the action pool
        The dequeue can get exhausted - this can happen in release builds as well (of course).
        We must therefore deal with it ALWAYS.

    \Input      pContext          -
    \Input      pFuncDef          -
    \Input      nParams           -
    \Input      pAltInputValue    - pointer to AptValue that was passed as a member of AptAltInput object
                                when alternate input was added.

    \Output
        None.

    \Notes
        None.

    \Version    1.0        12/06/05 (ASK) First Version

*/
/************************************************************************************************F*/
void AptActionQueueC::AddAltInputFunctionBack(AptCIH* pContext, AptValue* pFuncDef, int nParams, ACTION_PROF_DECL_PARAM ACTION_TYPE_DECL_PARAM AptValue * pAltInputValue)
{
    APT_ASSERT(pContext->getIsDefined());

    AptActionPool *pTempItem = IncrementDequeLocation(mpEndDeque);
    if(pTempItem != mpStartDeque)
    {
        mpEndDeque->eActionType = AAT_ALT_INP_FUNCTION;
        //mpEndDeque->input = input;
        mpEndDeque->mAltInputValue = pAltInputValue ;
        APT_INCSAFE(mpEndDeque->mAltInputValue) ;
        mpEndDeque->function.pContext = pContext;
        APT_INC(mpEndDeque->function.pContext);
        mpEndDeque->function.pFuncDef = pFuncDef;
        APT_INC(mpEndDeque->function.pFuncDef);
        mpEndDeque->function.nParams = nParams;
#if APT_PROF_ENABLED && APT_PROFILE_ACTIONSCRIPT_ENABLED
        mpEndDeque->strFunction = strFunction;
#endif
#if (EA_DEBUG)
        mpEndDeque->eAptActionType = (AptActionType) eAptActionType ;
#endif
        mpEndDeque = pTempItem;
    }
    else
    {
        mpMemMetrics->mHighwaterMaxCount = INT32_MAX;
        int32_t nPolicy = AptHighwaterMemoryMetrics::sOverflowCallback->OnOverflowDetected( *mpMemMetrics, NULL, NULL );
        if ( nPolicy & (int32_t)AptHighwaterMemoryMetrics::POLICY_APT_WILL_ASSERT )
        {
            APT_ASSERTM( false, mpMemMetrics->mHelpfulHint );
        }
        else
        {
            APT_DEBUGPRINT(APT_DEBUG_MSG_DEBUG_LVL, "!!!!!!!!!!!!! AptAnimationPoolData:  Dequeue is full !!!!!!!!" );
        }
}
}

#endif //#if defined(APT_ALTERNATE_INPUT)

/*F************************************************************************************************/
/*!
    \Function    AptAnimationTarget::AddAnalogInput

    \Description Adds the analog stick, trigger or shoulder info to the input queue

    \Input      analogInput            -

    \Output     None.

    \Notes      None.

    \Version    1.0        09/01/05 (LEO) First Version
    \Version    2.0        01/05/07 (JP)  Added analog triggers (issue 106856)
*/
/************************************************************************************************F*/
void AptAnimationTarget::AddAnalogInput(AptAnalogStickInfo analogInput)
{
    bool bAdd = false;

    switch(analogInput.nSide)
    {
        case AptInputType_LeftAnalogStick:
            bAdd = true;
            sgAStickLeft[analogInput.nController] = analogInput;
            break;
        case AptInputType_RightAnalogStick:
            bAdd = true;
            sgAStickRight[analogInput.nController] = analogInput;
            break;
        case AptInputType_PadL:
            // (JP) After a single keydown message from a trigger, getAnalogTriggerInfo can be called to get the "intensity" data
            // Keeping bAdd false here makes this special case no invasive, avoiding additional messages in the queue.
            if (analogInput.fXAxisValue != 0.0f)
            {
                sgATriggers[analogInput.nController].fXAxisValue = analogInput.fXAxisValue;
                sgATriggers[analogInput.nController].fYAxisValue = 0.0f;
                if (gbSavedInputsEnabled)
                {
                    // added this extra type of SaveInputRecord for trigger analog data. This is a special case because,
                    // for trigger buttons we just have normal key up/key down messages, but now we also support analog values
                    // for triggers on xenon. These analog values are
                    // stored in static array called AptAnimationTarget::sgATriggers[AptInputController_NumControllers]
                    // We save those values to saved input file and replay them. There will be a corresponding AddInput for keyup/keydown
                    // along with these AddAnalogInput call. But it might be before this record or after this record, depending on how
                    // user has passed to Apt originally.
                    // The analog data is more related to keydown message and not to key up message.
                    // If you call Key.getAnalogTriggerInfo() on keyup/keydown event, you will get same values.
                    AptSavedInputRecordTriggers triggerInfo;
                    triggerInfo.nTick = gnCurTick;
                    triggerInfo.nInput = SET_TRIGGER_INPUT();
#if defined(EA_SYSTEM_BIG_ENDIAN)
                    triggerInfo.nInput = ((triggerInfo.nInput << 24) & 0xff000000);
#endif
                    triggerInfo.mAnalogTriggerInfo = analogInput;
                    gAptFuncs.pfnDebugAddSavedInput(&triggerInfo, sizeof(AptSavedInputRecordTriggers));
                }
            }
            break;
        case AptInputType_PadR:
            sgATriggers[analogInput.nController].fYAxisValue = analogInput.fYAxisValue;
            sgATriggers[analogInput.nController].fXAxisValue = 0.0f;
            if (analogInput.fYAxisValue != 0.0f)
            {
                if (gbSavedInputsEnabled)
                {
                    AptSavedInputRecordTriggers triggerInfo;
                    triggerInfo.nTick = gnCurTick;
                    triggerInfo.nInput = SET_TRIGGER_INPUT();
#if defined(EA_SYSTEM_BIG_ENDIAN)
                    triggerInfo.nInput = ((triggerInfo.nInput << 24) & 0xff000000);
#endif
                    triggerInfo.mAnalogTriggerInfo = analogInput;
                    gAptFuncs.pfnDebugAddSavedInput(&triggerInfo, sizeof(AptSavedInputRecordTriggers));
                }
            }
            break;
        default:
            break;
    }

    if(bAdd)
    {
        uint32_t nInput = SET_ANALOG_INPUT(analogInput.nSide, AptInputState_Pressed, analogInput.nController);
        bool bAdded = AddInput(nInput);

        if (gbSavedInputsEnabled && bAdded)
        {
            // we have already added nInput to SavedInputs and now we only want to add the AnalogData to savedinputs.
            gAptFuncs.pfnDebugAddSavedInput((AptSavedInputRecord *)&analogInput, sizeof(AptAnalogStickInfo));
        }

    }
}

/*F************************************************************************************************/
/*!
    \Function    AptAnimationTarget::AddGestureInput

    \Description Adds the gesture info to the input queue

    \Input      gestureInput            -

    \Output     None.

    \Notes      None.

    \Version    1.0        06/07/11 (BRH) First Version
*/
/************************************************************************************************F*/
void AptAnimationTarget::AddGestureInput(AptGestureInfo gestureInput)
{
    maQueuedGestures[gestureInput.nController][gestureInput.nGesture - AptInputType_GestureStart] = gestureInput;

    uint32_t nInput = SET_GESTURE_INPUT(gestureInput.nGesture,AptInputState_Pressed, gestureInput.nController);

    AddInput(nInput);
}

/*F************************************************************************************************/
/*!
    \Function    AptAnimationTarget::addInput

    \Description
        Adds the input to inputQueue

    \Input      nInput            - input integer

    \Output
        true if input is added to queue, false otherwise.

    \Notes
        -Added code to print32_t out the input passed into the addInput function

    \Version    1.0        09/14/04 (LEO) First Version
        1.1        03/22/05 (LEO) Added Big Endian support
        1.2         02/08/2007 (ASK) now returns true if input is added to inputqueue other wise false

*/
/************************************************************************************************F*/
bool AptAnimationTarget::AddInput(uint32_t nInput)
{

#if defined(APT_PRINT_INPUT)
    int32_t eType;
    AptInputController eController;
    AptInputState eState;

    if(INPUT_IS_KEY(&nInput))
    {
        GET_INPUT(nInput, eType, eState, eController);
        if(eController == AptInputController_Mouse)
            APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, "%i Adding Mouse Input: eType[%i] eState[%i] eController[%i]\n", nInput, eType, eState, eController);
        else
            APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, "%i Adding Keyboard Input: eType[%i] eState[%i] eController[%i]\n", nInput, eType, eState, eController);
    }
    else if(INPUT_IS_ANALOG(&nInput))
    {
        GET_INPUT(nInput, eType, eState, eController);
        APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, "%i Adding Analog Input: eType[%i] eState[%i] eController[%i]\n", nInput, eType, eState, eController);
    }
    else if(INPUT_IS_GESTURE(&nInput))
    {
        GET_INPUT(nInput, eType, eState, eController);
        APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, "%i Adding Gesture Input: eType[%i] eState[%i] eController[%i]\n", nInput, eType, eState, eController);
    }
    else if(INPUT_IS_MOUSE(&nInput))
    {
        float fX = 0.f, fY = 0.f;
        GET_MOUSE_INPUT(nInput, fX, fY);
        APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, "%i Adding Mouse Input: mouse_x[%f] mouse_f[%f]\n", nInput, fX, fY);
    }
#endif
    if(mnQueuedInputs >= miMaxQueuedInputs)
    {
        AptHighwaterMemoryMetrics* pMetrics  = AptHighwaterMemoryMetricsImpl::GetMetrics(AptHighwaterMemoryMetrics::TYPE_QUEUED_INPUTS );
        pMetrics->mHighwaterMaxCount = INT32_MAX;
        int32_t nPolicy = AptHighwaterMemoryMetrics::sOverflowCallback->OnOverflowDetected( *pMetrics, maQueuedInputs, &nInput );
        if ( nPolicy & (int32_t)AptHighwaterMemoryMetrics::POLICY_APT_WILL_ASSERT )
        {
            APT_ASSERTM( (mnQueuedInputs < miMaxQueuedInputs), pMetrics->mHelpfulHint );
        }

        return false;
    }

#if defined(APT_USE_REPEATED_INPUT_OPTI)
    // DC - Don't repeat the message, we end up doing thousands of repeated messages. (Release 17.00)
    // $bug687 - DC - 17.02 - Doh! my bad. Was removing too much. The intent is basically to
    // prevent us from getting 3 billion of the same message, (cause then we will run the event handlers
    // 3 billion times). This should take care of the issue without cutting too much off.
    if ( mnQueuedInputs > 0 )
    {
        if ( maQueuedInputs[ mnQueuedInputs - 1 ] == nInput )
        {
            return false;
        }
    }
#endif

    maQueuedInputs[mnQueuedInputs] = nInput;
    mnQueuedInputs++;

    if (gbSavedInputsEnabled)
    {
        AptSavedInputRecordInput inputRecord;
        inputRecord.nTick = gnCurTick;
#if defined(EA_SYSTEM_BIG_ENDIAN)
        // Byte swap nInput
        volatile uint32_t temp = nInput;

        char * buf = (char *)&temp;

        for (int32_t i=0; i<4; i+=2) {
            buf[i]   ^= buf[i+1];
            buf[i+1] ^= buf[i];
            buf[i]   ^= buf[i+1];
        }

        short * buf2 = (short *)buf;

        for (int32_t i=0; i<2; i+=2) {
            buf2[i]   ^= buf2[i+1];
            buf2[i+1] ^= buf2[i];
            buf2[i]   ^= buf2[i+1];
        }
        nInput = temp;
#endif
        inputRecord.nInput = nInput;

        gAptFuncs.pfnDebugAddSavedInput(&inputRecord, APT_SAVEINPUTS_RECORD_SIZE);
    }
    return true;
}

/*F************************************************************************************************/
/*!
    \Function    AptAnimationTarget::AddInput

    \Description
        Adds the input to the input queue

    \Input      eType             -
    \Input      eState            -
    \Input      eController       -

    \Output
        None.

    \Notes
        None.

    \Version    1.0        09/01/05 (LEO) First Version

*/
/************************************************************************************************F*/
void AptAnimationTarget::AddInput(AptInputType eType, AptInputState eState, AptInputController eController)
{
    uint32_t nInput = 0;
#if defined(APT_USE_MOUSE)
    if(eController == AptInputController_Mouse)
    {
        nInput = SET_KEYBOARD_INPUT(eType, eState, eController);
    }
    else
#endif
    {
        nInput = SET_KEYBOARD_INPUT(eType, eState, eController);
    }
    AddInput(nInput);
}

#if defined(APT_USE_MOUSE)
void AptAnimationTarget::AddInput(int32_t x, int32_t y)
{
    // Skip this code if the mouse flag is false
    uint32_t nInput = SET_MOUSE_INPUT(x, y);
    AddInput(nInput);
}
#endif

AptGestureInfo *AptAnimationTarget::GetGestureInfo(int32_t Controller, AptInputType gestureID)
{
    if(gestureID >= AptInputType_GestureStart && gestureID <= AptInputType_GestureEnd)
    {
        return &maQueuedGestures[Controller][gestureID - AptInputType_GestureStart];
    }
    else
    {
        return NULL;
    }
}

/*F************************************************************************************************/
/*!
    Removes all of the timer functions associated with a movie clip instance.  Only script
    timers can be associated with a movie clip.  Native (C++) functions only exist in objects and
    don't have knowledge of their parents.  They only get removed explicitly or when the pool
    is shut down

    \Input *pAnimCIH    - the movie clip instance to remove the timer functions for

    \Output
        None.

    \Notes
        None.

    \Version    1.0        04/23/04 (JSH) First Version
    \Version    1.1        03/16/05 (LEO) $bug867 removeTimerFunctions erroneously removes timers

*/
/************************************************************************************************F*/
void AptAnimationTarget::RemoveTimerFunctions(AptCIH *pAnimCIH)
{
    // go through the list and remove timers belonging to the movie clip instance
    for (int32_t i = 0; i < GetMaxIntervalTimers(); i++)
    {
        if (maIntervalTimers[i].bValid)
        {
            // if we're a valid script function belonging to the movie clip instance, remove us
            if((pAnimCIH->GetCharacterInst() != NULL) &&
                ((maIntervalTimers[i].pCBFunction->isScriptFunction() &&
                (maIntervalTimers[i].pCBFunction->c_scriptfunction()->mpCIH->GetCharacterInst() == NULL ||
                (maIntervalTimers[i].pCBFunction->c_scriptfunction()->mpCIH->GetCharacterInst() == pAnimCIH->GetCharacterInst())
                ))))        // (LEO) removed mpFuncAnim
            {
                APT_DEC(maIntervalTimers[i].pCBFunction);
                APT_DECSAFE(maIntervalTimers[i].pContext);
                maIntervalTimers[i].CleanParams();       // set interval fix for supporting params.
                maIntervalTimers[i].bValid = false;
            }
        }
    }
}

#if defined APT_USE_BUTTONS
/*F************************************************************************************************/
/*!
    \Function    AptAnimationTarget::ClearBIL

    \Description
        Clears out the BIL list

    \Input
        None.

    \Output
        None.

    \Notes
        None.

    \Version    1.0        09/01/05 (LEO) First Version

*/
/************************************************************************************************F*/
void AptAnimationTarget::ClearBIL()
{
    for (int32_t i = 0; i < snBILCount; i++)
    {
        APT_DEC(saButtonInstanceList[i].pCIH);
    }
    snBILCount = 0;
}

/*F************************************************************************************************/
/*!
    \Function    AptAnimationTarget::AppendButtonToBIL

    \Description
        Adds a button to the BIL list

    \Input      *pBI              -
    \Input      *pMatrix          -

    \Output
        None.

    \Notes
        None.

    \Version    1.0        09/01/05 (LEO) First Version

*/
/************************************************************************************************F*/
void AptAnimationTarget::AppendButtonToBIL(AptCIH *pBI, AptMatrix *pMatrix)
{
    AptHighwaterMemoryMetrics* pMetrics  = AptHighwaterMemoryMetricsImpl::GetMetrics(AptHighwaterMemoryMetrics::TYPE_BUTTON_INSTANCES);
    if ( snBILCount >= siButtonInstanceListSize )
    {
        pMetrics->mHighwaterMaxCount = INT32_MAX;
        int32_t nPolicy = AptHighwaterMemoryMetrics::sOverflowCallback->OnOverflowDetected( *pMetrics, saButtonInstanceList, pBI );
        if ( nPolicy & (int32_t)AptHighwaterMemoryMetrics::POLICY_APT_WILL_ASSERT )
        {
            APT_ASSERTM( (snBILCount < siButtonInstanceListSize), pMetrics->mHelpfulHint );
        }
        return;
    }

    saButtonInstanceList[snBILCount].pCIH = pBI;
    APT_INC(pBI);
    saButtonInstanceList[snBILCount].matrix = *pMatrix;
    snBILCount++;

    if ( snBILCount > pMetrics->mHighwaterMaxCount )
    {
        pMetrics->mHighwaterMaxCount = snBILCount;
    }
}

/*F************************************************************************************************/
/*!
    \Function    AptAnimationTarget::RemoveFromBIL

    \Description
        Removes button from BIL list

    \Input      *pBI              -

    \Output
        None.

    \Notes
        None.

    \Version    1.0        09/01/05 (LEO) First Version

*/
/************************************************************************************************F*/
void AptAnimationTarget::RemoveFromBIL(AptCIH *pBI)
{
    for (int32_t i = 0; i <= snBILCount - 1; i++)        // bug fixed in 0.7.11
    {
        if (pBI == saButtonInstanceList[i].pCIH)
        {
            APT_ASSERT(pBI->IsButtonInst());
            APT_DEC(pBI);
            memmove(&saButtonInstanceList[i], &saButtonInstanceList[i + 1], sizeof(saButtonInstanceList[i]) * (snBILCount - i));
            snBILCount--;
        }
    }
}
#endif

/*F************************************************************************************************/
/*!
    \Function    AptAnimationTarget::SetInputMask

    \Description
        Sets the input mask pointer

    This is some legacy feature that no one knows about... and no one uses??

    \Input      *pMask            -

    \Output
        None.

    \Notes
        None.

    \Version    1.0        09/01/05 (LEO) First Version

*/
/************************************************************************************************F*/
void AptAnimationTarget::SetInputMask(AptCIH *pMask)
{
    // TODO BUGBUG; this needs to be INCd and DECd appropriately
    mpInputMask = pMask;
}

/*F************************************************************************************************/
/*!
    \Function    AptAnimationTarget::IsInputMasked

    \Description
        Checks if the AptCIH passed is input masked

    This is some legacy feature that no one knows about... and no one uses??

    \Input      *pObj             -

    \Output     bool              -

    \Notes
        None.

    \Version    1.0        09/01/05 (LEO) First Version

*/
/************************************************************************************************F*/
bool AptAnimationTarget::IsInputMasked(AptCIH *pObj)
{
    if (mpInputMask == NULL)
    {
        return false;
    }

    AptCIH *pTemp = pObj;

    while (pTemp != NULL)
    {
        if (pTemp == mpInputMask)
        {
            return false;
        }
        else
        {
            pTemp = pTemp->GetDisplayListParent();
        }
    }
    return true;
}

/*F************************************************************************************************/
/*!
    \Function    AptAnimationTarget::RegisterReferences

    \Description
        This calls APT_REGISTER_REFERENCE_SAFE on all of the references.
    \Input
        None.

    \Output
        None.

    \Notes
        None.

    \Version    1.0        09/09/04 (DC)  Bringin' it to ya
    \Version    1.1        05/19/05 (DC)  Now Registers the RemList Items.
    \Version    1.2        12/6/05  (ASK) added mAltListenerSet code for alternate input handling

*/
/************************************************************************************************F*/
void AptAnimationTarget::RegisterReferences()
{
#if defined APT_USE_BUTTONS
    APT_REGISTER_REFERENCE_ANONYMOUS_SAFE(mpFocusButton, "AptAnimationTarget::mpFocusButton", APT_REFREG_IS_APTCIH);
#endif
    APT_REGISTER_REFERENCE_ANONYMOUS_SAFE(mpInputMask,  "AptAnimationTarget::mpInputMask", APT_REFREG_IS_APTCIH);

    for (int32_t i = 0; i < snNewInsts; i++)
    {
        if(sapNewInsts[i])
        {
            // $bug981 Memory stomp due incorrect code in RegisterReferences(). - removed * in front of mapNewInsts[i]
            APT_REGISTER_REFERENCE_ANONYMOUS(sapNewInsts[i], "AptAnimationTarget::mapNewInsts", APT_REFREG_IS_APTCIH);
        }
    }

    int32_t nLoopCount = 0;
#if defined APT_USE_BUTTONS
    for (int32_t i = 0; i < snBILCount; i++)
    {
        APT_REGISTER_REFERENCE_ANONYMOUS(saButtonInstanceList[i].pCIH, "AptAnimationTarget::saButtonInstanceList[i].pCIH", APT_REFREG_IS_APTCIH);
    }

    // Warning, these loops go through a number of items that are null,
    // is there a more efficient way to do this loop?
    nLoopCount = mButtonSet.capacity();
    for (int32_t i = 0; i < nLoopCount; i++)
    {
        APT_REGISTER_REFERENCE_ANONYMOUS_SAFE(mButtonSet.maElements[i], "AptAnimationTarget::mButtonSet.aElements", APT_REFREG_IS_APTVALUE);
    }
#endif

    // Warning, these loops go through a number of items that are null,
    // is there a more efficient way to do this loop?
    nLoopCount = mListenerSet.capacity();
    for (int32_t i = 0; i < nLoopCount; i++)
    {
        APT_REGISTER_REFERENCE_ANONYMOUS_SAFE(mListenerSet.maElements[i], "AptAnimationTarget::mListenerSet.aElements", APT_REFREG_IS_APTVALUE);
    }

    // Warning, these loops go through a number of items that are null,
    // is there a more efficient way to do this loop?
#if defined(APT_USE_MOUSE)
    nLoopCount = mMouseListenerSet.capacity();
    for (int32_t i = 0; i < nLoopCount; i++)
    {
        APT_REGISTER_REFERENCE_ANONYMOUS_SAFE(mMouseListenerSet.maElements[i], "AptAnimationTarget::mMouseListenerSet.aElements", APT_REFREG_IS_APTVALUE);
    }
#endif

#if defined(APT_ALTERNATE_INPUT)
    nLoopCount = mAltListenerSet.capacity();
    for (int i = 0; i < nLoopCount; i++)
    {
        APT_REGISTER_REFERENCE_ANONYMOUS_SAFE(mAltListenerSet.maElements[i], "AptAnimationPoolData::mouseListenerSet.aElements", APT_REFREG_IS_APTVALUE);
    }
#endif

    // Warning, these loops go through a number of items that are null,
    // is there a more efficient way to do this loop?
    nLoopCount = mInputSet.capacity();
    for (int32_t i = 0; i < nLoopCount; i++)
    {
        APT_REGISTER_REFERENCE_ANONYMOUS_SAFE(mInputSet.maElements[i], "AptAnimationTarget::inputSet.aElements", APT_REFREG_IS_APTVALUE);
    }

    //  mDisplayList
    if(mDisplayList.pState)
    {
        mDisplayList.pState->RegisterReferences(NULL);
    }


    //  aActionPool
    mpAptActionPool->RegisterReferences();

    //  maIntervalTimers
    for(int32_t i = 0 ; i < GetMaxIntervalTimers() ; ++i)
    {
        if (maIntervalTimers[i].bValid == false)
        {
            continue;
        }

        APT_REGISTER_REFERENCE_ANONYMOUS(maIntervalTimers[i].pCBFunction, "AptAnimationTarget::maIntervalTimers[i].pCBFunction", APT_REFREG_IS_APTVALUE);
        APT_REGISTER_REFERENCE_ANONYMOUS(maIntervalTimers[i].pContext, "AptAnimationTarget::maIntervalTimers[i].pContext", APT_REFREG_IS_APTVALUE);

        nLoopCount = maIntervalTimers[i].pParams.size();
        for (int32_t j = 0; j < nLoopCount; j++)
        {
            AptValue * pValue = maIntervalTimers[i].pParams.at(j);
            APT_REGISTER_REFERENCE_ANONYMOUS(pValue, "AptAnimationTarget::maIntervalTimers[i].pParams", APT_REFREG_IS_APTVALUE);
        }
    }
    for(int32_t i = 0; i < snDelayedReleaseCount; i++)
    {
        AptValue * pValue = sapDelayedReleaseList[i];
        if(pValue != NULL)
        {
            APT_REGISTER_REFERENCE_ANONYMOUS(pValue, "apDelayedReleaseList[i]", APT_REFREG_IS_APTVALUE);
        }
    }

    return ;
}

/*F************************************************************************************************/
/*!
    \Function    AptAnimationTarget::RemoveCIHReferences

    \Description
        Removes all the CIH references (that are in apDelayedReleaseList) which are referenced
        in various data structures inside AnimationTarget. Like various input, listener queues,
        or timer functions.

    \Input
        None.

    \Output
        None.

    \Notes
        None.

    \Version    1.0        04/12/05 (LEO) First Version
    \Version    1.1        12/6/05  (ASK) added mAltListenerSet code for alternate input handling
*/
/************************************************************************************************F*/
void AptAnimationTarget::RemoveCIHReferences()
{
#if defined APT_USE_BUTTONS
    APT_REGISTER_REFERENCE_ANONYMOUS_SAFE(mpFocusButton,"AptAnimationTarget::mpFocusButton", APT_REFREG_IS_APTCIH);
#endif
    APT_REGISTER_REFERENCE_ANONYMOUS_SAFE(mpInputMask,  "AptAnimationTarget::mpInputMask", APT_REFREG_IS_APTCIH);

    for (int32_t i = 0; i < snNewInsts; i++)
    {
        if(sapNewInsts[i] != NULL)
            APT_REGISTER_REFERENCE_ANONYMOUS_SAFE(sapNewInsts[i], "mapNewInsts Element", APT_REFREG_IS_APTVALUE);
    }

    int32_t nLoopCount = 0;
#if defined APT_USE_BUTTONS
    for (int32_t i = 0; i < snBILCount; i++)
    {
        APT_REGISTER_REFERENCE_ANONYMOUS_SAFE(saButtonInstanceList[i].pCIH, "saButtonInstanceList Element", APT_REFREG_IS_APTCIH);
    }

    // Warning, these loops go through a number of items that are null,
    // is there a more efficient way to do this loop?
    nLoopCount = mButtonSet.capacity();
    for (int32_t i = 0; i < nLoopCount; i++)
    {
        if(mButtonSet.maElements[i])
            APT_REGISTER_REFERENCE_ANONYMOUS_SAFE(mButtonSet.maElements[i], "mButtonSet Element", APT_REFREG_IS_APTVALUE);
    }
#endif

    // Warning, these loops go through a number of items that are null,
    // is there a more efficient way to do this loop?
    nLoopCount = mListenerSet.capacity();
    for (int32_t i = 0; i < nLoopCount; i++)
    {
        if(mListenerSet.maElements[i])
            APT_REGISTER_REFERENCE_ANONYMOUS_SAFE(mListenerSet.maElements[i], "mListenerSet Element", APT_REFREG_IS_APTVALUE);
    }

    // Warning, these loops go through a number of items that are null,
    // is there a more efficient way to do this loop?
#if defined(APT_USE_MOUSE)
    nLoopCount = mMouseListenerSet.capacity();
    for (int32_t i = 0; i < nLoopCount; i++)
    {
        if(mMouseListenerSet.maElements[i])
            APT_REGISTER_REFERENCE_ANONYMOUS_SAFE(mMouseListenerSet.maElements[i], "mMouseListenerSet Element", APT_REFREG_IS_APTVALUE);
    }
#endif

#if defined(APT_ALTERNATE_INPUT)
    nLoopCount = mAltListenerSet.capacity();
    for (int i = 0; i < nLoopCount; i++)
    {
        if(mAltListenerSet.maElements[i])
            APT_REGISTER_REFERENCE_ANONYMOUS_SAFE(mAltListenerSet.maElements[i], "mouseListenerSet Element", APT_REFREG_IS_APTVALUE);
    }
#endif

    // Warning, these loops go through a number of items that are null,
    // is there a more efficient way to do this loop?
    nLoopCount = mInputSet.capacity();
    for (int32_t i = 0; i < nLoopCount; i++)
    {
        if(mInputSet.maElements[i])
            APT_REGISTER_REFERENCE_ANONYMOUS_SAFE(mInputSet.maElements[i], "inputSet Element", APT_REFREG_IS_APTVALUE);
    }

    //  maIntervalTimers
    for(int32_t i = 0 ; i < GetMaxIntervalTimers() ; ++i)
    {
        if (maIntervalTimers[i].bValid == false)
        {
            continue;
        }

        maIntervalTimers[i].pCBFunction->RegisterReferences();
        maIntervalTimers[i].pContext->RegisterReferences();

        nLoopCount = maIntervalTimers[i].pParams.size();
        for (int32_t j = 0; j < nLoopCount; j++)
        {
            AptValue * pValue = maIntervalTimers[i].pParams.at(j);
            APT_REGISTER_REFERENCE_ANONYMOUS_SAFE(pValue, "IntervalTimerParam", APT_REFREG_IS_APTVALUE);
            if(pValue != maIntervalTimers[i].pParams.at(j))
            {
                APT_ASSERT(pValue == gpUndefinedValue);
                maIntervalTimers[i].pParams.SetAt(j, gpUndefinedValue);
            }
        }
    }
    return ;
}

#if defined(APT_USE_BUTTONS)
/*F************************************************************************************************/
/*!
    \Function    AptAnimationTarget::SetValidFocusButton

    \Description
        This does some crazy stuff to figure out what the valid focus button is

    \Input
        None.

    \Output
        None.

    \Notes
        None.

    \Version    1.0        09/01/05 (LEO) First Version

*/
/************************************************************************************************F*/
void AptAnimationTarget::SetValidFocusButton()
{
    int32_t nX=0, nY=0;

    if (mpFocusButton == NULL || GetCharacterGridPosition(mpFocusButton->GetInstanceName(), &nX, &nY) == false)
    {
        // roll out current bad focus
        if (mpFocusButton)
        {
            mpFocusButton->gotoState(AptCharacterButtonRecordState_Up);
            DoButtonActions(mpFocusButton, AptActionConditionFlag_OverUpToIdle);
        }

        APT_DECSAFE(mpFocusButton);
        mpFocusButton = 0;

        // find a good focus button
        int32_t nCount = 0;
        for (int32_t i = 0; i < mButtonSet.capacity(); i++)
        {
            if (nCount == mButtonSet.mnElements)
                break;
            if (mButtonSet.maElements[i] == NULL)
                continue;

            if (mButtonSet.maElements[i]->GetInstanceName().IsEmpty() == false)
            {
                if (GetCharacterGridPosition(mButtonSet.maElements[i]->GetInstanceName(), &nX, &nY))
                {
                    mpFocusButton = mButtonSet.maElements[i];
                    APT_INC(mpFocusButton);
                    break;
                }
            }
            nCount++;
        }

        // roll into new focus
        if (mpFocusButton)
        {
            mpFocusButton->gotoState(AptCharacterButtonRecordState_Over);
            DoButtonActions(mpFocusButton, AptActionConditionFlag_IdleToOverUp);
        }
    }
}
#endif

#if defined(APT_USE_MOUSE)
/*F************************************************************************************************/
/*!
    \Function    AptAnimationTarget::dragMovieClip

    \Description
        Insert function's purpose here

    \Input
        None.

    \Output
        None.

    \Notes
        None.

    \Version    1.0        06/28/04 (LEO) First Version

*/
/************************************************************************************************F*/
void AptAnimationTarget::DragMovieClip()
{
    float fTmpX = snXMousePos - mDragPos.tx;                     // Update postion based on initial mouse click position
    float fTmpY = snYMousePos - mDragPos.ty;                     // Update postion based on initial mouse click position

    // Update MC position only if the new postion is within the boundary box given by the user.
    if(mDragPos.a != -9999.f && fTmpX < mDragPos.a)
    {
        fTmpX = mDragPos.a;
    }
    if(mDragPos.c != -9999.f && fTmpX > mDragPos.c)
    {
        fTmpX = mDragPos.c;
    }
    if(mDragPos.b != -9999.f && fTmpY < mDragPos.b)
    {
        fTmpY = mDragPos.b;
    }
    if(mDragPos.d != -9999.f && fTmpY > mDragPos.d)
    {
        fTmpY = mDragPos.d;
    }
    mpDragMC->c_cih()->SetProceduralProperty(AptProceduralProperty_X, fTmpX, true);
    mpDragMC->c_cih()->SetProceduralProperty(AptProceduralProperty_Y, fTmpY, true);
}
#endif


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
/// AptIntervalTimer
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
/*F************************************************************************************************/
/*!
    \Function    AptIntervalTimer::cleanParams

    \Description
        Clears the param list

    \Input
        None.

    \Output
        None

    \Notes
        None.

    \Version    1.0        07/21/05 (DC) Bringin' it to ya

*/
/************************************************************************************************F*/
void AptIntervalTimer::CleanParams()
{
    int i;
    int nP = pParams.size();
    for(i = 0; i < nP; i++)
    {
        pParams.pop();
    }
}


int32_t AptIntervalTimer::GenerateId()
{
    static int32_t snId = 0;
    snId++;
    return snId;
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
/// AptActionQueueC
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
/*F************************************************************************************************/
/*!
    \Function    AptActionQueueC::ClearActions

    \Description
        Clears all actions from the action pool

    \Input
        None.

    \Output
        None.

    \Notes
        None.

    \Version    1.0        10/27/04 (LEO) First Version

*/
/************************************************************************************************F*/
void AptActionQueueC::ClearActions()
{
    AptActionPool *pCur = GetFirstItem();
    while(!IsLastItem(pCur))
    {
        if(pCur->eActionType == AAT_ACTION)
        {
            APT_DEC(pCur->action.pCIH);
        }
        else if (pCur->eActionType == AAT_FUNCTION)
        {
            APT_DEC(pCur->function.pContext);
            APT_DEC(pCur->function.pFuncDef);
        }
#if defined (APT_ALTERNATE_INPUT)
        else if (pCur->eActionType == AAT_ALT_INP_FUNCTION)
        {
            APT_DEC(pCur->function.pContext);
            APT_DEC(pCur->function.pFuncDef);
            APT_DECSAFE(pCur->mAltInputValue) ;
        }
#endif
        else
        {
            APT_ASSERT(0);
        }
        pCur->eActionType = AAT_NONE;
        pCur = GetNextItem(pCur);
    }
    mpStartDeque = mpEndDeque = &maActionPool[0];
}

#if defined (APT_SPREAD_ACTIONS_MULTI_FRAMES)
void AptActionQueueC::ClearAction(AptActionPool * pCur)
{
        if(pCur->eActionType == AAT_ACTION)
        {
            APT_DEC(pCur->action.pCIH);
        }
        else if (pCur->eActionType == AAT_FUNCTION)
        {
            APT_DEC(pCur->function.pContext);
            APT_DEC(pCur->function.pFuncDef);
        }
#if defined (APT_ALTERNATE_INPUT)
        else if (pCur->eActionType == AAT_ALT_INP_FUNCTION)
        {
            APT_DEC(pCur->function.pContext);
            APT_DEC(pCur->function.pFuncDef);
            APT_DECSAFE(pCur->mAltInputValue) ;
        }
#endif
        else
        {
            //APT_ASSERT(0);
        }
        pCur->eActionType = AAT_NONE;
}


void AptActionQueueC::SetAsStart(AptActionPool *pCur)
{
    mpStartDeque = pCur;
}
#endif
/*F************************************************************************************************/
/*!
    \Function    AptActionQueueC::AddActionBack

    \Description
        Adds an item to the action pool

    The deque can get exhausted - this can happen in release builds as well (of course).
    We must therefore deal with it ALWAYS.

    \Input      *pActionBlock     - pointer to action block
    \Input      *pCIH             - The context in which the actions will be called
    \Input      input             - AptInput associated with action

    \Output
        None.

    \Notes
        None.

    \Version    1.0        10/27/04 (LEO) First Version

*/
/************************************************************************************************F*/
void AptActionQueueC::AddActionBack(AptActionBlock *pActionBlock, AptCIH *pCIH, ACTION_PROF_DECL_PARAM ACTION_TYPE_DECL_PARAM AptInput input)
{
    APT_ASSERT(pCIH->getIsDefined());

    AptActionPool *pTempItem = IncrementDequeLocation(mpEndDeque);
    if(pTempItem != mpStartDeque)
    {
        mpEndDeque->eActionType = AAT_ACTION;
        mpEndDeque->action.nFrame = pCIH->GetSpriteInstBase()->mnGotoAnded;         // Set the frame this action was execute on to the queued obj
        mpEndDeque->action.pBlock = pActionBlock;
        mpEndDeque->action.pCIH = pCIH;
        APT_INC(pCIH);
        mpEndDeque->input = input;
#if APT_PROF_ENABLED && APT_PROFILE_ACTIONSCRIPT_ENABLED
        mpEndDeque->strFunction = strFunction;
#endif
#if defined(EA_DEBUG)
        mpEndDeque->eAptActionType = (AptActionType) eAptActionType ;
#endif
        mpEndDeque = pTempItem;
    }
    else
    {
        mpMemMetrics->mHighwaterMaxCount = INT32_MAX;
        int32_t nPolicy = AptHighwaterMemoryMetrics::sOverflowCallback->OnOverflowDetected( *mpMemMetrics, NULL, NULL );
        if ( nPolicy & (int32_t)AptHighwaterMemoryMetrics::POLICY_APT_WILL_ASSERT )
        {
            APT_ASSERTM( false, mpMemMetrics->mHelpfulHint );
        }
    }
}

/*F************************************************************************************************/
/*!
    \Function    AptActionQueueC::AddActionFront

    \Description
        Adds an item to the action pool

        // The deque can get exhausted - this can happen in release builds as well (of course).
        // We must therefore deal with it ALWAYS.

    \Input      *pActionBlock     - pointer to action block
    \Input      *pCIH             - The context in which the actions will be called
    \Input      input             - AptInput associated with action

    \Output
        None.

    \Notes
        None.

    \Version    1.0        10/27/04 (LEO) First Version

*/
/************************************************************************************************F*/
void AptActionQueueC::AddActionFront(AptActionBlock *pActionBlock, AptCIH *pCIH, ACTION_PROF_DECL_PARAM ACTION_TYPE_DECL_PARAM AptInput input)
{
    APT_ASSERT(pCIH->getIsDefined());

    AptActionPool *pTempItem = DecrementDequeLocation(mpStartDeque);
    if(pTempItem != mpEndDeque)
    {
        mpStartDeque = pTempItem;
        mpStartDeque->action.nFrame = pCIH->GetSpriteInstBase()->mnGotoAnded;           // Set the frame this action was execute on to the queued obj
        mpStartDeque->eActionType = AAT_ACTION;
        mpStartDeque->action.pBlock = pActionBlock;
        mpStartDeque->action.pCIH = pCIH;
        APT_INC(pCIH);
        mpStartDeque->input = input;
#if APT_PROF_ENABLED && APT_PROFILE_ACTIONSCRIPT_ENABLED
        mpStartDeque->strFunction = strFunction;
#endif
#if defined(EA_DEBUG)
        mpStartDeque->eAptActionType = (AptActionType) eAptActionType ;
#endif
    }
    else
    {
        mpMemMetrics->mHighwaterMaxCount = INT32_MAX;
        int32_t nPolicy = AptHighwaterMemoryMetrics::sOverflowCallback->OnOverflowDetected( *mpMemMetrics, NULL, NULL );
        if ( nPolicy & (int32_t)AptHighwaterMemoryMetrics::POLICY_APT_WILL_ASSERT )
        {
            APT_ASSERTM( false, mpMemMetrics->mHelpfulHint );
        }
    }
}

/*F************************************************************************************************/
/*!
    \Function    AptActionQueueC::AddFunctionBack

    \Description
        Adds an item to the action pool

        // The deque can get exhausted - this can happen in release builds as well (of course).
        // We must therefore deal with it ALWAYS.

    \Input      pContext          - context in which function will be called
    \Input      pFuncDef          - actual function definition
    \Input      nParams           - number of parameter to the function
    \Input      input             - any AptInput associated with it.

    \Output
        None.

    \Notes
        None.

    \Version    1.0        10/27/04 (LEO) First Version

*/
/************************************************************************************************F*/
void AptActionQueueC::AddFunctionBack(AptCIH* pContext, AptValue* pFuncDef, int nParams, ACTION_PROF_DECL_PARAM ACTION_TYPE_DECL_PARAM AptInput input)
{
    APT_ASSERT(pContext->getIsDefined());

    AptActionPool *pTempItem = IncrementDequeLocation(mpEndDeque);
    if(pTempItem != mpStartDeque)
        {
        mpEndDeque->eActionType = AAT_FUNCTION;
        mpEndDeque->input = input;
        mpEndDeque->function.pContext = pContext;
        APT_INC(mpEndDeque->function.pContext);
        mpEndDeque->function.pFuncDef = pFuncDef;
        APT_INC(mpEndDeque->function.pFuncDef);
        mpEndDeque->function.nParams = nParams;
#if APT_PROF_ENABLED && APT_PROFILE_ACTIONSCRIPT_ENABLED
        mpEndDeque->strFunction = strFunction;
#endif
#if defined(EA_DEBUG)
        mpEndDeque->eAptActionType = (AptActionType) eAptActionType ;
#endif
        mpEndDeque = pTempItem;
        }
    else
    {
        mpMemMetrics->mHighwaterMaxCount = INT32_MAX;
        int32_t nPolicy = AptHighwaterMemoryMetrics::sOverflowCallback->OnOverflowDetected( *mpMemMetrics, NULL, NULL );
        if ( nPolicy & (int32_t)AptHighwaterMemoryMetrics::POLICY_APT_WILL_ASSERT )
        {
            APT_ASSERTM( false, mpMemMetrics->mHelpfulHint );
        }
    }
}

/*F************************************************************************************************/
/*!
    \Function    AptActionQueueC::AddFunctionFront

    \Description
        Adds an item to the action pool

        // The deque can get exhausted - this can happen in release builds as well (of course).
        // We must therefore deal with it ALWAYS.

    \Input      pContext          - context in which function will be called
    \Input      pFuncDef          - actual function definition
    \Input      nParams           - number of parameter to the function
    \Input      input             - any AptInput associated with it.

    \Output
        None.

    \Notes
        None.

    \Version    1.0        10/27/04 (LEO) First Version

*/
/************************************************************************************************F*/
void AptActionQueueC::AddFunctionFront(AptCIH* pContext, AptValue* pFuncDef, int nParams, ACTION_PROF_DECL_PARAM ACTION_TYPE_DECL_PARAM AptInput input)
{
    APT_ASSERT(pContext->getIsDefined());

    AptActionPool *pTempItem = DecrementDequeLocation(mpStartDeque);
    if(pTempItem != mpEndDeque)
    {
        mpStartDeque = pTempItem;
        mpStartDeque->eActionType = AAT_FUNCTION;
        mpStartDeque->input = input;
        mpStartDeque->function.pContext = pContext;
        APT_INC(mpStartDeque->function.pContext);
        mpStartDeque->function.pFuncDef = pFuncDef;
        APT_INC(mpStartDeque->function.pFuncDef);
        mpStartDeque->function.nParams = nParams;
#if APT_PROF_ENABLED && APT_PROFILE_ACTIONSCRIPT_ENABLED
        mpStartDeque->strFunction = strFunction;
#endif
#if defined(EA_DEBUG)
        mpStartDeque->eAptActionType = (AptActionType) eAptActionType ;
#endif
    }
    else
    {
        mpMemMetrics->mHighwaterMaxCount = INT32_MAX;
        int32_t nPolicy = AptHighwaterMemoryMetrics::sOverflowCallback->OnOverflowDetected( *mpMemMetrics, NULL, NULL );
        if ( nPolicy & (int32_t)AptHighwaterMemoryMetrics::POLICY_APT_WILL_ASSERT )
        {
            APT_ASSERTM( false, mpMemMetrics->mHelpfulHint );
        }
    }
}

/*F************************************************************************************************/
/*!
    \Function    AptActionQueueC::RemoveActionFor

    \Description
        Goes through the action queue and removes any pending actions for the pCIH passed in.

    \Input      *pCIH             - AptCIH for which actions are removed.

    \Output
        None.

    \Notes
        None.

    \Version    1.0        10/27/04 (LEO) First Version

*/
/************************************************************************************************F*/
void AptActionQueueC::RemoveActionFor(AptCIH *pCIH)
{
    AptActionPool *pCur = GetFirstItem();
    while (!IsLastItem(pCur))
    {
        if(pCur->eActionType == AAT_ACTION)
        {
            if (pCur->action.pCIH == pCIH)
            {
                if (pCur == GetCurItem()) // $bug846 - MCB - if pCur is the CIH that is in runActions, don't remove it. It will remove itself at the end of runActions.
                {
                    pCur = GetNextItem(pCur);
                    continue;
                }
                if (pCur < mpEndDeque)
                {
                    APT_DEC(pCIH);
                    memmove(pCur, pCur + 1, (mpEndDeque - pCur - 1)*sizeof(AptActionPool));
                    mpEndDeque = DecrementDequeLocation( mpEndDeque );
                    break;
                }
                else if (pCur > mpStartDeque)
                {
                    APT_DEC(pCIH);
                    memmove(mpStartDeque + 1, mpStartDeque, (pCur - mpStartDeque)*sizeof(AptActionPool));
                    mpStartDeque = IncrementDequeLocation( mpStartDeque );
                    break;
                }
                else if (pCur == mpStartDeque)
    {
                    mpStartDeque = IncrementDequeLocation( mpStartDeque );
                    break;
                }
                else
        {
                    APT_ASSERT(NOT_REACHED);
                }
            }
        }
        pCur = GetNextItem(pCur);
        }
}

/*F************************************************************************************************/
/*!
    Gets the deque size.

    \Input      None.

    \Output     int32_t           - The deque size.

    \Notes
        None.

    \Version
        1.0        03/22/04 (ONA) First Version

*/
/************************************************************************************************F*/
int32_t    AptActionQueueC::GetDequeSize() const
{
    int32_t iDelta = mpEndDeque - mpStartDeque;
    if (iDelta >= 0)
        {
        return  (iDelta);
        }

    //  Add the roll-over size
    iDelta = miActionPoolSize + iDelta;
    APT_ASSERT(iDelta > 0);             //  iDelta should be now positive
    return  (iDelta);
}

/*F************************************************************************************************/
/*!
    Gets the ActionPool pointer from a given index.
    The index is relative to the start of the deque.

    \Input      iIndex            - The index of the ActionPool.

    \Output     AptActionPool *   - The ActionPool pointer at the index.

    \Notes
        None.

    \Version
    1.0        03/22/04 (ONA) First Version

*/
/************************************************************************************************F*/
AptActionQueueC::AptActionPool *  AptActionQueueC::GetDequeLocation(const int32_t iIndex) const
{
    int32_t iOffset = mpStartDeque - maActionPool;
    APT_ASSERT((iOffset >= 0) && (iOffset < miActionPoolSize));
    iOffset += iIndex;
    iOffset %= miActionPoolSize;
    if (iOffset >= 0)
    {
        return  (&maActionPool[iOffset]);
    }
    iOffset += miActionPoolSize;
    return  (&maActionPool[iOffset]);
}

/*F************************************************************************************************/
/*!
    \Function    AptActionQueueC::RegisterReferences

    \Description
        Registers references for GC objects inside action queue.

    \Input
        None.

    \Output
        None.

    \Notes
        None.

    \Version    1.0        10/27/04 (LEO) First Version

*/
/************************************************************************************************F*/
void AptActionQueueC::RegisterReferences()
{
    int nLoopCount = GetDequeSize();
    for (int i = 0; i < nLoopCount; i++)
    {
        AptActionPool * pActionPool;
        pActionPool = GetDequeLocation(i);
        if (pActionPool->eActionType == AAT_ACTION)
        {
            APT_REGISTER_REFERENCE_ANONYMOUS(pActionPool->action.pCIH, "AptAnimationPoolData::action.pCIH", APT_REFREG_IS_APTCIH);
        }
        else if (pActionPool->eActionType == AAT_FUNCTION)
        {
            APT_REGISTER_REFERENCE_ANONYMOUS(pActionPool->function.pContext, "AptAnimationPoolData::function.pContext", APT_REFREG_IS_APTCIH);
            APT_REGISTER_REFERENCE_ANONYMOUS(pActionPool->function.pFuncDef, "AptAnimationPoolData::function.pFuncDef", APT_REFREG_IS_APTVALUE);
        }
#if defined (APT_ALTERNATE_INPUT)
        else if (pActionPool->eActionType == AAT_ALT_INP_FUNCTION)
        {
            APT_REGISTER_REFERENCE_ANONYMOUS(pActionPool->function.pContext, "AptAnimationPoolData::function.pContext", APT_REFREG_IS_APTCIH);
            APT_REGISTER_REFERENCE_ANONYMOUS(pActionPool->function.pFuncDef, "AptAnimationPoolData::function.pFuncDef", APT_REFREG_IS_APTVALUE);
        }
#endif
        else
        {
            APT_ASSERT(NOT_REACHED && "Encountered invalid pActionPool->eActionType");
        }
    }
}
