// (c) 1999-2006 Criterion Software Limited and its licensors. All Rights Reserved


#include "ustr.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>


#ifndef ASSERT
#include <assert.h>
#define ASSERT assert
#endif

#if defined(_MSC_VER)
#pragma warning(disable: 4996) // warning C4996: 'fscanf' was declared deprecated
#endif

/*!
 * \file ustr.cpp
 * \brief Unicode string routines
 */

const USTR USTR_MIN = 0;
const USTR USTR_MAX = 0xffff;

/*
; ABSTRACT
;
; USTR - Unicode String data type
;
; SUMMARY
;
; #include "realfont.h"
;
; DESCRIPTION
;
; USTR is the data type used to abstract a Unicode string.
;
; SEE ALSO
;
; USTR Overview
;
; EXAMPLE
;
; ustr_eg.c
;
; END ABSTRACT
*/

/*
;
; ABSTRACT
;
; USTR_length - return the number of characters in a Unicode string
;
;
; SUMMARY
;
; #include "realfont.h"
;
; int USTR_length(str);
;
;    const USTR* str;    // string to count characters in
;
; DESCRIPTION
;
; Returns the number of characters (not bytes!) in the specified string.
; Does not include any terminating characters.
;
; RETURN VALUE
;
; Number of characters in specified string.
;
; SEE ALSO
;
; <HTML A HREF="../overview/ustr.htm>USTR Overview<HTML /A>
;
; EXAMPLE
;
; ustr_eg.c
;
; <HTML A HREF="ustr_eg.c">Download the source<HTML /A>
;
; END ABSTRACT
;
*/

int USTR_length(const USTR* str)
{
    int len;

    ASSERT(str != NULL);

    len = 0;
    while (*str != USTR_TERMINATOR)
    {
        ++str;
        ++len;
    }
    return len;
}

/*
;
; ABSTRACT
;
; USTR_findchar - find the target character in the source str
;
;
; SUMMARY
;
; #include "realfont.h"
;
; int USTR_find(source, target, pos);
;
;    const USTR* source;    // string to search
;    USTR target;           // character to find
;
; DESCRIPTION
;
; Finds the target character in the source string.
;
; RETURN VALUE
;
; The starting position of the target chararacter in source.  If
; target is not found USTR_NPOS is returned.
;
; SEE ALSO
;
; <HTML A HREF="../overview/ustr.htm>USTR Overview<HTML /A>
;
; EXAMPLE
;
; ustr_eg.c
;
; <HTML A HREF="ustr_eg.c">Download the source<HTML /A>
;
; END ABSTRACT
;
*/

int USTR_findchar(const USTR* src, USTR target)
{
    int pos;

    ASSERT(src != NULL);

    pos = 0;
    while (src[pos] != USTR_TERMINATOR)
    {
        if (src[pos] == target)
        {
            return pos;
        }
        ++pos;
    }
    return USTR_NPOS;
}

/*
;
; ABSTRACT
;
; USTR_findchar_reverse - find the target character in the source str
;    by searching from right to left (in the opposite direction from
;    USTR_findchar)
;
;
; SUMMARY
;
; #include "realfont.h"
;
; int USTR_findchar_reverse(search_start, src, target);
;
;    const USTR* search_start; // position where from the search starts
;    const USTR* src;          // string to search
;    USTR target;              // character to find
;
; DESCRIPTION
;
; Finds the target character in the source string.
;
; RETURN VALUE
;
; The starting position of the target chararacter in source.  If
; target is not found USTR_NPOS is returned.
;
; SEE ALSO
;
; <HTML A HREF="../overview/ustr.htm>USTR Overview<HTML /A>
;
; EXAMPLE
;
; ustr_eg.c
;
; <HTML A HREF="ustr_eg.c">Download the source<HTML /A>
;
; END ABSTRACT
;
*/

int USTR_findchar_reverse(const USTR* src, int startPos, USTR target)
{
    const USTR* pos;

    ASSERT(src != NULL);

    pos = src + startPos;
    while (pos >= src)
    {
        if (*pos == target)
        {
            return pos - src;
        }

        pos--;
    }

    return USTR_NPOS;
}

