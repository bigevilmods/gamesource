// (c) 1994-2007 Electronic Arts Inc.

#include <string.h>
#include "gimex.h"
#include "newgimex.h"

/****************************************************************/
/*  Export Functions                                            */
/****************************************************************/

int GCALL NEW_wopen(GINSTANCE **gxptr, GSTREAM *g, const char *pathname, int numframes)
{
    return(1);
}

int GCALL NEW_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes)
{
    return(1);
}

int GCALL NEW_wclose(GINSTANCE *gx)
{
    return(1);
}


