// (c) 1994-2007 Electronic Arts Inc.

#include <string.h>
#include <assert.h>
#include "gimex.h"
#include "psdgimex.h"

#define GOFFSETRED              (&((ARGB*)0)->r - (unsigned char*)(0))
#define GOFFSETGREEN            (&((ARGB*)0)->g - (unsigned char*)(0))
#define GOFFSETBLUE             (&((ARGB*)0)->b - (unsigned char*)(0))
#define GOFFSETALPHA            (&((ARGB*)0)->a - (unsigned char*)(0))

/****************************************************************/
/*  Export Functions                                            */
/****************************************************************/

/*
 * Encode a run of pixels.
 */

static int PackBitsEncode(char *dest, char *source, int cc)
{
    unsigned char *bp = (unsigned char *) source;
    unsigned char *op, *lastliteral;
    int n;
    int b;
    enum { BASE, LITERAL, RUN, LITERAL_RUN } state;

    op = (unsigned char*)dest;
    state = BASE;
    lastliteral = 0;
    while (cc > 0)
    {
        /*
        * Find the longest string of identical bytes.
        */
        b = *bp++, cc--, n = 1;
        for (; cc > 0 && b == *bp; cc--, bp++)
            n++;

        again:
        switch (state)
        {
            case BASE:              /* initial state, set run/literal */
                if (n > 1)
                {
                    state = RUN;
                    if (n > 128)
                    {
                        *op++ = (unsigned char) -127;
                        *op++ = (unsigned char) b;
                        n    -= 128;
                        goto again;
                    }
                    *op++ = (unsigned char) -(n-1);
                    *op++ = (unsigned char) b;
                }
                else
                {
                    lastliteral = op;
                    *op++       = 0;
                    *op++       = (unsigned char)b;
                    state       = LITERAL;
                }
                break;

            case LITERAL:           /* last object was literal string */
                if (n > 1)
                {
                    state = LITERAL_RUN;
                    if (n > 128)
                    {
                        *op++ = (unsigned char) -127;
                        *op++ = (unsigned char) b;
                        n -= 128;
                        goto again;
                    }
                    *op++ = (unsigned char) -(n-1);         /* encode run */
                    *op++ = (unsigned char) b;
                }
                else
                {                    /* extend literal */
                    if (++(*lastliteral) == 127)
                        state = BASE;
                    *op++ = (unsigned char)b;
                }
                break;
            case RUN:               /* last object was run */
                if (n > 1) {
                    if (n > 128) {
                        *op++ = (unsigned char) -127;
                        *op++ = (unsigned char) b;
                        n -= 128;
                        goto again;
                    }
                    *op++ = (unsigned char) -(n-1);
                    *op++ = (unsigned char) b;
                } else {
                    lastliteral = op;
                    *op++ = 0;
                    *op++ = (unsigned char)b;
                    state = LITERAL;
                }
                break;
            case LITERAL_RUN:       /* literal followed by a run */
                /*
                * Check to see if previous run should
                * be converted to a literal, in which
                * case we convert literal-run-literal
                * to a single literal.
                */
                if (n == 1 && -op[-2] == 1 && *lastliteral < 126) {
                    state = (((*lastliteral) += 2) == 127 ?
                             BASE : LITERAL);
                    op[-2] = op[-1];        /* replicate */
                } else
                    state = RUN;
                goto again;
        }
    }
    return(op-(unsigned char*)dest);
}

