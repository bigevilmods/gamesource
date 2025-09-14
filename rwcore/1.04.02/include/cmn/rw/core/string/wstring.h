// (c) Electronic Arts. All Rights Reserved.

/// \file
/// \deprecated Use EASTL's string class instead.
///
/// Wide String class definition

#ifndef RW_CORE_STRING_WSTRING_H
#define RW_CORE_STRING_WSTRING_H
#include <rw/core/base/ostypes.h>
#include <rw/core/resource/resourceallocator.h>


#if defined(EA_PLATFORM_WINDOWS) || defined(EA_PLATFORM_XENON)
#pragma warning(push)
#pragma warning(disable:4996) // disable deprecated warnings against EAStdC memory functions.
#endif
#include <EAStdC/EAMemory.h>
#include <EAStdC/EAString.h>


#include <rw/core/string/wstringconfig.h>

#ifdef __GNUC__

namespace TestingSizeOfWchar_t
{
// This will create a compile time error if expr is 0
#define RW_COMPILE_TIME_ASSERT(expr,msg) \
    struct CompileTimeAssert { unsigned int msg : expr; };

    // This is to make sure that client-code is compiled using the -fshort-wchar when using GCC
    RW_COMPILE_TIME_ASSERT(sizeof(wchar_t) == 2, COMPILE_ASSERT_Wchar_size_is_not_2);
}
#endif


namespace rw
{
namespace core
{
// forward declaration
class WString;

/*
 WString allocator interface class
 This has functions to allow the allocator to handle the length and capacity
 It also has a functions to allow the allocator to handle modification of the
 string so Ref-Counted strings can update their ref-counts
 The Assign and Compare functions will only be called if both strings are of the same
 type to allow optimizations.
*/
class IWStringAllocator
{
public:
    virtual void        Modify(char16_t** p) = 0;
    virtual char16_t*    Allocate(size_t n) = 0;
    virtual void        Deallocate(char16_t* p) = 0;

    virtual uint32_t    GetAllocatorType() const = 0;

    virtual void        Assign(WString& s1, char16_t** p1, const WString& s2, char16_t* p2) = 0;
    virtual int32_t     Compare(const char16_t* s1, const char16_t* s2) = 0;

    virtual void        SetLength(char16_t *p, size_t n) = 0;
    virtual size_t      GetLength(const char16_t *p) const = 0;
    virtual size_t      GetCapacity(const char16_t *p) const = 0;

    // default constructor & destructor
    IWStringAllocator() {}
    virtual ~IWStringAllocator() {}
};
/*
 WString class
 This contains all of the functionality for handling strings. All of the functions
 are implemented to use a single pointer for the start of the string and use the
 allocator to determine the length and capacity for doing work. This will allow us
 to have different allocators to handle ref-counted strings and stack based strings.

 The provided allocators are setup to store all of the string specific information at
 the start of the string buffer. This will let us have a single instance of the allocator
 that can handle all of the strings of that type.

 If no allocator is provided, the string will use whatever default allocator is supplied.
 This will usually be an allocator that dynamicly allocates memory for the string from
 the heap.
*/
class WString
{
public:
    /*
    The types of strings
    */
    enum WStringTypes
    {
        STRING_TYPE_ALLOCATED,
        STRING_TYPE_REFCOUNT,
        STRING_TYPE_CONST,
        STRING_TYPE_FIXED
    };

    /*
    'npos' means non-valid position or simply non-position.
    */
    #ifdef RW_COMPILER_NO_STATIC_CONSTANTS
        enum { npos = -1 };
    #else
        static const size_t npos = static_cast<size_t>(-1);
    #endif

    /*
     CtorDoNotInitialize exists so that we can create a constructor that allocates but doesn't
     initialize and also doesn't collide with any other constructor declaration.
    */
    struct CtorDoNotInitialize{};

    /*
     CtorSprintf exists so that we can create a constructor that accepts printf-style
     arguments but also doesn't collide with any other constructor declaration.
    */
    struct CtorSprintf{};

    /*
    // CtorBuffer exists so that we can create a constructor that provides a preallocated
    // buffer to use
    */
    struct CtorBuffer{};

