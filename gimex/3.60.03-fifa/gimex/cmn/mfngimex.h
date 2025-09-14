// (c) 1994-2007 Electronic Arts Inc.


#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

#define FNT_about       MFN_about
#define FNT_is          MFN_is
#define FNT_open        MFN_open
#define FNT_info        MFN_info
#define FNT_read        MFN_read
#define FNT_close       MFN_close
#define FNT_wopen       MFN_wopen
#define FNT_write       MFN_write
#define FNT_wclose      MFN_wclose

#define SIGNATURE       GMAKEID('F','n','t','M')
#define OLDSIGNATURE    GMAKEID('F','N','T','M')
#define EXTENSION0      ".mfn"
#define EXTENSION1      ""
#define EXTENSION2      ""
#define EXTENSION3      ""
#define MACTYPE0        GMAKEID('.','m','f','n')
#define MACTYPE1        0
#define MACTYPE2        0
#define MACTYPE3        0
#define SHORTTYPE       "mfn"
#define WORDTYPE        "MFN"
#define LONGTYPE        "Sony PSP Font File"

#define FNTGETMEM                   ggeti
#define FNTPUTMEM                   gputi
#define FNTGETFIELD(field)          FNTGETMEM((void *)&(field), sizeof(field))
#define FNTPUTFIELD(field, data)    FNTPUTMEM((void *)&(field), data, sizeof(field))

/* shape codes for different bit depths */
#define SHAPECODE4      0x5c    /* 4 bit colour indexed */
#define SHAPECODE8	    0x5d
#define SHAPECODE16     0x5a
#define SHAPECODE32     0x5b

#include "fntgimex.h"

#ifdef __cplusplus
}
#endif
