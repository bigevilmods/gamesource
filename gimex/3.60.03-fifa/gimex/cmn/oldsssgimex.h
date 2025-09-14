// (c) 1998-2007 Electronic Arts Inc.


#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

#define OLDSHP_about     OLDSSS_about
#define OLDSHP_is        OLDSSS_is
#define OLDSHP_open      OLDSSS_open
#define OLDSHP_info      OLDSSS_info
#define OLDSHP_read      OLDSSS_read
#define OLDSHP_close     OLDSSS_close
#define OLDSHP_wopen     OLDSSS_wopen
#define OLDSHP_write     OLDSSS_write
#define OLDSHP_wclose    OLDSSS_wclose

#define OLDSHPSIGNATURE     GMAKEID('S','S','F','S')
#define OLDFNTSIGNATURE     GMAKEID('F','N','T','S')
#define OLDSHPMULTIFRAME    1
#define OLDSHPEXTENSION0    ".sss"
#define OLDSHPEXTENSION1    ".oldsss"
#define OLDSHPEXTENSION2    ""
#define OLDSHPEXTENSION3    ""
#define OLDSHPMACTYPE0      GMAKEID('.','s','s','s')
#define OLDSHPMACTYPE1      0
#define OLDSHPMACTYPE2      0
#define OLDSHPMACTYPE3      0
#define OLDSHPSHORTTYPE     "OLDSSS"
#define OLDSHPWORDTYPE      "OLDSSS"
#define OLDSHPLONGTYPE      "Old PS2 Streaming Shape"
#define OLDSHPDATAALIGNMENT 63
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

