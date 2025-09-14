// (c) 1994-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*             ILBM - Amiga InterLeaved BitMap Gimex                */
/*                                                                  */
/*                    by FrANK G. Barchard, EAC                     */
/*               Original Latenight Code by Brad Gour               */
/*             Initial Gimex conversion by Erik Pauker              */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History:                                       */
/* ------- ----   -- --------                                       */
/*    0.90 870101 BG Latenight 2                                    */
/*    1.00 940517 EP Based on latenight lbm module                  */
/*    1.01 940531 FB Updated to 1.03 Gimex spec                     */
/*    1.02 940614 FB partially updated to Gimex 1.5                 */
/*    1.03 940715 FB complete 1.5 update.  Considerable rewrite     */
/*    1.04 940715 FB started 24 bit, better parsing                 */
/*    1.05 940717 FB better parsing, error checking                 */
/*    1.06 940717 FB 24 bit, use mask on 24 bit                     */
/*    1.07 941001 FB casting for SGI                                */
/*    1.08 941027 FB export work buffer size increased for packing  */
/*    1.09 941028 FB move max rowbytes to lbmgimex.h                */
/*    1.10 941224 FB word align on write                            */
/*    1.11 941227 FB support x/y location                           */
/*    1.12 950131 FB treat -128 code as run for Photoshop style     */
/*    1.13 950516 FB openlbm return 1 or 0 (number of frames)       */
/*    1.14 950529 FB ginstance signature                            */
/*    1.15 951212 FB optimized getplanerow, alpharow                */
/*    1.16 961107 FB crng, anno & grab chunk import/export          */
/*    1.17 961120 FB crng dont write if 0 size                      */
/*    1.18 970618 GS check before writing optional fields           */
/*    1.19 970814 KO 24bit write                                    */
/*    1.20 980501 KO islbm made more strict, MAT files are similar  */
/*    1.21 000910 FB max 24 bits, not 32                            */
/*    1.22 001003 FB maya iff import                                */
/*    1.23 010122 ID maya iff import                                */
/*    1.24 010124 ID maya iff import compression                    */
/*    1.25 020306 AE IFF import bug corrected                       */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 1-8,24 bit                                                       */
/* Reentrant                                                        */
/* Files: lbmwrite.cpp lbmread.cpp lbmgimex.h                       */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Format Notes:                                                    */
/* -------------                                                    */
/* ILBM files support 1-8 bit indexed, and 24 bit direct, with      */
/* RLE on bit planes.                                               */
/* There is an optional mask plane.                                 */
/* PBM is a packed pixel (chunky) 8 bit variant.                    */
/* Supports default position, center point and comment              */
/* Maya iff is 32 bit with same layout as standard iff but unique   */
/* chunk IDs                                                        */
/*                                                                  */
/* Standard iff 24 bit header (john.lbm)                            */
/* -------------------------------------                            */
/* 00000: 464f524d 000295c4 494c424d 424d4844  FORM...DILBMBMHD     */
/* 00010: 00000014 01000100 00000000 18000100  ................     */
/* 00020: 00000506 01000100 424f4459 0002959c  ........BODY....     */
/* 00030: 1fb798db 985d6e6b a7bef025 350653d2  .7.[.]nk'>p%5.SR     */
/* 00040: e078888e 38c14b1d 7e734cdd a86cd1fa  `x..8AK..sL](lQz     */
/*                                                                  */
/* Maya iff 32 bit (correct.iff)                                    */
/* -----------------------------                                    */
/* 00000: 464f5234 000b8fe0 43494d47 46564552  FOR4...`CIMGFVER     */
/* 00010: 00000004 00010001 54424844 00000020  ........TBHD...      */
/* 00020: 0000020c 00000169 00010001 00000003  .......i........     */
/* 00030: 00000036 00000000 00000000 00000000  ...6............     */
/* 00040: 41555448 0000001e 6576206f 6e207361  AUTH....ev on sa     */
/* 00050: 7463686d 6f2c2049 52495836 3420362e  tchmo, IRIX64 6.     */
/* 00060: 35204950 33300000 44415445 00000018  5 IP30..DATE....     */
/* 00070: 54756520 53657020 31322032 323a3239  Tue Sep 12 22:29     */
/* 00080: 3a353820 32303030 464f5234 000b8f14  :58 2000FOR4....     */
/* 00090: 54424d50 52474241 00004008 00000000  TBMPRGBA..@.....     */
/* 000a0: 003f003f ff3c3f42 ff3c4040 ff3b4040  .?.?.<?B.<@@.;@@     */
/* 000b0: ff3c4141 ff3d4043 ff3f4141 ff3d4545  .<AA.=@C.?AA.=EE     */
/*                                                                  */
/* Dump of some dpaint colour range animation chunks:               */
/* 003a0: 43524e47 00000008 152a0099 0000000f  CRNG.....*......     */
/* 003b0: 43524e47 00000008 2b950132 0002101f  CRNG....+..2....     */
/* 003c0: 43524e47 00000008 172a0265 0000202f  CRNG.....*.e.. /     */
/* 003d0: 43524e47 00000008 2ebf04cb 0002303f  CRNG.....?.K..0?     */
/* 003e0: 43524e47 00000008 1f2a0999 0000404f  CRNG.....*....@O     */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */
#include <string.h>
#include "gimex.h"
#include "lbmgimex.h"

