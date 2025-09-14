/*H************************************************************************************************/
/*!

\File
                     AptUtil.cpp

\Description
                     This provides common utilities functions previously defined in ActionScript (by ION/Central UI).
                     Moved to C++ to improve ActionScript execution performance.

\Notes
                     None.

\Copyright
                     (c) 2008 Electronic Arts Inc.

\Version    1.0        07/03/08 ($senzee) First Version
            1.1        09/18/08 ($senzee) fix corresponding to EATech #116310 fixed typo of isInteger() to toInteger()
            1.2        10/01/08 (ASK) Fixed other bugs in Utils functions and added getAptVersion
            1.3        01/06/09 (JP) Fixing an error in SubMinor field (and adding patch property) for AptGetVersion #116889
            1.4        01/29/09 (RB) Fixed bug in replaceString to allow 3 or 4 arguments (ignoreCase flag is optional).
                                     Fixed bug to allow reverseSearchArray to work.
                                     Refactored common code from searchArray/reverseSearchArray into GetSearchArrayParams.
                                     Added bounds-checking to formatNumberString.
                                     Ensured C++ implementations return same error values as their Cafe counterparts.
                                     Added comments.
            1.5        09/29/11 (CB) Added colorMatrixMultiply which mults 4x5 arrays ~98% faster than the AS side.
                                     This makes animating the ColorMatrix ~25% faster overall.
                                     Apt 3.00.00 Original CL #758613
*/
/************************************************************************************************H*/

#include "_Apt.h"
#include "AptUtilMembers.h"
#include "AptObject/AptUtil.h"
#include "MainInline.h"
#include "AptValueHelper.h"
#include "AptValueFactory.h"

// helper for warnings about sending too many args
#if APT_DEBUG_LEVEL >= APT_DEBUG_MSG_NORMAL_LVL
#   define ASSERT_IF_TOO_MANY_DEFINED_ARGS(maximumUsed, numArgsSent, message) \
    for (int i=maximumUsed; i<numArgsSent; ++i) \
        { \
        if (!gAptActionInterpreter.stackAt(i)->isUndefined()) \
            { \
            APT_ASSERTM(false, message); \
            break; \
            } \
        }
#else
#   define ASSERT_IF_TOO_MANY_DEFINED_ARGS(maximumUsed, numArgsSent, message)
#endif


#if APT_USE_UTILITY
enum {
    AptUtil_formatNumberString = 1,
    AptUtil_replaceString,
    AptUtil_trimString,
    AptUtil_trimLeftString,
    AptUtil_trimRightString,
    AptUtil_searchArray,
    AptUtil_reverseSearchArray,
    AptUtil_countArray,
    AptUtil_getAptVersion,
    AptUtil_safeForIn,
    AptUtil_convertHsvToColorTransform,
    AptUtil_colorMatrixMultiply,
    AptUtil_profileBlockStart,
    AptUtil_profileBlockEnd,
};
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
void AptUtil::CleanNativeFunctions()
{
#if APT_USE_UTILITY
    NATIVE_MEMBER_FUNCTION_DESTROY(formatNumberString);
    NATIVE_MEMBER_FUNCTION_DESTROY(replaceString);
    NATIVE_MEMBER_FUNCTION_DESTROY(trimString);
    NATIVE_MEMBER_FUNCTION_DESTROY(trimLeftString);
    NATIVE_MEMBER_FUNCTION_DESTROY(trimRightString);
    NATIVE_MEMBER_FUNCTION_DESTROY(searchArray);
    NATIVE_MEMBER_FUNCTION_DESTROY(reverseSearchArray);
    NATIVE_MEMBER_FUNCTION_DESTROY(countArray);
    NATIVE_MEMBER_FUNCTION_DESTROY(getAptVersion);
    NATIVE_MEMBER_FUNCTION_DESTROY(safeForIn);
    NATIVE_MEMBER_FUNCTION_DESTROY(convertHsvToColorTransform);
    NATIVE_MEMBER_FUNCTION_DESTROY(colorMatrixMultiply);
#if defined(APT_DO_AS_PROFILE)
    NATIVE_MEMBER_FUNCTION_DESTROY(profileBlockStart);
    NATIVE_MEMBER_FUNCTION_DESTROY(profileBlockEnd);
#endif
#endif
}

#if APT_USE_UTILITY
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
        1.0        07/07/08 ($senzee) First Version

