// (c) 1998-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*               ICO - Windows Icons/Cursors Gimex                  */
/*                                                                  */
/*                   by Ian H. Davids, EAC                          */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History:                                       */
/* ------- ----   -- --------                                       */
/*    1.00 010228 ID Original                                       */
/*    1.01 010312 ID fixed monochrome images                        */
/*    1.02 010319 ID enhanced is function to be semi foolproof      */
/*    1.03 011031 FB ported to unix                                 */
/*    1.04 031209 FB indexed 2,3,5,6,7 bump bpp properly            */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 4 bit or more                                                    */
/* Reentrant                                                        */
/*                                                                  */
/* Files: icoread.cpp icowrite.cpp icogimex.h                       */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Format Notes:                                                    */
/* -------------                                                    */
/*  Check MSDN for formatting information. NOTE windows does NOT    */
/*  like TOP_DOWN DIB's so most of my DIB's are bottom up           */
/*  This reader supports both bottom up and top down DIB's.         */
/*  but only writes bottom up since windows cannot seem to interpret*/
/*  those correctly. the format of a ICON is as follows:            */
/*  ICONDIR  header with general purpose information such as DIB    */
/*           type, and frame count.                                 */
/*  ICONDIRENTRY: an array of these structures follows with info    */
/*                on width height (max 254X254 min 2x2)             */
/*                bitdepth, bytes in the actual resource and        */
/*                an offset of where the image is in the file.      */
/*  ICONIMAGE: This is special image header with a BIMAPINFOHEADER  */
/*             containing some redundant information but also some  */
/*             necessary information. if height is negative then the*/
/*             DIB is top down, if it is positive then the DIB is   */
/*             bottom up. following this is an array of RGBQUADS    */
/*             any colour palette and finally XOR/AND masks.        */
/*  image data follows.                                             */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include <string.h>
#include "gimex.h"
#include "icogimex.h"

/****************************************************************/
/*  Local Functions                                             */
/****************************************************************/

#define BOTTOM_UP 1
#define TOP_DOWN  2

//------------------------------------------------------------------
// read in true colour icon. For now I assume the icon is 565 format
// it might not be so but I have no way of testing this at the moment.
static int ReadTrueColorImage(GICONDIRENTRY *DirEntry,
                       char * dest,GINSTANCE *gx,
                       int width, int height)
{
    int i = 0;
    int j = 0;
    int ok = 0;
    int T = 0;
    int rowbytes = width*2;
    int ImageOffset = ICOGETFIELD(DirEntry->dwImageOffset);
    char * rowbuf = NULL;
    char * r = NULL;
    char * d = dest;

    gseek(gx->gstream,(GPOS)0);
    gseek(gx->gstream,(GPOS)(ImageOffset + sizeof(GICONIMAGE))); // read into the data.

    rowbuf = (char *)galloc(rowbytes);    
    if (rowbuf)
    {
        for(i = 0; i < height; i++)
        {
            r = rowbuf;
            ok = gread(gx->gstream,r,width*2);
            if (!ok) break;
            for(j = 0; j < width; j++)
            {
                T=ggeti(r,2);
                ((ARGB *)d)->a = 0xFF;
                ((ARGB *)d)->r = (unsigned char)((((T>>11)&31)*255+ 16)/31); 
                ((ARGB *)d)->g = (unsigned char)((((T>> 5)&63)*255+ 32)/63); 
                ((ARGB *)d)->b = (unsigned char)((((T>> 0)&31)*255+ 16)/31); 
                r+=2;
                d+=4;

            }
        }
    }
    if (rowbuf)
        gfree(rowbuf);
    return ok;
}

