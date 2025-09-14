// (c) 1998-2007 Electronic Arts Inc.


#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SHP_about     GSH_about
#define SHP_is        GSH_is
#define SHP_open      GSH_open
#define SHP_info      GSH_info
#define SHP_read      GSH_read
#define SHP_close     GSH_close
#define SHP_wopen     GSH_wopen
#define SHP_write     GSH_write
#define SHP_wclose    GSH_wclose

#define SHPSIGNATURE     GMAKEID('S','h','p','G')
#define FNTSIGNATURE     GMAKEID('F','n','t','G')
#define SHPMULTIFRAME    1
#define SHPEXTENSION0    ".gsh"
#define SHPEXTENSION1    ".newgsh"
#define SHPEXTENSION2    ""
#define SHPEXTENSION3    ""
#define SHPMACTYPE0      GMAKEID('.','g','s','h')
#define SHPMACTYPE1      0
#define SHPMACTYPE2      0
#define SHPMACTYPE3      0
#define SHPSHORTTYPE     "GSH"
#define SHPWORDTYPE      "GSH"
#define SHPLONGTYPE      "GameCube Shape File"
#define SHPDATAALIGNMENT 31
#define SHPMIPMAPDATAALIGNMENT 31
#define SHPCLUTDATAALIGNMENT 31
#define SHPPACKEDDATAALIGNMENT 31

#define ggetx ggetm
#define gputx gputm

#include "shpgimex.h"

#ifdef __cplusplus
}
#endif

