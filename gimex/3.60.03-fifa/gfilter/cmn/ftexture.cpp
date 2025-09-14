// (c) 1997-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*                   Texture - Texture Map Image                    */
/*                                                                  */
/*                        by Gerry Shaw, EAC                        */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History                                        */
/* ------- ------ -- -------                                        */
/*    1.00 970128 GS initial version                                */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 32 bit                                                           */
/* texture(numvertices, vertices);                                  */
/* int numvertices;                 (2 or vertices required)        */
/* FTEXTURE_VERTEX vertices[]       (array of vertices for polygon) */
/*                                                                  */
/* Source bitmap is optional, can generate lighted polygon          */
/*                                                                  */
/* Texture map an image or polygon with lighting, blinear filtering */
/* and alpha blending.                                              */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include "ftexture.h"
#include <string.h>


/****************************************************************/
/* Reusable Internal Functions                                  */
/****************************************************************/

static float minx =  99999.0f;
static float miny =  99999.0f;
static float maxx = -99999.0f;
static float maxy = -99999.0f;

#define TEXTUREINRANGE(val, Min,Max)    ((val >= Min) && (val < Max))
#define TEXTURECLAMP(val, Min,Max)      (val > Max ? Max : (val < Min ? Min : val))
#define TEXTUREFRAC(val)                (val - ((int)val))

static void FTEX_WritePixel32(GBITMAP* pDst, int x, int y, ARGB Color)
{
    int width    = pDst->ginfo->width;
    int height   = pDst->ginfo->height;
    int bpp      = pDst->ginfo->bpp;
    int rowbytes = pDst->rowbytes;

    x = (int)(x-minx);
    y = (int)(y-miny);

    if (TEXTUREINRANGE(x, 0, width) && TEXTUREINRANGE(y, 0, height))
    {
        ARGB* pBase = (ARGB*)((char *)pDst->image + y*rowbytes + x*bpp/8);

        pBase->a = Color.a;
        pBase->r = Color.r;
        pBase->g = Color.g;
        pBase->b = Color.b;
    }
}

static void FTEX_ReadPixel32(GBITMAP* pSrc, int x, int y, int* pValid, ARGB* pColor)
{
    int width    = pSrc->ginfo->width;
    int height   = pSrc->ginfo->height;
    int bpp      = pSrc->ginfo->bpp;
    int rowbytes = pSrc->rowbytes;

    if (TEXTUREINRANGE(x, 0, width) && TEXTUREINRANGE(y, 0, height))
    {
        ARGB* pBase = (ARGB*)((char *)pSrc->image + y*rowbytes + x*bpp/8);

        pColor->a = pBase->a;
        pColor->r = pBase->r;
        pColor->g = pBase->g;
        pColor->b = pBase->b;

        *pValid = 1;
    }
    else
    {
        pColor->a = 0;
        pColor->r = 0;
        pColor->g = 0;
        pColor->b = 0;

        *pValid = 0;
    }
}


