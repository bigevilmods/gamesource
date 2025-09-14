// (c) 1994-2020 Electronic Arts Inc.

/* format spec */
/* https://www.adobe.com/devnet-apps/photoshop/fileformatashtml/   */



/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*                  PSD -  Adobe Photoshop Gimex                    */
/*                                                                  */
/*            by FrANK G. Barchard & Julian Fong, EAC               */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History:                                       */
/* ------- ----   -- --------                                       */
/*    1.00 941014 FB export based on TGA and Encyclopedia of GFF    */
/*    1.01 941016 FB import                                         */
/*    1.02 950529 FB ginstance signature                            */
/*    1.03 951108 FB import subtype to specify start channel        */
/*    1.04 970514 JF layers support                                 */
/*    1.05 970523 GS wopenpsd returns correct error code            */
/*    1.06 970526 JF import 1 bit PSDs, better error checking       */
/*    1.07 970527 JF negative layers count, raw data. free memory   */
/*    1.08 980305 KO numcolours corrected for 8bit palette          */
/*    1.09 980504 KO Stores path knots in hotspots                  */
/*    1.10 980506 KO Composite image is frame -1.                   */
/*    1.11 980514 KO Path calcs resulted in extra path points       */
/*    1.12 980608 KO Allow for GIMEX_HOTSPOTTBL_SIZE path points    */
/*    1.13 980611 KO Framecount 0 if no layers.                     */
/*    1.14 980823 FB dpi image resource                             */
/*    1.15 000523 FB ginfo respect layer channels                   */
/*    1.16 000623 FB info fail if beyond last frame                 */
/*    1.17 001121 FB close export without any frames                */
/*    1.18 010627 ID keep visible flag and blendmethod              */
/*    1.19 011218 FB long names                                     */
/*    1.20 020123 AE layer names no longer truncated after export   */
/*    1.21 031107 FB support empty layers                           */
/*    1.22 060822 JP canvas size                                    */
/*    1.23 200722 FR psb big file support                           */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* Support 1,3,4 channels of 8 bit                                  */
/* Reentrant                                                        */
/* Files: psdread.cpp psdwrite.cpp psdgimex.h                       */
/*                                                                  */
/* Compresses (packbits code from TIFF) and Decompresses            */
/*                                                                  */
/* PSD supports multiple channels.  The way this gimex module       */
/* supports that is the subtype indicates the number of channels.   */
/* After calling infopsd, set the ginfo subtype to the channel      */
/* to read.                                                         */
/* JF: This is not supported for layered images. Besides, I don't   */
/* think that code worked for images with more than 8 channels.     */
/*                                                                  */
/* Any path resources found in the file will be converted to        */
/* hotspots (Max 256).  Only the knots of the bezier curves of the  */
/* paths are saved. (KO)                                            */
/*                                                                  */
/* The composite flattened image can be retrieved as frame -1 (KO)  */
/*                                                                  */
/* The mode determines the number of channels that will be read.    */
/* Channels are read in order RED, GREEN, BLUE, ALPHA.              */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Format Notes:                                                    */
/* -------------                                                    */
/* 1-32 channels of either 1 or 8 bit                               */
/* packbits compression                                             */
/*                                                                  */
/* For more information on the file format see the Photoshop plugin */
/* SDK.  It contains a PDF describing the file format in detail     */
/*                                                                  */
/* 6 channel greyscale example (chan.psd)                           */
/* --------------------------------------                           */
/* 00000: 38425053 00010000 00000000 00060000  8BPS............     */
/* 00010: 00280000 00a00008 00010000 00000000  .(... ..........     */
/* 00020: 01143842 494d03ed 00000000 00100047  ..8BIM.m.......G     */
/* 00030: f99a0001 00010047 f99a0001 00013842  y......Gy.....8B     */
/* 00040: 494d03ee 00000000 00291042 61726368  IM.n.....).Barch     */
/* 00050: 61726420 4368616e 6e656c06 43616d65  ard Channel.Came     */
/* 00060: 726f0344 6f670645 6e67696d 61054672  ro.Dog.Engima.Fr     */
/* 00070: 414e4b00 3842494d 03ef0000 00000046  ANK.8BIM.o.....F     */
/* 00080: 0000ffff 00000000 00000032 01000000  ...........2....     */
/* 00090: ffff0000 00000000 00320100 0000ffff  .........2......     */
/* 000a0: 00000000 00000032 01000000 ffff0000  .......2........     */
/* 000b0: 00000000 00320100 0000ffff 00000000  .....2..........     */
/* 000c0: 00000032 01003842 494d03f3 00000000  ...2..8BIM.s....     */
/* 000d0: 00080000 00000000 00003842 494d2710  ..........8BIM'.     */
/* 000e0: 00000000 000a0001 00000000 00000002  ................     */
/* 000f0: 3842494d 03f40000 00000012 00350000  8BIM.t.......5..     */
/* 00100: 0001002d 00000006 00000000 00013842  ...-..........8B     */
/* 00110: 494d03f7 00000000 001c0000 ffffffff  IM.w............     */
/* 00120: ffffffff ffffffff ffffffff ffffffff  ................     */
/* 00130: ffff03e8 00000000 00000001 00040004  ...h............     */
/* 00140: 00040004 00040004 00040004 00040004  ................     */
/* 00150: 00040023 00210021 002c0049 00610050  ...#.!.!.,.I.a.P     */
/* 00160: 005f0065 0060005c 0056005c 006b0068  ._.e.`.\.V.\.k.h     */
/* 00170: 0062005c 00600004 00040004 00040004  .b.\.`..........     */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include <string.h>
//#include <stdint.h>
#include "gimex.h"
#include "psdgimex.h"

