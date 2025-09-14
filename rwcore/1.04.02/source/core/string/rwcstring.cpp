// (c) Electronic Arts. All Rights Reserved.

/// \file
/// \deprecated.  Use EASTL's String class intead of this one.
/// Implementation of C++ string streams

/*      String class implementation */
#include "rwsharedheaders.hpp"
#include <stdio.h>      // _vsnprintf
#include <ctype.h>      // toupper, tolower
//Use EAStdC
#include <EASTL/algorithm.h>
#include <EAStdC/EAMemory.h>
#include <EAStdC/EAAlignment.h>
#include <EAStdC/EASprintf.h>
#include <EAStdC/EAString.h>


#include "rw/core/string/stringconfig.h"
#include "rw/core/string/string.h"
#if defined(EA_PLATFORM_PS4)
#include <stdarg.h>
#endif
#define GCCOPTIMIZE 0

using namespace rw::core;

//lint -e{956, 1776} non-const, non-volatile static or external variable.  This is not const because it gets assigned to non-const strings in this module.
char8_t *String::mEmpty = const_cast<char*>("");

//lint -e{956} non-const, non-volatile static or external variable.
IStringAllocator *String::mDefaultAllocator = NULL;

// RW_STRING_INITIAL_CAPACITY
// As of this writing, this must be > 0.
const size_t RW_STRING_INITIAL_CAPACITY = 8;
const size_t  RW_STRING_MAX_CAPACITY = 1000000;

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
utility function to verify if a character is UTF8
*/
inline bool UTF8_IsValid(const int32_t iCharacter)
{
    rwFUNCTION();
    if (iCharacter > 0x10FFFF)
    {
        rwRETURN(false);
    }
    rwRETURN(true);
}
/**
 CharStringUninitializedCopy
 This is similar to STL's uninitialized_copy but can make the
 assumption that CharType variables don't need to be constructed,
 but merely assigned.
*/
inline char8_t* CharStringUninitializedCopy(char8_t* pDestination, const char8_t* pSource, size_t n)
{
    rwFUNCTION();
   STRING_MEMMOVE(pDestination, pSource, n);
   rwRETURN(pDestination + n);
}
/**
 CharStringUninitializedFillN
 This is similar to STL's uninitialized_fill_n but can make the assumption that
 CharType variables don't need to be constructed, but merely assigned.
*/
inline char8_t* CharStringUninitializedFillN(char8_t* pDestination, size_t n, char8_t c)
{
    rwFUNCTION();
    STRING_MEMFILL(pDestination, static_cast<uint8_t>(c), n);
    rwRETURN(pDestination + n);
}
/**
 Constructor for the String class
*/
String::String()
: mAllocator(mDefaultAllocator), mString(mEmpty)
{
    rwFUNCTION();
    rwASSERTM(mAllocator != NULL, ("String::String mAllocator is NULL"));
    rwRETURNVOID();
}
/**
 Constructor for the String class
 \param a The string allocator to use for this string
*/
String::String(IStringAllocator *a)
: mAllocator(a), mString(mEmpty)
{
    rwFUNCTION();
    if (mAllocator == NULL)
    {
        mAllocator = mDefaultAllocator;
        rwASSERTM(mAllocator != NULL, ("String::String mAllocator is NULL"));
    }
    rwRETURNVOID();
}
/**
 Constructor for the String class

 \param p Pointer to a buffer to use for this string
 \param a The string allocator to use for this string
*/
String::String(CtorBuffer, char8_t* p, IStringAllocator *a)
: mAllocator(a), mString(p)
{
    rwFUNCTION();
    if (mAllocator == NULL)
    {
        mAllocator = mDefaultAllocator;
        rwASSERTM(mAllocator != NULL, ("String::String mAllocator is NULL"));
    }
    rwRETURNVOID();
}
/**
 Constructor for the String class

 \param s A source string to copy into this string
*/
String::String(const String& s)
: mAllocator(mDefaultAllocator), mString(mEmpty)
{
    rwFUNCTION();
    rwASSERTM(mAllocator != NULL, ("String::String mAllocator is NULL"));

    RangeInitialize(s.GetPointer(), s.GetLength());
    rwRETURNVOID();
}
/**
 Constructor for the String class

 \param s A source string to copy into this string
 \param a The string allocator to use for this string
*/
String::String(const String& s, IStringAllocator *a)
: mAllocator(a), mString(mEmpty)
{
    rwFUNCTION();
    if (mAllocator == NULL)
    {
        mAllocator = mDefaultAllocator;
        rwASSERTM(mAllocator != NULL, ("String::String mAllocator is NULL"));
    }

    RangeInitialize(s.GetPointer(), s.GetLength());
    rwRETURNVOID();
}
/**
 Constructor for the String class

 \param s A source string to copy into this string
 \param pos The start of the portion to copy
 \param n The size of the portion to copy
*/
String::String(const String& s, size_t pos, size_t n)
: mAllocator(mDefaultAllocator), mString(mEmpty)
{
    rwFUNCTION();
    rwASSERTM(mAllocator != NULL, ("String::String mAllocator is NULL"));
    rwASSERTM(pos <= s.mAllocator->GetLength(s.mString),
        ("String::String called with a start position in the source string that is out of range"));

    RangeInitialize(s.GetPointer() + pos, eastl::min(n, static_cast<size_t>(s.mAllocator->GetLength(s.mString) - pos)));

    rwRETURNVOID();
}
/**
 Constructor for the String class

 \param s A source string to copy into this string
 \param pos The start of the portion to copy
 \param n The size of the portion to copy
 \param a The string allocator to use for this string
*/
String::String(const String& s, size_t pos, size_t n, IStringAllocator *a)
: mAllocator(a), mString(mEmpty)
{
    rwFUNCTION();
    if (mAllocator == NULL)
    {
        mAllocator = mDefaultAllocator;
        rwASSERTM(mAllocator != NULL, ("String::String mAllocator is NULL"));
    }

    rwASSERTM(pos <= s.mAllocator->GetLength(s.mString),
        ("String::String called with a start position in the source string that is out of range"));

    RangeInitialize(s.GetPointer() + pos, eastl::min(n, s.mAllocator->GetLength(s.mString) - pos));

    rwRETURNVOID();
}
/**
 Constructor for the String class

 \param p A char buffer to copy into this string
 \param n The number of characters to use from the char buffer
*/
String::String(const char8_t* p, size_t n)
: mAllocator(mDefaultAllocator), mString(mEmpty)
{
    rwFUNCTION();
    rwASSERTM(mAllocator != NULL, ("String::String mAllocator is NULL"));
    rwASSERTM(p != NULL, ("String::String called with an invalid buffer"));

    RangeInitialize(p, n);
    rwRETURNVOID();
}
/**
 Constructor for the String class

 \param p A char buffer to copy into this string
 \param n The number of characters to use from the char buffer
 \param a The string allocator to use for this string
*/
String::String(const char8_t* p, size_t n, IStringAllocator *a)
: mAllocator(a), mString(mEmpty)
{
    rwFUNCTION();
    rwASSERTM(mAllocator != NULL, ("String::String mAllocator is NULL"));
    rwASSERTM(p != NULL, ("String::String called with an invalid buffer"));

    RangeInitialize(p, n);
    rwRETURNVOID();
}
/**
 Constructor for the String class

 \param p A char buffer to copy into this string
*/
String::String(const char8_t* p)
: mAllocator(mDefaultAllocator), mString(mEmpty)
{
    rwFUNCTION();
    rwASSERTM(mAllocator != NULL, ("String::String mAllocator is NULL"));
    rwASSERTM(p != NULL, ("String::String called with an invalid buffer"));

    RangeInitialize(p, STRING_STRINGLENGTH(p));
    rwRETURNVOID();
}
/**
 Constructor for the String class

 \param p A char buffer to copy into this string
 \param a The string allocator to use for this string
*/
String::String(const char8_t* p, IStringAllocator *a)
: mAllocator(a), mString(mEmpty)
{
    rwFUNCTION();
    rwASSERTM(p != NULL, ("String::String called with an invalid buffer"));

    if (mAllocator == NULL)
    {
        mAllocator = mDefaultAllocator;
        rwASSERTM(mAllocator != NULL, ("String::String mAllocator is NULL"));
    }
    RangeInitialize(p, STRING_STRINGLENGTH(p));
    rwRETURNVOID();
}
/**
 Constructor for the String class

 \param n The number of character to initialize the string to
 \param c A character to use
*/
String::String(size_t n, char8_t c)
: mAllocator(mDefaultAllocator), mString(mEmpty)
{
    rwFUNCTION();
    rwASSERTM(mAllocator != NULL, ("String::String mAllocator is NULL"));
    SizeInitialize(n, c);
    rwRETURNVOID();
}
/**
 Constructor for the String class

 \param n The number of character to initialize the string to
 \param c A character to use
 \param a The string allocator to use for this string
*/
String::String(size_t n, char8_t c, IStringAllocator *a)
: mAllocator(a), mString(mEmpty)
{
    rwFUNCTION();
    if (mAllocator == NULL)
    {
        mAllocator = mDefaultAllocator;
        rwASSERTM(mAllocator != NULL, ("String::String mAllocator is NULL"));
    }
    SizeInitialize(n, c);
    rwRETURNVOID();
}
/**
 Constructor for the String class

 \param pBegin The start of a string to copy
 \param pEnd The end of the string
*/
String::String(const char8_t* pBegin, const char8_t* pEnd)
: mAllocator(mDefaultAllocator), mString(mEmpty)
{
    rwFUNCTION();
    rwASSERTM(mAllocator != NULL, ("String::String mAllocator is NULL"));

    RangeInitialize(pBegin, static_cast<size_t>(pEnd - pBegin));
    rwRETURNVOID();
}
/**
 Constructor for the String class

 \param pBegin The start of a string to copy
 \param pEnd The end of the string
 \param a The string allocator to use for this string
*/
String::String(const char8_t* pBegin, const char8_t* pEnd, IStringAllocator *a)
: mAllocator(a), mString(mEmpty)
{
    rwFUNCTION();
    if (mAllocator == NULL)
    {
        mAllocator = mDefaultAllocator;
        rwASSERTM(mAllocator != NULL, ("String::String mAllocator is NULL"));
    }
    RangeInitialize(pBegin, static_cast<size_t>(pEnd - pBegin));
    rwRETURNVOID();
}
/**
 Constructor for the String class

 \param n The size of space to reserve
*/
String::String(CtorDoNotInitialize, size_t n)
: mAllocator(mDefaultAllocator), mString(mEmpty)
{
    rwFUNCTION();
    rwASSERTM(mAllocator != NULL, ("String::String mAllocator is NULL"));
    rwASSERTM(n <= GetMaxSize(),
        ("String::String called with a space to reserve size that is larger than size_t - 2"));

    mString = mAllocator->Allocate(n+1);
    mAllocator->SetLength(mString, 0);
    rwRETURNVOID();
}
/**
 Constructor for the String class

 \param n The size of space to reserve
 \param a The string allocator to use for this string
*/
String::String(CtorDoNotInitialize, size_t n, IStringAllocator *a)
: mAllocator(a), mString(mEmpty)
{
    rwFUNCTION();
    rwASSERTM(n <= GetMaxSize(),
        ("String::String called with a space to reserve size that is larger than size_t - 2"));

    if (mAllocator == NULL)
    {
        mAllocator = mDefaultAllocator;
        rwASSERTM(mAllocator != NULL, ("String::String mAllocator is NULL"));
    }
    mString = mAllocator->Allocate(n+1);
    mAllocator->SetLength(mString, 0);
    rwRETURNVOID();
}
/**
 Constructor for the String class

 \param pFormat The sprintf style format string
 \param ... The sprintf parameters
*/
//lint -e{1916} ellipsis encountered
String::String(CtorSprintf, const char8_t* pFormat, ...)
: mAllocator(mDefaultAllocator), mString(mEmpty)
{
    rwFUNCTION();
    rwASSERTM(mAllocator != NULL, ("String::String mAllocator is NULL"));

    mString = mAllocator->Allocate(STRING_STRINGLENGTH(pFormat) + 1); // We'll need at least this much.

    va_list arguments;
    va_start(arguments, pFormat);
    Sprintf_va_list(pFormat, arguments);
    va_end(arguments);
    rwRETURNVOID();
}
/**
 Constructor for the String class

 \param a The string allocator to use for this string
 \param pFormat The sprintf style format string
 \param ... The sprintf parameters
*/
//lint -e{1916} ellipsis encountered
String::String(CtorSprintf, IStringAllocator *a, const char8_t* pFormat, ...)
: mAllocator(a), mString(mEmpty)
{
    rwFUNCTION();
    if (mAllocator == NULL)
    {
        mAllocator = mDefaultAllocator;
        rwASSERTM(mAllocator != NULL, ("String::String mAllocator is NULL"));
    }

    mString = mAllocator->Allocate(STRING_STRINGLENGTH(pFormat) + 1); // We'll need at least this much.

    va_list arguments;
    va_start(arguments, pFormat);
    Sprintf_va_list(pFormat, arguments);
    va_end(arguments);
    rwRETURNVOID();
}