*/
/************************************************************************************************F*/
AptValue *AptUtil::objectMemberLookup(AptValue * const pContext, const AptNativeString * const pName) const
{
    UtilMembers* pProp = pContext ? UtilMembersIndex::in_word_set(pName->c_str(), pName->Size()) : NULL;
    if (pProp)
    {
        switch(pProp->nIndex)
        {
            case AptUtil_formatNumberString:
                NATIVE_MEMBER_FUNCTION_DISPATCH(formatNumberString);
                break;
            case AptUtil_replaceString:
                NATIVE_MEMBER_FUNCTION_DISPATCH(replaceString);
                break;
            case AptUtil_trimString:
                NATIVE_MEMBER_FUNCTION_DISPATCH(trimString);
                break;
            case AptUtil_trimLeftString:
                NATIVE_MEMBER_FUNCTION_DISPATCH(trimLeftString);
                break;
            case AptUtil_trimRightString:
                NATIVE_MEMBER_FUNCTION_DISPATCH(trimRightString);
                break;
            case AptUtil_searchArray:
                NATIVE_MEMBER_FUNCTION_DISPATCH(searchArray);
                break;
            case AptUtil_reverseSearchArray:
                NATIVE_MEMBER_FUNCTION_DISPATCH(reverseSearchArray);
                break;
            case AptUtil_countArray:
                NATIVE_MEMBER_FUNCTION_DISPATCH(countArray);
                break;
            case AptUtil_getAptVersion:
                NATIVE_MEMBER_FUNCTION_DISPATCH(getAptVersion);
                break;
            case AptUtil_safeForIn:
                NATIVE_MEMBER_FUNCTION_DISPATCH(safeForIn);
                break;
            case AptUtil_convertHsvToColorTransform:
                NATIVE_MEMBER_FUNCTION_DISPATCH(convertHsvToColorTransform);
                break;
            case AptUtil_colorMatrixMultiply:
                NATIVE_MEMBER_FUNCTION_DISPATCH(colorMatrixMultiply);
                break;
#if defined(APT_DO_AS_PROFILE)
            case AptUtil_profileBlockStart:
                NATIVE_MEMBER_FUNCTION_DISPATCH(profileBlockStart);
                break;
            case AptUtil_profileBlockEnd:
                NATIVE_MEMBER_FUNCTION_DISPATCH(profileBlockEnd);
                break;
#endif
        }
    }
    return 0;
}

/*F************************************************************************************************/
/*!
    Helper function for the formatNumberString AptUtil function

    \Input      number - value for which we want a pretty string
                decimals - how many place you would like to appear after the decimal point
                minimumCommaSize - if you have at least this many digits to the left of the decimal
                  point, it separates out the blocks of three into commas.
                  (1000000.2 would become 1,000,000.2)

    \Output     None. text gets set to a pretty string version of your number

*/
/************************************************************************************************F*/
static void formatNumber(float number, int32_t decimals, int32_t minimumCommaSize, char8_t *text)
{
    enum { MAX_STR_SIZE = 512, MAX_DECIMALS = 32 };

    char8_t *periodOrEnd = text;

    if (decimals <= 0)
    {
        int32_t chars = sprintf(text, "%d", (int32_t)number);
        periodOrEnd = text + chars;
    }
    else
    {
        APT_ASSERT(decimals <= MAX_DECIMALS);

        if (decimals > MAX_DECIMALS)
        {
            decimals = MAX_DECIMALS;
        }
        int32_t chars = sprintf(text, "%.*f", decimals, number);
        periodOrEnd = text + chars - decimals - 1; // -1 for the period
    }

    int32_t size       = periodOrEnd - text,
    prefixSize = 0;

    const char8_t *p = text;
    while (*p && !isdigit(*p))
    {
        prefixSize++;
        p++;
    }

    if (minimumCommaSize >= 3 && (size - prefixSize) > minimumCommaSize)
    {
        char8_t tmp[MAX_STR_SIZE];
        const char8_t *p = text;
        char8_t       *q = tmp;
        while (!isdigit(*p)) // copy minus sign, etc. coming before the number
        {
            *q++ = *p++;
        }
        *q++ = *p++; // copy first digit
        for (; p < periodOrEnd; p++, q++)
        {
            if ((periodOrEnd - p) % 3 == 0)
            {
                *q++ = ',';
            }
            *q = *p;
        }
        strcpy(q, p);
        strcpy(text, tmp);
    }
}

