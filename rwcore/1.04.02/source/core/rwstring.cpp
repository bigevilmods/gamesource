// (c) Electronic Arts. All Rights Reserved.

/// \file
/// String functions.

/*************************************************************************************************************
 Includes
 */
#include "rwsharedheaders.hpp"
#include <string.h>
#include <stdlib.h>

/*************************************************************************************************************
 Defines
 */

/**
Used by rwGenericSprintf function to control length of output string buffer.
*/
#define rw_MAXGENERICSPRINTFSTRINGLEN 1024

/*************************************************************************************************************
 Enums
 */

/*************************************************************************************************************
 Typedef Enums
 */

/*************************************************************************************************************
 Typedef Structs + Unions
 */

/*************************************************************************************************************
 Typedef Function Pointers
 */

/*************************************************************************************************************
 Structs + Unions
 */

/*************************************************************************************************************
 Static Variables
 */

/*************************************************************************************************************
 Global Variables
 */

/*************************************************************************************************************
 Function Prototypes
 */

/*************************************************************************************************************
 Static Functions
 */

/*************************************************************************************************************
 Internal Functions
 */

#ifndef ABS
#define ABS(x) ((x) > 0 ? (x) : -(x))
#endif

/**
RwMemcpy Copies memory from source to destination.  Asserts if the buffers overlap.
Use RwMemmove to copy data if the buffers may overlap.
\param dst Memory location to copy data to.
\param src Memory location to copy data from.
\param nbytes Number of bytes to copy
\return The value of dst.
\sa RwMemmove
*/
void *  RwMemcpy(void *dst, const void *src, size_t nbytes)
{
    rwFUNCTION();

    // assert if the buffers overlap
    rwASSERT(static_cast<size_t>(ABS((intptr_t)dst-(intptr_t)src)) >= nbytes);

    rwRETURN(memcpy(dst, src, nbytes));
}

/**
RwMemmove Copies memory from source to destination.  The source and destination
buffers may overlap.  We use own own implementation because on PS3, memmove can
still call memcpy even if the buffer overlap, which is possibly unsafe.
\param dst Memory location to copy data to.
\param src Memory location to copy data from.
\param nbytes Number of bytes to copy
\return The value of dst.
\sa RwMemcopy
*/

void *  RwMemmove(void *dst, const void *src, size_t nbytes)
{
    rwFUNCTION();

    const unsigned char* sc=static_cast<const unsigned char*>(src);
    unsigned char* dc=static_cast<unsigned char*>(dst);

    if (dc < sc)
    {
        // copy from the beginning toward the end
        while (nbytes != 0)
        {
            *dc++ = *sc++;
            --nbytes;
        }
    }
    else
    {
        // copy from the end toward the beginning
        sc += nbytes;
        dc += nbytes;

        while (nbytes != 0)
        {
            *--dc = *--sc;
            --nbytes;
        }
   }

    rwRETURN(dst);
}

/**
rwGenericSprintf inline version of printf style argument list to string.
\param format The 'printf' style format string / text.
\param ... The variable argument list for the parameters of the printf string.
\return A string holding the formatted text. Do NOT delete this string. The maximum length of this string
        is dictated by the rw_MAXGENERICSPRINTFSTRINGLEN value.
*/
RwChar *
rwGenericSprintf(const RwChar *format, ...)
{
    va_list ap;
    RwChar *pString;

    rwFUNCTION();

    /* No debug macros here */

    va_start(ap, format);
    pString = rwGenericSprintfVL(format, ap);
    va_end(ap);

    rwRETURN(pString);
}

/**
rwGenericSprintfVL takes a va_list and so can be called by a function who's input is a variable argument
list (...) such as rwGenericSprintf or rwDebugGenericSprintf.
\param format The 'printf' style format string / text.
\param args The variable argument list for the parameters of the printf string (already had va_start applied
            to it).
\return A string holding the formatted text. Do NOT delete this string. The maximum length of this string
*/
RwChar *
rwGenericSprintfVL(const RwChar *format, va_list args)
{
    rwFUNCTION();

    static RwChar outputMsg[rw_MAXGENERICSPRINTFSTRINGLEN];

    RwVsnprintf(outputMsg, rw_MAXGENERICSPRINTFSTRINGLEN, format, args);

    rwRETURN(outputMsg);
}

/*************************************************************************************************************
 External Functions
 */

#ifndef RwStricmp
int32_t
RwStricmp(const RwChar *string1, const RwChar *string2)
{
    RwChar char1;
    RwChar char2;

    rwFUNCTION();
    rwASSERT((const RwChar *)0 != string1);
    rwASSERT((const RwChar *)0 != string2);

    do
    {
        char1 = *string1++;
        if ('A' <= char1 && 'Z' >= char1) char1 += 32;
        char2 = *string2++;
        if ('A' <= char2 && 'Z' >= char2) char2 += 32;
        if (char1 != char2) rwRETURN(char1 - char2);
    } while (0 != char1);

    rwRETURN(0);
}
#endif