static void FTEX_BlinearReadPixel(GBITMAP* pSrc, float u, float v, ARGB* pColor)
{
    int topvalid = 1;
    int botvalid = 1;

    /* source point (x1,y1) are real numbers (ie, 11.3, 12.4) */
    /* use bi-linear filtering on integer corner points to best value */

    int ax, ay; /* a = top left */
    int bx, by;  /* b = top right */
    int cx, cy;  /* c = bottom left */
    int dx, dy;  /* d = bottom right */

    ARGB a, b, c, d;

    float w,h;

    int avalid;     /* valid flags indicate if pixel value is valid  */
    int bvalid;     /* (false if pixel is outside of source image) */
    int cvalid;
    int dvalid;

    FARGB top;      /* top = a*(1-w)   + b*w */
    FARGB bot;      /* bot = c*(1-w)   + d*w */
    FARGB val;      /* val = top*(1-h) + bot*h */

    /* kinda tricky to handle the case when points are negative (pain in the butt) */
    int x = (int)u;
    int y = (int)v;

    if (u >= 0.0)   { ax = x; bx = x+1; cx = x;   dx = x+1; }
    else            { ax = x; bx = x-1; cx = x;   dx = x-1; }

    if (v >= 0.0)   { ay = y; by = y;   cy = y+1; dy = y+1; }
    else            { ay = y; by = y;   cy = y-1; dy = y-1; }

    FTEX_ReadPixel32(pSrc, ax,ay, &avalid, &a);
    FTEX_ReadPixel32(pSrc, bx,by, &bvalid, &b);
    FTEX_ReadPixel32(pSrc, cx,cy, &cvalid, &c);
    FTEX_ReadPixel32(pSrc, dx,dy, &dvalid, &d);

    /* again handle negatives */
    if (u >= 0.0)
        w = TEXTUREFRAC(u);
    else
        w = 1.0f - TEXTUREFRAC(u);

    if (v >= 0.0)
        h = TEXTUREFRAC(v);
    else
        h = 1.0f - TEXTUREFRAC(v);


    if (avalid && bvalid)
    {
        top.a = a.a*(1-w) + b.a*w;
        top.r = a.r*(1-w) + b.r*w;
        top.g = a.g*(1-w) + b.g*w;
        top.b = a.b*(1-w) + b.b*w;
    }
    else if (avalid)
    {
        top.a = a.a;
        top.r = a.r;
        top.g = a.g;
        top.b = a.b;
    }
    else if (bvalid)
    {
        top.a = b.a;
        top.r = b.r;
        top.g = b.g;
        top.b = b.b;
    }
    else
    {
        top.a = 0.0f;
        top.r = 0.0f;
        top.g = 0.0f;
        top.b = 0.0f;
        topvalid = 0;
    }

    if (cvalid && dvalid)
    {
        bot.a = c.a*(1-w) + d.a*w;
        bot.r = c.r*(1-w) + d.r*w;
        bot.g = c.g*(1-w) + d.g*w;
        bot.b = c.b*(1-w) + d.b*w;
    }
    else if (cvalid)
    {
        bot.a = c.a;
        bot.r = c.r;
        bot.g = c.g;
        bot.b = c.b;
    }
    else if (dvalid)
    {
        bot.a = d.a;
        bot.r = d.r;
        bot.g = d.g;
        bot.b = d.b;
    }
    else
    {
        bot.a = 0.0f;
        bot.r = 0.0f;
        bot.g = 0.0f;
        bot.b = 0.0f;
        botvalid = 0;
    }

    if (topvalid && botvalid)
    {
        val.a = top.a*(1-h) + bot.a*h;
        val.r = top.r*(1-h) + bot.r*h;
        val.g = top.g*(1-h) + bot.g*h;
        val.b = top.b*(1-h) + bot.b*h;
    }
    else if (topvalid)
    {
        val.a = top.a;
        val.r = top.r;
        val.g = top.g;
        val.b = top.b;
    }
    else if (botvalid)
    {
        val.a = bot.a;
        val.r = bot.r;
        val.g = bot.g;
        val.b = bot.b;
    }
    else
    {
        val.a = 0.0f;
        val.r = 0.0f;
        val.g = 0.0f;
        val.b = 0.0f;
    }

    pColor->a = (unsigned char)TEXTURECLAMP(val.a, 0.0,255.0);
    pColor->r = (unsigned char)TEXTURECLAMP(val.r, 0.0,255.0);
    pColor->g = (unsigned char)TEXTURECLAMP(val.g, 0.0,255.0);
    pColor->b = (unsigned char)TEXTURECLAMP(val.b, 0.0,255.0);
}


/****************************************************************/
/* Internal Functions                                           */
/****************************************************************/

typedef struct
{
    int                 NumVertices;
    FTEXTURE_VERTEX*    Vertices;
    GBITMAP*            pTexture;
} RENDERINFO;

/* TODO: make this #defines */
static int IncIndex(int val, int max)
{
    val++;
    if (val >= max)
        val = 0;
    return val;
}

/* TODO: make this #defines */
static int DecIndex(int val, int max)
{
    val--;
    if (val < 0)
        val = max-1;
    return val;
}

static void GetEdge(RENDERINFO* pRenderInfo, int VertexIndex, FTEXTURE_VERTEX* pEdge)
{
    memcpy(pEdge, &(pRenderInfo->Vertices[VertexIndex]), sizeof(FTEXTURE_VERTEX));
}

static void GetVelocity(FTEXTURE_VERTEX* p1, FTEXTURE_VERTEX* p2, FTEXTURE_VERTEX* pVelocity, float Delta)
{
    pVelocity->x           = (p2->x - p1->x) / Delta;
    pVelocity->y           = (p2->y - p1->y) / Delta;
    pVelocity->z           = (p2->z - p1->z) / Delta;
    pVelocity->w           = (p2->w - p1->w) / Delta;

    pVelocity->colour.a    = (p2->colour.a - p1->colour.a) / Delta;
    pVelocity->colour.r    = (p2->colour.r - p1->colour.r) / Delta;
    pVelocity->colour.g    = (p2->colour.g - p1->colour.g) / Delta;
    pVelocity->colour.b    = (p2->colour.b - p1->colour.b) / Delta;

    pVelocity->specular.a  = (p2->specular.a - p1->specular.a) / Delta;
    pVelocity->specular.r  = (p2->specular.r - p1->specular.r) / Delta;
    pVelocity->specular.g  = (p2->specular.g - p1->specular.g) / Delta;
    pVelocity->specular.b  = (p2->specular.b - p1->specular.b) / Delta;

    pVelocity->u           = (p2->u - p1->u) / Delta;
    pVelocity->v           = (p2->v - p1->v) / Delta;
}

