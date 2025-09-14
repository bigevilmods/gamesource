// (c) 1994-2007 Electronic Arts Inc.


#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

#define FNT_about       FFN_about
#define FNT_is          FFN_is
#define FNT_open        FFN_open
#define FNT_info        FFN_info
#define FNT_read        FFN_read
#define FNT_close       FFN_close
#define FNT_wopen       FFN_wopen
#define FNT_write       FFN_write
#define FNT_wclose      FFN_wclose

#define SIGNATURE       GMAKEID('F','n','t','F')
#define OLDSIGNATURE    GMAKEID('F','N','T','F')
#define EXTENSION0      ".ffn"
#define EXTENSION1      ""
#define EXTENSION2      ""
#define EXTENSION3      ""
#define MACTYPE0        GMAKEID('.','f','f','n')
#define MACTYPE1        0
#define MACTYPE2        0
#define MACTYPE3        0
#define SHORTTYPE       "FFN"
#define WORDTYPE        "FFN"
#define LONGTYPE        "PC Font File"

#define FNTGETMEM                   ggeti
#define FNTPUTMEM                   gputi
#define FNTGETFIELD(field)          FNTGETMEM((void *)&(field), sizeof(field))
#define FNTPUTFIELD(field, data)    FNTPUTMEM((void *)&(field), data, sizeof(field))

/* shape codes for different bit depths */
#define SHAPECODE4      0x79
#define SHAPECODE8	    0x7b
#define SHAPECODEDXT	0x60
#define SHAPECODE16     0x6d
#define SHAPECODE32     0x7d

#include "fntgimex.h"

#ifdef __cplusplus
}
#endif

