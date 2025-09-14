// (c) 1997-2007 Electronic Arts Inc.

#ifndef __S3WRITE
#define __S3WRITE   1

#if defined(_MSC_VER)
#pragma warning(push,1)
#endif

#include <stdlib.h>
#include <math.h>

#if defined(_MSC_VER)
#pragma warning(pop)
#pragma warning(disable: 4514)
#endif

#include "codex.h"
#include "s3tccodex.h"
#include "string.h"

/************************************************************/
/* Code taken from S3_quant.h, S3's quantizer header        */
/************************************************************/

/*
 *   Copyright (c) 1997-8  S3 Inc.  All Rights Reserved.
 *
 *   Module Name:  s3_quant.h
 *
 *   Purpose:   Headr file for <quantizer.c> -
 *                S3 linear interpolation encoding and decoding
 *              mechanism for texture map compression.
 *
 *   Author:   Konstantine Iourcha, Dan Hung
 *
 *   Revision History:
 *    version Beta 1.00.00-98-03-26
 */
#ifndef _S3_QUANT_H_

#define _S3_QUANT_H_


#define    MAX_PIXEL_PER_BLOCK        16

    /* RGB block structure, contains array of texels with colors in RGB888
       format as well as variables for coded results                */

typedef struct _RGBBlock {
    /* INPUTS */
    int n;            /* number of texels for  this block        */
    double colorChannel[MAX_PIXEL_PER_BLOCK][3];
                /* array of RGB channel values
                   assumed to be  normalized to [0.,1.]        */
    double weight[3];        /* quantization weights on each channel        */
    int inLevel;        /* quantization level (type)
                   3 - quantized to line with two endpoints
                       and single interpolant inbetween
                       plus trancparency index if needed;
                   4 - quantized to line with two endpoints
                       and two interpolants inbetween        */

    /* OUTPUTS */
    int outLevel;        /* actual number of quantized points        */
    int endPoint[2][3];        /* quantized end points in RGB565 format,
                   unordered                    */
    int index[MAX_PIXEL_PER_BLOCK];
                /* texel indices to quantized points        */
} RGBBlock;

    /* Alpha block structure, contains array of texels with alpha in
       8-bit format plus various variables for coded results            */
typedef struct _AlphaBlock {
    /* INPUTS */
    int n;            /* number of alpha values for this block    */
    float alpha[MAX_PIXEL_PER_BLOCK];
                /* array of alpha normalized to [0.,1.]        */
    int need0;        /* !=0 if exact 0.0 is needed in compressed
                   format ==0 otherwise                */
    int need1;        /* !=0 if exact 1.0 is needed in compressed
                    format==0 otherwise                */

    /* OUTPUTS */
    int outLevel;        /* actual number of quantized points        */
    int endPoint[2];    /* quantized end points in 8-bit alpha,
                   unordered                    */
    int index[MAX_PIXEL_PER_BLOCK];
                /* alpha texel indices to quantized
                   points                    */
} AlphaBlock;

    /* for details on the mathematical basis of this, see easys3tc.doc
       "S3TC made easy"                            */

    /* function which encodes a block                    */
static void CodeRGBBlock(RGBBlock *b);

static void CodeAlphaBlock(AlphaBlock *b);

#endif

/************************************************************/
/* Code taken from S3_quant.c, S3's quantizer header        */
/* Removed assertions.                                      */
/************************************************************/

/*****************************************************************************
**   Copyright (c) 1997-8  S3 Inc.  All Rights Reserved.            **
**                                        **
**   Module Name:  s3_quant.c                            **
**                                        **
**   Purpose:   S3 texture map compression quantizers for RGB            **
**        and alpha.                            **
**                                        **
**        The quantizers computes suboptimal solution of the         **
**        corresponsing                            **
**        mean square error minimisation (MMSE) problem.            **
**        (for details on the mathematical basis of this, see         **
**        easys3tc.doc "S3TC made easy"                    **
**                                        **
**                                        **
**   Author:   Konstantine Iourcha, Dan Hung                    **
**                                        **
**   Revision History:                                **
**    version Beta 1.00.00-98-03-26                        **
**                                        **
**                                        **
******************************************************************************/

/*****************************************************************************
**                                        **
**    INCLUDE FILES                                **
**                                        **
*****************************************************************************/

#include <stdlib.h>
#include <math.h>
//#include <assert.h>
//#include "S3_quant.h"

/*****************************************************************************
**                                        **
**    MACROS AND MAGIC NUMBERS                        **
**                                        **
*****************************************************************************/

    /* IMPORTANT PERFORMACE TUNNING PARAMETERS

       <STATIC_ARR> can be defined as <static> or as < > (blank)
       In the former case some <double> arrays will be defined as <static>,
       otherwise they will be automatic.

       <STATIC>  has the similar effect for some additional <double> variables

    The need to use it arises from the fact, that the MS VC++ 4.2 does not
    do stack frame alignment (or at least we were not been able to
    configure it to do stack frame aligments)

    The new MS VC++ 5.00 (with or without service pack 3) does do stack '
    aligment, but the FPU schedule / optimizer does not do a good job at
    scheduling FPU (or at least we were not been able to configure it to do
    good scheduling).

    As a result, in some cases we observed the code from v.4.2 with
    misaligned data running faster then the samed code compiled with
    v.5.00 (with good aligment).

    The misaligned data in this code, can cause up to 2 x (!) performace
    degradation.

    Most of known "workarounds" for stack frame alignmet are either
    non-portable, messy or complex (or all of them).

    As we bilieve the problem should be solved in the compiler (which
    actually is very easy), we use the simplest possible approach.

    Note, that the problem is Intel specific, and on most of "normal"
    machines defining <double> arrays as static vs. automatic will
    decrease performace because automatic provides better cache coherency,
    especially with direct-mapped write-no-allocate caches.

    (On DEC Alpha NT platform the perfomace drop is about 20 -25% using
    static)

    SO, THE RULES for the parametes are:
        - if you compile code for Intel machine using compiler which
          does NOT do stack frame alignment, set both parameter to <static>

        - otherwise, set them to <> (blank)

        - better yet, try both (or three combinations) and do performance
          profiling

        - when <STATIC> defined as <static>, there are no reason to have
          <STATIC_ARR> as <> (blank). The opposite combination might
          work in some cases
                                            */
/*
#define STATIC_ARR
#define STATIC
*/

#define STATIC_ARR static
#define STATIC static



    /*    The ALL_SAME_THRESHOLD defines a cube in RGB space with the edges
    aligned to the coordinate axis and having length of twice the value
    of ALL_SAME_THRESHOLD. If for some cube position (the center of the
    cube is in the first point color) all point of the block are inside,
    then the block collapses to a single color cluster and goes into
    <allSame bypass>. For 256 levels grid, the value should be
    1/256/2/2 or less (1/256/4/2).                        */

#define ALL_SAME_THRESHOLD 1/256./4./2.

    /*  Muximum number of <seach43Mult> calls (iterations) for one block.
        Note, that on average that will be just between 1 and 2 calls
    (about 1.2) made per block, as the iterator stops anyhow when it
    cannot get further improvement.

    However on some blocks, the iterator can run into very int infinite
    loop without forced exit.

    On rare ocasions for some blocks the number of iterations might
    be higher, so the the reasonable number should not be too small
    as it will results in poorer quanlity on some blocks and vitually no
    performance gain.

    The following numbers were define using some statistical profiling
    on the large set of images and believed to be optimal            */

#define UNCLIPPED_ITERATION_LIMIT 16

    /*    Maximum number of <seachClipped43Mult> calls (iterations) for one
    block, similar to UNCLIPPED_ITERATION_LIMIT, see above            */

#define CLIPPED_ITERATION_LIMIT 8

    /* During <seach43Mult> / <seachClipped43Mult> iterative calls
       each configuration is defined by the ordering of the points, which
       is described by the index.

       Index log keeps some number of indicies already checked, to provide
       loop detection and exit.

       Unless UNCLIPPED_ITERATION_LIMIT and CLIPPED_ITERATION_LIMIT are very
       big (100 and up), the following parameter should be equal to the
       biggest of those two values                        */

#define INDEX_LOG_SIZE 16

    /* maximum number of levels (values in the linear ramp) for alpha
       quantizer                                */

#define MAX_ALPHA_LEVELS    8

    /* Maximum number of iteration for incremental optimizer of alpha
       quatization.

       As alpha quantizer has completely different nature then RGB quantizer
       this number should be much bigger then the iteration limits for RGB
       quantizer. (But the iterations are much faster too).

       The next number was defined based on statistical profiling        */

#define ALPHA_ITERATION_LIMIT    64


    /* Macro to clip <X> to the range [X_MIN, X_MAX] for <float> or <double>*/
#define CLIP(X, X_MIN, X_MAX)                            \
    (((fabs(X_MIN - X) + X_MIN ) + (X_MAX - fabs(X_MAX - X))) * 0.5 )



/*****************************************************************************
**                                        **
**    FUNCTION PROTOTYPES                            **
**                                        **
*****************************************************************************/

static void getAxis(int n,double (*compresseddata)[3],double axis[3]);

static void getDiameter(int n, double (*compresseddata)[3],double axis[3]);

static void sortProjection(int n, double (*compresseddata)[3], double axis[3],
            int index [MAX_PIXEL_PER_BLOCK], int reverse);

inline static int sameOrder (int n,
  int index1[MAX_PIXEL_PER_BLOCK], int index2[MAX_PIXEL_PER_BLOCK]);

static double search43Mult (int *levLim, int n, double (*compresseddata)[3], double *pMult,
  int idx [MAX_PIXEL_PER_BLOCK], double endPointOut[2][3],
  double axis[3] );

static double searchClipped43Mult (int *levLim, int n, double (*compresseddata)[3], double *pMult,
  int idx [MAX_PIXEL_PER_BLOCK], double range[2][3],double endPointOut[2][3],
  double axis[3] );

static double roundMult (int nColors, int n, double (*compresseddata)[3], double *pMult, double w[3],
  double endPointIn[2][3], int endPointOut[2][3],
  int index [MAX_PIXEL_PER_BLOCK]);

static double allSame(int *nColors, int n, double (*compresseddata)[3], double weight[3],
  int endPointOut[2][3], int index [MAX_PIXEL_PER_BLOCK]);


static double mapAndRoundMult(int *nColors, int levelLimit, int n,  double (*compresseddata)[3],
    double *pMult, double weight[3], double endPointIn[2][3], int endPointOut[2][3],
    int index [MAX_PIXEL_PER_BLOCK]);

static double quantizeAlpha (int level, int n,
  double alpha[MAX_PIXEL_PER_BLOCK], int endPointOut[2],
  int  index [MAX_PIXEL_PER_BLOCK]);



/*****************************************************************************
**                                        **
**    RGB QUANTIZER "FRONT END FUNCTIONS" - geting optimal line        **
**      (direction), ordering, etc.                        **
**                                        **
*****************************************************************************/


/*
**
**  statci void getAxis(int n,double (*compresseddata)[3],double axis[3])
**
**    Derive a direction for the best-fit line, based on 3D optimization.
**
**    The direction is an eigenvector, corresponding to the biggest
**    eigenvalue of the covariance matrix of color points.
**    (The same as an eigenvector, corresponding to the smallest
**    eigenvalue of the tensor of inertia of color points.)
**
**    Note, that if the biggest eigenvalue and the next one are close, the
**    function will give relatively poor approximation of the eigenvector.
**    This is not very important, though, as some other error bounds (which
**    actually affect the resulting quantization error) will be satisfied.
**
**    Also, if the two (three) biggest eigenvectors are the same, the result
**    will be any vector from the appropriate plane (space)
**
**  Input:
**
**    n    - number of points (per block)
**    compresseddata    - array of points coordinates (RGB), weighted,
**          should be centered (!)
**
**  Output
**    axis    - resulting eigenvector approximation (best fit axis)
**          (actully it will be very poor approximation of the
**          eigenvector in some cases, this is fine for our purposes,
**          see above)
**
*/

