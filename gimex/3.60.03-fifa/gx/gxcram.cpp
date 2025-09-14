// (c) 1991-2004 Electronic Arts Inc.

/*------------------------------------------------------------------*/
/*                                                                  */
/*                          CRAMANY v1.15                           */
/*                                                                  */
/*						by Lydia Chan, EAC		                   */
/*                                                                  */
/*                   Main program - Sep 25, 2001                    */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History:                                       */
/* ------- ----   -- --------                                       */
/* 1.00    991126 LC cram any shapefile! based on crampsh           */
/* 1.01    991203 LC added more features and not just for PSH files */
/* 1.02    991209 FB multiple file support                          */
/* 1.03    991216 LC set texture page width option                  */
/* 1.04    000107 LC widthin option; fails if width > tpage width   */
/* 1.05    000112 FB x wrap 4096                                    */
/* 1.06    000114 LC print out shapes that failed cram              */
/* 1.07    010316 LC fixed calc of vram wastage and texture used    */
/* 1.08    010319 LC error if texture pages are bigger than VRAM    */
/* 1.09    010320 LC better output for file not found               */
/* 1.10    010322 LC cramany respects tpage height with option      */
/* 1.11    010406 LC clutsort and group sorting by bpp and clut     */
/* 1.12    010409 ID Spun off from cramany for integration into gx  */
/* 1.13    010411 ID Added ability to cram and flatten              */
/* 1.14    010824 LC Added space option                             */
/* 1.15    010925 FB C++                                            */
/*                                                                  */
/*------------------------------------------------------------------*/

#define __NOINLINE__ 1
#include <stdlib.h>
#include <ctype.h>
#include "gimex.h"
#include "gimex.h"
#include "gopen.h"
#include "stdgimex.h"
#include "cram.h"
#include "cram.cpp"
#include "frot.cpp"
#include "fflip.cpp"
#include "gxcram.h"






/*
;
; ABSTRACT
;
; cramany - optimizes VRAM layout of shape files
;
; SUMMARY
;
; cramany files.*sh
;   [x y width height] top left position, width & height of the VRAM bounding box
;                      (default is 0 256 1024 256)
;   [#fixed]           the first # of shapes are fixed (don't move them!)
;   [ROTate]           rotate to fit better
;   [TRAnspose]        transpose to fit better
;   [VERbose]          print out more information
;   [CHEckedge]        checking the texture page boundaries and the edges too
;   [ODDodd]           if the texture page is odd width and even height,
;                      increase height so it is odd too (a fix for Sony problem)
;   [BPPsort]          sort the shapes by bpp & place them in separate
;                      texture pages
;   [CLUTsort]         sort the shapes by clut bit depth & place them in separate
;                      texture pages
;	[group#]           sort the shapes by specified bpp and clut bit depth & place
;			           them in the same texture page
;			           usage: group{<bpp>-<clut>} eg. group8-565,8888-0 group4444-0
;					   this example will place 8 bit images with 565 cluts and 32 bit images
;					   in the same texturepage; 16 bit images are grouped in another texturepage;
;	                   all the other images will be defaulted to another texturepage
;   [pagewidth#]       set the texture page width (default is 256)
;   [pageheight#]      set the texture page height (default is 256)
;   [wrapwidth#]       set the max width, beyond which wrap to new row (default is 4096)
;   [WIDthin]          cram fails if the width of the image is bigger than the texture page width
;   [space#]           puts #pixels of space between each shape
;
;     @file				 response file, containing a list of options and files
;     -nologo			 disable banner
;     -v0				 no console output
;     -v1				 prints source file name and errors
;     -v2				 status line for each file (default)
;     -v3				 additional info
;
; DESCRIPTION
;
; CRAMANY crams shapes into vram by setting the position where they should
; go in vram, and optionally rotating them for a better fit.
;
; Shapes can be sorted by type (bit depth) using the BPP option, so that 
; all shapes of a type are crammed into a texture page.
;
; It is recommended that you use a vram area that is wide (ie 4096) but
; only 256 tall. 
;
; A list of optimal rectangles for texture pages printed to the console.
; Either use that, or at runtime, calculate a bounding rectangle for all
; textures of a shape type. ie 4 bit textures might use from 0,0 to 200,256.
;
; Please note that for VRAM with height larger than 256, textures can respect 
; texture page widths but are unable to respect texture page height. 
;
; RETURNS
;
; no return code
;
; SEE ALSO
;
; huff, Spit
;
; EXAMPLE
;
; L:\enviro\demo\cramany>cramany bob.psh fred.psh 0 0 256 256
; 2 files, x:0 y:0 w:256 h:256 no rotate
; backing up bob.psh to bob.bak
; bob.psh: Reading, 15 frames
; backing up fred.psh to fred.bak
; fred.psh: Reading, 1 frames
; 16 frames total
; Analysing...
; Cram Successful
; fball.psh: Reading, Writing bob.psh
; fred.psh: Reading, Writing fred.psh
; vram used: 0,0 .. 227,215 (bpp offset)
; vram wastage: 45.32%
;
; END ABSTRACT
;
*/