static void UpdateEdge(FTEXTURE_VERTEX* pVertex, FTEXTURE_VERTEX* pVelocity)
{
    pVertex->x           += pVelocity->x;
    pVertex->y           += pVelocity->y;
    pVertex->z           += pVelocity->z;
    pVertex->w           += pVelocity->w;
    pVertex->colour.a    += pVelocity->colour.a;
    pVertex->colour.r    += pVelocity->colour.r;
    pVertex->colour.g    += pVelocity->colour.g;
    pVertex->colour.b    += pVelocity->colour.b;
    pVertex->specular.a  += pVelocity->specular.a;
    pVertex->specular.r  += pVelocity->specular.r;
    pVertex->specular.g  += pVelocity->specular.g;
    pVertex->specular.b  += pVelocity->specular.b;
    pVertex->u           += pVelocity->u;
    pVertex->v           += pVelocity->v;
}

static void RenderPixel(GBITMAP* pDst, int x, int y, FTEXTURE_VERTEX* pVertex, GBITMAP* pTexture)
{
    if (TEXTUREINRANGE(x-minx, 0, pDst->ginfo->width) && TEXTUREINRANGE(y-miny, 0, pDst->ginfo->height))
    {
        ARGB TColor;        // color read from texture, transparent black if no texture
        ARGB LColor;        // color after lighting is applied

        // get texture color
        if (pTexture)
        {
            FTEX_BlinearReadPixel(pTexture, pVertex->u, pVertex->v, &TColor);
        }
        else
        {
            // set color to transparent black, it will get color from lighting (next step)
            TColor.a = TColor.r = TColor.g = TColor.b = 255;
        }

        // apply lighting
        LColor.a = (unsigned char)TEXTURECLAMP((pVertex->colour.a * TColor.a) + pVertex->specular.a*255.0f, 0.0,255.0);
        LColor.r = (unsigned char)TEXTURECLAMP((pVertex->colour.r * TColor.r) + pVertex->specular.r*255.0f, 0.0,255.0);
        LColor.g = (unsigned char)TEXTURECLAMP((pVertex->colour.g * TColor.g) + pVertex->specular.g*255.0f, 0.0,255.0);
        LColor.b = (unsigned char)TEXTURECLAMP((pVertex->colour.b * TColor.b) + pVertex->specular.b*255.0f, 0.0,255.0);

        // write to buffer
        FTEX_WritePixel32(pDst, x,y, LColor);
    }
}

void RenderLine(GBITMAP* pDst, FTEXTURE_VERTEX* pLeftEdge, FTEXTURE_VERTEX* pRightEdge, int y, GBITMAP* pTexture)
{
    int x;
    int x_min;
    int x_max;

    FTEXTURE_VERTEX Velocity;
    FTEXTURE_VERTEX Vertex;

    float Delta = pRightEdge->x - pLeftEdge->x;
    if (Delta == 0.0f)
    {
        // draw a single pixel and then return (this allows 2 point polys)
        RenderPixel(pDst, (int)(pLeftEdge->x),y, pLeftEdge, pTexture);
        return;
    }
    if (pRightEdge->x - pLeftEdge->x < 0)
    {
        // swap values
        FTEXTURE_VERTEX* pTempEdge = pRightEdge;
        pRightEdge = pLeftEdge;
        pLeftEdge = pTempEdge;
        Delta = -Delta;
    }

    GetVelocity(pLeftEdge, pRightEdge, &Velocity, Delta);
    memcpy(&Vertex, pLeftEdge, sizeof(FTEXTURE_VERTEX));

    x_min = (int)(pLeftEdge->x);
    x_max = (int)(pRightEdge->x);
    for (x = x_min; x < x_max; x++)
    {
        RenderPixel(pDst, x,y, &Vertex, pTexture);
        UpdateEdge(&Vertex, &Velocity);
    }
}

