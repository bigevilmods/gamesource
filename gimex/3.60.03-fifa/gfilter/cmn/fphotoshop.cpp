// (c) 1995-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*                    Photoshop - Photoshop Filters                 */
/*                                                                  */
/*                    by Bernard E. Ebinu, EAC                      */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History                                        */
/* ------- ------ -- -------                                        */
/*  1.00   021002 BE color reduction from 32 to 15,and 8 bits       */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 32 bit                                                           */
/* photoshop(int bpp)                                               */
/* int bpp;  8,16,24,32                                             */
/* Note, old module supported levels and would make into 8 bit      */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */


#include "fphotoshop.h"
#include <objbase.h>
#include <windows.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <io.h>
//#import "ScriptingSupport.8li"
#include "ScriptingSupport.tlh"
using namespace Photoshop;


typedef struct
{
    /* TARGA FILE HEADER structure 18 bytes */
    char identsize;             /* size of ID field that follows 18 byte header (0 usually) */
    char colourmaptype;         /* type of colour map 0=none, 1=has palette */
    char imagetype;             /* type of image 0=none,1=indexed,2=rgb,3=grey,+8=rle packed */

    char colourmapstart[2];     /* first colour map entry in palette */
    char colourmaplength[2];    /* number of colours in palette */
    char colourmapbits;         /* number of bits per palette entry 15,16,24,32 */

    char xstart[2];             /* image x origin */
    char ystart[2];             /* image y origin */
    char width[2];              /* image width in pixels */
    char height[2];             /* image height in pixels */
    char bits;                  /* image bits per pixel 8,16,24,32 */
    char descriptor;            /* image descriptor bits (vh flip bits) */
} TGAHeaderStruct;

#define TGAGETSIGNEDFIELD(field) ((int)(ggeti((void *)&field, sizeof(field))<<(32-sizeof(field)*8))>>(32-sizeof(field)*8))
#define TGAGETFIELD(field) ggeti((void *)&field, sizeof(field))
#define TGAPUTFIELD(field,data) gputi((void *)&field,(unsigned int)(data),sizeof(field))


/****************************************************************/
/* Internal Functions                                           */
/****************************************************************/
static void TGAflipline(unsigned char *linebase,int width,int bpp)
{
    int i;

    if (bpp==32)
    {
        int *p1;
        int *p2;
        int swaplong;

        p1 = (int *) (linebase);
        p2 = ((int *) linebase) + width-1;
        for (i=0; i<(width>>1); ++i)
        {
            swaplong = *p1;
            *p1++ = *p2;
            *p2-- = swaplong;
        }
    }
    else
    {
        unsigned char *p1;
        unsigned char *p2;
        unsigned char swapchar;

        p1 = linebase;
        p2 = linebase+width-1;
        for (i=0; i<(width>>1); ++i)
        {
            swapchar = *p1;
            *p1++ = *p2;
            *p2-- = swapchar;
        }
    }
}