GXCRAMHANDLE *GXCRAM_Init(int shapeType,int transpose,int rotate,int verbose,int centerpoint,int checkedge,int oddOdd,
						int bppsort,int clutsort,int groupsort,GXCRAMGROUP * group,int tpageWidth,int tpageHeight,int twrapWidth,	int widthin, int space, 
						int x, int y, int w, int h,int numfixed)
{
	GXCRAMHANDLE * gxCramHandle;
	int i = 0;
	CRAMLINK *cramlist = NULL;
    gxCramHandle = (GXCRAMHANDLE *)galloc(sizeof(GXCRAMHANDLE));
	if(gxCramHandle)
	{
		memset(gxCramHandle,0,sizeof(GXCRAMHANDLE));
		gxCramHandle->init = 1;
		cramlist = &(gxCramHandle->cramlink);
		cramlist->next = NULL;
		cramlist->numrects = 0;
		gxCramHandle->maxx = 0;						// rightmost x value, returned so we know where we can do another cram on a different file
		gxCramHandle->maxy = 0;
		gxCramHandle->minx = 1024;
		gxCramHandle->miny = 512;
		gxCramHandle->bppsort = bppsort;
		gxCramHandle->centerpoint = centerpoint;
		gxCramHandle->checkedge   = checkedge;
		gxCramHandle->clutsort    = clutsort;
		gxCramHandle->errors      = 0;
		gxCramHandle->frame       = 0;
		gxCramHandle->frames      = 0;
		for(i = 0; i < 20; i++)
			gxCramHandle->group[i]       = group[i];
		if(clutsort)
		{
			gxCramHandle->groupsort   = -1 + groupsort;
			for (i = 0; i < 20; i++)			    // init the number of textures per group to 0
				gxCramHandle->group[i].num = 0;
		}
		else
			gxCramHandle->groupsort   += groupsort; // group 0 stores the other textures that dont fit in any grouping
													// group 1 is the first grouping of textures
		gxCramHandle->numfixed    = numfixed;
		gxCramHandle->numTexturePages = 0;
		gxCramHandle->oddOdd      = oddOdd;
		gxCramHandle->rotate      = rotate;
		gxCramHandle->shapeType   = shapeType;
		gxCramHandle->spaceUsage  = 0;
		gxCramHandle->tpageHeight = tpageHeight;
		gxCramHandle->tpageWidth  = tpageWidth;
		if(gxCramHandle->tpageHeight < 9)                   // just to be sure.
			gxCramHandle->tpageHeight = 256;
		if(gxCramHandle->tpageWidth < 9)
			gxCramHandle->tpageWidth = 256;
		gxCramHandle->transpose   = transpose;
		gxCramHandle->verbose     = verbose;
		gxCramHandle->w           = w;
		gxCramHandle->h           = h;
		if(gxCramHandle->w < 9)
			gxCramHandle->w = 1024;
		if(gxCramHandle->h < 9)
			gxCramHandle->h = 256;
		gxCramHandle->x           = x;
		gxCramHandle->y           = y;
		gxCramHandle->twrapWidth  = twrapWidth;
		if(!gxCramHandle->twrapWidth)
			gxCramHandle->twrapWidth = gxCramHandle->w;

		gxCramHandle->widthin     = widthin;
		gxCramHandle->space     = space;

        memset(cramlist->cramrects,0,sizeof(cramlist->cramrects));
		if (groupsort)
		{
			int j = 0;
			
			cramlist = &gxCramHandle->cramlink; // reset pointer to the first link
			// the first group is for those textures that arent explicitely grouped
			for (j = 1; j <= groupsort; j++)
			{
				cramlist->next = (CRAMLINK*)galloc(sizeof(CRAMLINK));
				cramlist = cramlist->next;
				memset(cramlist->cramrects,0,sizeof(cramlist->cramrects));
				cramlist->next = NULL;
				cramlist->numrects = 0;
				cramlist->cramrects[cramlist->numrects].group = j;
			}
		}
		if (clutsort)
		{
			cramlist->cramrects[0].group = -1; // just to mark this as the first link and not to 
											   // ignore it when creating a new link for a new grouping
		}
		if(verbose)
			printf("x:%d y:%d w:%d h:%d %s\n",x,y,w,h, rotate ? (transpose ? "transpose" : "rotate") : "no rotate");
	} // if gxCramHandle
	return(gxCramHandle);
}
		
	
//-------------------------------------------------------------------------------
int GXCRAM_AddToCramList(GXCRAMHANDLE * gxCramHandle, GINFO * ginfo)
{


	CRAMLINK * cramlist = &gxCramHandle->cramlink; // reset pointer to the first link
	gxCramHandle->frames++;

	// separate textures by bpp
	if (gxCramHandle->bppsort)
	{
		// try to figure out which cramlist to put this shape in
		while ((ginfo->originalbpp != cramlist->cramrects[0].originalbpp) && (cramlist->next != NULL))
		{
			cramlist = cramlist->next;
		}

		// if not the same bpp and the current link already has a bpp and the next link is null, we have to create a new link
		if ((ginfo->originalbpp != cramlist->cramrects[0].originalbpp) && (cramlist->cramrects[0].bpp != 0) && (cramlist->next == NULL) )
		{
			cramlist->next = (CRAMLINK*)galloc(sizeof(CRAMLINK));
			cramlist = cramlist->next;
			memset(cramlist->cramrects,0,sizeof(cramlist->cramrects));
			cramlist->next = NULL;
			cramlist->numrects = 0;
			cramlist->cramrects[cramlist->numrects].originalbpp    = ginfo->originalbpp;
		}
	}
	else if (gxCramHandle->clutsort)
	{

		int cramgroup = -1;
		int group_channels;
		int i, j;

		group_channels = ginfo->alphabits*1000 + ginfo->redbits*100 + ginfo->greenbits*10 + ginfo->bluebits;
		// figure out which group this texture would go in
		for (i = 0; i <= gxCramHandle->groupsort; i++)
		{
			for (j = 0; j < gxCramHandle->group[i].num; j++)
			{
				if ((ginfo->originalbpp == gxCramHandle->group[i].image[j].bpp) && (group_channels == gxCramHandle->group[i].image[j].palette))
				{
					cramgroup = i;
					break;
				}
				else if ((ginfo->bpp > 8) && (group_channels == gxCramHandle->group[i].image[j].bpp))
				{
					cramgroup = i;
					break;
				}
			}
		}
		
		// we need to make a new group and a new cram link
		if (cramgroup == -1)
		{
			
			gxCramHandle->groupsort++;
			
			if (ginfo->bpp == 8)
			{
				gxCramHandle->group[gxCramHandle->groupsort].image[gxCramHandle->group[gxCramHandle->groupsort].num].bpp = ginfo->originalbpp;
				gxCramHandle->group[gxCramHandle->groupsort].image[gxCramHandle->group[gxCramHandle->groupsort].num].palette = group_channels;
			}
			else
			{
				gxCramHandle->group[gxCramHandle->groupsort].image[gxCramHandle->group[gxCramHandle->groupsort].num].bpp = group_channels;
				gxCramHandle->group[gxCramHandle->groupsort].image[gxCramHandle->group[gxCramHandle->groupsort].num].palette = 0;
			}	
			gxCramHandle->group[gxCramHandle->groupsort].num++;

			if (cramlist->cramrects[cramlist->numrects].group != -1)
			{
				// iterate to the end of the list
				while (cramlist->next != NULL)
					cramlist = cramlist->next;
				// we need to make a new cramlist for this new clut group
				cramlist->next = (CRAMLINK*)galloc(sizeof(CRAMLINK));
				cramlist = cramlist->next;
				memset(cramlist->cramrects,0,sizeof(cramlist->cramrects));
				cramlist->next = NULL;
				cramlist->numrects = 0;
				cramlist->cramrects[cramlist->numrects].group  = gxCramHandle->groupsort;
			}
			else 
			{
				// dont create a new link if its the very first one
				cramlist->cramrects[0].group  = gxCramHandle->groupsort;
			}
		}
		// or figure out which group
		else 
		{
			while ((cramgroup != cramlist->cramrects[0].group) && (cramlist->next != NULL))
				cramlist = cramlist->next;
		}
	}
	else if (gxCramHandle->groupsort)
	{
		int cramgroup = 0;
		int group_channels;
		int i, j;

		group_channels = ginfo->alphabits*1000 + ginfo->redbits*100 + ginfo->greenbits*10 + ginfo->bluebits;
		// figure out which group this texture would go in
		for (i = 0; i <= gxCramHandle->groupsort; i++)
		{
			for (j = 0; j < gxCramHandle->group[i].num; j++)
			{
				if ((ginfo->originalbpp == gxCramHandle->group[i].image[j].bpp) && (group_channels == gxCramHandle->group[i].image[j].palette))
				{
					cramgroup = i;
					break;
				}
				else if ((ginfo->bpp > 8) && (group_channels == gxCramHandle->group[i].image[j].bpp))
				{
					cramgroup = i;
					break;
				}
			}
		}

		// now figure out which cram list 		
		while ((cramgroup != cramlist->cramrects[0].group) && (cramlist->next != NULL))
		{
			cramlist = cramlist->next;
		}
	}

	// if the bpp matches or in the first link, or just no bppsort
	//if ((ginfo->originalbpp == cramlist->cramrects[0].originalbpp) || ((cramlist == &cramlink) && (cramlist->next == NULL)))
	cramlist->cramrects[cramlist->numrects].width  = ginfo->width;
	cramlist->cramrects[cramlist->numrects].height = ginfo->height;
	//cramlist->cramrects[cramlist->numrects].originalbpp    = ginfo->originalbpp;
	// round up the bpp since cram works best with powers of 2
	if (ginfo->originalbpp <= 4)
		cramlist->cramrects[cramlist->numrects].bpp = 4;
	else if (ginfo->originalbpp <= 8)
		cramlist->cramrects[cramlist->numrects].bpp = 8;
	else if (ginfo->originalbpp <= 16)
		cramlist->cramrects[cramlist->numrects].bpp = 16;
	else
		cramlist->cramrects[cramlist->numrects].bpp = 32;

	cramlist->cramrects[cramlist->numrects].originalbpp = ginfo->originalbpp;
	cramlist->cramrects[cramlist->numrects].done   = 0;
	cramlist->cramrects[cramlist->numrects].x      = ginfo->defaultx;
	cramlist->cramrects[cramlist->numrects].y      = ginfo->defaulty;
	cramlist->cramrects[cramlist->numrects].rotate = (ginfo->subtype&(SUBROTATE|SUBTRANSPOSE))!=0;
	cramlist->framenumber[cramlist->numrects] = gxCramHandle->frames-1;////ginfo->framenum+gxCramHandle->frames*MAXFRAMES;
	strcpy(cramlist->framename[cramlist->numrects], ginfo->framename);
	// what to do here?
	if(gxCramHandle->frames < gxCramHandle->numfixed)
		cramlist->cramrects[cramlist->numrects].done = 1;
	return((cramlist->numrects++));
	
	
}


