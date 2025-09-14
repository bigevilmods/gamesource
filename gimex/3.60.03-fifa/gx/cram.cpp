// (c) 1995-2004 Electronic Arts Inc.

/*------------------------------------------------------------------*/
/*                                                                  */
/*                         VRAM Optimizer                           */
/*                                                                  */
/*                  by L.Ancessi, F.Barchard, EAC                   */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History:                                       */
/* ------- ----   -- --------                                       */
/*    1.00 95xxxx LA initial version in Idefix                      */
/*    1.01 960508 FB extracted code from Idefix for gx              */
/*    1.02 960509 GS renamed structures to be MFC friendly          */
/*    1.03 960509 FB free block                                     */
/*    1.04 960715 GS enablerotate option works                      */
/*    1.05 970207 GS correct error handling when cram fails         */
/*    1.06 970210 FB allow initial rotate and done flags            */
/*    1.07 970602 FB ignoreuv 0=no,1=maybe,2=yes                    */
/*    2.00 971008 GS new API interface                              */
/*    2.01 990505 LC added a handler to pass around                 */
/*    2.02 990506 LC check for if texture is not within 256 boundary*/
/*    2.03 990607 LC added exception if texture > 256 boundary      */
/*    2.04 990607 LC option for odd width/even height to odd height */
/*    2.05 010823 LC havent updated log in a while - look at cramany*/
/*    2.06 010823 LC added cram->space for space between textures   */
/*                                                                  */
/*------------------------------------------------------------------*/

#include "cram.h"
#include "gimex.h"
#include "memory.h"
#include "math.h"
#include <stdlib.h>



/* global cram state */
struct CRAM
{
    int texturepagealignment;
    int texturepageheight;
    int texturepagewidth;
    int x;
    int y;
    int width;
    int height;
    bool rotate;
    int checktexturepage;
    int checkedges;
    int initalized;
    int platform;
    int odd_odd;
	int widthin;
	int space;
};


/*static CRAM cram; */

/*---------------------------------------------------------------------- */
/* Option */
/*---------------------------------------------------------------------- */

typedef struct { int v; int h; } CPOINT;
typedef struct { int top; int left; int bottom; int right; } CRECT;

bool gHgpu=false;    /* Speed up GPU */

/*---------------------------------------------------------------------- */
/* Type */
/*---------------------------------------------------------------------- */

typedef struct ShapeRecord
{
    CPOINT Size;               /* The current size of the shape */
    int    Bpp;                /* The current color depth (4,8,16) */
    bool   Rotate;             /* Flag to rotate the shape in the VRAM */
    CPOINT Pos;                /* The current position in the VRAM */
    int    Done;               /* A working value flag */
} ShapeRecord;

typedef struct ImageList
{
    int         NbShape;            /* The number of Shape */
    ShapeRecord WorkList[1];        /* The list of Work record for each shapes */
} ImageList,*ImageListPtr,**ImageListHdle;

/*---------------------------------------------------------------------- */
/* Optimisation of the list of shapes "TheShapes" */
/* in the VRAM space define by "VRAMRect" */
/*---------------------------------------------------------------------- */

bool VRAMOptimisation(CRAM *cram, ImageListHdle TheShapes,CRECT VRAMRect);

CPOINT GetSize(CRECT Rect)
{
    CPOINT rsize;

    rsize.v = Rect.bottom-Rect.top;
    rsize.h = Rect.right-Rect.left;
    return(rsize);
}

/*------------------------------------------------------------------------------ */
/* VRAM Optimisation search rectangle configuration */
/*------------------------------------------------------------------------------ */

CRECT   gStackRect[10000];  /* Global stack of 10000 rectangles */
int     gCountRect;

CPOINT GetVRAMSize(CRAM *cram, int Bpp,CPOINT Size,bool Rotate)
{
    short   TSwitch;

    if (Rotate)                                                             /* Rotate the bounds so, switch h and w */
    {
        TSwitch=(short)Size.h;
        Size.h=Size.v;
        Size.v=TSwitch;
    }

    if (cram->platform == CRAM_PLATFORM_PSX)
    {
        /* 4 bits shape */
        if (Bpp==4) Size.h=(Size.h+3)/4;
        /* 8 bits shape */
        if (Bpp==8) Size.h=(Size.h+1)/2;
    }
    return(Size);
}

