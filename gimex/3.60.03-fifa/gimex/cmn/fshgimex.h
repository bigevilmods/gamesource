// (c) 1994-2007 Electronic Arts Inc.


#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SHP_about     FSH_about
#define SHP_is        FSH_is
#define SHP_open      FSH_open
#define SHP_info      FSH_info
#define SHP_read      FSH_read
#define SHP_close     FSH_close
#define SHP_wopen     FSH_wopen
#define SHP_write     FSH_write
#define SHP_wclose    FSH_wclose

#define SHPSIGNATURE     GMAKEID('S','h','p','F')
#define FNTSIGNATURE     GMAKEID('F','n','t','F')
#define SHPMULTIFRAME    1
#define SHPEXTENSION0    ".fsh"
#define SHPEXTENSION1    ".newfsh"
#define SHPEXTENSION2    ""
#define SHPEXTENSION3    ""
#define SHPMACTYPE0      GMAKEID('.','f','s','h')
#define SHPMACTYPE1      0
#define SHPMACTYPE2      0
#define SHPMACTYPE3      0
#define SHPSHORTTYPE     "FSH"
#define SHPWORDTYPE      "FSH"
#define SHPLONGTYPE      "PC Shape File"
#define SHPDATAALIGNMENT 15
#define SHPMIPMAPDATAALIGNMENT 0
#define SHPCLUTDATAALIGNMENT 15
#define SHPPACKEDDATAALIGNMENT 15

#define ggetx ggeti
#define gputx gputi

#include "shpgimex.h"

#ifdef __cplusplus
}
#endif

