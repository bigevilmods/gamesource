/*H************************************************************************************************/
/*!

    \File
        AptCIH.cpp

    \Description
        Simple description of main purpose of file.

    \Notes
        None.

    \Copyright
        (c) 2005 Electronic Arts Inc.

    \Version
        1.0        03/07/04 (ONA) First Version
        2.0         04/1/05 (ASK) Changed AptCIH class for decoupling
                Lot of accessor functions are added that access RenderItem and then
                use its functions to get its members.
                Many functions which were inlined previously are moved to cpp file and are
                not inlined any more. This is just for getting those functions working.
                We will optize them later for inlining and may be later move them back to header file.
        2.1   08/09/06 (ASK)  Added AptCIHNativeFunctionHelper class for easier debugging.
                Only gets defined if APT_USE_CIH_NATIVEFUNC_HELPER is defined.
                Changed NATIVE_MEMBER_* macros calls to use APT_CIH_NATIVE_MEMBER_* macros.
                If APT_USE_CIH_NATIVEFUNC_HELPER is defined then Native functions get defined for
                AptCIHNativeFunctionHelper class and not AptCIH class.

                Also modified lots of function headers to have correct function names and correct comments.
        2.2     11/27/06 (ASK) Changed links to coreforge for external references to Apt Wiki page
        2.3     02/27/07 (JP)  Creating custom control render item only once
        2.4     04/27/07 (BSR) Fix custom control memory leak in ProcessCustomControls
        2.5     05/04/07 (RB)  Fix NaN values in get/set rotation, xscale and yscale.  Added assert not NaN.
        2.6     09/05/07 (RB)  Added back bugfixes removed during lockless Apt implementation.
        2.7     09/20/07 (ASK)  Added 2 new member variable bit fields to denote if CIH has blendModes or filters
        2.8     10/05/07 (ASK) Added blendmode and filter support
        2.9     11/15/07 (JP)  Setting force creation to false in CreateCustomControlWithZID
        2.10    01/31/08 (RB)  Avoid rev-mw warning about mixning int and int32_t. Declare GetBlendModeValue as static.
        2.11    03/05/08 (RB)  Removed unused static variable PI.
        2.12    04/01/08 (JP)  Adding cases for parseInt and parseFloat
        2.13    04/29/08 (RB)  In objectMemberLookup, changed the way filter property is cloned
        2.14    06/25/08 (RB)  In Unresolve, we want to be able to reset text.nFontID.
                               This ensures our legacy regression tests pass cleanly.
        2.15    07/29/08 (RB)  EATech 114985: Added ability to track AptHighwaterMemoryMetrics for zombie vector
        2.16    08/20/08 (BT)  Added Mike Meyer's getBounds and MultParentMatrix fixes.
        2.17    04/30/09 (RB)  Display class name and SWF filename in the ActionScript callstack when a class's ctor
                               is called from AptCIH::AssociateInstToClass.
        2.17    06/09/09 (DG)  FogBugz 1233: AptNativeHash change from Tiger10:
                               change in objectMemberSet method (see method header for detail).
        2.18    07/09/09 (DG)  1664: Added leading handling in several places

        2.19    07/10/09 (RB)  Made GetFirstChild() inline. Moved _visible to be first in GetProceduralProperty.
        2.20    09/28/09 (MZ)  Added tracking property-handling code
        2.21    11/10/09 (RB)  Handle _parent as a sprite property. Previously it was only considered to be global.
        2.22    11/11/09 (RB)  Removed setInterval and getInterval from MovieClip because they are members of _global.
*/
/************************************************************************************************H*/

#include <float.h>          // Need to include this because of problem with eabase vs native float types on ps3

#include "AptCIH.h"
#include "_AptSet.h"
#include "_Apt.h"
#include "AptTarget.h"
#include "AptRenderableGeometry.h"
#include "AptRenderableString.h"
#include "AptAnimationTarget.h"
#include "Display/AptDisplayListState.h"
#include "AptValue/AptValueVector.h"                    // $BUG253 LEO    14/47
#include "AptFrameStack.h"
#include "Display/AptRenderingContext.h"
#include "AptObject/AptTextFormat.h"
#include "AptObject/AptGlobalExtensionObject.h"
#include "AptObject/AptGlobalObject.h"
#include "TextFormat.h"
#include "AptTextMembers.h"
#include "AptSpriteMembersConsts.h"
#include "Display/AptRenderingContext.h"
#include "Display/AptDisplayListState.h"
#include "AptBCRenderTree.h"
#include "MainInline.h"
#include "AptGlobal.h"
#include "AptProfile.h"
#include "AptMemoryMetricsImpl.h"
#include "AptCallStack.h"
#include "AptDebugger/AptDebugger.h"
#include "AptDebugReport.h"

#if !defined(APT_ENABLE_INLINE)
#include "AptCIH.inl"
#endif

// Some platforms flip Z and various rotation properties.  Bottlenecking that all here to avoid #ifdefs everywhere
#if (defined(EA_PLATFORM_MICROSOFT) && defined(EA_PLATFORM_CONSOLE)) || (defined(EA_PLATFORM_SONY) && defined(EA_PLATFORM_CONSOLE)) || defined(EA_PLATFORM_WINDOWS) || defined(EA_PLATFORM_PS3) // EA_PLATFORM_SONY not defined on PS3 until EABase 2.00.29
#define APTCIH_Z_MULTIPLIER   (1)
#else
#error "Unsupported Platform"
#endif

APT_DEFINE_OBJECT_METRICS(AptCIH, AptValueGC);
uint32_t  (*AptCIH::sCIHProcessCb) (AptCIH * pCIH, void * pVoid)= NULL;
uint32_t  (*AptCIH::sCIHProcessCb1) (AptCIH * pCIH, void * pVoid)= NULL;
uint32_t  (*AptCIH::sCIHProcessCb2) (AptCIH * pCIH, void * pVoid) = NULL;
#if defined(APT_USE_BUTTONS)
uint32_t  (*AptCIH::sCIHButtonProcessCb) (AptCIH * pCIH, void * pVoid) = NULL;
#endif
bool AptCIH::bEarlyReturn = true;       // default is set to true as we do want to return early
int32_t AptCIH::nTreeDepth = 0 ;

#if defined(APT_ENABLE_DEBUG_REPORT)
EA::Apt::Debug::AptReport* AptCIH::mReport = NULL;
#endif

int AptCIH::sEmptyAssetString = -1;
#define TODEGREES           57.2957795f
#define TORADIANS           0.01745329f
#define FLASH_SCALE_LIMIT   1.13225728740063f

// XXX more events has to added to this like rollover, dragout etc.
// removed static because this is accessed from other files.
// hardcoded the array size as it was getting difficult to access it from other files.
//static ClipEventType _aClipEvents[] =
ClipEventType _aClipEvents[] =
{
    { AptEventActionFlag_EnterFrame,        SC_onEnterFrame },
#if defined(APT_USE_MOUSE)
    { AptEventActionFlag_Press,             SC_onPress },
    { AptEventActionFlag_Release,           SC_onRelease },
    { AptEventActionFlag_ReleaseOutside,    SC_onReleaseOutside },
    { AptEventActionFlag_MouseDown,         SC_onMouseDown },
    { AptEventActionFlag_MouseUp,           SC_onMouseUp },
    { AptEventActionFlag_MouseMove,         SC_onMouseMove },
#endif
    { AptEventActionFlag_KeyUp,             SC_onKeyUp },
    { AptEventActionFlag_KeyDown,           SC_onKeyDown },
    { AptEventActionFlag_OnLoad,            SC_onLoad },
    { AptEventActionFlag_Unload,            SC_onUnload },
    { AptEventActionFlag_Data,              SC_onData },
#if defined(APT_USE_MOUSE)
    { AptEventActionFlag_RollOver,          SC_onRollOver },
    { AptEventActionFlag_RollOut,           SC_onRollOut },
    { AptEventActionFlag_DragOver,          SC_onDragOver },
    { AptEventActionFlag_DragOut,           SC_onDragOut },
    { AptEventActionFlag_Wheel,             SC_onMouseWheel},               // F#563    6
#endif
};
#if defined(APT_USE_MOUSE)
#define APTACTION_CLIP_EVENT_ARRAY_SIZE   17
#else
#define APTACTION_CLIP_EVENT_ARRAY_SIZE   6
#endif

// assert for not-a-number float values in AptCIH matrix
#if defined(EA_PLATFORM_XENON) || defined(EA_PLATFORM_WINDOWS) || (defined(EA_PLATFORM_MICROSOFT) && defined(EA_PLATFORM_CONSOLE))
#   define APT_ASSERT_NOT_NAN( x )      APT_ASSERT( !_isnan((x)) && "AptCIH: NAN result" )
#elif defined(EA_PLATFORM_PS3)
#   define APT_ASSERT_NOT_NAN( x )      APT_ASSERT( !isnan((x))  && "AptCIH: NAN result" )
#else
#   define APT_ASSERT_NOT_NAN( x )
#endif
#define    APT_ASSERT_NOT_NAN_MATRIX( m )  APT_ASSERT_NOT_NAN( ((m)->a) );  APT_ASSERT_NOT_NAN( ((m)->b) ); \
                                           APT_ASSERT_NOT_NAN( ((m)->c) );  APT_ASSERT_NOT_NAN( ((m)->d) );

static inline AptAssetString GetZID(AptCharacterTextInst *pText)
{
    AptRenderableString *pRenderable = pText ? pText->GetRenderable() : 0;
    return pRenderable ? pRenderable->mString : 0;
}

extern AptCallStack *gAptOptCallStack;

/*F************************************************************************************************/
/*!
    \Function    AptCIH::~AptCIH

    \Description
        Destructor

    \Output

    \Notes
        None.

    \Version    1.0        08/09/06 (ASK) First Version

*/
/************************************************************************************************F*/
AptCIH::~AptCIH()
{
    APT_ASSERT(mpCharacterInst == NULL);

#if defined(EA_DEBUG)
    int32_t     nDelayedListSize = AptAnimationTarget::GetDelayedReleaseListSize();
    AptCIH  **  pDelayedReleaseList = AptAnimationTarget::GetDelayedReleaseList();
    for(int32_t i = 0; i < nDelayedListSize; i++)
    {
        if(pDelayedReleaseList[i] == this)
        {
            APT_ASSERT(0 && "Deleting a CIH with references!");
        }
    }
#endif

    if(fRot)
    {
        APT_FREE_ARRAY(fRot, float, 1);
    }
    mpPrev = NULL;
    mpNext = NULL;
    mpParent = NULL;
    fRot = NULL;
}


/*F************************************************************************************************/
/*!
    \Function    AptCIH::_gotoAndX

    \Description
        Acts as the gotoFrame instruction in actionscript.

        This function sets the frame number and state of the sprite for a GotoAndPlay/GotoAndStop call in actionscript.

    \Output     AptValue * - returns gpUndefinedValue

    \Notes
        None.

    \Version    1.0        05/04/04 (LEO) First Version

*/
/************************************************************************************************F*/

AptValue *AptCIH::_gotoAndX(AptValue *pThis, int nParams, int bPlay)
{
    if (nParams < 1) return gpUndefinedValue;
    AptValue *pFrame = gAptActionInterpreter.stackAt(0);
    int nFrame;

    if (pThis->c_cih()->IsLevelInst()) return gpUndefinedValue;

    if (pFrame->isString())
    {
       nFrame = pThis->c_cih()->GetSpriteInstBase()->GetCharacterConst()->sprite.movie.labelToFrame(pFrame->c_string()->GetInternalString()) + 1;
    }
    else
    {
        nFrame = pFrame->toInteger();
    }

    //if(pThis->c_cih()->GetSpriteInstBase()->bJustLoaded == 1 && pThis->c_cih()->GetSpriteInstBase()->nFrame == -1)    // (LEO) $bug881 gotoAndPlay of newly created MC crashes Apt
    //{
    //    pThis->c_cih()->tick();
    //}

    // ### Added if condition for gotoAndPlay frame < 0, or to a label which isn't defined, it would reset mbIsPlaying to true... bad!
    if(nFrame - 1 >= 0)
    {
        pThis->c_cih()->jumpToFrame(nFrame - 1);
        pThis->c_cih()->SetIsPlaying(bPlay ? 1 : 0);
    }
    return gpUndefinedValue;
}

/*F************************************************************************************************/
/*!
    \Function    AptCIH::setTextFormat

    \Description
    For TextFields, this will take a TextFormat object and copy the desired setting to the TextField.

    \Output

    \Notes
    We only create a TextFormat object in the AptCharacterTextInst when setTextFormat is called, otherwise we don't need one.

    \Version    1.0        05/04/04 (LEO) First Version
    \Version    1.1        10/00/07 (JP)  Updating the FontID
    \Version    1.2        06/25/08 (RB)  Don't hardcode FontID to -1. Instead set as -FontID.
*/
/************************************************************************************************F*/
APT_CIH_NATIVE_MEMBER_FUNCTION(AptCIH, setTextFormat)
{
    if (nParams > 3)
    {
        return gpUndefinedValue;
    }
    AptValue *pParam = gAptActionInterpreter.stackAt(0);

    if (pParam->getIsDefined() && pParam->isTextFormat())
    {
        AptCharacterTextInst *pTextInst = pThis->c_cih()->GetDynamicTextInst();
        AptTextFormat *pTextFormat = pParam->c_textformat();

		// The code in this function only makes sense if the AptRenderItem is castable to AptRenderItemDynamicText (due to calling GetTextFormatConst),
		// so we'll skip all the logic below in all other cases. This prevents a crash that can occur when calling setTextFormat from action script code,
		// even when the function is not defined in the object.
		if (pTextInst->GetRenderItem()->GetCharacterType() != AptCharacterType_Text)
		{
			return gpUndefinedValue;
		}

        // $FR#598 CSB - Created Temporary FontStyle to keep track of pMyTextFormat->nFontStyle before it gets overwritten
        unsigned int nTempFontStyle;

        // Create a new TextFormat Obj, or copy it to the text inst
        if(pTextInst->GetTextFormatConst() == NULL)
        {
			pTextInst->SetTextFormat(new TextFormat(pTextFormat->GetTextFormat()));
			nTempFontStyle = pTextInst->GetTextFormatConst()->nFontStyle | pTextFormat->GetTextFormat()->nFontStyle;
        }
        else
        {
			nTempFontStyle = pTextInst->GetTextFormatConst()->nFontStyle | pTextFormat->GetTextFormat()->nFontStyle;
			pTextInst->GetTextFormatWritable()->copyTextFormatObj(pTextFormat->GetTextFormat());
        }

		pTextInst->GetTextFormatWritable()->nFontStyle = nTempFontStyle;

        // Now go look for the new font
        // (LEO) updated fix for Bug509
        if(pTextFormat->GetTextFormat()->pFontName.IsEmpty() == false)
        {
            // Update the font name in the text format object so we can pass it back to the aux libs.
            pTextInst->GetTextFormatWritable()->pFontName = pTextFormat->GetTextFormat()->pFontName;
        }

        // Now copy other info over to the TextInst
        if(pTextFormat->GetTextFormat()->nColor != -1)
        {
            if(pTextInst->GetTextColor() != pTextFormat->GetTextFormat()->nColor)
            {
                pTextInst->SetTextColor(pTextFormat->GetTextFormat()->nColor);
            pTextInst->ClearStateFlags(APT_TEXTFIELD_NONE);
                pTextInst->SetStateFlags( APT_TEXTFIELD_FONTSIZE | APT_TEXTFIELD_DIRTY | APT_TEXTFIELD_TEXTCOLOR);
            }
        }
        if(pTextFormat->GetTextFormat()->fSize != -1)
        {
            if(pTextFormat->GetTextFormat()->fSize <= 0)                  // (LEO) if the font size is zero, set it to one.
            {
                pTextFormat->GetTextFormat()->fSize = 1.f;
            }

            if(pTextInst->GetFontSize() != pTextFormat->GetTextFormat()->fSize)
            {
            pTextInst->SetFontSize( pTextFormat->GetTextFormat()->fSize );
            pTextInst->ClearStateFlags(APT_TEXTFIELD_NONE);
                pTextInst->SetStateFlags(APT_TEXTFIELD_FONTSIZE | APT_TEXTFIELD_DIRTY | APT_TEXTFIELD_TEXTWIDTH);
        }
        }
        if(pTextFormat->GetTextFormat()->eAlignment != AptStringAlignment_None)
        {
            if(pTextInst->GetAlignment() != pTextFormat->GetTextFormat()->eAlignment)
            {
            pTextInst->SetAlignment( pTextFormat->GetTextFormat()->eAlignment );
            pTextInst->ClearStateFlags(APT_TEXTFIELD_NONE);
                pTextInst->SetStateFlags(APT_TEXTFIELD_ALIGNCHANGE | APT_TEXTFIELD_DIRTY | APT_TEXTFIELD_TEXTWIDTH);
        }
        }

        /*
            only wipe the font if we were passed in a new font name, this fixes the issue where actionscript
            uses the setTextFormat call to change properties other then the font name.
        */
        if(pTextFormat->GetTextFormat()->pFontName.IsEmpty() == false)
        {
        // Avoiding that getTextFormat recover the FontName using the ID
        int nFontID = pTextInst->GetCharacterConst()->text.nFontID;
        if ( nFontID > 0 )
        {
            pTextInst->GetCharacterWritable()->text.nFontID = -nFontID;
        }
        }

        if (pTextFormat->GetTextFormat()->nLeading != TextFormat::UNDEFINED_LEADING_VALUE)
        {
            pTextInst->SetLeading(pTextFormat->GetTextFormat()->nLeading);
        }

        if (pTextFormat->GetTextFormat()->nTracking != TextFormat::UNDEFINED_TRACKING_VALUE)
        {
            pTextInst->SetTracking(pTextFormat->GetTextFormat()->nTracking);
        }
    }
    return gpUndefinedValue;
}


/*F************************************************************************************************/
/*!
    \Function    AptCIH::SetCharacterInst

    \Description
        Sets the characterinst

    \Input      pData             - AptCharacterInst * to be set.

    \Output

    \Notes
        None.

    \Version    1.0        04/01/05 (ASK) First Version
    \Version    1.1        07/06/05 (DC)  $bug941 - Fixed bug with having deleted pointer by setting
                                          the new mpCharacterInst before deleting the old one!

*/
/************************************************************************************************F*/
void        AptCIH::SetCharacterInst(AptCharacterInst * pData, bool bMoveDataFromCurrent)
{
    if (pData == NULL)      // lets create level inst for this.
    {
        pData = AptCharacterInst::CreateCharacterInst(NULL) ;
    }

    APT_ASSERT(pData != NULL);

    if(pData != mpCharacterInst)
    {
        // DC - Set the New one before deleting the old one!
        AptCharacterInst * pOldCharInst = mpCharacterInst;
        mpCharacterInst = pData;

        if(pOldCharInst != NULL)
        {
            // If we are going to or from a level inst, store off render info.
            if(bMoveDataFromCurrent)
                mpCharacterInst->MoveRenderDataFrom(pOldCharInst);

            // $bug1012 - Timers not cleared when movie is unloaded via LoadMovie.
            if (IsAnimationInst())
            {
                GetTargetSim()->GetAnimationTarget()->RemoveTimerFunctions(this);
            } // end of bug fix

            pOldCharInst->DestroyGCPointers();
            delete pOldCharInst;
        }

        // the new character inst (and it's render item) need to be updated.
        AptCharacterInst::ItemMoved(this);
    }
}


/*F************************************************************************************************/
/*!
    \Function    AptCIH::getNewTextFormat

    \Description
    For TextFields, this will copy the TextFormat object and return it.

    \Version    1.0        05/04/04 (LEO) First Version
    \Version    1.1        07/10/09 (DG) added set for leading attribute
    \Version    1.2        09/28/09 (MZ) added set for tracking attribute
*/
/************************************************************************************************F*/
APT_CIH_NATIVE_MEMBER_FUNCTION(AptCIH, getNewTextFormat)
{
    if (nParams > 0)
    {
        return gpUndefinedValue;
    }

    AptTextFormat *pTextFormatObj;

    // $FR#598 CSB - Change for extra parameter for Font Style 11/36
    if(pThis->c_cih()->GetDynamicTextInst()->GetTextFormatConst() == NULL)
    {
        pThis->c_cih()->GetDynamicTextInst()->SetTextFormat( new TextFormat(gpUndefinedValue, -1.0f, 0xFFFFFFFFu, -1, -1, -1, 0, 0, gpUndefinedValue, -1, -1, -1, TextFormat::UNDEFINED_LEADING_VALUE, TextFormat::UNDEFINED_TRACKING_VALUE) );
    }

    pTextFormatObj = new AptTextFormat(pThis->c_cih()->GetDynamicTextInst()->GetTextFormatConst());

    if(pTextFormatObj->GetTextFormat()->nColor == -1)
    {
        pTextFormatObj->GetTextFormat()->nColor = pThis->c_cih()->GetDynamicTextInst()->GetTextColor();
    }

    AptCharacterAnimation *pAnim = &pThis->c_cih()->GetCharacterInst()->GetCharacterConst()->pParentAnim->animation;

    // $FR#598 CSB - Fix how getNewTextFormat handles getting font names 12/36
    if ((pThis->c_cih()->GetDynamicTextInst()->GetCharacterConst()->text.nFontID < pAnim->nCharacters) &&
        (pThis->c_cih()->GetDynamicTextInst()->GetCharacterConst()->text.nFontID >= 0 ) &&
        (pAnim->apCharacters[pThis->c_cih()->GetDynamicTextInst()->GetCharacterConst()->text.nFontID]->eType == AptCharacterType_Font))
    {
        pTextFormatObj->GetTextFormat()->pFontName = pAnim->apCharacters[pThis->c_cih()->GetDynamicTextInst()->GetCharacterConst()->text.nFontID]->font.szName;
    }
    else
    {
        pTextFormatObj->GetTextFormat()->pFontName = "";
    }

    pTextFormatObj->GetTextFormat()->eAlignment = pThis->c_cih()->GetDynamicTextInst()->GetAlignment();
    pTextFormatObj->GetTextFormat()->fSize      = pThis->c_cih()->GetDynamicTextInst()->GetFontSize();
    pTextFormatObj->GetTextFormat()->nLeading   = pThis->c_cih()->GetDynamicTextInst()->GetLeading();
    pTextFormatObj->GetTextFormat()->nTracking  = pThis->c_cih()->GetDynamicTextInst()->GetTracking();
    return pTextFormatObj;
}

/*F************************************************************************************************/
/*!
    \Function    AptCIH::getTextFormat

    \Description
    Gets the textformat object

    \Notes
    -$FR#598 CSB - Added support for getTextFormat (i.e. to retrieve Font Style) 14/40
    -(LEO) $bug860 TextFormat issues

    \Version    1.0        03/10/05 (LEO) First Version
    \Version    1.1        03/24/05 (LEO) $bug887 setTextFormat() with RGB(255, 255, 255) does not work in 18.06.
    \Version    1.2        07/10/09 (DG) added leading attribute handling
    \Version    1.3        09/28/09 (MZ) added tracking attribute handling

*/
/************************************************************************************************F*/
APT_CIH_NATIVE_MEMBER_FUNCTION(AptCIH, getTextFormat)
{
    if (nParams > 2)
    {
        return gpUndefinedValue;
    }

    AptTextFormat *pTextFormatObj = new AptTextFormat(gpUndefinedValue, 0.0f, 0xFFFFFFFFu, false, false, false, 0, 0, AptBoolean::Create(true), 0, 0, 0, 0, 0);

    if(pThis->c_cih()->GetDynamicTextInst()->GetTextFormatConst() == NULL)
    {
        pThis->c_cih()->GetDynamicTextInst()->SetTextFormat( new TextFormat(gpUndefinedValue, -1.0f, 0xFFFFFFFFu, -1, -1, -1, 0, 0, gpUndefinedValue, -1, -1, -1, -1, -1) );
    }

    pTextFormatObj->GetTextFormat()->copyTextFormatObj(pThis->c_cih()->GetDynamicTextInst()->GetTextFormatConst());
    TextFormat *texFormat = pTextFormatObj->GetTextFormat();

    if ((texFormat->nFontStyle & AptFontStyle_isItalicSet) != AptFontStyle_isItalicSet)
    {
        texFormat->nFontStyle |= AptFontStyle_isItalicSet;
    }
    if ((texFormat->nFontStyle & AptFontStyle_isUnderlineSet) != AptFontStyle_isUnderlineSet)
    {
        texFormat->nFontStyle |= AptFontStyle_isUnderlineSet;
    }
    if ((texFormat->nFontStyle & AptFontStyle_isBoldSet) != AptFontStyle_isBoldSet)
    {
        texFormat->nFontStyle |= AptFontStyle_isBoldSet;
    }

    if(texFormat->nColor == -1)
    {
        texFormat->nColor = pThis->c_cih()->GetDynamicTextInst()->GetTextColor() & 0xffffff;
    }

    AptCharacterAnimation *pAnim = &pThis->c_cih()->GetCharacterInst()->GetCharacterConst()->pParentAnim->animation;

    if ((pThis->c_cih()->GetDynamicTextInst()->GetCharacterConst()->text.nFontID < pAnim->nCharacters) &&
        (pThis->c_cih()->GetDynamicTextInst()->GetCharacterConst()->text.nFontID >= 0) &&
        (pAnim->apCharacters[pThis->c_cih()->GetDynamicTextInst()->GetCharacterConst()->text.nFontID]->eType == AptCharacterType_Font))
    {
        texFormat->pFontName = pAnim->apCharacters[pThis->c_cih()->GetDynamicTextInst()->GetCharacterConst()->text.nFontID]->font.szName;
    }

    texFormat->eAlignment = pThis->c_cih()->GetDynamicTextInst()->GetAlignment();
    texFormat->fSize      = pThis->c_cih()->GetDynamicTextInst()->GetFontSize();
    texFormat->nLeading  = pThis->c_cih()->GetDynamicTextInst()->GetLeading();
    texFormat->nTracking  = pThis->c_cih()->GetDynamicTextInst()->GetTracking();
    return pTextFormatObj;
}

/*F************************************************************************************************/
/*!
    \Function    AptCIH::localToGlobal

    \Description
    Converts the movieclip coordinates from local to global space and sets them in AptObject that is
    passed on stack.

    \Notes
        $FR#586 DG - Added support for localToGlobal (i.e. to conversion from local to global co-ordinates)

    \Version    1.0        10.11.2004 (DG) First Version
                1.1        11/21/07 (JP) Checking that pX and pY are not NULL
*/
/************************************************************************************************F*/
APT_CIH_NATIVE_MEMBER_FUNCTION(AptCIH, localToGlobal)  //DG 10.11.2004 FR586
{
    APT_ASSERT(nParams < 2);                //only 1 parameter

    if(nParams > 0)
    {
        AptValue *pParam = gAptActionInterpreter.stackAt(0);    // Get parameter from stack

        if (!pParam->ContainsNativeHashVirtual())
        {
            return gpUndefinedValue;
        }
        AptNativeHash *pHash = pParam->GetNativeHashVirtual();

        AptNativeString xCoordinate = "x", yCoordinate = "y";

        AptValue * pX = pHash->Lookup(&xCoordinate) ;
        AptValue * pY = pHash->Lookup(&yCoordinate) ;

        if(pX != NULL && pY != NULL)
        {
            // This is the matrix for the object passed in, in local space
            AptMatrix coords = gIdentityMatrix;
            coords.tx = pX->toFloat();
            coords.ty = pY->toFloat();

            // Take the current object and figure out what the matrix is to go to global space
            AptMatrix localToGlobalMat = gIdentityMatrix;
            pThis->c_cih()->MultParentMatrix(pThis->c_cih(), localToGlobalMat);
            gpRenderingContext->multMatrix(&localToGlobalMat, &coords, &coords);

            // Set the global coords in the return object
            pX = AptFloat::Create(coords.tx);
            pHash->Set(&xCoordinate, pX);
            pY = AptFloat::Create(coords.ty);
            pHash->Set(&yCoordinate, pY);
        }
    }
    return gpUndefinedValue;
}

/*F************************************************************************************************/
/*!
    \Function    AptCIH::globalToLocal

    \Description
    Converts x, y attributes of an object from global to local space

    \Notes

    \Version    1.0        03.03.2008 (JP) First Version
*/
/************************************************************************************************F*/
APT_CIH_NATIVE_MEMBER_FUNCTION(AptCIH, globalToLocal)
    {
    APT_ASSERT(nParams < 2);                //only 1 parameter

    if(nParams > 0)
    {
        AptValue *pParam = gAptActionInterpreter.stackAt(0);    // Get parameter from stack

        if (!pParam->ContainsNativeHashVirtual())
        {
            return gpUndefinedValue;
        }
        AptNativeHash *pHash = pParam->GetNativeHashVirtual();

        AptNativeString xCoordinate = "x", yCoordinate = "y";

        AptValue * pX = pHash->Lookup(&xCoordinate) ;
        AptValue * pY = pHash->Lookup(&yCoordinate) ;

        if(pX != NULL && pY != NULL)
        {
            // This is the matrix for the object passed in, in global space
            AptMatrix coords = gIdentityMatrix;
            coords.tx = pX->toFloat();
            coords.ty = pY->toFloat();

            // Take the current object and figure out what the matrix would be to go to global space
            AptMatrix localToGlobalMat = gIdentityMatrix;
            pThis->c_cih()->MultParentMatrix(pThis->c_cih(), localToGlobalMat);

            // Invert that matrix to go from global to local space.  It's theoretically
            // possible this matrix could be noninvertible, but not for normal MovieClips
            // (apparently if _width = 0 somewhere, that would be bad).
            AptMatrix globalToLocalMat;
            bool hasInverse = globalToLocalMat.AptMatrixInverse(&localToGlobalMat);
            APT_ASSERT(hasInverse);
            if (hasInverse)
            {
                // Multiple by the passed-in coords and we've got our local coords
                gpRenderingContext->multMatrix(&globalToLocalMat, &coords, &coords);
            }

            // Set the local coords in the return object
            pX = AptFloat::Create(coords.tx);
            pHash->Set(&xCoordinate, pX);
            pY = AptFloat::Create(coords.ty);
            pHash->Set(&yCoordinate, pY);
        }
    }
    return gpUndefinedValue;
    }

APT_CIH_NATIVE_MEMBER_FUNCTION(AptCIH, gotoAndStop)
{
    return AptCIH::_gotoAndX(pThis, nParams, 0);
}

APT_CIH_NATIVE_MEMBER_FUNCTION(AptCIH, gotoAndPlay)
{
    return AptCIH::_gotoAndX(pThis, nParams, 1);
}


/*F************************************************************************************************/
/*!
    \Function    findCharacterInLibrary

    \Description
        Finds the movie to attach in the movies library.

        When a attachMovie is called, this function recursively looks in all the exported animation instances until it finds it.  The strange thing is,
        it only checks one level deep for imported items?

    \Output     NO RETURN         -

    \Notes
        None.

    \Version    1.0        05/04/04 (LEO) First Version
    \Version    1.1        06/03/05 (DC)  Made sure that the file pointers are setup on library items.
    \Version    1.2        09/05/07 (RB)  Added back bugfix removed during lockless Apt implementation.
                                          The bugfix used to be wrapped by #ifdef APT_DECOUPLED_RENDERING.

*/
/************************************************************************************************F*/

inline AptCharacter *findCharacterInLibrary(AptCIH *pThis, AptNativeString *sIDName, bool bSearchImports)
{
    // This function searches through the animations for a movie symbol, if not found locally, then it searches the parent.
    //  This was added for bug 374
    int i;
    const AptCharacter * pCharacter = pThis->GetCharacterInst()->GetCharacterConst();
    if (NULL != pCharacter->pParentAnim)
    {
    AptCharacterAnimation *pAnim = &pCharacter->pParentAnim->animation;      // Get parent Animation
    for (i = 0; i < pAnim->nExports; i++)                                                               // Search export items
    {
        if (sIDName->EqualNoCase(pAnim->aExports[i].szName))
        {
            AptCharacter * pRetValue = pAnim->apCharacters[pAnim->aExports[i].nID];
            if(pRetValue->m_pAnimFile.pData == NULL)
            {
                pRetValue->m_pAnimFile = pCharacter->m_pAnimFile;
            }
            return pRetValue;                                         // item found, return
        }
    }

    if(bSearchImports)
    {
        // DC 2004/4/22 -- Added search of imports. This is needed to attach symbols from
        // imported libraries. Bug #457
        for (i = 0; i < pAnim->nImports; i++)
        {
            if (sIDName->EqualNoCase(pAnim->aImports[i].szName))
            {
                return pAnim->apCharacters[pAnim->aImports[i].nID];
            }
        }
    }
    }

    AptCIH *pThispParent = pThis->c_cih ()->GetDisplayListParent();
    if(pThispParent)                                                            // If parent animation is valid, search it
    {
        // only search imports on first level, not parent level
        return findCharacterInLibrary(pThispParent, sIDName, true);            // Recursion time
    }
    return NULL;                                                                // Nothing found, return NULL
}

/*F************************************************************************************************/
/*!
    \Function    AptCIH::attachMovie

    \Description
        The corresponding MovieClip.attachMovie method from Flash.

    \Notes
        None.

    \Version    1.0        05/04/04 (LEO) First Version
    \Version    1.1        01/13/09 (JP)  Uncommenting the pNewCharacter assert

*/
/************************************************************************************************F*/

