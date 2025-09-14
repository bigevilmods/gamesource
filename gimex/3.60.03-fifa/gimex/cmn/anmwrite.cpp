// (c) 1994-2007 Electronic Arts Inc.

#include <string.h>
#include "gimex.h"
#include "anmgimex.h"

/****************************************************************/
/*  Internal Functions                                          */
/****************************************************************/

/**************************************************************************/
/*  cropimage                                                             */
/*    Centers source image in destination, cropping or filling with black */
/*    Note that destination for ANM is always 320x200                     */
/**************************************************************************/

void cropimage(unsigned char *source, unsigned char *dest, int destwidth, int destheight, int width, int height, int rowbytes)
{
    int x;
    int y;
    int cx;
    int cy;
    unsigned char *lineptr;
    unsigned char *s;

    cx = width/2-destwidth/2;
    cy = height/2-destheight/2;

    lineptr = dest;
    for (y=cy; (y<cy+destheight); ++y)
    {
        if (y<0 || y>=height)
        {
            for (x=0; x<destwidth; ++x)
            {
                *lineptr++ = 0;
            }
        }
        else
        {
            s = source+y*rowbytes+cx;
            for (x=cx; x<cx+destwidth; ++x)
            {
                if (x<0 || x>=width)
                {
                    *lineptr++ = 0;
                    ++s;
                }
                else
                {
                    *lineptr++ = *s++;
                }
            }
        }
    }
}

/**************************************************************************/
/*  check_for_dump                                                        */
/*     Used by compress_data to check for and handle a dump in the        */
/*     compression of an animation.                                       */
/**************************************************************************/

void check_for_dump(unsigned int *dump_size,
                    unsigned char *dest,   unsigned int *d_offset,
                    unsigned char *source, unsigned int s_offset)
{
    int count;

    if (*dump_size > 0)
    {
        if (*dump_size > 127)
        {
            /* Long Dump */
            dest[(*d_offset)++] = 0x80;
            gputi( &dest[*d_offset], *dump_size + 0x8000, 2);
            *d_offset += 2;
        }
        else
        {
            /* Short Dump */
            dest[(*d_offset)++] = (unsigned char)*dump_size;
        }

        /* Copy data over */
        for (count = (s_offset - *dump_size); count < (int)s_offset; count++)
            dest[(*d_offset)++] = (unsigned char)source[count];

        *dump_size = 0;
    }
}



/**************************************************************************/
/*  compress_data                                                         */
/*     Compresses the frame pointed to by cur_frame                       */
/*     and puts resulting data at dest, returns the size of the           */
/*     compressed data.                                                   */
/**************************************************************************/