void PushStackRect(CRECT r)
{
    gStackRect[gCountRect++]=r;                                             /* Add a rectangle in the stack */
}

bool SearchTargetRectangle(int *iTargetRectangle)
{
    int    i,CurrentDelta,BestDelta=0;

    *iTargetRectangle=-1;
    for (i=0; i<gCountRect; i++)
    {
        CurrentDelta=gStackRect[i].right-gStackRect[i].left;
        if (CurrentDelta)                                                   /* The rectangle is not empty */
        {
            if ((CurrentDelta>BestDelta)||(*iTargetRectangle==-1))          /* Best value or first valid value */
            {
                BestDelta=CurrentDelta;
                *iTargetRectangle=i;
            }
        }
    }
    if (*iTargetRectangle!=-1) return(true);                                    /* A target rectangle matched */
    else return(false);
}

bool CheckSize(CRAM *cram, CRECT targetRect, CPOINT shapeSize, CPOINT size,int bpp)
{
    /* return true if the shape can fit in the target rectangle */

	int cramSpace, touchEdge;
	cramSpace = cram->space;
	touchEdge = (targetRect.bottom%cram->texturepageheight) + (targetRect.right%cram->texturepagewidth);

	// if the target rectangle is on the edge of a texturepage, we'll ignore the extra spacing because we allow textures to be put on the edge of a texturepage
	if (!touchEdge)
		cramSpace = 0;

    /* The shape can fit in the target rectangle */
    if (((GetSize(targetRect).h - (shapeSize.h + cramSpace)) >= 0) && ((GetSize(targetRect).v - (shapeSize.v + cramSpace)) >= 0))
    {
        int x,y,u,v,distx;
		//int disty;
        CPOINT vsize;

/*          int ignoreuv = 2; \\ 2=0,1=CRAM_CHECKUV_IF_SMALLER,0=CRAM_CHECKUV */
/*          if (cram.flags & CRAM_CHECKUV_IF_SMALLER) */
/*              ignoreuv = 1; */
/*          else if (cram.flags & CRAM_CHECKUV) */
/*              ignoreuv = 0; */
/* */
/*          if (flags == 0) */
/*              return true; */
/* */

        x = targetRect.left;                /* The position of the shape */
        y = targetRect.top;
        u = x % cram->texturepagewidth;      /* Calculation of the uv coordinate */
        v = y % cram->texturepageheight;    // the u,v coordinates only give us the difference with the texture page size

        if (cram->checktexturepage == CRAM_IGNORE_TEXTUREPAGE)
            return true;

        /* force image to respect texture pages only if image is larger than the alignment  */
        if ((cram->checktexturepage == CRAM_CHECK_TEXTUREPAGE_IF_POSSIBLE) &&
            (size.h > cram->texturepagealignment || size.v > cram->texturepageheight))
        {
            /* if texture is bigger than texture page alignment and bigger than texture page, force in the texture page */
            if (size.h > cram->texturepagewidth) 
				if (cram->widthin) return false;
				else return true;
			
            distx = 0;
            if (cram->platform == CRAM_PLATFORM_PSX)
			{
                distx = ((x%cram->texturepagewidth)+GetVRAMSize(cram, bpp, size, cram->rotate).h)*16/bpp;
			}
            else
			{
                distx = (x%cram->texturepagewidth)+size.h; // x distance of the shape from the left side of the texturepage
			}


            /* checking texture page boundary limitation  */
			if (cram->checkedges && (distx < cram->texturepagewidth)) return true;
            if (!cram->checkedges && (distx <= cram->texturepagewidth)) return true;
        }
/*            if (ignoreuv==2 || (ignoreuv==1 && (w > cram.texturepagewidth || h > cram.texturepageheight))) */
/*               return(true); */

        /* the texture page is odd width and even height, increase height so it is odd too (Sony problem) */
        /* but only allow if not out of bounds */
        if (cram->odd_odd && (shapeSize.h&1) && !(shapeSize.v&1) && (((shapeSize.v+y+1)>targetRect.bottom) || ((shapeSize.v+y+1)>=(cram->y+cram->height))))
            return false;

        vsize = GetVRAMSize(cram, bpp, size, cram->rotate);

        if (cram->checkedges)
        {
            if ((u   < cram->texturepagewidth) &&
                (u+vsize.h < cram->texturepagewidth) &&
                (v   < cram->texturepageheight) &&
                (v+vsize.v < cram->texturepageheight))
            {
                return true;
            }
        }
        else
        {
            if ((u   <= cram->texturepagewidth) &&
                (u+vsize.h <= cram->texturepagewidth) &&
                (v   <= cram->texturepageheight) &&
                (v+vsize.v <= cram->texturepageheight))
            {
                return true;
            }
        }
    }
    return false;
}

