// (c) 1998-2007 Electronic Arts Inc.


#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

#define OLDSHP_about     OLDMSS_about
#define OLDSHP_is        OLDMSS_is
#define OLDSHP_open      OLDMSS_open
#define OLDSHP_info      OLDMSS_info
#define OLDSHP_read      OLDMSS_read
#define OLDSHP_close     OLDMSS_close
#define OLDSHP_wopen     OLDMSS_wopen
#define OLDSHP_write     OLDMSS_write
#define OLDSHP_wclose    OLDMSS_wclose

#define OLDSHPSIGNATURE     GMAKEID('S','S','F','M')
#define OLDFNTSIGNATURE     GMAKEID('F','N','T','M')
#define OLDSHPMULTIFRAME    1
#define OLDSHPEXTENSION0    ".mss"
#define OLDSHPEXTENSION1    ".oldmss"
#define OLDSHPEXTENSION2    ""
#define OLDSHPEXTENSION3    ""
#define OLDSHPMACTYPE0      GMAKEID('.','m','s','s')
#define OLDSHPMACTYPE1      0
#define OLDSHPMACTYPE2      0
#define OLDSHPMACTYPE3      0
#define OLDSHPSHORTTYPE     "OLDMSS"
#define OLDSHPWORDTYPE      "OLDMSS"
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

