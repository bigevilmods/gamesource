// (c) Electronic Arts. All Rights Reserved.

/// \file
/// \deprecated Use EASTL's string class instead.
///
/// String class definition

#ifndef RW_CORE_STRING_STRING_H
#define RW_CORE_STRING_STRING_H
#include <rw/core/base/ostypes.h>
#include <rw/core/resource/resourceallocator.h>

#include <EAStdC/EAMemory.h>
#include <EAStdC/EAString.h>

#include <rw/core/string/stringconfig.h>

namespace rw
{
namespace core
{
// forward declaration
class String;

/*
 String allocator interface class
 This has functions to allow the allocator to handle the length and capacity
 It also has a functions to allow the allocator to handle modification of the
 string so Ref-Counted strings can update their ref-counts
 The Assign and Compare functions will only be called if both strings are of the same
 type to allow optimizations.
*/
class IStringAllocator
{
public:
    virtual void        Modify(char8_t** p) = 0;
    virtual char8_t*    Allocate(size_t n) = 0;
    virtual void        Deallocate(char8_t* p) = 0;

    virtual uint32_t    GetAllocatorType() const = 0;

    virtual void        Assign(String& s1, char8_t** p1, const String& s2, char8_t* p2) = 0;
    virtual int32_t     Compare(const char8_t* s1, const char8_t* s2) = 0;

    virtual void        SetLength(char8_t *p, size_t n) = 0;
    virtual size_t      GetLength(const char8_t *p) const = 0;
    virtual size_t      GetCapacity(const char8_t *p) const = 0;

    // default constructor & destructor
    IStringAllocator() {}
    virtual ~IStringAllocator() {}
};
/*
 String class
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
class String
{
public:
    /*
    The types of strings
    */
    enum StringTypes
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
     CtorBuffer exists so that we can create a constructor that provides a preallocated
     buffer to use
    */
    struct CtorBuffer{};

    // Constructors
    explicit String();
    explicit String(IStringAllocator *a); // C++ basic_string uses explicit, so we follow suit.

    // use the provided buffer instead of allocating
    String(CtorBuffer unused, char8_t* p, IStringAllocator *a);

    String(const String& s);
    String(const String& s, IStringAllocator *a);

    String(const String& s, size_t pos, size_t n = npos);
    String(const String& s, size_t pos, size_t n, IStringAllocator *a);

    String(const char8_t* p, size_t n);
    String(const char8_t* p, size_t n, IStringAllocator* a);

    //lint -e{1931} although this can be used for implicit conversion,
    //we need to leave the explicit out because othersize the "assign a
    //char8_t to a String operator" won't compile.
    String(const char8_t* p);
    String(const char8_t* p, IStringAllocator *a);

    String(size_t n, char8_t c);
    String(size_t n, char8_t c, IStringAllocator *a);

    String(const char8_t* pBegin, const char8_t* pEnd);
    String(const char8_t* pBegin, const char8_t* pEnd, IStringAllocator *a);

    String(CtorDoNotInitialize unused, size_t n); //This is not a standard C++ basic_string constructor.
    String(CtorDoNotInitialize unused, size_t n, IStringAllocator *a);

    //lint -e{1916} ellipsis encountered
    String(CtorSprintf unused, const char8_t* pFormat, ...); //This is not a standard C++ basic_string constructor.

    //lint -e{1916} ellipsis encountered
    String(CtorSprintf unused, IStringAllocator *a, const char8_t* pFormat, ...);

    void PreDerivedDtorBaseCleanUp();
    
    // Destructor
    ~String();

    // Operator =
    String& operator=(const String& s);
    String& operator=(const char8_t* p);
    String& operator=(char8_t c);

    // Allocator functions
    const IStringAllocator* GetAllocator() const
    {
        rwFUNCTION();
        rwRETURN(mAllocator);
    }
    static void             SetDefaultAllocator(IStringAllocator *a)
    {
        rwFUNCTION();
        mDefaultAllocator = a;
        rwRETURNVOID();
    }

