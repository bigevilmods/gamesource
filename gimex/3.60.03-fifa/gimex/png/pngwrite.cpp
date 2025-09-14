// (c) 1994-2007 Electronic Arts Inc.

#include <string.h>
#include "gimex.h"

#define PNG_GIMEX_INTERNAL
#include "pnggimex.h"
#undef PNG_GIMEX_INTERNAL


/**********************
* Internal Functions *
**********************/

static void PNG_write_data(png_structp png_ptr, png_bytep data, png_uint_32 length)
{
    GSTREAM *g;
    png_uint_32 check;

    g = (GSTREAM*)png_get_io_ptr(png_ptr);

    check = gwrite(g, data, length);
    if (check != length)
    {
        png_error(png_ptr, "Write Error");
    }
}

static void PNG_flush_data(png_structp png_ptr)
{
    /* Do nothing */
}

static void PNG_write_warning_fn(png_structp png_ptr, png_const_charp warning_msg)
{
    /* Do nothing except in debug mode */
#ifdef DEBUG
    printf(warning_msg);
#endif
}

static int PNG_write_gimex(const GINFO *ginfo, png_infop info_ptr, png_structp png_ptr, char *source, int rowbytes)
{
    png_bytep *row_pointers;
    int i;
    int j;
    unsigned char *tempbuf;
    int temprowbytes;
    unsigned char *dstptr;
    unsigned char *src8ptr;
    ARGB *srcptr;

    if (!setjmp(png_ptr->jmpbuf))
    {
        row_pointers = (png_bytep *)galloc(sizeof(png_bytep) * ginfo->height);

        if (row_pointers)
        {
            /* Calculate the rowbytes */
            if (ginfo->bpp == 32)       // 32 bpp
            {
                if (info_ptr->color_type & PNG_COLOR_MASK_ALPHA)
                    temprowbytes = ginfo->width * 4;
                else
                    temprowbytes = ginfo->width * 3;
            }
            // TDH Change 2/17/06 - Added (minimal) support for 16 bit gray scale format.
            else if (ginfo->bpp == 16)  // 16 bpp
            {
                temprowbytes = ginfo->width * 2;
            }
            // TDH end change
            else                        // 8 bpp
            {
                if (info_ptr->color_type & PNG_COLOR_MASK_ALPHA)
                    temprowbytes = ginfo->width * 2;
                else
                    temprowbytes = ginfo->width;
            }

            /* Read the data into a temp buf and convert it. */
            tempbuf = (unsigned char *)galloc(ginfo->height * temprowbytes);
            if (tempbuf)
            {
                for (i=0; i<ginfo->height; i++)
                {
                    row_pointers[i] = tempbuf + temprowbytes * i;
                }

                for (i=0; i<ginfo->height; i++)
                {
                    if (ginfo->bpp == 32)                           /* 32 bpp */
                    {
                        dstptr = (unsigned char*)row_pointers[i];
                        srcptr = (ARGB*)((unsigned char*)source + rowbytes*i);

                        if (info_ptr->color_type & PNG_COLOR_MASK_ALPHA)
                        {
                            for (j=0; j<ginfo->width; j++)
                            {
                                *dstptr++ = srcptr->r;
                                *dstptr++ = srcptr->g;
                                *dstptr++ = srcptr->b;
                                *dstptr++ = srcptr->a;
                                srcptr++;
                            }
                        }
                        else
                        {
                            for (j=0; j<ginfo->width; j++)
                            {
                                *dstptr++ = srcptr->r;
                                *dstptr++ = srcptr->g;
                                *dstptr++ = srcptr->b;
                                srcptr++;
                            }
                        }
                    }
                    // TDH Change 2/17/06 - Added (minimal) support for 16 bit gray scale formats.
                    else if (ginfo->bpp == 16)
                    {
                        dstptr = (unsigned char*)row_pointers[i];
                        src8ptr = (unsigned char*)source + rowbytes*i;

                        // To do: endian swap?
#if defined(EA_SYSTEM_LITTLE_ENDIAN)
                        {
                            unsigned short* src = (unsigned short *)src8ptr;
                            unsigned short* dst = (unsigned short *)dstptr;
                            int j;

                            for (j=0; j<ginfo->width; j++)
                            {
                                unsigned short p = *src++;
                                *dst++ = (p >> 8) | (p << 8);
                            }
                        }
#else
                        memcpy( dstptr, src8ptr, ginfo->width*2 );
#endif
                    }
                    // TDH end change
                    else                                            /* 8 bpp */
                    {
                        dstptr = (unsigned char*)row_pointers[i];
                        src8ptr = (unsigned char*)source + rowbytes*i;

                        if ( (info_ptr->color_type == PNG_COLOR_TYPE_GRAY) ||
                             (info_ptr->color_type == PNG_COLOR_TYPE_GRAY_ALPHA) )  /* GREY or GREY_ALPHA */
                        {
                            int sd;
                            int sr;
                            int alpha;
                            int bpp;

                            alpha = info_ptr->color_type & PNG_COLOR_MASK_ALPHA;

                            bpp = ginfo->originalbpp;
                            if (bpp < 1) bpp = 1;
                            if (bpp > 8) bpp = 8;

                            if (bpp == 1)           /* Must special case the 1bpp */
                            {
                                sd = 1;
                                sr = 0;
                            }
                            else
                            {
                                sd = (1<<bpp)-1;    /* Source depth */
                                sr = (sd+1)>>1;     /* Source round */
                            }

                            for (j=0; j<ginfo->width; j++)
                            {
                                *dstptr++ = ((ginfo->colourtbl[*src8ptr].r)*255+sr)/sd;           /* Scale the greyscale value up to 8 bpp */

                                if (alpha)
                                    *dstptr++ = ginfo->colourtbl[*src8ptr].a; /* Add the alpha bits after the colour bits */
                                src8ptr++;
                            }
                        }
                        else                                                        /* PALETTE */
                        {
                            for (j=0; j<ginfo->width; j++)
                            {
                               *dstptr++ = *src8ptr++;
                            }
                        }
                    }
                }

                png_write_image(png_ptr, row_pointers);

                gfree(tempbuf);
            }

            gfree(row_pointers);
        }
    }
    else            /* Jump here on error */
    {
        /* Error reading the file */
        return(0);
    }

    return(1);
}