#define GOFFSETRED      (&((ARGB*)0)->r - (unsigned char*)(0))
#define GOFFSETGREEN    (&((ARGB*)0)->g - (unsigned char*)(0))
#define GOFFSETBLUE     (&((ARGB*)0)->b - (unsigned char*)(0))
#define GOFFSETALPHA    (&((ARGB*)0)->a - (unsigned char*)(0))

/****************************************************************/
/*  Internal Functions                                          */
/****************************************************************/

static int PackBitsDecode(unsigned char *dest, unsigned char *source, int occ)
{
    signed char *op = (signed char*)dest;
    signed char *bp = (signed char*)source;

    while (occ>0)
    {
        int n = (int) *bp++;
        int b;

        if (n < 0)  /* replicate next byte -n+1 times */
        {
            if (n == -128)  /* nop */
                continue;
            n = -n + 1;
            occ -= n;
            for (b = *bp++; n-- > 0;)
                *op++ = b;
        }
        else        /* copy next n+1 bytes literally */
        {
            memcpy(op, bp, ++n);
            op += n; occ -= n;
            bp += n;
        }
    }
    return (1);
}

/* read a variable size field from the stream based on psd version */
static long long getvariablesizefield(GSTREAM* g, int psdversion)
{
    
    if (psdversion == 2) {
        char buf[8];
        gread(g, buf, 8);
        return ggetu64be(buf);
    }
    else
    {
        char buf[4];
        gread(g, buf, 4);
        return ggetu32be(buf);
    }
}

#define PATH_SIZE 26

static void PSD_readresources(GSTREAM *g, PSDINFO *psd, int resourcesize)
{
    int resourceend;
    char buf[8];
    int rid;
    int error = 0;
    int i;

    resourceend = (int)gtell(g) + resourcesize;

    while( (gtell(g) < resourceend) && !error )
    {
        gread(g, buf, 6);
        rid = ggetm(buf+4, 2);

        if (ggetm(buf, 4) == GMAKEID('8','B','I','M'))
        {
            if ( ((rid >= 2000) && (rid < 2999)) || (rid == 1025) )
            {
                /* Skip the name string */
                gread(g, buf, 1);
                gseek(g, gtell(g)+(buf[0]&~1)+1);

                gread(g, buf, 4);
                if (ggetm(buf, 4) % PATH_SIZE == 0)
                {
                    int pathcount;
                    pathcount = ggetm(buf, 4)/PATH_SIZE;

                    for(i=0; (i<pathcount) && !error; i++)
                    {
                        gread(g, buf, 2);
                        switch(ggetm(buf, 2))
                        {
                            case 1:
                            case 2:
                            case 4:
                            case 5:
                                /* Bezier path points */
                                gseek(g, gtell(g)+8);
                                gread(g, buf, 8);
                                gseek(g, gtell(g)+8);
                                psd->hotspottbl[psd->numhotspots][0] = (int)((float)((int)ggetm(buf+4, 4)<<3) / (float)0x8000000 * psd->width);
                                psd->hotspottbl[psd->numhotspots][1] = (int)((float)((int)ggetm(buf  , 4)<<3) / (float)0x8000000 * psd->height);
                                psd->numhotspots++;

                                if (psd->numhotspots == GIMEX_HOTSPOTTBL_SIZE)
                                    error = 1;

                                break;
                            default:
                                gseek(g, gtell(g)+24);
                        }
                    }
                }
                else
                {
                    error = 1;
                }

            }
            else if (rid == 1005)   // dpi
            {
                PSDDpiResource dpiresource;

                /* Skip this resource */

                /* Skip the name string */
                gread(g, buf, 1);
                gseek(g, gtell(g)+(buf[0]&~1)+1);


/* 00022: 3842494d 03ed0000 00000010 00fe0000  8BIM.m.......... */
/* 00032: 00020001 00fe0000 00020001 3842494d  ............8BIM */

                gread(g, buf, 4);   // resource size (16)
                if (ggetm(buf,4)==16)
                {
                    gread(g, &dpiresource, 16);   // resource size (16)
                    psd->dpi = PSDGETFIELD(dpiresource.xdpi);
                }
                else
                {
                    gseek(g, gtell(g) + (ggetm(buf, 4)+1)&~1 );
                }
            }
            else
            {
                /* Skip this resource */

                /* Skip the name string */
                gread(g, buf, 1);
                gseek(g, gtell(g)+(buf[0]&~1)+1);

                gread(g, buf, 4);
                gseek(g, gtell(g) + (ggetm(buf, 4)+1)&~1 );
            }
        }
        else
        {
            error = 1;
        }
    }

    gseek(g,resourceend);
}

