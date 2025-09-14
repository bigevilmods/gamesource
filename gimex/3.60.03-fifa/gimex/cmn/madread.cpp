// (c) 1994-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*             Madcow - Motion And Discrete COsine Wizard           */
/*                                                                  */
/*                        by Kaz Okuda, EAC                         */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History:                                       */
/* ------- ----   -- --------                                       */
/*    1.00 970909 KO Uses madcow encoder by Patrick Ratto           */
/*    1.01 970911 KO Added importing                                */
/*    1.02 970916 KO Ignore audio chunks,trunk size to x*16,        */
/*    1.03 970916 FB Name tweeks                                    */
/*    1.04 971002 KO Doesn't update previous frame on extra frame   */
/*    1.05 971008 KO Correct fps on import, frame index             */
/*    1.06 971103 KO Reorders the data for motorola                 */
/*    1.07 971120 KO export crops frame size up. Resize odd frames  */
/*    1.08 980514 KO imp. quality=70, subtype changed, mem leak     */
/*    1.09 990803 FB ginfo->fps                                     */
/*    1.10 000404 FB about->movie                                   */
/*    1.11 000925 FB can export in about                            */
/*    1.12 020624 FB 29.97 fps default, framesize/max defaults      */
/*    1.13 020625 FB q80 default                                    */
/*    1.14 020710 FB buffer overflow bug fix                        */
/*    1.15 020925 FB 601 YUV coefficients & floats.  ver=1          */
/*	  1.16 021108 AE crc=reserved, flags for frame type, ver=2		*/
/*	  1.17 021212 AE added MAV (AVI with MAD movie chunks)			*/
/*	  1.18 030527 FB gop10,1 default, and clamp min framesize       */
/*	  1.19 030603 FB step quant faster for timeout                  */
/*	  1.20 031203 FB export buffer overflow fix on small images     */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* Files: madread.cpp madwrite.cpp madgimex.h                       */
/*        madenc.c madenc.h madlab.c madlab.h                       */
/*        madidct.c maddeca.c                                       */
/*                                                                  */
/* On export ginfo->subtype is used for a few things                */
/*      The low byte is the maximum frame size in KB                */
/*      The next byte is the number of frames per group             */
/*      The next byte of 1 means make extra frames (0 == no extra)  */
/*      The high byte is unused                                     */
/* Subtype of 0 uses defaults                                       */
/*      maxframesize = width*height*3/20                            */
/*      frames per group is 4 if fps<=15, 8 if fps>15               */
/*      extra frames only if fps>15                                 */
/* ginfo->framesize is used to specify the average framesize after  */
/*      compression (framesize*framespergroup is maximum groupsize) */
/*      default: sqrt(width*height)*42                              */
/* ginfo->fps is used to specify the frames per second (default 15) */
/* ginfo->quality is used for the motion threshold (100-quality)*10 */
/*      Default of q=70 -> mt=300.                                  */
/* ginfo->packed is not used on export                              */
/*                                                                  */
/* On import ginfo->subtype is used for similar things as above     */
/*      maximum frame size over the entire movie in KB              */
/*      Average number of frames per group over the movie           */
/*      The next byte of 1 means there is an extra frame in movie   */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include <string.h>
#include <math.h>
#include "gimex.h"
#include "madgimex.h"
#include "idxgimex.h"

#define CHUNKLOAD  0
#define CHUNKSKIP  1


/****************************************************************/
/*  Internal Functions                                          */
/****************************************************************/

static unsigned char cliprgb(float x)
{
    if (x > 255.0) return(255);
    if (x < 0.0) return(0);
    return((unsigned char) (x+0.5));
}

