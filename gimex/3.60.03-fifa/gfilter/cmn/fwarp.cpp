// (c) 1996-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*                Warp - Warp Image                                 */
/*                                                                  */
/*                        by Ian Davids, EAC                        */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History                                        */
/* ------- ------ -- -------                                        */
/*    1.00 010124 ID original from alex warp                        */
/*    1.01 010131 ID added waves warp and control line warps.       */
/*    1.02 010220 ID added use of internal fmod                     */
/*    1.03 010313 ID fixed the use of non static functions          */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* check out www.axlrosen.net                                       */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include "fwarp.h"
#include <string.h>
#include <math.h>


#if defined(__APPLE__)
inline float sqrtf(float v)
{
    return (float) sqrt((double) v);
}
#endif


#define NUMOPTSCONTLINES 13           // number of control points for a minimal control line warp.
#define NUMOPTSWAVES  3               // number of options required to do a wave warp.
#define AUGMENTEDWARP 8               // do a warp and fill or not with color
#define FILL  1                       // fill option
#define NOOP  0                       // default
#define CLAMP 2                       // same as default for now.
#define VERTICAL    1                 // decision contants for waves.
#define HORIZONTAL  2
#define PI          3.141592654


// info on which warper type to use.
#define TYPEDEFAULT     1
#define TYPEWAVES       2
#define TYPECONTLINES   3
#define TYPECONTLINESBI 4

// helper structures. All from Alex Rosen's design.
struct Rectangle
{
    float x;
    float y;
    float width;
    float height;
};

struct Point
{
    float x;
    float y;
};

#define gwfmod(a,b) (a - ((int)(a/b))*b)       //SGI does not support fmod so create my own.

//------------------------------------------------------------------------------------------------------
// Internal Functions...
//------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------
// helper function for waves.
static int WarpRound(double x)
{
    int tx = (int)x;
    double t = gwfmod((double)tx,x);
    if(t >= .5)
        tx = (int)ceil(x);
    else
        tx = (int)x;
    return(tx);
}


//------------------------------------------------------------------------------------------------------
// calculate the UV offset from the line segment P Q.
static struct Point* CalculateUV(struct Point* P,struct Point *Q,struct Point* X)
{
    struct Point *UV;
    struct Point PQ;
    struct Point PX;
    double s = (Q->x - P->x);       // get the new vector PQ in s and m for calculations later.
    double m = (Q->y - P->y);
    double magPQ = sqrt(s*s + m*m);
    PQ.x = (float)s;
    PQ.y = (float)m;
    PX.x = X->x - P->x;
    PX.y = X->y - P->y;
    UV  = (struct Point *)galloc(sizeof(struct Point));
    /*D or V*/UV->y = (float)(PX.x*PQ.y + PX.y*(-PQ.x))/(float)magPQ;
    /*F or U*/UV->x = (float)(PX.x*PQ.x + PX.y*PQ.y) / (float)(magPQ*magPQ);
    return(UV);
}


//------------------------------------------------------------------------------------------------------
// Calculate the point in the source data from the UV offset.
static void CalculateIndexXY(struct Point * X,struct Point * P, struct Point*Q, struct Point * UV)
{
    struct Point PQ;
    double magPQ;
    PQ.x = Q->x - P->x;
    PQ.y = Q->y - P->y;
    magPQ = sqrt(PQ.x*PQ.x + PQ.y*PQ.y);
    X->x = (float)(P->x + UV->x*PQ.x + UV->y*(PQ.y)/(float)magPQ);
    X->y = (float)(P->y + UV->x*PQ.y + UV->y*(-PQ.x)/(float)magPQ);
}

//------------------------------------------------------------------------------------------------------
// calculate the weight that this line will contribute to the warp at this point in space.
// based on the length of the line and the V cood. ie the distance of the point to the line.
static double CalculateWeight(struct Point *P,struct Point *Q,struct Point *UV)
{
    double s = (Q->x - P->x);
    double m = (Q->y - P->y);
    double weight = s*s + m*m;
    double t = UV->y*UV->y;
    double dist = t*t;
    weight *= weight;                      // square the length.
    weight /= (dist == 0)?(1):(dist);      // divide by either the distance v to the fourth or the nearset we can.
    weight *= weight;                      // square the whole thing.

    return weight;
}