//------------------NOW WE BEGIN TO DO CRAM-------------------------------
int GXCRAM_DoCram(GXCRAMHANDLE * gxCramHandle,GINFO **ginfodata,char ** fdata)
{
	int handleHeight, handleWidth;
	int curHeight = 0;
	int ok = 0;
	int i = 0;
    int totaldonethisloop = -1;
    int totaldonelastloop = -1;
    int totaldonelastlastloop = -1;
	char * f = *fdata;
	int sig = 'S'<<24 | 'H'<<16 | 'P' << 8 | 'P';
	GINFO * ginfo = *ginfodata;
	CRAMLINK * cramlist = NULL;
	if(gxCramHandle->frame == gxCramHandle->frames)
		gxCramHandle->frame = 0;
	if(gxCramHandle->verbose && gxCramHandle->frame == 0)
	{
		printf("%d frames total\n",gxCramHandle->frames);
		printf("Analysing... \n");
	}
	/* *** CRAM *** */



				/*    ok = cram(frames,cramrects, x,y,w,h, rotate,0,0); */
	gxCramHandle->handle = CRAM_init();

					// need to set the platform for each shapefile type
	if (gxCramHandle->shapeType == sig)
		CRAM_setstate(gxCramHandle->handle, CRAMSTATE_PLATFORM, CRAM_PLATFORM_PSX);
	else
		CRAM_setstate(gxCramHandle->handle, CRAMSTATE_PLATFORM, CRAM_PLATFORM_DEFAULT);
	CRAM_setstate(gxCramHandle->handle, CRAMSTATE_CHECK_TEXTUREPAGE, CRAM_CHECK_TEXTUREPAGE_IF_POSSIBLE);
	CRAM_setstate(gxCramHandle->handle, CRAMSTATE_ALLOW_ROTATE, gxCramHandle->rotate);
	CRAM_setstate(gxCramHandle->handle, CRAMSTATE_CHECK_EDGES, gxCramHandle->checkedge);
	CRAM_setstate(gxCramHandle->handle, CRAMSTATE_ODD_ODD, gxCramHandle->oddOdd);
	CRAM_setstate(gxCramHandle->handle, CRAMSTATE_WIDTHIN, gxCramHandle->widthin);
	CRAM_setstate(gxCramHandle->handle, CRAMSTATE_SPACE, gxCramHandle->space);
	CRAM_settexturepagesize(gxCramHandle->handle,gxCramHandle->tpageWidth,gxCramHandle->tpageHeight);
					
	CRAM_setrect(gxCramHandle->handle,gxCramHandle->x,gxCramHandle->y,gxCramHandle->w,gxCramHandle->h);

	handleHeight = gxCramHandle->handle->height; // store the actual VRAM height
	handleWidth = gxCramHandle->handle->width; // store the actual VRAM width
	CRAM_setrect(gxCramHandle->handle,gxCramHandle->x,gxCramHandle->y,gxCramHandle->w,gxCramHandle->handle->height); // set the VRAM height to 256 for the texture page height
	gxCramHandle->handle->height = gxCramHandle->handle->texturepageheight; // init this to the texture page height
	curHeight = gxCramHandle->handle->texturepageheight; // init this to the texture page height

	cramlist = &gxCramHandle->cramlink; // reset the cram list to the first one
	if(gxCramHandle->frame == 0)
	{
		while ((cramlist != NULL) && (curHeight <= handleHeight))
		{
            // check if we have a texture that is never going to fit and we are just looping around trying to fit it. 
            // then lets abort.
            if(totaldonethisloop == 0 && totaldonelastloop == 0 && totaldonelastlastloop == 0)
            {
                if(gxCramHandle->verbose)
                    printf("ABORT - Single texture to big to fit on any texture page, increase texture page size\n");
                break;
            }
            else
            {
                // propagate information as to whether I have done anything for the last 3 loops.
                // this is important so I never get caught looping indefinetly.
                totaldonelastlastloop = totaldonelastloop;
                totaldonelastloop     = totaldonethisloop;
                totaldonethisloop     = 0;
            }
			// if the next texture page is bigger than the specified VRAM, cram failed
			if (gxCramHandle->x >= handleWidth)
			{
				gxCramHandle->errors++;
				break;
			}

			// if the first cram link has no textures, go to the next one
			if (gxCramHandle->groupsort && !cramlist->numrects)
				cramlist = cramlist->next;

			ok = CRAM_process(gxCramHandle->handle, cramlist->numrects, cramlist->cramrects);

			if (!ok) gxCramHandle->errors++;
							
			gxCramHandle->handle->width = handleWidth; // reset this value to original value
							
			if (gxCramHandle->bppsort || gxCramHandle->groupsort || gxCramHandle->clutsort)
			{
				int w, i, h;
				if(gxCramHandle->verbose)
					printf("Texture Page %d: position (%d,%d) %dbpp %d textures\n", gxCramHandle->numTexturePages, gxCramHandle->handle->x, gxCramHandle->handle->y, cramlist->cramrects[0].bpp, cramlist->numrects);

				// reset the rectangle so that the next set of bpps is in the next texturepage
				// also find the actual texture page size
				gxCramHandle->maxx = 0; gxCramHandle->maxy = 0; gxCramHandle->minx = gxCramHandle->handle->width; gxCramHandle->miny = gxCramHandle->handle->height; 
				gxCramHandle->spaceUsage = 0;
				for (i=0; i< cramlist->numrects; i++)
				{
					if (cramlist->cramrects[i].done == 1)
					{
						// print out the list of textures in this page
						printf("     file#%d #%d \'%s\': position(%d,%d) size %dx%d\n",cramlist->framenumber[i]/MAXFRAMES, cramlist->framenumber[i]%MAXFRAMES, cramlist->framename[i], cramlist->cramrects[i].x, cramlist->cramrects[i].y % gxCramHandle->twrapWidth, cramlist->cramrects[i].width, cramlist->cramrects[i].height + gxCramHandle->handle->height * (cramlist->cramrects[i].x / gxCramHandle->twrapWidth));

						// find the actual size of the texture page
						if (gxCramHandle->shapeType == sig)
							w = (cramlist->cramrects[i].width*cramlist->cramrects[i].bpp+15)/16;
						else
							w = cramlist->cramrects[i].width;

							h = cramlist->cramrects[i].height;

						if (gxCramHandle->maxx < cramlist->cramrects[i].x + w)
							gxCramHandle->maxx = cramlist->cramrects[i].x + w;
						if (gxCramHandle->maxy < cramlist->cramrects[i].y + h)
							gxCramHandle->maxy = cramlist->cramrects[i].y + h;
						if (gxCramHandle->minx > cramlist->cramrects[i].x)
							gxCramHandle->minx = cramlist->cramrects[i].x;
						if (gxCramHandle->miny > cramlist->cramrects[i].y)
							gxCramHandle->miny = cramlist->cramrects[i].y;

						gxCramHandle->spaceUsage += w*h;
					}
				}

				// print out the actual size of the texture page
				if(gxCramHandle->verbose)
				{
					printf("Texture Page Used: %dx%d\n", (gxCramHandle->maxx)-(gxCramHandle->minx), (gxCramHandle->maxy)-(gxCramHandle->miny));
					printf("Texture Page Wastage: %.2f%%\n", (1-(double)(gxCramHandle->spaceUsage)/((double)((gxCramHandle->maxx)-(gxCramHandle->minx)) * ((gxCramHandle->maxy)-(gxCramHandle->miny))))*100);

				}
				// this gets us to the next texture page (width of 256)
				gxCramHandle->x = gxCramHandle->maxx + ((gxCramHandle->handle->texturepagewidth) - ((gxCramHandle->maxx)%(gxCramHandle->handle->texturepagewidth)));
			}
			else
			{
				// calculate the next x value
				int w, i, h;
								
				gxCramHandle->maxx = 0; 
				gxCramHandle->maxy = 0; 
				gxCramHandle->minx = gxCramHandle->handle->width; 
				gxCramHandle->miny = gxCramHandle->handle->height; 
				gxCramHandle->spaceUsage = 0;
				for (i=0; i< cramlist->numrects; i++)
				{
					// only calc with the textures most recently processed
					if (cramlist->cramrects[i].done == 1)
					{
						// find the actual size of the texture page
						if (gxCramHandle->shapeType == sig)
							w = (cramlist->cramrects[i].width*cramlist->cramrects[i].bpp+15)/16;
						else
							w = cramlist->cramrects[i].width;

							h = cramlist->cramrects[i].height;

						if (gxCramHandle->maxx < cramlist->cramrects[i].x + w)
							gxCramHandle->maxx = cramlist->cramrects[i].x + w;
						if (gxCramHandle->maxy < cramlist->cramrects[i].y + h)
							gxCramHandle->maxy = cramlist->cramrects[i].y + h;
						if (gxCramHandle->minx > cramlist->cramrects[i].x)
							gxCramHandle->minx = cramlist->cramrects[i].x;
						if (gxCramHandle->miny > cramlist->cramrects[i].y)
							gxCramHandle->miny = cramlist->cramrects[i].y;
					}
				}

				gxCramHandle->x = gxCramHandle->maxx + ((gxCramHandle->handle->texturepagewidth) - ((gxCramHandle->maxx)%(gxCramHandle->handle->texturepagewidth)));
			}

			gxCramHandle->numTexturePages++;
			
			// if it didnt fit on this row try to fit the rest of the undone textures on the next row 
			if (!ok && (curHeight < handleHeight))
			{
				gxCramHandle->errors--;
			}

			// mark the ones that have been set in place already so that they wont be used to 
			// calculate the farthest right texture
			for(i=0; i < cramlist->numrects; i++)
			{
				if (cramlist->cramrects[i].done == 1)
                {
					cramlist->cramrects[i].done = 2;
                    totaldonethisloop++;
                }
			}

			// if successfully crammed one set of textures, do the next set
			if (ok)
				cramlist = cramlist->next;
							
			// if at the horizontal end of the VRAM, go back to the next row
			if (gxCramHandle->x >= gxCramHandle->handle->width)
			{
				gxCramHandle->x = 0;
				CRAM_setrect(gxCramHandle->handle,gxCramHandle->x,gxCramHandle->handle->y + gxCramHandle->handle->texturepageheight,gxCramHandle->handle->width,gxCramHandle->handle->texturepageheight);
				curHeight += gxCramHandle->handle->texturepageheight;
			}
			else
			{
				CRAM_setrect(gxCramHandle->handle,gxCramHandle->x,gxCramHandle->handle->y,gxCramHandle->handle->width-gxCramHandle->x,gxCramHandle->handle->height);
			}
		}
	}// if first frame only.
	else
	{
		ok = 1;
	}
	if(gxCramHandle->frame == 0)
	{
		gxCramHandle->maxx = 0; gxCramHandle->maxy = 0; 
		gxCramHandle->minx = handleWidth; 
		gxCramHandle->miny = handleHeight; 
		gxCramHandle->spaceUsage = 0;// reset for later
	}

	// WRITE INFO
	i = 0;													
	cramlist = &gxCramHandle->cramlink;
	if (gxCramHandle->bppsort)
	{
		// have to find the rect that goes with this shape

		// find the list that has this frame
		while ((ginfo->originalbpp != cramlist->cramrects[0].originalbpp) && (cramlist->next != NULL))
			cramlist = cramlist->next;

		// reuse numTexturePages
		gxCramHandle->numTexturePages = 0;
		// find the rect that has this frame
		while (cramlist->framenumber[gxCramHandle->numTexturePages] != gxCramHandle->frame/*ginfo->framenum+gxCramHandle->frames*MAXFRAMES*/)
			gxCramHandle->numTexturePages++;
	}
	else if (gxCramHandle->groupsort)
	{
		// have to find the rect that goes with this shape

		int cramgroup = 0;
		int group_channels;
		int i, j;

		group_channels = ginfo->alphabits*1000 + ginfo->redbits*100 + ginfo->greenbits*10 + ginfo->bluebits;
		// figure out which group this texture would go in
		for (i = 0; i <= gxCramHandle->groupsort; i++)
		{
			for (j = 0; j < gxCramHandle->group[i].num; j++)
			{
				if ((ginfo->originalbpp == gxCramHandle->group[i].image[j].bpp) && (group_channels == gxCramHandle->group[i].image[j].palette))
				{
					cramgroup = i;
					break;
				}
				else if ((ginfo->bpp > 8) && (group_channels == gxCramHandle->group[i].image[j].bpp))
				{
					cramgroup = i;
					break;
				}
			}
		}

		// now figure out which cram list 	
		while ((cramgroup != cramlist->cramrects[0].group) && (cramlist->next != NULL))
		{
			cramlist = cramlist->next;
		}

		// reuse numTexturePages
		gxCramHandle->numTexturePages = 0;
		// find the rect that has this frame
		while (cramlist->framenumber[gxCramHandle->numTexturePages] != gxCramHandle->frame/*ginfo->framenum+gxCramHandle->frames*MAXFRAMES*/)
		{
			if (gxCramHandle->numTexturePages > cramlist->numrects)
			{
				if(gxCramHandle->verbose)
					printf("ERROR: cant find texture!\n");
				return(0);
			}
			gxCramHandle->numTexturePages++;
		}
	}
	else
		gxCramHandle->numTexturePages = gxCramHandle->frame;
																														
	if (gxCramHandle->verbose)
		printf("  '%s' %d @ %d,%d %s\n",ginfo->framename,gxCramHandle->frame,cramlist->cramrects[gxCramHandle->numTexturePages].x % gxCramHandle->twrapWidth,cramlist->cramrects[gxCramHandle->numTexturePages].y,cramlist->cramrects[gxCramHandle->numTexturePages].rotate?(gxCramHandle->transpose?"tranposed":"rotated"):"");

	/* Rotate shape */
	if (cramlist->cramrects[gxCramHandle->numTexturePages].rotate)
	{
		FINSTANCE *fx;
		GBITMAP source[1];
		FOP fop[1];
		int drowbytes;
		GINFO *ginfo0;
		char *f0;

		source[0].ginfo = ginfo;
		source[0].image = f;
		source[0].rowbytes = ginfo->width*ginfo->bpp/8;
		fop[0].i = 3;  /* 90 CCW */
		openrot(&fx, 1, source, 1, fop);
		ginfo0 = inforot(fx, 0);
		drowbytes = ginfo0->width * ginfo0->bpp/8;
		f0 = (char *) galloc(drowbytes * ginfo0->height);
		filterrot(fx, ginfo0, f0, drowbytes);
		closerot(fx);
		if (gxCramHandle->transpose)
		{
			GINFO *ginfo1;
			char *f1;

			source[0].ginfo = ginfo0;
			source[0].image = f0;
			source[0].rowbytes = ginfo0->width*ginfo0->bpp/8;
			fop[0].i = 2;  /* vertical */
			openflip(&fx, 1, source, 1, fop);
			ginfo1 = infoflip(fx, 0);
			drowbytes = ginfo1->width * ginfo1->bpp/8;
			f1 = (char *) galloc(drowbytes * ginfo1->height);
			filterflip(fx, ginfo1, f1, drowbytes);
			closeflip(fx);

			gfree(f0);
			gfree(ginfo0);
			f0 = f1;
			ginfo0 = ginfo1;
		}

		ginfo0->centerx = ginfo->centerx; /* keep hotspots unrotated */
		ginfo0->centery = ginfo->centery;
		memcpy(ginfo0->hotspottbl,ginfo->hotspottbl,sizeof(ginfo0->hotspottbl));

		gfree(f);
		gfree(ginfo);
		f = f0;
		ginfo = ginfo0;
	}

	/* find right most x value */
	{
		int w;
		int h = (ginfo->height);

		if (gxCramHandle->shapeType == sig)
		{
			// round the bpp up, if not a power of 2
			if (ginfo->originalbpp <= 4)
				w = (ginfo->width*4+15)/16;
			else if (ginfo->originalbpp <= 8)
				w = (ginfo->width*8+15)/16;
			else if (ginfo->originalbpp <= 16)
				w = (ginfo->width*16+15)/16;
			else
				w = (ginfo->width*32+15)/16;

			//w = (ginfo->width*ginfo->originalbpp+15)/16;
		}
		else
			w = ginfo->width;

		if (gxCramHandle->maxx < cramlist->cramrects[gxCramHandle->numTexturePages].x + w)
			gxCramHandle->maxx = cramlist->cramrects[gxCramHandle->numTexturePages].x + w;
		if (gxCramHandle->maxy < cramlist->cramrects[gxCramHandle->numTexturePages].y + h)
			gxCramHandle->maxy = cramlist->cramrects[gxCramHandle->numTexturePages].y + h;
		if (gxCramHandle->minx > cramlist->cramrects[gxCramHandle->numTexturePages].x)
			gxCramHandle->minx = cramlist->cramrects[gxCramHandle->numTexturePages].x;
		if (gxCramHandle->miny > cramlist->cramrects[gxCramHandle->numTexturePages].y)
			gxCramHandle->miny = cramlist->cramrects[gxCramHandle->numTexturePages].y;

		// add the amount of space that it took up
		gxCramHandle->spaceUsage += w*h;
	}

	if (gxCramHandle->centerpoint)
	{
		ginfo->centerx = cramlist->cramrects[gxCramHandle->numTexturePages].x;
		ginfo->centery = cramlist->cramrects[gxCramHandle->numTexturePages].y;
	}
	ginfo->defaultx = cramlist->cramrects[gxCramHandle->numTexturePages].x % gxCramHandle->twrapWidth;
	ginfo->defaulty = cramlist->cramrects[gxCramHandle->numTexturePages].y +
		gxCramHandle->h * (cramlist->cramrects[gxCramHandle->numTexturePages].x / gxCramHandle->twrapWidth);
	ginfo->subtype &= ~(SUBTRANSPOSE|SUBROTATE);

	if (cramlist->cramrects[gxCramHandle->numTexturePages].rotate)
	{
		if (gxCramHandle->transpose)
			ginfo->subtype |= SUBTRANSPOSE;
		else
			ginfo->subtype |= SUBROTATE;
	}
	// print out the shapes that caused cram to fail
	if (gxCramHandle->errors && ((gxCramHandle->frame+1) == gxCramHandle->frames))
	{
		if(gxCramHandle->verbose)
			printf("Failed shapes: \n");

		cramlist = &gxCramHandle->cramlink; // reset pointer to the first link
		while (cramlist != NULL)
		{
			for (i=0; i< cramlist->numrects; i++)
				if (cramlist->cramrects[i].done == 0)
					if(gxCramHandle->verbose)
						printf("     file#%d #%d \'%s\': position(%d,%d) size %dx%d\n",cramlist->framenumber[gxCramHandle->frame]/MAXFRAMES, cramlist->framenumber[i]%MAXFRAMES, cramlist->framename[i], cramlist->cramrects[i].x % gxCramHandle->twrapWidth, cramlist->cramrects[i].y, cramlist->cramrects[i].width, cramlist->cramrects[i].height);
							cramlist = cramlist->next;
		}
	}
	if(gxCramHandle->verbose && (gxCramHandle->frame+1) == gxCramHandle->frames)
	{
		printf("vram used: %d,%d .. %d,%d", gxCramHandle->minx,gxCramHandle->miny,gxCramHandle->maxx,gxCramHandle->maxy);
		if (gxCramHandle->shapeType == sig) printf(" (bpp offset)\n");
		else printf(" (pixel offset)\n");

		printf("vram wastage: %.2f%%\n", (1-(double)(gxCramHandle->spaceUsage)/((double)(gxCramHandle->maxx-gxCramHandle->minx) * (gxCramHandle->maxy-gxCramHandle->miny)))*100);
	}
	if(gxCramHandle->verbose)
	{
		if((gxCramHandle->frame+1) == gxCramHandle->frames)
			printf("Cram %s\n",!gxCramHandle->errors?"Successful":"Failed");
	}
	CRAM_close(gxCramHandle->handle);
    gxCramHandle->frame++;
	return(gxCramHandle->errors);
}

