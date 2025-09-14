// (c) Electronic Arts. All Rights Reserved.

/// \file
/// \deprecated.  Use EASTL's String class intead of this one.
/// Implementation of C++ string streams for wide character streams

#include "rwsharedheaders.hpp"
#include <stdio.h>      // _vsnprintf
#include <ctype.h>      // toupper, tolower

#if defined(EA_PLATFORM_WINDOWS) || defined(EA_PLATFORM_XENON)
#pragma warning(push)
#pragma warning(disable:4996) // disable deprecated warnings against EAStdC memory functions.
#endif
#include <EAStdC/EAAlignment.h>
#include <EAStdC/EAMemory.h>
#include <EAStdC/EAString.h>
#include <EASTL/algorithm.h>
#include <EAStdC/EASprintf.h>


#include "rw/core/string/wstringconfig.h"
#include "rw/core/string/wstring.h"


#define GCCOPTIMIZE 0

//using namespace rw;
using namespace rw::core;

//lint -e{956, 1776} non-const, non-volatile static or external variable.  This is not const because it gets assigned to non-const strings in this module.
#ifdef rwHOST_PS2
// this is OK for GCC 2.95.3, but other compilers don't like it.
char16_t *WString::mEmpty = (const char16_t *)L"";
#else
char16_t *WString::mEmpty = (char16_t *)const_cast<wchar_t*>(L"");
#endif

//lint -e{956} non-const, non-volatile static or external variable.
IWStringAllocator *WString::mDefaultAllocator = NULL;
/**
 RW_WSTRING_INITIAL_CAPACITY
 As of this writing, this must be > 0.
*/
const size_t RW_WSTRING_INITIAL_CAPACITY = 16;

#if defined(RW_STRING_USE_EXCEPTIONS) && !defined(RW_COMPILER_NO_EXCEPTIONS)
   #include <stdexcept>       // std::out_of_range, std::length_error.
   #define RW_STRING_TRY try
   #define RW_STRING_THROW(x) throw x
   #define RW_STRING_CATCH(action) catch(...){ action; throw; }
#else
   #define RW_STRING_TRY
   #define RW_STRING_THROW(x)
   #define RW_STRING_CATCH(action)
#endif
/**
 utility function to verify if a character is UTF16
*/
inline bool UTF16_IsValid(const int32_t iCharacter)
{
    rwFUNCTION();
    if (iCharacter > 0x10FFFF)
    {
        rwRETURN(false);
    }
    rwRETURN(true);
}
/**
 CharWStringUninitializedCopy
 This is similar to STL's uninitialized_copy but can make the
 assumption that CharType variables don't need to be constructed,
 but merely assigned.
*/
inline char16_t* CharWStringUninitializedCopy(char16_t* pDestination, const char16_t* pSource, size_t n)
{
    rwFUNCTION();
   STRING_MEMMOVE(pDestination, pSource, 2*n); // two byte per char16_t type
   rwRETURN(pDestination + n);
}
/**
 CharWStringUninitializedFillN
 This is similar to STL's uninitialized_fill_n but can make the assumption that
 CharType variables don't need to be constructed, but merely assigned.
*/
inline char16_t* CharWStringUninitializedFillN(char16_t* pDestination, size_t n, char16_t c)
{
    rwFUNCTION();
    WSTRING_MEMFILL(pDestination, static_cast<uint8_t>(c), n*2); // Two bytes per each char16_t type
    rwRETURN(pDestination + n);
}

//void ThrowLengthException()
//{
//    RW_STRING_THROW(std::length_error("rw::core::WString"));
//}
//
//
//void ThrowRangeException()
//{
//    RW_STRING_THROW(std::out_of_range("rw::core::WString"));
//}
//
//
//void ThrowInvalidArgumentException()
//{
//    RW_STRING_THROW(std::invalid_argument("rw::core::WString"));
//}
/**
 Constructor for the WString class
*/
WString::WString()
: mAllocator(mDefaultAllocator), mWString(mEmpty)
{
    rwFUNCTION();
    rwASSERTM(mAllocator != NULL, ("WString::WString mAllocator is NULL"));
    rwRETURNVOID();
}
/**
 Constructor for the WString class
\param a The string allocator to use for this string
*/
WString::WString(IWStringAllocator *a)
: mAllocator(a), mWString(mEmpty)
{
    rwFUNCTION();
    if (mAllocator == NULL)
    {
        mAllocator = mDefaultAllocator;
        rwASSERTM(mAllocator != NULL, ("WString::WString mAllocator is NULL"));
    }
    rwRETURNVOID();
}
/**
 Constructor for the WString class
 \param p Pointer to a buffer to use for this string
 \param a The string allocator to use for this string
*/
WString::WString(CtorBuffer, char16_t* p, IWStringAllocator *a)
: mAllocator(a), mWString(p)
{
    rwFUNCTION();
    if (mAllocator == NULL)
    {
        mAllocator = mDefaultAllocator;
        rwASSERTM(mAllocator != NULL, ("WString::WString mAllocator is NULL"));
    }
    rwRETURNVOID();
}
/**
 Constructor for the WString class
 \param s A source string to copy into this string
*/
WString::WString(const WString& s)
: mAllocator(mDefaultAllocator), mWString(mEmpty)
{
    rwFUNCTION();
    rwASSERTM(mAllocator != NULL, ("WString::WString mAllocator is NULL"));

    RangeInitialize(s.GetPointer(), s.GetLength());
    rwRETURNVOID();
}
/**
 Constructor for the WString class

 \param s A source string to copy into this string
 \param a The string allocator to use for this string
*/
WString::WString(const WString& s, IWStringAllocator *a)
: mAllocator(a), mWString(mEmpty)
{
    rwFUNCTION();
    if (mAllocator == NULL)
    {
        mAllocator = mDefaultAllocator;
        rwASSERTM(mAllocator != NULL, ("WString::WString mAllocator is NULL"));
    }

    RangeInitialize(s.GetPointer(), s.GetLength());
    rwRETURNVOID();
}
/**
 Constructor for the WString class

 \param s A source string to copy into this string
 \param pos The start of the portion to copy
 \param n The size of the portion to copy
*/
WString::WString(const WString& s, size_t pos, size_t n)
: mAllocator(mDefaultAllocator), mWString(mEmpty)
{
    rwFUNCTION();
    rwASSERTM(mAllocator != NULL, ("WString::WString mAllocator is NULL"));
    rwASSERTM(pos <= s.mAllocator->GetLength(s.mWString),
        ("WString::WString called with a start position in the source string that is out of range"));
    RangeInitialize(s.GetPointer() + pos, eastl::min(n, s.mAllocator->GetLength(s.mWString) - pos));
    rwRETURNVOID();
}
/**
 Constructor for the WString class

 \param s A source string to copy into this string
 \param pos The start of the portion to copy
 \param n The size of the portion to copy
 \param a The string allocator to use for this string
*/
WString::WString(const WString& s, size_t pos, size_t n, IWStringAllocator *a)
: mAllocator(a), mWString(mEmpty)
{
    rwFUNCTION();
    if (mAllocator == NULL)
    {
        mAllocator = mDefaultAllocator;
        rwASSERTM(mAllocator != NULL, ("WString::WString mAllocator is NULL"));
    }

    rwASSERTM(pos <= s.mAllocator->GetLength(s.mWString),
        ("WString::WString called with a start position in the source string that is out of range"));

    RangeInitialize(s.GetPointer() + pos, eastl::min(n, s.mAllocator->GetLength(s.mWString) - pos));
    rwRETURNVOID();
}
/**
 Constructor for the WString class

 \param p A char buffer to copy into this string
 \param n The number of characters to use from the char buffer
*/
WString::WString(const char16_t* p, size_t n)
: mAllocator(mDefaultAllocator), mWString(mEmpty)
{
    rwFUNCTION();
    rwASSERTM(mAllocator != NULL, ("WString::WString mAllocator is NULL"));
    rwASSERTM(p != NULL, ("WString::WString called with an invalid buffer"));

    RangeInitialize(p, n);
    rwRETURNVOID();
}
/**
 Constructor for the WString class

 \param p A char buffer to copy into this string
 \param n The number of characters to use from the char buffer
 \param a The string allocator to use for this string
*/
WString::WString(const char16_t* p, size_t n, IWStringAllocator *a)
: mAllocator(a), mWString(mEmpty)
{
    rwFUNCTION();
    rwASSERTM(mAllocator != NULL, ("WString::WString mAllocator is NULL"));
    rwASSERTM(p != NULL, ("WString::WString called with an invalid buffer"));

    RangeInitialize(p, n);
    rwRETURNVOID();
}
/**
 Constructor for the WString class

 \param p A char buffer to copy into this string
*/
WString::WString(const char16_t* p)
: mAllocator(mDefaultAllocator), mWString(mEmpty)
{
    rwFUNCTION();
    rwASSERTM(mAllocator != NULL, ("WString::WString mAllocator is NULL"));
    rwASSERTM(p != NULL, ("WString::WString called with an invalid buffer"));

    RangeInitialize(p, WSTRING_STRINGLENGTH(p));
    rwRETURNVOID();
}
/**
 Constructor for the WString class

 \param p A char buffer to copy into this string
 \param a The string allocator to use for this string
*/
WString::WString(const char16_t* p, IWStringAllocator *a)
: mAllocator(a), mWString(mEmpty)
{
    rwFUNCTION();
    rwASSERTM(p != NULL, ("WString::WString called with an invalid buffer"));

    if (mAllocator == NULL)
    {
        mAllocator = mDefaultAllocator;
        rwASSERTM(mAllocator != NULL, ("WString::WString mAllocator is NULL"));
    }
    RangeInitialize(p, WSTRING_STRINGLENGTH(p));
    rwRETURNVOID();
}
/**
 Constructor for the WString class

 \param n The number of character to initialize the string to
 \param c A character to use
*/
WString::WString(size_t n, char16_t c)
: mAllocator(mDefaultAllocator), mWString(mEmpty)
{
    rwFUNCTION();
    rwASSERTM(mAllocator != NULL, ("WString::WString mAllocator is NULL"));
    SizeInitialize(n, c);
    rwRETURNVOID();
}
/**
 Constructor for the WString class

 \param n The number of character to initialize the string to
 \param c A character to use
 \param a The string allocator to use for this string
*/
WString::WString(size_t n, char16_t c, IWStringAllocator *a)
: mAllocator(a), mWString(mEmpty)
{
    rwFUNCTION();
    if (mAllocator == NULL)
    {
        mAllocator = mDefaultAllocator;
        rwASSERTM(mAllocator != NULL, ("WString::WString mAllocator is NULL"));
    }
    SizeInitialize(n, c);
    rwRETURNVOID();
}
/**
 Constructor for the WString class

 \param pBegin The start of a string to copy
 \param pEnd The end of the string
*/
WString::WString(const char16_t* pBegin, const char16_t* pEnd)
: mAllocator(mDefaultAllocator), mWString(mEmpty)
{
    rwFUNCTION();
    rwASSERTM(mAllocator != NULL, ("WString::WString mAllocator is NULL"));

    RangeInitialize(pBegin, static_cast<size_t>(pEnd - pBegin));
    rwRETURNVOID();
}
/**
 Constructor for the WString class

 \param pBegin The start of a string to copy
 \param pEnd The end of the string
 \param a The string allocator to use for this string
*/
WString::WString(const char16_t* pBegin, const char16_t* pEnd, IWStringAllocator *a)
: mAllocator(a), mWString(mEmpty)
{
    rwFUNCTION();
    if (mAllocator == NULL)
    {
        mAllocator = mDefaultAllocator;
        rwASSERTM(mAllocator != NULL, ("WString::WString mAllocator is NULL"));
    }
    RangeInitialize(pBegin, static_cast<size_t>(pEnd - pBegin));
    rwRETURNVOID();
}
/**
 Constructor for the WString class

 \param n The size of space to reserve
*/
WString::WString(CtorDoNotInitialize, size_t n)
: mAllocator(mDefaultAllocator), mWString(mEmpty)
{
    rwFUNCTION();
    rwASSERTM(mAllocator != NULL, ("WString::WString mAllocator is NULL"));
    rwASSERTM(n <= GetMaxSize(),
        ("WString::WString called with a space to reserve size that is larger than size_t - 2"));

    mWString = mAllocator->Allocate(n+1);
    mAllocator->SetLength(mWString, 0);
    rwRETURNVOID();
}
/**
 Constructor for the WString class

 \param n The size of space to reserve
 \param a The string allocator to use for this string
*/
WString::WString(CtorDoNotInitialize, size_t n, IWStringAllocator *a)
: mAllocator(a), mWString(mEmpty)
{
    rwFUNCTION();
    rwASSERTM(n <= GetMaxSize(),
        ("WString::WString called with a space to reserve size that is larger than size_t - 2"));

    if (mAllocator == NULL)
    {
        mAllocator = mDefaultAllocator;
        rwASSERTM(mAllocator != NULL, ("WString::WString mAllocator is NULL"));
    }
    mWString = mAllocator->Allocate(n+1);
    mAllocator->SetLength(mWString, 0);
    rwRETURNVOID();
}
/**
 Constructor for the WString class

 \param pFormat The sprintf style format string
 \param ... The sprintf parameters
*/
//lint -e{1916} ellipsis encountered
WString::WString(CtorSprintf, const char16_t* pFormat, ...)
: mAllocator(mDefaultAllocator), mWString(mEmpty)
{
    rwFUNCTION();
    rwASSERTM(mAllocator != NULL, ("WString::WString mAllocator is NULL"));

    mWString = mAllocator->Allocate(WSTRING_STRINGLENGTH(pFormat) + 1); // We'll need at least this much.

    va_list arguments;
    va_start(arguments, pFormat);
    Sprintf_va_list(pFormat, arguments);
    va_end(arguments);
    rwRETURNVOID();
}
/**
 Constructor for the WString class

 \param a The string allocator to use for this string
 \param pFormat The sprintf style format string
 \param ... The sprintf parameters
*/
//lint -e{1916} ellipsis encountered
WString::WString(CtorSprintf, IWStringAllocator *a, const char16_t* pFormat, ...)
: mAllocator(a), mWString(mEmpty)
{
    rwFUNCTION();
    if (mAllocator == NULL)
    {
        mAllocator = mDefaultAllocator;
        rwASSERTM(mAllocator != NULL, ("WString::WString mAllocator is NULL"));
    }

    mWString = mAllocator->Allocate(WSTRING_STRINGLENGTH(pFormat) + 1); // We'll need at least this much.

    va_list arguments;
    va_start(arguments, pFormat);
    Sprintf_va_list(pFormat, arguments);
    va_end(arguments);
    rwRETURNVOID();
}

