// (c) 2007 Electronic Arts Inc.

#ifndef EADXT_H
#define EADXT_H 1

#ifndef ARGB
#define ARGB ARGB
#if (defined(_MSC_VER) && !defined(_M_PPC)) || defined(__i386__) /* Windows/Linux/XBox/Intel Mac */

    typedef struct
    {
        unsigned char b,g,r,a;
    } ARGB;

#elif defined(__R5900) || defined(SGI) /* PS2/SGI */

    typedef struct
    {
        unsigned char r,g,b,a;
    } ARGB;

#else /* PS3/XBox360/GameCube/Wii/PPC Mac */

    typedef struct
    {
        unsigned char a,r,g,b;
    } ARGB;
#endif
#endif

/* RGB options */
#define EADXT_MODE_RGB_NONE                    0x010

/* ALPHA options */
#define EADXT_MODE_ALPHA_NONE                  0x000
#define EADXT_MODE_ALPHA_EXPLICIT              0x100
#define EADXT_MODE_ALPHA_INTERPOLATED          0x200
#define EADXT_MODE_ALPHA_INTERPOLATED_FROM_RED 0x400

/* Swizzle options */
#define EADXT_MODE_ENDIAN_XENON                0x800
#define EADXT_MODE_ENDIAN_WII                  0x1000

/* Masks */
#define EADXT_MODE_FORMAT_MASK				   0x7FF
#define EADXT_MODE_SWIZZLE_MASK				   0x1800

unsigned int eadxt(void *dataOut, const void *dataIn, int rowbytes, int numblocks, int mode, int quality);
unsigned int eadxtimage(void *dataOut, int rowoutbytes, const void *dataIn, int width, int height, int rowbytes, int mode, int quality);
void eadxtimageOpenMP(void *dataOut, int rowoutbytes, const void *dataIn, int width, int height, int rowbytes, int mode, int quality);

#endif
