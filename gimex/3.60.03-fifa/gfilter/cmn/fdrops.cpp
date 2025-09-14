// (c) 1995-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*                    Drops - Drop Shadow Filter                    */
/*                                                                  */
/*                    by Ian H. Davids, EAC                         */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History                                        */
/* ------- ------ -- -------                                        */
/*    1.00 010612 ID Original uses basic raytracer with a twist     */
/*    1.01 010625 ID added multipass functionality for fontwiz      */
/*    1.02 010813 ID changed sky vector to make rot more intuitive  */
/*    1.03 010825 ID changed remap code to fix alpha ramp bug       */
/*    1.04 010827 ID added antialiasing/supersampling option        */
/*    1.05 010831 ID added global palette to 8bpp                   */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 8,32 bit                                                         */
/* drops(int type)                                                  */
/*                                                                  */
/*                                                                  */
/* Takes Glyph texturemap and creates a drop shadowed version of    */
/*    it attempting to preserver maximum transparency. eventually   */
/*    will try to produce umbra and penumbra.                       */
/*  Layout and Ideas taken from:                                    */
/*             The Persistence of Vision Raytracer                  */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include "gfilter.h"
#include "fdrops.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#if defined(_MSC_VER)
#pragma warning(disable:4514)
#endif
#include <math.h>

double Temp = 0;



//------------------------------------
// Defines
//------------------------------------

// CONSTANTS //
                                  // misc constants
#define HUGEDIST        2.0e10    // max viewing distance... for now.
#define MAXTRACELEVEL   5
#define DEPTH_TOLERANCE 1.0e-6    // plane depth tolerance.
#define MAXDISTANCE     1.0e7     // highest distance out that is viewable.
#define ERRORMARGIN     1.0e-10   // Very small number to round out.
#define TEXPLANE        0         // plane identifiers. (PlaneNumber)
#define SHADOWPLANE     1
                                  // supported light source types
#define POINTLIGHT      0
#define SPOTLIGHT       1
#define CYLINDRICAL     2
#define ARPOINTLIGHT    3         // area lights are what allow ubmra and penumbra.
#define ARSPOTLIGHT     4
#define ARCYLINDRICAL   5

#define DIFFUSECOEFFICIENT 1
//  MACROS   //
#define Vinit(s,a,b,c)                (((s).x) = (float)(a),((s).y) = (float)(b),((s).z) = (float)(c))
#define SetColour(s,A,R,G,B)          {((s).x) = R;((s).y) = G;((s).z) = B;((s).a) = A;}
#define LinComb(V,V1,c1,V2,c2,V3,c3)  {(V).x = (float)(c1*(V1).x) + (float)(c2*(V2).x) + (float)(c3*(V3).x);    \
                                       (V).y = (float)(c1*(V1).y) + (float)(c2*(V2).y) + (float)(c3*(V3).y);    \
                                       (V).z = (float)(c1*(V1).z) + (float)(c2*(V2).z) + (float)(c3*(V3).z);    }
#define LinComb2(V,V1,c1,V2,c2)       {(V).x = (float)(c1*(V1).x) + (float)(c2*(V2).x);                         \
                                       (V).y = (float)(c1*(V1).y) + (float)(c2*(V2).y);                         \
                                       (V).z = (float)(c1*(V1).z) + (float)(c2*(V2).z);                         }

#define Cross(A,V1,V2)                {(A).x = ((V1).y*(V2).z - (V1).z*(V2).y);                                 \
                                       (A).y = ((V1).z*(V2).x - (V1).x*(V2).z);                                 \
                                       (A).z = ((V1).x*(V2).y - (V1).y*(V2).x);                                 }
#define AddVec(V,V1,V2)               {(V).x = (V1).x + (V2).x; (V).y = (V1).y + (V2).y; (V).z = (V1).z + (V2).z;}
#define Normalize(V1,V2)              {(Temp = 1.0/sqrt((V2).x*(V2).x + (V2).y*(V2).y + (V2).z*(V2).z));(V1).x = (float)((V2).x*Temp);(V1).y = (float)((V2).y*Temp);(V1).z = (float)((V2).z*Temp);}
#define Dot(Ans,V1,V2)                (Ans = (V1).x*(V2).x + (V1).y*(V2).y + (V1).z*(V2).z)
#define CLAMP(x,w)                    ((x < 0)?(x = 0):((x > w)?(x -= 1):x))
#define CLAMPV(x,w)                   ((x < 0)?(x = 0):((x > w)?(w):x))
#define Sub(D,V1,V2)                  {(D).x = (V1).x - (V2).x; (D).y = (V1).y - (V2).y; (D).z = (V1).z - (V2).z;}
#define Length(S,V)                   {S=(float)(sqrt((V).x*(V).x + (V).y*(V).y + (V).z*(V).z));}
#define DivScalar(S,V)                {(V).x/=(S);(V).y/=(S);(V).z/=(S);}
#define MulScalar(S,V)                {(V).x*=(S);(V).y*=(S);(V).z*=(S);}
#define CopyColour(C1,C2)             {(C1).x = (C2).x;(C1).y = (C2).y;(C1).z = (C2).z;(C1).a = (C2).a;}
#define AddColour(C1,C2)              {(C1).x += (C2).x;(C1).y += (C2).y;(C1).z += (C2).z;(C1).a += (C2).a;}
#define CONVCOL(C,V)                  ((C).a = (unsigned char)((V)->a*255), (C).r = (unsigned char)((V)->x*255), (C).g = (unsigned char)((V)->y*255), (C).b = (unsigned char)((V)->z*255))
#define CopyVec(C1,C2)                {(C1).x = (C2).x;(C1).y = (C2).y;(C1).z = (C2).z;}
#define CopyPixel(csx, csy, cdx, cdy,cdest,cgfo) {ARGB *d = (ARGB *)(cdest);                                                                \
                                                    if((cgfo)->bpp <= 8)                                                                    \
                                                        (cdest)[(cdy)*(cgfo)->width + (cdx)] = (char)(cdest)[(csy)*(cgfo)->width + (csx)];  \
                                                    else                                                                                    \
                                                        d[(cdy)*(cgfo)->width + (cdx)] = d[(csy)*(cgfo)->width + (csx)];                    }



//------------------------------------
// Structures.
//------------------------------------


//--------------------------
// structs for colour
// remapping.
typedef struct
{
    unsigned int argb;
    int count;
} XHIST;

//--------------------------
// structures for colour
// remapping.
typedef struct
{
    int numcolours;
    int histcolours;
    int histsize;
    XHIST *hist;
} XACT;


//--------------------------
// basic vector used for
// most calculations.
typedef struct Vec
{
    float x;
    float y;
    float z;
    float a;
}Vec;

//--------------------------
// basic ray structure
// used for each ray I
// shoot out into my
// world.
typedef struct LRay
{
    Vec Direction;
    Vec Start;
}LRay;

//--------------------------
// structure for holding
// instersection point
// locations.
typedef struct INTERSECTION
{
    float Distance;
    int PlaneNum;
    Vec Direction;
    Vec Point;
}INTERSECTION;


//--------------------------
// location of the eyepoint
// member of the RC.
typedef struct Camera
{
    Vec Up;
    Vec Right;
    Vec Direction;
    Vec Location;
    Vec Look_At;
    Vec Sky;
}Camera;


//--------------------------
// light source, can
// specify colour.
typedef struct Lumier
{
    Vec Location;
    Vec Colour;
    Vec Direction;
    int type;                       // type of light source, cylinder, point etc.
    float FadePower;
    float FadeDistance;
    float Coefficient;
    float Radius;
    float Falloff;
    float NumLights;                // area lights have a light list.
    void * Next;                    // area lights have a light list.
}Lumier;



//--------------------------
// All global information
// stored here.
typedef struct RenderContext
{
    int imag;
    int numimagecolours;
    int ScreenWidth;
    int ScreenHeight;
    int TraceLevel;                 // here for expandability. If I decide to do reflection calcs.
    float TexturePlaneOffset;
    float ShadowPlaneOffset;
    Camera Cam;
    Lumier Light;
    GINFO * ginfo;                  // keep ginfo information so I can get at clut.
    char  * data8;                  // just to make life easier have a point to
    ARGB  * data32;                 // the data in both forms.
    int returncolours;
    int state;                                              // for GUI based systems that want to do multipass give
    int numstates;                  // me how many lines to interlace and I will do it for you.
    int rowbytesdest;
    int rowbytessrc;
    int mulsupersample;
    GINFO * commonclut;
    int globpal;
}RenderContext;




/****************************************************************/
/* Internal Functions                                           */
/****************************************************************/
static void do_TextureMap(RenderContext *RC,Vec *Colour,INTERSECTION *Best_Intersect,LRay *Ray,float weight);
static float Intersect(RenderContext *RC,INTERSECTION * Next_Intersect,LRay *Ray,int PlaneNumber);
static int Map3Dto2D_Planar(RenderContext * RC,Vec * Point,float * u, float * v);
static void GetImageColour(RenderContext * RC, float x, float y,Vec * Colour);
static float cubic_spline(float bottom, float top, float position);
static float Attenuate_Light(RenderContext *RC, LRay *Ray,float distance);
static void do_diffuse(RenderContext *RC,LRay * SRay,Vec *Colour,float Attenuation);
static int do_light(RenderContext * RC,Vec *Colour,INTERSECTION * Best_Intersect, float * Attenuation, LRay * LSRay,int PlaneNum);
static void do_PigmentMap(RenderContext *RC,Vec *Colour,INTERSECTION *Best_Intersect,LRay *Ray,float weight);
static void do_TextureMap(RenderContext *RC,Vec *Colour,INTERSECTION *Best_Intersect,LRay *Ray,float weight);
static void Do_Colouring(RenderContext *RC,INTERSECTION *Best_Intersect,Vec *Colour,LRay *Ray,float weight);
static float Trace_Ray(RenderContext * RC,LRay * Ray,Vec * Colour,float weight);
static int Make_Ray(RenderContext * RC,LRay * Ray,double x, double y);
static void Trace(RenderContext * RC,double x,double y,Vec *Colour);
//static int findcolour(const ARGB *palette, int numcolours, const ARGB *color);
static void PlotPixel(RenderContext *RC,int x, int y,Vec * Colour,char * dest,GINFO * ginfo,int rowbytes,int imag);
static void GetOptions(int numoptions, FOP *options,RenderContext * RC,GINFO * ginfo);




