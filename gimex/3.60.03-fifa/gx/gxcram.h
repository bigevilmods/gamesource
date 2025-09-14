// (c) 1994-2004 Electronic Arts Inc.
//-----------------------------------------------------------------------------------------
// Wrapper for integration of cram into gx
//               - NOTE THIS ONLY WORKS ON SHAPES!
//-----------------------------------------------------------------------------------------

#ifndef _GXCRAM_H_
#define _GXCRAM_H_ 1

#include "gimex.h"
#include "cram.h"

#define CRAMMAXRECTS 10000

#ifndef SUBROTATE
#define SUBROTATE       0x800
#endif
#ifndef SUBTRANSPOSE
#define SUBTRANSPOSE   0x1000
#endif

#define MAXFRAMES 1000000

//-----------------------------------------------------------------------------------------
typedef struct GXCRAMGROUP	
{
	int num; 
	struct IMAGE
	{
		int bpp;
		int palette;
	} image[20];
} GXCRAMGROUP;

//-----------------------------------------------------------------------------------------
// used in the cram linked list.
typedef struct CRAMLINK
{
	CRAMRECT cramrects[CRAMMAXRECTS];
	int numrects;
	int framenumber[CRAMMAXRECTS];
	char framename[CRAMMAXRECTS][GIMEX_FRAMENAME_SIZE];
	struct CRAMLINK *next;
} CRAMLINK;

//-----------------------------------------------------------------------------------------
// maintenance information for successive calls to AddToCramList() etc..
typedef struct GXCRAMHANDLE
{
    int init;
    int frames;
    int frame;
//    int argi;
    int errors;//=0;

    int x;// = 0;      /* default dimensions of cram rect */
    int y;// = 256;
    int w;// = 1024;
    int h;// = 256;
    int numfixed;// = 0;
    int rotate;// = 0; /* non zero if crammer can rotate shapes */
    int transpose;// = 0;
    int verbose;// = 0;
	int checkedge;// = 0;
	int centerpoint;// = 0;
	int oddOdd;// = 0;
	int bppsort;// = 0;
	int clutsort;// = 0;
	int tpageWidth;// = 256;
	int tpageHeight;// = 256;
	int twrapWidth;// = 0;
	int widthin;// = 0;
	int space;// = 0;

	int groupsort;// = 0;
	GXCRAMGROUP group[20]; // the first dimension stores the grouping number, the second stores
														// all the bit depths for that group
	int numTexturePages;// = 0;
	int shapeType;
	int spaceUsage;// = 0;

    int maxx;// = 0; /* rightmost x value, returned so we know where we can do another cram on a different file */
    int maxy;// = 0; /* rightmost x value, returned so we know where we can do another cram on a different file */
    int minx;// = 1024; /* rightmost x value, returned so we know where we can do another cram on a different file */
    int miny;// = 512; /* rightmost x value, returned so we know where we can do another cram on a different file */
	CRAMLINK cramlink;
	CRAMRECT cramrects[CRAMMAXRECTS];
	CRAM *handle;

}GXCRAMHANDLE;

//-----------------------------------------------------------------------------------------
// called on second pass after the cram list has been made. This will succesively 
// cram each frame and modify the data and the ginfo structure accordingly.
int GXCRAM_DoCram(GXCRAMHANDLE * gxCramHandle,GINFO **ginfodata,char ** fdata);
//-----------------------------------------------------------------------------------------
// add an item to the current cram list to be parsed in the second pass.
int GXCRAM_AddToCramList(GXCRAMHANDLE * gxCramHandle, GINFO * ginfo);      
//-----------------------------------------------------------------------------------------
// closes and deletes the current cramhandle.
int GXCRAM_Close(GXCRAMHANDLE *gxCramHandle);
//-----------------------------------------------------------------------------------------
// is the current pass of DoCram the last frame to be crammed? 
// used to let me know when to call CramClose.
int GXCRAM_isLast(GXCRAMHANDLE * gxCramHandle);
//-----------------------------------------------------------------------------------------
// check if a cram handle has been initialized can pass in a null pointer this is
// assumed to be non initialized.
int GXCRAM_isInit(GXCRAMHANDLE * gxCramHandle);
//-----------------------------------------------------------------------------------------
// must be called before anything else. (except isInit) This function returns a valid 
// cram handle to be used add, and then cram all shapes.
GXCRAMHANDLE *GXCRAM_Init(int shapeType,			// pass  in the shape code ie SSH etc.
						  int transpose,			// should I transpose?
						  int rotate,				// can I rotate?
						  int verbose,				// should I spit out dump information.
						  int centerpoint,			// should I update centerpoint?
						  int checkedge,			// should I checkedge?
						  int oddOdd,				// should I if one side odd make other side odd?
						  int bppsort,				// should I sort into like groupings of bpp?
						  int clutsort,				// should I sort based on clut?
						  int groupsort,			// should I sort based on groups?
						  GXCRAMGROUP * group,		// group list defined as size 20.
						  int tpageWidth,			// texture page width.
						  int tpageHeight,			// texture page height.
						  int twrapWidth,			// should I wrap?
						  int widthin,				
						  int space,				
						  int x,					// page start
						  int y, 
						  int w,					// page width and height.
						  int h,
						  int numfixed);			// fixed items?

//-----------------------------------------------------------------------------------------
// helper function for cramming and flattening at the same time function updates the 
// composite structures and memory sizes for flatten to operate correctly.
int GXCRAM_UpdateFlatten(GXCRAMHANDLE *gxCramHandle,GINFO **composite_ginfo,char **composite_f,int *composite_rowbytes);

#endif