static void getAxis(int n,double (*compresseddata)[3],double axis[3]) {
    STATIC_ARR double s[3][3];        /* self-conjugated covariance matrix of compresseddata[i]
                   and its power of 2^n                */
    STATIC_ARR double t[3][3];        /* second copy of covariance matrix s[i]
                   for static unroll to avoid data dependency
                                        */
    STATIC double sp,f;        /* factors used to scale s            */

    int c,i,j;            /* counter variables                */

    /* initialize upper triangle of s                        */

    s[0][0] = s[0][1] = s[0][2] = s[1][1] = s[1][2] = s[2][2] = 0.0;

    /* calculate upper triangle of s = sum of self-conjugated matrix over all
       compresseddata[i]'s = sum(compresseddata[i]xT(compresseddata[i])); where T() = transpose function        */
    for (i=0;i<n;i++) {
    s[0][0] += compresseddata[i][0] * compresseddata[i][0];
    s[0][1] += compresseddata[i][0] * compresseddata[i][1];
    s[0][2] += compresseddata[i][0] * compresseddata[i][2];
    s[1][1] += compresseddata[i][1] * compresseddata[i][1];
    s[1][2] += compresseddata[i][1] * compresseddata[i][2];
    s[2][2] += compresseddata[i][2] * compresseddata[i][2];
    }

    /* start repetitive multiply to solve for optimal vector            */
    /* OUTER LOOP START                             */

    for (c = 0; c < 3; c++) {
    /* the number of iteration of this (and inner loop), 3 and 4 are
       defined as to reach required error bound                 */

    /*  scale matrix s to bound coefficients to prevent underflow/overflow,

        let sp = s[0][0] + s[1][1] + s[2][2]

    as s is positive defined, for  its biggest eigenvlaue lM

        max(s[][])   <= lM <=  sp;

    On the other hand, lM * 3 >= sp and lM >= sp / 3
    So,  1 / 3 <= lM / sp  <= 1 and for the scaler of 3.5 * sp
        1 < = 3.5 / 3 <= lM * (3.5 / sp) <= 3.5
    and
        max(s[][]) * (3.5 / sp) <= 3.5,
    Hence the biggest eigenvalue will not degenerate during power
    computation and we have and upper bound of 3.5 ^ n for coefficients
    of  s^n.                                */

    sp = s[0][0] + s[1][1] + s[2][2];
    f = 3.5 / sp;
    for (i = 0; i < 3; i++)
        for (j = i; j < 3; j++)
        s[i][j] *= f;

    /* derive the optimal vector via repetitive multiply
       4 iterations can be sustaned without overflow since initial eigenvalue
       has an upperbound of 4.0 (actually 3.5), i.e. initial exponent of 2.

       Maximum exponent for double is 1024 and each iteration the exponent is
       increased fourfold, hence  1024 = 2^10 > 2 * 4^4 (4 iterations
       wothout exponent overflow).                        */

    for (j = 0; j < 4; j++) {
    /* t = s*s; set up the matrix multiply in such a way as to avoid data
       dependencies via the use of t (compiles to faster code),
       compute the upper triangle only                        */
        t[0][0] = s[0][0] * s[0][0] + s[0][1] * s[0][1] + s[0][2] * s[0][2];
        t[0][1] = s[0][0] * s[0][1] + s[0][1] * s[1][1] + s[0][2] * s[1][2];
        t[0][2] = s[0][0] * s[0][2] + s[0][1] * s[1][2] + s[0][2] * s[2][2];
        t[1][1] = s[0][1] * s[0][1] + s[1][1] * s[1][1] + s[1][2] * s[1][2];
        t[1][2] = s[0][1] * s[0][2] + s[1][1] * s[1][2] + s[1][2] * s[2][2];
        t[2][2] = s[0][2] * s[0][2] + s[1][2] * s[1][2] + s[2][2] * s[2][2];

        s[0][0] = t[0][0] * t[0][0] + t[0][1] * t[0][1] + t[0][2] * t[0][2];
        s[0][1] = t[0][0] * t[0][1] + t[0][1] * t[1][1] + t[0][2] * t[1][2];
        s[0][2] = t[0][0] * t[0][2] + t[0][1] * t[1][2] + t[0][2] * t[2][2];
        s[1][1] = t[0][1] * t[0][1] + t[1][1] * t[1][1] + t[1][2] * t[1][2];
        s[1][2] = t[0][1] * t[0][2] + t[1][1] * t[1][2] + t[1][2] * t[2][2];
        s[2][2] = t[0][2] * t[0][2] + t[1][2] * t[1][2] + t[2][2] * t[2][2];
    }
    }
    /* OUTER LOOP END                                */

    /* find maximum diagonal coefficient in s, this square of the norm of
       corresponding column (row) of t                        */
    i = s[0][0] > s[1][1] ?
    (s[0][0] > s[2][2] ? 0 : 2) : (s[1][1] > s[2][2] ? 1 : 2);

    /* normalize and return axis                        */
    f = 1.0 / sqrt(s[i][i]);

    for (j = 0; j < i; j++)
    axis[j] = t[j][i] * f;
    for (     ; j < 3; j++)
    axis[j] = t[i][j] * f;
}

/*
**
**  static void getDiameter(int n, double (*compresseddata)[3],double axis[3])
**
**    Derive a direction for the best-fit line, based on 3D optimization.
**
**    getDiameter returns the diameter of the set of 3D point compresseddata[],
**    which represent the vector between two compresseddata points that has the
**    maximum distance over all such vectors among the points compresseddata[].
**
**  Input:
**
**    n    - number of points (per block)
**    compresseddata    - array of points coorsinates (RGB), weigthed,
**          might of might not be centered
**
**
**  Output
**    axis    - resulting diameter approximation (best fit axis)
**
*/
static void getDiameter(int n, double (*compresseddata)[3],double axis[3]) {
    STATIC double dia = 0;
    STATIC double tmpDia = 0;
    int diaInd0 = 0;
    int diaInd1 = 0;
    int i= 0;
    int j= 0;
    /* exhaustive search for diameter of the points / texels.
       diameter here is defined as largest distance between two points (texels).
       (actually, in code below the variable dia is diameter squared until the
       end)                                    */
    for (dia =0.0, i = 0; i < n; i++) {
    for (j = i; j < n; j++) {
    /* calculate vector between texels compresseddata[i] and compresseddata[j]                */
        double c0,c1,c2;
        c0 = compresseddata[i][0] - compresseddata[j][0];
        c1 = compresseddata[i][1] - compresseddata[j][1];
        c2 = compresseddata[i][2] - compresseddata[j][2];
    /* square distance between two texels compresseddata[i] and compresseddata[j]                */
        tmpDia = c0*c0 + c1*c1 + c2*c2;
    /* find largest square distance and corresponding vector            */
        if (tmpDia>dia) {
        dia = tmpDia;
        diaInd0 = i;
        diaInd1 = j;
        }
    }
    }

    /* calculate largest distance and return normalized diameter vector        */
    dia = 1.0 / sqrt(dia);
    axis[0] = (compresseddata[diaInd0][0] - compresseddata[diaInd1][0]) * dia;
    axis[1] = (compresseddata[diaInd0][1] - compresseddata[diaInd1][1]) * dia;
    axis[2] = (compresseddata[diaInd0][2] - compresseddata[diaInd1][2]) * dia;
}


/*
**
**  static void sortProjection(int n, double (*compresseddata)[3], double axis[3],
**            int index [MAX_PIXEL_PER_BLOCK], int reverse)
**
**    Sort the projection of 3D points compresseddata[] onto vector axis[].
**      The sort is stable (this is importatnt for its intended usaga),
**    i.e. the points with equal projections should have the same order
**    in the output as they have in the input. (Or reversing stable
**    if reverse set to 1).
**
**
**  Input:
**
**    n    - number of points (per block)
**    compresseddata    - array of points coorsinates (RGB), weigthed,
**          might of might not be centered
**    axis    - vector to projec on.
**    reverse - if 0 the elements with equal projections shold have
**          the same order in the output array as they have in the
**          input; if 1 they should have exactly opposite order.
**
**  Output
**    index    - index defining the order of projections; the original
**          point array has  unchanged order
**
*/
static void sortProjection(int n, double (*compresseddata)[3], double axis[3],
            int index [MAX_PIXEL_PER_BLOCK], int reverse) {

    STATIC_ARR double projection [MAX_PIXEL_PER_BLOCK];
    int mask [MAX_PIXEL_PER_BLOCK];
    int i, j, k;

    /* compute projections and intialize index                    */
    for (i = 0; i < n; i++) {
    projection[i] =
        compresseddata[i][0] * axis[0] + compresseddata[i][1] * axis[1] + compresseddata[i][2] * axis[2];
    mask[i] = 1;
    }

    /* sort, revers controls order of equal elements                */
    if (reverse) {
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++)
        if (mask[j])
            break;
        for (k = j++; j < n; j++)  {
        if (mask[j] && projection[j] <= projection[k])
            k = j;
        }
        mask[k] = 0;
        index[i] = k;
    }
    }
    else {
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++)
        if (mask[j])
            break;
        for (k = j++; j < n; j++)  {
        if (mask[j] && projection[j] < projection[k])
            k = j;
        }
        mask[k] = 0;
        index[i] = k;
    }
    }
}

/*
**
**  static int sameOrder (int n,
**    int index1[MAX_PIXEL_PER_BLOCK], int index2[MAX_PIXEL_PER_BLOCK])
**
**    Check if the two indicies has the same or exactly reversd order
**
**  Input:
**
**    n    - number of points (per block)
**    index1    - first index to compare
**    index2    - second index to compare
**
**  Output:    none, all input values are unchanged
**
**  Return value: 1 if the indecies have the same or reveresed orded,
**          0 otherwise
**
*/

inline static int sameOrder (int n,
  int index1[MAX_PIXEL_PER_BLOCK], int index2[MAX_PIXEL_PER_BLOCK]) {

    int i;
    /*  check if the same order                            */
    for (i = 0; i < n; i++)
    if (index1[i] != index2[i])
        break;
    /* if not, check if reverse order                        */
    if (i < n)
    for (i = 0; i < n; i++)
        if (index1[i] != index2[n-1-i])
        break;
    return(i == n);
}


/*****************************************************************************
**                                        **
**    RGB QUANTIZER COLOR RAMP FITTING FUNCTIONS                **
**      (in "continuous" <double> based 3D  model                **
**                                        **
*****************************************************************************/


/*
**
**  static double search43Mult (int *levLim, int n, double (*compresseddata)[3],
**      double *pMult, int idx [MAX_PIXEL_PER_BLOCK], double endPointOut[2][3],
**    double axis[3] )
**
**    Given set of 3D points compresseddata[], and ordering in idx[], go through
**      different clustering such that each cluster consist of consecutive
**    points as defined by the order in idx[].
**    For each clustering compute color ramp with minimal square error
**      according to weights.  then return color ramp w/ minimal error among
**    all possible ramp over all possible clustering.
**
**    Note that the endpoints on the color ramp may be outside color cube.
**
**  Input:
**    int *levLim    - maximum allowed number of the points in color ramp
**              minus one.  (possible value 2 or 3)
**    int n        - number of points
**    double (*compresseddata)[3]    - 3D points (should be centered, weighted, and
**              multiplied by multiplicity factor pMult[].)
**      double *pMult    - multiplicity factor (sum of all factors should be
**              1.0)
**    int idx []    - order of 3D points to use for clustering
**
**  Output:
**    int *levLim    - actual number of points in color ramp minus one.
**              (might be 1, 2, or 3).
**    double endPointOut[2][3]
**            - endpoints of the optimal ramp (duh!)
**    double axis[3]  - direction of the ramp, provided for additional
**              convenience.
**
**  Return value: some linear functional of square error for this particular
**    color ramp (as returned in output).
**    It's not actually square error but you can compare
**    outputs, and the smaller is better.  For it's used for
**    comparison purposes.
**
*/

static double search43Mult (int *levLim, int n, double (*compresseddata)[3], double *pMult,
  int idx [MAX_PIXEL_PER_BLOCK], double endPointOut[2][3],
  double axis[3] ) {

    STATIC_ARR double qs   [16][3];
    STATIC_ARR double mult [16];

    /* interpolants stack for 3 levels                        */
    STATIC_ARR double kq    [3][3];
    STATIC_ARR double k2    [3];
    STATIC_ARR double sK    [3];

    STATIC double numer = 0;
    STATIC double denom = 1.;

    int lev = 0;
    int vLev;

    int i, j;
    int i0, i1, i2;
    int jk[4];




    for (i = 0; i < n; i++)
        for(j=0; j < 3; j++) {
           qs[i][j] = compresseddata[idx[i]][j];
           mult[i] = pMult[idx[i]];
    }

    kq[0][0] = kq[0][1] = kq[0][2] = k2[0] = sK[0] = 0;
    kq[1][0] = kq[1][1] = kq[1][2] = k2[1] = sK[1] = 0;
    kq[2][0] = kq[2][1] = kq[2][2] = k2[2] = sK[2] = 0;


    for (i0 = n; ; ) {
        for (i1 = n; ; ) {
        for (i2 = n; ; ) {
            double num_ = - kq[2][0] * kq[2][0]
                  - kq[2][1] * kq[2][1]
                  - kq[2][2] * kq[2][2];

        if (numer * k2[2] >= denom * num_) {
            numer = num_;
            denom = k2[2];
            jk[0] = i0;
            jk[1] = i1;
            jk[2] = i2;

        }
        if ((--i2 < i1) || (*levLim < 3))
            break;
        kq[2][0] = kq[2][0] + qs[i2][0];
            kq[2][1] = kq[2][1] + qs[i2][1];
            kq[2][2] = kq[2][2] + qs[i2][2];
            k2[2]    = k2[2]    +
            (5.  - 2. * sK[2] - mult[i2]) * mult[i2];
            sK[2]    = sK[2]    + mult[i2];
            lev = 3;
        }
        if (--i1 < i0)
            break;
        kq[2][0] = kq[1][0] = kq[1][0] + qs[i1][0];
        kq[2][1] = kq[1][1] = kq[1][1] + qs[i1][1];
        kq[2][2] = kq[1][2] = kq[1][2] + qs[i1][2];
        k2[2]    = k2[1]    = k2[1]    +
        (3.  - 2. * sK[1] - mult[i1]) * mult[i1];
        sK[2]    = sK[1]    = sK[1]    + mult[i1];
        lev = 2;
    }
    if (--i0 <1)
        break;
    kq[2][0] = kq[1][0] = kq[0][0] = kq[0][0] + qs[i0][0];
    kq[2][1] = kq[1][1] = kq[0][1] = kq[0][1] + qs[i0][1];
    kq[2][2] = kq[1][2] = kq[0][2] = kq[0][2] + qs[i0][2];
    k2[2]    = k2[1]    = k2[0]    = k2[0]    +
        (1.  - 2. * sK[0] - mult[i0]) * mult[i0];
    sK[2]    = sK[1]    = sK[0]    = sK[0]    + mult[i0];
    lev = 1;
    }
    /* get endpoints and error                            */
//    assert(numer < 0);

    jk[3] = n;
    /* collapse jk[] to have standard representation for 2 clusters
       (important to have deterministic quantization in some ambigious
       cases                                    */

    /* <vLev> is number of clusters minus 1; <lev> is (was) the highest
       cluster number; some clusters in between might be empty, so ,<vLev>
       and <lev> are not the same                        */
    vLev = (jk[0] != jk[1]) + (jk[1] != jk[2]) + (jk[2] != jk[3]);
    /* move it all the way to the top                        */
    if (vLev == 1)
    {
        /* gerry shaw: i commented out the jk[i] == jk[i+1] because all it is cause a complier warning */
        for (i = 2; i--; /*jk[i] == jk[i+1]*/)
            ; /* empty body on purpose */

        /* gerry shaw: i commented out the i<3 because all it is cause a complier warning */
        for ( ; i++; /*i< 3*/)
            jk[i+1] = jk[i];
    }

    for (kq[2][0] = kq[2][1] = kq[2][2] = sK[2] = k2[2]= 0., lev = i = j = 0;
      j < 4; j++) {
        for (; i < jk[j]; i++) {
            sK[2] += (double) j * mult[i];
        k2[2] += (double) j * (double) j * mult[i];
        kq[2][0] += (double) j * qs[i][0];
        kq[2][1] += (double) j * qs[i][1];
        kq[2][2] += (double) j * qs[i][2];
    }
    lev += (jk[j] != n);
    }
    k2[2] -= sK[2] * sK[2];
    {
        double k0  =     - sK[2];
        double k1  = (double)lev - sK[2];
        double num_ = - kq[2][0] * kq[2][0]
              - kq[2][1] * kq[2][1]
              - kq[2][2] * kq[2][2];

        for (j = 0; j < 3; j++) {
        endPointOut[0][j] = (k0 * kq[2][j]) /k2[2];
        endPointOut[1][j] = (k1 * kq[2][j]) /k2[2];
        axis[j] = kq[2][j];
    }
    *levLim = (vLev == 1) ? 1 : lev;
    return(num_/k2[2]);
    }
}

