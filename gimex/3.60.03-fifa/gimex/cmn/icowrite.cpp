// (c) 1994-2007 Electronic Arts Inc.

#include <string.h>
#include "gimex.h"
#include "icogimex.h"


/****************************************************************/
/*  Internal Functions                                          */
/****************************************************************/

//------------------------------------------------------------------------------------
// this builds a standard WIN9x 16 Colour palette. 
GRGBQUAD * Build16ColorPalette()
{
    int intensity[] = {128,255};  // I want to generate with these intensities.
    int i = 0;
    int j = 0;
    int k = 0;
    int m = 0;
    GRGBQUAD * P = (GRGBQUAD *)galloc(sizeof(GRGBQUAD)*16);
    memset(P,0,sizeof(GRGBQUAD)*16);
    for(m = 0; m < 2;m++)
    {
        for(i = 0; i < 2; i++)
        {
            for(j = 0; j < 2; j++)
            {
                for(k = 0; k < 2; k++)
                {
                    P[m*8+i*4+j*2+k].rgbRed   = (char)(intensity[m]*i);
                    P[m*8+i*4+j*2+k].rgbGreen = (char)(intensity[m]*j);  
                    P[m*8+i*4+j*2+k].rgbBlue  = (char)(intensity[m]*k);
                }
            }
        }
    }
    P[8].rgbRed = P[8].rgbGreen = P[8].rgbBlue = (char)192;  // set the middle color to be light gray not black.

    return(P);
}

//------------------------------------------------------------------------------------
// general purpose, write my directory header.
void SetupAndWriteHeader(GICONDIR *WinHeader,GINSTANCE *gx,const GINFO *ginfo,int frames)
{
   int subtype = 0;
   gseek(gx->gstream,(GPOS)0);
   memset(WinHeader,0,sizeof(GICONDIR));               
   ICOPUTFIELD(WinHeader->idReserved,0);               // MUST be zero
   if(ginfo->subtype == (GICO_TYPEICON-1) || (ginfo->subtype == GICO_TYPECURS-1))
       subtype = ginfo->subtype+1;                     // the funny stuff with the 1 is to go from 0/1 (gimex subytype format) to 1/2 ico/cur format.
   else
       subtype = GICO_TYPEICON;                        // Default to icon type.
   ICOPUTFIELD(WinHeader->idType,subtype);             // Type 1 is icon.
   ICOPUTFIELD(WinHeader->idCount,frames);             // count of number of frames in icon.
   gwrite(gx->gstream,WinHeader,sizeof(GICONDIR));
   return;
}

