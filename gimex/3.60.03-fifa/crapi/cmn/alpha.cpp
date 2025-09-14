// (c) 1995-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*               ALPHA - WU 4D with alpha CRapi Module              */
/*                                                                  */
/*        by Jeff Lefferts, Kaz Okuda and Frank Barchard EAC        */
/*                                                                  */
/*                    Main Module - Mar 24, 2003                    */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History                                        */
/* ------- ------ -- -------                                        */
/*    1.00 970606 KO based on recolor by Jeff Lefferts based on WU  */
/*    1.01 970606 KO support numcolors removed InitRecolor          */
/*    1.02 970609 KO added support for 8 bit input                  */
/*    1.03 970922 GS fixed bug with setting NULL inverse table      */
/*    1.04 980625 KO allocate palette in close not open.            */
/*    1.05 981224 FB bumped to 5666 cube                            */
/*    1.06 990212 FB dynamic size, limited to 4555 (looks better)   */
/*    1.07 010329 FB use ARGB directly, pad table with zeros        */
/*    1.08 010406 FB revert to 1.06 but with pad table fixed        */
/*    1.09 010515 ID fixed small bug with palette size              */
/*    1.10 020328 ID fix to do with palette quality, 6666 cube      */
/*    1.11 030324 ID respectalpha                                   */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* Color reduction using variance minimization.  See Wu's article in*/
/* Graphics Gems II.  Based on "Under The Rainbow", Dec. 1995, Game */
/* Developer, by Matt Pritchard and Rich Goldrich.                  */
/*                                                                  */
/* Includes alpha information.                                      */
/* Supports 32 and 8 bit input image                                */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include <string.h>
#include <ctype.h>
#include "alpha.h"


/****************************************************************/
/* BEGIN Internal declarations from def.h in recolor            */
/****************************************************************/

        #ifdef DEBUG
        #undef DEBUG
        #endif

        // Enable debugging code (stringent error checking, etc.)
        #define DEBUG                           1

        // -------- Standard types
        typedef char            BOOL;   // Boolean value (non-0 = TRUE)
        typedef char            CHAR;   // ascii character (8-bit signed)
        typedef unsigned char   BYTE;   // 8-bit unsigned
        typedef unsigned char   UCHAR;  // 8-bit unsigned
        typedef short           SHORT;  // 16-bit signed
        typedef unsigned short  USHORT; // 16-bit unsigned
        typedef int             LONG;   // 32-bit signed
        typedef unsigned int    ULONG;  // 32-bit unsigned

        #define UINT32_MAX 0xFFFFFFFF

        #if !(__MC68K__ || __powerc)
                // Don't need these defined for the Mac
                #define FALSE                   0
                #define TRUE                    1
        #endif

        // -------- Standard names for constants
        #define ON                                      TRUE
        #define OFF                                     FALSE


        // -------- ASSERT macro, terminates program on errors if DEBUG is on
        #if DEBUG

                #if __MC68K__ || __powerc
                        // Macintosh ASSERTs
                        #include <UException.h>
                        #define ASSERT(x)       Assert_(x)

                #elif defined(__sgi)
                        // SGI ASSERTs
                        #define ASSERT(x)
                        #define ThrowIfNil_(data)

                #else
                        // PC ASSERTs
                        #define ASSERT(x)       {if (!(x))\
                                {{_asm int 3}\
                                ErrorExit("\nASSERT: line %d, file %s\n",\
                                __LINE__, __FILE__);}}


                        #define ThrowIfNil_(data)       {if ((data) == NULL)\
                                {{_asm int 3}\
                                ErrorExit("\nNull Pointer detected: line %d, file %s\n",\
                                __LINE__, __FILE__);}}
                #endif

        #else
                // Turn off debugging macro
                #define ASSERT(x)

                #if !(__MC68K__ || __powerc)
                        // Turn off debugging macro
                        #define ThrowIfNil_(data)
                #endif

        #endif  // DEBUG


        // -------- returns Nth argument on the stack, past firstArg.  For example:
        //              VA_ARG (firstArg, 0) returns firstArg
        //              VA_ARG (firstArg, 1) returns first 32-bit arg past firstArg
        #define VA_ARG(firstArg,n)              *((LONG *) &(firstArg) + n)

        // Return MAXimum or MINimum of two passed values
        #define MAX(a,b) (((a) > (b)) ? (a) : (b))
        #define MIN(a,b) (((a) < (b)) ? (a) : (b))

/****************************************************************/
/* END Internal declarations from def.h in recolor              */
/****************************************************************/



/****************************************************************/
/* BEGIN Internal declarations from graphics.h in recolor       */
/****************************************************************/

//--------------------- Macros ---------------------------------

// Alpha value thresholds for PSX
#define PSX_TRANSPARENT         192
#define PSX_TRANSLUCENT         64

// Isolate RGB or A from a ULONG.
#define GET_RED(rgba)           ((BYTE) rgba)
#define GET_GREEN(rgba)         ((BYTE) (rgba >> 8))
#define GET_BLUE(rgba)          ((BYTE) (rgba >> 16))
#define GET_ALPHA(rgba)         ((BYTE) (rgba >> 24))

// Shift RGB or A into place, to OR into a ULONG.
#define PUT_RED(red)            ( (ULONG) (red))
#define PUT_GREEN(green)        (((ULONG) (green)) << 8)
#define PUT_BLUE(blue)          (((ULONG) (blue))  << 16)
#define PUT_ALPHA(alpha)        (((ULONG) (alpha)) << 24)


//--------------------- Procedures -----------------------------

//static USHORT  PSXPixel(ULONG pixel); JF - this is not used.
static ULONG   ConvertDepth(int srcRange, int destRange, ULONG value);
static ULONG   ConvertDepthMulti(int srcRange, int destRange, ULONG value, ULONG weight);
static ULONG   ConvertBitDepth(int srcDepth, int destDepth, ULONG value);
static ULONG   ConvertBitDepthMulti(int srcDepth, int destDepth, ULONG value, ULONG weight);

/****************************************************************/
/* END Internal declarations from graphics.h in recolor         */
/****************************************************************/



/****************************************************************/
/* BEGIN Internal declarations from varymin.h in recolor        */
/****************************************************************/

//--------------------- Macros ---------------------------------