//------------------------------------------------------------------------------
// do a plane ray intersection. This is a specific version of the more generalized
// intersection calculations associated with quadrics.
static float Intersect(RenderContext *RC,INTERSECTION * Next_Intersect,LRay *Ray,int PlaneNumber)
{
    float NormalFromOrigin    = 0;
    float NormalFromDirection = 0;
    float Dist                = 0;
    Vec NormalVec;
    Vinit(NormalVec,0,-1,0);                                // hardcode my plane normal.
    if(PlaneNumber == TEXPLANE)
        Dist = RC->TexturePlaneOffset;
    else
        Dist = RC->ShadowPlaneOffset;
    Dot(NormalFromDirection,NormalVec,Ray->Direction);
    if(fabs(NormalFromDirection) < ERRORMARGIN)
        return 0;
    Dot(NormalFromOrigin,NormalVec,Ray->Start);
    Next_Intersect->Distance = -(NormalFromOrigin + Dist) / NormalFromDirection;
    if(Next_Intersect->Distance >= DEPTH_TOLERANCE && Next_Intersect->Distance <= MAXDISTANCE)
    {
        Next_Intersect->Point.x = (Ray->Start).x + Next_Intersect->Distance * (Ray->Direction).x;
        Next_Intersect->Point.y = (Ray->Start).y + Next_Intersect->Distance * (Ray->Direction).y;
        Next_Intersect->Point.z = (Ray->Start).z + Next_Intersect->Distance * (Ray->Direction).z;
        Next_Intersect->PlaneNum = PlaneNumber;                                                    // save the plane number for lighting.
        return 1;
    }
    else
        return 0;
}



//------------------------------------------------------------------------------
// go from a 3D point to a 2D point... assume on the XZ plane.
static int Map3Dto2D_Planar(RenderContext * RC,Vec * Point,float * u, float * v)
{
    float x = Point->x;
    float z = Point->z;
    //float z = Point->z;
    if((x < -0.5 || x > 0.5) || (z < -0.5 || z > 0.5))
        return 0;
    *u = (float)fmod((double)(x) * (double)(RC->ScreenWidth) + (double)((double)RC->ScreenWidth/2.0),RC->ScreenWidth);            // screen height and width
    *v = (float)fmod((double)(-z) * (double)(RC->ScreenHeight)  + (double)((double)RC->ScreenHeight/2.0),RC->ScreenHeight);          // are same size as image.
                                                                                       // do from -.5 to +.5
    return 1;
}




//-------------------------------------------------------------------------------
// standard round up or down.
static int QuickRound(float val)
{
    if((val - (int)val) >= 0.5)
        return((int)val + 1);
    return((int)val);
}


//------------------------------------------------------------------------------
// get the colour from my image map, no bilinear filtering... for now.
static void GetImageColour(RenderContext * RC, float x, float y,Vec * Colour)
{
    int Ix = (int)x;
    int Iy = (int)y;
    int index  = 0;
    int i = 0;
    //float a,r,g,b;
   Vec CornerColours[4];
   Ix = QuickRound(x);
   Iy = QuickRound(y);
   CLAMP(Ix,(RC->ScreenWidth - 1));
   CLAMP(Iy,(RC->ScreenHeight - 1));
   if(RC->ginfo->bpp == 8)             // have to work with index.
   {
      //index = RC->data8[Iy * RC->ginfo->width + Ix];
      index = RC->data8[Iy * RC->rowbytessrc + Ix];
      CornerColours[i].x = (float)((float)(RC->ginfo->colourtbl[index].r)/255.0);
      CornerColours[i].y = (float)((float)(RC->ginfo->colourtbl[index].g)/255.0);
      CornerColours[i].z = (float)((float)(RC->ginfo->colourtbl[index].b)/255.0);
      CornerColours[i].a = (float)((float)(RC->ginfo->colourtbl[index].a)/255.0);
   }
   else
   {
      index = Iy * RC->ginfo->width + Ix;
      //index = Iy * RC->rowbytessrc + Ix;
      CornerColours[i].x = (float)((float)(RC->data32[index].r)/255.0);
      CornerColours[i].y = (float)((float)(RC->data32[index].g)/255.0);
      CornerColours[i].z = (float)((float)(RC->data32[index].b)/255.0);
      CornerColours[i].a = (float)((float)(RC->data32[index].a)/255.0);
   }
   Colour->x = CornerColours[0].x;
   Colour->y = CornerColours[0].y;
   Colour->z = CornerColours[0].z;
   Colour->a = CornerColours[0].a;
    return;
}


/*
//------------------------------------------------------------------------------
// get the colour from my image map. This does bilinear interpolation. Not the
// fastest in the world but at least it will look better.
static void GetImageColour(RenderContext * RC, float x, float y,Vec * Colour)
{
    int Ix = (int)x;
    int Iy = (int)y;
    int index  = 0;
    int i = 0;
    int xadder = 0;
    int yadder = -1;
    //float a,r,g,b;
    Vec CornerColours[4];
    for(i = 0; i < 4; i++)                 // doing bilinear interpolation.
    {
       Ix = Ix + xadder;
       Iy = Iy + yadder;
       CLAMP(Ix,RC->ScreenWidth);
       CLAMP(Iy,RC->ScreenHeight);
       if(RC->ginfo->bpp == 8)             // have to work with index.
       {
          index = RC->data8[Iy * RC->ginfo->width + Ix];
          CornerColours[i].x = (float)((float)(RC->ginfo->colourtbl[index].r)/255.0);
          CornerColours[i].y = (float)((float)(RC->ginfo->colourtbl[index].g)/255.0);
          CornerColours[i].z = (float)((float)(RC->ginfo->colourtbl[index].b)/255.0);
          CornerColours[i].a = (float)((float)(RC->ginfo->colourtbl[index].a)/255.0);
       }
       else
       {
          index = Iy * RC->ginfo->width + Ix;
          CornerColours[i].x = (float)((float)(RC->data32[index].r)/255.0);
          CornerColours[i].y = (float)((float)(RC->data32[index].g)/255.0);
          CornerColours[i].z = (float)((float)(RC->data32[index].b)/255.0);
          CornerColours[i].a = (float)((float)(RC->data32[index].a)/255.0);
       }
                               // swap between adding 1 or adding nothing.
       xadder =  1 - xadder;
       if(i > 1)               // only swap y's on second round so we get all permutations.
           yadder = -1 - yadder;
    }
    {                          // do bilinear interpolation.
        float m = x - (int)x;
        float n = y - (int)y;
        float M1 = n*m;
        float M2 = n*(1-m);
        float M3 = m*(1-n);
        float M4 = (1-m)*(1-n);
        if(m == 0 && n == 0)
        {
           Colour->x = CornerColours[0].x;
           Colour->y = CornerColours[0].y;
           Colour->z = CornerColours[0].z;
           Colour->a = CornerColours[0].a;
        }
        else
        {
          Colour->x = (M1*CornerColours[0].x) + (M2*CornerColours[1].x) + (M3*CornerColours[2].x) + (M4*CornerColours[3].x);
          Colour->y = (M1*CornerColours[0].y) + (M2*CornerColours[1].y) + (M3*CornerColours[2].y) + (M4*CornerColours[3].y);
          Colour->z = (M1*CornerColours[0].z) + (M2*CornerColours[1].z) + (M3*CornerColours[2].z) + (M4*CornerColours[3].z);
          Colour->a = (M1*CornerColours[0].a) + (M2*CornerColours[1].a) + (M3*CornerColours[2].a) + (M4*CornerColours[3].a);
        }
    }
    return;
}

*/

//------------------------------------------------------------------------------
// almost entirely stolen from POV this is the basic falloff calculation for
// my light source. With this I calculate umbra and penumbra.
static float cubic_spline(float bottom, float top, float position)
{
    if(position < bottom)
        return 0;
    if(position >= top)
        return 1;
    if(top == bottom)                                   // just in case I made a screw up.
        return 0;

    position = (position - bottom) / (top - bottom);    // normalize range.
    return ((3 - 2*position) * position * position);
}




//------------------------------------------------------------------------------
// POINT LIGHT  SOURCE
// Light attenuation calculation. This figures out if I am in shadow or not
// and does the soft shadowing for my strange light source types.
// again most ideas here taken from the POV layout.
static float Attenuate_Light(RenderContext *RC, LRay *Ray,float distance)
{
    float Attenuation = 1.0;                // by default we don't attenuate.
    float Len,Scalar,CosTheta;
    Vec TempV;
    Vec OutPoint;
    if(RC->Light.type == SPOTLIGHT)
    {
        Sub(TempV,RC->Light.Location,Ray->Start);
        Dot(Scalar,TempV,RC->Light.Direction);
        Dot(CosTheta,Ray->Direction,RC->Light.Direction);
        //CosTheta *= -1.0;
        if(CosTheta > 0)//RC->Light.Radius)                    // if we are within cone then lets to the interp to find attenuation.
        {
            //LinComb2(OutPoint,TempV,1.0,RC->Light.Direction,-Scalar);
            //Length(Len,OutPoint);
            //if(Len < RC->Light.Radius || !RC->Light.Radius)
            //{

                Attenuation = (float)pow(CosTheta,RC->Light.Coefficient);
                if(RC->Light.Radius > 0)        // do we have a soft falloff?
                    Attenuation *= cubic_spline(RC->Light.Falloff,RC->Light.Radius,CosTheta);
            //}
            //else
            //  Attenuation = 0;
        }
        else
        {
            Attenuation = 0;
        }
    }
    else if(RC->Light.type == CYLINDRICAL)
    {
       Sub(TempV,RC->Light.Location,Ray->Start);
       Dot(Scalar,TempV,RC->Light.Direction);
       if(Scalar > 0)
       {

           LinComb2(OutPoint,TempV,1.0,RC->Light.Direction,-Scalar);
           Length(Len,OutPoint);
           if(Len < RC->Light.Falloff)
           {
               Len         = (float)(1.0 - Len/RC->Light.Falloff);
               Attenuation = (float)pow(Len,RC->Light.Coefficient);
               if(RC->Light.Radius > 0 && Len < RC->Light.Radius)
                   Attenuation *= cubic_spline((float)(1.0- RC->Light.Radius/RC->Light.Falloff),1.0,Len);

           }
           else
               Attenuation = 0;
       }
       else
           Attenuation = 0;
    }
    if(Attenuation > 0)                   // okay so type specific attenuation is done
    {                                     // now attenuate based on distance.
        if(RC->Light.FadePower > 0 && fabs(RC->Light.FadeDistance) > ERRORMARGIN)
            Attenuation *= (float)(2.0 / (1.0 + pow(distance / RC->Light.FadeDistance,RC->Light.FadePower)));
    }
    return(Attenuation);
}