//------------------------------------------------------------------------------------------------------
// Warp image by a list of control line pairs. This allows more flexible control over the warping of
// the image
static void WarpImageByContLines(char* dest,char *src,int mWidth, int mHeight, int numlines, struct Point * LineArray)
{
    struct Point DSUM;
    struct Point *UV;
    struct Point Currentxy;
    struct Point Newxy;
    double weightsum = 0;
    double weight = 0;
    double xi,yi;
    int index = 0;
    int index2 = 0;
    int ind = 0;
    int W = mWidth*4;
    int index3 = 0;
    int row,col,i;

    for(row = 0; row < mHeight; row++)
    {
        ind = row*W;
        for(col = 0; col < mWidth; col++)
        {

            DSUM.x = 0;
            DSUM.y = 0;
            weightsum = 0;
            for(i = 0; i < numlines; i++)
            {
              index3 = i*4;
              Currentxy.x = (float)col;
              Currentxy.y = (float)row;
              UV = CalculateUV(&LineArray[index3],&LineArray[index3 +1],&Currentxy);
              CalculateIndexXY(&Newxy,&LineArray[index3+2],&LineArray[index3+3],UV);
              Newxy.x = Newxy.x - Currentxy.x;
              Newxy.y = Newxy.y - Currentxy.y;
              weight = CalculateWeight(&LineArray[index3+2],&LineArray[index3+3],UV);
              //weight = 1;
              DSUM.x = Newxy.x*(float)weight;
              DSUM.y = Newxy.y*(float)weight;
              weightsum += weight;
              gfree(UV);
            }
            yi = (double)row + DSUM.y/weightsum;
            xi = (double)col + DSUM.x/weightsum;

            if(xi > mWidth)
                xi = mWidth-1;
            if(xi < 0)
                xi = 0;
            if(yi > mHeight)
                yi = mHeight-1;
            if(yi < 0)
                yi = 0;
            index  = ind + col*4;
            index2 = (int)yi*W + (int)xi*4;
            // must do bilinear interpolation later!
            dest[index + 0] = src[index2 + 0];
            dest[index + 1] = src[index2 + 1];
            dest[index + 2] = src[index2 + 2];
            dest[index + 3] = src[index2 + 3];
        }
    }

}