#define GOFFSETRED      (&((ARGB*)0)->r - (unsigned char*)(0))
#define GOFFSETGREEN    (&((ARGB*)0)->g - (unsigned char*)(0))
#define GOFFSETBLUE     (&((ARGB*)0)->b - (unsigned char*)(0))
#define GOFFSETALPHA    (&((ARGB*)0)->a - (unsigned char*)(0))

/****************************************************************/
/*  Local Functions                                             */
/****************************************************************/

static int NextField(unsigned char *FilePtr, int align)
{
    return(8+((ggetm(FilePtr+4,4)+align)&~align));
}


static short get_short( unsigned char ** Data )
{
      short t;
//    char buf[2];
//    buf[0] = *(*Data);
//    (*Data)++;
//    buf[1] = *(*Data);
//    (*Data)++;
//    return ( buf[0] << 8 ) + ( buf[1] << 0 );
      t = ggetm(*Data,2);
      (*Data) += 2;
      return(t);
}

static long get_long( unsigned char ** Data )
{
      long t;
//    char buf[4];
//    buf[0] = *(*Data);
//    (*Data)++;
//    buf[1] = *(*Data);
//    (*Data)++;
//    buf[2] = *(*Data);
//    (*Data)++;
//    buf[3] = *(*Data);
//    (*Data)++;

//    return ( buf[0] << 24 ) + (buf[1]<<16) + (buf[2] << 8) + ( buf[3] << 0 );
      t = ggetm(*Data,4);
      (*Data) += 4;
      return(t);
}




static void CnvtPalette(unsigned char *SrcPtr, GINFO *ginfo)
{
    int i;
    int ColourCount;
    unsigned char *ptr;

    ptr = (unsigned char *) SrcPtr + 8;       /* skip the "CMAP" and length field */

    ColourCount = (int)ggetm(SrcPtr+4,4)/3;

    if (ColourCount > 256)
    {
        ColourCount = 256;
    }

    ginfo->numcolours = ColourCount;

    for (i = 0; i < ColourCount; ++i)
    {
        ginfo->colourtbl[i].a = 255;
        ginfo->colourtbl[i].r = *ptr++;
        ginfo->colourtbl[i].g = *ptr++;
        ginfo->colourtbl[i].b = *ptr++;
    }
}



static void LBMUnPackBits(unsigned char **srcHandle, unsigned char **destHandle, int n)
{
    int  i;
    int  c;
    char val;
    unsigned char *s;
    unsigned char *d;

    s = *srcHandle;
    d = *destHandle;

    while (n>0)
    {
        c = (int) ((signed char) *s++);
        if (c >= 0)         /* (0-127) copy n+1 bytes literally */
        {
            ++c;
            n -= c;
            for (i = 0; i < c; ++i)
            {
                *d++ = *s++;
            }
        }
        else /* if (c != -128) \* (-1..-127) repeat byte -n+1 times *\ */
        {
            c = -c+1;
            n -= c;
            val = *s++;
            for (i = 0; i < c; ++i)
            {
                *d++ = val;
            }
        }
    }
    *srcHandle  = s;
    *destHandle = d;
}

static void SetAlphaRow(unsigned char *d, int width)
{
    int x;

    for (x=0; x<width; ++x)
    {
        *(d+GOFFSETALPHA) = (unsigned char)255;
        d += 4;
    }
}

static void ExtendAlphaRow(unsigned char *d, int width)
{
    int x;

    for (x=0; x<width; ++x)
    {
        *(d+GOFFSETALPHA) = ((signed char) *(d+GOFFSETALPHA))>>7;   /* asr */
        d += 4;
    }
}


static void GetPlaneRow(unsigned char *dest, unsigned char *source, int width, int plane, int planes)
{
    int i;
    unsigned char bits;
    char bitplane;
    int destmod=1;
    int channel;
    int bitsperchannel;
    int channeltbl[4];

    channeltbl[0] =      GOFFSETRED;
    channeltbl[1] =      GOFFSETGREEN;
    channeltbl[2] =      GOFFSETBLUE;
    channeltbl[3] =      GOFFSETALPHA;

    if (planes>=16)
    {
        bitsperchannel = gmin(8,planes/3);
        channel = channeltbl[plane/bitsperchannel];
        dest += channel;
        plane %= bitsperchannel;
        destmod = 4;
    }

    bitplane = (char) (1<<plane);
    for (i=0; i<width; i+=8)
    {
        bits = (unsigned char) *source++;
        if (bits&(1<<7)) *dest |= bitplane;
        dest += destmod;
        if (bits&(1<<6)) *dest |= bitplane;
        dest += destmod;
        if (bits&(1<<5)) *dest |= bitplane;
        dest += destmod;
        if (bits&(1<<4)) *dest |= bitplane;
        dest += destmod;
        if (bits&(1<<3)) *dest |= bitplane;
        dest += destmod;
        if (bits&(1<<2)) *dest |= bitplane;
        dest += destmod;
        if (bits&(1<<1)) *dest |= bitplane;
        dest += destmod;
        if (bits&(1<<0)) *dest |= bitplane;
        dest += destmod;
    }
}