unsigned int compress_data(unsigned char *cur_frame, unsigned char *last_frame,  unsigned char *dest, unsigned int size)
{
    unsigned int s_offset  = 0;
    unsigned int d_offset;
    unsigned int len;
    unsigned int dump_size = 0;
    unsigned char          data;
    unsigned char          do_dump;


        /* We first write the header to dest */
    dest[0] = 66;                          /* Always 66 */
    dest[1] = 0;                           /* No extra bytes to skip.  (Make this always 0) */
    gputi( &dest[2], 1, 2);                /* Extra byte count */
    d_offset = 4;


    while (s_offset != size )
    {
        do_dump = 1;

        if (last_frame)
        {
            /* First check to see if cur_frame and last_frame have the same data. (if last_frame != NULL) */
            len = 0;
            while ((cur_frame[len + s_offset] == last_frame[len + s_offset]) && ((len + s_offset) != size) )
                len++;

            if (len == size) break;  /* If we are skipping the whole frame, then just write the terminator */

            if (  len > 1 )    /* Only skip if len > 1 */
            {
                /* We have found a skip, first do a dump if necessary */
                check_for_dump(&dump_size, (unsigned char *)dest, &d_offset, cur_frame, s_offset);

                if (len > 0x7FFF) len = 0x7FFF;      /* Can only skip 0x7FFF bytes maximum */

                do_dump = 0;
                s_offset += len;

                if (len > 127)
                {
                    /* Long Skip */
                    dest[d_offset++] = (unsigned char) 0x80;
                    gputi( &dest[d_offset], len, 2);
                    d_offset += 2;
                } else
                    /* Short Skip  */
                    dest[d_offset++] = (unsigned char) (len + 0x80);
            }
        }

        /* Check to see if we have a run of a specific character     */
        len  = 1;
        data = cur_frame[s_offset];
        while ( (len + s_offset) != size && cur_frame[s_offset + len] == data)
            len++;

        if ( len > 3 )   /* Only do a run if lenght of run is > 3 */
        {

            /* We have found a run, first do a dump if necessary */
            check_for_dump(&dump_size, (unsigned char *)dest, &d_offset, cur_frame, s_offset);
            do_dump = 0;

            if (len > 16383)   len = 16383;

            s_offset += len;

            if (len > 255)
            {
                /* Long Run - handles runs with sizes 256 to 16383 */
                dest[d_offset++] = (unsigned char)0x80;
                len = len + 0xc000;               /* Set top two bits to indicate a int run */
                gputi( &dest[d_offset], len, 2);
                d_offset += 2;
                dest[d_offset++] = data;

            } else
            {
                /* Short Run - handles sizes up to 255 */
                dest[d_offset++] = (unsigned char)0;
                dest[d_offset++] = (unsigned char)len;
                dest[d_offset++] = (unsigned char)data;
            }
        }

        if (do_dump && s_offset != size)
        {
            /* If we couldn't do any compression this time round, we move up s_offset  */
            /* and increase the # of characters we will dump. */
            s_offset++;
            dump_size++;

            /* We check to see if dump_size has reached it's maximum allowable size, if it has, do the dump */
            if (dump_size == 16383)
                check_for_dump(&dump_size, (unsigned char *)dest, &d_offset, cur_frame, s_offset);
        }
    }

    /* Check to see if we have a dump in memory before we end */
    check_for_dump(&dump_size, (unsigned char *)dest, &d_offset, cur_frame, s_offset);

    /* We are done, put in the end of frame indicator */
    dest[d_offset++] = (unsigned char)0x80;
    dest[d_offset++] = (unsigned char)0x00;
    dest[d_offset++] = (unsigned char)0x00;

    return(d_offset);
}



/**************************************************************************/
/*  make_page                                                             */
/*     Sets up a new page.  Clears thepage buffer, sets nRecords to zero  */
/*     and clears recsizes[].                                             */
/**************************************************************************/

void make_page(ANMINFO *anm)
{
    int count;

    /* Clear thepage */
    memset((void *) anm->thepage ,0, 65536L);

    ANMPUTFIELD( anm->curlp.nRecords, 0);

    /* Intialize recsizes to 0 */
    for (count = 0; count < 256; count++)
        ANMPUTFIELD(anm->recsizes[count], 0);
}

/*************************************************************************/
/*  dump_page                                                            */
/*     Dumps a page to the file.  If last_page == 0, we assume 64k size. */
/*     Otherwise, we calculate size, and this is the last page.          */
/*************************************************************************/

void dump_page(char last_page, GSTREAM *stream, ANMINFO *anm)
{
    unsigned int size, total = 0;
    int           numrec, count;
    char zero[2];


    /* Calculate nBytes */
    numrec = ANMGETFIELD(anm->curlp.nRecords) & 0x3fff;
    for(count = 0; count < numrec; count++)
        total = total + ANMGETFIELD(anm->recsizes[count]);
    ANMPUTFIELD(anm->curlp.nBytes, total);


    /* Figure out size of the page */
    if (last_page == 0)
        size = 65536L;
    else
    {
        /* Calculate size */
        size  = 8;                                /* Fixed part of header */
        size += numrec * 2;                       /* Record size part of header */
        size += ANMGETFIELD(anm->curlp.nBytes);   /* Data size */

    }

    /* Copy header data to LParray */
    memcpy( &anm->LpArray[ANMGETFIELD(anm->curlpnum)], &anm->curlp, 6);

    /* Output page header to file */
    gwrite(stream, &anm->curlp, 6);
    ANMPUTFIELD(zero, 0);        /* Next field is always 0, put 0 in baserecord */
    gwrite(stream, &zero, 2);    /* and then write it to file */
    size -= 8;                                    /* Adjust output size */

    for (count = 0; count < numrec; count++)
    {
        gwrite(stream, &anm->recsizes[count], 2); /* Output records sizes */
        size -= 2;                                /* Adjust output size    */
    }

    /* Now we dump the page to the file */
    gwrite(stream, anm->thepage, size);

    /* Update baserecord for the next Large Page */
    ANMPUTFIELD( anm->curlp.baseRecord, ANMGETFIELD(anm->curlp.baseRecord) + numrec );

}



