// (c) 2003-2004 Electronic Arts Inc.


#include "spit.h"
#include "gopen.h"
#include "oldpshgimex.h"
#include "madgimex.h"
#include "range.h"
#include "ranger.h"
#include "stdgimex.h"
#include "aboutlib.h"
#include "ustr.h"

int GINFO_palettehistogram(char *f,GINFO *ginfo,int *histogram);
void GINFO_showabout(GABOUT *about);
void GINFO_showaboutex(GABOUT *about,const char *fname,int ok);
int GINFO_showpalette(GINFO *ginfo, int *histogram, bool outsidecolours, const char *OPTpalrange, int verbose);
void showframeinfo(GINFO *ginfo,int *histogram, bool outsidecolours, int OPTcenter, int OPTcomment, int verbose);
void initstats(int *stats);
void updatestats(int *stats,GINFO *ginfo);
void showstats(int *stats,int psx);
bool justshowpalette(int OPTverbose,int OPTbanner, const char *OPTpalrange);