/*--------------- blows image up to 32 bits
static int ReadPaletteImage(GICONIMAGE *ImageHeader,GICONDIRENTRY *DirEntry,GINFO *ginfo,
                     char *dest,GINSTANCE *gx,int width,
                     int height,int PaletteSize,int origbpp,int direction)
{
    int i = 0;
    int j = 0;
    int ok = 0;
    int T = 0;
    int rowbytes = ((width*origbpp) + 7)/8;               // round up to nearest byte.
    int ImageOffset = ICOGETFIELD(DirEntry->dwImageOffset);
    char * rowbuf = NULL;
    char * r = NULL;
    char * d = dest;
    GRGBQUAD *Palette = NULL;
    gseek(gx->gstream,(GPOS)0);
    gseek(gx->gstream,(GPOS)(ImageOffset + sizeof(GBITMAPINFOHEADER))); // read into the palette.
    if (direction == BOTTOM_UP)
       d = dest + width*height*4 - width*4;       // go to end of image - one line.

    Palette = (GRGBQUAD *)galloc(PaletteSize * sizeof(GRGBQUAD));
    rowbuf = galloc(rowbytes);
    if (rowbuf && Palette)
    {
        memset(Palette,0,PaletteSize * sizeof(GRGBQUAD));
        memset(rowbuf,0,rowbytes);
        // read in the palette.
        ok = gread(gx->gstream,Palette,sizeof(GRGBQUAD) * PaletteSize);
       


        for(i = 0; i < height; i++)
        {
            r = rowbuf;
            ok = gread(gx->gstream,r,rowbytes);
            if (!ok)break;
            for(j = 0; j < width; j++)
            {
                T=ggeti(r,1);
                if (origbpp == 4) // 16 color palette
                {
                    char val1 = (unsigned char)((T>>4)&15);       // strip off first four bits.
                    char val2 = (unsigned char)(T&15);            // strip off second four bits.
                    ((ARGB *)d)->a = 0xFF;
                    ((ARGB *)d)->r = Palette[val1].rgbRed;
                    ((ARGB *)d)->g = Palette[val1].rgbGreen;
                    ((ARGB *)d)->b = Palette[val1].rgbBlue;
                    
                    d+=4;
                    ((ARGB *)d)->a = 0xFF;
                    ((ARGB *)d)->r = Palette[val2].rgbRed;
                    ((ARGB *)d)->g = Palette[val2].rgbGreen;
                    ((ARGB *)d)->b = Palette[val2].rgbBlue;
                    j++;  // we did two pixels in one so increment one extra time for loop.
                    r++;
                    d+=4;
                }
                else // 256 color palette
                {
                    ((ARGB *)d)->a = 0xFF;
                    ((ARGB *)d)->r = Palette[T].rgbRed;
                    ((ARGB *)d)->g = Palette[T].rgbGreen;
                    ((ARGB *)d)->b = Palette[T].rgbBlue;
                    r++;
                    d+=4;
                }
            }// for width
            if (direction == BOTTOM_UP)
                d -= width*8;                        // done with this line go up to beg of next line.
        }// for height
    }// if buffers ok

    if (Palette)
        gfree(Palette);
    if (rowbuf)
        gfree(rowbuf);
    return ok;
}
*/
static int ReadPaletteImage(GICONIMAGE *ImageHeader,GICONDIRENTRY *DirEntry,GINFO *ginfo,
                     char *dest,GINSTANCE *gx,int width,
                     int height,int PaletteSize,int origbpp,int direction)
{
    int i = 0;
    int j = 0;
    int ok = 0;
    int T = 0;
    int rowbytes = ((width*origbpp) + 7)/8;               // round up to nearest byte.
    int ImageOffset = ICOGETFIELD(DirEntry->dwImageOffset);
    char * rowbuf = NULL;
    char * r = NULL;
    char * d = dest;
    GRGBQUAD *Palette = NULL;
    gseek(gx->gstream,(GPOS)0);
    gseek(gx->gstream,(GPOS)(ImageOffset + sizeof(GBITMAPINFOHEADER))); // read into the palette.
    if (direction == BOTTOM_UP)
       d = dest + width*height - width;       // go to end of image - one line.

    Palette = (GRGBQUAD *)galloc(PaletteSize * sizeof(GRGBQUAD));
    rowbuf = (char *)galloc(rowbytes);
    if (rowbuf && Palette)
    {
        memset(Palette,0,PaletteSize * sizeof(GRGBQUAD));
        memset(rowbuf,0,rowbytes);
        // read in the palette.
        ok = gread(gx->gstream,Palette,sizeof(GRGBQUAD) * PaletteSize);
       


        for(i = 0; i < height; i++)
        {
            r = rowbuf;
            ok = gread(gx->gstream,r,rowbytes);
            if (!ok)break;
            for(j = 0; j < width; j++)
            {
                T=ggeti(r,1);
                if (origbpp == 4) // 16 color palette
                {
                    char val1 = (unsigned char)((T>>4)&15);       // strip off first four bits.
                    char val2 = (unsigned char)(T&15);            // strip off second four bits.
                    *d = val1;
                    d++;
                    *d = val2;
                    j++;  // we did two pixels in one so increment one extra time for loop.
                    r++;
                    d++;
                }
                else // 256 color palette
                {
                    *d = *r;
                    r++;
                    d++;
                }
            }// for width
            if (direction == BOTTOM_UP)
                d -= width*2;                        // done with this line go up to beg of next line.
        }// for height
    }// if buffers ok

    if (Palette)
        gfree(Palette);
    if (rowbuf)
        gfree(rowbuf);
    return ok;
}