/*            x=TargetRectangle.left;                                         \* The position of the shape *\ */
/*            y=TargetRectangle.top;                                          \* The position of the shape *\ */
/*            u=x%64;v=y%256;                                                 \* Calculation of the uv coordinate *\ */
/*            if (Bpp==4) u<<=2; */
/*            if (Bpp==8) u<<=1; */
/*            if (u<256) */
/*            { */
/*                if ((u+w)<256) */
/*                { */
/*                    if (v<256) */
/*                    { */
/*                        if ((v+h)<256) return(true); */
/*                    } */
/*                } */
/*            } */
/*   } */
/*    return false; */
/*} */

bool SearchShapeRect(CRAM *cram, CRECT TargetRectangle,ImageListHdle TheShapes,int *iShape,bool *Rotate)
{
    int    NbShape,i,Bpp;
    int    CurrentDelta,BestDelta=0;
    CPOINT  onSizeShape,nSizeShape,cnSizeShape;
    CPOINT  orSizeShape,rSizeShape,crSizeShape;

    *iShape=-1;
    NbShape=(**TheShapes).NbShape;
    for (i=0; i<NbShape; i++)                                               /* Scan all the shapes */
    {
        if (!(**TheShapes).WorkList[i].Done)                                /* It is a free rectangle */
        {
            Bpp=(**TheShapes).WorkList[i].Bpp;
            onSizeShape=(**TheShapes).WorkList[i].Size;
            nSizeShape=GetVRAMSize(cram,Bpp,onSizeShape,false);                  /* Reel size of the shape in the VRAM */
            cnSizeShape=GetVRAMSize(cram,16,onSizeShape,false);                  /* Size to check uv coordinate */
            orSizeShape=(**TheShapes).WorkList[i].Size;
            rSizeShape=GetVRAMSize(cram,Bpp,orSizeShape,true);                   /* Reel size of the shape in the VRAM */
            crSizeShape=GetVRAMSize(cram,16,orSizeShape,true);                   /* Size to check uv coordinate */
            if (CheckSize(cram,TargetRectangle,nSizeShape,cnSizeShape,Bpp))      /* Ckeck the configuration of this shape in the target rectangle */
            {
				int cramSpace, touchEdge;
				cramSpace = cram->space;
				touchEdge = (TargetRectangle.bottom % cram->texturepageheight) + (TargetRectangle.right % cram->texturepagewidth);

				// if the target rectangle is on the edge of a texturepage, we'll ignore the extra spacing because we allow textures to be put on the edge of a texturepage
				if (!touchEdge)
					cramSpace = 0;

                CurrentDelta=GetSize(TargetRectangle).h-(nSizeShape.h + cramSpace);			// this holds the horizontal wasted space
                if ((CurrentDelta<BestDelta)||(*iShape==-1))                    // if the wasted space is smaller, take that shape
                {
                    // this will always be true because of !gHgpu (gHgpu is false)
					if (((gHgpu)&&(*iShape==-1))||((gHgpu)&&(nSizeShape.v>=nSizeShape.h))||(!gHgpu)||(!(cram->rotate)))
                    {
                        *iShape=i;
                        BestDelta=CurrentDelta;
                        *Rotate=false;
                    }

                }
            }
            if (cram->rotate)
            {
                if (CheckSize(cram,TargetRectangle,rSizeShape,crSizeShape,Bpp))  /* Ckeck the configuration of this shape in the target rectangle */
                {
					int cramSpace, touchEdge;
					cramSpace = cram->space;
					touchEdge = (TargetRectangle.bottom % cram->texturepageheight) + (TargetRectangle.right%cram->texturepagewidth);

					// if the target rectangle is on the edge of a texturepage, we'll ignore the extra spacing because we allow textures to be put on the edge of a texturepage
					if (!touchEdge)
						cramSpace = 0;
                    CurrentDelta=GetSize(TargetRectangle).h-(rSizeShape.h + cramSpace);
                    if ((CurrentDelta<BestDelta)||(*iShape==-1))                /* The result is better that the previous */
                    {
                        if (((gHgpu)&&(*iShape==-1))||((gHgpu)&&(rSizeShape.v>=rSizeShape.h))||(!gHgpu))
                        {
                            *iShape=i;
                            BestDelta=CurrentDelta;
                            *Rotate=true;
                        }
                    }
                }
            }
        }
    }
    if (*iShape!=-1) 
		return(true);                                          /* A shape matched */
    else 
		return(false);
}