APT_CIH_NATIVE_MEMBER_FUNCTION(AptCIH, attachMovie)
{
    AptValue *pIDName = gAptActionInterpreter.stackAt(0);
    AptValue *pTargetName = gAptActionInterpreter.stackAt(1);
    AptValue *pDepth = gAptActionInterpreter.stackAt(2);
    AptValue *pInitObject = (nParams >= 4) ? gAptActionInterpreter.stackAt(3) : NULL ;



    AptNativeString sIDName;
    pIDName->toString(sIDName);

    AptCharacter *pNewCharacter = 0;
    AptCIH *pCurCIH = pThis->c_cih();
    if(pCurCIH->GetSpriteInstBase()->GetCreatedDynamic() == true)
    {
        pCurCIH = pCurCIH->GetDisplayListParent();
    }
    pNewCharacter = findCharacterInLibrary(pCurCIH, &sIDName, true);     // This was added for bug 374, search for character

#if defined(EA_DEBUG)
    if (!pNewCharacter)
    {
        APT_DEBUGPRINT(APT_DEBUG_MSG_DEBUG_LVL, "Trying to attachMovie: '%s' could not be found.\n", sIDName.ConstRawPtr());
    }
#endif
    APT_ASSERT(pNewCharacter);   // This was commented for a long time. We were asked to uncomment it, considering that it's a useful assert

    if(pNewCharacter)
    {
        AptNativeString sBuf;
        pTargetName->toString(sBuf);
        AptCIH *pParentCIH = pThis->c_cih();

        AptCIH * pInserted = pParentCIH->InsertChild(pThis->c_cih(), pNewCharacter, pDepth->toInteger() + AptDisplayList::BASE_MOVIE_DEPTH, &sBuf, pInitObject);
        GetTargetSim()->GetAnimationTarget()->TickNewInsts();


        if(pInserted)
        {
            return pInserted;
        }

#if defined(EA_DEBUG)
        APT_DEBUGPRINT(APT_DEBUG_MSG_DEBUG_LVL, "MovieClip instance not found after Attach Movie\n");
        APT_DEBUGPRINT(APT_DEBUG_MSG_DEBUG_LVL, "Attaching: Item \'%s\' as \'%s\'.\n", sIDName.c_str(),sBuf.c_str() );
        APT_ASSERT(false && "attachMovie Failed see TTL for more info." );
#endif
    }

    return gpUndefinedValue;
}
/*F************************************************************************************************/
/*!
    \Function    AptCIH::loadMovie

    \Description
         The corresponding MovieClip.loadMovie method from Flash.

    \Output

    \Notes
        None.

    \Version    1.0        05/04/04 (LEO) First Version
                1.1        03/18/05 (LEO) $bug876 Cannot load .SWF files - 0.18.05

*/
/************************************************************************************************F*/
APT_CIH_NATIVE_MEMBER_FUNCTION(AptCIH, loadMovie)
{
    AptValue *pNewMovie = gAptActionInterpreter.stackAt(0);
    //AptValue *pMethod = gAptActionInterpreter.stackAt(1);             // Not supported yet
    APT_ASSERT(nParams == 1);

    AptNativeString sMovieName;
    pNewMovie->toString(sMovieName);
    AptNativeString sBuf = sMovieName;

    // try to convert to a full path if possible
    AptActionInterpreter::getName(pThis->c_cih(), sMovieName);

    GetTargetSim()->GetLinker()->Load(sBuf, sMovieName);

    return gpUndefinedValue;
}

/*F************************************************************************************************/
/*!
    \Function   AptCIH::unloadMovie

    \Description
         The corresponding MovieClip.unloadMovie method from Flash.

    \Output

    \Notes
        - (LEO) The original code would remove the unloaded movieclip from the display list which was incorrect.  We now
        simply unload the movieclip and leave it in the display list.

    \Version    1.0        05/04/04 (LEO) First Version
*/
/************************************************************************************************F*/
APT_CIH_NATIVE_MEMBER_FUNCTION(AptCIH, unloadMovie)
{
    APT_ASSERT(nParams == 0);
    AptCIH *pCIH = pThis->c_cih();

    AptNativeString sBuf;               // (LEO) $BUG670    5/5
    AptActionInterpreter::getName(pCIH, sBuf);
    GetTargetSim()->GetLinker()->Load(AptNativeString(""), sBuf);

    return gpUndefinedValue;
}

/*F************************************************************************************************/
/*!
    \Function    AptCIH::loadMovieNum

    \Description
         The corresponding MovieClip.loadMovieNum method from Flash.

    \Output

    \Notes
        None.

    \Version    1.0        08/25/08 ($senzee) First Version

*/
/************************************************************************************************F*/
APT_CIH_NATIVE_MEMBER_FUNCTION(AptCIH, loadMovieNum)
{
    APT_ASSERT(nParams >= 2);

    AptValue *pNewMovie = gAptActionInterpreter.stackAt(0);
    AptValue *pLevel    = gAptActionInterpreter.stackAt(1);
  //AptValue *pMethod   = gAptActionInterpreter.stackAt(2);             // Not supported yet (or probably ever)

    int32_t nLevelInt = pLevel ? pLevel->toInteger() : -1;
    APT_ASSERT( nLevelInt >= 0 && nLevelInt <= 31 );
    if ( !(nLevelInt >= 0 && nLevelInt <= 31) )
    {
        return gpUndefinedValue;
    }

    AptNativeString sMovieName;
    pNewMovie->toString(sMovieName);

    AptNativeString sBuf = sMovieName;

    // try to convert to a full path if possible
    AptActionInterpreter::getName(pThis->c_cih(), sMovieName);

    char8_t level[64];
    sprintf(level, "_level%u", nLevelInt);
    AptLoadAnimation(sBuf.c_str(), level);

    return gpUndefinedValue;
}

/*F************************************************************************************************/
/*!
    \Function   AptCIH::unloadMovieNum

    \Description
         The corresponding MovieClip.unloadMovieNum method from Flash.

    \Output

    \Version    1.0        08/25/08 ($senzee) First Version
*/
/************************************************************************************************F*/
APT_CIH_NATIVE_MEMBER_FUNCTION(AptCIH, unloadMovieNum)
{
    APT_ASSERT(nParams >= 1);
    AptValue *pLevel = gAptActionInterpreter.stackAt(0);
    int32_t nLevelInt = pLevel ? pLevel->toInteger() : -1;
    APT_ASSERT( nLevelInt >= 0 && nLevelInt <= 31 );
    if ( !(nLevelInt >= 0 && nLevelInt <= 31) )
    {
        return gpUndefinedValue;
    }

    char8_t level[64];
    sprintf(level, "_level%u", nLevelInt);
    AptLoadAnimation("", level);

    return gpUndefinedValue;
}

/*F************************************************************************************************/
/*!
    \Function    AptCIH::duplicateMovieClip

    \Description
        The corresponding MovieClip.duplicateMovieClip method from Flash.

    \Output     AptValue * - Reference to newly created movie clip

    \Notes
        None.

    \Version    1.0        05/04/04 (LEO) First Version
*/
/************************************************************************************************F*/
APT_CIH_NATIVE_MEMBER_FUNCTION(AptCIH, duplicateMovieClip)
{
    AptValue *pTarget = gAptActionInterpreter.stackAt(0);
    AptValue *pDepth = gAptActionInterpreter.stackAt(1);
    AptValue *pInitObject = (nParams >= 3) ? gAptActionInterpreter.stackAt(2) : NULL ;
    int nDepthInt;

    nDepthInt = pDepth->toInteger();
    AptValue * pRef = gAptActionInterpreter._doCloneSprite(pThis->c_cih(), NULL, pThis->c_cih(), pTarget,
                        nDepthInt + AptDisplayList::BASE_MOVIE_DEPTH, pInitObject);
    // $BUG889 DUG duplicateMovieClip returning invalid movie clip
    return pRef;
}

/*F************************************************************************************************/
/*!
    \Function    AptCIH::removeMovieClip

    \Description
        The corresponding MovieClip.removeMovieClip method from Flash.
    \Output

    \Notes
        None.

    \Version    1.0        05/04/04 (LEO) First Version
*/
/************************************************************************************************F*/
APT_CIH_NATIVE_MEMBER_FUNCTION(AptCIH, removeMovieClip)
{
    AptValue *pObject = NULL;
    gAptActionInterpreter.valueToObject(pThis->c_cih(), NULL, pThis->c_cih(), &pObject);
    if (pObject && pObject->isCIH())
    {
        AptCIH *pCIH = pObject->c_cih();
        AptCharacterSpriteInstBase *pParentInst = pCIH->GetDisplayListParent()->GetSpriteInstBase();
        pParentInst->mDisplayList.removeClonedObject(pCIH);
    }
    return gpUndefinedValue;
}

/*F************************************************************************************************/
/*!
    \Function    AptCIH::removeTextField

    \Description
        The corresponding MovieClip.removeTextField method from Flash.

    \Output

    \Notes
        None.

    \Version    1.0        05/04/04 (LEO) First Version

*/
/************************************************************************************************F*/
APT_CIH_NATIVE_MEMBER_FUNCTION(AptCIH, removeTextField)
{
    AptValue *pObject = NULL;
    gAptActionInterpreter.valueToObject(pThis->c_cih(), NULL, pThis->c_cih(), &pObject);
    if (pObject && pObject->isCIH())
    {
        AptCIH *pCIH = pObject->c_cih();
        AptCharacterSpriteInstBase *pParentInst = pCIH->GetDisplayListParent()->GetSpriteInstBase();
        pParentInst->mDisplayList.removeClonedObject(pCIH);
//      pParentInst->mDisplayList.removeObject(pCIH);
    }
    return gpUndefinedValue;
}

/*F************************************************************************************************/
/*!
    \Function    AptCIH::createTextField

    \Description
        The corresponding MovieClip.createTextField method from Flash.
    \Output

    \Notes
        None.

    \Version    1.0        05/04/04 (LEO) First Version

*/
/************************************************************************************************F*/
APT_CIH_NATIVE_MEMBER_FUNCTION(AptCIH, createTextField)
{
    if (nParams != 6)
        return gpUndefinedValue;
    AptValue *pInstName = gAptActionInterpreter.stackAt(0);
    AptValue *pDepth = gAptActionInterpreter.stackAt(1);
    AptValue *pX = gAptActionInterpreter.stackAt(2);
    AptValue *pY = gAptActionInterpreter.stackAt(3);
    AptValue *pWidth = gAptActionInterpreter.stackAt(4);
    AptValue *pHeight = gAptActionInterpreter.stackAt(5);

    int nDepth ;
    float   fX, fY, fWidth, fHeight ;

    nDepth = pDepth->toInteger();
    fX = pX->toFloat();
    fY = pY->toFloat();
    fWidth = pWidth->toFloat();
    fHeight = pHeight->toFloat();

    AptNativeString sInstName;
    pInstName->toString(sInstName);
    AptCIH *pSourceCIH = pThis->c_cih();

    AptCIH * pNewText = pSourceCIH->InsertChild(NULL, AptCharacterHelper::GetAptTextCharacter(), nDepth + AptDisplayList::BASE_MOVIE_DEPTH, &sInstName, NULL);

    // now set the bCreatedDynamic field in newly created AptCharacterTextInst to true
    // so that pCharacter created above gets deleted while AptCharacterTextInst gets deleted.
    if (pNewText->IsDynamicTextInst())
    {
        AptCharacterTextInst *pTextInst = pNewText->GetDynamicTextInst();
        pTextInst->SetCreatedDynamic( true );
        pTextInst->SetIsVisible(true);

        // LEO added next line to ensure the dynamic text field is handled correctly
        pTextInst->SetStateFlags( APT_TEXTFIELD_FUPDATE | APT_TEXTFIELD_DIRTY );

        pNewText->SetProceduralProperty(AptProceduralProperty_X, fX + 2.f);
        pNewText->SetProceduralProperty(AptProceduralProperty_Y, fY + 2.f);

        pTextInst->GetBoundsWritable().fRight = pTextInst->GetBoundsWritable().fLeft + fWidth;
        pTextInst->GetBoundsWritable().fBottom = pTextInst->GetBoundsWritable().fTop + fHeight;

#if defined(APT_USE_MOUSE)
        if(bDefaultMouseWheel)          // set if mouse events and the default mouse wheel flag is true  7/
        {
            GetTargetSim()->GetAnimationTarget()->GetMouseListenerSet()->add(pNewText);
            GetTargetSim()->GetAnimationTarget()->GetInputSet()->add(pNewText);
        }
#endif
    }

    return gpUndefinedValue;
}

/*F************************************************************************************************/
/*!
    \Function    AptCIH::getDepth

    \Description
        The corresponding MovieClip.getDepth method from Flash.
    \Output

    \Notes
        None.

    \Version    1.0        05/04/04 (LEO) First Version
*/
/************************************************************************************************F*/
APT_CIH_NATIVE_MEMBER_FUNCTION(AptCIH, getDepth)
{
    if (pThis->isCIH())
    {
        // DC 2004/12/04 fix to Get Depth, flash stores the numbers as based off
        // of -16384 instead of 0. This offset seems to be consistent.
        // Bug 299.
        return AptInteger::Create(pThis->c_cih()->GetDepth() - AptDisplayList::BASE_MOVIE_DEPTH) ;
    }

    return gpUndefinedValue;
}

/*F************************************************************************************************/
/*!
    \Function    AptCIH::getInstanceAtDepth
    
    \Description
        The corresponding MovieClip.getInstanceAtDepth method from Flash.
    \Output
    
    \Notes
        None.
        
    \Version    1.0        05/04/04 (LEO) First Version
*/
/************************************************************************************************F*/
APT_CIH_NATIVE_MEMBER_FUNCTION(AptCIH, getInstanceAtDepth)
{
    if (pThis->isCIH())
    {
        AptValue *pDepth = gAptActionInterpreter.stackAt(0);
        int targetDepth = pDepth->toInteger() + AptDisplayList::BASE_MOVIE_DEPTH;  // Must re-normalize to a zero based index.

        const AptCIH *pChild = pThis->c_cih()->GetFirstChild();
        while(pChild)
        {
            if(pChild->isCIH())
            {
                AptCIH * pChildInst = pChild->c_cih();
                if(pChildInst->GetDepth() == targetDepth)
                {
                    return pChildInst;
                }
            }
            pChild = pChild->GetDisplayListNext();
        }
    }

    return gpUndefinedValue;
}

/*F************************************************************************************************/
/*!
    \Function    AptCIH::swapDepths

    \Description
        The corresponding MovieClip.swapDpeths method from Flash.
    \Output

    \Notes
        None.

    \Version    1.0        05/04/04 (LEO) First Version
    \Version    1.1        03/04/05 (DC)  Fixed bug for crash when swapping to the depth it was
                                          already at, and added support for float depths.

*/
/************************************************************************************************F*/
APT_CIH_NATIVE_MEMBER_FUNCTION(AptCIH, swapDepths)
{
    if (nParams != 1 && pThis->isCIH())
        return gpUndefinedValue;

    AptCIH * pThisInst = pThis->c_cih() ;
    AptValue *pTarget = gAptActionInterpreter.stackAt(0);

    AptCIH * pTargetCIH = NULL;
    if (pTarget->isCIH())
        pTargetCIH = pTarget->c_cih() ;
    else if (pTarget->isString())       // parameter is a target name and not a depth
    {
        // search on name in parents display list
        AptNativeString sInstName;
        pTarget->toString(sInstName);

        pThisInst->GetDisplayListParent()->GetSpriteInstBase()->mDisplayList.pState->findInst(
            0, &sInstName, &pTargetCIH) ;
    }
    else if (pTarget->isInteger() || pTarget->isFloat()) // parameter is a depth number and not target string.
    {
        int targetDepth = pTarget->toInteger() + AptDisplayList::BASE_MOVIE_DEPTH; // Must re-normalize to a zero based index.

        // If the inst is already at that depth, then there is nothing to do!
        if(targetDepth == pThisInst->GetDepth())
            return gpUndefinedValue;

        // search for depth number in parents display list
        pThisInst->GetDisplayListParent()->GetSpriteInstBase()->mDisplayList.pState->findInst(
            targetDepth, NULL, &pTargetCIH) ;
    }

    // Don't swap if the target isn't valid, and don't swap with yourself :)
    if ( pTargetCIH && (pTargetCIH->getIsDefined()) && pTargetCIH != pThisInst)
    {
        // if target movieclip is defined and found in parents display list then swap the depths.
        AptDisplayListState * pState = pThisInst->GetDisplayListParent()->GetDisplayListState();
        if(pState == NULL)
        {
            APT_ASSERT(false && "Attempting to operate on Display list of object with none!");
            return gpUndefinedValue;
        }
        pState->swapDepths(pTargetCIH, pThisInst);
    }
    else if(pTarget->isInteger() || pTarget->isFloat())
    {
        // movieClip with given depth is not existing so reinsert this to its position.
        pThisInst->GetDisplayListParent()->GetSpriteInstBase()->mDisplayList.pState->ChangeDepth(pTarget->toInteger() + AptDisplayList::BASE_MOVIE_DEPTH, pThisInst);
    }

    return gpUndefinedValue;
}

/*F************************************************************************************************/
/*!
    \Function    AptCIH::setMask

    \Description
        The corresponding MovieClip.setMask method from Flash.

    \Output

    \Notes
        None.

    \Version    1.0        05/04/04 (LEO) First Version
    \Version    1.1        05/04/04 (DC)  Get SetMask working (preliminary).
*/
/************************************************************************************************F*/
APT_CIH_NATIVE_MEMBER_FUNCTION(AptCIH, setMask)
{
    if (nParams != 1 && pThis->isCIH())
        return gpUndefinedValue;

    AptCIH * pThisInst = pThis->c_cih() ;
    AptValue *pTarget = gAptActionInterpreter.stackAt(0);

    AptCIH * pTargetCIH = NULL;
    if (pTarget->isCIH())
    {
        pTargetCIH = pTarget->c_cih() ;
    }
    // If the target is undefined here, that means we are trying to use setMask to remove a mask,
    // which is a valid operation.

        pThisInst->SetMask(pTargetCIH);
    
    return gpUndefinedValue;
}

/*F************************************************************************************************/
/*!
    \Function    AptCIH::getBounds

    \Description
        The corresponding MovieClip.getBounds method from Flash.

    \Output

    \Notes
        None.

    \Version    1.0        05/04/04 (LEO) First Version
    \Version    2.0        08/20/08 (MMeyer) Use Matrix calculations, rather than arithmetic, to correctly take rotational, etc. transforms into account (and not simply translations)

*/
/************************************************************************************************F*/
APT_CIH_NATIVE_MEMBER_FUNCTION(AptCIH, getBounds)
{
    if (nParams > 1)
        return gpUndefinedValue;

    AptCIH *pTargetSpace = pThis->c_cih();                          // default target space is local coords
    AptRect rectInGlobalCoords;
    AptRect rectInTargetCoords;

    if(nParams == 1)
    {
        AptValue *pNewSpace = gAptActionInterpreter.stackAt(0);     // get targetCoordinateSpace
        if(pNewSpace->isUndefined())
            return gpUndefinedValue;
        pTargetSpace = pNewSpace->c_cih();
    }
    AptObject *pObject = new AptObject(AptVFT_Object);              // create new bounds obj

    // michaelmeyer 08/06/2008 - get the bounding rect in global coords
    pThis->c_cih()->GetGlobalBoundingRect(&rectInGlobalCoords);

    // michaelmeyer 08/06/2008 - get a transform from the target to global space
    AptMatrix targetTransform = gIdentityMatrix;
    pThis->c_cih()->MultParentMatrix(pTargetSpace, targetTransform);

    // invert that to get the transform from global to target
    AptMatrix targetTransformInverse;
    bool hasInverse = targetTransformInverse.AptMatrixInverse(&targetTransform);

    // convert the bounds to the target space
    // Somehow we totally failed to get an invertible matrix.  Maybe the
    // target space for getBounds is an invalid, uninitialized, or has been
    // shrunk down to 0 width or height?
    APT_ASSERT(hasInverse);
    if (hasInverse)
    {
        rectInTargetCoords.fLeft = FLT_MAX;
        rectInTargetCoords.fRight = -FLT_MAX;
        rectInTargetCoords.fBottom = -FLT_MAX;
        rectInTargetCoords.fTop = FLT_MAX;
        AptRenderingContext::expandBoundingRect(&rectInGlobalCoords, &targetTransformInverse, &rectInTargetCoords);
    }
    else
    {
        return gpUndefinedValue;
    }

    AptFloat *pXMin = AptFloat::Create(rectInTargetCoords.fRight);
    pObject->Set(StringPool::GetString(SC_xMax), pXMin);
    AptFloat *pXMax = AptFloat::Create(rectInTargetCoords.fLeft);
    pObject->Set(StringPool::GetString(SC_xMin), pXMax);
    AptFloat *pYMin = AptFloat::Create(rectInTargetCoords.fBottom);
    pObject->Set(StringPool::GetString(SC_yMax), pYMin);
    AptFloat *pYMax = AptFloat::Create(rectInTargetCoords.fTop);
    pObject->Set(StringPool::GetString(SC_yMin), pYMax);
    return pObject;
}

/*F************************************************************************************************/
/*!
    \Function    AptCIH::startDrag

    \Description
        The corresponding MovieClip.startDrag method from Flash.
    \Output

    \Notes
        None.

    \Version    1.0        06/03/04 (LEO) First Version
*/
/************************************************************************************************F*/
APT_CIH_NATIVE_MEMBER_FUNCTION(AptCIH, startDrag)
{
    APT_INC(pThis);
#if defined(APT_USE_MOUSE)
    GetTargetSim()->GetAnimationTarget()->SetDragMC(pThis);                    // Set the pDragMC to this instance

    GetTargetSim()->GetAnimationTarget()->GetDragPos()->tx = 0.f;                  // Reset the mDragPos
    GetTargetSim()->GetAnimationTarget()->GetDragPos()->ty = 0.f;
    GetTargetSim()->GetAnimationTarget()->GetDragPos()->a = -9999.f;
    GetTargetSim()->GetAnimationTarget()->GetDragPos()->b = -9999.f;
    GetTargetSim()->GetAnimationTarget()->GetDragPos()->c = -9999.f;
    GetTargetSim()->GetAnimationTarget()->GetDragPos()->d = -9999.f;

    // If there are no parameters or the first one is false, set the offset vector
    if(nParams == 0 || gAptActionInterpreter.stackAt(0)->c_boolean()->toInteger() == 0)
    {
        const AptMatrix *matrix = pThis->c_cih()->GetPositionMatrixConst() ;

        GetTargetSim()->GetAnimationTarget()->GetDragPos()->tx = GetTargetSim()->GetAnimationTarget()->GetXMousePos() - matrix->tx;
        GetTargetSim()->GetAnimationTarget()->GetDragPos()->ty = GetTargetSim()->GetAnimationTarget()->GetYMousePos() - matrix->ty;
    }

    // Now set the boundary box limits
    if(nParams > 0)
    {
        GetTargetSim()->GetAnimationTarget()->GetDragPos()->a = gAptActionInterpreter.stackAt(1)->toFloat();
        GetTargetSim()->GetAnimationTarget()->GetDragPos()->b = 0;                 // Flash will default these to zero if they are not passed in
        GetTargetSim()->GetAnimationTarget()->GetDragPos()->c = 0;
        GetTargetSim()->GetAnimationTarget()->GetDragPos()->d = 0;
    }
    if(nParams > 1)
    {
        GetTargetSim()->GetAnimationTarget()->GetDragPos()->b = gAptActionInterpreter.stackAt(2)->toFloat();
    }
    if(nParams > 2)
    {
        GetTargetSim()->GetAnimationTarget()->GetDragPos()->c = gAptActionInterpreter.stackAt(3)->toFloat();
    }
    if(nParams > 3)
    {
        GetTargetSim()->GetAnimationTarget()->GetDragPos()->d = gAptActionInterpreter.stackAt(4)->toFloat();
    }
#endif // #if defined(APT_USE_MOUSE)
    return gpUndefinedValue;
}

/*F************************************************************************************************/
/*!
    \Function    AptCIH::hitTest

    \Description
        The corresponding MovieClip.hitTest method from Flash.

    \Output

    \Notes
        If the shape test flag is true, then it is up to the auxiliary library to make the point in shape test.  We assume that if they use this flag they have
        provided the callback, otherwise we do a simple point in boundary box test.

    \Version    1.0        05/04/04 (LEO) First Version
*/
/************************************************************************************************F*/
APT_CIH_NATIVE_MEMBER_FUNCTION(AptCIH, hitTest)
{
    if (nParams == 1)
    {
        // This case is hitTest for MovieClip to MovieClip
        AptValue * pValue = gAptActionInterpreter.stackAt(0);
        if(pValue->isCIH(true))
        {
            AptCIH *pTarget = pValue->c_cih();
            AptRect thisRect;
            AptRect targetRect;
            pThis->c_cih()->GetBoundingRect(&thisRect);
            pTarget->GetBoundingRect(&targetRect);
            // DC -- 2004/12/04 -- added new algorithm for hit testing. Much faster
            // and should work for all hit cases. To overlap, by definition, each
            // side of the target must be less (left,top) or greater (right,bottom)
            // then it's opposite side. I.e. for any overlap to be possible the
            // right side of the target *must* be greater then the left side of the
            // current. I am using this logic on all four sides of the target.
            if( targetRect.fLeft   <= thisRect.fRight && targetRect.fRight >= thisRect.fLeft &&
                targetRect.fBottom >= thisRect.fTop   && targetRect.fTop   <= thisRect.fBottom)
            {
                return AptInteger::Create(1);
            }
        }
    }
    else if (nParams > 1)
    {
        // This case if for point in MovieClip hitTest
        float fX, fY;
        int bShapeHitTest = 0;
        fX = gAptActionInterpreter.stackAt(0)->toFloat();
        fY = gAptActionInterpreter.stackAt(1)->toFloat();

        if (nParams > 2)
        {
            bShapeHitTest = gAptActionInterpreter.stackAt(2)->toInteger();
        }

        if (bShapeHitTest)
        {
            APT_ASSERT(gAptFuncs.pfnPointHitTest);                                  // We will assume that if the user tries to hitTest on shape flag, they have defined their own hitTest function
            return AptInteger::Create(gAptFuncs.pfnPointHitTest(fX, fY, pThis));    // We will pass the call back function the whole AptCIH since it contains the geometry of itself and everything within it
        }
        else
        {
            AptRect rect;
            pThis->c_cih()->GetBoundingRect(&rect);
            if (fX >= rect.fLeft && fX <= rect.fRight && fY >= rect.fTop && fY <= rect.fBottom)
            {
                return AptInteger::Create(1);
            }
        }
    }
    return AptInteger::Create(0);
}

/*F************************************************************************************************/
/*!
    \Function    AptCIH::createEmptyMovieClip

    \Description
        The corresponding MovieClip.createEmptyMovieClip method from Flash.

    \Output

    \Notes
        None.

    \Version    1.0        05/04/04 (LEO) First Version
    \Version    1.1        2/21/07  (ASK) $Bug EATech 107144 Problem with createEmptyMovieClip
                            There was some code that was merged out long time back, but then I realized
                            The fix is not needed here and it can be done in AssociateInstToClass by early returning
                            from that function. So removed the fix that added here to set pParentAnim of
                            global empty character everytime. pParentAnim is of static global empty AptCharacter
                            is never used now in this particular case.
*/
/************************************************************************************************F*/
APT_CIH_NATIVE_MEMBER_FUNCTION(AptCIH, createEmptyMovieClip)
{
    if (nParams != 2)
        return gpUndefinedValue;

    AptValue *pInstName = gAptActionInterpreter.stackAt(0);
    AptValue *pDepth = gAptActionInterpreter.stackAt(1);
    int nDepth;

    nDepth = pDepth->toInteger();

    AptNativeString sInstName;
    pInstName->toString(sInstName);

    AptCIH *pSourceCIH = pThis->c_cih();

    // $Bug 1015  mpCharacter NULL crash in createEmptyMovieClip() - added extra assert and early return 1/1
    const AptCharacter * pMCCharacter = pThis->c_cih()->GetCharacterInst()->GetCharacterConst() ;
#if !defined (DO_COVERAGE)
    APT_ASSERT(pMCCharacter && "Movieclip that is trying to call createEmptyMovieClip is not a valid movieclip") ;
#endif
    if (!pMCCharacter)
    {
        return gpUndefinedValue;
    }// end of fix

    AptCIH * pNewInst = pSourceCIH->InsertChild(NULL, AptCharacterHelper::GetAptMovieCharacter(), nDepth  + AptDisplayList::BASE_MOVIE_DEPTH, &sInstName, NULL);

    // now set the bCreatedDynamic field in newly created AptCharacterSpriteInst to true
    // so that pCharacter created above gets deleted while AptCharacterSpriteInst gets deleted.
    if (pNewInst && pNewInst->isCIH())
        pNewInst->GetSpriteInst()->SetCreatedDynamic( true );

    if(pNewInst)
    {
        return pNewInst;
    }
    return gpUndefinedValue;
}

/*F************************************************************************************************/
/*!
    \Function    AptCIH::loadVariables

    \Description
        The corresponding MovieClip.loadVariables method from Flash.

    \Output

    \Notes
        None.

    \Version    1.0        05/04/04 (LEO) First Version
*/
/************************************************************************************************F*/
APT_CIH_NATIVE_MEMBER_FUNCTION(AptCIH, loadVariables)
{
    if (nParams > 0)
    {
        AptValue *pFrame = gAptActionInterpreter.stackAt(0);
        AptNativeString sBuf;
        pFrame->toString(sBuf);
        gAptActionInterpreter.loadVariables(pThis, NULL, &sBuf);
    }
    return gpUndefinedValue;
}


/*F************************************************************************************************/
/*!
    \Function    AptCIH::stop

    \Description
    The corresponding MovieClip.stop method from Flash.

    \Input
    \Output

    \Notes
    None.

    \Version    1.0        08/09/06 (ASK) First Version - Added Function header
*/
/************************************************************************************************F*/
APT_CIH_NATIVE_MEMBER_FUNCTION(AptCIH, stop)
{
    if (pThis->c_cih()->IsSpriteInstBase())
    {
        pThis->c_cih()->SetIsPlaying(false);
    }
    return gpUndefinedValue;
}

/*F************************************************************************************************/
/*!
    \Function    AptCIH::play

    \Description
    The corresponding MovieClip.play method from Flash.

    \Input
    \Output

    \Notes
    None.

    \Version    1.0        08/09/06 (ASK) First Version - Added Function header
*/
/************************************************************************************************F*/
APT_CIH_NATIVE_MEMBER_FUNCTION(AptCIH, play)
{
    if (pThis->c_cih()->IsSpriteInstBase())
    {
        pThis->c_cih()->SetIsPlaying(true);
    }
    return gpUndefinedValue;
}

/*F************************************************************************************************/
/*!
    \Function    AptCIH::nextFrame

    \Description
    The corresponding MovieClip.nextFrame method from Flash.

    \Input
    \Output

    \Notes
    None.

    \Version    1.0        08/09/06 (ASK) First Version - Added Function header
*/
/************************************************************************************************F*/
APT_CIH_NATIVE_MEMBER_FUNCTION(AptCIH, nextFrame)
{
    pThis->c_cih()->jumpToFrame(pThis->c_cih()->GetSpriteInstBase()->mnFrame + 1);
    pThis->c_cih()->SetIsPlaying(false);
    return gpUndefinedValue;
}

/*F************************************************************************************************/
/*!
    \Function    AptCIH::prevFrame

    \Description
    The corresponding MovieClip.prevFrame method from Flash.

    \Input
    \Output

    \Notes
    None.

    \Version    1.0        08/09/06 (ASK) First Version - Added Function header
*/
/************************************************************************************************F*/
APT_CIH_NATIVE_MEMBER_FUNCTION(AptCIH, prevFrame)
{
    pThis->c_cih()->jumpToFrame(pThis->c_cih()->GetSpriteInstBase()->mnFrame - 1);
    pThis->c_cih()->SetIsPlaying(false);
    return gpUndefinedValue;
}


/*F************************************************************************************************/
/*!
    \Function    AptCIH::getBytesTotal

    \Description
    The corresponding MovieClip.getBytesTotal method from Flash.

    \Input
    \Output

    \Notes
    None.

    \Version    1.0        08/09/06 (ASK) First Version - Added Function header
*/
/************************************************************************************************F*/
APT_CIH_NATIVE_MEMBER_FUNCTION(AptCIH, getBytesTotal)
{
    AptNativeString sBuf;

    if(pThis->c_cih()->GetCharacterInst() == NULL)       // (LEO) $bug786
    {
        return AptFloat::Create(0.f);
    }
    if (pThis->isCIH())
    {
        if (pThis->c_cih()->IsAnimationInst())
        {
            sBuf += pThis->c_cih()->GetAnimationInst()->mpFile->GetName() ;
        }
    }

    APT_ASSERT(gAptFuncs.pfnGetBytesTotal) ;
    // call the callback function only if movieclip is a animationinst as auxiliary library knows
    // only about sizes of swf/big files and not the individual movieclips inside it.
    float fFloat = 0.0f ;
    if (pThis->c_cih()->IsAnimationInst())
    {
        APT_PROFILE_AUXILIARY(pfnGetBytesTotal);
        fFloat = (float) gAptFuncs.pfnGetBytesTotal(sBuf.ConstRawPtr(), AptGetBytesEnum_MovieClip);
    }
    return AptFloat::Create(fFloat);
}

/*F************************************************************************************************/
/*!
    \Function    AptCIH::getBytesLoaded

    \Description
    The corresponding MovieClip.getBytesLoaded method from Flash.

    \Input
    \Output

    \Notes
    None.

    \Version    1.0        08/09/06 (ASK) First Version - Added Function header
*/
/************************************************************************************************F*/
APT_CIH_NATIVE_MEMBER_FUNCTION(AptCIH, getBytesLoaded)
{
    AptNativeString sBuf;
//  AptActionInterpreter::getName(pThis->c_cih(), sBuf);
    if (pThis->isCIH())
    {
        if (pThis->c_cih()->IsAnimationInst())
        {
            sBuf += pThis->c_cih()->GetAnimationInst()->mpFile->GetName() ;
        }
    }

    APT_ASSERT(gAptFuncs.pfnGetBytesLoaded) ;
    float fFloat = 0.0f ;
    // call the callback function only if movieclip is a animationinst as auxiliary library knows
    // only about sizes of swf/big files and not the individual movieclips inside it.
    if (pThis->c_cih()->IsAnimationInst())
    {
        APT_PROFILE_AUXILIARY(pfnGetBytesLoaded);
        fFloat = (float) gAptFuncs.pfnGetBytesLoaded(sBuf.ConstRawPtr(), AptGetBytesEnum_MovieClip);
    }
    return AptFloat::Create(fFloat);
}

