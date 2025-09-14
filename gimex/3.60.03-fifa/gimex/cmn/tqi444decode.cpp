// (c) 1997-2007 Electronic Arts Inc.
/*
 *		TQI - Transformed Quantized Image Encoder
 *		
 *		Martin Griffiths, 
 *		
 *
 *
 *
 *		Last updated : 20th May 1999.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#include <fcntl.h>
#include <math.h>
#include "tqi444decode.h"

// Some globals..

static PIXELFORMAT_DESC pixformat;
static unsigned int display_init=0;
static unsigned char rand_table[256+4];

// Prescale values used to perform the Inverse Discrete Cosine Transform. 
// They are represented as 16.16 fixed point numbers.
// The formula used for idctPrescale[8*i+j] is f(i)*f(j), where
// f(0) = 1/sqrt(8) and f(i) = 1/(4*cos(i*PI/16)) for i=1..7.

static int idctPrescale[64] =
{
    0x02000, 0x01712, 0x0187e, 0x01b37, 0x02000, 0x028ba, 0x03b21, 0x073fc,
    0x01712, 0x010a2, 0x011a8, 0x0139f, 0x01712, 0x01d5d, 0x02aa1, 0x0539f,
    0x0187e, 0x011a8, 0x012bf, 0x014d4, 0x0187e, 0x01f2c, 0x02d41, 0x058c5,
    0x01b37, 0x0139f, 0x014d4, 0x01725, 0x01b37, 0x022a3, 0x03249, 0x062a3,
    0x02000, 0x01712, 0x0187e, 0x01b37, 0x02000, 0x028ba, 0x03b21, 0x073fc,
    0x028ba, 0x01d5d, 0x01f2c, 0x022a3, 0x028ba, 0x033d6, 0x04b42, 0x0939f,
    0x03b21, 0x02aa1, 0x02d41, 0x03249, 0x03b21, 0x04b42, 0x06d41, 0x0d650,
    0x073fc, 0x0539f, 0x058c5, 0x062a3, 0x073fc, 0x0939f, 0x0d650, 0x1a463
};

#ifndef PI
#define PI (3.14159265358979323846)
#endif

static void make_prescale(void)
{	
	for (int i = 0 ; i < 8 ; i++)
	{	for (int j=0 ; j < 8 ; j++)
		{	double fi;
			double fj;	
			if (i==0)
			{	fi = 1.0 / sqrt(8.0);
			} else
			{	fi = 1.0/(4*cos(i*PI/16));
			}

			if (j==0)
			{	fj = 1.0 / sqrt(8.0);
			} else
			{	fj = 1.0/(4*cos(j*PI/16));
			}

			idctPrescale[i*8+j] = (int) ((fi*fj) * (1<<23));
			
		}
	}
	



}



/*
 *	Debug related stuff.
 */

/*#ifdef DEBUG

static FILE *debugfile;

static void Msg( LPSTR fmt, ... )
{	char        buff[256];

	lstrcpy( buff, "TQI Image Decoder Message:" );
    //wvsprintf(&buff[lstrlen(buff)], fmt, (LPVOID)(&fmt+1) );
	wvsprintf(&buff[lstrlen(buff)], fmt, (char *)(&fmt+1) );
    lstrcat( buff, "\r\n" );
	fputs(buff,debugfile);
    OutputDebugString( buff );	
} 

static void tqi_open_debug(void)
{		debugfile = fopen("c:\\debug.log","w");

		if (debugfile == NULL)
		{	OutputDebugString("DEBUG logfile fopen FAILED!\n");
		} else
		{	OutputDebugString("DEBUG log opened!\n");
		}
}

static void tqi_close_debug(void)
{		fclose(debugfile);
		OutputDebugString("DEBUG log closed!\n");
}


#endif


static void tqi_error(char *text)
{
#ifdef DEBUG
	Msg(text);
	tqi_close_debug();
	MessageBox(NULL,text,"FMV Playback Error",MB_OK);
#endif

	exit(1);
}

static void tqi_warning(char *text)
{
#ifdef DEBUG
	Msg("%s\n",text);
#endif
} 
*/

//	Fixed point multiply..

//
#if defined(_MSC_VER) || defined(__WATCOMC__)
#define fixedmult(a,b) ((int) (((__int64)(a)*(__int64)(b) + 0x8000L) >> 16))
#else
#define fixedmult(a,b) ((int) (((int int)(a)*(int int)(b) + 0x8000LL) >> 16))
#endif

/*
static int fixedmult(int multplier,int multiplicand)
{	__asm {
			push edx
			mov		edx,multplier
			mov		eax,multiplicand
			imul	edx
			shl     edx,16        
			shr     eax,16        
			adc 	eax,edx
			pop		edx
	}
   
}

*/