/*
**
**  static double searchClipped43Mult (int *levLim, int n, double (*compresseddata)[3],
**    double *pMult,int idx [MAX_PIXEL_PER_BLOCK], double range[2][3],
**    double endPointOut[2][3],double axis[3] )
**
**    Given set of 3D points compresseddata[], and ordering in idx[], go through
**      different clustering such that each cluster consist of consecutive
**    points as defined by the order in idx[].
**    For each clustering compute color ramp inside clipping volume with
**    minimal square error according to weights.
**    Then return color ramp w/ minimal error among all possible ramp
**    over all possible clustering.
**
**    Note that the endpoints on the color ramp are guaranteed to be inside
**        the clipping rectangular volume (unlike search43Mult() ).
**
**  Input:
**    int *levLim    - maximum allowed number of the points in color ramp
**              minus one.  (possible value 2 or 3)
**    int n        - number of points
**    double (*compresseddata)[3]    - 3D points (should be centered, weighted, and
**              multiplied by multiplicity factor pMult[].)
**      double *pMult    - multiplicity factor (sum of all factors should be
**              1.0)
**    int idx []    - order of 3D points to use for clustering
**    double range[2][3]
**            - clipping rectangular volume for centered and
**              weighted points compresseddata[].
**
**  Output:
**    int *levLim    - actual number of points in color ramp minus one.
**              (might be 1, 2, or 3).
**    double endPointOut[2][3]
**            - endpoints of the optimal ramp (duh!)
**    double axis[3]  - direction of the ramp, provided for additional
**              convenience.
**
**  Return value: some linear functional of square error for this particular
**    color ramp (as returned in output).
**    It's not actually square error but you can compare
**    outputs, and the smaller is better.  For it's used for
**    comparison purposes.
**
*/

static double searchClipped43Mult (int *levLim, int n, double (*compresseddata)[3], double *pMult,
  int idx [MAX_PIXEL_PER_BLOCK], double range[2][3],double endPointOut[2][3],
  double axis[3] ) {

    STATIC_ARR double qs   [16][3];
    STATIC_ARR double mult [16];

    /* interpolants stack for 3 levels                        */
    STATIC_ARR double kq    [3][3];
    STATIC_ARR double k2    [3];
    STATIC_ARR double sK    [3];

    STATIC double numer = 0;
    STATIC double denom = 1.;

    int lev = 0;
    int vLev;

    int i, j;
    int i0, i1, i2;
    int jk[4];


    for (i = 0; i < n; i++)
        for(j=0; j < 3; j++) {
           qs[i][j] = compresseddata[idx[i]][j];
           mult[i] = pMult[idx[i]];
    }

    kq[0][0] = kq[0][1] = kq[0][2] = k2[0] = sK[0] = 0;
    kq[1][0] = kq[1][1] = kq[1][2] = k2[1] = sK[1] = 0;
    kq[2][0] = kq[2][1] = kq[2][2] = k2[2] = sK[2] = 0;


    for (i0 = n; ; ) {
        for (i1 = n; ; ) {
        for (i2 = n; ; ) {
            double k0  =     - sK[2];
            double k1  = (double) lev - sK[2];
            double f00 = k2[2] + k0 * k0;
            double f01 = k2[2] + k0 * k1;
            double f11 = k2[2] + k1 * k1;
            double f0011 = f00 * f11;
            double den_ =
          k2[2] * f0011 * f0011 * (double)lev * (double)lev;
            double num_ = 0;

            for (j = 0; j < 3; j++) {
            double x0,  x1;
            double x0m, x1m;
            double x0M, x1M;

            x1m = range[0][j] * k2[2];
            x1M = range[1][j] * k2[2];

            x0 = k0 * kq[2][j];
            x1 = k1 * kq[2][j];

            x0m = x1m - x0;
            x0M = x1M - x0;

            x1m = (x1m - x1) * f00;
            x1M = (x1M - x1) * f00;

            x0 = (CLIP(0., x0m, x0M)) * f01;

            x0m *= f0011;
            x0M *= f0011;

            x1 = CLIP (x0, x1m, x1M);

            x0 = x1 * f01;
            x1 *= f11;

            x0 = CLIP(x0, x0m, x0M);
    /* "rotate" varialbles                            */
                x0m = x0 - x1;
                x0M = k0 * x1 - k1 * x0;

            num_ += k2[2] * x0m * x0m + x0M * x0M -
              kq[2][j] * kq[2][j] * den_;
        }
        den_ *= k2[2];

        if (numer * den_ >= denom * num_) {
            numer = num_;
            denom = den_;
            jk[0] = i0;
            jk[1] = i1;
            jk[2] = i2;

        }
        if ((--i2 < i1) || (*levLim < 3))
            break;
        kq[2][0] = kq[2][0] + qs[i2][0];
            kq[2][1] = kq[2][1] + qs[i2][1];
            kq[2][2] = kq[2][2] + qs[i2][2];
            k2[2]    = k2[2]    +
            (5.  - 2. * sK[2] - mult[i2]) * mult[i2];
            sK[2]    = sK[2]    + mult[i2];
            lev = 3;
        }
        if (--i1 < i0)
            break;
        kq[2][0] = kq[1][0] = kq[1][0] + qs[i1][0];
        kq[2][1] = kq[1][1] = kq[1][1] + qs[i1][1];
        kq[2][2] = kq[1][2] = kq[1][2] + qs[i1][2];
        k2[2]    = k2[1]    = k2[1]    +
        (3.  - 2. * sK[1] - mult[i1]) * mult[i1];
        sK[2]    = sK[1]    = sK[1]    + mult[i1];
        lev = 2;
    }
    if (--i0 <1)
        break;
    kq[2][0] = kq[1][0] = kq[0][0] = kq[0][0] + qs[i0][0];
    kq[2][1] = kq[1][1] = kq[0][1] = kq[0][1] + qs[i0][1];
    kq[2][2] = kq[1][2] = kq[0][2] = kq[0][2] + qs[i0][2];
    k2[2]    = k2[1]    = k2[0]    = k2[0]    +
        (1.  - 2. * sK[0] - mult[i0]) * mult[i0];
    sK[2]    = sK[1]    = sK[0]    = sK[0]    + mult[i0];
    lev = 1;
    }
    /* get endpoints and error                            */
//    assert(numer < 0);

    jk[3] = n;
    /* collapse jk[] to have standard representation for 2 clusters
       (important to have deterministic quantization in some ambigious
       cases                                    */
    /* <vLev> is number of clusters minus 1; <lev> is (was) the highest
       cluster number; some clusters in between might be empty, so ,<vLev>
       and <lev> are not the same                        */
    vLev = (jk[0] != jk[1]) + (jk[1] != jk[2]) + (jk[2] != jk[3]);
    /* move it all the way to the top                        */
    if (vLev == 1)
    {
        /* gerry shaw: again I've commented out the jk[i] = jk[i+1] because it's just generating a warning */
        for (i = 2; i--; /*jk[i] == jk[i+1]*/);
        /* empty body on purpose            */
        for ( ; i++; /*i< 3*/) /* same thing here about a warning and no function */
            jk[i+1] = jk[i];
    }

    for (kq[2][0] = kq[2][1] = kq[2][2] = sK[2] = k2[2]= 0., lev = i = j = 0;
      j < 4; j++) {
        for (; i < jk[j]; i++) {
            sK[2] += (double) j * mult[i];
        k2[2] += (double) j * (double) j * mult[i];
        kq[2][0] += (double) j * qs[i][0];
        kq[2][1] += (double) j * qs[i][1];
        kq[2][2] += (double) j * qs[i][2];
    }
    lev += (jk[j] != n);
    }
    k2[2] -= sK[2] * sK[2];
    {
        double k0  =     - sK[2];
        double k1  = (double)lev - sK[2];
        double f00 = k2[2] + k0 * k0;
        double f01 = k2[2] + k0 * k1;
        double f11 = k2[2] + k1 * k1;
    double f0011 = f00 * f11;
    double den_ =
      k2[2] * f0011 * f0011 * (double)lev * (double)lev;
    double num_ = 0;


        for (j = 0; j < 3; j++) {
        double x0,  x1;
        double x0m, x1m;
        double x0M, x1M;

        x1m = range[0][j] * k2[2];
        x1M = range[1][j] * k2[2];

        x0 = k0 * kq[2][j];
        x1 = k1 * kq[2][j];

        x0m = x1m - x0;
        x0M = x1M - x0;

        x1m = (x1m - x1) * f00;
        x1M = (x1M - x1) * f00;

        x0 = (CLIP(0., x0m, x0M)) * f01;

        x0m *= f0011;
        x0M *= f0011;

        x1 = CLIP (x0, x1m, x1M);

        x0 = x1 * f01;
        x1 *= f11;

        x0 = CLIP(x0, x0m, x0M);
    /* "rotate" varialbles                            */
            x0m = x0 - x1;
        x0M = k0 * x1 - k1 * x0;

        num_ += k2[2] * x0m * x0m + x0M * x0M -
          kq[2][j] * kq[2][j] * den_;

        endPointOut[0][j] = (x0 / f0011 + k0 * kq[2][j]) /k2[2];
        endPointOut[1][j] = (x1 / f0011 + k1 * kq[2][j]) /k2[2];
        axis[j] = kq[2][j];
    }
    *levLim = (vLev == 1) ? 1 : lev;
    den_ *= k2[2];
    return(num_/den_);
    }
}

/*****************************************************************************
**                                        **
**    RGB QUANTIZER "BACK-END" FUNCTIONS                        **
**      (rounding to s3tc grid, etc.        )                **
**                                        **
*****************************************************************************/


/*
**
**  static double roundMult (int nColors, int n, double (*compresseddata)[3],
**  double *pMult, double w[3], double endPointIn[2][3],
**  int endPointOut[2][3], int index [MAX_PIXEL_PER_BLOCK])
**
**    Do optinmal rounding for the ramp of three or four
**    colors, compute new colors and two bits to map old
**    colors to new colors.
**
**    We assume, that the <endPointOut> are in 5.6.5 format and the decoder
**        1. first upscales them like ....
**        2. computes intermidiate points like ...
**        exactly as specified in S3TC.
**        Basically the function provide optimized endpoints for this
**        particular encoder (in terms of weighted square error.)
**
**  Input:
**      int nColors     - number of colors on ramp (three or four)
**    int n,        - number of points to quantize
**    double (*compresseddata)[3]    - array of <n> points each of three componets, assume
**              to be already weighted by the weights <w>
**      double *pMult    - multiplicity factor (sum of all factors should be
**              1.0)
**    double w[3]    - weight coeeficients for the corrsponding channels
**              (primary colors)
**    endPointIn[2][3]- two endpoints of the ramp, weighted by the weights
**              <w>, which is the result of the continuous
**              quantization.
**
**  Output
**    int endPointOut[2][3] - rounded endpoints (format integer 5, 6, 5
**              right aligned and NOT PACKED)
**    int index [MAX_PIXEL_PER_BLOCK] - indecies mapping original point to
**              the 4 colors ramp computed with end-
**                  points <ndPointOut>  The indicies are like follows:
**              unpacked, and the mapping is as specified in S3TC
**              spec.
**      NOTE: the indices are as in S3TC but the order of the endpoints is
**        arbitrary, so during packing one needs to check the ordering
**        of the endpoints and possibly change the indices to reflect
**        swapping of endpoints.
**
**  Return value is error measure (linear relation of square error).
**
*/

