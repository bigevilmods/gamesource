// (c) 1994-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*                       GMX - Gimex File Format                    */
/*                                                                  */
/*                      by Andrew Brownsword, EAC                   */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History:                                       */
/* ------- ----   -- --------                                       */
/*    1.00 941103 AB created from MVI gimex                         */
/*    1.10 941202 AB return codes,offset tbl,save full ginfo        */
/*    1.11 941202 FB notes                                          */
/*    2.00 950316 AB not backwardly compatible format (see below)   */
/*    2.01 950317 FB array style copy bitmap                        */
/*    2.02 950320 GS removed 2.0 from long name                     */
/*    2.03 950525 GS clear pack buffer before using                 */
/*    2.04 950529 FB ginstance signature                            */
/*    2.05 950911 FB read/write old ginfo into Gimex 2.0 structure  */
/*    2.06 951010 GS saves subtype value on export                  */
/*    2.07 951024 FB canimport reading framename, export use strcpy */
/*    2.08 951025 FB limit hot spots to 32 (error tolerance)        */
/*    2.09 951106 GS fixed bug with hotspots                        */
/*    2.10 961007 FB huf codex takes *opts now                      */
/*    2.11 961220 GS added comment support                          */
/*    2.12 970910 FB open check for zeroed section header           */
/*    2.13 980109 KO fixed defaulty in frame header.                */
/*    2.14 990729 FB GMX2 info includes fps                         */
/*    2.15 001128 FB 64 bit 'section' header, 64 bit offsets        */
/*    2.16 001204 FB close alloc 12 not 8 bytes on stack            */
/*    2.17 001205 FB version in section header (G235)               */
/*    2.18 010401 FB ignore huff size, allowing more than 8 MB G218 */
/*    2.19 010809 FB if alphabits=0, stamp 255 into alpha channel   */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 8/32 bit multiframe                                              */
/* Reentrant                                                        */
/* Files: gmxread.cpp gmxwrite.cpp gmxgimex.h                           */
/*                                                                  */
/* Chunk based format for saving Gimex images.                      */
/*                                                                  */
/* Defined using GINFO which uses ints, so wont work on 16 or 64    */
/* bit platforms.                                                   */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Format Notes:                                                    */
/* -------------                                                    */
/* A GMX file may contain 2 types of chunks, sections and frames.   */
/* Other chunk types are ignored.  The section chunks indicate the  */
/* size of a group of frames and the number of frames therein.      */
/* Section headers are optional and are ignored if they appear to   */
/* not be correct.                                                  */
/*                                                                  */
/* The GMX section chunks are as follows:                           */
/*  <size of section>                                               */
/*  <number of frames>                                              */
/*  <high 32 bits of size>                                          */
/*  <version of GMX> 'G217'                                         */
/*                                                                  */
/* The GMX frame chunks are as follows:                             */
/*  <type> <size> <hdr size> <packed GINFO> <packed frame data>     */
/*                                                                  */
/* All fields are stored in little-endian format, sizes are 4 bytes.*/
/*                                                                  */
/* NOTE: An error in the original frame header made the             */
/*       defaulty 16 bytes instead of 4.  This is fixed but the     */
/*       importer can handle both types.                            */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include <string.h>
#include "gimex.h"
#include "gmxgimex.h"
#include "codex.h"
#include "huffcodex.h"

#define kOffsetAllocInc 256
typedef struct {
    GPOS current;
    unsigned int numoffsets;     /* # entries in offsets array (allocated in blocks of 256) */
    GPOS *offsets;
} GMXReadData;


static void addgmxframe (GMXReadData* rd, GPOS newframepos)
{
    /* increase size of offsets array, if necessary */
    if ((rd->numoffsets % kOffsetAllocInc) == 0)
    {
        unsigned int newcount = (rd->numoffsets / kOffsetAllocInc + 1) * kOffsetAllocInc;
        GPOS * newoffsets = (GPOS *) galloc(newcount * sizeof(GPOS));

        if (rd->numoffsets)
        {   /* if there WERE any offsets, copy into new array and deallocate old array */
            memcpy((char*)newoffsets, (char*)rd->offsets, rd->numoffsets * sizeof(GPOS));
            gfree(rd->offsets);
        }

        rd->offsets = newoffsets;
    }

    /* append new offset, and increase number of offsets */
    rd->offsets[rd->numoffsets++] = newframepos;
}