static void	make_rand(void)
{	for (int i = 0 ; i < 256+3 ; i++)
		rand_table[i] = (rand());
		
}



// Create a clip table for RGB conversion

static int __inline YUVround( float x )
{
    if( x >= 0 ) return((int)(x+0.5));
    return((int)(x-0.5));
}

//#define TQI444FLOAT_TO_INT(_i,_f){float _ff = _f; __asm fld _ff __asm fistp dword ptr _i}
#define TQI444FLOAT_TO_INT(_i,_f) (_i = (int)(((float)(_f))+((((float)(_f))>=0.0f)?0.5f:-0.5f)))

#define SCALE_P2(_x) ( 1<<(8-(_x)) )
#define SCALEHP2(_x) ( 1<<(7-(_x)) ) 
#define POW2MASK(_x) ((1<<(_x))-1)

static void make_clip(void)
{	int i;
	if ((pixformat.BitsPerPixel == 16))
	{	for (i=-384; i<640; i++)
		{	int c = ( (i<0) ? 0 : ((i>255) ? 255 : i) );
			if (tqi444_dither_mode == TQI_FLOYD_DITHER)
			{	tqi444_clipA[i+384] = (c >> ( 8 - pixformat.Abits)) | ((c << (pixformat.Abits+24))&(255<<24));
				tqi444_clipR[i+384] = (c >> ( 8 - pixformat.Rbits)) | ((c << (pixformat.Rbits+24))&(255<<24));
				tqi444_clipG[i+384] = (c >> ( 8 - pixformat.Gbits)) | ((c << (pixformat.Gbits+24))&(255<<24));
				tqi444_clipB[i+384] = (c >> ( 8 - pixformat.Bbits)) | ((c << (pixformat.Bbits+24))&(255<<24));
			} else
			{	tqi444_clipA[i+384] = (c >> ( 8 - pixformat.Abits)) << pixformat.Ashifts;
				tqi444_clipR[i+384] = (c >> ( 8 - pixformat.Rbits)) << pixformat.Rshifts;
				tqi444_clipG[i+384] = (c >> ( 8 - pixformat.Gbits)) << pixformat.Gshifts;
				tqi444_clipB[i+384] = (c >> ( 8 - pixformat.Bbits)) << pixformat.Bshifts;

			}


		}		
	} else
	{	for (i=-384; i<640; i++)
		{	int c = ( (i<0) ? 0 : ((i>255) ? 255 : i) );
			tqi444_clipA[i+384] = (c >> ( 8 - pixformat.Abits)) << pixformat.Ashifts;
			tqi444_clipR[i+384] = (c >> ( 8 - pixformat.Rbits)) << pixformat.Rshifts;
			tqi444_clipG[i+384] = (c >> ( 8 - pixformat.Gbits)) << pixformat.Gshifts;
			tqi444_clipB[i+384] = (c >> ( 8 - pixformat.Bbits)) << pixformat.Bshifts;

		}	


	}
#if 0
    float   cb,cr,y;
    int     r,g,b;
    int     val;
    int     v1,v2;   

    for( i = -256 ; i < 256 ; ++i )
    {	int index = i + 256;
		int val = i;
		if (val < -128) val = -128;
		else if (val > 127) val = 127;
       
		r = (256 + val) & 0x1ff;
		
        tqi444_yuvluty[(index+128+16) &0x1ff] = ((r) << 23) + ((r) << 11) + (r+128);

        cb = (float) val;
		TQI444FLOAT_TO_INT(g, (-0.342*cb) );		
		TQI444FLOAT_TO_INT(b, (1.772*cb) );

        tqi444_yuvlutcb[(index+128) & 0x1ff] = ((g & 0x1ff) << 11) + (b & 0x1ff);

        cr = (float) val;
        TQI444FLOAT_TO_INT(r, (1.402*cr) );
        TQI444FLOAT_TO_INT(g, ( -0.711*cr ));

        tqi444_yuvlutcr[(index+128) & 0x1ff ] = ((r & 0x1ff) << 23) + ((g & 0x1ff) << 11);
    }

    for( i = -256 ; i < 256 ; ++i )
    {   val = i + 128;
        if( val < 0 ) val = 0;
        else if( val > 255 ) val = 255;
        v1 = val >> (8-pixformat.Rbits);
        tqi444_yuvlutr[256+i] =  (v1 << pixformat.Rshifts);

    }

    for( i = -256 ; i < 256 ; ++i )
    {
        val = i + 128;
        if( val < 0 ) val = 0;
        else if( val > 255 ) val = 255;
        v1 = val >> (8-pixformat.Gbits);  
        tqi444_yuvlutg[256+i] =  (v1 << pixformat.Gshifts);
    }

    for( i = -256 ; i < 256 ; ++i )
    {
        val = i + 128;

        if( val < 0 ) val = 0;
        else if( val > 255 ) val = 255;
        v1 = val >> (8-pixformat.Bbits);
        tqi444_yuvlutb[256+i] =  (v1 << pixformat.Bshifts);
    }
#endif

}