int PreparePhotoshopWrite( char *file_name, GBITMAP *source)
{
    /*********************************************************
    // Finding the targa module, writing the current
    **********************************************************/
    GINFO           *ginfo = source->ginfo;
    GSTREAM         *ps_gstream = 0;
    GINSTANCE       *ps_gx = 0;
    TGAHeaderStruct tgaheader;
    char            ps_file_name[128];
    char            *file_buf;
    char            *s_image;
    int             success = 0, ok = 0;
    int             bpl = ginfo->width * ginfo->originalbpp / 8;

    GetTempPath( 128,ps_file_name );
    strcat( ps_file_name, "Temp.PSbak.tga" );

    /* TARGA FILE HEADER structure 18 bytes */
    TGAPUTFIELD(tgaheader.identsize, 0);                    /* size of ID field that follows 18 byte header (0 usually) */
    TGAPUTFIELD(tgaheader.colourmaptype, 0 );               /* type of colour map 0=none, 1=has palette */
    TGAPUTFIELD(tgaheader.imagetype, 2);                    /* type of image 0=none,1=indexed,2=rgb,3=grey,+8=rle packed */

    TGAPUTFIELD(tgaheader.colourmapstart,0);                /* first colour map entry in palette */
    TGAPUTFIELD(tgaheader.colourmaplength, 0);              /* number of colours in palette */
    TGAPUTFIELD(tgaheader.colourmapbits, 32);               /* number of bits per palette entry 15,16,24,32 */

    TGAPUTFIELD(tgaheader.xstart, ginfo->defaultx);         /* image x origin */
    TGAPUTFIELD(tgaheader.ystart, -ginfo->defaulty);        /* image y origin */
    TGAPUTFIELD(tgaheader.width, ginfo->width);             /* image width in pixels */
    TGAPUTFIELD(tgaheader.height, ginfo->height);           /* image height in pixels */
    TGAPUTFIELD(tgaheader.bits, 32);                        /* image bits per pixel 8,16,24,32 */
    TGAPUTFIELD(tgaheader.descriptor, ginfo->alphabits);           /* image descriptor bits (vh flip bits) */


    ps_gstream = gwopen( ps_file_name);
    if( ps_gstream )
    {
        ok = gwrite( ps_gstream, (char *) &tgaheader, 18);
        file_buf = (char *)galloc( bpl * 3 + 16);
        if (file_buf)
        {
            for ( int y=0; y < ginfo->height && ok; ++y)
            {
                s_image = (char *)( (char *)source->image +(ginfo->height-y-1)*source->rowbytes);

                unsigned char *s=(unsigned char *)s_image;
                unsigned char *d=(unsigned char *)file_buf;

                for (int x=0; x < ginfo->width; ++x)
                {
                    d[0] = ((ARGB *) s)->b;
                    d[1] = ((ARGB *) s)->g;
                    d[2] = ((ARGB *) s)->r;
                    d[3] = ((ARGB *) s)->a;
                    d += 4;
                    s += 4;
                }
                bpl = d-(unsigned char *)file_buf;
                ok = gwrite( ps_gstream,file_buf,bpl );
            }
            success = 1;
            gfree(file_buf);
        }
    }
    gclose( ps_gstream );
    strcpy( file_name, ps_file_name );
    return( success );
}

