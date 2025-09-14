// (c) 1998-2007 Electronic Arts Inc.


#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

#define OLDSHP_about     OLDYSS_about
#define OLDSHP_is        OLDYSS_is
#define OLDSHP_open      OLDYSS_open
#define OLDSHP_info      OLDYSS_info
#define OLDSHP_read      OLDYSS_read
#define OLDSHP_close     OLDYSS_close
#define OLDSHP_wopen     OLDYSS_wopen
#define OLDSHP_write     OLDYSS_write
#define OLDSHP_wclose    OLDYSS_wclose

#define OLDSHPSIGNATURE     GMAKEID('S','S','F','Y')
#define OLDFNTSIGNATURE     GMAKEID('F','N','T','Y')
#define OLDSHPMULTIFRAME    1
#define OLDSHPEXTENSION0    ".yss"
#define OLDSHPEXTENSION1    ".oldyss"
#define OLDSHPEXTENSION2    ""
#define OLDSHPEXTENSION3    ""
#define OLDSHPMACTYPE0      GMAKEID('.','y','s','s')
#define OLDSHPMACTYPE1      0
#define OLDSHPMACTYPE2      0
#define OLDSHPMACTYPE3      0
#define OLDSHPSHORTTYPE     "OLDYSS"
#define OLDSHPWORDTYPE      "OLDYSS"
#define OLDSHPLONGTYPE      "Old Sony PS3 Streaming Shape"
#define OLDSHPDATAALIGNMENT 15
#define OLDSHPMIPMAPDATAALIGNMENT 15
#define OLDSHPCLUTDATAALIGNMENT 15
#define OLDSHPPACKEDDATAALIGNMENT 15

#define OLDggetx ggetm
#define OLDgputx gputm
#define OLDSHPGETFIELD(field) ggetm((void *)&field, sizeof(field))
#define OLDSHPPUTFIELD(field,data) gputm((void *)&field,(unsigned int)(data),sizeof(field))

#include "oldshpgimex.h"

#ifdef __cplusplus
}
#endif