//------------------------------------------------------------------------------------
void SetupAndWriteColorDirEntry(GICONDIRENTRY *DirEntry,const GINFO *ginfo,GINSTANCE *gx,int colors,GICONINTERNAL *gint)
{
    int i = 0;
    //int masksize = 128;                                  // current mask size is 128 bytes of zeros.
    int divider  = 2;
    int numCLUTentries = (colors==255)?256:16; 
    unsigned int imagebytes   = 0;
    unsigned int XORmaskbytes = 0;
    if(colors ==0 || colors == 255)                      // am I doing 4bpp or 8bpp.
        divider = 1;
    else
        divider = 2;
    memset(DirEntry,0,sizeof(GICONDIRENTRY));
    ICOPUTFIELD(DirEntry->bWidth,ginfo->width);
    ICOPUTFIELD(DirEntry->bHeight,ginfo->height);
    ICOPUTFIELD(DirEntry->bColorCount,colors);           // number of colours in resource
    ICOPUTFIELD(DirEntry->bReserved,0);                  // MUST be zero
    // do I need to save hotspot information?
    if(ginfo->subtype == (GICO_TYPECURS-1))
    {
        ICOPUTFIELD(DirEntry->wPlanes,ginfo->hotspottbl[0][0]);          // save hotspot information here, windows doesn't document this for some reason.
        ICOPUTFIELD(DirEntry->wBitCount,ginfo->hotspottbl[0][1]);                    
    }
    else
    {
        ICOPUTFIELD(DirEntry->wPlanes,0);                    // number of planes in image.
        ICOPUTFIELD(DirEntry->wBitCount,0);                  // just set to zero, does not seem to be used.
    }
    imagebytes   = (ginfo->width*ginfo->height+(divider/2))/divider;  // 4bpp or 8bpp round up to nearest byte and count the bytes.
    if(ginfo->width <=16)
		XORmaskbytes = ((ginfo->height*ginfo->width +4) /8)*2;            // note two colour masks.
	else 
		XORmaskbytes = ((ginfo->height*ginfo->width +4) /8);            // note two colour masks.
#ifdef DEBUG
//    printf("img: %d, XOR: %d, Bit: %d, Clut: %d DirEnt: %d\n",imagebytes, XORmaskbytes,sizeof(GBITMAPINFOHEADER),sizeof(GRGBQUAD)*numCLUTentries,sizeof(GICONDIRENTRY));
#endif
    ICOPUTFIELD(DirEntry->dwBytesInRes,(imagebytes + sizeof(GBITMAPINFOHEADER) + sizeof(GRGBQUAD)*numCLUTentries + XORmaskbytes)); // must include the image header & palette.
    ICOPUTFIELD(DirEntry->dwImageOffset,gint->NextOffsetInFile);    // offset of the image in the file.
    if(gint->NumImagesWritten == 0)                                 // reserve space in file for other direntries.
    {
        for(i = 0; i < gint->NumImagesTotal; i++)
        {
            gwrite(gx->gstream,DirEntry,sizeof(GICONDIRENTRY));
        }
    }
    else
    {
        gseek(gx->gstream,(GPOS)(sizeof(GICONDIR)+sizeof(GICONDIRENTRY)*gint->NumImagesWritten));  // seek to the correct direntry over icon header + # direntries.
        gwrite(gx->gstream,DirEntry,sizeof(GICONDIRENTRY));
    }
}


