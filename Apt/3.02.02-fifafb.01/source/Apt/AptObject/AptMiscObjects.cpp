/*H************************************************************************************************/
/*!

\File
AptMiscObjects.cpp

\Description
Simple description of main purpose of file.

\Notes
None.

\Copyright
(c) 2005 Electronic Arts Inc.

\Version    1.0        01/04/05 (MCB) First Version
\Version    1.1        03/18/05 (RB)  Changed GET_INPUT to GET_ACTION_TYPE and/or
                                      GET_CONTROLLER_TYPE. Reduces warnings on non-VC compilers.
\Version    1.2        06/16/09 (RB)  Only include AptLoadVarsMembers.h when APT_USE_LOADVARS_OBJECT is defined.
*/
/************************************************************************************************H*/

#include "_Apt.h"
#include "AptTarget.h"
#include "AptAnimationTarget.h"
#include "AptObject/AptKey.h"
#include "AptKeyMembers.h"
#include "AptObject/AptMathObj.h"
#include "AptMathMembers.h"
#include "AptObject/AptLoadVars.h"
#if APT_USE_LOADVARS_OBJECT
#include "AptLoadVarsMembers.h"
#endif
#if defined (APT_USE_STAGE_OBJECT)
#include "AptObject/AptStage.h"
#include "AptStageMembers.h"
#endif
#include "AptObject/AptError.h"
#include "AptObject/AptStringObject.h"
#include "MainInline.h"

static const int AptKey_BACKSPACE = 1;
static const int AptKey_CAPSLOCK = 2;
static const int AptKey_CONTROL = 3;
static const int AptKey_DELETEKEY = 4;
static const int AptKey_DOWN = 5;
static const int AptKey_END = 6;
static const int AptKey_ENTER = 7;
static const int AptKey_ESCAPE = 8;
static const int AptKey_HOME = 9;
static const int AptKey_INSERT = 10;
static const int AptKey_LEFT = 11;
static const int AptKey_PGDN = 12;
static const int AptKey_PGUP = 13;
static const int AptKey_RIGHT = 14;
static const int AptKey_SHIFT = 15;
static const int AptKey_SPACE = 16;
static const int AptKey_TAB = 17;
static const int AptKey_UP = 18;
static const int AptKey_isDown = 100;
static const int AptKey_isToggled = 101;
static const int AptKey_getCode = 102;
static const int AptKey_getController = 103;
static const int AptKey_addListener = 104;
static const int AptKey_removeListener = 105;
static const int AptKey_getAnalogStickInfo = 106;
static const int AptKey_getAscii = 107;
static const int AptKey_getAnalogTriggerInfo = 108;    // JP (issue 106856)
static const int AptKey_getGestureInfo = 109;

#if APT_USE_MATH_OBJECT
static const int AptMathObj_sin = 1;
static const int AptMathObj_cos = 2;
static const int AptMathObj_atan2 = 3;
static const int AptMathObj_round = 4;
static const int AptMathObj_min = 5;
static const int AptMathObj_max = 6;
static const int AptMathObj_abs = 7;
static const int AptMathObj_acos = 8;
static const int AptMathObj_asin = 9;
static const int AptMathObj_atan = 10;
static const int AptMathObj_ceil = 11;
static const int AptMathObj_exp = 12;
static const int AptMathObj_floor = 13;
static const int AptMathObj_log = 14;
static const int AptMathObj_pow = 15;
static const int AptMathObj_random = 16;
static const int AptMathObj_sqrt = 17;
static const int AptMathObj_tan = 18;
#endif

#if APT_USE_LOADVARS_OBJECT
static const int AptLoadVars_load = 1;
static const int AptLoadVars_send = 2;
static const int AptLoadVars_sendAndLoad = 3;
static const int AptLoadVars_getBytesTotal = 4;
static const int AptLoadVars_getBytesLoaded = 5;
static const int AptLoadVars_loaded = 6;
static const int AptLoadVars_toString = 7;
static const int AptLoadVars_contentType = 8;
#endif

#if defined (APT_USE_STAGE_OBJECT)
static const int AptStage_align = 1;                    // Why would we support this?
static const int AptStage_height = 2;
static const int AptStage_width = 3;
static const int AptStage_scaleMode = 4;                // Why would we support this?
static const int AptStage_addListener = 100;
static const int AptStage_removeListener = 101;
#endif
/*F************************************************************************************************/
/*!
    Releases the AptNativeScript functions.

    \Input      None.

    \Output     None.

    \Notes
    This function needs to be called at least during AptShutdown.

    \Version
        1.0        04/05/04 (ONA) First Version

*/
/************************************************************************************************F*/
void    AptMathObj::CleanNativeFunctions()
{
#if APT_USE_MATH_OBJECT
    NATIVE_MEMBER_FUNCTION_DESTROY(sin);
    NATIVE_MEMBER_FUNCTION_DESTROY(cos);
    NATIVE_MEMBER_FUNCTION_DESTROY(atan2);
    NATIVE_MEMBER_FUNCTION_DESTROY(round);
    NATIVE_MEMBER_FUNCTION_DESTROY(min);
    NATIVE_MEMBER_FUNCTION_DESTROY(max);

    NATIVE_MEMBER_FUNCTION_DESTROY(abs);
    NATIVE_MEMBER_FUNCTION_DESTROY(acos);
    NATIVE_MEMBER_FUNCTION_DESTROY(asin);
    NATIVE_MEMBER_FUNCTION_DESTROY(atan);
    NATIVE_MEMBER_FUNCTION_DESTROY(ceil);
    NATIVE_MEMBER_FUNCTION_DESTROY(exp);
    NATIVE_MEMBER_FUNCTION_DESTROY(floor);
    NATIVE_MEMBER_FUNCTION_DESTROY(log);
    NATIVE_MEMBER_FUNCTION_DESTROY(pow);
    NATIVE_MEMBER_FUNCTION_DESTROY(random);
    NATIVE_MEMBER_FUNCTION_DESTROY(sqrt);
    NATIVE_MEMBER_FUNCTION_DESTROY(tan);
#endif
}

