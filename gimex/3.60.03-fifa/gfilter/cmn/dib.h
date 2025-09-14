// (c) 1999-2007 Electronic Arts Inc.

#ifndef DIB_H
#define DIB_H

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_MSC_VER)
#pragma warning(push,1)
#include <windows.h>
#pragma warning(pop)
#endif

typedef struct DIB DIB;

/* construction\destruction */
DIB*        DIB_create(int width, int height, int bpp);
void        DIB_destroy(DIB*);

/* attributes */
int         DIB_getwidth(const DIB*);       /* image width in pixels */
int         DIB_getheight(const DIB*);      /* image height in pixels */
int         DIB_getbpp(const DIB*);         /* number bits per pixel */
int         DIB_getrowbytes(const DIB*);    /* number of bytes per row */
void*       DIB_getbase(DIB*);				/* ptr to the bits which can be modified */

/* palette attributes (for DIB's that <= 8bpp) */
RGBQUAD*    DIB_getcolourtable(DIB*);       /* ptr to colour table - which can be modified */
int         DIB_getcolourcount(const DIB*); /* number of colour table entries */

/* windows attributes */
HBITMAP				DIB_getbitmap(const DIB*);     /* handle of GDI bitmap object */
const BITMAPINFO*	DIB_getbitmapinfo(const DIB*); /* bitmap info */

/* operations */
void		DIB_draw(const DIB* dib, HDC hdc, int x, int y);

#ifdef __cplusplus
}
#endif

#endif
