// (c) 1994-2007 Electronic Arts Inc.


#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

#define OLDSHP_about     OLDPSH_about
#define OLDSHP_is        OLDPSH_is
#define OLDSHP_open      OLDPSH_open
#define OLDSHP_info      OLDPSH_info
#define OLDSHP_read      OLDPSH_read
#define OLDSHP_close     OLDPSH_close
#define OLDSHP_wopen     OLDPSH_wopen
#define OLDSHP_write     OLDPSH_write
#define OLDSHP_wclose    OLDPSH_wclose

#define OLDSHPSIGNATURE     GMAKEID('S','H','P','P')
#define OLDFNTSIGNATURE     GMAKEID('F','N','T','P')
#define OLDSHPMULTIFRAME    1
#define OLDSHPEXTENSION0    ".psh"
#define OLDSHPEXTENSION1    ".oldpsh"
#define OLDSHPEXTENSION2    ""
#define OLDSHPEXTENSION3    ""
#define OLDSHPMACTYPE0      GMAKEID('.','p','s','h')
#define OLDSHPMACTYPE1      0
#define OLDSHPMACTYPE2      0
#define OLDSHPMACTYPE3      0
#define OLDSHPSHORTTYPE     "OLDPSH"
#define OLDSHPWORDTYPE      "OLDPSH"
#define OLDSHPLONGTYPE      "Old PSX Shape File"
#define OLDSHPDATAALIGNMENT 15
#define OLDSHPMIPMAPDATAALIGNMENT 15
#define OLDSHPCLUTDATAALIGNMENT 15
#define OLDSHPPACKEDDATAALIGNMENT 15

#define OLDggetx ggeti
#define OLDgputx gputi
#define OLDSHPGETFIELD(field) ggeti((void *)&field, sizeof(field))
#define OLDSHPPUTFIELD(field,data) gputi((void *)&field,(unsigned int)(data),sizeof(field))

#include "oldshpgimex.h"

#ifdef __cplusplus
}
#endif