#if 1
int PreparePhotoshopRead( GBITMAP *dest_bitmap, GINFO *gin, char *dest_im, int dest_rowbytes )
{
    unsigned char *linebuf;
    unsigned char *linebase;
    char            *dest = dest_im;
    char            file_path[128];
    int             rowbytes = dest_rowbytes;
    int y;
    int width;
    int height;
    int bpp;
    int colours;
    int packed;
    int offset;
    int ok = 0;
    int identsize;
    int colourlen;
    int colourbits;
    int descriptor;
    int vfrowbytes;
    TGAHeaderStruct tgaheader;
    GSTREAM         *ps_gstream = 0;
    FILE            *fstream;
    GINFO           *ginfo = gin;


    GetTempPath( 128,file_path );
    strcat( file_path, "Temp.PSbak.tga" );
    fstream = fopen( file_path, "r" );

    if( fstream )
    {
        fseek( fstream, 0, SEEK_SET );
        ok = fread( (char *)&tgaheader, 18,1,fstream );
        if (ok)
        {

            colours = 0;
            packed  = 0;
            width       = TGAGETFIELD(tgaheader.width);
            height      = TGAGETFIELD(tgaheader.height);
            bpp         = TGAGETFIELD(tgaheader.bits);
            colourlen   = TGAGETFIELD(tgaheader.colourmaplength);
            colourbits  = TGAGETFIELD(tgaheader.colourmapbits);
            identsize   = 0;
            descriptor  = TGAGETFIELD(tgaheader.descriptor);

            offset = 18+identsize;
            if (TGAGETFIELD(tgaheader.colourmaptype))           /* type of colour map 0=none, 1=has palette */
                offset += colourlen*((colourbits+7)/8);

            fseek(fstream, offset, SEEK_SET );

            if (descriptor&0x20)             /* start at top */
            {
                linebase = (unsigned char *) dest;
                vfrowbytes = rowbytes;
            }
            else                            /* start of last row */
            {
                linebase = (unsigned char *) dest + (rowbytes * (height - 1));
                vfrowbytes = -rowbytes;
            }

            linebuf = (unsigned char *) galloc(((width*((bpp+7)&~7)))>>3);
            if (linebuf)
            {
                for (y=0; y<height && ok; ++y)
                {
                    rowbytes = ((width*((bpp+7)&~7)))>>3;
                    //ok = read((int) fstream->_file, linebuf,(unsigned int)rowbytes);
					ok = fread(linebuf, (unsigned int)rowbytes, 1, fstream);
                    ftell( fstream );

                    unsigned char *d = linebase;
                    unsigned char *s = linebuf;
                    int i = 0;

                    switch( bpp )
                    {
                    case 8:     // uncompressed 8 bit index image
                        for (i=0; i<width; ++i)
                        {
                            *d++ = *s++;
                        }
                        break;

                    case 15:    /* uncompressed 15 bit direct image */
                    case 16:    /* uncompressed 16 bit direct image */
                        for (i=0; i<width; ++i)
                        {
                            if (ginfo->alphabits)
                                ((ARGB *) d)->a = (unsigned char)(((*(s+1)>>7)&1)*255);
                            else
                                ((ARGB *) d)->a = (unsigned char)(255);

                            ((ARGB *) d)->r = (unsigned char)(((*(s+1)>>2)&31)*255/31);
                            ((ARGB *) d)->g = (unsigned char)((((*(s+1)&3)<<3) | ((*(s+0)>>5)&7))*255/31);
                            ((ARGB *) d)->b = (unsigned char)((*s&31)*255/31);
                            d += 4;
                            s += 2;
                        }
                        break;

                    case 24:    /* uncompressed 24 bit direct image */
                        for (i=0; i<width; ++i)
                        {
                            ((ARGB *) d)->a = (unsigned char)(255);
                            ((ARGB *) d)->r = s[2];
                            ((ARGB *) d)->g = s[1];
                            ((ARGB *) d)->b = s[0];
                            d += 4;
                            s += 3;
                        }
                        break;

                    case 32:    // 32 bit indexed image
                        for ( i=0; i<width; ++i)
                        {
                            ((ARGB *) d)->a = s[3];
                            ((ARGB *) d)->r = s[2];
                            ((ARGB *) d)->g = s[1];
                            ((ARGB *) d)->b = s[0];
                            d += 4;
                            s += 4;
                        }
                        break;
                    }

                    if (descriptor&0x10)
                        TGAflipline(linebase,width,ginfo->bpp);
                    linebase += vfrowbytes;
                }
                ok = 1;
                gfree(linebuf);
            }
        }
        else
        {
            printf( "PHOTOSHOP FILTER ERROR -- could not read opened gstream\n" );
        }
    }
    else
    {
        ok = 0;
        printf( "PHOTOSHOP FILTER ERROR -- could not open stream\n" );
    }
    return(ok);
}
#endif


