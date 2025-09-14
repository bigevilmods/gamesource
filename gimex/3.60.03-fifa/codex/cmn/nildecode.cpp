// (c) 1997-2007 Electronic Arts Inc.

#ifndef __NILREAD
#define __NILREAD 1

#include <string.h>
#include "codex.h"
#include "nilcodex.h"

/****************************************************************/
/*  Information Functions                                       */
/****************************************************************/

bool GCALL CNIL_is(const void *compresseddata)
{
    return false;
}


/****************************************************************/
/*  Decode Functions                                            */
/****************************************************************/

int GCALL CNIL_size(const void *compresseddata)
{
    return 0;
}

int GCALL CNIL_decode(void *dest, const void *compresseddata, int *compressedsize)
{
    return 0;
}

#endif