#if APT_USE_MATH_OBJECT
/*F************************************************************************************************/
/*!
    Returns the AptValue for the context and the member name.

    \Input      pContext          - The context passed in.
    \Input      pName             - The member name.

    \Output     AptValue *        - The AptValue of the member.
                                    gpUndefinedValue if the member is unknown.

    \Notes
        None.

    \Version
        1.0        03/26/04 (ONA) First Version

*/
/************************************************************************************************F*/
AptValue *    AptMathObj::objectMemberLookup(AptValue * const pContext, const AptNativeString * const pName) const
{
    MathMembers *pProp = pContext ? MathMembersIndex::in_word_set(pName->c_str(), pName->Size()) : NULL;

    if (pProp)
    {
        switch(pProp->nIndex)
        {
            case(AptMathObj_sin):
            {
                NATIVE_MEMBER_FUNCTION_DISPATCH(sin);
            }
            break;
            case AptMathObj_cos:

            {
                NATIVE_MEMBER_FUNCTION_DISPATCH(cos);
            }
            break;
            case AptMathObj_atan2:

            {
                NATIVE_MEMBER_FUNCTION_DISPATCH(atan2);
            }
            break;
            case AptMathObj_round:

            {
                NATIVE_MEMBER_FUNCTION_DISPATCH(round);
            }
            break;
            case AptMathObj_min:

            {
                NATIVE_MEMBER_FUNCTION_DISPATCH(min);
            }
            break;
            case AptMathObj_max:

            {
                NATIVE_MEMBER_FUNCTION_DISPATCH(max);
            }
            break;
            case AptMathObj_abs:

            {
                NATIVE_MEMBER_FUNCTION_DISPATCH(abs);
            }
            break;
            case AptMathObj_acos:

            {
                NATIVE_MEMBER_FUNCTION_DISPATCH(acos);
            }
            break;
            case AptMathObj_asin:

            {
                NATIVE_MEMBER_FUNCTION_DISPATCH(asin);
            }
            break;
            case AptMathObj_atan:

            {
                NATIVE_MEMBER_FUNCTION_DISPATCH(atan);
            }
            break;
            case AptMathObj_ceil:

            {
                NATIVE_MEMBER_FUNCTION_DISPATCH(ceil);
            }
            break;
            case AptMathObj_exp:

            {
                NATIVE_MEMBER_FUNCTION_DISPATCH(exp);
            }
            break;
            case AptMathObj_floor:

            {
                NATIVE_MEMBER_FUNCTION_DISPATCH(floor);
            }
            break;
            case AptMathObj_log:

            {
                NATIVE_MEMBER_FUNCTION_DISPATCH(log);
            }
            break;
            case AptMathObj_pow:

            {
                NATIVE_MEMBER_FUNCTION_DISPATCH(pow);
            }
            break;
            case AptMathObj_random:

            {
                NATIVE_MEMBER_FUNCTION_DISPATCH(random);
            }
            break;
            case AptMathObj_sqrt:

            {
                NATIVE_MEMBER_FUNCTION_DISPATCH(sqrt);
            }
            break;
            case AptMathObj_tan:

            {
                NATIVE_MEMBER_FUNCTION_DISPATCH(tan);
            }

        }
    }

#if defined(EA_DEBUG)
    if (
        (pName->EqualNoCase("sin")) ||
        (pName->EqualNoCase("cos")) ||
        (pName->EqualNoCase("atan2")) ||
        (pName->EqualNoCase("round")) ||
        (pName->EqualNoCase("min")) ||
        (pName->EqualNoCase("max")) ||
        (pName->EqualNoCase("abs")) ||
        (pName->EqualNoCase("acos")) ||
        (pName->EqualNoCase("asin")) ||
        (pName->EqualNoCase("atan")) ||
        (pName->EqualNoCase("ceil")) ||
        (pName->EqualNoCase("exp")) ||
        (pName->EqualNoCase("floor")) ||
        (pName->EqualNoCase("log")) ||
        (pName->EqualNoCase("pow")) ||
        (pName->EqualNoCase("random")) ||
        (pName->EqualNoCase("sqrt")) ||
        (pName->EqualNoCase("tan"))
        )
    {
        APT_DEBUGPRINT(APT_DEBUG_MSG_WARNING_LVL, "AptMathObj: Incorrect case for '%s'.\n", pName->c_str());
        APT_ASSERT(0);
    }
#endif

    return 0;
}

NATIVE_MEMBER_FUNCTION(AptMathObj, sin)
{
    if (nParams < 1) return gpUndefinedValue;
    AptValue *pParam = gAptActionInterpreter.stackAt(0);
    return AptFloat::Create(sinf(pParam->toFloat()));
}

NATIVE_MEMBER_FUNCTION(AptMathObj, cos)
{
    if (nParams < 1) return gpUndefinedValue;
    AptValue *pParam = gAptActionInterpreter.stackAt(0);
    return AptFloat::Create(cosf(pParam->toFloat()));
}

NATIVE_MEMBER_FUNCTION(AptMathObj, atan2)
{
    if (nParams < 2) return gpUndefinedValue;
    AptValue *pParam0 = gAptActionInterpreter.stackAt(0);
    AptValue *pParam1 = gAptActionInterpreter.stackAt(1);
    return AptFloat::Create(atan2f(pParam0->toFloat(), pParam1->toFloat()));
}

NATIVE_MEMBER_FUNCTION(AptMathObj, round)
{
    if (nParams <= 0) return gpUndefinedValue;
    AptValue *pParam0 = gAptActionInterpreter.stackAt(0);
    float fVal = pParam0->toFloat();
    int nVal = fVal > 0 ? (int)(fVal + 0.5f) : (int)(fVal - 0.5f);
    return AptInteger::Create(nVal);
}

NATIVE_MEMBER_FUNCTION(AptMathObj, min)
{
    if (nParams < 2) return gpUndefinedValue;
    AptValue *pParam0 = gAptActionInterpreter.stackAt(0);
    AptValue *pParam1 = gAptActionInterpreter.stackAt(1);
    return AptFloat::Create(pParam0->toFloat() < pParam1->toFloat() ? pParam0->toFloat() : pParam1->toFloat());
}

NATIVE_MEMBER_FUNCTION(AptMathObj, max)
{
    if (nParams < 2) return gpUndefinedValue;
    AptValue *pParam0 = gAptActionInterpreter.stackAt(0);
    AptValue *pParam1 = gAptActionInterpreter.stackAt(1);
    return AptFloat::Create(pParam0->toFloat() > pParam1->toFloat() ? pParam0->toFloat() : pParam1->toFloat());
}


NATIVE_MEMBER_FUNCTION(AptMathObj, abs)
{
    if (nParams < 1) return gpUndefinedValue;
    AptValue *pParam0 = gAptActionInterpreter.stackAt(0);

    if(pParam0->isInteger())
    {
        return AptInteger::Create(abs(pParam0->toInteger()));
    }
    else // $bug733 - MCB - modified abs to handle floats.
    {
        return AptFloat::Create(fabsf((pParam0->toFloat())));
    }
}

NATIVE_MEMBER_FUNCTION(AptMathObj, acos)
{
    if (nParams < 1) return gpUndefinedValue;
    AptValue *pParam = gAptActionInterpreter.stackAt(0);
    return AptFloat::Create(acosf(pParam->toFloat()));
}

NATIVE_MEMBER_FUNCTION(AptMathObj, asin)
{
    if (nParams < 1) return gpUndefinedValue;
    AptValue *pParam = gAptActionInterpreter.stackAt(0);
    return AptFloat::Create(asinf(pParam->toFloat()));
}
NATIVE_MEMBER_FUNCTION(AptMathObj, atan)
{
    if (nParams < 1) return gpUndefinedValue;
    AptValue *pParam = gAptActionInterpreter.stackAt(0);
    return AptFloat::Create(atanf(pParam->toFloat()));
}
NATIVE_MEMBER_FUNCTION(AptMathObj, ceil)
{
    if (nParams < 1) return gpUndefinedValue;
    AptValue *pParam = gAptActionInterpreter.stackAt(0);
    return AptFloat::Create(ceilf(pParam->toFloat()));
}
NATIVE_MEMBER_FUNCTION(AptMathObj, exp)
{
    if (nParams < 1) return gpUndefinedValue;
    AptValue *pParam = gAptActionInterpreter.stackAt(0);
    return AptFloat::Create(expf(pParam->toFloat()));
}
NATIVE_MEMBER_FUNCTION(AptMathObj, floor)
{
    if (nParams < 1) return gpUndefinedValue;
    AptValue *pParam = gAptActionInterpreter.stackAt(0);
    return AptFloat::Create(floorf(pParam->toFloat()));
}
NATIVE_MEMBER_FUNCTION(AptMathObj, log)
{
    if (nParams < 1) return gpUndefinedValue;
    AptValue *pParam = gAptActionInterpreter.stackAt(0);
    return AptFloat::Create(logf(pParam->toFloat()));
}
NATIVE_MEMBER_FUNCTION(AptMathObj, pow)
{
    if (nParams < 2) return gpUndefinedValue;
    AptValue *pParam0 = gAptActionInterpreter.stackAt(0);
    AptValue *pParam1 = gAptActionInterpreter.stackAt(1);
    return AptFloat::Create(powf(pParam0->toFloat(), pParam1->toFloat()));
}

