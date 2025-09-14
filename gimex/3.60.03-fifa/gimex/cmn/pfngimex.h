// (c) 1995-2007 Electronic Arts Inc.


#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

#define FNT_about       PFN_about
#define FNT_is          PFN_is
#define FNT_open        PFN_open
#define FNT_info        PFN_info
#define FNT_read        PFN_read
#define FNT_close       PFN_close
#define FNT_wopen       PFN_wopen
#define FNT_write       PFN_write
#define FNT_wclose      PFN_wclose

#define SIGNATURE       GMAKEID('F','n','t','P')
#define OLDSIGNATURE    GMAKEID('F','N','T','P')
#define EXTENSION0      ".pfn"
#define EXTENSION1      ""
#define EXTENSION2      ""
#define EXTENSION3      ""
#define MACTYPE0        GMAKEID('.','p','f','n')
#define MACTYPE1        0
#define MACTYPE2        0
#define MACTYPE3        0
#define SHORTTYPE       "PFN"
#define WORDTYPE        "PFN"
#define LONGTYPE        "PSX Font File"

#define FNTGETMEM                   ggeti
#define FNTPUTMEM                   gputi
#define FNTGETFIELD(field)          FNTGETMEM((void *)&(field), sizeof(field))
#define FNTPUTFIELD(field, data)    FNTPUTMEM((void *)&(field), data, sizeof(field))

/* shape codes for different bit depths */
#define SHAPECODE4      0x40
#define SHAPECODE8	    0x41

#include "fntgimex.h"

#ifdef __cplusplus
}
#endif

