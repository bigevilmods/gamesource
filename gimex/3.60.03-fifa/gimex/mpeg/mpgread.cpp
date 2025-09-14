// (c) 1996-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*            MPEG - Motion Picture Experts Group Gimex             */
/*                                                                  */
/*                    by Patrick Ratto, EAC                         */
/*                  based on MSSG Mpeg Library                      */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History:                                       */
/* ------- ----   -- --------                                       */
/*    1.00 970101 FB ezmpeg version                                 */
/*    1.01 980101 NG mssg version rewrite                           */
/*    1.02 990818 SD rewrite based on newer mssg                    */
/*    1.03 000404 FB about->movie field                             */
/*    1.04 000519 PR rewrite for .M2V and .MPC file support         */
/*    1.05 000605 PR YUV exporter array bounds overshoot fix        */
/*    1.06 000606 PR added ability to read system streams           */
/*    1.07 000606 PR set decoder quiet flag + minor fixes           */
/*    1.08 020206 FB renamed to MPC to avoid -MPG confusion         */
/*    1.09 021203 FB added MPA (MPeg AVI)                           */
/*	  1.10 021205 AE MPA import/export complete						*/
/*	  1.11 060411 FB MPV add MVhd header chunk to MPC				*/
/*	  1.12 060516 FB bump max frame to 1 MB from 200 KB     		*/
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 32 bit import/export                                             */
/* decompresses                                                     */
/* non-reentrant                                                    */
/* Files: mpeg/*.* (several)                                        */
/*                                                                  */
/* See Also                                                         */
/* http://www.mpeg.org/MPEG/MSSG/                                   */
/*                                                                  */
/* For reading, the following file formats are supported:           */
/*                                                                  */
/* MPG - MPEG system streams (MPEG-1 and MPEG-2 supported)          */
/* M1V - MPEG-1 video stream                                        */
/* M2V - MPEG-2 video stream                                        */
/* MPC - chunkified MPEG-2 video stream, for use with the REAL      */
/*       libraries and our PLAYMPC players for PS2 and PC           */
/* PSS - Sony's video format for the PS2                            */
/* VOB - the format used on DVDs (MPEG-2 video and AC3 audio)       */
/* MPA - AVI chunkified MPEG-2 video stream, for use with RCMP      */
/* MPV - MPC with MVhd header                                       */
/*                                                                  */
/* For writing, both M2V and MPC files are supported. When using    */
/* GX, the following command line options are used to control the   */
/* encoding process:                                                */
/*                                                                  */
/* -framesize<n>                                                    */
/* Specifies the average frame size in bytes for the compressed     */
/* movie. The default is 30*sqrt(width*height), which translates    */
/* to 2 Mbps for 320x240 30 fps videos and 4 Mbps for 640x480 30    */
/* fps videos. Note that the bitrate can be determined using        */
/* bitrate = 8*framesize*fps                                        */
/*                                                                  */
/* -fps<n>                                                          */
/* Specifies the frame rate in frames per second. The following     */
/* values are supported: 11.988, 12, 12.5, 14.985, 15, 23.976, 24,  */
/* 25, 29.97, 30, 50, 59.94 and 60. If the specified rate is        */
/* different, the nearest match from the above list will be used.   */
/* The default value is 30 fps.                                     */
/*                                                                  */
/* -gop<n>,<m>                                                      */
/* Specifies the Group of Pictures (GOP) pattern to use for         */
/* encoding. n is the distance between I frames and determines      */
/* the GOP size. m is the distance between successive I or P        */
/* frames (so there are m-1 B frames between I or P frames).        */
/* The default value is 15,3. Note that n must be divisible         */
/* by m and that m must be one of 1, 2 or 3. For the PC, it         */
/* is probably a good idea to set m=2 so that the frame rate        */
/* can drop to a steady 15 fps on low end PCs.                      */
/*                                                                  */
/* This is VCD header, which contains an mpeg2 system stream        */
/*                                                                  */
/* E:\tools\gx>xd *.dat -h                                          */
/* XDump v1.65 (Cobra) 001208 by FrANK Barchard, EAC                */
/* 00000: 52494646 e4f07320 43445841 666d7420  RIFFdps CDXAfmt      */
/* 00010: 10000000 00000000 15555841 01000000  .........UXA....     */
/* 00020: 00000000 64617461 c0f07320 00ffffff  ....data@ps ....     */
/* 00030: ffffffff ffffff00 00180202 01006000  ..............`.     */
/* 00040: 01006000 00000000 00000000 00000000  ..`.............     */
/* 00050: 00000000 00000000 00000000 00000000  ................     */
/* 00060: 00000000 00000000 00000000 00000000  ................     */
/*                                                                  */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#include <stdlib.h>
#include <string.h>

