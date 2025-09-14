// (c) 1998-2007 Electronic Arts Inc.


#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SHP_about     MSH_about
#define SHP_is        MSH_is
#define SHP_open      MSH_open
#define SHP_info      MSH_info
#define SHP_read      MSH_read
#define SHP_close     MSH_close
#define SHP_wopen     MSH_wopen
#define SHP_write     MSH_write
#define SHP_wclose    MSH_wclose

#define SHPSIGNATURE     GMAKEID('S','h','p','M')
#define FNTSIGNATURE     GMAKEID('F','n','t','M')
#define SHPMULTIFRAME    1
#define SHPEXTENSION0    ".msh"
#define SHPEXTENSION1    ".newmsh"
#define SHPEXTENSION2    ""
#define SHPEXTENSION3    ""
#define SHPMACTYPE0      GMAKEID('.','m','s','h')
#define SHPMACTYPE1      0
#define SHPMACTYPE2      0
#define SHPMACTYPE3      0
#define SHPSHORTTYPE     "MSH"
#define SHPWORDTYPE      "MSH"
#define SHPLONGTYPE      "Sony PSP Shape File"
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

