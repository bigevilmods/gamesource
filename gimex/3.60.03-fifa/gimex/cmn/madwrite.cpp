// (c) 1994-2007 Electronic Arts Inc.

#include <string.h>
#include <math.h>
#include "gimex.h"
#include "madgimex.h"
#include "madenc.h"

#include "stdio.h"

#if defined(__APPLE__)
inline float sqrtf(float v)
{
	return (float) sqrt((double) v);
}
#endif

struct MADAviIndex
{
   int Identifier;
   int Flags;
   int Offset;
   int Length;
};

/* Required globals for writting MAD AVI's (MAVs)*/
const int mavchunkid = GMAKEID('c','d','0','0');
MADAviIndex *gpMADAviIndex=NULL;
GSTREAM* gWriteStream;
int gSubtype;
int gCurrentFrame = 0;
int gDataSize =0;
unsigned int gMaxChunkSize=0;


// MAD callback function. 
// This function gets called before every chunk is written
// to the stream. Perfect place for writting 
// AVI chunk headers
void mavwritechunk(CHUNKHDR *hdr)
{
	if(SUBTYPE_MAV & gSubtype)
	{
	
		gwrite(gWriteStream,&mavchunkid,4);
		gwrite(gWriteStream,&hdr->size,4);
			
		int frameType = ggetm(hdr->id,4);

		gpMADAviIndex[gCurrentFrame].Identifier = GMAKEID('c','d','0','0'); //ggeti(buf,4);
		gpMADAviIndex[gCurrentFrame].Flags		 = (frameType == GMAKEID('M','A','D','k')) ? 0x10 : 0;	// 0x10: keyframe
		gpMADAviIndex[gCurrentFrame].Offset	 = 4 + gDataSize;
		gpMADAviIndex[gCurrentFrame].Length	 = hdr->size;
	
		gDataSize += hdr->size + 8; // +8 is for '00dcsize' : 8 bytes
		gCurrentFrame++;

		if(hdr->size > gMaxChunkSize)
		{
			gMaxChunkSize = hdr->size;
		}
	}

}

static int writeAVIHeaders(const GINFO* ginfo, const MADINFO* info, GINSTANCE* gx)
{
	int ok=0;
    char buf[56];
    int hsize;
    int bpp;
    int TimeBetweenFrames;
    int datahandler;
	int maxDataSize;
	int maxBps;

    GSTREAM *g = gx->gstream;

	float fps = info->fps;
    
    datahandler = 0; /* default */
    bpp = 32;  
	TimeBetweenFrames = (int)(1000000.0f/fps);
	maxDataSize = ginfo->width * ginfo->height * 4;	
	maxBps = (int) ((float) maxDataSize * fps);

    /* Output initial file chunk header (Leaving buf+4 for later) */
    gputm(buf,GMAKEID('R','I','F','F'),4);
    gputm(buf+8,GMAKEID('A','V','I',' '),4);
    ok = gwrite(g,buf,12);

    /* Output AVI header chunk header */
    hsize = 192;
    gputm(buf,GMAKEID('L','I','S','T'),4);
    gputi(buf+4,hsize,4);
    gputm(buf+8,GMAKEID('h','d','r','l'),4);
    ok = gwrite(g,buf,12);

    /* Output AVI header */
    gputm(buf,GMAKEID('a','v','i','h'),4);
    gputi(buf+4,56,4);
    ok = gwrite(g,buf,8);

    gputi(buf, TimeBetweenFrames, 4);   /* TimeBetweenFrames (1000000/15fps) */
    gputi(buf+4, maxBps , 4);			/* MaxBps */
    gputi(buf+8, 0, 4);                 /* PaddingGranularity */
    gputi(buf+12, 0x00000010, 4);       /* Flags (Index exists) */
    gputi(buf+16, gx->frames, 4);		/* TotalFrames */
    gputi(buf+20, 0, 4);                /* InitFrames */
    gputi(buf+24, 1, 4);                /* Streams */
    gputi(buf+28, maxDataSize, 4);      /* SuggestedBufferSize (Modified later)*/
    gputi(buf+32, ginfo->width, 4);     /* Width */
    gputi(buf+36, ginfo->height, 4);    /* Height */
    gputi(buf+40, TimeBetweenFrames,4); /* TimeScale */
    gputi(buf+44, 0, 4);                /* DataRate */
    gputi(buf+48, 0, 4);                /* StartTime */
    gputi(buf+52, 0, 4);                /* DataLength */
    ok = gwrite(g,buf,56);

    /* Output video stream header chunk header */
    hsize -= 76;
    gputm(buf,GMAKEID('L','I','S','T'),4);
    gputi(buf+4,hsize,4);
    gputm(buf+8,GMAKEID('s','t','r','l'),4);
    ok = gwrite(g,buf,12);

    /* Output video stream header */
    gputm(buf,GMAKEID('s','t','r','h'),4);
    gputi(buf+4,56,4);
    ok = gwrite(g,buf,8);

    gputm(buf, GMAKEID('v','i','d','s'), 4); /* DataType */
    gputm(buf+4, datahandler, 4);       /* DataHandler */
    gputi(buf+8,  0x00000000, 4);       /* Flags */
    gputi(buf+12, 0, 4);                /* Priority */
    gputi(buf+16, 0, 4);                /* InitialFrames */
    gputi(buf+20, TimeBetweenFrames, 4);/* TimeScale */
    gputi(buf+24, 0x000f4240, 4);       /* DataRate */
    gputi(buf+28, 0, 4);                /* StartTime */
    gputi(buf+32, gx->frames, 4);     /* DataLength */
    gputi(buf+36, 0x00080000, 4);       /* SuggestedBufferSize (See above)*/
    gputi(buf+40, 0, 4);				/* Quality (10000 Max) */
    gputi(buf+44, 0, 4);                /* SampleSize */
    gputi(buf+48, 0, 4);                /* Reserved */
    gputi(buf+52, 0, 4);                /* Reserved */
    ok = gwrite(g,buf,56);

    /* Output Bitmap info header */
    hsize = 40;
    gputm(buf,GMAKEID('s','t','r','f'),4);
    gputi(buf+4,hsize,4);
    ok = gwrite(g,buf,8);

    gputi(buf, 40, 4);							 /* Size */
    gputi(buf+4, ginfo->width, 4);				 /* Width */
    gputi(buf+8, ginfo->height, 4);              /* Height */
    gputi(buf+12, 1, 2);						 /* Planes */
    gputi(buf+14, 32, 2);						 /* BitCount */
    gputi(buf+16, GMAKEID('k','D','A','M'), 4);  /* Compression (MADk) is FOURCC */
    gputi(buf+20, maxDataSize, 4);				 /* ImageSize */ 
    gputi(buf+24, 0, 4);						 /* XpelsMeter */
    gputi(buf+28, 0, 4);						 /* YpelsMeter */
    gputi(buf+32, 0, 4);						 /* NumberOfColours */
    gputi(buf+36, 0, 4);						 /* ImportantColours */
    ok = gwrite(g,buf,40);

    /* Output movie chunk header */
    gputm(buf,GMAKEID('L','I','S','T'),4);
    gputi(buf+4,0,4); /* Correct size to be given later (on close) */
    gputm(buf+8,GMAKEID('m','o','v','i'),4);
    ok = gwrite(g,buf,12);

    return(ok);

}