static int UnPackILBMPic(LBMHeaderStruct *Hdr, unsigned char *Data, GINFO *ginfo, unsigned char *dest, int rowbytes)
{
    int row;
    int plane;
    int planes;
    int width;
    int BytesPerPlaneRow;
    unsigned char *workdest;
    unsigned char *LinePtr;
    int MaxHeight;
    unsigned char *IFFWorkPtr;
    int ok=0;

    planes = LBMGETFIELD(Hdr->nPlanes);
    IFFWorkPtr = (unsigned char *) galloc(MAXROWBYTES);
    if (IFFWorkPtr)
    {
        /* calculate bytes per plane, rounded to word */

        width = LBMGETFIELD(Hdr->w);
        BytesPerPlaneRow = ((LBMGETFIELD(Hdr->w)+15)&~15)/8;

        /* unpack it one line at a time */

        LinePtr = dest;

        MaxHeight = ginfo->height;

        for (row = 0; row < MaxHeight; ++row)
        {
            memset(LinePtr, 0, rowbytes);
            for (plane=0; plane<planes; ++plane)
            {
                workdest = IFFWorkPtr;
                if (LBMGETFIELD(Hdr->compression))
                {
                    /* unpack a row */
                    LBMUnPackBits(&Data, &workdest, BytesPerPlaneRow);
                    workdest = IFFWorkPtr;
                }
                else
                {
                    memcpy(workdest, Data, BytesPerPlaneRow);
                    Data += BytesPerPlaneRow;
                }

                /* convert bit-plane info to byte per pixel format */
                GetPlaneRow(LinePtr, workdest, width, plane, planes);
            }

            /* if masking, DPaint seems to add an extra plane (?) */

            if (LBMGETFIELD(Hdr->masking)==1)
            {
                /* unpack a row */

                workdest = IFFWorkPtr;
                LBMUnPackBits(&Data, &workdest, BytesPerPlaneRow);

                if (planes>=16)
                {
                    GetPlaneRow(LinePtr, workdest, width, 31, 32);
                    ExtendAlphaRow(LinePtr, width);
                }
            }
            else
                if (planes>=16)
                    SetAlphaRow(LinePtr, width);

            LinePtr += rowbytes;
        }
        gfree(IFFWorkPtr);
        ok = 1;
    }
    return(ok);
}

static int UnPackPBMPic(LBMHeaderStruct *Hdr, unsigned char *Data, GINFO *ginfo, unsigned char *dest, int rowbytes)
{
    int row;
    int BytesPerPlaneRow;
    unsigned char *workdest;
    unsigned char *LinePtr;
    int MaxHeight;
    unsigned char *IFFWorkPtr;
    int ok=0;

    IFFWorkPtr = (unsigned char *) galloc(MAXROWBYTES);
    if (IFFWorkPtr)
    {

        BytesPerPlaneRow = LBMGETFIELD(Hdr->w);

        /* unpack it one line at a time */

        LinePtr = dest;

        MaxHeight = ginfo->height;

        for (row = 0; row < MaxHeight; ++row)
        {
            workdest = IFFWorkPtr;

            if (LBMGETFIELD(Hdr->compression))
            {
                /* unpack a row */

                LBMUnPackBits(&Data, &workdest, BytesPerPlaneRow);
                workdest = IFFWorkPtr;
                memcpy(LinePtr, workdest, rowbytes);
            }
            else
            {
                memcpy(LinePtr, Data, rowbytes);
                Data += BytesPerPlaneRow;
            }

            LinePtr += rowbytes;
        }
        gfree(IFFWorkPtr);
        ok = 1;
    }
    return(ok);
}