static double roundMult (int nColors, int n, double (*compresseddata)[3], double *pMult,
  double w[3], double endPointIn[2][3], int endPointOut[2][3],
  int index [MAX_PIXEL_PER_BLOCK]) {

    STATIC_ARR double  ramp[3][4][4] ;    /* float ramps        - weighted iEP
                       first  index        - channel number
                   second index        - ramp number;
                   third index        - (new)point number        */

    STATIC_ARR double  rampVal[3][4][4*MAX_PIXEL_PER_BLOCK];
                /* rampVal[i][j][k] - error of mapping
                   channel i of a point unto ramp j.  where
                   k = point # * cluster # mapped to        */
    STATIC double m  = 0;
    STATIC double cf = 0;

    static const int bitNum[3] = {5,6,5};
                /* number of bits per channel            */

    int     iRamp[3][4][4] ;    /* integer ramps with differnt rounding        */

    int i= 0;
    int j= 0;
    int k= 0;
    int i0= 0;
    int c = 0;



    int    lSB = 0;            /* leftmost bit which can be non-zero when
                   channel is scaled to 0..255            */

//    assert(nColors == 3 || nColors == 4);
    /* "unweight endpoints map; compute the lower and higher values of the
    grid (taking upscaling into account) and weight them back        */
    for (i = 0; i < 3; i++) {
    lSB = (1 << (8 - bitNum[i]));
//    assert(w[i] != 0);
    /* some defence for real time                        */
    for (j = 0; j < 2; j++) {
        if (w[i] != 0)
        cf = endPointIn[j][i] / w[i] * 255.;
        else
        cf = 0.;
        c = (int)floor (cf);
    /* clip, just in case                            */
        c = c < 0 ? 0 : ( c < 256 ? c : (256 - lSB));
    /* strip LSB's, according to a new format                    */
        c &= (256 - lSB);
    /* check if expanded color still smaller then the FP one, decrease if
       needed and possible                            */
        if ((double)(c + (c >> bitNum[i])) >  cf)
        c = (c - lSB) < 0 ? c : (c - lSB);
    /* "rounding" down to the "expanded" color grid                 */
        iRamp [i][0][j]  =  iRamp [i][1+j][j] = c + (c >> bitNum[i]);
    /* get next value up the grid if exists                    */
        c = (c  + lSB) < 256 ? (c  + lSB) : c;
    /* "rounding" up to the "expanded" color grid                */
        iRamp [i][2-j][j] =  iRamp [i][3][j] = c + (c >> bitNum[i]);

    }
    }
    /* generate intermediate points and floating point ramps            */
//    assert(n <= MAX_PIXEL_PER_BLOCK);

    /* check for 4 point quantization or 3 point quantization            */
    if (nColors == 3) {
    /* 3 points quantization                            */
        for (i = 0; i < 3; i++) {
        for (j = 0; j < 4; j++) {
        double *p;
    /* compute intermediate points                        */
        iRamp [i][j][2] = (iRamp [i][j][0] +     iRamp [i][j][1]) / 2;
        for (k = 0; k < 3; k++) {
            ramp[i][j][k] = (double)iRamp[i][j][k] * w[i] / 255.;
        }

    /* computer rampVal = error of mapping particular original point k onto a
       cluster point (0,1,2,3) on a ramp j in channel i.            */
        for (p = rampVal[i][j], k = 0; k < n; k++) {
            *p++ = pMult[k] *
              (compresseddata[k][i] - ramp[i][j][0]) * (compresseddata[k][i] - ramp[i][j][0]);
            *p++ = pMult[k] *
              (compresseddata[k][i] - ramp[i][j][1]) * (compresseddata[k][i] - ramp[i][j][1]);
            *p++ = pMult[k] *
              (compresseddata[k][i] - ramp[i][j][2]) * (compresseddata[k][i] - ramp[i][j][2]);
        }
        }
    }
    } else if (nColors == 4) {
    /* 4 points quantization                            */
    for (i = 0; i < 3; i++) {
        for (j = 0; j < 4; j++) {
        double *p;
    /* compute intermediate points                        */
        iRamp [i][j][2] =
          (2 * iRamp [i][j][0] +     iRamp [i][j][1] + 1) / 3;
        iRamp [i][j][3] =
          (    iRamp [i][j][0] + 2 * iRamp [i][j][1] + 1) / 3;
        for (k = 0; k < 4; k++) {
            ramp[i][j][k] = (double)iRamp[i][j][k] * w[i] / 255.;
        }

    /* computer rampVal = error of mapping particular original point k onto a
       cluster point (0,1,2,3) on a ramp j in channel i.            */
        for (p = rampVal[i][j], k = 0; k < n; k++) {
                *p++ = pMult[k] *
              (compresseddata[k][i] - ramp[i][j][0]) * (compresseddata[k][i] - ramp[i][j][0]);
            *p++ = pMult[k] *
              (compresseddata[k][i] - ramp[i][j][1]) * (compresseddata[k][i] - ramp[i][j][1]);
            *p++ = pMult[k] *
              (compresseddata[k][i] - ramp[i][j][2]) * (compresseddata[k][i] - ramp[i][j][2]);
            *p++ = pMult[k] *
              (compresseddata[k][i] - ramp[i][j][3]) * (compresseddata[k][i] - ramp[i][j][3]);
        }
        }
    }
    }

    /* macro to compute minimum square error of point N across all ramps    */

#define ACCUMULATE_MIN_POINT_ERROR(N)                        \
         a =  p0[4*N+0] + p1[4*N+0]    + p2[4*N+0];            \
         b =  p0[4*N+1] + p1[4*N+1]    + p2[4*N+1];            \
         c = a + b - fabs(a-b);                        \
         a =  p0[4*N+2] + p1[4*N+2]    + p2[4*N+2];            \
         b =  p0[4*N+3] + p1[4*N+3]    + p2[4*N+3];            \
         a = a + b - fabs(a-b);                        \
         d += a + c - fabs(a-c);                    \


#define ACCUMULATE_MIN_POINT_ERROR_3(N)                        \
         a =  p0[3*N+0] + p1[3*N+0]    + p2[3*N+0];            \
         b =  p0[3*N+1] + p1[3*N+1]    + p2[3*N+1];            \
         c = a + b - fabs(a-b);                        \
         a =  2 * (p0[3*N+2] + p1[3*N+2]+ p2[3*N+2]);            \
         d += a + c - fabs(a-c);                    \

//    assert(MAX_PIXEL_PER_BLOCK <= 16);

    /* check for 4 points or 3 points quantization                */
    if (nColors == 4) {
    /* 4 point quantization                            */
    /* across all ramps compute the minimum error for all points and find the
       ramp with the minimum error.                        */
    m = 2. * (w[0] * w[0] + w[1] * w[1] + w[2]*w[2]) *
      (double)MAX_PIXEL_PER_BLOCK;

    for (i0 = -1, i = 0; i < 64; i++) {
        double a,b,c;
        double *p0 = rampVal[0][i & 0x3];
        double *p1 = rampVal[1][(i >> 2) & 0x3];
        double *p2 = rampVal[2][(i >> 4)];
        double d = 0.;
        switch (n) {
        case 16: ACCUMULATE_MIN_POINT_ERROR(15)
        case 15: ACCUMULATE_MIN_POINT_ERROR(14)
        case 14: ACCUMULATE_MIN_POINT_ERROR(13)
        case 13: ACCUMULATE_MIN_POINT_ERROR(12)
        case 12: ACCUMULATE_MIN_POINT_ERROR(11)
        case 11: ACCUMULATE_MIN_POINT_ERROR(10)
        case 10: ACCUMULATE_MIN_POINT_ERROR( 9)
        case  9: ACCUMULATE_MIN_POINT_ERROR( 8)
        case  8: ACCUMULATE_MIN_POINT_ERROR( 7)
        case  7: ACCUMULATE_MIN_POINT_ERROR( 6)
        case  6: ACCUMULATE_MIN_POINT_ERROR( 5)
        case  5: ACCUMULATE_MIN_POINT_ERROR( 4)
        case  4: ACCUMULATE_MIN_POINT_ERROR( 3)
        case  3: ACCUMULATE_MIN_POINT_ERROR( 2)
        case  2: ACCUMULATE_MIN_POINT_ERROR( 1)
        case  1: ACCUMULATE_MIN_POINT_ERROR( 0)
        }
    /* m = minimum amount of error in ramps so far, i0 = index of ramp with
       minimum amount of error so far.                        */
        if (d < m) {
        m = d;
        i0 = i;
        }
    }
    } else if (nColors == 3) {
    /* 3 point quantization                            */
    /* across all ramps compute the minimum error for all points and find the
       ramp withe the minimum error.                        */
    m = 2. * (w[0] * w[0] + w[1] * w[1] + w[2]*w[2]) *
      (double)MAX_PIXEL_PER_BLOCK;

        for (i0 = -1, i = 0; i < 64; i++) {
        double a,b,c;
        double *p0 = rampVal[0][i & 0x3];
        double *p1 = rampVal[1][(i >> 2) & 0x3];
        double *p2 = rampVal[2][(i >> 4)];
        double d = 0.;
        switch (n) {
        case 16: ACCUMULATE_MIN_POINT_ERROR_3(15)
        case 15: ACCUMULATE_MIN_POINT_ERROR_3(14)
        case 14: ACCUMULATE_MIN_POINT_ERROR_3(13)
        case 13: ACCUMULATE_MIN_POINT_ERROR_3(12)
        case 12: ACCUMULATE_MIN_POINT_ERROR_3(11)
        case 11: ACCUMULATE_MIN_POINT_ERROR_3(10)
        case 10: ACCUMULATE_MIN_POINT_ERROR_3( 9)
        case  9: ACCUMULATE_MIN_POINT_ERROR_3( 8)
        case  8: ACCUMULATE_MIN_POINT_ERROR_3( 7)
        case  7: ACCUMULATE_MIN_POINT_ERROR_3( 6)
        case  6: ACCUMULATE_MIN_POINT_ERROR_3( 5)
        case  5: ACCUMULATE_MIN_POINT_ERROR_3( 4)
        case  4: ACCUMULATE_MIN_POINT_ERROR_3( 3)
        case  3: ACCUMULATE_MIN_POINT_ERROR_3( 2)
        case  2: ACCUMULATE_MIN_POINT_ERROR_3( 1)
        case  1: ACCUMULATE_MIN_POINT_ERROR_3( 0)
        }
    /* m = minimum amount of error in ramps so far, i0 = index of ramp with
       minimum amount of error so far.                        */
        if (d < m) {
            m = d;
        i0 = i;
        }
    }
    }
#undef ACCUMULATE_MIN_POINT_ERROR_4
#undef ACCUMULATE_MIN_POINT_ERROR_3
    {
    int j0 = 0;
    double *p0 = ramp[0][i0 & 0x3];
    double *p1 = ramp[1][(i0 >> 2) & 0x3];
    double *p2 = ramp[2][(i0 >> 4)];
    double a,b, d;

    /* output endpoints of the ramp                        */
//    assert (i0 != -1);
    for (i = 0; i < 3; i++) {
        for (j = 0; j < 2; j++)
    /* right align 5.6.5 color values                        */
        endPointOut[j][i] =
          iRamp[i][(i0 >> (i << 1)) & 0x3][j] >> (8 - bitNum[i]);
    }

    /* having found the optimal ramp, now search through all possible
       clustering / index mapping combination on the ramp for each point to
       determine the optimal clustering / index mapping for each point.        */
    d = 0;
    for (i = 0; i < n; i++) {
            b = 2. * (w[0] * w[0] + w[1] * w[1] + w[2]*w[2]);

        for (j0 = -1,j = 0; j < nColors; j++) {
         a = ((compresseddata[i][0] - p0[j]) * (compresseddata[i][0] - p0[j]) +
             (compresseddata[i][1] - p1[j]) * (compresseddata[i][1] - p1[j]) +
             (compresseddata[i][2] - p2[j]) * (compresseddata[i][2] - p2[j])) *  pMult[i];
        if (a < b) {
            b = a;
            j0 = j;
        }
        }
    /* d = accumulation of error so far with this particular clustering /
       index mapping combination. j0 = index of nearest pointin the ramp.   */
        d += b;
//        assert(j0 != -1);
        index[i] = j0;
    }
    return(d);
    }

}

/*
**
** static double allSame(int *nColors, int n, double (*compresseddata)[3],
**   double weight[3], int endPointOut[2][3],
**   int index [MAX_PIXEL_PER_BLOCK])
**
**    Given that all points are within the ALL_SAME_THRESHOLD
**    of the first one.  This function computes the best point that
**    could be used to represent these points in terms of weighted
**    mean square error.
**
**    Note that the point is in some format as representable via S3TC
**    i.e. it could be endpoint or interpolated point (half or one-third).
**
**    Also note that because the input is floating point, we don't
**    generally assume that the input points actually have the same
**    values. Instead, we assume that they are in the the small sphere with
**    the center in the first point. The radius of the sphere should be
**    smaller then half of 1 / 255.
**
**    We assume, that the <endPointOut> are in 5.6.5 format and the decoder
**        1. first upscales them like ....
**        2. computes intermidiate points like ...
**        exactly as specified in S3TC.
**        Basically the function provide optimized endpoints for this
**        particular encoder (in terms of weighted square error.)
**
**  Input:
**    int n,        - number of points to quantize
**    double (*compresseddata)[3]    - array of <n> points each of three componets, assume
**              to be already weighted by the weights <w>
**              1.0)
**    double weight[3]- weight coeeficients for the corrsponding channels
**              (primary colors)
**
**
**  Output
**    int endPointOut[2][3] - rounded endpoints (format integer 5, 6, 5
**              right aligned and NOT PACKED)
**    int index [MAX_PIXEL_PER_BLOCK] - indecies mapping original point to
**              the 4 colors ramp computed with end-
**                  points <ndPointOut>  The indicies are like follows:
**              unpacked, and the mapping is as specified in S3TC
**              spec.
**
**      NOTE: the indices are as in S3TC but the order of the endpoints is
**        arbitrary, so during packing one needs to check the ordering
**        of the endpoints and possibly change the indices to reflect
**        swapping of endpoints.
**
**  Return value is error measure (linear relation of square error).
**
*/
static double allSame(int *nColors, int n, double (*compresseddata)[3], double weight[3],
  int endPointOut[2][3], int index [MAX_PIXEL_PER_BLOCK]) {

    /* Because the input is floating point, we don't generally assume that
    ** the input points actually have the same values. Instead, we assume
    ** that they are in the the small sphere with the center in the first
    ** point. The radius of the sphere should be smaller then 1 / 255 / 2
    */

    /* Table of representable (by means of S3T block) points of 256 grid.
       This should be static, and suppose to be initialized to 0.
       We use dynamic initialization for flexibility reasons.            */

    STATIC_ARR double colorError[3];
                /* squared error for the block for each of
                   endpoint, one half, one third
                   representations                 */

    static struct gridPoint_ {
    unsigned char valid;    /* if set, the value (defined by first index)
                   in the channel (defined by the second index)
                   is represetable with the coefficient defined
                   by the third index.                */
    unsigned char point[2];    /* endpoints to interpolate on, if the
                   value is represetable            */
    } grid[256][3][3];        /* first index    - value of the channel
                   second index    - cannel (color), RGB
                   third index    - representation type:
                   endpoint, one half, one third         */

    static int initFlag = 0;    /* set if the table has been initialized    */
    static int size[3] = {5,6,5};
                /* with in bits of the RGB values for S3TC
                   endpoints                    */
    static int intCoeff[3][4] =
    {{1, 0, 0, 1},{1, 1, 0, 2},{1, 2, 1, 3}};
                /* array of interpolation coeffitients to get
                   "endpoint", "one half", and "one third"
                   points like this:
                   (intCoeff[j][0] * p0 +intCoeff[j][1] * p1 +
                    intCoeff[j][2]) /intCoeff[j][3]        */


    int channelValue[3][3];    /* channelValue[i][j] is the best representable
                   8 bit value for the channel i and
                   representation (endpoint, one half, one
                   third)                    */


    int i,j,k,l,m;

//    assert(*nColors == 3 || *nColors == 4);

    if (! initFlag ) {
    int p,p0,p1;

    initFlag = 1;
    /* initialize table                                */
    for (l = 0; l < 3; l++) {
        for (i = (1 << size[l]) - 1; i >=0; i--) {
    /* we use always the first 1/3rd point, so j should run from the beginning
       of the range; performance is not an issue here                */
        for (j = (1 << size[l]) - 1; j >= 0 ; j--) {
    /* compute end values                            */
            p0 = (i << (8-size[l])) | (i >> (2 * size[l] - 8));
            p1 = (j << (8-size[l])) | (j >> (2 * size[l] - 8));
            for(k=0; k < 3; k++) {
    /* compute represetable value with the end values above            */
            p = (intCoeff[k][0] * p0 +intCoeff[k][1] * p1 +
              intCoeff[k][2]) /intCoeff[k][3];
    /* let's try to make interval smaller; good for the decoder accuracy    */
            if (! grid[p][l][k].valid ||
              abs(grid[p][l][k].point[1] -
                  grid[p][l][k].point[0]) > abs(p1-p0)) {
    /* mark representable points                        */
                grid[p][l][k].valid  = 1;
                grid[p][l][k].point[0] =
                  (unsigned char) (i << (8-size[l]));
                grid[p][l][k].point[1] =
                  (unsigned char) (j << (8-size[l]));
            }
            }
        }
        }
    }
    }

    /* For all of coefficients...                        */
    for (j = 0; j < *nColors - 1; j++) {
        int delta;        /* look a representable value above or below*/
        int cTopBot[2];        /* closest representable values above and
                   below                    */
        STATIC_ARR double error[2];    /* error (for the block) for the the value
                   cTopBot[]                    */

    colorError[j] = 0;
    /* For all of color shannels                        */
    for (i = 0; i < 3; i++) {

    /* Pick the first input point, and round its j'th channel to 8-bit grid */
        int c = (int)floor (compresseddata[0][i] * 255. / weight[i] + 0.5);

    /* Clip, just in case                            */
        c = c < 0 ? 0 : ( c < 256 ? c : 255);

    /* get the represetable point first above then below and pick the one with
       the smallest error for the set of original points;            */
        for (delta = 1, l = 0; l < 2; l++, delta = -delta) {
    /* If the value of c is representable, to get the second point we need to
       move in the direction  of compresseddata[0][i] from it                */
        if (! grid[k = c][i][j].valid ||
          (compresseddata[0][i] * 255. / weight[i] - (double) c) * (double) delta > 0 ) {
            k = c + delta;
                k = k < 0 ? 0 : (k < 256 ? k : 255);
    /* 0 and 255 are representable with any of coefficients, so the
       loop will stop                                */
                for ( ;!grid[k][i][j].valid; k += delta);
                /* empty loop body on purpose            */
            }
    /* Compute error for this channel                        */
        for (error[l] = 0, m = 0; m < n; m++) {
            double d = (double)k  * weight[i] - compresseddata[m][i] *255.;
            error[l] += d*d;
        }
            cTopBot[l] = k;
        }

        if (error[0] < error[1]) {
        colorError[j] += error[0];
        channelValue[i][j] = cTopBot[0];
        }
        else if (error[0] > error[1]) {
        colorError[j] += error[1];
        channelValue[i][j] = cTopBot[1];
        }
        else {
        colorError[j] += error[1];
    /* analog of even/odd rounding                        */
        channelValue[i][j] = (c & 1) ? cTopBot[0]:cTopBot[1];
        }

    }

    }

    if (*nColors == 4)
    j = ( (colorError[0] <= colorError[1]) ?
      ( (colorError[0] <= colorError[2]) ? 0 : 2) :
      ( (colorError[1] <= colorError[2]) ? 1 : 2) );
    else if (*nColors == 3)
    j = ( (colorError[0] <= colorError[1]) ? 0: 1 );

    for (i = 0; i < 3; i++)
        for (k = 0; k < 2; k++)
    /* right align 5.6.5 color values                        */
        endPointOut[k][i] =
          grid[channelValue[i][j]][i][j].point[k] >> (8-size[i]);

    for (i = 0; i < n; i++)
        index[i] = j + 1;

    if (j != 0)
    *nColors = j + 2;
    /* else just keep nColors it as it was                    */

    return(colorError[j]);
}