#undef PATH_SIZE

/****************************************************************/
/*  Information Functions                                       */
/****************************************************************/

GABOUT * GCALL PSD_about(void)
{
    GABOUT *info;

    info = (GABOUT *) galloc(sizeof(GABOUT));
    if (info)
    {
        memset((void *) info, 0, sizeof(GABOUT));

        info->signature       = GMAKEID('8','B','P','S');
        info->size            = sizeof(GABOUT);
        info->version         = GIMEX_VERSION;
        info->canimport       = 1;    /* supports importing */
        info->canexport       = 1;    /* supports exporting */
        info->importpacked    = 1;    /* supports importing packed */
        info->exportpacked    = 1;    /* supports exporting packed */
        info->import8         = 1;    /* supports importing 8 bit indexed */
        info->export8         = 0;    /* supports exporting 8 bit indexed */
        info->import32        = 1;    /* supports importing 32 bit direct rgb */
        info->export32        = 1;    /* supports exporting 32 bit direct rgb */
        info->multiframe      = 1;    /* supports multiple frames */
        info->multifile       = 0;    /* format requires additional files or resource fork */
        info->multisize       = 1;    /* supports each frame different size */
        info->framebuffer     = 1;    /* module requires memory to buffer entire frame */
        info->external        = 0;    /* module uses external resources */
        info->usesfile        = 1;    /* module is file based */
        info->maxframename    = LAYER_MAXNAME;   /* max frame name character */
        info->defaultquality  = 100;  /* default pack quality */
        info->mactype[0]      = GMAKEID('8','B','P','S');   /* mac file system types used */
        strcpy(info->extensions[0], ".psd");                /* extensions */
        strcpy(info->extensions[1], ".psb");                /* extensions */
        strcpy(info->authorstr,     "Frank, Julian");  /* author */
        strcpy(info->versionstr,    "1.23");                /* version # */
        strcpy(info->shorttypestr,  "PSD");                 /* type */
        strcpy(info->wordtypestr,   "Photoshop");           /* wordtype */
        strcpy(info->longtypestr,   "Adobe Photoshop");     /* longtype */
    }
    return(info);
}

/* check for reasonable header: */
/* 8BPS id */

int GCALL PSD_is(GSTREAM *g)
{
    unsigned char psdheader[4];
    int ok=0;

    gseek(g,(GPOS)0);
    if (gread(g,psdheader,4))
    {
        int sig = ggetm(psdheader, 4);
        
        if (sig == GMAKEID('8', 'B', 'P', 'S')) /* Regular PSD */
            ok = 100;
        else if (sig == GMAKEID('8', 'B', 'P', 'B')) /* Big file PSB */
            ok = 100;
    }
    return(ok);
}


/****************************************************************/
/*  Import Functions                                            */
/****************************************************************/

