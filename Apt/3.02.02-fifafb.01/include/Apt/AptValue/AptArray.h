/*H************************************************************************************************/
/*!

    \File
        AptArray.h

    \Description
        Simple description of main purpose of file.

    \Notes
        None.

    \Copyright
        (c) 2004 Electronic Arts Inc.

    \Version
        1.0        03/06/04 (ONA) First Version
        1.1        03/05/08 (RB)  Remove extra ';' after NATIVE_MEMBER_FUNCTION_DECL.
        1.2        08/14/08 ($senzee) Added find functionality in support of AptUtil functions.
        1.3        02/03/09 (RB)  Removed defaultFindComparator.
        1.4        10/30/09 (RB)  Replaced usage of APT_VALUE_GC_NEW_DELETE_OPERATORS with
                                  plain operator new/delete declarations.

*/
/************************************************************************************************H*/

#ifndef __APTARRAY_H__
#define __APTARRAY_H__

#include "AptObject/AptObject.h"
// FIFA_BEGIN | SENDBACK | mwebb | 2015 - 05 - 26 | Adding Frostbite support
#include "Module.h"
// FIFA_END

class AptArray : public AptObject
{
public:

    typedef int (* fnFindComparator_t)(AptValue *value, AptValue *equals, const AptNativeString *equalsStr, const AptNativeString *property);

	// FIFA_BEGIN | SENDBACK | mwebb | 2015 - 05 - 26 | Adding Frostbite support
    //void                set(int nIndex, AptValue *pValue);
    //AptValue *          get(int nIndex);
    //void                toString(char *szBuf, const char *szSeparator = ",");
    //int                 find(fnFindComparator_t comparator, int start, AptValue *equals, AptValue *property, bool reverse = false);
    //APT_INLINE int      length()
    //{
    //    return mnLength ;
    //}

    //static int          objectFindComparator(AptValue *value, AptValue *equals, const AptNativeString *equalsStr, const AptNativeString *property);

    //// the contructor and new/delete operators should not really be public, but legacy client code requires access
    //static void *operator new(size_t size);
    //void* operator new (size_t t, void *p);
    //static void operator delete(void *p, size_t size);
    //AptArray();
	FB_APT_API void                set(int nIndex, AptValue *pValue);
	FB_APT_API AptValue *          get(int nIndex);
	FB_APT_API void                toString(char *szBuf, const char *szSeparator = ",");
	FB_APT_API int                 find(fnFindComparator_t comparator, int start, AptValue *equals, AptValue *property, bool reverse = false);
	FB_APT_API APT_INLINE int      length()
	{
		return mnLength;
	}

	FB_APT_API static int          objectFindComparator(AptValue *value, AptValue *equals, const AptNativeString *equalsStr, const AptNativeString *property);

	// the contructor and new/delete operators should not really be public, but legacy client code requires access
	FB_APT_API static void *operator new(size_t size);
	FB_APT_API void* operator new (size_t t, void *p);
	FB_APT_API static void operator delete(void *p, size_t size);
	FB_APT_API AptArray();
	// FIFA_END

APT_ACCESS_INTERNAL:
    APT_DECLARE_OBJECT_METRICS

    AptArray(int nElements, AptValue ** pAptValue);

    static void CleanNativeFunctions();

    virtual AptValue *  objectMemberLookup(AptValue * const pContext, const AptNativeString * const pName) const;
    virtual bool        objectMemberSet(AptValue * const pContext, const AptNativeString * const pName, AptValue * const pValue);
    void                toString(AptNativeString &sBuf, const char *szSeparator = ",");

    AptValue *  GetAt(const int32_t nIndex) const;
    void        SetAt(const int32_t nIndex, AptValue * const pNewValue);


    NATIVE_MEMBER_FUNCTION_DECL(concat)
    NATIVE_MEMBER_FUNCTION_DECL(join)
    NATIVE_MEMBER_FUNCTION_DECL(pop)
    NATIVE_MEMBER_FUNCTION_DECL(push)
    NATIVE_MEMBER_FUNCTION_DECL(shift)
    NATIVE_MEMBER_FUNCTION_DECL(unshift)
    NATIVE_MEMBER_FUNCTION_DECL(reverse)
    NATIVE_MEMBER_FUNCTION_DECL(sort)
    NATIVE_MEMBER_FUNCTION_DECL(splice)
    NATIVE_MEMBER_FUNCTION_DECL(slice)
    NATIVE_MEMBER_FUNCTION_DECL(sortOn)

protected:
    virtual ~AptArray();
    virtual void RegisterReferences();
    virtual void DestroyGCPointers();

private:
    void        _reserve(int nSize);
    AptValue ** mpValues;
    int         mnCapacity;  // Num Items Allocated In the Array.
    int         mnLength;    // Num Items In the Array.

    static int defaultSortCompareFunc(const void *a, const void *b);
    static int defaultSortOnCompareFunc(const void *a, const void *b);
    static int scriptFunctionSortFunc(const void *a, const void *b);

private:
    // Do not call. Garbage collected objects cannot be created as C++ arrays.
    static void *operator new[](size_t size);
    static void operator delete[](void *p);

};

#endif  //  __APTARRAY_H__

/*H*************************************************************************************************
    EOF
*************************************************************************************************H*/

