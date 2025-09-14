// (c) 1994-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*           JPeg - JPEG Joint Picture Expert Group Gimex           */
/*                                                                  */
/*                    by FrANK G. Barchard, EAC                     */
/*                  based on JPeg lib by Tom Lane                   */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History:                                       */
/* ------- ----   -- --------                                       */
/*    1.00 940414 FB Alchemy plugin JPeg support                    */
/*    1.01 950102 FB JPeg library based module                      */
/*    1.02 950106 FB Exporter                                       */
/*    1.03 950529 FB ginstance signature                            */
/*    1.04 970603 KO Upgraded to version 6a JPEG group library      */
/*    1.05 970708 KO isjpg() now uses library to check the file     */
/*    1.06 970710 KO Can handles different colourspaces (not well)  */
/*    1.07 971118 KO Exports 8 bit, handles greyscale               */
/*    1.08 971120 KO subtype 1 makes ARGB jpegs                     */
/*    1.09 980506 KO ginfo.import/export --> canimport/canexport    */
/*    1.10 990812 FB dpi (density)                                  */
/*    1.11 991206 FB quality 100 if packed=0                        */
/*    1.12 001207 FB check header before calling jpeg               */
/*    1.13 061023 SK read/write images that have no DHT (for mjpeg) */
/*    1.14 070904 FB cmyk fix by Franklin Cho                       */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 24 bit import                                                    */
/* decompresses                                                     */
/* Non-Reentrant                                                    */
/* Files: jpgread.c jpgwrite.c jpggimex.h                           */
/*        j*.c j*.h (many files. see gimexdb)                       */
/*                                                                  */
/* Subtypes:                                                        */
/*          1       0x1 alpha channel                               */
/*          2       0x2 No DHT chunk                                */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Format Notes:                                                    */
/* -------------                                                    */
/* JPG files support 8,15,16,24,32 bit depths.                      */
/* 8 bit can be grey scale                                          */
/* Runlength compression                                            */
/* Additional Footer information optional                           */
/* x/y origin relative to lower left with origin in lower left      */
/* descriptor: 00vhaaaa                                             */
/*   h horizontal flip                                              */
/*   v vertical flip                                                */
/*   a alpha bits                                                   */
/* Progressive Jpeg support (New to version 6)                      */
/*                                                                  */
/* Headers                                                          */
/* JFIF: 0xffd8ffe0xxxx4a46494600                                   */
/* eXIF: ffd8ffe1 15fc4578 69660000 49492a00                        */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */
#include <string.h>
#include <stdio.h>
#include <setjmp.h>
#include "gimex.h"
#include "jpeglib.h"
#include "jerror.h"
#include "jpggimex.h"

/****************************************************************/
/*  Internal Variables                                          */
/****************************************************************/

static int JPGreadgimexmarker=0;

/****************************************************************/
/*  Internal Functions                                          */
/****************************************************************/


// COPIED FROM jdmarker.c

/*
 * Macros for fetching data from the data source module.
 *
 * At all times, cinfo->src->next_input_byte and ->bytes_in_buffer reflect
 * the current restart point; we update them only when we have reached a
 * suitable place to restart if a suspension occurs.
 */

#define GETJOCTET(value) (value)

/* Declare and initialize local copies of input pointer/count */
#define INPUT_VARS(cinfo)  \
        struct jpeg_source_mgr * datasrc = (cinfo)->src;  \
        const JOCTET * next_input_byte = datasrc->next_input_byte;  \
        size_t bytes_in_buffer = datasrc->bytes_in_buffer

/* Unload the local copies --- do this only at a restart boundary */
#define INPUT_SYNC(cinfo)  \
        ( datasrc->next_input_byte = next_input_byte,  \
          datasrc->bytes_in_buffer = bytes_in_buffer )

/* Reload the local copies --- seldom used except in MAKE_BYTE_AVAIL */
#define INPUT_RELOAD(cinfo)  \
        ( next_input_byte = datasrc->next_input_byte,  \
          bytes_in_buffer = datasrc->bytes_in_buffer )

