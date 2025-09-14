// (c) 1994-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*                        Png - Png Gimex                           */
/*                                                                  */
/*                       by Kaz Okuda, EAC                          */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History:                                       */
/* ------- ----   -- --------                                       */
/*    1.00 970819 KO Original PNG gimex module                      */
/*    1.01 970820 KO Greyscale palette fills 0-255 in numcolours    */
/*    1.02 970821 KO Added Export, staticized functions             */
/*    1.03 980506 KO ginfo.import/export --> canimport/canexport    */
/*    1.04 990428 KO Sets screen gamma to 2.5, uses alpha           */
/*    1.05 990429 KO Disable gamma correction, upgraded libpng      */
/*    1.06 990706 KO Correctly exports and imports alpha in 8bpp    */
/*    1.07 991215 FB Updated to PNG 1.0.5 lib                       */
/*    1.08 991222 FB bpp tests on write to avoid assert             */
/*    1.09 000525 KO PNG_write calls png_write_end to clean up      */
/*    1.10 010611 ID minimize galloc size so we don't overflow max  */
/*    1.11 070410 FB 16 bit grey by Jon Parise                      */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* Files: pngread.c pngwrite.c pnggimex.h                           */
/*        A bunch of files form libpng and zlib libraries           */
/*                                                                  */
/* Imports/Exports all png formats                                  */
/*                                                                  */
/* Chooses best format given ginfo                                  */
/*                                                                  */
/*          Color    Allowed    Interpretation                      */
/*          Type    Bit Depths                                      */
/*                                                                  */
/*          0       1,2,4,8,16  Each pixel is a grayscale sample.   */
/*                                                                  */
/*          2       8,16        Each pixel is an R,G,B triple.      */
/*                                                                  */
/*          3       1,2,4,8     Each pixel is a palette index;      */
/*                              a PLTE chunk must appear.           */
/*                              a TRNS may contain alpha values.    */
/*                                                                  */
/*          4       8,16        Each pixel is a grayscale sample,   */
/*                              followed by an alpha sample.        */
/*                                                                  */
/*          6       8,16        Each pixel is an R,G,B triple,      */
/*                              followed by an alpha sample.        */
/*                                                                  */
/* Currently using libpng v1.05 and zlib v1.13                      */
/* Changes to libpng sourse...                                      */
/* 1.   Renamed pngread.c to pnglread.c and pngwrite.c to           */
/*      pnglwrite.c because of a conflict with the gimex files.     */
/* 2.   Enabled PNG_USER_MEM_SUPPORTED in pngconf.h line 417.       */
/*                                                                  */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include <string.h>
#include "gimex.h"

#define PNG_GIMEX_INTERNAL
#include "pnggimex.h"
#undef PNG_GIMEX_INTERNAL

/*******************
 * Local Functions *
 *******************/

static void PNG_read_data(png_structp png_ptr, png_bytep data, png_uint_32 length)
{
    GSTREAM *g;
    png_uint_32 check;

    g = (GSTREAM*)png_get_io_ptr(png_ptr);

    check = gread(g, data, length);
    if (check != length)
    {
        png_error(png_ptr, "Read Error");
    }
}

static void PNG_read_warning_fn(png_structp png_ptr, png_const_charp warning_msg)
{
    /* Do nothing */
}