/*
;
; ABSTRACT
;
; USTR_find - find the target string in the source str
;
;
; SUMMARY
;
; #include "realfont.h"
;
; int USTR_find(source, target);
;
;    const USTR* source;    // string to search
;    const USTR* target;    // string to find
;
; DESCRIPTION
;
; Finds the target string in the source string.
;
; RETURN VALUE
;
; The starting character position of target in source.  If
; target is not found USTR_NPOS is returned.
;
; SEE ALSO
;
; <HTML A HREF="../overview/ustr.htm>USTR Overview<HTML /A>
;
; EXAMPLE
;
; ustr_eg.c
;
; <HTML A HREF="ustr_eg.c">Download the source<HTML /A>
;
; END ABSTRACT
;
*/

int USTR_find(const USTR* src, const USTR* target)
{
    int targetlen;
    int pos;

    ASSERT(src != NULL);
    ASSERT(target != NULL);

    targetlen = USTR_length(target);

    pos = 0;
    while (src[pos] != USTR_TERMINATOR)
    {
        if (USTR_ncompare(src+pos, target, targetlen) == 0)
        {
            return pos;
        }
        ++pos;
    }
    return USTR_NPOS;
}

/*
;
; ABSTRACT
;
; USTR_compare - compare two strings
; USTR_ncompare - compare two partial strings
;
;
; SUMMARY
;
; #include "realfont.h"
;
; int USTR_compare(str1, str2);
; int USTR_ncompare(str1, str2, count);
;
;    const USTR* str1;    // first string to compare
;    const USTR* str2;    // second string to compare
;    int         count;   // number of characters to compare
;
; DESCRIPTION
;
; Compares the two strings.  The strings must be terminated
; with a USTR_TERMINATOR.  Note this function does a simple
; character compare and does not take into account certain
; locale specific comparisons.  Because of this some strings
; in some languages will not compare correctly.  This will
; be fixed in future versions of the library.
;
; RETURN VALUE
;
; The return value for each of these functions indicates the
; lexicographic relation of str1 to str2.
;
; Value   Relationship
; -----   ------------
; < 0     str1 is less than str2
; 0       str1 is identical to str2
; > 0     str1 is greater than str2
;
; SEE ALSO
;
; <HTML A HREF="../overview/ustr.htm>USTR Overview<HTML /A>
;
; EXAMPLE
;
; ustr_eg.c
;
; <HTML A HREF="ustr_eg.c">Download the source<HTML /A>
;
; END ABSTRACT
;
*/

int USTR_compare(const USTR* str1, const USTR* str2)
{
    return USTR_ncompare(str1, str2, INT_MAX);
}

int USTR_ncompare(const USTR* str1, const USTR* str2, int count)
{
    int pos;
    int result;

    ASSERT(str1 != NULL);
    ASSERT(str2 != NULL);
    ASSERT(count >= 0);

    pos    = 0;
    result = 0;
    while ((pos < count) && (result == 0))
    {
        result = str1[pos] - str2[pos];

        /* if we come to end of a string we break */
        /* but we want to compute result first (so this goes here instead of in the while loop) */
        /* we want to compute a result even if one of the strings has a NULL */
        if ((str1[pos] == USTR_TERMINATOR) || (str1[pos] == USTR_TERMINATOR))
            break;

        ++pos;
    }
    return result;
}

/*
;
; ABSTRACT
;
; USTR_copy - copy a string into another string
; USTR_ncopy - copy a partial string into another string
;
;
; SUMMARY
;
; #include "realfont.h"
;
; void USTR_copy(dst, src);
; void USTR_ncopy(dst, src, count);
;
;    USTR       *dst;     // string to copy into
;    const USTR* src;     // string to copy from
;    int         count;   // number of characters to copy from src
;
; DESCRIPTION
;
; Copy src into dst.  The strings must be terminated with a
; USTR_TERMINATOR.  You must ensure that dst is large enough
; to hold all the characters that will be copied into it.
;
; RETURN VALUE
;
; None.
;
; SEE ALSO
;
; <HTML A HREF="../overview/ustr.htm>USTR Overview<HTML /A>
;
; EXAMPLE
;
; ustr_eg.c
;
; <HTML A HREF="ustr_eg.c">Download the source<HTML /A>
;
; END ABSTRACT
;
*/

void USTR_copy(USTR* dst, const USTR* src)
{
    USTR_ncopy(dst, src, INT_MAX);
}