/* Internal macro for INPUT_BYTE and INPUT_2BYTES: make a byte available.
 * Note we do *not* do INPUT_SYNC before calling fill_input_buffer,
 * but we must reload the local copies after a successful fill.
 */
#define MAKE_BYTE_AVAIL(cinfo,action)  \
        if (bytes_in_buffer == 0) {  \
          if (! (*datasrc->fill_input_buffer) (cinfo))  \
            { action; }  \
          INPUT_RELOAD(cinfo);  \
        }  \
        bytes_in_buffer--

/* Read a byte into variable V.
 * If must suspend, take the specified action (typically "return FALSE").
 */
#define INPUT_BYTE(cinfo,V,action)  \
        MAKESTMT( MAKE_BYTE_AVAIL(cinfo,action); \
                  V = GETJOCTET(*next_input_byte++); )

/* As above, but read two bytes interpreted as an unsigned 16-bit integer.
 * V should be declared unsigned int or perhaps INT32.
 */
#define INPUT_2BYTES(cinfo,V,action)  \
        MAKESTMT( MAKE_BYTE_AVAIL(cinfo,action); \
                  V = ((unsigned int) GETJOCTET(*next_input_byte++)) << 8; \
                  MAKE_BYTE_AVAIL(cinfo,action); \
                  V += GETJOCTET(*next_input_byte++); )

// END COPY FROM jdmarker.c

// cmyk_rgb re-implemented by Franklin Cho at the Sims Division.
// Based on CMYK -> CMY -> RGB conversion equations found in http://www.easyrgb.com/math.php?MATH=M14#text14
//
//   CMYK [0..1] -> CMY [0..1] :
//   C = ( C * ( 1 - K ) + K )
//   M = ( M * ( 1 - K ) + K )
//   Y = ( Y * ( 1 - K ) + K )
//
//   CMY [0..1] -> RGB [0..255] :
//   R = ( 1 - C ) * 255
//   G = ( 1 - M ) * 255
//   B = ( 1 - Y ) * 255
//
// In the CMYK test image I used (GrayRails.jpg, etc.), 0 mapped to full cyan, etc. and 255 mapped to black.
// So, I transformed the equations to conform to these mappings.

static void cmyk_rgb(unsigned char cmyk[4], unsigned char rgb[3])
{
    rgb[0] =  (unsigned char)gmin((cmyk[0] - 255)*cmyk[3]/255 + cmyk[3], 255);
    rgb[1] =  (unsigned char)gmin((cmyk[1] - 255)*cmyk[3]/255 + cmyk[3], 255);
    rgb[2] =  (unsigned char)gmin((cmyk[2] - 255)*cmyk[3]/255 + cmyk[3], 255);
}


static void JPGreadline(char *source, char *dest, int width, int channels, int colourspace, int marker)
{
    unsigned char *s;
    unsigned char outbuf[3];
    ARGB *d;
    int i;

    s = (unsigned char *) source;
    d = (ARGB *) dest;

    if (marker)
    {
        for (i=0; i<width; ++i)
        {
            d[i].a = s[i*4+0];
            d[i].r = s[i*4+1];
            d[i].g = s[i*4+2];
            d[i].b = s[i*4+3];
        }
    }
    else
    {
        switch(colourspace)
        {
            default:
            case JCS_RGB:
                for (i=0; i<width; ++i)
                {
                    d[i].a = 255;
                    d[i].r = s[i*3+0];
                    d[i].g = s[i*3+1];
                    d[i].b = s[i*3+2];
                }
                break;

            case JCS_CMYK:

                for (i=0; i<width; ++i)
                {
                    cmyk_rgb(&s[i*4], outbuf);

                    d[i].a = 255;
                    d[i].r = outbuf[0];
                    d[i].g = outbuf[1];
                    d[i].b = outbuf[2];
                }

                break;

            case JCS_GRAYSCALE:
                memcpy(dest,source,width);
                break;
        }
    }
}