static int PNGisgreyscale(const ARGB *src, int colours)
{
    int i;
    int ok=1;

    for (i=0; (i<colours) && ok; ++i)
    {
        if (src[i].r!=src[i].g || src[i].r!=src[i].b)
            ok = 0;
    }

    return(ok);
}

static png_voidp PNGWRITE_malloc PNGARG((png_structp png_ptr, png_size_t n))
{
    return (galloc(n));
}

static void PNGWRITE_free PNGARG((png_structp png_ptr, png_structp ptr))
{
    gfree(ptr);
}    

/****************************************************************/
/*  Export Functions                                            */
/****************************************************************/

int GCALL PNG_wopen(GINSTANCE **gxptr, GSTREAM *g, const char *pathname, int numframes)
{
    GINSTANCE *gx;
    int ok=0;

    gx = (GINSTANCE *) galloc(sizeof(GINSTANCE));
    if (gx)
    {
        memset(gx, 0, sizeof(GINSTANCE));
        gx->gstream = g;
        gx->framenum = 0;
        *gxptr = gx;
        ok = 1;
    }
    return(ok);
}

int GCALL PNG_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes)
{
    png_infop info_ptr;
    png_structp png_ptr;
    png_colorp palette=0;
    png_bytep transpalette=0;
    int color_type;
    int bpc;
    int ok=0;
    int i;

    if (gx->framenum == 0)  /* Only allow one write */
    {
        // This is the original method.  png_create_write_struct_2 is not guaranteed so if there are problems try this.
        //png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, (void *)NULL, NULL, PNG_write_warning_fn);

        png_ptr = png_create_write_struct_2(PNG_LIBPNG_VER_STRING, (void *)NULL, NULL, PNG_write_warning_fn, NULL, PNGWRITE_malloc, (png_free_ptr)PNGWRITE_free);
        if (png_ptr)
        {
            info_ptr = png_create_info_struct(png_ptr);
            if (info_ptr)
            {
                png_set_write_fn(png_ptr, (voidp) gx->gstream,(png_rw_ptr)PNG_write_data, PNG_flush_data);

                if (!setjmp(png_ptr->jmpbuf))
                {
                    if (ginfo->bpp == 32)                           /* 32 bpp */
                    {
#if 0
                        bpc = ginfo->redbits;
                        if (bpc < ginfo->greenbits)
                            bpc = ginfo->greenbits;
                        if (bpc < ginfo->bluebits)
                            bpc = ginfo->bluebits;

                        if (bpc == 1) bpc = 1;
                        else if (bpc == 2) bpc = 2;
                        else if (bpc <= 4) bpc = 4;
                        else bpc = 8;
                        /* No more than 8 bpc allowed */
#endif
                        bpc = 8;
                        if (ginfo->alphabits)
                            color_type = PNG_COLOR_TYPE_RGB_ALPHA;
                        else
                            color_type = PNG_COLOR_TYPE_RGB;
                    }
                    // TDH Change 2/17/06 - Added (minimal) support for 16 bit gray scale format.
                    else if (ginfo->bpp == 16)                       // 16 bpp
                    {
                        if (ginfo->alphabits)
                        {
                            // Assume LA88 format
                            bpc = 8;
                            color_type = PNG_COLOR_TYPE_GRAY_ALPHA;
                        }
                        else
                        {
                            bpc = 16;
                            color_type = PNG_COLOR_TYPE_GRAY;
                        }
                    }
                    // TDH End change
                    else                                             /* 8 bpp */
                    {
                        bpc = ginfo->originalbpp;
                        if (bpc == 1) bpc = 1;
                        else if (bpc == 2) bpc = 2;
                        else if (bpc <= 4) bpc = 4;
                        else bpc = 8;

                        if (PNGisgreyscale(&ginfo->colourtbl[0], ginfo->numcolours))
                        {
                            if (ginfo->alphabits)
                            {
                                color_type = PNG_COLOR_TYPE_GRAY_ALPHA;
                                /* GRAY_ALPHA only supports 8 or 16 bpc */
                                bpc = 8;
                            }
                            else
                                color_type = PNG_COLOR_TYPE_GRAY;
                        }
                        else
                        {
                            color_type = PNG_COLOR_TYPE_PALETTE;
                            palette = (png_colorp) galloc(ginfo->numcolours * sizeof(png_color));
                            for (i=0; i<ginfo->numcolours; i++)
                            {
                                palette[i].red = ginfo->colourtbl[i].r;
                                palette[i].green = ginfo->colourtbl[i].g;
                                palette[i].blue = ginfo->colourtbl[i].b;
                            }
                        }
                    }

                    info_ptr->width = ginfo->width;
                    info_ptr->height = ginfo->height;
                    info_ptr->bit_depth = bpc;
                    info_ptr->color_type = color_type;

                    if (color_type == PNG_COLOR_TYPE_PALETTE)
                    {
                        info_ptr->valid |= PNG_INFO_PLTE;
                        info_ptr->palette = palette;
                        info_ptr->num_palette = ginfo->numcolours;

                        if (ginfo->alphabits > 0)
                        {
                            /* Write out an tRNS chunk with the alpha levels for each palette entry */
                            transpalette = (png_bytep)galloc(info_ptr->num_palette * sizeof(png_bytep));
                            if (transpalette)
                            {
                                for (i=0; i<info_ptr->num_palette; ++i)
                                {
                                    transpalette[i] = ginfo->colourtbl[i].a;
                                }
                                png_set_tRNS(png_ptr, info_ptr, transpalette, info_ptr->num_palette, NULL);
                            }
/*
								if (info_ptr->valid & PNG_INFO_tRNS)
								{
									info_ptr->color_type |= PNG_COLOR_MASK_ALPHA;
								}
*/
                        }
                    }

                    if (ginfo->dpi)
                    {
                        int ppm;
                        ppm = (int) (ginfo->dpi / 0.039370079f);

                        png_set_pHYs(png_ptr, info_ptr, ppm, ppm, PNG_RESOLUTION_METER);
                    }

                    /* Write the file information */
                    png_write_info(png_ptr, info_ptr);

                    /* Expect one pixel per byte */
                    if (info_ptr->bit_depth < 8)
                        png_set_packing(png_ptr);

                    ok = PNG_write_gimex(ginfo, info_ptr, png_ptr, source, rowbytes);

                    /* Write the end */
                    png_write_end(png_ptr, info_ptr);

                    if (palette)
                        gfree(palette);

                    if (transpalette)
                        gfree(transpalette);

                }

                png_destroy_write_struct(&png_ptr, &info_ptr);

            }
        }
        gx->framenum++;
    }

    return(ok);
}

int GCALL PNG_wclose(GINSTANCE *gx)
{
    int ok=0;

    if (gx)
    {
        ok = gfree(gx);
    }

    return(ok);
}