int GCALL PSD_open(GINSTANCE **gxptr, GSTREAM *g, const char *pathname,bool framecountflag)
{
    PSDHeaderStruct psdheader;
    GINSTANCE *gx;
    int ok=1;
    unsigned char buf[8];
    long long fieldsize, channellength, fulllength;
    PSDINFO * psd = NULL;
    PSDLAYERINFO * layers = NULL;
    signed short int numlayers = 1;
    int i, j;
    signed short channelid;
    long long startextradata, extradatalength;
    long long startpixeldata;
    int key;                    // for photoshop > v.4, keys
    
    int sig;                    // signature

    /* Allocate instance */
    gx = (GINSTANCE *) galloc(sizeof(GINSTANCE));
    if (!gx)
        return 0;

    memset((void *) gx, 0, sizeof(GINSTANCE));
    gx->signature=GMAKEID('8','B','P','S');
    gx->size=sizeof(GINSTANCE);
    gx->gstream = g;
    gx->framenum = -1;
    *gxptr = gx;

    /* Allocate photoshop info */
    gx->gref = galloc(sizeof(PSDINFO));
    if (!gx->gref)
    {
        gfree(gx);
        return 0;
    }
    psd = (PSDINFO *) gx->gref;
    memset((void *) psd, 0, sizeof(PSDINFO));

    /* Read file header */
    gseek(gx->gstream, 0);
    ok = gread(gx->gstream, (unsigned char *) &psdheader, 26);   /* read header */
    if (ok)
    {
        psd->version     = PSDGETFIELD(psdheader.version);
        psd->width       = PSDGETFIELD(psdheader.columns);
        psd->height      = PSDGETFIELD(psdheader.rows);
        psd->channels    = (short) PSDGETFIELD(psdheader.channels);
        psd->depth       = (short) PSDGETFIELD(psdheader.depth);
        psd->mode        = (short) PSDGETFIELD(psdheader.mode);
    }

    /*
     * If mode is something we can't handle, abort now.
     * We are prepared to handle: bitmap, grayscale, indexed, rgb,
     * and duotone (which we treat as grayscale)
     */
    if (psd->mode != 0 && psd->mode != 1 && psd->mode != 2 &&
        psd->mode != 3 && psd->mode != 4 && psd->mode != 8)
    {
        gfree(psd);
        gfree(gx);
        return 0;
    }

    /* Read palette */
    ok = gread(g, buf, 4);
    fieldsize = ggetm(buf, 4);
    if (fieldsize)
        ok = gread(g, psd->pal, (int)fieldsize);

    /* Image resources */
    ok = gread(g, buf, 4);
    fieldsize = ggetm(buf,4);
    PSD_readresources(g, psd, (int)fieldsize);

    /* Read length of misc info section */
    fieldsize = getvariablesizefield(g, psd->version);  /* 8 bytes for **PSB** */
    
    /*
     * If no misc info section, no layers, skip ahead and get the
     * image offset
     */
    if (fieldsize == 0)
    {
        numlayers = 1;
        psd->layerinfo = NULL;
        psd->imgoffset = gtell(g);
    }
    else
    {
        /* Store offset of composite image */
        psd->imgoffset = gtell(g) + fieldsize;

        /* 16 bit psd files have an extra little bit of info 
           here before the layer info stuff starts.  Haven't
           figured out what exactly it is, but we skip over it.
         */
        if (psd->depth == 16)
        {
            ok = 1;
            long long cpos = gtell(g);
            // skip over two 32bit 0s.
            gseek(g, cpos + 8);

            gread(g, buf, 4);
            int sig = ggetm(buf, 4);
            if (sig == GMAKEID('8', 'B', 'I', 'M'))
            {
                gread(g, buf, 4);
                sig = ggetm(buf, 4);
                if (!(sig == GMAKEID('L', 'r', '1', '6') || sig == GMAKEID('F', 'M', 's', 'k')))
                {
                    ok = 0;
                }
                else
                {
                    ok = 0;
                }
                if (!ok) // didn't get what we expect, so go back and try as we o/w would have.
                {
                    gseek(g, cpos);
                }

            }
        }


        /* Read length of layers info section */
        fieldsize = getvariablesizefield(g, psd->version); /* 8 bytes for **PSB** */
                  

        /* Read layers count */
        ok = gread(g, buf, 2);
        numlayers = (signed short int) ggetm(buf, 2);
        if (numlayers < 0)
            numlayers = -numlayers;

        if(numlayers)
            psd->layerinfo = (PSDLAYERINFO *) galloc(numlayers * sizeof(PSDLAYERINFO));
        else
        {
psd->layerinfo = 0;
numlayers = 1;
        }

        if (psd->layerinfo)
        {
            layers = (PSDLAYERINFO*)psd->layerinfo;
            memset((void*)layers, 0, numlayers * sizeof(PSDLAYERINFO));

            fulllength = 0;

            for (i = 0; i < numlayers; i++)
            {
                /* Read top, left, etc */
                ok = gread(g, buf, 4);
                layers[i].top = ggetm(buf, 4);
                ok = gread(g, buf, 4);
                layers[i].left = ggetm(buf, 4);
                ok = gread(g, buf, 4);
                layers[i].bottom = ggetm(buf, 4);
                ok = gread(g, buf, 4);
                layers[i].right = ggetm(buf, 4);
                ok = gread(g, buf, 2);
                /* Read number of channels */
                layers[i].channels = (short)ggetm(buf, 2);

                for (j = 0; j < 4; j++)
                {
                    layers[i].offsets[j] = -1;
                }

                /* Read channel length info structures */
                for (j = 0; j < layers[i].channels; j++)
                {
                    ok = gread(g, buf, 2);
                    channelid = (signed short)ggetm(buf, 2);

                    channellength = getvariablesizefield(g, psd->version);

                    if (channelid == -1)
                        layers[i].offsets[3] = fulllength;
                    else if (channelid >= 0 && channelid < 3)
                        layers[i].offsets[channelid] = fulllength;

                    fulllength += channellength;
                }
                /* Skip blend mode signature, key, opacity, clipping,
                flags, filler */
                //gseek(g, gtell(g) + 12);
                gseek(g, gtell(g) + 4);            // skip over blend signature.
                ok = gread(g, buf, 4);             // read in blend method.
                layers[i].blendmethod = ggetm(buf, 4);
                ok = gread(g, buf, 1);             // read in opacity level.
                layers[i].opacity = ggetm(buf, 1);
                gseek(g, gtell(g) + 1);            // skip clipping info.
                ok = gread(g, buf, 1);             // read in flags info.
                layers[i].flags = ggetm(buf, 1);
                gseek(g, gtell(g) + 1);            // skip filler.
                /* Read extra data size */
                ok = gread(g, buf, 4);
                extradatalength = ggetm(buf, 4);
                startextradata = (int)gtell(g);

                /* Read layer mask data size and skip it */
                ok = gread(g, buf, 4);
                fieldsize = ggetm(buf, 4);
                gseek(g, gtell(g) + fieldsize);

                /* Read layer blending ranges data size and skip it */
                ok = gread(g, buf, 4);
                fieldsize = ggetm(buf, 4);
                gseek(g, gtell(g) + fieldsize);

                /* Read layer name */
                ok = gread(g, buf, 1);
                if (buf[0]) {
                    ok = gread(g, layers[i].name, buf[0]);
                    /* pad to word alignment */
                    int namelen = strlen(layers[i].name);
                    gseek(g, gtell(g) + ((namelen + 1) & ~1) - (namelen-1));
                }


                // Starting in Photoshop 4, there are more possible misc. resources
                // that may go after this point. Photoshop 5 and 6 each introduced even
                // more. See Photoshop file formats spec, for more details.
                
                while (gtell(g) < startextradata + extradatalength)
                {
                    /* read signature */
                    ok = gread(g, buf, 4);
                    sig = ggetm(buf, 4);

                    /* read key */
                    ok = gread(g, buf, 4);
                    key = ggetm(buf, 4);

                    /* Read the length of the field */
                    ok = gread(g, buf, 4);
                    int length = ggetm(buf, 4);

                    long long start = gtell(g);

                    if (sig != GMAKEID('8', 'B', 'I', 'M'))
                    {
                        /* 
                           Not good, we're lost, don't bother reading the rest of the blocks
                           Skip this block read loop and jump to the start of the pixel data.
                        */
                        break;
                    }

                    if (key == GMAKEID('l', 's', 'c', 't')) /* Layer set control flag */
                    {
                        ok = gread(g, buf, 4);
                        layers[i].layerset = ggetm(buf, 4);
                    }
                    else if (key == GMAKEID('l', 'u', 'n', 'i'))    // Unicode layer name key
                    {
                        // temp unicode name var
                        char namet[2 * LAYER_MAXNAME]; // temp unicode name var
                       
                        /* Read in Unicode layer name length*/
                        ok = gread(g, buf, 4);
                        int namelen = 2 * ggetm(buf, 4); // times 2 because its unicode

                        /* Read Unicode name into namet*/
                        gread(g, namet, namelen);

                        /* store name as an ASCII string (ie. convert to ASCII from unicode) */
                        for (j = 0; j < namelen / 2; j++)
                            layers[i].name[j] = namet[2 * j + 1];
                    }

                    // align
                    gseek(g, start + length);

                }
            

                /* Skip to end of extra data section (due to padding) */
                gseek(g, startextradata + extradatalength);
            } /* end layer loop */

            /* Get where the pixel data is */
            startpixeldata = (int)gtell(g);

            /* Add this to offsets */
            for (i = 0; i < numlayers; i++)
            {
                for (j = 0; j < 4; j++)
                {
                    if (layers[i].offsets[j] >= 0)
                        layers[i].offsets[j] += startpixeldata;
                }
            }
        } /* layer info alloced */
    } /* has layers */

    if (ok == 0)
        return (ok);
    else
    {
        gx->frames = numlayers;
        return(numlayers);
    }
}