void WString::PreDerivedDtorBaseCleanUp()
{
    rwFUNCTION();
    mAllocator->Deallocate(mWString);
    mWString = NULL;
    mAllocator = NULL;
    rwRETURNVOID();
}

/**
 Destructor for the WString class
*/
WString::~WString()
{
    if (mAllocator != NULL && mWString != NULL)
    {
        PreDerivedDtorBaseCleanUp();
    }
}
/**
 Assignment operator

 \param s The source string to assign
 \return A reference to the destination string
*/
WString& WString::operator=(const WString& s)
{
    rwFUNCTION();
    if (&s != this)
    {
        if (mAllocator->GetAllocatorType() == s.GetAllocator()->GetAllocatorType())
        {
            mAllocator->Assign(*this, &mWString, s, s.mWString);
        }
        else
        {
            Assign(s.mWString, s.GetLength());
        }
    }
    rwRETURN(*this);
}
/**
 Assignment operator

 \param p The source character buffer to assign
 \return A reference to the destination string
*/
WString& WString::operator=(const char16_t* p)
{
    rwFUNCTION();
    rwRETURN(Assign(p, WSTRING_STRINGLENGTH(p)));
}
/**
 Assignment operator

 \param c The source character to assign
 \return A reference to the destination string
*/
WString& WString::operator=(char16_t c)
{
    rwFUNCTION();
    rwRETURN(Assign(1, c));
}
/**
 Returns the capacity of the string

 \return The capacity of the string, does not include space for the NULL terminator
*/
size_t WString::GetCapacity() const
{
    rwFUNCTION();
    rwRETURN(mAllocator->GetCapacity(mWString));
}
/**
 Returns the length of the current string

 \return The string length in bytes, does not include the NULL terminator
*/
size_t WString::GetLength() const
{
    rwFUNCTION();
    rwRETURN(mAllocator->GetLength(mWString));
}

