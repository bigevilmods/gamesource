// (c) 1995-2007 Electronic Arts Inc.


#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

#define FNT_about       TFN_about
#define FNT_is          TFN_is
#define FNT_open        TFN_open
#define FNT_info        TFN_info
#define FNT_read        TFN_read
#define FNT_close       TFN_close
#define FNT_wopen       TFN_wopen
#define FNT_write       TFN_write
#define FNT_wclose      TFN_wclose

#define SIGNATURE       GMAKEID('F','n','t','3')
#define OLDSIGNATURE    GMAKEID('F','N','T','M')
#define EXTENSION0      ".3fn"
#define EXTENSION1      ""
#define EXTENSION2      ""
#define EXTENSION3      ""
#define MACTYPE0        GMAKEID('.','3','f','n')
#define MACTYPE1        0
#define MACTYPE2        0
#define MACTYPE3        0
#define SHORTTYPE       "3FN"
#define WORDTYPE        "3FN"
#define LONGTYPE        "3DO Font File"

#define FNTGETMEM                   ggetm
#define FNTPUTMEM                   gputm
#define FNTGETFIELD(field)          FNTGETMEM((void *)&(field), sizeof(field))
#define FNTPUTFIELD(field, data)    FNTPUTMEM((void *)&(field), data, sizeof(field))

/* shape codes for different bit depths */
#define SHAPECODE4      0x03
#define SHAPECODE8	    0x66

#include "fntgimex.h"

#ifdef __cplusplus
}
#endif