//------------------------------------------------------------------------------------------------------
// Warp image by a list of control line pairs. This allows more flexible control over the warping of
// the image
// NOTE THIS FUNCTION IS REAL SLOW!!!! it uses bilinear interpolation!
static void WarpImageByContLinesBi(char* dest,char *src,int mWidth, int mHeight, int numlines, struct Point * LineArray)
{
    struct Point DSUM;
    struct Point *UV;
    struct Point Currentxy;
    struct Point Newxy;
    double weightsum = 0;
    double weight = 0;
    double xi,yi;
    int index = 0;
    int index2 = 0;
    int ind = 0;
    int W = mWidth*4;
    int index3 = 0;
    int row,col,i;
    double dx = 0;
    double dy = 0;
    double idx = 0;
    double idy = 0;
    char one,two,three,four;
    char val1,val2,val;
    int i1,i2,i3,i4;

    for(row = 0; row < mHeight; row++)                 // go through image and....
    {
        ind = row*W;
        for(col = 0; col < mWidth; col++)
        {

            DSUM.x = 0;
            DSUM.y = 0;
            weightsum = 0;
            for(i = 0; i < numlines; i++)             // for each control line...
            {
              index3 = i*4;
              Currentxy.x = (float)col;
              Currentxy.y = (float)row;               // for the current point..
              UV = CalculateUV(&LineArray[index3],&LineArray[index3 +1],&Currentxy);  // calculate the inverse mapping.
              CalculateIndexXY(&Newxy,&LineArray[index3+2],&LineArray[index3+3],UV);  // use this to determine point in old picture.
              Newxy.x = Newxy.x - Currentxy.x;                                        // calculate my difference.
              Newxy.y = Newxy.y - Currentxy.y;
              weight = CalculateWeight(&LineArray[index3+2],&LineArray[index3+3],UV); // how much does this line contribute to this?
              DSUM.x = Newxy.x*(float)weight;
              DSUM.y = Newxy.y*(float)weight;                                                 // calculate my summed difference vector.
              weightsum += weight;
              gfree(UV);                                                              // clean up.
            }
            yi = (double)row + DSUM.y/weightsum;        // get x,y in old picture for row,col.
            xi = (double)col + DSUM.x/weightsum;

            if(xi > mWidth)                             // clip the offsets.
                xi = mWidth-1;
            if(xi < 0)
                xi = 0;
            if(yi > mHeight)
                yi = mHeight-1;
            if(yi < 0)
                yi = 0;

            index  = ind + col*4;                      // find index into new picture.
            index2 = (int)yi;//(int)yi*W + (int)xi*4;
            index3 = (int)xi;                          // calculate the indexs for bilinear interp.
            if (index3 == 0)
                dx = 0;
            else
                dx = gwfmod(xi,index3);                // get my inbetweener.
            if(index2 == 0)
                dy = 0;
            else
                dy = gwfmod(yi,index2);

            i1 = (int)yi*W;                            // calculate my offsets of all four corner points.
            i2 = (int)ceil(yi)*W;
            i3 = (int)xi*4;
            i4 = (int)ceil(xi)*4;
            index2 = i1 + i3;
            index3 = i2 + i3;
            i3 = i2 + i4;
            i2 = i1 + i4;
            idx = 1.0-dx;
            idy = 1.0-dy;
            // for each colour channel do biliear interpolation.
            one   = src[index2 + 0];  // go in a clockwise direction and get colour at points.
            two   = src[i2     + 0];
            three = src[i3     + 0];
            four  = src[index3 + 0];
            val1 = (char)((float)((float)one*(float)(idx))   + (float)((float)two*(float)(dx)));
            val2 = (char)((float)((float)three*(float)(idx)) + (float)((float)four*(float)(dx)));
            val =  (char)((float)((float)val1*(float)(idy))  + (float)((float)val2*(float)(dy)));
            dest[index + 0] = val;//src[(int)yi*W + (int)xi*4 + 0];

            one   = src[index2 + 1];
            two   = src[i2     + 1];
            three = src[i3     + 1];
            four  = src[index3 + 1];
            val1 = (char)((float)((float)one*(float)(idx))   + (float)((float)two*(float)(dx)));
            val2 = (char)((float)((float)three*(float)(idx)) + (float)((float)four*(float)(dx)));
            val =  (char)((float)((float)val1*(float)(idy))  + (float)((float)val2*(float)(dy)));
            dest[index + 1] = val; //src[index2 + 1];

            one   = src[index2 + 2];
            two   = src[i2     + 2];
            three = src[i3     + 2];
            four  = src[index3 + 2];
            val1 = (char)((float)((float)one*(float)(idx))   + (float)((float)two*(float)(dx)));
            val2 = (char)((float)((float)three*(float)(idx)) + (float)((float)four*(float)(dx)));
            val =  (char)((float)((float)val1*(float)(idy))  + (float)((float)val2*(float)(dy)));
            dest[index + 2] = val;//src[index2 + 2];

            one   = src[index2 + 3];
            two   = src[i2     + 3];
            three = src[i3     + 3];
            four  = src[index3 + 3];
            val1 = (char)((float)((float)one*(float)(idx))   + (float)((float)two*(float)(dx)));
            val2 = (char)((float)((float)three*(float)(idx)) + (float)((float)four*(float)(dx)));
            val =  (char)((float)((float)val1*(float)(idy))  + (float)((float)val2*(float)(dy)));
            dest[index + 3] = val;//src[index2 + 3];
        }
    }

}