static void JPGselectoutputcolourspace(struct jpeg_decompress_struct *cinfo)
{
    /*
     * Ask for RGB color if supported, CMYK is OK because I don't know what YCCK is.
     *
     * NOTE: This takes into account the colour conversions currently supported by
     * the JPEG library and more support may be added later (Like supporting
     * YCCK->RGB directly).  Info on supported conversions may be found in
     *
     *      jinit_color_deconverter() in jdcolor.c
     *
     * If out_color_space is not changed it will do no conversion.
     */

    if (cinfo->jpeg_color_space == JCS_YCbCr)
    {
        cinfo->out_color_space = JCS_RGB;
    }
    else if (cinfo->jpeg_color_space == JCS_YCCK)
    {
        cinfo->out_color_space = JCS_CMYK;
    }

}

boolean JPGmarkerparser (j_decompress_ptr cinfo)
{
    int length;
    int buffp;
    char b[JPGGIMEXMARKERSIZE];
    INPUT_VARS(cinfo);

    INPUT_2BYTES(cinfo, length, return FALSE);
    length -= 2;

    if (length >= JPGGIMEXMARKERSIZE)
    {
        for (buffp = 0; buffp < JPGGIMEXMARKERSIZE; buffp++)
        {
            INPUT_BYTE(cinfo, b[buffp], return FALSE);
        }
        length -= JPGGIMEXMARKERSIZE;
    }

    if (!strncmp(b, JPGGIMEXMARKER, JPGGIMEXMARKERSIZE))
    {
        // Markers match
        JPGreadgimexmarker = 1;
    }

    INPUT_SYNC(cinfo);
    if (length > 0)               /* skip any remaining data -- could be lots */
        (*cinfo->src->skip_input_data) (cinfo, (long) length);

    return TRUE;
}


/****************************************************************/
/*  JPeg Library Interface                                      */
/****************************************************************/

extern JSAMPLE * image_buffer;  /* Points to large array of R,G,B-order data */
extern int image_height;    /* Number of rows in image */
extern int image_width;     /* Number of columns in image */

/******************** JPEG DECOMPRESSION SAMPLE INTERFACE *******************/

struct my_error_mgr {
  struct jpeg_error_mgr pub;    /* "public" fields */

  jmp_buf setjmp_buffer;    /* for return to caller */
};

typedef struct my_error_mgr * my_error_ptr;

/*
 * Here's the routine that will replace the standard error_exit method:
 */

METHODDEF(void)
my_error_exit (j_common_ptr cinfo)
{
  /* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
  my_error_ptr myerr = (my_error_ptr) cinfo->err;

  /* Always display the message. */
  /* We could postpone this until after returning, if we chose. */
  /* KO - Perhaps it is bad to output any error message at all? */
  /* (*cinfo->err->output_message) (cinfo); */

  /* Return control to the setjmp point */
  longjmp(myerr->setjmp_buffer, 1);
}

/****************************************************************/
/*  Information Functions                                       */
/****************************************************************/

GABOUT * GCALL JPG_about(void)
{
    GABOUT *info;

    info = (GABOUT *) galloc(sizeof(GABOUT));
    if (info)
    {
        memset(info, 0, sizeof(GABOUT));

        info->signature       = GMAKEID('J','P','E','G');
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
        info->defaultquality  = 80;   /* default pack quality */
        info->mactype[0]      = GMAKEID('J','P','E','G');   /* mac file system types used */
        strcpy(info->extensions[0], ".jpg");                /* extensions */
        strcpy(info->extensions[1], ".jpeg");               /* extensions */
        strcpy(info->extensions[2], ".jfif");               /* extensions */
        strcpy(info->authorstr,     "FrANK G. Barchard");   /* author */
        strcpy(info->versionstr,    "1.14");                /* version # */
        strcpy(info->shorttypestr,  "JPG");                 /* type */
        strcpy(info->wordtypestr,   "JPeg");                /* wordtype */
        strcpy(info->longtypestr,   "JPeg");                /* longtype */
    }
    return(info);
}

/* check for reasonable header: */
/* JFIF: 0xffd8ffe0xxxx4a46494600 */
/* eXIF: ffd8ffe1 15fc4578 69660000 49492a00 */
/* Now uses lib to check for valid file */

