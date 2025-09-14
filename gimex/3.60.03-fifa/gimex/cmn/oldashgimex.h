// (c) 1998-2007 Electronic Arts Inc.


#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

#define OLDSHP_about     OLDASH_about
#define OLDSHP_is        OLDASH_is
#define OLDSHP_open      OLDASH_open
#define OLDSHP_info      OLDASH_info
#define OLDSHP_read      OLDASH_read
#define OLDSHP_close     OLDASH_close
#define OLDSHP_wopen     OLDASH_wopen
#define OLDSHP_write     OLDASH_write
#define OLDSHP_wclose    OLDASH_wclose

#define OLDSHPSIGNATURE     GMAKEID('S','H','P','A')
#define OLDFNTSIGNATURE     0
#define OLDSHPMULTIFRAME    1
#define OLDSHPEXTENSION0    ".ash"
#define OLDSHPEXTENSION1    ".oldash"
#define OLDSHPEXTENSION2    ""
#define OLDSHPEXTENSION3    ""
#define OLDSHPMACTYPE0      GMAKEID('.','a','s','h')
#define OLDSHPMACTYPE1      0
#define OLDSHPMACTYPE2      0
#define OLDSHPMACTYPE3      0
#define OLDSHPSHORTTYPE     "OLDASH"
#define OLDSHPWORDTYPE      "OLDASH"
#define OLDSHPLONGTYPE      "Old GBA Shape File"
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