static void MADyuvtorgb(YUVPAIR *src, ARGB *dest, int width, int height, int rowbytes)
{
    int         y1,y2,cb,cr;
    int         stride;
    int         i,j;
    float       r;
    float       g;
    float       b;

    stride = rowbytes/4;

    for (i = 0; i < height; ++i)
    {
        for (j = 0; j < width/2; ++j)
        {
            y1 = ((int)src[i*width/2+j].y1) * 2 + 1 - 16;
            y2 = ((int)src[i*width/2+j].y2) * 2 + 1 - 16;
            cb = ((int)src[i*width/2+j].cb) * 2 - 127;
            cr = ((int)src[i*width/2+j].cr) * 2 - 127;

            r =                     + 1.596027f * cr  ;
            g = - (0.391762f * cb) - (0.812968f * cr) ;
            b = +  2.017232f * cb                     ;

//            r = 1.4020f*cr;
//            g = -0.3441f*cb - 0.7141f*cr;
//            b = 1.7720f*cb;

            float y = 1.164383f * y1;

            dest[i*stride+j*2].r = cliprgb(y + r);
            dest[i*stride+j*2].g = cliprgb(y + g);
            dest[i*stride+j*2].b = cliprgb(y + b);
            dest[i*stride+j*2].a = 255;

            y = 1.164383f * y2;

            dest[i*stride+j*2+1].r = cliprgb(y + r);
            dest[i*stride+j*2+1].g = cliprgb(y + g);
            dest[i*stride+j*2+1].b = cliprgb(y + b);
            dest[i*stride+j*2+1].a = 255;
        }
    }
}

static int GCALL isvideochunk(char *header)
{
    int ok = 0;

    switch(ggetm(header,4))
    {
        case GMAKEID('M','A','D','m'):
        case GMAKEID('M','A','D','e'):
        case GMAKEID('M','A','D','k'):
            ok = 1;
    }
    return(ok);
}

#define MADisalphanum(c) (c>=' ' && c<='z')

static int MADischunkid(char *id)
{
    if (MADisalphanum(id[0])
     && MADisalphanum(id[1])
     && MADisalphanum(id[2])
     && MADisalphanum(id[3]))
        return(1);
    return(0);
}

static int MADread(GSTREAM *g, char *buf, int len, int doit)
{
    int ok = 0;

    switch (doit)
    {
        default:
        case CHUNKLOAD:
            ok = gread(g,buf,len);
            break;

        case CHUNKSKIP:
            ok = gseek(g,gtell(g)+len);
            break;
    }
    return(ok);
}


/****************************************************************/
/*  Get Chunk                                                   */
/*  - loads next chunk into memory.  no parsing if just a chunk */
/*  - if not a chunk, convert into chunk                        */
/****************************************************************/

/* get a chunk.  just loads the chunk if chunk based file. */
/* non-chunk based 'chunks' are converted into chunks */

static char *MADgetchunk(GSTREAM *g, int doit)
{
    int  ok=0;
    int len;
    int filelen;
    int curpos;
    char buf[16];
    char *chunk=0;

    curpos =  (int)gtell(g);
    filelen = (int)glen(g);
    if (curpos<filelen)
        ok = gread(g,buf,8);
    if (ok)
    {
        /* if chunk, just load it */

        len = ggeti(buf+4,4);
        ok = 0;
        if (MADischunkid(buf) && (len+curpos)<=filelen)
        {
            chunk = (char*) galloc(len);
            if (chunk)
            {
                memcpy(chunk,buf,8);
                ok = MADread(g,chunk+8,len-8,doit);
                if (!ok)
                {
                    gfree(chunk);
                    chunk = 0;
                }
            }
        }
    }
    return(chunk);
}

static int MADnumchunks(GSTREAM *g, MADINFO *mad)
{
    char buf[8];
    int newpos=0;
    int frames=0;
    int ok=1;
    IDXStruct index;
    int maxsize = 0;
    int keycount = 0;
    int extracount = 0;
	int dataSize = 0;

    gseek(g,newpos);

	if(mad->subtype & SUBTYPE_MAV)
	{
		/* skip the headers */
		gread(g,buf,4);
		newpos += 4;
		while(ggetm(buf,4) != GMAKEID('m','o','v','i'))
		{
			newpos += 1;//(int) gtell(g) + 1;
			gseek(g, newpos);
			gread(g,buf,4);
			//gseek(g, newpos + 1);
		}
				
		gseek(g, gtell(g) -8);
		gread(g,buf,4);
		dataSize = ggeti(buf,4);
		
		/* g is pointing at first AVI chunk id (ie. '00dc') */	
		gseek(g, gtell(g) + 4);
		newpos += 4;
		
	}

    while(ok && (gread(g, buf, 8) == 8))
    {
        if( (mad->subtype & SUBTYPE_MAV) )
		{
			if(ggetm(buf, 4) == GMAKEID('0','0','d','c'))
			{
				gread(g, buf, 8); // the first read, reads in '00dcsize'
				newpos += 8;
			}
			else if	(ggetm(buf, 4) == GMAKEID('i','d','x','1')) // end of the movi data
			{
				ok = 0;
			}
			else 	// non-video avi chunk type
			{
				newpos += 8;
			}
		
		}

		if (isvideochunk(buf))
        {

            /* Set the ID */
            memcpy(&index.identifier[0], buf, 4);

            /* Set the flags */
            switch(ggetm(buf,4))
            {
                case GMAKEID('M','A','D','m'):
                    index.flags = IDXMOTION;
                    break;
                case GMAKEID('M','A','D','e'):
                    index.flags = IDXEXTRA;
                    extracount++;
                    break;
                case GMAKEID('M','A','D','k'):
                    index.flags = IDXKEY;
                    keycount++;
                    break;
            }
            index.offset = newpos;

            index.length = ggeti(buf+4, 4);
            if (index.length > maxsize)
            {
                maxsize = index.length;
            }

            IDXaddindex(mad->table, &index);
            frames++;
		
		}
			
		newpos += ggeti(buf+4, 4);
     

        if ( (newpos >= 0) && (newpos <= glen(g)-8) )
        {
            gseek(g, newpos);
        }
        else
        {
            ok = 0;
        }
    }

    mad->maxframesize = maxsize;
    mad->keycount = keycount;
    mad->extracount = extracount;

    return(frames);
}

