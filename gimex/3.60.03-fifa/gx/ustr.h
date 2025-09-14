// (c) 1999-2006 Criterion Software Limited and its licensors. All Rights Reserved

#ifndef __USTR_H
#define __USTR_H 1

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************************/
/* USTR API - Unicode String API                                            */
/****************************************************************************/

/* datatypes */
typedef unsigned short  USTR;       /* USTR stands for Unicode String */

/* constants */
#define USTR_NPOS       (-1)
#define USTR_TERMINATOR (USTR)(0)

/* attributes */
int     USTR_length(const USTR *str);                       /* returns number of characters excluding terminator */

/* searching */
int     USTR_findchar(const USTR *src, USTR target);        /* returns USTR_NPOS if target not found in src*/
int     USTR_find(const USTR *src, const USTR *target);     /* returns USTR_NPOS if target not found in src*/
int     USTR_compare (const USTR *str1, const USTR *str2);  /* returns < 0 if str1 < str2, 0 if str == str2, > 0 if str1 > str2 */
int     USTR_ncompare(const USTR *str1, const USTR *str2, int count);   /* returns < 0 if str1 < str2, 0 if str == str2, > 0 if str1 > str2 */

/* copying */
void    USTR_copy (USTR *dst, const USTR *src);             /* copies src into dst */
void    USTR_ncopy(USTR *dst, const USTR *src, int count);  /* copies count characters from src into dst */
void    USTR_copycstr(USTR *dst, const char *csrc);         /* copies ASCII string csrc into dst */

/* formatting */
void    USTR_format1(USTR *dst, const USTR *formatstr, const USTR *str1);
void    USTR_format2(USTR *dst, const USTR *formatstr, const USTR *str1, const USTR *str2);
        /* replaces %1 and %2 with str1 and str2, they do not have to be in numerical order, having multiple %1/%2's are OK */
        /* NOTE: To format numbers using sprintf with a C style string and then use USTR_copycstr to make a Unicode string. */

#ifdef __cplusplus
}

void USTR_strcpy(USTR *dst, const char *src);
void USTR_strcpy(char *dst, const USTR *src);

#endif

#endif /* __REALFONT_H */