int PSDAddLayer(PSDINFO * psd, const GINFO * ginfo, char * source,
                int rowbytes, int framenum)
{
    unsigned char * srcptr;
    unsigned char * dstptr;
    int i, j, ok = 0;
    PSDLAYERINFO * layer;

    layer               = psd->layerinfo+framenum;
    layer->left         = ginfo->defaultx;
    layer->right        = ginfo->defaultx + ginfo->width;
    layer->top          = ginfo->defaulty;
    layer->bottom       = ginfo->defaulty + ginfo->height;
    layer->packed       = ginfo->packed;
    strcpy (layer->name, ginfo->framename);


    if (ginfo->dpi)
        psd->dpi = (int) (ginfo->dpi*65536.0f);

    /* Allocate RGBA buffer for data */
    layer->data = (char *) galloc(ginfo->width * ginfo->height * 4);

    /* If source bytes are in indexed mode, we need to convert to RGBA */
    if (ginfo->bpp == 8)
    {
        for (i = 0; i < ginfo->height; i++)
        {
            srcptr = (unsigned char *)(source + i * rowbytes);
            dstptr = (unsigned char *)(layer->data + i * (ginfo->width * 4));
            for (j = 0; j < ginfo->width; j++)
            {
                *(dstptr+GOFFSETRED)    = ginfo->colourtbl[*srcptr].r;
                *(dstptr+GOFFSETGREEN)  = ginfo->colourtbl[*srcptr].g;
                *(dstptr+GOFFSETBLUE)   = ginfo->colourtbl[*srcptr].b;
                *(dstptr+GOFFSETALPHA)  = ginfo->colourtbl[*srcptr].a;
                dstptr+=4;
                srcptr++;
            }
        }
        ok = 1;
    }
    else if (ginfo->bpp == 32)
    {
        for (i = 0; i < ginfo->height; i++)
        {
            srcptr = (unsigned char *)(source + i * rowbytes);
            dstptr = (unsigned char *)(layer->data + i * (ginfo->width * 4));
            memcpy(dstptr, srcptr, ginfo->width *  4);
        }
        ok = 1;
    }
    return ok;
}

int GCALL PSD_wopen(GINSTANCE **gxptr, GSTREAM *g, const char *pathname, int numframes)
{
    PSDINFO * psd;
    GINSTANCE *gx;
    int ok = 0;

    gx = (GINSTANCE *) galloc(sizeof(GINSTANCE));
    if (gx)
    {
        memset(gx, 0, sizeof(GINSTANCE));

        gx->gstream = g;
        gx->frames = numframes;
        gx->size = sizeof(GINSTANCE);
        *gxptr = gx;

        /* Allocate photoshop info */
        gx->gref = galloc(sizeof(PSDINFO));
        if (gx->gref)
        {
            psd = (PSDINFO *) gx->gref;
            memset ((void *) psd, 0, sizeof(PSDINFO));

            /* Allocate layers subinfo */
            psd->layerinfo = (PSDLAYERINFO *) galloc(numframes * sizeof(PSDLAYERINFO));
            if (psd->layerinfo)
            {
                memset ((void *) psd->layerinfo, 0, sizeof(PSDLAYERINFO) * numframes);
                ok = 1;
            }
        }
    }
    return(ok);
}


int GCALL PSD_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes)
{
    PSDINFO * psd;
    int ok = 0;

    /* Add to our layer information */
    psd = (PSDINFO *) gx->gref;
    if (gx->framenum < gx->frames)
    {
        ok = PSDAddLayer(psd, ginfo, source, rowbytes, gx->framenum);
        ++gx->framenum;
    }
    return (ok);
}

/*
 * Writes a single channel (as part of the layer section, not the
 * image section) and returns the length written
 */