    /*
     Returns a constant pointer to the current string buffer

     \return pointer to the current string buffer
    */
    const char8_t* GetPointer() const
    {
        rwFUNCTION();
        rwRETURN(mString);
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
    char8_t operator[](size_t n) const;
    char8_t GetAt(size_t n) const;
    void    SetAt(size_t n, char8_t c);

    // Append operations
    String& operator+=(const String& s);
    String& operator+=(const char8_t* p);
    String& operator+=(char8_t c);
    String& Append(const String& s);
    String& Append(const String& s, size_t pos, size_t n);
    String& Append(const char8_t* p, size_t n);
    String& Append(const char8_t* p);
    String& Append(size_t n, char8_t c);
    String& Append(const char8_t* pBegin, const char8_t* pEnd);
    void    PushOnBack(char8_t c);
    void    PopFromBack();

    // Assignment operations
    String& Assign(const String& s);
    String& Assign(const String& s, size_t pos, size_t n);
    String& Assign(const char8_t* p, size_t n);
    String& Assign(const char8_t* p);
    String& Assign(size_t n, char8_t c);
    String& Assign(const char8_t* pBegin, const char8_t* pEnd);

    // Insertion operations
    String& Insert(size_t pos, const String& s);
    String& Insert(size_t pos, const String& s, size_t beg, size_t n);
    String& Insert(size_t pos, const char8_t* p, size_t n);
    String& Insert(size_t pos, const char8_t* p);
    String& Insert(size_t pos, size_t n, char8_t c);

    // Erase operations
    String& Erase(size_t pos = 0, size_t n = npos);

    // Replacement operations
    String& Replace(size_t pos, size_t n, const String& s);
    String& Replace(size_t pos1, size_t n1, const String& s, size_t pos2, size_t n2);
    String& Replace(size_t pos, size_t n1, const char8_t* p, size_t n2);
    String& Replace(size_t pos, size_t n1, const char8_t* p);
    String& Replace(size_t pos, size_t n1, size_t n2, char8_t c);

    // Find operations
    size_t Find(const String& s, size_t pos = 0) const;
    size_t Find(const char8_t* p, size_t pos = 0) const;
    size_t Find(const char8_t* p, size_t pos, size_t n) const;
    size_t Find(char8_t c, size_t pos = 0) const;

    // Reverse find operations
    size_t Rfind(const String& s, size_t pos = npos) const;
    size_t Rfind(const char8_t* p, size_t pos = npos) const;
    size_t Rfind(const char8_t* p, size_t pos, size_t n) const;
    size_t Rfind(char8_t c, size_t pos = npos) const;

    // Find first-of operations
    size_t FindFirstOf(const String& s, size_t pos = 0) const;
    size_t FindFirstOf(const char8_t* p, size_t pos = 0) const;
    size_t FindFirstOf(const char8_t* p, size_t pos, size_t n) const;
    size_t FindFirstOf(char8_t c, size_t pos = 0) const;

    // Find last-of operations
    size_t FindLastOf(const String& s, size_t pos = npos) const;
    size_t FindLastOf(const char8_t* p, size_t pos = npos) const;
    size_t FindLastOf(const char8_t* p, size_t pos, size_t n) const;
    size_t FindLastOf(char8_t c, size_t pos = npos) const;

    // Find first not-of operations
    size_t FindFirstNotOf(const String& s, size_t pos = 0) const;
    size_t FindFirstNotOf(const char8_t* p, size_t pos = 0) const;
    size_t FindFirstNotOf(const char8_t* p, size_t pos, size_t n) const;
    size_t FindFirstNotOf(char8_t c, size_t pos = 0) const;

    // Find last not-of operations
    size_t FindLastNotOf(const String& s,  size_t pos = npos) const;
    size_t FindLastNotOf(const char8_t* p, size_t pos = npos) const;
    size_t FindLastNotOf(const char8_t* p, size_t pos, size_t n) const;
    size_t FindLastNotOf(char8_t c, size_t pos = npos) const;

    // Substring functionality
    String Substr(size_t pos = 0, size_t n = npos) const;
    String Left(size_t n) const;
    String Right(size_t n) const;

    // Comparison operations
    int         Compare(const String& s) const;
    int         Compare(size_t pos1, size_t n1, const String& s) const;
    int         Compare(size_t pos1, size_t n1, const String& s, size_t pos2, size_t n2) const;
    int         Compare(const char8_t* p) const;
    int         Compare(size_t pos1, size_t n1, const char8_t* p) const;
    int         Compare(size_t pos1, size_t n1, const char8_t* p, size_t n2) const;
    static int  Compare(const char8_t* pBegin1, const char8_t* pEnd1, const char8_t* pBegin2, const char8_t* pEnd2);

    // returns the hash value for the string
    size_t    Hash() const;

    // converts the string to all upper/lower case
    void      MakeLower();
    void      MakeUpper();

    // sprintf functions
    String&   Sprintf_va_list(const char8_t* pFormat, va_list arglist);

    //lint -e{1916} ellipsis encountered
    String&   Sprintf(const char8_t* pFormat, ...);

    // UTF8 specific functions
    const char8_t*  GetBufferUTF8(size_t n) const;
    size_t          GetLengthUTF8() const;
    int32_t         GetAtUTF8(size_t n) const;
    void            SetAtUTF8(size_t n, int32_t c);
    String&         AppendUTF8(const char8_t* p, size_t n);
    String&         InsertUTF8(size_t pos, const char8_t* p, size_t n);
    size_t          FindUTF8(const char8_t* p, size_t pos = 0) const;
    String          SubstrUTF8(size_t pos = 0, size_t n = npos) const;
    void            MakeLowerUTF8();
    void            MakeUpperUTF8();

    // UTf8 helper functions
    static const char8_t*   GetBufferUTF8(const char8_t *p, size_t n);
    static size_t           GetLengthUTF8(const char8_t *p);
    static const char8_t*   ReadCharacterUTF8(const char8_t *p, int32_t *pCharacter);
    static int32_t          GetCharacterUTF8(const char8_t *pBuffer);
    static int32_t          GetCharacterSizeUTF8(const char8_t *p);
    static int32_t          GetCharacterSizeUTF8(const int32_t c);
    static void             SetCharacterUTF8(char8_t *p, int32_t c);

    // Used for empty strings. This allows us to not have to allocate memory for
    // empty strings and reduces null pointer checks.  Not const because it gets
    // assigned to non-const strings in the implementation.
    static char8_t *mEmpty;

private:
    // helper functions
    void RangeInitialize(const char8_t *p, size_t n);
    void SizeInitialize(size_t n, char8_t c);
    void Insert(char8_t *p, const char8_t* pBegin, const char8_t* pEnd);
    String& Erase(char8_t *pBegin, const char8_t *pEnd);
    String& Replace(char8_t *pBegin1, char8_t *pEnd1, const char8_t *pBegin2, const char8_t *pEnd2);

    // helper search functions
    static const char8_t* StringFind(const char8_t* pBegin, const char8_t* pEnd, const char8_t c);
    static const char8_t* StringFindEnd(const char8_t* pBegin, const char8_t* pEnd, char8_t c);
    static const char8_t* StringRFind(const char8_t* pRBegin, const char8_t* pREnd, const char8_t c);
    static const char8_t* StringSearch(const char8_t* p1Begin, const char8_t* p1End, const char8_t* p2Begin, const char8_t* p2End);
    static const char8_t* StringRSearch(const char8_t* p1Begin, const char8_t* p1End, const char8_t* p2Begin, const char8_t* p2End);
    static const char8_t* StringFindFirstOf(const char8_t* p1Begin, const char8_t* p1End, const char8_t* p2Begin, const char8_t* p2End);
    static const char8_t* StringRFindFirstOf(const char8_t* p1RBegin, const char8_t* p1REnd, const char8_t* p2Begin, const char8_t* p2End);
    static const char8_t* StringFindFirstNotOf(const char8_t* p1Begin, const char8_t* p1End, const char8_t* p2Begin, const char8_t* p2End);
    static const char8_t* StringRFindFirstNotOf(const char8_t* p1RBegin, const char8_t* p1REnd, const char8_t* p2Begin, const char8_t* p2End);

    // pointer to the allocator being used for the string
    IStringAllocator *mAllocator;

    // pointer to the string buffer, also passed to allocator for additional storage
    char8_t          *mString;

    // the default allocator of the string class
    static IStringAllocator *mDefaultAllocator;
};

// string operators
String operator+(const String& x,             const String& y);
String operator+(const char8_t* p,            const String& y);
String operator+(char8_t c,                   const String& y);
String operator+(const String& x,             const char8_t* p);
String operator+(const String& x,             const char8_t  c);

bool operator==(const String&  x,              const String&  y);
bool operator==(const char8_t* p,              const String&  y);
bool operator==(const String&  x,              const char8_t* p);
bool operator!=(const String&  x,              const String&  y);
bool operator!=(const char8_t* p,              const String&  y);
bool operator!=(const String&  x,              const char8_t* p);

bool operator< (const String&  x,           const String&  y);
bool operator< (const char8_t* p,           const String&  y);
bool operator< (const String&  x,           const char8_t* p);
bool operator> (const String&  x,           const String&  y);
bool operator> (const char8_t* p,           const String&  y);
bool operator> (const String&  x,           const char8_t* p);
bool operator<=(const String&  x,           const String&  y);
bool operator<=(const char8_t* p,           const String&  y);
bool operator<=(const String&  x,           const char8_t* p);
bool operator>=(const String&  x,           const String&  y);
bool operator>=(const char8_t* p,           const String&  y);
bool operator>=(const String&  x,           const char8_t* p);

/*
 Default string allocator class, dynamicly allocates all memory from the core allocator
 supplied in the constructor. There is 4 bytes of overhead allocated at the start of the
 string buffer to cache the capacity and length of the string
*/
class StringCoreAllocator : public IStringAllocator
{
public:
    explicit StringCoreAllocator(rw::IResourceAllocator *allocator);