    // Constructors
    explicit WString();
    explicit WString(IWStringAllocator *a); // C++ basic_string uses explicit, so we follow suit.

    // use the provided buffer instead of allocating
    WString(CtorBuffer unused, char16_t* p, IWStringAllocator *a);

    WString(const WString& s);
    WString(const WString& s, IWStringAllocator *a);

    WString(const WString& s, size_t pos, size_t n = npos);
    WString(const WString& s, size_t pos, size_t n, IWStringAllocator *a);

    WString(const char16_t* p, size_t n);
    WString(const char16_t* p, size_t n, IWStringAllocator* a);

    //lint -e{1931} although this can be used for implicit conversion,
    //we need to leave the explicit out because othersize the "assign a
    //char16_t to a WString operator" won't compile.
    WString(const char16_t* p);
    WString(const char16_t* p, IWStringAllocator *a);

    WString(size_t n, char16_t c);
    WString(size_t n, char16_t c, IWStringAllocator *a);

    WString(const char16_t* pBegin, const char16_t* pEnd);
    WString(const char16_t* pBegin, const char16_t* pEnd, IWStringAllocator *a);

    WString(CtorDoNotInitialize unused, size_t n); //This is not a standard C++ basic_string constructor.
    WString(CtorDoNotInitialize unused, size_t n, IWStringAllocator *a);

    //lint -e{1916} ellipsis encountered
    WString(CtorSprintf unused, const char16_t* pFormat, ...); //This is not a standard C++ basic_string constructor.

    //lint -e{1916} ellipsis encountered
    WString(CtorSprintf unused, IWStringAllocator *a, const char16_t* pFormat, ...);
    
    void PreDerivedDtorBaseCleanUp();

    // Destructor
    ~WString();

    // Operator =
    WString& operator=(const WString& s);
    WString& operator=(const char16_t* p);
    WString& operator=(char16_t c);

    // Allocator functions
    const IWStringAllocator* GetAllocator() const
    {
        rwFUNCTION();
        rwRETURN(mAllocator);
    }

    static void             SetDefaultAllocator(IWStringAllocator *a)
    {
        rwFUNCTION();
        mDefaultAllocator = a;
        rwRETURNVOID();
    }
    /*
     Returns a constant pointer to the current string buffer

     \return pointer to the current string buffer
    */
    const char16_t* GetPointer() const
    {
        rwFUNCTION();
        rwRETURN(mWString);
    }

    /*
    returns the capacity of the string
    */
    size_t GetCapacity() const;

    /*
    returns the number of characters in the string
    */
    size_t GetLength() const;

    /*
    returns true if the string has no characters
    */
    bool   IsEmpty() const;

    /*
    set the capacity of the string, will not truncate a string
    */
    void   SetCapacity(size_t n = 0);

    /*
    clears our the contents of the string
    */
    void   Clear();

    /*
    Returns the maximum possible size of a string

     \return the maximum possible size of a string
    */
    size_t GetMaxSize() const
    {
        rwFUNCTION();
        rwRETURN(size_t(-2));
    }

    // Element access
    char16_t operator[](size_t n) const;
    char16_t GetAt(size_t n) const;
    void    SetAt(size_t n, char16_t c);

    // Append operations
    WString& operator+=(const WString& s);
    WString& operator+=(const char16_t* p);
    WString& operator+=(char16_t c);
    WString& Append(const WString& s);
    WString& Append(const WString& s, size_t pos, size_t n);
    WString& Append(const char16_t* p, size_t n);
    WString& Append(const char16_t* p);
    WString& Append(size_t n, char16_t c);
    WString& Append(const char16_t* pBegin, const char16_t* pEnd);
    void    PushOnBack(char16_t c);
    void    PopFromBack();

    // Assignment operations
    WString& Assign(const WString& s);
    WString& Assign(const WString& s, size_t pos, size_t n);
    WString& Assign(const char16_t* p, size_t n);
    WString& Assign(const char16_t* p);
    WString& Assign(size_t n, char16_t c);
    WString& Assign(const char16_t* pBegin, const char16_t* pEnd);