// Color Reduction flags
#define CR_INV_SRC              0x00001
#define CR_INV_DEST             0x00002
#define CR_ONE_PALETTE  0x00004
#define CR_PAL_ONLY             0x00008
#define CR_ANTI_ALIAS   0x00010
#define CR_ZERO_ALPHA   0x00020
#define CR_BKGD_COLOR   0x00040
#define CR_BKGD_INDEX   0x00080
#define CR_SORT_LUMIN   0x00100
#define CR_SORT_POP             0x00200
#define CR_NO_PALETTE   0x00400
#define CR_FIXED_ALPHA  0x00800
#define CR_PAD_PAL              0x01000
#define CR_PAD_TIM              0x02000
#define CR_PREATTENUATE 0x04000
#define CR_SCALE_ALPHA  0x08000
#define CR_DITHER_ALPHA 0x10000

// The color tesseract (4D color "cube")
#define CUBE_PTR(r, g, b, a) (gColorSpace +     ((a) * alphaStride) + \
                                                ((r) * redStride) + \
                                                ((g) * greenStride) + \
                                                (b))


//--------------------- Structures -----------------------------

enum {DITHER_NONE, DITHER_FLOYD, DITHER_BURKES, DITHER_STUCKI};

enum {EXTRA_R, EXTRA_G, EXTRA_B, EXTRA_A};

typedef struct
        {
        // These four can change for each image to process
        BOOL    src32Bit;              // Did the source file have an alpha channel?
        BOOL    srcPaletted;           // Was the source file paletted?
        int    xPos;                   // Position of the bitmap (written into the header)
        int    yPos;

        // These are for matching transparent/opaque exactly (CR_FIXED_ALPHA)
        BOOL    anyTransparent; // Did any source file have transparency?       (A = 255)
        BOOL    anyTranslucent; // Did any source file have translucency?       (0 < A < 255)
        BOOL    anyOpaque;      // Did any source file have opaque pixels?      (A = 0)

        // These are global options
        int     colors;                 // Number of colors to quantize to
        ULONG   crFlags;                // Color reduction flags (see CR_* flags above)
        int     rBits, gBits, bBits;    // Number of bits to sample R, G, B
        int     maxBits;                // Max. of rBits, gBits, bBits (to match scales)
        int     alphaVals;              // Number of levels to sample A (alpha)
        int     palBits;                // Output palette bits 16 24 32 -- need to default if not specified
        int     extraBit;               // Where extra bit goes (see EXTRA_* enum above)
        int     timIndexBits;           // Number of bits to write tim indices, 4 or 8.
        int     palFirst;               // First palette entry number
        char   *palName;                // Palette name if saved separately
        int     dither;                 // Dither type (see DITHER_* enum above)
        int     newXPos;                // if not -1, move image to this position
        int     newYPos;                // if not -1, move image to this position
        int     xPosPal;                // Position of the palette (written into the header)
        int     yPosPal;
        BOOL    trueColor;              // Output a RGBA direct image (no palette)
        ULONG   transColor;             // Color to turn transparent
        int     transIndex;             // Where to put transparent color in palette (-1 = don't care)
        double  opaqueWeight;          // If > 1, tends to prefer creating opaque colors
        int     alphaScale;
        } PREFS;


typedef struct NAMES
        {
        struct  NAMES *next;
        char    *inName;
        char    *outName;
        } NAMES;

//--------------------- Procedures -----------------------------

static int InitColorCube(PREFS *gPrefs);
static void FillColorCube(const GINFO *ginfo, char *image, int rowbytes);
static int    MakePalette(ULONG *palette);
static void   FreeColorCube(void);

/****************************************************************/
/* END Internal declarations from varymin.h in recolor          */
/****************************************************************/


/****************************************************************/
/* BEGIN inclusion of graphics.c from recolor                   */
/****************************************************************/

//--------------------- Procedures -----------------------------

//--------------------------------------------------------------------------
// Converts a value in a certain range to a value in a different range.
// The error of converting back & forth is kept to a minimum, and the range
// of the destination is preserved.  That is, the highest value in the
// srcRange will convert to the highest value in the destRange, and values
// in between will be scaled evenly in the range.
//--------------------------------------------------------------------------
static __inline ULONG ConvertDepth(int srcRange, int destRange, ULONG value)
        {
        // Avoid divide by 0
        if ((srcRange <= 1) || (destRange <= 1))
                return(0);

        if (destRange < srcRange)
                {
                // Distribute maximum error over range
                value += ((srcRange / destRange) / 2) - (value / destRange);

                // Scale to new range
                value = (value * destRange) / srcRange;
                }
        else if (destRange > srcRange)
                {
                // Scale highest source value to highest dest. value (preserves range)
                value = (value * (destRange - 1)) / (srcRange - 1);
                }

        return (value);
        }


//--------------------------------------------------------------------------
// Similar to ConvertDepth, but "value" represents the sum of "weight"
// values.  The value is scaled, and then averaged, so the returned value
// will be in the range 0 to destRange-1.
//--------------------------------------------------------------------------
static __inline ULONG ConvertDepthMulti(int srcRange, int destRange, ULONG value, ULONG weight)
        {
        value = (value + (weight / 2)) / weight;
        return (ConvertDepth(srcRange, destRange, value));
        }


//--------------------------------------------------------------------------
// Same as ConvertDepth, but takes a bit depth as input instead of a range.
//--------------------------------------------------------------------------
static __inline ULONG ConvertBitDepth(int srcDepth, int destDepth, ULONG value)
        {
        return(ConvertDepth(1 << srcDepth, 1 << destDepth, value));
        }


//--------------------------------------------------------------------------
// Same as ConvertDepthMulti, but takes a bit depth as input instead of a range.
//--------------------------------------------------------------------------
static __inline ULONG ConvertBitDepthMulti(int srcDepth, int destDepth, ULONG value, ULONG weight)
        {
        return(ConvertDepthMulti(1 << srcDepth, 1 << destDepth, value, weight));
        }


/****************************************************************/
/* END inclusion of graphics.c from recolor                     */
/****************************************************************/


/****************************************************************/
/* BEGIN inclusion of varymin.c from recolor                    */
/****************************************************************/

//--------------------------------------------------------------------------
// Color reduction using variance minimization.  See Wu's article in
// Graphics Gems II.  Based on "Under The Rainbow", Dec. 1995, Game
// Developer, by Matt Pritchard and Rich Goldrich.
//
// Adapted by Jeff Lefferts, December 1995 - February 1996.  Added alpha
// channel support for PSX and dithering.
//
// TBD / possible enhancements:
//              Take into account Windows default palette (or any fixed palette).
//              Smart Dither: Does not use error-adjusted pixel value if it's too far
//                      away from original pixel value.  Tolerance adjustable.
//              RGB only version for no alpha?
//
//--------------------------------------------------------------------------