    virtual void        Modify(char8_t** p);
    virtual char8_t*    Allocate(size_t n);
    virtual void        Deallocate(char8_t* p);

    virtual uint32_t    GetAllocatorType() const;

    virtual void        Assign(String& s1, char8_t** p1, const String& s2, char8_t* p2);
    virtual int32_t     Compare(const char8_t* s1, const char8_t* s2);

    virtual void        SetLength(char8_t *p, size_t n);
    virtual size_t      GetLength(const char8_t *p) const;
    virtual size_t      GetCapacity(const char8_t *p) const;

private:
    IResourceAllocator *mCoreAllocator;

    /*
     layout of data
     the mBuffer would be the actual size of the character string
     This would return a pointer to the mBuffer to be stored in the String::mString member
    */
    struct Data
    {
        uint16_t mCapacity;
        uint16_t mLength;
        char8_t  mBuffer[256];
    };
};
/*
 Ref-counted string allocator class, dynamicly allocates all memory from the core
 allocator supplied in the constructor. There is 6 bytes of overhead allocated at
 the start of the string buffer to cache the capacity and length of the string and to
 store the current reference count.
*/
class StringRefAllocator : public IStringAllocator
{
public:
    explicit StringRefAllocator(IResourceAllocator *allocator);

    virtual void        Modify(char8_t** p);
    virtual char8_t*    Allocate(size_t n);
    virtual void        Deallocate(char8_t* p);