static int PNG_read_gimex(GINFO *ginfo, png_infop info_ptr, png_structp png_ptr, char *dest, int rowbytes)
{
    png_bytep *row_pointers;
    int i;
    int j;
    unsigned char *srcptr;
    ARGB *dstptr;

    if (!setjmp(png_ptr->jmpbuf))
    {
        row_pointers = (png_bytep *)galloc(sizeof(png_bytep) * ginfo->height);
        if (row_pointers)
        {
            unsigned char *tempbuf=NULL;
            if (ginfo->bpp == 64)
            {
                /*
                 * Read the data into a temp buf and convert it.
                 */
                tempbuf = (unsigned char *) galloc(ginfo->height * info_ptr->rowbytes);
                if (tempbuf)
                {
                    for (i=0; i<ginfo->height; i++)
                    {
                        row_pointers[i] = tempbuf + info_ptr->rowbytes * i;
                    }

                    png_read_image(png_ptr, row_pointers);


                    for (i=0; i<ginfo->height; i++)
                    {
                        unsigned short *dstptr16 = (unsigned short *)((char *)dest + rowbytes*i);
                        unsigned short *srcptr16 = (unsigned short *)row_pointers[i];

                        if (info_ptr->color_type == PNG_COLOR_TYPE_RGB)            // 48bpp
                        {
                            for (j=0; j<ginfo->width; j++)
                            {
                                dstptr16[0] = srcptr16[0];
                                dstptr16[1] = srcptr16[1];
                                dstptr16[2] = srcptr16[2];
                                dstptr16[3] = 0xFFFF;
                                dstptr16 += 4;
                                srcptr16 += 3;
                            }
                        }
                        
                        else if (info_ptr->color_type == PNG_COLOR_TYPE_RGB_ALPHA) // 64bpp
                        {
                            for (j=0; j<ginfo->width; j++)
                            {
                                dstptr16[0] = srcptr16[0];
                                dstptr16[1] = srcptr16[1];
                                dstptr16[2] = srcptr16[2];
                                dstptr16[3] = srcptr16[3];
                                dstptr16 += 4;
                                srcptr16 += 4;
                            }
                        }
                    }

                    gfree(tempbuf);
                }
            }
            else if (ginfo->bpp == 32)
            {
                /*
                 * Read the data into a temp buf and convert it.
                 */
                // IAN only take real image size originally Kaz had this as 4times the image size
                // this causes galloc to balk on the gfree as it attempt to malloc a large image
                // (say 4949X2468 image buffer would be 146569584 which is to much for gfree aparently)
                tempbuf = (unsigned char *)galloc(ginfo->height * info_ptr->rowbytes);
                if (tempbuf)
                {
                    for (i=0; i<ginfo->height; i++)
                    {
                        row_pointers[i] = tempbuf + info_ptr->rowbytes * i;
                    }

                    png_read_image(png_ptr, row_pointers);


                    for (i=0; i<ginfo->height; i++)
                    {
                        srcptr = (unsigned char*)row_pointers[i];
                        dstptr = (ARGB*)((unsigned char*)dest + rowbytes*i);

                        if (info_ptr->color_type == PNG_COLOR_TYPE_RGB)            // 24bpp
                        {
                            for (j=0; j<ginfo->width; j++)
                            {
                                dstptr->r = *srcptr++;
                                dstptr->g = *srcptr++;
                                dstptr->b = *srcptr++;
                                dstptr->a = 255;
                                dstptr++;
                            }
                        }
                        
                        else if (info_ptr->color_type == PNG_COLOR_TYPE_RGB_ALPHA) // 32bpp
                        {
                            for (j=0; j<ginfo->width; j++)
                            {
                                dstptr->r = *srcptr++;
                                dstptr->g = *srcptr++;
                                dstptr->b = *srcptr++;
                                dstptr->a = *srcptr++;
                                dstptr++;
                            }
                        }
                    }

                    gfree(tempbuf);
                }
            }
            else                                            // 8 bpp
            {
                for (i=0; i<ginfo->height; i++)
                {
                    row_pointers[i] = (png_bytep)(dest + (rowbytes * i));
                }
                png_read_image(png_ptr, row_pointers);

            }

            gfree(row_pointers);
        }
    }
    else            // Jump here on error
    {
        /* Error reading the file */
        return(0);
    }

    return(1);
}

static png_voidp PNGREAD_malloc PNGARG((png_structp png_ptr, png_size_t n))
{
    return (galloc(n));
}    

static void PNGREAD_free PNGARG((png_structp png_ptr, png_structp ptr))
{
    gfree(ptr);
}    

/****************************************************************/
/*  Information Functions                                       */
/****************************************************************/