//---------------------------------------------------------------------------------------------------------------------------
// function should read and decompress an rle compressed tile. However as of yet untested.
static unsigned char *GetCIMGCompressedRLETile(unsigned char **Data,int tile_width,int tile_height,int *tile_size,int numchannels)
{
   int i = 0;									// looper for copying in data from buffer.
   int m = 0;									// number of channels counter.
   int n = 0;									// counter for bytes to decrypt.
   unsigned char *ColorChannels[4];
   unsigned char *s;										// internal pointers to allow me to save typeing.
   unsigned char *d;
   unsigned char c;
   unsigned char count;
   char val;
   unsigned char *DestBuffer;
   unsigned char * saver = *Data;						// I want to always no matter if the block is corrupted be able
												// to jump to the end of the block so save my start here.
   int offset = 0;                              // a bit safer than playing with pointers, can do error checks.
												// remove this if speed is needed.
   int pad = 0;                                 // used to align to next block, blocks 4 byte aligned,
   int real_tile_size = tile_width*tile_height; // size of one channel.

   for(i = 0; i < numchannels; i++)
		ColorChannels[i] = (unsigned char *) galloc(real_tile_size); 

	for(m = 0; m < numchannels; m++)      // do this for each channel [-A-][-B-][-G-][-R-]
	{
			offset = 0;
			d = ColorChannels[m];
			s = *Data;
			if(d)                         // note DO NOT TRY TO REUSE THE LBM DECOMPRESSION
			{							  // they do the single value run check wrong!
				n = real_tile_size;
				while (n>0)               
				{
					c = *s++;
					if ((signed char)c >= 0)         /* (0-127) copy n+1 bytes literally */
					{
						++c;
						if((offset + c) > real_tile_size)break;    // just in case corrupted data.
						n -= c;
						for (i = 0; i < c; ++i)
						{
							d[offset + i] = *s++;
						}
						offset+= i;
						if(offset >= real_tile_size)break;         // error check.
					}
					else // if the byte is negative treat as sign magnitude not 2's compliment!
					{
						count = (c&0x7F)+1;
						n -= count;
						val = *s++;
						if((offset + count) > real_tile_size)break;  // error check.
						for (i = 0; i < count; ++i)
						{
							d[offset + i] = val;
						}
						offset+= i;
						if(offset >= real_tile_size)break;           // error check.
					}
				}//while(n>0)
				*Data = s;
			}// if(d)
	}// for loop
		
	DestBuffer = (unsigned char *) galloc(real_tile_size*4);
	if(numchannels == 3)  // we only have RGB information in the buffer so stuff the alpha channel.
	{
		for(i = 0; i < real_tile_size; i++)
		{
            DestBuffer[i*4+0] = (unsigned char)0xFF;//ColorChannels[0][i];
			DestBuffer[i*4+3] = ColorChannels[0][i];
			DestBuffer[i*4+2] = ColorChannels[1][i];
			DestBuffer[i*4+1] = ColorChannels[2][i];
			
		}
	}
	else if(numchannels == 4) // we have an alpha channel in our information so pop it into the buffer.
	{
		for(i = 0; i < real_tile_size; i++)
		{
			DestBuffer[i*4+0] = ColorChannels[0][i];
			DestBuffer[i*4+3] = ColorChannels[1][i];  // the flip the bits BGR instead of RGB.
			DestBuffer[i*4+2] = ColorChannels[2][i];
			DestBuffer[i*4+1] = ColorChannels[3][i];
		}
	}

	for(i = 0; i < numchannels; i++)
		gfree(ColorChannels[i]);

	pad = *tile_size%4; // find overflow for pab.
	pad = (4 - pad)%4;

	*Data = saver + *tile_size - 8 + pad;    // do this before I change the tile size to the real tile size.
	*tile_size = real_tile_size*4;           // change tile size to reflect buffer size.
	return(DestBuffer);

}


//-------------------------------------------------------------------------------------------------------------
// assumes the image data is not compressed reads the tile out of buffer, must be queued up to RGBA tag.
static unsigned char *GetCIMGTile(unsigned char **Data,int *tile_width,int *tile_height,int *x1,int *y1,int *x2,int *y2,int *tile_size,int numchannels)
{
   int ID = ggetm(*Data,4);            // tag check. is this an RGBA block?
   unsigned char * DestBuf;
   int i = 0;
   int index = 0;

   *Data = *Data + 4;

   if(ID == RGBA)
   {
	    *tile_size = get_long(Data);  // get header information and send back up the pipeline.    
		*x1   = get_short(Data);
		*y1   = get_short(Data);
		*x2   = get_short(Data);
		*y2   = get_short(Data);
		*tile_width  = *x2 - *x1 + 1; 
		*tile_height = *y2 - *y1 + 1;

//#ifdef DEBUG
//		printf("\nTile Data:\nTile Size:%d,\nTile StartX:%d,\nTile StartY:%d,\nTile EndX:%d,\nTile EndY:%d,\nTile Width:%d,\nTile Height:%d\n",*tile_size,*x1,*y1,*x2,*y2,*tile_width,*tile_height);
//#endif
		
	    if ((*tile_size) >= ((*tile_width)*(*tile_height)*numchannels + 8 )) 
		{ /* Tile is not compressed so just read in the image data */                
		   if(numchannels == 4)
		   {
//#ifdef DEBUG
//			   printf("Tile is uncompressed and has RGBA data. \n");
//#endif
				(*tile_size) -= 8;
				DestBuf = (unsigned char *) galloc(*tile_size);
                for(i = 0; i < (*tile_size); i+=4)
				{
                    ((ARGB*)(&DestBuf[i]))->a = ((ARGB*)(&((*Data)[i])))->a;
                    ((ARGB*)(&DestBuf[i]))->r = ((ARGB*)(&((*Data)[i])))->r;
                    ((ARGB*)(&DestBuf[i]))->g = ((ARGB*)(&((*Data)[i])))->g;
                    ((ARGB*)(&DestBuf[i]))->b = ((ARGB*)(&((*Data)[i])))->b;
                }
				*Data = *Data + *tile_size; // tile size include the x1,x2,y1,y2 data have to skip that.
				return(DestBuf);
		   }
		   else if(numchannels == 3)
		   {
//#ifdef DEBUG
//			   printf("Tile is uncompressed and has only RGB data.\n");
//#endif
			   	(*tile_size) -= 8;
				DestBuf = (unsigned char *) galloc(((*tile_size) + (*tile_width)*(*tile_height)));
				index = 0;
				for(i = 0; i < (*tile_size); i+=3)
				{
                   // We read in RGBA from file, need to write as ARGB, so reverse.

					((ARGB*)(&DestBuf[index]))->a = (*Data)[i+2];
                    ((ARGB*)(&DestBuf[index]))->r = (*Data)[i+1];
                    ((ARGB*)(&DestBuf[index]))->g = (*Data)[i+0];
                    ((ARGB*)(&DestBuf[index]))->b = 0xFF;
					index += 4;
				
					
				}
				*Data = *Data + *tile_size; // tile size include the x1,x2,y1,y2 data have to skip that.
				return(DestBuf);
		   }
		} // end IF uncompressed.
		else 
		{
//#ifdef DEBUG
//			printf("Tile is compressed.... decompressing...\n");
//#endif 
			DestBuf = GetCIMGCompressedRLETile(Data,*tile_width,*tile_height,tile_size,numchannels);
			return(DestBuf);
		}
   }


//#ifdef DEBUG
//	   printf("Bad Image Block Header Skipping Block...ID = %d\n",ID);
//#endif
   *Data = *Data + *tile_size - 4; // skip both the offset as above and the size info.

   return((unsigned char *)NULL);
}

