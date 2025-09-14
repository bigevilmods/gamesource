// (c) 1998-2007 Electronic Arts Inc.


#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SHP_about     MSS_about
#define SHP_is        MSS_is
#define SHP_open      MSS_open
#define SHP_info      MSS_info
#define SHP_read      MSS_read
#define SHP_close     MSS_close
#define SHP_wopen     MSS_wopen
#define SHP_write     MSS_write
#define SHP_wclose    MSS_wclose

#define SHPSIGNATURE     GMAKEID('S','s','f','M')
#define FNTSIGNATURE     GMAKEID('F','n','t','M')
#define SHPMULTIFRAME    1
#define SHPEXTENSION0    ".mss"
#define SHPEXTENSION1    ".newmss"
#define SHPEXTENSION2    ""
#define SHPEXTENSION3    ""
#define SHPMACTYPE0      GMAKEID('.','m','s','s')
#define SHPMACTYPE1      0
#define SHPMACTYPE2      0
#define SHPMACTYPE3      0
#define SHPSHORTTYPE     "MSS"
#define SHPWORDTYPE      "MSS"
#define SHPLONGTYPE      "Sony PSP Streaming Shape"
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