static int seeknextchunk (GSTREAM *g, GPOS position, unsigned int chunktype)
{
    unsigned char chunk[8];
    if (gseek(g, position) && gread(g, chunk, 8))
    {
        /* skip this chunk */
        position += ggeti(chunk + 4, 4);

        /* seek & read until we can seek and read no more... */
        while (gseek(g, position) && gread(g, chunk, 8))
        {
            /* if we found the right chunk type seek to chunk start and return success */
            if (ggetm(chunk, 4) == chunktype)
            {
                gseek(g, position);
                return 1;
            }

            /* wrong chunk type, skip it */
            position += ggeti(chunk + 4, 4);
        }
    }

    return 0;
}


static int seektoframe (GINSTANCE *gx, int framenum)
{
    int ok = 0;
    GMXReadData* rd = (GMXReadData*)gx->gref;

    if (framenum == gx->framenum)
    {   /* we are AT the frame requested */
        ok = 1;
    }
    else if (gx->frames <= framenum)
    {   /* frame past end of file requested */
        ok = 0;
    }
    else if ((int)rd->numoffsets > framenum)
    {   /* we've got an offset for the requested frame */
        rd->current = rd->offsets[framenum];
        gx->framenum = framenum;
        ok = 1;
    }
    else
    {
        if (rd->numoffsets > 0)
        {   /* seek to last frame we have an offset for */
            seektoframe(gx, rd->numoffsets - 1);
        }
        else
        {   /* no offsets, seek to start of file */
            char chunk[8];

            rd->current = 0;
            gx->framenum = -1;
            gseek(gx->gstream, 0);

            /* detect a frame at position zero */
            gread(gx->gstream, chunk, 8);
            if (ggetm(chunk, 4) == kGMXFrame)
            {
                gx->framenum = 0;
                addgmxframe(rd, 0);
            }

            gseek(gx->gstream, 0);
        }

        ok = 1;

        /* advance from frame chunk to frame chunk until we reach the requested frame */
        while (gx->framenum < framenum)
        {
            if (!seeknextchunk(gx->gstream, rd->current, kGMXFrame))
            {   /* there are fewer frames than we expected, correct the count */
                gx->frames = gx->framenum + 1;
                gx->framenum = -1;
                gseek(gx->gstream, 0);

                ok = 0;
            }

            rd->current = gtell(gx->gstream);

            if (ok)
            {
                gx->framenum += 1;
                addgmxframe(rd, rd->current);
            }
        }
    }

    return gseek(gx->gstream, rd->current) && ok;
}

/****************************************************************/
/*  Information Functions                                       */
/****************************************************************/

GABOUT * GCALL GMX_about(void)
{
    GABOUT *info;

    info = (GABOUT *) galloc(sizeof(GABOUT));
    if (info)
    {
        memset(info, 0, sizeof(GABOUT));

        info->signature       = GMAKEID('G','I','M','X');
        info->size            = sizeof(GABOUT);
        info->version         = GIMEX_VERSION;
        info->canimport       = 1;    /* supports importing */
        info->canexport       = 1;    /* supports exporting */
        info->import8         = 1;    /* supports importing 8 bit indexed */
        info->export8         = 1;    /* supports exporting 8 bit indexed */
        info->import32        = 1;    /* supports importing 32 bit direct rgb */
        info->export32        = 1;    /* supports exporting 32 bit direct rgb */
        info->multiframe      = 1;    /* supports multiple frames */
        info->multifile       = 0;    /* format requires additional files or resource fork */
        info->multisize       = 1;    /* supports each frame different size */
        info->framebuffer     = 1;    /* module requires memory to buffer entire frame */
        info->external        = 0;    /* module uses external resources */
        info->usesfile        = 1;    /* module is file based */
        info->globalpalette   = 0;    /* one palette per file */
        info->file64          = 1;    /* this format supports 64 bit file sizes */
        info->maxframename    = 255;  /* max frame name character */
        info->defaultquality  = 100;  /* default pack quality */
        info->mactype[0]      = GMAKEID('G','I','M','X');   /* mac file system types used */
        strcpy(info->extensions[0], ".gmx");                /* extensions */
        strcpy(info->authorstr,     "F.Barchard and A.Brownsword");        /* author */
        strcpy(info->versionstr,    "2.19");                /* version # */
        strcpy(info->shorttypestr,  "GMX");                 /* type */
        strcpy(info->wordtypestr,   "Gimex");               /* wordtype */
        strcpy(info->longtypestr,   "Gimex Images");        /* longtype */
    }
    return(info);
}

