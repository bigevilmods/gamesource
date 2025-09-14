// (c) 1994-2007 Electronic Arts Inc.


#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SHP_about     YSS_about
#define SHP_is        YSS_is
#define SHP_open      YSS_open
#define SHP_info      YSS_info
#define SHP_read      YSS_read
#define SHP_close     YSS_close
#define SHP_wopen     YSS_wopen
#define SHP_write     YSS_write
#define SHP_wclose    YSS_wclose

#define SHPSIGNATURE     GMAKEID('S','s','f','Y')
#define FNTSIGNATURE     GMAKEID('F','n','t','Y')
#define SHPMULTIFRAME    1
#define SHPEXTENSION0    ".yss"
#define SHPEXTENSION1    ".newyss"
#define SHPEXTENSION2    ""
#define SHPEXTENSION3    ""
#define SHPMACTYPE0      GMAKEID('.','y','s','s')
#define SHPMACTYPE1      0
#define SHPMACTYPE2      0
#define SHPMACTYPE3      0
#define SHPSHORTTYPE     "YSS"
#define SHPWORDTYPE      "YSS"
#define SHPLONGTYPE      "PS3 Streaming Shape"
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