//------------------------------------------------------------------------------------
void SetupAndWrite16ColorImage(const GINFO *ginfo,char *source,int ImageOffset,GINSTANCE *gx)
{
   GBITMAPINFOHEADER Bi;    // the only part of the ImageHeader I can set and write other stuff has to happen dynamically.
   GRGBQUAD *P;             // my 16 color palette.
   int i,j,m;                 // my rc loopers
   char *rowbuf = 0;
   char *R      = 0;        // below. write from bottom up from left to right so position at bottom left of source.
   char *s      = source + ginfo->width*ginfo->height - ginfo->width;
   char t       = 0;
   // write my bitmap info header.
   ICOPUTFIELD(Bi.biSize,sizeof(GBITMAPINFOHEADER));
   ICOPUTFIELD(Bi.biWidth,ginfo->width);
   ICOPUTFIELD(Bi.biHeight,(ginfo->height*2));                       // this would be negative for top down.
   ICOPUTFIELD(Bi.biPlanes,1);
   ICOPUTFIELD(Bi.biBitCount,4);                                     // bit count for 16 color image.
   ICOPUTFIELD(Bi.biCompression,0);                                  // not suported in icons.
   ICOPUTFIELD(Bi.biSizeImage,(ginfo->width*ginfo->height+1)/2);     // size of the image in bytes.
   ICOPUTFIELD(Bi.biXPelsPerMeter,0);                                // n/a
   ICOPUTFIELD(Bi.biYPelsPerMeter,0);                                // n/a
   ICOPUTFIELD(Bi.biClrUsed,0);                                      // n/a
   ICOPUTFIELD(Bi.biClrImportant,0);                                 // n/a
   gseek(gx->gstream,(GPOS) 0);
   gseek(gx->gstream,ImageOffset);
   gwrite(gx->gstream,&Bi,sizeof(GBITMAPINFOHEADER));

   P = (GRGBQUAD*)galloc(sizeof(GRGBQUAD)*16);                       // get and write my palette.
   for(i = 0; i < 16; i++)
   {
        P[i].rgbRed   = ginfo->colourtbl[i].r;                       // grab the palette from gimex and write it.
        P[i].rgbGreen = ginfo->colourtbl[i].g;
        P[i].rgbBlue  = ginfo->colourtbl[i].b;
   }
   gwrite(gx->gstream,P,sizeof(GRGBQUAD)*16);
   gfree(P);
   rowbuf = (char *)galloc((ginfo->width+1)/2);                      // for each row.
   for(i = 0; i < ginfo->height;i++)
   {
        memset(rowbuf,0,(ginfo->width+1)/2);
        R = rowbuf;
        for(j = 0; j < ginfo->width;j+=2)                            // doing two pixels at a time.
        {
           *R = 0;
           *R |= ((*s)&15)<<4;                                       // compact into 4bpp
           s++;
           *R |= ((*s)&15);
           s++;
           R++;
        }
        gwrite(gx->gstream,rowbuf,(ginfo->width+1)/2);               // write my rowbuffer.
        s-= ginfo->width*2;                                          // write from bottom up with start at left hand side.
   }
   if(rowbuf)
    gfree(rowbuf);
   // write out and and or masks.
    s = source + ginfo->width*ginfo->height - ginfo->width;
    t = 0;
	if(ginfo->width <= 16)
	{
		int count = 0;
        int current = 0;
        char andscanline[32];
        char orscanline[32];
        for(i = 0; i < ginfo->height; i++)
        {
            count = 0;
            memset(andscanline,0,32);
            memset(orscanline,0,32);
            for(j = 0; j < ginfo->width;j++)
            {
                current = count/8;
                andscanline[current] |= (((ginfo->colourtbl[(*s)].a < 32)?1:0)<<(count%8));
                orscanline[current]  |= (((ginfo->colourtbl[(*s)].a < 32)?0:1)<<(count%8));
                s++;
                count++;
            }
            s -= ginfo->width*2;     // go back two rows.
            for(m = 0; m < (current + 1)>>1;m++)
			{
				gwrite(gx->gstream,&andscanline[m+1],1);
				gwrite(gx->gstream,&andscanline[m],1);
			}
            for(m = 0; m < (current + 1)>>1;m++)
			{
				gwrite(gx->gstream,&orscanline[m+1],1);
				gwrite(gx->gstream,&orscanline[m],1);

			}

        }
    }
    else    
	{                                                      // but windows needs this.
        int count = 0;
        int current = 0;
        char andscanline[32];
        char orscanline[32];
        for(i = 0; i < ginfo->height; i++)
        {
            count = 0;
            memset(andscanline,0,32);
            memset(orscanline,0,32);
			for(j = 0;j < ginfo->width;j++)
			{
				current = count/8;
				andscanline[current] |= (((ginfo->colourtbl[(*s)].a < 96)?1:0)<<(7 - (count&0x07)));
				//orscanline[current]  |= (((ginfo->colourtbl[(*s)].a < 96)?1:0)<<(7 - (count&0x07)));
				s++;
				count++;
			}
            gwrite(gx->gstream,andscanline,(ginfo->width+4)/8);
	        //gwrite(gx->gstream,andscanline,(ginfo->width+4)/8);
            s -= (ginfo->width*2);     // go back two rows.
        }
    }



}

