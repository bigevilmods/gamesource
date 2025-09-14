// (c) 1994-2007 Electronic Arts Inc.


#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SHP_about     YSH_about
#define SHP_is        YSH_is
#define SHP_open      YSH_open
#define SHP_info      YSH_info
#define SHP_read      YSH_read
#define SHP_close     YSH_close
#define SHP_wopen     YSH_wopen
#define SHP_write     YSH_write
#define SHP_wclose    YSH_wclose

#define SHPSIGNATURE     GMAKEID('S','h','p','Y')
#define FNTSIGNATURE     GMAKEID('F','n','t','Y')
#define SHPMULTIFRAME    1
#define SHPEXTENSION0    ".ysh"
#define SHPEXTENSION1    ".newysh"
#define SHPEXTENSION2    ""
#define SHPEXTENSION3    ""
#define SHPMACTYPE0      GMAKEID('.','y','s','h')
#define SHPMACTYPE1      0
#define SHPMACTYPE2      0
#define SHPMACTYPE3      0
#define SHPSHORTTYPE     "YSH"
#define SHPWORDTYPE      "YSH"
#define SHPLONGTYPE      "PS3 Shape File"
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