/*F************************************************************************************************/
/*!
    Implementation of the ActionScript function AptUtil.trimString/trimLeftString/trimRightString

    \Input      type            - can be LEFT, RIGHT or BOTH
                paramCount      - number of params passed to ActionScript function
                stack param 0   - the string to trim
                stack param 1   - Optional; string containing the chars to remove

    \Output     The given string (passed from ActionScript as stack param 0) without the trimmed chars.
                Returns undefined if string not given.

*/
/************************************************************************************************F*/
AptValue *AptUtil::trim(Trim type, int32_t paramCount)
{
    bool             failed     = true;
    AptNativeString  *text       = 0;
    const char8_t    *characters = " \t\r\n\f";

    // if there are too many (defined) args sent, you are using it wrong and
    // maybe we should warn you about that
    ASSERT_IF_TOO_MANY_DEFINED_ARGS(2, paramCount, "Sent too many args to AptUtil.trim*()! It expects (text:String, characters:String=' \t\r\n\f'):String");

    if (paramCount >= 1)
    {
        AptValue *pText  = gAptActionInterpreter.stackAt(0),
                 *pChars = paramCount >= 2 ? gAptActionInterpreter.stackAt(1) : 0;

        if (pText->isString())
        {
            if ((text = static_cast<AptString *>(pText)->GetInternalString()) != 0)
            {
                failed = false;
            }
            AptNativeString *chars = pChars ? static_cast<AptString *>(pChars)->GetInternalString() : 0;
            if (chars)
            {
                characters = chars->GetBuffer();
            }
        }
    }

    if (failed)
    {
        APT_ASSERTM(false, "AptUtil.trim*() expects (text:String, characters:String=' \t\r\n\f'):String");

        return gpUndefinedValue;
    }

    AptString *newString = AptString::Create();
    newString->cpy(text);

    switch (type)
    {
        case LEFT:  newString->GetInternalString()->TrimLeft(characters);  break;
        case RIGHT: newString->GetInternalString()->TrimRight(characters); break;
        default:    newString->GetInternalString()->Trim(characters);      break;
    }

    return newString;
}


namespace {

    /*F************************************************************************************************/
    /*!
        Refactors some common functionality from searchArray() and countArray() below.
        Converts the params passed to the ActionScript function into AptValues.

        \input      stack param 0: Array to search.
        \input      stack param 1: Index to start searching at.
        \input      stack param 2: Value to look for.
        \input      stack param 3: Optional; indicates a property to search for, assuming array contains Objects.

        \output     ppArray:       Array to search, as AptValue.
        \output     ppEquals:      Value to look for, as AptValue.
        \output     ppProperty:    Property to search for; NULL if not applicable.

        \Returns    Index to start searching at.

        \Version
            1.1        01/29/09 (RB)  Added comment block
    */
    /************************************************************************************************F*/
    static int32_t GetArraySearchParams(
        int32_t nParams,
        AptArray** ppArray,
        AptValue** ppEquals,
        AptValue** ppProperty
        )
    {
        bool             failed     = true;
        AptArray        *array      = 0;
        AptValue        *equals     = 0,
                        *property   = 0;
        int32_t         start       = 0;

        // if there are too many (defined) args sent, you are using it wrong and
        // maybe we should warn you about that
        ASSERT_IF_TOO_MANY_DEFINED_ARGS(4, nParams, "Sent too many args to AptUtil.countArray/searchArray()! It expects (array:Array, start:Number, equals:Object, property:Object=undefined):Number");

        if (nParams >= 3)
        {
            AptValue *pArray      = gAptActionInterpreter.stackAt(0),
                     *pStart      = gAptActionInterpreter.stackAt(1);
            equals                = gAptActionInterpreter.stackAt(2),
            property              = nParams > 3 ? gAptActionInterpreter.stackAt(3) : 0;

            if (equals && pArray && pArray->isArray() && pStart && pStart->isNumber())
            {
                failed = false;
                array = static_cast<AptArray *>(pArray);
                start = AptValueHelper::NumberToInteger(pStart);

                // clamp start number between 0 and array length
                int32_t arrLength = pArray->c_array()->length();
                if (start < 0 || start >= arrLength)
                {
                    // Don't assert on startIndex 0 even when it is technically outside the valid range.
                    // I think we need it to be okay to search an empty array - Mike M.
                    if ( 0 != start)
                    {
                        APT_ASSERTM( false, "AptUtil.countArray/searchArray() expects start position between 0 and array length");
                    }

                    if (start >= arrLength)
                    {
                        start = arrLength-1;
                    }
                    else
                    {
                        start = 0;
                    }
                }
            }
        }

        if (failed)
        {
            APT_ASSERTM(false, "AptUtil.countArray/searchArray() expects (array:Array, start:Number, equals:Object, property:Object=undefined):Number");

            return -1;
        }

        *ppArray = array;
        *ppEquals = equals;
        *ppProperty = property;
        return start;
    }
}