/*F************************************************************************************************/
/*!
    \Function    AptCIH::CleanNativeFunctions()

    \Description
    Cleans up the native functions for AptCIH

    \Input
    \Output

    \Notes
    Overrides Function in AptValue

    \Version    1.0        08/09/06 (ASK) First Version - Added Function header

*/
/************************************************************************************************F*/
void    AptCIH::CleanNativeFunctions()
{
    APT_CIH_NATIVE_MEMBER_FUNCTION_DESTROY(gotoAndPlay);
    APT_CIH_NATIVE_MEMBER_FUNCTION_DESTROY(gotoAndStop);
    APT_CIH_NATIVE_MEMBER_FUNCTION_DESTROY(prevFrame);
    APT_CIH_NATIVE_MEMBER_FUNCTION_DESTROY(nextFrame);
    APT_CIH_NATIVE_MEMBER_FUNCTION_DESTROY(stop);
    APT_CIH_NATIVE_MEMBER_FUNCTION_DESTROY(play);
    APT_CIH_NATIVE_MEMBER_FUNCTION_DESTROY(loadVariables);
    APT_CIH_NATIVE_MEMBER_FUNCTION_DESTROY(attachMovie);
    APT_CIH_NATIVE_MEMBER_FUNCTION_DESTROY(loadMovie);
    APT_CIH_NATIVE_MEMBER_FUNCTION_DESTROY(unloadMovie);
    APT_CIH_NATIVE_MEMBER_FUNCTION_DESTROY(duplicateMovieClip);
    APT_CIH_NATIVE_MEMBER_FUNCTION_DESTROY(removeMovieClip);
    APT_CIH_NATIVE_MEMBER_FUNCTION_DESTROY(createTextField);
    APT_CIH_NATIVE_MEMBER_FUNCTION_DESTROY(removeTextField);
    APT_CIH_NATIVE_MEMBER_FUNCTION_DESTROY(getDepth);
    APT_CIH_NATIVE_MEMBER_FUNCTION_DESTROY(getInstanceAtDepth);
    APT_CIH_NATIVE_MEMBER_FUNCTION_DESTROY(getBounds);
    APT_CIH_NATIVE_MEMBER_FUNCTION_DESTROY(hitTest);
    APT_CIH_NATIVE_MEMBER_FUNCTION_DESTROY(startDrag);
    APT_CIH_NATIVE_MEMBER_FUNCTION_DESTROY(createEmptyMovieClip);
    APT_CIH_NATIVE_MEMBER_FUNCTION_DESTROY(getNewTextFormat);
    APT_CIH_NATIVE_MEMBER_FUNCTION_DESTROY(getTextFormat);
    APT_CIH_NATIVE_MEMBER_FUNCTION_DESTROY(setTextFormat);
    APT_CIH_NATIVE_MEMBER_FUNCTION_DESTROY(getBytesTotal);
    APT_CIH_NATIVE_MEMBER_FUNCTION_DESTROY(getBytesLoaded);
    APT_CIH_NATIVE_MEMBER_FUNCTION_DESTROY(swapDepths);
    APT_CIH_NATIVE_MEMBER_FUNCTION_DESTROY(setMask);
    APT_CIH_NATIVE_MEMBER_FUNCTION_DESTROY(localToGlobal);
    APT_CIH_NATIVE_MEMBER_FUNCTION_DESTROY(globalToLocal);
}


/*F************************************************************************************************/
/*!
    \Function    AptCIH::SetHasClass

    \Description
        here to signify that this object is an actionscript class

    \Input      bHasClass         - new Value

    \Output
        None.

    \Notes
        Overrides Function in AptValue

    \Version    1.0        04/12/05 (DC) Bringin' it to ya

*/
/************************************************************************************************F*/
void AptCIH::SetHasClass(int bHasClass)
{
    mbHasClass = (bHasClass) ? 1 : 0;
}

/*F************************************************************************************************/
/*!
    \Function    AptCIH::GetHasClass

    \Description
        here to signify that this object is an actionscript class

    \Input
        None.

    \Output     bool           - returns true if AptCIH has a class

    \Notes
        Overrides Function in AptValue

    \Version    1.0        04/12/05 (DC) Bringin' it to ya

*/
/************************************************************************************************F*/
bool AptCIH::GetHasClass() const
{
    return mbHasClass;
}


/*F************************************************************************************************/
/*!
    \Function    AptCIH::GetXScale

    \Description
        Returns the scaling in X represented by the given matrix

        The algorithm for this function follows the GetCosAngle assumptions:
        1)Translate V1 <1, 0> by the sprites matrix to get V2 <x2, y2>
        V1 = <fX1 = 1.f, fY1 = 0.f>
        V2 = <fX2 = fX1 * matrix.a + fY1 * matrix.c, fY2 = fX1 * matrix.b + fY1 * matrix.d>
        2)Find the length of V2
        |V2| = sqrt((V2.x * V2.x) + (V2.y * V2.y))

    \Input
    None.

    \Output     float      -  the scaling in X represented by the AptCIH's matrix

    \Version    1.0        05/02/07 (RB) First Version with input from Richard Geldard

*/
/************************************************************************************************F*/
static float GetXScale(const AptMatrix * const matrix)
{
    return (float)sqrtf((matrix->a * matrix->a) + (matrix->b * matrix->b));
}


/*F************************************************************************************************/
/*!
    \Function    AptCIH::GetYScale

    \Description
        Returns the scaling in Y represented by the given matrix

    \Output     float      -  the scaling in Y represented by the AptCIH's matrix

    \Version    1.0        05/02/07 (RB) First Version with input from Richard Geldard

*/
/************************************************************************************************F*/
static float GetYScale(const AptMatrix * const matrix)
{
    return (float)sqrtf((matrix->c * matrix->c) + (matrix->d * matrix->d));
}


/*F************************************************************************************************/
/*!
    \Function    AptCIH::GetVectorLength

    \Description
        Returns the Vector length for a simple vector translated by the AptCIH's matrix

        The algorithm for this function follows the GetCosAngle assumptions:
        1)Translate V1 <1, 0> by the sprites matrix to get V2 <x2, y2>
        V1 = <fX1 = 1.f, fY1 = 0.f>
        V2 = <fX2 = fX1 * matrix.a + fY1 * matrix.c, fY2 = fX1 * matrix.b + fY1 * matrix.d>
        2)Find the length of V2
        |V2| = sqrt((V2.x * V2.x) + (V2.y * V2.y))

    \Input
    None.

    \Output     float             - Returns the Vector length for a simple vector translated by the AptCIH's matrix

    \Notes
    None.

    \Version    1.0        01/21/05 (LEO) First Version

*/
/************************************************************************************************F*/
float AptCIH::GetVectorLength(const AptMatrix * const matrix)
{
    if(matrix->b == 0.f && matrix->c == 0.f)
    {
        return matrix->a;
    }
    return (float)sqrtf((matrix->a * matrix->a) + (matrix->b * matrix->b));
}

/*F************************************************************************************************/
/*!
    \Function    AptCIH::GetCosAngle

    \Description
        Returns the cos(angle) for a the current AptCIH's matrix

        The algorithm for this function is as follows:
        1)Translate V1 <1, 0> by the sprites matrix to get V2 <x2, y2>
        V1 = <fX1 = 1.f, fY1 = 0.f>
        V2 = <fX2 = fX1 * matrix.a + fY1 * matrix.c, fY2 = fX1 * matrix.b + fY1 * matrix.d>
        2)Find the cos(angle) between V1 and V2 using the dot product.
        |V1| = 1, |V2| = sqrt((V2.x * V2.x) + (V2.y * V2.y))
        V1.V2 / (|V1| * |V2|)    where . = v1.x * v2.x + v1.y * v2.y
        Since V1 <1, 0> is a simple vector, the math has been simplified to cancel out any unneeded math
        so V1.V2 / (|V1| * |V2|) = V2.x / |V2|

    \Input
    None.

    \Output     float             - Returns the cos(angle) for a the current AptCIH's matrix

    \Notes
    None.

    \Version    1.0        01/21/05 (LEO) First Version

*/
/************************************************************************************************F*/
float AptCIH::GetCosAngle(const AptMatrix * const matrix)
{
    if(matrix->b == 0.f && matrix->c == 0.f)
    {
        return 1.f;
    }
    return (matrix->a / GetVectorLength(matrix));       // = cos Angle
}


/*F************************************************************************************************/
/*!
    \Function    AptCIH::Remove

    \Description
        Removes the AptCIH from different lists and also calls ClearCIH

    \Input
        bDestroyGC - boolean to check if to destroy garbage collected objects in it.

    \Output
        None.

    \Notes
        -Added code to remove the AptCIH from the action pool and the button handler pool

    \Version    1.0        08/04/04 (LEO) First Version
        -Added case for zombie sprites, we no longer want to undefine z zombie sprite.
*/
/************************************************************************************************F*/
void AptCIH::Remove(const bool bDestroyGC)
{
    GetTargetSim()->GetLinker()->CancelLoad(this);
#if defined APT_USE_BUTTONS
    if (GetTargetSim()->GetAnimationTarget()->GetFocusButton() == this)
    {
        APT_DEC(GetTargetSim()->GetAnimationTarget()->GetFocusButton());
        GetTargetSim()->GetAnimationTarget()->SetFocusButton(NULL);
    }
#endif

    GetTargetSim()->GetAnimationTarget()->GetActionPool()->RemoveActionFor(this);           // remove any actions associated with the original context (since we're replaceing it)
#if defined APT_USE_BUTTONS
    GetTargetSim()->GetAnimationTarget()->RemoveFromBIL(this);
#endif


    // $bug791 - MCB - If we're shutting down, don't run onUnload functions.
    // $bug 106899 (ASK) Redundant code in AptCIH.cpp::void AptCIH::Remove(const bool bDestroyGC)
    ClearCIH(bDestroyGC);

    if (getRefCount() > 1)
    {
        APT_ASSERT(mpNext == NULL && mpPrev == NULL && "This should already be done!");
        if(GetCIHState() == AptCIH::AptCIHState_Normal)
        {
            setIsDefined(0);
        }
    }
    APT_DEC(this);
}

/*F************************************************************************************************/
/*!
    \Function    AptCIH::ClearDepends

    \Description
        Not used any more

    \Notes
        None.

    \Version    1.0        04/12/05 (LEO) First Version

*/
/************************************************************************************************F*/
void AptCIH::ClearDepends()
{
    if(GetSpriteInstBase() != NULL && GetSpriteInstBase()->GetCharacterConst()->sprite.movie.nFrames > 0)
    {
        for (int i = 0; i < GetSpriteInstBase()->GetCharacterConst()->sprite.movie.aFrames[0].nControls; i++)
        {
            AptControl *pControl = GetSpriteInstBase()->GetCharacterConst()->sprite.movie.aFrames[0].apControls[i];
            if (pControl->eType == AptControlType_DoInitAction)
            {
                if(pControl->initAction.nSpriteID < 0)
                {
                    pControl->initAction.nSpriteID = -pControl->initAction.nSpriteID;
                }
            }
        }

    }

#if defined APT_USE_BUTTONS
    if (GetTargetSim()->GetAnimationTarget()->GetFocusButton() == this)
    {
        APT_DEC(GetTargetSim()->GetAnimationTarget()->GetFocusButton());
        GetTargetSim()->GetAnimationTarget()->SetFocusButton(NULL);
    }
#endif

    // $bug791 - MCB - If we're shutting down, don't run onUnload functions.
    if (gAptActionInterpreter.bShutDown)
    {
        ClearCIH(true);
    }
    else
    {
        ClearCIH(true);
    }

    GetTargetSim()->GetAnimationTarget()->GetActionPool()->RemoveActionFor(this);           // remove any actions associated with the original context (since we're replaceing it)
#if defined APT_USE_BUTTONS
    GetTargetSim()->GetAnimationTarget()->RemoveFromBIL(this);
#endif
}





/*F************************************************************************************************/
/*!
    \Function    AptCIH::ResetInitActions

    \Description
        It resets the InitActions that are defined on first frame.

    \Notes
        This was added because of a bug, where if you unload a file and quickly reload the same file
        before it completely unloads then the iniactions were not getting executed properly.
        So this function is called from AptLinker::Update, just before this AptCIH is ticked to first
        frame.
        $bug 107350  removeMovieClip followed closely by loadMovie fails


    \Version    1.0        02/08/07 (ASK) First Version

*/
/************************************************************************************************F*/
void AptCIH::ResetInitActions()
{
    if(GetSpriteInstBase() != NULL && GetSpriteInstBase()->GetCharacterConst()->sprite.movie.nFrames > 0)
    {
        AptFrame frame0 = GetSpriteInstBase()->GetCharacterConst()->sprite.movie.aFrames[0] ;
        for (int i = 0; i < frame0.nControls; i++)
        {
            AptControl *pControl = frame0.apControls[i];
            if (pControl->eType == AptControlType_DoInitAction)
            {
                if(pControl->initAction.nSpriteID < 0)
                {
                    pControl->initAction.nSpriteID = -pControl->initAction.nSpriteID;
                }
            }
        }

    }
}



/*F************************************************************************************************/
/*!
    \Function    AptCIH::AptCIH

    \Description
        Constructor

    \Input      eType             - AptValue type which is in most of case of type of characterinst inside it.
    \Input      pInstData         - AptCharacter Inst
    \Input      _pParent          - Parent movieclip.

    \Output

    \Notes
        None.

    \Version    1.0        04/01/05 (ASK) First Version

*/
/************************************************************************************************F*/
AptCIH::AptCIH( AptCharacter *pCharacter, AptCIH * _pParent)
    : AptValueGC(AptVFT_CharacterInstHandle, CO_CIH),
    mbSkipEval(0),
    mpCharacterInst(NULL)
{
    mpParent = _pParent;
    APT_INCSAFE(mpParent);
    mbASChange = 0;
    mbHasClass = 0;
    mbInRemList= 0;
    mbCIHState = AptCIHState_Normal;
    mnZombieCounter = 0;
    setGCRoot(1);
    mnCreatedOnFrame = -1;               // (LEO) $BUG670    4/5
    mbHasBlendMode  = 0 ;
    mbHasFilters = 0;
    mbGPDirty = 0;
    mbChildGPDirty = 0;

    SetAllowDelayedDeletion(false);
    fRot = NULL;
    mpNext = NULL ;
    mpPrev = NULL ;
    mbInCtor = 0;

    // Don't call SetCharacterInst, it will try to place the character inst, but we are not ready.
    mpCharacterInst = AptCharacterInst::CreateCharacterInst(pCharacter);

    // Should not be NULL anymore.
    APT_ASSERT(mpCharacterInst != NULL);

    if(pCharacter == NULL || pCharacter->eType == AptCharacterType_Sprite || pCharacter->eType == AptCharacterType_Button ||
       pCharacter->eType == AptCharacterType_Morph  || pCharacter->eType == AptCharacterType_Animation ||
       pCharacter->eType == AptCharacterType_CustomControl)
    {
        SetDirtyState(true, true);      // These objects should be dirty since they contain display lists
    }
    else
    {
        SetDirtyState(false, false);    // These are leaf nodes and do not need to be ticked
    }
}


/*F************************************************************************************************/
/*!
    \Function    AptCIH::PreDestroy

    \Description
    PreDestroy the object, mainly to keep compatibility with old __dec behavior.

    \Input

    \Output

    \Notes
        None.

    \Version
        1.0        03/07/04 (ONA) First Version

*/
/************************************************************************************************F*/
void    AptCIH::PreDestroy()
{
#if defined(APT_ENABLE_DEBUG_REPORT)
    if(mReport)
    {
        mReport->EndTrack(this);
    }
#endif
    if (gAptFuncs.pfnOnUnload)
        (gAptFuncs.pfnOnUnload)(this);
    // Go up the Chain.
    AptValueGC::PreDestroy();
}

/*F************************************************************************************************/
/*!
    \Function AptCIH::DestroyGCPointers

    \Descrption
    Release all the contained GC pointers.

    \Input      None.

    \Output     None.

    \Notes
        This function is called when the object needs to be released and just before the destruction.
        There is a separation between GC objects and non-GC objects because the Garbage Collector
        will take care of the GC objects. We don't want to destruct them also in the destructor of this class.

    \Version
        1.0        02/19/04 (ONA) First Version

*/
/************************************************************************************************F*/
void    AptCIH::DestroyGCPointers()
{
    APT_ASSERT(mpPrev == NULL);
    APT_ASSERT(mpNext == NULL);
    AptCharacterInst * pTemp = mpCharacterInst;
    mpCharacterInst = NULL;

    APT_DECSAFE(mpParent);
    mpParent = 0;

    // Go up the Chain.
    AptValueGC::DestroyGCPointers();

    if (pTemp != NULL)
    {
        pTemp->DestroyGCPointers();
        delete pTemp;
    }
}


/*F************************************************************************************************/
/*!
    \Function AptCIH::ClearCIH

    \Descrption
    This cleans up anything that no longer needs to be around when the animation is not going to be
    displayed anymore. This can would often be followed by a call to delete the object.

    \Input      bDestroyGC - if true then we need to remove our references to other objects.
                            if false then don't de-reference any outside pointers! they may not be there.

    \Output     None.

    \Notes
        must work with pValue->pData == 0

    \Version 1.0        08/02/04 (DC) First Version
             1.1        08/02/04 (DC) Fixed issue where onUnload would be called even if bDestroyGC==false.
             2.0        08/04/04 (LEO) First Version
                -Added support for bug 253.  If the AptCIH is a zombie, then we don't want to do anything.  We also check if the sprite will become a zombie when cleaning it out.
             2.0.1      09/09/04 (LEO) changed the zombie xml print out call
             3.0        07/29/08 (RB)  Added ability to track AptHighwaterMemoryMetrics for zombie vector

*/
/************************************************************************************************F*/
void    AptCIH::ClearCIH(const bool bDestroyGC)
{
    if(GetCIHState() == AptCIH::AptCIHState_Zombie)          // (LEO) we don't need to re-clear a zombie sprite...
    {
        return;
    }

    if (isUndefined()) return;

    SetDirtyState(false, false);                            // Set the flag to false since we are getting rid of this sprite
#if defined APT_USE_BUTTONS
    GetTargetSim()->GetAnimationTarget()->GetButtonSet()->remove(this);
#endif
    GetTargetSim()->GetAnimationTarget()->GetInputSet()->remove(this);

    // (LEO) release the previous held onPress and onRollOver sprites
    if(GetTargetSim()->GetAnimationTarget()->GetOnPressObject() == this)
    {
        GetTargetSim()->GetAnimationTarget()->SetOnPressObject(gpUndefinedValue);
    }
    if(GetTargetSim()->GetAnimationTarget()->GetOnRollOverObject() == this)
    {
        GetTargetSim()->GetAnimationTarget()->SetOnRollOverObject(gpUndefinedValue);
    }
    //bug fix for #259 removeMovieClip doesn't remove event listeners.
    //added code to remove key and mouse listener from listener set if this AptCIH is deleted
    GetTargetSim()->GetAnimationTarget()->GetListenerSet()->remove(this) ;
#if defined(APT_USE_MOUSE)
        GetTargetSim()->GetAnimationTarget()->GetMouseListenerSet()->remove(this) ;
#endif

#if defined APT_USE_BUTTONS
    GetTargetSim()->GetAnimationTarget()->RemoveFromBIL(this);
#endif
    for (int32_t i = 0; i < GetTargetSim()->GetAnimationTarget()->GetNewInstSize(); i++)
    {
        if (GetTargetSim()->GetAnimationTarget()->GetNewInsts()[i] == this)
        {
            APT_DEC(GetTargetSim()->GetAnimationTarget()->GetNewInsts()[i]);
            GetTargetSim()->GetAnimationTarget()->GetNewInsts()[i] = NULL;
        }
    }

    if (IsAnimationInst())
    {
        GetTargetSim()->GetAnimationTarget()->RemoveTimerFunctions(this);
    }

    AptCharacterInst * pData = mpCharacterInst;


    if(HasMask())       // If this has a mask, clear the mask-maskee dependency
    {
        AptCIH * pMyMask = GetMask();
        APT_ASSERT(pMyMask != NULL);

        pMyMask->SetIsMask(false, NULL);

        AptNativeString hashKey = APTCIH_MASK_HASHNAME;
        AptNativeString hashKey2 = APTCIH_MASKEDITEM_HASHNAME;
        AptNativeHash *pHash = pMyMask->GetNativeHash();

        pHash->Unset(&hashKey2);

        pHash = GetNativeHash();
        pHash->Unset(&hashKey);
        SetHasMask(false, NULL);
    }
    if(IsMask())        // If this is a mask, clear the mask-maskee dependency
    {
        AptNativeString hashKey = APTCIH_MASK_HASHNAME;
        AptNativeString hashKey2 = APTCIH_MASKEDITEM_HASHNAME;
        AptNativeHash *pHash = GetNativeHash();

        AptValue *pMaskedItem = pHash->Lookup(&hashKey2);

        APT_ASSERT(pMaskedItem != NULL);
        APT_ASSERT(pMaskedItem->isCIH());
        APT_ASSERT(pMaskedItem->c_cih()->HasMask());

        pMaskedItem->c_cih()->SetHasMask(false, NULL);
        SetIsMask(false, NULL);

        pHash->Unset(&hashKey2);
        pHash = pMaskedItem->c_cih()->GetNativeHash();
        pHash->Unset(&hashKey);
    }

    if (pData != NULL)
    {
        if (gAptActionInterpreter.bShutDown == false)
        {
            // $bug601 - DC - 1 of 1 - Don't call onUnload if bDestroyGC==false This is the case during Apt
            // shutdown. (at which time the function may already have been deleted!)
            if (IsSpriteInst() && HasEvent(AptEventActionFlag_Unload))
            {
                queueClipEvents(AptEventActionFlag_Unload, 0, false);           // handles unload clip action
                AptValue *pOnUnloadFnc = findChild(StringPool::GetString(SC_onUnload), NULL);           // check for defined unload call, yes this seems strange, but flash handles both onUnloads also....
                if (pOnUnloadFnc && pOnUnloadFnc->getIsDefined() && pOnUnloadFnc->isScriptFunction())
                {
                    AptScriptFunctionBase *pFunc = pOnUnloadFnc->c_scriptfunction();
#if APT_PROF_ENABLED && APT_PROFILE_ACTIONSCRIPT_ENABLED
                        APT_PROF_ENTER( (char8_t *)(const uint8_t *)pFunc->GetName() );
#endif

                    gAptActionInterpreter.callFunction(this, (AptValue *)pFunc, 0);

#if APT_PROF_ENABLED && APT_PROFILE_ACTIONSCRIPT_ENABLED
                        APT_PROF_LEAVE( (char8_t *)(const uint8_t *)pFunc->GetName() );
#endif
                    gAptActionInterpreter.stackPop();
                }
            }
        }


        // this change was made by EAC to Apt 1.02.04-ion.  It may be a fix to something that's causing single-frame zombies in NASCAR / Cafe 3
        // Clear out the movie's properties, this gets rid of any direct self references that may tigger zombies.
        if(ContainsNativeHashVirtual())
        {
            GetNativeHashVirtual()->ClearData();
        }


        // (LEO) If this animation contains any external references at this time, we need to clean everything and not delete the pData
        // 253 Flow broken on backward : http://globaltechdocs.ea.com/MediaWiki/index.php?title=Apt:External_function_references
        if(!gAptActionInterpreter.bShutDown && GetZombieCount() > 0 && IsAnimationInst() && this != _AptGetAnimationAtLevel(0))             // $BUG253 LEO    15/47
        {
            if(gpZombieVector != NULL && !gpZombieVector->IsVectorFull())
            {
                GetAnimationInst()->mDisplayList.clear(true);                // Destory everything associated with this sprite to ensure as little memory possible is being used.
                GetNativeHashVirtual()->ClearData();
                if ((gpValuesToRelease->GetNumValues() != 0))
                {
                    gpValuesToRelease->ReleaseValues();
                }
                // DC - Object is not a zombie if clearing its display list sends us back to Zombie = 0.
                // This is the case if objects on the display list are what are holding the references.
                if(GetZombieCount() > 0)
                {
                    SetASChanged(0);
                    SetHasClass(0);
					// FIFA_BEGIN | SENDBACK | mwebb | Mar 6 2015 | change to allow PrintZombieDump to be an int
					//#if defined(APT_ENABLE_ZOMBIE_OUTPUT)
					//if (bPrintZombieReferences)
                    #if defined(APT_XML_MEMORY_DUMP_SUPPORTED)
					if(iPrintZombieReferences > 0)
					// FIFA_END
                    {
                        APT_DEBUGPRINT(APT_DEBUG_MSG_WARNING_LVL, "ZOMBIE DETECTED [name = %s  file = %s.swf] HAS %d EXTERNAL FUNCTION REFERENCES\n",
                        mMyName.c_str(),
                        GetAnimationInst()->mpFile.Get()->GetName().c_str(),
                        GetZombieCount());
					}
#endif
                    SetReleaseAtEnd();
                    gpZombieVector->PushValue(this);
                    SetCIHState(AptCIH::AptCIHState_Zombie);
                    GetAnimationInst()->mpFile.Get()->setState(AptFile::Zombie);

#if defined(APT_ENABLE_ZOMBIE_OUTPUT)
                    AptHighwaterMemoryMetrics* pMetrics  = AptHighwaterMemoryMetricsImpl::GetMetrics(AptHighwaterMemoryMetrics::TYPE_ZOMBIEVECTOR);
                    pMetrics->mHighwaterMaxCount = gpZombieVector->GetHighWaterValues();
#endif // APT_ENABLE_ZOMBIE_OUTPUT

					// FIFA_BEGIN | SENDBACK | mwebb | Mar 6 2015 | change to allow PrintZombieDump to be an int
					//#if defined(APT_ENABLE_ZOMBIE_OUTPUT)
					//if (bPrintZombieReferences)
					#if defined(APT_XML_MEMORY_DUMP_SUPPORTED)
                    if(iPrintZombieReferences > 1)
					// FIFA_END
                        AptGC::PrintZombieExternalReferenceMapXML(this);
#endif

                    if(gAptFuncs.pfnHandleZombieState != NULL)
                        gAptFuncs.pfnHandleZombieState(false, false, mMyName.c_str(), GetAnimationInst()->mpFile.Get()->GetName().c_str());
                    AptPartialGarbageCollection();
                    return;
                }
            }
            else
            {
                if(gpZombieVector != NULL)
                {
                    AptHighwaterMemoryMetrics* pMetrics  = AptHighwaterMemoryMetricsImpl::GetMetrics(AptHighwaterMemoryMetrics::TYPE_ZOMBIEVECTOR);
#if !defined(EA_DEBUG)
                    pMetrics->mHighwaterMaxCount = INT32_MAX;
#endif // EA_DEBUG
                    int32_t nPolicy = AptHighwaterMemoryMetrics::sOverflowCallback->OnOverflowDetected( *pMetrics, NULL, NULL );
                    if ( nPolicy & (int32_t)AptHighwaterMemoryMetrics::POLICY_APT_WILL_ASSERT )
                    {
                       APT_ASSERTM( (!gpZombieVector->IsVectorFull()), pMetrics->mHelpfulHint );
                    }
                }

                // Here, the zombie vector is full, so we simply print this message and allow Apt to work how it previously did.
                //  Of course this will break things so you should go and increase the zombie vector size.
                //APT_ASSERT(false && "ZOMBIE VECTOR IS FULL.  YOU SHOULD EITHER FIX YOUR SWF FILE OR INCREASE THE SIZE OF THE VECTOR");
                #if defined(APT_ENABLE_ZOMBIE_OUTPUT)
				// FIFA_BEGIN | SENDBACK | mwebb | Mar 6 2015 | change to allow PrintZombieDump to be an int
				//if (bPrintZombieReferences)
                if(iPrintZombieReferences > 0)
				// FIFA_END
                {
                    APT_DEBUGPRINT(APT_DEBUG_MSG_WARNING_LVL, "ZOMBIE DETECTED :: VECTOR FULL [name = %s  file = %s.swf] HAS %d EXTERNAL FUNCTION REFERENCES.  THERE IS NO SPACE LEFT TO KEEP TRACK THEREFOR IT WILL BE REMOVED FROM MEMORY\n",
						mMyName.c_str(), 
						GetAnimationInst()->mpFile.Get()->GetName().c_str(), 
						GetZombieCount());
				}
#endif
                SetCIHState(AptCIH::AptCIHState_Deleted);

				// FIFA_BEGIN | SENDBACK | mwebb | Mar 6 2015 | change to allow PrintZombieDump to be an int
				//#if defined(APT_ENABLE_ZOMBIE_OUTPUT)
				//if (bPrintZombieReferences)
                #if defined(APT_XML_MEMORY_DUMP_SUPPORTED)
                if(iPrintZombieReferences > 1)
				// FIFA_END
                    AptGC::PrintZombieExternalReferenceMapXML(this);
#endif
                if(gAptFuncs.pfnHandleZombieState != NULL)
                    gAptFuncs.pfnHandleZombieState(true, false, mMyName.c_str(), GetAnimationInst()->mpFile.Get()->GetName().c_str());
            }
        }
        // added next line in 0.15.05 to change the state on this CIH
        setGCRoot(0);

        if(bDestroyGC)
        {
            AptCharacterInst *pTemp = mpCharacterInst;
            mpCharacterInst = AptCharacterInst::CreateCharacterInst(NULL);

            mpCharacterInst->MoveRenderDataFrom(pTemp);
            // Don't call accessor function for this, it would think we are moving it.
            pTemp->DestroyGCPointers();
            delete pTemp;
        }
        else
        {
            AptCharacterInst *pTemp = mpCharacterInst;
            mpCharacterInst = NULL;
            pTemp->DestroyGCPointers();
            delete pTemp;
        }

    }

    SetASChanged(0);
    SetHasClass(0);         //  Super fix...
}

/*F************************************************************************************************/
/*!
    \Function    AptCIH::ForceCleanNativeHash

    \Description
        Cleans out the native hash

    \Input

    \Output

    \Notes
        This should only be called on AptCIH's which are having a new swf loaded into them

    \Version    1.0        11/04/05 (LEO)

*/
/************************************************************************************************F*/
void AptCIH::ForceCleanNativeHash()
{
    AptNativeHash *pTmpHash = GetCharacterInst()->GetNativeHash();
    if(pTmpHash != NULL)
    {
        pTmpHash->DestroyGCPointers();
        delete pTmpHash;
        pTmpHash = NULL;
        GetCharacterInst()->SetNativeHash(NULL);
    }
}

/*F************************************************************************************************/
/*!
    \Function    AptCIH::GetDisplayListState

    \Description
        returns the display list state of this object (or NULL if it has none).

    \Input

    \Output     AptDisplayListState * -   pointer to displaylist state object

    \Notes
        This is safe to call on any CIH object

    \Version    1.0        04/20/05 (DC) Bringin' it to ya

*/
/************************************************************************************************F*/
AptDisplayListState            *AptCIH::GetDisplayListState()
{
    if(IsSpriteInstBase())
    {
        return GetSpriteInstBase()->mDisplayList.pState;
    }
#if defined APT_USE_BUTTONS
    if(IsButtonInst())
    {
        return GetButtonInst()->mDisplayList.pState;
    }
#endif
    return NULL;
}
/*F************************************************************************************************/
/*!
    \Function    AptCIH::ReplaceChild

    \Description
        Replaces the a child CIH with another.

    \Input      *pNew             - new AptCIH object
    \Input      *pOld             - old AptCIH object that has to be replaced

    \Output

    \Notes
        None.

    \Version    1.0        04/20/05 (DC) Bringin' it to ya

*/
/************************************************************************************************F*/
void AptCIH::ReplaceChild( AptCIH *pNew,  AptCIH *pOld)
{
    AptDisplayListState * pState = GetDisplayListState();
    if(pState == NULL)
    {
        APT_ASSERT(false && "Attempting to operate on Display list of object with none!");
        return;
    }

    AptCIH *pPrevious = pOld->GetDisplayListPrevious();
    pNew->SetInstanceName(pOld->GetInstanceName());
    pNew->GetCharacterInst()->MoveRenderDataFrom(pOld->GetCharacterInst());

    pState->AddToDelayReleaseList(pOld, false);

    pState->insert(pPrevious, pNew);

    if(pNew->mMyName.IsEmpty() == false)
    {
        AptNativeHash *pHash = mpCharacterInst->GetNativeHash();
        if(pHash != NULL)
        {
            AptValue *pTmper = pHash->Lookup(&pOld->mMyName);
            if(pTmper == NULL || !pTmper->isCIH(true))
            {
                pHash->Set(&pOld->mMyName, pNew);
            }
        }
    }
}

/*F************************************************************************************************/
/*!
    \Function    AptCIH::ReplaceZombieChild

    \Description
        Same as RepalceChild, but for Zombie instances

    \Input      *pNew             - new AptCIH object
    \Input      *pOld             - old AptCIH object that has to be replaced

    \Output

    \Notes
        None.

    \Version    1.0        05/09/05 (LEO) First Version

*/
/************************************************************************************************F*/
void AptCIH::ReplaceZombieChild( AptCIH *pNew,  AptCIH *pOld)
{
    AptDisplayListState * pState = GetDisplayListState();
    if(pState == NULL)
    {
        APT_ASSERT(false && "Attempting to operate on Display list of object with none!");
        return;
    }

    AptCIH *pPrevious = pOld->GetDisplayListPrevious();
    pNew->SetInstanceName(pOld->GetInstanceName());
    pNew->GetCharacterInst()->CopyRenderDataFrom(pOld->GetCharacterInst());

    pState->AddToDelayReleaseList(pOld, false);

    pState->insert(pPrevious, pNew);
}

/*F************************************************************************************************/
/*!
    \Function    AptCIH::InsertChild

    \Description
        Inserts a newly created CIH object based on the passed character.

        it is added with the passed depth / instance name, and using the passed init object if provided.

    \Input      pSourceCIH        - source AptCIH in which to insert new child.
    \Input      pNew              - AptCharacter associated with newly created AptCIH
    \Input      nDepth            - depth at which to insert the child in displaylist
    \Input      sInstanceName     - name of new AptCIH instance
    \Input      pInitObject       - pointer to AptObject for initialization values

    \Output     AptCIH *          - newly created AptCIH

    \Notes
        None.

    \Version    1.0        04/20/05 (DC) Bringin' it to ya

*/
/************************************************************************************************F*/
AptCIH * AptCIH::InsertChild(  AptCIH *pSourceCIH, AptCharacter *pNew,  int     nDepth, AptNativeString * sInstanceName, AptValue * pInitObject)
{
    //AptDisplayListState * pState = GetDisplayListState();
    //if(pState == NULL)
    //{
    //    APT_ASSERT(false && "Attempting to operate on Display list of object with none!");
    //    return NULL;
    //}
    //AptCIH * pInserted = pState->insert(nDepth, new AptCIH( pNew, this));

    AptCIH * pInserted = GetSpriteInstBase()->mDisplayList.placeObject(
        NULL,
        nDepth,
        pNew,
        sInstanceName,
        this,
        1,
        -1,
        NULL,
        NULL,
        (pSourceCIH != NULL) ? pSourceCIH->GetSpriteInstBase()->mpClipActions : NULL,
        0.f,
        pInitObject);

    return pInserted;
}


