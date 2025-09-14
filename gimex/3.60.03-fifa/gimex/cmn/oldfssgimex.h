// (c) 1994-2007 Electronic Arts Inc.


#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

#define OLDSHP_about     OLDFSS_about
#define OLDSHP_is        OLDFSS_is
#define OLDSHP_open      OLDFSS_open
#define OLDSHP_info      OLDFSS_info
#define OLDSHP_read      OLDFSS_read
#define OLDSHP_close     OLDFSS_close
#define OLDSHP_wopen     OLDFSS_wopen
#define OLDSHP_write     OLDFSS_write
#define OLDSHP_wclose    OLDFSS_wclose

#define OLDSHPSIGNATURE     GMAKEID('S','S','F','F')
#define OLDFNTSIGNATURE     GMAKEID('F','N','T','F')
#define OLDSHPMULTIFRAME    1
#define OLDSHPEXTENSION0    ".fss"
#define OLDSHPEXTENSION1    ".oldfss"
#define OLDSHPEXTENSION2    ""
#define OLDSHPEXTENSION3    ""
#define OLDSHPMACTYPE0      GMAKEID('.','f','s','s')
#define OLDSHPMACTYPE1      0
#define OLDSHPMACTYPE2      0
#define OLDSHPMACTYPE3      0
#define OLDSHPSHORTTYPE     "OLDFSS"
#define OLDSHPWORDTYPE      "OLDFSS"
#define OLDSHPLONGTYPE      "Old PC Streaming Shape"
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