#include "gimex.h"
#include "mpggimex.h"
#include "mpeg2dec.h"

#ifdef __cplusplus
extern "C" {
#endif

void MPD_renderinit(void);
void MPD_render(unsigned char *src[], char *dest, int rowbytes);
void MPD_renderrestore(void);

#ifdef __cplusplus
}
#endif


#define SIGNATURE       GMAKEID('M','P','E','G')

typedef struct
{
    int             offset;
    unsigned short  vidbytes;
    unsigned char   type;
    unsigned char   reserved;
}
FRAMEINFO;

#define MAXFRAMESIZE 1048576
typedef struct
{
    int             subtype;
    int             frames;
    FRAMEINFO       *frameinfo[1024];
    GSTREAM         *file;
    unsigned char   data[MAXFRAMESIZE];
    int             datasize;
    int             readpos;
    int             vidbytes;
    int             lastdecode;
}
DECODEINFO;


static int MPGR_initialized = 0;


static void *mpdalloc(unsigned int size)
{
    return(galloc(size));
}

static void mpdfree(void *ptr)
{
    gfree(ptr);
}

static int getbyte(DECODEINFO *info)
{
    int i;

    i = info->readpos & 4095;
    if (i == 0)
    {
        gseek(info->file, (GPOS)info->readpos);
        info->datasize = gread(info->file, info->data, 4096);
    }

    if (i == info->datasize) return(-1);

    ++info->readpos;
    return(info->data[i]);
}

static int findstartcode(DECODEINFO *info)
{
    int zeros;
    int ch;

    zeros = 0;
    while ((ch = getbyte(info)) >= 0)
    {
        if ((zeros>1) && (ch==1)) return(1);

        if (ch==0) ++zeros;
        else zeros = 0;
    }

    return(0);
}

static int getvidbyte(DECODEINFO *info)
{
    int size;
    int hsize;
    int pos;
    int ch;
    int i;

    if (info->vidbytes == 0)
    {
        while (findstartcode(info))
        {
            ch = getbyte(info);
            if (ch == 0xbd)
            {
                size = (getbyte(info) << 8) + getbyte(info);
                for (i=0; i<size; ++i) getbyte(info);
            }
            else if (ch == 0xe0)
            {
                size = (getbyte(info) << 8) + getbyte(info);
                pos = info->readpos;

                ch = getbyte(info);
                while (ch == 0xff) ch = getbyte(info);

                if ((ch & 0xc0) == 0x80) // MPEG-2
                {
                    getbyte(info);
                    hsize = getbyte(info);
                    for (i=0; i<hsize; ++i) getbyte(info);
                }
                else // MPEG-1
                {
                    if ((ch & 0xc0) == 0x40)
                    {
                        getbyte(info);
                        ch = getbyte(info);
                    }

                    if ((ch & 0xf0) == 0x20)
                    {
                        for (i=0; i<4; ++i) getbyte(info);
                    }
                    else if ((ch & 0xf0) == 0x30)
                    {
                        for (i=0; i<9; ++i) getbyte(info);
                    }
                }

                info->vidbytes = size - (info->readpos - pos);
                break;
            }
        }
    }

    --info->vidbytes;
    return(getbyte(info));
}

static int vidfindpicture(DECODEINFO *info)
{
    int state;
    int ch;

    state = 0;
    while ((ch = getbyte(info)) >= 0)
    {
        if (state < 2)
        {
            if (ch == 0) ++state;
            else state = 0;
        }
        else if (state == 2)
        {
            if (ch == 1) ++state;
            else if (ch != 0) state = 0;
        }
        else
        {
            if (ch == 0) return(1);
            else state = 0;
        }
    }

    return(0);
}

static int sysfindpicture(DECODEINFO *info)
{
    int state;
    int ch;

    state = 0;
    while ((ch = getvidbyte(info)) >= 0)
    {
        if (state < 2)
        {
            if (ch == 0) ++state;
            else state = 0;
        }
        else if (state == 2)
        {
            if (ch == 1) ++state;
            else if (ch != 0) state = 0;
        }
        else
        {
            if (ch == 0) return(1);
            else state = 0;
        }
    }

    return(0);
}

