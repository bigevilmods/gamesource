// (c) 1998-2007 Electronic Arts Inc.


#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

#define OLDSHP_about     OLDYSH_about
#define OLDSHP_is        OLDYSH_is
#define OLDSHP_open      OLDYSH_open
#define OLDSHP_info      OLDYSH_info
#define OLDSHP_read      OLDYSH_read
#define OLDSHP_close     OLDYSH_close
#define OLDSHP_wopen     OLDYSH_wopen
#define OLDSHP_write     OLDYSH_write
#define OLDSHP_wclose    OLDYSH_wclose

#define OLDSHPSIGNATURE     GMAKEID('S','H','P','Y')
#define OLDFNTSIGNATURE     GMAKEID('F','N','T','Y')
#define OLDSHPMULTIFRAME    1
#define OLDSHPEXTENSION0    ".ysh"
#define OLDSHPEXTENSION1    ".oldysh"
#define OLDSHPEXTENSION2    ""
#define OLDSHPEXTENSION3    ""
#define OLDSHPMACTYPE0      GMAKEID('.','y','s','h')
#define OLDSHPMACTYPE1      0
#define OLDSHPMACTYPE2      0
#define OLDSHPMACTYPE3      0
#define OLDSHPSHORTTYPE     "OLDYSH"
#define OLDSHPWORDTYPE      "OLDYSH"
#define OLDSHPLONGTYPE      "Old Sony PS3 Shape File"
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

