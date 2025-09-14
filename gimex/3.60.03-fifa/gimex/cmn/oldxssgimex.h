// (c) 1994-2007 Electronic Arts Inc.


#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

#define OLDSHP_about     OLDXSS_about
#define OLDSHP_is        OLDXSS_is
#define OLDSHP_open      OLDXSS_open
#define OLDSHP_info      OLDXSS_info
#define OLDSHP_read      OLDXSS_read
#define OLDSHP_close     OLDXSS_close
#define OLDSHP_wopen     OLDXSS_wopen
#define OLDSHP_write     OLDXSS_write
#define OLDSHP_wclose    OLDXSS_wclose

#define OLDSHPSIGNATURE     GMAKEID('S','S','F','X')
#define OLDFNTSIGNATURE     GMAKEID('F','N','T','X')
#define OLDSHPMULTIFRAME    1
#define OLDSHPEXTENSION0    ".xss"
#define OLDSHPEXTENSION1    ".oldxss"
#define OLDSHPEXTENSION2    ""
#define OLDSHPEXTENSION3    ""
#define OLDSHPMACTYPE0      GMAKEID('.','x','s','s')
#define OLDSHPMACTYPE1      0
#define OLDSHPMACTYPE2      0
#define OLDSHPMACTYPE3      0
#define OLDSHPSHORTTYPE     "OLDXSS"
#define OLDSHPWORDTYPE      "OLDXSS"
#define OLDSHPLONGTYPE      "Old XBox Streaming Shape"
#define OLDSHPDATAALIGNMENT 127
#define OLDSHPMIPMAPDATAALIGNMENT 0
#define OLDSHPCLUTDATAALIGNMENT 63
#define OLDSHPPACKEDDATAALIGNMENT 15

#define OLDggetx ggeti
#define OLDgputx gputi
#define OLDSHPGETFIELD(field) ggeti((void *)&field, sizeof(field))
#define OLDSHPPUTFIELD(field,data) gputi((void *)&field,(unsigned int)(data),sizeof(field))

#include "oldshpgimex.h"

#ifdef __cplusplus
}
#endif


