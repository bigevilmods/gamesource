// (c) Electronic Arts. All Rights Reserved.

/// \file
/// Interface to allow the RW and platform overloading of string.h

#ifndef RW_STRING_H
#define RW_STRING_H

/*************************************************************************************************************
 Includes
 */
#include <limits.h>
#if defined(__PPCBROADWAY__)
#include <../include/string.h>  // Metrowerks includes this file unless specified like ../include/string.h.
#else
#include <string.h>
#endif
#include <stdio.h>
#include <stdarg.h>

#include <rw/core/base/ostypes.h>

/*************************************************************************************************************
 Defines
 */

#define RwMemchr(SRC, C, LENGTH)       memchr(SRC, C, LENGTH)
#define RwMemcmp(S1, S2, N)            memcmp(S1, S2, N)
void *  RwMemcpy(void *, const void *, size_t);
void *  RwMemmove(void *, const void *, size_t);
#define RwMemset(DST, C, LENGTH)       memset(DST, C, LENGTH)

#define RwStrcat(DST, SRC)             strcat(DST, SRC)
#define RwStrchr(STRING, C)            strchr(STRING, C)
#define RwStrcmp(A, B)                 strcmp(A, B)
#define RwStrcoll(STRA, STRB)          strcoll(STRA, STRB)
#define RwStrcpy(DST, SRC)             strcpy(DST, SRC)
#define RwStrcspn(S1, S2)              strcspn(S1, S2)
#define RwStrerror(ERRNUM)             strerror(ERRNUM)
#define RwStrlen(STR)                  strlen(STR)
#define RwStrncat(DST, SRC, LENGTH)    strncat(DST, SRC, LENGTH)
#define RwStrncmp(A, B, LENGTH)        strncmp(A, B, LENGTH)
#define RwStrncpy(DST, SRC, LENGTH)    strncpy(DST, SRC, LENGTH)
#define RwStrpbrk(S1, S2)              strpbrk(S1, S2)
#define RwStrrchr(STRING, C)           strrchr(STRING, C)
#define RwStrspn(S1, S2)               strspn(S1, S2)
#define RwStrstr(S1, S2)               strstr(S1, S2)
#define RwStrtok(SOURCE, DELIM)        strtok(SOURCE, DELIM)
#define RwStrxfrm(S1, S2, N)           strxfrm(S1, S2, N)

#if defined(_MSC_VER)
#define RwStricmp _stricmp
#define RwStrnicmp _strnicmp
#define RwVsnprintf rwSafeVsnprintf
#elif defined RWCOMPILERPRODG
#define RwVsnprintf rwSafeVsnprintf
#endif

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
 Global Variables
 */

/*************************************************************************************************************
 Function prototypes
 */

#ifndef RwStricmp
extern int32_t RwStricmp(const RwChar *string1, const RwChar *string2);
#endif

#ifndef RwStrnicmp
extern int32_t RwStrnicmp(const RwChar *string1, const RwChar *string2, uint32_t length);
#endif

#ifndef RwStristr
extern RwChar * RwStristr(RwChar *string1, const RwChar *string2);
#endif

RwChar * rwGenericSprintf(const RwChar *format, ...);
RwChar *rwGenericSprintfVL(const RwChar *format, va_list args);

#ifdef RwVsnprintf
extern int32_t
rwSafeVsnprintf(RwChar *buffer, size_t count, const RwChar *format, va_list argptr);
#endif

#ifndef RwVsnprintf
extern int32_t
RwVsnprintf(char *buffer, size_t count, const RwChar *format, va_list argptr);
#endif

extern int32_t
RwSnprintf(RwChar *buffer, size_t count, const RwChar *format, ...);

#endif /* RW_STRING_H */
