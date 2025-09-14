// (c) 1998-2007 Electronic Arts Inc.


#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

#define OLDSHP_about     OLDGSS_about
#define OLDSHP_is        OLDGSS_is
#define OLDSHP_open      OLDGSS_open
#define OLDSHP_info      OLDGSS_info
#define OLDSHP_read      OLDGSS_read
#define OLDSHP_close     OLDGSS_close
#define OLDSHP_wopen     OLDGSS_wopen
#define OLDSHP_write     OLDGSS_write
#define OLDSHP_wclose    OLDGSS_wclose

#define OLDSHPSIGNATURE     GMAKEID('S','S','F','G')
#define OLDFNTSIGNATURE     GMAKEID('F','N','T','G')
#define OLDSHPMULTIFRAME    1
#define OLDSHPEXTENSION0    ".gss"
#define OLDSHPEXTENSION1    ".oldgss"
#define OLDSHPEXTENSION2    ""
#define OLDSHPEXTENSION3    ""
#define OLDSHPMACTYPE0      GMAKEID('.','g','s','s')
#define OLDSHPMACTYPE1      0
#define OLDSHPMACTYPE2      0
#define OLDSHPMACTYPE3      0
#define OLDSHPSHORTTYPE     "OLDGSS"
#define OLDSHPWORDTYPE      "OLDGSS"
#define OLDSHPLONGTYPE      "Old GameCube Streaming Shape"
#define OLDSHPDATAALIGNMENT 31
#define OLDSHPMIPMAPDATAALIGNMENT 31
#define OLDSHPCLUTDATAALIGNMENT 31
#define OLDSHPPACKEDDATAALIGNMENT 31

#define OLDggetx ggetm
#define OLDgputx gputm
#define OLDSHPGETFIELD(field) ggetm((void *)&field, sizeof(field))
#define OLDSHPPUTFIELD(field,data) gputm((void *)&field,(unsigned int)(data),sizeof(field))

#include "oldshpgimex.h"

#ifdef __cplusplus
}
#endif

