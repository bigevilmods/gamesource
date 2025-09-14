// (c) 1994-2007 Electronic Arts Inc.

#ifndef __MADDEC
#define __MADDEC 1

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_MSC_VER)
#pragma optimize("g", off)
#endif

/****************************************************************/
/*  Madcow Internals                                            */
/****************************************************************/

typedef struct
{
    char    cr;
    char    cb;
    char    y2;
    char    y1;

} YUVPAIR;

/* Madcow decoder */

void madinitdecode(unsigned short *src, int motion, int quality);
void maddecodemacroblock(YUVPAIR *src, YUVPAIR *dest, int width);

// internal functions
int madvlcdecode(void);
void madoutputrow(const int *luma, const int *chroma, YUVPAIR *dest);

typedef struct
{
    char    id[4];
    char	size[4];
    char    reserved[4];	// used to be CRC
    char    time[4];		// fps = (1000.0 * 65536.0)/time;
    char    width[2];
    char    height[2];
    char    ver[1];			// 0 = original , 1 = 601 style YUV coeficients . 2 = flags present
    char    quality[1];
    char    flags[1];		// 1 = keyframe , 2 = extra frame
	char    reserved2[1];
} MADCHUNKHDR;

#define MADKEYFLAG    0x0
#define MADEXTRAFLAG  0x1
#define MADMOTIONFLAG 0x2

/* Variables and function for computing an IDCT (if file IDCT). */

extern int idctinput[8*8];
extern int const idctprescale[8*8];
extern void idctcompute(int *dest, int stride);

#ifdef __cplusplus
}
#endif
#endif