NATIVE_MEMBER_FUNCTION(AptMathObj, random)
{
    return AptFloat::Create((float)AptRand() / 0xffffffff);
}
NATIVE_MEMBER_FUNCTION(AptMathObj, sqrt)
{
    if (nParams < 1) return gpUndefinedValue;
    AptValue *pParam = gAptActionInterpreter.stackAt(0);
    return AptFloat::Create(sqrtf(pParam->toFloat()));
}
NATIVE_MEMBER_FUNCTION(AptMathObj, tan)
{
    if (nParams < 1) return gpUndefinedValue;
    AptValue *pParam = gAptActionInterpreter.stackAt(0);
    return AptFloat::Create(tanf(pParam->toFloat()));
}
#endif


//-----------------------------------------------------------------------------------------------

/*F************************************************************************************************/
/*!
    Releases the AptNativeScript functions.

    \Input      None.

    \Output     None.

    \Notes
        This function needs to be called at least during AptShutdown.

    \Version
        1.0        04/05/04 (ONA) First Version

*/
/************************************************************************************************F*/
void    AptKey::CleanNativeFunctions()
{
    NATIVE_MEMBER_FUNCTION_DESTROY(isDown);
    NATIVE_MEMBER_FUNCTION_DESTROY(isToggled);
    NATIVE_MEMBER_FUNCTION_DESTROY(getCode);
    NATIVE_MEMBER_FUNCTION_DESTROY(getController);
    NATIVE_MEMBER_FUNCTION_DESTROY(addListener);
    NATIVE_MEMBER_FUNCTION_DESTROY(removeListener);
    NATIVE_MEMBER_FUNCTION_DESTROY(getAnalogStickInfo);
    NATIVE_MEMBER_FUNCTION_DESTROY(getAnalogTriggerInfo);    // JP (issue P18-106856)
    NATIVE_MEMBER_FUNCTION_DESTROY(getGestureInfo);
    NATIVE_MEMBER_FUNCTION_DESTROY(getAscii);
}

/*F************************************************************************************************/
/*!
    Returns the AptValue for the context and the member name.

    \Input      pContext          - The context passed in.
    \Input      pName             - The member name.

    \Output     AptValue *        - The AptValue of the member.
                                    gpUndefinedValue if the member is unknown.

    \Notes
        None.

    \Version
        1.0        03/26/04 (ONA) First Version

*/
/************************************************************************************************F*/
AptValue *    AptKey::objectMemberLookup(AptValue * const pContext, const AptNativeString * const pName) const
{
    KeyMembers *pProp = pContext && pContext->isKey() ? KeyMembersIndex::in_word_set(pName->c_str(), pName->Size()) : NULL;

    if (pProp)
    {
        switch(pProp->nIndex)
        {
            case(AptKey_BACKSPACE):
            {
                return AptInteger::Create(8);
            }
            break;
            case AptKey_CAPSLOCK:

            {
                return AptInteger::Create(20);
            }
            break;
            case AptKey_CONTROL:

            {
                return AptInteger::Create(17);
            }
            break;
            case AptKey_DELETEKEY:

            {
                return AptInteger::Create(46);
            }
            break;
            case AptKey_DOWN:

            {
                return AptInteger::Create(40);
            }
            break;
            case AptKey_END:

            {
                return AptInteger::Create(35);
            }
            break;
            case AptKey_ENTER:

            {
                return AptInteger::Create(13);
            }
            break;
            case AptKey_ESCAPE:

            {
                return AptInteger::Create(27);
            }
            break;
            case AptKey_HOME:

            {
                return AptInteger::Create(36);
            }
            break;
            case AptKey_INSERT:

            {
                return AptInteger::Create(45);
            }
            break;
            case AptKey_LEFT:

            {
                return AptInteger::Create(37);
            }
            break;
            case AptKey_PGDN:

            {
                return AptInteger::Create(34);
            }
            break;
            case AptKey_PGUP:

            {
                return AptInteger::Create(33);
            }
            break;
            case AptKey_RIGHT:

            {
                return AptInteger::Create(39);
            }
            break;
            case AptKey_SHIFT:

            {
                return AptInteger::Create(16);
            }
            break;
            case AptKey_SPACE:

            {
                return AptInteger::Create(32);
            }
            break;
            case AptKey_TAB:

            {
                return AptInteger::Create(9);
            }
            break;
            case AptKey_UP:

            {
                return AptInteger::Create(38);
            }
            break;
            case AptKey_isDown:

            {
                NATIVE_MEMBER_FUNCTION_DISPATCH(isDown);
            }
            break;
            case AptKey_isToggled:

            {
                NATIVE_MEMBER_FUNCTION_DISPATCH(isToggled);
            }
            break;
            case AptKey_getCode:

            {
                NATIVE_MEMBER_FUNCTION_DISPATCH(getCode);
            }
            break;
            case AptKey_getController:

            {
                NATIVE_MEMBER_FUNCTION_DISPATCH(getController);
            }
            break;
            case AptKey_addListener:

            {
                NATIVE_MEMBER_FUNCTION_DISPATCH(addListener);
            }
            break;
            case AptKey_removeListener:

            {
                NATIVE_MEMBER_FUNCTION_DISPATCH(removeListener);
            }
            break;
            case AptKey_getAnalogStickInfo:

            {
                NATIVE_MEMBER_FUNCTION_DISPATCH(getAnalogStickInfo) ;
            }
            break;
            case AptKey_getAnalogTriggerInfo:    // JP (issue P18-106856)
            {
                NATIVE_MEMBER_FUNCTION_DISPATCH(getAnalogTriggerInfo) ;
            }
                        break;
            case AptKey_getGestureInfo:
            {
                NATIVE_MEMBER_FUNCTION_DISPATCH(getGestureInfo) ;
            }
            break;
            case AptKey_getAscii:
            {
                NATIVE_MEMBER_FUNCTION_DISPATCH(getAscii) ;
            }
        }
    }
    return 0;
}

// flash has two different sets of keycodes, one for buttonactions, one for Key.XYZ. This translates from buttonactions which are
// the ones we "use" (and are in Apt.h) to what Key.getCode wants.
static const int BUTTONACTIONCODE_TO_KEYCODE[] =
{
    -1, // unused
    37, // LEFT = 1,
    39, // RIGHT = 2,
    36, // HOME = 3,
    35, // END = 4,
    45, // INSERT = 5,
    46, // DELETE = 6,
    -1, // unused
    8, // BACKSPACE = 8,
    -1, // unused
    -1, // unused
    -1, // unused
    -1, // unused
    13, // ENTER = 13,
    38, // UP = 14,
    40, // DOWN = 15,
    33, // PGUP = 16,
    34, // PGDN = 17,
    9, // TAB = 18,
    27, // ESCAPE = 19,
};