    virtual uint32_t    GetAllocatorType() const;

    virtual void        Assign(String& s1, char8_t** p1, const String& s2, char8_t* p2);
    virtual int32_t     Compare(const char8_t* s1, const char8_t* s2);

    virtual void        SetLength(char8_t *p, size_t n);
    virtual size_t      GetLength(const char8_t *p) const;
    virtual size_t      GetCapacity(const char8_t *p) const;

private:
    IResourceAllocator *mCoreAllocator;

    /*
     layout of data
     the mBuffer would be the actual size of the character string
     This would return a pointer to the mBuffer to be stored in the String::mString member
    */
    struct Data
    {
        uint16_t mRefCount;
        uint16_t mCapacity;
        uint16_t mLength;
        char8_t  mBuffer[256];
    };
};
/*
 Const string allocator class, this is intended to be used for read-only strings
 the buffer to use is passed in the constructor, any calls to the
 Modify/Assign/Allocate/Deallocate functions will assert, GetLength and GetCapacity will
 return the string length
*/
class StringConstAllocator : public IStringAllocator
{
public:
    StringConstAllocator();

    virtual void        Modify(char8_t** p);
    virtual char8_t*    Allocate(size_t n);
    virtual void        Deallocate(char8_t* p);

    virtual uint32_t    GetAllocatorType() const;

    virtual void        Assign(String& s1, char8_t** p1, const String& s2, char8_t* p2);
    virtual int32_t     Compare(const char8_t* s1, const char8_t* s2);

    virtual void        SetLength(char8_t *p, size_t n);
    virtual size_t      GetLength(const char8_t *p) const;
    virtual size_t      GetCapacity(const char8_t *p) const;
};
/*
 Fixed string allocator class, this is a base class for the StringSizedAllocator template
 that will create an in-place buffer to use for the string. The Allocate/Deallocate
 functions will assert and the GetCapacity needs to be implemented by the template.
*/
class StringFixedAllocator : public IStringAllocator
{
public:
    virtual void        Modify(char8_t** p);
    virtual char8_t*    Allocate(size_t n);
    virtual void        Deallocate(char8_t* p);

    virtual uint32_t    GetAllocatorType() const;

    virtual void        Assign(String& s1, char8_t** p1, const String& s2, char8_t* p2);
    virtual int32_t     Compare(const char8_t* s1, const char8_t* s2);