//------------------------------------------------------------------------------
// AREA LIGHT  SOURCE
// Light attenuation calculation. This figures out if I am in shadow or not
// and does the soft shadowing for my strange light source types.
// again most ideas here taken from the POV layout.
static float Area_Attenuate_Light(Lumier *Light, LRay *Ray,float distance)
{
    float Attenuation = 1.0;                // by default we don't attenuate.
    float Len,Scalar,CosTheta;
    Vec TempV;
    Vec OutPoint;
    if(Light->type == SPOTLIGHT || Light->type == ARSPOTLIGHT)
    {
        Sub(TempV,Light->Location,Ray->Start);
        Dot(Scalar,TempV,Light->Direction);
        Dot(CosTheta,Ray->Direction,Light->Direction);
        if(CosTheta > 0)                    // if we are within cone then lets to the interp to find attenuation.
        {
                Attenuation = (float)pow(CosTheta,Light->Coefficient);
                if(Light->Radius > 0)       // do we have a soft falloff?
                    Attenuation *= cubic_spline(Light->Falloff,Light->Radius,CosTheta);
        }
        else
        {
            Attenuation = 0;
        }
    }
    else if(Light->type == CYLINDRICAL || Light->type == ARCYLINDRICAL)
    {
       Sub(TempV,Light->Location,Ray->Start);
       Dot(Scalar,TempV,Light->Direction);
       if(Scalar > 0)
       {

           LinComb2(OutPoint,TempV,1.0,Light->Direction,-Scalar);
           Length(Len,OutPoint);
           if(Len < Light->Falloff)
           {
               Len         = (float)(1.0 - Len/Light->Falloff);
               Attenuation = (float)pow(Len,Light->Coefficient);
               if(Light->Radius > 0 && Len < Light->Radius)
                   Attenuation *= cubic_spline((float)(1.0- Light->Radius/Light->Falloff),1.0,Len);

           }
           else
               Attenuation = 0;
       }
       else
           Attenuation = 0;
    }
    if(Attenuation > 0)                   // okay so type specific attenuation is done
    {                                     // now attenuate based on distance.
        if(Light->FadePower > 0 && fabs(Light->FadeDistance) > ERRORMARGIN)
            Attenuation *= (float)(2.0 / (1.0 + pow(distance / Light->FadeDistance,Light->FadePower)));
    }
    return(Attenuation);
}


//------------------------------------------------------------------------------
// do basic diffuse lighting without certain coefficients since I am not letting
// the user have control over brilliancy etc.
static void do_diffuse(RenderContext *RC,LRay * SRay,Vec *Colour,float Attenuation)
{
    Vec Pigment;
    Vec NormalToSurface;
    float AngleOfIncidence, Intensity;
    float NormalDirection;
    Vinit(NormalToSurface,0,-1,0);                          // set normal to y axis.
    Dot(NormalDirection,NormalToSurface,SRay->Direction);  // check if I'm pointing away from ray.
    if(NormalDirection > 0)
        NormalToSurface.y = -1;                            // flip direction if I'm pointing away from camera.

    SetColour(Pigment,1,0,0,0);                            // set back plane to colour 1.
                                                           // find my Theta to do lambertian calc.
    Dot(AngleOfIncidence,NormalToSurface,RC->Light.Direction);
                                                           // assume brilliancy is 1
    Intensity = (float)fabs(AngleOfIncidence);
    Intensity *= DIFFUSECOEFFICIENT * Attenuation;         // attenuate by falloff.
    Colour->x += Intensity * Pigment.x * RC->Light.Colour.x;
    Colour->y += Intensity * Pigment.y * RC->Light.Colour.y;
    Colour->z += Intensity * Pigment.z * RC->Light.Colour.z;
    Colour->a += Intensity * Pigment.a * RC->Light.Colour.a;
}


//------------------------------------------------------------------------------
// basic layout taken from POV with a few modifications, no more media
// interaction, and only the basics that are needed here.
static int do_light(RenderContext * RC,Vec *Colour,INTERSECTION * Best_Intersect, float * Attenuation, LRay * LSRay,int PlaneNum)
{
    Vec LightColour;
    LRay LightRay;
    float LightDepth;
    int isIntersect = 0;
    float BitAttenuation = 1;
    if(RC->Light.type == POINTLIGHT || RC->Light.type == SPOTLIGHT || RC->Light.type == CYLINDRICAL)
    {
                                                    // set default attenuation value.
        (*Attenuation) = 1.0;
                                                    // get light source colour.
        SetColour(LightColour,RC->Light.Colour.a,RC->Light.Colour.x,RC->Light.Colour.y,RC->Light.Colour.z);

                                                    // make Light ray.
        Vinit(LightRay.Start,Best_Intersect->Point.x,Best_Intersect->Point.y,Best_Intersect->Point.z);
        Sub(LightRay.Direction,RC->Light.Location,Best_Intersect->Point);
                                                    // find length of light ray.
        Length(LightDepth,LightRay.Direction);
                                                    // now normalize light direction, but don't do the length calc again.
        DivScalar(LightDepth,LightRay.Direction);
                                                    // Attenuate Light.
        *Attenuation = Attenuate_Light(RC,&LightRay,LightDepth);
                                                    // Scale Light Colour by attenuation.
        MulScalar((*Attenuation),LightColour);
        (*LSRay) = LightRay;                        // copy over light ray so higher level function can have it to
                                                    // do diffuse lighting.

        if(PlaneNum == SHADOWPLANE)                 // if we are back plane check for shadow.
        {
            INTERSECTION Temp_Intersect;
            if(Intersect(RC,&Temp_Intersect,&LightRay,TEXPLANE))
            {
                if(Temp_Intersect.Distance < Best_Intersect->Distance)  // we have an intersection so do lighting check.
                {
                    float x,y;
                    Vec TempColour;                                    // get my 2D tex coords.
                    if(Map3Dto2D_Planar(RC,&(Temp_Intersect.Point),&x,&y))
                    {
                        GetImageColour(RC,x,y,&TempColour);            // if it's on bitmap then get bitmap colour.
                        if(TempColour.a != 0)                          // check if not completely transparent.
                        {
                            float TempAttenuate = (float)(1.0 - TempColour.a);  // figure out this contribution to the attenuation.
                            MulScalar(TempAttenuate,(*Colour));        // modify my colour appropriately.
                            //(*Attenuation) -= (TempAttenuate);           // added it to global attenuation.
                            if(RC->Light.type == POINTLIGHT && RC->Light.FadePower == 0 && RC->Light.FadeDistance == 0)
                            {
                                (*Attenuation) *= TempColour.a;
                            }
                            if((*Attenuation) < 0)                     // clamp my attenuation factor.
                                (*Attenuation) = 0;
                            isIntersect = 1;
                        }

                    }
                }
            }

        }
        if((*Attenuation) == 1.0 && !isIntersect)   // magic number letting me know that this should be a
        {                                           // transparent pixel and not a opaque one.
            AddColour((*Colour),LightColour);
            Colour->a = 0;                          // completly transparent pixel.
            return 1;                               // point is not obscured from this light
        }
        else
        {
            if(isIntersect)
            {
                AddColour((*Colour),LightColour);
                Colour->a = (float)((*Attenuation));       // try this for now.. set transparency level
                                                                 // to the attenuation value.
                return 0;                                        // point is obscured from this light.
            }
            else
            {
                AddColour((*Colour),LightColour);       // I don't want to be able to see any of the
                Colour->a = 0;                          // shadow ring made by my light source so make non intersected items
                                                        // completly transparent pixel.
                return 1;                               // point is not obscured from this light
            }
        }
    }// done doing single point light sources.
    else // well we have an area light so lets do a loop.
    {

        int count = 0;
        Lumier * Current = &RC->Light;
                                                        // set default attenuation value.
        (*Attenuation) = 0.0;                           // start at zero and work upwards.

        for(count = 0; count < RC->Light.NumLights && (Current);count++)
        {
                                                        // get light source colour.
            SetColour(LightColour,Current->Colour.a,Current->Colour.x,Current->Colour.y,Current->Colour.z);

                                                        // make Light ray.
            Vinit(LightRay.Start,Best_Intersect->Point.x,Best_Intersect->Point.y,Best_Intersect->Point.z);
            Sub(LightRay.Direction,Current->Location,Best_Intersect->Point);
                                                        // find length of light ray.
            Length(LightDepth,LightRay.Direction);
                                                        // now normalize light direction, but don't do the length calc again.
            DivScalar(LightDepth,LightRay.Direction);
                                                        // Attenuate Light.
            BitAttenuation = Area_Attenuate_Light(Current,&LightRay,LightDepth);
                                                        // Scale Light Colour by attenuation.
            BitAttenuation /= (float)RC->Light.NumLights;

            (*LSRay) = LightRay;                        // copy over light ray so higher level function can have it to
                                                        // do diffuse lighting.
            if(PlaneNum == SHADOWPLANE)                 // if we are back plane check for shadow.
            {
                INTERSECTION Temp_Intersect;
                if(Intersect(RC,&Temp_Intersect,&LightRay,TEXPLANE))
                {
                    if(Temp_Intersect.Distance < Best_Intersect->Distance)  // we have an intersection so do lighting check.
                    {
                        float x,y;
                        Vec TempColour;                                    // get my 2D tex coords.
                        if(Map3Dto2D_Planar(RC,&(Temp_Intersect.Point),&x,&y))
                        {
                            GetImageColour(RC,x,y,&TempColour);            // if it's on bitmap then get bitmap colour.
                            if(TempColour.a != 0)                          // check if not completely transparent.
                            {
                                float TempAttenuate = (float)(1.0 - TempColour.a) / (float)(RC->Light.NumLights);  // figure out this contribution to the attenuation.
                                MulScalar(TempAttenuate,(*Colour));        // modify my colour appropriately.
                                (*Attenuation) += BitAttenuation;            // if we had an intersection with the texture then
                                if(RC->Light.type == ARPOINTLIGHT && RC->Light.FadePower == 0 && RC->Light.FadeDistance == 0 && count == (RC->Light.NumLights -1)) // only do on last pass.
                                {
                                    (*Attenuation) *= TempColour.a;
                                }
                                if((*Attenuation) < 0)                     // clamp my attenuation factor.
                                    (*Attenuation) = 0;
                                isIntersect = 1;
                            }                                              // this lights shadow contributes to the overall shadow.
                        }
                    }
                }
            }
            Current = (Lumier *)Current->Next;              // go to next light in list.
        }  // for all lights.
        if((*Attenuation) == 1.0 && !isIntersect)   // magic number letting me know that this should be a
        {                                           // transparent pixel and not a opaque one.
            AddColour((*Colour),LightColour);
            Colour->a = 0;                          // completly transparent pixel.
            return 1;                               // point is not obscured from this light
        }
        else
        {
            if(isIntersect)
            {
                AddColour((*Colour),LightColour);
                Colour->a = (float)((*Attenuation));       // try this for now.. set transparency level
                                                                 // to the attenuation value.
                return 0;                                        // point is obscured from this light.
            }
            else
            {
                AddColour((*Colour),LightColour);       // I don't want to be able to see any of the
                Colour->a = 0;                          // shadow ring made by my light source so make non intersected items
                                                        // completly transparent pixel.
                return 1;                               // point is not obscured from this light
            }
        }

    }// if area light sources.

}