    // Insertion operations
    WString& Insert(size_t pos, const WString& s);
    WString& Insert(size_t pos, const WString& s, size_t beg, size_t n);
    WString& Insert(size_t pos, const char16_t* p, size_t n);
    WString& Insert(size_t pos, const char16_t* p);
    WString& Insert(size_t pos, size_t n, char16_t c);

    // Erase operations
    WString& Erase(size_t pos = 0, size_t n = npos);

    // Replacement operations
    WString& Replace(size_t pos, size_t n, const WString& s);
    WString& Replace(size_t pos1, size_t n1, const WString& s, size_t pos2, size_t n2);
    WString& Replace(size_t pos, size_t n1, const char16_t* p, size_t n2);
    WString& Replace(size_t pos, size_t n1, const char16_t* p);
    WString& Replace(size_t pos, size_t n1, size_t n2, char16_t c);

    // Find operations
    size_t Find(const WString& s, size_t pos = 0) const;
    size_t Find(const char16_t* p, size_t pos = 0) const;
    size_t Find(const char16_t* p, size_t pos, size_t n) const;
    size_t Find(char16_t c, size_t pos = 0) const;

    // Reverse find operations
    size_t Rfind(const WString& s, size_t pos = npos) const;
    size_t Rfind(const char16_t* p, size_t pos = npos) const;
    size_t Rfind(const char16_t* p, size_t pos, size_t n) const;
    size_t Rfind(char16_t c, size_t pos = npos) const;

    // Find first-of operations
    size_t FindFirstOf(const WString& s, size_t pos = 0) const;
    size_t FindFirstOf(const char16_t* p, size_t pos = 0) const;
    size_t FindFirstOf(const char16_t* p, size_t pos, size_t n) const;
    size_t FindFirstOf(char16_t c, size_t pos = 0) const;

    // Find last-of operations
    size_t FindLastOf(const WString& s, size_t pos = npos) const;
    size_t FindLastOf(const char16_t* p, size_t pos = npos) const;
    size_t FindLastOf(const char16_t* p, size_t pos, size_t n) const;
    size_t FindLastOf(char16_t c, size_t pos = npos) const;

    // Find first not-of operations
    size_t FindFirstNotOf(const WString& s, size_t pos = 0) const;
    size_t FindFirstNotOf(const char16_t* p, size_t pos = 0) const;
    size_t FindFirstNotOf(const char16_t* p, size_t pos, size_t n) const;
    size_t FindFirstNotOf(char16_t c, size_t pos = 0) const;

    // Find last not-of operations
    size_t FindLastNotOf(const WString& s,  size_t pos = npos) const;
    size_t FindLastNotOf(const char16_t* p, size_t pos = npos) const;
    size_t FindLastNotOf(const char16_t* p, size_t pos, size_t n) const;
    size_t FindLastNotOf(char16_t c, size_t pos = npos) const;

    // Substring functionality
    WString Substr(size_t pos = 0, size_t n = npos) const;
    WString Left(size_t n) const;
    WString Right(size_t n) const;

    // Comparison operations
    int         Compare(const WString& s) const;
    int         Compare(size_t pos1, size_t n1, const WString& s) const;
    int         Compare(size_t pos1, size_t n1, const WString& s, size_t pos2, size_t n2) const;
    int         Compare(const char16_t* p) const;
    int         Compare(size_t pos1, size_t n1, const char16_t* p) const;
    int         Compare(size_t pos1, size_t n1, const char16_t* p, size_t n2) const;
    static int  Compare(const char16_t* pBegin1, const char16_t* pEnd1, const char16_t* pBegin2, const char16_t* pEnd2);

    /*
    returns the hash value for the string
    */
    size_t    Hash() const;

    // converts the string to all upper/lower case
    void      MakeLower();
    void      MakeUpper();

    // sprintf functions
    WString&   Sprintf_va_list(const char16_t* pFormat, va_list arglist);

    //lint -e{1916} ellipsis encountered
    WString&   Sprintf(const char16_t* pFormat, ...);