static int outputMADAviIndex(GINSTANCE* gx, float fps)
{

	int riffSize;
	int moviSize;
	int sizeIndex;
	int suggestedBufferSize;
	int maxBps;
	char buf[4];
	GSTREAM* g = gx->gstream;

	sizeIndex = gx->frames * sizeof(MADAviIndex);
	gputm(buf,GMAKEID('i','d','x','1'),4);
	gwrite(g, buf, 4);
	
	gputi(buf, sizeIndex , 4);
	gwrite(g, buf, 4);

	gwrite(g, gpMADAviIndex, sizeIndex);
					
	/* Write in size of RIFF chunk*/				
	riffSize = 0xe0 + gDataSize + sizeIndex;	// header size + movie data size + index size 
	gseek(g,4);
	gwrite(g, &riffSize, 4);	
	
	/* Write in size of LIST:movi chunk*/
	moviSize = gDataSize+4;
	gseek(g,0xd8); 
	gwrite(g,&moviSize,4);

	/* Write in updated Avi header info */
	maxBps = (int) ((float)gMaxChunkSize * fps);
	suggestedBufferSize = gMaxChunkSize + 1024; // +1024 for possible overflow
	
	//MaxBps 
	gseek(g,0x24);
	gwrite(g,&maxBps,4);
	
	// Suggested Buffer size
	gseek(g,0x3c);
	gwrite(g,&suggestedBufferSize,4);
	gseek(g,0x90);
	gwrite(g,&suggestedBufferSize,4);

	gfree(gpMADAviIndex);

	return 1;


}

/****************************************************************/
/*  Export Functions                                            */
/****************************************************************/

int GCALL MAD_wopen(GINSTANCE **gxptr, GSTREAM *g, const char *pathname, int numframes)
{
    GINSTANCE *gx;
    int ok=0;

	madcallback = mavwritechunk;
	gWriteStream = g;

    gx = (GINSTANCE *) galloc(sizeof(GINSTANCE));
    if (gx)
    {
        memset(gx, 0, sizeof(GINSTANCE));
        gx->gstream = g;
        gx->framenum = 0;
        gx->frames = numframes;
        gx->gref = galloc(sizeof(MADINFO));
        if (gx->gref)
        {
            memset(gx->gref, 0, sizeof(MADINFO));
            *gxptr = gx;
            ok = 1;
        }
    }
    return(ok);
}

