// (c) 1994-2007 Electronic Arts Inc.

#ifndef __XPRGIMEX
#define __XPRGIMEX 1

#include "gimex.h"

#ifdef __cplusplus
extern "C" {
#endif

#define XPR_MAXFRAMENAME 64

typedef enum
{
    GPUTEXTUREFORMAT_1_REVERSE                      = 0,
    GPUTEXTUREFORMAT_1                              = 1,
    GPUTEXTUREFORMAT_8                              = 2,
    GPUTEXTUREFORMAT_1_5_5_5                        = 3,
    GPUTEXTUREFORMAT_5_6_5                          = 4,
    GPUTEXTUREFORMAT_6_5_5                          = 5,
    GPUTEXTUREFORMAT_8_8_8_8                        = 6,
    GPUTEXTUREFORMAT_2_10_10_10                     = 7,
    GPUTEXTUREFORMAT_8_A                            = 8,
    GPUTEXTUREFORMAT_8_B                            = 9,
    GPUTEXTUREFORMAT_8_8                            = 10,
    GPUTEXTUREFORMAT_Cr_Y1_Cb_Y0_REP                = 11,
    GPUTEXTUREFORMAT_Y1_Cr_Y0_Cb_REP                = 12,
    GPUTEXTUREFORMAT_16_16_EDRAM                    = 13, // EDRAM render target only
    GPUTEXTUREFORMAT_8_8_8_8_A                      = 14,
    GPUTEXTUREFORMAT_4_4_4_4                        = 15,
    GPUTEXTUREFORMAT_10_11_11                       = 16,
    GPUTEXTUREFORMAT_11_11_10                       = 17,
    GPUTEXTUREFORMAT_DXT1                           = 18,
    GPUTEXTUREFORMAT_DXT2_3                         = 19,
    GPUTEXTUREFORMAT_DXT4_5                         = 20,
    GPUTEXTUREFORMAT_16_16_16_16_EDRAM              = 21, // EDRAM render target only
    GPUTEXTUREFORMAT_24_8                           = 22,
    GPUTEXTUREFORMAT_24_8_FLOAT                     = 23,
    GPUTEXTUREFORMAT_16                             = 24,
    GPUTEXTUREFORMAT_16_16                          = 25,
    GPUTEXTUREFORMAT_16_16_16_16                    = 26,
    GPUTEXTUREFORMAT_16_EXPAND                      = 27,
    GPUTEXTUREFORMAT_16_16_EXPAND                   = 28,
    GPUTEXTUREFORMAT_16_16_16_16_EXPAND             = 29,
    GPUTEXTUREFORMAT_16_FLOAT                       = 30,
    GPUTEXTUREFORMAT_16_16_FLOAT                    = 31,
    GPUTEXTUREFORMAT_16_16_16_16_FLOAT              = 32,
    GPUTEXTUREFORMAT_32                             = 33,
    GPUTEXTUREFORMAT_32_32                          = 34,
    GPUTEXTUREFORMAT_32_32_32_32                    = 35,
    GPUTEXTUREFORMAT_32_FLOAT                       = 36,
    GPUTEXTUREFORMAT_32_32_FLOAT                    = 37,
    GPUTEXTUREFORMAT_32_32_32_32_FLOAT              = 38,
    GPUTEXTUREFORMAT_32_AS_8                        = 39,
    GPUTEXTUREFORMAT_32_AS_8_8                      = 40,
    GPUTEXTUREFORMAT_16_MPEG                        = 41,
    GPUTEXTUREFORMAT_16_16_MPEG                     = 42,
    GPUTEXTUREFORMAT_8_INTERLACED                   = 43,
    GPUTEXTUREFORMAT_32_AS_8_INTERLACED             = 44,
    GPUTEXTUREFORMAT_32_AS_8_8_INTERLACED           = 45,
    GPUTEXTUREFORMAT_16_INTERLACED                  = 46,
    GPUTEXTUREFORMAT_16_MPEG_INTERLACED             = 47,
    GPUTEXTUREFORMAT_16_16_MPEG_INTERLACED          = 48,
    GPUTEXTUREFORMAT_DXN                            = 49,
    GPUTEXTUREFORMAT_8_8_8_8_AS_16_16_16_16         = 50,
    GPUTEXTUREFORMAT_DXT1_AS_16_16_16_16            = 51,
    GPUTEXTUREFORMAT_DXT2_3_AS_16_16_16_16          = 52,
    GPUTEXTUREFORMAT_DXT4_5_AS_16_16_16_16          = 53,
    GPUTEXTUREFORMAT_2_10_10_10_AS_16_16_16_16      = 54,
    GPUTEXTUREFORMAT_10_11_11_AS_16_16_16_16        = 55,
    GPUTEXTUREFORMAT_11_11_10_AS_16_16_16_16        = 56,
    GPUTEXTUREFORMAT_32_32_32_FLOAT                 = 57,
    GPUTEXTUREFORMAT_DXT3A                          = 58,
    GPUTEXTUREFORMAT_DXT5A                          = 59,
    GPUTEXTUREFORMAT_CTX1                           = 60,
    GPUTEXTUREFORMAT_DXT3A_AS_1_1_1_1               = 61,
    GPUTEXTUREFORMAT_8_8_8_8_GAMMA_EDRAM            = 62, // EDRAM render target only
    GPUTEXTUREFORMAT_2_10_10_10_FLOAT_EDRAM         = 63, // EDRAM render target only
} GPUTEXTUREFORMAT;


/****************************************************************/
/*  XPR Raw Gimex                                               */
/****************************************************************/

/* Information Functions */

GABOUT * GCALL XPR_about(void);
int      GCALL XPR_is(GSTREAM *g);

/* Import Functions */

int      GCALL XPR_open(GINSTANCE **gx, GSTREAM *g, const char *pathname,bool framecountflag);
GINFO *  GCALL XPR_info(GINSTANCE *gx, int framenum);
int      GCALL XPR_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes);
int      GCALL XPR_close(GINSTANCE *gx);

/* Export Functions */

int      GCALL XPR_wopen(GINSTANCE **gx, GSTREAM *g, const char *pathname, int numframes);
int      GCALL XPR_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes);
int      GCALL XPR_wclose(GINSTANCE *gx);

#ifdef __cplusplus
}
#endif
#endif