/*F************************************************************************************************/
/*!
    Implementation of AptUtil.searchArray/reverseSearchArray.
    Searches for a value in an array. Can also search for a property of an Object, assuming array contains Objects.

    \input      reverse:       True if and only if we should search backwards instead of forwards.
    \input      paramCount:    Number of parameters passed to the ActionScript function on the stack.
    \input      stack param 0: Array to search.
    \input      stack param 1: Index to start searching at.
    \input      stack param 2: Value to look for.
    \input      stack param 3: Optional; indicates a property to search for, assuming array contains Objects.

    \Returns    Index of the matching item, or -1 if not found.

    \Version
        1.1        01/29/09 (RB)  Added comment block
*/
/************************************************************************************************F*/
AptValue *AptUtil::search(bool reverse, int32_t paramCount)
{
    AptArray        *array      = 0;
    AptValue        *equals     = 0,
                    *property   = 0;

    int32_t start = GetArraySearchParams( paramCount, &array, &equals, &property );

    return AptInteger::Create( start < 0 ? -1 : array->find(AptArray::objectFindComparator, start, equals, property, reverse) );
}


/*F************************************************************************************************/
/*!
    Formats a number to a specific number of decimal places, leaving integers alone, and truncating floats.

    \input      stack param 0: The number to be formatted.
    \input      stack param 1: The number of decimal places.
    \input      stack param 2: Optional. If true, inserts comma into thousands place.

    \Returns    Formatted string

    \Version
        1.1        01/29/09 (RB)  Added comment block
*/
/************************************************************************************************F*/
NATIVE_MEMBER_FUNCTION(AptUtil, formatNumberString)
{
    bool     failed    = false;
    float    number    = 0.f;
    int32_t  decimals  = 0;
    bool     commas    = false;

#if (APT_CURR_DEBUG_MSG_LVL >= APT_DEBUG_MSG_ASSERT_LVL) // we get an init'ed-but-not-referenced warning if this is defined but the asserts won't be used
    const  char * BAD_ARGS_MSG = "AptUtil.formatNumberString() expects (number:Number, decimals:Number) or (number:Number, decimals:Number, commas:Boolean)";
#endif

    // if there are too many (defined) args sent, you are using it wrong and
    // maybe we should warn you about that
    ASSERT_IF_TOO_MANY_DEFINED_ARGS(3, nParams, "Sent too many args to AptUtil.formatNumberString()! It expects (number:Number, decimals:Number) or (number:Number, decimals:Number, commas:Boolean)");

    if ( 2 <= nParams )
    {
        AptValue *pNumber   = gAptActionInterpreter.stackAt(0);
        AptValue *pDecimals = gAptActionInterpreter.stackAt(1);

        if (pNumber->isNumber() && pDecimals->isNumber())
        {
            number   = AptValueHelper::NumberToFloat(pNumber);
            decimals = AptValueHelper::NumberToInteger(pDecimals);

            if ( (number > (float)0x7FFFFFFF) || (number < -(float)0x7FFFFFFF) )
            {
                APT_ASSERTM( false, ("AptUtil.formatNumberString(): number outside of valid range ([>= -2^31 and < 2^31])"));
                failed = true;
            }
            if (decimals < 0)
            {
                APT_ASSERTM( false, ("AptUtil.formatNumberString(): number of decimal places should be non-negative"));
                failed = true;
            }
        }
        else
        {
            APT_ASSERTM( false, (BAD_ARGS_MSG));
            failed = true;
        }

        if ( (!failed) && (3 <= nParams) )
        {
            AptValue *pCommas   = gAptActionInterpreter.stackAt(2);
            if (pCommas->isBoolean())
            {
                commas   = pCommas->toBool();
            }
            else if (pCommas->isUndefined())
            {
                // nothing. treat undefined the same as if it was not provided at all..
            }
            else
            {
                APT_ASSERTM( false, (BAD_ARGS_MSG));
                failed = true;
            }
        }
    }
    else
    {
        APT_ASSERTM( false, (BAD_ARGS_MSG));
        failed = true;
    }

    if (failed)
    {
        return gpUndefinedValue;
    }

    enum { MAX_STR_SIZE = 512 };

    char8_t text[MAX_STR_SIZE];

    formatNumber(number, decimals, commas ? 3 : -1, text);

    return AptString::Create(text);
}