void USTR_ncopy(USTR* dst, const USTR* src, int count)
{
    int pos;

    ASSERT(dst != NULL);
    ASSERT(src != NULL);
    ASSERT(count >= 0);

    pos = 0;
    while ((pos < count) && (src[pos] != USTR_TERMINATOR))
    {
        dst[pos] = src[pos];
        ++pos;
    }
    dst[pos] = USTR_TERMINATOR;
}

/* prevents sign extension warnings */
#define USTR_ASCII2UNICODE(ch)  ((USTR)((unsigned char)(ch)))
/*
;
; ABSTRACT
;
; USTR_copycstr - copy a 8 bit C string into a Unicode string
;
;
; SUMMARY
;
; #include "realfont.h"
;
; void USTR_copycstr(dst, csrc);
;
;    USTR       *dst;     // Unicode string to copy into
;    const char *csrc;    // 8 bit ANSI C string to copy from
;
; DESCRIPTION
;
; Copy and convert csrc into dst.  cstr must be terminated with
; a NULL character.  You must ensure that dst is large enough
; to hold all the characters that will be copied into it.
;
; The common use of this function is to convert a C string
; created with sprintf into a Unicode string so that you
; can use USTR_format1 and USTR_format2.
;
; RETURN VALUE
;
; None.
;
; SEE ALSO
;
; <HTML A HREF="../overview/ustr.htm>USTR Overview<HTML /A>
;
; EXAMPLE
;
; ustr_eg.c
;
; <HTML A HREF="ustr_eg.c">Download the source<HTML /A>
;
; END ABSTRACT
;
*/
void USTR_copycstr(USTR* dst, const char *csrc)
{
    int pos;

    ASSERT(dst != NULL);
    ASSERT(csrc != NULL);

    pos = 0;
    while (csrc[pos] != '\0')
    {
        dst[pos] = USTR_ASCII2UNICODE(csrc[pos]);
        ++pos;
    }
    dst[pos] = USTR_TERMINATOR;
}

/*
;
; ABSTRACT
;
; USTR_tocstr - convert a UNICODE string to its ASCII representation
;
;
; SUMMARY
;
; #include "realfont.h"
;
; void USTR_tocstr(dst, src);
;
;    char*        dst;    // 8 bit ANSI C string to copy into
;    const USTR* src;    // UNICODE string to copy from
;
; DESCRIPTION
;
; Copy and convert src into dst.  src must be terminated with
; a USTR_TERMINATOR character.  You must ensure that dst is large
; enough to hold all the characters that will be copied into it.
;
; Since this conversion is not always possible, UNICODE characters
; that don't have an appropriate ASCII representation are converted
; into blank spaces.  Length of dst upon termination of USTR_tocstr
; shall be the same as the length of src.
;
; RETURN VALUE
;
; None.
;
; SEE ALSO
;
; <HTML A HREF="../overview/ustr.htm>USTR Overview<HTML /A>
;
; EXAMPLE
;
; ustr_eg.c
;
; <HTML A HREF="ustr_eg.c">Download the source<HTML /A>
;
; END ABSTRACT
;
*/

void USTR_tocstr(char* dst, const USTR* src)
{
    const USTR* pos;
    USTR wch;

    ASSERT(dst);
    ASSERT(src);

    pos = src;
    while (*pos != USTR_TERMINATOR)
    {
        wch = *pos;
        if (wch > UCHAR_MAX)
            wch = ' ';

        *dst = (char)wch;

        pos++;
        dst++;
    }

    *dst = '\0';
}

/*******************************************************************/
/* formatting */

/*
;
; ABSTRACT
;
; USTR_format1 - format a string with runtime data
; USTR_format2
;
;
; SUMMARY
;
; #include "realfont.h"
;
; void USTR_format1(dst, formatstr, str1);
; void USTR_format2(dst, formatstr, str1, str2);
;
;    USTR       *dst;       // string to store result into
;    const USTR* formatstr; // formating string
;    const USTR* str1;      // string to replace %1 with
;    const USTR* str2;      // string to replace %2 with
;
; DESCRIPTION
;
; Substitutes the characters "%1" and "%2" with str1 and str2.
; The newly formed string is stored in dst. For example, if the
; formatstr is "File %1 not found", and str1 is equal to
; "C:\MYFILE.TXT", then dst will contain the string
; “File C:\MYFILE.TXT not found”.
;
; If the format characters "%1" or "%2" appear in the string more
; than once, multiple substitutions will be made. They do not have
; to be in numerical order.
;
; RETURN VALUE
;
; None.
;
; SEE ALSO
;
; <HTML A HREF="../overview/ustr.htm>USTR Overview<HTML /A>
;
; EXAMPLE
;
; ustr_eg.c
;
; <HTML A HREF="ustr_eg.c">Download the source<HTML /A>
;
; END ABSTRACT
;
*/