    virtual void        SetLength(char8_t *p, size_t n);
    virtual size_t      GetLength(const char8_t *p) const;
    virtual size_t      GetCapacity(const char8_t *p) const;

protected:
    StringFixedAllocator();
};

// A template to provide a fixed allocator of a specificed size. This can be used to
// create a string allocator that has the buffer allocated on the stack or in a structure.
template<uint16_t _capacity>
class StringSizedAllocator : public StringFixedAllocator
{
public:
    StringSizedAllocator()
    : StringFixedAllocator()
    {
        rwFUNCTION();
        rwRETURNVOID();
    }

    virtual size_t GetCapacity(const char8_t*) const
    {
        rwFUNCTION();
        rwRETURN(_capacity);
    }
};

// A template to provide a fixed size string from the stack or in a structure.
// Usage would be: StringSized<256> aLongString;

//lint -e{1509} base class destructor for class 'String' is not virtual.
template<uint16_t _capacity>
class StringSized : public String
{
public:
    StringSized()
    : String(CtorBuffer(), mBuffer, &mSizedAllocator)
    {
        rwFUNCTION();
        *mBuffer = 0;
        rwRETURNVOID();
    }
    ~StringSized()
    {
        rwFUNCTION();
        PreDerivedDtorBaseCleanUp();
        rwRETURNVOID();
    }

private:
    StringSizedAllocator<_capacity> mSizedAllocator;
    char8_t mBuffer[_capacity];
};

/*
 Compares the 2 strings for equality

 \param x The first string
 \param y The second string

 \return true if the strings are the same, false otherwise
*/
inline bool operator==(const String& x, const String& y)
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
inline bool operator==(const char8_t* s, const String& y)
{
    rwFUNCTION();
    const size_t n(STRING_STRINGLENGTH(s));
    rwRETURN((n == y.GetLength()) && (STRING_MEMCOMPARE(s, y.GetPointer(), n) == 0));
}
/*
 Compares the 2 strings for equality

 \param x The first string
 \param s The second string

 \return true if the strings are the same, false otherwise
*/
inline bool operator==(const String& x, const char8_t* s)
{
    rwFUNCTION();
    const size_t n = STRING_STRINGLENGTH(s);
    rwRETURN((x.GetLength() == n) && (STRING_MEMCOMPARE(x.GetPointer(), s, n) == 0));
}
/*
 Compares the 2 strings for inequality

 \param x The first string
 \param y The second string

 \return true if the strings are different, false otherwise
*/
inline bool operator!=(const String& x, const String& y)
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
inline bool operator!=(const char8_t* s, const String& y)
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
inline bool operator!=(const String& x, const char8_t* s)
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
inline bool operator<(const String& x, const String& y)
{
    rwFUNCTION();
    //lint -e{931} both sides have side effects
    int cmp = String::Compare(x.GetPointer(), x.GetPointer() + x.GetLength(), y.GetPointer(), y.GetPointer() + y.GetLength());

    rwRETURN(cmp < 0);
}
/*
 Compares the 2 strings for less than

 \param s The first string
 \param y The second string

 \return true if s is less than y
*/
inline bool operator<(const char8_t* s, const String& y)
{
    rwFUNCTION();
    const size_t n = STRING_STRINGLENGTH(s);
    //lint -e{931} both sides have side effects
    int cmp = String::Compare(s, s + n, y.GetPointer(), y.GetPointer() + y.GetLength());
    rwRETURN(cmp < 0);
}
/*
 Compares the 2 strings for less than

 \param x The first string
 \param s The second string

 \return true if x is less than s
*/
inline bool operator<(const String& x, const char8_t* s)
{
    rwFUNCTION();
    size_t n = STRING_STRINGLENGTH(s);
    //lint -e{931} both sides have side effects
    int cmp = String::Compare(x.GetPointer(), x.GetPointer() + x.GetLength(), s, s + n);
    rwRETURN(cmp < 0);
}
/*
 Compares the 2 strings for greater than

 \param x The first string
 \param y The second string

 \return true if x is greater than y
*/
inline bool operator>(const String& x, const String& y)
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
inline bool operator>(const char8_t* s, const String& y)
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
inline bool operator>(const String& x, const char8_t* s)
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
inline bool operator<=(const String& x, const String& y)
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
inline bool operator<=(const char8_t* s, const String& y)
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
inline bool operator<=(const String& x, const char8_t* s)
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
inline bool operator>=(const String& x, const String& y)
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
inline bool operator>=(const char8_t* s, const String& y)
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
inline bool operator>=(const String& x, const char8_t* s)
{
    rwFUNCTION();
    rwRETURN(!(x < s));
}

} // namespace core
} // namespace rw

#endif // RW_CORE_STRING_STRING_H
