// (c) 1994-2007 Electronic Arts Inc.

/* Include this code in your tools and libraries */
/* */
/* Two main entry points: */
/* */
/* int packshapedata(char *s, char *d, int w, int h, int bpp) */
/* */
/* Generic raw packer.  use it with shapes, cels, whatever */
/* */
/* int packshpfile(char *s, char *d) */
/* */
/* Packs a shape file, preserving links. */
/* types supported are: */
/* 0x20      - plut */
/* 0x21      - short plut */
/* 0x22      - vga palette */
/* 0x16,0x56 - uncoded 16 */
/* 0x05,0x45 - coded 8 */

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************/
/*  Types used by Animator                                      */
/****************************************************************/

typedef int bboolean;
typedef signed char Int8;
typedef signed short Int16;
typedef unsigned char UInt8;
typedef unsigned short UInt16;
typedef unsigned int UInt32;
typedef Int8 *PInt8;
typedef Int16 *PInt16;
#define ttrue -1
#define ffalse 0

/****************************************************************/
/*  Pack 16bpp Frame                                            */
/****************************************************************/

int packshapedata(char *s, int srcRowByte, char *d, int w, int h, int sbits, int bpp,int TransColor,int BlackPacked);

#ifdef __cplusplus
}
#endif

