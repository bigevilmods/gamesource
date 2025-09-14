// (c) 1998-2007 Electronic Arts Inc.


#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

#define OLDSHP_about     OLDZSS_about
#define OLDSHP_is        OLDZSS_is
#define OLDSHP_open      OLDZSS_open
#define OLDSHP_info      OLDZSS_info
#define OLDSHP_read      OLDZSS_read
#define OLDSHP_close     OLDZSS_close
#define OLDSHP_wopen     OLDZSS_wopen
#define OLDSHP_write     OLDZSS_write
#define OLDSHP_wclose    OLDZSS_wclose

#define OLDSHPSIGNATURE     GMAKEID('S','S','F','M')
#define OLDFNTSIGNATURE     GMAKEID('F','N','T','M')
#define OLDSHPMULTIFRAME    1
#define OLDSHPEXTENSION0    ".zss"
#define OLDSHPEXTENSION1    ".oldzss"
#define OLDSHPEXTENSION2    ""
#define OLDSHPEXTENSION3    ""
#define OLDSHPMACTYPE0      GMAKEID('.','m','s','s')
#define OLDSHPMACTYPE1      0
#define OLDSHPMACTYPE2      0
#define OLDSHPMACTYPE3      0
#define OLDSHPSHORTTYPE     "OLDZSS"
#define OLDSHPWORDTYPE      "OLDZSS"
#define OLDSHPLONGTYPE      "Old Sony PSP Streaming Shape"
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

