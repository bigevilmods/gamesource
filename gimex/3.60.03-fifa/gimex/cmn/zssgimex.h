// (c) 1994-2007 Electronic Arts Inc.


#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SHP_about     ZSS_about
#define SHP_is        ZSS_is
#define SHP_open      ZSS_open
#define SHP_info      ZSS_info
#define SHP_read      ZSS_read
#define SHP_close     ZSS_close
#define SHP_wopen     ZSS_wopen
#define SHP_write     ZSS_write
#define SHP_wclose    ZSS_wclose

#define SHPSIGNATURE     GMAKEID('S','s','f','Z')
#define FNTSIGNATURE     GMAKEID('F','n','t','Z')
#define SHPMULTIFRAME    1
#define SHPEXTENSION0    ".zss"
#define SHPEXTENSION1    ".newzss"
#define SHPEXTENSION2    ""
#define SHPEXTENSION3    ""
#define SHPMACTYPE0      GMAKEID('.','z','s','s')
#define SHPMACTYPE1      0
#define SHPMACTYPE2      0
#define SHPMACTYPE3      0
#define SHPSHORTTYPE     "ZSS"
#define SHPWORDTYPE      "ZSS"
#define SHPLONGTYPE      "Xenon Streaming Shape"
#define SHPDATAALIGNMENT 127
#define SHPMIPMAPDATAALIGNMENT 0
#define SHPCLUTDATAALIGNMENT 63
#define SHPPACKEDDATAALIGNMENT 15

#define ggetx ggetm
#define gputx gputm

#include "shpgimex.h"

#ifdef __cplusplus
}
#endif