static int Countbits(int d)
{	int i,c=0;
	for (i=0;i<32;i++)
		if (d & (1<<i))	c++;
	return c;
}

static int Countshifts(int d)
{	int c=0;
	if (d)
	{	while (!(d&1))
		{	d>>=1;
			c++;
		} 
	}
	return c;
}

static void make_quant(void)
{	//make_prescale();
	for( int i = 0 ; i < 8 ; ++i )
	{	for( int j = 0 ; j < 8 ; ++j )
		{	tqi444_Quant[8*i+j] = fixedmult(8 << 16, idctPrescale[8*i+j] );
			//tqi444_Quant[8*i+j] = idctPrescale[8*i+j];
		}
	}
}

void tqi444_set_dither_mode (DitherMode dither_mode)
{	unsigned int old_dither_mode = tqi444_dither_mode;
	tqi444_dither_mode = (unsigned int) dither_mode;
	if (tqi444_dither_mode != old_dither_mode)
		make_clip();

}

void tqi444_set_pixeldecode_format (unsigned int dwRBitMask,unsigned int dwGBitMask,unsigned int dwBBitMask,unsigned int dwABitMask)
{
	if (display_init == 0)
	{	
		make_rand();
		make_quant();
		
	}
	if ((display_init == 0) || (pixformat.dwRBitMask != dwRBitMask) || (pixformat.dwGBitMask != dwGBitMask) || (pixformat.dwGBitMask != dwGBitMask))
	{	pixformat.AlphaMask = dwABitMask;
		pixformat.Abits = Countbits(dwABitMask);
		pixformat.Rbits = Countbits(dwRBitMask);
		pixformat.Gbits = Countbits(dwGBitMask);
		pixformat.Bbits = Countbits(dwBBitMask);
		pixformat.Ashifts = Countshifts(dwABitMask);
		pixformat.Rshifts = Countshifts(dwRBitMask);
		pixformat.Gshifts = Countshifts(dwGBitMask);
		pixformat.Bshifts = Countshifts(dwBBitMask);
		if ( (pixformat.Rbits+pixformat.Gbits+pixformat.Bbits) > 16) 
			pixformat.BitsPerPixel = 32;
		else
			pixformat.BitsPerPixel = 16;

		tqi444_Alpha_Mask = pixformat.AlphaMask; 
		tqi444_Red_Mask   = ((255<<24) << pixformat.Rbits)&(255<<24);
		tqi444_Green_Mask   = ((255<<24) << pixformat.Gbits)&(255<<24);
		tqi444_Blue_Mask   = ((255<<24) << pixformat.Bbits)&(255<<24);
	
		tqi444_Alpha_Shift = pixformat.Ashifts;
		tqi444_Red_Shift   = pixformat.Rshifts;
		tqi444_Green_Shift = pixformat.Gshifts;
		tqi444_Blue_Shift  = pixformat.Bshifts;

		tqi444_Alpha_Slide = ( 8 - pixformat.Abits);
		tqi444_Red_Slide =   ( 8 - pixformat.Rbits);
		tqi444_Green_Slide = ( 8 - pixformat.Gbits);
		tqi444_Blue_Slide =  ( 8 - pixformat.Bbits);

		tqi444_Alpha_Upper = 	255 >> ( tqi444_Alpha_Slide );	
		tqi444_Red_Upper   = 	255 >> ( tqi444_Red_Slide );	
		tqi444_Green_Upper = 	255 >> ( tqi444_Green_Slide );
		tqi444_Blue_Upper  = 	255 >> ( tqi444_Blue_Slide );
		

		make_clip();


	}
	display_init = 1;

}





static void tqi444_set_colorkey (unsigned int red,unsigned int green,unsigned int blue)
{	
	tqi444_ColorKey32 = tqi444_ColorKey16 = ((red >> tqi444_Red_Slide) << tqi444_Red_Shift) |
					 ((green >> tqi444_Green_Slide) << tqi444_Green_Shift) |
					((blue >> tqi444_Blue_Slide)); 
	tqi444_ColorKey16 |= (tqi444_ColorKey16<<16);

}




/*--------------------------------------------------*/



//	Decode all macroblocks of the current picture (16bpp)