/* check for reasonable header:  kGMXFrame */

int GCALL GMX_is(GSTREAM *g)
{
    unsigned char chunk[4];
    int ok=0;

    gseek(g,(GPOS)0);
    if (gread(g,chunk,4))
    {
        if ((ggetm(chunk,4) == kGMXFrame) || (ggetm(chunk,4) == kGMXSection))
            ok = 100;
    }
    return(ok);
}


/****************************************************************/
/*  Import Functions                                            */
/****************************************************************/

int GCALL GMX_open(GINSTANCE **gxptr, GSTREAM *g, const char *pathname,bool framecountflag)
{
    int frames = 0;
    int ok;
    GINSTANCE *gx = (GINSTANCE *)galloc(sizeof(GINSTANCE));
    GMXReadData *rd = (GMXReadData*)galloc(sizeof(GMXReadData));

    if (gx && rd)
    {
        GPOS sectionstart = 0;
        char gmxheader[kGMXsectionSize];
        GPOS streamlen = glen(g);

        /* initialize our GINSTANCE & internal data */
        memset(gx, 0, sizeof(GINSTANCE));
        memset(rd, 0, sizeof(GMXReadData));

        gx->frames = 0;
        gx->signature=GMAKEID('G','I','M','X');
        gx->size=sizeof(GINSTANCE);
        gx->framenum = -1;
        gx->gstream = g;
        gx->gref = (char*)rd;

        /* count number of frames by sections (if any) */

        do
        {
            GPOS sectionlen;

            gseek(gx->gstream, sectionstart);
            ok = gread(gx->gstream, gmxheader, kGMXsectionSize);
            if (ok)
            {
                if (ggeti(gmxheader+4,4)<=16)
                    sectionlen = ggeti(gmxheader+8,4);  // 32 bit
                else if (ggeti(gmxheader+4,4)<=20)  // 20 byte style
                    sectionlen = (GPOS)ggeti(gmxheader+8,4)|(((GPOS)ggeti(gmxheader+16,4))<<32); // 64 bit
                else // 24 byte section
                    sectionlen = (GPOS)ggeti(gmxheader+8,4)|(((GPOS)ggeti(gmxheader+20,4))<<32); // 64 bit

                if (sectionlen>0)
                {
                    frames += ggeti(gmxheader+12,4);
                    sectionstart += sectionlen;
                }
            }

        } while (ok && (sectionstart<streamlen));

        /* didn't get a good frame count by sections, so count individual frame chunks */
        if (!ok)
        {
            GPOS position = 0;
            frames = 0;
            while (seeknextchunk(gx->gstream, position, kGMXFrame))
            {
                position = gtell(gx->gstream);
                addgmxframe(rd, position);
                frames += 1;
            }
        }

        /* record frame count and seek back to start */
        gseek(gx->gstream, 0);
        gx->frames = frames;
    }
    else if (gx)
    {
        /* galloc(gx) succeeded, but galloc(rd) failed */
        gfree(gx);
        gx = NULL;
    }

    *gxptr = gx;
    return frames;
}