    // UTF16 specific functions
    const char16_t* GetBufferUTF16(size_t n) const;
    size_t          GetLengthUTF16() const;
    int32_t         GetAtUTF16(size_t n) const;
    void            SetAtUTF16(size_t n, int32_t c);
    WString&        AppendUTF16(const char16_t* p, size_t n);
    WString&        InsertUTF16(size_t pos, const char16_t* p, size_t n);
    size_t          FindUTF16(const char16_t* p, size_t pos = 0) const;
    WString         SubstrUTF16(size_t pos = 0, size_t n = npos) const;
    void            MakeLowerUTF16();
    void            MakeUpperUTF16();

    // UTf16 helper functions
    static const char16_t*  GetBufferUTF16(const char16_t *p, size_t n);
    static size_t           GetLengthUTF16(const char16_t *p);
    static const char16_t*  ReadCharacterUTF16(const char16_t *p, int32_t *pCharacter);
    static int32_t          GetCharacterUTF16(const char16_t *pBuffer);
    static int32_t          GetCharacterSizeUTF16(const char16_t *p);
    static int32_t          GetCharacterSizeUTF16(const int32_t c);
    static void             SetCharacterUTF16(char16_t *p, int32_t c);

    /*
     Used for empty strings. This allows us to not have to allocate memory for
     empty strings and reduces null pointer checks.  Not const because it gets
     assigned to non-const strings in the implementation.
    */
    static char16_t *mEmpty;

private:
    // helper functions
    void RangeInitialize(const char16_t *p, size_t n);
    void SizeInitialize(size_t n, char16_t c);
    void Insert(char16_t *p, const char16_t* pBegin, const char16_t* pEnd);
    WString& Erase(char16_t *pBegin, const char16_t *pEnd);
    WString& Replace(char16_t *pBegin1, char16_t *pEnd1, const char16_t *pBegin2, const char16_t *pEnd2);

    // helper search functions
    static const char16_t* WStringFind(const char16_t* pBegin, const char16_t* pEnd, const char16_t c);
    static const char16_t* WStringFindEnd(const char16_t* pBegin, const char16_t* pEnd, char16_t c);
    static const char16_t* WStringRFind(const char16_t* pRBegin, const char16_t* pREnd, const char16_t c);
    static const char16_t* WStringSearch(const char16_t* p1Begin, const char16_t* p1End, const char16_t* p2Begin, const char16_t* p2End);
    static const char16_t* WStringRSearch(const char16_t* p1Begin, const char16_t* p1End, const char16_t* p2Begin, const char16_t* p2End);
    static const char16_t* WStringFindFirstOf(const char16_t* p1Begin, const char16_t* p1End, const char16_t* p2Begin, const char16_t* p2End);
    static const char16_t* WStringRFindFirstOf(const char16_t* p1RBegin, const char16_t* p1REnd, const char16_t* p2Begin, const char16_t* p2End);
    static const char16_t* WStringFindFirstNotOf(const char16_t* p1Begin, const char16_t* p1End, const char16_t* p2Begin, const char16_t* p2End);
    static const char16_t* WStringRFindFirstNotOf(const char16_t* p1RBegin, const char16_t* p1REnd, const char16_t* p2Begin, const char16_t* p2End);

    // pointer to the allocator being used for the string
    IWStringAllocator *mAllocator;

    // pointer to the string buffer, also passed to allocator for additional storage
    char16_t          *mWString;

    // the default allocator of the string class
    static IWStringAllocator *mDefaultAllocator;
};

// string operators
WString operator+(const WString& x,             const WString& y);
WString operator+(const char16_t* p,            const WString& y);
WString operator+(char16_t c,                   const WString& y);
WString operator+(const WString& x,             const char16_t* p);
WString operator+(const WString& x,             const char16_t  c);

bool operator==(const WString&  x,              const WString&  y);
bool operator==(const char16_t* p,              const WString&  y);
bool operator==(const WString&  x,              const char16_t* p);
bool operator!=(const WString&  x,              const WString&  y);
bool operator!=(const char16_t* p,              const WString&  y);
bool operator!=(const WString&  x,              const char16_t* p);

bool operator< (const WString&  x,           const WString&  y);
bool operator< (const char16_t* p,           const WString&  y);
bool operator< (const WString&  x,           const char16_t* p);
bool operator> (const WString&  x,           const WString&  y);
bool operator> (const char16_t* p,           const WString&  y);
bool operator> (const WString&  x,           const char16_t* p);
bool operator<=(const WString&  x,           const WString&  y);
bool operator<=(const char16_t* p,           const WString&  y);
bool operator<=(const WString&  x,           const char16_t* p);
bool operator>=(const WString&  x,           const WString&  y);
bool operator>=(const char16_t* p,           const WString&  y);
bool operator>=(const WString&  x,           const char16_t* p);
/*
 Default string allocator class, dynamicly allocates all memory from the core allocator
 supplied in the constructor. There is 4 bytes of overhead allocated at the start of the
 string buffer to cache the capacity and length of the string
*/
class WStringCoreAllocator : public IWStringAllocator
{
public:
    explicit WStringCoreAllocator(rw::IResourceAllocator *allocator);

