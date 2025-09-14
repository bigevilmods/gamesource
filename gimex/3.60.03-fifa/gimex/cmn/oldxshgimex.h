// (c) 1994-2007 Electronic Arts Inc.


#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

#define OLDSHP_about     OLDXSH_about
#define OLDSHP_is        OLDXSH_is
#define OLDSHP_open      OLDXSH_open
#define OLDSHP_info      OLDXSH_info
#define OLDSHP_read      OLDXSH_read
#define OLDSHP_close     OLDXSH_close
#define OLDSHP_wopen     OLDXSH_wopen
#define OLDSHP_write     OLDXSH_write
#define OLDSHP_wclose    OLDXSH_wclose

#define OLDSHPSIGNATURE     GMAKEID('S','H','P','X')
#define OLDFNTSIGNATURE     GMAKEID('F','N','T','X')
#define OLDSHPMULTIFRAME    1
#define OLDSHPEXTENSION0    ".xsh"
#define OLDSHPEXTENSION1    ".oldxsh"
#define OLDSHPEXTENSION2    ""
#define OLDSHPEXTENSION3    ""
#define OLDSHPMACTYPE0      GMAKEID('.','x','s','h')
#define OLDSHPMACTYPE1      0
#define OLDSHPMACTYPE2      0
#define OLDSHPMACTYPE3      0
#define OLDSHPSHORTTYPE     "OLDXSH"
#define OLDSHPWORDTYPE      "OLDXSH"
#define OLDSHPLONGTYPE      "Old XBox Shape File"
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