GINFO * GCALL GMX_info(GINSTANCE *gx, int framenum)
{
    GINFO * ginfo = NULL;
    char *  packginfo = NULL;
    V3GMXHeaderStruct *finfo = NULL;
    unsigned int ginfosize;
    char gmxframe[kGMXheaderSize];
    int i;
    int ok=0;

    if (seektoframe(gx, framenum))
    {
        /* read header and allocate unpack buffer */

        gread(gx->gstream, gmxframe, kGMXheaderSize);
        ginfosize = ggeti(gmxframe+8, 4);
        packginfo = (char *)galloc(ginfosize);

        if (packginfo)        /* read and decode header */
        {
            if (gread(gx->gstream, packginfo, ginfosize))   /* read the packed GINFO block into memory */
            {
                int gallocsize;
                ginfosize = HUFF_size(packginfo);

                /* when a field is added the value will be set to zero since the finfo block is cleared */
                /* added when comments were implemented */
                if (ginfosize < sizeof(V3GMXHeaderStruct))
                    gallocsize = sizeof(V3GMXHeaderStruct);
                else
                    gallocsize = ginfosize;

                finfo = (V3GMXHeaderStruct *) galloc(gallocsize);  /* allocate FINFO block */
                if (finfo)
                {
                    memset(finfo, 0, gallocsize);
                    ok = 1;
                }
            }
        }
    }

    if (ok)
    {
        int decodesize;

        /* If the header is the old version, move the fields */
        decodesize = HUFF_decode((void *)finfo,packginfo, NULL);
        if (decodesize == sizeof(V1GMXHeaderStruct) )
        {
            V3GMXHeaderStruct *ninfo = (V3GMXHeaderStruct *)galloc(sizeof(V3GMXHeaderStruct));
            V1GMXHeaderStruct *oldinfo = (V1GMXHeaderStruct *)finfo;

            #define COPYFIELD(ffield,oldfield) memcpy(ffield,oldfield,sizeof(oldfield));

            memset(ninfo,0,sizeof(V3GMXHeaderStruct));
            COPYFIELD(ninfo->signature,oldinfo->signature);
            COPYFIELD(ninfo->size,oldinfo->size);
            COPYFIELD(ninfo->version,oldinfo->version);
            COPYFIELD(ninfo->framenum,oldinfo->framenum);
            COPYFIELD(ninfo->width,oldinfo->width);
            COPYFIELD(ninfo->height,oldinfo->height);
            COPYFIELD(ninfo->bpp,oldinfo->bpp);
            COPYFIELD(ninfo->originalbpp,oldinfo->originalbpp);
            COPYFIELD(ninfo->numcolours,oldinfo->originalcolours);
            COPYFIELD(ninfo->colourtbl,oldinfo->colourtbl);
            COPYFIELD(ninfo->subtype,oldinfo->subtype);
            COPYFIELD(ninfo->packed,oldinfo->packed);
            COPYFIELD(ninfo->quality,oldinfo->quality);
            COPYFIELD(ninfo->alphabits,oldinfo->alphabits);
            COPYFIELD(ninfo->redbits,oldinfo->redbits);
            COPYFIELD(ninfo->greenbits,oldinfo->greenbits);
            COPYFIELD(ninfo->bluebits,oldinfo->bluebits);
            COPYFIELD(ninfo->centerx,oldinfo->centerx);
            COPYFIELD(ninfo->centery,oldinfo->centery);
            COPYFIELD(ninfo->defaultx,oldinfo->defaultx);
            COPYFIELD(ninfo->defaulty,oldinfo->defaulty);
            COPYFIELD(ninfo->numhotspots,oldinfo->numhotspots);
            COPYFIELD(ninfo->framename,oldinfo->framename);
            COPYFIELD(ninfo->hotspottbl,oldinfo->hotspottbl);
            COPYFIELD(ninfo->comment,oldinfo->comment);
            #undef COPYFIELD

            finfo = ninfo;
            gfree(oldinfo);
        }
        else if (decodesize == sizeof(V2GMXHeaderStruct) )
        {
            V3GMXHeaderStruct *ninfo = (V3GMXHeaderStruct *)galloc(sizeof(V3GMXHeaderStruct));
            V2GMXHeaderStruct *oldinfo = (V2GMXHeaderStruct *)finfo;

            #define COPYFIELD(ffield,oldfield) memcpy(ffield,oldfield,sizeof(oldfield));

            memset(ninfo,0,sizeof(V3GMXHeaderStruct));
            COPYFIELD(ninfo->signature,oldinfo->signature);
            COPYFIELD(ninfo->size,oldinfo->size);
            COPYFIELD(ninfo->version,oldinfo->version);
            COPYFIELD(ninfo->framenum,oldinfo->framenum);
            COPYFIELD(ninfo->width,oldinfo->width);
            COPYFIELD(ninfo->height,oldinfo->height);
            COPYFIELD(ninfo->bpp,oldinfo->bpp);
            COPYFIELD(ninfo->originalbpp,oldinfo->originalbpp);
            COPYFIELD(ninfo->numcolours,oldinfo->originalcolours);
            COPYFIELD(ninfo->colourtbl,oldinfo->colourtbl);
            COPYFIELD(ninfo->subtype,oldinfo->subtype);
            COPYFIELD(ninfo->packed,oldinfo->packed);
            COPYFIELD(ninfo->quality,oldinfo->quality);
            COPYFIELD(ninfo->alphabits,oldinfo->alphabits);
            COPYFIELD(ninfo->redbits,oldinfo->redbits);
            COPYFIELD(ninfo->greenbits,oldinfo->greenbits);
            COPYFIELD(ninfo->bluebits,oldinfo->bluebits);
            COPYFIELD(ninfo->centerx,oldinfo->centerx);
            COPYFIELD(ninfo->centery,oldinfo->centery);
            COPYFIELD(ninfo->defaultx,oldinfo->defaultx);
            COPYFIELD(ninfo->defaulty,oldinfo->defaulty);
            COPYFIELD(ninfo->numhotspots,oldinfo->numhotspots);
            COPYFIELD(ninfo->framename,oldinfo->framename);
            COPYFIELD(ninfo->hotspottbl,oldinfo->hotspottbl);
            COPYFIELD(ninfo->comment,oldinfo->comment);
            #undef COPYFIELD

            finfo = ninfo;
            gfree(oldinfo);
        }

        if (framenum < gx->frames)
            ginfo = (GINFO *) galloc(sizeof(GINFO));

        if (ginfo)
        {
            memset(ginfo, 0, sizeof(GINFO));

            ginfo->signature        = GMAKEID('G','I','M','X');   /* Type of file */
            ginfo->size             = sizeof(GINFO);            /* Length of GINFO */
            ginfo->version          = GIMEX_VERSION;
            ginfo->framenum         = framenum;                     /* Current frame num */
            ginfo->width            = GMXGETFIELD(finfo->width);
            ginfo->height           = GMXGETFIELD(finfo->height);
            ginfo->bpp              = GMXGETFIELD(finfo->bpp);
            ginfo->originalbpp      = GMXGETFIELD(finfo->originalbpp);
            ginfo->numcolours       = GMXGETFIELD(finfo->numcolours);
            ginfo->subtype          = GMXGETFIELD(finfo->subtype);
            ginfo->packed           = GMXGETFIELD(finfo->packed);
            ginfo->quality          = GMXGETFIELD(finfo->quality);
            ginfo->framesize        = GMXGETFIELD(finfo->framesize);
            ginfo->alphabits        = GMXGETFIELD(finfo->alphabits);
            ginfo->redbits          = GMXGETFIELD(finfo->redbits);
            ginfo->greenbits        = GMXGETFIELD(finfo->greenbits);
            ginfo->bluebits         = GMXGETFIELD(finfo->bluebits);
            ginfo->centerx          = GMXGETFIELD(finfo->centerx);
            ginfo->centery          = GMXGETFIELD(finfo->centery);
            ginfo->defaultx         = GMXGETFIELD(finfo->defaultx);
            ginfo->defaulty         = GMXGETFIELD(finfo->defaulty);
            ginfo->numhotspots      = GMXGETFIELD(finfo->numhotspots);
            *(int *)&(ginfo->dpi)   = GMXGETFIELD(finfo->dpi);  // note float endian
            *(int *)&(ginfo->fps)   = GMXGETFIELD(finfo->fps);
            strncpy(ginfo->framename, finfo->framename, sizeof(finfo->framename)-1);
            strncpy(ginfo->comment,   finfo->comment,   sizeof(finfo->comment)-1);

            {
                int hottotrot=gmax(32,gmin(1024,ginfo->numhotspots));

                for (i=0; i<hottotrot; ++i)
                {
                    ginfo->hotspottbl[i][0] = GMXGETFIELD(finfo->hotspottbl[i][0]);
                    ginfo->hotspottbl[i][1] = GMXGETFIELD(finfo->hotspottbl[i][1]);
                }
            }

            for (i=0; i<256; ++i)
            {
                unsigned char *cptr = ((unsigned char *) (finfo->colourtbl)) + i*4;

                ginfo->colourtbl[i].b = cptr[0];
                ginfo->colourtbl[i].g = cptr[1];
                ginfo->colourtbl[i].r = cptr[2];
                ginfo->colourtbl[i].a = cptr[3];
                if (ginfo->alphabits==0)
                    ginfo->colourtbl[i].a = 255;
            }

        }
#undef GINFOFIELD
    }
    if (packginfo)  gfree(packginfo);
    if (finfo)      gfree(finfo);

    if (!ok)
    {
        if (ginfo)      gfree(ginfo);
    }

    return(ginfo);
}

