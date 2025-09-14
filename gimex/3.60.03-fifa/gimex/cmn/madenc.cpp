// (c) 1997-2007 Electronic Arts Inc.

#include <stdio.h>
#include <stdlib.h>

#include "gimex.h"
#include "madlab.h"
#include "madenc.h"


/* Maximum allowable frames in a group. */
#define MAXGROUPFRAMES  50

/* Initialization parameters. */
static int             framewidth;
static int             frameheight;
static GSTREAM         *outstream;
static ENCODEPARAMS    parms;

/* Information on group of frames. */
static int             framecount; /* frames currently buffered */
static int             rlelimit; /* size of RLE buffers */
static int             lastqualityindex; /* from last frame of previous group */
static FRAMETYPE       framebuffer[MAXGROUPFRAMES];
static int             qualityindex[MAXGROUPFRAMES];
static unsigned short  *rlebuffer[MAXGROUPFRAMES];
static MOTIONINFOTYPE  *motioninfo[MAXGROUPFRAMES];
static int             frameperiod[MAXGROUPFRAMES];

/* Temporary work buffers. */
static FRAMETYPE       tempframe1;
static FRAMETYPE       tempframe2;
static unsigned short  *temprle;
static unsigned char   *outputbuffer;


/* Called with chunk header each time a frame is output. */
MADCALLBACKFN madcallback;

/*
;
; Releases all of the memory allocated by madinitencode().
;
*/

static void releasestorage(void)
{
    int     i;

    for (i = 0; i < parms.groupframes; ++i)
    {
        madreleaseframe(&framebuffer[i]);

        if (rlebuffer[i] != NULL)
        {
            gfree(rlebuffer[i]);
            rlebuffer[i] = NULL;
        }

        if (motioninfo[i] != NULL)
        {
            gfree(motioninfo[i]);
            motioninfo[i] = NULL;
        }
    }

    madreleaseframe(&tempframe1);
    madreleaseframe(&tempframe2);

    if (temprle != NULL)
    {
        gfree(temprle);
        temprle = NULL;
    }

    if (outputbuffer != NULL)
    {
        gfree(outputbuffer);
        outputbuffer = NULL;
    }
}

/*
;
; Adjusts the quality indices of the first n buffered frames so that
; they do not exceed maxbits each if encoded as key frames. On exit, the
; RLE buffers will contain the encoded data at these quality indices.
;
*/

static void adjustquality(int n, int maxbits)
{
    unsigned short  *temp;
    int             lastquality;
    int             bits;
    int             i;

    lastquality = lastqualityindex;

    for (i = 0; i < n; ++i)
    {
        /* Try the quality index from the previous frame. */
        madsetquality(lastquality);
        if (madcompressframe(&framebuffer[i], rlebuffer[i], rlelimit) == rlelimit)
        {
            bits = maxbits+1;
        }
        else
        {
            madsizeframe(rlebuffer[i], NULL, &bits, NULL);
        }

        if (bits > maxbits)
        {
            /* If it doesn't fit, reduce the quality until it does. */
            while (bits > maxbits && lastquality<256)
            {
				if (lastquality<20)
	                ++lastquality;
				else if (lastquality<30)
	                lastquality += 2;
				else 
	                lastquality += 8;

                madsetquality(lastquality);

                if (madcompressframe(&framebuffer[i], rlebuffer[i], rlelimit) == rlelimit)
                {
                    bits = maxbits+1;
                }
                else
                {
                    madsizeframe(rlebuffer[i], NULL, &bits, NULL);
                }
            }
        }
        else
        {
            /* If it fits, increase the quality until it doesn't. */
            /*for (;;) */
            while(lastquality > 1)
            {
                --lastquality;
                madsetquality(lastquality);
                if (madcompressframe(&framebuffer[i], temprle, rlelimit) == rlelimit)
                {
                    bits = maxbits+1;
                }
                else
                {
                    madsizeframe(temprle, NULL, &bits, NULL);
                }

                if (bits > maxbits)
                {
                    /* It no longer fits, exit with the last quality that did. */
                    ++lastquality;
                    break;
                }
                else
                {
                    /* It still fits, save the RLE buffer in case this is it. */
                    temp = rlebuffer[i];
                    rlebuffer[i] = temprle;
                    temprle = temp;
                }
            }
        }

        qualityindex[i] = lastquality;
    }
}

