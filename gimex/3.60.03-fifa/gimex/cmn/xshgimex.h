// (c) 1994-2007 Electronic Arts Inc.


#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SHP_about     XSH_about
#define SHP_is        XSH_is
#define SHP_open      XSH_open
#define SHP_info      XSH_info
#define SHP_read      XSH_read
#define SHP_close     XSH_close
#define SHP_wopen     XSH_wopen
#define SHP_write     XSH_write
#define SHP_wclose    XSH_wclose

#define SHPSIGNATURE     GMAKEID('S','h','p','X')
#define FNTSIGNATURE     GMAKEID('F','n','t','X')
#define SHPMULTIFRAME    1
#define SHPEXTENSION0    ".xsh"
#define SHPEXTENSION1    ".newxsh"
#define SHPEXTENSION2    ""
#define SHPEXTENSION3    ""
#define SHPMACTYPE0      GMAKEID('.','x','s','h')
#define SHPMACTYPE1      0
#define SHPMACTYPE2      0
#define SHPMACTYPE3      0
#define SHPSHORTTYPE     "XSH"
#define SHPWORDTYPE      "XSH"
#define SHPLONGTYPE      "XBox Shape File"
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