static void RenderPoly(GBITMAP* pDst, RENDERINFO* pRenderInfo)
{
    int y;                          // current row being processed
    int y_min;                      // top most y value in poly
    int y_max;                      // bottom most y value in poly

    int VertexIndex;                // temp used when finding top/bottom vertices
    int TopVertexIndex = 0;         // index of the top most vertex
    int BottomVertexIndex;          // index of the bottom most vertex
    int LeftVertexIndex  = 0;       // current index the left edge is working on
    int RightVertexIndex = 0;       // current index the right edge is working on

    FTEXTURE_VERTEX LeftEdge;           // current left edge being processed
    FTEXTURE_VERTEX RightEdge;          // current right edge being processed
    FTEXTURE_VERTEX NextLeftEdge;       // next left edge to be processed
    FTEXTURE_VERTEX NextRightEdge;      // next right edge to be processed

    FTEXTURE_VERTEX LeftVelocity;       // rate of change of edge values along left slope
    FTEXTURE_VERTEX RightVelocity;      // rate of change of edge values along right slope

    int NumVertices = pRenderInfo->NumVertices; // shortcut

    // find top vertex
    y_min = 99999;
    for (VertexIndex = 0; VertexIndex < NumVertices; VertexIndex++)
    {
        int y_test = (int)(pRenderInfo->Vertices[VertexIndex].y);
        if (y_test < y_min)
        {
            y_min = y_test;
            TopVertexIndex = VertexIndex;
        }
    }

    // find bottom vertex
    y_max = 0;
    for (VertexIndex = 0; VertexIndex < NumVertices; VertexIndex++)
    {
        int y_test = (int)(pRenderInfo->Vertices[VertexIndex].y);
        if (y_test > y_max)
        {
            y_max = y_test;
            BottomVertexIndex = VertexIndex;
        }
    }

    // set initial left vertex to top vertex
    LeftVertexIndex  = TopVertexIndex;
    GetEdge(pRenderInfo, LeftVertexIndex, &LeftEdge);

    // set initial right vertex to the last vertex on the same line as the top vertex
    // this will make sure horizontal edges are drawn correctly
    // do this by checking one index pass the one we want, once we find the first one we
    // don't want, we will have the one that we do want
    RightVertexIndex = TopVertexIndex;
    GetEdge(pRenderInfo, RightVertexIndex+1, &RightEdge);
    while (RightEdge.y <= LeftEdge.y)
    {
        RightVertexIndex = IncIndex(RightVertexIndex, NumVertices);
        GetEdge(pRenderInfo, RightVertexIndex+1, &RightEdge);
    }
    GetEdge(pRenderInfo, RightVertexIndex, &RightEdge); // this is the one we want (not index+1)


    // find the next left vertex (keepin decrementing index until next y value is below current y
    NextLeftEdge = LeftEdge;
    while (NextLeftEdge.y <= LeftEdge.y)
    {
        LeftVertexIndex  = DecIndex(LeftVertexIndex, NumVertices);
        GetEdge(pRenderInfo, LeftVertexIndex, &NextLeftEdge);
    }

    // now find next right vertex index in the same mannar as next left vertex (see above)
    NextRightEdge = RightEdge;
    while (NextRightEdge.y <= RightEdge.y)
    {
        RightVertexIndex = IncIndex(RightVertexIndex, NumVertices);
        GetEdge(pRenderInfo, RightVertexIndex, &NextRightEdge);
    }

    // calc initial velocity for both edges
    GetVelocity(&LeftEdge,  &NextLeftEdge,  &LeftVelocity, (NextLeftEdge.y - LeftEdge.y));
    GetVelocity(&RightEdge, &NextRightEdge, &RightVelocity, (NextRightEdge.y - RightEdge.y));

    // begin row loop
    for (y = y_min; y < y_max; y++)
    {
        // check to see if we have past the vertex in the current segment
        // if edge has run out, get another vertex that is below it

        if (y > NextLeftEdge.y)
        {
            // start of new edge is end of last edge
            LeftEdge = NextLeftEdge;

            // find end of new edge
            while (NextLeftEdge.y <= LeftEdge.y)
            {
                LeftVertexIndex  = DecIndex(LeftVertexIndex, NumVertices);
                GetEdge(pRenderInfo, LeftVertexIndex, &NextLeftEdge);
            }

            GetVelocity(&LeftEdge, &NextLeftEdge, &LeftVelocity, (NextLeftEdge.y - LeftEdge.y));
        }

        if (y > NextRightEdge.y)
        {
            // start of new edge is end of last edge
            RightEdge = NextRightEdge;

            // find end of new edge
            while (NextRightEdge.y <= RightEdge.y)
            {
                RightVertexIndex = IncIndex(RightVertexIndex, NumVertices);
                GetEdge(pRenderInfo, RightVertexIndex, &NextRightEdge);
            }

            GetVelocity(&RightEdge, &NextRightEdge, &RightVelocity, (NextRightEdge.y - RightEdge.y));
        }

        RenderLine(pDst, &LeftEdge, &RightEdge, y, pRenderInfo->pTexture);

        // update edges based on edge velocities
        UpdateEdge(&LeftEdge,  &LeftVelocity);
        UpdateEdge(&RightEdge, &RightVelocity);
    }
}