/*
;
; Performs motion analysis on the first n buffered frames.
; Uses the quality index of each frame to set the motion threshold
; (lower quality frames can have a higher motion threshold).
;
; Motion analysis is performed independently of compression.
; To prevent slow moving scenes from updating only on key frames,
; we apply the motion compensation to each frame before it can be
; used as a reference frame.
;
*/

static void analyzemotion(int n)
{
    signed char *temp;
    int         i;

    /* The key frame is the first reference frame. */
    madcopyframe(&framebuffer[0], &tempframe1);

    for (i = 1; i < n; ++i)
    {
        madsetmotionthreshold(parms.motionthreshold*qualityindex[i]);
        madanalyzemotion(&framebuffer[i], &tempframe1, motioninfo[i]);

        /* If this is a motion frame (not an extra frame), apply the */
        /* motion compensation to it and use it as a reference for */
        /* the next frame. */
        if (!(parms.extraflag && ((i % 2) == 1)))
        {
            madcopyframe(&framebuffer[i], &tempframe2);
            madapplymotion(&tempframe1, &tempframe2, motioninfo[i]);
            temp = tempframe1.data;
            tempframe1.data = tempframe2.data;
            tempframe2.data = temp;
        }
    }
}

/*
;
; Computes and returns the total size of the first n buffered frames
; in bytes if they were output now (not including the chunk header
; and pad bytes).
;
*/

static int computegroupsize(int n)
{
    int     groupsize;
    int     key;
    int     motion;
    int     i;

    /* Do the key frame. */
    madsizeframe(rlebuffer[0], NULL, &key, NULL);
    groupsize = (key+7)/8;

    /* Do the motion\extra frames. */
    for (i = 1; i < n; ++i)
    {
        madsizeframe(rlebuffer[i], motioninfo[i], &key, &motion);
        groupsize += (motion+7)/8;
    }

    return(groupsize);
}

/*
;
; Outputs the Chunk Header structure.
; Writes each element individually correcting for byte order
; so will work on SGI.
;
*/
static int outputheader(GSTREAM *g, CHUNKHDR *header)
{
    int ok=1;
    char buf[4];

    ok &= (4 == gwrite(g, &header->id[0], 4));

    gputi(buf, header->size, 4);
    ok &= (4 == gwrite(g,buf,4));

    //gputi(buf, header->reserved, 4);	// used to be crc
    ok &= (4 == gwrite(g,&header->reserved[0],4));

    gputi(buf, header->time, 4);
    ok &= (4 == gwrite(g,buf,4));

    gputi(buf, header->width, 2);
    ok &= (2 == gwrite(g,buf,2));

    gputi(buf, header->height, 2);
    ok &= (2 == gwrite(g,buf,2));

    ok &= (1 == gwrite(g, &header->ver, 1));
    ok &= (1 == gwrite(g, &header->quality, 1));
	ok &= (1 == gwrite(g, &header->flags, 1));
    ok &= (1 == gwrite(g, &header->reserved2, 1));

    /*ok = gwrite(g, &header, sizeof(header)) == 0; */
    return(ok);
}