GABOUT * GCALL PNG_about(void)
{
    GABOUT *info;

    info = (GABOUT *) galloc(sizeof(GABOUT));
    if (info)
    {
        memset(info, 0, sizeof(GABOUT));

        info->signature       = GMAKEID(0,'P','N','G');
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
        info->multisize       = 0;    /* supports each frame different size */
        info->framebuffer     = 1;    /* module requires memory to buffer entire frame */
        info->external        = 0;    /* module uses external resources */
        info->usesfile        = 1;    /* module is file based */
        info->maxframename    = 0;    /* max frame name character */
        info->defaultquality  = 100;  /* default pack quality */
        info->mactype[0]      = GMAKEID( 0 , 'P' , 'N' , 'G' );   /* mac file system types used */
        strcpy(info->extensions[0], ".png");                /* extensions */
        strcpy(info->authorstr,     "Kaz Okuda");           /* author */
        strcpy(info->versionstr,    "1.11");                /* version # */
        strcpy(info->shorttypestr,  "PNG");                 /* type */
        strcpy(info->wordtypestr,   "PNG");                 /* wordtype */
        strcpy(info->longtypestr,   "Portable Network Graphics");   /* longtype */
    }
    return(info);
}

int GCALL PNG_is(GSTREAM *g)
{
    int ok=0;
    unsigned char buf[8];

    gseek(g,(GPOS)0);
    gread(g, buf, 8);

    ok = png_check_sig(buf, 8);
    if (!ok)
    {
        ok = png_check_sig(buf, 4);
        if (ok)
            ok = 50;
    }
    else
    {
        ok = 100;
    }

    return(ok);
}

/****************************************************************/
/*  Import Functions                                            */
/****************************************************************/

int GCALL PNG_open(GINSTANCE **gxptr, GSTREAM *g, const char *pathname,bool framecountflag)
{
    int ok=0;
    GINSTANCE *gx;
    PNGINFO *png;

    gx = (GINSTANCE *) galloc(sizeof(GINSTANCE));
    if (gx)
    {
        memset(gx, 0, sizeof(GINSTANCE));

        gx->gref = (char *) galloc(sizeof(PNGINFO));
        if (gx->gref)
        {

            png = (PNGINFO*) gx->gref;

            /* Create and initialize the png_struct with the desired error handler
               functions.  If you want to use the default stderr and longjump method,
               you can supply NULL for the last three parameters.  We also check that
               the header file is compatible with the library version.
             */

            // This is the original method.  png_create_read_struct_2 is not guaranteed so if there are problems try this.
            //png->png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, (void *)NULL, NULL, PNG_read_warning_fn);

            png->png_ptr = png_create_read_struct_2(PNG_LIBPNG_VER_STRING, (void *)NULL, NULL, PNG_read_warning_fn, NULL, PNGREAD_malloc, (png_free_ptr)PNGREAD_free);
            if (png->png_ptr)
            {
                png->info_ptr = png_create_info_struct(png->png_ptr);
                if (png->info_ptr)
                {
                    png_set_read_fn(png->png_ptr, (voidp) g, (png_rw_ptr) PNG_read_data);

                    if (!setjmp(png->png_ptr->jmpbuf))
                    {
                        /* Read the file information */
                        gseek(g, 0);
                        png_read_info(png->png_ptr, png->info_ptr);

                        gx->frames = 1;
                        gx->framenum = 0;
                        gx->signature=GMAKEID(0,'P','N','G');
                        gx->size=sizeof(GINSTANCE);
                        gx->gstream = g;
                        *gxptr = gx;
                        ok = gx->frames;
                    }
                    else
                    {
                        png_destroy_read_struct(&png->png_ptr, &png->info_ptr, (png_infopp)NULL);
                    }
                }
                else
                {
                    // Free png_ptr
                    png_destroy_read_struct(&png->png_ptr, (png_infopp)NULL, (png_infopp)NULL);
                }
            }
        }
    }

    return(ok);
}