/* clean defines for this section */
#undef TEXTUREINRANGE
#undef TEXTURECLAMP
#undef TEXTUREFRAC

/****************************************************************/
/* Filter Functions                                             */
/****************************************************************/

FPUBLIC FABOUT* abouttexture(void)
{
    FABOUT* about = (FABOUT*) galloc(sizeof(FABOUT));
    if (about)
    {
        memset(about, 0, sizeof(FABOUT));

        about->signature =          GMAKEID('F','T','E','X');
        about->size =               sizeof(FABOUT);
        about->version =            GFILTER_VERSION;
        about->filter8 =            0;
        about->filter32 =           1;
        about->minbitmaps =         0;
        about->maxbitmaps =         1;
        about->filterclass =        FCLASS_GENERAL;
        strcpy(about->authorstr,    "Gerry Shaw");
        strcpy(about->versionstr,   "1.00");
        strcpy(about->namestr,      "Texture");
        strcpy(about->longnamestr,  "Texture Map");
        about->numoptions =         2;

        about->option[0].type =            FOP_INT;
        about->option[0].min.i =           2;
        about->option[0].max.i =           999;
        about->option[0].def.i =           3;
        strcpy(about->option[0].name,      "NumVertices");

        about->option[1].type =            FOP_PTR;
        strcpy(about->option[1].name,      "Vertices");
    }
    return about;
}

FPUBLIC int opentexture(FINSTANCE** FilterInstance, int numsources, GBITMAP sources[], int numoptions, FOP options[])
{
    int ok=0;
    FINSTANCE* fx=0;

    /* allocate instance */
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
    *FilterInstance = fx;
    return ok;
}


FPUBLIC GINFO* infotexture(FINSTANCE* fx, int framenum)
{
    GINFO* ginfo = NULL;

    // to calc size look at all the vertices and track the min/max x/y values

    FTEXTURE_VERTEX* pVertexList = (FTEXTURE_VERTEX*)(fx->option[1].p);

    int NumVertices = fx->option[0].i;
    int VertexIndex;
    for (VertexIndex = 0; VertexIndex < NumVertices; VertexIndex++)
    {
        FTEXTURE_VERTEX* pVertex = &(pVertexList[VertexIndex]);

        if (pVertex->x > maxx)
            maxx = pVertex->x;
        if (pVertex->x < minx)
            minx = pVertex->x;
        if (pVertex->y > maxy)
            maxy = pVertex->y;
        if (pVertex->y < miny)
            miny = pVertex->y;
    }

    /* check for valid size texture */
    if ((int)(maxx-minx > 0 && (int)(maxy-miny) > 0))
    {
        ginfo = (GINFO*)galloc(fx->source[0].ginfo->size);
        if (ginfo)
        {
            /* copy ginfo to destination */
            memcpy(ginfo, fx->source[0].ginfo, fx->source[0].ginfo->size);
            ginfo->framenum = framenum;
            ginfo->width    = (int)(maxx-minx);
            ginfo->height   = (int)(maxy-miny);

            /* scale center point and hotspots */
            ginfo->defaultx += (int)minx;
            ginfo->defaulty += (int)miny;
        }
    }
    return ginfo;
}


FPUBLIC int filtertexture(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes)
{
    RENDERINFO RenderInfo;
    GBITMAP    Dst;

    RenderInfo.NumVertices = fx->option[0].i;
    RenderInfo.Vertices = (FTEXTURE_VERTEX*)fx->option[1].p;

    if (fx->numsources)
        RenderInfo.pTexture = &(fx->source[0]);
    else
        RenderInfo.pTexture = NULL;

    Dst.image    = dest;
    Dst.ginfo    = ginfo;
    Dst.rowbytes = rowbytes;

    RenderPoly(&Dst, &RenderInfo);
    return 1;
}

FPUBLIC int closetexture(FINSTANCE* fx)
{
    return gfree((void*)fx);
}