/*F************************************************************************************************/
/*!
    \Function    AptCIH::RemoveChild

    \Description
        Removes the passed child from this display list.

    \Input      pToRemove        - Pointer to AptCIH that is removed from this AptCIH

    \Output
        None.

    \Notes
        None.

    \Version    1.0        04/20/05 (DC) Bringin' it to ya

*/
/************************************************************************************************F*/
void AptCIH::RemoveChild(  AptCIH *pToRemove )
{
    AptDisplayListState * pState = GetDisplayListState();
    if(pState == NULL)
    {
        APT_ASSERT(false && "Attempting to operate on Display list of object with none!");
        return;
    }
    pState->remove(pToRemove);
}
/*F************************************************************************************************/
/*!
    \Function    AptCIH::SwapChildrenDepths

    \Description
        Swaps the depths and list locations of the passed objects.

    \Input      *p0               - pointer to first AptCIH
    \Input      *p1               - pointer to second AptCIH

    \Output
        None.

    \Notes
        None.

    \Version    1.0        04/20/05 (DC) Bringin' it to ya

*/
/************************************************************************************************F*/
void AptCIH::SwapChildrenDepths( AptCIH *p0, AptCIH *p1 )
{
    AptDisplayListState * pState = GetDisplayListState();
    if(pState == NULL)
    {
        APT_ASSERT(false && "Attempting to operate on Display list of object with none!");
        return;
    }
    pState->swapDepths(p0, p1);
}

/*F************************************************************************************************/
/*!
    \Function AptCIH::RegisterReferences

    \Descrption
    Gets the Garbage Collected pointer given an index.
    You need to call before GetNumGCPointers to receive the number of available pointer.

    \Input      nIndex            - The index of the GC pointer.

    \Output     AptValue *        - The GC pointer.

    \Notes
        This method should not be called directly by the user, only the Garbage Collector should call it.

    \Version
        1.0        03/26/04 (ONA) First Version

*/
/************************************************************************************************F*/
void    AptCIH::RegisterReferences()
{
    if( APT_REFERENCES_REGISTERED(this) )
        return;

    //No Need to call parent, there is none for now.
    //AptValueGC::RegisterReferences();

    AptNativeHash *pNativeHash = GetNativeHash();

    APT_REGISTER_REFERENCE_SAFE(mpParent,     "Parent", APT_REFREG_IS_APTCIH);

    if(pNativeHash)
    {
        pNativeHash->RegisterReferences(this);
    }

    if(mpCharacterInst)
    {
        if (mpCharacterInst->IsSpriteInstBase())
        {
            AptCharacterSpriteInstBase * pChar = GetSpriteInstBase();
            AptDisplayListState * pDisp = pChar->mDisplayList.getState();
            APT_ASSERT(pDisp);
            pDisp->RegisterReferences(this);
        }
#if defined APT_USE_BUTTONS
        else if (mpCharacterInst->IsButtonInst())
        {
            AptCharacterButtonInst * pChar = GetButtonInst();
            AptDisplayListState * pDisp = pChar->mDisplayList.getState();
            APT_ASSERT(pDisp);
            pDisp->RegisterReferences(this);
        }
#endif
    }
    return;
}

/*F************************************************************************************************/
/*!
    \Function AptCIH::GetNativeHashVirtual

    \Descrption
    Gets the AptNativeHash of this instance with a virtual call.

    \Input

    \Output     AptNativeHash *   - The AptNativeHash.

    \Notes
        This method is virtual. You should call the static method instead of this one.
        In fact, you should try to compile with the static method, if it doesn't work, use this one.

    \Version
        1.0        03/07/04 (ONA) First Version

*/
/************************************************************************************************F*/
AptNativeHash * AptCIH::GetNativeHashVirtual()
{
    return  (GetNativeHash());
}

/*F************************************************************************************************/
/*!
    \Function AptCIH::ContainsNativeHashVirtual

    \Descrption
    Tells if this instance contains an AptNativeHash.

    \Input

    \Output     bool              - true if this instance contains an AptNativeHash.
                                    false otherwise.

    \Notes
        This method is virtual.

    \Version
        1.0        03/08/04 (ONA) First Version

*/
/************************************************************************************************F*/
bool    AptCIH::ContainsNativeHashVirtual() const
{
    return  (((AptCIH *)this)->GetNativeHash() != NULL);
}

/*F************************************************************************************************/
/*!
    \Function AptCIH::objectMemberLookup

    \Descrption
    Returns the AptValue for the context and the member name.

    \Input      pContext          - The context passed in.
    \Input      pName             - The member name.

    \Output     AptValue *        - The AptValue of the member.
                                    gpUndefinedValue is the member is unknown.

    \Notes
        None.

    \Version
        1.0        03/26/04 (ONA) First Version
    \Version    1.1        12/12/05 (LEO) Added Apt_3D code
    \Version    1.2        12/15/05 (LEO) Added _renderflags support
    \Version    1.3        10/05/07 (ASK) Added blendmode and filter support
    \Version    1.4        04/29/08 (RB)  Don't do shallow copy when cloning filter properties
    \Version    1.5        11/11/09 (RB)  Removed support for setInterval and getInterval

*/
/************************************************************************************************F*/
AptValue * AptCIH::objectMemberLookup(AptValue * const pContext, const AptNativeString * const pName) const
{
    if(pContext->c_cih()->IsNone())    // (LEO) simply return undefined if this AptCIH is the gpUndefined AptCIH
    {
        return gpUndefinedValue;
    }
    if (pContext->c_cih()->IsDynamicTextInst())
    {
        // If AptCharacterInst is a TextInst, first check text properties
        TextMembers *pProp = TextMembersIndex::in_word_set(pName->c_str(), pName->Size());
        if (pProp)
        {
            AptCharacterTextInst *pTextInst = ((AptCIH*)pContext)->GetDynamicTextInst();
            switch(pProp->nIndex)
            {
            case AptTextPropertyautoSize:
                {
                    AptString *pTemp = AptString::Create();
                    switch(pTextInst->GetBoxAlignment())
                    {
                    case AptStringAlignment_Left:
                        {
                            pTemp->cpy(StringPool::GetString(SC_left));
                        }
                        break;
                        case AptStringAlignment_Right:

                        {
                            pTemp->cpy(StringPool::GetString(SC_right));
                        }
                        break;
                        case AptStringAlignment_Center:

                        {
                            pTemp->cpy(StringPool::GetString(SC_center));
                        }
                        break;
                        case AptStringAlignment_None:

                        {
                            pTemp->cpy(StringPool::GetString(SC_none));
                        }
                        break;
                        default:
                        {
                            pTemp->cpy(StringPool::GetString(SC_none));
                        }
                    }
                    return pTemp;
                }
                break;
                case AptTextPropertybackground:

                {
                    return AptBoolean::Create(pTextInst->GetDrawsBackground());
                }
                break;
                case AptTextPropertybackgroundColor:

                {
                    return AptInteger::Create(pTextInst->GetBackgroundColor() & 0x00ffffff);
                }
                break;
                case AptTextPropertyborder:

                {
                    return AptBoolean::Create(pTextInst->GetDrawsBorder() != 0);
                }
                break;
                case AptTextPropertyborderColor:

                {
                    return AptInteger::Create(pTextInst->GetBorderColor() & 0x00ffffff);
                }

                break;
                case AptTextPropertylength:

                {
                    AptCIH *    pCIH = pContext->c_cih();
                    pTextInst->UpdateText(pCIH);
                    return AptInteger::Create(pTextInst->GetTextValueConst().Size());
                }
                break;
                case AptTextPropertymaxChars:

                {
                    return gpUndefinedValue;
                    //return new AptInteger(pTextInst->nMaxChars);
                }
                break;
                case AptTextPropertymaxscroll:

                {
                    // 0.15.05 - bug fix for #502 maxscroll not being updated fast enough
                    if(pTextInst->GetStateFlags() & (APT_TEXTFIELD_DIRTY))
                    {
                        pContext->c_cih()->EnsureStringAllocated(pContext->c_cih()->GetDisplayListParent());
                    }
                    //
                    return AptInteger::Create(pTextInst->GetMaxScroll());
                    //
                }
                break;
                case AptTextPropertymultiline:

                {
                    return AptBoolean::Create(pTextInst->GetMultiline());
                }
                break;
                case AptTextPropertyscroll:

                {
                    // 0.15.05 - bug fix for #502 maxscroll not being updated fast enough
                    if(pTextInst->GetStateFlags() & (APT_TEXTFIELD_DIRTY))
                    {
                        pContext->c_cih()->EnsureStringAllocated(pContext->c_cih()->GetDisplayListParent());
                    }
                    //
                    return AptInteger::Create(pTextInst->GetScroll());
                    //
                }
                break;
                case AptTextPropertytext:

                {
                    AptCIH *    pCIH = pContext->c_cih();
                    pTextInst->UpdateText(pCIH);
                    AptString * pString = AptString::Create();
                    pString->str = pTextInst->GetTextValueConst();
                    return  (pString);
                }
                break;
                case AptTextPropertytextColor:

                {
                    //return new AptInteger(pTextInst->GetCharacterConst(gnCurrUpdateTick)->text.nColour & 0xffffff);
                    return AptInteger::Create(pTextInst->GetTextColor() & 0xffffff);       // switched back to textInst->nColour in 0.11.00
                }

                break;
                case AptTextPropertytextHeight:

                {
                    if(pTextInst->GetTextValueConst().IsEmpty())
                    {
                        return AptFloat::Create(0.f);
                    }
                    if(pTextInst->GetStateFlags() & (APT_TEXTFIELD_DIRTY))
                    {
                        pContext->c_cih()->EnsureStringAllocated(pContext->c_cih()->GetDisplayListParent());
                    }
                    return AptFloat::Create(pTextInst->GetTextHeight());        // removed +4
                }
                break;
                case AptTextPropertytextWidth:

                {
                    if(pTextInst->GetTextValueConst().IsEmpty())
                    {
                        return AptFloat::Create(0.f);
                    }
                    if(pTextInst->GetStateFlags() & (APT_TEXTFIELD_TEXTWIDTH))
                    {
                        pContext->c_cih()->EnsureStringAllocated(pContext->c_cih()->GetDisplayListParent());
                    }
                    return AptFloat::Create(pTextInst->GetTextWidth());         // removed +4
                }
                break;
                case AptTextPropertytype:

                {
                    // Apt does not support input TextFields so just return dynamic
                    AptString *pTemp = AptString::Create();
                    pTemp->cpy("dynamic");
                    return pTemp;
                }
                break;
                case AptTextPropertyvariable:

                {
                    if (pTextInst->GetVarValueConst().IsEmpty() == false)
                    {
                        AptString * pString = AptString::Create();
                        pString->str = pTextInst->GetVarValueConst();
                        return  (pString);
                    }
                    return  (gpUndefinedValue);
                }
                break;
                case AptTextPropertywordWrap:

                {
                    return AptBoolean::Create(pTextInst->GetWordWrap());
                }
                break;
                case AptTextProperty_height:

                {
                    if(pTextInst->GetStateFlags() & (APT_TEXTFIELD_DIRTY))
                    {
                        pContext->c_cih()->EnsureStringAllocated(pContext->c_cih()->GetDisplayListParent());
                    }
                    float fFloat = 0.0f;
                    bool bWordWrap = pTextInst->GetWordWrap();
                    if ( pTextInst->GetBoxAlignment() == AptStringAlignment_None || bWordWrap )
                    {
                        //$bug1046 - RB - 02/20/2006 - return original bounding box
                        AptRect rect;
                        pContext->c_cih()->GetBoundingRect(&rect);
                        fFloat = rect.fBottom - rect.fTop;
                        fFloat = (fFloat < 0) ? 0 : fFloat;
                    }
                    else
                    {
                        // (RB) i.e. actual_text_height + (2*gutter_height)
                        fFloat = pTextInst->GetTextHeight() + 4.0f;
                    }
                    return AptFloat::Create(fFloat);
                }
                break;
                case AptTextProperty_width:

                {
                    if(pTextInst->GetStateFlags() & (APT_TEXTFIELD_DIRTY))
                    {
                        pContext->c_cih()->EnsureStringAllocated(pContext->c_cih()->GetDisplayListParent());
                    }

                    float fFloat = 0.0f;
                    bool bWordWrap = pTextInst->GetWordWrap();
                    if ( pTextInst->GetBoxAlignment() == AptStringAlignment_None || bWordWrap )
                    {
                        //$bug1046 - RB - 02/20/2006 - return original bounding box
                        AptRect rect;
                        pContext->c_cih()->GetBoundingRect(&rect);
                        fFloat = rect.fRight - rect.fLeft;
                        fFloat = (fFloat < 0) ? 0 : fFloat;
                    }
                    else
                    {
                        // return text limits
                        // (RB) i.e. actual_text_width + (2*gutter_width)
                        fFloat = pTextInst->GetTextWidth() + 4.0f;
                    }
                    return AptFloat::Create(fFloat);
                }
                break;
                case AptTextPropertymouseWheelEnabled:      // (LEO) added support for TextMouseWheel   8/
                {
                    return AptBoolean::Create(pTextInst->GetMouseWheelEnabled());
                }
            }
        }
    }

    SpriteMembers *pProp = pContext && pContext->isCIH() ? SpriteMembersIndex::in_word_set(pName->c_str(), pName->Size()) : NULL;
    if (pProp)
    {
        APT_ASSERT(pContext);
        AptCIH *pCIH = pContext->c_cih();
        AptCharacterInst *pCharInst = pCIH->mpCharacterInst;
        float fFloat;
        switch(pProp->nIndex)
        {
        case AptPropertyNumber_x:
            {
                // $Bug557 - DC - 1 of 1 - _x changes when the autosize parameter is not "None" or "Left" then the
                // _x value will change if the text field is dirty. We must re-allocate and get the new _x
                // value.
                if(pCIH->IsDynamicTextInst())
                {
                    AptCharacterTextInst *pTextInst = pCIH->GetDynamicTextInst();

                    if( pTextInst->GetBoxAlignment() != AptStringAlignment_None &&
                        pTextInst->GetBoxAlignment() != AptStringAlignment_Left &&
                        pTextInst->GetStateFlags() & (APT_TEXTFIELD_DIRTY))
                    {
                        pCIH->EnsureStringAllocated(pCIH->GetDisplayListParent());
                    }
                }
                fFloat =  pCIH->GetProceduralProperty(AptProceduralProperty_X);
                // $bug898 PDS [ #898 ] Shift of text and keyboard issue
                // flash seems to add the bounding box left and top differently based on
                // the box alignment.
                if(pCIH->IsDynamicTextInst())
                {
                    float fLeft = pCIH->GetDynamicTextInst()->GetBoundsConst().fLeft;
                    fFloat += fLeft;
                }
            }

            break;
            case AptPropertyNumber_y:

            {
                fFloat =  pCIH->GetProceduralProperty(AptProceduralProperty_Y);
                if(pCIH->IsDynamicTextInst())
                {
                    // $bug898 PDS [ #898 ] Shift of text and keyboard issue
                    fFloat += pCIH->GetDynamicTextInst()->GetBoundsConst().fTop;
                }
            }

            break;
#if defined(APT_3D)
            case AptPropertyNumber_z:
            {
                fFloat =  pCIH->GetProceduralProperty(AptProceduralProperty_Z) * APTCIH_Z_MULTIPLIER;
            }
            break;
            case AptPropertyNumber_zscale:
            {
                fFloat =  pCIH->GetProceduralProperty(AptProceduralProperty_ZScale);
            }
            break;
            case AptPropertyNumber_xrotation:
            {
                fFloat = pCIH->GetProceduralProperty(AptProceduralProperty_XRot) * APTCIH_Z_MULTIPLIER;
            }
            break;
            case AptPropertyNumber_yrotation:
            {
                fFloat = pCIH->GetProceduralProperty(AptProceduralProperty_YRot) * APTCIH_Z_MULTIPLIER;
            }
            break;
#endif
#if defined(APT_RENDER_FLAGS)
            case AptSpriteMethod__renderflags:
            {
                AptRenderItemSprite *pRenderItem = (AptRenderItemSprite *)pCIH->GetSpriteInstBase()->GetRenderItemWritable();
                APT_ASSERT(pRenderItem != NULL && "NULL RENDER ITEMS NOT GOOD");

                AptString * pString = AptString::Create();
                pString->str = pRenderItem->GetRenderPropertiesStr();
                return pString ;
            }
            break ;
#endif

            case AptPropertyNumber_alpha:
            {
                fFloat =  pCIH->GetProceduralProperty(AptProceduralProperty_Alpha);
            }
            break;

            case AptPropertyNumber_visible:
            {
                return AptBoolean::Create(pCIH->GetVisible() ? 1 : 0);
                }

            case AptPropertyNumber_currentframe:
            {
                AptCharacterSpriteInstBase *pSprInst = pCIH->GetSpriteInstBase();
                fFloat = (float)(pSprInst->mnFrame + 1);
            }
            break;

            case AptPropertyNumber_parseInt:
            {
                return gpCBparseInt;
            }

            case AptPropertyNumber_parseFloat:
            {
                return gpCBparseFloat;
            }
            break;

            case AptPropertyNumberisNaN:

            {
                return gpCBisNaN ;
            }
            break;
            case AptPropertyNumberunescape:

            {
                return gpCBunescape ;
            }
            break;
            case AptPropertyNumberescape:

            {
                return gpCBescape ;
            }
            break;
            case AptPropertyNumber_boolean:

            {
                return gpCBboolean ;
            }
            break;
            case AptPropertyNumber_target:

            {
                AptNativeString sBuf;
                AptActionInterpreter::getName2(pCIH, sBuf);
                AptString *pRet = AptString::Create();
                pRet->str = sBuf;
                return pRet;
            }

            break;
            case AptPropertyNumber_width:

            {
                fFloat =  pCIH->GetProceduralProperty(AptProceduralProperty_Width);
            }

            break;
            case AptPropertyNumber_height:

            {
                fFloat =  pCIH->GetProceduralProperty(AptProceduralProperty_Height);
            }

            break;
            case AptPropertyNumber_xscale:

            {
                fFloat =  pCIH->GetProceduralProperty(AptProceduralProperty_XScale);
            }

            break;
            case AptPropertyNumber_yscale:

            {
                fFloat = pCIH->GetProceduralProperty(AptProceduralProperty_YScale);
            }

            break;
            case AptPropertyNumber_rotation:

            {
                fFloat = pCIH->GetProceduralProperty(AptProceduralProperty_Rotation);
            }

            break;
            case  AptPropertyNumber_filters:
            {
                if (pCharInst->IsSpriteInstBase() || pCharInst->IsDynamicTextInst() || pCharInst->IsButtonInst())
                {
                    AptNativeHash * pHash = pCIH->GetNativeHashVirtual();
                    if (pHash == NULL && pCharInst->IsDynamicTextInst())
                    {
                        // dynamic text instances do not have hash table in them created at init time like spriteinsts
                        // so create a new one only when filters are accessed.
                        pCharInst->SetNativeHash(new AptNativeHash(APT_OBJECTHASHSIZE));
                    }

                    AptValue* pFilters = pCIH->GetNativeHashVirtual()->Lookup( StringPool::GetString(SC_filters) );
                    if ( pFilters == NULL || pFilters == gpUndefinedValue) // lazy initialization
                    {
                        pFilters = (AptValue *) new AptArray();  // create empty AptArray
                        pCIH->GetNativeHashVirtual()->Set( StringPool::GetString(SC_filters), pFilters );
                    }
                    // Now we clone pFilters, because the user always modifies a copy
                    AptArray * pFilterCopy = new AptArray();
                    int nFilterArrLen = pFilters->c_array()->length();
                    for (int i = 0; i < nFilterArrLen; i++)
                    {
                        AptValue * pValue = pFilters->c_array()->GetAt(i);
                        // now make a copy of this object
                        APT_ASSERT(pValue->isObject()); // because all filters are AptObjects inside array
                        AptObject * pNewObject = pValue->c_object()->Clone(true);
                        pFilterCopy->set(i, pNewObject);
                    }
                    return  (AptValue*) pFilterCopy;    // copy of pFilters;
                }
                else
                {
                    // If you hit this, then perhaps we need to support this for Image insts?  If so,
                    // we'll need to check the above  code and make sure it will work for Images as well.
                    // This includes determining if Images have a hash associated with them, or if they need
                    // to be handled more like Dynamic Text insts in the above code - Colin C. 3/16/12
                    APT_FAIL("filters property is supported only for Movieclips, Text, Buttons")
                    return (gpUndefinedValue);
                }
            }

            break;
            case  AptPropertyNumber_blendMode:
                {
                    uint32_t blendMode = GetBlendMode(pContext);

                    if (SC_LAST != blendMode)
                    {
                        // use blendMode (1-based) as index into the StringPool
                        if (blendMode < (SC_BlendHardlight - SC_BlendNormal) + 2 && blendMode > (SC_BlendNormal - SC_BlendNormal) )
                        {
                            AptString * pReturnString = AptString::Create();
                            pReturnString->cpy(StringPool::GetString((StringCode) (SC_BlendNormal + (blendMode - 1)) ));
                            return pReturnString;
                        }
                        else
                        {
                            return (AptInteger::Create(blendMode));
                        }
                    }
                    else
                    {
                        // failed to get a good blend mode
                        AptString * pReturnString = AptString::Create();
                        pReturnString->cpy(StringPool::GetString(SC_BlendNormal) );
                        return pReturnString;
                    }
                }

            break;
            case AptPropertyNumber_totalframes:
            {
                fFloat = (float)pCharInst->GetCharacterConst()->sprite.movie.nFrames;
            }

            break;
            case AptPropertyNumber_framesloaded:

            {
                fFloat = (float)pCharInst->GetCharacterConst()->sprite.movie.nFrames;
            }

            break;
            case AptPropertyNumber_xmouse:

            {
                AptMatrix cur = gIdentityMatrix;
                MultParentMatrix(pCIH, cur);
                float nx = (float)GetTargetSim()->GetAnimationTarget()->GetXMousePos() - cur.tx;      // Translate then rotate mouse position
                float ny = (float)GetTargetSim()->GetAnimationTarget()->GetYMousePos() - cur.ty;
                fFloat = nx * cur.a - ny * cur.b;
            }

            break;
            case AptPropertyNumber_ymouse:

            {
                AptMatrix cur = gIdentityMatrix;
                MultParentMatrix(pCIH, cur);
                float nx = (float)GetTargetSim()->GetAnimationTarget()->GetXMousePos() - cur.tx;      // Translate then rotate mouse position
                float ny = (float)GetTargetSim()->GetAnimationTarget()->GetYMousePos() - cur.ty;
                fFloat = nx * cur.c + ny * cur.d;
            }

            break;

            case AptPropertyNumber_parent:
            {
                return pCIH->GetDisplayListParent();
            }
            break;

            case AptSpriteMethod_hitTest:

            {
                APT_CIH_NATIVE_MEMBER_FUNCTION_DISPATCH(hitTest);
            }
            break;
            case AptSpriteMethod_startDrag:

            {
                APT_CIH_NATIVE_MEMBER_FUNCTION_DISPATCH(startDrag);
            }
            break;
            case AptSpriteMethod_loadVariables:

            {
                APT_CIH_NATIVE_MEMBER_FUNCTION_DISPATCH(loadVariables);
            }

            break;
            case AptSpriteMethod_gotoAndPlay:

            {
                APT_CIH_NATIVE_MEMBER_FUNCTION_DISPATCH(gotoAndPlay);
            }

            break;
            case AptSpriteMethod_gotoAndStop:

            {
                APT_CIH_NATIVE_MEMBER_FUNCTION_DISPATCH(gotoAndStop);
            }

            break;
            case AptSpriteMethod_attachMovie:

            {
                APT_CIH_NATIVE_MEMBER_FUNCTION_DISPATCH(attachMovie);
            }

            break;
            case AptSpriteMethod_play:

            {
                APT_CIH_NATIVE_MEMBER_FUNCTION_DISPATCH(play);
            }

            break;
            case AptSpriteMethod_stop:

            {
                APT_CIH_NATIVE_MEMBER_FUNCTION_DISPATCH(stop);
            }

            break;
            case AptSpriteMethod_prevFrame:

            {
                APT_CIH_NATIVE_MEMBER_FUNCTION_DISPATCH(prevFrame);
            }

            break;
            case AptSpriteMethod_nextFrame:

            {
                APT_CIH_NATIVE_MEMBER_FUNCTION_DISPATCH(nextFrame);
            }

            break;
            case AptSpriteMethod_getBytesTotal:

            {
                APT_CIH_NATIVE_MEMBER_FUNCTION_DISPATCH(getBytesTotal);
                //              fFloat = (float)1024;
            }

            break;
            case AptSpriteMethod_getBytesLoaded:

            {
                APT_CIH_NATIVE_MEMBER_FUNCTION_DISPATCH(getBytesLoaded);
                //              fFloat = (float)1024;
            }
            break;
            case AptSpriteMethod_duplicateMovieClip:

            {
                APT_CIH_NATIVE_MEMBER_FUNCTION_DISPATCH(duplicateMovieClip) ;
            }

            break;
            case AptSpriteMethod_removeMovieClip:

            {
                APT_CIH_NATIVE_MEMBER_FUNCTION_DISPATCH(removeMovieClip) ;
            }
            break;
            case AptSpriteMethod_loadMovie:

            {
                APT_CIH_NATIVE_MEMBER_FUNCTION_DISPATCH(loadMovie) ;
            }
            break;
            case AptSpriteMethod_unloadMovie:

            {
                APT_CIH_NATIVE_MEMBER_FUNCTION_DISPATCH(unloadMovie) ;
            }

            break;

            case AptSpriteMethod_loadMovieNum:

            {
                APT_CIH_NATIVE_MEMBER_FUNCTION_DISPATCH(loadMovieNum) ;
            }
            break;
            case AptSpriteMethod_unloadMovieNum:

            {
                APT_CIH_NATIVE_MEMBER_FUNCTION_DISPATCH(unloadMovieNum) ;
            }

            break;

            case AptSpriteMethod_createTextField:

            {
                APT_CIH_NATIVE_MEMBER_FUNCTION_DISPATCH(createTextField);
            }
            break;
            case AptSpriteMethod_removeTextField:

            {
                APT_CIH_NATIVE_MEMBER_FUNCTION_DISPATCH(removeTextField);
            }
            break;
            case AptSpriteMethod_getDepth:

            {
                APT_CIH_NATIVE_MEMBER_FUNCTION_DISPATCH(getDepth);
            }
            break;
            case AptSpriteMethod_getInstanceAtDepth:

            {
                APT_CIH_NATIVE_MEMBER_FUNCTION_DISPATCH(getInstanceAtDepth);
            }
            break;
            case AptSpriteMethod_getBounds:

            {
                APT_CIH_NATIVE_MEMBER_FUNCTION_DISPATCH(getBounds);
            }
            break;
            case AptSpriteMethod_createEmptyMovieClip:

            {
                APT_CIH_NATIVE_MEMBER_FUNCTION_DISPATCH(createEmptyMovieClip);
            }
            break;
            case AptSpriteMethod_swapDepths:

            {
                APT_CIH_NATIVE_MEMBER_FUNCTION_DISPATCH(swapDepths);
            }
            break;
            case AptSpriteMethod_setMask:

            {
                APT_CIH_NATIVE_MEMBER_FUNCTION_DISPATCH(setMask);
            }
            break;
            case AptSpriteMethod_localToGlobal:

            {
                APT_CIH_NATIVE_MEMBER_FUNCTION_DISPATCH(localToGlobal);
            }
            break;
            case AptSpriteMethod_globalToLocal:
            {
                APT_CIH_NATIVE_MEMBER_FUNCTION_DISPATCH(globalToLocal);
            }
            break;

            case AptPropertyNumber_skipeval:
            {
                AptBoolean *pBoolean = AptBoolean::Create(pCIH->IsSkipEval());
                return pBoolean;
            }
            break;

            case AptPropertyNumber_CustomControlType:
            {
                // ion_common and customrenderhandlers look in the hash table
                // to find _CustomControlType, so even though we made
                // _CustomControlType an intrinsic, we still store it there
                // for backwards compatibility
                return pCIH->GetNativeHashVirtual()->Lookup(StringPool::GetString(SC__CustomControlType));
            }
            break;

            case AptPropertyNumber_name:

            {
                AptString * pString = AptString::Create();
                pString->str = pCIH->GetInstanceName();
                return  (pString);
            }

            break;
            case AptPropertyNumber_url:

            {
                AptString *pTemp = AptString::Create();
                AptNativeString sBuf;
                APT_ASSERT(pContext);
                AptActionInterpreter::getName(pContext->c_cih(), sBuf);         // ### LEO should use getName2???
                pTemp->cpy(sBuf);
                return pTemp;
            }
            break;
            case AptSpriteMethod_getTextFormat:

            {
                APT_CIH_NATIVE_MEMBER_FUNCTION_DISPATCH(getTextFormat);
            }
            break;
            case AptSpriteMethod_getNewTextFormat:

            {
                APT_CIH_NATIVE_MEMBER_FUNCTION_DISPATCH(getNewTextFormat);
            }
            break;
            case AptSpriteMethod_setTextFormat:

            {
                APT_CIH_NATIVE_MEMBER_FUNCTION_DISPATCH(setTextFormat);
            }
            break;
#if defined(EA_PLATFORM_WINDOWS) || (defined(EA_PLATFORM_MICROSOFT) && defined(EA_PLATFORM_CONSOLE) && !defined(EA_PLATFORM_XENON))
            case AptPropertyNumber_droptarget:
            {
                AptString * pString = AptString::Create();
                if(pCIH->IsSpriteInst())
                {
                    pString->str = pCIH->GetSpriteInst()->GetDropTarget();
                }
                return  (pString);
            }
#endif
            default:
            {
                return NULL;
                //APT_ASSERT(NOT_REACHED);          // ### LEO this was removed since objectMemberLookup is now called first from GetVariable....
            }
        }
        return AptFloat::Create(fFloat);
    }
    else
    {
        return 0;
    }
}

/*F************************************************************************************************/
/*!
    \Function GetBlendModeValue

    \Descrption
        Returns blendMode number for string literal passed in

    \Input      pString          - pointer to AptNativeString that contains blendMode name

    \Output     uint32_t         - blendMode number if string matches one of the 14 blendmodes
                                       otherwise 1
    \Notes
        Called from ObjectMemberSet function for case when value is of string type

    \Version
        1.0        10/15/07 (ASK) First Version
*/
/************************************************************************************************F*/
static uint32_t GetBlendModeValue(AptNativeString * pString)
{
    // this is dependent on the stringpool having the blendmode strings init
    uint32_t nRetBlendMode = 1;

    for (int sc = SC_BlendNormal; sc <= SC_BlendHardlight; sc++)
    {
        if (pString->CompareNoCase(*StringPool::GetString((StringCode)sc)) == 0)
        {
            nRetBlendMode = (sc - SC_BlendNormal) + 1;
            return nRetBlendMode;

        }
    }
    return nRetBlendMode;
}