GINFO * GCALL PNG_info(GINSTANCE *gx, int framenum)
{
    GINFO *ginfo=0;
    PNGINFO *png;
    int ok = 1;

    png = (PNGINFO*)gx->gref;


    ginfo = (GINFO *) galloc(sizeof(GINFO));
    if (ginfo)
    {
        memset(ginfo, 0, sizeof(GINFO));

        int bpc = png->info_ptr->bit_depth;
        int originalbpp = 32;
        int bpp = 32;
        int alphabits=bpc;
        int redbits=bpc;
        int greenbits=bpc;
        int bluebits=bpc;

        switch(png->info_ptr->color_type)
        {
            case PNG_COLOR_TYPE_PALETTE:
                originalbpp = bpc;
                bpp = 8;
                break;
            case PNG_COLOR_TYPE_GRAY:
                originalbpp = bpc;
                alphabits = greenbits = bluebits = 0;
                bpp = 8;
                break;
            case PNG_COLOR_TYPE_RGB:
                originalbpp = bpc*3;
                alphabits = 0;
                bpp = 32;
                break;
            case PNG_COLOR_TYPE_GRAY_ALPHA:
                originalbpp = bpc*2;
                greenbits = bluebits = 0;
                bpp = 32;
                break;

            case PNG_COLOR_TYPE_RGB_ALPHA:
                originalbpp = bpc*4;
                bpp = 32;
                break;
        }

        ginfo->signature        = GMAKEID(0,'P','N','G'); /* Type of file */
        ginfo->size             = sizeof(GINFO);            /* Length of GINFO */
        ginfo->version          = GIMEX_VERSION;
        ginfo->framenum         = 0;                    /* Current frame num */
        ginfo->width            = png->info_ptr->width; /* Width of bitmap in pixels */
        ginfo->height           = png->info_ptr->height;/* Height of bitmap in pixels */
        ginfo->framesize        = 0;
        ginfo->subtype          = 0;
        ginfo->packed           = 0;
        ginfo->quality          = 100;          /* Quality of lossy packing 0..100 */
        ginfo->alphabits        = alphabits;    /* Number of bits in alpha channel */
        ginfo->redbits          = redbits;      /* Number of bits in red channel */
        ginfo->greenbits        = greenbits;    /* Number of bits in grreen channel */
        ginfo->bluebits         = bluebits;     /* Number of bits in blue channel */
        ginfo->centerx          = 0;            /* Center Point relative to upper left corner */
        ginfo->centery          = 0;
        ginfo->defaultx         = 0;            /* Default Coordinate Point */
        ginfo->defaulty         = 0;
        ginfo->numhotspots      = 0;            /* Number of Hot Spots defined */
        ginfo->framename[0]     = 0;            /* Null terminated name of frame/image */
        ginfo->bpp              = bpp;          /* Bits per pixel (8 or 32) */
        ginfo->originalbpp      = originalbpp;
        ginfo->numcolours       = 0;            /* Number of colours in original indexed palette */

        {
            int sizetext;
            png_get_text(png->png_ptr, png->info_ptr, (png_textp *) &ginfo->comment[0], &sizetext);
        }
        ginfo->dpi = (float)png_get_x_pixels_per_meter(png->png_ptr, png->info_ptr) * 0.039370079f;

        if (bpp <= 8)
        {
            int i;

            if (png->info_ptr->color_type == PNG_COLOR_TYPE_GRAY)   /* Make grayscale palette */
            {
                ginfo->numcolours = 1<<bpp;

                for (i = 0; i < ginfo->numcolours; i++)
                {
                    unsigned char intensity;

                    intensity = (unsigned char) ((i * 255) / (ginfo->numcolours - 1));
                    ginfo->colourtbl[i].r = intensity;
                    ginfo->colourtbl[i].g = intensity;
                    ginfo->colourtbl[i].b = intensity;
                    ginfo->colourtbl[i].a = 255;
                }
            }
            else                                                    /* Get palette from file */
            {
                ginfo->numcolours = png->info_ptr->num_palette;

                if (png->info_ptr->num_palette)
                {
                    png_bytep alpha_table = NULL;
                    png_color_16p chromakey = 0;
                    int num_alpha = 0;

                    /* Get the alpha info for the palette if it exists, currently ignoring chromakey */
                    if (png_get_tRNS(png->png_ptr, png->info_ptr, &alpha_table, &num_alpha, &chromakey))
                    {
                        ginfo->alphabits = 8;
                    }
                    else
                    {
                        alpha_table = NULL;
                    }

                    for (i = 0; i < ginfo->numcolours; i++)
                    {
                        ginfo->colourtbl[i].r = png->info_ptr->palette[i].red;
                        ginfo->colourtbl[i].g = png->info_ptr->palette[i].green;
                        ginfo->colourtbl[i].b = png->info_ptr->palette[i].blue;

                        if (alpha_table && (i < num_alpha))
                            ginfo->colourtbl[i].a = alpha_table[i];
                        else
                            ginfo->colourtbl[i].a = 255;
                    }
                }
                else
                {
                    ok = 0;
                }
            }
        }

        if (!ok)
        {
            gfree(ginfo);
            ginfo = 0;
        }
    }

    return(ginfo);
}