/*
//----------------------------------------------------------------------------------------
static int ReadMonochromeImage(GICONIMAGE *ImageHeader,GICONDIRENTRY *DirEntry,GINFO *ginfo,
                     char *dest,GINSTANCE *gx,int width,
                     int height,int PaletteSize,int origbpp,int direction)
{
    int i = 0;
    int j = 0;
    int k = 0;
    int ok = 0;
    int T = 0;
    int rowbytes = ((width*origbpp) + 7)/8;               // round up to nearest byte.
    int ImageOffset = ICOGETFIELD(DirEntry->dwImageOffset);
    char * rowbuf = NULL;
    char * r = NULL;
    char * d = dest;

    GRGBQUAD *Palette = NULL;
    gseek(gx->gstream,(GPOS)0);
    gseek(gx->gstream,(GPOS)(ImageOffset + sizeof(GBITMAPINFOHEADER))); // read into the palette.
    if (direction == BOTTOM_UP)
       d = dest + width*height*4 - width*4;       // go to end of image - one line.

    if ((width%8) != 0) // I do not support cursors with a width that does not fit into a byte.
    {
#ifdef DEBUG
        printf("monochrome image is not a multiple of 8\n");
#endif
        return 0;
    }

    Palette = (GRGBQUAD *)galloc(2 *sizeof(GRGBQUAD));
    rowbuf = galloc(rowbytes);

    if (rowbuf && Palette)
    {
        memset(Palette,0,2 * sizeof(GRGBQUAD));
        memset(rowbuf,0,rowbytes);
        // read in the palette.
        ok = gread(gx->gstream,Palette,sizeof(GRGBQUAD) * 2);
        if (!ok){gfree(rowbuf);gfree(Palette);return 0;}           // if error end

        
        for(i = 0; i < height; i++)
        {
            r = rowbuf;
            ok = gread(gx->gstream,r,rowbytes);
            if (!ok)break;
            for(j = 0; j < rowbytes; j++)
            {
                T=ggeti(r,1);
                for(k = 7; k >= 0; k--)
                {
                   char val = (T>>k)&1;
                   ((ARGB *)d)->a = 0xFF;
                   ((ARGB *)d)->r = Palette[val].rgbRed;
                   ((ARGB *)d)->g = Palette[val].rgbGreen;
                   ((ARGB *)d)->b = Palette[val].rgbBlue;
                   d+=4;
                }
                r++;
            }// for width
            if (direction == BOTTOM_UP)
                d -= width*8;                        // done with this line go up to beg of next line.
        }// for height
    }// if buffers ok

    if (Palette)
        gfree(Palette);
    if (rowbuf)
        gfree(rowbuf);
    return ok;
}
*/

