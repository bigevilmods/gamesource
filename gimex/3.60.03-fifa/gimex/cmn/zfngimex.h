// (c) 1994-2007 Electronic Arts Inc.


#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

#define FNT_about       ZFN_about
#define FNT_is          ZFN_is
#define FNT_open        ZFN_open
#define FNT_info        ZFN_info
#define FNT_read        ZFN_read
#define FNT_close       ZFN_close
#define FNT_wopen       ZFN_wopen
#define FNT_write       ZFN_write
#define FNT_wclose      ZFN_wclose

#define SIGNATURE       GMAKEID('F','n','t','Z')
#define OLDSIGNATURE    GMAKEID('F','N','T','Z')
#define EXTENSION0      ".zfn"
#define EXTENSION1      ""
#define EXTENSION2      ""
#define EXTENSION3      ""
#define MACTYPE0        GMAKEID('.','z','f','n')
#define MACTYPE1        0
#define MACTYPE2        0
#define MACTYPE3        0
#define SHORTTYPE       "zfn"
#define WORDTYPE        "ZFN"
#define LONGTYPE        "XBox360 Font File"

#define FNTGETMEM                   ggetm
#define FNTPUTMEM                   gputm
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