// Returns if the string is empty
//
// \return true if the string is empty, false otherwise
bool WString::IsEmpty() const
{
    rwFUNCTION();
    rwRETURN(*mWString == '\0');
}
/**
 Sets the capacity of the string. This will not allow the capacity to shrink smaller than
 the current string length.

 \param n The size of the capacity to use, does not include the NULL terminator
*/
void WString::SetCapacity(size_t n)
{
    rwFUNCTION();
    rwASSERTM(n <= GetMaxSize(),
        ("WString::SetCapacity called with a space to reserve size that is larger than size_t - 2"));

    n = eastl::max(n, mAllocator->GetLength(mWString)) + 1;
    
    if((n - 1) != mAllocator->GetCapacity(mWString))  //If there is something to do...
    {
        char16_t* pNewBegin = mAllocator->Allocate(n);
        char16_t* pNewEnd   = pNewBegin;

        pNewEnd = CharWStringUninitializedCopy(pNewBegin, mWString, mAllocator->GetLength(mWString));
        *pNewEnd = 0;

        size_t originalLength = mAllocator->GetLength(mWString);

        mAllocator->Deallocate(mWString);
        mWString      = pNewBegin;
        mAllocator->SetLength(mWString, originalLength);
    }
    rwRETURNVOID();
}
/**
 Clears out the string contents
*/
void WString::Clear()
{
    rwFUNCTION();
    if (!IsEmpty())
    {
        mAllocator->Modify(&mWString);
        *mWString = 0;
        mAllocator->SetLength(mWString, 0);
    }
    rwRETURNVOID();
}
/**
 Returns the character at the specified position

 \param n The index of the character to return
 \return The character
*/
char16_t WString::operator[](size_t n) const
{
    rwFUNCTION();
    rwASSERTM(n < mAllocator->GetLength(mWString),
        ("WString::operator[] called with an index out of range"));

    rwRETURN(mWString[n]);
}
/**
 Returns the character at the specified position

 \param n The index of the character to return
 \return The character
*/
char16_t WString::GetAt(size_t n) const
{
    rwFUNCTION();
    rwASSERTM(n < mAllocator->GetLength(mWString),
        ("WString::GetAt called with an index out of range"));

    rwRETURN(mWString[n]);
}
/**
 Sets the character at the specified position to a new value

 \param n The index of the character to set
 \param c The character to set
*/
void WString::SetAt(size_t n, char16_t c)
{
    rwFUNCTION();
    rwASSERTM(n < mAllocator->GetLength(mWString),
        ("WString::SetAt called with an index out of range"));

    mAllocator->Modify(&mWString);
    mWString[n] = c;
    if (c == '\0')
    {
        mAllocator->SetLength(mWString, n);
    }
    rwRETURNVOID();
}
/**
 Appends the parameters to the current string

 \param s The source string to append
 \return A reference to the new string
*/
WString& WString::operator+=(const WString& s)
{
    rwFUNCTION();
    mAllocator->Modify(&mWString);
    rwRETURN(Append(s.GetPointer(), s.mAllocator->GetLength(s.mWString)));
}
/**
 Appends the parameters to the current string

 \param p The source character buffer to append
 \return A reference to the new string
*/
WString& WString::operator+=(const char16_t* p)
{
    rwFUNCTION();
    mAllocator->Modify(&mWString);
    rwRETURN(Append(p, WSTRING_STRINGLENGTH(p)));
}
/**
 Appends the parameters to the current string

 \param c The source character to append
 \return A reference to the new string
*/
WString& WString::operator+=(char16_t c)
{
    rwFUNCTION();
    PushOnBack(c);
    rwRETURN(*this);
}
/**
 Appends the parameters to the current string

 \param s The source string to append
 \return A reference to the new string
*/
WString& WString::Append(const WString& s)
{
    rwFUNCTION();
    mAllocator->Modify(&mWString);
    rwRETURN(Append(s.mWString, s.mWString + s.mAllocator->GetLength(s.mWString)));
}
/**
 Appends the parameters to the current string

 \param s The source string to append
 \param pos The start position in the source string
 \param n The number of characters from the source string
 \return A reference to the new string
*/
WString& WString::Append(const WString& s, size_t pos, size_t n)
{
    rwFUNCTION();
    size_t len = s.mAllocator->GetLength(s.mWString);

    rwASSERTM(pos <= len,
        ("WString::Append called with a start position in the source string that is out of range"));

    mAllocator->Modify(&mWString);

    rwRETURN(Append(s.mWString + pos, s.mWString + pos + eastl::min(n, len - pos)));
}
/**
 Appends the parameters to the current string

 \param p The source character buffer to append
 \param n The number of characters from the source buffer
 \return A reference to the new string
*/
WString& WString::Append(const char16_t* p, size_t n)
{
    rwFUNCTION();
    mAllocator->Modify(&mWString);
    rwRETURN(Append(p, p + n));
}
/**
 Appends the parameters to the current string

 \param p The source character buffer to append
 \return A reference to the new string
*/
WString& WString::Append(const char16_t* p)
{
    rwFUNCTION();
    mAllocator->Modify(&mWString);
    rwRETURN(Append(p, p + WSTRING_STRINGLENGTH(p)));
}
/**
 Appends the parameters to the current string

 \param n The number of characters to append
 \param c The character to append
 \return A reference to the new string
*/
WString& WString::Append(size_t n, char16_t c)
{
    rwFUNCTION();
    const size_t s = mAllocator->GetLength(mWString);

    rwASSERTM((n <= GetMaxSize()) || (s <= (GetMaxSize() - n)),
        ("WString::Append size param or destination string cannot exceed size_t - 2"));

    if (n > 0)
    {
        mAllocator->Modify(&mWString);
    }

    if ((s + n) > mAllocator->GetCapacity(mWString))
    {
        SetCapacity(s + eastl::max(s, n));
    }

    if (n > 0)
    {
        CharWStringUninitializedFillN(mWString + mAllocator->GetLength(mWString), n, c);
        mWString[s+n] = '\0';
        mAllocator->SetLength(mWString, s+n);
    }
    rwRETURN(*this);
}
/**
 Appends the parameters to the current string

 \param pBegin The start of the source string to append
 \param pEnd The end of the source string to append
 \return A reference to the new string
*/
WString& WString::Append(const char16_t* pBegin, const char16_t* pEnd)
{
    rwFUNCTION();
    if (pBegin != pEnd)
    {
        mAllocator->Modify(&mWString);
        const size_t nOldSize(mAllocator->GetLength(mWString));
        const size_t n(static_cast<size_t>(pEnd - pBegin));

        rwASSERTM((n <= GetMaxSize()) || (nOldSize < (GetMaxSize() - n)),
            ("WString::Append source string or destination string cannot exceed size_t - 2"));

        if ((nOldSize + n) > mAllocator->GetCapacity(mWString))
        {
            const size_t nLength = nOldSize + eastl::max(nOldSize, n) + 1;
            char16_t *pNewBegin = mAllocator->Allocate(nLength);
            char16_t *pNewEnd   = pNewBegin;

            pNewEnd = CharWStringUninitializedCopy(pNewBegin, mWString, nOldSize);
            pNewEnd = CharWStringUninitializedCopy(pNewEnd, pBegin,  n);
            *pNewEnd = 0;

            mAllocator->Deallocate(mWString);
            mWString      = pNewBegin;
            mAllocator->SetLength(mWString, static_cast<size_t>(pNewEnd - mWString));
        }
        else
        {
            char16_t *mpEnd = mWString + nOldSize;
            CharWStringUninitializedCopy(mpEnd, pBegin, n);
            mWString[nOldSize + n] = 0;
            mAllocator->SetLength(mWString, nOldSize + n);
        }
    }

    rwRETURN(*this);
}
/**
 Appends the character to the end of the string

 \param c The source string to append
*/
void WString::PushOnBack(char16_t c)
{
    rwFUNCTION();
    mAllocator->Modify(&mWString);
    const size_t s = mAllocator->GetLength(mWString);
    if ((s + 1) > mAllocator->GetCapacity(mWString))
    {
        SetCapacity(s + eastl::max(s, static_cast<size_t>(1)));
    }
    mWString[s] = c;
    mWString[s+1] = 0;
    mAllocator->SetLength(mWString, s+1);
    rwRETURNVOID();
}
/**
 Removes the last character from the end of the string
*/
void WString::PopFromBack()
{
    rwFUNCTION();
    mAllocator->Modify(&mWString);
    const size_t s = mAllocator->GetLength(mWString);
    mWString[s-1] = 0;
    mAllocator->SetLength(mWString, s-1);
    rwRETURNVOID();
}
/**
 Assigns the current string based on the parameters

 \param s The source string to assign
 \return A reference to the destination string
*/
WString& WString::Assign(const WString& s)
{
    rwFUNCTION();
    mAllocator->Modify(&mWString);
    if (mAllocator->GetAllocatorType() == s.mAllocator->GetAllocatorType())
    {
        mAllocator->Assign(*this, &mWString, s, s.mWString);
        rwRETURN(*this);
    }
    else
    {
        rwRETURN(Assign(s.mWString, s.mWString + s.mAllocator->GetLength(s.mWString)));
    }
}
/**
 Assigns the current string based on the parameters

 \param s The source string to assign
 \param pos The starting position in the string
 \param n The number of characters to copy
 \return A reference to the destination string
*/
WString& WString::Assign(const WString& s, size_t pos, size_t n)
{
    rwFUNCTION();
    rwASSERTM(pos <= s.mAllocator->GetLength(s.mWString),
        ("WString::Assign called with a start position in the source string that is out of range"));

    mAllocator->Modify(&mWString);
    rwRETURN(Assign(s.mWString + pos, s.mWString + pos + eastl::min(n, s.mAllocator->GetLength(s.mWString) - pos)));
}
/**
 Assigns the current string based on the parameters

 \param p The source character bugger to assign
 \param n The number of characters to copy
 \return A reference to the destination string
*/
WString& WString::Assign(const char16_t* p, size_t n)
{
    rwFUNCTION();
    mAllocator->Modify(&mWString);
    rwRETURN(Assign(p, p + n));
}
/**
 Assigns the current string based on the parameters

 \param p The source character bugger to assign
 \return A reference to the destination string
*/
WString& WString::Assign(const char16_t* p)
{
    rwFUNCTION();
    mAllocator->Modify(&mWString);
    rwRETURN(Assign(p, p + WSTRING_STRINGLENGTH(p)));
}
/**
 Assigns the current string based on the parameters

 \param n The number of characters to assign
 \param c The character to assign
 \return A reference to the destination string
*/
WString& WString::Assign(size_t n, char16_t c)
{
    rwFUNCTION();
    mAllocator->Modify(&mWString);
    const size_t s = mAllocator->GetLength(mWString);
    if (n <= mAllocator->GetCapacity(mWString))
    {
        WSTRING_MEMFILL(mWString, static_cast<uint8_t>(c), n*2); // Two bytes per each char16_t type
        Erase(mWString + n, mWString + s);
    }
    else
    {
        WSTRING_MEMFILL(mWString, static_cast<uint8_t>(c), s*2); // Two bytes per each char16_t type
        Append(n - s, c);
    }
    rwRETURN(*this);
}
/**
 Assigns the current string based on the parameters

 \param pBegin The start of the character buffer to assign
 \param pEnd The end of the character buffer to assign
 \return A reference to the destination string
*/
WString& WString::Assign(const char16_t* pBegin, const char16_t* pEnd)
{
    rwFUNCTION();
    mAllocator->Modify(&mWString);
    const size_t n = static_cast<size_t>(pEnd - pBegin);
    size_t len = mAllocator->GetLength(mWString);
    if (static_cast<size_t>(n) <= len)
    {
        STRING_MEMCOPY(mWString, pBegin, n*2 ); // two bytes per char16_t type
        Erase(mWString + n, mWString + len);
    }
    else
    {
        STRING_MEMCOPY(mWString, pBegin, len*2); // two bytes per char16_t type
        Append(pBegin + len, pEnd);
    }
    rwRETURN(*this);
}
/**
 Inserts the parameters into the current string

 \param pos Position in the current string
 \param s The string to insert
 \return A reference to the destination string
*/
WString& WString::Insert(size_t pos, const WString& s)
{
    rwFUNCTION();
    rwASSERTM(pos <= mAllocator->GetLength(mWString),
        ("WString::Insert called with a start position in the current string that is out of range"));
    rwASSERTM(mAllocator->GetLength(mWString) <= (GetMaxSize() - s.mAllocator->GetLength(s.mWString)),
        ("WString::Insert destination string cannot exceed size_t - 2"));

    mAllocator->Modify(&mWString);
    Insert(mWString + pos, s.mWString, s.mWString + s.mAllocator->GetLength(s.mWString));
    rwRETURN(*this);
}
/**
 Inserts the parameters into the current string

 \param pos Position in the current string
 \param s The string to insert
 \param beg The start location in the source string
 \param n The number of characters in the source string
 \return A reference to the destination string
*/
WString& WString::Insert(size_t pos, const WString& s, size_t beg, size_t n)
{
    rwFUNCTION();
    rwASSERTM((pos <= mAllocator->GetLength(mWString)) || (beg <= s.mAllocator->GetLength(s.mWString)),
        ("WString::Insert position in current string, or start location in source string is out of range"));

    size_t nLength = eastl::min(n, s.mAllocator->GetLength(s.mWString) - beg);

    rwASSERTM(mAllocator->GetLength(mWString) <= (GetMaxSize() - nLength),
        ("WString::Insert destination string cannot exceed size_t - 2"));

    mAllocator->Modify(&mWString);
    Insert(mWString + pos, s.mWString + beg, s.mWString + beg + nLength);
    rwRETURN(*this);
}
/**
 Inserts the parameters into the current string

 \param pos Position in the current string
 \param p The character buffer to insert
 \param n The number of characters in the source buffer
 \return A reference to the destination string
*/
WString& WString::Insert(size_t pos, const char16_t* p, size_t n)
{
    rwFUNCTION();
    rwASSERTM(pos <= mAllocator->GetLength(mWString),
        ("WString::Insert position in current string is out of range"));

    rwASSERTM(mAllocator->GetLength(mWString) <= (GetMaxSize() - n),
        ("WString::Insert destination string cannot exceed size_t - 2"));

    mAllocator->Modify(&mWString);
    Insert(mWString + pos, p, p + n);
    rwRETURN(*this);
}
/**
 Inserts the parameters into the current string

 \param pos Position in the current string
 \param p The character buffer to insert
 \return A reference to the destination string
*/
WString& WString::Insert(size_t pos, const char16_t* p)
{
    rwFUNCTION();
    rwASSERTM(pos <= mAllocator->GetLength(mWString),
        ("WString::Insert position in current string is out of range"));

    size_t nLength = WSTRING_STRINGLENGTH(p);

    rwASSERTM(mAllocator->GetLength(mWString) <= (GetMaxSize() - nLength),
        ("WString::Insert destination string cannot exceed size_t - 2"));

    mAllocator->Modify(&mWString);
    Insert(mWString + pos, p, p + nLength);
    rwRETURN(*this);
}
/**
 Inserts the parameters into the current string

 \param pos Position in the current string
 \param n The number of characters to insert
 \param c The character to insert
 \return A reference to the destination string
*/
WString& WString::Insert(size_t pos, size_t n, char16_t c)
{
    rwFUNCTION();
    if (n != 0)
    {
        mAllocator->Modify(&mWString);

        char16_t *p = mWString + pos;
        const size_t len = mAllocator->GetLength(mWString);
        char16_t *mpEnd = mWString + len;
        if ((mAllocator->GetCapacity(mWString) - len) >= (n + 1))
        {
            const size_t nElementsAfter = static_cast<size_t>(mpEnd - p);
            const char16_t *pOldEnd = mpEnd;

            if (nElementsAfter >= n)
            {
                CharWStringUninitializedCopy(mpEnd + 1, (mpEnd - n) + 1, n);
                mpEnd += n;
                STRING_MEMMOVE(p + n, p, ((nElementsAfter - n) + 1)*2); // two bytes per char16_t type
                WSTRING_MEMFILL(p, static_cast<uint8_t>(c), n*2); // two bytes per char16_t type
            }
            else
            {
                CharWStringUninitializedFillN(mpEnd + 1, (n - nElementsAfter) - 1, c);
                mpEnd += n - nElementsAfter;
                RW_STRING_TRY
                {
                    CharWStringUninitializedCopy(mpEnd, p, static_cast<size_t>((pOldEnd + 1) - p));
                    mpEnd += nElementsAfter;
                }
                RW_STRING_CATCH(mpEnd = pOldEnd);
                WSTRING_MEMFILL(p, static_cast<uint8_t>(c), (nElementsAfter + 1)*2); // Two bytes per each char16_t type
            }
            mAllocator->SetLength(mWString, static_cast<size_t>(mpEnd - mWString));
        }
        else
        {
            const size_t nOldSize = mAllocator->GetLength(mWString);
            const size_t nLength = nOldSize + eastl::max(nOldSize, n) + 1;
            char16_t *pNewBegin = mAllocator->Allocate(nLength);
            char16_t *pNewEnd   = pNewBegin;

            pNewEnd = CharWStringUninitializedCopy(pNewBegin, mWString, static_cast<size_t>(p - mWString));
            pNewEnd = CharWStringUninitializedFillN(pNewEnd, n, c);
            pNewEnd = CharWStringUninitializedCopy(pNewEnd, p, static_cast<size_t>(mpEnd - p));
            *pNewEnd = 0;

            mAllocator->Deallocate(mWString);
            mWString      = pNewBegin;
            mAllocator->SetLength(mWString, static_cast<size_t>(pNewEnd - mWString));
        }
    }
    rwRETURN(*this);
}
/**
 Inserts the parameters into the current string

 \param p The location in the current string to insert
 \param pBegin The start of the source character buffer
 \param pEnd The end of the source character buffer
*/
void WString::Insert(char16_t *p, const char16_t* pBegin, const char16_t* pEnd)
{
    rwFUNCTION();
    const size_t n = static_cast<size_t>(pEnd - pBegin);

    if (n != 0)
    {
        mAllocator->Modify(&mWString);

        const size_t len = mAllocator->GetLength(mWString);
        char16_t *mpEnd = mWString + len;
        if (((mAllocator->GetCapacity(mWString) - len) >= (n + 1)) && ((pBegin > mpEnd) || (pEnd < mWString))) // If we don't need to reallocate...
        {
            const size_t nElementsAfter = static_cast<size_t>(mpEnd - p);
            const char16_t *pOldEnd = mpEnd;

            if (nElementsAfter >= n) // If the newly inserted characters entirely fit within the size of the original string...
            {
                STRING_MEMMOVE(mpEnd + 1, (mpEnd - n) + 1, n*2); // two bytes per char16_t type
                mpEnd += n;
                STRING_MEMMOVE(p + n, p, ((nElementsAfter - n) + 1)*2); // two bytes per char16_t type
                STRING_MEMCOPY(p, pBegin, static_cast<size_t>(pEnd - pBegin)*2); // two bytes per char16_t type
            }
            else
            {
                const char16_t* pMid = pBegin;

                pMid += (nElementsAfter + 1);
                STRING_MEMMOVE(mpEnd + 1, pMid, static_cast<size_t>(pEnd - pMid)*2); // two bytes per char16_t type
                mpEnd += n - nElementsAfter;
                RW_STRING_TRY
                {
                   STRING_MEMMOVE(mpEnd, p, static_cast<size_t>((pOldEnd - p) + 1)*2); // two bytes per char16_t type
                   mpEnd += nElementsAfter;
                }
                RW_STRING_CATCH(mpEnd = pOldEnd);
                STRING_MEMCOPY(p, pBegin, static_cast<size_t>(pMid - pBegin)*2); // two bytes per char16_t type
            }
            mAllocator->SetLength(mWString, static_cast<size_t>(mpEnd - mWString));
        }
        else // Else we need to reallocate to implement this.
        {
            const size_t nOldSize = len;
            const size_t nLength  = nOldSize + eastl::max(nOldSize, static_cast<size_t>(n)) + 1; // We grow the capacity by an amount possibly greater than needed.
            char16_t *pNewBegin       = mAllocator->Allocate(nLength);
            char16_t *pNewEnd         = pNewBegin;

            pNewEnd = CharWStringUninitializedCopy(pNewBegin, mWString, static_cast<size_t>(p - mWString));
            pNewEnd = CharWStringUninitializedCopy(pNewEnd,   pBegin,  static_cast<size_t>(pEnd - pBegin));
            pNewEnd = CharWStringUninitializedCopy(pNewEnd,   p,       static_cast<size_t>(mpEnd - p));
            *pNewEnd = 0;

            mAllocator->Deallocate(mWString);
            mWString      = pNewBegin;
            mAllocator->SetLength(mWString, static_cast<size_t>(pNewEnd - mWString));
        }
    }
    rwRETURNVOID();
}
/**
 Erases a portion of the current string, the end of the string will be moved forward if a
 portion of the middle of the string is erased.

 \param pos Position in the current string to start erasing
 \param n The number of characters to erase
 \return A reference to the string
*/
WString& WString::Erase(size_t pos, size_t n)
{
    rwFUNCTION();
    rwASSERTM(pos <= mAllocator->GetLength(mWString),
        ("WString::Erase position in current string is out of range"));

    mAllocator->Modify(&mWString);
    Erase(mWString + pos, mWString + pos + eastl::min(n, mAllocator->GetLength(mWString) - pos));
    rwRETURN(*this);
}
/**
 Erases a portion of the current string, the end of the string will be moved forward if a
 portion of the middle of the string is erased.

 \param pBegin The start of the string to erase
 \param pEnd The end of the string to erase
 \return A reference to the string
*/
WString& WString::Erase(char16_t *pBegin, const char16_t *pEnd)
{
    rwFUNCTION();
    if (pBegin != pEnd)
    {
        mAllocator->Modify(&mWString);

        const char16_t *mpEnd = mWString + mAllocator->GetLength(mWString);
        STRING_MEMMOVE(pBegin, pEnd, static_cast<size_t>((mpEnd - pEnd) + 1)*2); // two bytes per char16_t type
        const char16_t *pNewEnd = (mpEnd - (pEnd - pBegin));
        mAllocator->SetLength(mWString, static_cast<size_t>(pNewEnd - mWString));
    }
   rwRETURN(*this);
}
/**
 Replaces a portion of the current string with something new

 \param pos Position in the current string to replace
 \param n The number of characters to replace
 \param s The source string
 \return A reference to the string
*/
WString& WString::Replace(size_t pos, size_t n, const WString& s)
{
    rwFUNCTION();
    rwASSERTM(pos <= mAllocator->GetLength(mWString),
        ("WString::Replace position in current string is out of range"));

    const size_t nLength = eastl::min(n, mAllocator->GetLength(mWString) - pos);

    rwASSERTM(mAllocator->GetLength(mWString) <= (GetMaxSize() - nLength),
        ("WString::Replace destination string cannot exceed size_t - 2"));

    mAllocator->Modify(&mWString);
    rwRETURN(Replace(mWString + pos, mWString + pos + nLength, s.mWString, s.mWString + s.mAllocator->GetLength(s.mWString)));
}
/**
 Replaces a portion of the current string with something new

 \param pos1 Position in the current string to replace
 \param n1 The number of characters to replace
 \param s The source string
 \param pos2 Position in the source string to start
 \param n2 The number of characters from the source string
 \return A reference to the new string
*/
WString& WString::Replace(size_t pos1, size_t n1, const WString& s, size_t pos2, size_t n2)
{
    rwFUNCTION();
    rwASSERTM((pos1 <= mAllocator->GetLength(mWString)) || (pos2 <= s.mAllocator->GetLength(s.mWString)),
        ("WString::Replace position in current string, or start location in source string is out of range"));

    const size_t nLength1 = eastl::min(n1, mAllocator->GetLength(mWString)   - pos1);
    const size_t nLength2 = eastl::min(n2, s.mAllocator->GetLength(s.mWString) - pos2);

    rwASSERTM((mAllocator->GetLength(mWString) - nLength1) < (GetMaxSize() - nLength2),
        ("WString::Replace destination string cannot exceed size_t - 2"));

    mAllocator->Modify(&mWString);
    rwRETURN(Replace(mWString + pos1, mWString + pos1 + nLength1, s.mWString + pos2, s.mWString + pos2 + nLength2));
}
/**
 Replaces a portion of the current string with something new

 \param pos Position in the current string to replace
 \param n1 The number of characters to replace
 \param p The source character buffer
 \param n2 The number of characters from the source buffer
 \return A reference to the new string
*/
WString& WString::Replace(size_t pos, size_t n1, const char16_t* p, size_t n2)
{
    rwFUNCTION();
    rwASSERTM(pos <= mAllocator->GetLength(mWString),
        ("WString::Replace position in current string is out of range"));
    
    const size_t nLength = eastl::min(n1, mAllocator->GetLength(mWString) - pos);

    rwASSERTM((n2 <= GetMaxSize()) || ((mAllocator->GetLength(mWString) - nLength) < (GetMaxSize() - n2)),
        ("WString::Replace source string or destination string cannot exceed size_t - 2"));

    mAllocator->Modify(&mWString);
    rwRETURN(Replace(mWString + pos, mWString + pos + nLength, p, p + n2));
}
/**
 Replaces a portion of the current string with something new

 \param pos Position in the current string to replace
 \param n1 The number of characters to replace
 \param p The source character buffer
 \return A reference to the new string
*/
WString& WString::Replace(size_t pos, size_t n1, const char16_t* p)
{
    rwFUNCTION();
    rwASSERTM(pos <= mAllocator->GetLength(mWString),
        ("WString::Replace position in current string is out of range"));

    const size_t nLength = eastl::min(n1, mAllocator->GetLength(mWString) - pos);

    rwASSERTM((WSTRING_STRINGLENGTH(p) <= GetMaxSize()) ||
        ((mAllocator->GetLength(mWString) - nLength) < (GetMaxSize() - WSTRING_STRINGLENGTH(p))),
        ("WString::Replace source string or destination string cannot exceed size_t - 2"));

    mAllocator->Modify(&mWString);
    rwRETURN(Replace(mWString + pos, mWString + pos + nLength, p, p + WSTRING_STRINGLENGTH(p)));
}
/**
 Replaces a portion of the current string with something new

 \param pos Position in the current string to replace
 \param n1 The number of characters to replace
 \param n2 The number of source characters to insert
 \param c The source character
 \return A reference to the new string
*/
WString& WString::Replace(size_t pos, size_t n1, size_t n2, char16_t c)
{
    rwFUNCTION();
    rwASSERTM(pos <= mAllocator->GetLength(mWString),
        ("WString::Replace position in current string is out of range"));

    const size_t nLength = eastl::min(n1, mAllocator->GetLength(mWString) - pos);

    rwASSERTM((n2 <= GetMaxSize()) || ((mAllocator->GetLength(mWString) - nLength) < (GetMaxSize() - n2)),
        ("WString::Replace source string or destination string cannot exceed size_t - 2"));

    mAllocator->Modify(&mWString);

    char16_t       *pBegin = mWString + pos;
    const char16_t *pEnd = mWString + pos + nLength;

    if(nLength >= n2)
    {
        WSTRING_MEMFILL(pBegin, static_cast<uint8_t>(c), n2 *2); // Two bytes per each char16_t type
        Erase(pBegin + n2, pEnd);
    }
    else
    {
        WSTRING_MEMFILL(pBegin, static_cast<uint8_t>(c), nLength *2); // Two bytes per each char16_t type
        Insert(static_cast<size_t>(pEnd - mWString), n2 - nLength, c);
    }
    rwRETURN(*this);

}
/**
 Replaces a portion of the current string with something new

 \param pBegin1 The start pointer of the characters to be replaced
 \param pEnd1 The end pointer of the characters to be replaced
 \param pBegin2 The start pointer for the source character buffer
 \param pEnd2 The end pointer for the source character buffer
 \return A reference to the new string
*/
WString& WString::Replace(char16_t *pBegin1, char16_t *pEnd1, const char16_t *pBegin2, const char16_t *pEnd2)
{
    rwFUNCTION();
    mAllocator->Modify(&mWString);

    const size_t nLength1 = static_cast<size_t>(pEnd1 - pBegin1);
    const size_t nLength2 = static_cast<size_t>(pEnd2 - pBegin2);

    if (nLength1 >= nLength2) // If we have a non-expanding operation...
    {
        if ((pBegin2 > pEnd1) || (pEnd2 <= pBegin1))  // If we have a non-overlapping operation...
        {
            STRING_MEMCOPY(pBegin1, pBegin2, static_cast<size_t>(pEnd2 - pBegin2)*2);// two bytes per char16_t type
        }
        else
        {
            STRING_MEMMOVE(pBegin1, pBegin2, static_cast<size_t>(pEnd2 - pBegin2)*2); // two bytes per char16_t type
        }
        Erase(pBegin1 + nLength2, pEnd1);
    }
    else // Else we are expanding.
    {
        if ((pBegin2 > pEnd1) || (pEnd2 <= pBegin1)) // If we have a non-overlapping operation...
        {
            const char16_t* const pMid2 = pBegin2 + nLength1;

            if ((pEnd2 <= pBegin1) || (pBegin2 > pEnd1))
            {
                STRING_MEMCOPY(pBegin1, pBegin2, static_cast<size_t>(pMid2 - pBegin2)*2); // two bytes per char16_t type
            }
            else
            {
                STRING_MEMMOVE(pBegin1, pBegin2, static_cast<size_t>(pMid2 - pBegin2)*2); // two bytes per char16_t type
            }
            Insert(pEnd1, pMid2, pEnd2);
        }
        else // else we have an overlapping operation.
        {
            // I can't think of any easy way of doing this without allocating temporary memory.
            const size_t nOldSize     = mAllocator->GetLength(mWString);
            const size_t nNewCapacity = nOldSize + eastl::max(nOldSize, size_t(nLength2 - nLength1)) + 1; // We grow the capacity by an amount possibly greater than needed.
            const char16_t *mpEnd      = mWString + nOldSize;
            char16_t *pNewBegin        = mAllocator->Allocate(nNewCapacity);
            char16_t *pNewEnd          = pNewBegin;

            pNewEnd = CharWStringUninitializedCopy(pNewBegin, mWString, static_cast<size_t>(pBegin1 - mWString));
            pNewEnd = CharWStringUninitializedCopy(pNewEnd,   pBegin2, static_cast<size_t>(pEnd2 - pBegin2));
            pNewEnd = CharWStringUninitializedCopy(pNewEnd,   pEnd1,   static_cast<size_t>(mpEnd - pEnd1));
            *pNewEnd = 0;

            mAllocator->Deallocate(mWString);
            mWString = pNewBegin;
            mAllocator->SetLength(mWString, static_cast<size_t>(pNewEnd - pNewBegin));
        }
    }
    rwRETURN(*this);
}
/**
 Searches for a string in the current string

 \param s The string to search for
 \param pos The start location in the current string
 \return The index of the search string in the current string if it exists,
         otherwise WString::npos if not found
*/
size_t WString::Find(const WString& s, size_t pos) const
{
    rwFUNCTION();
    rwRETURN(Find(s.mWString, pos, s.mAllocator->GetLength(s.mWString)));
}
/**
 Searches for a string in the current string

 \param p The string to search for
 \param pos The start location in the current string
 \return The index of the search string in the current string if it exists,
         otherwise WString::npos if not found
*/
size_t WString::Find(const char16_t* p, size_t pos) const
{
    rwFUNCTION();
    rwRETURN(Find(p, pos, WSTRING_STRINGLENGTH(p)));
}
/**
 Searches for a string in the current string
\note Note that a search for a zero length string starting at position = end() returns end() and not npos.
 \param p The string to search for
 \param pos The start location in the current string
 \param n The number of characters in the search string
 \return The index of the search string in the current string if it exists,
         otherwise WString::npos if not found
*/
size_t WString::Find(const char16_t* p, size_t pos, size_t n) const
{
    rwFUNCTION();
    const size_t len = mAllocator->GetLength(mWString);
    if ((pos + n) > len)
    {
        rwRETURN(npos);
    }
    else
    {
        const char16_t* const pBegin  = mWString + pos;
        const char16_t *mpEnd = mWString + len;
        const char16_t* const pResult = WStringSearch(pBegin, mpEnd, p, p + n);

        if (static_cast<size_t>(mpEnd - pResult) >= n) // Don't check for simply 'if (mpEnd - pResult)', as it doesn't yield true when n == 0.
        {
            rwRETURN(static_cast<size_t>(pResult - mWString));
        }
        rwRETURN(npos);
    }
}
/**
 Searches for a character in the current string

 \param c The character to search for
 \param pos The start location in the current string
 \return The index of the search character in the current string if it exists,
         otherwise WString::npos if not found
*/
size_t WString::Find(char16_t c, size_t pos) const
{
    rwFUNCTION();
    if (pos >= mAllocator->GetLength(mWString))
    {
        rwRETURN(npos);
    }
    else
    {
        const char16_t* const pBegin  = mWString + pos;
        const char16_t *mpEnd = mWString + mAllocator->GetLength(mWString);
        const char16_t * pResult = WStringFind(pBegin, mpEnd, c);

        if (pResult != mpEnd)
        {
             rwRETURN(static_cast<size_t>(pResult - mWString));
        }
        rwRETURN(npos);
    }
}
/**
 Searches for a string in the current string, starting from the end

 \param s The string to search for
 \param pos The start location in the current string, starting from the end
 \return The index of the search string in the current string if it exists,
         otherwise WString::npos if not found
*/
size_t WString::Rfind(const WString& s, size_t pos) const
{
    rwFUNCTION();
    rwRETURN(Rfind(s.mWString, pos, s.mAllocator->GetLength(s.mWString)));
}
/**
 Searches for a string in the current string, starting from the end

 \param p The string to search for
 \param pos The start location in the current string, starting from the end
 \return The index of the search string in the current string if it exists,
         otherwise WString::npos if not found
*/
size_t WString::Rfind(const char16_t* p, size_t pos) const
{
    rwFUNCTION();
    rwRETURN(Rfind(p, pos, WSTRING_STRINGLENGTH(p)));
}
/**
 Searches for a string in the current string, starting from the end

 \param p The string to search for
 \param pos The start location in the current string, starting from the end
 \param n The number of characters in the search string

 \return The index of the search string in the current string if it exists,
         otherwise WString::npos if not found
*/
size_t WString::Rfind(const char16_t* p, size_t pos, size_t n) const
{
    rwFUNCTION();
    // Note that a search for a zero length string starting at position = end() returns end() and not npos.
    // Note by Paul Pedriana: I am not sure how this should behave in the case of n == 0 and pos > size.
    // The standard seems to suggest that rfind doesn't act exactly the same as find in that input pos
    // can be > size and the return value can still be other than npos. Thus, if n == 0 then you can
    // never return npos, unlike the case with find.
    const size_t nLength = mAllocator->GetLength(mWString);

    if (n > nLength)
    {
        rwRETURN(npos);
    }
    else if (n == 0)
    {
        rwRETURN(eastl::min(nLength, pos));
    }
    else
    {
        const char16_t * pEnd    = mWString + eastl::min(nLength - n, pos) + n;
        const char16_t * pResult = WStringRSearch(mWString, pEnd, p, p + n);

        if (pResult != pEnd)
        {
            rwRETURN(static_cast<size_t>(pResult - mWString));
        }
        rwRETURN(npos);
    }
}
/**
 Searches for a character in the current string, starting from the end

 \param c The character to search for
 \param pos The start location in the current string, starting from the end
 \return The index of the search character in the current string if it exists,
         otherwise WString::npos if not found
*/
size_t WString::Rfind(char16_t c, size_t pos) const
{
    rwFUNCTION();
    // If n is zero or pos is >= size, we return npos.
    const size_t nLength = mAllocator->GetLength(mWString);

    if (nLength  == 0)
    {
        rwRETURN(npos);
    }
    else
    {
        const char16_t* const pEnd    = mWString + eastl::min(nLength - 1, pos) + 1;

        const char16_t* const pResult = WStringRFind(pEnd, mWString, c);

        if (pResult != mWString)
        {
            rwRETURN(static_cast<size_t>((pResult - 1) - mWString));
        }
        rwRETURN(npos);
    }
}
/**
 Searches for any of a set of characters in the current string

 \param s The set of characters to search for
 \param pos The start location in the current string
 \return The index of the first character found if any exist,
         otherwise WString::npos if none are found
*/
size_t WString::FindFirstOf(const WString& s, size_t pos) const
{
    rwFUNCTION();
   rwRETURN(FindFirstOf(s.mWString, pos, s.mAllocator->GetLength(s.mWString)));
}
/**
 Searches for any of a set of characters in the current string

 \param p The set of characters to search for
 \param pos The start location in the current string
 \return The index of the first character found if any exist,
         otherwise WString::npos if none are found
*/
size_t WString::FindFirstOf(const char16_t* p, size_t pos) const
{
    rwFUNCTION();
   rwRETURN(FindFirstOf(p, pos, WSTRING_STRINGLENGTH(p)));
}
/**
 Searches for any of a set of characters in the current string

 \param p The set of characters to search for
 \param pos The start location in the current string
 \param n The number of characters in the search set
 \return The index of the first character found if any exist,
         otherwise WString::npos if none are found
*/
size_t WString::FindFirstOf(const char16_t* p, size_t pos, size_t n) const
{
    rwFUNCTION();
    // If n is zero or pos is >= size, we return npos.
    if (pos >= mAllocator->GetLength(mWString))
    {
        rwRETURN(npos);
    }
    else
    {
        const char16_t* const pBegin = mWString + pos;
        const char16_t *mpEnd = mWString + mAllocator->GetLength(mWString);
        const char16_t * pResult = WStringFindFirstOf(pBegin, mpEnd, p, p + n);

        if (pResult != mpEnd)
        {
            rwRETURN(static_cast<size_t>(pResult - mWString));
        }
        rwRETURN(npos);
    }
}
/**
 Searches for a character in the current string

 \param c The character to search for
 \param pos The start location in the current string
 \return The index of the first character found if any exist,
         otherwise WString::npos if none are found
*/
size_t WString::FindFirstOf(char16_t c, size_t pos) const
{
    rwFUNCTION();
    rwRETURN(Find(c, pos));
}
/**
 Searches for any of a set of characters in the current string, starting from the end

 \param s The set of characters to search for
 \param pos The start location in the current string, starting from the end
 \return The index of the first character found if any exist,
         otherwise WString::npos if none are found
*/
size_t WString::FindLastOf(const WString& s, size_t pos) const
{
    rwFUNCTION();
   rwRETURN(FindLastOf(s.mWString, pos, s.mAllocator->GetLength(s.mWString)));
}
/**
 Searches for any of a set of characters in the current string, starting from the end

 \param p The set of characters to search for
 \param pos The start location in the current string, starting from the end
 \return The index of the first character found if any exist,
         otherwise WString::npos if none are found
*/
size_t WString::FindLastOf(const char16_t* p, size_t pos) const
{
    rwFUNCTION();
   rwRETURN(FindLastOf(p, pos, WSTRING_STRINGLENGTH(p)));
}
/**
 Searches for any of a set of characters in the current string, starting from the end

 \param p The set of characters to search for
 \param pos The start location in the current string, starting from the end
 \param n The number of characters in the search set
 \return The index of the first character found if any exist,
         otherwise WString::npos if none are found
*/
size_t WString::FindLastOf(const char16_t* p, size_t pos, size_t n) const
{
    rwFUNCTION();
    // If n is zero or pos is >= size, we return npos.
    const size_t nLength = mAllocator->GetLength(mWString);

    if (nLength == 0)
    {
        rwRETURN(npos);
    }
    else
    {
        const char16_t* const pEnd    = mWString + eastl::min(nLength - 1, pos) + 1;

        const char16_t* const pResult = WStringRFindFirstOf(pEnd, mWString, p, p + n);

        if (pResult != mWString)
        {
             rwRETURN(static_cast<size_t>((pResult - 1) - mWString));
        }
        rwRETURN(npos);
    }
}
/**
 Searches for a character in the current string, starting from the end

 \param c The character to search for
 \param pos The start location in the current string, starting from the end
 \return The index of the first character found if any exist,
         otherwise WString::npos if none are found
*/
size_t WString::FindLastOf(char16_t c, size_t pos) const
{
    rwFUNCTION();
    rwRETURN(Rfind(c, pos));
}
/**
 Searches for the first instance of a character that is not in a set of characters

 \param s The set of characters
 \param pos The start location in the current string
 \return The index of the first character found if any exist,
         otherwise WString::npos if none are found
*/
size_t WString::FindFirstNotOf(const WString& s, size_t pos) const
{
    rwFUNCTION();
   rwRETURN(FindFirstNotOf(s.mWString, pos, s.mAllocator->GetLength(s.mWString)));
}
/**
 Searches for the first instance of a character that is not in a set of characters

 \param p The set of characters
 \param pos The start location in the current string
 \return The index of the first character found if any exist,
         otherwise WString::npos if none are found
*/
size_t WString::FindFirstNotOf(const char16_t* p, size_t pos) const
{
    rwFUNCTION();
   rwRETURN(FindFirstNotOf(p, pos, WSTRING_STRINGLENGTH(p)));
}
/**
 Searches for the first instance of a character that is not in a set of characters

 \param p The set of characters
 \param pos The start location in the current string
 \param n The number of characters in the set
 \return The index of the first character found if any exist, otherwise WString::npos if none are found
*/
size_t WString::FindFirstNotOf(const char16_t* p, size_t pos, size_t n) const
{
    rwFUNCTION();
    // If pos is >= size, we return npos.
    if (pos > mAllocator->GetLength(mWString))
    {
        rwRETURN(npos);
    }
    else
    {
        const char16_t *mpEnd = mWString + mAllocator->GetLength(mWString);
        const char16_t * pResult = WStringFindFirstNotOf(mWString + pos, mpEnd, p, p + n);
        if (pResult != mpEnd)
        {
            rwRETURN(static_cast<size_t>(pResult - mWString));
        }
        rwRETURN(npos);
    }
}
/**
 Searches for the first instance of a character that is not the character specified

 \param c The character
 \param pos The start location in the current string

 \return The index of the first character found if any exist,
         otherwise WString::npos if none are found
*/
size_t WString::FindFirstNotOf(char16_t c, size_t pos) const
{
    rwFUNCTION();
    if (pos > mAllocator->GetLength(mWString))
    {
        rwRETURN(npos);
    }
    else
    {
        // Todo: Possibly make a specialized version of WStringFindFirstNotOf(pBegin, pEnd, c).
        const char16_t *mpEnd = mWString + mAllocator->GetLength(mWString);
        const char16_t *pResult = WStringFindFirstNotOf(mWString + pos, mpEnd, &c, &c + 1);
        rwRETURN((pResult != mpEnd) ? (static_cast<size_t>(pResult - mWString)) : npos);
    }
}
/**
 Searches for the first instance of a character that is not in a set of characters,
 starting from the end

 \param s The set of characters
 \param pos The start location in the current string, starting from the end
 \return The index of the first character found if any exist,
         otherwise WString::npos if none are found
*/
size_t WString::FindLastNotOf(const WString& s,  size_t pos) const
{
    rwFUNCTION();
   rwRETURN(FindLastNotOf(s.mWString, pos, s.mAllocator->GetLength(s.mWString)));
}
/**
 Searches for the first instance of a character that is not in a set of characters,
 starting from the end

 \param p The set of characters
 \param pos The start location in the current string, starting from the end
 \return The index of the first character found if any exist,
         otherwise WString::npos if none are found
*/
size_t WString::FindLastNotOf(const char16_t* p, size_t pos) const
{
    rwFUNCTION();
   rwRETURN(FindLastNotOf(p, pos, WSTRING_STRINGLENGTH(p)));
}
/**
 Searches for the first instance of a character that is not in a set of characters,
 starting from the end

 \param p The set of characters
 \param pos The start location in the current string, starting from the end
 \param n The number of characters in the set
 \return The index of the first character found if any exist,
         otherwise WString::npos if none are found
*/
size_t WString::FindLastNotOf(const char16_t* p, size_t pos, size_t n) const
{
    rwFUNCTION();
    const size_t nLength = mAllocator->GetLength(mWString);

    if (nLength == 0)
    {
        rwRETURN(npos);
    }
    else
    {
        const char16_t* const pEnd    = mWString + eastl::min(nLength - 1, pos) + 1;

        const char16_t* const pResult = WStringRFindFirstNotOf(pEnd, mWString, p, p + n);

        if (pResult != mWString)
        {
            rwRETURN(static_cast<size_t>((pResult - 1) - mWString));
        }
        rwRETURN(npos);
    }
}
/**
 Searches for the first instance of a character that is not the character specified,
 starting from the end

 \param c The character
 \param pos The start location in the current string, starting from the end
 \return The index of the first character found if any exist,
         otherwise WString::npos if none are found
*/
size_t WString::FindLastNotOf(char16_t c, size_t pos) const
{
    rwFUNCTION();
    const size_t nLength = mAllocator->GetLength(mWString);

    if (nLength == 0)
    {
        rwRETURN(npos);
    }
    else
    {
        // Todo: Possibly make a specialized version of WStringRFindFirstNotOf(pBegin, pEnd, c).
        const char16_t* const pEnd    = mWString + eastl::min(nLength - 1, pos) + 1;

        const char16_t* const pResult = WStringRFindFirstNotOf(pEnd, mWString, &c, &c + 1);

        if (pResult != mWString)
        {
            rwRETURN(static_cast<size_t>((pResult - 1) - mWString));
        }
        rwRETURN(npos);
    }
}
/**
 Returns a new string that is a sub-string of the current string

 \param pos The start location in the current string
 \param n The number of characters desired
 \return A new string that is a copy of that portion of the current string
*/
WString WString::Substr(size_t pos, size_t n) const
{
    rwFUNCTION();
    rwASSERTM(pos <= mAllocator->GetLength(mWString),
        ("WString::Substr position in current string is out of range"));

    rwRETURN(WString(mWString + pos, mWString + pos + eastl::min(n, mAllocator->GetLength(mWString) - pos)));

}
/**
 Returns a new string that is the left most portion of the current string

 \param n The number of characters desired
 \return A new string that is the left most portion of the current string
*/
WString WString::Left(size_t n) const
{
    rwFUNCTION();
    const size_t nLength = mAllocator->GetLength(mWString);
    if (n >= nLength)
    {
        rwRETURN(*this);
    }
    rwRETURN(Substr(0, n));
}
/**
 Returns a new string that is the right most portion of the current string

 \param n The number of characters desired
 \return A new string that is the right most portion of the current string
*/
WString WString::Right(size_t n) const
{
    rwFUNCTION();
    const size_t nLength = mAllocator->GetLength(mWString);
    if (n >= nLength)
    {
        rwRETURN(*this);
    }
    rwRETURN(Substr(nLength - n, n));
}
/**
 Compares the contents of the current string with the one passed in

 \param s The string to compare
 \return < 0 if the current string is less than the one passed in \n
         0 if they are the same\n
         > 0 if the current string is greater than the one passed in
*/
int WString::Compare(const WString& s) const
{
    rwFUNCTION();
    rwRETURN(Compare(mWString, mWString + mAllocator->GetLength(mWString), s.mWString, s.mWString + s.mAllocator->GetLength(s.mWString)));
}
/**
 Compares the contents of the current string with the one passed in

 \param pos1 The position to start comparing in the current string
 \param n1 The number of characters to compare
 \param s The string to compare

 \return < 0 if the current string is less than the one passed in \n
         0 if they are the same \n
         > 0 if the current string is greater than the one passed in
*/
int WString::Compare(size_t pos1, size_t n1, const WString& s) const
{
    rwFUNCTION();
    rwASSERTM(pos1 <= mAllocator->GetLength(mWString),
        ("WString::Compare position in current string is out of range"));

    rwRETURN(Compare(mWString + pos1,
                   mWString + pos1 + eastl::min(n1, mAllocator->GetLength(mWString) - pos1),
                   s.mWString,
                   s.mWString + s.mAllocator->GetLength(s.mWString)));

}
/**
 Compares the contents of the current string with the one passed in

 \param pos1 The position to start comparing in the current string
 \param n1 The number of characters to compare in the current string
 \param s The string to compare
 \param pos2 The position to start comparing in the passed string
 \param n2 The number of characters to compare in the passed string
 \return < 0 if the current string is less than the one passed in \n
         0 if they are the same \n
         > 0 if the current string is greater than the one passed in
*/
int WString::Compare(size_t pos1, size_t n1, const WString& s, size_t pos2, size_t n2) const
{
    rwFUNCTION();
    rwASSERTM((pos1 <= mAllocator->GetLength(mWString)) || (pos2 <= s.mAllocator->GetLength(s.mWString)),
        ("WString::Compare position in current string or passed string is out of range"));

    rwRETURN(Compare(mWString + pos1,
                   mWString + pos1 + eastl::min(n1, mAllocator->GetLength(mWString) - pos1),
                   s.mWString + pos2,
                   s.mWString + pos2 + eastl::min(n2, mAllocator->GetLength(mWString) - pos2)));

}
/**
 Compares the contents of the current string with the one passed in

 \param p The string to compare
 \return < 0 if the current string is less than the one passed in \n
         0 if they are the same \n
         > 0 if the current string is greater than the one passed in
*/
int WString::Compare(const char16_t* p) const
{
    rwFUNCTION();
    rwRETURN(Compare(mWString, mWString + mAllocator->GetLength(mWString), p, p + WSTRING_STRINGLENGTH(p)));
}
/**
 Compares the contents of the current string with the one passed in

 \param pos1 The position to start comparing in the current string
 \param n1 The number of characters to compare in the current string
 \param p The string to compare
 \return < 0 if the current string is less than the one passed in \n
         0 if they are the same \n
         > 0 if the current string is greater than the one passed in
*/
int WString::Compare(size_t pos1, size_t n1, const char16_t* p) const
{
    rwFUNCTION();
    rwASSERTM(pos1 <= mAllocator->GetLength(mWString),
        ("WString::Compare position in current string is out of range"));

    rwRETURN(Compare(mWString + pos1,
                   mWString + pos1 + eastl::min(n1, mAllocator->GetLength(mWString) - pos1),
                   p,
                   p + WSTRING_STRINGLENGTH(p)));

}
/**
 Compares the contents of the current string with the one passed in

 \param pos1 The position to start comparing in the current string
 \param n1 The number of characters to compare in the current string
 \param p The string to compare
 \param n2 The number of characters to compare in the passed string
 \return < 0 if the current string is less than the one passed in \n
         0 if they are the same \n
         > 0 if the current string is greater than the one passed in
*/
int WString::Compare(size_t pos1, size_t n1, const char16_t* p, size_t n2) const
{
    rwFUNCTION();
    rwASSERTM(pos1 <= mAllocator->GetLength(mWString),
        ("WString::Compare position in current string is out of range"));

    rwRETURN(Compare(mWString + pos1,
                   mWString + pos1 + eastl::min(n1, mAllocator->GetLength(mWString) - pos1),
                   p,
                   p + n2));
}
/**
 Compares the contents of 2 strings

 \param pBegin1 The start of the first string to compare
 \param pEnd1 The end of the first string to compare
 \param pBegin2 The start of the second string to compare
 \param pEnd2 The end of the second string to compare
 \return < 0 if string1 is less than string2 \n
         0 if they are the same \n
         > 0 if string1 is greater than string2
*/
int WString::Compare(const char16_t* pBegin1, const char16_t* pEnd1, const char16_t* pBegin2, const char16_t* pEnd2)
{
    rwFUNCTION();
    const size_t  n1   = static_cast<size_t>(pEnd1 - pBegin1);
    const size_t  n2   = static_cast<size_t>(pEnd2 - pBegin2);
    const size_t  nMin = eastl::min(n1, n2);
    const int32_t cmp  = STRING_MEMCOMPARE(pBegin1, pBegin2, nMin);

    rwRETURN((cmp != 0) ? cmp : ((n1 < n2) ? -1 : ((n1 > n2) ? 1 : 0)));
}
/**
 Computes a hash value for the current string
 \return the hash value
*/
size_t WString::Hash() const
{
    rwFUNCTION();
    unsigned long h(0);
    for (const char16_t *i(mWString), *iEnd(mWString + mAllocator->GetLength(mWString)); i != iEnd; ++i)
    {
        h = (5 * h) + static_cast<uint8_t>(*i);
    }
    rwRETURN(static_cast<size_t>(h));
}

