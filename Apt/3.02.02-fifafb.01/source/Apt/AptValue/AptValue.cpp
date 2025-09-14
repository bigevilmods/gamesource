/*H************************************************************************************************/
/*!

    \File
        AptValue.cpp

    \Description
        Simple description of main purpose of file.

    \Notes
        None.

    \Copyright
        (c) 2005 Electronic Arts Inc.

    \Version
        1.0        03/07/04 (ONA) First Version
        1.01       02/13/07 (RB)  Minor optimization of isMCInParentChain().
        1.02       08/22/07 (RB)  Reverted EATech 109497 change due to broken dynamic cafe controls.
        1.03       09/05/07 (RB)  Fix by $senzee to avoid assert in AptValue::Release
        1.04       01/31/08 (RB)  One-line change in findChild to avoid rev-mw warning
                                  about mixing int and int32_t.
        1.05       03/05/08 (RB)  In setGCRoot, don't compare unsigned >= 0.
        1.06       06/12/08 (JP)  Adding support for the MovieClipLoader class
        1.07       08/14/08 ($senzee) Added GetClassName(), SetClassName() for improved opt call stack information,
                                      Also added support for AptUtil.
        1.08       03/06/09 (RB)  Don't check for SwfVersion==7. Check >= 7 instead.
        1.09       06/09/09 (DG)  FogBug 1233: AptNativeHash change from Tiger10:
                                  change in findChild method (see method header for detail).
        1.10       10/23/09 (Colin C.) Change in findChild to not return items in global scope when searching for members.
        1.11       11/10/09 (RB)   In findChild: Don't crash when looking up _global._root. Don't look in
                                   for global properties/methods when looking for a member.
*/
/************************************************************************************************H*/

#include "_Apt.h"
#include "String/StringPool.h"
#include "Display/AptRenderingContext.h"
#include "AptFrameStack.h"
#include "AptGlobal.h"
#include "AptValue/AptValueVector.h"
#include "AptObject/AptStringObject.h"
#include "AptObject/AptDate.h"
#include "AptObject/AptMathObj.h"
#include "AptObject/AptKey.h"
#include "AptObject/AptLoadVars.h"
#include "AptObject/AptSound.h"
#include "AptObject/AptMouse.h"
#include "AptObject/AptError.h"
#include "AptObject/AptStage.h"
#include "AptObject/AptUtil.h"
#include "AptObject/AptGlobalObject.h"
#include "AptObject/AptGlobalExtensionObject.h"
#include "MainInline.h"


//#define APT_USE_LOCKS_APTVALUE_INCDEC   1

#if !defined(APT_ENABLE_INLINE)
    #include "AptValue/AptValue.inl"
#endif

static const int AptObject_target = 1;
static const int AptObjectthis = 2;
static const int AptObject_root = 3;
static const int AptObjectKey = 4;
static const int AptObjectMath = 5;
static const int AptObject_level0 = 6;
static const int AptObject_level1 = 7;
static const int AptObject_level2 = 8;
static const int AptObject_level3 = 9;
static const int AptObject_level4 = 10;
static const int AptObject_level5 = 11;
static const int AptObject_level6 = 12;
static const int AptObject_level7 = 13;
static const int AptObject_level8 = 14;
static const int AptObject_level9 = 15;
static const int AptObjectparent = 16;
static const int AptObjectextern = 17;
static const int AptObjectsuper = 18;
static const int AptObject_global = 19;
static const int AptObjectMouse = 20;
static const int AptObject_level10 = 21;
static const int AptObject_level11 = 22;
static const int AptObject_level12 = 23;
static const int AptObject_level13 = 24;
static const int AptObject_level14 = 25;
static const int AptObject_level15 = 26;
static const int AptObject_level16 = 27;
static const int AptObject_level17 = 28;
static const int AptObject_level18 = 29;
static const int AptObject_level19 = 30;
static const int AptObject_level20 = 31;
static const int AptObject_level21 = 32;
static const int AptObject_level22 = 33;
static const int AptObject_level23 = 34;
static const int AptObject_level24 = 35;
static const int AptObject_String  = 36;
static const int AptObject_Stage  = 37;
static const int AptObject_AlternateInput  = 38;
static const int AptObject_AptUtil = 39;

AptMatrix gIdentityMatrix;
AptCXForm gIdentityCXForm;
AptNativeFunction *gpCBsetInterval;
AptNativeFunction *gpCBclearInterval;
AptNativeFunction *gpCBisNaN;
AptNativeFunction *gpCBunescape;
AptNativeFunction *gpCBescape;
AptNativeFunction *gpCBboolean ;
AptNativeFunction *gpASSetPropFlags;
AptNativeFunction *gpCBparseInt;
AptNativeFunction *gpCBparseFloat;

#if APT_USE_MATH_OBJECT
AptMathObj *    gpGlobalMathObject;
#endif
AptKey *        gpGlobalKeyObject;
#if defined(APT_USE_MOUSE)
AptMouse *      gpGlobalMouseObject;
#endif
AptNone *       gpUndefinedValue;
AptCIHNone*     gpUndefinedCIH;
AptExtern *     gpExternValue;
#if defined (APT_USE_STAGE_OBJECT)
AptStage *      gpGlobalStageObject;
#endif
AptString *     gpGlobalStringObject;    // (DC) Global String Object Added for Flash Player 7 / AS 2.0 support. (Release 17.0)

#if APT_USE_UTILITY
AptUtil   *    gpGlobalAptUtilObject;
#endif
#if defined(APT_ALTERNATE_INPUT)
AptAlternateInput *gpGlobalAltInputObject;
#endif

AptRenderingContext *gpRenderingContext;
AptValue * gpGlobalObjectPrototype ;
AptValue * gpGlobalMovieclipPrototype ;


// This is required to keep the hash table of Object.registerClass() registration
// it first gets created in object.registerclass
AptNativeHash     * gpObjRegistrationHash ;
AptNativeFunction * gpObjRegistrationFunc ;

int gbLeaksInPool;

//#define CHECK_REFCOUNT
#ifdef CHECK_REFCOUNT
AptValue *  mValuesToTest[10] =
{
    (AptValue*)0x81629408,
    (AptValue*)0,
    (AptValue*)0,
    (AptValue*)0,
    (AptValue*)0,
    (AptValue*)0,
    (AptValue*)0,
    (AptValue*)0,
    (AptValue*)0,
    (AptValue*)0
};
#endif

APT_DEFINE_OBJECT_METRICS_EXPLICIT(AptValue, NULL);
APT_DEFINE_OBJECT_METRICS(AptValueGC, AptValue);
APT_DEFINE_OBJECT_METRICS(AptValueNoGC, AptValue);

bool AptValue::sbSuspendRefcountDeletions = false;

void  (*AptValue::sReferenceRegistrationCb) (const AptValue * pFromRef, AptValue * &pToRef, const char * pReferenceName, int32_t bFlag)= NULL;

#if defined(APT_XML_EXTENDED_MEMORY_DUMP_SUPPORTED)
uint32_t AptValue::snCurrentAllocationNumber=0;   // DC - 11/1/04 - made this an extended feature.
#endif

#if defined(APT_ENABLE_DEBUG_REPORT)
EA::Apt::Debug::AptReport* AptValue::mReport = NULL;
#endif

/*F************************************************************************************************/
/*!
    \Function    Constructor

    \Input      eType             - vtbl index (type of the object)

    \Output
        None.

    \Notes
        - (LEO) Added Fix for Bug 663

    \Version    1.0        03/30/04 (JSH) First Version
                1.1        04/20/04 (LEO) Second Version
                2.0        02/08/05 (LEO) $bug817: gpObjectPrototype deleted early due to MAX_REFCOUNT is reached.

*/
/************************************************************************************************F*/
AptValue::AptValue(AptVirtualFunctionTable_Indices eType)
{
    setVtblIndex(eType);
    setRefCount(0);
    setIsDefined(1);
    setGCMark(false);
    setGCRoot(0);
    SetAllowDelayedDeletion(true);

    #if (APT_DEBUG_LEVEL >= APT_DEBUG_NORMAL)
        ClearDestroyedGC();
    #endif
#if defined(APT_ENABLE_DEBUG_REPORT)
    if(mReport)
    {
        mReport->StartTrack(this);
    }
#endif
#if defined(APT_XML_EXTENDED_MEMORY_DUMP_SUPPORTED)
    mnID = snCurrentAllocationNumber++;           // DC - 11/1/04 - made this an extended feature.
#endif
    //  All the newly created values are flagged
    //  So they are automatically destroyed at the end of the frame
    //  if not used anymore
    // DC - Extension objects and native functions should not be placed in the deferred vector
    if(eType == AptVFT_Prototype || eType == AptVFT_ScriptFunction1 || eType == AptVFT_ScriptFunction2 || eType == AptVFT_NativeFunction || eType == AptVFT_Extension || eType == AptVFT_ExternalFunction)             // (LEO) $BUG663 1/1  These etypes not being used in the deferred vector.
    {
        ClearReleaseAtEnd();
    }
    else
    {
        SetReleaseAtEnd();
        gpValuesToRelease->PushValue(this);
    }
    SetMaxRefCountHit(false);               // (LEO) $bug817: gpObjectPrototype deleted early due to MAX_REFCOUNT is reached.
}

/*F************************************************************************************************/
/*!
    \Function    Constructor

    \Description
        For CIH we use this constructor, so the pointer is not pushed on for defered release

    \Input      eType             - vtbl index (type of the object)
    \Input      eCIH              - CO_CIH

    \Output
        None.

    \Notes
        None.

    \Version    1.0        03/30/04 (JSH) First Version
                2.0        02/08/05 (LEO) $bug817: gpObjectPrototype deleted early due to MAX_REFCOUNT is reached.

*/
/************************************************************************************************F*/
AptValue::AptValue(AptVirtualFunctionTable_Indices eType, const CIH_ONLY eCIH)
{
    setVtblIndex(eType);
    setRefCount(0);
    setIsDefined(1);
    setGCMark(false);
    setGCRoot(0);
    SetAllowDelayedDeletion(false);
    #if (APT_DEBUG_LEVEL >= APT_DEBUG_NORMAL)
        ClearDestroyedGC();
    #endif

#if defined(APT_ENABLE_DEBUG_REPORT)
    if(mReport)
    {
        mReport->StartTrack(this);
    }
#endif
#if defined(APT_XML_EXTENDED_MEMORY_DUMP_SUPPORTED)
        mnID = snCurrentAllocationNumber++;      // DC - 11/1/04 - made this an extended feature.
#endif
    SetMaxRefCountHit(false);               // (LEO) $bug817: gpObjectPrototype deleted early due to MAX_REFCOUNT is reached.
    ClearReleaseAtEnd();
}