static int sysfindsequence(DECODEINFO *info)
{
    int state;
    int ch;

    state = 0;
    while ((ch = getvidbyte(info)) >= 0)
    {
        if (state < 2)
        {
            if (ch == 0) ++state;
            else state = 0;
        }
        else if (state == 2)
        {
            if (ch == 1) ++state;
            else if (ch != 0) state = 0;
        }
        else
        {
            if (ch == 0xb3) return(1);
            else state = 0;
        }
    }

    return(0);
}

static void registerframe(DECODEINFO *info, int offset, int vidbytes, int type)
{
    FRAMEINFO   *frameinfo;
    int         frame;
    int         indexlo,indexhi;

    frame = info->frames;
    indexlo = frame % 1024;
    indexhi = frame / 1024;

    if (indexlo > 0)
        frameinfo = info->frameinfo[indexhi];
    else
    {
        if (indexhi >= 1024) frameinfo = NULL;
        else frameinfo = (FRAMEINFO*) galloc(1024*sizeof(FRAMEINFO));
        info->frameinfo[indexhi] = frameinfo;
    }

    if (frameinfo)
    {
        frameinfo[indexlo].offset = offset;
        frameinfo[indexlo].vidbytes = (unsigned short) vidbytes;
        frameinfo[indexlo].type   = (unsigned char) type;
        if (type > 0) info->frames = frame+1;
    }
}

static void countframes(DECODEINFO *info, GSTREAM *g)
{
    unsigned char   header[32];
    int             offset;
    int             count;
    int             readpos;
    int             vidbytes;
    int             type;
	unsigned int subtype_mpvc = SUBTYPE_MPC;

    info->frames = 0;
    info->file = g;
    info->readpos = 0;
    info->vidbytes = 0;

    offset = 0;
    gseek(g, (int)offset);
    count = gread(g, header, 14);

    if (ggetm(header,4) == 0x000001b3)
    {
        info->subtype = SUBTYPE_M2V;
        while (vidfindpicture(info))
        {
            getbyte(info);
            type = (getbyte(info) >> 3) & 0x03;
            if (info->frames == 0) registerframe(info, 0, 0, type);
            else registerframe(info, info->readpos-6, 0, type);
        }
        registerframe(info, info->readpos, 0, 0);
    }
	else if(ggetm(header,4) == GMAKEID('R','I','F','F') &&	// MPC data encoded in AVI format	
			ggetm(header+8,4) == GMAKEID('A','V','I',' ') )
	{
		int end = 0;
		info->subtype = SUBTYPE_MPA;
		
		/* Get to first MPCh chunk */
		gseek(g,0);
		gread(g,header,4);
		offset += 4;
		while(ggetm(header,4) != GMAKEID('m','o','v','i'))
		{
			offset += 1;
			gseek(g, offset);
			gread(g,header,4);
		}
		gseek(g,gtell(g)+8);
		offset += 12;	// add 12 to skip 'movi' + '00dc' + 'size'
        
		
		count = gread(g, header, 14);
	
		/* while more to read and we haven't reached the index chunk */
		while (count == 14 && !end)
	    {
			if (ggetm(header,4) == GMAKEID('M','P','C','h'))
            {
                if (header[11] == 0x00) type = (header[13] >> 3) & 0x03;
                else type = I_TYPE;
                registerframe(info, offset+8, 0, type);
            }
            offset += ggeti(header+4,4) + 8; // + 8 is for the '00dcsize'
            gseek(g, (GPOS)offset-8);
			
			gread(g,header,8);
			if( ggetm(header,4) == GMAKEID('i','d','x','1') )
				end = 1;

			count = gread(g, header, 14);
        }
        registerframe(info, offset, 0, 0);

	}
	
	else if (ggetm(header,4) == 0x000001ba
          || ggetm(header,4) == 0x52494646)
    {
        if (ggetm(header,4) == 0x52494646)
        {
            info->subtype = SUBTYPE_VCD;
            info->readpos = 0x10184;
        }
        else
            info->subtype = SUBTYPE_MPG;
        sysfindsequence(info);
        readpos = info->readpos - 4;
        vidbytes = info->vidbytes + 4;
        if (sysfindpicture(info))
        {
            registerframe(info, readpos, vidbytes, I_TYPE);
            while (sysfindpicture(info))
            {
                readpos = info->readpos - 4;
                vidbytes = info->vidbytes + 4;
                getvidbyte(info);
                type = (getvidbyte(info) >> 3) & 0x03;
                registerframe(info, readpos, vidbytes, type);
            }
        }
        registerframe(info, info->readpos, 0, 0);
    }
    else
    {
        while (count == 14)
        {
            if (ggetm(header,4) == GMAKEID('M','V','h','d'))
				subtype_mpvc = SUBTYPE_MPV;
			else if (ggetm(header,4) == GMAKEID('M','P','C','h'))
            {
                if (header[11] == 0x00) type = (header[13] >> 3) & 0x03;
                else type = I_TYPE;
                registerframe(info, offset+8, 0, type);
            }
            offset += ggeti(header+4,4);
            gseek(g, (GPOS)offset);
            count = gread(g, header, 14);
        }
        info->subtype = subtype_mpvc;
        registerframe(info, offset, 0, 0);
    }
}