/*F************************************************************************************************/
/*!
    Find and replace all instances of strFind with strReplace, return the new string.

    \input      strText         The original text.
    \input      strFind         The text to be replaced.
    \input      streReplace     The text to replace the found string with.
    \input      ignoreCase      Optional. If true, ignore case. Defaults to false.

    \Returns    New string

    \Version
        1.1        01/29/09 (RB)  Added comment block
*/
/************************************************************************************************F*/
NATIVE_MEMBER_FUNCTION(AptUtil, replaceString)
{
    bool             failed     = true;
    AptNativeString *text       = 0;
    const char8_t   *find       = 0,
                    *replace    = 0;
    bool             ignoreCase = false;

    // if there are too many (defined) args sent, you are using it wrong and
    // maybe we should warn you about that
    ASSERT_IF_TOO_MANY_DEFINED_ARGS(4, nParams, "Sent too many args to AptUtil.replaceString()! It expects (text:String, find:String, replace:String, ignoreCase:Boolean=false):String");

    if (nParams >= 3)
    {
        AptValue *pText       = gAptActionInterpreter.stackAt(0),
                 *pFind       = gAptActionInterpreter.stackAt(1),
                 *pReplace    = gAptActionInterpreter.stackAt(2);

        if (pText->isString() && pFind->isString() && pReplace->isString() )
        {
            text       = static_cast<AptString *>(pText)->GetInternalString();
            find       = static_cast<AptString *>(pFind)->GetInternalString()->GetBuffer();
            replace    = static_cast<AptString *>(pReplace)->GetInternalString()->GetBuffer();

            if (text && find && replace)
            {
                if ( nParams >= 4 )
                {
                    AptValue* pIgnoreCase = gAptActionInterpreter.stackAt(3);
                    if ( pIgnoreCase->isBoolean() )
                    {
                        ignoreCase = pIgnoreCase->toBool();
                        failed = false;
                    }
                }
                else
                {
                    failed = false;
                }
            }
        }
    }

    APT_ASSERTM((!failed), ("AptUtil.replaceString() expects (text:String, find:String, replace:String, ignoreCase:Boolean=false):String"));

    if (failed)
    {
        return gpUndefinedValue;
    }

    AptString *newString = AptString::Create();
    newString->cpy(text);
    newString->GetInternalString()->Replace(find, replace, ignoreCase);

    return newString;
}


/*F************************************************************************************************/
/*!
    C++ wrapper for the ActionScript function AptUtil.trimString.
    Trims from both ends of the given string.

    \Input      stack param 0   - the string to trim
                stack param 1   - Optional; string containing the chars to remove

    \Output     The given string without the trimmed chars.
                Returns undefined if string not given.

*/
/************************************************************************************************F*/
NATIVE_MEMBER_FUNCTION(AptUtil, trimString)
{
    return trim(BOTH, nParams);
}


/*F************************************************************************************************/
/*!
    C++ wrapper for the ActionScript function AptUtil.trimLeftString.
    Trims only from the left side of the given string.

    \Input      stack param 0   - the string to trim
                stack param 1   - Optional; string containing the chars to remove

    \Output     The given string without the trimmed chars.
                Returns undefined if string not given.

*/
/************************************************************************************************F*/
NATIVE_MEMBER_FUNCTION(AptUtil, trimLeftString)
{
    return trim(LEFT, nParams);
}


/*F************************************************************************************************/
/*!
    C++ wrapper for the ActionScript function AptUtil.trimRightString.
    Trims only from the right side of the given string.

    \Input      stack param 0   - the string to trim
                stack param 1   - Optional; string containing the chars to remove

    \Output     The given string without the trimmed chars.
                Returns undefined if string not given.

*/
/************************************************************************************************F*/
NATIVE_MEMBER_FUNCTION(AptUtil, trimRightString)
{
    return trim(RIGHT, nParams);
}

/*F************************************************************************************************/
/*!
    C++ wrapper for the ActionScript function AptUtil.searchArray
    Searches for a value in an array. Can also search for a property of an Object, assuming array contains Objects.

    \input      stack param 0: Array to search.
    \input      stack param 1: Index to start searching at.
    \input      stack param 2: Value to look for.
    \input      stack param 3: Optional; indicates a property to search for, assuming array contains Objects.

    \Returns    Index of the matching item, or -1 if not found (returned as AptValue).
                Also returns -1 if user does not actually pass in an array.

    \Version
        1.1        01/29/09 (RB)  Added comment block
*/
/************************************************************************************************F*/
NATIVE_MEMBER_FUNCTION(AptUtil, searchArray)
{
    return search(false, nParams);
}


/*F************************************************************************************************/
/*!
    C++ wrapper for the ActionScript function AptUtil.reverseSearchArray
    Searches backwards for a value in an array. Can also search for a property of an Object, assuming array contains Objects.

    \input      stack param 0: Array to search.
    \input      stack param 1: Index to start searching at.
    \input      stack param 2: Value to look for.
    \input      stack param 3: Optional; indicates a property to search for, assuming array contains Objects.

    \Returns    Index of the matching item, or -1 if not found (returned as AptValue).
                Also returns -1 if user does not actually pass in an array.

    \Version
        1.1        01/29/09 (RB)  Added comment block
*/
/************************************************************************************************F*/
NATIVE_MEMBER_FUNCTION(AptUtil, reverseSearchArray)
{
    return search(true, nParams);
}