/*F************************************************************************************************/
/*!
    \Function    AptValue::SetAllowDelayedDeletion

    \Description
        This tells Apt Value internals whether or not the object should be put on the deferred vector.

    \Input      bAllowed          -

    \Output
        None.

    \Notes
        None.

    \Version    1.0        07/18/05 (DC) Bringin' it to ya

*/
/************************************************************************************************F*/
void         AptValue::SetAllowDelayedDeletion(bool bAllowed)
{
    mValueBitfield.mbAllowsDelayedDeletion = bAllowed ? 1 : 0;
}

/*F************************************************************************************************/
/*!
    \Function    AptValue::setRefCount

    \Description
        Sets the object's reference count

    \Input      n       - the value to set the ref count to

    \Output
        None.

    \Notes
        None.

    \Version    1.0        03/30/04 (JSH) First Version

*/
/************************************************************************************************F*/
void AptValue::setRefCount(uint32_t n)
{

    // $bug584,$bug573 - DC - 1 of 1 - Added in 17.00
    // This does not fix the reason why we got to a uMaxRefCount, but it prevents us from crashing
    // when ref counts would roll back to 0. This still asserts in Debug mode to help us address
    // possible ref counting issues...
    if(n > MAX_REFCOUNT)
    {
        //APT_ASSERT(false); // Assert on Debug Builds. This is typically a refcount problem. we shouldn't be hitting this in normal execution.
        SetMaxRefCountHit(true);        // (LEO) -$bug817: gpObjectPrototype deleted early due to MAX_REFCOUNT is reached.
        n = MAX_REFCOUNT;
    }

    mValueBitfield.mnReferenceCount = n;
}


/*F************************************************************************************************/
/*!
    \Function    AptValue::setVtblIndex

    \Description
        set the vtbl index (type) of the object

    \Input      n   - the value to set the vtbl index to

    \Output
        None.

    \Notes
        None.

    \Version    1.0        03/30/04 (JSH) First Version

*/
/************************************************************************************************F*/
void AptValue::setVtblIndex(AptVirtualFunctionTable_Indices n)
{
    APT_ASSERT(n > AptVFT_xxx);
    APT_ASSERT(n < AptVFT_NumVFTs);
    mValueBitfield.meValueType = n;
}

/*F************************************************************************************************/
/*!
    \Function    AptValue::setIsDefined

    \Description
        Set whether or not the object is defined

    \Input      bDefined          - 1 to set as defined, 0 to set as undefined

    \Output
        None.

    \Notes
        None.

    \Version    1.0        03/30/04 (JSH) First Version

*/
/************************************************************************************************F*/
void AptValue::setIsDefined(bool bDefined)
{
    mValueBitfield.mbIsDefined = bDefined ? 1 : 0;
}

/*F************************************************************************************************/
/*!
    \Function    AptValue::setGCMark

    \Description
        Sets the GC mark value for the object

    \Input      bMark             - the GC mark value for the object

    \Output
        None.

    \Notes
        None.

    \Version    1.0        03/30/04 (JSH) First Version

*/
/************************************************************************************************F*/
void AptValue::setGCMark(bool bMark)
{
    mValueBitfield.mbHasRegisterReferenceMark = bMark ? 1 : 0;
}

/*F************************************************************************************************/
/*!
    \Function    AptValue::setGCRoot

    \Description
        Set the GC root counter for the object

    \Input      nRoot             - the value to set the GC root counter to

    \Output
        None.

    \Notes
        None.

    \Version    1.0        03/30/04 (JSH) First Version
    \Version    1.1        03/05/08 (RB)  Don't compare unsigned >= 0

*/
/************************************************************************************************F*/
void AptValue::setGCRoot(uint32_t nRoot)
{
    APT_ASSERT(nRoot <= MAX_GCROOT);
    mValueBitfield.mnGCRootCount = nRoot;
}
/*F************************************************************************************************/
/*!
    \Function    AptValue::incGCRoot

    \Description
        increment the GC root counter for the object

    \Input
        None.

    \Output
        None.

    \Notes
        None.

    \Version    1.0        03/30/04 (JSH) First Version

*/
/************************************************************************************************F*/
void AptValue::incGCRoot()
{
    APT_ASSERT(mValueBitfield.mnGCRootCount < MAX_GCROOT);

    if( mValueBitfield.mnGCRootCount < MAX_GCROOT )
        mValueBitfield.mnGCRootCount++;
}

/*F************************************************************************************************/
/*!
    \Function    AptValue::decGCRoot

    \Description
        decrement the GC root counter for the object

    \Input
        None.

    \Output
        None.

    \Notes
        None.

    \Version    1.0        03/30/04 (JSH) First Version

*/
/************************************************************************************************F*/
void AptValue::decGCRoot()
{
    APT_ASSERT(mValueBitfield.mnGCRootCount > 0);
    if( mValueBitfield.mnGCRootCount > 0)
        mValueBitfield.mnGCRootCount--;
}

/*F************************************************************************************************/
/*!
    \Function    AptValue::SetClassName()

    \Description
        Sets the name of the class this AptValue represents.

    \Version    1.0        08/14/08 ($senzee) First Version
*/
/************************************************************************************************F*/
void AptValue::SetClassName(const char *name)
{
    AptNativeHash *pHash = const_cast<AptValue *>(this)->GetNativeHashVirtual();
    if (pHash)
    {
        AptPrototype *pPrototype = (AptPrototype *)pHash->GetPrototype();
        if (pPrototype)
            pPrototype->SetClassName(name);
    }
}

/*F************************************************************************************************/
/*!
    \Function    AptValue::GetClassName()

    \Description
        Returns the name of the class this AptValue represents.

    \Version    1.0        08/14/08 ($senzee) First Version
*/
/************************************************************************************************F*/
const char *AptValue::GetClassName() const
{
    AptNativeHash *pHash = const_cast<AptValue *>(this)->GetNativeHashVirtual();
    if (pHash)
    {
        AptPrototype *pPrototype = (AptPrototype *)pHash->GetPrototype();
        const char *name = pPrototype ? pPrototype->GetClassName() : 0;
        if (name)
            return name;
    }
    AptVirtualFunctionTable_Indices index = mValueBitfield.meValueType;
    switch (index)
    {
    case AptVFT_StringValue:
    case AptVFT_StringObject:                return "String";
    case AptVFT_Property:                    return "Property";
    case AptVFT_CIHNone:
    case AptVFT_None:                        return "None";
    case AptVFT_Register:                    return "Register";
    case AptVFT_Boolean:                     return "Boolean";
    case AptVFT_Float:                       return "Float";
    case AptVFT_Integer:                     return "Integer";
    case AptVFT_Lookup:                      return "Lookup";
    case AptVFT_ScriptFunction1:
    case AptVFT_ScriptFunction2:
    case AptVFT_ScriptFunctionByteCodeBlock: return "Function";
    case AptVFT_NativeFunction:              return "NativeFunction";
    case AptVFT_FrameStack:                  return "FrameStack";
    case AptVFT_Extern:                      return "Extern";
    case AptVFT_CharacterInstHandle:         return "CharacterInstHandle";
    case AptVFT_Sound:                       return "Sound";
    case AptVFT_Array:                       return "Array";
    case AptVFT_Math:                        return "Math";
    case AptVFT_Key:                         return "Key";
    case AptVFT_Global:                      return "Global";
    case AptVFT_ScriptColour:                return "ScriptColor";
    case AptVFT_Object:                      return "Object";
    case AptVFT_Prototype:                   return "Prototype";
    case AptVFT_Date:                        return "Date";
    case AptVFT_MovieClip:                   return "MovieClip";
    case AptVFT_Mouse:                       return "Mouse";
    case AptVFT_XmlNode:                     return "XmlNode";
    case AptVFT_Xml:                         return "Xml";
    case AptVFT_XmlAttributes:               return "XmlAttributes";
    case AptVFT_LoadVars:                    return "LoadVars";
    case AptVFT_TextFormat:                  return "TextFormat";
    case AptVFT_Extension:                   return "Extension";
    case AptVFT_GlobalExtension:             return "GlobalExtension";
    case AptVFT_Stage:                       return "Stage";
    case AptVFT_Error:                       return "Error";
    case AptVFT_MovieClipLoader:             return "MovieClipLoader";
    case AptVFT_AptUtil:                     return "AptUtil";
    case AptVFT_ExternalFunction:            return "ExternalFunction";
    default:                                 return NULL;
    };
}

#if (APT_DEBUG_LEVEL >= APT_DEBUG_NORMAL)

/*F************************************************************************************************/
/*!
    \Function    AptValue::SetDestroyedGC

    \Description
        Insert function's purpose here

    \Input
        None.

    \Output
        None.

    \Notes
        None.

    \Version    1.0        03/30/04 (JSH) First Version

*/
/************************************************************************************************F*/
void AptValue::SetDestroyedGC()
{
    mValueBitfield.mbDestroyedGC = 1;
}

/*F************************************************************************************************/
/*!
    \Function    AptValue::ClearDestroyedGC

    \Description
        Insert function's purpose here

    \Input
        None.

    \Output
        None.

    \Notes
        None.

    \Version    1.0        03/30/04 (JSH) First Version

*/
/************************************************************************************************F*/
void AptValue::ClearDestroyedGC()
{
    mValueBitfield.mbDestroyedGC = 0;
}

#endif

/*F************************************************************************************************/
/*!
    \Function    AptValue::SetReleaseAtEnd

    \Description
        Insert function's purpose here

    \Input
        None.

    \Output
        None.

    \Notes
        None.

    \Version    1.0        03/30/04 (JSH) First Version

*/
/************************************************************************************************F*/
void AptValue::SetReleaseAtEnd()
{
    mValueBitfield.mbIsInDeferredVector = 1;
}

/*F************************************************************************************************/
/*!
    \Function    AptValue::ClearReleaseAtEnd

    \Description
        Insert function's purpose here

    \Input
        None.

    \Output
        None.

    \Notes
        None.

    \Version    1.0        03/30/04 (JSH) First Version

*/
/************************************************************************************************F*/
void AptValue::ClearReleaseAtEnd()
{
    mValueBitfield.mbIsInDeferredVector = 0;
}

