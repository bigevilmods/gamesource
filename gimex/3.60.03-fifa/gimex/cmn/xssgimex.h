// (c) 1994-2007 Electronic Arts Inc.


#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SHP_about     XSS_about
#define SHP_is        XSS_is
#define SHP_open      XSS_open
#define SHP_info      XSS_info
#define SHP_read      XSS_read
#define SHP_close     XSS_close
#define SHP_wopen     XSS_wopen
#define SHP_write     XSS_write
#define SHP_wclose    XSS_wclose

#define SHPSIGNATURE     GMAKEID('S','s','f','X')
#define FNTSIGNATURE     GMAKEID('F','n','t','X')
#define SHPMULTIFRAME    1
#define SHPEXTENSION0    ".xss"
#define SHPEXTENSION1    ".newxss"
#define SHPEXTENSION2    ""
#define SHPEXTENSION3    ""
#define SHPMACTYPE0      GMAKEID('.','x','s','s')
#define SHPMACTYPE1      0
#define SHPMACTYPE2      0
#define SHPMACTYPE3      0
#define SHPSHORTTYPE     "XSS"
#define SHPWORDTYPE      "XSS"
#define SHPLONGTYPE      "XBox Streaming Shape"
#define SHPDATAALIGNMENT 127
#define SHPMIPMAPDATAALIGNMENT 0
#define SHPCLUTDATAALIGNMENT 63
#define SHPPACKEDDATAALIGNMENT 15

#define ggetx ggeti
#define gputx gputi

#include "shpgimex.h"

#ifdef __cplusplus
}
#endif