/*F************************************************************************************************/
/*!
    C++ wrapper for the ActionScript function AptUtil.countArray
    Counts the number of times a value occurs in an array. Can also search for a property of an Object,
    assuming array contains Objects.

    \input      stack param 0: Array to search.
    \input      stack param 1: Index to start searching at.
    \input      stack param 2: Value to look for.
    \input      stack param 3: Optional; indicates a property to search for, assuming array contains Objects.

    \Returns    Number of matching item, or 0 if not found (returned as AptValue).
                Also returns 0 if user does not actually pass in an array.

    \Version
        1.1        01/29/09 (RB)  Added comment block
*/
/************************************************************************************************F*/
NATIVE_MEMBER_FUNCTION(AptUtil, countArray)
{
    AptArray        *array      = 0;
    AptValue        *equals     = 0,
                    *property   = 0;

    int32_t count = 0;
    int32_t start = GetArraySearchParams( nParams, &array, &equals, &property );
    if ( start >= 0 )
    {
        start--;
        while ((start = array->find(AptArray::objectFindComparator, start+1, equals, property, false)) != -1)
        {
            count++;
        }

    }
    return AptInteger::Create(count);
}


namespace {
    static inline int32_t ExtractDecimal(int32_t iValue, int32_t iShifts)
    {
        iValue = (iValue >> iShifts) & 0xff;   // Return iValue directly if 2.10 is going to be represented as 0x20a0000
        return (iValue & 0xf) + (iValue >> 4) * 10; // Conversion if 2.10 is represented as 0x2100000
    }
}


/*F************************************************************************************************/
/*!
    C++ wrapper for the ActionScript function AptUtil.getAptVersion.
    Returns components of the Apt version number as Object properties.

    \Input      None.

    \Returns    Object with the numeric properties Major, Minor, SubMinor, Patch and the string property Version.

    \Notes      On the C++ side, Apt version 2.10.02 is represented as 0x02100200. In this case, the
                returned Object would contain:

                Major =     2
                Minor =    10
                Subminor =  2
                Patch =     0
                Version =   "2.10.02"

                As another example, Apt version 2.16.01-3 would be represented in C++ as 0x02160103. The
                object returned by this method would contain:

                Major =     2
                Minor =    16
                Subminor =  1
                Patch =     3
                Version =   "2.16.01-3"

                So, note that the version string will contain a dash only if the patch != 0.

    \Version
        1.1     01/06/09 (RB) Added comment. Fixed bugs related to SubMinor. Removed MajorStr et al.
*/
/************************************************************************************************F*/
NATIVE_MEMBER_FUNCTION(AptUtil, getAptVersion)
{
    enum {
        FORMATTED_VERSION_SIZE = 32,
        MAJOR_SHIFTS = 24,
        MINOR_SHIFTS = 16,
        SUBMINOR_SHIFTS = 8,
        PATCH_SHIFTS = 0
    };

    AptObject * pObject = new AptObject(AptVFT_Object);

    static const char8_t majorPropertyName[] = "Major";
    static const char8_t minorPropertyName[] = "Minor";
    static const char8_t subminorPropertyName[] = "SubMinor";
    static const char8_t patchPropertyName[] = "Patch";
    static const char8_t versionPropertyName[] = "Version";

    int32_t iMajor = ExtractDecimal( APT_VERSION, MAJOR_SHIFTS);
    int32_t iMinor = ExtractDecimal( APT_VERSION, MINOR_SHIFTS);
    int32_t iSubminor = ExtractDecimal( APT_VERSION, SUBMINOR_SHIFTS);
    int32_t iPatch = ExtractDecimal( APT_VERSION, PATCH_SHIFTS);

    AptNativeString pTmpStr = majorPropertyName;
    pObject->Set(&pTmpStr, AptInteger::Create( iMajor ));

    pTmpStr = minorPropertyName;
    pObject->Set(&pTmpStr, AptInteger::Create( iMinor ));

    pTmpStr = subminorPropertyName;
    pObject->Set(&pTmpStr, AptInteger::Create( iSubminor ));

    pTmpStr = patchPropertyName;
    pObject->Set(&pTmpStr, AptInteger::Create( iPatch ));

    pTmpStr = versionPropertyName;
    char8_t compoundVersionStr[FORMATTED_VERSION_SIZE];
    sprintf(compoundVersionStr, (iPatch > 0)? "%d.%02d.%02d-%d" : "%d.%02d.%02d", iMajor, iMinor, iSubminor, iPatch );
    pObject->Set(&pTmpStr, AptString::Create(compoundVersionStr));

    return pObject ;
}