/*F************************************************************************************************/
/*!
    \Function    *AptValue::c_string

    \Description
        Insert function's purpose here

    \Input
        None.

    \Output     AptString         * -

    \Notes
        None.

    \Version    1.0        03/30/04 (JSH) First Version

*/
/************************************************************************************************F*/
AptString *AptValue::c_string() const
{
    APT_ASSERT(isString());
    AptVirtualFunctionTable_Indices t = getVtblIndex();

    if(t == AptVFT_StringValue )
    {
        return (AptString *)this;
    }
    else
    {
        //lint -esym(1542, AptStringObject::mpStringObject)
        return ((AptStringObject*)this)->mpStringObject;
    }
}
/*F************************************************************************************************/
/*!
    \Function    AptValue::toInteger

    \Description
        Converts the AptValue to an integer

    Insert your long description here.

    \Input
        None.

    \Output     int               -

    \Notes
        -Added bug fix 545 String Hex values not converting to numbers correctly

    \Version    1.0        06/28/04 (LEO) First Version
    \Version    1.1        08/04/04 (DC)  Changed the way Objects are converted (if defined -> 1)

*/
/************************************************************************************************F*/
int AptValue::toInteger(void) const
{
    if (isUndefined())
    {
        return 0;
    }

    switch(getVtblIndex())
    {
        case AptVFT_StringObject:
        case AptVFT_StringValue:
        {
            AptString       *pString = c_string();
            AptNativeString *sBuf    = pString->GetInternalString();
            if(sBuf->Size() > 2 && sBuf->operator[](0) == '0' && sBuf->operator[](1) == 'x')        // (LEO) Bug545 String Hex values not converting to numbers correctly
            {
                return strtol(sBuf->c_str(), NULL, 16);
            }
            return atoi(sBuf->c_str());
        }
        break;
        case AptVFT_Boolean:
        {
            return ((AptBoolean *)this)->GetBool();
        }
        break;
        case AptVFT_Integer:
        {
            return ((AptInteger *)this)->GetInt();
        }
        break;
        case AptVFT_Float:
        {
            float fVal = (((AptFloat *)this)->GetFloat());
            APT_ASSERT((fVal <= (float)INT32_MAX  && fVal >= (float)INT32_MIN) && "This float value does not lie in the integer value range; this may cause unexpected behaviour.");
            return (fVal > (float)INT32_MAX) ? INT32_MAX : (fVal < (float)INT32_MIN) ? INT32_MIN : (int32_t)fVal;
        }
        break;
        default:
        {
            // DC - Objects are "true" if they are defined (or in this case not undefined)
            return (this != gpUndefinedValue) ? 1 : 0;
        }
    }
}


/*F************************************************************************************************/
/*!
\Function    AptValue::toFloat

\Description
Converts the AptValue to a float.

Insert your long description here.

\Input      None.

\Output     float               -  value converted to boolean

\Notes

\Version    1.0        06/28/04 (DC) First Version

*/
/************************************************************************************************F*/
float AptValue::toFloat(void) const
{
    if (isUndefined())
    {
        return 0.f;
    }

    switch(getVtblIndex())
    {
        case AptVFT_StringValue:
        case AptVFT_StringObject:
        {
            return (float)Apt_atoff( c_string()->GetInternalString()->c_str());
        }
        break;
        case AptVFT_Boolean:

        {
            return c_boolean()->GetBool()     ? 1.0f : 0.0f;
        }
        break;
        case AptVFT_Integer:

        {
            return (float) c_integer()->GetInt();
        }
        break;
        case AptVFT_Float:

        {
            return c_float()->GetFloat();
        }
        break;
        default:
        {
            // DC - Objects are "true" if they are defined (or in this case not undefined)
            return (this != gpUndefinedValue) ? 1.0f : 0.0f;
        }
    }
}

/*F************************************************************************************************/
/*!
\Function    AptValue::toBool

\Description
        Converts the AptValue to a boolean. Note that This conversion is slightly different for
        Flash 7 Files.

Insert your long description here.

\Input      None.

\Output     bool               -  value converted to boolean

\Notes

\Version    1.0        06/28/04 (DC) First Version

*/
/************************************************************************************************F*/
bool AptValue::toBool(void) const
{
    switch(getVtblIndex())
    {
    case AptVFT_StringValue:
    case AptVFT_StringObject:
        {
            if(AptGetSwfVersion()>=7)
            {
                // Much simpler Conversion in Flash 7 thankfully.
                return !c_string()->str.IsEmpty() ;
            }
            else
            {
                // Must convert to Int or string in Flash 6 (which sux)
                // we first handle Hex numbers (must be integers), if not hex, we convert to a float.
                AptString       *pString = c_string();
                AptNativeString *sBuf    = pString->GetInternalString();
                if(sBuf->Size() > 2 && sBuf->operator[](0) == '0' && sBuf->operator[](1) == 'x')
                {
                    return strtol(sBuf->c_str(), NULL, 16) != 0;
                }
                else
                {
                    return Apt_atoff( c_string()->GetInternalString()->c_str()) != 0.0f;
                }
            }
        }
    case AptVFT_Boolean:
        {
            return c_boolean()->GetBool();
        }
    case AptVFT_Integer:
        {
            return c_integer()->GetInt() != 0;
        }
    case AptVFT_Float:
        {
            return c_float()->GetFloat() != 0.0f;
        }
    default:
        {
            // Objects are "true" if they are defined (or in this case not undefined)
            return (this != gpUndefinedValue);
        }
    }
}

void AptValue::toString(char *szBuf) const
{
    AptNativeString sBuf;
    toString(sBuf);
    strcpy(szBuf, sBuf.ConstRawPtr());
}

/*F************************************************************************************************/
/*!
    \Function    AptValue::toString

    \Description
        Returns the string value of the AptValue

    Insert your long description here.

    \Input      &sBuf             -

    \Output
        None.

    \Notes
        -Added support for all AptVFT types.

    \Version    1.0        06/30/04 (LEO) First Version
    \Version    1.1        03/06/09 (RB)  Check GetSwfVersion() >= 7 instead of == 7

*/
/************************************************************************************************F*/
void AptValue::toString(AptNativeString &sBuf) const
{
    if (isUndefined())
    {
        if(AptGetSwfVersion() >= 7)
        {
            sBuf = *StringPool::GetString(SC_Undefined);
        }
        else
        {
            sBuf.Clear();
        }
        return;
    }

    // XXX be careful that all the sprintfs below are able to fit here!
    char szTemp[128];

    switch(getVtblIndex())
    {
        case AptVFT_StringValue:
        case AptVFT_StringObject:
        {
            sBuf = *(c_string()->GetInternalString());
            break;
        }
        case AptVFT_Boolean :
        {
            if (((AptBoolean *)this)->GetBool())
            {
                sBuf = *StringPool::GetString(SC_true);
            }
            else
            {
                sBuf = *StringPool::GetString(SC_false);
            }
            break ;
        }
        case AptVFT_Integer :
        {
            sprintf(szTemp, "%d", ((AptInteger *)this)->GetInt());
            sBuf = szTemp;
            break;
        }
        case AptVFT_Float :
        {
            AptFloat *pF = (AptFloat *)this;
            if (fmod(pF->GetFloat(), 1.f) == 0.f)
            {
                sprintf(szTemp, "%d", (int)pF->GetFloat());
            }
            else
            {
                sprintf(szTemp, "%f", pF->GetFloat());
            }
            sBuf = szTemp;
            break;
        }
        case AptVFT_Array :
        {
            AptArray *pA = this->c_array();
            pA->toString(sBuf);
            break;
        }
#if defined(APT_USE_SOUND_OBJECT)
        case AptVFT_Sound :
        {
            sBuf = "[sound]";
            break;
        }
#endif
        case AptVFT_NativeFunction :
        {
            sBuf = "[native function]";
            break;
        }
        case AptVFT_ScriptFunction1 : // DC - 11/1/04 - Gave derived Script functions their own VFT.
        case AptVFT_ScriptFunction2 :
        case AptVFT_ScriptFunctionByteCodeBlock :
        {
            sBuf = "[function]";
            break;
        }
        case AptVFT_Global :
#if APT_USE_MATH_OBJECT
        case AptVFT_Math :
#endif
        case AptVFT_Key :
        case AptVFT_Object :
#if APT_USE_SCRIPTCOLOUR_OBJECT
        case AptVFT_ScriptColour :
#endif
        case AptVFT_TextFormat :
        case AptVFT_Stage:
#if APT_USE_LOADVARS_OBJECT
        case AptVFT_LoadVars:
#endif
        case AptVFT_Mouse:
        {
            sBuf = "[object Object]";
            break;
        }
        case AptVFT_Xml:
        case AptVFT_XmlAttributes:
        case AptVFT_XmlNode:
        {
            sBuf = "[object]";
            break;
        }
        case AptVFT_Prototype :
        {
            sBuf = "[object (prototype)]";
            break;
        }
        case AptVFT_Date :
        {
            AptDate *pA = this->c_date();
            pA->toString(sBuf);
            break;
        }
        case AptVFT_MovieClip :
        {
            sBuf = "[MovieClip]";
            break;
        }
        case AptVFT_MovieClipLoader :
        {
            sBuf = "[MovieClipLoader]";
            break;
        }
        case AptVFT_Register:
        {
            sBuf = "[Register]";
            break;
        }
        case AptVFT_Lookup :
        {
            sBuf = "[Lookup]";
            break;
        }
        case AptVFT_Extern:
        {
            sBuf = "[Extern]";
            break;
        }
        case AptVFT_FrameStack :
        {
            sBuf = "[FrameStack]";
            break;
        }
        case AptVFT_Extension :
        {
            sBuf = "[Extension]";
            break;
        }
        case AptVFT_GlobalExtension :
        {
            sBuf = "[GlobalExtension]";
            break;
        }
        case AptVFT_Error: // (DC) Error Object Added for Flash Player 7 / AS 2.0 support. (Release 17.0)
        {
            AptError * pError = (AptError*)this;
            sBuf = pError->msMessage;
            break;
        }
        case AptVFT_CharacterInstHandle:
        {
            // Get NAme only works on These Types.
            AptCIH * pCIH = c_cih();
            if(pCIH->GetCharacterInst() == NULL)
                sBuf = pCIH->GetInstanceName();
            else if(pCIH->IsSpriteInstBase() || pCIH->IsButtonInst() || pCIH->IsDynamicTextInst() || pCIH->IsStaticTextInst() || pCIH->IsLevelInst() || pCIH->IsImageInst())
                AptActionInterpreter::getName(c_cih(), sBuf);
            else if(pCIH->IsShapeInst())
                sBuf = "Shape Instance";
            else
                sBuf = "No Instance Name";
            break;
        }
        case AptVFT_ExternalFunction :
        {
            sBuf = "[external function]";
            break;
        }
        default:
        {
            sprintf(szTemp, "[Type=0x%X]", getVtblIndex());
            sBuf = szTemp;
            break;
        }
    }
}