int GCALL MAD_write(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes)
{
    int ok=0;
    char *buf=0;
    int bufrowbytes=0;
    MADINFO *mad;
    float fps;
    int period;

	
    if (ginfo->bpp>8)
    {
        /* Check for reasonable fps field */
        gSubtype = ginfo->subtype;
		fps = 29.97f;
        if ( ginfo->fps>=0.01f && ginfo->fps<=1000.0f)
        {
            fps = ginfo->fps;
		}

        period = (int)((2000 * 65536 + fps)/(2*fps));

        mad = (MADINFO*)gx->gref;
        if (mad)
        {

            ok = 1;
			
            if (gx->framenum == 0)
            {
                ENCODEPARAMS p;
                int avgsize;
                int maxsize;
                int groupframes;
                int flags;

				mad->fps = fps;
                /* Compute average and maximum frame sizes */

                avgsize = ginfo->framesize?ginfo->framesize:(int)sqrtf((float)(ginfo->width*ginfo->height)*42.0f); // about 22k for 640x480

				int minsize = (int)(ginfo->width*ginfo->height/(16*16)*6);// * 1.2)+256;
				if (avgsize<minsize)
					avgsize=minsize;

                /*
                 * Use the subtype for the maxframesize (in KB), group size, and extraflag.
                 */
                if ( ginfo->subtype & 0x0000ff )
                    maxsize     = (ginfo->subtype & 0xff) * 1024;
                else
                    maxsize     = 256+(int) ((float)avgsize*(1.2f+fps/100.0f)); // 1.2 * avg for 30 fps

                if ( ginfo->subtype & 0x00ff00 )
                {
                    groupframes = (ginfo->subtype>>8) & 0xff;
                    flags       = (ginfo->subtype>>16) & 0xff;
                }
                else  // gop 10,1 default
                {
                    groupframes = (int)(fps/3.0 + 0.5);
                    if (groupframes<1) groupframes = 1;
                    if (groupframes>30) groupframes = 30;
					flags = 0;  // was flags       = (fps>15) ? 1 : 0;
                }
				if (groupframes > gx->frames)
					groupframes = gx->frames;

                p.maxframesize     = maxsize;
                p.maxgroupsize     = avgsize*groupframes;
                p.groupframes      = groupframes;
                p.extraflag        = flags & MADEXTRAFLAG;
                p.motionthreshold  = (100 - ginfo->quality) * 10;

                mad->width = (ginfo->width+15)&~15;
                mad->height = (ginfo->height+15)&~15;


				/* If we're writing out a MAV (MAD AVI) write the headers here*/
				if(ginfo->subtype & SUBTYPE_MAV)
				{
					writeAVIHeaders(ginfo, mad, gx);
					mad->MADAviIndex = (MADAviIndex *) galloc(gx->frames * sizeof(MADAviIndex));
					gpMADAviIndex = mad->MADAviIndex;
				}
				
                if (madinitencode(mad->width, mad->height, &p, gx->gstream) != 0)
                    ok = 0;
            }
            if (ok)
            {
                if ( (mad->width != ginfo->width) || (mad->height != ginfo->height) )
                {
                    /* Enlarge size to multiple of 16 */

                    int i;

                    bufrowbytes = mad->width*sizeof(ARGB);
                    buf = (char *) galloc(mad->height*bufrowbytes);
                    if (buf)
                    {
                        memset(buf, 0, mad->height*bufrowbytes);
                        for (i=0;i<ginfo->height;i++)
                        {
                            memcpy(buf+i*bufrowbytes, source+i*rowbytes, gmin(rowbytes, bufrowbytes));
                        }

                        if (madencode((ARGB*)buf, bufrowbytes, period) != 0)
                            ok = 0;

                        gfree(buf);
                    }
                }
                else            /* Size does not need to be adjusted */
                {
                    if (madencode((ARGB*)source, rowbytes, period) != 0)
                        ok = 0;
                }
            }

            gx->framenum++;
        }
    }

    return(ok);
}

int GCALL MAD_wclose(GINSTANCE *gx)
{
    int ok=0;

    if (gx)
    {       
        ok = (madendencode() == 0);

		/* Output the AVI index for MAV files */
		if(gSubtype & SUBTYPE_MAV)
		{
			outputMADAviIndex(gx, ((MADINFO *) gx->gref)->fps);
		}

		if (gx->gref) 
			gfree(gx->gref);
        
		gfree(gx);

    }
    return(ok);
}