//-------------------------------------------------------------------------------------------------------------
// assume queued up with a single tile in the buffer and dest buffer is large enough to hold entier picture,
// copies tile over into the main image.
static void FormatCIMGOutput(unsigned char *dest,unsigned char *TileBuffer,int tile_width,int tile_height,int image_width,int image_height,int x0,int y0)
{
	int row = 0;
	int col = 0;
	int offset = 0;
	int flipy = image_height - y0 - tile_height;    // flip the image vertically... UGH but necessary.
	int start = flipy*image_width*4 + x0*4;			// calculate the starting offset in the image buffer.
	//int start = y0*image_width*4 + x0*4;
	unsigned char * d = dest;								
	int real_tile_width  = tile_width*4;		    // try to minimize multiplications etc. 
	int real_image_width = image_width*4;
	int index  = 0;
	int index2 = 0;

	for(row = 0; row < tile_height; row++)
	{
		offset = real_tile_width*(tile_height - row - 1);  // have to flip each tile so this 
														   // is a bit more complex than it could be.
		for(col = 0; col < real_tile_width;col+=4)
		{
			index = start + col;
			index2 = offset + col;
			d[index + 3/*start + (col*4+3)*/] = TileBuffer[index2 + 0/*offset + (col*4+0)*/];
			d[index + 2/*start + (col*4+2)*/] = TileBuffer[index2 + 1/*offset + (col*4+1)*/];
			d[index + 1/*start + (col*4+1)*/] = TileBuffer[index2 + 2/*offset + (col*4+2)*/];
			d[index + 0/*start + (col*4+0)*/] = TileBuffer[index2 + 3/*offset + (col*4+3)*/];

		}
		start += real_image_width;
	}
	return;
}

#define RGB_FLAG   0x01
#define ALPHA_FLAG 0x02
static int UnPackCIMGPic(CIMGHeaderStruct *Hdr, unsigned char *Data, GINFO *ginfo, unsigned char *dest, int rowbytes)
{

    //int row;
    //int col;
    int i;
    unsigned char *TileBuffer;
    unsigned char *d;
    unsigned char **pd;
    int tiles = LBMGETFIELD(Hdr->tiles);
    int flags = LBMGETFIELD(Hdr->flags);
    int numchannels = 0;
    //unsigned long  size;
    int tile_width,tile_height,tile_size;
    int x1,x2,y1,y2;

	if(flags & RGB_FLAG)
	   numchannels += 3;
	if(flags & ALPHA_FLAG)
	   numchannels += 1;
    // unpack it one line at a time

//#ifdef DEBUG
//    printf("The CIMG Unpacker is at work...\n");
//#endif

    d = Data - 16;  // have to do this to grab size information... UGH don't want to break old code.
	pd = &d;

//#ifdef DEBUG
//	printf("# TILES: %d\n",tiles);
//#endif

	for(i = 0; i < tiles; i++)
	{
		TileBuffer = GetCIMGTile(pd,&tile_width,&tile_height,&x1,&y1,&x2,&y2,&tile_size,numchannels);
		if(TileBuffer)
		{
			FormatCIMGOutput(dest,TileBuffer,tile_width,tile_height,ginfo->width,ginfo->height,x1,y1);
			gfree(TileBuffer);
		}
	}

//#ifdef DEBUG
//    printf("The CIMG Unpacker is done...\n");
//#endif

    return(1);
}

