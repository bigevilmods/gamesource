// (c) 1994-2007 Electronic Arts Inc.

/*********************************************************************************
ABSTRACT 

replay_eg.cpp - GIMEX example, writes AVI animation

DESCRIPTION

This is an example of how to use the GIMEX API to write out frames 
from an animated sequence. It reads in the framebuffer from the 
window and converts the data to AVI format and stores the data, one 
frame at a time. It reads 10 frames. The output file is called 
output.avi.

Please see the GIMEX <HTML a href="../sdk/gettingstarted/compile.htm">compiling and linking<HTML /a> for the files that 
are required to build this example. You will also need to download 
<HTML a href="../sdk/gettingstarted/download.htm">gimex.dll or gimex.lib<HTML /a> and the <HTML a href="http://tools.eac.ea.com/pc">REAL libraries<HTML /a>
in order to run this example.

SOURCE
@*********************************************************************************/

#include <3rash.h>
#include "dx7.h"
#include "stdio.h"
#include "stdlib.h"
#include <string.h>
#include <math.h>
#include "eac.h"
#include "eacfile.h"
#include "sdclip.h"
#include "eacgraph.h"
#include "eacmath.h"
#include "eacrmath.h"
#include "eacx.h"
#include "realgraph.h"
#include "realmath.h"
#include "gimex.h"
#include "gopen.h"
#if defined(_MSC_VER) && _MSC_VER>=1400
#define ENABLE_OMP 1
#include <omp.h>
#endif

// define a 3d cube

/*
      4---5
     /|  /
    0 - 1 |
    | 7-|-6
    |/  |/
    3---2
*/

#define SPAN 5

#define CUBEPOINTS (SPAN*SPAN*SPAN)

// cube vertices in world space
COORD3 vCubeVertices3D[CUBEPOINTS];

// clip flag for each vertex
unsigned char caGClipFlags[CUBEPOINTS];

// transform into screen space thrash vertices
THRASHVERTEX vCubeScreenVertices[CUBEPOINTS];

#define FRAMES 10

void initvcube(void)
{
	int i=0,j=0;
	for (i=0; i<CUBEPOINTS; i++)
	{
		vCubeScreenVertices[i].x = 0;
		vCubeScreenVertices[i].y = 0;
		vCubeScreenVertices[i].z = 0;
		vCubeScreenVertices[i].rhw = 0;
		switch(j)
		{		
			case 0: vCubeScreenVertices[i].colour = 0xff00ffff; break;
			case 1: vCubeScreenVertices[i].colour = 0xff00ff00; break;
			case 2: vCubeScreenVertices[i].colour = 0xffff0000; break;
		}			
		j=(j+1)%3;
		vCubeScreenVertices[i].specular = 0x00000000;
		vCubeScreenVertices[i].u = 0;
		vCubeScreenVertices[i].v = 0;
	}
}


void createcube(void)
{
	int i,j,k;
	float step=4.0f / ((float)SPAN);
	
	for(k=0; k<SPAN; k++)
	{
		for(j=0; j<SPAN; j++)
		{
			for(i=0; i<SPAN; i++)
			{
				vCubeVertices3D[(k*SPAN*SPAN)+(j*SPAN)+i].x = (float)-2.0+(step*i);
				vCubeVertices3D[(k*SPAN*SPAN)+(j*SPAN)+i].y = (float)-2.0+(step*j);
				vCubeVertices3D[(k*SPAN*SPAN)+(j*SPAN)+i].z = (float)-2.0+(step*k);						
			}							
		}	
	}
}

int compare_rhw(const void *vp, const void *vq)
{
  if (((THRASHVERTEX *) vp)->rhw > ((THRASHVERTEX *) vq)->rhw)
  	return(1);
  else
	return(-1);
}


void DrawCenteredSprite(THRASHVERTEX *center, float scale)
{
	THRASHVERTEX topleft, bottomright;
	unsigned char flags1,flags2;

	topleft.x = center->x - (scale * 1.0f);
	topleft.y = center->y - (scale * 1.0f);
	topleft.z = center->z;
	topleft.rhw = center->rhw;
	topleft.colour = center->colour;
	topleft.specular = center->specular;
	topleft.u = 0.0f;
	topleft.v = 0.0f;		

	bottomright.x = center->x + (scale * 1.0f);
	bottomright.y = center->y + (scale * 1.0f);
	bottomright.z = center->z;
	bottomright.rhw = center->rhw;
	bottomright.colour = center->colour;
	bottomright.specular = center->specular;
	bottomright.u = 1.0f;
	bottomright.v = 1.0f;		

	CLIP_setflags4((float *)&topleft,&flags1);
	CLIP_setflags4((float *)&bottomright,&flags2);

	if((!flags1) && (!flags2))
		THRASH_drawsprite(&topleft,&bottomright);
}