void KillTargetRectangle(int iTargetRectangle)
{
    int        iTL=-1,iBR=-1;
    int        i;
    bool     KillIt;

    for (i=0; i<gCountRect; i++)                                            /* Search the 2 rectangle that touch the one to kill */
    {
        if (i!=iTargetRectangle)
        {
            if (gStackRect[iTargetRectangle].top==gStackRect[i].bottom) iTL=i;
            if (gStackRect[iTargetRectangle].bottom==gStackRect[i].top) iBR=i;
        }
    }
    if ((iTL!=-1)&&(iBR!=-1))                                               /* The rectangle is in the middle */
    {
        if (gStackRect[iTL].left<gStackRect[iBR].left) iBR=-1;              /* Find the best one */
        else iTL=-1;
    }
    if (iTL!=-1) gStackRect[iTL].bottom=gStackRect[iTargetRectangle].bottom;/* Merge the 2 rectangle */
    if (iBR!=-1) gStackRect[iBR].top=gStackRect[iTargetRectangle].top;      /* Merge the 2 rectangle */
    KillIt=true;
    if ((gCountRect==1)&&(GetSize(gStackRect[iTargetRectangle]).h>64))      /* Cross boundary */
    {
        gStackRect[iTargetRectangle].left=((gStackRect[iTargetRectangle].left+64)/64)*64;
        if (gStackRect[iTargetRectangle].left<gStackRect[iTargetRectangle].right) KillIt=false;
    }
    if (KillIt)
    {
        gStackRect[iTargetRectangle]=gStackRect[gCountRect-1];              /* Kill the rectangle */
        gCountRect--;
    }
}

bool VRAMOptimisation(CRAM *cram, ImageListHdle TheShapes,CRECT VRAMRect)
{
    int        CountProcess,NbShape,Bpp;
    int        i,iTargetRectangle,iShape;
    CPOINT      SizeShape;
    CRECT       TargetRectangle;
    bool     Rotate;

    gCountRect=0;                                                           /* Reset the stack */
    PushStackRect(VRAMRect);                                                /* Push in the list the start rectangle */
    NbShape=(**TheShapes).NbShape;

    CountProcess=0;
    for (i=0; i<NbShape; ++i)
        if ((**TheShapes).WorkList[i].Done)     /* count # done already */
            ++CountProcess;

    while (CountProcess<NbShape)                                            /* Process all the shapes */
    {
        if (!SearchTargetRectangle(&iTargetRectangle))
            return(false);       /* Search the next rectangle to fit */
        TargetRectangle=gStackRect[iTargetRectangle];
        if (SearchShapeRect(cram,TargetRectangle,TheShapes,&iShape,&Rotate))     /* Search a shapes that can fit this rectangle */
        {
            (**TheShapes).WorkList[iShape].Rotate=Rotate;                   /* The shape is rotated to fit in the target rectangle */
/*              (**TheShapes).WorkList[iShape].Pos=TargetRectangle[topLeft];    \\ Take the top left of the target rectangle */
            (**TheShapes).WorkList[iShape].Pos.v=TargetRectangle.top;   /* Take the top left of the target rectangle */
            (**TheShapes).WorkList[iShape].Pos.h=TargetRectangle.left;  /* Take the top left of the target rectangle */
            (**TheShapes).WorkList[iShape].Done=true;                       /* Cant use this shape anymore */
            CountProcess++;                                                 /* One more done */
            Bpp=(**TheShapes).WorkList[iShape].Bpp;
            SizeShape=(**TheShapes).WorkList[iShape].Size;
            SizeShape=GetVRAMSize(cram, Bpp,SizeShape,Rotate);                    /* Calculation the reel size of the shape in the VRAM */
			
			gStackRect[iTargetRectangle].top+=SizeShape.v;                  /* Reduce the size of the existing target rectangle */
			if ((gStackRect[iTargetRectangle].top % cram->texturepageheight) != 0)    /* add on the extra space if the rectangle isnt on the edge of a texturepage */
				gStackRect[iTargetRectangle].top+= cram->space;                  /* Reduce the size of the existing target rectangle */
            
			TargetRectangle.bottom=TargetRectangle.top+SizeShape.v;
			if ((TargetRectangle.bottom%cram->texturepageheight) != 0)			/* add on the extra space if the rectangle isnt on the edge of a texturepage */
				TargetRectangle.bottom+= cram->space;
            
			if (cram->odd_odd && (SizeShape.h&1) && !(SizeShape.v&1))    /* if odd width & even height, add one extra row of pixels to make even height */
            {
                ++gStackRect[iTargetRectangle].top;
                ++TargetRectangle.bottom;
            }
			
            TargetRectangle.left+=SizeShape.h;                              /* Create the new target rectangle */
			if ((TargetRectangle.left % cram->texturepagewidth) != 0)				/* add on the extra space if the rectangle isnt on the edge of a texturepage */
				TargetRectangle.left+= cram->space;

            PushStackRect(TargetRectangle);                                 /* Push the new target rectangle in the list */
        }
        else 
		{
			KillTargetRectangle(iTargetRectangle);    	/* No Solution for this rectangle so kill it */
		}

    }
    return(true);
}