/*
;
; Outputs the first n buffered frames to the output stream.
; A chunk header is created for each frame and the chunks are
; padded out to a multiple of four bytes each.
; Returns a non-zero value if an error occurs.
;
; The MADCOW chunk header has the following layout:
;
;   Chunk ID (4 bytes): MADk (key), MADm (motion), MADe (extra)
;   Chunk Size (4 bytes) in bytes
;	reserved	(4 bytes) : used to be CRC   
;   Time (4 bytes): milliseconds to next frame (fixed 16.16)
;   Frame width (2 bytes) in pixels
;   Frame height (2 bytes) in pixels
;   Version number (1 byte):  0 = original , 1 = 601 style YUV coeficients . 2 = flags present
;   Quality index (1 byte)
;	Flags (1 byte): : 0= motion, 1 = keyframe, 2 = extra frame
;   Reserved2 (1 byte)
;
*/

static int outputframes(int n)
{
    CHUNKHDR    header;
    int         count;
    int         i;

    /* Set up the chunk header. */
    header.id[0] = 'M';
    header.id[1] = 'A';
    header.id[2] = 'D';

    header.reserved[0] = 0;	// used to be CRC
	header.reserved[1] = 0;
	header.reserved[2] = 0;
	header.reserved[3] = 0;

    header.time = frameperiod[0];
    header.width = (short)framewidth;
    header.height = (short)frameheight;
    header.ver = 2;  // version 1 changes YUV to 601 standard, ver 2 means flags present
	header.reserved2[0] = 0;
    //header.reserved2[1] = 0;

    /* Do the key frame. */
    count = madencodeframe(rlebuffer[0], NULL, outputbuffer);
    while ((count % 4) != 0)
    {
        outputbuffer[count] = 0;
        ++count;
    }

    header.id[3] = 'k';
    header.size = sizeof(header) + count;
    header.quality = (unsigned char) qualityindex[0];
	header.flags = 0x01; 

    if (madcallback) madcallback(&header);
    if (outputheader(outstream, &header) == 0) return(1);
    if (gwrite(outstream, outputbuffer, count) == 0) return(1);

    /* Do the motion\extra frames. */
    for (i = 1; i < n; ++i)
    {
        count = madencodeframe(rlebuffer[i], motioninfo[i], outputbuffer);
        while ((count % 4) != 0)
        {
            outputbuffer[count] = 0;
            ++count;
        }

        if (parms.extraflag && ((i % 2) == 1)) 
		{
			header.id[3] = 'e'; 
			header.flags = 0x02;
		}
        else 
		{
			header.id[3] = 'm';
			header.flags = 0x00;
		}
        header.size = sizeof(header) + count;
        header.quality = (unsigned char) qualityindex[i];
        header.time = frameperiod[i];

        if (madcallback) madcallback(&header);
        if (outputheader(outstream, &header) == 0) return(1);
        if (gwrite(outstream, outputbuffer, count) == 0) return(1);
    }

    return(0);
}

/*
;
; Encodes and outputs a group of n frames.
; Returns a non-zero value if an error occurs.
;
; The encoder works as follows:
;
; 1) Compute the quality index of each frame so that it does not exceed
;    the maximum allowable frame size if encoded as a key frame.
; 2) Perform motion analysis using the quality indices computed in
;    step 1 to determine the motion thresholds.
; 3) Figure out how large the group of frames is with motion compensation
;    factored in.
; 4) If the group is too large, recompute the quality indices to reduce
;    the maximum frame size by the excess per frame. The idea here is that
;    we did not get as much as we expected from the motion compensation,
;    so the motion must be very hectic and we can therefore compress
;    the frames more without it being noticeable.
; 5) Output the group of frames.
;
*/

static int encodegroup(int n)
{
    int     groupsize;
    int     excess;
    int     error;

    adjustquality(n, 8*parms.maxframesize);
    analyzemotion(n);
    groupsize = computegroupsize(n);
    excess = (groupsize - parms.maxgroupsize)/parms.groupframes;
    if (excess > 200)
        adjustquality(n, 8*(parms.maxframesize-excess));
    lastqualityindex = qualityindex[n-1];
    error = outputframes(n);
    return(error);
}