/*************************************************************************/
/*  write_data_to_page                                                   */
/*     writes data from source to the page, increases # of frames in     */
/*     current page and checks for end of page.                          */
/*************************************************************************/

void write_data_to_page(unsigned char *source, ANMINFO *anm, unsigned int data_size, GSTREAM *stream)
{
    unsigned int offset = 0;
    int           count;
    int           numrec;

    /* Calculate offset into thepage */

    numrec = ANMGETFIELD(anm->curlp.nRecords)  & 0x3fff;
    for (count = 0; count < numrec; count++)
        offset = offset + ANMGETFIELD(anm->recsizes[count]);

    if ( (offset + data_size) > 65015L )    /* Large Page can contain maximun 65015 bytes (excluding header) */
    {
        /* Frame is too big for page, dump old page and start a new one */
        dump_page( 0, stream, anm);

        /* Add one to the Lp count and make new page */
        ANMPUTFIELD( anm->curlpnum, ANMGETFIELD(anm->curlpnum) + 1);
        make_page(anm);
        offset = 0;
        numrec = 0;
    }

    /* Add one to frame count in current page */
    ANMPUTFIELD( anm->curlp.nRecords, ANMGETFIELD(anm->curlp.nRecords) + 1);

    ANMPUTFIELD(anm->recsizes[numrec], data_size);    /* Add size for new frame */
    /* Copy data from source to thepage */
    memcpy(anm->thepage + offset, source,data_size);

}



/****************************************************************/
/*  Export Functions                                            */
/****************************************************************/