/*F************************************************************************************************/
/*!
\Function    AptKey, isDown

\Description


\Notes
    Note that this only works in onKeyPress events, This feature is NOT fully supported at this time.

\Version    1.0        08/04/04 (DC) First Version
\Version    1.1        04/20/04 (DC) Fixed to work when no input is valid.
\Version    1.2        03/18/08 (RB) Use GET_ACTION_TYPE instead of GET_INPUT.
*/
/************************************************************************************************F*/
NATIVE_MEMBER_FUNCTION(AptKey, isDown)
{
    int nInt = 0;
    AptValue *pValue = gAptActionInterpreter.stackAt(0);

    if( INPUT_IS_KEY(&gAptActionInterpreter.input) )
    {
        GET_ACTION_TYPE(gAptActionInterpreter.input, nInt);

        // getCode always uses uppercase
        if (nInt >= AptInputType_ASCII32 && nInt <= AptInputType_ASCII126)
        {
            nInt = toupper(nInt);
        }
        else if (nInt < APT_ARRAYSIZE(BUTTONACTIONCODE_TO_KEYCODE))
        {
            nInt = BUTTONACTIONCODE_TO_KEYCODE[nInt];
            //APT_ASSERT(nInt != -1);
        }

        int lookingForValue = pValue->toInteger();
        return AptBoolean::Create((nInt) == lookingForValue);
    }
    return AptBoolean::Create(false);
}



/*F************************************************************************************************/
/*!
\Function    AptKey, isToggled

\Description
    returns info about num-lock and caps-lock.
\Notes
None.

\Version    1.0        04/20/04 (ASK) First Version

*/
/************************************************************************************************F*/
NATIVE_MEMBER_FUNCTION(AptKey, isToggled)
{
    APT_ASSERT(false); // Not Implemented

    return AptBoolean::Create(false);
}


/*F************************************************************************************************/
/*!
\Function    AptKey, getCode

\Description
This function is implementation of key.getAscii() actionscript function.

This function will just return whatever game/auxiliary library has passed in to
AptAddToInputQueue function if controller is AptInputController_Keyboard.

\Notes
None.

\Version    1.0        04/20/04 (ASK) First Version
\Version    1.2        03/18/08 (RB) Use GET_ACTION_TYPE instead of GET_INPUT.
*/
/************************************************************************************************F*/
NATIVE_MEMBER_FUNCTION(AptKey, getCode)
{
    int nInt;
    GET_ACTION_TYPE(gAptActionInterpreter.input, nInt);

    // getCode always uses uppercase
    if (nInt >= AptInputType_ASCII32 && nInt <= AptInputType_ASCII126)
    {
        nInt = toupper(nInt);
    }
    else if (nInt < APT_ARRAYSIZE(BUTTONACTIONCODE_TO_KEYCODE))
    {
        nInt = BUTTONACTIONCODE_TO_KEYCODE[nInt];
        //APT_ASSERT(nInt != -1);
    }

    return AptInteger::Create(nInt);
}



/*F************************************************************************************************/
/*!
    \Function    AptKey, getAscii

    \Description
        This function is implementation of key.getAscii() actionscript function.

    This function will just return whatever game/auxilliary library has passed in to
    AptAddToInputQueue function if controller is AptInputController_Keyboard.

    \Notes
        None.

    \Version    1.0        04/20/04 (ASK) First Version
    \Version    1.1        03/18/08 (RB) Use GET_ACTION_TYPE and GET_ACTION_CONTROLLER instead of GET_INPUT.
*/
/************************************************************************************************F*/
NATIVE_MEMBER_FUNCTION(AptKey, getAscii)
{
    int nInt;
    int eController;
    GET_ACTION_TYPE(gAptActionInterpreter.input, nInt);
    GET_ACTION_CONTROLLER(gAptActionInterpreter.input, eController);

    if (eController == AptInputController_Keyboard)
    {
        return AptInteger::Create(nInt);
    }
    else if (nInt < APT_ARRAYSIZE(BUTTONACTIONCODE_TO_KEYCODE))
    {
        nInt = BUTTONACTIONCODE_TO_KEYCODE[nInt];
        //APT_ASSERT(nInt != -1);
    }

    return AptInteger::Create(nInt);
}


NATIVE_MEMBER_FUNCTION(AptKey, getController)
{
    int eController;  // Actually of type AptInputController but easier to handle as int here.
    GET_ACTION_CONTROLLER(gAptActionInterpreter.input, eController);

    eController -= AptInputController_Gamepad0; // convert to 0 based (assuming gamepad constants are continguous)
// commented out because user-list survey results show that users want it 0-based.
//  nInt += 1;

    return AptInteger::Create(eController);
}


// link to these listener objects may remain in mListenerSet even if sometimes object is
// geting deleted/out of scope, and because there is a link to the object in this
// listenerset, its refcount will be atleast 1 ans so it won't get deleted and might stay
// active in memory.
// actionscript User has to take care of calling removeListener whenever the scope of object is getting
// finished, this will remove object from listenerset and decrement refcount, which might finally end
// up in correctly deleting the object.
NATIVE_MEMBER_FUNCTION(AptKey, addListener)
{
    if (nParams != 1)
        return gpUndefinedValue;

    AptValue *pParam = gAptActionInterpreter.stackAt(0);
    if (pParam->getIsDefined())
    {
        if (pParam->isCIH()) // $bug788 - MCB - don't add to listener set if the CIH is a Zombie.
        {
            if (((AptCIH*)pParam)->GetCIHState() != AptCIH::AptCIHState_Normal)
            {
                return gpUndefinedValue;
            }
        }
        // object is defined and valid so now add it to mListenerSet in GetTargetSim()->GetAnimationTarget().
        if(!GetTargetSim()->GetAnimationTarget()->GetListenerSet()->has(pParam))
            GetTargetSim()->GetAnimationTarget()->GetListenerSet()->add(pParam) ;
    }

    return gpUndefinedValue;
}

// actionscript programmer should remember to call this function when object is going out of scope
// Note : if same object is also listening to some other non-key events like mouseUp etc then
// even in that case the object will be removed from the mListenerSet and object willnot be
// listening to any more event. This is because currently there is only one listenerset
// for all types of events.
NATIVE_MEMBER_FUNCTION(AptKey, removeListener)
{
    if (nParams != 1)
        return AptBoolean::Create(false) ;

    AptValue *pParam = gAptActionInterpreter.stackAt(0);
    if (pParam->getIsDefined())
    {
        // object is defined and valid so now remove it from listener set
        if(GetTargetSim()->GetAnimationTarget()->GetListenerSet()->has(pParam))
        {
            GetTargetSim()->GetAnimationTarget()->GetListenerSet()->remove(pParam) ;
            return AptBoolean::Create(true) ;
        }
    }

    return AptBoolean::Create(false) ;
}