/*F************************************************************************************************/
/*!
    C++ wrapper for the ActionScript function AptUtil.colorMatrixMultiply
    Multiplies two 4x5 matrices and returns the result.

    \input      stack param 0: Matrix A to multiply.
    \input      stack param 1: Matrix B to multiply.

    \Returns    AptArray with the multiplied result.
*/
/************************************************************************************************F*/
NATIVE_MEMBER_FUNCTION(AptUtil, colorMatrixMultiply)
{
    const int kArraySize = 5 * 4;

    AptValue *pOutputMatrix[kArraySize];
    float pMatrixA[kArraySize];
    float pMatrixB[kArraySize];

    AptValue *pFilterA       = gAptActionInterpreter.stackAt(0);
    AptValue *pFilterB       = gAptActionInterpreter.stackAt(1);

    for (int32_t i = 0; i < kArraySize; i++)
    {
        pMatrixA[i] = static_cast<AptArray *>(pFilterA)->get(i)->toFloat();
        pMatrixB[i] = static_cast<AptArray *>(pFilterB)->get(i)->toFloat();
    }

    // There is not an existing 5x4 mult func around.
    //  AptMath may be a better place for this in the future if it is needed.
    float col[kArraySize];
    float tempVal;
    int32_t i;
    int32_t j;
    int32_t k;

    for (i = 0; i < 4; i++)
    {
        for (j = 0; j < 5; j++)
        {
            col[j] = pMatrixB[j + i * 5];
        }
        for (j = 0; j < 4; j++)
        {
            tempVal = 0;
            for (k= 0; k < 5; k++)
            {
                tempVal += pMatrixA[j + k * 5] * col[k];
            }
            pMatrixB[j + i * 5] = tempVal;
        }
    }

    // Push this to something that has apt values.
    for (int32_t i = 0; i < kArraySize; i++)
    {
        pOutputMatrix[i] = AptFloat::Create( pMatrixB[i] );
    }

    return( AptValueFactory::CreateArray( kArraySize, pOutputMatrix ) );
}

/*F************************************************************************************************/
/*!
    Description goes here...

    \Input

    \Returns

    \Notes
*/
/************************************************************************************************F*/
#if defined(APT_DO_AS_PROFILE)
NATIVE_MEMBER_FUNCTION(AptUtil, profileBlockStart)
{
    const AptValue* pBlockName = gAptActionInterpreter.stackAt(0);
    APT_ASSERT(pBlockName->isString() && "Bad parameter type");
    AptNativeString auxStr;
    pBlockName->toString(auxStr);
    gAptActionInterpreter.mFunctionProfiler.LogBlockStart(auxStr);
    return gpUndefinedValue;
}
#endif // APT_DO_AS_PROFILE


/*F************************************************************************************************/
/*!
    Description goes here...

    \Input

    \Returns

    \Notes
*/
/************************************************************************************************F*/
#if defined(APT_DO_AS_PROFILE)
NATIVE_MEMBER_FUNCTION(AptUtil, profileBlockEnd)
{
    const AptValue* pBlockName = gAptActionInterpreter.stackAt(0);
    APT_ASSERT(pBlockName->isString() && "Bad parameter type");
    AptNativeString auxStr;
    pBlockName->toString(auxStr);
    gAptActionInterpreter.mFunctionProfiler.LogBlockEnd();
    return gpUndefinedValue;
}
#endif // APT_DO_AS_PROFILE

/******************************************************************************/
/*! _SafeForIn

    \brief helper function to do the actual work of AptUtil.SafeForIn
           Given the args, (someObject, someFunction),
           For each key in someObject,
                call someFunction(someObject, key, value)
           if someFunction returns false, it stops iterating early

    \param object    - (see above)
    \param function  - (see above)
    \return            - void

*/
/******************************************************************************/
void _SafeForIn(AptValue * object, AptValue * func)
{
    bool keepGoing = true;
    AptNativeHash *pNativeHash = object->GetNativeHashVirtual();
    while (pNativeHash && keepGoing)
    {
        for (AptHashItem *pItem = pNativeHash->GetFirstItem(); pItem; pItem = pNativeHash->GetNextItem(pItem))
        {
            if( (pItem->Key.EqualNoCase(*StringPool::GetString(SC___proto__))) ||
                (pItem->Key.EqualNoCase(*StringPool::GetString(SC_prototype))) )
            {
                continue;
            }
            AptString * pString = AptString::Create();
            pString->cpy(&pItem->Key);
            AptValue * retVal = AptCallFunctionObject(func, 3, object, pString, pItem->mValue);

            if ( !retVal->isUndefined() && !retVal->toBool() )
            {
                APT_DEC(retVal);
                keepGoing = false;
                break;
            }

            APT_DEC(retVal);
        }
        AptValue *pProto = pNativeHash->Get__Proto__();
        pNativeHash = pProto ? pProto->GetNativeHashVirtual() : 0;
    }
}