// This is a very simple ASCII-only lower case conversion function
void WString::MakeLower()
{
    rwFUNCTION();
    mAllocator->Modify(&mWString);
    mWString = WSTRING_TOLOWER(mWString);

    rwRETURNVOID();
}

// This is a very simple ASCII-only upper case conversion function
void WString::MakeUpper()
{
    rwFUNCTION();
    mAllocator->Modify(&mWString);

    mWString = WSTRING_TOUPPER(mWString);
    rwRETURNVOID();
}
/**
 Prints a formatted string into the current string, expanding if needed

 \param pFormat The sprintf style format string
 \param arglist The list of arguments
 \return A reference to the string
*/
WString& WString::Sprintf_va_list(const char16_t* pFormat, va_list arglist)
{
    rwFUNCTION();
    mAllocator->Modify(&mWString);

    // From unofficial C89 extension documentation:
    // The Vsnprintf The number of characters written into the array,
    // not counting the terminating null character, or a negative value
    // if count or more characters are requested to be generated.
    // An error can occur while converting a value for output.

    // From the C99 standard:
    // The Vsnprintf function returns the number of characters that would have
    // been written had n been sufficiently large, not counting the terminating
    // null character, or a negative value if an encoding error occurred.
    // Thus, the null-terminated output has been completely written if and only
    // if the returned value is nonnegative and less than n.

    int nReturnValue = WSTRING_VSNPRINTF(mWString, mAllocator->GetCapacity(mWString), pFormat, arglist);

    if (nReturnValue >= static_cast<int32_t>(mAllocator->GetCapacity(mWString)))
    {
        // In this case we definitely have C99 Vsnprintf behaviour.
        SetCapacity(static_cast<size_t>(nReturnValue+1)); //'+1' because 'nReturnValue' doesn't include the terminating 0.
        nReturnValue = WSTRING_VSNPRINTF(mWString, mAllocator->GetCapacity(mWString), pFormat, arglist);
    }
    else if (nReturnValue < 0)
    {
        // In this case we either have C89 extension behaviour or C99 behaviour.
        size_t n = eastl::max(RW_WSTRING_INITIAL_CAPACITY - 1, mAllocator->GetCapacity(mWString) * 2); // - 1 because the resize call below will add one for NULL terminator and we want to keep allocations on fixed block sizes.

        for(; (nReturnValue < 0) && (n < 1000000); n *= 2)
        {
            SetCapacity(n);
            if( mAllocator->GetCapacity(mWString) )
                nReturnValue = WSTRING_VSNPRINTF(mWString, mAllocator->GetCapacity(mWString)+1, pFormat, arglist);
        }
    }

    if (nReturnValue >= 0)
    {
        mAllocator->SetLength(mWString, static_cast<size_t>(nReturnValue));
    }

    rwRETURN(*this);
}
/**
 Prints a formatted string into the current string, expanding if needed

 \param pFormat The sprintf style format string
 \param ... The list of arguments
 \return A reference to the string
*/
//lint -e{1916} ellipsis encountered
WString& WString::Sprintf(const char16_t* pFormat, ...)
{
    rwFUNCTION();
    va_list arguments;
    va_start(arguments, pFormat);
    Sprintf_va_list(pFormat, arguments);
    va_end(arguments);

    rwRETURN(*this);
}
/**
 Returns the number of potentially multibyte characters that the current string holds

 \return A number of characters
*/
size_t WString::GetLengthUTF16() const
{
    rwFUNCTION();
    rwRETURN(GetLengthUTF16(mWString));
}
/**
 Returns the number of potentially multibyte characters that the passed string holds

 \param p The string to check
 \return A number of characters
*/
size_t WString::GetLengthUTF16(const char16_t *p)
{
    rwFUNCTION();
    int32_t     nSize = 0;
    int32_t     iCharacter(0);
    const char16_t *pBufferToParse = p;
    for(;; ++nSize)
    {
        pBufferToParse = ReadCharacterUTF16(pBufferToParse, &iCharacter);
        if (iCharacter == 0)
        {
            break;
        }
    }
    rwRETURN(static_cast<size_t>(nSize));
}
/**
 Returns the first potentially multibyte character in the string

 \param pBuffer The string to get the character from
 \return The character
*/
int32_t WString::GetCharacterUTF16(const char16_t *pBuffer)
{
    rwFUNCTION();
    int32_t iCharacter(0);
    ReadCharacterUTF16(pBuffer, &iCharacter);
    rwRETURN(iCharacter);
}
/**
 Reads the first potentially multibyte character in the string

 \param pBuffer The string to get the character from
 \param pCharacter OUTPUT The character that was read
*/
const char16_t* WString::ReadCharacterUTF16(const char16_t *pBuffer, int32_t *pCharacter)
{
    rwFUNCTION();
    int32_t         unicode = 0;
    uint8_t         cChar0, cChar1, cChar2, cChar3;
    const char16_t *    pCurrentBuffer;

    cChar0 = static_cast<unsigned char>(*pBuffer);
    rwASSERT((cChar0 != 0xFE) && (cChar0 != 0xFF));   //  No byte can contain 0xFE or 0xFF
    if  (cChar0 <= 127)
    {
        unicode = cChar0;
        pCurrentBuffer = pBuffer + 1;
    }
    else
    {
        rwASSERT((cChar0 & 0xC0) == 0xC0);            //  The top two bits need to be equal to 1

        if ((cChar0 & 0xE0) == 0xC0)
        {
            unicode = (cChar0 & 0x1F) << 6;

            cChar1 = static_cast<uint8_t>(*(pBuffer + 1));
            rwASSERT((cChar1 & 0xC0) == 0x80);        //  All subsequent code should be b10xxxxxx
            unicode |= cChar1 & 0x3F;

            rwASSERT(unicode >= 0x80);                //  Check that we have the smallest coding
            rwASSERT(unicode < 0x0800);

            pCurrentBuffer = pBuffer + 2;
        }
        else if ((cChar0 & 0xF0) == 0xE0)
        {
            unicode = (cChar0 & 0xF) << 12;

            cChar1 = static_cast<uint8_t>(*(pBuffer + 1));
            rwASSERT((cChar1 & 0xC0) == 0x80);         //  All subsequent code should be b10xxxxxx
            unicode |= (cChar1 & 0x3F) << 6;

            cChar2 = static_cast<uint8_t>(*(pBuffer + 2));
            rwASSERT((cChar2 & 0xC0) == 0x80);        //  All subsequent code should be b10xxxxxx
            unicode |= cChar2 & 0x3F;

            rwASSERT(unicode >= 0x800);               //  Check that we have the smallest coding
            rwASSERT(unicode < 0x10000);

            pCurrentBuffer = pBuffer + 3;
        }
        else
        {
            rwASSERT((cChar0 & 0xf8) == 0xf0);        //  We handle the unicode but not UCS-4
            unicode = (cChar0 & 0x7) << 18;

            cChar1 = static_cast<uint8_t>(*(pBuffer + 1));
            rwASSERT((cChar1 & 0xC0) == 0x80);        //  All subsequent code should be b10xxxxxx
            unicode |= (cChar1 & 0x3F) << 12;

            cChar2 = static_cast<uint8_t>(*(pBuffer + 2));
            rwASSERT((cChar2 & 0xC0) == 0x80);        //  All subsequent code should be b10xxxxxx
            unicode |= (cChar2 & 0x3F) << 6;

            cChar3 = static_cast<uint8_t>(*(pBuffer + 3));
            rwASSERT((cChar3 & 0xC0) == 0x80);        //  All subsequent code should be b10xxxxxx
            unicode |= cChar3 & 0x3F;

            rwASSERT(unicode >= 0x10000);             //  Check that we have the smallest coding
            rwASSERT(unicode <= 0x10FFFF);            //  Unicode and not ucs-4

            pCurrentBuffer = pBuffer + 4;
        }
    }

    rwASSERT(UTF16_IsValid(unicode));
    *pCharacter = unicode;

    rwRETURN(pCurrentBuffer);
}
/**
 Returns the potentially multibyte character at the location specified in the current string

 \param n The position to get the character at
 \return The character
*/
int32_t WString::GetAtUTF16(size_t n) const
{
    rwFUNCTION();
    const char16_t *pBuffer;
    pBuffer = GetBufferUTF16(mWString, n);
    if (pBuffer == NULL)
    {
        //  After the end of the string
        rwRETURN(-1);
    }
    rwRETURN(GetCharacterUTF16(pBuffer));
}
/**
 Returns a pointer to the location specified in the potentially multibyte current string

 \param n The position to get
 \return A pointer to the location
*/
const char16_t *WString::GetBufferUTF16(size_t n) const
{
    rwFUNCTION();
    rwRETURN(GetBufferUTF16(mWString, n));
}
/**
 Returns a pointer to the location specified in the potentially multibyte passed string

 \param p The string
 \param n The position to get
 \return A pointer to the location
*/
const char16_t *WString::GetBufferUTF16(const char16_t *p, size_t n)
{
    rwFUNCTION();
    size_t         i;
    int32_t        iCharacter(0);
    const char16_t *pBufferToParse = p;

    for(i = 0 ; i < n ; ++i)
    {
        pBufferToParse = ReadCharacterUTF16(pBufferToParse, &iCharacter);
        if (iCharacter == 0)
        {
            //  If iIndex is greater than the UTF16 size, we indicate it by a NULL char16_t *
            rwRETURN(static_cast<char16_t*>(NULL));
        }
    }
    rwRETURN(pBufferToParse);
}
/**
 Sets the potentially multibyte character at the location specified in the current string,
 this only works if the new character and old character are the same size

 \param n The position to set the character at
 \param c The character to set
 \return The character
*/
void WString::SetAtUTF16(size_t n, int32_t c)
{
    rwFUNCTION();
    mAllocator->Modify(&mWString);

    size_t         i;
    int32_t        iCharacter(0);
    const char16_t *pBufferToParse = mWString;

    for(i = 0 ; i < n ; ++i)
    {
        pBufferToParse = ReadCharacterUTF16(pBufferToParse, &iCharacter);
        if (iCharacter == 0)
        {
            //  If iIndex is greater than the UTF16 size, we don't do anything
            rwRETURNVOID();
        }
    }
    SetCharacterUTF16(const_cast<char16_t*>(pBufferToParse), c);
    rwRETURNVOID();
}
/**
 Sets the potentially multibyte character at the location specified in the passed string,
 this only works if the new character and old character are the same size

 \param pBuffer The string that points at the location to set
 \param iCharacter The character to set
 \return The character
*/
void WString::SetCharacterUTF16(char16_t *pBuffer, int32_t iCharacter)
{
    rwFUNCTION();
    rwASSERT(iCharacter != 0);
    rwASSERT(GetCharacterUTF16(pBuffer) != 0);
    rwASSERT(GetCharacterSizeUTF16(pBuffer) == GetCharacterSizeUTF16(iCharacter));
    rwASSERT(UTF16_IsValid(iCharacter));

    if (iCharacter < 0x80)
    {
        *pBuffer = static_cast<char16_t>(iCharacter);
    }
    else if (iCharacter < 0x0800)
    {
        //lint -e{702} shift right of signed quantity (int)
        *(pBuffer + 0) = static_cast<char16_t>((iCharacter >> 6) | 0xC0);
        *(pBuffer + 1) = static_cast<char16_t>((iCharacter & 0x3F) | 0x80);
    }
    else if (iCharacter < 0x10000)
    {
        //lint -e{702} shift right of signed quantity (int)
        *(pBuffer + 0) = static_cast<char16_t>((iCharacter >> 12) | 0xE0);
        //lint -e{702} shift right of signed quantity (int)
        *(pBuffer + 1) = static_cast<char16_t>(((iCharacter >> 6) & 0x3F) | 0x80);
        *(pBuffer + 2) = static_cast<char16_t>((iCharacter & 0x3F) | 0x80);
    }
    else
    {
        //lint -e{702} shift right of signed quantity (int)
        *(pBuffer + 0) = static_cast<char16_t>((iCharacter >> 18) | 0xF0);
        //lint -e{702} shift right of signed quantity (int)
        *(pBuffer + 1) = static_cast<char16_t>(((iCharacter >> 12) & 0x3F) | 0x80);
        //lint -e{702} shift right of signed quantity (int)
        *(pBuffer + 2) = static_cast<char16_t>(((iCharacter >> 6) & 0x3F) | 0x80);
        *(pBuffer + 3) = static_cast<char16_t>((iCharacter & 0x3F) | 0x80);
    }
    rwRETURNVOID();
}
/**
 Appends the potentially multibyte string to the current string

 \param p The string to append
 \param n The number of potentially multibyte characters in the passed string
 \return A reference to the current string
*/
WString& WString::AppendUTF16(const char16_t* p, size_t n)
{
    rwFUNCTION();
    size_t         i;
    int32_t         iCharacter(0);
    const char16_t *    pBuffer = p;
    for (i = 0 ; i < n ; ++i)
    {
        pBuffer = ReadCharacterUTF16(pBuffer, &iCharacter);
        if (iCharacter == 0)
        {
            break;
        }
    }

    rwRETURN(Append(p, pBuffer));
}
/**
 Inserts the potentially multibyte string into the current string at the location specified

 \param pos The location in the current string
 \param p The string to append
 \param n The number of potentially multibyte characters in the passed string
 \return A reference to the current string
*/
WString& WString::InsertUTF16(size_t pos, const char16_t* p, size_t n)
{
    rwFUNCTION();
    size_t         i;
    int32_t         iCharacter(0);
    const char16_t *    pBuffer = p;
    for (i = 0 ; i < n ; ++i)
    {
        pBuffer = ReadCharacterUTF16(pBuffer, &iCharacter);
        if (iCharacter == 0)
        {
            break;
        }
    }
    const char16_t *pPos = GetBufferUTF16(mWString, pos);
    if (pPos == NULL)
    {
        // position is past end, append it instead
        rwRETURN(Append(p, pBuffer));
    }
    else
    {
        rwRETURN(Insert(static_cast<size_t>(pPos - mWString), p, static_cast<size_t>(pBuffer - p)));
    }
}
/**
 Searches for a string in the potentially multibyte current string

 \param pStrText The string to search for
 \param iStart The start location in the current string
 \return The index of the search string in the current string if it exists,
         otherwise WString::npos if not found
*/
size_t WString::FindUTF16(const char16_t* pStrText, size_t iStart) const
{
    rwFUNCTION();
    const char16_t *    pBuffer;
    const char16_t *    pBufferStart = mWString;
    pBuffer = GetBufferUTF16(pBufferStart, iStart);
    if (pBuffer == NULL)
    {
        //  The start is after the end, can't find it
        rwRETURN(npos);
    }

    size_t iStartASCII = static_cast<size_t>(pBuffer - pBufferStart);
    size_t iFoundASCII;

    //  We can use the standard search even in UTF16
    //  Because there is no ambiguity between ASCII and UTF16!
    iFoundASCII = Find(pStrText, iStartASCII);
    if (iFoundASCII == npos)
    {
        rwRETURN(npos);
    }

    //  We need now to convert the index found to the UTF16 index
    size_t          i;
    size_t          iFoundUTF16 = iStart;
    size_t          iSize;
    const char16_t *    pCurrentBuffer = pBufferStart;
    for (i = iStartASCII ; i < iFoundASCII ; ++iFoundUTF16)
    {
        //  Read each UTF16 character, and update the ASCII index accordingly
        iSize = static_cast<size_t>(GetCharacterSizeUTF16(pCurrentBuffer));
        pCurrentBuffer += iSize;        //  Next UTF16 character
        i+= iSize;                      //  Fixup with the ASCII character

    }
    rwASSERT(i == iFoundASCII);   //  We check that the index are well synchronized
    rwRETURN(iFoundUTF16);
}
/**
 Returns a new string that is a sub-string of the potentially multibyte current string

 \param iFirst The start location in the current string
 \param iCount The number of characters desired
 \return A new string that is a copy of that portion of the current string
*/
WString WString::SubstrUTF16(size_t iFirst, size_t iCount) const
{
    rwFUNCTION();
    const char16_t *    pBufferStart;
    const char16_t *    pBufferFirst;
    const char16_t *    pBufferCount;

    pBufferStart = mWString;
    pBufferFirst = GetBufferUTF16(pBufferStart, iFirst);
    if (pBufferFirst == NULL)
    {
        //  First is after the end of the string
        rwRETURN(WString());
    }
    pBufferCount = GetBufferUTF16(pBufferFirst, iCount);
    if (pBufferCount == NULL)
    {
        //  Count is after the end of the string
        rwRETURN(Substr(static_cast<size_t>(pBufferFirst - pBufferStart)));
    }

    rwRETURN(Substr(static_cast<size_t>(pBufferFirst - pBufferStart), static_cast<size_t>(pBufferCount - pBufferFirst)));
}
/**
 This is a lower case conversion function that only converts the ASCII characters in the
 potentially multibyte current string
*/
void WString::MakeLowerUTF16()
{
    rwFUNCTION();
    mAllocator->Modify(&mWString);

    int32_t  iCharacter(0);
    char16_t *pBuffer = mWString;
    char16_t *pPrevBuffer;
    for(;;)
    {
        pPrevBuffer = pBuffer;
        pBuffer = const_cast<char16_t*>(ReadCharacterUTF16(pBuffer, &iCharacter));
        if (iCharacter == 0)
        {
            break;
        }
        SetCharacterUTF16(pPrevBuffer, tolower(iCharacter));
    }
    rwRETURNVOID();
}
/**
 This is an upper case conversion function that only converts the ASCII characters in the
 potentially multibyte current string
*/
void WString::MakeUpperUTF16()
{
    rwFUNCTION();
    mAllocator->Modify(&mWString);

    int32_t  iCharacter(0);
    char16_t *pBuffer = mWString;
    char16_t *pPrevBuffer;
    for(;;)
    {
        pPrevBuffer = pBuffer;
        pBuffer = const_cast<char16_t*>(ReadCharacterUTF16(pBuffer, &iCharacter));
        if (iCharacter == 0)
        {
            break;
        }
        SetCharacterUTF16(pPrevBuffer, toupper(iCharacter));
    }
    rwRETURNVOID();
}
/**
 Returns the size in bytes of the potentially multibyte character in the passed string

 \param p The string pointing to the character to check
 \return The size of the character
*/
int32_t WString::GetCharacterSizeUTF16(const char16_t *p)
{
    rwFUNCTION();
    uint8_t cChar0;

    cChar0 = static_cast<uint8_t>(*p);
    rwASSERT((cChar0 != 0xFE) && (cChar0 != 0xFF));   //  No byte can contain 0xFE or 0xFF
    if  (cChar0 <= 127)
    {
        rwRETURN(1);
    }
    else
    {
        rwASSERT((cChar0 & 0xC0) == 0xC0);            //  The top two bits need to be equal to 1

        if ((cChar0 & 0xE0) == 0xC0)
        {
            rwRETURN(2);
        }
        else if ((cChar0 & 0xF0) == 0xE0)
        {
            rwRETURN(3);
        }
        else
        {
            //  We handle the unicode but not UCS-4
            rwRETURN(4);
        }
    }
}
/**
 Returns the size in bytes of the potentially multibyte character passed in

 \param c The character to check
 \return The size of the character
*/
int32_t WString::GetCharacterSizeUTF16(const int32_t c)
{
    rwFUNCTION();
    rwASSERT(UTF16_IsValid(c));
    if (c < 0x80)
    {
        rwRETURN(1);
    }
    else if (c < 0x0800)
    {
        rwRETURN(2);
    }
    else if (c < 0x10000)
    {
        rwRETURN(3);
    }
    else
    {
        rwASSERT(c < 0x00200000);
        //  We handle the unicode but not UCS-4
        //  See here: http://www.cl.cam.ac.uk/~mgk25/unicode.html#utf-8
        //  This website explains why the max size is 4 and not 6
        rwRETURN(4);
    }
}
/**
\internal
 helper function to initialize a string by copying the passed string
*/
void WString::RangeInitialize(const char16_t* p, size_t n)
{
    rwFUNCTION();
    rwASSERTM(n <= GetMaxSize(),
        ("WString::RangeInitialize source string cannot exceed size_t - 2"));

    rwASSERTM(!((p == NULL) && (n != 0)),
        ("WString::RangeInitialize called with an invalid buffer"));  // Only assert if n != 0.

    mWString = mAllocator->Allocate(n+1);

    char16_t* pEnd = CharWStringUninitializedCopy(mWString, p, n);
    *pEnd = 0;
    mAllocator->SetLength(mWString, static_cast<size_t>(pEnd - mWString));
    rwRETURNVOID();
}
/**
\internal
 helper function to initialize a string by set all characters to the passed one
*/
void WString::SizeInitialize(size_t n, char16_t c)
{
    rwFUNCTION();
    rwASSERTM(n <= GetMaxSize(),
        ("WString::SizeInitialize source string cannot exceed size_t - 2"));

    mWString = mAllocator->Allocate(n+1);

    char16_t* pEnd = CharWStringUninitializedFillN(mWString, n, c);
    *pEnd = 0;
    mAllocator->SetLength(mWString, static_cast<size_t>(pEnd - mWString));
    rwRETURNVOID();
}
/**
 CharTypeWStringFind
 Specialized char version of STL find() function.
*/
const char16_t* WString::WStringFind(const char16_t* pBegin, const char16_t* pEnd, const char16_t c)
{
    rwFUNCTION();
    while(pBegin < pEnd)
    {
        if(*pBegin == c)
        {
            rwRETURN(pBegin);
        }
        ++pBegin;
    }
    rwRETURN(pEnd);
}