/*F************************************************************************************************/
/*!
    getAnalogStickInfo

    \Input      None.

    \Output     Analog sticks information

    \Notes
                None.

    \Version
        1.0        04/05/04 (LDH) First Version
                Added to support analog stick feature request #239
                Game/auxiliary library adds AptInputType_LeftAnalogStick or AptInputType_RightAnalogStick
                to inputQueue, then actionscript checks for these codes through key.getCode.
                Once actionscript finds an analog stick event it should then call getAnalogStickInfo, via
                Key.getAnalogStickInfo(), to get position data from the analog stick.  Apt doesn't care about
                previous analog stick positions and only keeps the last postion data given to it
                from the game/auxiliary library.
        1.1     11/21/07 (JP) Checking if pAnalogInfo is NULL
        1.2     03/18/08 (RB) Removed param to ANALOG_INPUT.
*/
/************************************************************************************************F*/
NATIVE_MEMBER_FUNCTION(AptKey, getAnalogStickInfo)
{
    int nController = 0;
    AptInputType eType = AptInputType_LeftAnalogStick;
    if(gAptActionInterpreter.input != 0)
    {
        GET_ANALOG_INPUT(gAptActionInterpreter.input, eType, nController);
    }

    int nController0Based = nController - AptInputController_Gamepad0;  // convert to 0 based (assuming gamepad constants are continguous)

    AptObject * pObject = new AptObject(AptVFT_Object);

    // set controller id
    AptInteger * pController = AptInteger::Create(nController0Based);
    pObject->Set(StringPool::GetString(SC_controller), pController);

    // JP. TODO. Notice that the output of this function doesn't say if the information comes from the left stick or from the right stick
    AptAnalogStickInfo *pAnalogInfo = NULL;
    if(eType == AptInputType_LeftAnalogStick)
    {
        pAnalogInfo = GetTargetSim()->GetAnimationTarget()->GetAStickLeft(nController);
    }
    else if(eType == AptInputType_RightAnalogStick)
    {
        pAnalogInfo = GetTargetSim()->GetAnimationTarget()->GetAStickRight(nController);
    }

    APT_ASSERT(pAnalogInfo!=NULL && "invalid stick id extracted out of input")

    if(pAnalogInfo)
    {
        AptFloat * pfXAxis = AptFloat::Create(pAnalogInfo->fXAxisValue);
        AptFloat * pfYAxis = AptFloat::Create(pAnalogInfo->fYAxisValue);

        AptNativeString pTmpStr = "fXAxisValue";
            pObject->Set(&pTmpStr, pfXAxis);
            pTmpStr = "fYAxisValue";
            pObject->Set(&pTmpStr, pfYAxis);
    }
    return pObject ;
}

/*F************************************************************************************************/
/*!
    getAnalogTriggerInfo

    \Input      None.

    \Output     Analog triggers information

    \Notes      None.

    \Version
        1.0     01/04/07 (JP) First Version  (issue P18-106856)
                Call Key.getAnalogTriggerInfo() to get last position data from the triggers
        1.1     03/18/08 (RB) Use GET_ACTION_CONTROLLER instead of GET_INPUT.
*/
/************************************************************************************************F*/
NATIVE_MEMBER_FUNCTION(AptKey, getAnalogTriggerInfo)
{
    int nController = AptInputController_Gamepad0;
    // Gets the last controller that received input
    if(gAptActionInterpreter.input != 0)
    {
        GET_ACTION_CONTROLLER(gAptActionInterpreter.input, nController);
    }

    AptObject *pObject = new AptObject(AptVFT_Object);

    // 0 based controller
    pObject->Set(StringPool::GetString(SC_controller), AptInteger::Create(nController - AptInputController_Gamepad0));

    // Last values of the triggers
    AptAnalogStickInfo *pAnalogInfo = GetTargetSim()->GetAnimationTarget()->GetATriggers(nController);
    AptNativeString tempStr = "fLeftTrigger";
    pObject->Set(&tempStr, AptFloat::Create(pAnalogInfo->fXAxisValue));
    tempStr = "fRightTrigger";
    pObject->Set(&tempStr, AptFloat::Create(pAnalogInfo->fYAxisValue));

    return pObject ;
}

/*F************************************************************************************************/
/*!
    getGestureInfo

    \Input      None.

    \Output     gesture information

    \Notes
                None.

    \Version
        1.0        06/07/11 (BRH) First Version
                Added to support adding Gesture inputs to inputQueue,
                then actionscript checks for these codes through key.getCode.
                Once actionscript finds a gesture event it should then call getGestureInfo, via
                Key.getGestureInfo(), to get position data from the motion controller.
*/
/************************************************************************************************F*/
NATIVE_MEMBER_FUNCTION(AptKey, getGestureInfo)
{
    AptInputController nController = AptInputController_Gamepad0;
    AptInputType eType = AptInputType_NumInputs;

    if(gAptActionInterpreter.input != 0)
    {
        GET_ACTION_CONTROLLER(gAptActionInterpreter.input, nController);

        GET_ACTION_TYPE(gAptActionInterpreter.input, eType);
    }

    int nController0Based = nController - AptInputController_Gamepad0;  // convert to 0 based (assuming gamepad constants are continguous)

    AptObject * pObject = new AptObject(AptVFT_Object);

    // set controller id
    AptInteger * pController = AptInteger::Create(nController0Based);
    pObject->Set(StringPool::GetString(SC_controller), pController);

    AptGestureInfo *pGestureInfo = NULL;

    pGestureInfo = GetTargetSim()->GetAnimationTarget()->GetGestureInfo(nController, eType);

    APT_ASSERT(pGestureInfo!=NULL && "invalid controller id extracted out of input")

    if(pGestureInfo)
    {
        AptFloat * pfGestureData = AptFloat::Create(pGestureInfo->fGestureData);

        AptNativeString pTmpStr = "fGestureData";
            pObject->Set(&pTmpStr, pfGestureData);
    }
    return pObject ;
}

//----------------------------------------------------------------------
#if defined(APT_USE_MOUSE)
#include "AptObject/AptMouse.h"
/*F************************************************************************************************/
/*!
    Releases the AptNativeScript functions.

    \Input      None.

    \Output     None.

    \Notes
        This function needs to be called at least during AptShutdown.

    \Version
        1.0        04/05/04 (ONA) First Version

*/
/************************************************************************************************F*/
void    AptMouse::CleanNativeFunctions()
{
    NATIVE_MEMBER_FUNCTION_DESTROY(addListener);
    NATIVE_MEMBER_FUNCTION_DESTROY(removeListener);
}

/*F************************************************************************************************/
/*!
    Returns the AptValue for the context and the member name.

    \Input      pContext          - The context passed in.
    \Input      pName             - The member name.

    \Output     AptValue *        - The AptValue of the member.
                                    gpUndefinedValue if the member is unknown.

    \Notes
        None.

    \Version
        1.0        03/26/04 (ONA) First Version

*/
/************************************************************************************************F*/
AptValue *    AptMouse::objectMemberLookup(AptValue * const pContext, const AptNativeString * const pName) const
{
    if (*pName == "addListener")
    {
        NATIVE_MEMBER_FUNCTION_DISPATCH(addListener);
    }
    else if (*pName == "removeListener")
    {
        NATIVE_MEMBER_FUNCTION_DISPATCH(removeListener);
    }
    return 0;
}


// link to these listener objects may remain in mListenerSet even if sometimes object is
// geting deleted/out of scope, and because there is a link to the object in this
// listenerset, its refcount will be atleast 1 ans so it won't get deleted and might stay
// active in memory.
// actionscript User has to take care of calling removeListener whenever the scope of object is getting
// finished, this will remove object from listenerset and decrement refcount, which might finally end
// up in correctly deleting the object.
NATIVE_MEMBER_FUNCTION(AptMouse, addListener)
{
    if (nParams != 1)
        return gpUndefinedValue;

    AptValue *pParam = gAptActionInterpreter.stackAt(0);
    if (pParam->getIsDefined())
    {
        if (pParam->isCIH()) // $bug788 - MCB - don't add to listener set if the CIH is a Zombie.
        {
            if (((AptCIH*)pParam)->GetCIHState() != AptCIH::AptCIHState_Normal)
            {
                return gpUndefinedValue;
            }
        }
        // object is defined and valid so now add it to mListenerSet in GetTargetSim()->GetAnimationTarget().
        if(!GetTargetSim()->GetAnimationTarget()->GetMouseListenerSet()->has(pParam))
            GetTargetSim()->GetAnimationTarget()->GetMouseListenerSet()->add(pParam) ;
    }

    return gpUndefinedValue;
}