GINFO * GCALL PSD_info(GINSTANCE *gx, int framenum)
{
    PSDINFO * psd;
    GINFOPSD * ginfopsd;
    GINFO *ginfo=0;
    int bpp;
    int colours=0;
    int i;
    int ok = 1;
    int alphabits;
    int channels;

    psd = (PSDINFO *) gx->gref;
    if (framenum<gx->frames)
        ginfo    = (GINFO *) galloc(sizeof(GINFOPSD));

    ginfopsd = (GINFOPSD *)ginfo;
    if (ginfo)
    {
        memset((void *) ginfo, 0, sizeof(GINFO));

        /* template for ginfo */

        ginfo->signature        = GMAKEID('8','B','P','S'); /* Type of file */
        ginfo->size             = sizeof(GINFOPSD); /* Length of GINFO */
        ginfo->version          = GIMEX_VERSION;
        ginfo->framenum         = framenum;      /* Current frame num */
        ginfo->subtype          = psd->channels;  /* subtype */
        ginfo->packed           = 0;            /* hotoshop may mix between RLE and non-RLE between channels) */
        ginfo->quality          = 100;          /* Quality of lossy packing 0..100 */
        ginfo->numhotspots      = 0;            /* Number of Hot Spots defined */
        ginfo->centerx          = 0;            /* Center Point relative to upper left corner */
        ginfo->centery          = 0;

        ginfo->dpi              = (float)((double)psd->dpi/65536.0);

        ginfo->numhotspots = psd->numhotspots;
        memcpy(ginfo->hotspottbl, psd->hotspottbl, GIMEX_HOTSPOTTBL_SIZE*GIMEX_HOTSPOTTBL_VALUES*sizeof(int));

        /* Must have layers and starting at frame 0, frame -1 is the composite image */
        if (psd->layerinfo && (framenum != -1))
        {
            strncpy(ginfo->framename, psd->layerinfo[framenum].name, LAYER_MAXNAME-1);
            ginfo->defaultx         = psd->layerinfo[framenum].left;
            ginfo->defaulty         = psd->layerinfo[framenum].top;
            ginfo->width            = psd->layerinfo[framenum].right  - psd->layerinfo[framenum].left;
            ginfo->height           = psd->layerinfo[framenum].bottom - psd->layerinfo[framenum].top;
            channels                = psd->layerinfo[framenum].channels;
            ginfopsd->blendmethod   = psd->layerinfo[framenum].blendmethod;
            ginfopsd->opacitylevel  = psd->layerinfo[framenum].opacity;
            ginfopsd->visibleflag   = (psd->layerinfo[framenum].flags&0x02)?0:1;
// PAULDING START
            ginfopsd->canvasw       = psd->width;
            ginfopsd->canvash       = psd->height;
// PAULDING END
        }
        else
        {
            ginfo->framename[0]     = 0; /* Null terminated name of frame/image */
            ginfo->defaultx         = 0;
            ginfo->defaulty         = 0;
            ginfo->width            = psd->width; /* Width of bitmap in pixels */
            ginfo->height           = psd->height;/* Height of bitmap in pixels */
            channels                = psd->channels;
        }

        if (!ginfo->width && !ginfo->height)    // empty layer is 0x0.  Make it 1x1
        {
            ginfo->width = 1;
            ginfo->height = 1;
        }

        alphabits = (channels>=4)?8:0;

        ginfo->alphabits        = alphabits;    /* Number of bits in alpha channel */
        ginfo->redbits          = 8;            /* Number of bits in red channel */
        ginfo->greenbits        = 8;            /* Number of bits in green channel */
        ginfo->bluebits         = 8;            /* Number of bits in blue channel */

        /* 24,32 bpp true colour */
        if (psd->mode==3)    /* argb */
        {
            /* GS: restrict to 32, values > 32 just confuse users */
            /* FB: ignore channel depth */
//            bpp = gmin(psd->channels*psd->depth,32);
            bpp = gmin(channels*8,32);

            ginfo->bpp              = 32;       /* Bits per pixel (8 or 32) */
            ginfo->originalbpp      = bpp;
            ginfo->numcolours       = 0;        /* Number of colours in original indexed palette */
            if (bpp<=16)
                ginfo->redbits = ginfo->greenbits = ginfo->bluebits = gmin(8,psd->depth);
        }
        /* 8 grey or colour, with 24 bit palette */
        else
        {
            switch (psd->mode)
            {
                case 0: colours = 2; break;                  /* bitmap */
                case 1: colours = 1<<psd->depth; break;      /* gray scale */
                case 2: colours = 1<<psd->depth; break;      /* indexed */
            }

            if (!colours)    colours = (1 << psd->depth);
            if (colours>256) colours = 256;

            ginfo->bpp              = 8;                        /* Bits per pixel (8 or 32) */
            ginfo->originalbpp      = gmin(8,psd->depth);
            ginfo->numcolours       = colours;                  /* Number of colours in original indexed palette */

            if (psd->mode==2)
            {
                for (i=0; i<colours; ++i)
                {
                    ginfo->colourtbl[i].a = 0xff;
                    ginfo->colourtbl[i].r = psd->pal[i+0];
                    ginfo->colourtbl[i].g = psd->pal[i+256];
                    ginfo->colourtbl[i].b = psd->pal[i+512];
                }
            }
            else /* grey scale and bitmap */
            {
                for (i=0; i<colours; ++i)
                {
                    ginfo->colourtbl[i].a = 0xff;
                    ginfo->colourtbl[i].r = i*255/(colours-1);
                    ginfo->colourtbl[i].g = i*255/(colours-1);
                    ginfo->colourtbl[i].b = i*255/(colours-1);
                }
            }
        }
    }
    if (ginfo && !ok)
    {
        gfree(ginfo);
        ginfo = 0;
    }

    return(ginfo);
}