static void MADreorderbytes(unsigned short *data, int size)
{
    int i;

    for (i=0;i<size;i++)
    {
        data[i] = (unsigned short) ggeti(&data[i], 2);
    }
}

static int MADreadchunk(GINSTANCE *gx, char *dest, int rowbytes)
{
    MADCHUNKHDR header;
    MADINFO *mad;
    YUVPAIR *tempyuv;
    char *chunk;
    int width;
    int height;
    int index;
    int i;
    int j;
    int ok=0;

    mad = (MADINFO*)gx->gref;

    if ( gread(gx->gstream, &header, sizeof(MADCHUNKHDR)) == sizeof(MADCHUNKHDR) )
    {
        width = MADGETFIELD(header.width);
        height = MADGETFIELD(header.height);
        if (!mad->lastyuv)
            mad->lastyuv = (YUVPAIR *) galloc(width*height/2*sizeof(YUVPAIR));
        if (mad->lastyuv)
        {
            tempyuv = (YUVPAIR *) galloc(width*height/2*sizeof(YUVPAIR));
            if (tempyuv)
            {
                chunk = (char *) galloc(MADGETFIELD(header.size)-sizeof(MADCHUNKHDR));

                if ( gread(gx->gstream, chunk, MADGETFIELD(header.size)-sizeof(MADCHUNKHDR)) == (int)(MADGETFIELD(header.size) - sizeof(MADCHUNKHDR)) )
                {
                    MADreorderbytes((unsigned short*) chunk, (MADGETFIELD(header.size)-sizeof(MADCHUNKHDR))/2);

                    if ( MADGETFIELD(header.id) == GMAKEID( 'k', 'D' , 'A', 'M' ) )
                        madinitdecode((unsigned short*) chunk , 0, MADGETFIELD(header.quality));
                    else
                        madinitdecode((unsigned short*) chunk, 1, MADGETFIELD(header.quality));

                    for (i = 0; i < height; i += 16)
                    {
                        for ( j = 0; j < width; j += 16)
                        {
                            index = (i*width+j)/2;
                            maddecodemacroblock(&mad->lastyuv[index], &tempyuv[index], width);
                        }
                    }

                    MADyuvtorgb(tempyuv, (ARGB*)dest, width, height, rowbytes);

                    ok = 1;
                    gx->framenum++;

                    /* Only replace the last frame if not an extra chunk */
                    if ( MADGETFIELD(header.id) == GMAKEID( 'e', 'D' , 'A', 'M' ) )
                    {
                        gfree(tempyuv);
                    }
                    else
                    {
                        gfree(mad->lastyuv);
                        mad->lastyuv = tempyuv;
                    }
                }
                gfree(chunk);
            }
        }
    }
    return(ok);

}



/****************************************************************/
/*  Information Functions                                       */
/****************************************************************/