void String::PreDerivedDtorBaseCleanUp()
{
    rwFUNCTION();
    mAllocator->Deallocate(mString);
    mString = NULL;
    mAllocator = NULL;
    rwRETURNVOID();
}

/**
Destructor for the String class
*/
String::~String()
{
    if (mAllocator != NULL && mString != NULL)
    {
        PreDerivedDtorBaseCleanUp();
    }
}

/**
 Assignment operator

 \param s The source string to assign

 \return A reference to the destination string
*/
String& String::operator=(const String& s)
{
    rwFUNCTION();
    if (&s != this)
    {
        if (mAllocator->GetAllocatorType() == s.GetAllocator()->GetAllocatorType())
        {
            mAllocator->Assign(*this, &mString, s, s.mString);
        }
        else
        {
            Assign(s.mString, s.GetLength());
        }
    }
    rwRETURN(*this);
}
/**
 Assignment operator

 \param p The source character buffer to assign

 \return A reference to the destination string
*/
String& String::operator=(const char8_t* p)
{
    rwFUNCTION();
    rwRETURN(Assign(p, STRING_STRINGLENGTH(p)));
}
/**
 Assignment operator

 \param c The source character to assign

 \return A reference to the destination string
*/
String& String::operator=(char8_t c)
{
    rwFUNCTION();
    rwRETURN(Assign(1, c));
}
/**
 Returns the capacity of the string

 \return The capacity of the string, does not include space for the NULL terminator
*/
size_t String::GetCapacity() const
{
    rwFUNCTION();
    rwRETURN(mAllocator->GetCapacity(mString));
}
/**
 Returns the length of the current string

 \return The string length in bytes, does not include the NULL terminator
*/
size_t String::GetLength() const
{
    rwFUNCTION();
    rwRETURN(mAllocator->GetLength(mString));
}
/**
 Returns if the string is empty

 \return true if the string is empty, false otherwise
*/
bool String::IsEmpty() const
{
    rwFUNCTION();
    rwRETURN(*mString == '\0');
}
/**
 Sets the capacity of the string. This will not allow the capacity to shrink smaller than
 the current string length.

 \param n The size of the capacity to use, does not include the NULL terminator
*/
void String::SetCapacity(size_t n)
{
    rwFUNCTION();
    rwASSERTM(n <= GetMaxSize(),
        ("String::SetCapacity called with a space to reserve size that is larger than size_t - 2"));

    n = eastl::max(n, mAllocator->GetLength(mString)) + 1;

    if((n - 1) != mAllocator->GetCapacity(mString))  //If there is something to do...
    {
        char8_t* pNewBegin = mAllocator->Allocate(n);
        char8_t* pNewEnd   = pNewBegin;

        pNewEnd = CharStringUninitializedCopy(pNewBegin, mString, mAllocator->GetLength(mString));
        *pNewEnd = 0;

        size_t originalLength = mAllocator->GetLength(mString);

        mAllocator->Deallocate(mString);
        mString      = pNewBegin;
        mAllocator->SetLength(mString, originalLength);
    }
    rwRETURNVOID();
}

/**
Clears out the string contents
*/
void String::Clear()
{
    rwFUNCTION();
    if (!IsEmpty())
    {
        mAllocator->Modify(&mString);
        *mString = 0;
        mAllocator->SetLength(mString, 0);
    }
    rwRETURNVOID();
}
/**
 Returns the character at the specified position

 \param n The index of the character to return

 \return The character
*/
char8_t String::operator[](size_t n) const
{
    rwFUNCTION();
    rwASSERTM(n < mAllocator->GetLength(mString),
        ("String::operator[] called with an index out of range"));

    rwRETURN(mString[n]);
}
/**
 Returns the character at the specified position

 \param n The index of the character to return

 \return The character
*/
char8_t String::GetAt(size_t n) const
{
    rwFUNCTION();
    rwASSERTM(n < mAllocator->GetLength(mString),
        ("String::GetAt called with an index out of range"));

    rwRETURN(mString[n]);
}
/**
 Sets the character at the specified position to a new value

 \param n The index of the character to set
 \param c The character to set
*/
void String::SetAt(size_t n, char8_t c)
{
    rwFUNCTION();
    rwASSERTM(n < mAllocator->GetLength(mString),
        ("String::SetAt called with an index out of range"));

    mAllocator->Modify(&mString);
    mString[n] = c;
    if (c == '\0')
    {
        mAllocator->SetLength(mString, n);
    }
    rwRETURNVOID();
}
/**
 Appends the parameters to the current string

 \param s The source string to append

 \return A reference to the new string
*/
String& String::operator+=(const String& s)
{
    rwFUNCTION();
    mAllocator->Modify(&mString);
    rwRETURN(Append(s.GetPointer(), s.mAllocator->GetLength(s.mString)));
}
/**
 Appends the parameters to the current string

 \param p The source character buffer to append

 \return A reference to the new string
*/
String& String::operator+=(const char8_t* p)
{
    rwFUNCTION();
    mAllocator->Modify(&mString);
    rwRETURN(Append(p, STRING_STRINGLENGTH(p)));
}
/**
 Appends the parameters to the current string

 \param c The source character to append

 \return A reference to the new string
*/
String& String::operator+=(char8_t c)
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
String& String::Append(const String& s)
{
    rwFUNCTION();
    mAllocator->Modify(&mString);
    rwRETURN(Append(s.mString, s.mString + s.mAllocator->GetLength(s.mString)));
}
/**
 Appends the parameters to the current string

 \param s The source string to append
 \param pos The start position in the source string
 \param n The number of characters from the source string

 \return A reference to the new string
*/
String& String::Append(const String& s, size_t pos, size_t n)
{
    rwFUNCTION();
    size_t len = s.mAllocator->GetLength(s.mString);

    rwASSERTM(pos <= len,
        ("String::Append called with a start position in the source string that is out of range"));

    mAllocator->Modify(&mString);

    rwRETURN(Append(s.mString + pos, s.mString + pos + eastl::min(n, len - pos)));

}
/**
 Appends the parameters to the current string

 \param p The source character buffer to append
 \param n The number of characters from the source buffer

 \return A reference to the new string
*/
String& String::Append(const char8_t* p, size_t n)
{
    rwFUNCTION();
    mAllocator->Modify(&mString);
    rwRETURN(Append(p, p + n));
}
/**
 Appends the parameters to the current string

 \param p The source character buffer to append

 \return A reference to the new string
*/
String& String::Append(const char8_t* p)
{
    rwFUNCTION();
    mAllocator->Modify(&mString);
    rwRETURN(Append(p, p + STRING_STRINGLENGTH(p)));
}
/**
 Appends the parameters to the current string

 \param n The number of characters to append
 \param c The character to append

 \return A reference to the new string
*/
String& String::Append(size_t n, char8_t c)
{
    rwFUNCTION();
    const size_t s = mAllocator->GetLength(mString);

    rwASSERTM((n <= GetMaxSize()) || (s <= (GetMaxSize() - n)),
        ("String::Append size param or destination string cannot exceed size_t - 2"));

    if (n > 0)
    {
        mAllocator->Modify(&mString);
    }

    if ((s + n) > mAllocator->GetCapacity(mString))
    {

        SetCapacity(s + eastl::max(s, n));

    }

    if (n > 0)
    {
        CharStringUninitializedFillN(mString + mAllocator->GetLength(mString), n, c);
        mString[s+n] = '\0';
        mAllocator->SetLength(mString, s+n);
    }
    rwRETURN(*this);
}
/**
 Appends the parameters to the current string

 \param pBegin The start of the source string to append
 \param pEnd The end of the source string to append

 \return A reference to the new string
*/
String& String::Append(const char8_t* pBegin, const char8_t* pEnd)
{
    rwFUNCTION();
    if (pBegin != pEnd)
    {
        mAllocator->Modify(&mString);
        const size_t nOldSize(mAllocator->GetLength(mString));
        const size_t n(static_cast<size_t>(pEnd - pBegin));

        rwASSERTM((n <= GetMaxSize()) || (nOldSize < (GetMaxSize() - n)),
            ("String::Append source string or destination string cannot exceed size_t - 2"));

        if ((nOldSize + n) > mAllocator->GetCapacity(mString))
        {

            const size_t nLength = nOldSize + eastl::max(nOldSize, n) + 1;

            char8_t *pNewBegin = mAllocator->Allocate(nLength);
            char8_t *pNewEnd   = pNewBegin;

            pNewEnd = CharStringUninitializedCopy(pNewBegin, mString, nOldSize);
            pNewEnd = CharStringUninitializedCopy(pNewEnd, pBegin,  n);
            *pNewEnd = 0;

            mAllocator->Deallocate(mString);
            mString      = pNewBegin;
            mAllocator->SetLength(mString, static_cast<size_t>(pNewEnd - mString));
        }
        else
        {
            char8_t *mpEnd = mString + nOldSize;
            CharStringUninitializedCopy(mpEnd, pBegin, n);
            mString[nOldSize + n] = 0;
            mAllocator->SetLength(mString, nOldSize + n);
        }
    }

    rwRETURN(*this);
}
/**
 Appends the character to the end of the string

 \param c The source string to append
*/
void String::PushOnBack(char8_t c)
{
    rwFUNCTION();
    mAllocator->Modify(&mString);
    const size_t s = mAllocator->GetLength(mString);
    if ((s + 1) > mAllocator->GetCapacity(mString))
    {
        SetCapacity(s + eastl::max(s, static_cast<size_t>(1)));
    }
    mString[s] = c;
    mString[s+1] = 0;
    mAllocator->SetLength(mString, s+1);
    rwRETURNVOID();
}

