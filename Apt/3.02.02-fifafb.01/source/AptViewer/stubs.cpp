#if !defined(EA_PLATFORM_PSP)
#include "realfile.h"
#include "realinput.h"
#include "realdebug.h"
#include "realcodec.h"
#include "realmem.h"
#include "realmath.h"

//#include "libemu.h"
#endif
#include <stdio.h>

#if !defined(__SN_VER__)

#ifdef __cplusplus
extern "C" {
#endif

void __assert(const char * file, int line, const char *cond)
{
    DEBUG_abortmessage( "ERROR: ASSERT \"%s\", in file \"%s\", line (%d)\n", cond, file, line );
}


FILE *  fopen(const char *_name, const char *_type)
{
    return NULL;
}

int fseek(FILE *, long, int)
{
    return -1;
}

long ftell( FILE *)
{
    return -1;
}

size_t fread(_PTR, size_t _size, size_t _n, FILE *)
{
    return 0;
}

int fclose(FILE *)
{
    return -1;
}

#ifdef __cplusplus
} // extern "C"
#endif

#endif