int OpenPhotoshopApplyAction( _ApplicationPtr app_ref, const char *file_path, const char *action, int bpp = 32 )
{
    tagVARIANT              open_args;
    int                     success = 0;
    tagVARIANT              saveas_options, saveas_ascopy, saveas_extension;

    memset( &saveas_options, 0, sizeof(tagVARIANT) );
    memset( &saveas_ascopy, 0, sizeof(tagVARIANT) );
    memset( &saveas_extension, 0, sizeof(tagVARIANT) );

//  try
    {
        memset( &open_args, 0, sizeof(tagVARIANT) );
        open_args.vt |= VT_I4;
        open_args.lVal = Photoshop::psTargaOpen;
        app_ref->Open( file_path, open_args );
        _DocumentPtr    doc_ref( app_ref->GetActiveDocument() );

        _TargaSaveOptionsPtr    tga_save_options( "Photoshop.TargaSaveOptions" );
        if( bpp == 24 )
            tga_save_options->Resolution = Photoshop::psTarga24Bits;
        else if( bpp == 16 )
            tga_save_options->Resolution = Photoshop::psTarga16Bits;
        else if( bpp == 8 )
        {
            _IndexedConversionOptionsPtr convert8( "Photoshop.IndexedConversionOptions" );
            tagVARIANT                  bit8_options;

            memset( &bit8_options, 0, sizeof(tagVARIANT) );
            convert8->Palette = Photoshop::psExact;         // found under enum Photoshop::PsPaletteType
            convert8->PreserveExactColors = true;
            convert8->Forced = Photoshop::psBlackWhite;     // "			  " Photoshop::PsForcedColors
            convert8->Transparency = true;
            convert8->Matte = Photoshop::psNoMatte;         // "			  " Photoshop::PsMatteType

            bit8_options.vt |= VT_DISPATCH;
            bit8_options.pdispVal = convert8;

            doc_ref->ChangeMode( Photoshop::psConvertToIndexedColor, bit8_options);
        }
        else
            tga_save_options->Resolution = Photoshop::psTarga32Bits;


        tga_save_options->RLECompression = false;

        saveas_options.vt |= VT_DISPATCH;
        saveas_ascopy.vt |= VT_BOOL;                            // saving a boolean value
        saveas_extension.vt |= VT_I4;                           // saving an int

        saveas_options.pdispVal = tga_save_options;
        saveas_ascopy.boolVal = true;
        saveas_extension.lVal = Photoshop::psLowercase;         // Comes from enum Photoshop::PsExtensionType


        doc_ref->SaveAs( (char *)file_path, saveas_options, saveas_ascopy, saveas_extension );

        memset( &saveas_options, 0, sizeof(tagVARIANT) );
        saveas_options.vt |= VT_I4;                             // saving an int
        saveas_options.lVal = Photoshop::psDoNotSaveChanges;    //found under enum Photoshop::PsSaveOptions
        doc_ref->Close( saveas_options );
        doc_ref.Detach();
        success = 1;
    }
#if 0
    catch( _com_error e )
    {
        success = 0;
        printf("OPEN ERROR - Could not open %s\n %s",file_path, e.ErrorMessage() );
    }
#endif
    return(success);
}


/****************************************************************/
/* Filter Functions                                             */
/****************************************************************/
FPUBLIC FABOUT *aboutphotoshop(void)
{
    FABOUT *about = (FABOUT *) galloc(sizeof(FABOUT));
    if (about)
    {
        memset(about, 0, sizeof(FABOUT));

        about->signature =          GMAKEID('S','H','O','P');
        about->size =               sizeof(FABOUT);
        about->version =            GFILTER_VERSION;
        about->filter8 =            1;
        about->filter32 =           1;
        about->minbitmaps =         1;
        about->maxbitmaps =         1;
        about->filterclass =        FCLASS_GENERAL;
        strcpy(about->authorstr,    "Bernard Ebinu");
        strcpy(about->versionstr,   "1.00");
        strcpy(about->namestr,      "Photoshop");
        strcpy(about->longnamestr,  "Photoshop Filters");
        about->numoptions =         1;              // yet to be decided.

        about->option[0].type =     FOP_INT;
        about->option[0].min.i =    8;
        about->option[0].max.i =    32;
        about->option[0].def.i =    8;
        strcpy(about->option[0].name,     "Colour Reduce");
        strcpy(about->option[0].enumlist,  "8,16,24,32");

    }
    return(about);
}



FPUBLIC int openphotoshop(FINSTANCE **filterInstance, int numsources, GBITMAP *sources, int numoptions, FOP *options)
{
    int                 ok = 0;
    int                 prep_ok = 0, open_ok = 0;
    FINSTANCE           *fx;
    char                file_path[128];

    fx = (FINSTANCE *) galloc(sizeof(FINSTANCE));
    if (fx)
    {
        memset(fx, 0, sizeof(FINSTANCE));
        fx->frames          = 1;
        fx->fref            = 0;
        fx->numsources      = numsources;
        fx->source          = sources;
        fx->numoptions      = numoptions;
        fx->option          = options;

        CoInitialize( NULL );
        {
            _ApplicationPtr     app_ref;
            if( S_OK == app_ref.CreateInstance( "Photoshop.Application.7.1" ) )
            {
                app_ref->Visible = false ;
                prep_ok = PreparePhotoshopWrite( file_path, fx->source );
                open_ok = OpenPhotoshopApplyAction( app_ref, file_path,fx->option[0].p, fx->option[0].i );
                fx->fref = (char *)galloc( strlen( file_path ) );
                strcpy( (char *)fx->fref, file_path );

                app_ref->Quit();
                ok = ( prep_ok & open_ok ) ? 1 : 0;
            }
            else
            {
                gfree( (void *)fx );
                fx = 0;
            }
        }
        CoUninitialize();
    }

    *filterInstance = fx;
    return(ok);
}