//--------------------- Includes -------------------------------

// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <ctype.h>
// #include <limits.h>
// #include "def.h"
// #include "disk.h"
// #include "graphics.h"
// #include "varymin.h"
// #include "gimex.h"


//--------------------- Macros ---------------------------------

// Not done yet...
#define SMART_DITHER            FALSE


//--------------------- Structures -----------------------------

typedef struct BOX
        {
        ULONG   variance;               // weighted variance
        ULONG   weight;                 // total weight (number of pixels in box)
        ULONG   rgbaWeight;             // (R*R + G*G + B*B + A*A) of whole box, times weight

        ULONG   rWeight;                        // Total of (R * weight) for each color in box
        ULONG   gWeight;                        // Total of (G * weight) for each color in box
        ULONG   bWeight;                        // Total of (B * weight) for each color in box
        ULONG   aWeight;                        // Total of (A * weight) for each color in box

        int             rMin, gMin, bMin, aMin;         // Upper and lower bounds of box
        int             rMax, gMax, bMax, aMax;
        } BOX;


typedef struct DITHER
        {
        int     width, height;
        int     xOffset;                // never a Y offset
        int     parts;
        int     cell[15];               // Increase this number if larger dither matrix needed
        } DITHER;


//--------------------- Variables ------------------------------

static ULONG   greenStride, redStride, alphaStride, cubeSize;
static ULONG   gSquaresArray[512];             // initilized in InitRecolor (which is called once)

static BOX     *boxList;
static BOX     *nextBox;
static ULONG   *gColorSpace;
static PREFS    gPrefs;
static BOOL     gVerbose;

#if 0

// Dither matrices (increase size of cell array in DITHER above if adding larger matrix)
static DITHER floyd = {3, 2, -1, 16,           // width, height, xOffet, parts
                       {0, 0, 7,                // error diffusion matrix
                        3, 5, 1, }};

static DITHER burkes = {5, 2, -2, 32,          // width, height, xOffet, parts
                        {0, 0, 0, 8, 4,         // error diffusion matrix
                         2, 4, 8, 4, 2,}};

static DITHER stucki = {5, 3, -2, 42,          // width, height, xOffet, parts
                        {0, 0, 0, 8, 4,         // error diffusion matrix
                         2, 4, 8, 4, 2,
                         1, 2, 4, 2, 1, }};
#endif

//--------------------- Procedures -----------------------------

static void    GetStats(BOX *box);
static ULONG   Variance(BOX *box);
static void    ShrinkBox(BOX *box);
static void    SplitBox(BOX *oldBox);
//static int             FindColor(ULONG *palette, int colors, ULONG color);
//static void    DistributeError(ULONG *pixel, ULONG palColor, int xPos, int yPos,
//                                                int width, int height, DITHER *dither);




//--------------------------------------------------------------------------
// KO - Initialize the structures needed for the colour reduction
//--------------------------------------------------------------------------
static int InitColorCube(PREFS *gPrefs)
{
    int     i;

    // Scale samples to maximum bit depth sampled
    gPrefs->maxBits = gPrefs->rBits;
    if (gPrefs->gBits > gPrefs->maxBits)
        gPrefs->maxBits = gPrefs->gBits;
    if (gPrefs->bBits > gPrefs->maxBits)
        gPrefs->maxBits = gPrefs->bBits;

    // Set 4D cube dimensions
    greenStride = 1 << gPrefs->bBits;
    redStride = greenStride * (1 << gPrefs->gBits);
    alphaStride = redStride * (1 << gPrefs->rBits);
    cubeSize = alphaStride * gPrefs->alphaVals * sizeof (ULONG);

    // Initialized only once for a super-palette
    gPrefs->anyTransparent = FALSE;
    gPrefs->anyTranslucent = FALSE;
    gPrefs->anyOpaque = FALSE;

    // Fill up Squares table
    for (i = 0; i < 256; i++)
        gSquaresArray[ i + 255] = gSquaresArray[-i + 255] = i * i;

    // Allocate space for the color cube
    gColorSpace = (ULONG *) galloc(cubeSize);       // R, G, B, A cube

    if (gColorSpace)
    {
        // Clear cube counts to 0
        memset(gColorSpace, 0, cubeSize);
    }

    return (gColorSpace?1:0);
}


static void FreeColorCube(void)
{
    if (gColorSpace)
    {
        gfree(gColorSpace);
        gColorSpace = 0;
    }
}


//--------------------------------------------------------------------------
// Get weighted color statistics from RGB cube histogram for the box passed.
//--------------------------------------------------------------------------
static void GetStats(BOX *box)
        {
        int             rIndex, gIndex, bIndex, aIndex;
        int             rVal, gVal, bVal, aVal;
        ULONG   *redPtr, *grnPtr, *bluePtr, *alphaPtr;
        ULONG   weight, aSqr, arSqr, argSqr;

        // Initialize stats to 0
        box->weight = 0;
        box->rWeight = 0;
        box->gWeight = 0;
        box->bWeight = 0;
        box->aWeight = 0;
        box->rgbaWeight = 0;

        // Scan through the subset of the color cube
        alphaPtr = CUBE_PTR (box->rMin, box->gMin, box->bMin, box->aMin);
        for (aIndex = box->aMin; aIndex <= box->aMax; ++aIndex)
                {
                // Weight alpha to match RGB scale * 2
                aVal = ConvertDepth(gPrefs.alphaVals, 1 << (gPrefs.maxBits + 1), aIndex);
                aSqr = aVal * aVal;
                redPtr = alphaPtr;

                for (rIndex = box->rMin; rIndex <= box->rMax; ++rIndex)
                        {
                        // Weight red to match max. sample bits
                        rVal = ConvertBitDepth(gPrefs.rBits, gPrefs.maxBits, rIndex);
                        arSqr = aSqr + (rVal * rVal);
                        grnPtr = redPtr;

                        for (gIndex = box->gMin; gIndex <= box->gMax; ++gIndex)
                                {
                                // Weight green to match max. sample bits
                                gVal = ConvertBitDepth(gPrefs.gBits, gPrefs.maxBits, gIndex);
                                argSqr = arSqr + (gVal * gVal);
                                bluePtr = grnPtr;

                                for (bIndex = box->bMin; bIndex <= box->bMax; ++bIndex)
                                        {
                                        // Weight blue to match max. sample bits
                                        bVal = ConvertBitDepth(gPrefs.bBits, gPrefs.maxBits, bIndex);
                                        weight = *bluePtr++;                            // Number of pixels of this color
                                        if (weight != 0)
                                                {
                                                // TBD: Check for overflow?
                                                box->weight += weight;                  // Total pixels in the cube
                                                box->rWeight += rVal * weight;          // RGBA values multiplied by
                                                box->gWeight += gVal * weight;          //   number of pixels
                                                box->bWeight += bVal * weight;
                                                box->aWeight += aVal * weight;
                                                box->rgbaWeight += (argSqr + (bVal * bVal)) * weight;
                                                }
                                        }
                                grnPtr += greenStride;
                                }
                        redPtr += redStride;
                        }
                alphaPtr += alphaStride;
                }
        }