//----------------------------------------------------------------------------------------
static int ReadMonochromeImage(GICONIMAGE *ImageHeader,GICONDIRENTRY *DirEntry,GINFO *ginfo,
                     char *dest,GINSTANCE *gx,int width,
                     int height,int PaletteSize,int origbpp,int direction)
{
    int i = 0;
    int j = 0;
    int k = 0;
    int ok = 0;
    int T  = 0;
    int T2 = 0;
    int isMask = 0;
    int rowbytes = ((width*origbpp) + 7)/8;               // round up to nearest byte.
    int ImageOffset = ICOGETFIELD(DirEntry->dwImageOffset);
    int ImageSize = rowbytes*height;
    char * pImage = NULL;
    char * pXorMask = NULL;
    char * r = NULL;
    char * d = dest;
    char * x = NULL;
    GRGBQUAD *Palette = NULL;
    gseek(gx->gstream,(GPOS)0);
    gseek(gx->gstream,(GPOS)(ImageOffset + sizeof(GBITMAPINFOHEADER))); // read into the palette.
    if (direction == BOTTOM_UP)
       d = dest + width*height - width;       // go to end of image - one line.

    if ((width%8) != 0) // I do not support cursors with a width that does not fit into a byte.
    {
#ifdef DEBUG
//        printf("monochrome image is not a multiple of 8\n");
#endif
        return 0;
    }

    Palette  = (GRGBQUAD *)galloc(2 *sizeof(GRGBQUAD));
    pImage   = (char *)galloc(ImageSize);
    pXorMask = (char *)galloc(ImageSize);
    if (pImage && Palette && pXorMask)
    {
        memset(Palette,0,2 * sizeof(GRGBQUAD));
        //memset(rowbuf,0,rowbytes);
        // read in the palette.
        ok = gread(gx->gstream,Palette,sizeof(GRGBQUAD) * 2);
        if (!ok){gfree(pImage);gfree(pXorMask);gfree(Palette);return 0;}           // if error end

        ok = gread(gx->gstream,pImage,ImageSize);
        if (!ok){gfree(pImage);gfree(pXorMask);gfree(Palette);return 0;}           // if error end

        isMask = gread(gx->gstream,pXorMask,ImageSize);
        isMask = (isMask == ImageSize)?1:0;
        r = pImage;
        x = pXorMask;
        for(i = 0; i < height; i++)
        {           
            for(j = 0; j < rowbytes; j++)
            {
                T  = ggeti(r,1);
                T2 = ggeti(x,1);
                for(k = 7; k >= 0; k--)
                {
                   char val  = (unsigned char)((T>> k)&1);
                   char mask = (unsigned char)((T2>>k)&1);
                   if (!mask || !isMask)               // if no mask in file then just put colour, maks is an inverted mask so use 0 as draw and 1 as not draw.
                       *d = val;
                   else
                       *d = 2;                       // if not in my mask then transparent.
                   d++;
                }
                r++;
                x++;
            }// for width
            if (direction == BOTTOM_UP)
                d -= width*2;                        // done with this line go up to beg of next line.
        }// for height
    }// if buffers ok

    if (Palette)
        gfree(Palette);
    if (pImage)
        gfree(pImage);
    if (pXorMask)
        gfree(pXorMask);
    return ok;
}


/****************************************************************/
/*  Information Functions                                       */
/****************************************************************/


