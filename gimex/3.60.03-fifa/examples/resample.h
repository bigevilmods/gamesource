// (c) 1999-2007 Electronic Arts Inc.
/* Include files */
#include <gimex.h>


/* The following are constants used in the resampling process */
#define rsColourReductionMethod   "Wu"         /* Wu colour reduction */
#define rsResizeMethod            0            /* Lanczos resizing */
#define rsDitherMethod            1


/* Function prototypes */
GBITMAP *resampleimage(GBITMAP *sourceimage, int width, int height, int bpp);