#ifndef RwStrnicmp
int32_t
RwStrnicmp(const RwChar *string1, const RwChar *string2, uint32_t length)
{
    RwChar char1;
    RwChar char2;

    rwFUNCTION();
    rwASSERT((const RwChar *)0 != string1);
    rwASSERT((const RwChar *)0 != string2);
    rwASSERT(0 < length);

    do
    {
        char1 = *string1++;
        if ('A' <= char1 && 'Z' >= char1) char1 += 32;
        char2 = *string2++;
        if ('A' <= char2 && 'Z' >= char2) char2 += 32;
        if (char1 != char2) rwRETURN(char1 - char2);
        length--;
    } while (0 != char1 && 0 != length);

    rwRETURN(0);
}
#endif

#ifndef RwStristr
/**
Convert a character to it's upper case equivalent.

\param character The character to be converted

\return The upper case conversion of the supplied character
*/
static RwChar
upperchar(RwChar character)
{
    rwFUNCTION();

    if (character >= 'a' && character <= 'z')
    {
        character -= 'a' - 'A';
    }

    rwRETURN(character);
}

/**
Determine if a string exists within another string, with a case insensitive comparison.

\param string1 The string to be searched
\param string2 The string to search for

\return Pointer to the string to search for within the source string, or NULL if not found
*/

RwChar *
RwStristr(RwChar *string1, const RwChar *string2)
{
    size_t i;
    size_t j;
    size_t string2Length;
    size_t inLength;
    RwChar first;

    rwFUNCTION();

    string2Length = strlen(string2);
    inLength = strlen(string1) - string2Length;
    first = upperchar(string2[0]);

    for (i = 0; i < inLength; i++)
    {
        if (upperchar(string1[i]) == first)
        {
            for (j = 1; j < string2Length; j++)
            {
                if (upperchar(string1[i + j]) != upperchar(string2[j]))
                {
                    break;
                }
            }

            if (j == string2Length)
            {
                rwRETURN(&string1[i]);
            }
        }
    }

    rwRETURN(NULL);
}
#endif

#ifdef RwVsnprintf
/**
A Safe version of _vsnprintf(). _vsnprintf() will not NULL terminate it's target
string if it runs out of space. This version guarantees that the target string is
NULL terminated.

\param buffer Target buffer for output.
\param count Maximum number of characters to write.
\param format Format specification.
\param argptr Pointer to list of arguments.

\return The same as _snprintf. Returns the number of characters written to the
given buffer. If the number of characters required to write the string exceeds count,
then count characters are stored in buffer and a negative value is returned. The returned
buffer is guaranteed to be NULL terminated.
*/
int32_t
rwSafeVsnprintf(RwChar *buffer, size_t count, const RwChar *format, va_list args)
{
    int32_t charactersWritten;

    rwFUNCTION();
    rwASSERT(count);

    /* _vsnprintf does not put a NULL terminator at the end of the string if there is not
     * enough room in the given buffer. So ensure that there is always a NULL terminator */
    charactersWritten = _vsnprintf(buffer, count, format, args);

    /* make sure that it is NULL terminated */
    buffer[count-1] = (RwChar)0;

    rwRETURN(charactersWritten);
}
#endif

#ifndef RwVsnprintf
/**
An unsafe _vsnprintf() wrapper. The 'count' parameter is ignored and the target string
is assumed to have enough space.

\param buffer Target buffer for output.
\param count *UNUSED* Maximum number of characters to write.
\param format Format specification.
\param argptr Pointer to list of arguments.

\return The same as _snprintf. Returns the number of characters written to the
given buffer, excluding NULL termination.
*/
int32_t
RwVsnprintf(char *buffer, size_t RWUNUSED_RELEASE(count), const RwChar *format, va_list args)
{
    int32_t charactersWritten;

    rwFUNCTION();
    rwASSERT(count);

    /* we really don't want people using this function - or
     * someone needs to write a generic _vsnprintf() */

    // don't use rwDEPRECATED here or there will be infinite recursion

    /* TODO: don't do the length test - NASTY */
    charactersWritten = vsprintf(buffer, format, args);

    rwRETURN(charactersWritten);
}
#endif


/**
An _snprintf() wrapper. If this platform does not have a _vsnprintf() function
then this function is unsafe.

\param buffer Target buffer for output.
\param count Maximum number of characters to write. If this platform does not have
a _vsnprintf function then this parameter is *ignored*.
\param format Format specification.
\param argptr Pointer to list of arguments.

\return The same as _snprintf. Returns the number of characters written to the
given buffer, excluding NULL termination.
*/
int32_t
RwSnprintf(RwChar *buffer, size_t count, const RwChar *format, ...)
{
    va_list args;
    int32_t numCharacters;

    rwFUNCTION();

    va_start(args, format);
    numCharacters = RwVsnprintf(buffer, count, format, args);
    va_end(args);

    rwRETURN(numCharacters);
}