int GCALL PNG_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes)
{
    png_infop info_ptr;
    png_structp png_ptr;
//    png_color_16 my_background;
    int ok=0;

    // This is the original method.  png_create_read_struct_2 is not guaranteed so if there are problems try this.
    //png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, (void *)NULL, NULL, PNG_read_warning_fn);

    png_ptr = png_create_read_struct_2(PNG_LIBPNG_VER_STRING, (void *)NULL, NULL, PNG_read_warning_fn, NULL, PNGREAD_malloc, (png_free_ptr)PNGREAD_free);
    if (png_ptr)
    {
        info_ptr = png_create_info_struct(png_ptr);
        if (info_ptr)
        {
            png_set_read_fn(png_ptr, (voidp) gx->gstream, (png_rw_ptr) PNG_read_data);

            if (!setjmp(png_ptr->jmpbuf))
            {

                /* Read the file information */
                gseek(gx->gstream, 0);
                png_read_info(png_ptr, info_ptr);


                /* Turn off gamma correction for now.  Less lossy */

                /* Tell libpng to handle the gamma conversion for you */
                /* Screen gamma of 2.5 is good default (better than 1.0) */
                /* If there is no gamma information in the file then don't set the gamma */
                //if (info_ptr->valid & PNG_INFO_gAMA)
                //    png_set_gamma(png_ptr, 2.5, info_ptr->gamma);


                /* tell libpng to strip 16 bit/color files down to 8 bits/color */
                if (info_ptr->bit_depth == 16)
                {
                    // TDH - enable byte swapping and disable stripping of 16-bit gray scale
                    // down to 8 unless the bpp input has been explicitly set to force this.
                    if (ginfo->bpp == 16 || ginfo->bpp == 64)
                    {
#if defined(EA_SYSTEM_LITTLE_ENDIAN)
                        png_set_swap(png_ptr);
#endif
                    }
                    else
                        png_set_strip_16(png_ptr);
                }

                /* One pixel per byte */
                if (info_ptr->bit_depth < 8)
                    png_set_packing(png_ptr);

                ///* add a filler byte to RGB files (before or after each RGB triplet) */
                //if (info_ptr->color_type == PNG_COLOR_TYPE_RGB)
                //    png_set_filler(png_ptr, 0xff, PNG_FILLER_BEFORE);


                /* Convert GrayAlpha(16 or 32 bpp) images to RGB, unless specifically
                   requested otherwise. */
                if (info_ptr->color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
                {
                    if (ginfo->bpp <= 16)
                        png_set_strip_alpha(png_ptr);
                    else
                        png_set_gray_to_rgb(png_ptr);
                }
                else if (info_ptr->color_type == PNG_COLOR_TYPE_GRAY && ginfo->bpp >= 32)
                {
                    png_set_gray_to_rgb(png_ptr);
                }
                else if (info_ptr->color_type == PNG_COLOR_TYPE_RGB_ALPHA)
                {
                    if (!ginfo->alphabits)
                        png_set_strip_alpha(png_ptr);

                    if (ginfo->bpp <= 16)
                        png_set_rgb_to_gray(png_ptr, 1, -1, -1);
                }
                else if (info_ptr->color_type == PNG_COLOR_TYPE_RGB)
                {
                    if (ginfo->bpp <= 16)
                        png_set_rgb_to_gray(png_ptr, 1, -1, -1);
                }

                /* Update info structure. */
                png_read_update_info(png_ptr, info_ptr);

                ok = PNG_read_gimex(ginfo, info_ptr, png_ptr, dest, rowbytes);

            }
            else        // Jumps back here on error
            {
                /* Error reading the file */
                ok = 0;
            }
            png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
        }
    }

    return(ok);
}

int GCALL PNG_close(GINSTANCE *gx)
{
    PNGINFO *png;
    int ok=0;

    if (gx)
    {
        if (gx->gref)
        {
            png = (PNGINFO*)gx->gref;
            png_destroy_read_struct(&png->png_ptr, &png->info_ptr, (png_infopp)NULL);
            gfree(gx->gref);
        }
        ok = gfree(gx);
    }

    return(ok);
}