//------------------------------------------------------------------------------
// hack function to do a lighting calculation on my background. Does not do
// secondary reflections only the first ray and a simple in shadow test followed
// by a basic diffuse lighting calculation.
static void do_PigmentMap(RenderContext * RC,Vec * Colour,INTERSECTION * Best_Intersect,LRay * Ray,float weight)
{
    // only do reflective component otherwise this will be transparent.. I
    // don't want any colour on my back plane.
    // note I don't have to trace any reflected rays as
    float LightAngle = 0;
    float Attenuation = 1.0;
    float NormalDirection;
    Vec SurfaceNormal;
    LRay LSRay;                                               // Light Source Ray.
    int notobscured = 0;
    Vinit(SurfaceNormal,0,-1,0);
    Dot(NormalDirection,SurfaceNormal,Ray->Direction);        // check if I'm pointing away from ray.
    if(NormalDirection > 0)
        SurfaceNormal.y = -1;                                 // flip direction if I'm pointing away from camera.

    notobscured = do_light(RC,Colour,Best_Intersect,&Attenuation,&LSRay,SHADOWPLANE);

    Dot(LightAngle,SurfaceNormal,LSRay.Direction);            // check if light is on other side of plane.
    if(notobscured)
    {
        SetColour((*Colour),0,0,0,0);                         // set colour to transparent because this is a non obscured background pixel.
        return;
    }
/*    else if(fabs(LightAngle) < ERRORMARGIN)                         // check if light is on other side of plane.
    {
        SetColour((*Colour),1,0,0,0);                         // set colour to black object is in complete shadow.
        return;
    }*/
    else
    {
        // now I have to set the shadow colour.
        do_diffuse(RC,Ray,Colour,Attenuation);
    }
}




//------------------------------------------------------------------------------
// a hack again, this function finds a point on my texture map, if it's part of
// the glyph then I keep that colour. Otherwise I must do my lighting calculation
// on my back plane to see if I have a shadow pixel. If I do then I use the
// shadow colour OR if no shadow I scrap that idea and use transparent.
static void do_TextureMap(RenderContext *RC,Vec * Colour,INTERSECTION * Best_Intersect,LRay * Ray,float weight)
{
    float x = 0;
    float y = 0;
    if(!Map3Dto2D_Planar(RC,&(Best_Intersect->Point),&x,&y))
    {
        SetColour((*Colour),0,0,0,0); // map to transparent if outside texture.
    }
    else
    {
        GetImageColour(RC,x,y,Colour);
    }
    if(Colour->a != 0)         // for now (Later I will want to do actual illumination calcs.
    {                          // if we hit the texture then I don't have to do
        RC->imag = 1;          // any illumination as I have the colour I want
        return;                // stored in that texture.
    }                          // this simplification may give me problems around the drop shadow may end up doing
    else                       // real refraction here.
    {
        if(Intersect(RC,Best_Intersect,Ray,SHADOWPLANE))           // find intersection with back plane.
        {
            // now do a pigment colouring on back plane.
            do_PigmentMap(RC,Colour,Best_Intersect,Ray,weight);
        }
        else                                                       // if I don't hit back plane I must be facing
        {                                                          // the wrong way so make transparent.
            SetColour((*Colour),0,0,0,0);
        }
    }
}



//////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
static void Do_Colouring(RenderContext *RC,INTERSECTION *Best_Intersect,Vec *Colour,LRay *Ray,float weight)
{
    float NormalDirection = 0;
    Vec   NormalToSurface;
    Vinit(NormalToSurface,0,-1,0);                        // hardcode normal here.
    Dot(NormalDirection,NormalToSurface,Ray->Direction); // check if I'm pointing away from ray.
    if(NormalDirection > 0)
        NormalToSurface.y = -1;                          // flip direction if I'm pointing away from camera.
    if(Best_Intersect->PlaneNum == TEXPLANE)             // Texture Mapped plane.. have to grab texture point.
    {
        // grab the texture colour, do a lighting calculation and continue.
        do_TextureMap(RC,Colour,Best_Intersect,Ray,weight);
    }
    else if(Best_Intersect->PlaneNum == SHADOWPLANE)     // Raw Transparent Plane.
    {
        // figure out if I'm in shadow... do the colour stuff here.
        do_PigmentMap(RC,Colour,Best_Intersect,Ray,weight);
    }
    //do_texture_map........
}


//------------------------------------------------------------------------------
// loop over all objects and do an intersection test. If we found an intersection
// then Colour that pixel.
static float Trace_Ray(RenderContext * RC,LRay * Ray,Vec * Colour,float weight)
{
    int FoundAnIntersect = 0;
    int i = 0;
    INTERSECTION Best_Intersect;
    INTERSECTION Next_Intersect;
    SetColour((*Colour),0,0,0,0);
    if(RC->TraceLevel > MAXTRACELEVEL)
        return HUGEDIST;


    Best_Intersect.Distance = HUGEDIST;
    for(i = 0; i < 2; i++)                       // hardcode my number of objects here.
    {
        if(Intersect(RC,&Next_Intersect,Ray,i))
        {
            if(Next_Intersect.Distance < Best_Intersect.Distance)
            {
                Best_Intersect = Next_Intersect;
                FoundAnIntersect = 1;
            }
        }
    }
    if(FoundAnIntersect)
        Do_Colouring(RC,&Best_Intersect,Colour,Ray,weight);
    return(Best_Intersect.Distance);
}


//------------------------------------------------------------------------------
// create a ray originating from the camera and heading out into my world.
// returns true if x and y are within screen boundary false otherwise.
static int Make_Ray(RenderContext * RC,LRay * Ray,double x, double y)
{
    double intX,intY;
    // convert screen coords into logical coords between -0.5 to 0.5 //
    intX = (double)((double)x / (double)RC->ScreenWidth - 0.5);
    intY = (double)((double)(((double)RC->ScreenHeight/*- 1.0*/) - (double)y) / (double)RC->ScreenHeight - 0.5);
    if(intX < -0.5 || intX > 0.5 || intY < -0.5 || intY > 0.5)
        return(0);
    LinComb(Ray->Direction,RC->Cam.Direction,1.0,RC->Cam.Right,intX,RC->Cam.Up,intY);   // create the direction ray.
    Ray->Start = RC->Cam.Location;                                                      // set start of ray at camera location.
    Normalize(Ray->Direction,Ray->Direction);                                           // normalize my direction ray.
    return 1;
}


//------------------------------------------------------------------------------
// Begin the process of tracing the Ray.
static void Trace(RenderContext * RC,double x,double y,Vec *Colour)
{
    LRay Ray;
    RC->imag = 0;                        // we haven't hit the image. This flag is for colour remap later.
    if(Make_Ray(RC,&Ray,x,y))
        Trace_Ray(RC,&Ray,Colour,1.0);
    else
        SetColour((*Colour),0,0,0,0);    // if we failed to create the screen ray then
                                         // we better set the pixel to transparent.
}




/*
//------------------------------------------------------------------------------
// stolen from CRAPI, add a colour entry to the colour histogram.
static void xaddhist(XACT *xact,unsigned int p, int newcount)
{
    int i;

    // find colour and count

    for (i=0; i<xact->histcolours; ++i)
    {
        if (xact->hist[i].argb==p)
        {
            xact->hist[i].count += newcount;
            return;
        }
    }

    // adding new colour, realloc histogram if full

    i = xact->histcolours;
    if (i>=xact->histsize)
    {
        XHIST *hist;

        xact->histsize += 256;
        hist = (XHIST *) galloc(xact->histsize*sizeof(XHIST));
        if (xact->hist)
        {
            memcpy(hist,xact->hist,i*sizeof(XHIST));
            gfree(xact->hist);
        }
        xact->hist = hist;
    }

    // add new colour

    xact->hist[i].argb = p;
    xact->hist[i].count = newcount;
    xact->histcolours = i+1;
}

//------------------------------------------------------------------------------
// stolen from CRAPI compare two entries in the colour histogram.
static int xhistcmp(const void *h1, const void *h2)
{
    return((*(XHIST **)h2)->count - (*(XHIST **)h1)->count);
}
*/
/****************************************************************/
/* CRapi functions                                              */
/****************************************************************/