static int ProcessILBMBitmap(GINFO *ginfo, unsigned char *dest, int rowbytes, unsigned char *BasePtr, int process, int type)
{
    LBMHeaderStruct *Hdr      = 0;
    CIMGHeaderStruct *CIMGHdr = 0;
    int Offset;
    int ID;
    int FormLength;
    unsigned char *BodyPtr;
    int bpp;
    int colours = 0;
    int width;
    int height;
    int packed;
    int defaultx;
    int defaulty;
    int subtype;
    int numanims=0;
    int ok=0;
    int align=(type==CIMG)?(4-1):(2-1); // iff is 2 byte aligned, maya is 4
    int alphabits = 0;
    Hdr = 0L;
    BodyPtr = 0L;
    Offset = 12;

    FormLength = ggetm(BasePtr+4,4)+8;

    do
    {
        if (Offset>=FormLength || Offset<=0)
        {
            ok = -1;
        }
        else
        {
            ID = ggetm(BasePtr+Offset,4);
            if (ID==CMAP)
            {
                if (process==GETBHDR)
                    CnvtPalette(BasePtr+Offset, ginfo);
                ok |= 1;
            }
            else if (ID==BMHD || ID==TBHD)
            {
                Hdr = (LBMHeaderStruct *)(BasePtr+Offset+8);
                if (ID==BMHD)
                {
                    width    = LBMGETFIELD(Hdr->w);               /* Width of bitmap in pixels */
                    height   = LBMGETFIELD(Hdr->h);               /* Height of bitmap in pixels */
                    defaultx = LBMGETFIELD(Hdr->x);               /* Original X position */
                    defaulty = LBMGETFIELD(Hdr->y);               /* Original Y position */
                    bpp      = LBMGETFIELD(Hdr->nPlanes);         /* bpp of original bmp */
                    packed   = LBMGETFIELD(Hdr->compression);     /* Type of packing on original image. 0 none, 1 run, n other */
					alphabits= 0;
                }
                else
                {
                    CIMGHdr = (CIMGHeaderStruct *)(BasePtr+Offset+8);

                    width    = LBMGETFIELD(CIMGHdr->w);               /* Width of bitmap in pixels */
                    height   = LBMGETFIELD(CIMGHdr->h);               /* Height of bitmap in pixels */
                    defaultx = 0;
                    defaulty = 0;
                    bpp      = (LBMGETFIELD(CIMGHdr->flags) & RGB_FLAG)?((LBMGETFIELD(CIMGHdr->flags) & ALPHA_FLAG)?32:24):0;
                    packed   = LBMGETFIELD(CIMGHdr->compression);
					alphabits= (LBMGETFIELD(CIMGHdr->flags)&ALPHA_FLAG)?8:0;
			    }
                subtype = 0;
                if (type==PBM) subtype = 1;
                if (type==CIMG) subtype = 2;

                if (process==GETBHDR)
                {
                    /* template for ginfo */
                    ginfo->signature        = GMAKEID('I','L','B','M');   /* Type of file */
                    ginfo->size             = sizeof(GINFOOLDSHP);         /* Length of GINFO */
                    ginfo->version          = GIMEX_VERSION;
                    ginfo->framenum         = 0;                        /* Number of frames (if more than 1, openframeblt required) */
                    ginfo->width            = width;                    /* Width of bitmap in pixels */
                    ginfo->height           = height;                   /* Height of bitmap in pixels */
                    ginfo->subtype          = subtype;                  /* subtype 0 ilbm, 1 pbm */
                    ginfo->packed           = packed;
                    ginfo->quality          = 100;          /* Quality of lossy packing 0..100 */
                    ginfo->alphabits        = alphabits;    /* Number of bits in alpha channel */
                    ginfo->redbits          = 8;            /* Number of bits in red channel */
                    ginfo->greenbits        = 8;            /* Number of bits in grreen channel */
                    ginfo->bluebits         = 8;            /* Number of bits in blue channel */
                    ginfo->defaultx         = defaultx;     /* Default Coordinate Point */
                    ginfo->defaulty         = defaulty;
                    ginfo->numhotspots      = 0;            /* Number of Hot Spots defined */
                    ginfo->framename[0]     = 0;            /* Null terminated name of frame/image */

                    if (bpp>8)
                    {
                        ginfo->bpp          = 32;       /* Bits per pixel (8 or 32) */
                        ginfo->originalbpp  = bpp;
                        ginfo->numcolours   = 0;        /* Number of colours in original indexed palette */
                        ginfo->redbits = ginfo->greenbits = ginfo->bluebits = gmin(8,bpp/3);
                        ginfo->alphabits    = bpp-(gmin(8,bpp/3)*3);
                    }
                    else
                    {
                        if (!bpp)        bpp = 8;
                        if (bpp>8)       bpp = 8;
                        if (!colours)    colours = (1 << bpp);
                        if (colours>256) colours = 256;

                        ginfo->bpp          = 8;                        /* Bits per pixel (8 or 32) */
                        ginfo->originalbpp  = bpp;
                        ginfo->numcolours   = colours;                  /* Number of colours in original indexed palette */
                    }
                }
                if (bpp>=15)
                    ok |= 1;    /* dont need a palette */
                ok |= 2;
            }
            else if (ID==GRAB)
            {
                if (process==GETBHDR)
                {
                    ginfo->centerx = ggetm(BasePtr+Offset+8,2);
                    ginfo->centery = ggetm(BasePtr+Offset+10,2);
                }
            }
            else if (ID==CRNG)
            {
                if (process==GETBHDR)
                {
                    GINFOOLDSHP *ginfoshp = (GINFOOLDSHP *)ginfo;
                    int clock = ggetm(BasePtr+Offset+10,2);
                    int flag  = ggetm(BasePtr+Offset+12,2);
                    int start = ggetm(BasePtr+Offset+14,1);
                    int end   = ggetm(BasePtr+Offset+15,1);

                    ginfoshp->palanim[numanims].dir   = flag?-1:1;
                    ginfoshp->palanim[numanims].speed = (clock+273/2)/273;
                    ginfoshp->palanim[numanims].start = start;
                    ginfoshp->palanim[numanims].end   = end;
                    ++numanims;
                    ginfoshp->numanims = numanims;
                }
            }
            else if (ID==ANNO)
            {
                if (process==GETBHDR)
                {
                    int annolen = ggetm(BasePtr+Offset+4,4);
                    if (annolen>256) annolen = 256;
                    memcpy(ginfo->comment,BasePtr+Offset+8,annolen);
                }
            }
            else if (ID==BODY)
            {
                BodyPtr = BasePtr + Offset + 8;
                ok |= 4;
            }
            else if (ID==RGBA)
            {
                BodyPtr = BasePtr + Offset + 16;
                ok |= 4;
            }
            else if (ID==FOR4)
            {
                // FOR4 size TBMP RGBA
                if (ggetm(BasePtr+Offset+8,4)==GMAKEID('T','B','M','P'))
                {
                    Offset += 12;
                    continue;
                }
            }
            else if (ID==FVER)
            {
#ifdef DEBUG
                int v = ggetm(BasePtr+Offset+8,4);
                
#endif
            }
            Offset += NextField(BasePtr + Offset, align);
        }
    } while ((ok&7)!=7);

    /* We've got all the necessary information */

    if (ok==7)
    {
        ok = 1;
		
        if (process==DECODEILBM)
        {
			
            if (type==ILBM)
            {
				
                ok = UnPackILBMPic(Hdr, BodyPtr, ginfo, dest, rowbytes);
            }
            else if (type==PBM)
            {
				
                ok = UnPackPBMPic(Hdr, BodyPtr, ginfo, dest, rowbytes);
            }
            else if (type==CIMG)
            {
//#ifdef DEBUG
//                printf("CIMG body\n");
//#endif
				
                ok = UnPackCIMGPic(CIMGHdr, BodyPtr, ginfo, dest, rowbytes);
            }
        }
    }
    else
        ok = 0; /* didnt find necessary chunks */

    return(ok);
}