/*
;
; Initialize the MADCOW encoder.
; Returns a non-zero value if an error occurs.
;
; Parameters width and height specify the width and height in pixels
; of the images to be encoded. Parameter dest specifies the GSTREAM
; to which the encoded frames will be written. Parameter p specifies
; the encoding parameters as follows:
;
; maxframesize: maximum size of each frame if encoded without motion
;   compensation (in bytes).
;
; maxgroupsize: maximum size for each encoded group of frames. This
;   should normally be about 3/4 of maxframesize*groupframes for good
;   results. Note that the encoder does not guarantee that a particular
;   group will not exceed this size.
;
; groupframes: the number of frames in a group.
;
; extraflag: specifies whether to use extra frames. A movie encoded
;   without extra frames has one keyframe followed by a chain of motion
;   frames in each group. Movies with extra frames have the even frames
;   encoded as above and the odd frames encoded as extra frames, which
;   are simply motion frames that use the previous even frame as a
;   motion reference. This feature is normally used for double frame
;   rate movies (24 or 30 fps).
;
; motionthreshold: this is the threshold used for motion compensation.
;   The motion threshold is the limit on the sum of the square of errors
;   between a source and a reference block after DC correction. A value
;   of 300 is a good default. Note that the encoder automatically
;   increases the motion threshold for lower quality frames.
;
; period: this is the interval between frames in milliseconds, as a
;   16.16 fixed point number. This value is written to each chunk header.
;
*/

int madinitencode(int width, int height, ENCODEPARAMS *p, GSTREAM *dest)
{
    int     blocks;
    int     fail;
    int     i;

    if (((width % 16) != 0) || ((height % 16) != 0)) return(1);
    if (p->groupframes > MAXGROUPFRAMES) return(2);

    framewidth = width;
    frameheight = height;
    outstream = dest;
    parms = *p;

    framecount = 0;
    rlelimit = 2*parms.maxframesize;
    lastqualityindex = 1;

    blocks = (framewidth/16)*(frameheight/16);
    fail = 0;

    for (i = 0; i < parms.groupframes; ++i)
    {
        if (madallocateframe(&framebuffer[i], framewidth, frameheight)) fail = 1;
        rlebuffer[i] = (unsigned short *) galloc(rlelimit*sizeof(unsigned short));
        motioninfo[i] = (MOTIONINFOTYPE *) galloc(blocks*sizeof(MOTIONINFOTYPE));
        if ((rlebuffer[i] == NULL) || motioninfo[i] == NULL) fail = 1;
    }

    if (madallocateframe(&tempframe1, framewidth, frameheight)) fail = 1;
    if (madallocateframe(&tempframe2, framewidth, frameheight)) fail = 1;
    temprle = (unsigned short *) galloc(rlelimit*sizeof(unsigned short));

//    outputbuffer = (unsigned char *) galloc(parms.maxframesize+1);
    outputbuffer = (unsigned char *) galloc(parms.maxframesize+1024);  // 1024 has extra padding to allow overflow beyond size estimate

    if ((temprle == NULL) || outputbuffer == NULL) fail = 1;

    if (fail)
    {
        releasestorage();
        return(10);
    }

    madinitdct();
    madinitmotion();

    return(0);
}

/*
;
; Sends the specified frame to the MADCOW encoder.
; Frames will not be encoded until an entire group has been buffered.
; Returns a non-zero value if an error occurs.
;
*/

int madencode(ARGB *rgbframe, int rowbytes, int period)
{
    int     error;

    frameperiod[framecount] = period;
    madrgbtoyuv(rgbframe, rowbytes, &framebuffer[framecount]);

    ++framecount;
    if (framecount == parms.groupframes)
    {
        error = encodegroup(framecount);
        framecount = 0;
        return(error);
    }

    return(0);
}

/*
;
; Ends the encoding process and releases the resources allocated by
; madinitencode(). Any buffered frames will first be encoded and output.
; Returns a non-zero value if an error occurs.
;
*/

int madendencode(void)
{
    int     error;

    if (framecount == 0) error = 0;
    else error = encodegroup(framecount);

    releasestorage();

    return(error);
}