/**
Removes the last character from the end of the string
*/
void String::PopFromBack()
{
    rwFUNCTION();
    mAllocator->Modify(&mString);
    const size_t s = mAllocator->GetLength(mString);
    mString[s-1] = 0;
    mAllocator->SetLength(mString, s-1);
    rwRETURNVOID();
}
/**
 Assigns the current string based on the parameters

 \param s The source string to assign

 \return A reference to the destination string
*/
String& String::Assign(const String& s)
{
    rwFUNCTION();
    mAllocator->Modify(&mString);
    if (mAllocator->GetAllocatorType() == s.mAllocator->GetAllocatorType())
    {
        mAllocator->Assign(*this, &mString, s, s.mString);
        rwRETURN(*this);
    }
    else
    {
        rwRETURN(Assign(s.mString, s.mString + s.mAllocator->GetLength(s.mString)));
    }
}
/**
 Assigns the current string based on the parameters

 \param s The source string to assign
 \param pos The starting position in the string
 \param n The number of characters to copy

 \return A reference to the destination string
*/
String& String::Assign(const String& s, size_t pos, size_t n)
{
    rwFUNCTION();
    rwASSERTM(pos <= s.mAllocator->GetLength(s.mString),
        ("String::Assign called with a start position in the source string that is out of range"));

    mAllocator->Modify(&mString);
    rwRETURN(Assign(s.mString + pos, s.mString + pos + eastl::min(n, s.mAllocator->GetLength(s.mString) - pos)));
}
/**
 Assigns the current string based on the parameters

 \param p The source character bugger to assign
 \param n The number of characters to copy

 \return A reference to the destination string
*/
String& String::Assign(const char8_t* p, size_t n)
{
    rwFUNCTION();
    mAllocator->Modify(&mString);
    rwRETURN(Assign(p, p + n));
}
/**
 Assigns the current string based on the parameters

 \param p The source character bugger to assign

 \return A reference to the destination string
*/
String& String::Assign(const char8_t* p)
{
    rwFUNCTION();
    mAllocator->Modify(&mString);
    rwRETURN(Assign(p, p + STRING_STRINGLENGTH(p)));
}
/**
 Assigns the current string based on the parameters

 \param n The number of characters to assign
 \param c The character to assign

 \return A reference to the destination string
*/
String& String::Assign(size_t n, char8_t c)
{
    rwFUNCTION();
    mAllocator->Modify(&mString);
    const size_t s = mAllocator->GetLength(mString);
    if (n <= mAllocator->GetCapacity(mString))
    {
        STRING_MEMFILL(mString, static_cast<uint8_t>(c), n);
        Erase(mString + n, mString + s);
    }
    else
    {
        STRING_MEMFILL(mString, static_cast<uint8_t>(c), s);
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
String& String::Assign(const char8_t* pBegin, const char8_t* pEnd)
{
    rwFUNCTION();
    mAllocator->Modify(&mString);
    const size_t n = static_cast<size_t>(pEnd - pBegin);
    size_t len = mAllocator->GetLength(mString);
    if (static_cast<size_t>(n) <= len)
    {
        STRING_MEMCOPY(mString, pBegin, n );
        Erase(mString + n, mString + len);
    }
    else
    {
        STRING_MEMCOPY(mString, pBegin, len);
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
String& String::Insert(size_t pos, const String& s)
{
    rwFUNCTION();
    rwASSERTM(pos <= mAllocator->GetLength(mString),
        ("String::Insert called with a start position in the current string that is out of range"));
    rwASSERTM(mAllocator->GetLength(mString) <= (GetMaxSize() - s.mAllocator->GetLength(s.mString)),
        ("String::Insert destination string cannot exceed size_t - 2"));

    mAllocator->Modify(&mString);
    Insert(mString + pos, s.mString, s.mString + s.mAllocator->GetLength(s.mString));
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
String& String::Insert(size_t pos, const String& s, size_t beg, size_t n)
{
    rwFUNCTION();
    rwASSERTM((pos <= mAllocator->GetLength(mString)) || (beg <= s.mAllocator->GetLength(s.mString)),
        ("String::Insert position in current string, or start location in source string is out of range"));

    size_t nLength = eastl::min(n, s.mAllocator->GetLength(s.mString) - beg);

    rwASSERTM(mAllocator->GetLength(mString) <= (GetMaxSize() - nLength),
        ("String::Insert destination string cannot exceed size_t - 2"));

    mAllocator->Modify(&mString);
    Insert(mString + pos, s.mString + beg, s.mString + beg + nLength);
    rwRETURN(*this);
}
/**
 Inserts the parameters into the current string

 \param pos Position in the current string
 \param p The character buffer to insert
 \param n The number of characters in the source buffer

 \return A reference to the destination string
*/
String& String::Insert(size_t pos, const char8_t* p, size_t n)
{
    rwFUNCTION();
    rwASSERTM(pos <= mAllocator->GetLength(mString),
        ("String::Insert position in current string is out of range"));

    rwASSERTM(mAllocator->GetLength(mString) <= (GetMaxSize() - n),
        ("String::Insert destination string cannot exceed size_t - 2"));

    mAllocator->Modify(&mString);
    Insert(mString + pos, p, p + n);
    rwRETURN(*this);
}
/**
 Inserts the parameters into the current string

 \param pos Position in the current string
 \param p The character buffer to insert

 \return A reference to the destination string
*/
String& String::Insert(size_t pos, const char8_t* p)
{
    rwFUNCTION();
    rwASSERTM(pos <= mAllocator->GetLength(mString),
        ("String::Insert position in current string is out of range"));

    size_t nLength = STRING_STRINGLENGTH(p);

    rwASSERTM(mAllocator->GetLength(mString) <= (GetMaxSize() - nLength),
        ("String::Insert destination string cannot exceed size_t - 2"));

    mAllocator->Modify(&mString);
    Insert(mString + pos, p, p + nLength);
    rwRETURN(*this);
}
/**
 Inserts the parameters into the current string

 \param pos Position in the current string
 \param n The number of characters to insert
 \param c The character to insert

 \return A reference to the destination string
*/
String& String::Insert(size_t pos, size_t n, char8_t c)
{
    rwFUNCTION();
    if (n != 0)
    {
        mAllocator->Modify(&mString);

        char8_t *p = mString + pos;
        const size_t len = mAllocator->GetLength(mString);
        char8_t *mpEnd = mString + len;
        if ((mAllocator->GetCapacity(mString) - len) >= (n + 1))
        {
            const size_t nElementsAfter = static_cast<size_t>(mpEnd - p);
            const char8_t *pOldEnd = mpEnd;

            if (nElementsAfter >= n)
            {
                CharStringUninitializedCopy(mpEnd + 1, (mpEnd - n) + 1, n);
                mpEnd += n;
                STRING_MEMMOVE(p + n, p, (nElementsAfter - n) + 1);
                STRING_MEMFILL(p, static_cast<uint8_t>(c), n);
            }
            else
            {
                CharStringUninitializedFillN(mpEnd + 1, (n - nElementsAfter) - 1, c);
                mpEnd += n - nElementsAfter;
                RW_STRING_TRY
                {
                    CharStringUninitializedCopy(mpEnd, p, static_cast<size_t>((pOldEnd + 1) - p));
                    mpEnd += nElementsAfter;
                }
                RW_STRING_CATCH(mpEnd = pOldEnd);
                STRING_MEMFILL(p, static_cast<uint8_t>(c), nElementsAfter + 1);
            }
            mAllocator->SetLength(mString, static_cast<size_t>(mpEnd - mString));
        }
        else
        {
            const size_t nOldSize = mAllocator->GetLength(mString);

            const size_t nLength = nOldSize + eastl::max(nOldSize, n) + 1;

            char8_t *pNewBegin = mAllocator->Allocate(nLength);
            char8_t *pNewEnd   = pNewBegin;

            pNewEnd = CharStringUninitializedCopy(pNewBegin, mString, static_cast<size_t>(p - mString));
            pNewEnd = CharStringUninitializedFillN(pNewEnd, n, c);
            pNewEnd = CharStringUninitializedCopy(pNewEnd, p, static_cast<size_t>(mpEnd - p));
            *pNewEnd = 0;

            mAllocator->Deallocate(mString);
            mString      = pNewBegin;
            mAllocator->SetLength(mString, static_cast<size_t>(pNewEnd - mString));
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
void String::Insert(char8_t *p, const char8_t* pBegin, const char8_t* pEnd)
{
    rwFUNCTION();
    const size_t n = static_cast<size_t>(pEnd - pBegin);

    if (n != 0)
    {
        mAllocator->Modify(&mString);

        const size_t len = mAllocator->GetLength(mString);
        char8_t *mpEnd = mString + len;
        if (((mAllocator->GetCapacity(mString) - len) >= (n + 1)) && ((pBegin > mpEnd) || (pEnd < mString))) // If we don't need to reallocate...
        {
            const size_t nElementsAfter = static_cast<size_t>(mpEnd - p);
            const char8_t *pOldEnd = mpEnd;

            if (nElementsAfter >= n) // If the newly inserted characters entirely fit within the size of the original string...
            {
                STRING_MEMMOVE(mpEnd + 1, (mpEnd - n) + 1, n);
                mpEnd += n;
                STRING_MEMMOVE(p + n, p, ((nElementsAfter - n) + 1));
                STRING_MEMCOPY(p, pBegin, static_cast<size_t>(pEnd - pBegin));
            }
            else
            {
                const char8_t* pMid = pBegin;

                pMid += (nElementsAfter + 1);
                STRING_MEMMOVE(mpEnd + 1, pMid, static_cast<size_t>(pEnd - pMid));
                mpEnd += n - nElementsAfter;
                RW_STRING_TRY
                {
                   STRING_MEMMOVE(mpEnd, p, static_cast<size_t>((pOldEnd - p) + 1));
                   mpEnd += nElementsAfter;
                }
                RW_STRING_CATCH(mpEnd = pOldEnd);
                STRING_MEMCOPY(p, pBegin, static_cast<size_t>(pMid - pBegin));
            }
            mAllocator->SetLength(mString, static_cast<size_t>(mpEnd - mString));
        }
        else // Else we need to reallocate to implement this.
        {
            const size_t nOldSize = len;

            const size_t nLength  = nOldSize + eastl::max(nOldSize, static_cast<size_t>(n)) + 1; // We grow the capacity by an amount possibly greater than needed.

            char8_t *pNewBegin       = mAllocator->Allocate(nLength);
            char8_t *pNewEnd         = pNewBegin;

            pNewEnd = CharStringUninitializedCopy(pNewBegin, mString, static_cast<size_t>(p - mString));
            pNewEnd = CharStringUninitializedCopy(pNewEnd,   pBegin,  static_cast<size_t>(pEnd - pBegin));
            pNewEnd = CharStringUninitializedCopy(pNewEnd,   p,       static_cast<size_t>(mpEnd - p));
            *pNewEnd = 0;

            mAllocator->Deallocate(mString);
            mString      = pNewBegin;
            mAllocator->SetLength(mString, static_cast<size_t>(pNewEnd - mString));
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
String& String::Erase(size_t pos, size_t n)
{
    rwFUNCTION();
    rwASSERTM(pos <= mAllocator->GetLength(mString),
        ("String::Erase position in current string is out of range"));

    mAllocator->Modify(&mString);

    Erase(mString + pos, mString + pos + eastl::min(n, mAllocator->GetLength(mString) - pos));

    rwRETURN(*this);
}
/**
 Erases a portion of the current string, the end of the string will be moved forward if a
 portion of the middle of the string is erased.

 \param pBegin The start of the string to erase
 \param pEnd The end of the string to erase

 \return A reference to the string
*/
String& String::Erase(char8_t *pBegin, const char8_t *pEnd)
{
    rwFUNCTION();
    if (pBegin != pEnd)
    {
        mAllocator->Modify(&mString);

        const char8_t *mpEnd = mString + mAllocator->GetLength(mString);
        STRING_MEMMOVE(pBegin, pEnd, static_cast<size_t>((mpEnd - pEnd) + 1));
        const char8_t *pNewEnd = (mpEnd - (pEnd - pBegin));
        mAllocator->SetLength(mString, static_cast<size_t>(pNewEnd - mString));
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
String& String::Replace(size_t pos, size_t n, const String& s)
{
    rwFUNCTION();
    rwASSERTM(pos <= mAllocator->GetLength(mString),
        ("String::Replace position in current string is out of range"));

    const size_t nLength = eastl::min(n, mAllocator->GetLength(mString) - pos);

    rwASSERTM(mAllocator->GetLength(mString) <= (GetMaxSize() - nLength),
        ("String::Replace destination string cannot exceed size_t - 2"));

    mAllocator->Modify(&mString);
    rwRETURN(Replace(mString + pos, mString + pos + nLength, s.mString, s.mString + s.mAllocator->GetLength(s.mString)));
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
String& String::Replace(size_t pos1, size_t n1, const String& s, size_t pos2, size_t n2)
{
    rwFUNCTION();
    rwASSERTM((pos1 <= mAllocator->GetLength(mString)) || (pos2 <= s.mAllocator->GetLength(s.mString)),
        ("String::Replace position in current string, or start location in source string is out of range"));

    const size_t nLength1 = eastl::min(n1, mAllocator->GetLength(mString)   - pos1);
    const size_t nLength2 = eastl::min(n2, s.mAllocator->GetLength(s.mString) - pos2);

    rwASSERTM((mAllocator->GetLength(mString) - nLength1) < (GetMaxSize() - nLength2),
        ("String::Replace destination string cannot exceed size_t - 2"));

    mAllocator->Modify(&mString);
    rwRETURN(Replace(mString + pos1, mString + pos1 + nLength1, s.mString + pos2, s.mString + pos2 + nLength2));
}
/**
 Replaces a portion of the current string with something new

 \param pos Position in the current string to replace
 \param n1 The number of characters to replace
 \param p The source character buffer
 \param n2 The number of characters from the source buffer

 \return A reference to the new string
*/
String& String::Replace(size_t pos, size_t n1, const char8_t* p, size_t n2)
{
    rwFUNCTION();
    rwASSERTM(pos <= mAllocator->GetLength(mString),
        ("String::Replace position in current string is out of range"));

    const size_t nLength = eastl::min(n1, mAllocator->GetLength(mString) - pos);

    rwASSERTM((n2 <= GetMaxSize()) || ((mAllocator->GetLength(mString) - nLength) < (GetMaxSize() - n2)),
        ("String::Replace source string or destination string cannot exceed size_t - 2"));

    mAllocator->Modify(&mString);
    rwRETURN(Replace(mString + pos, mString + pos + nLength, p, p + n2));
}
/**
 Replaces a portion of the current string with something new

 \param pos Position in the current string to replace
 \param n1 The number of characters to replace
 \param p The source character buffer

 \return A reference to the new string
*/
String& String::Replace(size_t pos, size_t n1, const char8_t* p)
{
    rwFUNCTION();
    rwASSERTM(pos <= mAllocator->GetLength(mString),
        ("String::Replace position in current string is out of range"));

    const size_t nLength = eastl::min(n1, mAllocator->GetLength(mString) - pos);

    rwASSERTM((STRING_STRINGLENGTH(p) <= GetMaxSize()) ||
        ((mAllocator->GetLength(mString) - nLength) < (GetMaxSize() - STRING_STRINGLENGTH(p))),
        ("String::Replace source string or destination string cannot exceed size_t - 2"));

    mAllocator->Modify(&mString);
    rwRETURN(Replace(mString + pos, mString + pos + nLength, p, p + STRING_STRINGLENGTH(p)));
}
/**
 Replaces a portion of the current string with something new

 \param pos Position in the current string to replace
 \param n1 The number of characters to replace
 \param n2 The number of source characters to insert
 \param c The source character

 \return A reference to the new string
*/
String& String::Replace(size_t pos, size_t n1, size_t n2, char8_t c)
{
    rwFUNCTION();
    rwASSERTM(pos <= mAllocator->GetLength(mString),
        ("String::Replace position in current string is out of range"));

    const size_t nLength = eastl::min(n1, mAllocator->GetLength(mString) - pos);

    rwASSERTM((n2 <= GetMaxSize()) || ((mAllocator->GetLength(mString) - nLength) < (GetMaxSize() - n2)),
        ("String::Replace source string or destination string cannot exceed size_t - 2"));

    mAllocator->Modify(&mString);

    char8_t       *pBegin = mString + pos;
    const char8_t *pEnd = mString + pos + nLength;

    if(nLength >= n2)
    {
        STRING_MEMFILL(pBegin, static_cast<uint8_t>(c), n2);
        Erase(pBegin + n2, pEnd);
    }
    else
    {
        STRING_MEMFILL(pBegin, static_cast<uint8_t>(c), nLength);
        Insert(static_cast<size_t>(pEnd - mString), n2 - nLength, c);
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
String& String::Replace(char8_t *pBegin1, char8_t *pEnd1, const char8_t *pBegin2, const char8_t *pEnd2)
{
    rwFUNCTION();
    mAllocator->Modify(&mString);

    const size_t nLength1 = static_cast<size_t>(pEnd1 - pBegin1);
    const size_t nLength2 = static_cast<size_t>(pEnd2 - pBegin2);

    if (nLength1 >= nLength2) // If we have a non-expanding operation...
    {
        if ((pBegin2 > pEnd1) || (pEnd2 <= pBegin1))  // If we have a non-overlapping operation...
        {
            STRING_MEMCOPY(pBegin1, pBegin2, static_cast<size_t>(pEnd2 - pBegin2));
        }
        else
        {
            STRING_MEMMOVE(pBegin1, pBegin2, static_cast<size_t>(pEnd2 - pBegin2));
        }
        Erase(pBegin1 + nLength2, pEnd1);
    }
    else // Else we are expanding.
    {
        if ((pBegin2 > pEnd1) || (pEnd2 <= pBegin1)) // If we have a non-overlapping operation...
        {
            const char8_t* const pMid2 = pBegin2 + nLength1;

            if ((pEnd2 <= pBegin1) || (pBegin2 > pEnd1))
            {
                STRING_MEMCOPY(pBegin1, pBegin2, static_cast<size_t>(pMid2 - pBegin2));
            }
            else
            {
                STRING_MEMMOVE(pBegin1, pBegin2, static_cast<size_t>(pMid2 - pBegin2));
            }
            Insert(pEnd1, pMid2, pEnd2);
        }
        else // else we have an overlapping operation.
        {
            // I can't think of any easy way of doing this without allocating temporary memory.
            const size_t nOldSize     = mAllocator->GetLength(mString);

            const size_t nNewCapacity = nOldSize + eastl::max(nOldSize, size_t(nLength2 - nLength1)) + 1; // We grow the capacity by an amount possibly greater than needed.

            const char8_t *mpEnd      = mString + nOldSize;
            char8_t *pNewBegin        = mAllocator->Allocate(nNewCapacity);
            char8_t *pNewEnd          = pNewBegin;

            pNewEnd = CharStringUninitializedCopy(pNewBegin, mString, static_cast<size_t>(pBegin1 - mString));
            pNewEnd = CharStringUninitializedCopy(pNewEnd,   pBegin2, static_cast<size_t>(pEnd2 - pBegin2));
            pNewEnd = CharStringUninitializedCopy(pNewEnd,   pEnd1,   static_cast<size_t>(mpEnd - pEnd1));
            *pNewEnd = 0;

            mAllocator->Deallocate(mString);
            mString = pNewBegin;
            mAllocator->SetLength(mString, static_cast<size_t>(pNewEnd - pNewBegin));
        }
    }
    rwRETURN(*this);
}
/**
 Searches for a string in the current string

 \param s The string to search for
 \param pos The start location in the current string

 \return The index of the search string in the current string if it exists,
         otherwise String::npos if not found
*/
size_t String::Find(const String& s, size_t pos) const
{
    rwFUNCTION();
    rwRETURN(Find(s.mString, pos, s.mAllocator->GetLength(s.mString)));
}
/**
 Searches for a string in the current string

 \param p The string to search for
 \param pos The start location in the current string

 \return The index of the search string in the current string if it exists,
         otherwise String::npos if not found
*/
size_t String::Find(const char8_t* p, size_t pos) const
{
    rwFUNCTION();
    rwRETURN(Find(p, pos, STRING_STRINGLENGTH(p)));
}
/**
 Searches for a string in the current string

 \param p The string to search for
 \param pos The start location in the current string
 \param n The number of characters in the search string

 \return The index of the search string in the current string if it exists,
         otherwise String::npos if not found
*/
size_t String::Find(const char8_t* p, size_t pos, size_t n) const
{
    rwFUNCTION();
    // Note that a search for a zero length string starting at position = end() returns end() and not npos.
    const size_t len = mAllocator->GetLength(mString);
    if ((pos + n) > len)
    {
        rwRETURN(npos);
    }
    else
    {
        const char8_t* const pBegin  = mString + pos;
        const char8_t *mpEnd = mString + len;
        const char8_t* const pResult = StringSearch(pBegin, mpEnd, p, p + n);

        if (static_cast<size_t>(mpEnd - pResult) >= n) // Don't check for simply 'if (mpEnd - pResult)', as it doesn't yield true when n == 0.
        {
            rwRETURN(static_cast<size_t>(pResult - mString));
        }
        rwRETURN(npos);
    }
}
/**
 Searches for a character in the current string

 \param c The character to search for
 \param pos The start location in the current string

 \return The index of the search character in the current string if it exists,
         otherwise String::npos if not found
*/
size_t String::Find(char8_t c, size_t pos) const
{
    rwFUNCTION();
    if (pos >= mAllocator->GetLength(mString))
    {
        rwRETURN(npos);
    }
    else
    {
        const char8_t* const pBegin  = mString + pos;
        const char8_t *mpEnd = mString + mAllocator->GetLength(mString);
        const char8_t * pResult = StringFind(pBegin, mpEnd, c);

        if (pResult != mpEnd)
        {
             rwRETURN(static_cast<size_t>(pResult - mString));
        }
        rwRETURN(npos);
    }
}
/**
 Searches for a string in the current string, starting from the end

 \param s The string to search for
 \param pos The start location in the current string, starting from the end

 \return The index of the search string in the current string if it exists,
         otherwise String::npos if not found
*/
size_t String::Rfind(const String& s, size_t pos) const
{
    rwFUNCTION();
    rwRETURN(Rfind(s.mString, pos, s.mAllocator->GetLength(s.mString)));
}
/**
 Searches for a string in the current string, starting from the end

 \param p The string to search for
 \param pos The start location in the current string, starting from the end

 \return The index of the search string in the current string if it exists,
         otherwise String::npos if not found
*/
size_t String::Rfind(const char8_t* p, size_t pos) const
{
    rwFUNCTION();
    rwRETURN(Rfind(p, pos, STRING_STRINGLENGTH(p)));
}
/**
 Searches for a string in the current string, starting from the end

 \param p The string to search for
 \param pos The start location in the current string, starting from the end
 \param n The number of characters in the search string

 \return The index of the search string in the current string if it exists,
         otherwise String::npos if not found
*/
size_t String::Rfind(const char8_t* p, size_t pos, size_t n) const
{
    rwFUNCTION();
    // Note that a search for a zero length string starting at position = end() returns end() and not npos.
    // Note by Paul Pedriana: I am not sure how this should behave in the case of n == 0 and pos > size.
    // The standard seems to suggest that rfind doesn't act exactly the same as find in that input pos
    // can be > size and the return value can still be other than npos. Thus, if n == 0 then you can
    // never return npos, unlike the case with find.
    const size_t nLength = mAllocator->GetLength(mString);

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
        const char8_t * pEnd    = mString + eastl::min(nLength - n, pos) + n;

        const char8_t * pResult = StringRSearch(mString, pEnd, p, p + n);

        if (pResult != pEnd)
        {
            rwRETURN(static_cast<size_t>(pResult - mString));
        }
        rwRETURN(npos);
    }
}
/**
 Searches for a character in the current string, starting from the end

 \param c The character to search for
 \param pos The start location in the current string, starting from the end

 \return The index of the search character in the current string if it exists,
         otherwise String::npos if not found
*/
size_t String::Rfind(char8_t c, size_t pos) const
{
    rwFUNCTION();
    // If n is zero or pos is >= size, we return npos.
    const size_t nLength = mAllocator->GetLength(mString);

    if (nLength  == 0)
    {
        rwRETURN(npos);
    }
    else
    {
        const char8_t* const pEnd    = mString + eastl::min(nLength - 1, pos) + 1;

        const char8_t* const pResult = StringRFind(pEnd, mString, c);

        if (pResult != mString)
        {
            rwRETURN(static_cast<size_t>((pResult - 1) - mString));
        }
        rwRETURN(npos);
    }
}
/**
 Searches for any of a set of characters in the current string

 \param s The set of characters to search for
 \param pos The start location in the current string

 \return The index of the first character found if any exist,
         otherwise String::npos if none are found
*/
size_t String::FindFirstOf(const String& s, size_t pos) const
{
    rwFUNCTION();
   rwRETURN(FindFirstOf(s.mString, pos, s.mAllocator->GetLength(s.mString)));
}
/**
 Searches for any of a set of characters in the current string

 \param p The set of characters to search for
 \param pos The start location in the current string

 \return The index of the first character found if any exist,
         otherwise String::npos if none are found
*/
size_t String::FindFirstOf(const char8_t* p, size_t pos) const
{
    rwFUNCTION();
   rwRETURN(FindFirstOf(p, pos, STRING_STRINGLENGTH(p)));
}
/**
 Searches for any of a set of characters in the current string

 \param p The set of characters to search for
 \param pos The start location in the current string
 \param n The number of characters in the search set

 \return The index of the first character found if any exist,
         otherwise String::npos if none are found
*/
size_t String::FindFirstOf(const char8_t* p, size_t pos, size_t n) const
{
    rwFUNCTION();
    // If n is zero or pos is >= size, we return npos.
    if (pos >= mAllocator->GetLength(mString))
    {
        rwRETURN(npos);
    }
    else
    {
        const char8_t* const pBegin = mString + pos;
        const char8_t *mpEnd = mString + mAllocator->GetLength(mString);
        const char8_t * pResult = StringFindFirstOf(pBegin, mpEnd, p, p + n);

        if (pResult != mpEnd)
        {
            rwRETURN(static_cast<size_t>(pResult - mString));
        }
        rwRETURN(npos);
    }
}
/**
 Searches for a character in the current string

 \param c The character to search for
 \param pos The start location in the current string

 \return The index of the first character found if any exist,
         otherwise String::npos if none are found
*/
size_t String::FindFirstOf(char8_t c, size_t pos) const
{
    rwFUNCTION();
    rwRETURN(Find(c, pos));
}
/**
 Searches for any of a set of characters in the current string, starting from the end

 \param s The set of characters to search for
 \param pos The start location in the current string, starting from the end

 \return The index of the first character found if any exist,
         otherwise String::npos if none are found
*/
size_t String::FindLastOf(const String& s, size_t pos) const
{
    rwFUNCTION();
   rwRETURN(FindLastOf(s.mString, pos, s.mAllocator->GetLength(s.mString)));
}
/**
 Searches for any of a set of characters in the current string, starting from the end

 \param p The set of characters to search for
 \param pos The start location in the current string, starting from the end

 \return The index of the first character found if any exist,
         otherwise String::npos if none are found
*/
size_t String::FindLastOf(const char8_t* p, size_t pos) const
{
    rwFUNCTION();
   rwRETURN(FindLastOf(p, pos, STRING_STRINGLENGTH(p)));
}
/**
 Searches for any of a set of characters in the current string, starting from the end

 \param p The set of characters to search for
 \param pos The start location in the current string, starting from the end
 \param n The number of characters in the search set

 \return The index of the first character found if any exist,
         otherwise String::npos if none are found
*/
size_t String::FindLastOf(const char8_t* p, size_t pos, size_t n) const
{
    rwFUNCTION();
    // If n is zero or pos is >= size, we return npos.
    const size_t nLength = mAllocator->GetLength(mString);

    if (nLength == 0)
    {
        rwRETURN(npos);
    }
    else
    {

        const char8_t* const pEnd    = mString + eastl::min(nLength - 1, pos) + 1;

        const char8_t* const pResult = StringRFindFirstOf(pEnd, mString, p, p + n);

        if (pResult != mString)
        {
             rwRETURN(static_cast<size_t>((pResult - 1) - mString));
        }
        rwRETURN(npos);
    }
}
/**
 Searches for a character in the current string, starting from the end

 \param c The character to search for
 \param pos The start location in the current string, starting from the end

 \return The index of the first character found if any exist,
         otherwise String::npos if none are found
*/
size_t String::FindLastOf(char8_t c, size_t pos) const
{
    rwFUNCTION();
    rwRETURN(Rfind(c, pos));
}
/**
 Searches for the first instance of a character that is not in a set of characters

 \param s The set of characters
 \param pos The start location in the current string

 \return The index of the first character found if any exist,
         otherwise String::npos if none are found
*/
size_t String::FindFirstNotOf(const String& s, size_t pos) const
{
    rwFUNCTION();
   rwRETURN(FindFirstNotOf(s.mString, pos, s.mAllocator->GetLength(s.mString)));
}
/**
 Searches for the first instance of a character that is not in a set of characters

 \param p The set of characters
 \param pos The start location in the current string

 \return The index of the first character found if any exist,
         otherwise String::npos if none are found
*/
size_t String::FindFirstNotOf(const char8_t* p, size_t pos) const
{
    rwFUNCTION();
   rwRETURN(FindFirstNotOf(p, pos, STRING_STRINGLENGTH(p)));
}
/**
 Searches for the first instance of a character that is not in a set of characters

 \param p The set of characters
 \param pos The start location in the current string
 \param n The number of characters in the set

 \return The index of the first character found if any exist,
         otherwise String::npos if none are found
*/
size_t String::FindFirstNotOf(const char8_t* p, size_t pos, size_t n) const
{
    rwFUNCTION();
    // If pos is >= size, we return npos.
    if (pos > mAllocator->GetLength(mString))
    {
        rwRETURN(npos);
    }
    else
    {
        const char8_t *mpEnd = mString + mAllocator->GetLength(mString);
        const char8_t * pResult = StringFindFirstNotOf(mString + pos, mpEnd, p, p + n);
        if (pResult != mpEnd)
        {
            rwRETURN(static_cast<size_t>(pResult - mString));
        }
        rwRETURN(npos);
    }
}
/**
 Searches for the first instance of a character that is not the character specified

 \param c The character
 \param pos The start location in the current string

 \return The index of the first character found if any exist,
         otherwise String::npos if none are found
*/
size_t String::FindFirstNotOf(char8_t c, size_t pos) const
{
    rwFUNCTION();
    if (pos > mAllocator->GetLength(mString))
    {
        rwRETURN(npos);
    }
    else
    {
        // Todo: Possibly make a specialized version of StringFindFirstNotOf(pBegin, pEnd, c).
        const char8_t *mpEnd = mString + mAllocator->GetLength(mString);
        const char8_t *pResult = StringFindFirstNotOf(mString + pos, mpEnd, &c, &c + 1);
        rwRETURN((pResult != mpEnd) ? (static_cast<size_t>(pResult - mString)) : npos);
    }
}
/**
 Searches for the first instance of a character that is not in a set of characters,
 starting from the end

 \param s The set of characters
 \param pos The start location in the current string, starting from the end

 \return The index of the first character found if any exist,
         otherwise String::npos if none are found
*/
size_t String::FindLastNotOf(const String& s,  size_t pos) const
{
    rwFUNCTION();
   rwRETURN(FindLastNotOf(s.mString, pos, s.mAllocator->GetLength(s.mString)));
}
/**
 Searches for the first instance of a character that is not in a set of characters,
 starting from the end

 \param p The set of characters
 \param pos The start location in the current string, starting from the end

 \return The index of the first character found if any exist,
         otherwise String::npos if none are found
*/
size_t String::FindLastNotOf(const char8_t* p, size_t pos) const
{
    rwFUNCTION();
   rwRETURN(FindLastNotOf(p, pos, STRING_STRINGLENGTH(p)));
}
/**
 Searches for the first instance of a character that is not in a set of characters,
 starting from the end

 \param p The set of characters
 \param pos The start location in the current string, starting from the end
 \param n The number of characters in the set

 \return The index of the first character found if any exist,
         otherwise String::npos if none are found
*/
size_t String::FindLastNotOf(const char8_t* p, size_t pos, size_t n) const
{
    rwFUNCTION();
    const size_t nLength = mAllocator->GetLength(mString);

    if (nLength == 0)
    {
        rwRETURN(npos);
    }
    else
    {

        const char8_t* const pEnd    = mString + eastl::min(nLength - 1, pos) + 1;

        const char8_t* const pResult = StringRFindFirstNotOf(pEnd, mString, p, p + n);

        if (pResult != mString)
        {
            rwRETURN(static_cast<size_t>((pResult - 1) - mString));
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
         otherwise String::npos if none are found
*/
size_t String::FindLastNotOf(char8_t c, size_t pos) const
{
    rwFUNCTION();
    const size_t nLength = mAllocator->GetLength(mString);

    if (nLength == 0)
    {
        rwRETURN(npos);
    }
    else
    {
        // Todo: Possibly make a specialized version of StringRFindFirstNotOf(pBegin, pEnd, c).

        const char8_t* const pEnd    = mString + eastl::min(nLength - 1, pos) + 1;

        const char8_t* const pResult = StringRFindFirstNotOf(pEnd, mString, &c, &c + 1);

        if (pResult != mString)
        {
            rwRETURN(static_cast<size_t>((pResult - 1) - mString));
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
String String::Substr(size_t pos, size_t n) const
{
    rwFUNCTION();
    rwASSERTM(pos <= mAllocator->GetLength(mString),
        ("String::Substr position in current string is out of range"));

    rwRETURN(String(mString + pos, mString + pos + eastl::min(n, mAllocator->GetLength(mString) - pos)));

}
/**
 Returns a new string that is the left most portion of the current string

 \param n The number of characters desired

 \return A new string that is the left most portion of the current string
*/
String String::Left(size_t n) const
{
    rwFUNCTION();
    const size_t nLength = mAllocator->GetLength(mString);
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
String String::Right(size_t n) const
{
    rwFUNCTION();
    const size_t nLength = mAllocator->GetLength(mString);
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
         0 if they are the same \n
         > 0 if the current string is greater than the one passed in
*/
int String::Compare(const String& s) const
{
    rwFUNCTION();
    rwRETURN(Compare(mString, mString + mAllocator->GetLength(mString), s.mString, s.mString + s.mAllocator->GetLength(s.mString)));
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
int String::Compare(size_t pos1, size_t n1, const String& s) const
{
    rwFUNCTION();
    rwASSERTM(pos1 <= mAllocator->GetLength(mString),
        ("String::Compare position in current string is out of range"));

    rwRETURN(Compare(mString + pos1,
        mString + pos1 + eastl::min(n1, mAllocator->GetLength(mString) - pos1),
               s.mString,
               s.mString + s.mAllocator->GetLength(s.mString)));

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
int String::Compare(size_t pos1, size_t n1, const String& s, size_t pos2, size_t n2) const
{
    rwFUNCTION();
    rwASSERTM((pos1 <= mAllocator->GetLength(mString)) || (pos2 <= s.mAllocator->GetLength(s.mString)),
        ("String::Compare position in current string or passed string is out of range"));

    rwRETURN(Compare(mString + pos1,
        mString + pos1 + eastl::min(n1, mAllocator->GetLength(mString) - pos1),
               s.mString + pos2,
               s.mString + pos2 + eastl::min(n2, mAllocator->GetLength(mString) - pos2)));

}
/**
 Compares the contents of the current string with the one passed in

 \param p The string to compare

 \return < 0 if the current string is less than the one passed in \n
         0 if they are the same \n
         > 0 if the current string is greater than the one passed in
*/
int String::Compare(const char8_t* p) const
{
    rwFUNCTION();
    rwRETURN(Compare(mString, mString + mAllocator->GetLength(mString), p, p + STRING_STRINGLENGTH(p)));
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
int String::Compare(size_t pos1, size_t n1, const char8_t* p) const
{
    rwFUNCTION();
    rwASSERTM(pos1 <= mAllocator->GetLength(mString),
        ("String::Compare position in current string is out of range"));

    rwRETURN(Compare(mString + pos1,
                   mString + pos1 + eastl::min(n1, mAllocator->GetLength(mString) - pos1),
                   p,
                   p + STRING_STRINGLENGTH(p)));

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
int String::Compare(size_t pos1, size_t n1, const char8_t* p, size_t n2) const
{
    rwFUNCTION();
    rwASSERTM(pos1 <= mAllocator->GetLength(mString),
        ("String::Compare position in current string is out of range"));

    rwRETURN(Compare(mString + pos1,
                   mString + pos1 + eastl::min(n1, mAllocator->GetLength(mString) - pos1),
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
int String::Compare(const char8_t* pBegin1, const char8_t* pEnd1, const char8_t* pBegin2, const char8_t* pEnd2)
{
    rwFUNCTION();
    const size_t  n1   = static_cast<size_t>(pEnd1 - pBegin1);
    const size_t  n2   = static_cast<size_t>(pEnd2 - pBegin2);

    const size_t  nMin = eastl::min(n1, n2);
    const int32_t cmp  = STRING_MEMCOMPARE(pBegin1, pBegin2, nMin);

    rwRETURN(((cmp != 0) ? cmp : ((n1 < n2) ? -1 : ((n1 > n2) ? 1 : 0))));
}
/**
 Computes a hash value for the current string

 \return the hash value
*/
size_t String::Hash() const
{
    rwFUNCTION();
    unsigned long h(0);
    for (const char8_t *i(mString), *iEnd(mString + mAllocator->GetLength(mString)); i != iEnd; ++i)
    {
        h = (5 * h) + static_cast<uint8_t>(*i);
    }
    rwRETURN( static_cast<size_t>(h));
}

// This is a very simple ASCII-only lower case conversion function
void String::MakeLower()
{
    rwFUNCTION();
    mAllocator->Modify(&mString);
    mString = STRING_TOLOWER(mString);
    rwRETURNVOID();
}

// This is a very simple ASCII-only upper case conversion function
void String::MakeUpper()
{
    rwFUNCTION();
    mAllocator->Modify(&mString);
    mString = STRING_TOUPPER(mString);
    rwRETURNVOID();
}
/**
 Prints a formatted string into the current string, expanding if needed

 \param pFormat The sprintf style format string
 \param arglist The list of arguments

 \return A reference to the string
*/
String& String::Sprintf_va_list(const char8_t* pFormat, va_list arglist)
{
    rwFUNCTION();
    mAllocator->Modify(&mString);

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
#if defined(EA_PLATFORM_PS4)
	va_list cp1;
	va_copy(cp1,arglist);
    int nReturnValue = STRING_VSNPRINTF(mString, mAllocator->GetCapacity(mString), pFormat, cp1);
	va_end(cp1);
#else
	int nReturnValue = STRING_VSNPRINTF(mString, mAllocator->GetCapacity(mString), pFormat, arglist);
#endif
    if (nReturnValue >= static_cast<int32_t>(mAllocator->GetCapacity(mString)))
    {
        // In this case we definitely have C99 Vsnprintf behaviour.
        SetCapacity(static_cast<size_t>(nReturnValue+1)); //'+1' because 'nReturnValue' doesn't include the terminating 0.
        nReturnValue = STRING_VSNPRINTF(mString, mAllocator->GetCapacity(mString), pFormat, arglist);
    }
    else if (nReturnValue < 0)
    {
        // In this case we either have C89 extension behaviour or C99 behaviour.
        size_t n = eastl::max(RW_STRING_INITIAL_CAPACITY - 1, mAllocator->GetCapacity(mString) * 2); // - 1 because the resize call below will add one for NULL terminator and we want to keep allocations on fixed block sizes.

        for(; (nReturnValue < 0) && (n < RW_STRING_MAX_CAPACITY); n *= 2)
        {
            SetCapacity(n);
            if( mAllocator->GetCapacity(mString) )
			{

#if defined(EA_PLATFORM_PS4)
				va_list cp2;
				va_copy(cp2,arglist);
				nReturnValue = STRING_VSNPRINTF(mString, mAllocator->GetCapacity(mString)+1, pFormat, arglist);
				va_end(cp2);
#else
				nReturnValue = STRING_VSNPRINTF(mString, mAllocator->GetCapacity(mString)+1, pFormat, arglist);
#endif
			}
        }
    }

    if (nReturnValue >= 0)
    {
        mAllocator->SetLength(mString, static_cast<size_t>(nReturnValue));
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
String& String::Sprintf(const char8_t* pFormat, ...)
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
size_t String::GetLengthUTF8() const
{
    rwFUNCTION();
    rwRETURN(GetLengthUTF8(mString));
}
/**
 Returns the number of potentially multibyte characters that the passed string holds

 \param p The string to check

 \return A number of characters
*/
size_t String::GetLengthUTF8(const char8_t *p)
{
    rwFUNCTION();
    int32_t     nSize = 0;
    int32_t     iCharacter(0);
    const char *pBufferToParse = p;
    for(;; ++nSize)
    {
        pBufferToParse = ReadCharacterUTF8(pBufferToParse, &iCharacter);
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
int32_t String::GetCharacterUTF8(const char8_t *pBuffer)
{
    rwFUNCTION();
    int32_t iCharacter(0);
    ReadCharacterUTF8(pBuffer, &iCharacter);
    rwRETURN(iCharacter);
}
/**
 Reads the first potentially multibyte character in the string

 \param pBuffer The string to get the character from
 \param pCharacter OUTPUT The character that was read
*/
const char8_t* String::ReadCharacterUTF8(const char8_t *pBuffer, int32_t *pCharacter)
{
    rwFUNCTION();
    int32_t         unicode = 0;
    uint8_t         cChar0, cChar1, cChar2, cChar3;
    const char *    pCurrentBuffer;

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

    rwASSERT(UTF8_IsValid(unicode));
    *pCharacter = unicode;

    rwRETURN(pCurrentBuffer);
}
/**
 Returns the potentially multibyte character at the location specified in the current string

 \param n The position to get the character at

 \return The character
*/
int32_t String::GetAtUTF8(size_t n) const
{
    rwFUNCTION();
    const char8_t *pBuffer;
    pBuffer = GetBufferUTF8(mString, n);
    if (pBuffer == NULL)
    {
        //  After the end of the string
        rwRETURN(-1);
    }
    rwRETURN(GetCharacterUTF8(pBuffer));
}
/**
 Returns a pointer to the location specified in the potentially multibyte current string

 \param n The position to get

 \return A pointer to the location
*/
const char8_t *String::GetBufferUTF8(size_t n) const
{
    rwFUNCTION();
    rwRETURN(GetBufferUTF8(mString, n));
}
/**
 Returns a pointer to the location specified in the potentially multibyte passed string

 \param p The string
 \param n The position to get

 \return A pointer to the location
*/
const char8_t *String::GetBufferUTF8(const char8_t *p, size_t n)
{
    size_t         i;
    int32_t        iCharacter(0);
    const char8_t *pBufferToParse = p;

    for(i = 0 ; i < n ; ++i)
    {
        pBufferToParse = ReadCharacterUTF8(pBufferToParse, &iCharacter);
        if (iCharacter == 0)
        {
            //  If iIndex is greater than the UTF8 size, we indicate it by a NULL char8_t *
            rwRETURN(static_cast<char8_t*>(NULL));
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
void String::SetAtUTF8(size_t n, int32_t c)
{
    rwFUNCTION();
    mAllocator->Modify(&mString);

    size_t         i;
    int32_t        iCharacter(0);
    const char8_t *pBufferToParse = mString;

    for(i = 0 ; i < n ; ++i)
    {
        pBufferToParse = ReadCharacterUTF8(pBufferToParse, &iCharacter);
        if (iCharacter == 0)
        {
            //  If iIndex is greater than the UTF8 size, we don't do anything
            rwRETURNVOID();
        }
    }
    SetCharacterUTF8(const_cast<char8_t*>(pBufferToParse), c);
    rwRETURNVOID();
}
/**
 Sets the potentially multibyte character at the location specified in the passed string,
 this only works if the new character and old character are the same size

 \param pBuffer The string that points at the location to set
 \param iCharacter The character to set

 \return The character
*/
void String::SetCharacterUTF8(char8_t *pBuffer, int32_t iCharacter)
{
    rwFUNCTION();
    rwASSERT(iCharacter != 0);
    rwASSERT(GetCharacterUTF8(pBuffer) != 0);
    rwASSERT(GetCharacterSizeUTF8(pBuffer) == GetCharacterSizeUTF8(iCharacter));
    rwASSERT(UTF8_IsValid(iCharacter));

    if (iCharacter < 0x80)
    {
        *pBuffer = static_cast<char8_t>(iCharacter);
    }
    else if (iCharacter < 0x0800)
    {
        //lint -e{702} shift right of signed quantity (int)
        *(pBuffer + 0) = static_cast<char8_t>((iCharacter >> 6) | 0xC0);
        *(pBuffer + 1) = static_cast<char8_t>((iCharacter & 0x3F) | 0x80);
    }
    else if (iCharacter < 0x10000)
    {
        //lint -e{702} shift right of signed quantity (int)
        *(pBuffer + 0) = static_cast<char8_t>((iCharacter >> 12) | 0xE0);
        //lint -e{702} shift right of signed quantity (int)
        *(pBuffer + 1) = static_cast<char8_t>(((iCharacter >> 6) & 0x3F) | 0x80);
        *(pBuffer + 2) = static_cast<char8_t>((iCharacter & 0x3F) | 0x80);
    }
    else
    {
        //lint -e{702} shift right of signed quantity (int)
        *(pBuffer + 0) = static_cast<char8_t>((iCharacter >> 18) | 0xF0);
        //lint -e{702} shift right of signed quantity (int)
        *(pBuffer + 1) = static_cast<char8_t>(((iCharacter >> 12) & 0x3F) | 0x80);
        //lint -e{702} shift right of signed quantity (int)
        *(pBuffer + 2) = static_cast<char8_t>(((iCharacter >> 6) & 0x3F) | 0x80);
        *(pBuffer + 3) = static_cast<char8_t>((iCharacter & 0x3F) | 0x80);
    }
    rwRETURNVOID();
}
/**
 Appends the potentially multibyte string to the current string

 \param p The string to append
 \param n The number of potentially multibyte characters in the passed string

 \return A reference to the current string
*/
String& String::AppendUTF8(const char8_t* p, size_t n)
{
    rwFUNCTION();
    size_t         i;
    int32_t         iCharacter(0);
    const char *    pBuffer = p;
    for (i = 0 ; i < n ; ++i)
    {
        pBuffer = ReadCharacterUTF8(pBuffer, &iCharacter);
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
String& String::InsertUTF8(size_t pos, const char8_t* p, size_t n)
{
    rwFUNCTION();
    size_t         i;
    int32_t         iCharacter(0);
    const char *    pBuffer = p;
    for (i = 0 ; i < n ; ++i)
    {
        pBuffer = ReadCharacterUTF8(pBuffer, &iCharacter);
        if (iCharacter == 0)
        {
            break;
        }
    }
    const char *pPos = GetBufferUTF8(mString, pos);
    if (pPos == NULL)
    {
        // position is past end, append it instead
        rwRETURN(Append(p, pBuffer));
    }
    else
    {
        rwRETURN(Insert(static_cast<size_t>(pPos - mString), p, static_cast<size_t>(pBuffer - p)));
    }
}
/**
 Searches for a string in the potentially multibyte current string

 \param pStrText The string to search for
 \param iStart The start location in the current string

 \return The index of the search string in the current string if it exists,
         otherwise String::npos if not found
*/
size_t String::FindUTF8(const char8_t* pStrText, size_t iStart) const
{
    rwFUNCTION();
    const char *    pBuffer;
    const char *    pBufferStart = mString;
    pBuffer = GetBufferUTF8(pBufferStart, iStart);
    if (pBuffer == NULL)
    {
        //  The start is after the end, can't find it
        rwRETURN(npos);
    }

    size_t iStartASCII = static_cast<size_t>(pBuffer - pBufferStart);
    size_t iFoundASCII;

    //  We can use the standard search even in UTF8
    //  Because there is no ambiguity between ASCII and UTF8!
    iFoundASCII = Find(pStrText, iStartASCII);
    if (iFoundASCII == npos)
    {
        rwRETURN(npos);
    }

    //  We need now to convert the index found to the UTF8 index
    size_t          i;
    size_t          iFoundUTF8 = iStart;
    size_t          iSize;
    const char *    pCurrentBuffer = pBufferStart;
    for (i = iStartASCII ; i < iFoundASCII ; ++iFoundUTF8)
    {
        //  Read each UTF8 character, and update the ASCII index accordingly
        iSize = static_cast<size_t>(GetCharacterSizeUTF8(pCurrentBuffer));
        pCurrentBuffer += iSize;        //  Next UTF8 character
        i+= iSize;                      //  Fixup with the ASCII character

    }
    rwASSERT(i == iFoundASCII);   //  We check that the index are well synchronized
    rwRETURN(iFoundUTF8);
}
/**
 Returns a new string that is a sub-string of the potentially multibyte current string

 \param iFirst The start location in the current string
 \param iCount The number of characters desired

 \return A new string that is a copy of that portion of the current string
*/
String String::SubstrUTF8(size_t iFirst, size_t iCount) const
{
    rwFUNCTION();
    const char *    pBufferStart;
    const char *    pBufferFirst;
    const char *    pBufferCount;

    pBufferStart = mString;
    pBufferFirst = GetBufferUTF8(pBufferStart, iFirst);
    if (pBufferFirst == NULL)
    {
        //  First is after the end of the string
        rwRETURN(String());
    }
    pBufferCount = GetBufferUTF8(pBufferFirst, iCount);
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
void String::MakeLowerUTF8()
{
    rwFUNCTION();
    mAllocator->Modify(&mString);

    int32_t  iCharacter(0);
    char8_t *pBuffer = mString;
    char8_t *pPrevBuffer;
    for(;;)
    {
        pPrevBuffer = pBuffer;
        pBuffer = const_cast<char8_t*>(ReadCharacterUTF8(pBuffer, &iCharacter));
        if (iCharacter == 0)
        {
            break;
        }
        SetCharacterUTF8(pPrevBuffer, tolower(iCharacter));
    }
    rwRETURNVOID();
}
/**
 This is an upper case conversion function that only converts the ASCII characters in the
 potentially multibyte current string
*/
void String::MakeUpperUTF8()
{
    rwFUNCTION();
    mAllocator->Modify(&mString);

    int32_t  iCharacter(0);
    char8_t *pBuffer = mString;
    char8_t *pPrevBuffer;
    for(;;)
    {
        pPrevBuffer = pBuffer;
        pBuffer = const_cast<char8_t*>(ReadCharacterUTF8(pBuffer, &iCharacter));
        if (iCharacter == 0)
        {
            break;
        }
        SetCharacterUTF8(pPrevBuffer, toupper(iCharacter));
    }
    rwRETURNVOID();
}
/**
 Returns the size in bytes of the potentially multibyte character in the passed string

 \param p The string pointing to the character to check

 \return The size of the character
*/
int32_t String::GetCharacterSizeUTF8(const char8_t *p)
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
int32_t String::GetCharacterSizeUTF8(const int32_t c)
{
    rwFUNCTION();
    rwASSERT(UTF8_IsValid(c));
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
void String::RangeInitialize(const char8_t* p, size_t n)
{
    rwFUNCTION();
    rwASSERTM(n <= GetMaxSize(),
        ("String::RangeInitialize source string cannot exceed size_t - 2"));

    rwASSERTM(!((p == NULL) && (n != 0)),
        ("String::RangeInitialize called with an invalid buffer"));  // Only assert if n != 0.

    mString = mAllocator->Allocate(n+1);

    char8_t* pEnd = CharStringUninitializedCopy(mString, p, n);
    *pEnd = 0;
    mAllocator->SetLength(mString, static_cast<size_t>(pEnd - mString));
    rwRETURNVOID();
}
/**
\internal
 helper function to initialize a string by set all characters to the passed one
*/
void String::SizeInitialize(size_t n, char8_t c)
{
    rwFUNCTION();
    rwASSERTM(n <= GetMaxSize(),
        ("String::SizeInitialize source string cannot exceed size_t - 2"));

    mString = mAllocator->Allocate(n+1);

    char8_t* pEnd = CharStringUninitializedFillN(mString, n, c);
    *pEnd = 0;
    mAllocator->SetLength(mString, static_cast<size_t>(pEnd - mString));
    rwRETURNVOID();
}
/**
 CharTypeStringFind
 Specialized char version of STL find() function.
*/
const char8_t* String::StringFind(const char8_t* pBegin, const char8_t* pEnd, const char8_t c)
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
 StringFindEnd
 Specialized char version of STL find() from back function.
 Not the same as RFind because search range is specified as forward iterators.
*/
const char8_t* String::StringFindEnd(const char8_t* pBegin, const char8_t* pEnd, char8_t c)
{
    rwFUNCTION();
    const char8_t* pTemp = pEnd;
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
 StringRFind
 Specialized char8_t version of STL find() function in reverse.
*/
const char8_t* String::StringRFind(const char8_t* pRBegin, const char8_t* pREnd, const char8_t c)
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
 CharTypeStringFind
 Specialized char8_t version of STL find() function
 Purpose: find first instance of p2 within p1. Return p1Begin if not found or if either string is zero length.
*/
const char8_t* String::StringSearch(const char8_t* p1Begin, const char8_t* p1End, const char8_t* p2Begin, const char8_t* p2End)
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
        rwRETURN(StringFind(p1Begin, p1End, *p2Begin));
    }

    // General case.
    const char8_t* pTemp;
    const char8_t* pTemp1 = (p2Begin + 1);
    const char8_t* pCurrent = p1Begin;

    while (p1Begin != p1End)
    {
        p1Begin = StringFind(p1Begin, p1End, *p2Begin);
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
 StringRSearch
 Specialized char8_t version of STL find_end() function (which really is a reverse search function).
 Purpose: find last instance of p2 within p1. Return p1End if not found or if either string is zero length.
*/
const char8_t* String::StringRSearch(const char8_t* p1Begin, const char8_t* p1End, const char8_t* p2Begin, const char8_t* p2End)
{
    rwFUNCTION();
    // Test for zero length strings, in which case we have a match or a failure,
    // but the return value is the same either way.
    if ((p1Begin == p1End) || (p2Begin == p2End))
        rwRETURN(p1Begin);

    // Test for a pattern of length 1.
    if ((p2Begin + 1) == p2End)
        rwRETURN(StringFindEnd(p1Begin, p1End, *p2Begin));

    // Test for search string length being longer than string length.
    if ((p2End - p2Begin) > (p1End - p1Begin))
        rwRETURN(p1End);

    // General case.
    const char8_t* pSearchEnd = ((p1End - (p2End - p2Begin)) + 1);
    const char8_t* pCurrent1;
    const char8_t* pCurrent2;

    while (pSearchEnd != p1Begin)
    {
        // Search for the last occurrence of *p2Begin.
        pCurrent1 = StringFindEnd(p1Begin, pSearchEnd, *p2Begin);
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
 StringFindFirstOf
 Specialized char8_t version of STL find_first_of() function.
 This function is much like the C runtime strtok function, except the strings aren't null-terminated.
*/
const char8_t* String::StringFindFirstOf(const char8_t* p1Begin, const char8_t* p1End, const char8_t* p2Begin, const char8_t* p2End)
{
    rwFUNCTION();
    for ( ; p1Begin != p1End; ++p1Begin)
    {
        for (const char8_t* pTemp = p2Begin; pTemp != p2End; ++pTemp)
        {
            if (*p1Begin == *pTemp)
                rwRETURN(p1Begin);
        }
    }
    rwRETURN(p1End);
}

/**
 StringRFindFirstOf
 Specialized char8_t version of STL find_first_of() function in reverse.
 This function is much like the C runtime strtok function, except the strings aren't null-terminated.
*/
const char8_t* String::StringRFindFirstOf(const char8_t* p1RBegin, const char8_t* p1REnd, const char8_t* p2Begin, const char8_t* p2End)
{
    rwFUNCTION();
    for ( ; p1RBegin != p1REnd; --p1RBegin)
    {
        for (const char8_t* pTemp = p2Begin; pTemp != p2End; ++pTemp)
        {
            if (*(p1RBegin-1) == *pTemp)
                rwRETURN(p1RBegin);
        }
    }
    rwRETURN(p1REnd);
}


/**
 StringFindFirstNotOf
 Specialized char8_t version of STL find_first_not_of() function.
*/
const char8_t* String::StringFindFirstNotOf(const char8_t* p1Begin, const char8_t* p1End, const char8_t* p2Begin, const char8_t* p2End)
{
    rwFUNCTION();
    for( ; p1Begin != p1End; ++p1Begin)
    {
        const char8_t* pTemp;
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
 StringRFindFirstNotOf
 Specialized char8_t version of STL find_first_not_of() function in reverse.
*/
const char8_t* String::StringRFindFirstNotOf(const char8_t* p1RBegin, const char8_t* p1REnd, const char8_t* p2Begin, const char8_t* p2End)
{
    rwFUNCTION();
    for( ; p1RBegin != p1REnd; --p1RBegin)
    {
        const char8_t* pTemp;
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
String rw::core::operator+(const String& x, const String& y)
{
    rwFUNCTION();
#if GCCOPTIMIZE //defined to 0 as we don't want gcc optimization yet
    //!defined(__GNUC__) || (__GNUC__ >= 3) //GCC 2.x has a bug
    String result(String::CtorDoNotInitialize(), x.GetLength() + y.GetLength());
#else
    String::CtorDoNotInitialize temp;
    String result(temp, x.GetLength() + y.GetLength());
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
String rw::core::operator+(const char8_t* s, const String& y)
{
    rwFUNCTION();
    const size_t n = STRING_STRINGLENGTH(s);
#if GCCOPTIMIZE //defined to 0 as we don't want gcc optimization yet
    //!defined(__GNUC__) || (__GNUC__ >= 3) //GCC 2.x has a bug
    String result(String::CtorDoNotInitialize(), n + y.GetLength());
#else
    String::CtorDoNotInitialize temp;
    String result(temp, n + y.GetLength());
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
String rw::core::operator+(char8_t c, const String& y)
{
    rwFUNCTION();
#if GCCOPTIMIZE //defined to 0 as we don't want gcc optimization yet
    //!defined(__GNUC__) || (__GNUC__ >= 3) //GCC 2.x has a bug
    String result(String::CtorDoNotInitialize(), 1 + y.GetLength());
#else
    String::CtorDoNotInitialize temp;
    String result(temp, 1 + y.GetLength());
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
String rw::core::operator+(const String& x, const char8_t* p)
{
    rwFUNCTION();
    const size_t n = STRING_STRINGLENGTH(p);
#if GCCOPTIMIZE //defined to 0 as we don't want gcc optimization yet
    //!defined(__GNUC__) || (__GNUC__ >= 3) //GCC 2.x has a bug
    String result(String::CtorDoNotInitialize(), x.GetLength() + n);
#else
    String::CtorDoNotInitialize temp;
    String result(temp, x.GetLength() + n);
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
String rw::core::operator+(const String& x, const char8_t c)
{
    rwFUNCTION();
#if GCCOPTIMIZE //defined to 0 as we don't want gcc optimization yet
    //!defined(__GNUC__) || (__GNUC__ >= 3) //GCC 2.x has a bug
    String result(String::CtorDoNotInitialize(), x.GetLength() + 1);
#else
    String::CtorDoNotInitialize temp;
    String result(temp, x.GetLength() + 1);
#endif
    result.Append(x);
    result.PushOnBack(c);
    rwRETURN(result);
}