int GCALL JPG_is(GSTREAM *g)
{
    unsigned char jpgheader[4];
    int ok=0;

    gseek(g,(GPOS)0);
    if (gread(g,jpgheader,4))
    {
        if (ggetm(jpgheader,4)==0xffd8ffe0  // Jpeg
         || ggetm(jpgheader,4)==0xffd8ffe1
         || ggetm(jpgheader,4)==0xffd8ffed)
        {
#if defined(_MSC_VER)
            ok = 98;    // give intel library preference
#else
            ok = 100;
#endif
        }
    }
    return(ok);
}

#if 0
int GCALL JPG_is(GSTREAM *g)
{
    struct jpeg_decompress_struct cinfo;
    struct my_error_mgr jerr;
    int ok=0;

    gseek(g, 0);

    cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = my_error_exit;
    if (setjmp(jerr.setjmp_buffer))
    {
        jpeg_destroy_decompress(&cinfo);
    }
    else
    {
        jpeg_create_decompress(&cinfo);
        jpeg_stdio_src(&cinfo, g);
        if (jpeg_read_header(&cinfo, TRUE) == JPEG_HEADER_OK)
#if defined(_MSC_VER)
            ok = 99;    // give intel library preference
#else
            ok = 100;
#endif
        jpeg_destroy_decompress(&cinfo);
    }

    return(ok);
}
#endif

/****************************************************************/
/*  Import Functions                                            */
/****************************************************************/

int GCALL JPG_open(GINSTANCE **gxptr, GSTREAM *g, const char *pathname,bool framecountflag)
{
    GINSTANCE *gx;
    int ok=0;

    gx = (GINSTANCE *) galloc(sizeof(GINSTANCE));
    if (gx)
    {
        memset(gx, 0, sizeof(GINSTANCE));
        gx->frames = 1;
        gx->signature=GMAKEID('J','P','E','G');
        gx->size=sizeof(GINSTANCE);
        gx->gstream = g;
        *gxptr = gx;
        ok = 1;
    }
    return(ok);
}