//------------------------------------------------------------------------------------------------------
// Make an image wavy.
static void WaveImage(int typewave,char* dest,char *src,int mWidth, int mHeight)
{
    int x = 0;
    int y = 0;
    int xo = 0;
    int yo = 0;
    char *tempdest = (char *)galloc(mWidth*mHeight*4);
    int index = 0;
    int index2 = 0;
    int t = 0;
    int t2 = 0;
    if(typewave & VERTICAL)     // do this on the tempimage so that I can use it as source if I need
    {                           // to do both the vertical and horizontal.
        for(y = 0; y < mHeight; y++)
        {
            t = y*mWidth*4;
            for(x = 0; x < mWidth; x++)
            {
                xo = WarpRound(8.0 * sin(2.0 * PI * y / 128.0));
                index = t + x*4;
                index2 = t + 4*((x + xo + mWidth) % mWidth);
                tempdest[index + 0] = src[index2 + 0];
                tempdest[index + 1] = src[index2 + 1];
                tempdest[index + 2] = src[index2 + 2];
                tempdest[index + 3] = src[index2 + 3];

            }
        }
    }
    else//(typewave & HORIZONTAL)
    {
        for(y = 0; y < mHeight; y++)
        {
            t = mWidth*4;
            t2 = t*y;
            for(x = 0; x < mWidth; x++)
            {
                xo = x;
                yo = WarpRound(8.0 * cos(2.0 * PI * x / 128.0));
                index = t2 + x*4;
                index2 = ((y + yo + mHeight) % mHeight)*t + x*4;
                dest[index + 0] = src[index2 + 0];
                dest[index + 1] = src[index2 + 1];
                dest[index + 2] = src[index2 + 2];
                dest[index + 3] = src[index2 + 3];
            }
        }
        gfree(tempdest);
        return;
    }
    if(typewave & HORIZONTAL)  // well we get to do both vertical and horizontal waves so...
    {                          // do horizontal wave on tempimage.
        for(y = 0; y < mHeight; y++)
        {
            t = mWidth*4;
            t2 = t*y;
            for(x = 0; x < mWidth; x++)
            {
                xo = x;
                yo = WarpRound(8.0 * cos(2.0 * PI * x / 128.0));
                index = t2 + x*4;
                index2 = ((y + yo + mHeight) % mHeight)*t + x*4;
                dest[index + 0] = tempdest[index2 + 0];
                dest[index + 1] = tempdest[index2 + 1];
                dest[index + 2] = tempdest[index2 + 2];
                dest[index + 3] = tempdest[index2 + 3];
            }
        }
        gfree(tempdest);
        return;
    }
    memcpy(dest,tempdest,mWidth*mHeight*4);
    gfree(tempdest);
    return;
}


//------------------------------------------------------------------------------------------------------
// warp a quad. to a square... oooh special.
static void WarpRegion(struct Rectangle *fromRect, struct Point *nw,
                       struct Point *ne, struct Point *sw,
                       struct Point *se, int mWidth,
                       int mHeight, char * mFromPixels, char * mToPixels)
{

    int dx = (int)fromRect->width;
    int dy = (int)fromRect->height;
    int i,j;
    double invDX = 1.0/(double)dx;
    double invDY = 1.0/(double)dy;
    for(i = 0; i < dx;i++)
    {
        double accumeinc = i*invDX;
        double x1 = ne->x + ( nw->x - ne->x ) * accumeinc;  // calculate my stepping start pos.
        double y1 = ne->y + ( nw->y - ne->y ) * accumeinc;
        double x2 = se->x + ( sw->x - se->x ) * accumeinc;  // warp line endpoint.
        double y2 = se->y + ( sw->y - se->y ) * accumeinc;

        double xin = x1;
        double yin = y1;
        double dxin = ( x2 - x1 ) * invDY;                  // increment amount along warp line.
        double dyin = ( y2 - y1 ) * invDY;
        int toPixel = (int)(fromRect->x + i + fromRect->y * mWidth);

        for(j = 0; j < dy; j++)                             // this will screw up on 8 bpp bitmaps. fix later.
        {
            if(xin < 0)  xin = 0;
            if(xin >= mWidth) xin = mWidth - 1;             // clip stepper.
            if(yin < 0) yin = 0;
            if(yin >= mHeight) yin = mHeight - 1;
            mToPixels[toPixel*4+0] = mFromPixels[ 4*((int)xin + (int)yin*mWidth)+0]; // warp.
            mToPixels[toPixel*4+1] = mFromPixels[ 4*((int)xin + (int)yin*mWidth)+1];
            mToPixels[toPixel*4+2] = mFromPixels[ 4*((int)xin + (int)yin*mWidth)+2];
            mToPixels[toPixel*4+3] = mFromPixels[ 4*((int)xin + (int)yin*mWidth)+3];

            xin += dxin;                                    // go to next pixel along warp line.
            yin += dyin;
            toPixel += mWidth;
        }
    }
}

//------------------------------------------------------------------------------------------------------
static void ClipRect(struct Rectangle *r, int w, int h)
{
    if(r->x < 0){r->width  += r->x; r->x = 0;}
    if(r->y < 0){r->height += r->y; r->y = 0;}

    if(r->x + r->width >= w) r->width = w - r->x - 1;
    if(r->y + r->height >= h) r->height = h - r->y - 1;
}

