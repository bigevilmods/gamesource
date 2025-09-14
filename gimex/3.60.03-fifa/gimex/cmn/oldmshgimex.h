// (c) 1998-2007 Electronic Arts Inc.


#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

#define OLDSHP_about     OLDMSH_about
#define OLDSHP_is        OLDMSH_is
#define OLDSHP_open      OLDMSH_open
#define OLDSHP_info      OLDMSH_info
#define OLDSHP_read      OLDMSH_read
#define OLDSHP_close     OLDMSH_close
#define OLDSHP_wopen     OLDMSH_wopen
#define OLDSHP_write     OLDMSH_write
#define OLDSHP_wclose    OLDMSH_wclose

#define OLDSHPSIGNATURE     GMAKEID('S','H','P','M')
#define OLDFNTSIGNATURE     GMAKEID('F','N','T','M')
#define OLDSHPMULTIFRAME    1
#define OLDSHPEXTENSION0    ".msh"
#define OLDSHPEXTENSION1    ".oldmsh"
#define OLDSHPEXTENSION2    ""
#define OLDSHPEXTENSION3    ""
#define OLDSHPMACTYPE0      GMAKEID('.','m','s','h')
#define OLDSHPMACTYPE1      0
#define OLDSHPMACTYPE2      0
#define OLDSHPMACTYPE3      0
#define OLDSHPSHORTTYPE     "OLDMSH"
#define OLDSHPWORDTYPE      "OLDMSH"
#define OLDSHPLONGTYPE      "Old Sony PSP Shape File"
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