void AptValue::dumpToString(AptNativeString & sBuf)
{
    AptNativeString string_value;
    AptNativeHash *pNativeHash = GetNativeHashVirtual();
    if(pNativeHash)
    {
        AptHashItem *pHashItem = pNativeHash->GetFirstItem();
        sBuf += "{";
        while (pHashItem != NULL)
        {
            sBuf += pHashItem->Key.c_str();
            sBuf += "=";

            pHashItem->mValue->toString(string_value);
            sBuf += string_value;

            pHashItem = pNativeHash->GetNextItem(pHashItem);
            if(pHashItem != NULL)
            {
                sBuf += ",";
            }
        }
        sBuf += "}";
    }
}

static AptValue *_constructorObject(AptValue *pThis, int nParams)
{
    return gpUndefinedValue;
}

/*F************************************************************************************************/
/*!
    \Function    *_gObjRegistrationFunc

    \Description
        This is a global function that gets called when Object.registerClass is called.

    \Input      *pThis            -
    \Input      nParams           -

    \Output     AptValue          -

    \Notes
        None.

    \Version    1.0        05/26/04 (ASK) First Version
                In 0.15.05 added call to register the hash object with GC as a root so it is also tracable.

*/
/************************************************************************************************F*/
static AptValue *_gObjRegistrationFunc(AptValue *pThis, int nParams)
{
    if (nParams != 2)
        return AptBoolean::Create(false) ;

    AptValue *pParam1 = gAptActionInterpreter.stackAt(0);
    // assuming that first parameter has to be a quoted string
    if (pParam1->getIsDefined() && pParam1->isString())
    {
        AptValue *pParam2 = gAptActionInterpreter.stackAt(1);
        if (pParam1->isUndefined())
            return AptBoolean::Create(false) ;

        if (!gpObjRegistrationHash)
        {
            // first create gpObjRegistrationHash
            gpObjRegistrationHash = new AptNativeHash(APT_OBJECTHASHSIZE);
        }
        if (pParam2->isNone())
        {
            // unregister the class - so remove the entry from hash
            gpObjRegistrationHash->Unset(pParam1->c_string()->GetInternalString()) ;
        }
        else
        {
            gpObjRegistrationHash->Set(pParam1->c_string()->GetInternalString(), pParam2) ;
            pParam2->SetClassName(pParam1->c_string()->GetInternalString()->GetBuffer());
        }
        return AptBoolean::Create(true) ;
    }
    return AptBoolean::Create(false) ;
}

AptPrototype * gpObjectPrototype = NULL; // Prototype of "Object", Cached to prevent constant lookups.
AptPrototype * gpFunctionPrototype = NULL; // Prototype of "Function", Cached to prevent constant lookups.


/*F************************************************************************************************/
/*!
    \Function    _constructBuiltInObject

    \Description
        Helper for _constructBuiltInObjects

    \Input
        sc - StringPool enum for the name of the built-in object to set up.
        proto - object to set as __proto__ in the built-in object.  or NULL
                if you don't want to mess with __proto__

    \Output
        The class/constructor that was created.

*/
/************************************************************************************************F*/
AptValue * _constructBuiltInObject(StringCode sc, AptValue * proto=NULL)
{
    const AptNativeString *pName = StringPool::GetString(sc);
    AptPrototype *pPrototype = new AptPrototype();
    pPrototype->SetClassName(pName->c_str());
    pPrototype->setGCRoot(1);

    AptValue * pBuiltIn = new AptNativeFunction(_constructorObject);
    pBuiltIn->GetNativeHashVirtual()->SetPrototype(pPrototype);
    if (proto)
    {
        pBuiltIn->GetNativeHashVirtual()->Set__Proto__(proto);
    }
    pBuiltIn->setGCRoot(1);
    gpGlobalGlobalObject->Set(pName, pBuiltIn);

    return pBuiltIn;
}

/*F************************************************************************************************/
/*!
    \Function    _constructBuiltInObjects

    \Description
        Builds the native functions for all of the build in actionscript types

    \Input
        None.

    \Output
        None.

    \Notes
        None.

    \Version    1.0        07/18/05 (DC) Bringin' it to ya

*/
/************************************************************************************************F*/
static void _constructBuiltInObjects(void)
{
    //  Clean the vector for temporary construction
    gpValuesToRelease->ReleaseValues();

    // create prototype objects for all these builtin object
    // also point their __proto__ properties to Object.prototype
    // XXX - there might be need to set __proto__ property of all newly created prototypes
    // to Object.prototype.

    AptValue *pObject = _constructBuiltInObject(SC_Object);
    gpGlobalObjectPrototype = gpObjectPrototype = static_cast<AptPrototype*>(pObject->GetNativeHashVirtual()->GetPrototype());
    _constructBuiltInObject(SC_Array, gpObjectPrototype);
#if defined(APT_USE_SOUND_OBJECT)
    _constructBuiltInObject(SC_Sound, gpObjectPrototype);
#endif
#if APT_USE_SCRIPTCOLOUR_OBJECT
    _constructBuiltInObject(SC_Color, gpObjectPrototype);
#endif
    _constructBuiltInObject(SC_Date, gpObjectPrototype);
    _constructBuiltInObject(SC_MovieClipLoader, gpObjectPrototype);
    _constructBuiltInObject(SC_TextFormat, gpObjectPrototype);
    pObject = _constructBuiltInObject(SC_MovieClip, gpObjectPrototype);
    gpGlobalMovieclipPrototype = static_cast<AptPrototype*>(pObject->GetNativeHashVirtual()->GetPrototype());
    _constructBuiltInObject(SC_XML, gpObjectPrototype);
#if APT_USE_LOADVARS_OBJECT
    _constructBuiltInObject(SC_LoadVars, gpObjectPrototype);
#endif
    _constructBuiltInObject(SC_Error, gpObjectPrototype);
    _constructBuiltInObject(SC_String, gpObjectPrototype);

    AptValue * pFunction = _constructBuiltInObject(SC_Function);
    gpFunctionPrototype = static_cast<AptPrototype*>(pFunction->GetNativeHashVirtual()->GetPrototype());
    gpFunctionPrototype->GetNativeHashVirtual()->Set__Proto__(gpObjectPrototype); // Functions ARE objects.  (String and others above probably should be doing this!)
    pFunction->GetNativeHashVirtual()->Set__Proto__(gpFunctionPrototype); // in Flash, Function.__proto__ == Function.prototype  this is not true of other built-in classes like String.

    // XXX how above objects get deleted? - get deleted when gpGlobalGlobalObject gets deleted.

    // also create basic native functions of Object class.
    // this function cannot be declared with NATIVE macro because AptNativeFunction is derived from
    // AptObject and then it becomes recursive.
    // So for functions of 'Object' class define them globally and return pointer to those functions
    // from objectMemberLookup
    gpObjRegistrationFunc = new AptNativeFunction(_gObjRegistrationFunc);
    gpObjRegistrationFunc->setGCRoot(1);
    APT_INC(gpObjRegistrationFunc) ;

    //  Clean the vector for temporary construction
    gpValuesToRelease->ReleaseValues();
}

/*F************************************************************************************************/
/*!
    \Function    AptValueInitialize

    \Description
        Sets up the Apt Value global landscape.

    \Input
        None.

    \Output
        None.

    \Notes
        None.

    \Version    1.0        07/18/05 (DC) Bringin' it to ya

*/
/************************************************************************************************F*/
void AptValueInitialize(void)
{
    APT_ASSERT(AptVFT_NumVFTs <= 0x003f && "Number of VFT indexes must be expanded to fit the number of types");

    gpUndefinedValue = new AptNone();

    AptBoolean::Initialize();
    AptLookup::Initialize() ;
    AptRegister::Initialize() ;

    AptCharacterHelper::Initialize();

    gpUndefinedCIH = new AptCIHNone();
    gpUndefinedCIH->SetInstanceName( "EmptyCIH" );

    gpRenderingContext = new AptRenderingContext();

    gpExternValue = new AptExtern();

//  DEBUGdumpValueStats();
// Global Objects like Math, Stage, Mouse, Key are now reference counted

#if APT_USE_MATH_OBJECT
    gpGlobalMathObject = new AptMathObj;
    APT_INC(gpGlobalMathObject);
#endif
    gpGlobalKeyObject = new AptKey;
    APT_INC(gpGlobalKeyObject);
#if defined(APT_USE_MOUSE)
    gpGlobalMouseObject = new AptMouse;
    APT_INC(gpGlobalMouseObject);
#endif

#if APT_USE_UTILITY
    gpGlobalAptUtilObject = new AptUtil;
    APT_INC(gpGlobalAptUtilObject);
#endif

#if defined(APT_ALTERNATE_INPUT)
    gpGlobalAltInputObject = new AptAlternateInput;
    APT_INC(gpGlobalAltInputObject);
#endif

#if defined (APT_USE_STAGE_OBJECT)
    gpGlobalStageObject = new AptStage;
    APT_INC(gpGlobalStageObject);
#endif

    gpGlobalGlobalObject = new AptGlobal;
    APT_INC(gpGlobalGlobalObject);

    gpGlobalExtensionObject = new AptGlobalExtensionObject;
    APT_INC(gpGlobalExtensionObject);

     // (DC) Added for Flash Player 7 / AS 2.0 support. (Release 17.0) Added Support for Global String Object
    gpGlobalStringObject = AptString::Create();
    gpGlobalStringObject->setGCRoot(1);
    APT_INC(gpGlobalStringObject); // Prevent Deletion!

    gpObjRegistrationHash = NULL ;      // this is initialized only if object.registerclass gets called.

    _constructBuiltInObjects();

    AptColorHelperScale& vColorMul4 = gIdentityCXForm.scale;
    AptColorHelperTranslate& vColorAdd4 = gIdentityCXForm.translate;

    vColorMul4.SetValuef(AptColorHelper::Red, 255.f);
    vColorMul4.SetValuef(AptColorHelper::Green, 255.f );
    vColorMul4.SetValuef(AptColorHelper::Blue, 255.f );
    vColorMul4.SetValuef(AptColorHelper::Alpha, 255.f );
    vColorAdd4.SetValuef(AptColorHelper::Red, 0.f);
    vColorAdd4.SetValuef(AptColorHelper::Green, 0.f);
    vColorAdd4.SetValuef(AptColorHelper::Blue, 0.f);
    vColorAdd4.SetValuef(AptColorHelper::Alpha, 0.f);

    gIdentityMatrix.a = 1.f;
    gIdentityMatrix.b = 0.f;
    gIdentityMatrix.c = 0.f;
    gIdentityMatrix.d = 1.f;
    gIdentityMatrix.tx = 0.f;
    gIdentityMatrix.ty = 0.f;

    // set/clear interval
    // SetInterval, ClearInterval, isNan, unescape, escape, Boolean, AssetPropFlags, parseInt, parseFloat are now reference counted

    gpCBsetInterval = new AptNativeFunction(AptActionInterpreter::cbCallMethod_setInterval);
    gpCBsetInterval->setGCRoot(1);
    APT_INC(gpCBsetInterval);
    gpCBclearInterval = new AptNativeFunction(AptActionInterpreter::cbCallMethod_clearInterval);
    gpCBclearInterval->setGCRoot(1);
    APT_INC(gpCBclearInterval);
    gpCBisNaN = new AptNativeFunction(AptActionInterpreter::cbCallMethod_isNaN);
    gpCBisNaN->setGCRoot(1);
    APT_INC(gpCBisNaN);
    gpCBunescape = new AptNativeFunction(AptActionInterpreter::cbCallMethod_unescape);
    gpCBunescape->setGCRoot(1);
    APT_INC(gpCBunescape);
    gpCBescape = new AptNativeFunction(AptActionInterpreter::cbCallMethod_escape);
    gpCBescape->setGCRoot(1);
    APT_INC(gpCBescape);
    gpCBboolean = new AptNativeFunction(AptActionInterpreter::cbCallMethod_boolean);
    gpCBboolean->setGCRoot(1);
    APT_INC(gpCBboolean);
    // (DC) gpASSetPropFlags added for Flash Player 7 and AS 2.0 support. (Release 17.0)
    gpASSetPropFlags = new AptNativeFunction(AptActionInterpreter::cbCallMethod_ASSetPropFlags);
    gpASSetPropFlags->setGCRoot(1);
    APT_INC(gpASSetPropFlags);
    gpCBparseInt = new AptNativeFunction(AptActionInterpreter::cbCallMethod_parseInt);
    gpCBparseInt->setGCRoot(1);
    APT_INC(gpCBparseInt);
    gpCBparseFloat = new AptNativeFunction(AptActionInterpreter::cbCallMethod_parseFloat);
    gpCBparseFloat->setGCRoot(1);
    APT_INC(gpCBparseFloat);


    // from Apt-0.16.00 onwards - registers are now initialized in regStack.init()
    //  and apregisters from gAptActionInterpreter will not be used in any operation
    // we will remove them later once it stabilizes.
    memset(gAptActionInterpreter.apRegisters, 0, sizeof(gAptActionInterpreter.apRegisters));

    //  Clean the vector for temporary construction
    gpValuesToRelease->ReleaseValues();
}