//--------------------------------------------------------------------------
// Returns the total color error of all the pixels in the box passed.  This
// is the total difference of each pixel and the statistical RGB center of
// the box (taking the pixel usage into account).
//--------------------------------------------------------------------------
static __inline ULONG Variance(BOX *box)
        {
        double temp;

        // Add squares of R, G, B weighted totals, divide by weight
        temp = (double) box->rWeight * (double) box->rWeight;
        temp += (double) box->gWeight * (double) box->gWeight;
        temp += (double) box->bWeight * (double) box->bWeight;
        temp += (double) box->aWeight * (double) box->aWeight;
        temp /= (double) box->weight;

        temp = ((double) box->rgbaWeight) - temp;
        if (temp >= UINT32_MAX)
        {
            temp = UINT32_MAX-1;
        }
        return((ULONG) temp);
        }



//--------------------------------------------------------------------------
// Returns the total color error of all the pixels in the box passed.  This
// is the total difference of each pixel and the statistical RGB center of
// the box (taking the pixel usage into account).
//--------------------------------------------------------------------------
static __inline ULONG TVariance(BOX *box)
        {
        double temp;

        // Add squares of R, G, B weighted totals, divide by weight
        temp = (double) box->rWeight * (double) box->rWeight;
        temp += (double) box->gWeight * (double) box->gWeight;
        temp += (double) box->bWeight * (double) box->bWeight;
        temp += (double) box->aWeight * (double) box->aWeight;
        temp /= (double) box->weight;

        if (temp >= UINT32_MAX)
        {
            temp = UINT32_MAX-1;
        }
        return((ULONG) temp);
        }


//--------------------------------------------------------------------------
// Shrink the box passed to the smallest box still containing all the
// colors in the box.
//--------------------------------------------------------------------------
static void ShrinkBox(BOX *box)
        {
        int     aIndex, rIndex, gIndex, bIndex;
        ULONG   *alphaPtr, *redPtr, *grnPtr, *bluePtr;
        int     aMin, rMin, gMin, bMin;         // Upper and lower bounds of the shrunk box
        int     aMax, rMax, gMax, bMax;
        int     temp;

        // Initialize shrunk box to smaller-than-possible size
        aMin = box->aMax;
        rMin = box->rMax;
        gMin = box->gMax;
        bMin = box->bMax;
        aMax = box->aMin;
        rMax = box->rMin;
        gMax = box->gMin;
        bMax = box->bMin;

        // Scan through the subset of the color cube
        alphaPtr = CUBE_PTR (box->rMin, box->gMin, box->bMin, box->aMin);
        for (aIndex = box->aMin; aIndex <= box->aMax; ++aIndex)
                {
                redPtr = alphaPtr;

                for (rIndex = box->rMin; rIndex <= box->rMax; ++rIndex)
                        {
                        grnPtr = redPtr;

                        for (gIndex = box->gMin; gIndex <= box->gMax; ++gIndex)
                                {
                                bluePtr = grnPtr;

                                for (bIndex = box->bMin; bIndex <= box->bMax; ++bIndex)
                                        {
                                        if (*bluePtr++ != 0)
                                                {
                                                // Is this a new minimum for R, G, B, or A?
                                                if (rIndex < rMin)
                                                        rMin = rIndex;
                                                if (gIndex < gMin)
                                                        gMin = gIndex;
                                                if (bIndex < bMin)
                                                        bMin = bIndex;
                                                if (aIndex < aMin)
                                                        aMin = aIndex;

                                                // Is this a new maximum for R, G, B, or A?
                                                if (rIndex > rMax)
                                                        rMax = rIndex;
                                                if (gIndex > gMax)
                                                        gMax = gIndex;
                                                if (bIndex > bMax)
                                                        bMax = bIndex;
                                                if (aIndex > aMax)
                                                        aMax = aIndex;
                                                }
                                        }
                                grnPtr += greenStride;
                                }
                        redPtr += redStride;
                        }
                alphaPtr += alphaStride;
                }

        // Make sure this is a valid cube size
        if (rMin > rMax)
        {
            temp = rMin;
            rMin = rMax;
            rMax = temp;
        }
        if (gMin > gMax)
        {
            temp = gMin;
            gMin = gMax;
            gMax = temp;
        }
        if (bMin > bMax)
        {
            temp = bMin;
            bMin = bMax;
            bMax = temp;
        }
        if (aMin > aMax)
        {
            temp = aMin;
            aMin = aMax;
            aMax = temp;
        }

        // copy shrunk size to original box
        box->rMin = rMin;
        box->gMin = gMin;
        box->bMin = bMin;
        box->aMin = aMin;
        box->rMax = rMax;
        box->gMax = gMax;
        box->bMax = bMax;
        box->aMax = aMax;
        }