static int oldcramfunc(CRAM *cram, int numrects,CRAMRECT *rects, int x,int y,int width,int height)
{
    ImageList *sl;
    ImageList **slp;
    CRECT vrect;
    int i;
    int ok=1;

    sl = (ImageList *) galloc(500000);
    memset(sl,0,500000);

    sl->NbShape = numrects;

    for (i=0; i<numrects; ++i)
    {
        sl->WorkList[i].Size.h = rects[i].width;
        sl->WorkList[i].Size.v = rects[i].height;
        sl->WorkList[i].Bpp    = rects[i].bpp;
        sl->WorkList[i].Rotate = rects[i].rotate;
        sl->WorkList[i].Pos.h  = rects[i].x;
        sl->WorkList[i].Pos.v  = rects[i].y;
        sl->WorkList[i].Done   = rects[i].done;
    }
    vrect.top    = y;
    vrect.left   = x;
    vrect.bottom = y+height;
    vrect.right  = x+width;

    slp = &sl;

    ok = VRAMOptimisation(cram, slp,vrect);

    for (i=0; i<numrects; ++i)
    {
        rects[i].width  =  sl->WorkList[i].Size.h;
        rects[i].height =  sl->WorkList[i].Size.v;
        rects[i].bpp    =  sl->WorkList[i].Bpp;
        rects[i].rotate =  sl->WorkList[i].Rotate;
        rects[i].x      =  sl->WorkList[i].Pos.h;
        rects[i].y      =  sl->WorkList[i].Pos.v;
        rects[i].done   =  sl->WorkList[i].Done;
        if (rects[i].rotate)
        {
            rects[i].width  =  sl->WorkList[i].Size.v;
            rects[i].height =  sl->WorkList[i].Size.h;
        }
    }
    gfree(sl);
    return(ok);
}

/*\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\ */

CRAM* CRAM_init()
{
    CRAM *cram = (CRAM *) galloc(sizeof(CRAM));
    if (cram)
    {
        memset(cram, 0, sizeof(CRAM));

        cram->texturepagewidth = 64;
        cram->texturepagewidth  = 256;
        cram->texturepageheight = 256;
        cram->rotate = 1;
        cram->checktexturepage = CRAM_IGNORE_TEXTUREPAGE;
        cram->checkedges = 1;
        cram->initalized = 1;
        cram->platform = CRAM_PLATFORM_DEFAULT;
        cram->odd_odd = 1;
    }

    return(cram);
}


void CRAM_close(CRAM *cram)
{
    cram->initalized = 0;
    gfree(cram);
}