    virtual void        Modify(char16_t** p);
    virtual char16_t*    Allocate(size_t n);
    virtual void        Deallocate(char16_t* p);

    virtual uint32_t    GetAllocatorType() const;

    virtual void        Assign(WString& s1, char16_t** p1, const WString& s2, char16_t* p2);
    virtual int32_t     Compare(const char16_t* s1, const char16_t* s2);

    virtual void        SetLength(char16_t *p, size_t n);
    virtual size_t      GetLength(const char16_t *p) const;
    virtual size_t      GetCapacity(const char16_t *p) const;

private:
    IResourceAllocator *mCoreAllocator;

    /*
     layout of data
     the mBuffer would be the actual size of the character string
     This would return a pointer to the mBuffer to be stored in the WString::mWString member
    */
    struct Data
    {
        uint16_t mCapacity;
        uint16_t mLength;
        char16_t  mBuffer[256];
    };
};
/*
 Ref-counted string allocator class, dynamicly allocates all memory from the core
 allocator supplied in the constructor. There is 6 bytes of overhead allocated at
 the start of the string buffer to cache the capacity and length of the string and to
 store the current reference count.
*/
class WStringRefAllocator : public IWStringAllocator
{
public:
    explicit WStringRefAllocator(IResourceAllocator *allocator);

    virtual void        Modify(char16_t** p);
    virtual char16_t*    Allocate(size_t n);
    virtual void        Deallocate(char16_t* p);

    virtual uint32_t    GetAllocatorType() const;

    virtual void        Assign(WString& s1, char16_t** p1, const WString& s2, char16_t* p2);
    virtual int32_t     Compare(const char16_t* s1, const char16_t* s2);

    virtual void        SetLength(char16_t *p, size_t n);
    virtual size_t      GetLength(const char16_t *p) const;
    virtual size_t      GetCapacity(const char16_t *p) const;

private:
    IResourceAllocator *mCoreAllocator;
    /*
     layout of data
     the mBuffer would be the actual size of the character string
     This would return a pointer to the mBuffer to be stored in the WString::mWString member
    */
    struct Data
    {
        uint16_t mRefCount;
        uint16_t mCapacity;
        uint16_t mLength;
        char16_t  mBuffer[256];
    };
};
/*
 Const string allocator class, this is intended to be used for read-only strings
 the buffer to use is passed in the constructor, any calls to the
 Modify/Assign/Allocate/Deallocate functions will assert, GetLength and GetCapacity will
 return the string length
*/
class WStringConstAllocator : public IWStringAllocator
{
public:
    WStringConstAllocator();

    virtual void        Modify(char16_t** p);
    virtual char16_t*    Allocate(size_t n);
    virtual void        Deallocate(char16_t* p);

    virtual uint32_t    GetAllocatorType() const;

    virtual void        Assign(WString& s1, char16_t** p1, const WString& s2, char16_t* p2);
    virtual int32_t     Compare(const char16_t* s1, const char16_t* s2);