/*F************************************************************************************************/
/*!
    \Function    AptValueShutdown

    \Description
        Undoes what was done in init.

        Releases all memory and sets everything up so that the next call to init will succeed.

    \Input      bQuiet            -

    \Output
        None.

    \Notes
        None.

    \Version    1.0        07/18/05 (DC) Bringin' it to ya
    \Version    1.1        05/27/08 (ASK) Added cleanup code for parseFloat, parseInt

*/
/************************************************************************************************F*/
void AptValueShutdown(int bQuiet)
{
    if (gpObjRegistrationHash)
    {
        gpObjRegistrationHash->DestroyGCPointers();
        delete gpObjRegistrationHash;
        gpObjRegistrationHash = NULL ;
    }

    gpGlobalExtensionObject->DestroyGCPointers();
    APT_DEC(gpGlobalExtensionObject);
    gpGlobalExtensionObject = NULL;
    gpGlobalGlobalObject->DestroyGCPointers();
    APT_DEC(gpGlobalGlobalObject);
    gpGlobalGlobalObject = NULL;
    // #797 DUG Extending base classes causes crash on shutdown
    // Always force delete GlobalObject first, and then destroy pointers for global Math etc object
    // This mirrors the AptValueInitialize() sequence

#if APT_USE_MATH_OBJECT
    gpGlobalMathObject->DestroyGCPointers();
    APT_DEC(gpGlobalMathObject);
    gpGlobalMathObject = NULL;
#endif
    gpGlobalKeyObject->DestroyGCPointers();
    APT_DEC(gpGlobalKeyObject);
    gpGlobalKeyObject = NULL;
#if defined(APT_USE_MOUSE)
    gpGlobalMouseObject->DestroyGCPointers();
    APT_DEC(gpGlobalMouseObject);
    gpGlobalMouseObject = NULL;
#endif

#if APT_USE_UTILITY
    gpGlobalAptUtilObject->DestroyGCPointers();
    APT_DEC(gpGlobalAptUtilObject);
    gpGlobalAptUtilObject = NULL;
#endif

#if defined(APT_ALTERNATE_INPUT)
    gpGlobalAltInputObject->DestroyGCPointers();
    APT_DEC(gpGlobalAltInputObject);
    gpGlobalAltInputObject = NULL;
#endif

#if defined (APT_USE_STAGE_OBJECT)
    gpGlobalStageObject->DestroyGCPointers();
    APT_DEC(gpGlobalStageObject);
    gpGlobalStageObject = NULL;
#endif

    gpGlobalStringObject->DestroyGCPointers();
    APT_DEC(gpGlobalStringObject);
    gpGlobalStringObject = NULL;
    delete  gpRenderingContext;


    gpCBsetInterval->DestroyGCPointers();
    APT_DEC(gpCBsetInterval);
    gpCBsetInterval = NULL;
    gpCBclearInterval->DestroyGCPointers();
    APT_DEC(gpCBclearInterval);
    gpCBclearInterval = NULL;
    gpCBisNaN->DestroyGCPointers();
    APT_DEC(gpCBisNaN);
    gpCBisNaN = NULL;
    gpCBunescape->DestroyGCPointers();
    APT_DEC(gpCBunescape);
    gpCBunescape = NULL;
    gpCBescape->DestroyGCPointers();
    APT_DEC(gpCBescape);
    gpCBescape = NULL;
    gpCBboolean->DestroyGCPointers();
    APT_DEC(gpCBboolean);
    gpCBboolean = NULL ;
    // (DC) Added for Flash Player 7 / AS 2.0 support. (Release 17.0)
    gpASSetPropFlags->DestroyGCPointers();
    APT_DEC(gpASSetPropFlags);
    gpASSetPropFlags = NULL;

    gpObjRegistrationFunc->DestroyGCPointers();
    APT_DEC(gpObjRegistrationFunc);
    gpObjRegistrationFunc = NULL;

    gpCBparseInt->DestroyGCPointers();
    APT_DEC(gpCBparseInt);
    gpCBparseInt = NULL;


    gpCBparseFloat->DestroyGCPointers();
    APT_DEC(gpCBparseFloat);
    gpCBparseFloat = NULL;
}

void AptValueShutdownRemaining()
{
    gpUndefinedValue->ForceDelete();
    AptBoolean::Shutdown();
    AptLookup::Shutdown() ;
    AptRegister::Shutdown() ;
    AptCharacterHelper::Shutdown();
    gpExternValue->ForceDelete();
}

/*F************************************************************************************************/
/*!
    \Function    AptValue::urlEncode

    \Description
    This function encodes all the properties of a given AptValue into a AptNativeString object.
    All these values are concatenated by & character
    This gets called from LoadVar object also from customControl render case for AptCIH::render.


    \Input
        None.

    \Output     AptNativeString   -

    \Notes
        None.

    \Version    1.0        07/18/05 (DC) Bringin' it to ya

*/
/************************************************************************************************F*/
AptNativeString AptValue::urlEncode()
{
    APT_PROFILE(eAptValueUrlEncode);

    AptNativeString sProperties ;
    if (!this->getIsDefined())
    {
        return sProperties ;
    }

    AptNativeHash *pObjHash = GetNativeHashVirtual();
    if (!pObjHash)
        return sProperties ;

    AptNativeString sValueBuf;

    for (AptHashItem *pInitItem = pObjHash->GetFirstItem(); pInitItem; pInitItem = pObjHash->GetNextItem(pInitItem))
    {
        // TODO: theoretically, on every hash key there's supposed to be a DontEnum/DontDelete flag; we don't have that so hardcode some dontenum's here..
        if( (pInitItem->Key.EqualNoCase(*StringPool::GetString(SC___proto__))) ||
            (pInitItem->Key.EqualNoCase(*StringPool::GetString(SC_prototype))) )
        {
            continue;
        }

        pInitItem->mValue->toString(sValueBuf) ;
        sProperties += pInitItem->Key;
        sProperties += "=" ;
        sProperties += sValueBuf ;
        sProperties += "&" ;
    }
    /*
    int nLength = sProperties.Size() ;
    if (nLength)
    {
        if (sProperties[nLength - 1] == '&')
        {
            sProperties.Erase(sProperties.End() - 1, sProperties.End());
        }
    }
    */
    sProperties.EndWithRemove("&");

    return sProperties ;
}

/*F************************************************************************************************/
/*!
    \Function    AptValue::urlEncodeCustomRender

    \Description
        URL encodes custom render properties.

    Parses the property list for the AptValue object and filters out anything that is not a custom
    render property.  These are identified by the fact they start with an '_' and are not "_type"
    and not a function

    \Input
        None.

    \Output     AptNativeString   -

    \Notes
        None.

    \Version    1.0        02/20/04 (JSH) First Version

*/
/************************************************************************************************F*/
AptNativeString AptValue::urlEncodeCustomRender()
{
    APT_PROFILE(eAptValueUrlEncodeCustomRender);

    AptNativeString sProperties ;
    if (!this->getIsDefined())
    {
        return sProperties ;
    }

    AptNativeHash *pObjHash = GetNativeHashVirtual();
    if (!pObjHash)
        return sProperties ;

//    AptNativeString sKeyBuf;
    AptNativeString sValueBuf;

    for (AptHashItem *pInitItem = pObjHash->GetFirstItem(); pInitItem; pInitItem = pObjHash->GetNextItem(pInitItem))
    {
        AptNativeString strKey = pInitItem->Key;
        if (strKey[0] == '_')
        {
            const char *    pStrBuffer = strKey.c_str() + 1;
            if (strcmp(pStrBuffer, "_proto__") == 0)
            {
                continue;
            }
            if (strcmp(pStrBuffer, "type") == 0)
            {
                continue;
            }

            AptValue *  pValue = pInitItem->mValue;
            if (!pValue->isNativeFunction() && !pValue->isExternalFunction())
            {
                pValue->toString(sValueBuf);
                sProperties += strKey;
                sProperties += "=";
                sProperties += sValueBuf;
                sProperties += "&";
            }
        }
    }
    sProperties.EndWithRemove("&");
    return sProperties ;
}