static void tqi444_decode_image_16bpp(unsigned char *src, unsigned short *dst,unsigned int stride,unsigned int real_width,unsigned int real_height)
{	unsigned int width,height;
	unsigned short *dst2 = dst;
	tqi444_hasALPHA =  (src[4] >> 7);
	tqi444_hasCOLOURKEY =  (src[4] &1) ^ 1;
	tqi444_is_YUV = (src[4] >> 4) & 1;	

	tqi444_initdecode(src+8);
	width =  ((real_width)+7)   & 0xfffffff8;
	height = ((real_height)+7) & 0xfffffff8;
	
	unsigned int seed = (((int) dst)^123) & 255;

	for( unsigned int y = 0 ; y < height ; y += 8 )
	{	
		unsigned int tbheight,tbwidth;
		for (unsigned int  i=0 ; i < 8;i++)
		{	tqi444_TAlpha_Lower[i] = ((unsigned int)rand_table[seed]<<24) & tqi444_Alpha_Mask;
			tqi444_TRed_Lower[i] = ((unsigned int)rand_table[seed+1]<<24) & tqi444_Red_Mask;
			tqi444_TGreen_Lower[i] = ((unsigned int)rand_table[seed+2]<<24) & tqi444_Green_Mask;
			tqi444_TBlue_Lower[i] = ((unsigned int)rand_table[seed+3]<<24) & tqi444_Blue_Mask;
			seed += 5;
			seed &= 255;

		}
		if ((y+8) > real_height)
			tbheight = real_height - y;
		else
			tbheight = 8;
		for(unsigned int  x = 0 ; x < width ; x += 8 )
		{	if ((x+8) > real_width)
				tbwidth = real_width - x;
			else
				tbwidth = 8;
			if (tqi444_is_YUV)
				tqi444_decodeblock_yuv((unsigned short *) (dst2+x),stride,tbwidth,tbheight);
			else
				tqi444_decodeblock((unsigned short *) (dst2+x),stride,tbwidth,tbheight);

		}
		dst2 += (stride<<2);

	}




}

static void tqi444_decode_image_32bpp(unsigned char *src, unsigned int *dst,unsigned int stride,unsigned int real_width,unsigned int real_height)
{	unsigned int width,height;
	tqi444_initdecode(src+8);
	width =  ((real_width)+7)   & 0xfffffff8;
	height = ((real_height)+7) & 0xfffffff8;
	tqi444_hasALPHA =  (src[4] >> 7) ;
	tqi444_hasCOLOURKEY =  (src[4] &1) ^ 1;
	tqi444_is_YUV = (src[4] >> 4) & 1;	

	unsigned int seed = rand() & 255;

	for( unsigned int y = 0 ; y < height ; y += 8 )
	{	
		unsigned int tbheight,tbwidth;	
		
		if ((y+8) > real_height)
			tbheight = real_height - y;
		else
			tbheight = 8;
		for(unsigned int  x = 0 ; x < width ; x += 8 )
		{	if ((x+8) > real_width)
				tbwidth = real_width - x;
			else
				tbwidth = 8;

			if (tqi444_is_YUV)
				tqi444_decodeblock32_yuv((unsigned short *) (dst+x),stride,tbwidth,tbheight);
			else
				tqi444_decodeblock32((unsigned short *) (dst+x),stride,tbwidth,tbheight);

		}
		dst += (stride<<1);

	}




}

static unsigned short PrevFPURoundingFlags;

static void SetFPURoundingFlags(void)

{	
	/*unsigned short flags;
	_asm
	{	fnstcw word ptr flags;
	}
	flags &=~((3<<8)|(3<<10));
	flags |=(3<<8);
	_asm
	{	fldcw flags;
	}
	*/
}



static void __inline SaveFPURoundingFlags(void)
{
	/*unsigned short flags;
	_asm
	{	fnstcw word ptr flags;
	}
	PrevFPURoundingFlags=flags;
	*/
}


static void __inline RestoreFPURoundingFlags(void)
{
/*	unsigned short flags=PrevFPURoundingFlags;
	_asm
	{
		fldcw flags;
	}
	*/
}

//
// Decode a pic, creating a direct-draw surface which is returned.
//

int tqi444_decode_image (unsigned char *src,unsigned char * dst,unsigned int stride, int real_width, int real_height)
{	
	//SaveFPURoundingFlags();
	//SetFPURoundingFlags();

	tqi444_set_colorkey ((unsigned int) src[5],(unsigned int)src[6],(unsigned int)src[7]);

	if (pixformat.BitsPerPixel == 16)
		tqi444_decode_image_16bpp(src,(unsigned short *)dst,stride,real_width,real_height);
	else
		tqi444_decode_image_32bpp(src,(unsigned int *)dst,stride,real_width,real_height);

	//RestoreFPURoundingFlags();
	return 0;
}

int tqi444_decode_header (unsigned char *compressed_image_data, int *real_width, int *real_height)
{
	*real_width =  *((short*)compressed_image_data);
   	*real_height = *((short*)compressed_image_data+1);

	return 0;

}