#define SCALEMOUSE(x) ((x)>=0)?isqrt((x)*(x)*(x)):-isqrt(-(x)*(x)*(x))

// this function will uses GIMEX to prepare a file for writing out
static int wopenfile(GINSTANCE **gx, char *filename, int frames)
{
    GSTREAM *g;
    WINDOW windowptr[2];
    int ok=0;

    WINDOW_save(windowptr);
    WINDOW_null();
    
        g = gwopen(filename);
        if (g)
        {
            ok = GIMEX_wopen(gx, g, filename, frames);
			printf("open %d %d %d %s %d\n ", gx, g, ok, filename, frames);
        }

    WINDOW_restore(windowptr);
        
    return(ok);
}

// this function sets up the GINFO structure for writing out the frame with GIMEX
static void writeframe(GINSTANCE *gx)
{
    SHAPE *shp;
    GINFO ginfo;
    WINDOW windowptr[2];
    WINDOW *wndw;

    shp = createshapeadr(screenwidth, screenheight, screenbpp, mb_default);
    wndw = createwindowadr(320, 240, 32, mb_default);
    
    WINDOW_save(windowptr);
    WINDOW_display();

        readf(shp);

    window(wndw);

        WINDOW_clear(C_BLACK);
        bilinearscalefxya(shp, 0, 0, 320, 240);

    WINDOW_null();

        memset(&ginfo, 0, sizeof(GINFO));

        /*
         * Some defaults
         */
        ginfo.signature        = GMAKEID(0,0,0,0);          /* Type of file */
        ginfo.size             = sizeof(GINFO);            /* Length of GINFO */
        ginfo.version          = 200;
        ginfo.framenum         = 0;                         /* Current frame num */
        ginfo.width            = shapewidth(wndw->shape);        /* Width of bitmap in pixels */
        ginfo.height           = shapeheight(wndw->shape);       /* Height of bitmap in pixels */
        ginfo.subtype          = 0;
        ginfo.packed           = GMAKEID('d','i','v','c');
        ginfo.quality          = 70;
        ginfo.redbits          = screenbpp/3;
        ginfo.greenbits        = screenbpp/3;
        ginfo.bluebits         = screenbpp/3;
        ginfo.alphabits        = screenbpp-ginfo.redbits-ginfo.greenbits-ginfo.bluebits;
        ginfo.bpp              = 32;
        ginfo.originalbpp      = screenbpp;
        ginfo.numcolours       = 0;

        if (!GIMEX_write(gx, &ginfo, SHAPE_data(wndw->shape), shaperowbytes(wndw->shape)))
        {
            abortmessage("Error writing output file");
        }
        
        purgememadr(shp);
        purgememadr(wndw);

    WINDOW_restore(windowptr);
}

// this function closes the file for writing with GIMEX
static int wclosefile(GINSTANCE *gx)
{
    int ok;
    WINDOW windowptr[2];
    GSTREAM *g;

    WINDOW_save(windowptr);
    WINDOW_null();

        g = gx->gstream;
        if (GIMEX_wclose(gx))
            ok = gclose(g);

    WINDOW_restore(windowptr);

    return(ok);    
}