GINFO * GCALL JPG_info(GINSTANCE *gx, int framenum)
{
    GINFO *ginfo=0;
    int i;
    int width;
    int height;
    int density;
    int channels;
    int bpp;
    int ok=1;
    int alphabits;
    struct jpeg_decompress_struct cinfo;
    struct my_error_mgr jerr;
    JSAMPARRAY buffer;      /* Output row buffer */
    int row_stride;     /* physical row width in output buffer */
    int read_marker = 0;


    /* read in JPEG frame info */

    cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = my_error_exit;
    if (setjmp(jerr.setjmp_buffer))
    {
        jpeg_destroy_decompress(&cinfo);
        return((GINFO *) 0);
    }

    jpeg_create_decompress(&cinfo);
    jpeg_set_marker_processor(&cinfo, JPGGIMEXMARKERNUMBER, JPGmarkerparser);

    gseek(gx->gstream, 0);
    jpeg_stdio_src(&cinfo, gx->gstream);

    JPGreadgimexmarker = 0;
    jpeg_read_header(&cinfo, TRUE);
    read_marker = JPGreadgimexmarker;

    jpeg_start_decompress(&cinfo);

    row_stride = cinfo.output_width * cinfo.output_components;

    width    = cinfo.output_width;
    height   = cinfo.output_height;
    density = 0;
    if (cinfo.density_unit)
        density   = cinfo.X_density;
    channels = cinfo.output_components;

    buffer = (*cinfo.mem->alloc_sarray)
        ((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

    while (cinfo.output_scanline < cinfo.output_height)
    {
        jpeg_read_scanlines(&cinfo, buffer, 1);
    }

    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);

    if (ok)
    {
        switch (channels)
        {
            default:
            case 3: bpp=24; alphabits=0; break;
            /* 4 channels is used for ycck and cmyk (not alpha) */
            case 4: bpp=24; alphabits=0; break;
            case 1: bpp=8;  alphabits=0; break;
        }

        ginfo = (GINFO *) galloc(sizeof(GINFO));
        if (ginfo)
        {
            memset(ginfo, 0, sizeof(GINFO));

            /* template for ginfo */

            ginfo->signature        = GMAKEID('J','P','E','G');   /* Type of file */
            ginfo->size             = sizeof(GINFO);            /* Length of GINFO */
            ginfo->version          = GIMEX_VERSION;
            ginfo->framenum         = 0;                        /* Current frame num */
            ginfo->width            = width;                    /* Width of bitmap in pixels */
            ginfo->height           = height;                   /* Height of bitmap in pixels */
            ginfo->subtype          = read_marker?JPGARGB:0;    /* subtype */
            ginfo->packed           = 1;
            ginfo->quality          = 70;           /* Quality of lossy packing 0..100 */
            ginfo->alphabits        = alphabits;    /* Number of bits in alpha channel */
            ginfo->redbits          = 8;            /* Number of bits in red channel */
            ginfo->greenbits        = 8;            /* Number of bits in grreen channel */
            ginfo->bluebits         = 8;            /* Number of bits in blue channel */
            ginfo->centerx          = 0;            /* Center Point relative to upper left corner */
            ginfo->centery          = 0;
            ginfo->defaultx         = 0;            /* Default Coordinate Point */
            ginfo->defaulty         = 0;
            ginfo->numhotspots      = 0;            /* Number of Hot Spots defined */
            ginfo->dpi              = (float) density;
            ginfo->framename[0]     = 0;            /* Null terminated name of frame/image */

            // Check if the file contains the DHT.
            if (!cinfo.read_dht)
                ginfo->subtype |= JPGNODHT;

            /* 15,16,24,32 bpp true colour */

            if (bpp>=15)
            {
                ginfo->bpp         = 32;       /* Bits per pixel (8 or 32) */
                ginfo->originalbpp = bpp;
                ginfo->numcolours  = 0;        /* Number of colours in original indexed palette */
            }
            else    /* 8 bit grey scale */
            {
                ginfo->bpp         = 8;        /* Bits per pixel (8 or 32) */
                ginfo->originalbpp = 8;
                ginfo->numcolours  = 256;      /* Number of colours in original indexed palette */
                for (i=0; i<256; ++i)
                {
                    ginfo->colourtbl[i].a = 0xff;
                    ginfo->colourtbl[i].r = (unsigned char)i;
                    ginfo->colourtbl[i].g = (unsigned char)i;
                    ginfo->colourtbl[i].b = (unsigned char)i;
                }
            }
        }
        if (ginfo && !ok)
        {
            gfree((char *)ginfo);
            ginfo = 0;
        }
    }

    return(ginfo);
}

int GCALL JPG_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes)
{
    int y;
    int width;
    int height;
    int channels;
    int ok=1;
    struct jpeg_decompress_struct cinfo;
    struct my_error_mgr jerr;
    JSAMPARRAY buffer;      /* Output row buffer */
    int row_stride;     /* physical row width in output buffer */
    int read_marker = 0;

    /* read in JPEG frame info */

    cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = my_error_exit;
    if (setjmp(jerr.setjmp_buffer))
    {
        jpeg_destroy_decompress(&cinfo);
        return(0);
    }

    jpeg_create_decompress(&cinfo);
    jpeg_set_marker_processor(&cinfo, JPGGIMEXMARKERNUMBER, JPGmarkerparser);

    gseek(gx->gstream, 0);
    jpeg_stdio_src(&cinfo, gx->gstream);

    JPGreadgimexmarker = 0;
    jpeg_read_header(&cinfo, TRUE);
    read_marker = JPGreadgimexmarker;

    JPGselectoutputcolourspace(&cinfo);
    jpeg_start_decompress(&cinfo);

    row_stride = cinfo.output_width * cinfo.output_components;

    width    = cinfo.output_width;
    height   = cinfo.output_height;
    channels = cinfo.output_components;

    buffer = (*cinfo.mem->alloc_sarray)
        ((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

    for (y=0; y<(int)cinfo.output_height; ++y)
    {
        jpeg_read_scanlines(&cinfo, buffer, 1);
        if (y<ginfo->height)
        {
            JPGreadline((char*)buffer[0],dest,width,channels,cinfo.out_color_space, read_marker);
            dest += rowbytes;
        }
    }

    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);

    return(ok);
}

int GCALL JPG_close(GINSTANCE *gx)
{
    int ok=0;

    if (gx)
        ok = gfree((void *) gx);

    return(ok);
}