//--------------------------------------------------------------------------
// Split a box along either the R, G, or B plane.  Finds a split point
// which minimizes the total color error (variance) of the pixels using
// the colors in the boxes.
//--------------------------------------------------------------------------
static void SplitBox(BOX *oldBox)
        {
        int             i;
        BOX             *newBox;

        BOX             splitBox;
        BOX             lowBox;         // Box with lower value colors
        BOX             highBox;        // Box with higher value colors
        BOX             slice;          // Box describing an R, G, or B plane

        ULONG   lowVariance, varianceR, varianceG, varianceB, varianceA;
        int             pickR=0L, pickG=0L, pickB=0L, pickA=0L;

        // Get a new box structure
        newBox = nextBox++;

        // Make a copy of the box, to split it
        memcpy(&splitBox, oldBox, sizeof (BOX));

        // Initial size of low box (empty) and high box (original box).
        memset(&lowBox, 0, sizeof (BOX));
        memcpy(&highBox, &splitBox, sizeof (BOX));

        // Find optimal split point on alpha axis
        slice.rMin = splitBox.rMin;
        slice.rMax = splitBox.rMax;
        slice.gMin = splitBox.gMin;
        slice.gMax = splitBox.gMax;
        slice.bMin = splitBox.bMin;
        slice.bMax = splitBox.bMax;
        varianceA = 0xFFFFFFFF;
        for (i = splitBox.aMin; i < splitBox.aMax; i++)
                {
                ULONG   totalVariance;

                // Calculate the statistics for the area being taken away from
                //   the high box and given to the low box
                slice.aMin = i;
                slice.aMax = i;
                GetStats(&slice);
                if (slice.weight != 0)
                        {
                        // ASSERT (slice.weight <= splitBox.weight);

                        // Update statistics for low box
                        lowBox.weight += slice.weight;
                        lowBox.rgbaWeight += slice.rgbaWeight;
                        lowBox.rWeight += slice.rWeight;
                        lowBox.gWeight += slice.gWeight;
                        lowBox.bWeight += slice.bWeight;
                        lowBox.aWeight += slice.aWeight;

                        // Update statistics for high box
                        highBox.weight -= slice.weight;
                        highBox.rgbaWeight -= slice.rgbaWeight;
                        highBox.rWeight -= slice.rWeight;
                        highBox.gWeight -= slice.gWeight;
                        highBox.bWeight -= slice.bWeight;
                        highBox.aWeight -= slice.aWeight;

                        // ASSERT (lowBox.weight + highBox.weight == splitBox.weight);

                        // Calculate both boxes' combined variance
                        totalVariance = Variance(&lowBox) + Variance(&highBox);
                        if (totalVariance < varianceA)
                                {
                                varianceA = totalVariance;
                                pickA = i;
                                }
                        }
                }

        // Initial size of low box (empty) and high box (original box).
        memset(&lowBox, 0, sizeof (BOX));
        memcpy(&highBox, &splitBox, sizeof (BOX));

        // Find optimal split point on red axis
        slice.gMin = splitBox.gMin;
        slice.gMax = splitBox.gMax;
        slice.bMin = splitBox.bMin;
        slice.bMax = splitBox.bMax;
        slice.aMin = splitBox.aMin;
        slice.aMax = splitBox.aMax;
        varianceR = 0xFFFFFFFF;
        for (i = splitBox.rMin; i < splitBox.rMax; i++)
                {
                ULONG   totalVariance;

                // Calculate the statistics for the area being taken away from
                //   the high box and given to the low box
                slice.rMin = i;
                slice.rMax = i;
                GetStats(&slice);
                if (slice.weight != 0)
                        {
                        // ASSERT (slice.weight <= splitBox.weight);

                        // Update statistics for low box
                        lowBox.weight += slice.weight;
                        lowBox.rgbaWeight += slice.rgbaWeight;
                        lowBox.rWeight += slice.rWeight;
                        lowBox.gWeight += slice.gWeight;
                        lowBox.bWeight += slice.bWeight;
                        lowBox.aWeight += slice.aWeight;

                        // Update statistics for high box
                        highBox.weight -= slice.weight;
                        highBox.rgbaWeight -= slice.rgbaWeight;
                        highBox.rWeight -= slice.rWeight;
                        highBox.gWeight -= slice.gWeight;
                        highBox.bWeight -= slice.bWeight;
                        highBox.aWeight -= slice.aWeight;

                        // ASSERT (lowBox.weight + highBox.weight == splitBox.weight);

                        // Calculate both boxes' combined variance
                        totalVariance = Variance(&lowBox) + Variance(&highBox);
                        if (totalVariance < varianceR)
                                {
                                varianceR = totalVariance;
                                pickR = i;
                                }
                        }
                }

        // Initial size of low box (empty) and high box (original box).
        memset(&lowBox, 0, sizeof (BOX));
        memcpy(&highBox, &splitBox, sizeof (BOX));

        // Find optimal split point on green axis
        slice.rMin = splitBox.rMin;
        slice.rMax = splitBox.rMax;
        slice.bMin = splitBox.bMin;
        slice.bMax = splitBox.bMax;
        slice.aMin = splitBox.aMin;
        slice.aMax = splitBox.aMax;
        varianceG = 0xFFFFFFFF;
        for (i = splitBox.gMin; i < splitBox.gMax; i++)
                {
                ULONG   totalVariance;

                // Calculate the statistics for the area being taken away from
                //   the high box and given to the low box
                slice.gMin = i;
                slice.gMax = i;
                GetStats(&slice);
                if (slice.weight != 0)
                        {
                        // ASSERT (slice.weight <= splitBox.weight);

                        // Update statistics for low box
                        lowBox.weight += slice.weight;
                        lowBox.rgbaWeight += slice.rgbaWeight;
                        lowBox.rWeight += slice.rWeight;
                        lowBox.gWeight += slice.gWeight;
                        lowBox.bWeight += slice.bWeight;
                        lowBox.aWeight += slice.aWeight;

                        // Update statistics for high box
                        highBox.weight -= slice.weight;
                        highBox.rgbaWeight -= slice.rgbaWeight;
                        highBox.rWeight -= slice.rWeight;
                        highBox.gWeight -= slice.gWeight;
                        highBox.bWeight -= slice.bWeight;
                        highBox.aWeight -= slice.aWeight;

                        // ASSERT (lowBox.weight + highBox.weight == splitBox.weight);

                        // Calculate both boxes' combined variance
                        totalVariance = Variance(&lowBox) + Variance(&highBox);
                        if (totalVariance < varianceG)
                                {
                                varianceG = totalVariance;
                                pickG = i;
                                }
                        }
                }

        // Initial size of low box (empty) and high box (original box).
        memset(&lowBox, 0, sizeof (BOX));
        memcpy(&highBox, &splitBox, sizeof (BOX));

        // Find optimal split point on blue axis
        slice.rMin = splitBox.rMin;
        slice.rMax = splitBox.rMax;
        slice.gMin = splitBox.gMin;
        slice.gMax = splitBox.gMax;
        slice.aMin = splitBox.aMin;
        slice.aMax = splitBox.aMax;
        varianceB = 0xFFFFFFFF;
        for (i = splitBox.bMin; i < splitBox.bMax; i++)
                {
                ULONG   totalVariance;

                // Calculate the statistics for the area being taken away from
                //   the high box and given to the low box
                slice.bMin = i;
                slice.bMax = i;
                GetStats(&slice);
                if (slice.weight != 0)
                        {
                        // ASSERT (slice.weight <= splitBox.weight);

                        // Update statistics for low box
                        lowBox.weight += slice.weight;
                        lowBox.rgbaWeight += slice.rgbaWeight;
                        lowBox.rWeight += slice.rWeight;
                        lowBox.gWeight += slice.gWeight;
                        lowBox.bWeight += slice.bWeight;
                        lowBox.aWeight += slice.aWeight;

                        // Update statistics for high box
                        highBox.weight -= slice.weight;
                        highBox.rgbaWeight -= slice.rgbaWeight;
                        highBox.rWeight -= slice.rWeight;
                        highBox.gWeight -= slice.gWeight;
                        highBox.bWeight -= slice.bWeight;
                        highBox.aWeight -= slice.aWeight;

                        // ASSERT (lowBox.weight + highBox.weight == splitBox.weight);

                        // Calculate low box's overall variance
                        totalVariance = Variance(&lowBox) + Variance(&highBox);
                        if (totalVariance < varianceB)
                                {
                                varianceB = totalVariance;
                                pickB = i;
                                }
                        }
                }

        // Pick lowest variance from R, G, B, A splits
        lowVariance = varianceR;
        i = 0;
        if (varianceG < lowVariance)
                {
                lowVariance = varianceG;
                i = 1;
                }
        if (varianceB < lowVariance)
                {
                lowVariance = varianceB;
                i = 2;
                }
        if (varianceA < lowVariance)
                {
                lowVariance = varianceA;
                i = 3;
        }
        // ASSERT (lowVariance != 0xFFFFFFFF);             // Can't improve things?!??

        // Set up low and high box sizes
        memcpy(&lowBox, &splitBox, sizeof (BOX));
        memcpy(&highBox, &splitBox, sizeof (BOX));
        switch (i)
                {
                case 0:
                        lowBox.rMax = pickR;
                        highBox.rMin = pickR + 1;
                        break;

                case 1:
                        lowBox.gMax = pickG;
                        highBox.gMin = pickG + 1;
                        break;

                case 2:
                        lowBox.bMax = pickB;
                        highBox.bMin = pickB + 1;
                        break;

                case 3:
                        lowBox.aMax = pickA;
                        highBox.aMin = pickA + 1;
                        break;
                }

        // Reduce size of boxes to minimum that still contains all the pixels
        ShrinkBox(&lowBox);
        ShrinkBox(&highBox);

        // Update statistics
        GetStats(&lowBox);
        // ASSERT (lowBox.weight != 0);
        highBox.weight = splitBox.weight - lowBox.weight;
        highBox.rgbaWeight = splitBox.rgbaWeight - lowBox.rgbaWeight;
        highBox.rWeight = splitBox.rWeight - lowBox.rWeight;
        highBox.gWeight = splitBox.gWeight - lowBox.gWeight;
        highBox.bWeight = splitBox.bWeight - lowBox.bWeight;
        highBox.aWeight = splitBox.aWeight - lowBox.aWeight;
        // ASSERT (highBox.weight != 0);

        // Create the boxes
        memcpy(oldBox, &lowBox, sizeof (BOX));
        memcpy(newBox, &highBox, sizeof (BOX));
        oldBox->variance = Variance(oldBox);
        newBox->variance = Variance(newBox);
        }