/*
//------------------------------------------------------------------------------
// stolen from CRAPI read over the image and generate a colour histogram.
static int readclut(XACT *xact, GINFO *ginfo, const void *source, int rowbytes,GINFO * commonclut)
{
    int i;
    int x;
    int y;
    int ok=0;
    unsigned char *s;
    xact->hist = (XHIST *) galloc(256*sizeof(XHIST));
    xact->histcolours = 0;
    xact->histsize = 256;
    xact->numcolours = 0;
    if(commonclut)
    {
        assert(ginfo->bpp == commonclut->bpp);
        assert(ginfo->numcolours);
    }
    if (xact)
    {
        if(commonclut)
        {
            // strip out the upper section of the colour table expecting it to be semi filled with colours
            // and add the common colours to the new clut with a large count.
            for(i = 0; i < commonclut->numcolours/2;i++)
            {
                xaddhist(xact,(*(unsigned int *)&commonclut->colourtbl[i]),commonclut->width*commonclut->height);
            }
        }
        for (i=0; i<256; ++i)
            xaddhist(xact,(*(unsigned int *)&ginfo->colourtbl[i]),0);

        for (y=0; y<ginfo->height; ++y)
        {
            s = (unsigned char *)source + y*rowbytes;
            for (x=0; x<ginfo->width; ++x)
            {
                if (ginfo->bpp==32)
                {
                    xaddhist(xact,(*(unsigned int *) s),1);
                    s += 4;
                }
                else
                {
                    xaddhist(xact,(*(unsigned int *)&ginfo->colourtbl[*s]),1);
                    s += 1;
                }
            }
        }

        ok = 1;
    }
    return(ok);
}

//------------------------------------------------------------------------------
// stolen from CRAPI, read over histogram and generate a new clut based on the
// histogram. number of colours actually used must always be < or == than numcolours
// return colours contains the used count.
static ARGB * closeclut(XACT *xact, int numcolours, int *returncolours)
{
    int i;
    int j;
    int ok=0;
    int realcolours=0;
    unsigned int *colourtbl=0;
    XHIST **histindex;

    if (xact)
    {
        if (xact->hist)
        {
            histindex = (XHIST **) galloc(xact->histcolours*sizeof(XHIST *));
            if (histindex)
            {
                for (i=0; i<xact->histcolours; ++i)
                    histindex[i] = xact->hist+i;

                qsort(histindex,xact->histcolours,sizeof(XHIST *),xhistcmp);

                realcolours = gmin(numcolours,xact->histcolours);

                for (i=0; i<realcolours; ++i)
                    histindex[i]->count = -histindex[i]->count;

                colourtbl = (unsigned int *) galloc(numcolours*sizeof(ARGB));
                if (colourtbl)
                {
                    memset(colourtbl,0,numcolours*sizeof(ARGB));
                    for (i=0,j=0; i<xact->histcolours; ++i)
                        if (xact->hist[i].count<0)
                            colourtbl[j++] = xact->hist[i].argb;

                    ok = 1;
                }
                gfree(histindex);
            }
        }
        if (xact->hist) gfree(xact->hist);
        if (xact)       gfree(xact);
    }
    if (returncolours)
        *returncolours = realcolours;
    return((ARGB *)colourtbl);
}


#define BIGDIST 1000000
//------------------------------------------------------------------------------
// stolen from CRAPI. Used to map the raytraced colour to the old palette for
// the image.
static int findcolour(const ARGB *palette, int numcolours, const ARGB *color)
{
    unsigned int closeDist=BIGDIST;
    unsigned int dist;
    unsigned int multiplier = 4;
    int i;
    const ARGB *pal;
    const ARGB *closepal;
    static unsigned int CRsquare[512];
    static int CRsquareinit=0;
    ARGB col;
    col.a = color->a;                                   // preattenuate the colour.
    col.r = (char)((color->a*color->r + 128)/255);
    col.g = (char)((color->a*color->g + 128)/255);
    col.b = (char)((color->a*color->b + 128)/255);
    // Fill up CRsquare table
    if (!CRsquareinit)
    {
        for (i=0; i<512; ++i)
            CRsquare[i] = (i-256)*(i-256);
        CRsquareinit = 1;
    }

    // Check all palette entries for closest color

    pal = closepal = palette;

    // don't make alpha as important all the time, if it is within
    // a critical range then we MUST match it, otherwise we can get a
    // semi good match in the alpha channel and be okay with that.

    if ((col.a<=16)
     || (col.a>=(128-16) && col.a<=(128+16))
     || (col.a>=240))
        multiplier = 4;
    else
        multiplier = 2;

    for (i=0; i<numcolours; ++i,++pal)
    {
        // distance = C(a1-a2)^2 + (r1-r2)^2 + (g1-g2)^2 + (b1-b2)^2
        multiplier = 4;

        dist   = CRsquare[col.r - pal->r + 256]
               + CRsquare[col.g - pal->g + 256]
               + CRsquare[col.b - pal->b + 256]
               + CRsquare[col.a - pal->a + 256]*multiplier;

        if (dist < closeDist)             // the version with no alpha weight
        {
            closeDist = dist;
            closepal = pal;
        }
    }

    // Return the index of the best match
    return((int)(closepal - palette));
}
#undef BIGDIST


//------------------------------------------------------------------------------
// remap my image to the new clut excluding the new shadow colours.
static char * RemapImage(RenderContext * RC,ARGB * newcolours,GINFO *ginfo,char * source,int rowbytes)
{
    int i = 0;
    //int size = ginfo->width*ginfo->height;
    int size = rowbytes*ginfo->height;
    char *tempdata = (char *) galloc(size);
    if(tempdata)
    {
        for(i = 0; i < size; i++)
        {
            tempdata[i] = findcolour((const ARGB *)newcolours,RC->returncolours,(const ARGB *)(&(ginfo->colourtbl[(unsigned char)(source[i])])));
        }
    }
    return(tempdata);
}


//------------------------------------------------------------------------------
// go over my image and clut and generate a new clut that has the
static char * GenerateClutAndRemap(RenderContext * RC,GINFO * ginfo,char *image,int rowbytes)
{
   ARGB * tempclut;
   ARGB curcol;
   int i        = 0;
   int colcount = 0;
   int maxccou  = 256;
   float scaler = 0;
   char * newimage = 0;

   if(RC->globpal)
   {
       tempclut = (ARGB *)galloc(sizeof(ARGB)*256);
       if(tempclut)
       {
           if(ginfo->originalbpp != 8)   // assume 4 bit
           {
               maxccou  = 16;
               colcount = 8;
           }
           else                          // assume 8 bit.
           {
               maxccou  = 256;
               colcount = 128;
           }
           RC->returncolours = colcount;
           tempclut[0].a = tempclut[0].r = tempclut[0].g = tempclut[0].b = 0;
           curcol.a = 255;
           for(i = 1; i < colcount; i++)
           {
                curcol.r = curcol.g = curcol.b = (unsigned char)((float)((float)(i)/(float)(colcount - 1))*255.0);
                tempclut[i] = curcol;
           }
           CONVCOL(curcol,(&(RC->Light.Colour)));
           curcol.a = 0;
           for(i = colcount; i < maxccou; i++)
           {
                curcol.a = (unsigned char)((float)((float)(i - (colcount - 1))/(float)(maxccou- colcount))*255.0);
                tempclut[i] = curcol;
           }
           newimage = RemapImage(RC,tempclut,ginfo,image,rowbytes);
           memcpy(ginfo->colourtbl,tempclut,sizeof(ARGB)*256);
           ginfo->numcolours = maxccou;
       }
       gfree(tempclut);

   }
   else
   {
       XACT *xref = (XACT *) galloc(sizeof(XACT));

       if(ginfo->originalbpp != 8)
       {
           if(ginfo->numcolours < 8)
           {
               colcount = ginfo->numcolours;
               maxccou = 16;
           }
           else if(ginfo->numcolours <= 16)
           {
               colcount = 8;
               maxccou  = 16;
           }
           else //if(colcount <= 256)
           {
               if(RC->commonclut)  // do some fancy gross stuff here restoring the numcolours count to that of the image data so I
               {                   // don't overseed my histogram on the next pass.
                   RC->commonclut->numcolours = RC->commonclut->hotspottbl[10][0];
               }
               else
               {
                   colcount = 127;
                   maxccou  = 256;
               }
           }
       }
       else
       {
           maxccou  = 256;
           if(ginfo->numcolours < 8)
               colcount = ginfo->numcolours;
           else if(ginfo->numcolours <= 16)
               colcount = 8;
           else //if(colcount < 256)
               colcount = 127;
       }
       readclut(xref, ginfo, (const char *)image, (ginfo->width*ginfo->bpp/8),RC->commonclut);
       tempclut = closeclut(xref, colcount, &(RC->returncolours));
       newimage = RemapImage(RC,tempclut,ginfo,image,rowbytes);
       if(tempclut)
       {
           memcpy(ginfo->colourtbl,tempclut,(sizeof(ARGB)*RC->returncolours));
           if(RC->returncolours < 2)
               RC->returncolours = 2;
           CONVCOL(curcol,(&(RC->Light.Colour)));
           curcol.a = 0;
           scaler = (float)(ginfo->numcolours/2); //- RC->returncolours);
           for(i = colcount; i < maxccou; i++)
           {
                curcol.a = (unsigned char)((float)((float)(i - colcount)/(float)(maxccou-colcount))*255.0);
                ginfo->colourtbl[i] = curcol;
           }
           gfree(tempclut);
       }
       else
       {
           if(RC->returncolours < 2)
               RC->returncolours = 2;
       }
   }
   RC->numimagecolours = RC->returncolours;

   ginfo->numcolours   = maxccou;
   ginfo->hotspottbl[10][0] = RC->returncolours;   // hide my real image colour count in the hotspots for later reuse.
                                                   // have to do this with the hopes that I can later seed my historgram
                                                   // to build common clut.
   //if(xact)              don't need to free this as it's freed in closeclut.
   //  gfree(xref);
   return(newimage);
}


*/