//------------------------------------------------------------------------------------
void SetupAndWriteMonoColorImage(const GINFO *ginfo,char *source,int ImageOffset,GINSTANCE *gx)
{
   GBITMAPINFOHEADER Bi;    // the only part of the ImageHeader I can set and write other stuff has to happen dynamically.
   GRGBQUAD *P;             // my 16 color palette.
   int i,j,k;               // my rc loopers
   char *pImage = 0;
   char *pXorBu = 0;
   char *R      = 0;        // below. write from bottom up from left to right so position at bottom left of source.
   char *X      = 0;
   char *s      = source + ginfo->width*ginfo->height - ginfo->width;
   // write my bitmap info header.
   ICOPUTFIELD(Bi.biSize,sizeof(GBITMAPINFOHEADER));
   ICOPUTFIELD(Bi.biWidth,ginfo->width);
   ICOPUTFIELD(Bi.biHeight,(ginfo->height*2));                         // this would be negative for top down.
   ICOPUTFIELD(Bi.biPlanes,1);
   ICOPUTFIELD(Bi.biBitCount,1);                                     // bit count for 16 color image.
   ICOPUTFIELD(Bi.biCompression,0);                                  // not suported in icons.
   ICOPUTFIELD(Bi.biSizeImage,(ginfo->width*ginfo->height));       // size of the image in bytes.
   ICOPUTFIELD(Bi.biXPelsPerMeter,0);                                // n/a
   ICOPUTFIELD(Bi.biYPelsPerMeter,0);                                // n/a
   ICOPUTFIELD(Bi.biClrUsed,0);                                      // n/a
   ICOPUTFIELD(Bi.biClrImportant,0);                                 // n/a
   gseek(gx->gstream,(GPOS) 0);
   gseek(gx->gstream,ImageOffset);
   gwrite(gx->gstream,&Bi,sizeof(GBITMAPINFOHEADER));

   P = (GRGBQUAD*)galloc(sizeof(GRGBQUAD)*2);                        // get and write my palette.
   for(i = 0; i < 2; i++)
   {
        P[i].rgbRed   = ginfo->colourtbl[i].r;                       // grab the palette from gimex and write it.
        P[i].rgbGreen = ginfo->colourtbl[i].g;
        P[i].rgbBlue  = ginfo->colourtbl[i].b;
   }
   gwrite(gx->gstream,P,sizeof(GRGBQUAD)*2);
   gfree(P);
   pImage = (char *)galloc(ginfo->width/8*ginfo->height);           // do the entire image at once.
   pXorBu = (char *)galloc(ginfo->width/8*ginfo->height);           // for the xor mask.
   memset(pImage,0,ginfo->width/8*ginfo->height);
   memset(pXorBu,0,ginfo->width/8*ginfo->height);
   R = pImage;
   X = pXorBu;
   for(i = 0; i < ginfo->height;i++)
   {
        
        
        for(j = 0; j < ginfo->width;j+=8)                            // doing two pixels at a time.
        {
            *R = 0;
            *X = 0;
            for(k = 7; k >= 0; k--)
            {
                *R |= ((*s)&1)<<k;                                   // compact into 4bpp
                if(ginfo->colourtbl[*s].a < 64)
                    *X |= 1<<k;
                s++;
            }
            R++;
            X++;
        }
        s-= ginfo->width*2;                                          // write from bottom up with start at left hand side.
   }
   gwrite(gx->gstream,pImage,ginfo->width/8*ginfo->height);          // write my rowbuffer.
   gwrite(gx->gstream,pXorBu,ginfo->width/8*ginfo->height);          // write my two xor and and masks.
   memset(pXorBu,0,ginfo->width/8*ginfo->height);
   gwrite(gx->gstream,pXorBu,ginfo->width/8*ginfo->height);          // write out second buffer just to make sure we have both AND and XOR masks.

   if(pImage)
       gfree(pImage);
   if(pXorBu)
       gfree(pXorBu);
   

}