// actionscript programmer should remember to call this function when object is going out of scope
// Note : if same object is also listening to some other non-key events like mouseUp etc then
// even in that case the object will be removed from the mListenerSet and object willnot be
// listening to any more event. This is because currently there is only one listenerset
// for all types of events.
NATIVE_MEMBER_FUNCTION(AptMouse, removeListener)
{
    if (nParams != 1)
        return AptBoolean::Create(false) ;

    AptValue *pParam = gAptActionInterpreter.stackAt(0);
    if (pParam->getIsDefined())
    {
        // object is defined and valid so now remove it from listener set
        if(GetTargetSim()->GetAnimationTarget()->GetMouseListenerSet()->has(pParam))
        {
            GetTargetSim()->GetAnimationTarget()->GetMouseListenerSet()->remove(pParam) ;
            return AptBoolean::Create(true) ;
        }
    }

    return AptBoolean::Create(false) ;
}
#endif

//------------------------------------------------------------------------------------------
// AptLoadVars object
#if APT_USE_LOADVARS_OBJECT
/*F************************************************************************************************/
/*!
    Returns the AptValue for the context and the member name.

    \Input      pContext          - The context passed in.
    \Input      pName             - The member name.

    \Output     AptValue *        - The AptValue of the member.
                                    gpUndefinedValue if the member is unknown.

    \Notes
        None.

    \Version
        1.0        03/26/04 (ONA) First Version

*/
/************************************************************************************************F*/
AptValue *    AptLoadVars::objectMemberLookup(AptValue * const pContext, const AptNativeString * const pName) const
{
    LoadVarsMembers *pProp = pContext ? LoadVarsMembersIndex::in_word_set(pName->c_str(), pName->Size()) : NULL;
    if (pProp)
    {
        switch(pProp->nIndex)
        {
            case(AptLoadVars_load):
            {
                NATIVE_MEMBER_FUNCTION_DISPATCH(load);
            }
            break;
            case AptLoadVars_send:

            {
                NATIVE_MEMBER_FUNCTION_DISPATCH(send) ;
            }
            break;
            case AptLoadVars_sendAndLoad:

            {
                NATIVE_MEMBER_FUNCTION_DISPATCH(sendAndLoad) ;
            }
            break;
            case AptLoadVars_getBytesTotal:

            {
                NATIVE_MEMBER_FUNCTION_DISPATCH(getBytesTotal) ;
            }
            break;
            case AptLoadVars_getBytesLoaded:

            {
                NATIVE_MEMBER_FUNCTION_DISPATCH(getBytesLoaded) ;
            }
            break;
            case AptLoadVars_loaded:

            {
        //      NATIVE_MEMBER_FUNCTION_DISPATCH(loaded) ;
                return AptBoolean::Create(this->iIsLoaded != 0) ;
            }
            break;
            case AptLoadVars_toString:

            {
                NATIVE_MEMBER_FUNCTION_DISPATCH(toString) ;
            }
            break;
            case AptLoadVars_contentType:

            {
        //      NATIVE_MEMBER_FUNCTION_DISPATCH(contentType) ;
                AptString *pString = AptString::Create();
                pString->cpy("application/x-www-form-urlencoded") ;
                return pString ;
            }
        }
    }

#if defined(EA_DEBUG)
    if (
        (pName->EqualNoCase("load")) ||
        (pName->EqualNoCase("send")) ||
        (pName->EqualNoCase("sendAndLoad")) ||
        (pName->EqualNoCase("getBytesTotal")) ||
        (pName->EqualNoCase("getBytesLoaded")) ||
        (pName->EqualNoCase("loaded")) ||
        (pName->EqualNoCase("toString")) ||
        (pName->EqualNoCase("contentType"))
        )
    {
        APT_DEBUGPRINT(APT_DEBUG_MSG_WARNING_LVL, "AptLoadVars: Incorrect case for '%s'.\n", pName->c_str());
        APT_ASSERT(0);
    }
#endif

    return 0;
}
#endif //#if APT_USE_LOADVARS_OBJECT

AptLoadVars::~AptLoadVars()
{
    //  Do nothing...
}

/*F************************************************************************************************/
/*!
    Releases the AptNativeScript functions.

    \Input      None.

    \Output     None.

    \Notes
    This function needs to be called at least during AptShutdown.

    \Version
        1.0        04/05/04 (ONA) First Version

*/
/************************************************************************************************F*/
void    AptLoadVars::CleanNativeFunctions()
{
#if APT_USE_LOADVARS_OBJECT
    NATIVE_MEMBER_FUNCTION_DESTROY(load);
    NATIVE_MEMBER_FUNCTION_DESTROY(send);
    NATIVE_MEMBER_FUNCTION_DESTROY(sendAndLoad);
    NATIVE_MEMBER_FUNCTION_DESTROY(getBytesTotal);
    NATIVE_MEMBER_FUNCTION_DESTROY(getBytesLoaded);
    NATIVE_MEMBER_FUNCTION_DESTROY(toString);
#endif
}

#if APT_USE_LOADVARS_OBJECT
// This function will call interpreter.loadvariables, with this object as context, so loadvariables
// function will fill all the variables inside this object.
NATIVE_MEMBER_FUNCTION(AptLoadVars, load)
{
    pThis->c_loadvars()->iIsLoaded = 0 ;
    if ( (nParams <= 0) || (nParams > 1))  return AptBoolean::Create(false) ;
    AptValue *pUrl = gAptActionInterpreter.stackAt(0);
    AptNativeString sBuf ;
    pUrl->toString(sBuf) ;

    // first unset all the properties currrently present in this loadVars object
    AptNativeHash *pObjHash = pThis->GetNativeHashVirtual();
    if (!pObjHash)
        return AptBoolean::Create(false) ;

    for (AptHashItem *pInitItem = pObjHash->GetFirstItem(); pInitItem; pInitItem = pObjHash->GetNextItem(pInitItem))
    {
        // TODO: theoretically, on every hash key there's supposed to be a DontEnum/DontDelete flag; we don't have that so hardcode some dontenum's here..
        if( (pInitItem->Key.Equal(*StringPool::GetString(SC___proto__))) ||
            (pInitItem->Key.Equal(*StringPool::GetString(SC_prototype))) )
        {
            continue;
        }

        pObjHash->Unset(&pInitItem->Key) ;
    }

    // now get new variables from auxiliary library.
    gAptActionInterpreter.loadVariables(pThis, NULL, &sBuf);
    pThis->c_loadvars()->iIsLoaded = 1 ;
    return AptBoolean::Create(true) ;
}

NATIVE_MEMBER_FUNCTION(AptLoadVars, send)
{
    if ( (nParams <= 0) || (nParams > 3))  return AptBoolean::Create(false) ;

    AptValue *pUrl = gAptActionInterpreter.stackAt(0);
    AptNativeString sUrl ;
    pUrl->toString(sUrl) ;

//  AptValue *pTarget = NULL ;
    AptNativeString sTarget ;
    if (nParams > 1)
    {
        AptValue *pTarget = gAptActionInterpreter.stackAt(1);
        pTarget->toString(sTarget) ;
    }

//  AptValue *pMethod = NULL ;
    AptNativeString sMethod ;
    if (nParams > 2)
    {
        AptValue *pMethod = gAptActionInterpreter.stackAt(2);
        pMethod->toString(sMethod) ;
    }

    AptNativeString sProp = pThis->urlEncode() ;

    APT_ASSERT(gAptFuncs.pfnSendVariables);
    {APT_PROFILE_AUXILIARY(pfnSendVariables);
       if (gAptFuncs.pfnSendVariables != NULL)
            gAptFuncs.pfnSendVariables(sUrl.ConstRawPtr(), sTarget.ConstRawPtr(), sMethod.ConstRawPtr(), sProp.ConstRawPtr(), 0);
    }

    return AptBoolean::Create(true) ;
}

