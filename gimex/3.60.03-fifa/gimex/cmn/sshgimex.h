// (c) 1998-2007 Electronic Arts Inc.


#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SHP_about     SSH_about
#define SHP_is        SSH_is
#define SHP_open      SSH_open
#define SHP_info      SSH_info
#define SHP_read      SSH_read
#define SHP_close     SSH_close
#define SHP_wopen     SSH_wopen
#define SHP_write     SSH_write
#define SHP_wclose    SSH_wclose

#define SHPSIGNATURE     GMAKEID('S','h','p','S')
#define FNTSIGNATURE     GMAKEID('F','n','t','S')
#define SHPMULTIFRAME    1
#define SHPEXTENSION0    ".ssh"
#define SHPEXTENSION1    ".newssh"
#define SHPEXTENSION2    ""
#define SHPEXTENSION3    ""
#define SHPMACTYPE0      GMAKEID('.','s','s','h')
#define SHPMACTYPE1      0
#define SHPMACTYPE2      0
#define SHPMACTYPE3      0
#define SHPSHORTTYPE     "SSH"
#define SHPWORDTYPE      "SSH"
#define SHPLONGTYPE      "PS2 Shape File"
#define SHPDATAALIGNMENT 63
#define SHPMIPMAPDATAALIGNMENT 15
#define SHPCLUTDATAALIGNMENT 15
#define SHPPACKEDDATAALIGNMENT 15

#define ggetx ggeti
#define gputx gputi

#include "shpgimex.h"

#ifdef __cplusplus
}
#endif

