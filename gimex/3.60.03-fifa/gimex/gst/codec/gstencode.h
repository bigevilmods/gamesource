// (c) 2004-2007 Electronic Arts Inc.
// gst.cpp : Defines the entry point for the console application.

#define DOALPHA 1
#define DUMPBLOCKS 1
#define SHAPE2 0
#define SWIZZLE 1

static const int DIFFPRECISSION=0;


// F2 5 step method
// 1. seed - wu
// while need more blocks
//   while lloyd changing 
//      2. map 
//      3. center
//   4. split
// 5. save

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "crapi.h"
#include "resampleimage.h"
#include "swizzle.h"


namespace Gst
{


// *************************************
// Declarations
// *************************************

struct ShapeFile;
	
ShapeFile * GSTCompressImage(unsigned char *image, int rowbytes, int width, int height, 
	int *sizeofOutput, int *sizeofTexture, const char *name,
	int BlockWidth=4,int BlockHeight=2,int DetailBpp=2,int BaseBpp=8,int ClutBpp=32, int SwizzleTexture=1, int SwizzleClut=1, int Fastness=1, int MetalBin=1);

}