NATIVE_MEMBER_FUNCTION(AptLoadVars, sendAndLoad)
{
    pThis->c_loadvars()->iIsLoaded = 0 ;
    if ( (nParams <= 0) || (nParams > 3))  return AptBoolean::Create(false) ;

    AptValue *pUrl = gAptActionInterpreter.stackAt(0);
    AptNativeString sUrl ;
    pUrl->toString(sUrl) ;

    AptValue *pTarget = NULL ;
    AptNativeString sTarget ;
    if (nParams > 1)
    {
        pTarget = gAptActionInterpreter.stackAt(1);
    }

    AptValue *pMethod = NULL ;
    AptNativeString sMethod ;
    if (nParams > 2)
    {
        pMethod = gAptActionInterpreter.stackAt(2);
        pMethod->toString(sMethod) ;
    }

    AptNativeString sProp = pThis->urlEncode() ;

    APT_ASSERT(gAptFuncs.pfnSendVariables);
    {APT_PROFILE_AUXILIARY(pfnSendVariables);
    if (gAptFuncs.pfnSendVariables != NULL)
        gAptFuncs.pfnSendVariables(sUrl.ConstRawPtr(), sTarget.ConstRawPtr(), sMethod.ConstRawPtr(), sProp.ConstRawPtr(), 1);
    }

    // first unset all the properties currrently present in this loadVars object
    AptNativeHash *pObjHash = pThis->GetNativeHashVirtual();
    if (!pObjHash)
        return AptBoolean::Create(false) ;

    for (AptHashItem *pInitItem = pObjHash->GetFirstItem(); pInitItem; pInitItem = pObjHash->GetNextItem(pInitItem))
    {
        // TODO: theoretically, on every hash key there's supposed to be a DontEnum/DontDelete flag; we don't have that so hardcode some dontenum's here..
        if( (pInitItem->Key.Equal(*StringPool::GetString(SC___proto__))) ||
            (pInitItem->Key.Equal(*StringPool::GetString(SC_prototype))) )
        {
            continue;
        }

        pObjHash->Unset(&pInitItem->Key) ;
    }
    // now call loadvariables to get result of send operation with NULL paramter.
    gAptActionInterpreter.loadVariables(pTarget, NULL, NULL);
    pThis->c_loadvars()->iIsLoaded = 1 ;
    return AptBoolean::Create(true) ;
}
NATIVE_MEMBER_FUNCTION(AptLoadVars, getBytesTotal)
{
    APT_ASSERT(gAptFuncs.pfnGetBytesTotal) ;
    APT_PROFILE_AUXILIARY(pfnGetBytesTotal);
    float fFloat = (float) gAptFuncs.pfnGetBytesTotal(NULL, AptGetBytesEnum_LoadVars);
    return AptFloat::Create(fFloat) ;
}
NATIVE_MEMBER_FUNCTION(AptLoadVars, getBytesLoaded)
{
    APT_ASSERT(gAptFuncs.pfnGetBytesLoaded) ;
    APT_PROFILE_AUXILIARY(pfnGetBytesLoaded);
    float fFloat = (float) gAptFuncs.pfnGetBytesLoaded(NULL, AptGetBytesEnum_LoadVars);
    return AptFloat::Create(fFloat) ;

//  return gpUndefinedValue ;
}

NATIVE_MEMBER_FUNCTION(AptLoadVars, toString)
{
//  return gpUndefinedValue ;
    AptString * pRet = AptString::Create();
    pRet->cpy(pThis->urlEncode().ConstRawPtr()) ;
    return pRet ;
}
#endif //#if APT_USE_LOADVARS_OBJECT

//------------------------------------------------------------------------------------------
// AptError object

AptValue *    AptError::objectMemberLookup(AptValue * const pContext, const AptNativeString * const pName) const
{
    if(pName->Equal("message"))
    {
        AptString * t = AptString::Create();
        t->cpy(msMessage);
        return t;
    }
    else if(pName->Equal("name"))
    {
        AptString * t = AptString::Create();
        t->cpy(msName);
        return t;
    }
    else if(pName->Equal("toString"))
    {
        NATIVE_MEMBER_FUNCTION_DISPATCH(toString) ;
    }

#if defined(EA_DEBUG)
    if (
        (pName->EqualNoCase("message")) ||
        (pName->EqualNoCase("name")) ||
        (pName->EqualNoCase("toString"))
        )
    {
        APT_DEBUGPRINT(APT_DEBUG_MSG_WARNING_LVL, "AptError: Incorrect case for '%s'.\n", pName->c_str());
        APT_ASSERT(0);
    }
#endif

    return 0;
}

bool AptError::objectMemberSet(AptValue * const pContext, const AptNativeString * const pName, AptValue * const pValue)
{
    if(pName->Equal("message"))
    {
        AptNativeString t;
        pValue->toString(t);
        msMessage.Duplicate(t);
        return true;
    }
    else if(pName->Equal("name"))
    {
        AptNativeString t;
        pValue->toString(t);
        msName.Duplicate(t);
        return true;
    }
#if defined(EA_DEBUG)
    if (
        (pName->EqualNoCase("message")) ||
        (pName->EqualNoCase("name"))
        )
    {
        APT_DEBUGPRINT(APT_DEBUG_MSG_WARNING_LVL, "AptError: Incorrect case for '%s'.\n", pName->c_str());
        APT_ASSERT(0);
    }
#endif

    return 0;
}

AptError::~AptError()
{
    //  Do nothing...
}
NATIVE_MEMBER_FUNCTION(AptError, toString)
{
    //  return gpUndefinedValue ;
    AptString * pRet = AptString::Create();
    pRet->cpy(pThis->urlEncode().ConstRawPtr()) ;
    return pRet ;
}

void    AptError::CleanNativeFunctions()
{
    NATIVE_MEMBER_FUNCTION_DESTROY(toString) ;
}

//------------------------------------------------------------------------------------------
// AptString object
AptStringObject::~AptStringObject()
{
    APT_DECSAFE(mpStringObject);
    mpStringObject = NULL;
}

void AptStringObject::setString(AptString*pString)
{
    APT_INC(pString)
        mpStringObject = pString;
}

AptValue *  AptStringObject::objectMemberLookup(AptValue * const pContext, const AptNativeString * const pName) const
{
    AptValue * pValue = mpStringObject->objectMemberLookup(pContext, pName);
    if(pValue == NULL)
    {
        pValue = AptObject::objectMemberLookup(pContext, pName);
    }
    return pValue;
}

//------------------------------------------------------------------------------------------
// AptStage object

#if defined (APT_USE_STAGE_OBJECT)
/*F************************************************************************************************/
/*!
    \Function    AptStage::CleanNativeFunctions

    \Description
        Cleans up the native functions

    \Input
        None.

    \Output
        None.

    \Notes
        None.

    \Version    1.0        06/28/04 (LEO) First Version

*/
/************************************************************************************************F*/
void    AptStage::CleanNativeFunctions()
{
#if defined (APT_USE_STAGE_OBJECT)
    NATIVE_MEMBER_FUNCTION_DESTROY(addListener);
    NATIVE_MEMBER_FUNCTION_DESTROY(removeListener);
#endif
}

