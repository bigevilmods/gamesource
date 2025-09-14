// (c) 1994-2007 Electronic Arts Inc.

/*------------------------------------------------------------------*/
/*                                                                  */
/*                 EAC Library Codex Interface v1.00                */
/*                                                                  */
/*                     by FrANK G. Barchard, EAC                    */
/*                                                                  */
/*                     Code Module - Oct 11, 1994                   */
/*                                                                  */
/*------------------------------------------------------------------*/

#include "eac.h"
#include "eacx.h"
#include "codex.h"

/* get motorola memory */

#ifdef ggetm
#undef ggetm
#undef ggeti
#endif
#ifdef gputm
#undef gputm
#undef gputi
#endif

unsigned int ggetm(void *src, int bytes)
{
    unsigned char *s = (unsigned char *) src;
    unsigned int value;

    value = 0L;
    while (bytes--)
    {
        value = (value<<8) + ((*s++));
    }
    return(value);
}

/* get intel memory */

unsigned int ggeti(void *src, int bytes)
{
    unsigned char *s = (unsigned char *) src;
    int            i = 0;
    unsigned int  value;

    value = 0L;
    while (bytes--)
    {
        value += ((*s++)) << (i);
        i += 8;
    }
    return(value);
}

/* put motorolla memory */

void gputm(void *dst, unsigned int data, int bytes)
{
    unsigned char *d = (unsigned char *) dst;
    unsigned int       pval;

    data <<= (4-bytes)*8;
    while (bytes)
    {
        pval = data >>  24;
        *d++  = (unsigned char) pval;
        data <<= 8;
        --bytes;
    }
}

/* put intel memory */

void gputi(void *dst, unsigned int data, int bytes)
{
    unsigned char *d = (unsigned char *) dst;
    unsigned int   pval;

    while (bytes)
    {
        pval = data;
        *d++  = (unsigned char) pval;
        data >>= 8;
        --bytes;
    }
}

