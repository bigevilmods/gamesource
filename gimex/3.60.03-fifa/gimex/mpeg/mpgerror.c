// (c) 2000-2007 Electronic Arts Inc.

#include <stdio.h>

void error(char *text)
{
#ifdef DEBUG
    printf(text);
#endif
}