//-----------------------------------------------------------------------------------------
int GXCRAM_isInit(GXCRAMHANDLE * gxCramHandle)
{
	if(!gxCramHandle)
		return 0;
	return(gxCramHandle->init);
}

//-----------------------------------------------------------------------------------------
int GXCRAM_isLast(GXCRAMHANDLE * gxCramHandle)
{
	if(!gxCramHandle)
		return 0;
	return(gxCramHandle->frame == gxCramHandle->frames);
}

//-----------------------------------------------------------------------------------------
int GXCRAM_Close(GXCRAMHANDLE *gxCramHandle)
{
	CRAMLINK * cramlist = (gxCramHandle->cramlink).next;    // don't free first item since it's
	CRAMLINK * current  = NULL;								// part of the GXCRAMHANDLE.
	while(cramlist)
	{
		current = cramlist;
		cramlist = cramlist->next;
		gfree(current);
	}
	gxCramHandle->init = 0;
	gfree(gxCramHandle);
	return 1;
}

/*
//-----------------------------------------------------------------------------------------
int GXCRAM_UpdateFlatten(GXCRAMHANDLE *gxCramHandle,GINFO **composite_ginfo,char **composite_f,int *composite_rowbytes)
{
	CRAMLINK * cramlist = &gxCramHandle->cramlink;
	GINFO * ginfo       = (*composite_ginfo);
	char *  f           = (*composite_f);
	char *  ftemp       = NULL;
	int oldwidth        = ginfo->width;
	int oldheight       = ginfo->height;
	int widthflag       = 0;
	int heightflag      = 0;
	int i;
	int width           = 0;
	int height          = 0;
	int defaultx        = 0;
	int defaulty        = 0;

	while(cramlist != NULL)                         // go through entire list
	{
		for(i = 0; i < cramlist->numrects;i++)      // for all entries.
		{
			// calculate the new defaultx and defaulty values.
			defaultx = cramlist->cramrects[i].x % gxCramHandle->twrapWidth;
			defaulty = cramlist->cramrects[i].y +
						gxCramHandle->h * (cramlist->cramrects[gxCramHandle->numTexturePages].x / gxCramHandle->twrapWidth);
			// figure out the width and height.
			width    = cramlist->cramrects[i].width;
			height   = cramlist->cramrects[i].height;
			if(ginfo->width < (width + defaultx))			// check if my final frame is going to
			{												// be bigger than the estimated frame.
				widthflag = 1;								// have to update for this.
				ginfo->width = (width + defaultx);
			}
			if(ginfo->height < (height + defaulty))
			{
				heightflag = 1;
				ginfo->height = (height + defaulty);
			}

		}
		cramlist = cramlist->next;                          // do all lists.
	}
	if(widthflag || heightflag)                             // if I need to update.
	{
		width  = oldwidth;									// find the max of the width and heights.
		height = oldheight;
		if(ginfo->width > oldwidth)											 // always choose the larger.
			width = ginfo->width;
		if(ginfo->height > oldheight)
			height = ginfo->height;
		ftemp = (char *) galloc((width*height/8*ginfo->bpp));				 // make bigger image memory.
		if(ftemp)															 // if I was successful
		{
			unsigned int fbytes = 0;
			unsigned int ftempbytes = 0;
			unsigned int ftemprowbytes = width/8*ginfo->bpp; 
			memset(ftemp,0,(width*height/8*ginfo->bpp));					 // clear the memory.
			for(i = 0; i < oldheight;i++)
			{
				memcpy((ftemp+ftempbytes),(f+fbytes),(*composite_rowbytes)); // copy over old image.
				ftempbytes += (ftemprowbytes);								 // place image in upper left corner 
				fbytes     += (*composite_rowbytes);						 // of image.
			}
			if(widthflag)													 // if I need to update the rowbytes.
				*composite_rowbytes = (ftemprowbytes);
			gfree(f);														 // make the composite_f point to my new
			(*composite_f) = ftemp;											 // memory.
		}
		else																 // be graceful and die nicely.
		{																	 // if I cannot malloc enough memory.
			ginfo->width  = oldwidth;
			ginfo->height = oldheight;
			return 0;
		}
	}
	return 1;																 // return success.
}
*/