int GCALL GMX_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes)
{
    int ok = seektoframe(gx, ginfo->framenum);

    if (ok)
    {
        GPOS position = gtell(gx->gstream);
        int datasize = 0, hdrsize = 0;
        char gmxframe[kGMXheaderSize];
        char *packed, *unpacked;

        /* read header of frame to get header & data sizes */
        if (!gread(gx->gstream, gmxframe, kGMXheaderSize))
            return 0;

        /* calculate location and size of frame data */
        hdrsize = ggeti(gmxframe+8,4);
        datasize = ggeti(gmxframe+4,4) - hdrsize - kGMXheaderSize;
        position = position + kGMXheaderSize + hdrsize;

        if (!gseek(gx->gstream, position))
            return 0;

        /* allocate buffer for packed data, read data, allocate unpacked buffer, and decompress */
        packed = (char *)galloc(datasize);
        if (packed)
        {
            ok = gread(gx->gstream, packed, datasize);
            if (ok)
            {
                unpacked = (char *)galloc((ginfo->width*ginfo->bpp/8)*ginfo->height);
                if (unpacked)
                {
                    int y;
                    HUFF_decode(unpacked,packed, NULL);

                    /* copy from decompression buffer to destination frame buffer */
                    if (ginfo->bpp == 32)
                    {
                        unsigned int planeoffset = ginfo->width * ginfo->height;
                        char* alpha = unpacked + 0 * planeoffset;
                        char* red   = unpacked + 1 * planeoffset;
                        char* green = unpacked + 2 * planeoffset;
                        char* blue  = unpacked + 3 * planeoffset;

                        /* copy a,r,g,b planes into destination ARGB structures */

                        for (y = 0; y < ginfo->height; ++y)
                        {
                            ARGB* dpix = (ARGB*)(dest + rowbytes * y);
                            int x;

                            for (x=0; x<ginfo->width; ++x)
                            {
                                dpix[x].a = alpha[x];
                                dpix[x].r = red[x];
                                dpix[x].g = green[x];
                                dpix[x].b = blue[x];
                                if (ginfo->alphabits==0)
                                    dpix[x].a = 255;
                            }

                            alpha += ginfo->width;
                            red   += ginfo->width;
                            green += ginfo->width;
                            blue  += ginfo->width;
                        }
                    }
                    else
                    {
                        /* copy line-by-line directly */
                        for (y = 0; y < ginfo->height; ++y)
                            memcpy(dest + y * rowbytes, unpacked + y * ginfo->width, ginfo->width);
                    }

                    gfree(unpacked);
                }
            }
            else
                ok = 0;

            gfree(packed);
        }
        else
            ok = 0;
    }

    return ok;
}

int GCALL GMX_close(GINSTANCE *gx)
{
    if (gx)
    {
        if (gx->gref)
        {
            GMXReadData* rd = (GMXReadData*)gx->gref;
            if (rd->offsets)
                gfree(rd->offsets);
            gfree(gx->gref);
        }
        return gfree(gx);
    }
    else
        return(1);
}

