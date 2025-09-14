// (c) Electronic Arts. All Rights Reserved.

/*------------------------------------------------------------------*/
/*                                                                  */
/*                    HUFF XE Decompresser v6.10                    */
/*                                                                  */
/*                     by FrANK G. Barchard, EAC                    */
/*                                                                  */
/*                  C Unpack Header - Jan 17, 1997                  */
/*                                                                  */
/*------------------------------------------------------------------*/

/****************************************************************/
/*  Includes                                                    */
/****************************************************************/

#define WANTBZIP2 0

#include    "spit/codex.h"
#include    "spit/refcodex.h"
#include    "spit/btreecodex.h"
#if WANTBZIP2
#include    "bzip2codex.h"
#endif
#include    "spit/zipcodex.h"
#include    "spit/huffcodex.h"

#include    "huffu.h"

/****************************************************************/
/*  Block Move                                                  */
/****************************************************************/

void SQBlockMove(const unsigned char *s, unsigned char *d, size_t len)
{
    while (len--)
        *d++ = *s++;
}

/****************************************************************/
/*  Generic Unpacker                                            */
/****************************************************************/

size_t SQunhuff(const unsigned char *source, unsigned char *dest, unsigned char *workbuf, int doit)
{
    unsigned int            type;
    size_t                    ulen;

    type = *source;
    type = (type<<8) + *((unsigned char *) (source+1));

    switch(type)
    {
        case 0x10fb:
        case 0x11fb:
        case 0x90fb:
        case 0x91fb:
            if (doit) ulen = REF_decode(dest, source);
            else      ulen = REF_size(source);
            break;

        case 0x12fb:
        case 0x13fb:
        case 0x92fb:
        case 0x93fb:
            if (doit) ulen = ZIP_decode(dest, source);
            else      ulen = ZIP_size(source);
            break;

#if WANTBZIP2
        case 0x1efb:
        case 0x1ffb:
        case 0x9efb:
        case 0x9ffb:
        case 0x425a: // 'BZ'
            if (doit) ulen = BZIP2_decode(dest, source);
            else      ulen = BZIP2_size(source);
            break;
#endif

        case 0x30fb:
        case 0x31fb:
        case 0x32fb:
        case 0x33fb:
        case 0x34fb:
        case 0x35fb:
        case 0xb0fb:
        case 0xb1fb:
        case 0xb2fb:
        case 0xb3fb:
        case 0xb4fb:
        case 0xb5fb:
            if (doit) ulen = HUFF_decode(dest, source);
            else      ulen = HUFF_size(source);
            break;

        case 0x46fb:
        case 0x47fb:
        case 0xa6fb:
        case 0xa7fb:
            if (doit) ulen = BTREE_decode(dest, source);
            else      ulen = BTREE_size(source);
            break;

        case 0x6bfb:
        case 0x6efb:
            ulen = *(unsigned char *) (source+2);
            ulen = (int) ((unsigned int) (ulen<<8) + (unsigned int) (*(unsigned char *) (source+3)));
            ulen = (int) ((unsigned int) (ulen<<8) + (unsigned int) (*(unsigned char *) (source+4)));
            if (doit)
                SQBlockMove(source+6, dest, ulen);
            break;

        case 0x6ffb:
            ulen = *(unsigned char *) (source+5);
            ulen = (int) ((unsigned int) (ulen<<8) + (unsigned int) (*(unsigned char *) (source+6)));
            ulen = (int) ((unsigned int) (ulen<<8) + (unsigned int) (*(unsigned char *) (source+7)));
            if (doit)
                SQBlockMove(source+8, dest, ulen);
            break;

        default:
            ulen = 0;
    }
    return(ulen);
}