/*F************************************************************************************************/
/*!
    \Function AptCIH::objectMemberSet

    \Descrption
    Sets the AptValue for the context and the member name.

    \Input      pContext          - The context passed in.
    \Input      pName             - The member name.
    \Input      pValue            - The value to set.

    \Output     bool              - true if the member was succesfully set.
                                    false otherwise (if not an index).

    \Notes
        None.

    \Version
        1.0        03/26/04 (ONA) First Version
    \Version    1.1        12/12/05 (LEO) Added Apt_3D code
    \Version    1.2        12/15/05 (LEO) Added _renderflags support
    \Version    1.3        10/05/07 (ASK) Added blendmode and filter support
    \Version    1.4        06/09/09 (DG)  FogBug 1233: AptNativeHash change from Tiger10.
*/
/************************************************************************************************F*/
bool    AptCIH::objectMemberSet(AptValue * const pContext, const AptNativeString * const pName, AptValue * const pValue)
{
    if (pContext->c_cih()->IsDynamicTextInst())
    {
        TextMembers *pProp = TextMembersIndex::in_word_set(pName->c_str(), pName->Size());
        if (pProp)
        {
            AptCharacterTextInst *pTextInst = pContext->c_cih()->GetDynamicTextInst();
            switch(pProp->nIndex)
            {
            case AptTextPropertyautoSize:
                {
                    AptNativeString szBuf;
                    pValue->toString(szBuf);
                    szBuf.MakeLower(); // $internal720 - MCB - Matched Flash behavior by making string compares case insensitive.

                    /// ### LEO updated for flags
                    if(pTextInst->GetBoxAlignment() == AptStringAlignment_None &&
                        (szBuf != *StringPool::GetString(SC_false) || szBuf != *StringPool::GetString(SC_none)) )
                    {
                        pTextInst->SetStateFlags( APT_TEXTFIELD_AUTOSIZE );
                    }
                    else
                    {
                        pTextInst->SetStateFlags( APT_TEXTFIELD_ASCHANGE );
                    }

                    if(szBuf == *StringPool::GetString(SC_left) || szBuf == *StringPool::GetString(SC_true))
                    {
                        pTextInst->SetBoxAlignment( AptStringAlignment_Left );
                    }
                    else if(szBuf == *StringPool::GetString(SC_center))
                    {
                        pTextInst->SetBoxAlignment(AptStringAlignment_Center );
                    }
                    else if(szBuf == *StringPool::GetString(SC_right))
                    {
                        pTextInst->SetBoxAlignment( AptStringAlignment_Right );
                    }
                    else if(szBuf == *StringPool::GetString(SC_false) || szBuf == *StringPool::GetString(SC_none))
                    {
                        pTextInst->SetBoxAlignment( AptStringAlignment_None );
                    }
                    pTextInst->ClearStateFlags(APT_TEXTFIELD_NONE);
                    pTextInst->SetStateFlags( APT_TEXTFIELD_DIRTY );
                    return  (true);
                }
                break;
                case AptTextPropertybackground:

                {
                    bool nVal = pValue->toBool();
                    pTextInst->SetDrawsBackground((bool) nVal );
                    pTextInst->ClearStateFlags(APT_TEXTFIELD_NONE);
                    pTextInst->SetStateFlags( APT_TEXTFIELD_BACKGROUND );
                    return  (true);
                }
                break;
                case AptTextPropertybackgroundColor:

                {
                    pTextInst->SetBackgroundColor( pValue->toInteger() );
                    pTextInst->ClearStateFlags(APT_TEXTFIELD_NONE);
                    pTextInst->SetStateFlags( APT_TEXTFIELD_BGCOLOR );
                    return  (true);
                }
                break;
                case AptTextPropertyborder:

                {
                    bool nVal = pValue->toBool();
                    pTextInst->SetDrawsBorder( (bool)nVal );
                    pTextInst->ClearStateFlags(APT_TEXTFIELD_NONE);
                    pTextInst->SetStateFlags( APT_TEXTFIELD_BORDER );
                    return  (true);
                }
                break;
                case AptTextPropertyborderColor:

                {
                    pTextInst->SetBorderColor( pValue->toInteger() );
                    pTextInst->ClearStateFlags(APT_TEXTFIELD_NONE);
                    pTextInst->SetStateFlags( APT_TEXTFIELD_BORDERCOLOR );
                    return  (true);
                }
                break;
                case AptTextPropertyhscroll:

                {
                    // Indicates the horizontal scroll value of a text field.
                    return  (true);
                }
                break;
                case AptTextPropertymaxChars:

                {
                    // The maximum number of characters that a text field can contain. Only used for input,
                    //  there is no restriction to num chars done in actionscript
                    return  (true);
                }
                break;
                case AptTextPropertymultiline:

                {
                    bool bVal = pValue->toBool();
                    pTextInst->SetMultiline(bVal);
                }
                break;
                case AptTextPropertyscroll:

                {
                    int nNewScroll = pValue->toInteger();
                    uint32_t nOldScroll = pTextInst->GetScroll();

                    // DC -16.1/17.0- Must reallocate to get updated pTextInst->nMaxScroll value for comparison.
                    if(pTextInst->GetStateFlags() & (APT_TEXTFIELD_DIRTY))
                    {
                        pContext->c_cih()->EnsureStringAllocated(pContext->c_cih()->GetDisplayListParent());
                    }

                    pTextInst->SetScroll( nNewScroll );
                    // 2004/05/5 - DC - Fixed minor issues with scrolling in general
                    // scroll values is a 1 based integer (not 0 based) and can range from
                    // 1 to MaxScroll (which is TotalLines-VisibleLines).
                    // Part of Bug 398
                    if (pTextInst->GetScroll() > pTextInst->GetMaxScroll())
                    {
                        pTextInst->SetScroll( pTextInst->GetMaxScroll() );
                    }
                    if (pTextInst->GetScroll() < 1)
                    {
                        pTextInst->SetScroll( 1 );
                    }

                    // DC -16.1/17.0- Moved comparison to see if scroll value changed to be after
                    // the bounds checking of the passed in value...
                    if(nOldScroll != pTextInst->GetScroll())
                    {
                        // added for fix for bug  479 setting textField.scroll property does not reallocate string in 0.15.03
                        pTextInst->ClearStateFlags(APT_TEXTFIELD_NONE);
                        pTextInst->SetStateFlags( APT_TEXTFIELD_TEXTCHANGE | APT_TEXTFIELD_DIRTY | APT_TEXTFIELD_TEXTWIDTH);
                    }
                }
                break;
                case AptTextPropertytext:

                {
                    AptNativeString sBuf;
                    pValue->toString(sBuf);

                    if (pTextInst->GetTextValueConst() != sBuf)
                    {
                        pTextInst->SetTextValue( sBuf );

                        if (pTextInst->GetVarValueConst().IsEmpty() == false)
                        {
                            AptString * pString;
                            AptCIH* pCIH = pContext->c_cih();
                            while (pCIH && !pCIH->IsSpriteInstBase())
                            {
                                if (pCIH->GetDisplayListParent() == NULL)
                                {
                                    break;
                                }
                                pCIH = pCIH->GetDisplayListParent();
                            }

                            // ONA 2004/4/5: Instead of modifying directly the string we create a new string and update it
                            //  There was a bug that modified directly the constant string!
                            pString = AptString::Create();
                            pString->str = sBuf;
                            gAptActionInterpreter.setVariable(pCIH, NULL, &pTextInst->GetVarValueConst(), pString);
                        }
                        pTextInst->ClearStateFlags(APT_TEXTFIELD_NONE);
                        pTextInst->SetStateFlags( APT_TEXTFIELD_TEXTCHANGE | APT_TEXTFIELD_DIRTY | APT_TEXTFIELD_TEXTWIDTH);
                    }
                    return  (true);
                }
                break;
                case AptTextPropertytextColor:

                {
                    pTextInst->SetTextColor( pValue->toInteger() | 0xff000000 );      // switched back to textInst->nColour in 0.11.00
                    pTextInst->ClearStateFlags(APT_TEXTFIELD_NONE);
                    pTextInst->SetStateFlags(APT_TEXTFIELD_TEXTCOLOR);
                    if(pTextInst->GetTextFormatConst() != NULL)
                    {
                        pTextInst->GetTextFormatWritable()->nColor = pTextInst->GetTextColor() & 0xffffff;
                    }
                    return  (true);
                }
                break;
                case AptTextPropertytype:

                {
                    // no need to set.. can only be dynamic
                    return  (true);
                }
                break;
                case AptTextPropertyvariable:

                {
                    AptNativeString strBuf;
                    pValue->toString(strBuf);

                    if (pTextInst->GetVarValueConst() != strBuf)
                    {
                        pTextInst->SetVarValue( strBuf );
                        pTextInst->SetText(pContext->c_cih());
                        pTextInst->ClearStateFlags(APT_TEXTFIELD_NONE);
                        pTextInst->SetStateFlags(APT_TEXTFIELD_TEXTCHANGE | APT_TEXTFIELD_DIRTY | APT_TEXTFIELD_TEXTWIDTH);
                    }
                    return  (true);
                }
                break;
                case AptTextPropertywordWrap:

                {
                    bool bVal = pValue->toBool();
                    if(pTextInst->GetWordWrap() != bVal)
                    {
                    pTextInst->SetWordWrap(bVal);
                    pTextInst->ClearStateFlags(APT_TEXTFIELD_NONE);
                        pTextInst->SetStateFlags(APT_TEXTFIELD_WORDWRAP | APT_TEXTFIELD_DIRTY | APT_TEXTFIELD_TEXTWIDTH);
                    }
                    return  (true);
                }
                break;
                case AptTextProperty_height:

                {
                    // added to resize text box size, we need to return so texture scale is left alone
                    // DC 2004/04/08 -- flash does not change the size if a
                    //                  negative number is passed in.
                    if(!pValue->isUndefined())      // $BUG853 DUG
                    {
                        float fFloat = pValue->toFloat();
                        if (fFloat < 0.0f)
                            return 1;
                        // DC 2004/04/07 -- Height is relative to the top side, not 0.
                        pTextInst->GetBoundsWritable().fBottom = pTextInst->GetBoundsWritable().fTop + fFloat;
                        pTextInst->ClearStateFlags(APT_TEXTFIELD_NONE);
                        pTextInst->SetStateFlags(APT_TEXTFIELD_HEIGHT | APT_TEXTFIELD_DIRTY);
                        return  (true);
                    }
                    return (false);
                }
                break;
                case AptTextProperty_width:

                {
                    // added to resize text box size, we need to return so texture scale is left alone

                    // DC 2004/04/08 -- flash does not change the size if a
                    //                  negative number is passed in.
                    if(!pValue->isUndefined())      // $BUG853 DUG
                    {
                        float fFloat = pValue->toFloat();
                        if (fFloat < 0.0f)
                            return 1;
                        // DC 2004/04/07 -- Width is relative to the left side, not 0.
                        pTextInst->GetBoundsWritable().fRight = pTextInst->GetBoundsWritable().fLeft + fFloat;
                        pTextInst->ClearStateFlags(APT_TEXTFIELD_NONE);
                        pTextInst->SetStateFlags(APT_TEXTFIELD_WIDTH | APT_TEXTFIELD_DIRTY);
                        return  (true);
                    }
                    return (false);
                }
                break;
                case AptTextPropertymouseWheelEnabled:
                      // (LEO) added support for TextMouseWheel   8/
                {
#if defined(APT_USE_MOUSE)
                    bool nVal = pValue->toBool();
                    pTextInst->SetMouseWheelEnabled(nVal);
                    if(nVal == 1 && !GetTargetSim()->GetAnimationTarget()->GetMouseListenerSet()->has(pContext))
                    {
                        GetTargetSim()->GetAnimationTarget()->GetMouseListenerSet()->add(pContext);
                        GetTargetSim()->GetAnimationTarget()->GetInputSet()->add(pContext->c_cih());
                    }
#endif
                }
            }
        }
    }


    if (pContext->c_cih()->mpCharacterInst)
    {
        switch (pContext->c_cih()->mpCharacterInst->GetCharacterType())
        {
            case AptCharacterType_Sprite:
            case AptCharacterType_Animation:
            case AptCharacterType_Button:
            case AptCharacterType_Text:
            case AptCharacterType_Level:
            case AptCharacterType_Image:
            {
                SpriteMembers *pProp = SpriteMembersIndex::in_word_set(pName->c_str(), pName->Size());
                if (pProp)
                {
                    AptCIH *pCIH = pContext->c_cih();
                    float fFloat;
                    switch(pProp->nIndex)
                    {
                    case AptPropertyNumber_x:
                        {
                            if (!pValue->isUndefined())
                            {
                                fFloat = pValue->toFloat();
                                if(pCIH->IsDynamicTextInst())
                                {
                                    fFloat += 2.f;
                                }
                                pCIH->SetProceduralProperty(AptProceduralProperty_X, fFloat, true);
                                return  (true);
                            }
                            return  (false);
                        }
                        break;
                        case AptPropertyNumber_y:

                        {
                            if (!pValue->isUndefined())
                            {
                                fFloat = pValue->toFloat();
                                if(pCIH->IsDynamicTextInst())
                                {
                                    fFloat += 2.f;
                                }
                                pCIH->SetProceduralProperty(AptProceduralProperty_Y, fFloat, true);
                                return  (true);
                            }
                            return  (false);
                        }
                        break;
    #if defined(APT_3D)
                        case AptPropertyNumber_z:
                        {
                            if (!pValue->isUndefined())
                            {
                                fFloat = pValue->toFloat();
                                pCIH->SetProceduralProperty(AptProceduralProperty_Z, fFloat * APTCIH_Z_MULTIPLIER, true);
                                return  (true);
                            }
                            return  (false);
                        }
                        break;
                        case AptPropertyNumber_zscale:
                        {
                            if (!pValue->isUndefined())
                            {
                                fFloat = pValue->toFloat();
                                pCIH->SetProceduralProperty(AptProceduralProperty_ZScale, fFloat, true);
                                return  (true);
                            }
                            return  (false);
                        }
                        break;
                        case AptPropertyNumber_yrotation:
                        {
                            if (!pValue->isUndefined())
                            {
                                fFloat = pValue->toFloat();
                                pCIH->SetProceduralProperty(AptProceduralProperty_YRot, fFloat * APTCIH_Z_MULTIPLIER, true);
                                return  (true);
                            }
                            return  (false);
                        }
                        break;
                        case AptPropertyNumber_xrotation:
                        {
                            if (!pValue->isUndefined())
                            {
                                fFloat = pValue->toFloat();
                                pCIH->SetProceduralProperty(AptProceduralProperty_XRot, fFloat * APTCIH_Z_MULTIPLIER, true);
                                return  (true);
                            }
                            return  (false);
                        }
                        break;
    #endif
                        case AptPropertyNumber_rotation:

                        {
                            if (!pValue->isUndefined())
                            {
                                fFloat = pValue->toFloat();
                                pCIH->SetProceduralProperty(AptProceduralProperty_Rotation, fFloat, true);
                                return  (true);
                            }
                            return  (false);
                        }
                        break;
                        case AptPropertyNumber_alpha:

                        {
                            if (!pValue->isUndefined())
                            {
                                fFloat = pValue->toFloat();
                                pCIH->SetProceduralProperty(AptProceduralProperty_Alpha, fFloat, true);
                                return  (true);
                            }
                            return  (false);
                        }
                        break;
                        case AptPropertyNumber_xscale:

                        {
                            if (!pValue->isUndefined())
                            {
                                fFloat = pValue->toFloat();
                                pCIH->SetProceduralProperty(AptProceduralProperty_XScale, fFloat, true);
                                return  (true);
                            }
                            return  (false);
                        }
                        break;
                        case AptPropertyNumber_yscale:

                        {
                            if (!pValue->isUndefined())
                            {
                                fFloat = pValue->toFloat();
                                pCIH->SetProceduralProperty(AptProceduralProperty_YScale, fFloat, true);
                                return  (true);
                            }
                            return  (false);
                        }
                        break;
                        case AptPropertyNumber_visible:

                        {
                            if (!pValue->isUndefined())
                            {
                                pCIH->SetVisible(pValue->toBool());
                                return  (true);
                            }
                            return  (false);
                        }
                        break;
                        case AptPropertyNumber_width:

                        {
                            if (!pValue->isUndefined())
                            {
                                fFloat = pValue->toFloat();
                                pCIH->SetProceduralProperty(AptProceduralProperty_Width, fFloat, true);
                                return (true) ;
                            }
                            return  (false);
                        }
                        break;
                        case AptPropertyNumber_height:

                        {
                            if (!pValue->isUndefined())
                            {
                                fFloat = pValue->toFloat();
                                pCIH->SetProceduralProperty(AptProceduralProperty_Height, fFloat, true);
                                return (true);
                            }
                            return  (false);
                        }
                        // $bug907 - RB - 06/07/2005 Apt 1.00.01 - Added a case for the name property.
                        break;

                        case AptPropertyNumber_skipeval:
                        {
                            if (pValue != NULL && pValue->isBoolean())
                            {
                                pCIH->SetSkipEval(pValue->c_boolean()->GetBool());
                            }
                            else
                            {
                                APT_ASSERT(pValue && pValue->isBoolean() && "_skipeval property has to be a boolean");
                            }
                        }
                        break;
                        case AptPropertyNumber_CustomControlType:
                        {
                            // _CustomControlType is stored in the hash table for
                            // backwards compatibility with ion_common and other code that looks
                            // directly in the hash table

    #if defined (APT_CUSTOM_CONTROL_USE_ZID)
                            AptValue * pOldType = pCIH->GetNativeHashVirtual()->Lookup(StringPool::GetString(SC__CustomControlType));
                            pCIH->GetNativeHashVirtual()->Set(StringPool::GetString(SC__CustomControlType), pValue);

                            if (pCIH->IsSpriteInstBase())
                            {
                                AptCharacterSpriteInstBase *pSpriteInst = pCIH->GetSpriteInstBase();

                                // if they changed the type of a custom control, lose the reference to the AptRenderableCustomControl RIGHT AWAY
                                if ( gAptInitParms.bCustomControlUseZid &&
                                    (pSpriteInst->mnIsCustomControl == AptCharacterSpriteInstBase::CustomControlState_IsCustomControlZid))
                                {
                                    // reminder: TO_STRING declares a variable.
                                    TO_STRING(pOldType, pOldTypeString);
                                    TO_STRING(pValue, pNewTypeString);
                                    if (*pOldTypeString != *pNewTypeString)
                                    {
                                        if (pSpriteInst->mnIsCustomControl != AptCharacterSpriteInstBase::CustomControlState_IsNotCustomControl)
                                        {
                                            const AptRenderItem * pOriginalRenderItem = pSpriteInst->GetRenderItem();
                                            if (AptCharacterType_CustomControl == pOriginalRenderItem->GetCharacterType())
                                            {
                                                AptRenderItemCustomControl * pRenderItem = (AptRenderItemCustomControl *) pSpriteInst->GetRenderItem();

                                                if (pRenderItem)
                                                {
                                                    pRenderItem->ClearZId();
                                                }
                                            }
                                        }
                                    }
                                }
                            }
    #else
                            pCIH->GetNativeHashVirtual()->Set(StringPool::GetString(SC__CustomControlType), pValue);
    #endif

                            return (true);
                        }
                        break;

                        case AptPropertyNumber_name:

                        {
                            if (!pValue->isUndefined())
                            {
                                AptCIH* pParent = pCIH->GetDisplayListParent();

                                if ( pParent )
                                {
                                    // Unset current name
                                    APT_INC( pCIH );
                                    pParent->GetCharacterInst()->GetNativeHash()->Unset(&pCIH->mMyName);

                                    // Set the name of the CIH to the new name.
                                    pValue->toString(pCIH->mMyName);

                                    // Change the parent CIH's native hash so that it references the new name and it's mapped to the CIH.
                                    pParent->GetCharacterInst()->GetNativeHash()->Set(&pCIH->mMyName, pCIH);
                                    APT_DEC( pCIH );
                                }
                                else
                                {
                                    // Set the name of the CIH to the new name.  Root level case.
                                    pValue->toString(pCIH->mMyName);
                                }

                                return  (true);
                            }
                            return  (false);
                        }
                        break;
                        case AptPropertyNumber_blendMode:
                        {
                            AptCharacterInst * pCharInst = pCIH->GetCharacterInst();
                            if (pCharInst->IsSpriteInstBase() || pCharInst->IsDynamicTextInst() || pCharInst->IsButtonInst())
                            {
                                int32_t nBlendMode = 1;     // SC_BlendNormal
                                if (pValue->isString())
                                {
                                    // we need to convert the string into number
                                    nBlendMode = static_cast<int32_t>( GetBlendModeValue(pValue->c_string()->GetInternalString()) );
                                }
                                else if (pValue->isInteger() || pValue->isFloat())     // it can be float also.
                                {
                                    nBlendMode = pValue->toInteger();
                                }
                                else
                                {
                                    APT_ASSERT(false && "blendMode value has to be an integer or a string");
                                }

                                pCIH->GetCharacterInst()->SetBlendMode(nBlendMode);
                                if (nBlendMode > 1)     // SC_BlendNormal
                                {
                                    pCIH->SetHasBlendMode(1);
                                }
                                else
                                {
                                    // 1 is "normal" so we don't want to create an effect object for this.
                                    pCIH->SetHasBlendMode(0);
                                }
                                return (true);
                            }
                            else
                            {
                                // If you need this and are working with Images, please search the file for
                                // BlendMode and add support everywhere, plus look in AptDisplayList for similar
                                // BlendMode code that checks the type of things - Colin C. 3/16/12
                                APT_FAIL("blendMode property is supported only for Movieclips, Text, Buttons")
                            }
                        }
                        break;

                        case AptPropertyNumber_filters:
                            {
                                AptCharacterInst * pCharInst = pCIH->GetCharacterInst();
                                if (pCharInst->IsDynamicTextInst() || pCharInst->IsSpriteInstBase() || pCharInst->IsButtonInst())
                                {
                                    // its really odd that inside Flash, similar to lookup function set function also makes a shallow copy
                                    // of the whole filters array

                                    APT_ASSERT(pValue != NULL);
                                    AptArray * pShallowCopy = NULL;

                                    if (pValue->isArray())
                                    {
                                        // Now we clone pFilters, because the user always modifies a copy
                                        pShallowCopy = new AptArray();
                                        int nFilterArrLen = pValue->c_array()->length();
                                        for (int i = 0; i < nFilterArrLen; i++)
                                        {
                                            AptValue * pFilter = pValue->c_array()->GetAt(i);
                                            // now make a copy of this object
                                            APT_ASSERT(pFilter->isObject()); // because all filters are AptObjects inside array
                                            AptObject * pNewObject = pFilter->c_object()->Clone(false);
                                            pShallowCopy->set(i, pNewObject);
                                        }
                                        AptNativeHash * pHash = pCIH->GetNativeHashVirtual();
                                        if (pHash == NULL && pCharInst->IsDynamicTextInst())
                                        {
                                            // dynamic text instances do not have hash table in them created at init time like spriteinsts
                                            // so create a new one only when filters are accessed.
                                            pCharInst->SetNativeHash(new AptNativeHash(APT_OBJECTHASHSIZE));
                                        }
                                        pCIH->GetNativeHashVirtual()->Set( StringPool::GetString(SC_filters), pShallowCopy );
                                        if (nFilterArrLen)
                                        {
                                            pCIH->SetHasFilterEffects(1);
                                        }
                                        else
                                        {
                                            pCIH->SetHasFilterEffects(0);
                                        }
                                    }
                                    else if (pValue == gpUndefinedValue)
                                    {
                                        pCIH->GetNativeHashVirtual()->Set( StringPool::GetString(SC_filters), gpUndefinedValue);
                                        pCIH->SetHasFilterEffects(0);
                                    }
                                    return (true);
                                }
                                else
                                {
                                    // If you hit this, then perhaps we need to support this for Image insts?  If so,
                                    // we'll need to check the above  code and make sure it will work for Images as well.
                                    // This includes determining if Images have a hash associated with them, or if they need
                                    // to be handled more like Dynamic Text insts in the above code - Colin C. 3/16/12
                                    APT_FAIL("filters property is supported only for Movieclips, Text, Buttons")
                                }
                            }
                            break;
    #if defined(APT_RENDER_FLAGS)
                        case AptSpriteMethod__renderflags:
                        {
                            AptRenderItemSprite *pRenderItem = (AptRenderItemSprite *)pCIH->GetSpriteInstBase()->GetRenderItemWritable();
                            APT_ASSERT(pRenderItem != NULL && "NULL RENDER ITEMS NOT GOOD");

                            if(pValue != NULL && pValue->isString())
                            {
                                pRenderItem->SetRenderPropertiesStr(pValue->c_string()->str);
                            }
                            else
                            {
                                APT_ASSERT(pValue->isString() && "_renderflags property has to be a string");
                            }
                        }
                        break ;
    #endif
                    case AptSpriteMethod_onData:
                    case AptSpriteMethod_onEnterFrame:
                    case AptSpriteMethod_onLoad:
                    case AptSpriteMethod_onUnload:
                        {
                            if(pContext->c_cih()->IsDynamicTextInst()) return false;
                            int nTableIndex = pProp->nIndex - AptSpriteMethod_onData;
                            int nFlag = aSpriteGperfToActionFlag[nTableIndex];

                            if(pCIH->ContainsNativeHashVirtual())
                            {
                                pCIH->GetNativeHashVirtual()->Set(pName, pValue);
                            if(pValue == NULL || pValue->isUndefined())
                            {
                                pCIH->RemoveEventHandler(nFlag);
                            }
                            else
                            {
                                pCIH->SetEventHandler(nFlag);
                                if(pProp->nIndex == AptSpriteMethod_onEnterFrame)           // Edge case here, if we add a onEnterFrame fucntion, we have to set this sprite to dirty
                                {
                                    pCIH->SetDirtyState(true, true);
                                }
                            }
                            }
                            return true;
                        }
                    case AptSpriteMethod_onDragOut:
                    case AptSpriteMethod_onDragOver:
                    case AptSpriteMethod_onKeyDown:
                    case AptSpriteMethod_onKeyUp:
                    case AptSpriteMethod_onMouseDown:
                    case AptSpriteMethod_onMouseMove:
                    case AptSpriteMethod_onMouseUp:
                    case AptSpriteMethod_onPress:
                    case AptSpriteMethod_onRelease:
                    case AptSpriteMethod_onReleaseOutside:
                    case AptSpriteMethod_onRollOut:
                    case AptSpriteMethod_onRollOver:
                        {
                            if(pContext->c_cih()->IsDynamicTextInst()) return false;
                            int nTableIndex = pProp->nIndex - AptSpriteMethod_onData;
                            int nFlag = aSpriteGperfToActionFlag[nTableIndex];

                            AptNativeHash *pNativeHash = pCIH->GetNativeHashVirtual();
                            pNativeHash->Set(pName, pValue);
                            if(pValue == NULL || pValue->isUndefined())
                            {
                                pCIH->RemoveEventHandler(nFlag);
                                if(pContext->c_cih()->IsSpriteInst() && !(pCIH->HasEvent(AptEventActionFlag_InputEvents))) // NEED TO CHECK FOR inputSET events only!
                                {
                                    if(GetTargetSim()->GetAnimationTarget()->GetInputSet()->has(pCIH))
                                    {
                                        GetTargetSim()->GetAnimationTarget()->GetInputSet()->remove(pCIH);
                                    }
                                }
                            }
                            else
                            {
                                pCIH->SetEventHandler(nFlag);
                                if((pContext->c_cih()->IsSpriteInst() || pContext->c_cih()->IsAnimationInst()) && !GetTargetSim()->GetAnimationTarget()->GetInputSet()->has(pCIH)) // $bug816 - MCB - Need to also add AnimationInst to the inputset
                                {
                                    GetTargetSim()->GetAnimationTarget()->GetInputSet()->add(pCIH);
                                }
                            }
                            return true;
                        }
                    }

    #if defined(EA_DEBUG)
                    if (
                        (pName->EqualNoCase("x")) ||
                        (pName->EqualNoCase("y")) ||
                        (pName->EqualNoCase("rotation")) ||
                        (pName->EqualNoCase("alpha")) ||
                        (pName->EqualNoCase("xscale")) ||
                        (pName->EqualNoCase("yscale")) ||
                        (pName->EqualNoCase("visible")) ||
                        (pName->EqualNoCase("width")) ||
                        (pName->EqualNoCase("height"))
                        )
                    {
                        APT_DEBUGPRINT(APT_DEBUG_MSG_WARNING_LVL, "AptCharacterInst (MovieClip): Incorrect case for '%s'.\n", pName->c_str());
                        APT_ASSERT(0);
                    }
    #endif


                    return  (false);
                }
                else if (pName->EqualNoCase("this"))         // keeping this code as original
                {
                    APT_ASSERT(NOT_REACHED);
                }
                else
                {
                    return  (false);
                }
                break;
            }

            default:
            {
                // Not sure what to do with this type of character
                break;
            }
        }
    }
    return  (false);
}
/*F************************************************************************************************/
/*!
    \Function AptCIH::Release

    \Descrption
    Decreases the reference counting of this instance.

    \Input

    \Output

    \Notes
        For AptCIH, when the reference counting reaches 0, we need to destruct directly the object.
        We can't do a defered destruction.

    \Version
        1.0        03/29/04 (ONA) First Version
        2.0        08/04/04 (LEO)
            -We don't want to remove the zombie sprite, so jump out early.

*/
/************************************************************************************************F*/
#if defined(APT_INC_DEC_MESSAGES)

void    AptCIH::Release(const char * szFuncName, const char * szFileName, int nLineNumber)
#else

void    AptCIH::Release()
#endif
{
    int32_t nRefCount = getRefCount();

    APT_ASSERT(nRefCount > 0);

    if(GetCIHState() == AptCIH::AptCIHState_Zombie && nRefCount == 1)                // $BUG253 LEO    16/47
    {
        return;
    }

    if(nRefCount == 1)
    {
        APT_ASSERT(mpNext == NULL && mpPrev == NULL);
    }

#if defined(APT_INC_DEC_MESSAGES)
        AptValueGC::Release(szFuncName, szFileName, nLineNumber);
#else
        AptValueGC::Release();
#endif
}

/*F************************************************************************************************/
/*!
    \Function AptCIH::EnsureStringAllocated

    \Descrption
    This function is for AptCharacterTextInst only.  It will check if the text inst object is required to create/recreate the associated renderable
    AptAssetString.  Cases for recreation for Apt are if the text has changed, the boundary box size or flag causes the text to render differently.
    But for most part, this decision is up to the auxiliary font renderer.

    \Input      pCIH           - Pointer to AptCIH

    \Output     None.

    \Notes
        None.

    \Version
        1.0        03/23/04 (ONA) First Version
        1.1        03/24/05 (LEO) $bug887 setTextFormat() with RGB(255, 255, 255) does not work in 18.06.
        1.2        07/10/09 (DG) 1664: added leading handling
        1.3        09/28/09 (MZ) added tracking handling
*/
/************************************************************************************************F*/
void    AptCIH::EnsureStringAllocated(AptCIH * pCIH)
{
    APT_PROFILE_LOOKUP(EnsureStringAllocated);
    AptCharacterTextInst *  pTextInst   = this->GetDynamicTextInst();
    const AptCharacter *    pChar       = pTextInst->GetCharacterConst();

    pTextInst->UpdateText(pCIH);

    if ((pTextInst->GetStateFlags() & APT_TEXTFIELD_NONE) == 0)
    {
        //  Need a new string allocation
        // if there was an old one, then get rid of it
        if (GetZID(pTextInst) && GetZID(pTextInst) != &sEmptyAssetString)
        {
            // ### LEO updated flag stuff
            pTextInst->SetZID(NULL);
        }

        AptCharacterAnimation *pParentAnim = &pCIH->c_cih()->mpCharacterInst->GetCharacterConst()->pParentAnim->animation;
        APT_ASSERT(pParentAnim);

        // and alloc the new one

        if (pTextInst->GetTextValueConst().IsEmpty())
        {
            pTextInst->SetZID( &sEmptyAssetString );

            // LEO added for bug493
            if(pTextInst->GetBoxAlignment() != AptStringAlignment_None)
            {
                if(pTextInst->GetWordWrap() == false) // EATech#104911
                {
                    pTextInst->GetBoundsWritable().fRight = pTextInst->GetBoundsConst().fLeft + 4.f;
                }
                pTextInst->GetBoundsWritable().fBottom = pTextInst->GetBoundsConst().fTop + 4.f;
            }

            pTextInst->SetTextWidth( 0.f );
            pTextInst->SetTextHeight( 0.f );
            pTextInst->SetMaxScroll(1) ; // bug $1025 maxscroll (pTextInst->mnMaxScroll) not set for empty string.
        }
        else
        {
            AptAllocateStringParameters asp;
            asp.bMultiline      = pTextInst->GetMultiline();
            asp.bWordWrap       = pTextInst->GetWordWrap();
            asp.eAlignment      = pTextInst->GetAlignment();
            asp.eBoxAlignment   = pTextInst->GetBoxAlignment();
            asp.fFontHeight     = pTextInst->GetFontSize();
            asp.nLeading        = pTextInst->GetLeading();
            asp.nTracking       = pTextInst->GetTracking();

            // If there was a SetTextFormat call done, override the color
            if(pTextInst->GetTextFormatConst() && pTextInst->GetTextFormatConst()->nColor != -1)
            {
                asp.nColour = pTextInst->GetTextFormatConst()->nColor | 0xff000000;
            }
            else
            {
                asp.nColour = pTextInst->GetTextColor();
            }

            asp.nLineOffset = pTextInst->GetScroll();

            // If there was a SetTextFormat call done, and the font name was changed, override the font
            if(pTextInst->GetTextFormatConst() && !pTextInst->GetTextFormatConst()->pFontName.IsEmpty())
            {
                asp.szFontName = pTextInst->GetTextFormatConst()->pFontName.c_str();
            }
            else if (pChar->text.nFontID >= 0)
            {
                asp.szFontName = pParentAnim->apCharacters[pTextInst->GetFontID()]->font.szName;
            }
            else
            {
                asp.szFontName = NULL ;
            }

            asp.szString = pTextInst->GetTextValueConst().c_str();

            // in release 0.13.00
            // The following was updated to use the new rBounds struct which maintains the textField's boundary box
            asp.x0 = pTextInst->GetBoundsConst().fLeft;
            asp.x1 = pTextInst->GetBoundsConst().fRight;
            asp.y0 = pTextInst->GetBoundsConst().fTop;
            asp.y1 = pTextInst->GetBoundsConst().fBottom;

            asp.nBackColor      = pTextInst->GetBackgroundColor();
            asp.nBorderColor    = pTextInst->GetBorderColor();
            asp.bBackground     = pTextInst->GetDrawsBackground();
            asp.bBorder         = pTextInst->GetDrawsBorder();
            asp.eFlags          = pTextInst->GetStateFlags();
            asp.pCurrString     = GetZID(pTextInst);
            if (asp.pCurrString == (AptAssetString)&AptCIH::sEmptyAssetString)
            {
                asp.pCurrString = NULL;
            }

            // $FR#598 CSB - Setting Font Style, Indent, LeftMargin, RightMargin and Checking if pMyTextFormat is NULL, and initializing 10/36

            if (pTextInst->GetTextFormatConst() == NULL)
            {
                asp.nFontStyle   = AptFontStyle_Normal;
                asp.nIndent      = -1;
                asp.nLeftMargin  = -1;
                asp.nRightMargin = -1;
            }
            else
            {
                if(pTextInst->GetTextFormatConst()->nFontStyle == AptFontStyle_None)
                {
                    asp.nFontStyle = AptFontStyle_Normal;
                }
                else
                {
                    asp.nFontStyle   = pTextInst->GetTextFormatConst()->nFontStyle;
                }
                asp.nIndent      = pTextInst->GetTextFormatConst()->nIndent;
                asp.nLeftMargin  = pTextInst->GetTextFormatConst()->nLeftMargin;
                asp.nRightMargin = pTextInst->GetTextFormatConst()->nRightMargin;
            }
            /// GO and create the new font assetid!
            {APT_PROFILE_AUXILIARY(pfnAllocateString);
            pTextInst->SetZID(gAptFuncs.pfnAllocateString(&asp)) ;                  /// <------------------------------
            }

            // We now need to get the new boundary info if autoSize was set     // added in 0.13.00
            if(pTextInst->GetBoxAlignment() != AptStringAlignment_None)
            {
                /// Now if either autoSize Right or Center was used, we need to set the new _x postion of the textField
                float fW = pTextInst->GetBoundsConst().fRight - pTextInst->GetBoundsConst().fLeft;
                float fW2 = asp.x1 - asp.x0;
                if(pTextInst->GetBoxAlignment() == AptStringAlignment_Center)
                {
                    this->SetProceduralProperty(AptProceduralProperty_X, (this->GetPositionMatrixConst()->tx - (fW2 - fW) / 2.f), true);
                }
                else if(pTextInst->GetBoxAlignment() == AptStringAlignment_Right)
                {
                    this->SetProceduralProperty(AptProceduralProperty_X, (this->GetPositionMatrixConst()->tx + fW) - fW2, true);
                }
            }


            if( FLOAT_NOT_EQUALS(pTextInst->GetBoundsConst().fLeft, asp.x0))
                pTextInst->GetBoundsWritable().fLeft    = asp.x0;
            if( FLOAT_NOT_EQUALS(pTextInst->GetBoundsConst().fRight, asp.x1))
                pTextInst->GetBoundsWritable().fRight    = asp.x1;
            if( FLOAT_NOT_EQUALS(pTextInst->GetBoundsConst().fTop, asp.y0))
                pTextInst->GetBoundsWritable().fTop    = asp.y0;
            if( FLOAT_NOT_EQUALS(pTextInst->GetBoundsConst().fBottom, asp.y1))
                pTextInst->GetBoundsWritable().fBottom    = asp.y1;
            if( pTextInst->GetMaxScroll() != asp.nMaxScroll)
                pTextInst->SetMaxScroll( asp.nMaxScroll );

            // adjust scroll
            if( pTextInst->GetScroll() > pTextInst->GetMaxScroll() )
            {
                pTextInst->SetScroll( pTextInst->GetMaxScroll() );
            }

            if( FLOAT_NOT_EQUALS(pTextInst->GetTextWidth(), asp.fTextWidth))
                pTextInst->SetTextWidth( asp.fTextWidth );
            if( FLOAT_NOT_EQUALS(pTextInst->GetTextHeight(), asp.fTextHeight))
                pTextInst->SetTextHeight( asp.fTextHeight );
            if( FLOAT_NOT_EQUALS(pTextInst->GetLength(), asp.fStrLen))
                pTextInst->SetLength( asp.fStrLen );
        }

		// FIFA_BEGIN | SENDBACK | mwebb | Mar 26 2015 | FIFA 16 change
		//pTextInst->ClearStateFlags(APT_TEXTFIELD_ALL);
		// FIFA_END
        pTextInst->SetStateFlags( APT_TEXTFIELD_NONE );
    }
}

