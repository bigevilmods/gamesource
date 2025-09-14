// (c) 1994-2007 Electronic Arts Inc.


#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SHP_about     ZSH_about
#define SHP_is        ZSH_is
#define SHP_open      ZSH_open
#define SHP_info      ZSH_info
#define SHP_read      ZSH_read
#define SHP_close     ZSH_close
#define SHP_wopen     ZSH_wopen
#define SHP_write     ZSH_write
#define SHP_wclose    ZSH_wclose

#define SHPSIGNATURE     GMAKEID('S','h','p','Z')
#define FNTSIGNATURE     GMAKEID('F','n','t','Z')
#define SHPMULTIFRAME    1
#define SHPEXTENSION0    ".zsh"
#define SHPEXTENSION1    ".newzsh"
#define SHPEXTENSION2    ""
#define SHPEXTENSION3    ""
#define SHPMACTYPE0      GMAKEID('.','z','s','h')
#define SHPMACTYPE1      0
#define SHPMACTYPE2      0
#define SHPMACTYPE3      0
#define SHPSHORTTYPE     "ZSH"
#define SHPWORDTYPE      "ZSH"
#define SHPLONGTYPE      "Xenon Shape File"
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