//------------------------------------------------------------------------------------
// write an image out with a 256 colour palette.
void SetupAndWrite256ColorImage(const GINFO *ginfo,char *source,int ImageOffset,GINSTANCE *gx,int bpp)
{
   GBITMAPINFOHEADER Bi;    // the only part of the ImageHeader I can set and write other stuff has to happen dynamically.
   GRGBQUAD *P;             // my 16 color palette.
   int i;                   // my rc loopers
   int j;
   unsigned char *s      = (unsigned char *)(source + ginfo->width*ginfo->height - ginfo->width); // see 16 colour write.
   char t = 0;
   // write my header.
   ICOPUTFIELD(Bi.biSize,sizeof(GBITMAPINFOHEADER));
   ICOPUTFIELD(Bi.biWidth,ginfo->width);
   ICOPUTFIELD(Bi.biHeight,(ginfo->height*2));                        // minus is to inform windows this is a top down DIB
   ICOPUTFIELD(Bi.biPlanes,1);
   ICOPUTFIELD(Bi.biBitCount,bpp);
   ICOPUTFIELD(Bi.biCompression,0);                                   // compression is not supported in icons.
   ICOPUTFIELD(Bi.biSizeImage,ginfo->width*ginfo->height/8);
   ICOPUTFIELD(Bi.biXPelsPerMeter,0);
   ICOPUTFIELD(Bi.biYPelsPerMeter,0);
   ICOPUTFIELD(Bi.biClrUsed,256);
   ICOPUTFIELD(Bi.biClrImportant,256);
   gseek(gx->gstream,(GPOS) 0);
   gseek(gx->gstream,ImageOffset);
   gwrite(gx->gstream,&Bi,sizeof(GBITMAPINFOHEADER));

   P = (GRGBQUAD*)galloc(sizeof(GRGBQUAD)*256);                       // get my colour palette and write it out.
   memset(P,0,sizeof(GRGBQUAD)*256);
   for(i = 0; i < 256; i++)
   {
        P[i].rgbRed   = ginfo->colourtbl[i].r;
        P[i].rgbGreen = ginfo->colourtbl[i].g;
        P[i].rgbBlue  = ginfo->colourtbl[i].b;
   }
   gwrite(gx->gstream,P,sizeof(GRGBQUAD)*256);
   gfree(P);
   for(i = 0; i < ginfo->height;i++)                                  // write as a bottom up DIB since windows seems to not like top_downs
   {
      gwrite(gx->gstream,s,ginfo->width);
      s -= ginfo->width;
   }
   s      = (unsigned char *)(source + ginfo->width*ginfo->height - ginfo->width); // see 16 colour write.
   t = 0;

    if(ginfo->width <= 16)
    {
        int count = 0;
        int current = 0;
        char andscanline[32];
        char orscanline[32];
        for(i = 0; i < ginfo->height; i++)
        {
            count = 0;
            memset(andscanline,0,32);
            memset(orscanline,0,32);
            for(j = 0; j < ginfo->width;j++)
            {
                current = count/8;
                andscanline[current] |= (((ginfo->colourtbl[(*s)].a < 96)?1:0)<<(count%8));
                orscanline[current]  |= (((ginfo->colourtbl[(*s)].a < 96)?0:1)<<(count%8));
                s++;
                count++;
            }
            s -= ginfo->width*2;     // go back two rows.
			if(current == 1)
				gwrite(gx->gstream,andscanline+1,1);
			gwrite(gx->gstream,andscanline,1);
			if(current == 1)					// note endian issue.
				gwrite(gx->gstream,orscanline+1,1);
			gwrite(gx->gstream,orscanline,1);                      
        }
    }
    else    
	{                                                      // but windows needs this.
        int count = 0;
        int current = 0;
        char andscanline[32];
        char orscanline[32];
        for(i = 0; i < ginfo->height; i++)
        {
            count = 0;
            memset(andscanline,0,32);
            memset(orscanline,0,32);
			for(j = 0;j < ginfo->width;j++)
			{
				current = count/8;
				andscanline[current] |= (((ginfo->colourtbl[(*s)].a < 96)?1:0)<<(7 - (count&0x07)));
				//orscanline[current]  |= (((ginfo->colourtbl[(*s)].a < 96)?1:0)<<(7 - (count&0x07)));
				s++;
				count++;
			}
            gwrite(gx->gstream,andscanline,(ginfo->width+4)/8);
	        //gwrite(gx->gstream,andscanline,(ginfo->width+4)/8);
            s -= (ginfo->width*2);     // go back two rows.
        }
    }

    t = 0;
	for(i = 0; i < ((ginfo->width*ginfo->height + 4)/8);i++)
		gwrite(gx->gstream,&t,1);


}

//------------------------------------------------------------------------------------
// helper routines for the drop to 16 colours from 32 bpp... UGLY I know but necessary.
#define FAKEDITHER(v) (v < 66?0:(v < 192?128:255))
#define BUMPDOWN(v,m,x) ((v == 255) ? ((m == 128 && x == 128)?128:((m==255 && x<255)||(m<255&& x==255)?255:v)):v)