FPUBLIC GINFO *infophotoshop(FINSTANCE *fx, int framenum)
{
    GINFO           *ginfo = 0;
    TGAHeaderStruct tgaheader;
    FILE            *fstream;
    unsigned char pal[256*4];
    int             bpp = 32;
    int             ok= 0;
    int             colours;
    int             colourstart;
    int             colourlen;
//    int               colourbits;

    fstream = fopen( (char *)fx->fref, "r" );
    if( fstream )
    {
        rewind( fstream );      // moving the file position to beginning of file
        ok = fread( (char *)&tgaheader, 18,1,fstream );

        if (ok)
        {
            ginfo   = (GINFO *) galloc(fx->source[0].ginfo->size);
            if (ginfo)
            {
                memcpy(ginfo, fx->source[0].ginfo, fx->source[0].ginfo->size);
                bpp = fx->option[0].i;
                if (bpp>=15)
                {
                    if (bpp==16)
                        bpp=15;
                    ginfo->bpp          = 32;       /* Bits per pixel (8 or 32) */
                    ginfo->originalbpp  = bpp;
                    ginfo->numcolours   = 0;        /* Number of colours in original indexed palette */
                    if (bpp<=16)
                        ginfo->redbits = ginfo->greenbits = ginfo->bluebits = 5;
                    if (bpp<32)
                        ginfo->alphabits= 0;
                }
                else
                {
                    ok                  = fseek(fstream,18+TGAGETFIELD(tgaheader.identsize), SEEK_SET);
                    colours             = 256;
                    colourlen           = TGAGETFIELD(tgaheader.colourmaplength);
                    colourstart         = TGAGETFIELD(tgaheader.colourmapstart);
                    ginfo->bpp          = 8;                /* Bits per pixel (8 or 32) */
                    ginfo->originalbpp  = bpp;
                    ginfo->numcolours   = colours;          /* Number of colours in original indexed palette */
                    ginfo->startcolour  = colourstart;      /* Number of colours in original indexed palette */

                    if (TGAGETFIELD(tgaheader.colourmaptype))           /* type of colour map 0=none, 1=has palette */
                    {
                        ginfo->alphabits    = 0;
                        //ok = read((int) fstream->_file, (char *)pal,(unsigned int)colours*3);
						ok = fread(pal, (unsigned int)colours * 3, 1, fstream);
                        int p = ftell( fstream );
                        for ( int i=0; i<colourlen; ++i)
                        {
                            ginfo->colourtbl[colourstart+i].a = (unsigned char)(255);
                            ginfo->colourtbl[colourstart+i].r = pal[i*3+2];
                            ginfo->colourtbl[colourstart+i].g = pal[i*3+1];
                            ginfo->colourtbl[colourstart+i].b = pal[i*3+0];
                        }
                        ok = 1;
                    }
                }
            }
        }
        else
        {
            ok = 0;
            printf( "PHOTOSHOP FILTER ERROR -- can't read targa header within INFO call\n" );
        }
    }
    else
    {
        ok = 0;
        printf( "PHOTOSHOP FILTER ERROR -- can't open %s within INFO call\n", (char *)fx->fref );
    }

    if (ginfo && !ok)
    {
        gfree((char *)ginfo);
        ginfo = 0;
    }
    return(ginfo);
}

FPUBLIC int filterphotoshop(FINSTANCE *fx, GINFO *ginfo, char *dest, int rowbytes)
{
    return ( PreparePhotoshopRead( fx->source, ginfo, dest, rowbytes ) );
}

FPUBLIC int closephotoshop(FINSTANCE *fx)
{
    int ok = 0;
    if( fx )
        ok = gfree((void *)fx);
    return(ok);
}