int main(void)
{
	THRASHTEXTURE *ball0;
    int key,i=0,j=0;

	int VBALL_pitch=0;
	int VBALL_head=0;
	int VBALL_iX = 0;
	int VBALL_iY = 0;
	int VBALL_iZ = 5;

	int screenbpp = 32;
	int         writing=0;
    GINSTANCE   *gx;
    char        filename[] = "output.avi";

    MATRIX3 m3Rotation = 
    {
        1.0,0.0,0.0,
        0.0,1.0,0.0,
        0.0,0.0,1.0
    };
    COORD3 vTrans;
    
    // start libs with THRASH as low level
    THRASHDRIVER_set(THRASHDRIVER_best());
    //inithal();
    SCREENmode=1;
    REAL_init();

    // TEXTURE
    {
        char *f;
        SHAPE *shp;
        SHAPE *clut;

        f = SHAPE_loadfile("replay",MB_LOW);

        shp = SHAPE_pointer(f,0);
        clut = SHAPE_clut(shp);
		shp = SHAPE_convertdepth(shp,screenbpp);
        ball0 = THRASH_talloc(SHAPE_width(shp),SHAPE_height(shp),shapetotexturetype(shp),clut?shapetotexturetype(clut):CLUT_NA,0);
        THRASH_tupdate(ball0,SHAPE_data(shp),clut?SHAPE_data(clut):0);

	}

	//Init the data
	initvcube();
	createcube();

    // set maths view settings
    rsetprojxy(0,0,640, 480, 0.05f, 500.0f, DEGREE2REAL(45), DEGREE2REAL(45) );
    // transform directly to thrash vertices
    rmpipeoutputsize(sizeof(THRASHVERTEX));

    CLIP_setclipmode(CLIPMODERGBSHADECOLOUR);
    // if not using eacmaths functions then need to setup clip window
    // CLIP_setclipparameters(float x, float y, float width, float height, float srhwnear, float srhwfar);
    // or to change the clip region from the projection window
    // here we have a 100 pixel border round the screen
    CLIP_setclipparameters(0, 0, 640, 480, 1.0f/0.05f, 1.0f/500.0f);
    
	if(!THRASH_setstate(STATE_DEPTHBUFFER,VALUE_DEPTHBUFFER_W))
		THRASH_setstate(STATE_DEPTHBUFFER,VALUE_DEPTHBUFFER_ON);

    THRASH_setstate(STATE_CULL, VALUE_CULL_COUNTERCLOCKWISE);    
    THRASH_setstate(STATE_TRANSPARENCY,VALUE_TRANSPARENCY_ALPHA);
    THRASH_setstate(STATE_SHADE,VALUE_SHADE_ON);   
    
    // rgb shading
    THRASH_setstate(STATE_BACKGROUNDCOLOUR, CA_BLACK);    
    THRASH_settexture(ball0);

	setmousepos(screenwidth/2,screenheight/2);

	GIMEX_init();
	GIMEX_find("avi", NULL); // set the GIMEX file format

    do
    {
        
        THRASH_window(THRASH_WINDOW_BACK);
        THRASH_clearwindow();

        // draw NUMBEROFCUBES
        {

		    vTrans.x = INT2REAL(VBALL_iX);
		    vTrans.y = INT2REAL(VBALL_iY);
		    vTrans.z = INT2REAL(VBALL_iZ);

		    m3setxrot(&m3Rotation,FIXED2REAL(VBALL_head));
		    m3multzrot(&m3Rotation,FIXED2REAL(VBALL_pitch),&m3Rotation);	// was yrot

            // set the current rotate translate
            rmpipexform(&m3Rotation);
            rmpipexlate(&vTrans);

            // if we have a convex hull that defines a mesh then we can 
            // first test if the mesh is visible and whether or not it needs clipped
            // for a cube mesh we just use its vertices as the convex hull as you cant get much simpler than a cube
            // its visible so do the maths
            // transform translate project and compute the clip flags (in parallel)
            rmpipe(RMP_XFORM+RMP_XLATE+RMP_PROJ,CUBEPOINTS, vCubeVertices3D, (float *)vCubeScreenVertices);

			qsort(vCubeScreenVertices,CUBEPOINTS,sizeof(THRASHVERTEX),compare_rhw);

            for(i=0; i<CUBEPOINTS; i++)
            {
            	DrawCenteredSprite(&vCubeScreenVertices[i],50.0f*(vCubeScreenVertices[i].rhw));
            }
        }

		initvcube();
        key=testkey();

        readmouse();
        VBALL_pitch += SCALEMOUSE(mx-screenwidth/2);
        VBALL_head += SCALEMOUSE(my-screenheight/2);
        if (mbuttons&M_LBUT) VBALL_iZ += 1;
        if (mbuttons&M_RBUT) VBALL_iZ -= 1;
                       
        switch (key)
        {
            case K_LEFT:    VBALL_iX -= 1; break;
            case K_RIGHT:   VBALL_iX += 1; break;
            case K_UP:      VBALL_iY += 1; break;
            case K_DOWN:    VBALL_iY -= 1; break;
            case ' ':       setmousepos(screenwidth/2,screenheight/2); break;
			case 's':
				// set up GIMEX for writing
		        if (!wopenfile(&gx, filename, FRAMES))
				    abortmessage("Unable to wopenfile %s", filename);
			    writing = FRAMES;
				break;
		}        

        THRASH_flushwindow();
	
		// keep writing out the frames until, written them all
        if (writing>0)
        {
			WINDOW_hidden();
            writeframe(gx);
            writing--;
            if (writing == 0)
            {
                if (!wclosefile(gx))
                    abortmessage("Error closing file");
            }
			WINDOW_null();
        }
		THRASH_pageflip();


    } while (key!=K_ESC);

    REAL_exit();

    return(0);
}

// END ABSTRACT