/*
**
**  static double mapAndRoundMult(int *nColors, int levelLimit, int n,
**    double (*compresseddata)[3], double *pMult,
**    double weight[3], double endPointIn[2][3], int endPointOut[2][3],
**    int index [MAX_PIXEL_PER_BLOCK])
**
**    Generate all possible color ramps given particular number of points
**    on a color ramp derived from continuous calculation.
**    Say you have a two point color ramp, it could be mapped to three
**    point or four point ramps in different ways.  So this function
**    explores such different mapping and find the best one.
**
**    This is needed to optimize small variance blocks into higher effective
**    resolution than 5,6,5 resolution.
**
**    We assume, that the <endPointOut> are in 5.6.5 format and the decoder
**        1. first upscales them like ....
**        2. computes intermidiate points like ...
**        exactly as specified in S3TC.
**        Basically the function provide optimized endpoints for this
**        particular encoder (in terms of weighted square error.)
**
**  Input:
**      int *nColors    - number of colors on ramp (two, three or four)
**    int levelLimit    - maximum allowed number of the points in color ramp
**    int n,        - number of points to quantize
**    double (*compresseddata)[3]    - array of <n> points each of three componets, assume
**              to be already weighted by the weights <w> (but not
**              centered!)
**      double *pMult    - multiplicity factor (sum of all factors should be
**              1.0)
**    double w[3]    - weight coeeficients for the corrsponding channels
**              (primary colors)
**    endPointIn[2][3]- two endpoints of the ramp, weighted by the weights
**              <w>, which is the result of the continuous
**              quantization.
**
**  Output
**      int *nColors    - number of actual points on ramp (three or four)
**    int endPointOut[2][3] - rounded endpoints (format integer 5, 6, 5
**              right aligned and NOT PACKED)
**    int index [MAX_PIXEL_PER_BLOCK] - indecies mapping original point to
**              the 4 colors ramp computed with end-
**                  points <ndPointOut>  The indicies are like follows:
**              unpacked, and the mapping is as specified in S3TC
**              spec.
**      NOTE: the indices are as in S3TC but the order of the endpoints is
**        arbitrary, so during packing one needs to check the ordering
**        of the endpoints and possibly change the indices to reflect
**        swapping of endpoints.
**
**  Return value is error measure (linear relation of square error).
**
*/

static double mapAndRoundMult(int *nColors, int levelLimit, int n, double (*compresseddata)[3], double *pMult,
    double weight[3], double endPointIn[2][3], int endPointOut[2][3],
    int index [MAX_PIXEL_PER_BLOCK]) {

    static int    mapNumber[2][3] =
    {{2, 0, 0}, {6, 2, 0}} ;
                /* Array holding number of additional different
                   mappings of short ramps (2 and 3 colors) to
                   inter ramps (besides endpoints to endpoints
                   on the smallest ramp)

                   first index  - 0 mapping to 3 level ramp
                            - 1 mapping to 4 level ramp
                   second index - <(input) nColors - 2>        */

    static double mapCoeff[2][2][6][2][2] =
                /* Array of coefficients to generate endpoint
                   colors for mapping short ramps (2 and 3
                   colors) to inter ramps.
                   Only initialized coefficients are used.

                   first index  - 0 mapping to 3 level ramp
                            - 1 mapping to 4 level ramp
                   second index - <(input) nColors - 2>
                          (there are now additional
                          mappings for four color ramp)
                   third index  - mapping number (up to five
                          possible)
                   forth index  - endpoint (0 or 1)
                   fith index   - coeeficient number (for the
                          first and second original
                          endpoints)            */
    {
    /* mappings to 3 color ramp                            */
    {
    /* two clusters                                */
        {
            {{  2. , -1. }, {  0. , 1. }},
            {{  1. ,  0. }, { -1. , 2. }}
        }
    /* three clusters                                */

    },
    /* mappings to 4 color ramp                            */
    {
    /* two clusters                                */
        {
            {{  1. ,  0. }, {  0. , 1. }},
            {{  1. ,  0. }, { -2. , 3. }},
            {{  3. , -2. }, {  0. , 1. }},
            {{  2. , -1. }, { -1. , 2. }},
            {{  1.5, -0.5}, {  0. , 1. }},
            {{  1. ,  0. }, { -0.5, 1.5}},

        },
    /* three clusters                                */
        {
            {{  1.5, -0.5}, {  0. , 1. }},
            {{  1. ,  0. }, { -0.5, 1.5}},
        }
        }
    };

    STATIC_ARR double colorInVar[2][3];
    STATIC_ARR double e[2];        /* error for current and next mapping        */
    int endPointOutVar[2][2][3];/* integer (5.6.5) endpoint colors for
                   current and nex mapping            */
    int outLevVar[2];        /* level (number of clusters in ramp) for
                   current and nex mapping            */
    int indexVar[2][MAX_PIXEL_PER_BLOCK];


    int i, j, k, l;
    int m;

//    assert(*nColors > 1);
//    assert(levelLimit > 1);
//    assert(*nColors <= levelLimit);

    /* check "default" mapping                            */

    m = 0;

    outLevVar[m] = (*nColors == 4 ? 1 : 0);

    e[m] = roundMult (outLevVar[m] + 3, n, compresseddata, pMult, weight, endPointIn, endPointOutVar[m],
      indexVar[m]);

    m = 1 - m;

    levelLimit -= 2;
    *nColors  -= 2;

    for (i = 0; i < levelLimit; i++) {    /* enumerate ramps (3 or 4 clusters)*/
    for (j = 0; j < mapNumber[i][*nColors]; j++) {
                    /* enumerate mappings for this ramp */
        for (k = 0; k < 2; k ++) {    /* enumerate two new endpoints        */
        for (l = 0; l < 3; l ++) {
                    /* enumerate three color channels for
                       endpoint                */
            colorInVar[k][l] =
            mapCoeff[i][*nColors][j][k][0] * endPointIn[0][l] +
            mapCoeff[i][*nColors][j][k][1] * endPointIn[1][l];
    /* check clipping, don't waist time if out of range (not suppose to be) */
    /* we assume lower bound is alway 0 here, as the color space is standrd,
       the points however are weigted, so the upper bound may vary        */

        }
        }
        outLevVar[m] = i;
        e[m] = roundMult (i + 3, n, compresseddata, pMult, weight, colorInVar, endPointOutVar[m], indexVar[m]);
    /* swap bufeers if new one is better                    */
        if (e[m] < e[1 - m])
        m = 1 - m;
    }
    }
    m = 1 - m;
    for (i = 0; i < n; i++)
    index[i] = indexVar[m][i];
    for (i = 0; i < 2; i++)
    for (j = 0; j < 3; j++)
        endPointOut[i][j] = endPointOutVar[m][i][j];
    *nColors = outLevVar[m] + 3;
    return(e[m]);
}


/*****************************************************************************
**                                        **
**    MAIN RGB QUANTIZER                                **
**                                        **
*****************************************************************************/

/*
**
**  void CodeRGBBlock(RGBBlock *block)
**
**    Main RGB quantizer, take in a block (see S3_quant.h for details of
**    block structure), and returns the block quantized.
**
**  Input:
**    RGBBlock *block - block of rgb color data and quantization parameters
**              (for details, see S3_quant.h)
**
**  Output
**    RGBBlock *block - block of rgb color data quantized as per S3TC spec
**              (for details, see S3_quant.h)
**
*/

