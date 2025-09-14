// (c) 1994-2007 Electronic Arts Inc.


#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SHP_about     PSH_about
#define SHP_is        PSH_is
#define SHP_open      PSH_open
#define SHP_info      PSH_info
#define SHP_read      PSH_read
#define SHP_close     PSH_close
#define SHP_wopen     PSH_wopen
#define SHP_write     PSH_write
#define SHP_wclose    PSH_wclose

#define SHPSIGNATURE     GMAKEID('S','h','p','P')
#define FNTSIGNATURE     GMAKEID('F','n','t','P')
#define SHPMULTIFRAME    1
#define SHPEXTENSION0    ".psh"
#define SHPEXTENSION1    ".newpsh"
#define SHPEXTENSION2    ""
#define SHPEXTENSION3    ""
#define SHPMACTYPE0      GMAKEID('.','p','s','h')
#define SHPMACTYPE1      0
#define SHPMACTYPE2      0
#define SHPMACTYPE3      0
#define SHPSHORTTYPE     "PSH"
#define SHPWORDTYPE      "PSH"
#define SHPLONGTYPE      "PSX Shape File"
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