static FRAMEINFO *queryframe(DECODEINFO *info, int num)
{
    if (num > info->frames) num = info->frames;
    return(info->frameinfo[num/1024] + (num%1024));
}

static void releaseframeinfo(DECODEINFO *info)
{
    int n;
    int i;

    n = (info->frames+1024) / 1024;
    for (i=0; i<n; ++i) gfree(info->frameinfo[i]);
}

static int framegetbyte(DECODEINFO *info)
{
    int readpos;

    readpos = info->readpos;
    if (readpos >= info->datasize) return(-1);

    info->readpos = readpos + 1;
    return(info->data[readpos]);
}

static int framefindstartcode(DECODEINFO *info)
{
    int zeros;
    int ch;

    zeros = 0;
    while ((ch = framegetbyte(info)) >= 0)
    {
        if ((zeros>1) && (ch==1)) return(1);

        if (ch==0) ++zeros;
        else zeros = 0;
    }

    return(0);
}

static int framegetvidbyte(DECODEINFO *info)
{
    int size;
    int hsize;
    int pos;
    int ch;
    int i;

    if (info->vidbytes == 0)
    {
        while (framefindstartcode(info))
        {
            ch = framegetbyte(info);
            if (ch == 0xbd)
            {
                size = (framegetbyte(info) << 8) + framegetbyte(info);
                for (i=0; i<size; ++i) framegetbyte(info);
            }
            else if (ch == 0xe0)
            {
                size = (framegetbyte(info) << 8) + framegetbyte(info);
                pos = info->readpos;

                ch = framegetbyte(info);
                while (ch == 0xff) ch = framegetbyte(info);

                if ((ch & 0xc0) == 0x80) // MPEG-2
                {
                    framegetbyte(info);
                    hsize = framegetbyte(info);
                    for (i=0; i<hsize; ++i) framegetbyte(info);
                }
                else // MPEG-1
                {
                    if ((ch & 0xc0) == 0x40)
                    {
                        framegetbyte(info);
                        ch = framegetbyte(info);
                    }

                    if ((ch & 0xf0) == 0x20)
                    {
                        for (i=0; i<4; ++i) framegetbyte(info);
                    }
                    else if ((ch & 0xf0) == 0x30)
                    {
                        for (i=0; i<9; ++i) framegetbyte(info);
                    }
                }

                info->vidbytes = size - (info->readpos - pos);
                break;
            }
        }
    }

    --info->vidbytes;
    return(framegetbyte(info));
}

static int MPGR_readframe(DECODEINFO *info, int num)
{
    FRAMEINFO   *frameinfo;
    int         offset;
    int         vidbytes;
    int         size;
    int         writepos;
    int         ch;

    if (num >= info->frames) return(0);

    frameinfo = queryframe(info, num);
    offset = frameinfo->offset;
    vidbytes = frameinfo->vidbytes;
    frameinfo = queryframe(info, num+1);
    size = frameinfo->offset - offset;
    if (size > 500000) return(0);

    gseek(info->file, (GPOS)offset);
    info->datasize = gread(info->file, info->data, size);
    if (info->datasize != size) return(0);

    if (info->subtype == SUBTYPE_MPG)
    {
        info->readpos = vidbytes;
        info->vidbytes = 0;
        writepos = vidbytes;
        while ((ch = framegetvidbyte(info)) >= 0)
        {
            info->data[writepos] = ch;
            ++writepos;
        }

        /* The first 3 bytes may be incorrect if the start code was */
        /* split by the system stream. This corrects the problem. */
        info->data[0] = 0;
        info->data[1] = 0;
        info->data[2] = 1;
    }

    return(1);
}


/****************************************************************/
/*  Information Functions                                       */
/****************************************************************/