//------------------------------------------------------------------------------------
// this is for 32 bit images.. I chose to dither them down to 16 colors since
// that was really simple and 32 bit icons don't really make sense anyways...
// anyone that tries it will get a nasty surprise.. This uses a 16 color
// generic windows palette.
void SetupAndWrite32BPPDownto16ColorImage(const GINFO *ginfo,char *source,int ImageOffset,GINSTANCE *gx)
{
	GBITMAPINFOHEADER Bi;    // the only part of the ImageHeader I can set and write other stuff has to happen dynamically.
	GRGBQUAD *P;             // my 16 color palette.
	int i,j;                 // my rc loopers
	unsigned int r,g,b;      // red green blue working values.
	unsigned int ro,go,bo;   // color channel on or off flags.
	int index;               // index into palette.
	int high;                // high intensity mode or low intensity.
	char *rowbuf = 0;
	char *R      = 0;        // bottom up left to right set s to correct position.
	char *s      = source + ginfo->width*ginfo->height*4 - ginfo->width*4;
	char t       = 0;
	// write my header.
	ICOPUTFIELD(Bi.biSize,sizeof(GBITMAPINFOHEADER));
	ICOPUTFIELD(Bi.biWidth,ginfo->width);
	ICOPUTFIELD(Bi.biHeight,(ginfo->height*2));
	ICOPUTFIELD(Bi.biPlanes,1);
	ICOPUTFIELD(Bi.biBitCount,4);
	ICOPUTFIELD(Bi.biCompression,0);
	ICOPUTFIELD(Bi.biSizeImage,(ginfo->width*ginfo->height+1)/2);
	ICOPUTFIELD(Bi.biXPelsPerMeter,0);
	ICOPUTFIELD(Bi.biYPelsPerMeter,0);
	ICOPUTFIELD(Bi.biClrUsed,0);
	ICOPUTFIELD(Bi.biClrImportant,0);
	gseek(gx->gstream,(GPOS) 0);
	gseek(gx->gstream,ImageOffset);
	gwrite(gx->gstream,&Bi,sizeof(GBITMAPINFOHEADER));

	// Now write my palette.
	P = Build16ColorPalette();                  // generate windows standard palette.
	gwrite(gx->gstream,P,sizeof(GRGBQUAD)*16);
	gfree(P);
	rowbuf = (char *)galloc((ginfo->width+1)/2);

	// now start to cheat dither the image.
	for(i = 0; i < ginfo->height;i++)
	{
        memset(rowbuf,0,(ginfo->width+1)/2);
        R = rowbuf;
        for(j = 0; j < ginfo->width;j+=2)      // doing two pixels at a time.
        {
            *R   = 0;                          // clear the current item for oring.
            r    = ((ARGB *)s)->r;             // get my colour values.
            g    = ((ARGB *)s)->g;
            b    = ((ARGB *)s)->b;

            r    = FAKEDITHER(r);              // clamp them at their respective depths.
            g    = FAKEDITHER(g);
            b    = FAKEDITHER(b);
            
            ro   = BUMPDOWN(r,g,b);            // bump them down to low intensity mode if necessary.
            go   = BUMPDOWN(g,r,b);
            bo   = BUMPDOWN(b,r,g);
            high = ro==255||go==255||bo==255;  // are we high intensity?
            
            ro   = ro>0;                       // is this channel on or off?
            go   = go>0;
            bo   = bo>0;
            index = high*8 + ro*4 + go*2 + bo; // calculate palette index.
            
            *R   |= (index&15)<<4;
            s    +=4;                          // get next pixel since we are using 4bpp.

            r    = ((ARGB *)s)->r;
            g    = ((ARGB *)s)->g;
            b    = ((ARGB *)s)->b;
            
            r    = FAKEDITHER(r);              // fake dither again.
            g    = FAKEDITHER(g);
            b    = FAKEDITHER(b);
            ro   = BUMPDOWN(r,g,b);            // bump into low intensity mode again.
            go   = BUMPDOWN(g,r,b);
            bo   = BUMPDOWN(b,r,g);
            high = ro==255||go==255||bo==255;  // are we high intensity?
            ro   = ro>0;                       // is this channel on or off?
            go   = go>0;
            bo   = bo>0;
            index = high*8 + ro*4 + go*2 + bo; // calculate palette index.

            *R   |= (index&15);
            s    +=4;
            R++;
        }
        gwrite(gx->gstream,rowbuf,((ginfo->width)/2));
        s -= ginfo->width*8;                   // write out bottom to top so go up to next row.
        
    }
    if(rowbuf)
		gfree(rowbuf);
    // write out and and or masks.
    s = source + ginfo->width*ginfo->height*4 - ginfo->width*4;
    {
        int count = 0;
        int rowbytes = ginfo->width*4;
        for(i = 0; i < ((ginfo->height*ginfo->width +4) /8); i++)
        {
            t = 0;
            for(j = 7; j >= 0;j--)
            {
                if(count == rowbytes)
                {
                    s -= ginfo->width*8;     // go back two rows.
                    count = 0;
                }
                else
                    count++;
                t |= (((((ARGB *)s)->a < 32)?1:0)<<j);
                s += 4;
             }
             gwrite(gx->gstream,&t,1);
        }
    }
}