//------------------------------------------------------------------------------
// allows the seamless work in both 8bpp and 32bpp. puts the current pixel
// in the image buffer, doing a remap to the palette if it's an 8bpp
// image.
static void PlotPixel(RenderContext *RC,int x, int y,Vec * Colour,char * dest,GINFO * ginfo,int rowbytes,int imag)
{
    ARGB Col;
    ARGB *d = (ARGB *)dest;
    CONVCOL(Col,Colour);
// I don't support 8bpp anymore.
//    if(ginfo->bpp <= 8)
//    {
//       //dest[y*ginfo->width + x] = (char)findcolour(ginfo->colourtbl, ginfo->numcolours, &Col);
//           if(imag)     // did I hit the image or not?
//           dest[y*rowbytes + x] = (char)findcolour(ginfo->colourtbl, RC->numimagecolours/*ginfo->numcolours*/, &Col);
//           else
//           dest[y*rowbytes + x] = (char)findcolour(ginfo->colourtbl, ginfo->numcolours, &Col);
//    }
//    else
       d[y*ginfo->width + x] = Col;
       //    d[y*rowbytes + x] = Col;
}

/*
// turned into macro.
//------------------------------------------------------------------------------
// copy one pixel in my image buffer to another location in my image buffer.
// not the fastest way of doing things but it's explicite.
static void CopyPixel(int sx, int sy, int dx, int dy,char *dest,GINFO *ginfo)
{
    ARGB *d = (ARGB *)dest;
    if(ginfo->bpp <= 8)
        dest[dy*ginfo->width + dx] = (char)dest[sy*ginfo->width + sx];
    else
        d[dy*ginfo->width + dx] = d[sy*ginfo->width + sx];

}
*/

//------------------------------------------------------------------------------
// copy one scanline above down to the scanline below... This is for the missing
// scanlines that I don't render on each pass.
static void CopyScanline(int sx, int sy, int dx, int dy, char *dest,GINFO *ginfo)
{
    int sourcex,destx;
    for(sourcex = sx, destx = dx;sourcex < ginfo->width && destx < ginfo->width;sourcex++, destx++)
    {
        CopyPixel(sourcex,sy,destx,dy,dest,ginfo);
    }
}

//------------------------------------------------------------------------------
// basically copy the main light source but use the Area light axis to modify
// the location of the light source.
static void * CreateLightSource(RenderContext *RC,int index,Vec *ArAxis1,Vec *ArAxis2,int SizeW,int SizeH)
{
    int row = (int)index/SizeW;                     // where are we in our grid of lights?
    int col = (int)index%SizeW;
    Lumier * Light;
    Vec Inc1;                                       // my location finding vectors.
    Vec Inc2;
    Light = (Lumier *)galloc(sizeof(Lumier));
    Vinit(Inc1,ArAxis1->x,ArAxis1->y,ArAxis1->z);   // initialize my incrementing variables.
    Vinit(Inc2,ArAxis2->x,ArAxis2->y,ArAxis2->z);
    DivScalar(((float)SizeW),Inc1);                 // take these vars and divide by the number of lights to get single increment.
    DivScalar(((float)SizeH),Inc2);
    MulScalar(((float)row),Inc1);                   // now increment row and col num along each axis.
    MulScalar(((float)col),Inc2);                   // initialize my starting location as the location specified for the light source.
    Vinit(Light->Location,RC->Light.Location.x,RC->Light.Location.y,RC->Light.Location.z);
    AddVec(Light->Location,Light->Location,Inc1);   // Add my vectors to find where in the world I am.
    AddVec(Light->Location,Light->Location,Inc2);   // I add each increment individually, really I could just do a linear combination but I
    Light->Coefficient  = RC->Light.Coefficient;    // thought this would be more explicite.
    Light->FadeDistance = RC->Light.FadeDistance;   // make all my lights in grid almost identical
    Light->FadePower    = RC->Light.FadePower;      // to main light source. This is a waste of
    Light->Falloff      = RC->Light.Falloff;        // space but will allow for further growth
    Light->Next         = 0;                        // of filter later when we might add more
    Light->NumLights    = RC->Light.NumLights;      // light sources. So don't take shortcut here.
    Light->Radius       = RC->Light.Radius;
    Light->type         = RC->Light.type;
    CopyColour(Light->Colour,RC->Light.Colour);
    CopyVec(Light->Direction,RC->Light.Direction);
    return ((void *)Light);
}



#define FLOATIZE(a,b,c) {x = (float)(a);y = (float)(b);z = (float)(c);}
//------------------------------------------------------------------------------
// function fills out all default values for camera and parses options into
// main structure.
static void GetOptions(int numoptions, FOP *options,RenderContext * RC,GINFO * ginfo)
{
    float x,y,z;
    int  SizeW = 1;                                               // how many area lights there are
    int  SizeH = 1;                                               // in WxH.
    Vec ArAxis1;                                                  // the area light axises.
    Vec ArAxis2;
    Vec TempLightDirection;                                       // store the light look at point till I get a chance
    assert(RC);                                                   // to compute all the light look at points.
    assert(numoptions);
    assert(options);
    memset(RC,0,sizeof(RenderContext));
    RC->ScreenWidth  = ginfo->width;
    RC->ScreenHeight = ginfo->height;
    RC->TexturePlaneOffset = 1;
    RC->ShadowPlaneOffset  = 0;
    RC->TraceLevel         = 0;                                   // here for expandability later.
    RC->Light.NumLights    = 1;                                   // this is not a area light so only one light.
    RC->Light.Next         = 0;
    RC->commonclut         = 0;
    Vinit(RC->Cam.Up,         0.0, 1.0, 0.0);
    Vinit(RC->Cam.Right,     -1.33, 0.0, 0.0);
    Vinit(RC->Cam.Direction,  0.0, -1.0, 0.0);
    Vinit(RC->Cam.Location,   0.0, 2, 0.0);
//    Vinit(RC->Cam.Sky,        0.0, 1.0, 0.0);
    Vinit(RC->Cam.Sky,        0.0, 0.0, 1.0);                     // set the sky vector to positive z so rotations
    Vinit(RC->Cam.Look_At,    0.0, 0.0, 0.0);                     // happen normally.
    Vinit(RC->Light.Colour,    0.0, 0.0, 0.0);
    Vinit(RC->Light.Location,  0.0, 3.0, 0.2);
    Vinit(RC->Light.Direction, 0.0,-3.0,-0.2);
    Vinit(ArAxis1,1,0,0);
    Vinit(ArAxis2,0,0,1);
    Normalize(RC->Light.Direction,RC->Light.Direction);           // make light point at origin if spotlight.
    RC->Light.Coefficient = (float)2.0;                           // Tightness of the light source.
    RC->Light.type        = POINTLIGHT;
    RC->Light.Radius      = (float)0.9;
    RC->Light.Falloff     = (float)0.9;
    RC->Light.FadePower   = 0;
    RC->Light.FadeDistance= 0;
    if(numoptions >= 1) RC->TexturePlaneOffset   = (float)options[0].f;
    if(numoptions >= 2) RC->ShadowPlaneOffset    = (float)options[1].f;
    if(numoptions >= 5) {FLOATIZE(options[2].f,options[3].f,options[4].f);
                         Vinit(RC->Light.Location,    x, y, z);}
    if(numoptions >= 8){FLOATIZE(options[5].f,options[6].f,options[7].f);
                         Vinit(RC->Cam.Look_At,    x, y, z);}
    if(numoptions >= 9){RC->Light.type          = options[8].i;}
    if(numoptions >= 12){FLOATIZE(options[9].f,options[10].f,options[11].f);
                         Vinit(RC->Light.Direction,    x, y, z);}
    if(numoptions >= 13) {RC->Light.Radius       = (float)options[12].f;}
    if(numoptions >= 14) {RC->Light.Falloff      = (float)options[13].f;}
    if(numoptions >= 15) {RC->Light.FadePower    = (float)options[14].f;}
    if(numoptions >= 16) {RC->Light.FadeDistance = (float)options[15].f;}
    if(numoptions >= 19) {FLOATIZE(options[16].f,options[17].f,options[18].f);    // area light stuff.
                            Vinit(ArAxis1,    x, y, z);}
    if(numoptions >= 22) {FLOATIZE(options[19].f,options[20].f,options[21].f);
                            Vinit(ArAxis2,    x, y, z);}
    if(numoptions >= 23) {SizeW = (int)options[22].i;}                          // more area light stuff.
    if(numoptions >= 24) {SizeH = (int)options[23].i;}
    if(numoptions >= 27) {FLOATIZE(options[24].f,options[25].f,options[26].f);    // Colour of my backplane.
                            Vinit(RC->Light.Colour,  x, y, z);}
    if(numoptions >= 30) {FLOATIZE(options[27].f,options[28].f,options[29].f);    // Camera Location.
                            Vinit(RC->Cam.Location,    x, y, z);}
    if(numoptions >= 31) {RC->Light.Coefficient = (float)options[30].f;}          // Tightness of falloff.
    if(numoptions >= 32) RC->numstates = (int)options[31].i;
    if(numoptions >= 33) RC->mulsupersample = (int)options[32].i;
    //if(numoptions >= 34) RC->commonclut = (GINFO *)options[33].p;
    //if(numoptions >= 35) RC->globpal    = (int)options[34].i;

    if(RC->numstates <= 0)
        RC->numstates = 1;

    if(RC->mulsupersample <= 0)
        RC->mulsupersample = 1;

    RC->state = 0;
    // now compute the new camera direction, based on look_at change.
    {
         Vec TempVec;
         Sub(RC->Cam.Direction,RC->Cam.Look_At,RC->Cam.Location);
         if(fabs((RC->Cam.Direction.x + RC->Cam.Direction.y +  RC->Cam.Direction.z))  < ERRORMARGIN)
             Vinit(RC->Cam.Direction,  0.0, -1.0, 0.0);    // camera pos and look at cannot be the same point.
         Normalize(RC->Cam.Direction,RC->Cam.Direction);
         //Normalize(RC->Cam.Right,RC->Cam.Right);

         // don't recompute up and right allow that as a feature later if needed.
         CopyVec(TempVec,RC->Cam.Right);
         Cross(RC->Cam.Right,RC->Cam.Sky,RC->Cam.Direction);
         if((RC->Cam.Right.x < ERRORMARGIN) &&
            (RC->Cam.Right.y < ERRORMARGIN) &&
            (RC->Cam.Right.z < ERRORMARGIN)  )
         {
            CopyVec(RC->Cam.Right,TempVec);
         }
         Normalize(RC->Cam.Right,RC->Cam.Right);
         Cross(RC->Cam.Up,RC->Cam.Direction,RC->Cam.Right);
         Normalize(RC->Cam.Up,RC->Cam.Up);

    }
    // recompute light direction here.
    {
        Vec TempVec;
        CopyVec(TempLightDirection,RC->Light.Direction);
        Sub(TempVec,RC->Light.Location,RC->Light.Direction);     // treat what's in direction as a look at point.
        CopyVec(RC->Light.Direction,TempVec);
        Normalize(RC->Light.Direction,RC->Light.Direction);

    }
    // if lightsource is a cylinderlight or a spotlight then modify the radius and falloff
    // values here to make them apropriate for the implimentation (0 ->1 or 1 -> 0)
    {
        if(RC->Light.type == SPOTLIGHT || RC->Light.type == ARSPOTLIGHT)
        {
            // first do the radius. Set to go from 1.0 -> 0.0 where 1 = 0 degree's
            // and 0.0 = 90 degree's. Make sure to clamp to the valid range.
            RC->Light.Radius = CLAMPV(RC->Light.Radius,(float)90.0);
            RC->Light.Radius = (float)(90.0 - RC->Light.Radius);  // make go from 1.0 -> 0.0
            RC->Light.Radius = (float)(RC->Light.Radius / 90.0);
            // now do the falloff. The valid range is again the same.
            RC->Light.Falloff = CLAMPV(RC->Light.Falloff,(float)90.0);
            RC->Light.Falloff = (float)(90.0 - RC->Light.Falloff);  // make go from 1.0 -> 0.0
            RC->Light.Falloff = (float)(RC->Light.Falloff / 90.0);
        }
        else if(RC->Light.type == CYLINDRICAL || RC->Light.type == ARCYLINDRICAL)
        {
            // first do the radius. Set to go from 1.0 -> 0.0 where 1 = 0 degree's
            // and 0.0 = 90 degree's. Make sure to clamp to the valid range.
            RC->Light.Radius = CLAMPV(RC->Light.Radius,(float)90.0);
            RC->Light.Radius = (float)(90.0 - RC->Light.Radius);  // make go from 1.0 -> 0.0
            RC->Light.Radius = (float)(RC->Light.Radius / 90.0);
            // now do the falloff. this one is a bit strange, the value goes from 0 ->1 but can be greater
            // than 1 for some strange reason. really fun at any rate.
            RC->Light.Falloff = CLAMPV(RC->Light.Falloff,(float)90.0);
            RC->Light.Falloff *= 4.0;                               // scale by 4 to make it more intuitive.
            RC->Light.Falloff = (float)(RC->Light.Falloff / 90.0);
        }
    }

    // have to create my area light list. These allow me to create umbra and penumbra.
    if(RC->Light.type == ARSPOTLIGHT || RC->Light.type == ARCYLINDRICAL || RC->Light.type == ARPOINTLIGHT)
    {
        int i = 0;
        Lumier * Current = &RC->Light;
        // first find out how many lights I'm going to have to make.
        RC->Light.NumLights = (float)SizeW*SizeH;
        for(i = 0; i < (RC->Light.NumLights - 1);i++)   // note first light is one of the lights in grid.
        {
            // build all my light sources.
            Current->Next = (void *)CreateLightSource(RC,i,&ArAxis1,&ArAxis2,SizeW,SizeH);
            Current= (Lumier *)Current->Next;
            // now that we are at that light source make point at same point all the other
            // lights are pointing at... hopefully this will allow for soft shadows.
            // use the global light look at point as my guide to where to point all my other lights.
            //Sub(TempVec,Current->Location,TempLightDirection);     // treat what's in direction as a look at point.
            //CopyVec(Current->Direction,TempVec);
            //Normalize(Current->Direction,Current->Direction);

        }

    }

}
#undef FLOATIZE