//--------------------------------------------------------------------------
// Add the pixels in the source bitmap to the color cube.
//--------------------------------------------------------------------------
static void FillColorCube(const GINFO *ginfo, char *image, int rowbytes)
        {
        ULONG   *cubePtr;
        ULONG   aVal;
        int     red, green, blue;
        char   *pixel;
        int     i;
        const ARGB    *pixelVal;

        pixel = image;
        for (i = 0; i < ginfo->width * ginfo->height; i++)
                {

                if (ginfo->bpp <= 8)
                {
                    pixelVal = ginfo->colourtbl + *pixel;
                    pixel++;
                }
                else
                {
                    pixelVal = (ARGB *)pixel;
                    pixel+=sizeof(ARGB);
                }


                // Get scaled alpha value from 8-bit value
                aVal = ConvertDepth(256, gPrefs.alphaVals, pixelVal->a);
                // ASSERT ((int) aVal < gPrefs.alphaVals);

                // For generating FIXED_ALPHA palettes
                if ((gPrefs.crFlags & CR_FIXED_ALPHA) != 0)
                        {
                        if (pixelVal->a == 0)
                                gPrefs.anyTransparent = TRUE;
                        else if (pixelVal->a == 255)
                                gPrefs.anyOpaque = TRUE;
                        else
                                {
                                gPrefs.anyTranslucent = TRUE;

                                // Make sure quantization did make it non-translucent
                                if (aVal == 0)
                                        ++aVal;                 // Don't be transparent!
                                else if ((int) aVal == gPrefs.alphaVals - 1)
                                        --aVal;                 // Don't be opaque!
                                }
                        }

                // Don't count transparent pixels if FIXED_ALPHA or transIndex is set
                if ((pixelVal->a != 0) ||
                         (((gPrefs.crFlags & CR_FIXED_ALPHA) == 0) && (gPrefs.transIndex == -1)))
                        {
                        // Increment the pixel count for this color
                        red = ConvertBitDepth(8, gPrefs.rBits, pixelVal->r);
                        green = ConvertBitDepth(8, gPrefs.gBits, pixelVal->g);
                        blue = ConvertBitDepth(8, gPrefs.bBits, pixelVal->b);

                        cubePtr = CUBE_PTR (red, green, blue, aVal);
                        ++*cubePtr;
                        }
                else
                        ((ARGB *)pixelVal)->a = 255;     // note kludge - should be const
                }
        }