/*F************************************************************************************************/
/*!
    \Function    *AptValue::findChild

    \Description
        Looks for the string name to see if it is a global keyword.

    \Input      pName             -
    \Input      pWith             -

    \Output     AptValue          -

    \Notes
        None.

    \Version    1.0        07/18/05 (DC) Bringin' it to ya
    \Version    1.1        06/09/09 (DG) FogBug 1233: AptNativeHash change from Tiger10:
                                         in case AptObjectsuper use if pStackTop->ContainsNativeHashVirtual() instead of: AptNativeHash* pNativeHash
    \Version    1.2        10/23/09 (Colin C.) If we are looking up a member, we don't want to also look in _global namespaces.
    \Version    1.3        11/11/09 (RB) Lookup setInterval and getInterval in AptGlobal (instead of AptCIH)
                                         because they are members of _global.
    \Version    1.4        11/20/09 (RB) Do look in reserved words if we're looking for a member of _global.
                                         Note, the AptUtil, Key, Math and Stage case statements could probably be moved to
                                         AptGlobal::Lookup() in the future.

*/
/************************************************************************************************F*/
AptValue *AptValue::findChild(const AptNativeString * pName, AptValue * pWith, bool bIsMember)
{
    AptValue *pCur = pWith ? pWith : this;

    int gperfIndex = -1;

#if !defined(APT_ALLOW_CLASS_MEMBER_RESERVED_WORDS_LOOKUP)
    // Don't look in the reserved words if the variable is a member of a class.
    // But do look in reserved words if we're looking for a member of _global.
    if(!bIsMember || pCur==gpGlobalGlobalObject)
    {
        APT_PROFILE(in_word_set);

        gperfIndex = gpGlobalGlobalObject->GetGperfIndex(pName);
    }
#else
    // This code is buggy - please see below comments
    {
        APT_PROFILE(in_word_set);

        gperfIndex = gpGlobalGlobalObject->GetGperfIndex(pName);
#if (APT_CURR_DEBUG_MSG_LVL >= APT_DEBUG_MSG_ASSERT_LVL)
        if(bIsMember && pCur!=gpGlobalGlobalObject && -1 != gperfIndex)
        {
            // TODO: Fix data.  Will M. 6/21/12
            // At this point, we know we are attempting to find a member of a class
            // which is not _global and has the same name as a reserved word
            //
            // I've noticed this happening when calling functions on an object in the following format:
            //
            //      someObject[someString].someFunc();
            //
            // The cases I've seen have used "this" as <someString> with the expectation
            // that <someFunc> will be called on <someObject>, as opposed to a child of <someObject>
            // This behavior is inconsistent with Flash and is not supported by this version of Apt.
            // In cases where this behavior is required, the following format should be used:
            //
            //      if("this" == someString)
            //      {
            //          someObject.someFunc();
            //      }
            //      else
            //      {
            //          someObject[someString].someFunc();
            //      }
            //
            // A similar pattern can be used if other reserved words are being used in this way.
            // Since Fifa's version of APT has allowed this behavior for some time, it's difficult to know
            // how widespread this usage is. If this functionality is completely removed, some function calls
            // will silently fail resulting in broken flows and hard to track down bugs.
            //
            // Therefore, at the moment we are going to let Fifa use the buggy version.  In order to help
            // us find and fix the data, we compare vs. the not-buggy version and assert if we get
            // different behavior.  If we do, then there's potentially a data problem that should be fixed!

            const int ERROR_BUFFER_SIZE = 1024;
            char errorBuffer[ERROR_BUFFER_SIZE];
            SNPRINTF(errorBuffer, ERROR_BUFFER_SIZE, "Found data behavior inconsistency attempting to find child with name \"%s\", likely a reserved word - see comments in AptValue.cpp around line %d.", pName->c_str(), __LINE__);
            errorBuffer[ERROR_BUFFER_SIZE-1] = '\0';
            APT_FAIL(errorBuffer);
        }
#endif
    }
#endif

    {APT_PROFILE(find_child);
    if (gperfIndex >= 0)
    {
        switch(gperfIndex)
        {
            case AptObjectthis:
            {
                APT_PROFILE(find_child_this);
                // added hack for super.
                AptValue * pValue = gAptActionInterpreter.thisStack.top();
                // now check if pValue is actually a any of the super of 'this' if not then return pValue
                AptValue * pThis = this ;
                if (pValue == pThis)
                    return pThis ;

                /// ### LEO ADDED FOR MAXIS SUPER BUG PART DUEX!
                if(gAptActionInterpreter.withStack.size() > 0 && !gAptActionInterpreter.withStack.top()->isUndefined())
                {
                    AptValue *pWithStkTop = gAptActionInterpreter.withStack.top();
                    APT_ASSERT(pWithStkTop != NULL);
                    APT_ASSERT(pWithStkTop->ContainsNativeHashVirtual());// && pWithStkTop->GetHasClass());

                    if(pValue == pWithStkTop)
                    {
                        return pValue;
                    }

                    AptNativeHash *pNativeHash = pWithStkTop->GetNativeHashVirtual();
                        AptValue *pTempValue;

                        while(pNativeHash)
                        {
                            pTempValue = pNativeHash->Get__Proto__();
                            if(pTempValue)
                            {
                                if(pTempValue == pValue)
                                {
                                    // This case is true, then we found that pObject is in the class hierarchy of pInterpreter->withStack.top()!!!
                                return pWithStkTop;
                                }
                                pNativeHash = pTempValue->GetNativeHashVirtual();
                            }
                            else
                            {
                                break;
                            }
                        }
                }

                AptNativeHash *pNativeHash = pThis->GetNativeHashVirtual();
                while(pNativeHash)
                {
                    AptValue *pValue1 = pNativeHash->Get__Proto__();
                    if (pValue1)
                    {
                        if (pValue1 == pValue)
                            return this ;
                        else
                            pNativeHash = pValue1->GetNativeHashVirtual();
                    }
                    else
                        break ;
                }

                return gAptActionInterpreter.thisStack.top();
            }

            break;
            case AptObject_root:
            {
                APT_PROFILE(find_child_root);
                AptCIH * pParentAnim = NULL;
                if (pCur->isCIH())
                {
                    pParentAnim = pCur->c_cih();
                }
                else
                {
                    if ( gAptActionInterpreter.mpCurrentFunction &&
                         gAptActionInterpreter.mpCurrentFunction->mpParentAnim &&
                         gAptActionInterpreter.mpCurrentFunction->mpParentAnim->isCIH() )
                    {
                // ASK - BUG#751 _root not found from object based key handler 1/1
                // we were not checking if the pCur is not CIH. In that case we need to find the
                // current executing function and look for its parent animation.
                // So we get the mpCurrentFunction->pParentAnim and get its root.
                        pParentAnim = gAptActionInterpreter.mpCurrentFunction->mpParentAnim;
                    }
                }

                if ( pParentAnim )
                {
                    // repeatedly set pParentAnim to its own display list parent,
                    // until we reach the root
                        while (pParentAnim->GetDisplayListParent())
                        {
                            pParentAnim = pParentAnim->GetDisplayListParent();
                        }
                        return pParentAnim;
                    }
                return gpUndefinedValue;
                    }

            break;
            case AptObject_global:
            {
                APT_PROFILE(find_child_other);
                return gpGlobalGlobalObject;
            }

            break;
            case AptObjectKey:
            {
                // Note: this case could probably be moved into AptGlobal::Lookup().
                APT_PROFILE(find_child_other);
                return gpGlobalKeyObject;
            }
            break;
            case AptObjectMouse:
            {
                APT_PROFILE(find_child_other);
#if defined(APT_USE_MOUSE)
                return gpGlobalMouseObject;
#else
                return gpUndefinedValue;
#endif
            }
            break;
            case AptObject_AlternateInput:
            {
                // Note: this case could probably be moved into AptGlobal::Lookup().
                APT_PROFILE(find_child_other);
#if defined(APT_ALTERNATE_INPUT)
                return gpGlobalAltInputObject;
#else
                APT_ASSERT(0 && "APT-Warning-AlternateInput object cannot be used as it is compiled out, make sure to compile with APT_ALTERNATE_INPUT defined as 1") ;
#endif
            }
            break ;
            case AptObjectMath:
            {
                // Note: this case could probably be moved into AptGlobal::Lookup().
                APT_PROFILE(find_child_other);
#if APT_USE_MATH_OBJECT
                return gpGlobalMathObject;
#else
                APT_ASSERT(0 && "APT-Warning-Math object cannot be used as it is compiled out, make sure to compile with APT_USE_MATH_OBJECT defined as 1") ;
                return gpUndefinedValue ;
#endif
            }

            // (DC) Added for Flash Player 7 / AS 2.0 support. (Release 17.0) (Added a Real Global String Object)
            // DC 2004/15/04 -- Added hacked support for a global string object
            break;
            case AptObject_String:
            {
                APT_PROFILE(find_child_other);
                return gpGlobalStringObject;
            }

            break;
            case AptObject_Stage:
            {
                // Note: this case could probably be moved into AptGlobal::Lookup().
                APT_PROFILE(find_child_other);
#if defined (APT_USE_STAGE_OBJECT)
                   return gpGlobalStageObject;
#else
                APT_ASSERT(0 && "APT-Warning-Stage object cannot be used as it is compiled out, make sure to compile with APT_USE_STAGE_OBJECT defined in Aptdefine.h") ;
                return gpUndefinedValue ;
#endif
            }

            case AptObject_AptUtil:
            {
                // Note: this case could probably be moved into AptGlobal::Lookup().
                APT_PROFILE(find_child_other);
#if APT_USE_UTILITY
                return gpGlobalAptUtilObject;
#else
                APT_ASSERT(0 && "APT-Warning-AptUtil object cannot be used as it is compiled out, make sure to compile with APT_USE_UTILITY defined in Aptdefine.h") ;
                return gpUndefinedValue ;
#endif
            }

            break;
            case AptObjectextern:
            {
                APT_PROFILE(find_child_other);
                return gpExternValue;
            }

            //  super fix - needs more testing
            break;
            case AptObjectsuper:
            {
                APT_PROFILE(find_child_super);
                AptValue * pStackTop = gAptActionInterpreter.thisStack.top() ;

                if (pStackTop->ContainsNativeHashVirtual())
                {
                    AptNativeHash *pNativeHash = pStackTop->GetNativeHashVirtual();
                    AptValue *pValue = pNativeHash->Get__Proto__();
                    if (pValue)
                    {
                        if ((pStackTop != this) && !pStackTop->isCIH() && (pStackTop->isMCInParentChain()))
                        {
                            if((pStackTop->isObject() && pStackTop->c_object()->getInMainInst()))
                            {
                                pNativeHash = pValue->GetNativeHashVirtual();
                                if (pNativeHash)
                                {
                                    pValue = pNativeHash->Get__Proto__();
                                    if (pValue)
                                    {
                                        return pValue;
                                    }
                                }
                            }
                            return pValue ;
                        }
                        else if(pStackTop->isPrototype())
                        {
                            // (LEO) Super Fix:
                            return pValue;
                        }

                        // DC -- This Hacks a problem with AS 2.0 objects where this would call every other
                        // super constructor. This is here because of how the constructors run in Apt which is
                        // in and of itself hacky. In any case this Hack just turns off the hack added for AS 1.0
                        // below that gets the __proto__'s __proto__ beucase that hack is not needed in AS 2.0 objexcts.
                        // Hopefully if we encapsulate these things in Apt 19 this won't be a problem any more.

                        if(pValue->isPrototype() && pValue->c_prototype()->GetSuperConstructor())
                        {
                            // $bug849 -ASK 0.18.04 super.function() defect - 1/3
                            // fix is added so we do not skip super the constructors or functions in inheritance hierarchy
                            // or we do not call them multiple times at same hierarchy.
                            // we use withstack when there is a super hierarchy in functions we are executing
                            if ((gAptActionInterpreter.withStack.size() > 0) && (gAptActionInterpreter.withStack.top() == pStackTop))
                            {
                                if (pStackTop->isCIH())
                                {
                                    if (!pStackTop->c_cih()->IsInCtor())    // gets set thru AptCIH::AssociateInstToClass
                                    {
                                        return (pValue->GetNativeHashVirtual()->Get__Proto__()) ;
                                    }
                                }
                                else if (pStackTop->isObject())
                                {
                                    if (pStackTop->GetHasClass())
                                    {
                                        return (pValue->GetNativeHashVirtual()->Get__Proto__()) ;
                                    }
                                }
                            }
                            // end of fix for $bug849
                            return pValue ;
                        }
                        else
                        {
                            pNativeHash = pValue->GetNativeHashVirtual();
                            if (pNativeHash)
                            {
                                pValue = pNativeHash->Get__Proto__();
                                if (pValue)
                                {
                                    return pValue;
                                }
                            }
                        }

                    }
                }
                return gpUndefinedValue;
            }

            break;
            case AptObjectparent:
            {
                APT_PROFILE(find_child_parent);
                if (pCur->isCIH())
                {
                    return pCur->c_cih()->GetDisplayListParent();
                }
            }

            break;
            case AptObject_target:
            {
                APT_PROFILE(find_child_other);
                return NULL;
            }
            case AptObject_level0:
            case AptObject_level1:
            case AptObject_level2:
            case AptObject_level3:
            case AptObject_level4:
            case AptObject_level5:
            case AptObject_level6:
            case AptObject_level7:
            case AptObject_level8:
            case AptObject_level9:
            case AptObject_level10:
            case AptObject_level11:
            case AptObject_level12:
            case AptObject_level13:
            case AptObject_level14:
            case AptObject_level15:
            case AptObject_level16:
            case AptObject_level17:
            case AptObject_level18:
            case AptObject_level19:
            case AptObject_level20:
            case AptObject_level21:
            case AptObject_level22:
            case AptObject_level23:
            case AptObject_level24:
            {
                int32_t nLevel = static_cast<int32_t>(atoi(pName->c_str() + 6));

                return _AptGetAnimationAtLevel(nLevel);
            }
            break ;

            case AptGlobal::setIntervalIdx:
            case AptGlobal::clearIntervalIdx:
            {
                return gpGlobalGlobalObject->Lookup(gperfIndex);
            }

            default:
                APT_ASSERT(NOT_REACHED);
            break;
        }
        return NULL;
    }
    else
    {
        if (pCur->isUndefined())
        {
            return NULL;
        }
        else
        {
            APT_PROFILE(find_child_iterateparents);
            AptNativeHash *pNativeHash = pCur->GetNativeHashVirtual();

#if defined APT_USE_BUTTONS
            // DC - Fixed instance where we would not be able to access certain members, i.e. we needed to chain. (Relase 17.00)
            if(pNativeHash == NULL && (pCur->isCIH() && pCur->c_cih()->IsButtonInst()))
            {
                //pNativeHash = pCur->c_cih()->GetDisplayListParent()->GetNativeHash();
                pNativeHash = pCur->c_cih()->GetNativeHash();      // (LEO) buttons now have a hash
            }
#endif

            while (pNativeHash)
            {
                AptValue *pRet = pNativeHash->Lookup(pName);
                if (pRet) return pRet;
//              if (pRet && pRet != gpUndefinedValue) return pRet;          // This was updated for unknown future bug
                AptValue *pProto = pNativeHash->Get__Proto__();
                if (pProto)
                {
                    pNativeHash = pProto->GetNativeHashVirtual();
                }
                else
                {
                    break;
                }
            }

            // If this is known to be a member, abort now.  Otherwise, look in _global stuff.
            if (bIsMember)
            {
                return NULL;
            }

            AptValue *pRet = gpGlobalExtensionObject->Lookup(pName);
            // $bug1001 APT with() output does not match Flash - 2/2 - added extra check for pWith to omit global scope.
            if (pRet == NULL && this != pWith)
            {
                pRet = gpGlobalGlobalObject->Lookup(pName);
            }
            return pRet;
        }
    }}
}