int GCALL PSD_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes)
{
    PSDINFO * psd;
    int ok = 1;
    int i, j, k;
    unsigned char * packlinebuf = 0;
    unsigned char * linebuf     = 0;
    unsigned char * rowtbl      = 0;
    unsigned char * lineptr     = 0;
    unsigned char * d           = 0;
    int packed;
    int planerowsize;
    int width;
    int height;
    int channels;
    int skipchannels;
    int readchannels;
    int planeoffsets[4] = {GOFFSETRED, GOFFSETGREEN, GOFFSETBLUE, GOFFSETALPHA};
    char buf[4];
    int layernum;

    /* -1 is a valid framenumber.  It will return the flattened composite image */
    if (ginfo->framenum < -1 || ginfo->framenum >= gx->frames)
        return (0);

    psd = (PSDINFO *) gx->gref;

    /* has layers */
    if (psd->layerinfo && (ginfo->framenum >= 0))
    {
        layernum = ginfo->framenum;

        /* Can't use raw psd->width etc since layers may have
        transparent areas cropped */
        width = psd->layerinfo[layernum].right -
            psd->layerinfo[layernum].left;
        height = psd->layerinfo[layernum].bottom -
            psd->layerinfo[layernum].top;
        channels = psd->layerinfo[layernum].channels;

        if (!width && !height)  // return empty layer as transparent pixel
        {
            if (ginfo->bpp==32)
                memset(dest,0,sizeof(ARGB));
            else
                dest[0] = 0;
            return 1;
        }


        /* line buffer */
        linebuf = (unsigned char *) galloc(width);
        if (!linebuf)
            return 0;

        packlinebuf = (unsigned char *) galloc(width*2+100);
        if (!packlinebuf)
        {
            gfree(linebuf);
            return 0;
        }

        for (k = 0; k < 4; k++)
        {
            /* Pad the channel */
            if (psd->layerinfo[layernum].offsets[k] == -1)
            {
                for (i = 0; i < height; i++)
                {
                    d = (unsigned char *) dest + i * rowbytes;

                    if (psd->mode==3)
                    {
                        d += planeoffsets[k];

                        for (j=0; j<width; ++j)
                        {
                            *d = (unsigned char) 0xFF;
                            d += 4;
                        }
                    }
                    /* indexed */
                    else if (psd->mode == 1 || psd->mode == 2 || psd->mode == 8)
                    {
                        for (j=0; j<width; ++j)
                        {
                            *d = (unsigned char) 0xFF;
                        }
                    }
                    /* bitmap */
                    else if (psd->mode == 0)
                    {
                        for (j=0; j<width; ++j)
                        {
                            *d++ = (unsigned char) 0xFF;
                        }
                    }
                }
                continue;
            }

            /* Start reading channel at offset */
            gseek(gx->gstream, psd->layerinfo[layernum].offsets[k]);

            /* Read packed status */
            gread(gx->gstream, buf, 2);
            packed = (1 == ggetm(buf, 2));
            if (packed)
            {
                /* Read channel row byte lengths */
                /* the size is version dependent PSD/PSB */
                int szbytes = (psd->version == 2) ? 4 : 2;
                rowtbl = (unsigned char *) galloc(height * szbytes);
                if (!rowtbl)
                {
                    gfree(linebuf);
                    gfree(packlinebuf);
                    return 0;
                }
                
                gread(gx->gstream, (void *) rowtbl, height * szbytes);

                for (i = 0; i < height; i++)
                {
                    
                    planerowsize = ggetm(rowtbl + i * szbytes, szbytes);

                    /* Read packed line */
                    gread(gx->gstream, packlinebuf, planerowsize);

                    /* Decompress packlinebuf */
                    PackBitsDecode(linebuf, packlinebuf, width);

                    /* Copy into destination */
                    lineptr = linebuf;
                    d = (unsigned char *) dest + i * rowbytes;
                    if (psd->mode==3)
                    {
                        d += planeoffsets[k];

                        for (j=0; j<width; ++j)
                        {
                            *d = *lineptr++;
                            d += 4;
                        }
                    }
                    /* indexed */
                    else if (psd->mode == 1 || psd->mode == 2 || psd->mode == 8)
                    {
                        for (j=0; j<width; ++j)
                        {
                            *d++ = *lineptr++;
                        }
                    }
                    else if (psd->mode == 0)
                    {
                        for (j=0; j<width; ++j)
                        {
                            *d++ = 1 - ((*lineptr >> (7 - (j&7))) & 1);
                            if ((j & 7) == 7)
                                ++lineptr;
                        }
                    }
                }
                gfree(rowtbl);
            }
            else
            {
                for (i = 0; i < height; i++)
                {
                    /* Read uncompressed line */
                    gread(gx->gstream, linebuf, width);
                    /* Copy into destination */
                    lineptr = linebuf;
                    d = (unsigned char *) dest + i * rowbytes;

                    if (psd->mode==3)
                    {
                        d += planeoffsets[k];

                        for (j=0; j<width; ++j)
                        {
                            *d = *lineptr++;
                            d += 4;
                        }
                    }
                    /* indexed */
                    else if (psd->mode == 1 || psd->mode == 2 || psd->mode == 8)
                    {
                        for (j=0; j<width; ++j)
                        {
                            *d++ = *lineptr++;
                        }
                    }
                    else if (psd->mode == 0)
                    {
                        for (j=0; j<width; ++j)
                        {
                            *d++ = 1 - ((*lineptr >> (7 - (j&7))) & 1);
                            if ((j & 7) == 7)
                                ++lineptr;
                        }
                    }
                }
            }
        }
        gfree(linebuf);
        gfree(packlinebuf);
    } /* End of layer handling */

    /* This is the old psd handling code */
    else
    {
        gseek(gx->gstream, psd->imgoffset);
        gread(gx->gstream, buf, 2);
        packed = (1 == ggetm(buf, 2));

        skipchannels = ginfo->subtype % psd->channels;

        /* RGB */
        if (psd->mode==3)
        {
            readchannels = gmin(4,psd->channels-skipchannels);
            if (psd->channels < 4)
            {
                for (i=0; i<psd->height; ++i)
                    memset(dest+i*rowbytes,255,psd->width*4);
            }
        }
        /* All others */
        else
            readchannels = 1;

        linebuf = (unsigned char *) galloc(psd->width);
        if (!linebuf)
            return 0;
        if (packed)
        {
            packlinebuf = (unsigned char *) galloc(psd->width*2+100);
            if (!packlinebuf)
            {
                gfree(linebuf);
                return 0;
            }
            rowtbl = (unsigned char *) galloc(psd->height*psd->channels*2);
            if (!rowtbl)
            {
                gfree(linebuf);
                gfree(packlinebuf);
                return 0;
            }
            ok = gread(gx->gstream,rowtbl, psd->height*psd->channels*2);
        }

        for (i=0; i<(skipchannels+readchannels); ++i)
        {
            for (j=0; j<psd->height && ok; ++j)
            {
                lineptr = linebuf;
                d = (unsigned char *)dest+j*rowbytes;
                if (packed)
                {
                    planerowsize = ggetm(rowtbl+i*(psd->height*2)+j*2,2);
                    ok = gread(gx->gstream, packlinebuf, planerowsize);
                    PackBitsDecode(linebuf, packlinebuf, psd->width * psd->depth / 8);
                }
                else
                {
                    gread(gx->gstream, linebuf, psd->width * psd->depth / 8);
                }
                if (i >= skipchannels && j < psd->height)
                {
                    if (psd->mode==3)
                    {
                        d += planeoffsets[i-skipchannels];

                        for (k=0; k<psd->width; ++k)
                        {
                            *d = *lineptr++;
                            d += 4;
                        }
                    }
                    /* indexed */
                    else if (psd->mode == 1 || psd->mode == 2 || psd->mode == 8)
                    {
                        for (k=0; k<psd->width; ++k)
                        {
                            *d++ = *lineptr++;
                        }
                    }
                    /* bitmap */
                    else if (psd->mode == 0)
                    {
                        for (k=0; k<psd->width; ++k)
                        {
                            *d++ = 1 - ((*lineptr >> (7 - (k&7))) & 1);
                            if ((k & 7) == 7)
                                ++lineptr;
                        }
                    }
                } /* Row loop */
            } /* Height loop */
        } /* End channel loop */
        if (packed)
        {
            gfree(rowtbl);
            gfree(packlinebuf);
        }
        gfree(linebuf);
    }
    return(ok);
}

int GCALL PSD_close(GINSTANCE *gx)
{
    PSDINFO * psd;
    bool ok = true;

    if (gx)
    {
        if (gx->gref)
        {
            psd = (PSDINFO *) gx->gref;
            if (psd->layerinfo)
                ok &= gfree(psd->layerinfo);
            ok &= gfree(psd);
        }
        ok &= gfree(gx);
    }

    return(ok);
}