/****************************************************************/
/* Filter Functions                                             */
/****************************************************************/

//------------------------------------------------------------------------------
FPUBLIC FABOUT *aboutdrops(void)
{
    FABOUT *about = (FABOUT *) galloc(sizeof(FABOUT));
    if (about)
    {
        memset(about, 0, sizeof(FABOUT));

        about->signature =          GMAKEID('D','R','O','P');
        about->size =               sizeof(FABOUT);
        about->version =            GFILTER_VERSION;
        about->filter8 =            0;                 // turn off 8bpp support
        about->filter32 =           1;                 // since it only seems to cause problems
        about->minbitmaps =         1;
        about->maxbitmaps =         1;
        about->filterclass =        FCLASS_GENERAL;
        strcpy(about->authorstr,    "Ian Davids");
        strcpy(about->versionstr,   "1.05");
        strcpy(about->namestr,      "DropS" );
        strcpy(about->longnamestr,  "Drop Shadow" );
        about->numoptions =         33;
        about->option[0].type =            FOP_FLOAT;
        about->option[0].min.f =           0;
        about->option[0].max.f =           1;
        about->option[0].def.f =           1;
        strcpy(about->option[0].name,      "TexPlaneOffest");
        about->option[1].type =            FOP_FLOAT;
        about->option[1].min.f =           -1;
        about->option[1].max.f =           .9;
        about->option[1].def.f =           0;
        strcpy(about->option[1].name,      "ShadowPlaneOffset");
        about->option[2].type =            FOP_FLOAT;
        about->option[2].min.f =           -100;
        about->option[2].max.f =           100;
        about->option[2].def.f =           0;
        strcpy(about->option[2].name,      "LightLocx");
        about->option[3].type =            FOP_FLOAT;
        about->option[3].min.f =           -100;
        about->option[3].max.f =           100;
        about->option[3].def.f =           3;
        strcpy(about->option[3].name,      "LightLocy");
        about->option[4].type =            FOP_FLOAT;
        about->option[4].min.f =           -100;
        about->option[4].max.f =           100;
        about->option[4].def.f =           0;
        strcpy(about->option[4].name,      "LightLocz");
        about->option[5].type =            FOP_FLOAT;
        about->option[5].min.f =           -100;
        about->option[5].max.f =           100;
        about->option[5].def.f =           0;
        strcpy(about->option[5].name,      "CamLookx");
        about->option[6].type =            FOP_FLOAT;
        about->option[6].min.f =           -100;
        about->option[6].max.f =           100;
        about->option[6].def.f =           0;
        strcpy(about->option[6].name,      "CamLooky");
        about->option[7].type =            FOP_FLOAT;
        about->option[7].min.f =           -100;
        about->option[7].max.f =           100;
        about->option[7].def.f =           0;
        strcpy(about->option[7].name,      "CamLookz");
        about->option[8].type =            FOP_INT;
        about->option[8].min.i =           0;
        about->option[8].max.i =           5;
        about->option[8].def.i =           POINTLIGHT;
        strcpy(about->option[8].name,      "LightType");
        about->option[9].type =            FOP_FLOAT;
        about->option[9].min.f =           -100;
        about->option[9].max.f =           100;
        about->option[9].def.f =           0;
        strcpy(about->option[9].name,      "LightDirx");
        about->option[10].type =            FOP_FLOAT;
        about->option[10].min.f =           -100;
        about->option[10].max.f =           100;
        about->option[10].def.f =           0;
        strcpy(about->option[10].name,      "LightDiry");
        about->option[11].type =            FOP_FLOAT;
        about->option[11].min.f =           -100;
        about->option[11].max.f =           100;
        about->option[11].def.f =           0;
        strcpy(about->option[11].name,      "LightDirz");
        about->option[12].type =            FOP_FLOAT;
        about->option[12].min.f =           0;
        about->option[12].max.f =           90;
        about->option[12].def.f =           30;
        strcpy(about->option[12].name,      "LightRad");
        about->option[13].type =            FOP_FLOAT;
        about->option[13].min.f =           0;
        about->option[13].max.f =           90;
        about->option[13].def.f =           35;
        strcpy(about->option[13].name,      "Falloff");
        about->option[14].type =            FOP_FLOAT;
        about->option[14].min.f =           0;
        about->option[14].max.f =           100;
        about->option[14].def.f =           0;
        strcpy(about->option[14].name,      "FadePower");
        about->option[15].type =            FOP_FLOAT;
        about->option[15].min.f =           0;
        about->option[15].max.f =           100;
        about->option[15].def.f =           0;
        strcpy(about->option[15].name,      "FadeDist");
        about->option[16].type =            FOP_FLOAT;
        about->option[16].min.f =           -100;
        about->option[16].max.f =           100;
        about->option[16].def.f =           1;
        strcpy(about->option[16].name,      "ArAxis1x");
        about->option[17].type =            FOP_FLOAT;
        about->option[17].min.f =           -100;
        about->option[17].max.f =           100;
        about->option[17].def.f =           0;
        strcpy(about->option[17].name,      "ArAxis1y");
        about->option[18].type =            FOP_FLOAT;
        about->option[18].min.f =           -100;
        about->option[18].max.f =           100;
        about->option[18].def.f =           0;
        strcpy(about->option[18].name,      "ArAxis1z");
        about->option[19].type =            FOP_FLOAT;
        about->option[19].min.f =           -100;
        about->option[19].max.f =           100;
        about->option[19].def.f =           0;
        strcpy(about->option[19].name,      "ArAxis2x");
        about->option[20].type =            FOP_FLOAT;
        about->option[20].min.f =           -100;
        about->option[20].max.f =           100;
        about->option[20].def.f =           0;
        strcpy(about->option[20].name,      "ArAxis2y");
        about->option[21].type =            FOP_FLOAT;
        about->option[21].min.f =           -100;
        about->option[21].max.f =           100;
        about->option[21].def.f =           1;
        strcpy(about->option[21].name,      "ArAxis2z");
        about->option[22].type =            FOP_INT;
        about->option[22].min.i =           0;
        about->option[22].max.i =           100;
        about->option[22].def.i =           1;
        strcpy(about->option[22].name,      "AreaNumLightW");
        about->option[23].type =            FOP_INT;
        about->option[23].min.i =           0;
        about->option[23].max.i =           100;
        about->option[23].def.i =           1;
        strcpy(about->option[23].name,      "AreaNumLightH");
        about->option[24].type =            FOP_FLOAT;
        about->option[24].min.f =           0;
        about->option[24].max.f =           1.0;
        about->option[24].def.f =           0;
        strcpy(about->option[24].name,      "ShadowColourR");
        about->option[25].type =            FOP_FLOAT;
        about->option[25].min.f =           0;
        about->option[25].max.f =           1.0;
        about->option[25].def.f =           0;
        strcpy(about->option[25].name,      "ShadowColourG");
        about->option[26].type =            FOP_FLOAT;
        about->option[26].min.f =           0;
        about->option[26].max.f =           1.0;
        about->option[26].def.f =           0;
        strcpy(about->option[26].name,      "ShadowColourB");
        about->option[27].type =            FOP_FLOAT;
        about->option[27].min.f =           0;
        about->option[27].max.f =           100;
        about->option[27].def.f =           0;
        strcpy(about->option[27].name,      "CamLocX");
        about->option[28].type =            FOP_FLOAT;
        about->option[28].min.f =           0;
        about->option[28].max.f =           100;
        about->option[28].def.f =           2;
        strcpy(about->option[28].name,      "CamLocY");
        about->option[29].type =            FOP_FLOAT;
        about->option[29].min.f =           0;
        about->option[29].max.f =           100;
        about->option[29].def.f =           0;
        strcpy(about->option[29].name,      "CamLocZ");
        about->option[30].type =            FOP_FLOAT;
        about->option[30].min.f =           0;
        about->option[30].max.f =           100;
        about->option[30].def.f =           2;
        strcpy(about->option[30].name,      "LightCoefficient");
        about->option[31].type =            FOP_INT;
        about->option[31].min.i =           1;
        about->option[31].max.i =           2000;
        about->option[31].def.i =           1;
        strcpy(about->option[31].name,      "NumPasses");
        about->option[32].type =            FOP_INT;
        about->option[32].min.i =           1;
        about->option[32].max.i =           4;
        about->option[32].def.i =           1;
        strcpy(about->option[32].name,      "MulSuperSample");
        //about->option[33].type =            FOP_PTR;
        //about->option[33].min.i =           0;
        //about->option[33].max.i =           -1;
        //about->option[33].def.i =           0;
        //strcpy(about->option[33].name,      "Ginfo common clut");
        //about->option[34].type =            FOP_INT;
        //about->option[34].min.i =           0;
        //about->option[34].max.i =           1;
        //about->option[34].def.i =           0;
        //strcpy(about->option[34].name,      "use hardcoded common font clut");


    }
    return(about);
}

