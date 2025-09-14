/*H************************************************************************************************/
/*!

    \File
        AptUtil.h

    \Description
        This provides common utilities functions previously defined in ActionScript (by ION/Central UI).
        Moved to C++ to improve ActionScript execution performance.

    \Notes

    \Copyright
        (c) 2008 Electronic Arts Inc.

    \Version 1.0        07/03/08 ($senzee) First version.

*/
/************************************************************************************************H*/

#ifndef __APTUTIL_H__
#define __APTUTIL_H__

#include "AptObject/AptObject.h"

class AptUtil : public AptObject
{
public:
#if APT_USE_UTILITY
    APT_VALUE_GC_NEW_DELETE_OPERATORS
    // DC - Added Metric defines
    APT_DECLARE_OBJECT_METRICS

    AptUtil() : AptObject(AptVFT_AptUtil, APT_OBJECTHASHSIZE)
    {
        setGCRoot(1) ;
    }

    virtual AptValue *objectMemberLookup(AptValue *const pContext, const AptNativeString *const pName) const;

    NATIVE_MEMBER_FUNCTION_DECL(formatNumberString);
    NATIVE_MEMBER_FUNCTION_DECL(replaceString);
    NATIVE_MEMBER_FUNCTION_DECL(trimString);
    NATIVE_MEMBER_FUNCTION_DECL(trimLeftString);
    NATIVE_MEMBER_FUNCTION_DECL(trimRightString);
    NATIVE_MEMBER_FUNCTION_DECL(searchArray);
    NATIVE_MEMBER_FUNCTION_DECL(reverseSearchArray);
    NATIVE_MEMBER_FUNCTION_DECL(countArray);
    NATIVE_MEMBER_FUNCTION_DECL(getAptVersion);
    NATIVE_MEMBER_FUNCTION_DECL(safeForIn);
    NATIVE_MEMBER_FUNCTION_DECL(convertHsvToColorTransform);
    NATIVE_MEMBER_FUNCTION_DECL(colorMatrixMultiply);
#if defined(APT_DO_AS_PROFILE)
    NATIVE_MEMBER_FUNCTION_DECL(profileBlockStart);
    NATIVE_MEMBER_FUNCTION_DECL(profileBlockEnd);
#endif

#else
    AptUtil() : AptObject(AptVFT_AptUtil, APT_OBJECTHASHSIZE)
    {
        APT_ASSERT(0 && "APT-Warning-AptUtil object cannot be used as it is compiled out, make sure to compile with APT_USE_UTILITY defined as 1") ;
    }
#endif

    static void CleanNativeFunctions();

protected:

    virtual ~AptUtil()
    {
        //  Do nothing...
    }

private:

    enum Trim { LEFT, RIGHT, BOTH };

    static AptValue *trim(Trim type, int32_t paramCount);
    static AptValue *search(bool reverse, int32_t paramCount);
};

#endif  //  __APTUTIL_H__