    virtual void        SetLength(char16_t *p, size_t n);
    virtual size_t      GetLength(const char16_t *p) const;
    virtual size_t      GetCapacity(const char16_t *p) const;
};
/*
 Fixed string allocator class, this is a base class for the WStringSizedAllocator template
 that will create an in-place buffer to use for the string. The Allocate/Deallocate
 functions will assert and the GetCapacity needs to be implemented by the template.
*/
class WStringFixedAllocator : public IWStringAllocator
{
public:
    virtual void        Modify(char16_t** p);
    virtual char16_t*    Allocate(size_t n);
    virtual void        Deallocate(char16_t* p);

    virtual uint32_t    GetAllocatorType() const;

    virtual void        Assign(WString& s1, char16_t** p1, const WString& s2, char16_t* p2);
    virtual int32_t     Compare(const char16_t* s1, const char16_t* s2);

    virtual void        SetLength(char16_t *p, size_t n);
    virtual size_t      GetLength(const char16_t *p) const;
    virtual size_t      GetCapacity(const char16_t *p) const;

protected:
    WStringFixedAllocator();
};
/*
 A template to provide a fixed allocator of a specificed size. This can be used to
 create a string allocator that has the buffer allocated on the stack or in a structure.
*/
template<uint16_t _capacity>
class WStringSizedAllocator : public WStringFixedAllocator
{
public:
    WStringSizedAllocator()
    : WStringFixedAllocator()
    {
        rwFUNCTION();
        rwRETURNVOID();
    }

