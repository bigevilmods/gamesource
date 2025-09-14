// (c) 1995-2004 Electronic Arts Inc.

#ifndef __JLZCODEX
#define __JLZCODEX 1

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __CODEX_H
#error "Include codex.h before jlzcodex.h"
#endif

/****************************************************************/
/*  JLZ Codex                                                   */
/****************************************************************/

/* Information Functions */

CODEXABOUT *GCALL JLZ_about(void);
bool        GCALL JLZ_is(const void *compresseddata);

/* Decode Functions */

int        GCALL JLZ_size(const void *compresseddata);
#ifdef __cplusplus
int        GCALL JLZ_decode(void *dest, const void *compresseddata, int *compressedsize=0);
#else
int        GCALL JLZ_decode(void *dest, const void *compresseddata, int *compressedsize);
#endif

/* Encode Functions */

#ifdef __cplusplus
int        GCALL JLZ_encode(void *compresseddata, const void *source, int sourcesize, int *opts=0);
#else
int        GCALL JLZ_encode(void *compresseddata, const void *source, int sourcesize, int *opts);
#endif

/****************************************************************/
/*  Internal                                                    */
/****************************************************************/

#define int32 int
#define uint32 unsigned int
#define uint16 unsigned short
#define uint8 unsigned char

#define bMemSet memset

// Compressed Data Header
class LZHeader
{
public:
	uint32 ID;						// "COMP"
	uint8  Version;					// Version of this compressed data
	uint8  HeaderSize;				// Sizeof this header
	uint16 Flags;					// LZFLAGS_xxx
	uint32 UncompressedSize;		// Size of uncompressed data
	uint32 CompressedSize;			// Size of compressed data including 'HeaderSize' (some confusion here - don't trust this)
};


#define JLZCOMP_ID				(('J' << 0) | ('D' << 8) | ('L' << 16) | ('Z' << 24))
#define JLZCOMP_VERSION			2

//
// Constants
//

static const int32 JLZ_HTSize			= 8192;

static const int32 NEAR_RUNBITS		= 12;
static const int32 NEAR_OFFBITS		= (16 - NEAR_RUNBITS);
static const int32 NEAR_RUNMAX			= (1 << NEAR_RUNBITS) + 2;
static const int32 NEAR_OFFMAX			= (1 << NEAR_OFFBITS);
static const int32 NEAR_RUNMASK		= (1 << NEAR_OFFBITS) - 1;

static const int32 FAR_RUNBITS			= 5;
static const int32 FAR_RUNMAX			= (1 << FAR_RUNBITS) + 2;
static const int32 FAR_RUNMASK			= (1 << FAR_RUNBITS) - 1;

static const int32 WINDOWSIZE			= (1 << (16 - FAR_RUNBITS)) + (1 << (16 - NEAR_RUNBITS));


#ifndef qmin
#define qmin(a,b) ((a)<(b)?(a):(b))
#endif

#ifndef qmax
#define qmax(a,b) ((a)>(b)?(a):(b))
#endif

#define JLZGETFIELD(field) ggeti((void *)&field, sizeof(field))
#define JLZPUTFIELD(field,data) gputi((void *)&field,(unsigned int)(data),sizeof(field))


#ifdef __cplusplus
}
#endif
#endif

