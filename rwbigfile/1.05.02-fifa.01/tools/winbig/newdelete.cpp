
#include <stdlib.h>

void *operator new[](size_t s, const char *, int, unsigned int, const char *, int)
{
    return malloc(s);
}