//------------------------------------------------------------------
GABOUT * GCALL ICO_about(void)
{
    GABOUT *info;

    info = (GABOUT *) galloc(sizeof(GABOUT));
    if (info)
    {
        memset(info, 0, sizeof(GABOUT));

        info->signature       = GMAKEID('I','C','O','N');
        info->size            = sizeof(GABOUT);
        info->version         = GIMEX_VERSION;
        info->canimport       = 1;    /* supports importing */
        info->canexport       = 1;    /* supports exporting */
        info->importpacked    = 0;    /* supports importing packed */
        info->exportpacked    = 0;    /* supports exporting packed */
        info->import8         = 1;    /* supports importing 8 bit indexed */
        info->export8         = 1;    /* supports exporting 8 bit indexed */
        info->import32        = 1;    /* supports importing 32 bit direct rgb */
        info->export32        = 0;    /* supports exporting 32 bit direct rgb */
        info->multiframe      = 1;    /* supports multiple frames */
        info->multifile       = 0;    /* format requires additional files or resource fork */
        info->multisize       = 1;    /* supports each frame different size */
        info->dotsubtype      = 1;    /* give .cur it's own subtype so that I can export properly */
        info->framebuffer     = 1;    /* module requires memory to buffer entire frame */
        info->external        = 0;    /* module uses external resources */
        info->usesfile        = 1;    /* module is file based */
        info->mipmap          = 0;    /* module supports mipmaps */
        info->maxframename    = 0;    /* max frame name character */
        info->defaultquality  = 100;  /* default pack quality */
        info->mactype[0]      = GMAKEID('.','W','I','N');
        strcpy(info->extensions[0], ".ico");                // extensions 
        strcpy(info->extensions[1], ".cur");                // extensions 
        strcpy(info->authorstr,     "Ian Davids");          // author
        strcpy(info->versionstr,    "1.04");                // version #
        strcpy(info->shorttypestr,  "ICO");                 // type 
        strcpy(info->wordtypestr,   "Icon");                // wordtype 
        strcpy(info->longtypestr,   "Windows Icon Cursor"); // longtype 
    }
    return(info);
}

//------------------------------------------------------------------
int GCALL ICO_is(GSTREAM *g)
{
    GICONDIR WinHeader;
    GICONDIRENTRY * GIE;
    int ok=0;

    gseek(g, 0);
    if (gread(g, &WinHeader, sizeof(WinHeader)))
    {
        // it's an icon if first field is zero and type is 1.
        if (!(ICOGETFIELD(WinHeader.idReserved)) && (ICOGETFIELD(WinHeader.idType) == GICO_TYPEICON || ICOGETFIELD(WinHeader.idType) == GICO_TYPECURS) && (ICOGETFIELD(WinHeader.idCount)) != 0)
            if ((ICOGETFIELD(WinHeader.idCount)) <= 20)
            {
                GIE = (GICONDIRENTRY *)galloc(sizeof(GICONDIRENTRY)*(ICOGETFIELD(WinHeader.idCount)));
                if (GIE)
                {
                    int byteread = sizeof(GICONDIRENTRY)*(ICOGETFIELD(WinHeader.idCount));
                    int rc = gread(g,GIE,byteread);
                    int i = 0;
                    if (rc == byteread)
                    {
                        for(i = 0; i < (int)(ICOGETFIELD(WinHeader.idCount)); i++)
                        {
                            if (ICOGETFIELD(GIE[i].bReserved) == 0 && ICOGETFIELD(GIE[i].bWidth) < 254 && ICOGETFIELD(GIE[i].bHeight) < 254)
                                ok = 100;
                            else
                                ok = 0;
                        }
                        
                    }
                    gfree(GIE);
                }

            }
            else
                ok = 90;     // if we have more than 10 frames then start to doubt this is an icon because TGA's look very much like icons.
    }
    return(ok);
}

/****************************************************************/
/*  Import Functions                                            */
/****************************************************************/