int GCALL ANM_wopen(GINSTANCE **gxptr, GSTREAM *g, const char *pathname, int numframes)
{
    GINSTANCE *gx;
    ANMINFO *anm;
    int  ok=0;

    gx = (GINSTANCE *) galloc(sizeof(GINSTANCE));
    if (gx)
    {
        memset(gx, 0, sizeof(GINSTANCE));
        *gxptr = gx;
        gx->gstream = g;
        gx->framenum = 0;  /* keep track of which frame were pointing at */
        gx->signature=GMAKEID('.','A','N','M');
        gx->size=sizeof(GINSTANCE);

        gseek(gx->gstream,(GPOS)0);

        /* Allocate memory for the animinfo structure */
        gx->gref = galloc(sizeof(ANMINFO));

        if (gx->gref)
        {
            anm = (ANMINFO *) gx->gref;
            memset((void *) anm,0,sizeof(ANMINFO));

            anm->curlpnum = 0xFFFF;      /* Set current lp num to an impossibly large # to indicate that */
                                         /* we have not written any actual frames yet. */


            /* Initialize header */
            ANMPUTFIELD(anm->DAHEADER.id, ' '<<24 | 'F'<<16 | 'P'<<8 | 'L');
            ANMPUTFIELD(anm->DAHEADER.maxLps, 256);
            ANMPUTFIELD(anm->DAHEADER.nLps, 0);                    /* Have to come back and change this when we know it */
            ANMPUTFIELD(anm->DAHEADER.nRecords, numframes + 1);    /* Assuming numframes if the total # of frames. */
                                                                           /* Add one because we will have the last to first delta as well. */
            ANMPUTFIELD(anm->DAHEADER.nFrames,  numframes + 1);

            ANMPUTFIELD(anm->DAHEADER.maxRecsPerLp, 256);
            ANMPUTFIELD(anm->DAHEADER.lpfTableOffset, 1280);
            ANMPUTFIELD(anm->DAHEADER.contentType, 'M'<<24 | 'I'<<16 | 'N'<<8 | 'A');
            ANMPUTFIELD(anm->DAHEADER.width, 320);                  /* 320 x 200 frame size only */
            ANMPUTFIELD(anm->DAHEADER.height, 200);

            ANMPUTFIELD(anm->DAHEADER.variant, 0);
            ANMPUTFIELD(anm->DAHEADER.hasLastDelta, 1);        /* Last delta always present */
            ANMPUTFIELD(anm->DAHEADER.lastDeltaValid, 1);
            ANMPUTFIELD(anm->DAHEADER.pixelType, 0);
            ANMPUTFIELD(anm->DAHEADER.CompressionType,1);

            ANMPUTFIELD(anm->DAHEADER.otherRecsPerFrm, 0);
            ANMPUTFIELD(anm->DAHEADER.bitmaptype, 1);

            ANMPUTFIELD(anm->DAHEADER.framesPerSecond, 30);          /*Default 30 frames per second */

            /* Write header  */
            gwrite(g,&anm->DAHEADER,sizeof(lpfileheader));

            /* Alocate memory used in the conversion */
            anm->thepage = (unsigned char *) galloc(65536L);

            if (anm->thepage)
            {
                anm->temp_win    = (unsigned char *) galloc(ANMGETFIELD(anm->DAHEADER.width) * ANMGETFIELD(anm->DAHEADER.height));
                if (anm->temp_win)
                {
                    anm->first_frame = (unsigned char *) galloc(ANMGETFIELD(anm->DAHEADER.width) * ANMGETFIELD(anm->DAHEADER.height));
                    if (anm->first_frame)
                    {
                        anm->temp_data   = (unsigned char *) galloc(65536L);
                        if (anm->temp_data)
                        {
                            anm->animwin = (unsigned char *) galloc(ANMGETFIELD(anm->DAHEADER.width) * ANMGETFIELD(anm->DAHEADER.height));
                            if (anm->animwin)
                                ok = gx->frames = numframes;
                        }
                    }
                }
            }
        }
    }
    return(ok);
}