/**
 WStringFindEnd
 Specialized char version of STL find() from back function.
 Not the same as RFind because search range is specified as forward iterators.
*/
const char16_t* WString::WStringFindEnd(const char16_t* pBegin, const char16_t* pEnd, char16_t c)
{
    rwFUNCTION();
    const char16_t* pTemp = pEnd;
    while(--pTemp >= pBegin)
    {
        if(*pTemp == c)
        {
            rwRETURN(pTemp);
        }
    }

    rwRETURN(pEnd);
}

/**
 WStringRFind
 Specialized char16_t version of STL find() function in reverse.
*/
const char16_t* WString::WStringRFind(const char16_t* pRBegin, const char16_t* pREnd, const char16_t c)
{
    rwFUNCTION();
    while(pRBegin > pREnd)
    {
        if(*(pRBegin-1) == c)
        {
            rwRETURN(pRBegin);
        }
        --pRBegin;
    }
    rwRETURN(pREnd);
}
/**
 CharTypeWStringFind
 Specialized char16_t version of STL find() function
 Purpose: find first instance of p2 within p1. Return p1Begin if not found or if either string is zero length.
*/
const char16_t* WString::WStringSearch(const char16_t* p1Begin, const char16_t* p1End, const char16_t* p2Begin, const char16_t* p2End)
{
    rwFUNCTION();
    // Test for zero length strings, in which case we have a match or a failure,
    // but the return value is the same either way.
    if ((p1Begin == p1End) || (p2Begin == p2End))
    {
        rwRETURN(p1Begin);
    }

    // Test for a pattern of length 1.
    if ((p2Begin + 1) == p2End)
    {
        rwRETURN(WStringFind(p1Begin, p1End, *p2Begin));
    }

    // General case.
    const char16_t* pTemp;
    const char16_t* pTemp1 = (p2Begin + 1);
    const char16_t* pCurrent = p1Begin;

    while (p1Begin != p1End)
    {
        p1Begin = WStringFind(p1Begin, p1End, *p2Begin);
        if (p1Begin == p1End)
        {
            rwRETURN(p1End);
        }

        pTemp = pTemp1;
        pCurrent = p1Begin;
        if (++pCurrent == p1End)
        {
            rwRETURN(p1End);
        }

        while (*pCurrent == *pTemp)
        {
            if (++pTemp == p2End)
            {
                rwRETURN(p1Begin);
            }
            if (++pCurrent == p1End)
            {
                rwRETURN(p1End);
            }
        }

        ++p1Begin;
    }

    rwRETURN(p1Begin);
}

