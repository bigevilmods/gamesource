// (c) 1998-2007 Electronic Arts Inc.


#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SHP_about     ASH_about
#define SHP_is        ASH_is
#define SHP_open      ASH_open
#define SHP_info      ASH_info
#define SHP_read      ASH_read
#define SHP_close     ASH_close
#define SHP_wopen     ASH_wopen
#define SHP_write     ASH_write
#define SHP_wclose    ASH_wclose

#define SHPSIGNATURE     GMAKEID('S','h','p','A')
#define FNTSIGNATURE     0
#define SHPMULTIFRAME    1
#define SHPEXTENSION0    ".ash"
#define SHPEXTENSION1    ".newash"
#define SHPEXTENSION2    ""
#define SHPEXTENSION3    ""
#define SHPMACTYPE0      GMAKEID('.','a','s','h')
#define SHPMACTYPE1      0
#define SHPMACTYPE2      0
#define SHPMACTYPE3      0
#define SHPSHORTTYPE     "ASH"
#define SHPWORDTYPE      "ASH"
#define SHPLONGTYPE      "Gameboy Advance Shape File"
#define SHPDATAALIGNMENT 15
#define SHPMIPMAPDATAALIGNMENT 15
#define SHPCLUTDATAALIGNMENT 15
#define SHPPACKEDDATAALIGNMENT 15

#define ggetx ggeti
#define gputx gputi

#include "shpgimex.h"

#ifdef __cplusplus
}
#endif