//------------------------------------------------------------------
int GCALL ICO_open(GINSTANCE **gxptr, GSTREAM *g, const char *pathname,bool framecountflag)
{
    GICONDIR WinHeader;
    GINSTANCE *gx;
    int ok=0;

    gseek(g,(GPOS)0);
    memset(&WinHeader, 0, sizeof(WinHeader));
    ok = gread(g, (char *) &WinHeader, sizeof(WinHeader)); /* read header */
    if (ok)
    {
        ok = 0;
        gx = (GINSTANCE *) galloc(sizeof(GINSTANCE));
        if (gx)
        {
            memset(gx, 0, sizeof(GINSTANCE));
            ok = ICOGETFIELD(WinHeader.idCount);
            gx->frames   = ok;
            gx->signature= GMAKEID('I','C','O','N');
            gx->size     = sizeof(GINSTANCE);
            gx->gstream  = g;
            *gxptr       = gx;
        }
    }
    return(ok);
}

//------------------------------------------------------------------
GINFO * GCALL ICO_info(GINSTANCE *gx, int framenum)
{
    int ok=0;
    GICONDIR WinHeader;
    GBITMAPINFOHEADER InfoHeader;
    GICONDIRENTRY *DirEntry;
    GINFO *ginfo       = 0;
    GRGBQUAD *Pal      = 0;
    int NumberOfFrames = 0;
    int width;
    int height;
    int redb,blueb,greenb,alphab,origbpp;
    int coltable = 0;
    int m        = 0;
    int bpp      = 0;
    int subtype  = 0;
    int hotspotx = 0;
    int hotspoty = 0;

    gseek(gx->gstream,(GPOS)0);
    memset(&WinHeader, 0, sizeof(WinHeader));

    ok = gread(gx->gstream, (char *) &WinHeader, sizeof(WinHeader));         // read header but not the DirEntries
    if (!ok) return 0;
    NumberOfFrames = ICOGETFIELD(WinHeader.idCount);                                                       // figure out number of frames.
    DirEntry = (GICONDIRENTRY *)galloc(sizeof(GICONDIRENTRY) * NumberOfFrames);                 // malloc my direntries.
    ok = gread(gx->gstream, (char *)(DirEntry), (sizeof(GICONDIRENTRY) * NumberOfFrames));          // read in all my directory headers.

    if (ok)
    {
        subtype = ICOGETFIELD(WinHeader.idType);
        width   = ICOGETFIELD(DirEntry[framenum].bWidth);
        height  = ICOGETFIELD(DirEntry[framenum].bHeight);
        hotspotx = ICOGETFIELD(DirEntry[framenum].wPlanes);
        hotspoty = ICOGETFIELD(DirEntry[framenum].wBitCount);
        // invalid check.
        if ((subtype != GICO_TYPEICON && subtype != GICO_TYPECURS) || width < 2 || width > 254 || height < 2 || height > 254)
        {
#ifdef DEBUG
//            if (height > 254 || height < 2)
//                printf("Height Invalid: %d - Acceptable Range 2x2 -> 254X254\n",height);
//            if (width > 254 || width < 2)
//                printf("Width  Invalid: %d - Acceptable Range 2x2 -> 254X254\n",width);
//            if (subtype != GICO_TYPEICON && subtype != GICO_TYPECURS)
//                printf("ICONDIR Type flag invalid: Valid types are icons and cursors\n");
#endif
            if (Pal)
                gfree(Pal);
            if (DirEntry)     // free up my directory entries.
                gfree(DirEntry);
            return 0;
        }
        // read out the Image Header.
        gseek(gx->gstream,ICOGETFIELD(DirEntry[framenum].dwImageOffset));
        //gread(gx->gstream,(char*) &ImageHeader,sizeof(ImageHeader));
        gread(gx->gstream,(char*) &InfoHeader,sizeof(GBITMAPINFOHEADER));

        origbpp  = ICOGETFIELD(InfoHeader.biBitCount);
        if (origbpp == 4)
        {
            alphab = 0;
            redb = blueb = greenb = 8;
            coltable = 16;
            Pal = (GRGBQUAD *)galloc(sizeof(GRGBQUAD)*16);
            gread(gx->gstream,(char*)Pal,sizeof(GRGBQUAD)*16);
            bpp = 8;
        }
        else if (origbpp == 8)
        {
            alphab   = 0;
            redb = blueb = greenb = 8;
            coltable = 256;
            Pal = (GRGBQUAD *)galloc(sizeof(GRGBQUAD)*256);
            gread(gx->gstream,(char*)Pal,sizeof(GRGBQUAD)*256);
            bpp = 8;
        }
        else if (origbpp == 16)   // possible support of 16 bit colors.
        {
            alphab   = 0;
            redb = blueb = 5;
            greenb = 6;
            coltable = 0;
            bpp = 32;
        }
        else if (origbpp == 1)
        {
            redb = blueb = greenb = 8;
            alphab = 0;
            coltable = 2;                           // have to change this later when I build the palette there should be 3 for transparency.
            Pal = (GRGBQUAD *)galloc(sizeof(GRGBQUAD)*2);
            gread(gx->gstream,(char*)Pal,sizeof(GRGBQUAD)*2);
            bpp = 8;
        }
        else                     // return error if I do not understand.
        {
#ifdef DEBUG
//            printf("origbpp: %d\n",origbpp);
#endif
            if (Pal)
                gfree(Pal);
            if (DirEntry)     // free up my directory entries.
                gfree(DirEntry);
            return 0;
        }

        ginfo = (GINFO *) galloc(sizeof(GINFO));
        if (ginfo)
        {
            memset(ginfo, 0, sizeof(GINFO));

            if (coltable == 256 || coltable == 16 || coltable == 2)
            {
                for(m = 0; m < coltable;m++)
                {
                    ginfo->colourtbl[m].a = 0xFF;
                    ginfo->colourtbl[m].r = Pal[m].rgbRed;
                    ginfo->colourtbl[m].g = Pal[m].rgbGreen;
                    ginfo->colourtbl[m].b = Pal[m].rgbBlue;
                }
                if (coltable == 2)// allow entry for xor mask in monochrome images, this is a kludge
                {
                    coltable = 3;
                    ginfo->colourtbl[2].a = 0;
                    ginfo->colourtbl[2].r = 0;
                    ginfo->colourtbl[2].g = 0;
                    ginfo->colourtbl[2].b = 0;
                }
            }
            if (subtype == GICO_TYPECURS)
            {
                ginfo->numhotspots = 1;
                ginfo->hotspottbl[0][0] = hotspotx;
                ginfo->hotspottbl[0][1] = hotspoty;
            }
            /* template for ginfo */

            ginfo->signature        = GMAKEID('I','C','O','N'); // Type of file 
            ginfo->size             = sizeof(GINFO);            // Length of GINFO 
            ginfo->version          = GIMEX_VERSION;
            ginfo->framenum         = framenum;                 // Number of frames (if more than 1, openframeblt required) 
            ginfo->width            = width;                    // Width of bitmap in pixels 
            ginfo->height           = height;                   // Height of bitmap in pixels 
            ginfo->subtype          = subtype-1;                // set subtype to idType-1 to match index addressing of internal format.
            ginfo->packed           = 0;                        // bitmaps are not compressed.
            ginfo->quality          = 100;                      // Quality of lossy packing 0..100 
            ginfo->alphabits        = alphab;                   // Number of bits in alpha channel 
            ginfo->redbits          = redb;                     // Number of bits in red channel 
            ginfo->greenbits        = greenb;                   // Number of bits in green channel 
            ginfo->bluebits         = blueb;                    // Number of bits in blue channel 
            ginfo->centerx          = 0;                        // Center Point relative to upper left corner 
            ginfo->centery          = 0;                        // 
            ginfo->defaultx         = 0;                        // Default Coordinate Point 
            ginfo->defaulty         = 0;
            ginfo->numhotspots      = 0;                        // Number of Hot Spots defined 
            ginfo->framename[0]     = 0;                        // Null terminated name of frame/image 
            ginfo->bpp              = bpp;                      // Bits per pixel (8 or 32) 
            ginfo->originalbpp      = origbpp;
            ginfo->numcolours       = coltable;                 // Number of colours in original indexed palette 
            ginfo->framesize        = width/8*height*origbpp;   // number of bytes in frame?
        }
    }
    if (Pal)
        gfree(Pal);
    if (DirEntry)     // free up my directory entries.
        gfree(DirEntry);
    return(ginfo);
}