/**
 WStringRSearch
 Specialized char16_t version of STL find_end() function (which really is a reverse search function).
 Purpose: find last instance of p2 within p1. Return p1End if not found or if either string is zero length.
*/
const char16_t* WString::WStringRSearch(const char16_t* p1Begin, const char16_t* p1End, const char16_t* p2Begin, const char16_t* p2End)
{
    rwFUNCTION();
    // Test for zero length strings, in which case we have a match or a failure,
    // but the return value is the same either way.
    if ((p1Begin == p1End) || (p2Begin == p2End))
        rwRETURN(p1Begin);

    // Test for a pattern of length 1.
    if ((p2Begin + 1) == p2End)
        rwRETURN(WStringFindEnd(p1Begin, p1End, *p2Begin));

    // Test for search string length being longer than string length.
    if ((p2End - p2Begin) > (p1End - p1Begin))
        rwRETURN(p1End);

    // General case.
    const char16_t* pSearchEnd = ((p1End - (p2End - p2Begin)) + 1);
    const char16_t* pCurrent1;
    const char16_t* pCurrent2;

    while (pSearchEnd != p1Begin)
    {
        // Search for the last occurrence of *p2Begin.
        pCurrent1 = WStringFindEnd(p1Begin, pSearchEnd, *p2Begin);
        if (pCurrent1 == pSearchEnd) // If the first char of p2 wasn't found,
            rwRETURN(p1End);            // then we immediately have failure.

        // In this case, *pTemp == *p2Begin. So compare the rest.
        pCurrent2 = p2Begin;
        while (*pCurrent1++ == *pCurrent2++)
        {
            if (pCurrent2 == p2End)
                rwRETURN(pCurrent1 - (p2End - p2Begin));
        }

        // A smarter algorithm might know to subtract more than just one,
        // but in most cases it won't make much difference anyway.
        --pSearchEnd;
    }

    rwRETURN(p1End);
}