GABOUT *GCALL MPG_about(void)
{
    GABOUT *info;

    info = (GABOUT*) galloc(sizeof(GABOUT));
    if (info)
    {
        memset(info, 0, sizeof(GABOUT));

        info->signature        = SIGNATURE;
        info->size             = sizeof(GABOUT);
        info->version          = GIMEX_VERSION;
        info->canimport        = 1;    /* supports importing */
        info->canexport        = 1;    /* supports exporting */
        info->importpacked     = 1;    /* supports importing packed */
        info->exportpacked     = 1;    /* supports exporting packed */
        info->import8          = 0;    /* supports importing 8 bit indexed */
        info->export8          = 0;    /* supports exporting 8 bit indexed */
        info->import32         = 1;    /* supports importing 32 bit direct rgb */
        info->export32         = 1;    /* supports exporting 32 bit direct rgb */
        info->multiframe       = 1;    /* supports multiple frames */
        info->multifile        = 0;    /* format requires additional files or resource fork */
        info->multisize        = 0;    /* supports each frame different size */
        info->framebuffer      = 1;    /* module requires memory to buffer entire frame */
        info->external         = 0;    /* module uses external resources */
        info->usesfile         = 1;    /* module is file based */
        info->dotsubtype       = 1;    /* subtype based on extension */
        info->movie            = 1;    /* movie format */
        info->mactype[0]       = GMAKEID('M','P','E','G');  /* mac file system types used */

        strcpy(info->extensions[0], ".mpc");                /* extensions */
        strcpy(info->extensions[1], ".m2v");
        strcpy(info->extensions[2], ".mpa");
        strcpy(info->extensions[3], ".mpv");
        strcpy(info->authorstr,    "Frank Barchard");       /* author */
        strcpy(info->versionstr,   "1.12");                 /* version # */
        strcpy(info->shorttypestr, "MPC");                  /* type */
        strcpy(info->wordtypestr,  "MPC");                  /* wordtype */
        strcpy(info->longtypestr,  "MPEG Video" );          /* longtype */
    }
    return(info);
}

/* VCD header */
/* 00000: 52494646 e4f07320 43445841 666d7420  RIFFdps CDXAfmt      */
/* 00010: 10000000 00000000 15555841 01000000  .........UXA....     */

// anim (icon) is RIFF, but is not an mpeg/vcd
// 00000: 52494646 a48c0000 41434f4e 616e6968  RIFF$...ACONanih

// 0x000001b3  M2V
// 0x000001ba  MPG
// 0x52494646  VCD
// 'S','C','H','l' or 'M','P','C','h'  MPC

int GCALL MPG_is(GSTREAM *g)
{
    unsigned char header[16];
    int code;
    int ok=0;

	GPOS p=0;
    gseek(g,p);
    if (gread(g, header, 16))
    {
        code = ggetm(header,4);
        while (code == GMAKEID('M','V','h','d') || code == GMAKEID('S','C','H','l') || code == GMAKEID('S','C','C','l'))
        {
			p += ggeti(header+4,4);
			if (gseek(g, p))
			{
			    if (gread(g, header, 16))
				{
					code = ggetm(header,4);
				}
				else break;
			}
			else break;
		}

		if ((code == 0x000001b3) || (code == 0x000001ba) || code == GMAKEID('M','P','C','h'))
            ok = 100;
        else if (code == 0x52494646)  // RIFF is generic, so we report 90 to allow another reader to override
		{              
			if( ggetm(header+8,4)==0x43445841)	// VCD
			{
				ok = 90;
			}
			else if( ggetm(header+8,4)==GMAKEID('A','V','I',' ') )	// dealing with AVI
            {
				// need to see if compression FOURCC is MPCh
				gseek(g,0xbc); 
				gread(g,header,4);
				if( ggetm(header,4) == GMAKEID('M','P','C','h') )
				{
					ok = 100;
				}
			}      
		}
		else if (ggetm(header,4) == GMAKEID('S','C','H','l'))
        {
            ok = gseek(g, (GPOS)ggeti(header+4,4));
            if (ok)
            {
                ok = gread(g, header, 8);
                if ((ok==8) && ggetm(header,4) == GMAKEID('M','P','C','h'))
                    ok = 100;
                else
                    ok = 0;
            }
        }
    }
    return(ok);
}


/****************************************************************/
/*  Import Functions                                            */
/****************************************************************/

