// (c) 2003-2004 Electronic Arts Inc.

#include "spit.h"
#include "stdgimex.h"

#include <windows.h>
#include <vfw.h>

static int ConvertFourCC(const char * str)
{
	int value = 0;
	int l = (int)strlen(str);
	value = 0;
	if (l == 4)
	{
		for(int i = 3; i >= 0; i--)
		{
			value |= ((int)(str[i])<<(i*8));
		}
		return value;
	}
	else
	{
		printf("ERROR: 4CC must be exactly 4 characters\n");
		return 0;
	}
}

void GX_ShowCodecs(const char *fourcc=0)
{
    ICINFO icinfo;
	HIC      hic;
	int fcc = 0;
	if (fourcc)
		fcc = ConvertFourCC(fourcc);
    memset(&icinfo,0,sizeof(ICINFO));
	for (int i = 0; ICInfo(0,i,&icinfo);i++)
	{
		int old4cc = 0;
		hic = ICOpen(icinfo.fccType,icinfo.fccHandler,ICMODE_QUERY);
		if (hic && (fcc == 0 || fcc == icinfo.fccHandler))
		{
			old4cc = icinfo.fccHandler;
			ICGetInfo(hic,&icinfo,sizeof(icinfo));
			printf("------------------------------------\n");
			printf("Initial 4CC:\t%c%c%c%c\n", (old4cc)&0xff,(old4cc>>8)&0xff,(old4cc>>16)&0xff,(old4cc>>24)&0xff);
			printf("FCC:\t\t%c%c%c%c\n",(icinfo.fccHandler)&0xff,(icinfo.fccHandler>>8)&0xff,(icinfo.fccHandler>>16)&0xff,(icinfo.fccHandler>>24)&0xff);
			wprintf(L"Name:\t\t%s\n",icinfo.szName);
			wprintf(L"Description:\t%s\n",icinfo.szDescription);
			printf("Version:\t%d\n",icinfo.dwVersion);
			wprintf(L"Driver:\t\t%s\n",icinfo.szDriver);
			printf("FLAGS:\t\t%s%s%s\n\t\t%s%s\n\t\t%s%s\n",(icinfo.dwFlags&VIDCF_COMPRESSFRAMES)?"Compress All Frames,":"",
		                                                                           (icinfo.dwFlags&VIDCF_CRUNCH)?"Compress To Frame Size,":"",
																				   (icinfo.dwFlags&VIDCF_DRAW)?"Can Draw,":"",
																				   (icinfo.dwFlags&VIDCF_FASTTEMPORALC)?"Temporal Compression,":"",
																				   (icinfo.dwFlags&VIDCF_FASTTEMPORALD)?"Temporal Decompression,":"",
																				   (icinfo.dwFlags&VIDCF_QUALITY)?"Quality Value Support,":"",
																				   (icinfo.dwFlags&VIDCF_TEMPORAL)?"InterFrame Compression,":"");

		}
		memset(&icinfo,0,sizeof(ICINFO));
		ICClose(hic);
	}
	return;
}