/**
 WStringFindFirstOf
 Specialized char16_t version of STL find_first_of() function.
 This function is much like the C runtime strtok function, except the strings aren't null-terminated.
*/
const char16_t* WString::WStringFindFirstOf(const char16_t* p1Begin, const char16_t* p1End, const char16_t* p2Begin, const char16_t* p2End)
{
    rwFUNCTION();
    for ( ; p1Begin != p1End; ++p1Begin)
    {
        for (const char16_t* pTemp = p2Begin; pTemp != p2End; ++pTemp)
        {
            if (*p1Begin == *pTemp)
                rwRETURN(p1Begin);
        }
    }
    rwRETURN(p1End);
}

/**
 WStringRFindFirstOf
 Specialized char16_t version of STL find_first_of() function in reverse.
 This function is much like the C runtime strtok function, except the strings aren't null-terminated.
*/
const char16_t* WString::WStringRFindFirstOf(const char16_t* p1RBegin, const char16_t* p1REnd, const char16_t* p2Begin, const char16_t* p2End)
{
    rwFUNCTION();
    for ( ; p1RBegin != p1REnd; --p1RBegin)
    {
        for (const char16_t* pTemp = p2Begin; pTemp != p2End; ++pTemp)
        {
            if (*(p1RBegin-1) == *pTemp)
                rwRETURN(p1RBegin);
        }
    }
    rwRETURN(p1REnd);
}


