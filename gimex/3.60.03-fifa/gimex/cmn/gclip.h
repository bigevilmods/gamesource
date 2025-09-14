// (c) 1997-2007 Electronic Arts Inc.

#ifndef __GCLIP_H
#define __GCLIP_H 1

#ifdef __cplusplus
extern "C" {
#endif

typedef void GCLIP;

GCLIP *GCLIP_wopen(void);
int GCLIP_write(GCLIP *gclip, unsigned int Format, void *pSrc, int DataSize);
int GCLIP_wclose(GCLIP *gclip);

#ifdef __cplusplus
}
#endif
#endif



