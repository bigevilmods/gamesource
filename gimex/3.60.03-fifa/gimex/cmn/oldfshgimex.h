// (c) 1994-2007 Electronic Arts Inc.


#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

#define OLDSHP_about     OLDFSH_about
#define OLDSHP_is        OLDFSH_is
#define OLDSHP_open      OLDFSH_open
#define OLDSHP_info      OLDFSH_info
#define OLDSHP_read      OLDFSH_read
#define OLDSHP_close     OLDFSH_close
#define OLDSHP_wopen     OLDFSH_wopen
#define OLDSHP_write     OLDFSH_write
#define OLDSHP_wclose    OLDFSH_wclose

#define OLDSHPSIGNATURE     GMAKEID('S','H','P','I')
#define OLDFNTSIGNATURE     GMAKEID('F','N','T','F')
#define OLDSHPMULTIFRAME    1
#define OLDSHPEXTENSION0    ".fsh"
#define OLDSHPEXTENSION1    ".oldfsh"
#define OLDSHPEXTENSION2    ""
#define OLDSHPEXTENSION3    ""
#define OLDSHPMACTYPE0      GMAKEID('.','f','s','h')
#define OLDSHPMACTYPE1      0
#define OLDSHPMACTYPE2      0
#define OLDSHPMACTYPE3      0
#define OLDSHPSHORTTYPE     "OLDFSH"
#define OLDSHPWORDTYPE      "OLDFSH"
#define OLDSHPLONGTYPE      "Old PC Shape File"
#define OLDSHPDATAALIGNMENT 15
#define OLDSHPMIPMAPDATAALIGNMENT 0
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

