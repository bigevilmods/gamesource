// (c) 1998-2007 Electronic Arts Inc.


#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

#define OLDSHP_about     OLDGSH_about
#define OLDSHP_is        OLDGSH_is
#define OLDSHP_open      OLDGSH_open
#define OLDSHP_info      OLDGSH_info
#define OLDSHP_read      OLDGSH_read
#define OLDSHP_close     OLDGSH_close
#define OLDSHP_wopen     OLDGSH_wopen
#define OLDSHP_write     OLDGSH_write
#define OLDSHP_wclose    OLDGSH_wclose

#define OLDSHPSIGNATURE     GMAKEID('S','H','P','G')
#define OLDFNTSIGNATURE     GMAKEID('F','N','T','G')
#define OLDSHPMULTIFRAME    1
#define OLDSHPEXTENSION0    ".gsh"
#define OLDSHPEXTENSION1    ".oldgsh"
#define OLDSHPEXTENSION2    ""
#define OLDSHPEXTENSION3    ""
#define OLDSHPMACTYPE0      GMAKEID('.','g','s','h')
#define OLDSHPMACTYPE1      0
#define OLDSHPMACTYPE2      0
#define OLDSHPMACTYPE3      0
#define OLDSHPSHORTTYPE     "OLDGSH"
#define OLDSHPWORDTYPE      "OLDGSH"
#define OLDSHPLONGTYPE      "Old GameCube Shape File"
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