//------------------------------------------------------------------
int GCALL ICO_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes)
{
    int ok=0;
    GICONDIR   WinHeader;
    GICONIMAGE ImageHeader;
    GICONDIRENTRY *DirEntry;
    int width          = 0;
    int height         = 0;
    int origbpp        = 0;
    int NumberOfFrames = 0;
    int PaletteSize    = 0;
    int framenum=ginfo->framenum;
    int direction      = 0;

    // Read Header
    memset(&WinHeader,0,sizeof(WinHeader));
    gseek(gx->gstream,(GPOS)0);
    ok = gread(gx->gstream, (char *) &WinHeader, sizeof(WinHeader));         // read header but not the DirEntries
    // Read in Directories.
    NumberOfFrames = ICOGETFIELD(WinHeader.idCount);                                                       // figure out number of frames.
    DirEntry = (GICONDIRENTRY *)galloc(sizeof(GICONDIRENTRY) * NumberOfFrames);                 // malloc my direntries.
    ok = gread(gx->gstream, (char *)DirEntry, sizeof(GICONDIRENTRY) * NumberOfFrames);          // read in all my directory headers.

    if (ok)
    {

        // get width and height.
        width   = ICOGETFIELD(DirEntry[framenum].bWidth);
        height  = ICOGETFIELD(DirEntry[framenum].bHeight);
     
        // read out the Image Header.
        gseek(gx->gstream,ICOGETFIELD(DirEntry[framenum].dwImageOffset));
        gread(gx->gstream,(char*) &ImageHeader,sizeof(ImageHeader));

        origbpp   = ICOGETFIELD(ImageHeader.icHeader.biBitCount);

        // figure out my direction. ie. if the image is top_down or bottom_up
        direction = ICOGETFIELD(ImageHeader.icHeader.biHeight);
        direction = (direction>0)?BOTTOM_UP:TOP_DOWN;

        if (origbpp > 16) // for now I'm not supporting greater than 16 bpp.
            {gfree(DirEntry);return(0);}
        // determine what type of image I am looking at and read it in accordingly.
        PaletteSize = (origbpp == 4)?16:(origbpp == 8?256:(origbpp == 1?2:(origbpp == 16?0:999))); // set palette size to 0/16/256
        if (!PaletteSize)
            ok = ReadTrueColorImage(&(DirEntry[framenum]),dest,gx,width,height);
        else if (PaletteSize == 16 || PaletteSize == 256)
            ok = ReadPaletteImage(&ImageHeader,&(DirEntry[framenum]),ginfo,dest,gx,width,height,PaletteSize,origbpp,direction);
        else if (PaletteSize == 2)
            ok = ReadMonochromeImage(&ImageHeader,&(DirEntry[framenum]),ginfo,dest,gx,width,height,PaletteSize,origbpp,direction);
#ifdef DEBUG
//        else
//            printf("Error: Bit Depth not supported: %d\n",origbpp);
#endif
    }
    if (DirEntry)
        gfree(DirEntry);
    return(ok);
}

int GCALL ICO_close(GINSTANCE *gx)
{
    int ok=0;

    if (gx)
        ok = gfree((void *) gx);

    return(ok);
}

#undef GICO_TYPECURS
#undef GICO_TYPEICON
#undef BOTTOM_UP
#undef TOP_DOWN
#undef ICOGETFIELD
#undef ICOPUTFIELD