//------------------------------------------------------------------------------------------------------
static void SetRect(struct Rectangle *r, int left, int top, int right, int bottom)
{
    r->x = (float)left; r->y = (float)top; r->width = (float)(right - left); r->height = (float)(bottom - top);
}

//------------------------------------------------------------------------------------------------------
static void SetPt(struct Point *pt, float x, float y)
{
    pt->x = x;
    pt->y = y;
}


//------------------------------------------------------------------------------------------------------
static void WarpPixels(GINFO *ginfo,char *mFromPixels,char *mToPixels,struct Point *mFromPoint,struct Point *mToPoint)
{
    int dx = (int)(mToPoint->x - mFromPoint->x);
    int dy = (int)(mToPoint->y - mFromPoint->y);
    int dist = (int)(sqrtf(float(dx*dx+dy*dy)*2));
    int mHeight = ginfo->height;
    int mWidth  = ginfo->width;
    struct Rectangle r;
    struct Point ne;
    struct Point nw;
    struct Point se;
    struct Point sw;
    //----------------
    memcpy(mToPixels,mFromPixels,(mWidth*mHeight*4));
    //----------------

    if(dist == 0) return;
        // warp northeast quadrant.
    SetRect(&r,(int)(mFromPoint->x - dist),(int)(mFromPoint->y - dist),(int)mFromPoint->x,(int)mFromPoint->y);
    ClipRect( &r, mWidth, mHeight );
    SetPt( &ne, r.x, r.y );
    SetPt( &nw, r.x+r.width, r.y );
    SetPt( &se, r.x, r.y+r.height );
    SetPt( &sw, mToPoint->x, mToPoint->y );
    //printf("rect: %f,%f,%f,%f, ne: %f,%f,nw: %f,%f, se: %f,%f, sw: %f,%f\n",r.x,r.y,r.width,r.height,ne.x,ne.y,nw.x,nw.y,se.x,se.y,sw.x,sw.y);
    WarpRegion( &r, &nw, &ne, &sw, &se, mWidth,mHeight,mFromPixels,mToPixels);

        // warp nortwest quadrant
    SetRect( &r, (int)mFromPoint->x, (int)(mFromPoint->y - dist), (int)(mFromPoint->x + dist), (int)(mFromPoint->y) );
    ClipRect( &r, mWidth, mHeight );
    SetPt( &ne, r.x, r.y );
    SetPt( &nw, r.x+r.width, r.y );
    SetPt( &se, mToPoint->x, mToPoint->y );
    SetPt( &sw, r.x+r.width, r.y+r.height );
    WarpRegion( &r, &nw, &ne, &sw, &se, mWidth,mHeight,mFromPixels,mToPixels);

        // warp southwest quadrant
    SetRect( &r, (int)(mFromPoint->x - dist), (int)mFromPoint->y, (int)mFromPoint->x, (int)(mFromPoint->y + dist) );
    ClipRect( &r, mWidth, mHeight );
    SetPt( &ne, r.x, r.y );
    SetPt( &nw, mToPoint->x, mToPoint->y );
    SetPt( &se, r.x, r.y+r.height );
    SetPt( &sw, r.x+r.width, r.y+r.height );
    WarpRegion( &r, &nw, &ne, &sw, &se, mWidth,mHeight,mFromPixels,mToPixels);

        // warp southeast quadrant
    SetRect( &r, (int)mFromPoint->x, (int)mFromPoint->y, (int)(mFromPoint->x + dist), (int)(mFromPoint->y + dist) );
    ClipRect( &r, mWidth, mHeight );
    SetPt( &ne, mToPoint->x, mToPoint->y );
    SetPt( &nw, r.x+r.width, r.y );
    SetPt( &se, r.x, r.y+r.height );
    SetPt( &sw, r.x+r.width, r.y+r.height );
    WarpRegion( &r, &nw, &ne, &sw, &se, mWidth,mHeight,mFromPixels,mToPixels);

}





//------------------------------------------------------------------------------------------------------
// Interface Functions...
//------------------------------------------------------------------------------------------------------

