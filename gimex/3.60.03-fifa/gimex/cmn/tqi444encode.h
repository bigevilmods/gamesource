// (c) 2000-2007 Electronic Arts Inc.
/* */
/* */
/*		TQI444 Encoder */
/* */
/*		Martin Griffiths, Bullfrog Productions Ltd. */
/* */
/* */
/*		Last Updated : 18th May 1999 */



#ifndef __TQI444ENCODE_H
#define __GIMEX_H 1

#ifndef ARGB
#define ARGB ARGB
#if (defined(_MSC_VER) && !defined(_M_PPC)) || defined(__i386__) 

    typedef struct
    {
        GCHANNEL b,g,r,a;
    } ARGB;

#elif defined(__R5900) || defined(SGI) /* PS2 */

    typedef struct
    {
        GCHANNEL r,g,b,a;
    } ARGB;

#else /* PS3/XBox360/GameCube/Mac */

    typedef struct
    {
        GCHANNEL a,r,g,b;
    } ARGB;
#endif
#endif


#define TQI444_ALPHA		(1<<0)
#define TQI444_COLOURKEY	(1<<1)
#define TQI444_YUV			(1<<2)

int tqi444_encode( ARGB *image,unsigned char *output,int width,int height, int pitch,ARGB ColorKey,int flags );


#endif