static void CodeRGBBlock(RGBBlock *block) {
    STATIC_ARR double compresseddata[MAX_PIXEL_PER_BLOCK][3];
                    /* RGB channel values, weighted            */
    STATIC_ARR double qC[MAX_PIXEL_PER_BLOCK][3];
                    /* RGB channel values, weighted, centered   */
    STATIC_ARR double gC[3];        /* RGB pixels gravity center, weighted        */

    /* Non-redundunt points are essential for rounding performace        */

    STATIC_ARR double qNoRep[MAX_PIXEL_PER_BLOCK][3];
                    /* RGB channel values, with equal points
                   removed                    */
    STATIC_ARR double qCNoRep[MAX_PIXEL_PER_BLOCK][3];

    STATIC_ARR double pMult[MAX_PIXEL_PER_BLOCK];
                    /* Multiplicity of each point of qNoRep
                   in the original set                */


    STATIC_ARR double weight[3];

    STATIC_ARR double range[2][3];        /* clipping range in centered coordinate
                   system                    */

    STATIC_ARR double axis[3];

    STATIC_ARR double e[2];
    STATIC_ARR double endPointOut[2][2][3];
    STATIC double nRec;

    int indexMult[MAX_PIXEL_PER_BLOCK];
                /* index to map compresseddata[] onto qNoRep            */
    int outIndexMult [MAX_PIXEL_PER_BLOCK];
                /* quantized index fro qNoRep            */

    int index[INDEX_LOG_SIZE][MAX_PIXEL_PER_BLOCK];
    int indexStore[2][MAX_PIXEL_PER_BLOCK];

    int levelLimit[2];

    int i = 0;
    int j = 0;
    int n = 0;
    int l = 0;

    int nNoRep       = 0;

    int indexLogEnd  = 0;
    int indexLogFull = 0;
    int sameIndex    = 0;

    int m        = 0;
    int count    = 0;
    int allsame  = 1;
    int clipFlag = 0;

    if (block == NULL)
    return;
    n = block->n;

    if (n == 0)
    return;

    nRec = 1/ (double) n;

    for (j = 0; j < 3; j++)
    weight[j] = (block->weight[j]<0)?0.:block->weight[j];

    /*  weight colors, check if all are the same and compute gravity center */
    for (j = 0; j < 3; j++) {
    for (gC[j] = 0., i = 0; i < n; i++) {
        allsame &=
          fabs(block->colorChannel[i][j] - block->colorChannel[0][j]) <
          ALL_SAME_THRESHOLD;
        gC[j] += (compresseddata[i][j] =  block->colorChannel[i][j] * weight[j]);
    }
    }

    if (allsame) {
    /* code the block                                */
    levelLimit[0] = block->inLevel;
        allSame(levelLimit, n, compresseddata, weight, block->endPoint, block->index );
    block->outLevel = levelLimit[0];
    return;
    }


    /* compute centered source points and range                    */
    for (j = 0; j < 3; j++) {
    for (gC[j] /= (double) n, i = 0; i < n; i++)
        qC[i][j] = compresseddata[i][j] - gC[j];
    range[0][j] = - gC[j];
    range[1][j] = weight[j] - gC[j];
    }

    /* get rid of multiplicity                            */
    for (nNoRep = i = 0; i < n; i++) {
    for (j = 0; j < nNoRep; j ++) {
        if (qNoRep[j][0] == compresseddata[i][0] && qNoRep[j][1] == compresseddata[i][1] &&
          qNoRep[j][2] == compresseddata[i][2])
            break;
    }
    if (j == nNoRep) {
        qNoRep[j][0] = compresseddata[i][0];
        qNoRep[j][1] = compresseddata[i][1];
        qNoRep[j][2] = compresseddata[i][2];
        qCNoRep[j][0] = qC[i][0];
        qCNoRep[j][1] = qC[i][1];
        qCNoRep[j][2] = qC[i][2];
        pMult[j] = 1;
        nNoRep++;
    }
    else {
            qCNoRep[j][0] += qC[i][0];
        qCNoRep[j][1] += qC[i][1];
        qCNoRep[j][2] += qC[i][2];
        pMult[j]++;
    }
    indexMult[i] = j;
    }

    /* normalize mulipicators                            */
    for (i = 0; i < nNoRep; i++) {
    pMult[i] *= nRec;
    qCNoRep[i][0] *= nRec;
    qCNoRep[i][1] *= nRec;
    qCNoRep[i][2] *= nRec;
    }


    /* two colors bypass                            */
    if (nNoRep == 2) {
    block->outLevel = 2;
    mapAndRoundMult(&(block->outLevel), block->inLevel,  nNoRep, qNoRep,
        pMult, weight, qNoRep, block->endPoint, outIndexMult);

    /* restore index with multiplicity                        */
    for (i = 0; i < n ; i++)
        block->index[i] = outIndexMult[indexMult[i]];
    return;
    }

    indexLogFull = indexLogEnd = 0;
    getAxis(n, qC, axis);
    sortProjection(nNoRep, qNoRep, axis, index[0], 0);

    for (i = 0; i < 2; i++) {
    count = UNCLIPPED_ITERATION_LIMIT;

    do {
    /* !!! search and search clipped use different levelLimint convention;
       it should be less by one                            */
        levelLimit[i] =  block->inLevel - 1;

    /* bypass the fist time when i== 1 to put new index on the index log
       (axis is coming from <getDiameter>                    */

        if (i - 1 + count != UNCLIPPED_ITERATION_LIMIT) {
            e[i] = search43Mult (levelLimit + i, nNoRep, qCNoRep, pMult,
        index[indexLogEnd], endPointOut[i], axis);
    /* biggest number of indexing to consider                    */
        m = i;
        }
        else {
    /* get new axis                                */
        getDiameter(nNoRep, qNoRep, axis);
        }

        indexLogEnd = (indexLogEnd + 1) % INDEX_LOG_SIZE;
        indexLogFull |= (indexLogEnd == 0);

        for (l =0; l < 2; l++) {
        sortProjection(nNoRep, qNoRep, axis, index [indexLogEnd],l );

        for (j = indexLogEnd - 1; j >= 0; j--)
        {
            sameIndex = sameOrder(nNoRep, index[j], index[indexLogEnd]);
            if (sameIndex)
                break;
        }

        if (!sameIndex && indexLogFull)
            for (j = indexLogEnd + 1; j < INDEX_LOG_SIZE; j++)
            {
                sameIndex = sameOrder(nNoRep, index[j], index[indexLogEnd]);
                if (sameIndex)
                    break;
            }
        if (!sameIndex || count == 0)
            break;
        }
    }
    while (count-- != 0 && ! sameIndex);

    for (j = 0; j < nNoRep; j++)
        indexStore[i][j] = index[indexLogEnd][j];
    }

    /* pick the best one                            */

    if (m == 1)
    m = (e[0] < e[1] ? 0 : 1);

    /* check clipping                                */
    for (clipFlag =0, i = 0; i < 2; i++)
    for (j = 0; j < 3; j++)
        clipFlag = clipFlag || (endPointOut[m][i][j] < range[0][j]) ||
          (endPointOut[m][i][j] > range[1][j]);

    if (clipFlag) {
    /* start with the best unclipped index                    */
    for (i = 0; i < nNoRep; i++)
        index[0][i] = indexStore[m][i];

    count = CLIPPED_ITERATION_LIMIT;
    indexLogFull = indexLogEnd = 0;

    do {
    /* !!! FIXME: seach and seach clipped use different levelLimint convention;
       it should be less by one                            */
        levelLimit[m] =  block->inLevel - 1;
        e[m] = searchClipped43Mult (levelLimit + m, nNoRep, qCNoRep, pMult,
        index[indexLogEnd],
        range, endPointOut[m], axis);

        indexLogEnd = (indexLogEnd + 1) % INDEX_LOG_SIZE;
        indexLogFull |= (indexLogEnd == 0);

        for (l =0; l < 2; l++) {
        sortProjection(nNoRep, qNoRep, axis, index [indexLogEnd],l );

        for (j = indexLogEnd - 1; j >= 0; j--)
        {
            sameIndex = sameOrder(nNoRep, index[j], index[indexLogEnd]);
            if (sameIndex)
                break;
        }

        if (!sameIndex && indexLogFull)
            for (j = indexLogEnd + 1; j < INDEX_LOG_SIZE; j++)
            {
                sameIndex = sameOrder(nNoRep, index[j], index[indexLogEnd]);
                if (sameIndex)
                    break;
            }
        if (!sameIndex)
            break;
        }

    }
    while (count-- != 0 && ! sameIndex);

    }

    /* transfom endpoints to original (uncentered) system            */
    for (i = 0; i < 2; i++)
    for (j = 0; j < 3; j++)
        endPointOut[1-m][i][j] = endPointOut[m][i][j] + gC[j];

    block->outLevel = levelLimit[m]+1;

    mapAndRoundMult(&(block->outLevel), block->inLevel,  nNoRep, qNoRep,
    pMult, weight, endPointOut[1-m], block->endPoint, outIndexMult);

    /* restore index with multiplicity                        */
    for (i = 0; i < n ; i++)
    block->index[i] = outIndexMult[indexMult[i]];

}

/*****************************************************************************
**                                        **
**    MAIN ALPHA QUANTIZER                                **
**                                        **
*****************************************************************************/

/*
**
** void CodeAlphaBlock(AlphaBlock *b)
**
**    Main alpha quantizer, take in a block containing alpha data
**    (see S3_quant.h for details of block structure), and returns the block
**    quantized.)
**
**  Input:
**    AlphaBlock *b - block of alpha data and quantization parameters
**              (for details, see S3_quant.h)
**
**  Output
**    AlphaBlock *b - block of alpha color data quantized as per S3TC spec
**              (for details, see S3_quant.h)
**
*/
static void CodeAlphaBlock(AlphaBlock *b) {
    /* quantization combinations to run
    comb#    ramp size    0 or 1 preserved?
    0        -    8        (0 in 1 in)
    1        -    6        (0 in 1 in)
    2        -    6        (0 out 1 in)
    3        -    6        (0 in 1 out)
    4        -    6        (0 out 1  out)
    */

    STATIC_ARR double  e    [5];

    STATIC_ARR double  alphaFiltered
                [5][MAX_PIXEL_PER_BLOCK];

    int        newLevel        [5] = {8,6,6,6,6};
    int        index        [5][MAX_PIXEL_PER_BLOCK];
    int        nFiltered        [5];
    int        endPointOut        [5][2];

    int        zeroMask        [5] = {0,0,1,0,1};
    int        oneMask        [5] = {0,0,0,1,1};
    int        mask[5];

    int        enforceZero;
    int        enforceOne;


    int i, j, k;

    /* filter alpha set to try different quantizations                */
    for (i = 0; i < 5; i++)
    nFiltered[i] = 0;

    for (i = 0; i < b->n; i++) {
    alphaFiltered[0][nFiltered[0]] = b->alpha[i];
    nFiltered[0] += 1;
    alphaFiltered[1][nFiltered[1]] = b->alpha[i];
    nFiltered[1] += 1;
    if (b->alpha[i] != 0.) {
        alphaFiltered[2][nFiltered[2]] = b->alpha[i];
        nFiltered[2] += 1;
    }
    if (b->alpha[i] != 1.) {
        alphaFiltered[3][nFiltered[3]] = b->alpha[i];
        nFiltered[3] += 1;
    }
    if (b->alpha[i] != 0. && b->alpha[i] != 1.) {
        alphaFiltered[4][nFiltered[4]] = b->alpha[i];
        nFiltered[4] += 1;
    }
    }
    enforceZero = (b->need0 && nFiltered[0] != nFiltered[2]);
    enforceOne  = (b->need1 && nFiltered[0] != nFiltered[3]);

    mask[0] = 1;
    mask[1] = 1;
    mask[2] = (nFiltered[2] != nFiltered[1]);
    mask[3] = (nFiltered[3] != nFiltered[1]);
    mask[4] = (nFiltered[4] != nFiltered[1]);

    for (i = 0; i < 5; i++) {
    if (mask[i]) {
        e[i] = quantizeAlpha (newLevel[i], nFiltered[i], alphaFiltered[i],
          endPointOut[i], index [i]);
    /*    see if  will be  OK to use                        */
        mask[i] = 1;
        if (enforceZero && ! zeroMask[i])
        mask[i] &= (endPointOut[i][0] == 0 || endPointOut[i][1] == 0);
        if (enforceOne  && ! oneMask[i])
        mask[i] &= (endPointOut[i][0] == 1 || endPointOut[i][1] == 1);
    }
    }

    /* get the best sutable                            */
    for (i = 0; i < 5; i++)
    if (mask[i])
        break;

    for (k = i++ ; i < 5; i++)
    if (mask[i] && e[i] < e[k])
        k = i;

    b->outLevel        = newLevel[k];
    b->endPoint[0]  = endPointOut[k][0];
    b->endPoint[1]  = endPointOut[k][1];
    for (i = j = 0; i < b->n; i++) {
    if (b->alpha[i] == 0. && zeroMask[k])
        b->index[i] = 6;
    else if (b->alpha[i] == 1. && oneMask[k])
        b->index[i] = 7;
    else
        b->index[i] = index[k][j++];
    }
}

/*****************************************************************************
**                                        **
**    ALPHA QUANTIZING ROUTINE                        **
**                                        **
*****************************************************************************/