int GCALL ANM_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes)
{
    unsigned short count;
    unsigned char  ok = 0;
    unsigned char  f_frame = 0;
    ANMINFO  *anm;
    unsigned int  bitmap_size, data_size;

    if (ginfo->bpp<=8)
    {
        anm = (ANMINFO *) gx->gref;
        bitmap_size = ANMGETFIELD(anm->DAHEADER.width) * ANMGETFIELD(anm->DAHEADER.height);

        /* First, make the bitmap into a normal 320x200 (or whatever size) bitmap. (instead of rowbytesx200) */
        cropimage((unsigned char *) source, anm->temp_win,ANMGETFIELD(anm->DAHEADER.width), ANMGETFIELD(anm->DAHEADER.height), ginfo->width, ginfo->height, rowbytes);
    /*    memcpy(anm->temp_win,(unsigned char *) source,bitmap_size); */

        if (anm->curlpnum == 0xFFFF)
        {
            /* We know that this is the first frame being written, we must first write the */
            /* pallette info and leave space for the 256 6 byte LP descriptors before we can save an image */
            f_frame = 1;

            /* Write 128 0's to the file for the color cycling info */
            for (count = 0; count < 128; count++)
                gwrite(gx->gstream, &ok, 1);

            for (count = 0; count < 256; count++)
            {
                gwrite(gx->gstream, &ginfo->colourtbl[count].b, 1);   /* write blue  */
                gwrite(gx->gstream, &ginfo->colourtbl[count].g, 1);   /* write green */
                gwrite(gx->gstream, &ginfo->colourtbl[count].r, 1);   /* write red   */
                gwrite(gx->gstream, &ok, 1);                      /* write 0 to the unused (ignored) byte */
            }

            /* Write 1536 0's to the file - 256 LP header structures will be placed here once we have calculated them */
            for (count = 0; count < 1536; count++)
            gwrite(gx->gstream, &ok, 1);

            /* Now copy this frame into the first_frame bitmap window */
            memcpy(anm->first_frame, anm->temp_win,  bitmap_size);

            /* We must initialize the page */
            ANMPUTFIELD( anm->curlp.baseRecord,  0);     /* Baserecord = 0 */
            ANMPUTFIELD( anm->curlpnum, 0);                      /* Currnt LP num = 0 */
            make_page(anm);                              /* Init page */

        }

        if (f_frame == 0)
            /* If this isn't the first frame, we compress the next frame using the last frame */
            data_size = compress_data((unsigned char *)anm->temp_win, (unsigned char *)anm->animwin, (unsigned char *)anm->temp_data, bitmap_size);
        else
            /* If this is the fist frame, we compress the frame with no last frame */
            data_size = compress_data((unsigned char *)anm->temp_win, NULL, (unsigned char *)anm->temp_data, bitmap_size);

        /* Copy current bitmap into last frame bitmap for next iteration. */
        memcpy(anm->animwin , anm->temp_win, bitmap_size);

        /* Now move the temp_data data into thepage. If we reach the end of the the_page, we write the */
        /* current page and start a new page. */
        write_data_to_page(anm->temp_data, anm, data_size, gx->gstream);

        gx->framenum++;
        ok = 1;
    }

    return(ok);
}




int GCALL ANM_wclose(GINSTANCE *gx)
{
    ANMINFO  *anm;
    int      ok = 0, count;
    unsigned int data_size, bitmap_size;

    if (gx)
    {
        anm = (ANMINFO *) gx->gref;
        if (gx->framenum)
        {

            /* Make delta from last frame to first */
            bitmap_size = ANMGETFIELD(anm->DAHEADER.width) * ANMGETFIELD(anm->DAHEADER.height);
            data_size = compress_data((unsigned char *)anm->first_frame, (unsigned char *)anm->animwin, (unsigned char *)anm->temp_data, bitmap_size);
            write_data_to_page(anm->temp_data, anm, data_size, gx->gstream);

            /* Dump the current large page, stored in thepage, to disk. */
            dump_page(1, gx->gstream, anm);

            /* Now we write the number of Large Pages and frames we used */
            gseek(gx->gstream, 6);
            ANMPUTFIELD(anm->DAHEADER.nLps, ANMGETFIELD(anm->curlpnum) + 1);
            gwrite(gx->gstream,&anm->DAHEADER.nLps, 2);
            ANMPUTFIELD(anm->DAHEADER.nRecords, gx->framenum + 1);
            gwrite(gx->gstream,&anm->DAHEADER.nRecords, 4);
            gseek(gx->gstream, 64);
            ANMPUTFIELD(anm->DAHEADER.nFrames, gx->framenum + 1);
            gwrite(gx->gstream,&anm->DAHEADER.nFrames, 4);


            /* Now write the LpArray data */
            gseek(gx->gstream, ANMGETFIELD(anm->DAHEADER.lpfTableOffset) );
            for (count = 0; count <= (int) ANMGETFIELD(anm->DAHEADER.nLps) ; count++)
                gwrite(gx->gstream, &anm->LpArray[count], sizeoflp_descriptor);
        }

        /* Free memory */
        if (anm->animwin)      gfree((void *) anm->animwin);
        if (anm->temp_data)    gfree((void *) anm->temp_data);
        if (anm->first_frame)  gfree((void *) anm->first_frame);
        if (anm->temp_win)     gfree((void *) anm->temp_win);
        if (anm->thepage)      gfree((void *) anm->thepage);
        if (gx->gref)          gfree((void *) gx->gref);

        ok = gfree((void *) gx);
    }

    return(ok);
}