    virtual size_t GetCapacity(const char16_t*) const
    {
        rwFUNCTION();
        rwRETURN(_capacity);
    }
};
/*
 A template to provide a fixed size string from the stack or in a structure.
 Usage would be: WStringSized<256> aLongWString;
*/
//lint -e{1509} base class destructor for class 'WString' is not virtual.
template<uint16_t _capacity>
class WStringSized : public WString
{
public:
    WStringSized()
    : WString(CtorBuffer(), mBuffer, &mSizedAllocator)
    {
        rwFUNCTION();
        *mBuffer = 0;
        rwRETURNVOID();
    }
    ~WStringSized()
    {
        rwFUNCTION();
        PreDerivedDtorBaseCleanUp();
        rwRETURNVOID();
    }

private:
    WStringSizedAllocator<_capacity> mSizedAllocator;
    char16_t mBuffer[_capacity];
};

/*
 Compares the 2 strings for equality

 \param x The first string
 \param y The second string
 \return true if the strings are the same, false otherwise
*/
inline bool operator==(const WString& x, const WString& y)
{
    rwFUNCTION();
    size_t xLen = x.GetLength();
    size_t yLen = y.GetLength();

    rwRETURN((xLen == yLen) &&
        (STRING_MEMCOMPARE(x.GetPointer(), y.GetPointer(), x.GetLength()) == 0));
}
/*
 Compares the 2 strings for equality

 \param s The first string
 \param y The second string
 \return true if the strings are the same, false otherwise
*/
inline bool operator==(const char16_t* s, const WString& y)
{
    rwFUNCTION();
    const size_t n(WSTRING_STRINGLENGTH(s));
    rwRETURN(((n == y.GetLength()) && (STRING_MEMCOMPARE(s, y.GetPointer(), n) == 0)));
}
/*
 Compares the 2 strings for equality

 \param x The first string
 \param s The second string
 \return true if the strings are the same, false otherwise
*/
inline bool operator==(const WString& x, const char16_t* s)
{
    rwFUNCTION();
    const size_t n = WSTRING_STRINGLENGTH(s);
    rwRETURN(((x.GetLength() == n) && (STRING_MEMCOMPARE(x.GetPointer(), s, n) == 0)));
}
/*
 Compares the 2 strings for inequality

 \param x The first string
 \param y The second string
 \return true if the strings are different, false otherwise
*/
inline bool operator!=(const WString& x, const WString& y)
{
    rwFUNCTION();
    rwRETURN(!(x == y));
}
/*
 Compares the 2 strings for inequality

 \param s The first string
 \param y The second string
 \return true if the strings are different, false otherwise
*/
inline bool operator!=(const char16_t* s, const WString& y)
{
    rwFUNCTION();
    rwRETURN(!(s == y));
}
/*
 Compares the 2 strings for inequality

 \param x The first string
 \param s The second string
 \return true if the strings are different, false otherwise
*/
inline bool operator!=(const WString& x, const char16_t* s)
{
    rwFUNCTION();
    rwRETURN(!(x == s));
}
/*
 Compares the 2 strings for less than

 \param x The first string
 \param y The second string
 \return true if x is less than y
*/
inline bool operator<(const WString& x, const WString& y)
{
    rwFUNCTION();
    //lint -e{931} both sides have side effects
    int cmp = WString::Compare(x.GetPointer(), x.GetPointer() + x.GetLength(), y.GetPointer(), y.GetPointer() + y.GetLength());

    rwRETURN(cmp < 0);
}
/*
 Compares the 2 strings for less than

 \param s The first string
 \param y The second string
 \return true if s is less than y
*/
inline bool operator<(const char16_t* s, const WString& y)
{
    rwFUNCTION();
    const size_t n = WSTRING_STRINGLENGTH(s);
    //lint -e{931} both sides have side effects
    int cmp = WString::Compare(s, s + n, y.GetPointer(), y.GetPointer() + y.GetLength());
    rwRETURN(cmp < 0);
}
/*
 Compares the 2 strings for less than

 \param x The first string
 \param s The second string
 \return true if x is less than s
*/
inline bool operator<(const WString& x, const char16_t* s)
{
    rwFUNCTION();
    size_t n = WSTRING_STRINGLENGTH(s);
    //lint -e{931} both sides have side effects
    int cmp = WString::Compare(x.GetPointer(), x.GetPointer() + x.GetLength(), s, s + n);
    rwRETURN(cmp < 0);
}
/*
 Compares the 2 strings for greater than

 \param x The first string
 \param y The second string
 \return true if x is greater than y
*/
inline bool operator>(const WString& x, const WString& y)
{
    rwFUNCTION();
    rwRETURN(y < x);
}
/*
 Compares the 2 strings for greater than

 \param s The first string
 \param y The second string
 \return true if s is greater than y
*/
inline bool operator>(const char16_t* s, const WString& y)
{
    rwFUNCTION();
    rwRETURN(y < s);
}
/*
 Compares the 2 strings for greater than

 \param x The first string
 \param s The second string
 \return true if x is greater than s
*/
inline bool operator>(const WString& x, const char16_t* s)
{
    rwFUNCTION();
    rwRETURN(s < x);
}
/*
 Compares the 2 strings for less than or equal

 \param x The first string
 \param y The second string
 \return true if x is less than or equal to y
*/
inline bool operator<=(const WString& x, const WString& y)
{
    rwFUNCTION();
    rwRETURN(!(y < x));
}
/*
 Compares the 2 strings for less than or equal

 \param s The first string
 \param y The second string
 \return true if s is less than or equal to y
*/
inline bool operator<=(const char16_t* s, const WString& y)
{
    rwFUNCTION();
    rwRETURN(!(y < s));
}
/*
 Compares the 2 strings for less than or equal

 \param x The first string
 \param s The second string
 \return true if x is less than or equal to s
*/
inline bool operator<=(const WString& x, const char16_t* s)
{
    rwFUNCTION();
    rwRETURN(!(s < x));
}
/*
 Compares the 2 strings for greater than or equal

 \param x The first string
 \param y The second string
 \return true if x is greater than or equal to y
*/
inline bool operator>=(const WString& x, const WString& y)
{
    rwFUNCTION();
    rwRETURN(!(x < y));
}
/*
 Compares the 2 strings for greater than or equal

 \param s The first string
 \param y The second string
 \return true if s is greater than or equal to y
*/
inline bool operator>=(const char16_t* s, const WString& y)
{
    rwFUNCTION();
    rwRETURN(!(s < y));
}
/*
 Compares the 2 strings for greater than or equal

 \param x The first string
 \param s The second string
 \return true if x is greater than or equal to s
*/
inline bool operator>=(const WString& x, const char16_t* s)
{
    rwFUNCTION();
    rwRETURN(!(x < s));
}

} // namespace core
} // namespace rw

#if defined(EA_PLATFORM_WINDOWS) || defined(EA_PLATFORM_XENON)
#pragma warning(pop)
#endif


#endif // RW_CORE_STRING_STRING_H
