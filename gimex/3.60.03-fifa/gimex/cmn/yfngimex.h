// (c) 1994-2007 Electronic Arts Inc.


#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

#define FNT_about       YFN_about
#define FNT_is          YFN_is
#define FNT_open        YFN_open
#define FNT_info        YFN_info
#define FNT_read        YFN_read
#define FNT_close       YFN_close
#define FNT_wopen       YFN_wopen
#define FNT_write       YFN_write
#define FNT_wclose      YFN_wclose

#define SIGNATURE       GMAKEID('F','n','t','Y')
#define OLDSIGNATURE    GMAKEID('F','N','T','Y')
#define EXTENSION0      ".yfn"
#define EXTENSION1      ""
#define EXTENSION2      ""
#define EXTENSION3      ""
#define MACTYPE0        GMAKEID('.','y','f','n')
#define MACTYPE1        0
#define MACTYPE2        0
#define MACTYPE3        0
#define SHORTTYPE       "yfn"
#define WORDTYPE        "YFN"
#define LONGTYPE        "Sony PS3 Font File"

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