/*F************************************************************************************************/
/*!
    \Function    AptCIH::DeallocAssetStringRecursive

    \Description
    This function will iterate through the whole display list and remove the AptAssetString pointers from all AptCharacterTestInst

    \Output

    \Notes
        None.

    \Version    1.0        05/04/04 (LEO) First Version
*/
/************************************************************************************************F*/
void AptCIH::DeallocAssetStringRecursive()
{
    if (IsSpriteInstBase())
    {
        AptCharacterSpriteInstBase *pSpriteInst = GetSpriteInstBase();
        pSpriteInst->mDisplayList.DeallocAssetStringRecursive();
    }
    else if (IsDynamicTextInst())
    {
        AptCharacterTextInst *pTextInst = GetDynamicTextInst();
        if (GetZID(pTextInst) && GetZID(pTextInst) != &sEmptyAssetString)
        {
            /// ### LEO added flags stuff
            pTextInst->SetStateFlags( APT_TEXTFIELD_FUPDATE | APT_TEXTFIELD_DIRTY );
            // $bug 985 Text not rendered - cleared APT_TEXTFIELD_NONE flag, because otherwise in next ensurestringallocated function
            // call, it evaluates  "if ((pTextInst->GetStateFlags() & APT_TEXTFIELD_NONE) == 0): condition to false and does not call
            // pfnAllocateString and nothing gets rendered, until flag is cleared by other operation.
            // if user has called the AptDeallocAllAssetString function then user expects that strings get reallocated.
            pTextInst->ClearStateFlags(APT_TEXTFIELD_NONE);
        }
        pTextInst->SetZID( 0 );
    }
}

/*F************************************************************************************************/
/*!
    \Function    AptCIH::GetBoundingRect

    \Description
    returns the boundary box of the object in pRect

    \Output

    \Notes
    None.

    \Version    1.0        05/04/04 (LEO) First Version
*/
/************************************************************************************************F*/
void AptCIH::GetBoundingRect(AptRenderingContext *pRenderingContext, const AptMatrix * pCurrentTransform, AptRect *pRect) const
{
    const AptMatrix *pMyTransformMatrix = GetPositionMatrixConst();
    const AptCharacterInst *pInst = mpCharacterInst;

    if (pInst == NULL || IsLevelInst())
        return;

    // This function can be called a number of times recuresively, so put the matrix on the heap to avoid stack overflow.
    AptMatrix *pMatrixMult = APT_MALLOC_ARRAY( AptMatrix, 1);

    pRenderingContext->multMatrix( pCurrentTransform, pMyTransformMatrix, pMatrixMult);

    if (pInst->IsSpriteInstBase())
    {
        const AptCharacterSpriteInstBase *pSpriteInst = GetSpriteInstBase();
        pSpriteInst->mDisplayList.GetBoundingRect(pRenderingContext, pMatrixMult, pRect);
    }
#if defined APT_USE_BUTTONS
    else if (pInst->IsButtonInst())
    {
        const AptCharacterButtonInst *pButtonInst = GetButtonInst();
        pButtonInst->mDisplayList.GetBoundingRect(pRenderingContext, pMatrixMult, pRect);
    }
#endif
    else if (pInst->IsDynamicTextInst())
    {
        const AptCharacterTextInst *pTextInst = GetDynamicTextInst();

        // ### LEO updated to use the new rBounds struct        // 0.13.00
        pRenderingContext->expandBoundingRect( &pTextInst->GetBoundsConst(), pMatrixMult, pRect);

    }
    else if (pInst->IsMorphInst())
    {
        APT_DEBUGPRINT(APT_DEBUG_MSG_DEBUG_LVL," [APT] Attempting to get the bounding box or shape morphed object\n [APT] bounding box calculation on morphs is not supported...\n");
    }
    else
    {
        pInst->GetCharacterConst()->GetBoundingRect(pRenderingContext, pMatrixMult, pRect);
    }

    APT_FREE_ARRAY(pMatrixMult, AptMatrix, 1);
}

/*F************************************************************************************************/
/*!
    \Function    AptCIH::GetBoundingRect

    \Description
        set the boundary box coords in pRect

    Iterates through the movieclip and its children to find the boundary coords

    \Input      *pRect            - AptRect object

    \Output
        None.

    \Notes
        None.

    \Version    1.0        08/24/04 (LEO) First Version

*/
/************************************************************************************************F*/
void AptCIH::GetBoundingRect(AptRect *pRect) const
{
    APT_ASSERT(pRect);
    pRect->fLeft = FLT_MAX;
    pRect->fRight = -FLT_MAX;
    pRect->fBottom = -FLT_MAX;
    pRect->fTop = FLT_MAX;
    GetBoundingRect(gpRenderingContext, &gIdentityMatrix, pRect);

    // $bug908 - RB - return 0-size extents for empty movieclips
    // PcLint warning Info 777: Testing floats for equality
    if (/*lint --e(777) */ (pRect->fBottom == -FLT_MAX) && (pRect->fTop == FLT_MAX) &&
        (pRect->fLeft == FLT_MAX) && (pRect->fRight == -FLT_MAX) )
    {
        pRect->fBottom = pRect->fTop = 0;
        pRect->fLeft = pRect->fRight = 0;
}
}

/*F************************************************************************************************/
/*!
    \Function    AptCIH::GetGlobalBoundingRect

    \Description
        Returns the coords in global based on the global coords

    \Input      *pRect            - AptRect object

    \Output
        None.

    \Notes
        None.

    \Version    1.0        08/24/04 (LEO) First Version

*/
/************************************************************************************************F*/
void AptCIH::GetGlobalBoundingRect(AptRect *pRect) const
{
    APT_ASSERT(pRect);

    // This function will not be called recursively, and the recursive bounding rect call below uses the heap,
    // so just use the stack for this one. should be no harm.
    AptMatrix MatrixCur = gIdentityMatrix;
    MultParentMatrix(this->GetDisplayListParent(), MatrixCur);

    pRect->fLeft = FLT_MAX;
    pRect->fRight = -FLT_MAX;
    pRect->fBottom = -FLT_MAX;
    pRect->fTop = FLT_MAX;
    GetBoundingRect(gpRenderingContext, &MatrixCur, pRect);
}

/*F************************************************************************************************/
/*!
    \Function    AptCIH::GetGlobalTranslation

    \Description
        Returns the global X and Y for this AptCIH

    \Input      *pfX              - pointer to float in which x value will be filled in
    \Input      *pfY              - pointer to float in which y value will be filled in

    \Output

    \Notes
        None.

    \Version    1.0        08/24/04 (LEO) First Version

*/
/************************************************************************************************F*/
void AptCIH::GetGlobalTranslation(float *pfX, float *pfY) const
{
    AptMatrix cur = gIdentityMatrix;
    MultParentMatrix(this, cur);

    if(pfX)
    {
        *pfX = cur.tx;
    }
    if(pfY)
    {
        *pfY = cur.ty;
    }
}

/*F************************************************************************************************/
/*!
    \Function    AptCIH::getProceduralProperty

    \Description
        returns the desired property from the afProperties array.

    \Input      eProperty         - enum of type AptProceduralProperty

    \Output     float             - value for that procedural property

    \Notes
        None.

    \Version    1.0        08/24/04 (LEO) First Version
    \Version    1.1        12/12/05 (LEO) Added Apt_3D code
    \Version    1.2        05/04/07 (RB)  Fixed NaN values for xscale, yscale, rotation
    \Version    1.3        07/10/09 (RB)  Moved _visible to be first because it's most common
    \Vesrion    1.4        10/04/10 (XC)  Moved _visible to its own function to stop bool->float->bool conversions
*/
/************************************************************************************************F*/
float AptCIH::GetProceduralProperty(AptProceduralProperty eProperty) const
{
    switch(eProperty)
    {
    case AptProceduralProperty_Visible:     // this case hits most frequently
        {
            APT_ASSERTM(false, "Please use GetVisible instead of GetProceduralProperty(AptProceduralProperty_Visible)");
            return ((float)mpCharacterInst->GetIsVisible()) ;
        }
    case AptProceduralProperty_Width:
        {
            AptRect rect;
            GetBoundingRect(&rect);
            float fFloat = rect.fRight - rect.fLeft;
            return fFloat = (fFloat < 0) ? 0 : fFloat;
        }
    case AptProceduralProperty_Height:
        {
            AptRect rect;
            GetBoundingRect(&rect);
            float fFloat = rect.fBottom - rect.fTop;
            return fFloat = (fFloat < 0) ? 0 : fFloat;
        }
    case AptProceduralProperty_Rotation:
        {
            const AptMatrix * matrix = GetPositionMatrixConst() ;

            APT_ASSERT_NOT_NAN_MATRIX( matrix );
            if(fRot != NULL)
            {
                return *fRot;
            }
            else
            {
                if(fabsf(matrix->b) < EPSILON && fabsf(matrix->c) < EPSILON)
                {
                    return 0.f;
                }
                float t2 = acosf(GetCosAngle(matrix)) * TODEGREES;
                return matrix->b < 0.f ? -t2 :  t2;
            }
        }
    case AptProceduralProperty_XScale:
        {
            const AptMatrix * matrix = GetPositionMatrixConst() ;

            APT_ASSERT_NOT_NAN_MATRIX( matrix );
            if(fabsf(matrix->b) < EPSILON && fabsf(matrix->c) < EPSILON)
            {
                return matrix->a * 100.f;
            }
            return (GetXScale(matrix) * 100.f);
        }
    case AptProceduralProperty_YScale:
        {
            const AptMatrix * matrix = GetPositionMatrixConst() ;

            APT_ASSERT_NOT_NAN_MATRIX( matrix );
            if(fabsf(matrix->b) < EPSILON && fabsf(matrix->c) < EPSILON)
            {
                return matrix->d * 100.f;
            }
            return (GetYScale(matrix) * 100.f);
        }
    case AptProceduralProperty_X:
        {
            const AptMatrix * matrix = GetPositionMatrixConst() ;

            return matrix->tx;
        }
    case AptProceduralProperty_Y:
        {
            const AptMatrix * matrix = GetPositionMatrixConst() ;

            return matrix->ty;
        }
#if defined(APT_3D)
    case AptProceduralProperty_Z:
       {
            return mpCharacterInst->GetZ();
       }
    case AptProceduralProperty_ZScale:
       {
            return mpCharacterInst->GetZScale() * 100.f;
       }
   case AptProceduralProperty_YRot:
       {
            return mpCharacterInst->GetYRot();
       }
   case AptProceduralProperty_XRot:
       {
            return mpCharacterInst->GetXRot();
       }
#endif
    case AptProceduralProperty_Alpha:
        {
            const AptCXForm * cxform = GetColorMatrixConst() ;

            // (LEO) Flash wants this value to be in 0-100 range, not 0-255, so convert it here
            return cxform->scale.GetValuef(AptColorHelper::Alpha) * 100 / AptColorHelperScale::SCALE_FACTOR;
        }
    case AptProceduralProperty_TR:
        {
            const AptCXForm * cxform = GetColorMatrixConst() ;

            return cxform->translate.GetValuef(AptColorHelper::Red);
        }
    case AptProceduralProperty_TG:
        {
            const AptCXForm * cxform = GetColorMatrixConst() ;

            return cxform->translate.GetValuef(AptColorHelper::Green);
        }
    case AptProceduralProperty_TB:
        {
            const AptCXForm * cxform = GetColorMatrixConst() ;

            return cxform->translate.GetValuef(AptColorHelper::Blue);
        }
    default:
        {
            break;
    }
    } // switch(eProperty)
    return -1;
}

/*F************************************************************************************************/
/*!
    \Function    AptCIH::GetVisible

    \Description
        returns the visible property

    \Version    1.0        10/04/10 (XC) First Version
*/
/************************************************************************************************F*/
bool AptCIH::GetVisible() const
{
    return mpCharacterInst->GetIsVisible();
}

/*F************************************************************************************************/
/*!
    \Function    AptCIH::SetProceduralProperty

    \Description
        Sets the procedural property for AptCIH

    \Input      eProperty         - enum of type AptProceduralProperty
    \Input      fValue            - value for that procedural property
    \Input      bASFlag           - flag to indicate if it is changed to Actionscript.

    \Output
        None.

    \Notes
        None.

    \Version    1.0        08/24/04 (LEO) First Version
                            -(LEO) $bug859 Setting _alpha removes the AS changed flag:  Moved setASChange(bASFlag) in each case stmt
    \Version    1.1        07/12/04 (LEO) $bug946 Set Height issue
    \Version    1.2        12/12/05 (LEO) Added Apt_3D code
    \Version    1.3        05/04/07 (RB)  Fixed NaN values for xscale, yscale, rotation
*/
/************************************************************************************************F*/
void AptCIH::SetProceduralProperty(AptProceduralProperty eProperty, float fValue, bool bASFlag)
{
    AptMatrix * matrix = NULL;
    AptCXForm * cxform = NULL;

    //SetDirtyState(true, true);      Don't think we need this here, so commenting it out for now... please remove before release!

    switch(eProperty)
    {
        case AptProceduralProperty_Width:
        {
            SetASChanged(bASFlag);
            AptRect rect;
            if(fValue < 0.f)
            {
                return;                           // flash does not change the size if a negative number is passed in.
            }
            else if(fValue == 0.f)
            {
                fValue = EPSILON;                   // Set to a small precision value if set to zero
            }

            // DC 2004/12/04 -- Apt doesn't handle setting the size of
            // an empty movie clip. (the behavior is somewhat undefined
            // anyway). BUG 451
            GetBoundingRect(&rect);
            float fWidthVal = (rect.fRight - rect.fLeft);
            if( fWidthVal == 0.0f)
            {
                return;
            }

            matrix = GetPositionMatrixWritable();
            // $bug798 - MCB - holy crappers batman.
            // If the width is being set, rotation must be taken into consideration when changing the xscale.
            // The original equation is only valid when rotation is zero. Reverse-engineering flash's behavior
                // for the case when rotation is non-zero yields a different equation.
                // $bug909 - RB
                // ----> However, this equation is not always valid if a scale operation precedes a rotation:
                // Let w be the previous width.  Then the xscale appears to be multiplied by a factor of (newValue/w).
                // The yscale is multiplied by an unknown amount.
            float newValue=0.f;

            if(fabsf(matrix->b) < EPSILON && fabsf(matrix->c) < EPSILON)
            {
                float fScaleScale = (fValue / (fWidthVal));
                if(fScaleScale < EPSILON)               // Bug351 precision problem
                {
                    fScaleScale = EPSILON;
                }
                newValue = matrix->a * fScaleScale * 100.f;

                if (newValue < FLASH_SCALE_LIMIT)
                {
                    newValue = FLASH_SCALE_LIMIT;
                }

                //matrix->a = newValue / 100.f;
                SetProceduralProperty(AptProceduralProperty_XScale, newValue, true);
            }
            else // use the original method of calculating the new xscale.
            {
                    float fCurrentWidth = GetProceduralProperty(AptProceduralProperty_Width);
                    if ( fCurrentWidth == 0.0f )
                    {
                        return;
                    }

                    // percentage by which the X scale changes
                    float fScaleScale = fValue / fCurrentWidth;
                if(fScaleScale < EPSILON)
                {
                    fScaleScale = EPSILON;
                }

                    // transform the local X axis
                    matrix->a *= fScaleScale;
                    matrix->b *= fScaleScale;
            }
            break;
        }
        case AptProceduralProperty_Height:
        {
            SetASChanged(bASFlag);
            AptRect rect;
            if(fValue < 0.f)
            {
                return;                           // flash does not change the size if a negative number is passed in.
            }
            else if(fValue == 0.f)
            {
                fValue = EPSILON;                   // Set to a small precision value if set to zero
            }

            // DC 2004/12/04 -- Apt doesn't handle setting the size of
            // an empty movie clip. (the behavior is somewhat undefined
            // anyway). BUG 451
            GetBoundingRect(&rect);
            float fHeightVal = (rect.fBottom - rect.fTop);
            if(fHeightVal == 0.0f)
            {
                return;
            }

            matrix = GetPositionMatrixWritable();
            // $bug798 - MCB - holy crappers batman.
            // If the height is being set, rotation must be taken into consideration when changing the yscale.
            // The original equation is only valid when rotation is zero. Reverse-engineering flash's behavior
                // for the case when rotation is non-zero yields a different equation.
                // $bug909 - RB
                // ----> However, this equation is not always valid if a scale operation precedes a rotation:
                // Let h be the previous height.  Then the yscale appears to be multiplied by a factor of (newValue/h).
                // The xscale is multiplied by an unknown amount.
            float newValue = 0.f;

                if(fabsf(matrix->b) < EPSILON && fabsf(matrix->c) < EPSILON)
            {
                float fScaleScale = (fValue / (fHeightVal));
                if(fScaleScale < EPSILON)               // Bug351 precision problem
                {
                    fScaleScale = EPSILON;
                }
                newValue = matrix->d * fScaleScale * 100.f;

                if (newValue < FLASH_SCALE_LIMIT)
                {
                    newValue = FLASH_SCALE_LIMIT;
                }

                SetProceduralProperty(AptProceduralProperty_YScale, newValue, true);
            }
            else // use the original method of calculating the new yscale.
            {
                    float fCurrentHeight = GetProceduralProperty(AptProceduralProperty_Height);
                    if ( fCurrentHeight == 0.0f )
                    {
                        return;
                    }

                    // percentage by which the Y scale changes
                    float fScaleScale = fValue / fCurrentHeight;
                if(fScaleScale < EPSILON)
                {
                    fScaleScale = EPSILON;
                }

                    // transform the local Y axis
                    matrix->c *= fScaleScale;
                    matrix->d *= fScaleScale;
            }
            break;
        }
        case AptProceduralProperty_Rotation:
        {
            SetASChanged(bASFlag);
            // DC 2004/13/04
            // ._rotation values in flash are forced to -180 to +180.
            // i.e. setting a value of 190 will result in a -170.

            if(fValue > 180.0f || fValue < -180.0f)
            {
                fmod(fValue,360.f);
            }
            if(fValue > 180.f)
            {
                fValue -= 360.f;
            }
            if(fRot == NULL)
            {
                fRot = APT_MALLOC_ARRAY(float, 1);
            }
            *fRot = fValue;

            matrix = GetPositionMatrixWritable();

            fValue *= TORADIANS;
            float fXS = GetXScale(matrix);
            float fYS = GetYScale(matrix);

            float c = cosf(fValue);
            float s = sinf(fValue);

            matrix->a = fXS * c;
            matrix->b = fXS * s;
            matrix->c = fYS * -s;
            matrix->d = fYS * c;
            APT_ASSERT_NOT_NAN_MATRIX( matrix );
            break;
        }
        case AptProceduralProperty_XScale:
        {
            SetASChanged(bASFlag);
            fValue /= 100.f;
            if(fValue == 0.0f)
            {
                fValue = 0.0001f;
            }

            matrix = GetPositionMatrixWritable();
            if(fabsf(matrix->b) < EPSILON && fabsf(matrix->c) < EPSILON)
            {
                    matrix->a = fValue;
            }
            else
            {
                float tmp = GetXScale(matrix);
                float c = matrix->a / tmp;
                float s = matrix->b / tmp;
                matrix->a = (fValue) * c;
                matrix->b = (fValue) * s;
            }
            APT_ASSERT_NOT_NAN_MATRIX( matrix );
            break;
        }
        case AptProceduralProperty_YScale:
        {
            SetASChanged(bASFlag);
            fValue /= 100.f;
            if(fValue == 0.0f)
            {
                fValue = 0.0001f;
            }
            matrix = GetPositionMatrixWritable();
            if(fabsf(matrix->b) < EPSILON && fabsf(matrix->c) < EPSILON)
            {
                    matrix->d = fValue;
            }
            else
            {
                float tmp = GetYScale(matrix);
                float c = matrix->d / tmp;
                float s = matrix->c / tmp;
                matrix->d = (fValue) * c;
                matrix->c = (fValue) * s;
            }
            APT_ASSERT_NOT_NAN_MATRIX( matrix );
            break;
        }
        case AptProceduralProperty_X:
        {
            SetASChanged(bASFlag);
            matrix = GetPositionMatrixWritable();
            matrix->tx = fValue;
            break;
        }
        case AptProceduralProperty_Y:
        {
            SetASChanged(bASFlag);
            matrix = GetPositionMatrixWritable();
            matrix->ty = fValue;
            break;
        }
#if defined(APT_3D)
        case AptProceduralProperty_Z:
        {
            SetASChanged(bASFlag);
            mpCharacterInst->SetZ(fValue);
            break;
        }
        case AptProceduralProperty_ZScale:
        {
            SetASChanged(bASFlag);
            fValue /= 100.f;
            if(fValue == 0.0f)
            {
                fValue = 0.0001f;
            }
            mpCharacterInst->SetZScale(fValue);
            break;
        }
        case AptProceduralProperty_YRot:
        {
            SetASChanged(bASFlag);
            mpCharacterInst->SetYRot(fValue);
            break;
        }
        case AptProceduralProperty_XRot:
        {
            SetASChanged(bASFlag);
            mpCharacterInst->SetXRot(fValue);
            break;
        }
#endif
        case AptProceduralProperty_Alpha:
        {
            SetASChanged(bASFlag);
            cxform = GetColorMatrixWritable();
            // $bug712 - MCB - Preventing negative alpha values; capping at zero.
            if(fValue < 0.f)
            {
                cxform->scale.SetValuef(AptColorHelper::Alpha, 0.f );
            }
            else
            {
                cxform->scale.SetValuef(AptColorHelper::Alpha, (int)((fValue / 100.f) * AptColorHelperScale::SCALE_FACTOR));
            }
            break;
        }
        case AptProceduralProperty_TR:
        {
            SetASChanged(bASFlag);
            cxform = GetColorMatrixWritable();
            cxform->translate.SetValuef(AptColorHelper::Red, fValue);
            break;
        }
        case AptProceduralProperty_TG:
        {
            SetASChanged(bASFlag);
            cxform = GetColorMatrixWritable();
            cxform->translate.SetValuef(AptColorHelper::Green, fValue);
            break;
        }
        case AptProceduralProperty_TB:
        {
            SetASChanged(bASFlag);
            cxform = GetColorMatrixWritable();
            cxform->translate.SetValuef(AptColorHelper::Blue, fValue);
            break;
        }
        case AptProceduralProperty_Visible:
        {
            APT_ASSERTM(false, "Please use SetVisible instead of SetProceduralProperty(AptProceduralProperty_Visible, bool)");
            bool boolValue;
            boolValue = (fValue > 0.5f) ? true : false;

            mpCharacterInst->SetIsVisible(boolValue) ;
            break;
        }
        default:
        {
            break;
        }
    } // switch(eProperty)
}

/*F************************************************************************************************/
/*!
    \Function    AptCIH::SetVisible

    \Description
        sets the visible property

    \Version    1.0        10/04/10 (XC) First Version
*/
/************************************************************************************************F*/
void AptCIH::SetVisible(bool bVisible)
{
    mpCharacterInst->SetIsVisible(bVisible);
}

/*F************************************************************************************************/
/*!
    \Function    AptCIH::AssociateInstToClass

    \Description
        When a Sprite is created, AssociateInstToClass checks if there is an associated Class with the sprite.

    When a Sprite is created, AssociateInstToClass is called to check if there is an associated Class with the sprite.  An association is created
    via the Object.registerClass() call in Flash.  This is the only way an association will be found using this function.  All Object.registerClass
    associations are kept in the global gpObjRegistrationHash object.

    \Output

    \Notes
        None.

    \Version    1.0        05/04/04 (LEO) First Version
    \Version    1.1        09/15/04 (DC)  18.00 Made compatible with new script function API.
    \Version    1.2        10/27/04 (LEO) Added checks to only return true is a listenSet event is found.
    \Version    1.3        02/21/07 (ASK) $Bug EATech 107144 Problem with createEmptyMovieClip
                                          Early retrun in case when movie is created thru createemptymovieclip
                                          as there is no need to check for any class associated with empty movieclip.
    \Version    1.4        04/30/09 (RB)  Display "<constructor>", class name, and SWF filename in the ActionScript
                                          callstack, when a class's ctor is run due to being associated with a movieclip.
*/
/************************************************************************************************F*/
void AptCIH::AssociateInstToClass()
{
    APT_PROFILE(AssociateInstToClass) ;
    // this creates prototype in itself and also
    // sets the __proto__ of this movieclip instance to point to MovieClip.prototype
    // which was already created as a global object.

    AptCharacterInst *pCharInst = mpCharacterInst;

    if ( mpCharacterInst == NULL || (!IsSpriteInst() && !IsAnimationInst()))
    {
        // currently no need to register any class for any other type of characters.
        return ;
    }

    if (GetSpriteInstBase()->GetCreatedDynamic())
    {
        return ;
    }

    APT_ASSERT(pCharInst);
    AptCharacterAnimation *pAnim   = &pCharInst->GetCharacterConst()->pParentAnim->animation;
    const AptCharacter *pCharacter = pCharInst->GetCharacterConst() ;

    // this should be done in any case whether a class is associated to it or not
    AptValue *pConstructorPrototypeMC = new AptPrototype();
    pCharInst->GetNativeHash()->SetPrototype(pConstructorPrototypeMC);

    // by default set the __proto__ to MovieClip.prototype and then overwrite it if
    // class registration is found
    AptValue * pMovieClip = gpGlobalGlobalObject->Lookup(StringPool::GetString(SC_MovieClip)) ;
    AptValue *pPrototypeMovieClip = pMovieClip->GetNativeHashVirtual()->GetPrototype();
    pCharInst->GetNativeHash()->Set__Proto__(pPrototypeMovieClip);

     if (pCharacter == AptCharacterHelper::GetAptMovieCharacter())
    {
        // basically if we are creating a new empty movieclip then there will not be any
        // class associated with it and no need to check for character in parent anim.
        return;
    }

    int iFound = 0 ;

    if (pAnim->nExports > 0)
    {
    // look for character in exports/library
        for (int i = 0; i < pAnim->nExports; i++)
        {
			// DT 36061 - nID can be negative, but it rarely crashes because the heap is valid (usually) close to where the array is allocated
            if ((pAnim->aExports[i].nID >= 0) && (pCharacter == pAnim->apCharacters[pAnim->aExports[i].nID]))
            {
                // character found - now get string
                AptValue * pValue1 = NULL ;
                if (gpObjRegistrationHash)
                {
                    AptNativeString strName(pAnim->aExports[i].szName);
                    pValue1 = gpObjRegistrationHash->Lookup(&strName) ;
                }
                // check if you get any association
                if (pValue1 && pValue1->getIsDefined())
                {
                    // some class is associated with this movieclip, so set the appropriate
                    // prototype values of this instance.

                    AptValue *pClassPrototype = pValue1->GetNativeHashVirtual()->GetPrototype();
                    if (pClassPrototype && pClassPrototype->getIsDefined())
                    {
                        pCharInst->GetNativeHash()->Set__Proto__(pClassPrototype);
                    }
                    AptScriptFunctionBase *pFuncDef = pValue1->c_scriptfunction();

                    // added constructor function to runActions pool instead of calling the constructor function immediately.
                    // this is because otherwise constructor cannot get access to component in-built properties defined
                    // in properties panel.
                    // adding it at back makes sure that it gets executed after AptEventActionFlag_Initialize function (which is queued
                    // in  addSetoCaches() ) that sets in-built properties get executed first and then the constructor.
                    //AptScriptFunction * pNewFunc = new AptScriptFunction(pFuncDef->pFunction, pFuncDef->pFrameStack, pFuncDef->pFuncAnim, this);
                    //GetTargetSim()->GetAnimationTarget()->AddFunctionBack(this, pNewFunc, 0, gNullInput);

                    // We need to tick this sprite so its properties are set for the cotr

                    // because we are ticking here, the children get placed on to stage and then child class constructors get called
                    // before we call parent class constructor down below in code.
                    // so added this #define to turn it on and off as needed at compile time
                    if (!gAptInitParms.bUseNewClassInitOrder)
                    {
                        this->tick();
                    }
                    this->SetHasClass(1);       // super crap

                    //void * pSavedValue   = gAptActionInterpreter.PrepareForExecution("AssociateInstToClass");
                    APT_DEFINE_ACTION_SETUP(this, pFuncDef, pAnim->aExports[i].szName, AptActionType_AssoInstToClass);
                    void * pSavedValue   = gAptActionInterpreter.PrepareForExecution(&oActionSetup);

#ifdef APT_DEBUGGER_ENABLE
                    bool isScriptActionFlag = AptDebugger::GetInstance()->PushCallStack(pFuncDef, "MovieClip", reinterpret_cast<unsigned char* const>(AptDebugger::INVALID_OFFSET),this);
#endif
                    gAptOptCallStack->Push("<constructor>", NULL, NULL, pAnim->aExports[i].szName );

#if APT_PROF_ENABLED && APT_PROFILE_ACTIONSCRIPT_ENABLED
                        APT_PROF_ENTER((char8_t *)(const uint8_t *)pFuncDef->GetName());
#endif
                    gAptActionInterpreter.withStack.push(this);

                    // RB: 05-18-06: must store this on createdObjectStack so super constructor
                    //               will use the correct object instance.
                    gAptActionInterpreter.createdObjectsStack.push(this);

                    this->SetInCtor(1) ;        // $bug849 -ASK 0.18.04 super.function() defect - 3/3
                    gAptActionInterpreter.callFunction(this, pFuncDef, 0, NULL, NULL, true);
                    this->SetInCtor(0) ;


#ifdef APT_DEBUGGER_ENABLE
                    if (isScriptActionFlag == true)
                    {
                        AptDebugger::GetInstance()->PopCallStack(pFuncDef);
                    }
#endif
                    gAptActionInterpreter.createdObjectsStack.pop();
                    gAptActionInterpreter.withStack.pop();

#if defined(APT_ENABLE_DEBUG_REPORT)
                    mLabel = pAnim->aExports[i].szName;
                    if(mReport)
                    {
                        mReport->StartTrack(this);
                    }
#endif

#if APT_PROF_ENABLED && APT_PROFILE_ACTIONSCRIPT_ENABLED
                        APT_PROF_LEAVE((char8_t *)(const uint8_t *)pFuncDef->GetName());
#endif

                    gAptOptCallStack->Pop();

                    gAptActionInterpreter.stackPop();

                    gAptActionInterpreter.CleanupAfterExecution(pSavedValue, &oActionSetup);  //oActionSetup is defined by the macro
                    iFound = FindAndSetEvents();
                }
                break ;     // break as there will be only one class registered.
            }
        }
    }

    // now also check if the class (and any of its parent classes) to which this movie is registered
    // has any standard event handlers defined in it. if yes then add this CIH to GetTargetSim()->GetAnimationTarget().inputSet
    // which keeps track of which CIH instances have event handlers defined for it.
    // This is actually time consuming to look for any of the event handler, defined in any of parent classes of
    // this class but currently there is not other way.

    if (iFound)
    {
        // there is at least one event handler defined in class prototype associated with this CIH
        // so add this CIH to input set.
        if(!GetTargetSim()->GetAnimationTarget()->GetInputSet()->has(this))
        {
            GetTargetSim()->GetAnimationTarget()->GetInputSet()->add(this);
        }
    }
}

#if defined APT_USE_BUTTONS
/*F************************************************************************************************/
/*!
    \Function    AptCIH::gotoState

    \Description
        Takes care of frame jumping for prevFrame/nextFrame/gotoFrame

    \Input      nNewState         - new button state

    \Output

    \Notes
        - This is for Button ints only

    \Version    1.0        08/24/04 (LEO) First Version

*/
/************************************************************************************************F*/
void AptCIH::gotoState(AptCharacterButtonRecordState nNewState)
{
    AptCharacterButtonInst *pButtonInst = GetButtonInst();

    if (nNewState != pButtonInst->mnState)
    {
        pButtonInst->mnState = nNewState;

        pButtonInst->mDisplayList.clear();

        for (int i = 0; i < pButtonInst->GetCharacterConst()->button.nButtonRecords; i++)
        {
            if (pButtonInst->GetCharacterConst()->button.aButtonRecords[i].eStates & pButtonInst->mnState)
            {
                AptCharacterButtonRecord *pBR = &pButtonInst->GetCharacterConst()->button.aButtonRecords[i];
                AptCXForm tmpCXForm(&pBR->cxform);
                pButtonInst->mDisplayList.placeObject(NULL, i, pBR->pCharacter, NULL, this, 0, -1, &tmpCXForm, &pBR->matrix);
            }
        }
    }
}
#endif