/*F************************************************************************************************/
/*!
    \Function    AptValue::isMCInParentChain

    \Description
        checks if MovieClip.prototype is in hierarchy chain

    \Input
        None.

    \Output     int               -

    \Notes
        None.

    \Version    1.0        07/18/05 (DC) Bringin' it to ya
    \Version    1.01       02/13/07 (RB) Removed AptValue->Lookup(SC_MovieClip) and
                                         AptValue->Lookup(SC_Object) in favor of using global vars.
                                         This slightly improves the speed of this method, which
                                         gets called for every AS method call.
*/
/************************************************************************************************F*/
int AptValue::isMCInParentChain() const
{
    APT_PROFILE(AptValue_isMCInParentChain);
    // pcurr might not be used
    int iRet = 0 ;

    // these can be made global ready to use variables to optimize here.
    AptValue *pPrototypeMovieClip = gpGlobalMovieclipPrototype;

    AptValue *pPrototypeObject = gpGlobalObjectPrototype;

    if (this == pPrototypeMovieClip)
        return 1 ;
    AptNativeHash *pNativeHash = ((AptValue *)this)->GetNativeHashVirtual();
    while(pNativeHash)
    {
        AptValue *pValue = pNativeHash->Get__Proto__();
        if (pValue)
        {
            if (pValue == pPrototypeMovieClip)
                return 1 ;
            else if (pValue == pPrototypeObject)
                return 0 ;

            pNativeHash = pValue->GetNativeHashVirtual();
        }
        else
            break ;
    }
    return iRet ;
}

/*F************************************************************************************************/
/*!
    \Function    AptValue::CanCreateScriptObject

    \Description
        Returns true if this object is capable of being the type of a dynamic scriptObject.

    \Input
        None.

    \Output     bool              -

    \Notes
        None.

    \Version    1.0        09/09/04 (DC) Bringin' it to ya

*/
/************************************************************************************************F*/
bool AptValue::CanCreateScriptObject() const
{
    switch(getVtblIndex())
    {
    case AptVFT_Object:
    case AptVFT_NativeFunction:
    case AptVFT_ScriptFunction1:
    case AptVFT_ScriptFunction2:
    case AptVFT_StringValue:
    case AptVFT_StringObject:
#if defined(APT_USE_SOUND_OBJECT)
    case AptVFT_Sound:
#endif
    case AptVFT_Array:
    case AptVFT_Date:
    case AptVFT_TextFormat:
#if APT_USE_SCRIPTCOLOUR_OBJECT
    case AptVFT_ScriptColour:
#endif
    case AptVFT_MovieClip:
    case AptVFT_MovieClipLoader:
    case AptVFT_Xml:
#if APT_USE_LOADVARS_OBJECT
    case AptVFT_LoadVars:
#endif
    case AptVFT_ExternalFunction:
    case AptVFT_Error:
        return true;
    default:
        return false;
    }
}


/*F************************************************************************************************/
/*!
\Function    AptValue::AddRef

\Description
This increments the reference counter.

There are a variety of switches you can turn on to track certain things, look at the function body
to see what is there. Note that there are parameters if APT_INC_DEC_MESSAGES is defined, but all of
the parameters are automatically generated, so you really don't need to worry about them.

\Input
None.

\Output
None.

\Notes
None.

\Version    1.0        09/09/04 (DC) Bringin' it to ya

*/
/************************************************************************************************F*/

#if defined(APT_INC_DEC_MESSAGES)

// This is a regular expression to find the messages associated with 032551B4
// ^(~(OBJ)|(OBJ,:a+,:a+,Th~(032551B4))).#\n
// Usually I use this and replace with nothing.

static long RefCountCallCounter = 0;
AptVirtualFunctionTable_Indices gVftTrackThisType = AptVFT_NumVFTs; /* Set this to whatever data type you want to track. */
AptValue * gpTrackThisObject = NULL;                          /* Set this to the address of the object you want to track. */

void    AptValue::AddRef(const char * szFuncName, const char * szFileName, int nLineNumber)
#else

void    AptValue::AddRef()
#endif
{
// DC - Added Increment / Decrement messages. Turn them on with APT_INC_DEC_MESSAGES
#if defined(APT_INC_DEC_MESSAGES)
    AptVirtualFunctionTable_Indices mYtype = getVtblIndex();
    if(mYtype == gVftTrackThisType || this == gpTrackThisObject /*|| (this->isCIH() && (this->c_cih()->GetInstanceName() == "gMainScreen")) */)
    {
        AptNativeString temp;
        toString(temp);
        APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, "OBJ,Inc,Ck%8.8X,Th%8.8X,RCt%4.4X,%s,%s:%d\n",RefCountCallCounter++, this, getRefCount(), temp.c_str() , szFuncName, nLineNumber);
    }