static int ProcessILBM(GINSTANCE *gx, GINFO *ginfo, unsigned char *dest, int rowbytes, int process)
{
    int FLength;
    int CurOffset = 0;
    int HunkID;
    int HunkLen;
    int ok=0;
    unsigned char *ref= (unsigned char *)gx->gref;

    FLength = (int)glen(gx->gstream);

    do
    {
        HunkID =  ggetm(ref,4);           /* get type of hunk */
        HunkLen = (ggetm(ref+4,4)+1)&~1;  /* get length of hunk (round to word) */

        if ((HunkID==FORM || HunkID==FOR4) && HunkLen>0 && HunkLen<FLength)
        {
            /* process a form */

            HunkID =  ggetm(ref+CurOffset+8,4);        /* get type of hunk */
            if (HunkID==ILBM || HunkID==PBM || HunkID==CIMG)
            {
                ok = ProcessILBMBitmap(ginfo, dest, rowbytes, ref + CurOffset, process, HunkID);
            }
        }
        CurOffset += HunkLen;

        if ((CurOffset>FLength) || (CurOffset<=0))
        {
            ok = -1;
        }
    } while (!ok);

    if (ok<0) ok = 0;

    return(ok);
}

/****************************************************************/
/*  Information Functions                                       */
/****************************************************************/

