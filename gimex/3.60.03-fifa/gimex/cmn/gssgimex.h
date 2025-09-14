// (c) 1998-2007 Electronic Arts Inc.


#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SHP_about     GSS_about
#define SHP_is        GSS_is
#define SHP_open      GSS_open
#define SHP_info      GSS_info
#define SHP_read      GSS_read
#define SHP_close     GSS_close
#define SHP_wopen     GSS_wopen
#define SHP_write     GSS_write
#define SHP_wclose    GSS_wclose

#define SHPSIGNATURE     GMAKEID('S','s','f','G')
#define FNTSIGNATURE     GMAKEID('F','n','t','G')
#define SHPMULTIFRAME    1
#define SHPEXTENSION0    ".gss"
#define SHPEXTENSION1    ".newgss"
#define SHPEXTENSION2    ""
#define SHPEXTENSION3    ""
#define SHPMACTYPE0      GMAKEID('.','g','s','s')
#define SHPMACTYPE1      0
#define SHPMACTYPE2      0
#define SHPMACTYPE3      0
#define SHPSHORTTYPE     "GSS"
#define SHPWORDTYPE      "GSS"
#define SHPLONGTYPE      "GameCube Streaming Shape"
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

