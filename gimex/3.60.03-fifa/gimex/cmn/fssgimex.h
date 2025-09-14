// (c) 1994-2007 Electronic Arts Inc.


#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SHP_about     FSS_about
#define SHP_is        FSS_is
#define SHP_open      FSS_open
#define SHP_info      FSS_info
#define SHP_read      FSS_read
#define SHP_close     FSS_close
#define SHP_wopen     FSS_wopen
#define SHP_write     FSS_write
#define SHP_wclose    FSS_wclose

#define SHPSIGNATURE     GMAKEID('S','s','f','F')
#define FNTSIGNATURE     GMAKEID('F','n','t','F')
#define SHPMULTIFRAME    1
#define SHPEXTENSION0    ".fss"
#define SHPEXTENSION1    ".newfss"
#define SHPEXTENSION2    ""
#define SHPEXTENSION3    ""
#define SHPMACTYPE0      GMAKEID('.','f','s','s')
#define SHPMACTYPE1      0
#define SHPMACTYPE2      0
#define SHPMACTYPE3      0
#define SHPSHORTTYPE     "FSS"
#define SHPWORDTYPE      "FSS"
#define SHPLONGTYPE      "PC Streaming Shape"
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