int GCALL MPG_open(GINSTANCE **ginstance, GSTREAM *g, const char *pathname,bool framecountflag)
{
    GINSTANCE   *gx;
    DECODEINFO  *info;

    *ginstance = NULL;
    if (MPGR_initialized) return(0); // decoder is not reentrant

    info = (DECODEINFO*) galloc(sizeof(DECODEINFO));
    if (info == NULL) return(0);

    MPD_initdecoder(mpdalloc, mpdfree);
    MPDquiet_flag = 1;
    countframes(info, g);
    if (MPGR_readframe(info, 0))
    {
        MPD_decodeframe(0, info->data);
        MPD_renderinit();
        info->lastdecode = 0;

        gx = (GINSTANCE*) galloc(sizeof(GINSTANCE));
        if (gx)
        {
            /* fill in instance fields */
            memset(gx, 0, sizeof(GINSTANCE));
            gx->signature = SIGNATURE;
            gx->size      = sizeof(GINSTANCE);
            gx->frames    = info->frames;
            gx->framenum  = 0;
            gx->gstream   = g;
            gx->gref      = (char*) info;

            MPGR_initialized = 1;
            *ginstance = gx;
            return(gx->frames);
        }
    }

    /* handle errors (no memory or failed read) - notice return statment above */
    MPD_renderrestore();
    MPD_restoredecoder();
    releaseframeinfo(info);
    gfree(info);
    return(0);
}

GINFO *GCALL MPG_info(GINSTANCE *gx, int framenum)
{
    GINFO       *ginfo;
    DECODEINFO  *info;
    FRAMEINFO   *frameinfo;

    ginfo = (GINFO*) galloc(sizeof(GINFO));
    if (ginfo)
    {
        info = (DECODEINFO*) gx->gref;
        frameinfo = queryframe(info, framenum);

        memset(ginfo, 0, sizeof(GINFO));

        ginfo->signature        = SIGNATURE;
        ginfo->size             = sizeof(GINFO);
        ginfo->version          = GIMEX_VERSION;
        ginfo->framenum         = framenum;
        ginfo->width            = MPDhorizontal_size;
        ginfo->height           = MPDvertical_size;
        ginfo->bpp              = 32;
        ginfo->originalbpp      = 24;
        ginfo->subtype          = info->subtype | (frameinfo->type << 6);
        ginfo->packed           = 1;
        ginfo->alphabits        = 0;
        ginfo->redbits          = 8;
        ginfo->greenbits        = 8;
        ginfo->bluebits         = 8;
        ginfo->fps              = (float)MPDframe_rate;
    }

    return(ginfo);
}

int GCALL MPG_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes)
{
    DECODEINFO  *info;
    FRAMEINFO   *frameinfo;
    int         targetframe;
    int         startframe;
    int         i;

    info = (DECODEINFO*) gx->gref;
    if (ginfo->framenum >= info->frames) return(0);

    targetframe = ginfo->framenum + 1;
    if (targetframe == info->frames) --targetframe;

    if (targetframe != info->lastdecode)
    {
        startframe = targetframe;

        frameinfo = queryframe(info, startframe);
        while (frameinfo->type == B_TYPE)
        {
            --startframe;
            frameinfo = queryframe(info, startframe);
        }

        if (startframe > 0)
        {
            do
            {
                --startframe;
                frameinfo = queryframe(info, startframe);
            }
            while (frameinfo->type != I_TYPE);
        }

        if ((info->lastdecode >= startframe) && (info->lastdecode < targetframe))
            startframe = info->lastdecode + 1;

        for (i=startframe; i<targetframe; ++i)
        {
            frameinfo = queryframe(info, i);
            if (frameinfo->type != B_TYPE)
            {
                MPGR_readframe(info, i);
                MPD_decodeframe(i, info->data);
            }
        }

        MPGR_readframe(info, targetframe);
        MPD_decodeframe(targetframe, info->data);
        info->lastdecode = targetframe;
    }

    if (ginfo->framenum == info->frames-1) MPD_render(MPDbackward_reference_frame, dest, rowbytes);
    else if (MPDpicture_coding_type==B_TYPE) MPD_render(MPDauxframe, dest, rowbytes);
    else MPD_render(MPDforward_reference_frame, dest, rowbytes);

    return(1);
}

int GCALL MPG_close(GINSTANCE *gx)
{
    DECODEINFO  *info;

    if (gx)
    {
        info = (DECODEINFO*) gx->gref;

        gfree(gx);
        MPD_renderrestore();
        MPD_restoredecoder();
        releaseframeinfo(info);
        gfree(info);
        MPGR_initialized = 0;
    }

    return(1);
}