FPUBLIC FABOUT* aboutwarp(void)
{
    FABOUT* about = (FABOUT*) galloc(sizeof(FABOUT));
    if (about)
    {
        memset(about, 0, sizeof(FABOUT));
        about->signature =          GMAKEID('W','A','R','P');
        about->size =               sizeof(FABOUT);
        about->version =            GFILTER_VERSION;
        about->filter8 =            0;
        about->filter32 =           1;
        about->minbitmaps =         1;
        about->maxbitmaps =         1;
        about->filterclass =        FCLASS_GENERAL;
        strcpy(about->authorstr,    "Ian Davids");
        strcpy(about->versionstr,   "1.01");
        strcpy(about->namestr,      "Warp");
        strcpy(about->longnamestr,  "Warp Image along control lines");
        about->numoptions =         13;
        about->option[0].type =            FOP_INT;
        about->option[0].min.i =           1;
        about->option[0].max.i =           4;
        about->option[0].def.i =           0;
        strcpy(about->option[0].name,      "Type:Def/Wave/ContLines");
        about->option[1].type =            FOP_INT;
        about->option[1].min.i =           1;
        about->option[1].max.i =           2;
        about->option[1].def.i =           0;
        strcpy(about->option[1].name,      "Clamp X/VertWave/NA");
        about->option[2].type =            FOP_INT;
        about->option[2].min.i =           1;
        about->option[2].max.i =           2;
        about->option[2].def.i =           0;
        strcpy(about->option[2].name,      "Clamp Y/HorzWave/NA");
        about->option[3].type =            FOP_INT;
        about->option[3].min.i =           0;
        about->option[3].max.i =           2000;
        about->option[3].def.i =           0;
        strcpy(about->option[3].name,      "FillColour/NA/NA");
        about->option[4].type =            FOP_INT;
        about->option[4].min.i =           0;
        about->option[4].max.i =           2000;
        about->option[4].def.i =           0;
        strcpy(about->option[4].name,      "ToPointX/NA/numlinepairs");
        about->option[5].type =            FOP_INT;
        about->option[5].min.i =           0;
        about->option[5].max.i =           2000;
        about->option[5].def.i =           0;
        strcpy(about->option[5].name,      "ToPointY/NA/X1.1");
        about->option[6].type =            FOP_INT;
        about->option[6].min.i =           0;
        about->option[6].max.i =           2000;
        about->option[6].def.i =           0;
        strcpy(about->option[6].name,      "FromPointX/NA/Y1.1");
        about->option[7].type =            FOP_INT;
        about->option[7].min.i =           0;
        about->option[7].max.i =           2000;
        about->option[7].def.i =           0;
        strcpy(about->option[7].name,      "FromPointY/NA/X1.2");
        about->option[8].type =            FOP_INT;
        about->option[8].min.i =           0;
        about->option[8].max.i =           2000;
        about->option[8].def.i =           0;
        strcpy(about->option[8].name,      "NA/NA/Y1.2");
        about->option[9].type =            FOP_INT;
        about->option[9].min.i =           0;
        about->option[9].max.i =           2000;
        about->option[9].def.i =           0;
        strcpy(about->option[9].name,      "NA/NA/X1.3");
        about->option[10].type =            FOP_INT;
        about->option[10].min.i =           0;
        about->option[10].max.i =           2000;
        about->option[10].def.i =           0;
        strcpy(about->option[10].name,      "NA/NA/Y1.3");
        about->option[11].type =            FOP_INT;
        about->option[11].min.i =           0;
        about->option[11].max.i =           2000;
        about->option[11].def.i =           0;
        strcpy(about->option[11].name,      "NA/NA/X1.4");
        about->option[12].type =            FOP_INT;
        about->option[12].min.i =           0;
        about->option[12].max.i =           2000;
        about->option[12].def.i =           0;
        strcpy(about->option[12].name,      "NA/NA/Y1.4");

    }
    return(about);
}

//------------------------------------------------------------------------------------------------------
FPUBLIC int openwarp(FINSTANCE **filterInstance, int numsources, GBITMAP *sources, int numoptions, FOP *options)
{
    int ok=0;
    FINSTANCE* fx=0;

    // allocate instance
    fx = (FINSTANCE*)galloc(sizeof(FINSTANCE));
    if (fx)
    {
        memset(fx, 0, sizeof(FINSTANCE));
        fx->frames      = 1;
        fx->numsources  = numsources;
        fx->source      = sources;
        fx->numoptions  = numoptions;
        fx->option      = options;
        ok = 1;

    }
    *filterInstance = fx;
    return ok;
}



