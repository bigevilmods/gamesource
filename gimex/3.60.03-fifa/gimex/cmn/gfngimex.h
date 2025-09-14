// (c) 1994-2007 Electronic Arts Inc.


#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

#define FNT_about       GFN_about
#define FNT_is          GFN_is
#define FNT_open        GFN_open
#define FNT_info        GFN_info
#define FNT_read        GFN_read
#define FNT_close       GFN_close
#define FNT_wopen       GFN_wopen
#define FNT_write       GFN_write
#define FNT_wclose      GFN_wclose

#define SIGNATURE       GMAKEID('F','n','t','G')
#define OLDSIGNATURE    GMAKEID('F','N','T','G')
#define EXTENSION0      ".gfn"
#define EXTENSION1      ""
#define EXTENSION2      ""
#define EXTENSION3      ""
#define MACTYPE0        GMAKEID('.','g','f','n')
#define MACTYPE1        0
#define MACTYPE2        0
#define MACTYPE3        0
#define SHORTTYPE       "gfn"
#define WORDTYPE        "GFN"
#define LONGTYPE        "GameCube Font File"

#define FNTGETMEM                   ggetm
#define FNTPUTMEM                   gputm
#define FNTGETFIELD(field)          FNTGETMEM((void *)&(field), sizeof(field))
#define FNTPUTFIELD(field, data)    FNTPUTMEM((void *)&(field), data, sizeof(field))

/* shape codes for different bit depths */
#define SHAPECODE4      0x18    /* 4 bit colour indexed */
#define SHAPECODE8	    0x19    /* 8 bit colour indexed */
#define SHAPECODE16     0x15    // 5a3

#include "fntgimex.h"

#ifdef __cplusplus
}
#endif
