// (c) 1998-2007 Electronic Arts Inc.


#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

#define OLDSHP_about     OLDSSH_about
#define OLDSHP_is        OLDSSH_is
#define OLDSHP_open      OLDSSH_open
#define OLDSHP_info      OLDSSH_info
#define OLDSHP_read      OLDSSH_read
#define OLDSHP_close     OLDSSH_close
#define OLDSHP_wopen     OLDSSH_wopen
#define OLDSHP_write     OLDSSH_write
#define OLDSHP_wclose    OLDSSH_wclose

#define OLDSHPSIGNATURE     GMAKEID('S','H','P','S')
#define OLDFNTSIGNATURE     GMAKEID('F','N','T','S')
#define OLDSHPMULTIFRAME    1
#define OLDSHPEXTENSION0    ".ssh"
#define OLDSHPEXTENSION1    ".oldssh"
#define OLDSHPEXTENSION2    ""
#define OLDSHPEXTENSION3    ""
#define OLDSHPMACTYPE0      GMAKEID('.','s','s','h')
#define OLDSHPMACTYPE1      0
#define OLDSHPMACTYPE2      0
#define OLDSHPMACTYPE3      0
#define OLDSHPSHORTTYPE     "OLDSSH"
#define OLDSHPWORDTYPE      "OLDSSH"
#define OLDSHPLONGTYPE      "Old PS2 Shape File"
#define OLDSHPDATAALIGNMENT 63
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

