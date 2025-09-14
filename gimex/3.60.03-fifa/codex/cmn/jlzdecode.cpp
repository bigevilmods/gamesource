// (c) 1995-2004 Electronic Arts Inc.

// 1.0 122 MB/s
// 1.1 improved performance. 200 MB/s -> 252 MB/s using uint, better branch order, removing size

//#define USEC 1
#ifndef __JLZREAD
#define __JLZREAD 1

#include <string.h>
#include "codex.h"
#include "jlzcodex.h"

/****************************************************************/
/*  Information Functions                                       */
/****************************************************************/

/* check for reasonable header: */
/* 10fb header */

bool GCALL JLZ_is(const void *compresseddata)
{
    bool ok=false;
    unsigned int packtype=ggeti(compresseddata,4);

    if (packtype==JLZCOMP_ID)
        ok = true;

    return(ok);
}


/****************************************************************/
/*  Decode Functions                                            */
/****************************************************************/

int GCALL JLZ_size(const void *compresseddata)
{
    int len=0;
    unsigned int packtype=ggeti(compresseddata,4);

    if (packtype==JLZCOMP_ID)
    {
        len = ggeti((char *)compresseddata+8,4);
    }

    return(len);
}

static inline void ShortMove(uint8 *pDest, uint8 *pSrc, uint32 Len)
{
	unsigned int Run = 0;
	do {
        pDest[Run] = pSrc[Run];
	} while(++Run < Len);
}

int32 JLZDecompress(uint8 *pSrc, uint8 *pOut)
{
	uint32 Offset, Run;

	// Header
	LZHeader *header = (LZHeader *)pSrc;

	if( JLZGETFIELD(header->ID) != JLZCOMP_ID )
	{
		//bPrintf("ERROR:  JLZDecompress() given data that was not LZCompressed!\n");
		return 0;
	}

	if( JLZGETFIELD(header->Version) != JLZCOMP_VERSION )
	{
		//bPrintf("ERROR:  JLZDecompress() given data that is from an old version of the compressor!  You have to REBUILD!\n");
		return 0;
	}

	uint8 *pEnd = pOut + JLZGETFIELD(header->UncompressedSize);
	pSrc += sizeof(LZHeader);

	// Data
	uint32 Control = 0x0100 | pSrc[0];
	uint32 RunType = 0x0100 | pSrc[1];
	pSrc += 2;

	while(pOut<pEnd)
	{
		if(!(Control & 1))			// Is it a literal
		{
            do
            {
           		*pOut++ = *pSrc++;
                Control >>= 1;
            } while (pOut<pEnd && !(Control & 1));

            if(Control == 0x01)
            {
                Control = 0x0100 | *pSrc++;
            }
            continue;
        }

        if(RunType & 1)		// Is it a short offset?
        {
            Run = (((uint32)(pSrc[0] >> NEAR_OFFBITS) << 8) | pSrc[1]) + 3;
            Offset = (pSrc[0] & NEAR_RUNMASK) + 1;

            // Long copy
            ShortMove(pOut, pOut - Offset, Run);
        }
        else
        {
            Offset = (((uint32)(pSrc[0] >> FAR_RUNBITS) << 8) | pSrc[1]) + NEAR_OFFMAX + 1;
            Run = (pSrc[0] & FAR_RUNMASK) + 3;

            // Short copy
            ShortMove(pOut, pOut - Offset, Run);
        }


        pSrc += 2;
        pOut += Run;

        Control >>= 1;
        if(Control == 0x01)
        {
            Control = 0x0100 | *pSrc++;
        }

        RunType >>= 1;
        if(RunType == 0x01)
        {
            RunType = 0x0100 | *pSrc++;
        }

	}

	return JLZGETFIELD(header->UncompressedSize);
}


int GCALL JLZ_decode(void *dest, const void *compresseddata, int *compressedsize)
{
    unsigned int ulen = JLZDecompress((uint8 *)compresseddata, (uint8 *)dest);

    if (compressedsize)
        *compressedsize =  ggeti((char *)compresseddata+12,4);

    return(ulen);
}

#endif