/*
**
**  static double quantizeAlpha (int level, int n,
**    double alpha[MAX_PIXEL_PER_BLOCK], int endPointOut[2],
**    int  index [MAX_PIXEL_PER_BLOCK])
**
**    Main alpha quantization routine.  Takes in alpha values, and via
**    heuristic method encode the alpha values on ramp divided
**    into 6 or 8 bins.
**
** INPUT
**    int level        - ramp size (6 or 8)
**    int n        - number of alpha values
**    double alpha[MAX_PIXEL_PER_BLOCK]
**            - actual alpha values
**
** OUTPUT
**    int endPointOut[2]
**            - 8 bit alpha ramp endpoints
**    int  index [MAX_PIXEL_PER_BLOCK])
**            - indices as in S3TC
**
** RETURN VALUE: error measurement (linear of mean square error)
**
*/
static double quantizeAlpha (int level, int n,
  double alpha[MAX_PIXEL_PER_BLOCK], int endPointOut[2],
  int  index [MAX_PIXEL_PER_BLOCK]) {

    STATIC_ARR double binBoundary[MAX_ALPHA_LEVELS -1];
                /* array of current bin (cluster) boundaries*/
    STATIC_ARR double alphaCentered[MAX_PIXEL_PER_BLOCK];
                /* array of centered alpha values        */

    STATIC_ARR double endPoint[2];
                /* current ramp endpoints, usually centered */
//    STATIC_ARR double endPoint_[2];
//                /* current ramp endpoints, usually centered */

    STATIC double qS;        /* average of uncentered alphas, (gravity
                   center)                     */
    STATIC double kS, kq, k2;
                /* curren values of linear and quadratic
                   forms to evaluate target functional, see
                   S3TC made easy                */
    STATIC double e;        /* error                    */
    STATIC double num;        /* numerator of the error functional        */
    STATIC double den;        /* denominator of the error functional        */

    STATIC double nD;        /* n in double                    */
    STATIC double nR;        /* reciprocal of n                */

    int cluster[MAX_ALPHA_LEVELS];
                /* number of points in each cluster, the
                   the index here goes sequentially  aint
                   linear ramp                    */
    int clusterEnd[2][MAX_ALPHA_LEVELS];
                /* smallest and biggest value (if exist)
                   in each cluster                */
    int newAlpha[MAX_ALPHA_LEVELS];
                /* final integer alpha values for each cluster
                   in the original coordinate system        */

    int i, j, k, l;
    int count = ALPHA_ITERATION_LIMIT;
                /* Counter to implement forced exit out of
                   incremental optimzer (as otherwise the
                   number of iterations can potentially be
                   very big and infinite cycling is possible
                   as well)
                   In practiacal cses, though, expect about
                   order of 3 iteretions per block on
                   average                    */

    int stable;            /* Flag to indicate the optimal solution
                   (on phase 1 or on phases 1 and 2 has been
                   reached and the optimization can be
                   stopped                    */
    int nClusters;        /* Current number of non-empty clusters        */


//    assert(level > 1 && level <= MAX_ALPHA_LEVELS);

    if (n == 0)
    return (0.);

    nD = (double) n;
    nR = 1 / nD;

    /* find minimum and maximum alphas, initialize index and compute sum    */
    for (qS = endPoint[0] = endPoint[1] = alpha[0], index[0] = 0,
      i = 1; i < n; i++) {
    if (endPoint[0] > alpha[i])
        endPoint[0] = alpha[i];
    if (endPoint[1] < alpha[i])
        endPoint[1] = alpha[i];
    qS += alpha[i];
    /* assign all points to cluster 0, so the optimazer will classify it as
       unstable and start to iterate (unless the case is actually
       degenerative)                                */
    index[0] = 0;
    }
   /* compute gravity center and centered values                */
    qS *= nR;
    for (i = 0; i < n; i++)
    alphaCentered[i] = alpha[i] - qS;

    endPoint[0] -= qS;
    endPoint[1] -= qS;

    /* run binning optimization                            */
    do {
    /* assume bining is stable                            */
    stable = 1;

    /* biuld bin boundaries and initialize clusters                */
    for (i = 0; i < level - 1; i++) {
        binBoundary[i] = endPoint[0] + (endPoint[1] - endPoint[0]) /
          (double)(level - 1) * (0.5 + (double) i);
        cluster[i] = 0;
    }
    cluster[level - 1] = 0;

    /* do binning, change this to binary search                    */

    for (i = 0; i < n; i++) {
        for (j = 0; j < level - 1; j++)
        if (alphaCentered[i] < binBoundary[j])
            break;
        cluster[j]++;
        stable  &= (index[i] == j);
        index[i] = j;
    }
    /* check if degenerate, to handle cases where all point are equal and
       similar                                    */
    for (nClusters = 0, j = 0; j < level; j++)
        nClusters += (cluster[j] != 0);

    if (nClusters == 1)
        break;

    /* stop otimization if the limit of iterations number is reached        */

    /* compute target functional components optimal endpoints            */
    kq = kS = k2 =0.;
        for(i = 0; i < n; i ++) {
            kq += alphaCentered[i] * (double)index[i];
        kS += (double)index[i];
        k2 += (double)index[i] * (double)index[i];
    }
    kS *= nR;
    k2 -= nD * kS * kS;

    /* compute endpoints, do optimal cliping and final targe functional        */
    {
        double k0  =    - kS;
        double k1  = (double) level - 1. - kS;
        double f00 = k2 + nD * k0 * k0;
        double f01 = k2 + nD * k0 * k1;
        double f11 = k2 + nD * k1 * k1;
        double f0011 = f00 * f11;
        double den_ =
          k2 * f0011 * f0011 *
          (double)(level - 1.) * (double)(level -1.);
//        double num_ = 0;

          {
            double x0,  x1;
            double x0m, x1m;
            double x0M, x1M;
    /* clipping range for the centered points is [-qS, 1-qS]            */
            x1m = - qS * k2;
            x1M = (1. - qS) * k2;

            x0 = k0 * kq;
            x1 = k1 * kq;

            x0m = x1m - x0;
            x0M = x1M - x0;

            x1m = (x1m - x1) * f00;
            x1M = (x1M - x1) * f00;

            x0 = (CLIP(0., x0m, x0M)) * f01;

            x0m *= f0011;
            x0M *= f0011;

            x1 = CLIP (x0, x1m, x1M);

            x0 = x1 * f01;
            x1 *= f11;

            x0 = CLIP(x0, x0m, x0M);
    /* "rotate" varialbles                            */
                x0m = x0 - x1;
                x0M = k0 * x1 - k1 * x0;

            num = k2 * x0m * x0m + nD * x0M * x0M -
              kq * kq * den_;
            den = den_ * k2;

//            endPoint_[0] = endPoint[0];
//            endPoint_[1] = endPoint[1];

            endPoint[0] = (x0 / f0011 + k0 * kq) / k2;
            endPoint[1] = (x1 / f0011 + k1 * kq) / k2;
        }
    }
    if (count-- == 0)
        break;

    if (stable) {
    /* if clustering is self consistent, try to improve it, moving biggest
       ot smallest points in the cluster to the next (previous) one,
       otherwise skip this and go to new end points computations        */

    /* initialize  index of minimum point of clusters                */
         for (j = 0; j < level; j++)
        clusterEnd[0][j] = -1;

    /* compute minimum and maximum points of the cluster            */
        for (i = 0; i < n; i++) {
    /* get cluster number                            */
        j = index[i];
    /* update minimum and maximum points of the cluster                */
        if (clusterEnd[0][j] == -1)
            clusterEnd[0][j] = clusterEnd[1][j] = i;
        else {
            if (alphaCentered[clusterEnd[0][j]] > alphaCentered[i])
            clusterEnd[0][j]  = i;
            if (alphaCentered[clusterEnd[1][j]] < alphaCentered[i])
            clusterEnd[1][j]  = i;
        }
        }
    /* try to move points up or down to the next bin                */
        for (l = 0; l < 2; l++) {
        double f = (l == 0 ? 1. : -1);
        double kST = kS + f * nR;
        double  k2T_0 = k2 + 1. - f * 2. * kS - nR;
        for (j = l; j < level - 1 + l; j++) {
    /* don't allow collupse to a single cluster                    */
            if (cluster[j] != 0 && (cluster[j] > 1 || nClusters > 2)) {
            double kqT = kq + f * alphaCentered[clusterEnd[1-l][j]];
            double k2T = k2T_0 + f * 2. * (double) j;
    /* compute endpoints, do optimal cliping and final targe functional        */

            double k0  =    - kST;
            double k1  = (double) level - 1.- kST;
            double f00 = k2T + nD * k0 * k0;
            double f01 = k2T + nD * k0 * k1;
            double f11 = k2T + nD * k1 * k1;
            double f0011 = f00 * f11;
            double den_ =
               k2T * f0011 * f0011 *
               (double)(level - 1.) * (double)(level -1.);
            double num_;

            double x0,  x1;
            double x0m, x1m;
            double x0M, x1M;
    /* clipping range for the centered points is [-qS, 1-qS]            */
            x1m = - qS * k2T;
            x1M = (1. - qS) * k2T;

            x0 = k0 * kqT;
            x1 = k1 * kqT;

            x0m = x1m - x0;
            x0M = x1M - x0;

            x1m = (x1m - x1) * f00;
            x1M = (x1M - x1) * f00;

            x0 = (CLIP(0., x0m, x0M)) * f01;

            x0m *= f0011;
            x0M *= f0011;

            x1 = CLIP (x0, x1m, x1M);

            x0 = x1 * f01;
            x1 *= f11;

            x0 = CLIP(x0, x0m, x0M);
    /* "rotate" varialbles                            */
                x0m = x0 - x1;
                x0M = k0 * x1 - k1 * x0;



            num_ = k2T * x0m * x0m + nD * x0M * x0M -
                  kqT * kqT * den_;
            den_ = den_ * k2T;

            if (num * den_  > num_ * den ) {
                endPoint[0] = (x0 / f0011 + k0 * kqT) / k2T;
                endPoint[1] = (x1 / f0011 + k1 * kqT) / k2T;
                num = num_;
                den = den_;
                stable = 0;
                break;
            }
            }
        }
        if (!stable)
            break;
        }
    }

    /* the very first run we checked vs index of all zeros, so the result
       will be "unstable" (as the endpoints were different) and we'll go to
       iterate into the optimizer                        */
    } while (! stable);

    for (i = 0; i < 2; i++) {
    newAlpha [i] = (int)floor((endPoint[i]  + qS) * 255. + 0.5);
        newAlpha[i] = (newAlpha[i] < 0 ? 0 : (newAlpha[i] > 255 ? 255 : newAlpha[i]));
    }

    for (i = 2; i < level; i++)
    newAlpha[i] = ((level - i) * newAlpha[0] +  (i - 1) * newAlpha[1] +
      (level - 2) / 2) / (level - 1);

    endPointOut[0] = newAlpha [0];
    endPointOut[1] = newAlpha [1];

    for (e = 0., i = 0; i < n; i++) {
    for (k = 0, j = 1; j < level; j++)
        if (fabs((double)newAlpha[j] - alpha[i] * 255.) <
          fabs((double)newAlpha[k] - alpha[i] * 255.))
        k = j;
    index[i] = k;
    e += ((double)newAlpha[k] - alpha[i] * 255.) * ((double)newAlpha[k] - alpha[i] * 255.);
    }
    return (e);
}



/************************************************************/
/* Code taken from S3_intrf.c, S3's exporter source         */
/* Modified to use unsigned char * rather than direct draw surfaces. */
/************************************************************/

#if 0
// determine number of bytes needed to compress given source image
static unsigned S3TCgetEncodeSize(int width, int height, unsigned dwEncodeType)
{
// calculate number of blocks and multiply by the number of bytes required per block
// if encoding alpha separately, requires 16 bytes per block, else 8 bytes
    return ((width+3)/4)*((height+3)/4)*
                ((dwEncodeType&_S3TC_ENCODE_ALPHA_MASK)?16:8);
}
#endif

#define ONEOVERMAX 0.003921568627450980392156862745098 // divides are bad so mult by 1/255.0