int GCALL writechannel(GSTREAM * g, char * data, int channel, int height, int width, int packed)
{
    unsigned char buf[4];
    int row_count_tag = 0;
    int curr_tag      = 0;
    char * rowtbl     = NULL;
    char * line       = NULL;
    char * packedline = NULL;
    int i, j;
    char * d          = NULL;
    char * lineptr    = NULL;
    int planeoffsets[4] = {GOFFSETRED, GOFFSETGREEN, GOFFSETBLUE, GOFFSETALPHA};
    int planerowsize;
    int length = 0;

    line = (char *) galloc(width);
    if (!line)
        goto error;

    /* Write compression short */
    if (packed)
    {
        packedline = (char *) galloc(width*2+100);
        rowtbl = (char *) galloc(height * 2);
        if (!packedline || !rowtbl)
            goto error;
        gputm(buf, 1, 2);
    }
    else
    {
        gputm(buf, 0, 2);
    }
    gwrite(g, buf, 2);
    length += 2;

    /* Skip row counts (we go back and write them later) */
    if (packed)
    {
        row_count_tag = (int)gtell(g);
        gwrite(g, rowtbl, height * 2);
        length += 2 * height;
    }

    for (i = 0; i < height; i++)
    {
        /* Get channel data, store in line buf */
        lineptr = line;
        d = data + (i * width * 4) + planeoffsets[channel];
        for (j = 0; j < width; j++)
        {
            *lineptr = *d;
            lineptr++;
            d+=4;
        }
        if (packed)
        {
            /* Pack */
            planerowsize = PackBitsEncode(packedline, line, width);
            /* Write */
            gwrite(g, packedline, planerowsize);
            length += planerowsize;
            gputm(rowtbl+i*2, planerowsize, 2);
        }
        else
        {
            gwrite(g, line, width);
            length += width;
        }
    }

    if (packed)
    {
        /* Go back, scribble row table */
        curr_tag = (int)gtell(g);
        gseek(g, row_count_tag);
        gwrite(g, rowtbl, height * 2);
        gseek(g, curr_tag);
        /* Done */
        gfree(rowtbl);
        gfree(packedline);
    }
    gfree(line);
    return length;

    error:
    if (line)
        gfree(line);
    if (packedline)
        gfree(packedline);
    if (rowtbl)
        gfree(rowtbl);
    return 0;
}