//------------------------------------------------------------------------------
FPUBLIC int opendrops(FINSTANCE* *filterInstance,  int numsources, GBITMAP *sources, int numoptions, FOP *options)
{
    int ok=0;
    FINSTANCE *fx=0;
    RenderContext * RC = 0;
    fx = (FINSTANCE *)galloc(sizeof(FINSTANCE));
    RC = (RenderContext *)galloc(sizeof(RenderContext));
    if (fx)
    {
        memset(fx, 0, sizeof(FINSTANCE));
        fx->frames      = 1;
        fx->numsources  = numsources;
        fx->source      = sources;
        fx->numoptions  = numoptions;
        fx->option      = options;
        fx->fref        = (char *)RC;                   // bookeeping information.
        ok = 1;
    }
    *filterInstance = fx;
    return(ok);
}

//------------------------------------------------------------------------------
FPUBLIC GINFO *infodrops(FINSTANCE *fx, int framenum)
{
    GINFO *ginfo;
    RenderContext *RC = 0;
    int rowbytes = 0;
    assert(fx);


    RC = (RenderContext *)fx->fref;

    if (fx->option)                                                     // get my options and fill my RenderContext.
        GetOptions(fx->numoptions, fx->option,RC,fx->source[0].ginfo);

    rowbytes = fx->source[0].rowbytes;
    // now copy over the ginfo, it shouldn't change.
    ginfo = (GINFO *) galloc(fx->source[0].ginfo->size);
    if (ginfo)
        memcpy(ginfo, fx->source[0].ginfo, fx->source[0].ginfo->size);


    // if we are 8bpp then we must remap to shadowclut for clutmatching later.
//    if(ginfo->bpp == 8)
//    {
//            // have to generate new clut with space for shadow...
//            // call the generate clut function.
//        RC->data8 = GenerateClutAndRemap(RC,ginfo,image,rowbytes);
//    }
//    else
// Currently I do not support 8bpp data.

        RC->data8 = 0;

    // if we are 32bpp then we will use this buffer later.
    RC->data32 = ((ARGB *) fx->source[0].image);
    // save a copy of the ginfo structure so I don't have to pass it around as an extra param.
    RC->ginfo  = ginfo;
    return(ginfo);
}

//------------------------------------------------------------------------------
FPUBLIC int filterdrops(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes)
{
    int x        = 0;
    int y        = 0;
    int i        = 0;
    int countx   = 0;
    int county   = 0;
    int sssquare = 0;
    int imaghit  = 0;
    double da    = 0;
    double dx    = 0;
    double dy    = 0;
    double dz    = 0;
    Vec Colour;
    Vec * ColArr;
    RenderContext *RC;
    memset(dest,0,ginfo->width*ginfo->height*ginfo->bpp/8);

                                                  // save image info in RC.
    RC = (RenderContext *)fx->fref;
    RC->rowbytesdest = rowbytes;                  // save the image rowbytes for use in texture mapping.
    RC->rowbytessrc  = fx->source[0].rowbytes;


    sssquare = RC->mulsupersample*RC->mulsupersample;


    if(RC->numstates == 1)                        // if we are not doing multipass stuff then here's
    {                                             // simple loop.
        ColArr = (Vec *) galloc(sizeof(Vec)*sssquare);
        for (y=0; y<ginfo->height; ++y)
        {
            for (x=0; x<ginfo->width; ++x)        // now process this row.
            {
               da = dx = dy = dz = 0;
               imaghit = 0;
               for(i = 0; i < sssquare;i++)
               {
                   double valx = (x - (double)(RC->mulsupersample>>1)/(double)RC->mulsupersample)+ (double)(i%RC->mulsupersample)/(double)(RC->mulsupersample+1);
                   double valy = (y - (double)(RC->mulsupersample>>1)/(double)RC->mulsupersample)+ (double)(i/RC->mulsupersample)/(double)(RC->mulsupersample+1);
                   if(valx < 0) valx = 0;
                   if(valy < 0) valy = 0;
                   if(valx > (ginfo->width-1)) valx = (ginfo->width-1);
                   if(valy > (ginfo->height-1)) valy = (ginfo->height-1);
                   Trace(RC,valx,valy,&ColArr[i]);
                   da += ColArr[i].a;
                   dx += ColArr[i].x;
                   dy += ColArr[i].y;
                   dz += ColArr[i].z;
                   if(RC->imag)
                       imaghit = 1;

               }
               Colour.a = (float)((double)da / (double)sssquare);
               Colour.x = (float)((double)dx / (double)sssquare);
               Colour.y = (float)((double)dy / (double)sssquare);
               Colour.z = (float)((double)dz / (double)sssquare);
               PlotPixel(RC,x,y,&Colour,dest,ginfo,RC->rowbytesdest,imaghit);
            }
        }
        if(ColArr) gfree(ColArr);

    }
    else                                          // for image processing of images with multiple pass requirements.
    {
        for (y=RC->state,county = 0; y<ginfo->height; y += RC->numstates,county+= RC->numstates)
        {
            int tempy = y;
            for (x=RC->state,countx = 0; x<ginfo->width; x += RC->numstates,countx+= RC->numstates)        // now process this row.
            {
                int tempx = x;
                Trace(RC,x,y,&Colour);
                PlotPixel(RC,x,y,&Colour,dest,ginfo,RC->rowbytesdest,RC->imag);
                while(tempx < (countx+RC->numstates))
                {
                     tempx++;
                     CopyPixel(x,y,tempx,y,dest,ginfo);
                }
            }
            while(tempy < (county+RC->numstates))          // fill in the rest of the buffer with image colour.
            {
                 tempy++;
                 CopyScanline(0,y,0,tempy,dest,ginfo);
            }

        }
        RC->state++;
    }
    return(1);                                    // return success.
}

//------------------------------------------------------------------------------
FPUBLIC int closedrops(FINSTANCE *fx)
{
    RenderContext * RC = 0;
    Lumier * Current;
    Lumier * Next;
    assert(fx);
    if(fx->fref)
    {
        RC = (RenderContext *)fx->fref;
        if(RC->Light.NumLights > 1 && RC->Light.Next)
        {
            Current = (Lumier *)RC->Light.Next;
            while(Current)
            {
                Next = (Lumier *)Current->Next;
                if(Current)
                    gfree(Current);
                Current = Next;
            }
        }
        if(RC->data8)
            gfree(RC->data8);
        gfree(fx->fref);
    }
    return(gfree((void *)fx));
}





//-----------------------------
// undef all my constants so
// they do not conflict with
// other modules.
#undef Vinit
#undef SetColour
#undef LinComb
#undef LinComb2
#undef Cross
#undef Normalize
#undef Dot
#undef CLAMP
#undef CLAMPV
#undef Sub
#undef Length
#undef AddVec
#undef DivScalar
#undef MulScalar
#undef CopyColour
#undef AddColour
#undef CONVCOL
#undef CopyVec
#undef CopyPixel
#undef HUGEDIST
#undef CONVCOL
#undef MAXTRACELEVEL
#undef ERRORMARGIN
#undef SPOTLIGHT
#undef CYLINDRICAL
#undef POINTLIGHT
#undef TEXPLANE
#undef SHADOWPLANE
#undef DIFFUSECOEFFICIENT