//------------------------------------------------------------------------------------------------------
FPUBLIC GINFO * infowarp(FINSTANCE *fx, int framenum)
{
    GINFO* ginfo = (GINFO*)galloc(fx->source[0].ginfo->size);
    if (ginfo)                                                      // all params the same in most cases.
    {
        memcpy(ginfo, fx->source[0].ginfo, fx->source[0].ginfo->size);
    }
    if(fx->option[0].i == TYPEDEFAULT)                                   // for default type I need to setup for fill.
    {
        if(fx->numoptions >= AUGMENTEDWARP)                              // I need a big enough picture
        {                                                                // to accomodate a warp to any side
            int dx,dy,dist,overflow,temp;                                // if I need to fill... UGH.
            dx = (int)(fx->option[4].i - fx->option[6].i);
            dy = (int)(fx->option[5].i - fx->option[7].i);
            dist = (int)(sqrtf((float)(dx*dx+dy*dy)/2.0f));
            if(fx->option[1].i == FILL)                                  // if I need to fill in X
            {
                overflow = (int)((float)fx->option[6].i - (float)dist);
                temp = ginfo->width;
                if(overflow < 0)
                    ginfo->width += (-overflow);


                overflow = (int)((float)fx->option[6].i + (float)dist);
                if(overflow > temp)
                    ginfo->width += (overflow - temp);

            }
            if(fx->option[2].i == FILL || (fx->option[2].i == FILL && fx->option[1].i == NOOP))
            {                                                            // if I need to fill in Y.
                overflow = (int)((float)fx->option[7].i - (float)dist);
                temp = ginfo->height;
                if(overflow < 0)
                    ginfo->height += (-overflow);

                overflow = (int)((float)fx->option[7].i + (float)dist);
                if(overflow > temp)
                    ginfo->height += (overflow - temp);

            }
        }
    }// if TYPEDEFAULT
    return(ginfo);
}


