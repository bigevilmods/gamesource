// (c) 1998-2007 Electronic Arts Inc.


#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

#define OLDSHP_about     OLDZSH_about
#define OLDSHP_is        OLDZSH_is
#define OLDSHP_open      OLDZSH_open
#define OLDSHP_info      OLDZSH_info
#define OLDSHP_read      OLDZSH_read
#define OLDSHP_close     OLDZSH_close
#define OLDSHP_wopen     OLDZSH_wopen
#define OLDSHP_write     OLDZSH_write
#define OLDSHP_wclose    OLDZSH_wclose

#define OLDSHPSIGNATURE     GMAKEID('S','H','P','M')
#define OLDFNTSIGNATURE     GMAKEID('F','N','T','M')
#define OLDSHPMULTIFRAME    1
#define OLDSHPEXTENSION0    ".zsh"
#define OLDSHPEXTENSION1    ".oldzsh"
#define OLDSHPEXTENSION2    ""
#define OLDSHPEXTENSION3    ""
#define OLDSHPMACTYPE0      GMAKEID('.','m','s','h')
#define OLDSHPMACTYPE1      0
#define OLDSHPMACTYPE2      0
#define OLDSHPMACTYPE3      0
#define OLDSHPSHORTTYPE     "OLDZSH"
#define OLDSHPWORDTYPE      "OLDZSH"
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