GABOUT * GCALL MAD_about(void)
{
    GABOUT *info;

    info = (GABOUT *) galloc(sizeof(GABOUT));
    if (info)
    {
        memset(info, 0, sizeof(GABOUT));

        info->signature       = GMAKEID('M','A','D','C');
        info->size            = sizeof(GABOUT);
        info->version         = GIMEX_VERSION;
        info->canimport       = 1;    /* supports importing */
        info->canexport       = 1;    /* supports exporting */
        info->importpacked    = 1;    /* supports importing packed */
        info->exportpacked    = 1;    /* supports exporting packed */
        info->import8         = 0;    /* supports importing 8 bit indexed */
        info->export8         = 0;    /* supports exporting 8 bit indexed */
        info->import32        = 1;    /* supports importing 32 bit direct rgb */
        info->export32        = 1;    /* supports exporting 32 bit direct rgb */
        info->multiframe      = 1;    /* supports multiple frames */
        info->multifile       = 0;    /* format requires additional files or resource fork */
        info->multisize       = 1;    /* supports each frame different size */
        info->framebuffer     = 0;    /* module requires memory to buffer entire frame */
        info->external        = 0;    /* module uses external resources */
        info->usesfile        = 1;    /* module is file based */
        info->maxframename    = 0;    /* max frame name character */
        info->defaultquality  = 80;   /* default pack quality */
        info->movie           = 1;    /* movie file format */
        info->dotsubtype      = 1;    /* subtype based on extension */
		info->mactype[0]      = GMAKEID('M','A','D','C');   /* mac file system types used */
        strcpy(info->extensions[0], ".mad");                /* extensions */
        strcpy(info->extensions[1], ".mav");
		strcpy(info->authorstr,     "Patrick Ratto, F Barchard"); /* author */
        strcpy(info->versionstr,    "1.20");                /* version # */
        strcpy(info->shorttypestr,  "Mad");                 /* type */
        strcpy(info->wordtypestr,   "Madcow");              /* wordtype */
        strcpy(info->longtypestr,   "Madcow Movie");        /* longtype */
    }
    return(info);
}

/* check for reasonable header: */
/* 'MADk' intel chunk as either first or second chunk (audio) */

int GCALL MAD_is(GSTREAM *g)
{
    unsigned char madheader[16];
	int ok=0;
    int chunklen;
    char *chunk;

    gseek(g,(GPOS)0);
    if (gread(g,madheader,8))
    {
        if (ggetm(madheader,4)==GMAKEID('M','A','D','k'))
            ok = 100;
		else if ( ggetm(madheader,4) == GMAKEID('R','I','F','F'))	// for MAV
		{
			gseek(g,0); 
			if (gread(g, madheader, 16))
			{
				if( ggetm(madheader+8,4)==GMAKEID('A','V','I',' ') )	// dealing with AVI
				{
					// need to see if compression FOURCC is MADk
					gseek(g,0xbc); 
					gread(g,madheader,4);
					if( ggetm(madheader,4) == GMAKEID('M','A','D','k') )
					{
						ok = 100;
					}
				}      
			}
		}
		else
        {
            chunklen = ggeti(madheader+4,4);
            if (chunklen>8 && chunklen<200000)
            {
                if (MADischunkid((char*)madheader))
                {
                    gseek(g,(GPOS)0);

                    chunk = MADgetchunk(g, 0);
                    if (chunk)
                    {
                        if (gtell(g)<glen(g))
                        {
                            if (gread(g,madheader,4))
                            {
                                if (ggetm(madheader,4)==GMAKEID('M','A','D','k'))
                                    ok = 100;
                            }
                        }
                        gfree(chunk);
                    }
                }
            }
        }
    }
    return(ok);
}


/****************************************************************/
/*  Import Functions                                            */
/****************************************************************/

int GCALL MAD_open(GINSTANCE **gxptr, GSTREAM *g, const char *pathname,bool framecountflag)
{
    GINSTANCE *gx;
    int ok=0;
    MADINFO *mad;
	char buf[4];

    gx = (GINSTANCE *) galloc(sizeof(GINSTANCE));
    if (gx)
    {
        memset(gx, 0, sizeof(GINSTANCE));
        gx->gstream = g;
        gx->framenum = 0;
        gx->gref = (char*)galloc(sizeof(MADINFO));
        if (gx->gref)
        {
            memset(gx->gref, 0, sizeof(MADINFO));
            *gxptr = gx;
            mad = (MADINFO *)gx->gref;
            mad->table = IDXnewtable(100);
            mad->lastframe = -1;

			/*Find out if we're dealing with MAV */
			/*In MAD_is we checked whether this was a MAD or a MAV */
			gseek(g,0);
			gread(g,buf,4);	
			if(ggetm(buf,4) == GMAKEID('R','I','F','F'))
				mad->subtype = SUBTYPE_MAV;
			else
				mad->subtype = 0;
			
            gx->frames = MADnumchunks(g, mad);
            ok = gx->frames;
        }
    }
    return(ok);
}