/*F************************************************************************************************/
/*!
    \Function    AptCIH::jumpToFrame

    \Description
        takes care of frame jumping for prevFrame/nextFrame/gotoFrame

    In the case of doing a nextFrame, we can simply do a doFrameControls on the next frame.
    in the normal case of a jump ahead, we can optimize by using the current display list state and only adding to or
    removeing items that the AptPseudoDisplayList explicitly says to.
    In the case of a jump back, we need create a pseudo list from frame 0, then we need to remove any items from the
    original display list that are not in the new list, and either add items that is in the pseudo display list that
    are not in the original, or merge data in cases where there are items at the same depth.

    The AptPseudoDisplayList contains the final state of the display list for a given frame.  removeObject controls will be placed
    where placeObject control tags were, so if there is no item in the original display list at the removeObject control depth, no
    work needs to be done.

    \Input      nTargetFrame      - frame to goto

    \Output
        None.

    \Notes
        -This function was updated to use a psuedo list when going to a target frame.  This saves total number of allocations and cycles.

    \Version    1.0        08/24/04 (LEO) First Version
                2.0        09/31/04 (LEO) Second Version

*/
/************************************************************************************************F*/
void AptCIH::jumpToFrame(int nTargetFrame)
{
    if(IsNone())
        return;
    AptCharacterSpriteInstBase *pSprInstBase = GetSpriteInstBase();
    APT_ASSERT(pSprInstBase->GetCharacterConst()->sprite.movie.nFrames >= 0) ;
    if (nTargetFrame < 0 || nTargetFrame >= (int)pSprInstBase->GetCharacterConst()->sprite.movie.nFrames)
    {
        return;
    }

    if (nTargetFrame == pSprInstBase->mnFrame)
    {
        return;
    }
    else if (nTargetFrame == pSprInstBase->mnFrame + 1)
    {
        pSprInstBase->mnFrame = nTargetFrame;
        pSprInstBase->GetCharacterConst()->sprite.movie.doFrameControls(&pSprInstBase->mDisplayList, this, pSprInstBase->mnFrame);
    }
    else
    {
        AptNativeHash *pOldObject = pSprInstBase->GetNativeHash();
        AptPseudoDisplayList *pDisplayTest = new AptPseudoDisplayList(this);      // $TODO (LEO) we might just want to keep this guy around to save a new and delete every time

        bool bJumpAhead = pSprInstBase->mnFrame < nTargetFrame ? true : false;        // if bSimple == true means a jump ahead, less work needs to be done

        //$bug1004 (LEO) Newly added MC asserts when jumping frames
        if(pSprInstBase->mbJustLoaded == 1)
            pSprInstBase->mnFrame = 0;

        for (pSprInstBase->mnFrame = pSprInstBase->mnFrame < nTargetFrame ? pSprInstBase->mnFrame : 0;
            pSprInstBase->mnFrame <= nTargetFrame && pSprInstBase->mnFrame < (int)pSprInstBase->GetCharacterConst()->sprite.movie.nFrames;
            pSprInstBase->mnFrame++)
        {
            pSprInstBase->GetCharacterConst()->sprite.movie.DoTemporaryFrameControls(pDisplayTest, pSprInstBase->mnFrame);
        }

        pSprInstBase->mnFrame = nTargetFrame;
        pSprInstBase->mDisplayList.mergeState(pDisplayTest, pOldObject, bJumpAhead);
        delete pDisplayTest;
    }

    // now, one way or the other we have an up to date display list for the target frame; queue its actions
    pSprInstBase->mnGotoAnded = pSprInstBase->mnFrame;  // set to the current frame number
    pSprInstBase->GetCharacterConst()->sprite.movie.queueFrameActions(this, pSprInstBase->mnFrame);
}

/*F************************************************************************************************/
/*!
    \Function    AptCIH::HasEvent

    \Description
        returns true if the passed bit is set in the event flags.

    This function actually has no knowledge of what the bits mean :). So it is actually up to
    the caller to decide what they are used for. the CIH just stores the bits :)

    \Input      nEvent            - event id

    \Output     int               - 1 if AptCIH has that event handler defined otherwise 0

    \Notes
        None.

    \Version    1.0        04/13/05 (DC) Bringin' it to ya

*/
/************************************************************************************************F*/
int32_t AptCIH::HasEvent(int32_t nEvent)    // (LEO) Clip event shit
{
    return HasClipEvent(nEvent) || HasEventMember(nEvent);
}

/*F************************************************************************************************/
/*!
    \Function    AptCIH::HasClipEvent

    \Description
        Checks if the event is a clip action.

    Clip actions are events that are defined on the instance of a movieclip

    \Input      nEvent            - event id

    \Output     int32_t           - 1 if AptCIH has that clip event handler defined otherwise 0

    \Notes
        None.

    \Version    1.0        08/25/05 (LEO) First Version

*/
/************************************************************************************************F*/
int32_t AptCIH::HasClipEvent(int32_t nEvent)
{
    return GetSpriteInstBase()->HasClipAction(nEvent);
}

/*F************************************************************************************************/
/*!
    \Function    AptCIH::HasEventMember

    \Description
        Checks if the event is a member of the movieclip

    Event members are events that are defined via a class definition or in actionscript

    \Input      nEvent            - event id

    \Output     int32_t           - 1 if AptCIH has that event handler defined otherwise 0

    \Notes
        !! temporary fix for OnLoad !! Added extra check for mp__proto__. Should probably look through entire proto chain!

    \Version    1.0        08/25/05 (LEO) First Version
                           06/14/06 (DUG) Integrated Craig Nielson's (CWN) fix. Added testsuite_Test.tar.bz2. Now above comment is no longer valid.
*/
/************************************************************************************************F*/
int32_t AptCIH::HasEventMember(int32_t nEvent)    // (LEO) Clip event shit
{
    AptNativeHash * pHash = GetNativeHashVirtual();

    // CWN Missing base class implementations as the notes above indicate it would
    while (pHash)
    {
        int32_t iRet = pHash->HasEventHandler(nEvent);
        if (iRet) return iRet;
        AptValue *pProto = pHash->Get__Proto__();
        if (pProto)
        {
            pHash = pProto->GetNativeHashVirtual();
        }
        else
        {
            break;
        }
    }
    // Not found in CIH or prototype chain
    return 0;
    // END

}

/*F************************************************************************************************/
/*!
    \Function    AptCIH::FindAndSetEvents

    \Description
        This function will search for any events and set them in the native hash

    \Input
        None.

    \Output     int               -   returns 1 event hander is found


    \Version    1.0        10/27/04 (LEO) Added checks to only return true is a listenSet event is found.

*/
/************************************************************************************************F*/
int AptCIH::FindAndSetEvents()
{
    // First check if the event was defined in the instance of the sprite
    int iFound = 0;
    AptNativeHash *pNativeHash = GetNativeHashVirtual();

    for(int i = 0; i < CLIP_EVENT_ARRAY_SIZE; i++)
    {
        if(!pNativeHash->HasEventHandler(_aClipEvents[i].nFlag) && _aClipEvents[i].nFlag & AptEventActionFlag_AllEvents)
        {
            AptValue *pRet = findChild(StringPool::GetString(_aClipEvents[i].eName), NULL);
            if(pRet)
            {
                pNativeHash->SetEventHandler(_aClipEvents[i].nFlag);
#if defined(APT_USE_MOUSE)
                if(_aClipEvents[i].nFlag & (AptEventActionFlag_KeyEvents | AptEventActionFlag_MouseEvents))  // (LEO) added the AptEventActionFlag_MouseEvents to handle mouse events properly
#else
                if(_aClipEvents[i].nFlag & (AptEventActionFlag_KeyEvents))
#endif
                {
                    iFound = 1;
                }
            }
        }
    }
    return iFound;
}

/*F************************************************************************************************/
/*!
    \Function    AptCIH::queueClipEvents

    \Description
        This function queues events for the CIH.

    \Input      nEventFlags       - events that need to be queued
    \Input      input             - AptInput associated with event
    \Input      bFromListenerSet  - boolean to tell whether event is registered for listenersets

    \Output     bool              -

    \Notes

    It looks in clipactions and also looks in the whole parent hierarchy of the movieclip for
    the event handler and the adds it to action queue.

    \Version    1.0        06/16/04 (ASK) First Version

*/
/************************************************************************************************F*/
bool AptCIH::queueClipEvents(int nEventFlags, AptInput input, int bFromListenerSet)
{
    APT_PROFILE_INPUT_LOOKUP(queueClipEvents) ;
    if(IsDynamicTextInst())            // (LEO) ignore events for text feilds    6/
    {
        return false;
    }
    AptCharacterSpriteInstBase *pSprInstBase = GetSpriteInstBase();


    if(!HasEvent(nEventFlags))
    {
        return false;
    }

    bool bQueuedClipEvent = false;

    // ASK  // in cases where there a class associated with the movie clip, it cannot be
    // decided just looking at mnObjectClipActions whether event handler is implemented or not
    // as mnObjectClipActions is only updated for this instance and not because any of the class
    // associated with it has that event handler.
    // this is more time consuming, but right now there is no other option

    if (pSprInstBase->mpClipActions)
    {
        for (int i=0; i < pSprInstBase->mpClipActions->nEventActions; i++)
        {
            if (pSprInstBase->mpClipActions->aEventActions[i].nTriggers & nEventFlags)
            {
                switch (nEventFlags)
                {
                case AptEventActionFlag_EnterFrame:
                {
                    GetTargetSim()->GetAnimationTarget()->AddActionFront(&pSprInstBase->mpClipActions->aEventActions[i].actions, this,
                        ACTION_PROF_CALL_PARAM(StringPool::GetString(SC_onEnterFrame)->GetBuffer())
                        ACTION_TYPE_CALL_PARAM(AptEventActionFlag_EnterFrame)
                        gNullInput);

                    bQueuedClipEvent = true;
                }
                break;
                case AptEventActionFlag_KeyPress:
                {
                    AptInputType eType;
                    GET_ACTION_TYPE(input,eType);
                    if (pSprInstBase->mpClipActions->aEventActions[i].nKeyCode == eType)
                    {
                        GetTargetSim()->GetAnimationTarget()->AddActionFront(&pSprInstBase->mpClipActions->aEventActions[i].actions, this,
                            ACTION_PROF_CALL_PARAM(StringPool::GetString(SC_onPress)->GetBuffer())
                            ACTION_TYPE_CALL_PARAM(AptEventActionFlag_KeyPress)
                            input);

                        bQueuedClipEvent = true;
                    }
                }
                break;
                case AptEventActionFlag_Initialize:
                case AptEventActionFlag_Construct:                                          // Added this for Flash7's new Construct ClipEvent
                case AptEventActionFlag_Unload:                                             // Added for onUnload action event
                {
                    // DC 18.00 - Changed to use AptScriptFunctionBase (Reduces duplication of code).
                    const AptNativeString *sEvent = StringPool::GetString( (nEventFlags==AptEventActionFlag_Initialize) ? SC_onLoad : SC_onUnload);

#if APT_PROF_ENABLED && APT_PROFILE_ACTIONSCRIPT_ENABLED
                    APT_PROF_ENTER((char8_t *) sEvent->GetBuffer());
#endif

                    AptScriptFunctionByteCodeBlock *pFuncValue = new AptScriptFunctionByteCodeBlock(
                        pSprInstBase->mpClipActions->aEventActions[i].actions.aActionStream,
                        -1,
                        gAptActionInterpreter.constantPool,  /* Should be the current constant pool */
                        sEvent->GetBuffer(),
                        this
                        );

                    APT_DEFINE_ACTION_SETUP(this, pFuncValue, "AptClipEvents", (AptActionType)nEventFlags);
                    void * pSavedValue   = gAptActionInterpreter.PrepareForExecution(&oActionSetup);

                    gAptActionInterpreter.thisStack.push(this);

                        APT_INC(pFuncValue);
                    gAptActionInterpreter.callFunction( this, pFuncValue, 0);
                    APT_ASSERT(pFuncValue->getRefCount() == 1);
                        APT_DEC(pFuncValue);

                    gAptActionInterpreter.thisStack.pop();
                    gAptActionInterpreter.CleanupAfterExecution(pSavedValue, &oActionSetup);  //oActionSetup is defined by the macro

#if APT_PROF_ENABLED && APT_PROFILE_ACTIONSCRIPT_ENABLED
                    APT_PROF_LEAVE((char8_t *)sEvent->GetBuffer());
#endif

                    bQueuedClipEvent = true;
                    return bQueuedClipEvent;
                }
                break ;
                default:
                {
#if APT_PROF_ENABLED && APT_PROFILE_ACTIONSCRIPT_ENABLED
                    int iClipEvent = APT_ARRAYSIZE(_aClipEvents);
                    AptNativeString strEvent;
                    while (iClipEvent--)
                    {
                        if (_aClipEvents[iClipEvent].nFlag == nEventFlags)
                        {
                            break;
                        }
                    }
                    APT_ASSERT(iClipEvent >= 0);
#endif

                    GetTargetSim()->GetAnimationTarget()->AddActionBack(&pSprInstBase->mpClipActions->aEventActions[i].actions, this,
                        ACTION_PROF_CALL_PARAM(StringPool::GetString(_aClipEvents[iClipEvent].eName)->GetBuffer())
                        ACTION_TYPE_CALL_PARAM(nEventFlags)
                        input);

                    bQueuedClipEvent = true;
                }
                break;
                }
            }
        }
    }

    // default is set to false.
    if (bFromListenerSet == false)
        return bQueuedClipEvent ;
    // again here mnObjectClipActions is not up to date with event handlers in associated class.

    if(!HasEventMember(nEventFlags))
    {
        return bQueuedClipEvent;
    }

    int32_t nEventIndex = -1;
    for (int i = 0; i < CLIP_EVENT_ARRAY_SIZE; i++)
    {
        if (_aClipEvents[i].nFlag & nEventFlags)
        {
            nEventIndex = i;
            break;
        }
    }

    if(nEventIndex == -1)           // (LEO) Jump out early mang!
    {
        return bQueuedClipEvent;
    }

    AptNativeHash * pThisNativeHash = GetNativeHashVirtual();
    AptValue *pValue = pThisNativeHash->Lookup(StringPool::GetString(_aClipEvents[nEventIndex].eName));
    if(pValue)
    {
        if(_aClipEvents[nEventIndex].nFlag == AptEventActionFlag_RollOut || _aClipEvents[nEventIndex].nFlag == AptEventActionFlag_RollOver)
        {
            // We need to send the RollOver/Out to the back so they get executed in the correct order.
            GetTargetSim()->GetAnimationTarget()->AddFunctionBack(this, pValue, 0,
                ACTION_PROF_CALL_PARAM(StringPool::GetString(_aClipEvents[nEventIndex].eName)->GetBuffer())
                ACTION_TYPE_CALL_PARAM(_aClipEvents[nEventIndex].nFlag)
                input);
        }
        else
        {
            GetTargetSim()->GetAnimationTarget()->AddFunctionFront(this, pValue, 0,
                ACTION_PROF_CALL_PARAM(StringPool::GetString(_aClipEvents[nEventIndex].eName)->GetBuffer())
                ACTION_TYPE_CALL_PARAM(_aClipEvents[nEventIndex].nFlag)
                input);
        }
        bQueuedClipEvent = true;
    }
    else
    {
        AptValue *pValue = findChild(StringPool::GetString(_aClipEvents[nEventIndex].eName), NULL);
        if(pValue && pValue->getIsDefined())
        {
            AptScriptFunctionBase * pFuncBase = pValue->c_scriptfunction() ;
            if(pFuncBase->mpCIH != this)
            {
                pValue = pFuncBase->Duplicate( this );

                AptCIH * pTempCIH = pFuncBase->mpCIH;

                pValue->c_scriptfunction()->mpParentAnim->DecZombieCount();          // (LEO) We also need to dec the Zombie counter!
                APT_DEC(pValue->c_scriptfunction()->mpParentAnim);
                //pValue->c_scriptfunction()->mpParentAnim = (AptCIH *) pTempCIH;      // (LEO) DOING THIS IS REALLY REALLY BAD, WE SHOULD NEVER CHAGNE THE mpParentAnim POINTER SINCE IT IS USED TO MAINTAIN THE ZOMBIE REFERENCES!!!!
                pValue->c_scriptfunction()->mpParentAnim = pTempCIH->GetRootAnimation(); // pFuncBase->mpParentAnim ;
                pValue->c_scriptfunction()->mpParentAnim->IncZombieCount();          // (LEO) Now we need to inc the new parents zombie counter
                APT_INC(pValue->c_scriptfunction()->mpParentAnim);

                pValue->setGCRoot(1);     // Must Make sure that event handlers don't get Garbage collected!
            }

            //[ #772 ] DUG MovieClip::attachMovie(), onLoad() not being called

            if(_aClipEvents[nEventIndex].nFlag == AptEventActionFlag_RollOut || _aClipEvents[nEventIndex].nFlag == AptEventActionFlag_RollOver || _aClipEvents[nEventIndex].nFlag == AptEventActionFlag_OnLoad)
            {
                // We need to send the RollOver/Out to the back so they get executed in the correct order.
                GetTargetSim()->GetAnimationTarget()->AddFunctionBack(this, pValue, 0,
                    ACTION_PROF_CALL_PARAM(StringPool::GetString(_aClipEvents[nEventIndex].eName)->GetBuffer())
                    ACTION_TYPE_CALL_PARAM(_aClipEvents[nEventIndex].nFlag)
                    input);
            }
            else
            {
                GetTargetSim()->GetAnimationTarget()->AddFunctionFront(this, pValue, 0,
                    ACTION_PROF_CALL_PARAM(StringPool::GetString(_aClipEvents[nEventIndex].eName)->GetBuffer())
                    ACTION_TYPE_CALL_PARAM(_aClipEvents[nEventIndex].nFlag)
                    input);
            }

            bQueuedClipEvent = true;
        }
    }
    return bQueuedClipEvent;

}

/*F************************************************************************************************/
/*!
    \Function    AptCIH::tick

    \Description
        takes care of regular frame advancement when the frame rate counter has ticked to the next tick
        tick()s all child sprites (that are active in its displayList)

    \Input

    \Output

    \Notes
        None.

    \Version    1.0        08/24/04 (LEO) First Version
    \Version    1.1        12/11/07 (JP) Restoring the early return when is not dirty and removing an ugly goto

*/
/************************************************************************************************F*/
uint32_t AptCIH::tick()
{
    // Restored. Removed in CL 33160, but was causing Sim thread hogging (Issue 112370).
    if(mbDirty == 0 || IsSkipEval())        // If we are not dirty, jump out early
    {
        return 0;
    }

    bool bDoFrameControls = true; // To avoid an ugly goto that was in the code

	// FIFA_BEGIN | SENDBACK | gmicle | 2018-10-23 | https://jira.frostbite.ea.com/browse/FB-108141 - Move performance markers from inside recursive functions to around their entry points, so that they work with Frostbite profiling tools (Delta Viewer).
    //APT_PROFILE_BEGIN( APT_CIH_TICK, (mMyName.c_str()[0] != false) ? mMyName.c_str() : "AptCIH::tick" );
    // FIFA_END

    if (IsSpriteInstBase())
    {
        AptCharacterSpriteInstBase *pSpriteInst = GetSpriteInstBase();

		// FIFA_BEGIN | SENDBACK | gmicle | 2018-10-23 | https://jira.frostbite.ea.com/browse/FB-108141 - Move performance markers from inside recursive functions to around their entry points, so that they work with Frostbite profiling tools (Delta Viewer).
        //APT_PROFILE(sprite_tick);
        // FIFA_END
		pSpriteInst->mnGotoAnded = 0;                       // We can simply reset the nGotoAnded flag here..

        if (pSpriteInst->mbIsPlaying || (pSpriteInst->mbJustLoaded && gAptInitParms.bUseNewClassInitOrder) )
        {
            // dynamically created emptymovieclip has bCreatedDynamic set to true.
            if(pSpriteInst->GetCreatedDynamic())
            {
                pSpriteInst->mnFrame = 0;
            }
            else
            {
                pSpriteInst->mnFrame++;
            }
            if (pSpriteInst->mnFrame == 1 && pSpriteInst->GetCharacterConst()->sprite.movie.nFrames == 1)
            {
                pSpriteInst->mnFrame = 0;
                bDoFrameControls = false;
            }
            else if (pSpriteInst->mnFrame == (int)pSpriteInst->GetCharacterConst()->sprite.movie.nFrames)
            {
                // this has to be jump to frame 0, not just clear the DL and replay the first frame
                // because buttons/sprites need to maintain their state from the last->first frame, which jump does
                APT_PROFILE_BEGIN( APT_CIH_TICK_ACTION, "AptCIH::jumpToFrame" );
                jumpToFrame(0);
                APT_PROFILE_END( APT_CIH_TICK_ACTION );
                // for fixing Bug 552 and  FR 283 - onEnterFrame to work on created movie clip
                // previously we were not calling queueclipevents for onenterframe when
                // we were on last frame, but for emptymovieclip we have only one frame so we are always on last frame.
                bDoFrameControls = false;
            }
        }

        //  APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, "%p.nFrame: %d\n", this, nFrame);
        if(bDoFrameControls)
        {
            if (pSpriteInst->mbIsPlaying || (pSpriteInst->mbJustLoaded && gAptInitParms.bUseNewClassInitOrder) )
            {
                // do control tags (but not actions)
				// FIFA_BEGIN | SENDBACK | gmicle | 2018-10-23 | https://jira.frostbite.ea.com/browse/FB-108141 - Move performance markers from inside recursive functions to around their entry points, so that they work with Frostbite profiling tools (Delta Viewer).
                //APT_PROFILE_BEGIN( APT_CIH_TICK_ACTION, "AptCIH::doFrameControls" );
                pSpriteInst->GetCharacterConst()->sprite.movie.doFrameControls(&pSpriteInst->mDisplayList, this, pSpriteInst->mnFrame);
                //APT_PROFILE_END( APT_CIH_TICK_ACTION );
                // FIFA_END
			}

            if (pSpriteInst->mbIsPlaying || (pSpriteInst->mbJustLoaded && gAptInitParms.bUseNewClassInitOrder) )
            {
                APT_PROFILE_BEGIN( APT_CIH_TICK_ACTION, "AptCIH::queueFrameActions" );
                pSpriteInst->mnGotoAnded = -1 * pSpriteInst->mnFrame;               // for queued frame actions we set to negative for queued obj
                pSpriteInst->GetCharacterConst()->sprite.movie.queueFrameActions(this, pSpriteInst->mnFrame);
                pSpriteInst->mnGotoAnded = pSpriteInst->mnFrame;                    // reset to original val
                APT_PROFILE_END( APT_CIH_TICK_ACTION );
            }
        }

        // adding extra comparison is part of FR 380 - Async load complete function
        if (!pSpriteInst->mbJustLoaded || IsAnimationInst())
        {
            //if (AptOptIsEnabled(APT_OPT_NO_ONENTER))      // (LEO) with the new mnObjectClipActions flag, we no longer need this, it is automatically done!
            if(HasEvent(AptEventActionFlag_EnterFrame))
            {
                APT_PROFILE_BEGIN( APT_CIH_TICK_ACTION, "AptCIH::queueClipEvents - onEnterFrame" );
                APT_PROFILE_INPUT_LOOKUP(QueueClipEvent_onEnterFrame);
                queueClipEvents(AptEventActionFlag_EnterFrame);
                APT_PROFILE_END( APT_CIH_TICK_ACTION );
            }
        }
        if (pSpriteInst->mbJustLoaded)
        {
            if(HasEvent(AptEventActionFlag_OnLoad))
            {
                APT_PROFILE_BEGIN( APT_CIH_TICK_ACTION, "AptCIH::queueClipEvents -- onLoad" );
                APT_PROFILE_INPUT_LOOKUP(QueueClipEvent_onLoad);
                queueClipEvents(AptEventActionFlag_OnLoad) ;
                APT_PROFILE_END( APT_CIH_TICK_ACTION );
            }
            pSpriteInst->mbJustLoaded = 0;
        }

        uint32_t bDirty = pSpriteInst->mDisplayList.tick();         // Get the branch dirty state
        if(!HasEvent(AptEventActionFlag_EnterFrame))                // Do only if we don't have a onEnterFrame
        {
            if(pSpriteInst->mbIsPlaying == false || pSpriteInst->GetCharacterConst()->sprite.movie.nFrames == 1)    // If I'm not playing or I only have one frame, update
            {
                mbDirty = bDirty;
            }
        }
        else                                                        // Else I must be dirty
        {
            mbDirty = 1;
        }

        //$rburkett AptCIH::ProcessCustomControls(this, NULL) ; // $bug 977 - Custom controls not working with decoupled rendering.
    }
#if defined APT_USE_BUTTONS
    else if (IsButtonInst())
    {
        AptCharacterButtonInst *pButtonInst = GetButtonInst();
        uint32_t bDirty = pButtonInst->mDisplayList.tick();         // Buttons are hardly used, but we must do the same here with the dirty flag
        mbDirty = bDirty;
    }
#endif
	// FIFA_BEGIN | SENDBACK | gmicle | 2018-10-23 | https://jira.frostbite.ea.com/browse/FB-108141 - Move performance markers from inside recursive functions to around their entry points, so that they work with Frostbite profiling tools (Delta Viewer).
    //APT_PROFILE_END( APT_CIH_TICK );
    // FIFA_END
	return mbDirty;
}

/*F************************************************************************************************/
/*!
    \Function    AptCIH::GeneralisedProcess

    \Description
        This is a generalized AptCIH process function

    This gets called from AptDisplayList::GeneralisedProcess


    \Input      pCIH              - pointer to AptCIH that will get processed
    \Input      pVoid             - void pointer

    \Output
        None.

    \Notes
        As this is static function we pass in AptCIH that needs to be processed
        and pVoid pointer is added which will be forwarded to callback function.
        This function calls the sCIHProcessCb function that has been set before
        start of the AptDisplayList::GeneralisedProcess at root level.
        It then calls AptDisplayList::GeneralisedProcess recursively on its display list.

    \Version    1.0        04/22/05 (ASK) First Version
    \Version    1.1        09/02/05 (DC)  Added Optimizations to bail out if non-visible
    \Version    1.2        09/03/05 (DC)  Added mbChildGPDirty flag to address some items not being processed.
    \Version    1.3        03/06/07 (ASK)  Added a static variable to AptCIH that decides if we want to early return
                                from this function or not. We will set it to false in cases when we are calling GeneralisedProcess
                                from functions like AptPrintAllDynamicText just to print all text instances.
                                Also added nTreeDepth to determine what level we are in at time of printing.
*/
/************************************************************************************************F*/
uint32_t     AptCIH::GeneralisedProcess(AptCIH * pCIH, void * pVoid)
{
    // Opt Note: Preload AptChar first to avoid register dependencies when loading
    // from aptChar pointer later.
    AptCharacterInst* aptChar = pCIH->GetCharacterInst();

    if (pCIH->IsSkipEval())
    {
        return 0;
    }

    if((AptCIH::bEarlyReturn) &&
        ( pCIH->getIsDefined() == false
        || pCIH->GetCIHState() == AptCIHState_Unloaded
        || !aptChar->GetIsVisible())
        )
    {
        return 0;
    }

	// FIFA_BEGIN | SENDBACK | gmicle | 2018-10-23 | https://jira.frostbite.ea.com/browse/FB-108141 - Move performance markers from inside recursive functions to around their entry points, so that they work with Frostbite profiling tools (Delta Viewer).
    //APT_PROFILE_BEGIN( APT_CIH_GENERALISED_PROCESS, pCIH->GetInstanceName().c_str()[0] != false ? pCIH->GetInstanceName().c_str() : "AptCIH::GeneralisedProcess" );
    // FIFA_END
	uint32_t bState = 0;
    if(AptCIH::sCIHProcessCb != NULL)
        bState |= AptCIH::sCIHProcessCb(pCIH, pVoid) ;
    if(AptCIH::sCIHProcessCb1 != NULL)
        bState |= AptCIH::sCIHProcessCb1(pCIH, pVoid) ;
    if(AptCIH::sCIHProcessCb2 != NULL)
        bState |= AptCIH::sCIHProcessCb2(pCIH, pVoid);

#if defined(APT_USE_BUTTONS)
    if(AptCIH::sCIHButtonProcessCb != NULL)
        bState |= AptCIH::sCIHButtonProcessCb(pCIH, pVoid);
#endif

    if (pCIH->IsSpriteInstBase())
    {
        AptCIH::nTreeDepth++;
        AptCharacterSpriteInstBase *pSpriteInst = pCIH->GetSpriteInstBase();
        bState |= AptDisplayList::GeneralisedProcess(&pSpriteInst->mDisplayList, pVoid) ;
        AptCIH::nTreeDepth--;
    }
#if defined APT_USE_BUTTONS
    else if (pCIH->IsButtonInst())
    {
        AptCIH::nTreeDepth++;
        AptCharacterButtonInst *pButtonInst = pCIH->GetButtonInst();
        bState |= AptDisplayList::GeneralisedProcess(&pButtonInst->mDisplayList, pVoid) ;
        AptCIH::nTreeDepth--;
    }
#endif
	// FIFA_BEGIN | SENDBACK | gmicle | 2018-10-23 | https://jira.frostbite.ea.com/browse/FB-108141 - Move performance markers from inside recursive functions to around their entry points, so that they work with Frostbite profiling tools (Delta Viewer).
    //APT_PROFILE_END( APT_CIH_GENERALISED_PROCESS );
    // FIFA_END
	return bState;
}



/*F************************************************************************************************/
/*!
    \Function    AptCIH::ProcessTextInst

    \Description
        Callback function for processing text instances

    \Input      pCIH              - AptCIH to be processed
    \Input      pVoid             - not used

    \Output
        None.

    \Notes
        This gets called through recursive pass of displaylist at end of AptUpdate
        to process all the text instances.

    \Version    1.0        04/22/05 (ASK) First Version

*/
/************************************************************************************************F*/
uint32_t AptCIH::ProcessTextInst(AptCIH * pCIH, void * pVoid)
{
    if(pCIH->IsDynamicTextInst())
    {
        if(pCIH->IsVisible())
        {
            AptCharacterTextInst *  pTextInst   = pCIH->GetDynamicTextInst();
            const AptAssetString zID = GetZID(pTextInst);
            if (zID == NULL || zID == &AptCIH::sEmptyAssetString || ((pTextInst->GetStateFlags() & APT_TEXTFIELD_NONE) == 0) )
            {
                pCIH->EnsureStringAllocated(pCIH->c_cih()->GetDisplayListParent()) ;    //, AptStringCBType_BatchUpdate) ;
            }
        }
        return 1;
    }
    return 0;
}




/*F************************************************************************************************/
/*!
    \Function    AptCIH::ProcessTextInstPrint

    \Description
    Callback function for processing text instances

    \Input      pCIH              - AptCIH to be processed
    \Input      pVoid             - not used

    \Output
        None.

    \Notes
    This gets called through recursive pass of displaylist from function AptPrintAllDynamicText
    to print all the text instances.

    \Version    1.0        03/06/07 (ASK) First Version

*/
/************************************************************************************************F*/
uint32_t AptCIH::ProcessTextInstPrint(AptCIH * pCIH, void * pVoid)
{
    if(pCIH->IsDynamicTextInst())
    {
        APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, "Dynamic Text [0X%p visible = %d  name =\"%s\"  parent =\"%s\"  text =\"%s\"]\n",
            pCIH,
            pCIH->GetDynamicTextInst()->GetIsVisible(),
            pCIH->mMyName.c_str(),
            pCIH->GetDisplayListParent() ? pCIH->GetDisplayListParent()->mMyName.c_str() : "",
            pCIH->GetDynamicTextInst()->GetTextValueConst().c_str()
            );
    }
    return 1;
}


static char     s_aPrintChar[64];
static char     s_cPrintChar = '\t';
static char     s_aEndChar1[] = ">";
static char     s_aEndChar2[] = "/>";
static char     s_aEndTag[] = "</TreeDepth";
/*F************************************************************************************************/
/*!
    \Function    AptCIH::PrintMovieclipTree

    \Description
    Callback function for printing movieclip tree

    \Input      pCIH              - AptCIH to be processed
    \Input      pVoid             - not used

    \Output
    None.

    \Notes
    This gets called through recursive pass of displaylist from function AptPrintMovieclipTree
    to print movieclip tree

    \Version    1.0        03/06/07 (ASK) First Version

*/
/************************************************************************************************F*/
uint32_t  AptCIH::PrintMovieclipTree(AptCIH * pCIH, void * pVoid)
{
    memset((void *)s_aPrintChar, s_cPrintChar, AptCIH::nTreeDepth );
    s_aPrintChar[AptCIH::nTreeDepth] = 0;
    char * pszEndChars = s_aEndChar2;

    const size_t MAX_LINE_LEN = 512u;
    const size_t MAX_TAG_LEN = 16u;

    // Three digits is the maximum that is guaranteed to fit into the char buffer. Do not increase this
    // without also increasing MAX_TAG_LEN to make room for additional characters, or we are going to
    // stomp all over your memory.
    const int MAX_TREE_DEPTH = 999u;

    char aszEndTag[MAX_LINE_LEN] = "";
    char aszEndTag1[MAX_TAG_LEN] = "";

    if(pCIH->GetFirstChild())
    {
        pszEndChars = s_aEndChar1;
    }

    if (pCIH->GetDisplayListNext() == NULL && pCIH->GetFirstChild() == NULL && AptCIH::nTreeDepth > 0)
    {
        AptCIH * pTemp = pCIH;
        int nTempTreeDepth = AptCIH::nTreeDepth;

        do
        {
            if(nTempTreeDepth > MAX_TREE_DEPTH)
            {
                APT_ASSERTM(false, "AptCIH::PrintMovieclipTree - depth is too great and not supported.");
                break;
            }
            sprintf(aszEndTag1, "%s%d>", s_aEndTag, nTempTreeDepth-1);

            if (strlen(aszEndTag) + strlen(aszEndTag1) >= MAX_LINE_LEN)
            {
                APT_ASSERTM(false, "AptCIH::PrintMovieclipTree - output line too long.");
                break;
            }
            strcat(aszEndTag, aszEndTag1);

            pTemp = pTemp->GetDisplayListParent();
            nTempTreeDepth--;
        } while(pTemp && pTemp->GetDisplayListNext() == NULL && nTempTreeDepth > 1);
    }
    APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, "%s<TreeDepth%d value=\"0X%p\" type=\"%d\" visible=\"%d\" depth=\"%d\" name=\"%s\"%s %s\n",
            s_aPrintChar,
            AptCIH::nTreeDepth,
            pCIH,
            pCIH->GetCharacterInst()->GetCharacterType(),
            pCIH->GetCharacterInst()->GetIsVisible(),
            pCIH->GetDepth(),
            pCIH->mMyName.c_str(),
            pszEndChars,
            aszEndTag);

    return 1;
}