//--------------------------------------------------------------------------
// Get a palette from the entries in the color cube.
//--------------------------------------------------------------------------
static int MakePalette(ULONG *palette)
        {
        ULONG   *palPtr, variance, mostVariance;
        ULONG   pixelVal, red, green, blue, alpha, highAlpha;
        BOX             *box, *worstBox, *newBox;
        int             i, j, boxes, colors, lowIndex, transIndex = -1;
        ULONG   *luminTbl, *luminPtr, lowVal, temp;
        BOOL    needsTransparent;

        // Do we need a transparent color?
        if ((gPrefs.transIndex != -1) ||
                (((gPrefs.crFlags & CR_FIXED_ALPHA) != 0) &&
                 (gPrefs.anyTransparent && (gPrefs.anyOpaque || gPrefs.anyTranslucent))))
        {
                needsTransparent = TRUE;
        }
        else
        {
                needsTransparent = FALSE;
        }

        // Set up list of box structures
        colors = gPrefs.colors;
        boxList = (BOX *) galloc(sizeof (BOX) * (colors + 1));
        if (!boxList) return(0);

        // Clear palette
        memset(palette, 0, gPrefs.colors * sizeof (ULONG));

        // If only transparent pixels, use just one color
        if (((gPrefs.crFlags & CR_FIXED_ALPHA) != 0) && !gPrefs.anyTranslucent &&
                !gPrefs.anyOpaque)
                {
                *palette = 0 | PUT_ALPHA (255);         // Transparent black
                return (1);                            // One color in palette
                }

        // Set up first box (entire color cube)
        nextBox = boxList + 1;
        boxList->rMin = 0;
        boxList->gMin = 0;
        boxList->bMin = 0;
        boxList->aMin = 0;
        boxList->rMax = (1 << gPrefs.rBits) - 1;                // RGB_VALS - 1;
        boxList->gMax = (1 << gPrefs.gBits) - 1;                // RGB_VALS - 1;
        boxList->bMax = (1 << gPrefs.bBits) - 1;                // RGB_VALS - 1;
        boxList->aMax = gPrefs.alphaVals - 1;                   // ALPHA_VALS - 1;
        ShrinkBox(boxList);
        GetStats(boxList);
        boxList->variance = Variance(boxList);

        // Starts with one box (the whole color cube)
        boxes = 1;

        // Asked to match transparent, translucent, & opaque exactly?
        if ((gPrefs.crFlags & CR_FIXED_ALPHA) != 0)
                {
                // If any semi-transparent colors, split cube on alpha plane so both
                //   alpha levels can be matched exactly.
                if ((colors > boxes) && gPrefs.anyTranslucent && gPrefs.anyOpaque &&
                        (gPrefs.alphaVals > 2))
                        {
                        // Get a new box structure
                        newBox = nextBox++;
                        ++boxes;

                        // Make a copy of the box, to split it
                        memcpy(newBox, boxList, sizeof (BOX));

                        // Slice on alpha == 1 plane
                        boxList->aMax = 0;
                        newBox->aMin = 1;

                        // Reduce size of boxes to minimum that still contains all the pixels
                        ShrinkBox(boxList);
                        ShrinkBox(newBox);

                        GetStats(boxList);
                        GetStats(newBox);
                        boxList->variance = Variance(boxList);
                        newBox->variance = Variance(newBox);
                        }
                }

        // Split boxes until correct number of boxes are created
        while (boxes < colors)
                {
                // Find box with the highest variance
                mostVariance = boxList->variance;
                if (boxList->aMax == 0)         // opacity weighting?
                        mostVariance = (ULONG) (gPrefs.opaqueWeight * mostVariance);
                worstBox = boxList;

                // See if one of the rest of the boxes is worse than the first
                for (box = boxList+1; box < nextBox; ++box)
                        {
                        variance = box->variance;
                        if (box->aMax == 0)             // opacity weighting?
                                variance = (ULONG) (gPrefs.opaqueWeight * variance);

                        if (variance > mostVariance)
                                {
                                worstBox = box;
                                mostVariance = variance;
                                }
                        }

                // No improvement needed!  (must be one box per color used)
                if (mostVariance == 0)
                        break;

                // Split the lamest box into two cooler boxes
                SplitBox(worstBox);
                ++boxes;
                }

        // Get palette from box statistics
        colors = boxes;
        palPtr = palette;

        for (box = boxList; box < nextBox; ++box)
                {
                // Get 8-bit RGBA values from stats
                if(box->weight)   // just in case we could only get less than 256 boxes.
                {
                    red = ConvertBitDepthMulti(gPrefs.maxBits, 8, box->rWeight, box->weight);
                    // ASSERT (red < 256);
                    green = ConvertBitDepthMulti(gPrefs.maxBits, 8, box->gWeight, box->weight);
                    // ASSERT (green < 256);
                    blue = ConvertBitDepthMulti(gPrefs.maxBits, 8, box->bWeight, box->weight);
                    // ASSERT (blue < 256);
                    alpha = ConvertDepthMulti(1 << (gPrefs.maxBits + 1), 256, box->aWeight, box->weight);
                    // ASSERT (alpha < 256);
                }
                else
                {
                    red = 0;
                    green = 0;
                    blue = 0;
                    alpha = 0;
                }
                // if (alpha == 255)
                if (alpha == 0)
                        {
                        if (transIndex != -1)           // Only one transparent color allowed
                                {
                                --colors;
                                continue;
                                }
                        else
                                {
                                transIndex = palPtr - palette;
                                }
                        }

                *palPtr++ = PUT_RED (red) | PUT_GREEN (green) | PUT_BLUE (blue) | PUT_ALPHA (alpha);
                }
        // ASSERT (palPtr == palette + colors);
        gfree(boxList);

        // Add transparency when matching alpha exactly, if needed
        if (needsTransparent && (transIndex == -1))
                {
                highAlpha = GET_ALPHA(palette[0]);
                transIndex = 0;

                // Replace highest alpha with transparent color
                for (i = 1; i < colors; ++i)
                        {
                        alpha = GET_ALPHA (palette[i]);
                        if (alpha > highAlpha)
                                {
                                highAlpha = alpha;
                                transIndex = i;
                                }
                        }

                // Set to full transparent
                palette[transIndex] = palette[transIndex] | PUT_ALPHA (255);
                }

        // Move transparent color to a particular index value?
        if (gPrefs.transIndex != -1)
                {
                // Must be a transparent color in the palette
                // ASSERT (transIndex != -1);

                // What color to swap the transparent color with
                if (gPrefs.transIndex >= colors)
                        {
                        pixelVal = palette[colors - 1];
                        palette[colors - 1] = 0;
                        }
                else
                        pixelVal = palette[gPrefs.transIndex];

                // Swap them
                palette[gPrefs.transIndex] = palette[transIndex];
                palette[transIndex] = pixelVal;

                // Check if swapped out of color range
                if (gPrefs.transIndex >= colors)
                        {
                        --colors;

                        // Palette must be padded
                        // ASSERT ((gPrefs.colors >= gPrefs.transIndex) &&
                        //        ((gPrefs.crFlags & CR_PAD_PAL) != 0));
                        }
                }

        // Sort palette if asked
        if ((gPrefs.crFlags & CR_SORT_LUMIN) != 0)
                {

                // Make table of luminances
                luminTbl = (ULONG *) galloc(colors * sizeof (double));
                luminPtr = luminTbl;
                palPtr = palette;
                for (i = 0; i < colors; ++i)
                        {
                        *luminPtr++ = (GET_RED (*palPtr) * 30) +
                                (GET_GREEN (*palPtr) * 59) +
                                (GET_BLUE (*palPtr) * 12);
                        ++palPtr;
                        }

                // Sort the colors
                for (i = 0; i < colors - 1; ++i)
                        {
                        if (i != gPrefs.transIndex)
                                {
                                // Find lowest luminance past current color
                                lowVal = luminTbl[i];
                                lowIndex = i;
                                for (j = i + 1; j < colors; ++j)
                                        {
                                        if ((luminTbl[j] < lowVal) && (j != gPrefs.transIndex))
                                                {
                                                lowVal = luminTbl[j];
                                                lowIndex = j;
                                                }
                                        }

                                // If there's a lower luminance color after this one...
                                if (lowIndex > i)
                                        {
                                        // Swap lowest luminance value with current color
                                        temp = luminTbl[i];
                                        luminTbl[i] = lowVal;
                                        luminTbl[lowIndex] = temp;

                                        // Swap colors in palette
                                        pixelVal = palette[i];
                                        palette[i] = palette[lowIndex];
                                        palette[lowIndex] = pixelVal;
                                        }
                                }
                        }

                gfree(luminTbl);
                }

        return (colors);
        }