void CRAM_setflags(CRAM *cram, unsigned int flags)
{

    CRAM_setstate(cram, CRAMSTATE_ALLOW_ROTATE, flags & CRAM_ROTATE);

    CRAM_setstate(cram, CRAMSTATE_CHECK_TEXTUREPAGE, CRAM_IGNORE_TEXTUREPAGE);
    if (flags & CRAM_CHECKUV)
        CRAM_setstate(cram, CRAMSTATE_CHECK_TEXTUREPAGE, CRAM_CHECK_TEXTUREPAGE);
    if (flags & CRAM_CHECKUV_IF_SMALLER)
        CRAM_setstate(cram, CRAMSTATE_CHECK_TEXTUREPAGE, CRAM_CHECK_TEXTUREPAGE_IF_POSSIBLE);
}

void CRAM_setstate(CRAM *cram, int state, int value)
{
    switch (state)
    {
    case CRAMSTATE_PLATFORM:
        cram->platform = value;
        break;

    case CRAMSTATE_CHECK_TEXTUREPAGE:
        cram->checktexturepage = value;
        break;

    case CRAMSTATE_CHECK_EDGES:
        cram->checkedges = value;
        break;

    case CRAMSTATE_TEXTUREPAGE_ALIGNMENT:
        cram->texturepagewidth = value;
        break;

    case CRAMSTATE_TEXTUREPAGE_WIDTH:
        cram->texturepagewidth = value;
        break;

    case CRAMSTATE_TEXTUREPAGE_HEIGHT:
        cram->texturepageheight = value;
        break;

    case CRAMSTATE_TARGET_X:
        cram->x = value;
        break;

    case CRAMSTATE_TARGET_Y:
        cram->y = value;
        break;

    case CRAMSTATE_TARGET_WIDTH:
        cram->width = value;
        break;

    case CRAMSTATE_TARGET_HEIGHT:
        cram->height = value;
        break;

    case CRAMSTATE_ALLOW_ROTATE:
        cram->rotate = value!=0;
        break;

    case CRAMSTATE_ODD_ODD:
        cram->odd_odd = value;
        break;
	
	case CRAMSTATE_WIDTHIN:
        cram->widthin = value;
        break;

	case CRAMSTATE_SPACE:
        cram->space = value;
        break;
    }
}

int CRAM_getstate(CRAM *cram,int state)
{
    switch (state)
    {
    case CRAMSTATE_PLATFORM:
        return cram->platform;

    case CRAMSTATE_INITIALIZED:
        return cram->initalized;

    case CRAMSTATE_CHECK_TEXTUREPAGE:
        return cram->checktexturepage;

    case CRAMSTATE_CHECK_EDGES:
        return cram->checkedges;

    case CRAMSTATE_TEXTUREPAGE_ALIGNMENT:
        return cram->texturepagealignment;

    case CRAMSTATE_TEXTUREPAGE_WIDTH:
        return cram->texturepagewidth;

    case CRAMSTATE_TEXTUREPAGE_HEIGHT:
        return cram->texturepageheight;

    case CRAMSTATE_TARGET_X:
        return cram->x;

    case CRAMSTATE_TARGET_Y:
        return cram->y;

    case CRAMSTATE_TARGET_WIDTH:
        return cram->width;

    case CRAMSTATE_TARGET_HEIGHT:
        return cram->height;

    case CRAMSTATE_ALLOW_ROTATE:
        return cram->rotate;

    case CRAMSTATE_ODD_ODD:
        return cram->odd_odd;
    }
    return 0;
}


void CRAM_settexturepagesize(CRAM *cram, int width, int height)
{
    CRAM_setstate(cram, CRAMSTATE_TEXTUREPAGE_WIDTH, width);
    CRAM_setstate(cram, CRAMSTATE_TEXTUREPAGE_HEIGHT, height);
}

void CRAM_setrect(CRAM *cram, int x, int y, int width, int height)
{
    CRAM_setstate(cram, CRAMSTATE_TARGET_X, x);
    CRAM_setstate(cram, CRAMSTATE_TARGET_Y, y);
    CRAM_setstate(cram, CRAMSTATE_TARGET_WIDTH, width);
    CRAM_setstate(cram, CRAMSTATE_TARGET_HEIGHT, height);
}

int CRAM_process(CRAM *cram, int numrects, CRAMRECT *rects)
{
    return oldcramfunc(cram, numrects, rects, cram->x,cram->y,cram->width,cram->height);
}