GINFO * GCALL MAD_info(GINSTANCE *gx, int framenum)
{
    GINFO *ginfo=0;
    MADCHUNKHDR header;
    MADINFO *mad;
    GSTREAM *g;
    int flags = 0;
    int quality;
    int subtype;

    g = gx->gstream;
    mad = (MADINFO*)gx->gref;

    if (framenum < gx->frames)
        ginfo = (GINFO *) galloc(sizeof(GINFO));

    if (ginfo)
    {
        memset(ginfo, 0, sizeof(GINFO));

        if ( IDXseekframe(mad->table, g, framenum) )
        {
            gread(g,&header,sizeof(MADCHUNKHDR));

            switch(MADGETFIELD(header.id))
            {
                case GMAKEID('m','D','A','M'):
                    flags = MADMOTIONFLAG;
                    break;
                case GMAKEID('e','D','A','M'):
                    flags = MADEXTRAFLAG;
                    break;
                default:
                    flags = MADKEYFLAG;
            }

            /*quality = (int) gmax((5.0 - log(MADGETFIELD(header.quality))\log(2.0)) * 25.0, 0); */
            /* Always set the quality to 70 so that if this frame is exported as a madcow it uses this default. */
            quality = 70;

            subtype = (mad->extracount?1:0)<<16 |
                      (mad->keycount?((gx->frames + (mad->keycount+1)/2) / mad->keycount):1)<<8 |
                      (mad->maxframesize / 1000);

            ginfo->signature        = GMAKEID('M','A','D','C');   /* Type of file */
            ginfo->size             = sizeof(GINFO);           /* Length of GINFO */
            ginfo->version          = GIMEX_VERSION;
            ginfo->framenum         = framenum;                     /* Current frame num */
            ginfo->width            = MADGETFIELD(header.width);    /* Width of bitmap in pixels */
            ginfo->height           = MADGETFIELD(header.height);   /* Height of bitmap in pixels */
            ginfo->subtype          = subtype;
            ginfo->packed           = ggetm(header.id,4);
            ginfo->quality          = quality;
            ginfo->framesize        = MADGETFIELD(header.size);     /* size of frame in bytes */
            ginfo->alphabits        = 0;
            ginfo->redbits          = 8;
            ginfo->greenbits        = 8;
            ginfo->bluebits         = 8;
            ginfo->centerx          = 0;
            ginfo->centery          = 0;
            ginfo->defaultx         = 0;
            ginfo->defaulty         = 0;
            ginfo->numhotspots      = 0;
            ginfo->bpp              = 32;
            ginfo->originalbpp      = 24;
            ginfo->numcolours       = 0;
            if (MADGETFIELD(header.time))
                ginfo->fps = 1000.0f * 65536.0f / (float)MADGETFIELD(header.time);


        }
    }
    return(ginfo);
}


int GCALL MAD_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes)
{
    int ok = 1;
    MADINFO *mad;
    int frame;

    mad = (MADINFO*)gx->gref;

    frame = IDXseekkey(mad->table, gx->gstream, ginfo->framenum);
    ok = (frame >= 0);

    /* Check if lastframe is between keyframe and current frame */
    if ( (frame < mad->lastframe+1) && (mad->lastframe < ginfo->framenum) )
    {
        frame = mad->lastframe+1;
        ok = IDXseekframe(mad->table, gx->gstream, frame);
    }

    while( ok && (frame <= ginfo->framenum) )
    {
        ok = MADreadchunk(gx, dest, rowbytes);
        frame++;
        ok = ok && IDXseekframe(mad->table, gx->gstream, frame);
    }

    /* If Correct termination */
    if (frame > ginfo->framenum)
        ok = 1;

    mad->lastframe = ginfo->framenum;

    return(ok);
}


int GCALL MAD_close(GINSTANCE *gx)
{
    MADINFO *mad;

    if (gx)
    {
        if (gx->gref)
        {
            mad = (MADINFO*)gx->gref;
            if (mad->lastyuv) gfree(mad->lastyuv);
            if (mad->table) IDXfreetable(mad->table);
            gfree(gx->gref);
        }
        gfree(gx);
    }
    return(1);
}