void USTR_format1(USTR* dst, const USTR* formatstr, const USTR* str1)
{
    USTR_format2(dst, formatstr, str1, NULL);
}

void USTR_format2(USTR* dst, const USTR* formatstr, const USTR* str1, const USTR* str2)
{
    /* replace the %1 in formatstr with str1, result goes into dst */
    /* count indicates max number of characters that can fit into dst */

    ASSERT(dst != NULL);
    ASSERT(formatstr != NULL);

    while (*formatstr != USTR_TERMINATOR)
    {
        if ((*formatstr == '%') && (*(formatstr+1) == '1'))
        {
            ASSERT(str1 != NULL);

            formatstr += 2; /* advance over %1 */
            USTR_copy(dst, str1);
            dst += USTR_length(str1);
        }
        else if ((*formatstr == '%') && (*(formatstr+1) == '2'))
        {
            ASSERT(str2 != NULL);

            formatstr += 2; /* advance over %2 */
            USTR_copy(dst, str2);
            dst += USTR_length(str2);
        }
        else
        {
            *dst = *formatstr;
            ++dst;
            ++formatstr;
        }
    }
    *dst = USTR_TERMINATOR;
}


// NEW USTR functions as of June 2002

/*
;
; ABSTRACT
;
; USTR_strcpy - copy Unicode string
;
;
; SUMMARY
;
; #include "realfont.h"
;
; void USTR_strcpy(USTR *dst, const char *src);
; void USTR_strcpy(char *dst, const USTR *src);
;
; DESCRIPTION
;
; Copy and convert src into dst.  src must be terminated with
; a NULL character.  You must ensure that dst is large enough
; to hold all the characters that will be copied into it.
;
; Unicode values 0..255 will use ascii encoding.
; Values >= 256 are encoded/decoded using the following notation
: \x1234 where 4 digits are the hexadecimal unicode.  There must be
; exactly 4 digits (0..9,a..f)
;
; RETURN VALUE
;
; None.
;
; SEE ALSO
;
; <HTML A HREF="../overview/ustr.htm>USTR Overview<HTML /A>
;
; EXAMPLE
;
; ustr_eg.c
;
; <HTML A HREF="ustr_eg.c">Download the source<HTML /A>
;
; END ABSTRACT
;
*/

#if defined(_MSC_VER)
#pragma optimize("g", off)
#endif

void USTR_strcpy(USTR *dst, const char *src)
{
    ASSERT(dst != NULL);
    ASSERT(src != NULL);

    while (*src)
    {
        bool isunicodemarkup=false;
        int unicode=0;
        // ie \x1a34
        if (src[0]=='\\' && src[1]=='x')
        {
            int digit=0;
            isunicodemarkup=true;
            for (int i=0; i<4 && src[2+i] && isunicodemarkup; ++i)
            {
                if (src[2+i]>='0' && src[2+i]<='9')
                    digit = ((int)(unsigned char)src[2+i])-'0';
                else if (src[2+i]>='a' && src[2+i]<='f')
                    digit = ((int)(unsigned char)src[2+i])-'a'+10;
                else
                {
                    isunicodemarkup = false;
                    break;
                }
                unicode = (unicode<<4)+digit;
            }
        }
        if (isunicodemarkup)
        {
            *dst++ = (USTR)unicode;
            src += 6;
        }
        else
        {
            *dst++ = (USTR)(unsigned char)*src;
            src += 1;
        }
    }
    *dst = USTR_TERMINATOR;
}

void USTR_strcpy(char *dst, const USTR *src)
{
    ASSERT(dst != NULL);
    ASSERT(src != NULL);

    while (*src)
    {
        // ie \x1a34
        if (src[0]>=256 || (src[0]==(USTR)'\\' && src[1]==(USTR)'x'))
        {
            sprintf(dst,"\\x%04x",src[0]);
            dst += 6;
        }
        else
            *dst++ = (char)(unsigned char)*src;
        ++src;
    }
    *dst++ = '\0';
}