//------------------------------------------------------------------------------------------------------
FPUBLIC int filterwarp(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes)
{

    struct Point * LinesArray;
    struct Point mFromPoint;
    struct Point mToPoint;
    char *tempimage;
    char *d;
    int bpp = ginfo->bpp;
    int fillc;
    int offsetx,offsety,i,row,col;
    int numberoflinepairs;

    if(fx->option[0].i == TYPEDEFAULT)
    {
        if(fx->numoptions >= AUGMENTEDWARP)                         // return if not enough params.
        {
            mToPoint.x = (float)fx->option[4].i;
            mToPoint.y = (float)fx->option[5].i;
            mFromPoint.x   = (float)fx->option[6].i;
            mFromPoint.y   = (float)fx->option[7].i;

            if(fx->option[1].i == FILL || fx->option[2].i == FILL)  // if either of the items should fill
            {                                                       // then expand the picture size.
                fillc = fx->option[3].i;
                offsetx = (int)(((float)ginfo->width - (float)fx->source[0].ginfo->width)/2.0);
                offsety = (int)(((float)ginfo->height- (float)fx->source[0].ginfo->height)/2.0);
                if(offsetx != 0 || offsety != 0)  // we need to copy over image into bigger canvas.
                {
                    if (bpp==8)  // if 8 bit do this.
                    {
                        tempimage = (char *)galloc(ginfo->height*ginfo->width);
                        d = tempimage;
                        for(i = 0; i < ginfo->width*ginfo->height;i++)   // fill temp buffer with color
                            *d++ = (unsigned char)fillc;

                        d = tempimage;
                        for(row = offsety; row < (ginfo->height - offsety); row++)  // copy over image.
                        {
                            for(col = offsetx; col < (ginfo->width - offsetx); col++)
                            {
                                d[row*ginfo->width + col] = (((char*)(fx->source[0].image))[(row - offsety)*(fx->source[0].ginfo->width) + (col-offsetx)]);
                            }
                        }
                    }
                    else  // 32 bit so calc rgba values and fill the new canvas.
                    {
                        char alpha = (unsigned char)((fillc>>24)&0x000000FF);                           // strip out colors channels.
                        char r     = (unsigned char)((fillc>>16)&0x000000FF);
                        char g     = (unsigned char)((fillc>>8 )&0x000000FF);
                        char b     = (unsigned char)((fillc>>0 )&0x000000FF);
                        int index = 0;
                        int index2 = 0;
                        int id = 0;
                        int id2 = 0;
                        tempimage = (char *)galloc(ginfo->height*ginfo->width*4);
                        d = tempimage;

                        for(row = 0; row < ginfo->height; row++)
                        {

                            index = row*ginfo->width*4;                                 // precalc indexes.
                            index2 = (row - offsety)*fx->source[0].ginfo->width*4;

                            for(col = 0; col < ginfo->width; col++)
                            {
                                id  = index + col*4;
                                if((row >= offsety && row < ginfo->height - offsety) &&  // if in center of image
                                (col >= offsetx && col < ginfo->width - offsetx))
                                { // copy over picture.
                                    id2 = index2 + (col - offsetx)*4;
                                    d[id + 0] = (((char*)(fx->source[0].image))[id2 + 0]);
                                    d[id + 1] = (((char*)(fx->source[0].image))[id2 + 1]);
                                    d[id + 2] = (((char*)(fx->source[0].image))[id2 + 2]);
                                    d[id + 3] = (((char*)(fx->source[0].image))[id2 + 3]);
                                }
                                else // fill with pad
                                {
                                    d[id + 3] = alpha;
                                    d[id + 2] = r;
                                    d[id + 1] = g;
                                    d[id + 0] = b;
                                }

                            }
                        }

                    }// else bpp
                mToPoint.x += offsetx;  // set the offsets so we warp actual picture not pad.
                mToPoint.y += offsety;
                mFromPoint.x   += offsetx;
                mFromPoint.y   += offsety;
                WarpPixels(ginfo,tempimage,dest,&mFromPoint,&mToPoint); // warp from temp buffer
                gfree(tempimage);                                       // clean up new image in dest.
                return(1);
                } // if offsets != 0
            }// if FILL
        }//if AUGM.
        else
            return(0);
        WarpPixels(ginfo,(char *)fx->source[0].image,dest,&mFromPoint,&mToPoint); // otherwise just do the regular warp.
        return(1);
    }// if TYPEDEFAULT

    // do a wave warp on the image possible later extension to controlled wave warp.
    else if(fx->option[0].i == TYPEWAVES)
    {
        if(fx->numoptions >= NUMOPTSWAVES)
        {
            int typewave = 0;
            typewave |= (fx->option[1].i)?VERTICAL:0;
            typewave |= (fx->option[2].i)?HORIZONTAL:0;
            WaveImage(typewave,dest,(char *)fx->source[0].image,ginfo->width,ginfo->height);
            return(1);
        }
        else
            return(0);
    }
    else if(fx->option[0].i == TYPECONTLINES || fx->option[0].i == TYPECONTLINESBI)
    {
        if(fx->numoptions >= NUMOPTSCONTLINES)
        {
            int opter;
            int opter2;
            numberoflinepairs = fx->option[4].i;
            LinesArray = (struct Point *)galloc(sizeof(struct Point)*numberoflinepairs*4);
            for(i = 0; i < numberoflinepairs;i++)
            {
                opter = i*4;
                opter2 = i*8 + 5;
                LinesArray[opter].x = (float)fx->option[opter2].i;
                LinesArray[opter].y = (float)fx->option[opter2+1].i;
                LinesArray[opter+1].x = (float)fx->option[opter2+2].i;
                LinesArray[opter+1].y = (float)fx->option[opter2+3].i;
                LinesArray[opter+2].x = (float)fx->option[opter2+4].i;
                LinesArray[opter+2].y = (float)fx->option[opter2+5].i;
                LinesArray[opter+3].x = (float)fx->option[opter2+6].i;
                LinesArray[opter+3].y = (float)fx->option[opter2+7].i;
            }

        }
        else
            return(0);
        if(fx->option[0].i == TYPECONTLINES)
        {
                //printf("Warping image without bilinear interpolation\n");
                WarpImageByContLines(dest,(char *)fx->source[0].image,ginfo->width, ginfo->height, numberoflinepairs, LinesArray);
                gfree(LinesArray);
                return(1);

        }
        else // Do Bilinear Interpolation...
        {
                //printf("Warping image with bilinear interpolation\n");
                WarpImageByContLinesBi(dest,(char *)fx->source[0].image,ginfo->width, ginfo->height, numberoflinepairs, LinesArray);
                gfree(LinesArray);
                return(1);
        }
    }
    else
        return(0);

}


//------------------------------------------------------------------------------------------------------
FPUBLIC int closewarp(FINSTANCE *fx)
{
    return gfree((void*)fx);
}