/*F************************************************************************************************/
/*!
    \Function    AptCIH::ProcessCustomControls

    \Description
        Callback function for processing custom controls

    \Input      pCIH              - AptCIH to be processed
    \Input      pVoid             - not used

    \Output
        None.

    \Notes
        This gets called through recursive pass of displaylist at end of AptUpdate
        to process all the custom controls.


    \Version    1.0        05/01/05 (ASK) First Version

*/
/************************************************************************************************F*/
uint32_t AptCIH::ProcessCustomControls(AptCIH * pCIH, void * pVoid)
{
    if (pCIH->IsSpriteInstBase())
    {
        AptCharacterSpriteInstBase *pSpriteInst = pCIH->GetSpriteInstBase();
        if (pSpriteInst->mnIsCustomControl == AptCharacterSpriteInstBase::CustomControlState_Unknown)
        {
            AptNativeHash *pHash = pSpriteInst->GetNativeHash();
            AptValue *pType = pHash ? pHash->Lookup(StringPool::GetString(SC__type)) : NULL;
            AptValue *pZid = pHash ? pHash->Lookup(StringPool::GetString(SC__CustomControlType)) : NULL;

            if (pType)
            {
                pSpriteInst->mnIsCustomControl = AptCharacterSpriteInstBase::CustomControlState_IsCustomControl;
            }
#if defined (APT_CUSTOM_CONTROL_USE_ZID)
            else if(pZid)
            {
                pSpriteInst->mnIsCustomControl = AptCharacterSpriteInstBase::CustomControlState_IsCustomControlZid;
            }
#endif
            else
            {
                pSpriteInst->mnIsCustomControl = AptCharacterSpriteInstBase::CustomControlState_IsNotCustomControl;
            }
        }

        AptRenderItemCustomControl *pItem = 0;

        // Double negatives much?
        if (pSpriteInst->mnIsCustomControl != AptCharacterSpriteInstBase::CustomControlState_IsNotCustomControl)
        {
            if (pSpriteInst->GetRenderItem()->GetCharacterType() != AptCharacterType_CustomControl)
            {
                const AptRenderItem * pOriginal = pSpriteInst->GetRenderItem() ;
                APT_ASSERT(pOriginal);

                AptCharacterType eType = pOriginal->GetCharacterType();

                if (eType == AptCharacterType_Sprite)
                    pItem = AptRenderItemCustomControl::CopyFromSprite((AptRenderItemSprite *)pOriginal, gnCurrUpdateTick, false);
                else if (eType == AptCharacterType_CustomControl)
                    pItem = (AptRenderItemCustomControl *)pOriginal;

                APT_ASSERT(pItem && "unknown type!");

                pSpriteInst->SetRenderItem(pItem) ;
                AptCharacterInst::ItemMoved(pCIH);

#if defined (APT_GATHER_MOVIECLIP_METRICS)
                AptCharacterInst::UpdateMovieClipInfo(pOriginal->GetCharacterType(), &gAptMovieclipInformation, -1);
                AptCharacterInst::UpdateMovieClipInfo(pItem->GetCharacterType(), &gAptMovieclipInformation, 1);
#endif
            }

            AptCIH *pRectObject = pSpriteInst->mDisplayList.getState()->GetFirstItem();
            APT_ASSERTM((pRectObject && pRectObject->IsShapeInst()), "Movieclip getting converted into custom-control object should only have a single shape rectangle inside it");
            AptValue *pTarget = gAptActionInterpreter.getVariable(pCIH, NULL, StringPool::GetString(SC__target));

            if (!pItem) pItem = (AptRenderItemCustomControl *)pSpriteInst->GetRenderItem();

            if (pSpriteInst->mnIsCustomControl == AptCharacterSpriteInstBase::CustomControlState_IsCustomControl)
            {
                AptValue *pType = pSpriteInst->GetNativeHash() ? pSpriteInst->GetNativeHash()->Lookup(StringPool::GetString(SC__type)) : NULL;

                // customcontrolrenderitem is already been created in previous ticks
                // for now always create a new revision , we will optimize later.

                AptNativeString sCustomProp1;

                AptRenderableGeometry *pGeometry = pRectObject->GetShapeInst()->GetCharacterConst()->shape.pRenderUnit;

                APT_ASSERT(pGeometry);

                if(gAptFuncs.pfnCustomControlUpdate == NULL || gAptFuncs.pfnCustomControlUpdate(pGeometry->mRenderUnit) == true)
                {
                    pItem->SetCustomPropertiesStr(pCIH->urlEncodeCustomRender()) ;
                }
                pItem->SetTargetStr(*pTarget->c_string()->GetInternalString()) ;
                APT_ASSERT(pType);
                if(pType)
                {
                    pItem->SetTypeStr(*pType->c_string()->GetInternalString()) ;
                }
            }
#if defined (APT_CUSTOM_CONTROL_USE_ZID)
            else if(gAptInitParms.bCustomControlUseZid && pSpriteInst->mnIsCustomControl == AptCharacterSpriteInstBase::CustomControlState_IsCustomControlZid)
            {
                // customcontrolrenderitem is already been created in previous ticks
                // for now always create a new revision , we will optimize later.

                // call callback to create the zid this should happen everytime
                APT_ASSERT(gAptFuncs.pfnCreateCustomControlZid != NULL) ;
                APT_PROFILE_AUXILIARY(pfnCreateCustomControlZid);

                AptRenderableGeometry *pGeometry = pRectObject->GetShapeInst()->GetCharacterConst()->shape.pRenderUnit;

                APT_ASSERT(pGeometry);

                AptAssetCustomControlZId pZid = gAptFuncs.pfnCreateCustomControlZid(
                    pGeometry->mRenderUnit,
                    pTarget->c_string()->GetInternalString()->c_str(),
                    (AptValue *)pCIH,
                    pItem->GetZidRef(),
                    false);

                pItem->SetZId( pZid );
            }
#endif
            return 1;
        }
    }
    return 0;
}

/*F************************************************************************************************/
/*!
    \Function    AptCIH::ProcessMaskMatricies

    \Description
        This is a callback to set the matrix information for masks

    \Input      pCIH              - AptCIH to be processed
    \Input      pVoid             - not used

    \Output
        None.

    \Notes
        None.

    \Version    1.0        05/10/05 (LEO) First Version

*/
/************************************************************************************************F*/
uint32_t AptCIH::ProcessMaskMatricies(AptCIH *pCIH, void *pVoid)
{
    if (pCIH->IsMask())
    {
        // DC - Masks need to have some alpha or come aux libs won't work. Since they aren't visibly rendered anyway,
        // it seems safe to bump up the alpha to a more reasonable level.
        if( pCIH->GetCharacterInst()->GetColorMatrixConst()->scale.GetValuef(AptColorHelper::Alpha) < (float)0x33 ) // This use to be compared to 0.20f
        {
            pCIH->GetCharacterInst()->GetColorMatrixWritable()->scale.SetValuef(AptColorHelper::Alpha, 0x33);
        }

        AptMatrix cur = gIdentityMatrix;
        AptCIH *pParent = pCIH->GetDisplayListParent();

        while (pParent != NULL)
        {
            const AptMatrix * pMatrix = pParent->GetPositionMatrixConst();
            gpRenderingContext->multMatrix(pMatrix, &cur, &cur);

            pParent = pParent->GetDisplayListParent();
        }

        pCIH->SetIsMask(true, &cur);
        return 1;
    }
    return 0;
}

#if defined APT_USE_BUTTONS
/*F************************************************************************************************/
/*!
    \Function    AptCIH::ProcessButtonState

    \Description
        This is a call back function set set buttons in the ButtonBIL list

    \Input      pCIH              - AptCIH to be processed
    \Input      pVoid             - not used

    \Output
        None.

    \Notes
        None.

    \Version    1.0        05/10/05 (LEO) First Version
*/
/************************************************************************************************F*/
uint32_t AptCIH::ProcessButtonState(AptCIH *pCIH, void *pVoid)
{
    if (pCIH->IsButtonInst())
    {
        AptMatrix cur = gIdentityMatrix;
        AptCIH *pParent = pCIH->GetDisplayListParent();

        while (pParent != NULL)
        {
            const AptMatrix * pMatrix = pParent->GetPositionMatrixConst();
            gpRenderingContext->multMatrix(pMatrix, &cur, &cur);

            pParent = pParent->GetDisplayListParent();
        }

        GetTargetSim()->GetAnimationTarget()->AppendButtonToBIL(pCIH, &cur);
        return 1;
    }
    return 0;
}
#endif

/*F************************************************************************************************/
/*!
    \Function    AptCIH::CheckIfHigher

    \Description
        Checks if passed in AptCIH is at higher depth in displaylist tree.

    \Input      *pSptTmp          - AptCIH to be checked

    \Output     bool              - true if passed in AptCIH is on top of this AptCIH

    \Notes
    -Updated algorithm since previous version didn't jump out early enough.

    \Version    1.0        06/23/04 (LEO) First Version
    1.1        07/21/04 (LEO) This was part of:
    -585    NHL - single mouse clicks should activate options

*/
/************************************************************************************************F*/
bool AptCIH::CheckIfHigher(const AptCIH *pSptTmp) const
{
    int nCount = GetParentCount();

    if(this == pSptTmp || nCount == 0)
    {
        return false;                   // Either they are the same, or this is the root
    }

    for(int i = 0; i <= nCount; i++)
    {
        int i1 = this->GetDepthOfParentAt(i) ;
        int i2 = pSptTmp->GetDepthOfParentAt(i);
        if(i1 > i2)
        {
            return true;                // This sprite is higher then the one passed in!
        }
        else if(i2 > i1)
        {
            return false;
        }
    }
    return false;                       // The passed in sprite is higher
}

/*F************************************************************************************************/
/*!
    \Function    AptCIH::GetParentCount

    \Description
        Returns the number of parents up the parent chain

    \Input

    \Output     int               - total number of parents in displaylist

    \Notes
    None.

    \Version    1.0        06/23/04 (LEO) First Version

*/
/************************************************************************************************F*/
int AptCIH::GetParentCount() const
{
    int nCount = 0;
    AptCIH *pSprtTmp = this->GetDisplayListParent();

    while(pSprtTmp)
    {
        nCount++;
        pSprtTmp = pSprtTmp->GetDisplayListParent();
    }
    return nCount;
}

/*F************************************************************************************************/
/*!
    \Function    AptCIH::GetDepthOfParentAt

    \Description
        Returns the nDepth value of the parent at the passed in parent chain depth

    \Input      nLvl              - Depth to be checked against

    \Output     int               -     nDepth of parent, or -1 if no parent found at the passed in depth

    \Notes
    None.

    \Version    1.0        06/23/04 (LEO) First Version

*/
/************************************************************************************************F*/
int AptCIH::GetDepthOfParentAt(int nLvl) const
{
    APT_ASSERT(nLvl >= 0);
    int nMyDepth = GetParentCount();

    const AptCIH *pSprtTmp = mpParent;

    if(nLvl > nMyDepth)
    {
        return -1;
    }

    pSprtTmp = this;
    while(nMyDepth != nLvl)
    {
        pSprtTmp = pSprtTmp->GetDisplayListParent();
        nMyDepth--;
    }
    return pSprtTmp->GetDepth();
}

/*F************************************************************************************************/
/*!
    \Function    AptCIH::isVisiable

    \Description
        If any of the parents in the chain of this sprite are invisible, it is also invisible.

    \Output
    Returns true if is visible and false if not.

    \Notes
    None.

    \Version    1.0        06/22/04 (LEO) First Version

*/
/************************************************************************************************F*/
bool AptCIH::IsVisible() const
{
    APT_ASSERT(!this->isUndefined());

    const AptCIH *pSprite = this;
    while(pSprite)
    {
        if(!pSprite->GetVisible())
        {
            return false;
        }
        pSprite = pSprite->GetDisplayListParent();
    }
    return true;
}

/*F************************************************************************************************/
/*!
    \Function    AptCIH::IsParent

    \Description
        Checks if passed sprite is in parent chain of this sprite

    \Input      *pParentTmp       - sprite to check

    \Output     bool              - true if passed in sprite is in parent chain

    \Notes
    None.

    \Version    1.0        07/21/04 (LEO) First Version
    -Added for 585  NHL - single mouse clicks should activate options

*/
/************************************************************************************************F*/
bool AptCIH::IsParent(AptCIH *pParentTmp) const
{
    APT_ASSERT(!this->isUndefined());

    const AptCIH *pSprite = mpParent;
    while(pSprite)
    {
        if(pParentTmp == pSprite)
        {
            return true;
        }
        pSprite = pSprite->GetDisplayListParent();
    }
    return false;
}

/*F************************************************************************************************/
/*!
    \Function    *AptCIH::GetRootAnimation

    \Description
        returns the level AptCIH this one is in

    \Input

    \Output     AptCIH            - returns root animation in the displaylist tree for this AptCIH

    \Notes
    None.

    \Version    1.0        08/04/04 (LEO) First Version       $BUG253 LEO    40/47
*/
/************************************************************************************************F*/
AptCIH *AptCIH::GetRootAnimation()
{
    AptCIH *pSprtTmp = this;

    if(IsNone())
        return _AptGetAnimationAtLevel(0);

    while(!pSprtTmp->IsAnimationInst() && !pSprtTmp->IsLevelInst())
    {
        pSprtTmp = pSprtTmp->GetDisplayListParent();
    }
    return pSprtTmp;
}

/*F************************************************************************************************/
/*!
    \Function    AptCIH::DecZombieCount

    \Description
        Decrements the zombie counter for this AptCIh

    \Input

    \Output

    \Notes
    None.

    \Version    1.0        08/05/04 (LEO) First Version

*/
/************************************************************************************************F*/
void AptCIH::DecZombieCount()                    // $BUG253 LEO    28/47
{
    mnZombieCounter = mnZombieCounter - 1;
    if(mnZombieCounter == 0)                   // If the zombie counter is zero, go clean it out of the zombie vector.
    {
        AptUpdateZombieVector();
    }
}

/*F************************************************************************************************/
/*!
    \Function    AptCIH::HasRenderData

    \Description
        Checks if the AptCIH has a displayable or renderable data or not.

    \Input
        None.

    \Output     bool              - true if it has render data.

    \Notes
        Basically if it is a shape or text or static text or a button instance then
        it has a render/displayable data in it.
        Otherwise if it is a sprite then we need to look into its displaylist
        if displaylist of sprite contains any render data.

    \Version    1.0        09/13/04 (ASK) First Version

*/
/************************************************************************************************F*/
bool    AptCIH::HasRenderData() const
{
    if (IsSpriteInstBase())
    {
        return (GetSpriteInstBase()->mDisplayList.getState()->HasRenderData());
    }
    else if (IsShapeInst() || IsDynamicTextInst() || IsStaticTextInst() || IsButtonInst())
    {
        return (true) ;
    }
    return (false) ;
}

/*F************************************************************************************************/
/*!
    \Function    AptCIH::GetMovieclipInfo

    \Description
        This function will check what type of AptCIH is this and then
        accordingly will increament the counters in AptMovieclipInformation

    \Input      pMCInfo    -    pointer to AptMovieclipInformation structure.

    \Output
        None.

    \Notes
        pMCInfo in in-out parameter.

    \Version    1.0        09/14/04 (ASK) First Version

*/
/************************************************************************************************F*/
void AptCIH::GetMovieclipInfo(AptMovieclipInformation * pMCInfo, bool bRecurse) const
{
    APT_ASSERT(pMCInfo) ;
    AptCharacterType  eType = GetCharacterInst()->GetRenderItem()->GetCharacterType();

    switch(eType)
    {
        case AptCharacterType_Shape :
                pMCInfo->nShapes++ ;
                break;
        case AptCharacterType_Sprite :
                pMCInfo->nMovieClips++ ;
                break;
        case AptCharacterType_Button :
#if defined APT_USE_BUTTONS
                pMCInfo->nButtons++ ;
                GetButtonInst()->mDisplayList.getState()->GetMovieclipInfo(pMCInfo) ;
#endif
                break;
        case AptCharacterType_Text :
                pMCInfo->nDynamicText++ ;
                break;
        case AptCharacterType_StaticText :
                pMCInfo->nStaticText++ ;
                break;
        case AptCharacterType_Morph :
                pMCInfo->nMorph++ ;
                break;
        case AptCharacterType_Animation :
                pMCInfo->nAnimations++ ;
                break;
        case AptCharacterType_CustomControl :
                pMCInfo->nCustomControls++;
                break;
        default :
            break ;
    }

    if (bRecurse == true && IsSpriteInstBase())
    {
        GetSpriteInstBase()->mDisplayList.getState()->GetMovieclipInfo(pMCInfo) ;
    }

    return ;
}


/*F************************************************************************************************/
/*!
 \Function    AptCIH::SetMask

\Description
Sets the given mask as the  inclusion mask of this object.

\Input      pMask             -

\Output
None.

\Notes
None.

\Version    1.0        03/07/05 (DC) Bringin' it to ya

*/
/************************************************************************************************F*/
void AptCIH::SetMask(AptCIH * pMask)
{
    AptNativeHash *pHash = NULL;

    // If pMask is NULL, that means we are trying to remove a mask.  If we have a mask,
    // we need to remove it first.
    if(pMask == NULL || HasMask())
    {
        // Decrement the former mask's isMask count before letting it go.
        if(HasMask())
        {
            AptCIH * pMyMask = GetMask();
            if(pMyMask != NULL)
            {
                pMyMask->SetIsMask(false, NULL) ;
                AptNativeString hashKey = APTCIH_MASKEDITEM_HASHNAME;
                pHash = pMyMask->GetNativeHash();
                pHash->Unset(&hashKey);
            }
        }

        if(ContainsNativeHashVirtual())
        {
            AptNativeString hashKey = APTCIH_MASK_HASHNAME;
            pHash = GetNativeHash();
            pHash->Unset(&hashKey);
        }
    }

    if(pMask != NULL && pMask->isCIH())
    {
        if(pMask->IsMask())
        {
            // If pMask was already masking something, release that relationship before creating the new mask relationship
            pMask->SetIsMask(false, NULL);
            AptNativeString hashKey = APTCIH_MASKEDITEM_HASHNAME;
            pHash = pMask->GetNativeHash();
            AptValue *pMaskedItem = pHash->Lookup(&hashKey);

            APT_ASSERT(pMaskedItem != NULL);
            APT_ASSERT(pMaskedItem->isCIH());
            APT_ASSERT(pMaskedItem->c_cih()->HasMask());

            pMaskedItem->c_cih()->SetHasMask(false, NULL);

            AptNativeString hashKey2 = APTCIH_MASK_HASHNAME;
            AptNativeHash *pHash2 = pMaskedItem->c_cih()->GetNativeHash();
            pHash2->Unset(&hashKey2);
            pHash->Unset(&hashKey);
        }

        AptMatrix cur = gIdentityMatrix;
        MultParentMatrix(GetDisplayListParent() , cur);

        // Good to Go.
        pMask->SetIsMask(true, &cur);

        AptNativeString hashKey = APTCIH_MASKEDITEM_HASHNAME;
        pHash = pMask->GetNativeHash();
        pHash->Set(&hashKey, this);

        // I'll Let anything with a Hash have a dynamic Mask
        if( ContainsNativeHashVirtual() )
        {
            AptNativeString hashKey = APTCIH_MASK_HASHNAME;
            pHash = GetNativeHash();
            pHash->Set( &hashKey, pMask);
        }
        SetHasMask(true, pMask->GetSpriteInstBase()->GetRenderItemWritable());
    }
    else
    {
        SetHasMask(false, NULL);
    }
}


/*F************************************************************************************************/
/*!
    \Function    AptCIH::GetMask

    \Description
    Return the object's Mask (or NULL if it has none).

    \Input      pMask             -

    \Output
    None.

    \Notes
    None.

    \Version    1.0        03/07/05 (DC) Bringin' it to ya
*/
/************************************************************************************************F*/
AptCIH * AptCIH::GetMask(void) const
{
    if(HasMask())
    {
        // I'll Let anything with a Hash have a dynamic Mask
        if( this->ContainsNativeHashVirtual())
        {
            AptNativeString hashKey = APTCIH_MASK_HASHNAME;
            AptNativeHash *pHash = this->GetNativeHash();
            return (AptCIH*)pHash->Lookup(&hashKey);
        }
    }
    return NULL;
}

/*F************************************************************************************************/
/*!
    \Function    AptCIH::SetIsPlaying

    \Description
        Sets the playing flag, and updates the dirty state if set to play

    \Input      bFlag             -

    \Output
        None.

    \Notes
        None.

    \Version    1.0        08/23/05 (LEO) First Version

*/
/************************************************************************************************F*/
void AptCIH::SetIsPlaying(bool bFlag)
{
    GetSpriteInstBase()->mbIsPlaying = bFlag ? 1 : 0;
    if(bFlag == true)               // Only update if we are setting dirty
    {
        SetDirtyState(bFlag, true);
    }
}

/*F************************************************************************************************/
/*!
\Function    AptCIH::MultParentMatrix

\Description  Makes the product with position matrix

\Input      pCur        - first AptCIH for the accumulative product
\Input      cur         - matrix that is multiplied by the positions

\Output     None.

\Notes      None.

\Version    1.0        10/13/07 (JP) First Version
\Version    1.1        08/20/08 (MMeyer) Fix order of matrix multiplication.

*/
/************************************************************************************************F*/
void AptCIH::MultParentMatrix(const AptCIH *pCur, AptMatrix &cur) const
{
    while (pCur)
    {
        // We want to transform the current matrix by the ancestors' transforms as we go up the chain
        // To transform B by A, you do A * B.
        gpRenderingContext->multMatrix(pCur->GetPositionMatrixConst(), &cur, &cur);
        pCur = pCur->GetDisplayListParent();
    }
}

/*F************************************************************************************************/
/*!
    \Function    AptCIH::SetDirtyState

    \Description
        Sets the dirty flag and recurses up the parent chain if bTraverse is true

    \Input      bVal              - value of the dirty flag
    \Input      bTraverse         - true if we want to set dirty state in parent tree

    \Output
        None.

    \Notes
        None.

    \Version    1.0        08/13/05 (LEO) First Version

*/
/************************************************************************************************F*/
void AptCIH::SetDirtyState(bool bFlag, bool bTraverse)
{
    if(bFlag == false || IsShapeInst() || IsDynamicTextInst() || IsStaticTextInst() || IsNone())    // Set the flag to false if bFlag is false or it is one to the following types
    {
        mbDirty = 0;
        return;
    }

    mbDirty = 1;

    if(bTraverse == false)
    {
        return;
    }

    AptCIH *pCIH = GetDisplayListParent();          // Set the dirty state up the parent chain if bTraverse is true

    while(pCIH != NULL && pCIH->mbDirty == 0)
    {
        pCIH->mbDirty = 1;
        pCIH = pCIH->GetDisplayListParent();
    }
}

///! Define the static AptCharacter members in the AptCharacterHelper class
AptCharacter *AptCharacterHelper::s_pDynamicText  = NULL;
AptCharacter *AptCharacterHelper::s_pDynamicMovie = NULL;

/*F************************************************************************************************/
/*!
    \Function    AptCharacterHelper::CreateTextCharacterInst

    \Description
        Creates a static AptCharacter for dynamic text instances

    There only needs to be single AptCharacter text object for dynamically created text fields, so the
    AptCharacterHelper class creates a single AptCharacter object for this purpose.

    \Input
        None.

    \Output
        None.

    \Notes
        None.

    \Version    1.0        08/24/05 (LEO) First Version

*/
/************************************************************************************************F*/
void AptCharacterHelper::CreateTextCharacterInst()
{
    APT_ASSERT(s_pDynamicText == NULL);
    s_pDynamicText = APT_MALLOC_ARRAY(AptCharacter, 1);
    memset(s_pDynamicText, 0, sizeof(AptCharacter));

    APT_ASSERT(s_pDynamicText);
    s_pDynamicText->eType = AptCharacterType_Text;

    s_pDynamicText->text.rBounds.fLeft  = -2.f;
    s_pDynamicText->text.rBounds.fTop   = -2.f;
    s_pDynamicText->text.rBounds.fRight = 2.f;
    s_pDynamicText->text.rBounds.fBottom = 2.f;

    s_pDynamicText->text.eAlignment   = AptStringAlignment_Left;
    s_pDynamicText->text.nColour      = 0x00000000;
    s_pDynamicText->text.fFontHeight  = 12.0f;
    s_pDynamicText->text.bReadOnly    = 0;
    s_pDynamicText->text.bMultiLine   = 0;
    s_pDynamicText->text.bWordWrap    = 0;

    // Set to null to signify that they are not setup. (i.e. dynamically created)
    s_pDynamicText->text.szInitialText = NULL;
    s_pDynamicText->text.szVariable = NULL;

    s_pDynamicText->text.nFontID = -1;
	// FIFA_BEGIN | SENDBACK | mwebb | Mar 26 2015 | FIFA 16 change
	AptCIH *pCIH = _AptGetAnimationAtLevel(0);
    AptCharacterAnimation *pAnim = &pCIH->GetCharacterInst()->GetCharacterConst()->pParentAnim->animation;
    s_pDynamicText->pParentAnim = pAnim->apCharacters[0];
	// FIFA_END
    s_pDynamicText->m_data.m_nDynamic = 1;

	// FIFA_BEGIN | SENDBACK | mwebb | Mar 26 2015 | FIFA 16 change
	//// Colin C. 8/8/12
	//// Previous code here grabbed the animation loaded at level 0 and used it both as a parent of s_pDynamicText,
	//// and also to find the first font inside it to use as the overall default font.  This is bad because:
	//// * There could easily be no movie loaded at level 0
	//// * There is no guarantee that the movie loaded at level 0 will stay around.  It could be unloaded
	////   by some user code.  This would cause the Parent anim to point to unloaded memory.
	////
	//// I do not see a reason to set the default font based on the first font we happen to have in the first
	//// movie loaded.  I also do not currently see a requirement that this movie even have a parent. Therefore,
	//// I am just going to set the parent to NULL and hope that's good enough.  I cannot find any code that indicates
	//// the need for a parent to be set, and I haven't hit any asserts or issues in my testing.  (Plus, a NULL value
	//// being problematic is much easier to find a fix for than dereferenced memory)
	////
	//// If it's not good enough, we need some other solution.  Here are some options:
	//// * Football created a "fake" static animation and filled in the values by hand.  This seemed to solve
	////   their problem.
	//// * We could track when movies are loaded into levels and update the parent of s_pDynamicText when a
	////   new one is loaded
	//s_pDynamicText->pParentAnim = NULL;
    for(int i = 0; i < pAnim->nCharacters; i++)        // Set a default font
    {
        if(pAnim->apCharacters[i]->eType == AptCharacterType_Font)
        {
            s_pDynamicText->text.nFontID = i;            // default
            break;
        }
    }
	// FIFA_END
}

/*F************************************************************************************************/
/*!
    \Function    AptCharacterHelper::CreateMovieCharacterInst

    \Description
        Creates a static AptCharacter for dynamic movie instances

    There only needs to be single AptCharacter movie object for dynamically created movieclips, so the
    AptCharacterHelper class creates a single AptCharacter object for this purpose.

    \Input
        None.

    \Output
        None.

    \Notes
        None.

    \Version    1.0        08/24/05 (LEO) First Version

*/
/************************************************************************************************F*/
void AptCharacterHelper::CreateMovieCharacterInst()
{
    s_pDynamicMovie = APT_MALLOC_ARRAY(AptCharacter, 1);
    APT_ASSERT(s_pDynamicMovie);
    memset(s_pDynamicMovie, 0, sizeof(AptCharacter));
    s_pDynamicMovie->eType = AptCharacterType_Sprite;
	// FIFA_BEGIN | SENDBACK | mwebb | Mar 26 2015 | FIFA 16 change
	//s_pDynamicMovie->m_data.m_nDynamic = 1;

	//// Colin C. 8/8/12
	//// Previous code here searched for the first animation loaded (at level 0, or later in a loop)
	//// and used it as a parent of s_pDynamicMovie.  This is bad because there is no guarantee that
	//// the movie loaded at level 0 (or above) will stay around.  It could be unloaded by some user code.
	//// This would cause the Parent anim to point to unloaded memory.
	////
	//// I do not currently see a requirement that this movie have a parent.  Therefore, I am just going to
	//// set the parent to NULL and hope that's good enough.  I cannot find any code that indicates the need
	//// for a parent to be set, and I haven't hit any asserts or issues in my testing.  (Plus, a NULL value
	//// being problematic is much easier to find a fix for than dereferenced memory)
	////
	//// If it's not good enough, we need some other solution.  Here are some options:
	//// * Football created a "fake" static animation and filled in the values by hand.  This seemed to solve
	////   their problem.
	//// * We could track when movies are loaded into levels and update the parent of s_pDynamicMovie when a
	////   new one is loaded
	//s_pDynamicMovie->pParentAnim = NULL;
    AptCIH *pCIH = _AptGetAnimationAtLevel(0);
    if (pCIH->IsAnimationInst() == false)
    {
        // that means that somehow level 0 SWF file is not loaded at all or it in not loaded in the memory yet.
        // so lets try to find out first valid animation level
        AptCIH * pTempCIH = pCIH->GetDisplayListNext();
        pCIH = pTempCIH;
        while(pTempCIH)
        {
            if (pTempCIH->IsAnimationInst() == true)
            {
                pCIH = pTempCIH;
                break;
            }
            else
            {
                pTempCIH = pTempCIH->GetDisplayListNext();
            }
        }
    }
    APT_ASSERT(pCIH);       // this means that something really bad has happened, none of the SWF files were loaded and we are still
    // trying to find run something SWF actions which should never happen

    AptCharacterAnimation *pAnim = &pCIH->GetCharacterInst()->GetCharacterConst()->pParentAnim->animation;
    s_pDynamicMovie->pParentAnim = pAnim->apCharacters[0];
    s_pDynamicMovie->m_data.m_nDynamic = 1; // $senzee-lockless not necessary
	// FIFA_END
}

/*F************************************************************************************************/
/*!
    \Function    AptCharacterHelper::Initialize

    \Description
        Sets the static members to NULL

    \Input
        None.

    \Output
        None.

    \Notes
        None.

    \Version    1.0        08/24/05 (LEO) First Version

*/
/************************************************************************************************F*/
void AptCharacterHelper::Initialize()
{
    s_pDynamicText = NULL;
    s_pDynamicMovie = NULL;
}

/*F************************************************************************************************/
/*!
    \Function    AptCharacterHelper::Shutdown

    \Description
        Deletes the static instances if they were created

    \Input
        None.

    \Output
        None.

    \Notes
        None.

    \Version    1.0        08/24/05 (LEO) First Version

*/
/************************************************************************************************F*/
void AptCharacterHelper::Shutdown()
{
    if(s_pDynamicText != NULL)
    {
        APT_FREE_ARRAY(s_pDynamicText, AptCharacter, 1);
    }
    if(s_pDynamicMovie != NULL)
    {
        APT_FREE_ARRAY(s_pDynamicMovie, AptCharacter, 1);
    }
    s_pDynamicText = NULL;
    s_pDynamicMovie = NULL;
}

/*F************************************************************************************************/
/*!
    \Function    *AptCharacterHelper::GetAptTextCharacter

    \Description
        Returns the statically created AptCharacter object

    \Input
        None.

    \Output     AptCharacter      -

    \Notes
        None.

    \Version    1.0        08/24/05 (LEO) First Version

*/
/************************************************************************************************F*/
AptCharacter *AptCharacterHelper::GetAptTextCharacter()
{
    if(s_pDynamicText == NULL)
    {
        CreateTextCharacterInst();
    }
    return s_pDynamicText;
}

/*F************************************************************************************************/
/*!
    \Function    *AptCharacterHelper::GetAptMovieCharacter

    \Description
        Returns the statically created AptCharacter object

    \Input
        None.

    \Output     AptCharacter      -

    \Notes
        None.

    \Version    1.0        08/24/05 (LEO) First Version

*/
/************************************************************************************************F*/
AptCharacter *AptCharacterHelper::GetAptMovieCharacter()
{
    if(s_pDynamicMovie == NULL)
    {
        CreateMovieCharacterInst();
    }
    return s_pDynamicMovie;
}


/*F************************************************************************************************/
/*!
    \Function    AptCIH::GetBlendMode

    \Description
        If it is a type for which blendMode is valid, return the AptValue's
        blend mode (one of the SC_Blend* values in the StringCode enumeration).
        If not, assert and return SC_LAST.

    \Output

    \Notes

*/
/************************************************************************************************F*/
uint32_t AptCIH::GetBlendMode(AptValue * const pObject)
{
    if (pObject->isCIH())
    {
        AptCIH *pCIH = pObject->c_cih();
        AptCharacterInst *pCharInst = pCIH->mpCharacterInst;

        if (pCharInst->IsSpriteInstBase())
        {
            return pCIH->GetSpriteInstBase()->GetBlendMode();
        }
        else if (pCharInst->IsDynamicTextInst())
        {
            return pCIH->GetDynamicTextInst()->GetBlendMode();
        }
        else if (pCharInst->IsButtonInst())
        {
            return pCIH->GetButtonInst()->GetBlendMode();
        }
    }

    APT_FAIL("blendMode property is supported only for Movieclips, Text, Buttons");
    return SC_LAST;
}
