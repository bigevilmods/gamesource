// (c) 1998-2004 Electronic Arts Inc.

#ifndef _GX_H_
#define _GX_H_

#define IDD_DIALOG                      101
#define IDI_APP                         101

#ifdef GCMP
#define TOOL_NAME "gcmp"
#else
#define TOOL_NAME "gx"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(GCALL)
#if defined(_MSC_VER) && !defined(_XBOX)
#define GCALL __stdcall
#else
#define GCALL
#endif
#endif

int GCALL gx(const char *commandline);
void GCALL GX_setprintf(int (*printf)(const char *format, ...));

#ifdef __cplusplus
}
#endif

#endif

