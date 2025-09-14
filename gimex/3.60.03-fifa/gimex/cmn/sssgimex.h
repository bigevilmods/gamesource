// (c) 1998-2007 Electronic Arts Inc.


#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SHP_about     SSS_about
#define SHP_is        SSS_is
#define SHP_open      SSS_open
#define SHP_info      SSS_info
#define SHP_read      SSS_read
#define SHP_close     SSS_close
#define SHP_wopen     SSS_wopen
#define SHP_write     SSS_write
#define SHP_wclose    SSS_wclose

#define SHPSIGNATURE     GMAKEID('S','s','f','S')
#define FNTSIGNATURE     GMAKEID('F','n','t','S')
#define SHPMULTIFRAME    1
#define SHPEXTENSION0    ".sss"
#define SHPEXTENSION1    ".newsss"
#define SHPEXTENSION2    ""
#define SHPEXTENSION3    ""
#define SHPMACTYPE0      GMAKEID('.','s','s','s')
#define SHPMACTYPE1      0
#define SHPMACTYPE2      0
#define SHPMACTYPE3      0
#define SHPSHORTTYPE     "SSS"
#define SHPWORDTYPE      "SSS"
#define SHPLONGTYPE      "PS2 Streaming Shape"
#define SHPDATAALIGNMENT 63
#define SHPMIPMAPDATAALIGNMENT 15
#define SHPCLUTDATAALIGNMENT 15
#define SHPPACKEDDATAALIGNMENT 15

#define ggetx ggeti
#define gputx gputi

#include "shpgimex.h"

#ifdef __cplusplus
}
#endif