/*F************************************************************************************************/
/*!
    \Function    AptStage::objectMemberLookup

    \Description
        Insert function's purpose here

    Insert your long description here.

    \Input      pContext          -
    \Input      pName             -

    \Output     AptValue *        -

    \Notes
        None.

    \Version    1.0        06/28/04 (LEO) First Version

*/
/************************************************************************************************F*/
AptValue *    AptStage::objectMemberLookup(AptValue * const pContext, const AptNativeString * const pName) const
{
    StageMembers *pProp = pContext ? StageMembersIndex::in_word_set(pName->c_str(), pName->Size()) : NULL;

    if (pProp)
    {
        switch(pProp->nIndex)
        {
            case(AptStage_align):
            {
                return gpUndefinedValue;                // Why are we going to support this?
            }
            break;
            case AptStage_height:

            {
                APT_ASSERT(gAptFuncs.pfnGetStageHeight);
                return AptFloat::Create(gAptFuncs.pfnGetStageHeight());
            }
            break;
            case AptStage_width:

            {
                APT_ASSERT(gAptFuncs.pfnGetStageWidth);
                return AptFloat::Create(gAptFuncs.pfnGetStageWidth());
            }
            break;
            case AptStage_scaleMode:

            {
                return gpUndefinedValue;                // Why are we going to support this?
            }
            break;
            case AptStage_addListener:

            {
                APT_ASSERT(false && "addListener is not supported yet");
                //NATIVE_MEMBER_FUNCTION_DISPATCH(addListener);
            }
            break;
            case AptStage_removeListener:

            {
                APT_ASSERT(false && "removeListener is not support yet");
                //NATIVE_MEMBER_FUNCTION_DISPATCH(removeListener);
            }
        }
    }

#if defined(EA_DEBUG)
    if (
        (pName->EqualNoCase("align")) ||
        (pName->EqualNoCase("height")) ||
        (pName->EqualNoCase("width")) ||
        (pName->EqualNoCase("scaleMode")) ||
        (pName->EqualNoCase("addListener")) ||
        (pName->EqualNoCase("removeListener"))
        )
    {
        APT_DEBUGPRINT(APT_DEBUG_MSG_WARNING_LVL, "AptMathObj: Incorrect case for '%s'.\n", pName->c_str());
        APT_ASSERT(0);
    }

#endif

    return 0;
}


NATIVE_MEMBER_FUNCTION(AptStage, addListener)
{
    //if (nParams != 1)
    //    return gpUndefinedValue;

    //AptValue *pParam = gAptActionInterpreter.stackAt(0);
    //if (pParam->getIsDefined())
    //{
    //    // object is defined and valid so now add it to mListenerSet in GetTargetSim()->GetAnimationTarget().
    //    if(!GetTargetSim()->GetAnimationTarget()->GetListenerSet()->has(pParam))
    //    {
    //        GetTargetSim()->GetAnimationTarget()->GetListenerSet()->add(pParam) ;
    //    }
    //}

    return gpUndefinedValue;
}

NATIVE_MEMBER_FUNCTION(AptStage, removeListener)
{
    //if (nParams != 1)
    //    return gpUndefinedValue;

    //AptValue *pParam = gAptActionInterpreter.stackAt(0);
    //if (pParam->getIsDefined())
    //{
    //    // object is defined and valid so now remove it from listener set
    //    if(GetTargetSim()->GetAnimationTarget()->GetListenerSet()->has(pParam))
    //    {
    //        GetTargetSim()->GetAnimationTarget()->GetListenerSet()->remove(pParam) ;
    //    }
    //}

    return gpUndefinedValue;
}
#endif  //#if defined (APT_USE_STAGE_OBJECT)

//************************************************************************************************


#if defined(APT_ALTERNATE_INPUT)
/*F************************************************************************************************/
/*!
    \Function    AptAlternateInput::CleanNativeFunctions

    \Description
    Cleans up the native functions

    \Input
    None.

    \Output
    None.

    \Notes
    None.

    \Version    1.0        12/06/05 (ASK) First Version

*/
/************************************************************************************************F*/
void    AptAlternateInput::CleanNativeFunctions()
{
    NATIVE_MEMBER_FUNCTION_DESTROY(addListener);
    NATIVE_MEMBER_FUNCTION_DESTROY(removeListener);
}

/*F************************************************************************************************/
/*!
    \Function    AptAlternateInput::objectMemberLookup

    \Description
    Returns the AptValue for the context and the member name.

    \Input      pContext          - The context passed in.
    \Input      pName             - The member name.

    \Output     AptValue *        - The AptValue of the member.
    gpUndefinedValue if the member is unknown.

    \Notes
    None.

    \Version    1.0        12/06/05 (ASK) First Version

*/
/************************************************************************************************F*/
AptValue *    AptAlternateInput::objectMemberLookup(AptValue * const pContext, const AptNativeString * const pName) const
{
    if (*pName == "addListener")
    {
        NATIVE_MEMBER_FUNCTION_DISPATCH(addListener);
    }
    else if (*pName == "removeListener")
    {
        NATIVE_MEMBER_FUNCTION_DISPATCH(removeListener);
    }
    return 0;
}


/*F************************************************************************************************/
/*!
    \Function    AptAlternateInput::addListener

    \Description
    Adds the passed in parameter to alternate input set

    \Input
    None

    \Output         gpUndefinedValue

    \Notes
    None.

    \Version    1.0        12/06/05 (ASK) First Version

*/
/************************************************************************************************F*/
NATIVE_MEMBER_FUNCTION(AptAlternateInput, addListener)
{
    if (nParams != 1)
        return gpUndefinedValue;

    AptValue *pParam = gAptActionInterpreter.stackAt(0);
    if (pParam->getIsDefined())
    {
        if (pParam->isCIH()) // $bug788 - MCB - don't add to listener set if the CIH is a Zombie.
        {
            if (((AptCIH*)pParam)->GetCIHState() != AptCIH::AptCIHState_Normal)
            {
                return gpUndefinedValue;
            }
        }
        // object is defined and valid so now add it to listenerSet in gpPool.

        if(!GetTargetSim()->GetAnimationTarget()->GetAlternateInputSet()->has(pParam))
            GetTargetSim()->GetAnimationTarget()->GetAlternateInputSet()->add(pParam);
    }

    return gpUndefinedValue;
}


/*F************************************************************************************************/
/*!
    \Function    AptAlternateInput::removeListener

    \Description
    removes the passed in parameter from alternate input set

    \Input
    None

    \Output         gpUndefinedValue

    \Notes
    Action script programmer should remember to call this function when object is going out of scope

    \Version    1.0        12/06/05 (ASK) First Version

*/
/************************************************************************************************F*/
NATIVE_MEMBER_FUNCTION(AptAlternateInput, removeListener)
{
    if (nParams != 1)
        return AptBoolean::Create(false) ;

    AptValue *pParam = gAptActionInterpreter.stackAt(0);
    if (pParam->getIsDefined())
    {
        // object is defined and valid so now remove it from listener set
        if(GetTargetSim()->GetAnimationTarget()->GetAlternateInputSet()->has(pParam->c_cih()))
        {
            GetTargetSim()->GetAnimationTarget()->GetAlternateInputSet()->remove(pParam->c_cih());
            return AptBoolean::Create(true) ;
        }
    }

    return AptBoolean::Create(false) ;
}
#endif //#if defined(APT_ALTERNATE_INPUT)
