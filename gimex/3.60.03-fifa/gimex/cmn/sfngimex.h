// (c) 1995-2007 Electronic Arts Inc.


#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

#define FNT_about       SFN_about
#define FNT_is          SFN_is
#define FNT_open        SFN_open
#define FNT_info        SFN_info
#define FNT_read        SFN_read
#define FNT_close       SFN_close
#define FNT_wopen       SFN_wopen
#define FNT_write       SFN_write
#define FNT_wclose      SFN_wclose

#define SIGNATURE       GMAKEID('F','n','t','S')
#define OLDSIGNATURE    GMAKEID('F','N','T','S')
#define EXTENSION0      ".sfn"
#define EXTENSION1      ""
#define EXTENSION2      ""
#define EXTENSION3      ""
#define MACTYPE0        GMAKEID('.','s','f','n')
#define MACTYPE1        0
#define MACTYPE2        0
#define MACTYPE3        0
#define SHORTTYPE       "SFN"
#define WORDTYPE        "SFN"
#define LONGTYPE        "Sony2 Font File"

#define FNTGETMEM                   ggeti
#define FNTPUTMEM                   gputi
#define FNTGETFIELD(field)          FNTGETMEM((void *)&(field), sizeof(field))
#define FNTPUTFIELD(field, data)    FNTPUTMEM((void *)&(field), data, sizeof(field))

/* shape codes for different bit depths */
#define SHAPECODE4      0x01
#define SHAPECODE8	    0x02
#define SHAPECODE32     0x05

#include "fntgimex.h"

#ifdef __cplusplus
}
#endif