/*F************************************************************************************************/
/*!
    Given the args, (someObject, someFunction),
    For each key in someObject,
        call someFunction(someObject, key, value)
    if someFunction returns false, it stops iterating early

    \Input    <see description above>

    \Returns  gpUndefinedValue

    \Notes
*/
/************************************************************************************************F*/
NATIVE_MEMBER_FUNCTION(AptUtil, safeForIn)
{
    // if there are too many (defined) args sent, you are using it wrong and
    // maybe we should warn you about that
    ASSERT_IF_TOO_MANY_DEFINED_ARGS(2, nParams, "Sent too many args to AptUtil.safeForIn()! It expects (someObject:Object, someFunction:Function=):String");

    _SafeForIn( gAptActionInterpreter.stackAt(0), gAptActionInterpreter.stackAt(1) );
    return gpUndefinedValue;
}

/*F************************************************************************************************/
/*!
    Given a color using the arguments (hue, saturation, brightness), return a transform Object
    that can be used by Color.setTransform
*/
/************************************************************************************************F*/
NATIVE_MEMBER_FUNCTION(AptUtil, convertHsvToColorTransform)
{
    ASSERT_IF_TOO_MANY_DEFINED_ARGS(3, nParams, "Sent too many args to AptUtil.transformFromHsv()! It expects (hue:Number, saturation:Number, brightness:Number");

    AptObject* transform = new AptObject(AptVFT_Object);

    AptValue* hueVal = gAptActionInterpreter.stackAt(0);
    AptValue* saturationVal = gAptActionInterpreter.stackAt(1);
    AptValue* brightnessVal = gAptActionInterpreter.stackAt(2);

    float hue = hueVal->toFloat();
    float saturation = saturationVal->toFloat();
    float brightness = brightnessVal->toFloat();

#if defined(APT_USE_FLASH_COLOR_RANGE)
    float redVal = 0.f;
    float greenVal = 0.f;
    float blueVal = 0.f;
#else
    int redVal = 0;
    int greenVal = 0;
    int blueVal = 0;
#endif

    if (0.f < brightness)
    {
        // Ensure hue is in the range [0, 360).
        hue = fmodf(hue, 360.f);
        if (hue < 0.f)
        {
            hue += 360.f;
        }

        // Please see this page for algorithm details:
        //   http://en.wikipedia.org/wiki/HSL_and_HSV#From_HSV
        saturation /= 100.f;
        brightness /= 100.f;
        hue /= 60.f;

        int i = static_cast<int>(floor(hue));
        float f = hue - i;
        float p = brightness * (1.f - saturation);
        float q = brightness * (1.f - (saturation * f));
        float t = brightness * (1.f - (saturation * (1.f - f)));

        float red, green, blue;
        switch(i)
        {
        case 0:
            red = brightness; green = t; blue = p;
            break;

        case 1:
            red = q; green = brightness; blue = p;
            break;

        case 2:
            red = p; green = brightness; blue = t;
            break;

        case 3:
            red = p; green = q; blue = brightness;
            break;

        case 4:
            red = t; green = p; blue = brightness;
            break;

        case 5:
            red = brightness; green = p; blue = q;
            break;

        default:
            // Not possible to get here - hue is in the range [0, 360),
            // then we divided it by 60 and floored it.  But I have to put
            // this here anyway to keep the compiler happy
            red = green = blue = 0.f;
            break;
        }

        // Simple rounding math to get us to an int within range
        // NOTE: The main difference here with APT_USE_FLASH_COLOR_RANGE
        // the range maxing out at 100 (when APT_USE_FLASH_COLOR_RANGE
        // is defined) or 255 (when it's not)
#if defined(APT_USE_FLASH_COLOR_RANGE)
        redVal = floor(red * 100.f + 0.5f);
        greenVal = floor(green * 100.f + 0.5f);
        blueVal = floor(blue * 100.f + 0.5f);
#else
        redVal = static_cast<int>(floor(red * 255.f + 0.5f));
        greenVal = static_cast<int>(floor(green * 255.f + 0.5f));
        blueVal = static_cast<int>(floor(blue * 255.f + 0.5f));
#endif
    }

    // Return an object like
    //    {
    //        ra : redValue
    //        ga : greenValue
    //        ba : blueValue
    //    }
    // This is the same sort of object Color.setTransform takes.
#if defined(APT_USE_FLASH_COLOR_RANGE)
    transform->Set(StringPool::GetString(SC_ra), AptFloat::Create(redVal));
    transform->Set(StringPool::GetString(SC_ga), AptFloat::Create(greenVal));
    transform->Set(StringPool::GetString(SC_ba), AptFloat::Create(blueVal));
#else
    transform->Set(StringPool::GetString(SC_ra), AptInteger::Create(redVal));
    transform->Set(StringPool::GetString(SC_ga), AptInteger::Create(greenVal));
    transform->Set(StringPool::GetString(SC_ba), AptInteger::Create(blueVal));
#endif
    return transform;
}


#endif // APT_USE_UTILITY