GABOUT * GCALL LBM_about(void)
{
    GABOUT *info;

    info = (GABOUT *) galloc(sizeof(GABOUT));
    if (info)
    {
        memset(info, 0, sizeof(GABOUT));

        info->signature       = GMAKEID('I','L','B','M');
        info->size            = sizeof(GABOUT);
        info->version         = GIMEX_VERSION;
        info->canimport       = 1;    /* supports importing */
        info->canexport       = 1;    /* supports exporting */
        info->importpacked    = 1;    /* supports importing packed */
        info->exportpacked    = 1;    /* supports exporting packed */
        info->import8         = 1;    /* supports importing 8 bit indexed */
        info->export8         = 1;    /* supports exporting 8 bit indexed */
        info->import32        = 1;    /* supports importing 32 bit direct rgb */
        info->export32        = 1;    /* supports exporting 32 bit direct rgb */
        info->multiframe      = 0;    /* supports multiple frames */
        info->multifile       = 0;    /* format requires additional files or resource fork */
        info->multisize       = 1;    /* supports each frame different size */
        info->framebuffer     = 1;    /* module requires memory to buffer entire frame */
        info->external        = 0;    /* module uses external resources */
        info->usesfile        = 1;    /* module is file based */
        info->maxframename    = 0;    /* max frame name character */
        info->defaultquality  = 100;  /* default pack quality */
        info->mactype[0]      = GMAKEID('I','L','B','M');   /* mac file system types used */
        strcpy(info->extensions[0], ".lbm");                /* extensions */
        strcpy(info->extensions[1], ".ilbm");
        strcpy(info->extensions[2], ".bbm");
#ifndef PLUGIN
        strcpy(info->extensions[3], ".iff");
#endif
        strcpy(info->authorstr,     "Frank Barchard");      /* authors */
        strcpy(info->versionstr,    "1.22");                /* version # */
        strcpy(info->shorttypestr,  "LBM");                 /* type */
        strcpy(info->wordtypestr,   "ILBM");                /* wordtype */
        strcpy(info->longtypestr,   "Amiga ILBM");           /* longtype */
    }
    return(info);
}

/* check for reasonable header: */

int GCALL LBM_is(GSTREAM *g)
{
    int FLength;
    char HunkHeader[12];
    int HunkID;
    int HunkLen;
    int ok=0;

    FLength = (int) glen(g);

    gseek(g, 0);
    if (gread(g, HunkHeader, 12))         /* type and length */
    {
        HunkID =  ggetm(HunkHeader,4);    /* get type of hunk */
        HunkLen = ggetm(HunkHeader+4,4);  /* get length of hunk */

        if ((HunkID==FORM || (HunkID==FOR4)) && (HunkLen>0) && (HunkLen<=FLength))
        {
            ok = 75;
            switch(ggetm(HunkHeader+8, 4))
            {
                case ILBM:
                case PBM:
                case CIMG:
                case GMAKEID('A','N','I','M'):
                    ok = 100;
                    break;
            }
        }
    }

    return(ok);
}

/****************************************************************/
/*  Import Functions                                            */
/****************************************************************/

int GCALL LBM_open(GINSTANCE **gxptr, GSTREAM *g, const char *pathname,bool framecountflag)
{
    GINSTANCE *gx;
    int ok=0;
    int FLength;
    char HunkHeader[8];
    int HunkID;
    int HunkLen;
    unsigned char *ref=0;

    gx = (GINSTANCE *) galloc(sizeof(GINSTANCE));
    if (gx)
    {
        memset(gx, 0, sizeof(GINSTANCE));
        gx->frames = 1;
        gx->signature=GMAKEID('I','L','B','M');
        gx->size=sizeof(GINSTANCE);
        gx->gstream = g;
        *gxptr = gx;

        FLength = (int)glen(gx->gstream);

        gseek(gx->gstream, 0);
        gread(gx->gstream, HunkHeader, 8);          /* type and length */

        HunkID  = ggetm(HunkHeader,4);              /* get type of hunk */
        HunkLen = (ggetm(HunkHeader+4,4)+1)&~1;      /* get length of hunk (round to word) */

        if ((HunkID==FORM || HunkID==FOR4) && HunkLen>0 && HunkLen<=FLength)
        {
            ref = (unsigned char *) galloc(HunkLen+8);
            if (ref)
            {
                memcpy(ref, HunkHeader, 8);    /* copy over FORM header */
                ok = gread(gx->gstream, ref + 8, HunkLen);    /* read FORM */
                gx->gref = ref;
                if (ok)
                    ok = 1;
            }
        }
    }
    if (!ok && ref)
    {
        gfree((void *)ref);
        *gxptr = 0;
    }
    if (!ok && gx)
    {
        gfree((void *)gx);
        *gxptr = 0;
    }

    return(ok);
}

GINFO * GCALL LBM_info(GINSTANCE *gx, int framenum)
{
    GINFO *ginfo;
    int ok;

    ginfo = (GINFO *) galloc(sizeof(GINFOOLDSHP));
    if (ginfo)
    {
        memset(ginfo, 0, sizeof(GINFOOLDSHP));
        ok = ProcessILBM(gx, ginfo, (unsigned char *) 0, 0, GETBHDR);
        if (!ok)
        {
            gfree((void *) ginfo);
            ginfo = 0;
        }
    }

    return(ginfo);
}

int GCALL LBM_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes)
{
    int ok;

    ok = ProcessILBM(gx, ginfo, (unsigned char *) dest, rowbytes, DECODEILBM);

    return(ok);
}

int GCALL LBM_close(GINSTANCE *gx)
{
    bool ok=0;

    if (gx)
    {
        ok = 1;
        if (gx->gref)
            ok &= gfree((void *) gx->gref);
        ok &= gfree((void *) gx);
    }
    return(ok);
}