#undef FAKEDITHER
#undef BUMPDOWN

/****************************************************************/
/*  export Functions                                            */
/****************************************************************/

//------------------------------------------------------------------------------------
int GCALL ICO_wopen(GINSTANCE **gxptr, GSTREAM *g, const char *pathname, int numframes)
{
    GINSTANCE *gx;
    int ok=0;
    GICONINTERNAL * gint = 0;
    gx   = (GINSTANCE *)galloc(sizeof(GINSTANCE));
    gint = (GICONINTERNAL *)galloc(sizeof(GICONINTERNAL));
    if (gx)
    {
        gint->NumImagesWritten = 0;
        gint->NumImagesTotal   = numframes;
        gint->NextOffsetInFile = sizeof(GICONDIR) + sizeof(GICONDIRENTRY)*numframes; 

        gx->frames= numframes;
        gx->gstream = g;
        gx->gref = gint;
        *gxptr = gx;
        ok = 1;
    }
    return ok;
}

//------------------------------------------------------------------------------------
int GCALL ICO_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes)
{
    GICONDIR WinHeader;
    GICONDIRENTRY DirEntry;
    GICONINTERNAL * gint = (GICONINTERNAL *)gx->gref;
    //int ImageOffset = sizeof(GICONDIR) + sizeof(GICONDIRENTRY)*gx->frames;
    gseek(gx->gstream, 0);

    memset(&WinHeader, 0, sizeof(GICONDIR));

    // icons only support 2X2 -> 254X254 resolutions.
    if (ginfo->width > 254 || ginfo->height > 254 || ginfo->width < 2 || ginfo->height < 2)
    {
#ifdef DEBUG
//        printf("Error: Icons range between 2X2 -> 254X254, please resize your image\n");
#endif
        return 0;
    }
    
	if (ginfo->bpp<=8)
    {
        
        if(gint->NumImagesWritten == 0)  // only write file header if needed.
            SetupAndWriteHeader(&WinHeader,gx,ginfo,gx->frames);

        if (ginfo->originalbpp==1)
        {
            SetupAndWriteColorDirEntry(&DirEntry,ginfo,gx,0,gint);              // monochrome images do not store colour count in direntry.
            SetupAndWriteMonoColorImage(ginfo,source,gint->NextOffsetInFile,gx);
        }
        else if (ginfo->originalbpp<=4)
        {
            SetupAndWriteColorDirEntry(&DirEntry,ginfo,gx,16,gint);
            SetupAndWrite16ColorImage(ginfo,source,gint->NextOffsetInFile,gx);
        }
        else // 8 bpp
        {
            SetupAndWriteColorDirEntry(&DirEntry,ginfo,gx,255,gint);  // set to zero flaging 256
            SetupAndWrite256ColorImage(ginfo,source,gint->NextOffsetInFile,gx,8);
        }
    }
	else // Image is 32bpp Dropping image colour count to 16 color indexed
    {
        if(gint->NumImagesWritten == 0)    // only write header if this is the first image.
            SetupAndWriteHeader(&WinHeader,gx,ginfo,gx->frames);
        SetupAndWriteColorDirEntry(&DirEntry,ginfo,gx,16,gint);
        SetupAndWrite32BPPDownto16ColorImage(ginfo,source,gint->NextOffsetInFile,gx);    
    }
    gint->NumImagesWritten++;
    gint->NextOffsetInFile = (int)gtell(gx->gstream);
    return 1;
}


//------------------------------------------------------------------------------------
int GCALL ICO_wclose(GINSTANCE *gx)
{
    int ok=0;

    if (gx)
    {
        if(gx->gref)
            gfree(gx->gref);
        gfree(gx);
        ok = 1;
    }
    return ok;
}

#undef GICO_TYPECURS
#undef GICO_TYPEICON
#undef ICOGETFIELD
#undef ICOPUTFIELD