#endif

#ifdef CHECK_REFCOUNT
    int32_t i;
    for(i = 0 ; i < sizeof(mValuesToTest) / sizeof(mValuesToTest[0]) ; ++i)
    {
        if (this == mValuesToTest[i])
        {
            break;  //  You can put a breakpoint here in debug
        }
    }
#endif
    IncrementAptValue();
    //setRefCount(getRefCount() + 1);
}

/*F************************************************************************************************/
/*!
    \Function    AptValue::IncrementAptValue

    \Description
        Takes care of atomically incrementing an apt value

    \Input
        None.

    \Output     int32_t           -

    \Notes
        None.

    \Version    1.0        07/18/05 (DC) Bringin' it to ya

*/
/************************************************************************************************F*/
int32_t AptValue::IncrementAptValue()
{
    uint32_t n = getRefCount() + 1;
    if(n > MAX_REFCOUNT)
    {
        //APT_ASSERT(false); // Assert on Debug Builds. This is typically a refcount problem. we shouldn't be hitting this in normal execution.
        SetMaxRefCountHit(true);        // (LEO) -$bug817: gpObjectPrototype deleted early due to MAX_REFCOUNT is reached.
        n = MAX_REFCOUNT;
    }
    mValueBitfield.mnReferenceCount = n;
    return n;
}


/*F************************************************************************************************/
/*!
    \Function    AptValue::DecrementAptValue

    \Description
        Takes care of atomically incrementing an apt value

    \Input
        None.

    \Output     int32_t           -

    \Notes
        None.

    \Version    1.0        07/18/05 (DC) Bringin' it to ya

*/
/************************************************************************************************F*/
int32_t AptValue::DecrementAptValue()
{
    uint32_t n = getRefCount() - 1;
    mValueBitfield.mnReferenceCount = n;
    return n;
}

/*F************************************************************************************************/
/*!
\Function    AptValue::Release

\Description
This decrements the ref count on the AptValue.

If it reaches zero then depending on current capacity of deferred vector
it is either deleted right away or pushed in to deferred vector. If sbSuspendRefcountDeletions is
true it won't actually delete any garbage collected objects (this is used mainly by the garbage
collector.

\Input
None.

\Output
None.

\Notes
None.

\Version    1.0        09/09/04 (DC) Bringin' it to ya
            1.1        02/07/05 (LEO) -$bug817: gpObjectPrototype deleted early due to MAX_REFCOUNT is reached.
            1.2        09/05/07 (RB) $senzee: Avoid assert by not releasing if GetMaxRefCountHit().
*/
/************************************************************************************************F*/
//int gnAptValuesReleasedFromRenderThread = 0 ;
#if defined(APT_INC_DEC_MESSAGES)
    void    AptValue::Release(const char * szFuncName, const char * szFileName, int nLineNumber)
#else
    void    AptValue::Release()
#endif
{
    if (GetMaxRefCountHit()) // $senzee, don't try to release if the max ref count was hit, we just don't know how many references there were
        return;

    int32_t nRefCount = DecrementAptValue();

// DC - Added Increment / Decrement messages. Turn them on with APT_INC_DEC_MESSAGES
#if defined(APT_INC_DEC_MESSAGES)
    AptVirtualFunctionTable_Indices mYtype = getVtblIndex();
    if(mYtype == gVftTrackThisType || this == gpTrackThisObject /*|| (this->isCIH() && (this->c_cih()->GetInstanceName() == "gMainScreen"))*/)
    {
        AptNativeString temp;
        toString(temp);
        APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, "OBJ,Dec,Ck%8.8X,Th%8.8X,RCt%4.4X,%s,%s:%d\n",RefCountCallCounter++, this, nRefCount + 1, temp.c_str() , szFuncName, nLineNumber);
    }
#endif

#ifdef CHECK_REFCOUNT
    int32_t i;
    for(i = 0 ; i < sizeof(mValuesToTest) / sizeof(mValuesToTest[0]) ; ++i)
    {
        if (this == mValuesToTest[i])
        {
            break;  //  You can put a breakpoint here in debug
        }
    }
#endif

    APT_ASSERT(nRefCount + 1 > 0);

    //BTurkelson 10/20/2008 - Restructured this because _ForceDeleteIfNecessary is the minority case.
    //Moved it to it's own helper function because it was causing a pretty major load-hit-store always.
    if(nRefCount==0)
    {
        _ForceDeleteIfNecessary();
    }
}

/*F************************************************************************************************/
/*!
\Function    AptValue::_ForceDeleteIfNecessary

\Description
I moved some of the functionality of AptValue::Release() that doesn't usually get hit into this
helper function, which allows the compiler to avoid a Load-Hit-Store on the Link Register in the
majority of cases... Which itself is caused by virtual functions IsGarBageCollected() and ForceDelete()

\Input
None.

\Output
None.

\Notes
None.

\Version    1.0        09/09/04 (BT) Yay Optimization!

*/
/************************************************************************************************F*/
void AptValue::_ForceDeleteIfNecessary()
    {
    if (sbSuspendRefcountDeletions==false || IsGarbageCollected() == false)
        {
        if((!GetMaxRefCountHit()) && GetAllowDelayedDeletion())
        {
            //  There is a chance we want to delete this object at the end of the frame
            if (IsReleaseAtEnd() == false)
            {
                // here we check first if the vector is full otherwise just delete it right away.
                if (!gpValuesToRelease->IsVectorFull())
                {
                    //  Flag not already set, we set the flag
                    //  So we will try to delete this instane at the end of the frame
                    SetReleaseAtEnd();
                    gpValuesToRelease->PushValue(this);
                }
                else
                {
                    this->ForceDelete();
                }
            }
        }
        else
        {
            ForceDelete();
        }
    }
}


/*F************************************************************************************************/
/*!
\Function    AptValueWithHash::GetNativeHashVirtual

\Description
    Returns the native hash of this object

\Input
None.

\Output
AptNativeHash *

\Notes
None.

\Version    1.0        09/10/04 (DC) First Version

*/
/************************************************************************************************F*/
AptNativeHash * AptValueWithHash::GetNativeHashVirtual()
{
    return  (&mNativeHash);
}


/*F************************************************************************************************/
/*!
\Function    AptValueWithHash::ContainsNativeHashVirtual

\Description
Returns the native hash of this object

\Input
None.

\Output
bool  -- true because we have one.

\Notes
None.

\Version    1.0        09/10/04 (DC) First Version

*/
/************************************************************************************************F*/
bool            AptValueWithHash::ContainsNativeHashVirtual() const
{
    return  (true);
}

/*F************************************************************************************************/
/*!
\Function    AptValueWithHash::RegisterReferences

\Description
This calls our parent version and then tells the native hash to register it's.

\Input
None.

\Output
None.

\Notes
None.

\Version    1.0        09/10/04 (DC) First Version

*/
/************************************************************************************************F*/
void            AptValueWithHash::RegisterReferences()
{
    if( APT_REFERENCES_REGISTERED(this) )
        return;

    // Right now This function is not defined up the inheritance tree.
    // So we aren't calling it. if at some point in time the function is implementes at the
    // base class level we need to uncomment this line.
    // AptValueGC::RegisterReferences();

    mNativeHash.RegisterReferences(this);
    return;
}


/*F************************************************************************************************/
/*!
\Function    AptValueWithHash::DestroyGCPointers

\Description
This calls our parent version and then tells the native hash to destroy it's.

\Input
None.

\Output
None.

\Notes
None.

\Version    1.0        09/10/04 (DC) First Version

*/
/************************************************************************************************F*/
void            AptValueWithHash::DestroyGCPointers()
{
    // Go up the Chain.
    AptValueGC::DestroyGCPointers();

    // Now now what we need to do.
    mNativeHash.DestroyGCPointers();
}

/*F************************************************************************************************/
/*!
    \Function    AptExtern::objectMemberSet

    \Description
        Insert function's purpose here

    Insert your long description here.

    \Input      pContext          -
    \Input      pName             -
    \Input      pValue            -

    \Output     bool              -

    \Notes
        None.

    \Version    1.0        07/21/05 (DC) Bringin' it to ya

*/
/************************************************************************************************F*/
bool        AptExtern::objectMemberSet(AptValue * const pContext, const AptNativeString * const pName, AptValue * const pValue)
{
    AptNativeString sBuf;
    pValue->toString(sBuf);
    APT_PROFILE_AUXILIARY(pfnSetExternVariable);
    gAptFuncs.pfnSetExternVariable(pName->c_str(), sBuf);
    return  (true);
}

// $senzee helper functions for more efficient string handling
const AptNativeString *AptValue::Get_ToString(AptNativeString &sPotentialBuffer)
{
    APT_ASSERT(this);
    const AptNativeString *pStr = &sPotentialBuffer;
    if (isString())
        pStr = c_string()->GetInternalString();
    else
        toString(sPotentialBuffer);
    return pStr;
}

// $senzee helper functions for more efficient string handling
void AptValue::Append_ToString(AptNativeString &sString) const
{
    APT_ASSERT(this);
    if (isString())
        sString += *(c_string()->GetInternalString());
    else if (sString.IsEmpty())
        toString(sString);
    else
    {
        AptNativeString sAppend;
        toString(sAppend);
        sString += sAppend;
    }
}
// FIFA_BEGIN | SENDBACK | mwebb | 2015 - 05 - 26 | Fix linker error for xb1
//#if defined(EA_COMPILER_CLANG)
#if defined(EA_COMPILER_CLANG) || defined(_MSC_VER)
// FIFA_END
namespace AVOIDLINKERROR
{
	typedef AptInteger*(AptValue::* P1)()const;
	P1 p1 = &AptValue::c_integer;

	typedef AptFloat*(AptValue::* P2)()const;
	P2 p2 = &AptValue::c_float;

	typedef AptBoolean*(AptValue::* P3)()const;
	P3 p3 = &AptValue::c_boolean;

	typedef AptObject*(AptValue::* P4)()const;
	P4 p4 = &AptValue::c_object;

	typedef bool (AptBoolean::* P5)()const;
	P5 p5 = &AptBoolean::GetBool;

}
#endif
/*H*************************************************************************************************
    EOF
*************************************************************************************************H*/