/****************************************************************/
/* END inclusion of varymin.c from recolor                    */
/****************************************************************/


/****************************************************************/
/* Internal Functions                                           */
/****************************************************************/


/*
 * Convert from recolor's palette format to crapi palette format
 */
static ARGB *ALPHAConvertToCrapi(ULONG *source, int numcolours)
{
    int i;
    ARGB *dptr;

    dptr = (ARGB *)galloc(numcolours * sizeof(ARGB));
    if (dptr)
    {
        for (i=0;i<numcolours;i++)
        {
            dptr[i].r = GET_RED(source[i]);
            dptr[i].g = GET_GREEN(source[i]);
            dptr[i].b = GET_BLUE(source[i]);
            dptr[i].a = GET_ALPHA(source[i]);
            if (dptr[i].a<=2)
                dptr[i].a = 0;
            if (dptr[i].a>=(128-1) && dptr[i].a<=(128+1))
                dptr[i].a = 128;
            if (dptr[i].a>=254)
                dptr[i].a = 255;
        }
        return(dptr);
    }
    return((ARGB *) 0);
}


/****************************************************************/
/* CRapi Functions                                              */
/****************************************************************/

CRABOUT *GCALL ALPHA_about(void)
{
    CRABOUT *about = (CRABOUT *)galloc(sizeof(CRABOUT));
    if (about)
    {
        memset(about, 0, sizeof(CRABOUT));

        about->signature =          GMAKEID('A','L','P','H');
        about->size =               sizeof(CRABOUT);
        about->version =            CRAPI_VERSION;
        about->respectalpha =       1;
        strcpy(about->authorstr,    "K.Okuda,J.Lefferts");
        strcpy(about->versionstr,   "1.11");
        strcpy(about->namestr,      "4D Wu");
        strcpy(about->longnamestr,  "4D Wu with alpha");
    }

    return(about);
}

CRINSTANCE *GCALL ALPHA_open(void)
{
    CRINSTANCE *crx;

    crx = (CRINSTANCE *)galloc(sizeof(CRINSTANCE));
    if (crx)
    {
        memset(crx, 0, sizeof(CRINSTANCE));
        crx->signature  = GMAKEID('A','L','P','H');
        crx->size       = sizeof(CRINSTANCE);
        crx->cref      = NULL;

        memset(&gPrefs,0,sizeof(gPrefs));
    }
    return(crx);
}

int GCALL ALPHA_read(CRINSTANCE *crx, const GINFO *ginfo, const void *source, int rowbytes)
{
    int ok           = 1;
    bool isnullchannel = (ginfo->alphabits == 0 ||
                          ginfo->redbits   == 0 ||
                          ginfo->greenbits == 0 ||
                          ginfo->bluebits  == 0);
    int abits        = 6;
    int rbits        = 5;
    int gbits        = 6;
    int bbits        = 5;

    if (!gPrefs.palBits)
    {
      // if any channel is zero 4dwu goes nuts.
      // for now use max channel depth to deal with
      // this.
      if(!isnullchannel)
      {
        abits = gmax(1,gmin(abits,ginfo->alphabits));
        rbits = gmax(1,gmin(rbits,ginfo->redbits));
        gbits = gmax(1,gmin(gbits,ginfo->greenbits));
        bbits = gmax(1,gmin(bbits,ginfo->bluebits));
      }

        gPrefs.palBits = 32;            // Palette bits 16 24 32
        gPrefs.crFlags = 0;             // color reduction flags: see recolor.h for CR_* flags
        gPrefs.dither = DITHER_NONE;
        gPrefs.palFirst = 0;            // First palette entry number
        gPrefs.alphaVals = 1<<abits;    // Default if not specified
        gPrefs.src32Bit = TRUE;         // The source has an alpha channel
        gPrefs.rBits = rbits;           // Default: sample at 555
        gPrefs.gBits = gbits;
        gPrefs.bBits = bbits;
        gPrefs.xPos = 0;
        gPrefs.newXPos = -1;            // Default (don't move image)
        gPrefs.yPos = 0;
        gPrefs.newYPos = -1;            // Default (don't move image)
        gPrefs.xPosPal = 0;
        gPrefs.yPosPal = 0;
        gPrefs.trueColor = FALSE;
        gPrefs.transColor = 0;
        gPrefs.transIndex = -1;         // Don't care where transparent color is in palette
        gPrefs.opaqueWeight = 1.0;
        gPrefs.palName = NULL;
        gPrefs.extraBit = EXTRA_A;
        gPrefs.srcPaletted = FALSE;
        gVerbose = FALSE;
        if (!InitColorCube(&gPrefs))
            ok = 0;
    }
    if (ok)
        FillColorCube(ginfo, (char *)source, rowbytes);

    return(ok);
}

ARGB *GCALL ALPHA_close(CRINSTANCE *crx, int numcolours, CRINVERSETABLE **inversetable, int *returncolours)
{
    int newColours=numcolours;
    ARGB *palette=0;
    ULONG *crapipalette;

    gPrefs.colors = numcolours;
    if (crx)
    {
        // Allocate space for the palette
        crapipalette = (ULONG *) galloc(numcolours * sizeof (ULONG));
        if (crapipalette)
        {
            memset(crapipalette,0,numcolours * sizeof (ULONG));
            newColours = MakePalette(crapipalette);
            palette = ALPHAConvertToCrapi(crapipalette, numcolours);
            gfree(crapipalette);
        }
        gfree(crx);
    }

    FreeColorCube();

    if (returncolours)
        *returncolours = newColours;

    return(palette);
}