int GCALL PSD_wclose(GINSTANCE *gx)
{
    unsigned char buf[4];
    PSDHeaderStruct psdheader;
    PSDINFO * psd         = NULL;
    bool ok               = false;
    int width = 0, height = 0;
    GSTREAM * g           = NULL;
    int i = 0, j = 0;
    int misc_info_pos, layer_info_pos;
    int length;
    int curr_pos;
    char *linebuf;
    char *packlinebuf;
    char * rowtbl;
    int rowtblpos;
    int planerowsize;
    int frames;

    g = gx->gstream;
    psd = (PSDINFO *) gx->gref;
    frames = gmin(gx->framenum + 1, gx->frames);
    /* Photoshop's limit on frames is 100 */
    frames = gmin(frames, 100);
    if (frames)
    {
        /*
         * Go through the layers and figure out the maximum
         * width/height we need to handle
         */
        for (i = 0; i < frames; i++)
        {
            if (psd->layerinfo[i].bottom > height)
                height = psd->layerinfo[i].bottom;
            if (psd->layerinfo[i].right > width)
                width = psd->layerinfo[i].right;
        }
        psd->width = width;
        psd->height = height;

        /* Write header */
        memset(&psdheader,0,sizeof(PSDHeaderStruct));
        PSDPUTFIELD(psdheader.signature, GMAKEID('8','B','P','S'));
        PSDPUTFIELD(psdheader.version, 1);
        PSDPUTFIELD(psdheader.channels, 3);
        PSDPUTFIELD(psdheader.rows, height);
        PSDPUTFIELD(psdheader.columns, width);
        PSDPUTFIELD(psdheader.depth, 8);
        PSDPUTFIELD(psdheader.mode, 3);
        ok = (26 == gwrite(g, (char *) &psdheader, 26));

        /* Skip color space */
        gputm(buf,0,4);
        ok &= (4 == gwrite(g, buf,4));

        if (psd->dpi)
        {
            char resourceheader[4+4+2+2+4];
            PSDDpiResource dpiresource;

            gputm(resourceheader+0,sizeof(resourceheader)-4+sizeof(dpiresource),4);
            gputm(resourceheader+4,ggetm("8BIM",4),4);
            gputm(resourceheader+8,1005,2); // DPI resource id
            gputm(resourceheader+10,0,2);   // No name
            gputm(resourceheader+12,sizeof(dpiresource),4);  // size
            PSDPUTFIELD(dpiresource.xdpi, psd->dpi);
            PSDPUTFIELD(dpiresource.xmeasure, 1);   // pixels per inch
            PSDPUTFIELD(dpiresource.xflag, 1);      // valid
            PSDPUTFIELD(dpiresource.ydpi, psd->dpi);
            PSDPUTFIELD(dpiresource.ymeasure, 1);   // pixels per inch
            PSDPUTFIELD(dpiresource.yflag, 1);      // valid

            ok &= (16 == gwrite(g, resourceheader, 16));
            ok &= (16 == gwrite(g, &dpiresource, 16));
        }
        else
        {
            /* Skip image resources */
            ok &= (4 == gwrite(g, buf,4));
        }


        /* Mark where length of misc info section goes */
        misc_info_pos = (int)gtell(g);
        ok &= (4 == gwrite(g, buf,4));
        /* Mark where length of layers info section goes */
        layer_info_pos = (int)gtell(g);
        ok &= (4 == gwrite(g, buf,4));

        /* Write number of layers */
        gputm(buf, gx->frames, 2);
        ok &= (2 == gwrite(g, buf, 2));

        /* Layer loop */
        for (i = 0; i < gx->frames; i++)
        {
            /* Layer rectangle */
            gputm(buf, psd->layerinfo[i].top, 4);
            ok &= (4 == gwrite(g, buf, 4));
            gputm(buf, psd->layerinfo[i].left, 4);
            ok &= (4 == gwrite(g, buf, 4));
            gputm(buf, psd->layerinfo[i].bottom, 4);
            ok &= (4 == gwrite(g, buf, 4));
            gputm(buf, psd->layerinfo[i].right, 4);
            ok &= (4 == gwrite(g, buf, 4));

            /* 4 channels written per layer */
            gputm(buf, 4, 2);
            ok &= (2 == gwrite(g, buf, 2));

            /* Channel length info - first do the transparency (alpha) */
            gputm(buf, 65535, 2);
            ok &= (2 == gwrite(g, buf, 2));
            psd->layerinfo[i].offsets[3] = (int)gtell(g);
            /* Fill this in later */
            ok &= (4 == gwrite(g, buf, 4));

            /* Now the same but for RGB */
            for (j = 0; j < 3; j++)
            {
                gputm(buf, j, 2);
                ok &= (2 == gwrite(g, buf, 2));
                psd->layerinfo[i].offsets[j] = (int)gtell(g);
                /* Fill this in later */
                ok &= (4 == gwrite(g, buf, 4));
            }

            /* Blend keys */
            strncpy((char *)buf, "8BIM", 4);
            ok &= (4 == gwrite(g, buf, 4));
            strncpy((char *)buf, "norm", 4);
            ok &= (4 == gwrite(g, buf, 4));
            /* Opacity, clipping, flags, filler */
            buf[0] = (unsigned char) 0xff; buf[1] = 0;
            /* Toggling bit 1 of the flags turns layer visibility off */
            if (i > 0)
                buf[2] = 2;
            else
                buf[2] = 0;
            buf[3] = 0;
            ok &= (4 == gwrite(g, buf, 4));
            /* Length of the extra data field (the next three fields NOT five!) */
            gputm(buf, 48 + ((strlen(psd->layerinfo[i].name) + 4) / 4) * 4, 4);
            ok &= (4 == gwrite(g, buf, 4));
            /* Write mask data (empty) */
            gputm(buf, 0, 4);
            ok &= (4 == gwrite(g, buf,4));
            /* Write blending ranges. There will be four channels +
            * composite, 8 bytes of data each, so length is 5*8=40 */
            gputm(buf, 40, 4);
            ok &= (4 == gwrite(g, buf, 4));
            /* These are just the default blending ranges */
            buf[0] = 0; buf[1] = 0;
            buf[2] = (unsigned char) 0xff; buf[3] = (unsigned char) 0xff;
            for (j = 0; j < 5; j++)
            {
                ok &= (4 == gwrite(g, buf,4));
                ok &= (4 == gwrite(g, buf,4));
            }
            /* Layer name. This is a pstring, so write the length of name */
            buf[0] = (unsigned char)strlen(psd->layerinfo[i].name);
            ok &= (1 == gwrite(g, buf, 1));
            j = strlen(psd->layerinfo[i].name);
            if (j)
                ok &= (j == gwrite(g, psd->layerinfo[i].name, j));
            /* pad (four byte boundary) */
            buf[0] = 0;
            j = 4 - ((strlen(psd->layerinfo[i].name) + 1) % 4);
            if (j < 4)
                while (j--)
                    ok &= (1 == gwrite(g, buf, 1));
        }

        /* Layer channels */
        for (i = 0; i < gx->frames; i++)
        {
            height = psd->layerinfo[i].bottom - psd->layerinfo[i].top;
            width = psd->layerinfo[i].right - psd->layerinfo[i].left;

            /* Do the alpha first */
            length = writechannel(g, psd->layerinfo[i].data, 3, height, width,
                                  psd->layerinfo[i].packed);
            /* Error */
            if (length == 0)
                return 0;

            /* Jump back in file and write in the length of the channel data */
            curr_pos = (int)gtell(g);
            gseek(g, psd->layerinfo[i].offsets[3]);
            gputm(buf, length, 4);
            ok &= (4 == gwrite(g, buf, 4));
            gseek(g, curr_pos);

            /* Same for R, G, B */
            for (j = 0; j < 3; j++)
            {
                length = writechannel(g, psd->layerinfo[i].data, j, height, width,
                                      psd->layerinfo[i].packed);
                curr_pos = (int)gtell(g);
                gseek(g, psd->layerinfo[i].offsets[j]);
                gputm(buf, length, 4);
                ok &= (4 == gwrite(g, buf, 4));
                gseek(g, curr_pos);
            }
        }

        /* Write pad byte */
        if ((gtell(g) % 2) == 1)
        {
            buf[0] = 0;
            ok &= (1 == gwrite(g, buf, 1));
        }

        curr_pos = (int)gtell(g);
        gseek(g, layer_info_pos);
        gputm(buf, curr_pos - layer_info_pos - 4, 4);
        ok &= (4 == gwrite(g, buf, 4));
        gseek(g, curr_pos);

        /* no mask data */
        gputm(buf, 0, 4);
        ok &= (4 == gwrite(g, buf, 4));

        /* Go back and fill in the length of the misc info section */
        curr_pos = (int)gtell(g);
        gseek(g, misc_info_pos);
        gputm(buf, curr_pos - misc_info_pos - 4, 4);
        ok &= (4 == gwrite(g, buf, 4));
        gseek(g, curr_pos);

        /* Compression  */
        gputm(buf, 1, 2);
        ok &= (2 == gwrite(g, buf, 2));

        width = psd->width;
        height = psd->height;
        linebuf = (char *) galloc(width);
        packlinebuf = (char *) galloc(width*2+100);

        /* Clear linebuf */
        memset(linebuf, 0, width);
        /* Allocate row table */
        rowtbl = (char *) galloc(height * 4 * 2);
        /* Store row table position */
        rowtblpos = (int)gtell(g);
        ok &= ((height * 4 * 2) == gwrite(g, rowtbl, height * 4 * 2));

        /* Write */
        for (i = 0; i < 4; i++)
        {
            for (j = 0; j < height; j++)
            {
                planerowsize = PackBitsEncode(packlinebuf, linebuf, width);
                ok &= (planerowsize == gwrite(g,packlinebuf,planerowsize));
                gputm(rowtbl+i*(height*2)+j*2,planerowsize,2);
            }
        }
        gseek(g,rowtblpos);
        ok &= ((height * 4 * 2) == gwrite(g,rowtbl,height*4*2));
        gfree(linebuf);
        gfree(packlinebuf);
        gfree(rowtbl);

        if (gx)
        {
            if (gx->gref)
            {
                psd = (PSDINFO *) gx->gref;
                if (psd->layerinfo)
                {
                    for (i = 0; i < frames; i++)
                    {
                        if (psd->layerinfo[i].data)
                            gfree(psd->layerinfo[i].data);
                    }
                    ok &= gfree(psd->layerinfo);
                }
                ok &= gfree(psd);
            }
            ok &= gfree(gx);
        }
    }
    return ok;
}