/**
 WStringFindFirstNotOf
 Specialized char16_t version of STL find_first_not_of() function.
*/
const char16_t* WString::WStringFindFirstNotOf(const char16_t* p1Begin, const char16_t* p1End, const char16_t* p2Begin, const char16_t* p2End)
{
    rwFUNCTION();
    for( ; p1Begin != p1End; ++p1Begin)
    {
        const char16_t* pTemp;
        for(pTemp = p2Begin; pTemp != p2End; ++pTemp)
        {
            if (*p1Begin == *pTemp)
                break;
        }
        if (pTemp == p2End)
            rwRETURN(p1Begin);
    }
    rwRETURN(p1End);
}

/**
 WStringRFindFirstNotOf
 Specialized char16_t version of STL find_first_not_of() function in reverse.
*/
const char16_t* WString::WStringRFindFirstNotOf(const char16_t* p1RBegin, const char16_t* p1REnd, const char16_t* p2Begin, const char16_t* p2End)
{
    rwFUNCTION();
    for( ; p1RBegin != p1REnd; --p1RBegin)
    {
        const char16_t* pTemp;
        for(pTemp = p2Begin; pTemp != p2End; ++pTemp)
        {
            if (*(p1RBegin-1) == *pTemp)
                break;
        }
        if (pTemp == p2End)
            rwRETURN(p1RBegin);
    }
    rwRETURN(p1REnd);
}
/**
 Appends the parameters together and creates a new string with the result

 \param x The first string
 \param y The second string
 \return The new string
*/
WString rw::core::operator+(const WString& x, const WString& y)
{
    rwFUNCTION();
#if GCCOPTIMIZE //defined to 0 as we don't want gcc optimization yet
    //!defined(__GNUC__) || (__GNUC__ >= 3) //GCC 2.x has a bug
    WString result(WString::CtorDoNotInitialize(), x.GetLength() + y.GetLength());
#else
    WString::CtorDoNotInitialize temp;
    WString result(temp, x.GetLength() + y.GetLength());
#endif
    result.Append(x);
    result.Append(y);
    rwRETURN(result);
}
/**
 Appends the parameters together and creates a new string with the result

 \param s The first character buffer
 \param y The second string
 \return The new string
*/
WString rw::core::operator+(const char16_t* s, const WString& y)
{
    rwFUNCTION();
    const size_t n = WSTRING_STRINGLENGTH(s);
#if GCCOPTIMIZE //defined to 0 as we don't want gcc optimization yet
    //!defined(__GNUC__) || (__GNUC__ >= 3) //GCC 2.x has a bug
    WString result(WString::CtorDoNotInitialize(), n + y.GetLength());
#else
    WString::CtorDoNotInitialize temp;
    WString result(temp, n + y.GetLength());
#endif
    result.Append(s, s + n);
    result.Append(y);
    rwRETURN(result);
}
/**
 Appends the parameters together and creates a new string with the result

 \param c The first character
 \param y The second string
 \return The new string
*/
WString rw::core::operator+(char16_t c, const WString& y)
{
    rwFUNCTION();
#if GCCOPTIMIZE //defined to 0 as we don't want gcc optimization yet
    //!defined(__GNUC__) || (__GNUC__ >= 3) //GCC 2.x has a bug
    WString result(WString::CtorDoNotInitialize(), 1 + y.GetLength());
#else
    WString::CtorDoNotInitialize temp;
    WString result(temp, 1 + y.GetLength());
#endif
    result.PushOnBack(c);
    result.Append(y);
    rwRETURN(result);
}
/**
 Appends the parameters together and creates a new string with the result

 \param x The first string
 \param p The second character buffer
 \return The new string
*/
WString rw::core::operator+(const WString& x, const char16_t* p)
{
    rwFUNCTION();
    const size_t n = WSTRING_STRINGLENGTH(p);
#if GCCOPTIMIZE //defined to 0 as we don't want gcc optimization yet
    //!defined(__GNUC__) || (__GNUC__ >= 3) //GCC 2.x has a bug
    WString result(WString::CtorDoNotInitialize(), x.GetLength() + n);
#else
    WString::CtorDoNotInitialize temp;
    WString result(temp, x.GetLength() + n);
#endif
    result.Append(x);
    result.Append(p, p + n);
    rwRETURN(result);
}
/**
 Appends the parameters together and creates a new string with the result

 \param x The first string
 \param c The second character
 \return The new string
*/
WString rw::core::operator+(const WString& x, const char16_t c)
{
    rwFUNCTION();
#if GCCOPTIMIZE //defined to 0 as we don't want gcc optimization yet
    //!defined(__GNUC__) || (__GNUC__ >= 3) //GCC 2.x has a bug
    WString result(WString::CtorDoNotInitialize(), x.GetLength() + 1);
#else
    WString::CtorDoNotInitialize temp;
    WString result(temp, x.GetLength() + 1);
#endif
    result.Append(x);
    result.PushOnBack(c);
    rwRETURN(result);
}

#if defined(EA_PLATFORM_WINDOWS) || defined(EA_PLATFORM_XENON)
#pragma warning(pop)
#endif