// encode (compress) given source image to given destination surface
// note: default weights could be {.3086, .6094, .0820}
unsigned int S3TCencode(int width, int height, int rowbytes, unsigned char *lpSrc, unsigned char *lpDst, unsigned dwEncodeType, float *weight, int rgb555, int BigEndian)
{
    int y= 0;                        // current Y line (steps 4 pixels each)
    unsigned char *lpSrcBuf = 0;    // current beginning of line in source buffer
    unsigned char *lpDstBuf = 0;    // current destination buffer pointer
//    int bpp= 0;                    // byte per pixel
//    unsigned aShift;            // bit shift for alpha component
//    unsigned aRef;                // adjusted alpha reference value

    int NextLine = rowbytes/sizeof(ARGB)-4;
    int pixels    = rowbytes*4;
    // precompute some useful values
//    bpp=4;  // Always 32bit data.

    // now encode stuff (...)
    lpSrcBuf=(unsigned char *)lpSrc;
    lpDstBuf=(unsigned char *)lpDst;

    // main y loop, go thru each upper-left most point on all 4x4 blocks

    for (y=0;y<(int)height;y+=4,lpSrcBuf+=pixels)
    {
        int x;                              // current X pixel (steps 4 pixels each)
        unsigned char *lpSrcCur;            // pointer to beginning of current source block
        int blockHeight;                    // height of current blocks in pixel

        blockHeight=gmin(height-y,4);
        lpSrcCur=lpSrcBuf;
        // main x loop, go thru each upper left most point on all 4x4 blocks
        for (x=0;x<(int)width;x+=4,lpSrcCur+=16)
        {
            int blockWidth= 0;               // width of current block in pixel
            RGBBlock b={0,};                      // current source block
            int k= 0;                        // misc loop var
            int index[16];                   // index of encoded texels in RGBBlock
                                             // if index[i] < 0, texel i is not quantizable
                                             // either due to colorkey, alpha compare, or
                                             // outside image boundry
            unsigned short endPt[2];         // RGB565 encoded quantized end points
            int bSwapped;                    // end points swapped?
            unsigned dwIndex=0;              // encoded index values

            for (k=0;k<3;k++)
                    b.weight[k]=weight[k];

            blockWidth=gmin(width-x,4);
            b.n=0;


            // Encode the Alpha block first since this is the way DX7 wants it.
            switch(dwEncodeType&_S3TC_ENCODE_ALPHA_MASK)
            {
                case S3TC_ENCODE_ALPHA_NONE:
                    break;
                case S3TC_ENCODE_ALPHA_EXPLICIT:
                    {
                        int iy;                        // inner block pixel loop variable
                        ARGB *lpCur;                // current pixel

                        lpCur=(ARGB*)lpSrcCur;

                        // go thru all pixels in the block and write out the alpha values
                        for (iy=0;iy<4;iy++,lpCur+=NextLine)
                        {
                            if (iy<blockHeight)            // skip if outside
                            {
                                unsigned short val = 0;    // current output value
                                int ix = 0;
                                for (ix=0;ix<4;ix++,lpCur+=1)
                                    // pack it...
                                    if (ix<blockWidth)
                                        val = (unsigned short)((val>>4) | (((lpCur->a * 15 + 128)/255) << 12));
                                    else
                                        val>>=4;
                                // write it out
                                //*(unsigned short *)lpDstBuf=val;
                                gputi(lpDstBuf, val, 2);
                            }
                            else
                                //*(unsigned short *)lpDstBuf=0;
                                gputi(lpDstBuf, 0, 2);

                            //gwrite(g, buf, 2);
                            lpDstBuf+=2;
                        }
                    }
                    break;
                case S3TC_ENCODE_ALPHA_INTERPOLATED:
                    {
                        AlphaBlock a={0,};                // current alpha source block
                        int ix,iy;                    // inner block pixel loop variable
                        ARGB *lpCur;                // current pixel
                        unsigned val;                // current output value

                        lpCur=(ARGB*)lpSrcCur;

                        a.n=0;
                        k=0;
                        // go thru all pixels in the block and set up the AlphaBlock structure a
                        for (iy=0;iy<4;iy++,lpCur+=NextLine)
                            for (ix=0;ix<4;ix++,lpCur+=1)
                                if (ix<blockWidth&&iy<blockHeight)
                                {
                                    index[k++]=a.n;
                                    //a.alpha[a.n++]=(cur==max)?1.f:cur/((float)max); // to ensure that 1.0 exists
                                    a.alpha[a.n++]=(lpCur->a==255)?1.0f:(float)(lpCur->a*ONEOVERMAX);
                                }
                                else
                                    index[k++]=-1;

                        // set flags -> do we need total opaqueness and total transparecy
                        // explicitly?
                        a.need0=dwEncodeType&S3TC_ENCODE_ALPHA_NEED0;
                        a.need1=dwEncodeType&S3TC_ENCODE_ALPHA_NEED1;

                        // check to see if there are no points to be quantized...
                        if (a.n==0) {
                            // zero out endpoints (they are invalid)
                            a.endPoint[0]=a.endPoint[1]=0;
                            //for (k=0;k<16;k++)
                            //    _ASSERTE(index[k]==-1);
                        } else
                            // !!! ENTRY INTO S3TC QUANTIZER !!!
                            CodeAlphaBlock(&a);

                        // check if endpoints are equal, if so, default to 6 points,
                        // second endpoint invalid, so increment to indicate 6 points.
                        // force all indices to endpoint 0.
                        if (a.endPoint[0]==a.endPoint[1]) {
                            a.endPoint[1]++;
                            for (k=0;k<a.n;k++) {
                                a.index[k]=0;
                            }
                            a.outLevel=6;
                        }

                        // need swapping?
                        if ((a.endPoint[0]>a.endPoint[1])==(a.outLevel==6))
                        {
                            int sw=a.endPoint[0];
                            a.endPoint[0]=a.endPoint[1];
                            a.endPoint[1]=sw;
                            bSwapped=1;
                        }
                        else
                            bSwapped=0;

                        // write out endpoints
                        for (k=0;k<2;k++)
                        {
                            //_ASSERTE(a.endPoint[k]>=0&&a.endPoint[k]<=255);
                            // *lpDstBuf++=(unsigned char)a.endPoint[k];
                            //gwrite(g, &a.endPoint[k], 1);
                            *lpDstBuf++ = (unsigned char)(a.endPoint[k]);
                        }

                        // handle indices
                        //memset(lpDstBuf,0,6);
                        val=0;
                        k=0;
                        // go thru indices and pack them...
                        // note packing is done 3 bytes (24 bits) at a time
                        // which correspond to 8 texels or 2 4-texel rows at a time
                        for (iy=0;iy<4;iy++)
                        {
                            for (ix=0;ix<4;ix++,k++)
                            {
                                val>>=3;
                                if (index[k]>=0)
                                {
                                    int curIndex;

                                    curIndex=a.index[index[k]];
                                    //_ASSERTE(curIndex>=0&&curIndex<=7);
                                    // handle indices swapping if needed
                                    // mapping as follows if swapped:
                                    // outLevel==8 : 0->1, 1->0, 2->7, 3->6, 4->5, 5->4, 6->3, 7->2
                                    // outLevel==6 : 0->1, 1->0, 2->5, 3->4, 4->3, 5->2, 6->6, 7->7
                                    if (bSwapped) {
                                        if (a.outLevel==8) {
                                            curIndex = (curIndex>1) ? (9-curIndex) : (curIndex==0?1:0);
                                        } else if (a.outLevel==6) {
                                            curIndex = (curIndex>5) ? curIndex : ( (curIndex>1)?(7-curIndex):(curIndex==0?1:0) );
                                        }

                                    }

                                    //_ASSERTE(curIndex>=0&&curIndex<=7);
                                    // pack it...
                                    val|=curIndex<<21;
                                }
                            }

                            // check to see if we've finished 2 texel rows...
                            // equivalent to having packed 3 bytes (24 bits)...
                            if (iy&1)
                            {
                                // write out packed indices
                                //*(unsigned *)(lpDstBuf)|=val;
                                // realign boundry
                                gputi(lpDstBuf, val, 3);
                                lpDstBuf+=3;
                                //buf[0] = val>>24;
                                //buf[1] = val>>16;
                                //buf[2] = val>>8;
                                //gwrite(g, buf, 3);
                                val=0;
                            }
                        }
                    }
                    break;
                default:
                    //_ASSERTE(0);
                    break;
            }

            // Encode the RGB stuff second since this is the way DX7 does it.
            {
                // non-palettized image
                int ix,iy;                    // inner block pixel loop variables
                ARGB *lpCur;                // current pixel

                lpCur=(ARGB*)lpSrcCur;
                // encode RGB stuff first...
                switch(dwEncodeType&_S3TC_ENCODE_RGB_MASK)
                {
                    case S3TC_ENCODE_RGB_COLOR_KEY:
                    case S3TC_ENCODE_RGB_FULL:
                        // go thru all pixels in the block and set up the RGBBlock structure b
                        for (k=0,iy=0;iy<4;iy++,lpCur+=NextLine)
                            for (ix=0;ix<4;ix++,lpCur+=1)
                                if (ix<blockWidth&&iy<blockHeight)
                                {
                                    double * pChannel;
                                    index[k++]=b.n;
                                    pChannel=b.colorChannel[b.n++];

                                    *pChannel++ = lpCur->r * ONEOVERMAX;
                                    *pChannel++ = lpCur->g * ONEOVERMAX;
                                    *pChannel++ = lpCur->b * ONEOVERMAX;
                                }
                                else
                                    index[k++]=-1;            // non-quantizable if outside image
                        break;
                    case S3TC_ENCODE_RGB_ALPHA_COMPARE:
                        // go thru all pixels in the block and set up the RGBBlock structure b
                        for (k=0,iy=0;iy<4;iy++,lpCur+=NextLine)
                            for (ix=0;ix<4;ix++,lpCur+=1)
                                if (ix<blockWidth&&iy<blockHeight)
                                {
                                    double * pChannel;
                                    //int    i;

                                    // alpha compare
                                    //if ((curVal&lpSrc->ddpfPixelFormat.dwRGBAlphaBitMask)>aRef)
                                    //{
                                    if (lpCur->a > 64)
                                    {
                                        index[k++]=b.n;
                                        pChannel=b.colorChannel[b.n++];

                                        *pChannel++ = lpCur->r * ONEOVERMAX;
                                        *pChannel++ = lpCur->g * ONEOVERMAX;
                                        *pChannel++ = lpCur->b * ONEOVERMAX;
                                    }
                                    else
                                        index[k++]=-1;        // non-quantizable if alpha compare
                                }
                                else
                                    index[k++]=-1;            // non-quantizable if outside image
                        break;
                    default:
                        //_ASSERTE(0);
                        break;
                }
            }


            //_ASSERTE(k==16);

            // input quantization is set to 3 if there are non-quantizable texels
            // so a spare flag can be used to indicate such, else set to 4.
            b.inLevel=b.n<blockWidth*blockHeight?3:4;

            // check to see if there are no points to be quantized...
            if (b.n==0) {
                // zero out endpoints (they are invalid)
                for (k=0;k<2;k++)
                    b.endPoint[k][0]=b.endPoint[k][1]=b.endPoint[k][2]=0;
                //for (k=0;k<16;k++)
                //  _ASSERTE(index[k]==-1);
            } else
                // !!! ENTRY INTO S3TC QUANTIZER !!!
                CodeRGBBlock(&b);


            // retrieve endpoints
            if (rgb555) // non standard rgb555
            {
                for (k=0;k<2;k++)
                {
                    endPt[k]=(unsigned short)(0x8000|(b.endPoint[k][2]<<10)|((b.endPoint[k][1]>>1)<<5)|b.endPoint[k][0]);
                }
            }
            else // rgb565 (standard)
            {
                endPt[0]=(unsigned short)((b.endPoint[0][0]<<11)|(b.endPoint[0][1]<<5)|b.endPoint[0][2]);
                endPt[1]=(unsigned short)((b.endPoint[1][0]<<11)|(b.endPoint[1][1]<<5)|b.endPoint[1][2]);
            }


            // check if end-points are equal, if so, default to 3 points,
            // second endpoint invalid, so increment to indicate 3 points.
            // force all indices to endpoint 0.
            if (endPt[0]==endPt[1]) {
                endPt[1]++;
                memset(b.index,0,sizeof(int)*b.n);
                b.outLevel=3;
            }

            // swap needed?
            if ((endPt[0]>endPt[1])==(b.outLevel==3))
            {
                unsigned short sw=endPt[0]; endPt[0]=endPt[1]; endPt[1]=sw;
                bSwapped=1;
            }
            else
                bSwapped=0;

            // write out end-points
            for (k=0;k<2;k++/*,lpDstBuf+=2*/)
            {
                //*(unsigned short *)lpDstBuf=endPt[k];
                //gputi(buf, endPt[k], 2);
                //gwrite(g, buf, 2);
// SWAP ORDER HERE FOR GSH
                if(BigEndian)
                    gputm(lpDstBuf, endPt[k], 2);
                else
                    gputi(lpDstBuf, endPt[k], 2);
                lpDstBuf+=2;
            }
// CHANGE ORDER HERE FOR GSH.
            // go thru indices and pack them...
            if(!BigEndian)
            {
                for (k=15;k>=0;k--)
                {
                    dwIndex<<=2;
                    // check if texel is not quantizable, if so, set index to 3
                    if (index[k]<0) {
                        // _ASSERTE((b.inLevel==3)&&(b.outLevel==3));  <- bad assertion, taken out 4/9/98
                        dwIndex|=3;
                    } else {
                        //_ASSERTE(b.index[index[k]]>=0&&b.index[index[k]]<b.outLevel);
                        // pack indices
                        dwIndex|=b.index[index[k]];
                        // if swapped, changed the index
                        if (bSwapped)
                        {
                            if (b.outLevel==4)
                                dwIndex^=1;
                            else
                                dwIndex^=((dwIndex&2)>>1)^1;
                        }
                    }
                }
            }
            else   // for bigendian format we have to reverse the bits in each row
            {      // 1  2  3  4        4  3  2  1
                   // 5  6  7  8    ->  8  7  6  5
                   // 9  10 11 12       12 11 10 9
                   // 13 14 15 16       16 15 14 13
                int i = 0;
                for (k=12;k>=0;k-=4)   // move down by groupings of 4.
                {
                    for(i = 0; i < 4; i++)
                    {
                        dwIndex<<=2;
                        // check if texel is not quantizable, if so, set index to 3
                        if (index[k + i]<0)
                        {
                            // _ASSERTE((b.inLevel==3)&&(b.outLevel==3));  <- bad assertion, taken out 4/9/98
                            dwIndex|=3;
                        }
                        else
                        {
                            //_ASSERTE(b.index[index[k+i]]>=0&&b.index[index[k+i]]<b.outLevel);
                            // pack indices
                            dwIndex|=b.index[index[k+i]];
                            // if swapped, changed the index
                            if (bSwapped)
                            {
                                if (b.outLevel==4)
                                    dwIndex^=1;
                                else
                                    dwIndex^=((dwIndex&2)>>1)^1;
                            }
                        }
                    } // end for i.
                }// end for k.
            }// end else bigendian.
            // write out indices
            //*(unsigned *)lpDstBuf=dwIndex;
            //lpDstBuf+=4;
            //gputi(buf, dwIndex, 4);
            //gwrite(g, buf, 4);
            gputi(lpDstBuf, dwIndex, 4);
            lpDstBuf+=4;

        }
    }

    return (unsigned int)((lpDstBuf - lpDst));
}

/****************************************************************/
/*  Internal Functions                                          */
/****************************************************************/

#define S3TCPUTFIELD(field,data) gputm((void *)&field,(unsigned int)(data),sizeof(field))

/****************************************************************/
/*  Encode Function                                             */
/****************************************************************/

int GCALL S3TC_encode(void *compressed, const void *uncompressed, int numbytes, int *options)
{
    int width= 0;
    int height= 0;
    int rowbytes= 0;
    int encodetype=S3TC_ENCODE_RGB_FULL;
    int mode= 0;
    int rgb555=0;
    int BigEndian = 0;
    int bigSize   = 0;
    float weight[] = {0.3f,0.4f,0.3f};
    int compressedsize=0;
    S3TCHEADER *header = (S3TCHEADER*)compressed;
    BIGS3TCHEADER *Bigheader = (BIGS3TCHEADER*)compressed;          // just in case my image size is greater
                                                                  // than 16MB (3 bytes S3TCHEADER SIZE FIELD)
    if(numbytes > 16000000)      // bigger than 16BM then have to use alternate header.
        bigSize = 1;

    if (options)
    {
        encodetype = options[0];
    }

/*  0 for no alpha,                                                 */
/*  1 for explicit alpha,                                           */
/*  2 for interpolated alpha.                                       */
/*  4 for bgr555 little endian                                      */
/*  8 for gsh bigendian                                             */
    mode = 0;
    if (encodetype&S3TC_ENCODE_ALPHA_EXPLICIT)
        mode = S3TC_MODE_ALPHA_EXPLICIT; // 1 = S3TC3
    else if (encodetype&S3TC_ENCODE_ALPHA_INTERPOLATED)
        mode = S3TC_ENCODE_ALPHA_INTERPOLATED;
    else
        mode = S3TC_MODE_RGB;
    if (encodetype & S3TC_ENCODE_RGB_555)
    {
        rgb555 = 1;
        mode |= S3TC_MODE_RGB555;
    }
    if(encodetype & S3TC_S3TC1B_ENCODEOPTION /*S3TC_S3TC1B_CODE*/)
    {
        BigEndian = 1;                      // set the bigendian flag.
        mode = S3TC_MODE_RGB;               // only S3TC1 mode
    }
    width = 4;
    rowbytes = 16;
    height = numbytes/rowbytes;
    if(encodetype & S3TC_ENCODE_ALPHA_EXPLICIT)
    {
        if(!bigSize)
            S3TCPUTFIELD(header->code, S3TC_S3TC3_CODE);
        else
            S3TCPUTFIELD(Bigheader->code, S3TC_BIGS3TC3_CODE);
    }
    else if(encodetype & S3TC_ENCODE_ALPHA_INTERPOLATED)
    {
        if(!bigSize)
            S3TCPUTFIELD(header->code, S3TC_S3TC5_CODE);
        else
            S3TCPUTFIELD(Bigheader->code, S3TC_BIGS3TC5_CODE);

    }
    else if(encodetype & S3TC_S3TC1B_ENCODEOPTION)
    {
        if(!bigSize)
        {
            S3TCPUTFIELD(header->code, S3TC_S3TC1B_CODE);          // we have set the BigEndian flag so mark as Bigendian GSH style S3TC.
            encodetype = encodetype&~S3TC_S3TC1B_ENCODEOPTION;     // just bookeeping so get rid of here so it doesn't interfere
        }                                                         // with compression to S3TC1.
        else
        {
            S3TCPUTFIELD(Bigheader->code, S3TC_BIGS3TC1B_CODE);    // we have set the BigEndian flag so mark as Bigendian GSH style S3TC.
            encodetype = encodetype&~S3TC_S3TC1B_ENCODEOPTION;     // just bookeeping so get rid of here so it doesn't interfere
        }                                                         // with compression to S3TC1.
    }
    else
    {
        if(!bigSize)
            S3TCPUTFIELD(header->code, S3TC_S3TC1_CODE);
        else
            S3TCPUTFIELD(Bigheader->code, S3TC_BIGS3TC1_CODE);
    }
    //S3TCPUTFIELD(header->code, (encodetype & S3TC_ENCODE_ALPHA_EXPLICIT) ? S3TC_S3TC3_CODE : S3TC_S3TC1_CODE);
    if(!bigSize)
    {
        S3TCPUTFIELD(header->size, numbytes);
        S3TCPUTFIELD(header->mode, mode);
    }
    else
    {
        S3TCPUTFIELD(Bigheader->size, numbytes);
        S3TCPUTFIELD(Bigheader->mode, mode);
    }



    compressedsize = S3TCencode(width, height, rowbytes, (unsigned char*)uncompressed, (unsigned char *)compressed + sizeof(S3TCHEADER), encodetype&~S3TC_ENCODE_RGB_555, weight, rgb555,BigEndian);

    return compressedsize + 8; // .NET seems to calculating 8 bytes for this header size sizeof(S3TCHEADER);
}

#undef S3TCPUTFIELD

#endif

