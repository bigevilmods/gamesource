#ifndef GXUTIL_H
#define GXUTIL_H

#include "gimex.h"
/****************************************************************************/
/* Internal functions                                                       */
/****************************************************************************/
namespace gxutil {

	static int GX_fileexist(char *nextname);

#define ISPOWER2(v) (!((v)&((v)-1)))

	static int IsValidGSHSwizzleWidthHeight(int w,int h,int bpp);

	static int IsValidSSHSwizzleWidthHeight(int w,int h,int bpp);

	static int IsValidXSHSwizzleWidthHeight(int w,int h,int bpp);

	static int IsValidFSHSwizzleWidthHeight(int w,int h,int bpp);

	static bool IsShape(unsigned int signature);

	static bool IsShape(unsigned int signature, unsigned char EndChar);

	static bool IsGstFourCC(unsigned int packed);

	static bool IsMovie(GABOUT * about);

	bool GX_ismipmap(char * shape_ID,int len);

	static void GXVerifyFrame(GINFO * ginfo,GABOUT *wabout);

	static void gsettitle(char *title);

	static int isvalidextension(GABOUT *about,char *ext,int type);

	static char *GXextension(char *filename);

	static void TTF_pointsize(const char *OPTpointsize, const char *OPTantialias, const char *OPTstyle, int OPTitalic, int OPTbold, const char *OPTcharset, const char *OPTtypeface, const char *OPTaspect, const char *OPTtighten, const char *OPTtracking, const char *OPTsmallcaps, const char *OPTascii, const char *OPTbaseline, int frame);

	static void GX_settooltitle(GABOUT * about,char *fnamebuf,int pass);

	static bool GX_verifyoutputfile(GABOUT * wabout,int SPITargnum,int * w,int morethanonefile);

	static bool GX_isdecimal(const char *data);

}
#endif
