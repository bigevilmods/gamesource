// (c) 1994-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*                QT3 - Apple Quicktime Gimex                       */
/*                                                                  */
/*            by Kaz Okuda and Frank Barchard, EAC                  */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History:                                       */
/* ------- ----   -- --------                                       */
/*    1.00 990415 KO Uses QuickTime 3 library                       */
/*    1.01 000320 FB expand filename into full pathname             */
/*    1.02 000404 FB about->movie                                   */
/*    1.03 021002 FB is relaxed 'wide' check to ignore size         */
/*    1.04 030326 FB set alpha to 255                               */
/*    1.05 030327 FB _is allow mp4, flx, png, mng, gif              */
/*    1.06 030508 FB don't copy alpha if its not there              */
/*    1.07 051011 FB import mpg, qt7 compat, InitQTML once          */
/*    1.08 051116 FB _is allow ftyp variation of quicktime          */
/*    1.09 060418 FB _is accept 'skip' chunk from Kodak mov files   */
/*    1.10 060821 FB shutdown, fps, count-1                         */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* Uses Quicktime SDK to import movies                              */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */
#include <string.h>
#include <stdio.h>
#include "gimex.h"
#include "qt3gimex.h"

#include "windows.h"
#include "qtml.h"
#include "movies.h"
#include "textutils.h"

typedef struct
{
    Movie movie;
    int valid;
    GWorldPtr gw;
    char *buffer;
    int rowbytes;
	float fps;

} QT3INFO;

#define	RECT_WIDTH(r)	(r.right-r.left)
#define	RECT_HEIGHT(r)	(r.bottom-r.top)

//#define QUICKTIMEDEBUG 1
//#define QTSHUTDOWN_ENABLE 1
// 
/****************************************************************/
/*  Internal Functions                                          */
/****************************************************************/

//enum {
//	kInitializeQTMLNoSoundFlag	= (1L << 0),					/* flag for requesting no sound when calling InitializeQTML*/
//	kInitializeQTMLUseGDIFlag	= (1L << 1)						/* flag for requesting GDI when calling InitializeQTML*/
//};

static int initCount=0;

static int QT3_InitializeQTML(int flags)
{
	int errorCode=0;

	if (!initCount)
	{
		errorCode = InitializeQTML(flags);

#ifdef QUICKTIMEDEBUG
		printf("InitializeQTML return %d\n",errorCode);
#endif


#ifdef QUICKTIMEDEBUG
//C:\env\egami\gx\dev>mathx -2147450879
//0xffffffff80008001
		int v = GetComponentVersion(0);

		printf("GetComponentVersion return %d\n",v);
#endif



	}
	if (!errorCode)
		++initCount;

	return errorCode;
}

static void QT3_TerminateQTML(void)
{
#ifdef QTSHUTDOWN_ENABLE
	if (initCount)
	{
		--initCount;
		if (!initCount)
			TerminateQTML();
	}
#endif
}



static void expandtofullpath(char *fullname,const char *pathname)
{
    char name[512];
    char *newname;

    GetFullPathName(pathname,512,name,&newname);
    strcpy(fullname,name);
#ifdef QUICKTIMEDEBUG
	printf("name %s <- %s\n",fullname,pathname);
#endif
    return;
}

static int QT3countframes(QT3INFO *qtinfo)
{
    TimeValue lasttime = 0;
    TimeValue currenttime = 0;
    TimeValue nexttime = 0;
    TimeValue frameduration;
//    TimeValue movieduration;
    OSType mediatype;
    int framecount = 0;
	int lastduration = 0;

//    movieduration = GetMovieDuration(qtinfo->movie);
    mediatype = VisualMediaCharacteristic;

    do
    {
        GetMovieNextInterestingTime(qtinfo->movie, nextTimeStep, 1, &mediatype, currenttime, 1, &nexttime, &frameduration);
        lasttime = currenttime;
        currenttime = nexttime;
		if (currenttime > 0)  // last frame has 'nexttime' of -1 and duration of zero
		{
			lastduration = frameduration;
		}
        framecount++;
#ifdef QUICKTIMEDEBUG
		printf("Current time is %d, framecount = %d, duration = %d\n", currenttime, framecount, frameduration);
#endif

    } while(currenttime > 0);

	if (lastduration)
		qtinfo->fps = (float)GetMovieTimeScale(qtinfo->movie)/(float)lastduration;

#ifdef QUICKTIMEDEBUG
	printf("Framerate %.3f\n", qtinfo->fps);
#endif
    return (framecount);
}

static TimeValue QT3gettimefromframe(QT3INFO *qtinfo, int framenum)
{
    TimeValue lasttime = 0;
    TimeValue currenttime = 0;
    TimeValue nexttime;
    TimeValue frameduration;
    OSType mediatype;
    int framecount = -1;

    mediatype = VisualMediaCharacteristic;

    do
    {
        GetMovieNextInterestingTime(qtinfo->movie, nextTimeStep, 1, &mediatype, currenttime, 1, &nexttime, &frameduration);
        lasttime = currenttime;
        currenttime = nexttime;
        framecount++;

    } while((currenttime > 0) && (framecount<framenum));


    return (lasttime);
    
}    

#if 0
static float  QT3getfps(QT3INFO *qtinfo)
{
	int fps = GetMovieTimeScale(qtinfo->movie);
#ifdef QUICKTIMEDEBUG
	printf("fps is %d\n", fps);
#endif
	return (float) fps;
}
#endif


/****************************************************************/
/*  Information Functions                                       */
/****************************************************************/

GABOUT * GCALL QT3_about(void)
{
    GABOUT *info;

    info = (GABOUT *) galloc(sizeof(GABOUT));
    if (info)
    {
        memset(info, 0, sizeof(GABOUT));

        info->signature       = GMAKEID('M','O','O','V');
        info->size            = sizeof(GABOUT);
        info->version         = GIMEX_VERSION;
        info->canimport       = 1;    /* supports importing */
        info->canexport       = 0;    /* supports exporting */
        info->importpacked    = 1;    /* supports importing packed */
        info->exportpacked    = 0;    /* supports exporting packed */
        info->import8         = 1;    /* supports importing 8 bit indexed */
        info->export8         = 0;    /* supports exporting 8 bit indexed */
        info->import32        = 1;    /* supports importing 32 bit direct rgb */
        info->export32        = 0;    /* supports exporting 32 bit direct rgb */
        info->multiframe      = 1;    /* supports multiple frames */
        info->multifile       = 0;    /* format requires additional files or resource fork */
        info->multisize       = 0;    /* supports each frame different size */
        info->framebuffer     = 0;    /* module requires memory to buffer entire frame */
        info->external        = 0;    /* module uses external resources */
        info->usesfile        = 1;    /* module is file based */
        info->maxframename    = 0;    /* max frame name character */
        info->defaultquality  = 100;  /* default pack quality */
        info->movie           = 1;    /* movie format */
        info->mactype[0]      = GMAKEID('M','o','o','V');   /* mac file system types used */
        strcpy(info->extensions[0], ".mov");                /* extensions */
        strcpy(info->extensions[1], ".qt");                 /* extensions */
        strcpy(info->extensions[2], ".qt3");                /* extensions */
        strcpy(info->authorstr,     "Kaz Okuda"); /* authors */
        strcpy(info->versionstr,    "1.10");                /* version # */
        strcpy(info->shorttypestr,  "QT3");                 /* type */
        strcpy(info->wordtypestr,   "QuickTime");           /* wordtype */
        strcpy(info->longtypestr,   "QuickTime");           /* longtype */
    }
    return(info);
}

/* check for reasonable header: */
/* has 8 byte chunk (len/id): */
/*  00000: 00821408 6d646174 00000000 00000000  ....mdat........    */
/*  00000: 00000301 6d6f6f76 0000006c 6d766864  ....moov...lmvhd    */
/*  00000: 000024a5 66726565 0000006c 6d766864  ..$%free...lmvhd    */
/*  00000: 00000008 77696465 01356008 6d646174  ....wide.5`.mdat    */

/*  00000: 00000020 66747970 71742020 20050300  ... ftypqt   ...    clip03_png.mov from Boris Amchislavsky */
/*  00010: 71742020 00000000 00000000 00000000  qt  ............    */
/*  00020: 00000008 77696465 35c83037 6d646174  ....wide5H07mdat    */

// food.mov
// 00000: 00000118 736b6970 20010000 6cca0000  ....skip ...lJ..
// 00110: 00000000 00000000 001b4a33 6d646174  ..........J3mdat

// xavier.wmv
// 00000: 3026b275 8e66cf11 a6d900aa 0062ce6c  0&2u.fO.&Y.*.bNl

// campea.m2v
// 00000: 000001b3 2801e014 15f92381 10111112  ...3(.`..y#.....
// dolphin.mpg
// 00000: 000001ba 21000100 078027d9 000001bb  ...:!.....'Y...;


#if 0

int GCALL QT3_testopen(const char *pathname)
{
    QT3INFO *qtinfo = NULL;
    int ok = 0;

    qtinfo = (QT3INFO*)galloc(sizeof(QT3INFO));
    if (qtinfo)
    {
        memset(qtinfo, 0, sizeof(QT3INFO));

        if (!QT3_InitializeQTML(kInitializeQTMLNoSoundFlag | kInitializeQTMLUseGDIFlag))
        {
            if (!EnterMovies())
            {
                FSSpec fspec;
                char name[512];
                OSErr result;
                short mf;

                memset(&fspec, 0, sizeof(fspec));
                expandtofullpath(name,pathname);
                c2pstr(name);
                result = FSMakeFSSpec(0, 0, (const unsigned char *)name, &fspec);

                //result = NewMovieFromUserProc(&qtinfo->movie, 0, NULL, QT3getdata, gx, NULL, 0);
                result = OpenMovieFile (&fspec, &mf, fsRdPerm);

#ifdef QUICKTIMEDEBUG
				printf("OpenMovieFile return %d\n",result);
#endif

                if (!result)
                {
                    CloseMovieFile (mf);
					ok = 1;
				}
			}
		}
	}
	return ok;
}
#endif

//HKEY_LOCAL_MACHINE\SOFTWARE\Apple Computer, Inc.\QuickTime

int GCALL QT3_is(GSTREAM *g)
{
    unsigned char buf[12];
    int ok=0;

    gseek(g,(GPOS)0);

    if (gread(g,buf,8))
    {
        if ( ggetm(buf+4,4)==GMAKEID('w','i','d','e') )
        {
            /* There is an empty (8byte) 'wide' chunk to skip at the begining of the file */
            gseek(g, 8);
        }
        else
        {
            gseek(g, 0);
        }
    }

    if (gread(g,buf,8))
    {
        if (ggetm(buf+4,4)==GMAKEID('s','k','i','p'))
		{
			ok = 5;
			gseek(g,ggetm(buf+0,4));
			if (gread(g,buf,8))
				ok = 10;
		}

		if ((ggetm(buf+4,4)==GMAKEID('m','d','a','t'))
         || (ggetm(buf+4,4)==GMAKEID('m','o','o','v'))
         || (ggetm(buf+4,4)==GMAKEID('f','t','y','p'))
         || (ggetm(buf+4,4)==GMAKEID('f','r','e','e')))
//         || (ggetm(buf+0,4)==0x3026b275)) // wmv
        {
            ok = 50;

            if (!QT3_InitializeQTML(kInitializeQTMLNoSoundFlag | kInitializeQTMLUseGDIFlag))
            {
                ok = 100;
                QT3_TerminateQTML();
            }
        }
    }

	// ***********************************
	// Quicktime support for other formats
	// ***********************************


	if (!ok)
	{
	    gseek(g, 0);
 	    gread(g,buf,12);

		// MPeg4
		// DCA.mp4
		// 00000: 00000020 66747970 6d703432 00000000  ... ftypmp42....
		// 00010: 6d703432 69736f6d 00000000 00000000  mp42isom........	

		// Miscellaneous
        if (ggetm(buf,3)=='FWS'      // Flash
		 ||	ggetm(buf,4)==0x8a4d4e47 // MNG
		 || ggetm(buf,4)==0x89504e47 // PNG
		 || ggetm(buf,4)==0xd0cf11e0 // FPX
		 || ggetm(buf,4)==0xff4fff51 // JPeg2000 - requires a plugin
		 || ggetm(buf,4)=='GIF8'     // GIF87 or 89
		 || ggetm(buf,4)==0xffd8ffe0 // Jpeg
         || ggetm(buf,4)==0xffd8ffe1 // Jpeg
         || ggetm(buf,4)==0xffd8ffed // Jpeg
         || ggetm(buf,4)==0x4d4d002a // KDC
         || ggetm(buf,4)==0x49492a00 // Tiff 'II'
         || ggetm(buf,4)==0x4d4d002a // Tiff 'MM'
//       || ggetm(buf,4)==0x464f5234 // maya iff 
         || ggetm(buf,4)==0x000001b3 // M2V
         || ggetm(buf,4)==0x000001ba // Mpg
         || ggetm(buf,4)==0x52494646 // VCD (mpeg2)
		 || (ggetm(buf+0,4)==0x00000020 && ggetm(buf+4,4)==0x66747970 && ggetm(buf+8,4)==0x6d703432) // MPeg4
		 ||	(ggetm(buf,4)=='RIFF' && ggetm(buf+8,4)=='AVI ')) // AVI
        {
			ok = 10;

            if (!QT3_InitializeQTML(kInitializeQTMLNoSoundFlag | kInitializeQTMLUseGDIFlag))
            {
                ok = 50;
				if (ggetm(buf,4)=='RIFF' && ggetm(buf+8,4)=='AVI ') // AVI
				{
				    gseek(g, 0xb8);	 // read codec
 				    gread(g,buf,8);
					if (ggetm(buf+4,4)==0 && ggeti(buf+2,2)<=24)
						ok = 90;
				}
			}
		}
	}

    return(ok);
}

/****************************************************************/
/*  Canimport Functions                                            */
/****************************************************************/

OSErr QT3getdata(int offset, int size, void *data, void *ref)
{
    GINSTANCE *gx = (GINSTANCE*)ref;
    gseek(gx->gstream, offset);
    gread(gx->gstream, data, size);

    return (0);
}    

int GCALL QT3_open(GINSTANCE **gxptr, GSTREAM *g, const char *pathname,bool framecountflag)
{
    GINSTANCE *gx = NULL;
    QT3INFO *qtinfo = NULL;
    int ok = 0;

    gx = (GINSTANCE *) galloc(sizeof(GINSTANCE));
    if (gx)
    {
        qtinfo = (QT3INFO*)galloc(sizeof(QT3INFO));
        if (qtinfo)
        {
            memset(gx, 0, sizeof(GINSTANCE));
            memset(qtinfo, 0, sizeof(QT3INFO));

            *gxptr = gx;
            gx->signature=GMAKEID('M','O','O','V');
            gx->size=sizeof(GINSTANCE);
            gx->gstream = g;
            gx->framenum = 0;  /* keep track of which frame were pointing at */
            gx->frames = 0;
            gx->gref = (char*)qtinfo;

            if (!QT3_InitializeQTML(kInitializeQTMLNoSoundFlag | kInitializeQTMLUseGDIFlag))
            {
                if (!EnterMovies())
                {
                    FSSpec fspec;
                    char name[512];
                    OSErr result;
                    short mf=0;

                    memset(&fspec, 0, sizeof(fspec));
                    expandtofullpath(name,pathname);
                    c2pstr(name);
                    result = FSMakeFSSpec(0, 0, (const unsigned char *)name, &fspec);

#ifdef QUICKTIMEDEBUG
					printf("FSMakeFSSpec return %d\n",result);
#endif
                    //result = NewMovieFromUserProc(&qtinfo->movie, 0, NULL, QT3getdata, gx, NULL, 0);
                    result = OpenMovieFile (&fspec, &mf, fsRdPerm);

#ifdef QUICKTIMEDEBUG
					printf("OpenMovieFile return %d\n",result);
#endif
					if (result<0)
					{
	                    result = OpenMovieFile (&fspec, &mf, fsCurPerm);
#ifdef QUICKTIMEDEBUG
						printf("OpenMovieFile(,,fsCurPerm) return %d\n",result);
#endif
					}

#ifdef QUICKTIMEDEBUG
					printf("OpenMovieFile return %d\n",result);
#endif
                    if (!result)
                    {
                        result = NewMovieFromFile (&qtinfo->movie, mf, NULL, NULL, newMovieActive, NULL);
#ifdef QUICKTIMEDEBUG
						printf("NewMovieFromFile return %d\n",result);
#endif

						if (!result)
                        {
                            ok = gx->frames = QT3countframes(qtinfo);
                            if (ok)
                            {
                                Rect bounds;

                                SetMoviePlayHints(qtinfo->movie, hintsHighQuality, hintsHighQuality);
                                GetMovieBox(qtinfo->movie, &bounds);

                                /* Create an offscreen rendering context */
                                qtinfo->buffer = (char *) galloc(RECT_WIDTH(bounds)*RECT_HEIGHT(bounds)*sizeof(ARGB));
                                qtinfo->rowbytes = RECT_WIDTH(bounds)*sizeof(ARGB);
                                if (qtinfo->buffer)
                                {
                                    result = NewGWorldFromPtr (&qtinfo->gw, k32BGRAPixelFormat, &bounds, NULL, NULL, 0, qtinfo->buffer, qtinfo->rowbytes);
                                    if (!result)
                                        SetMovieGWorld(qtinfo->movie, qtinfo->gw, GetGWorldDevice(qtinfo->gw));
                                    else
                                        ok = 0;

                                    if (!ok)
                                        gfree(qtinfo->buffer);
                                }
                                else
                                    ok = 0;
                            }

                            if (!ok)
                                DisposeMovie(qtinfo->movie);
                        }
                        CloseMovieFile (mf);
                    }
                }

//                if (!ok)
//                    QT3_TerminateQTML();  /* Something failed */
            }

            qtinfo->valid = ok;

            if (!ok)
            {
                gfree(qtinfo);
                qtinfo = NULL;
                gx->gref = NULL;
            }
        }

        if (!ok)
        {
            gfree(gx);
            gx = NULL;
            *gxptr = NULL;
        }
    }

    return(ok);
}


GINFO * GCALL QT3_info(GINSTANCE *gx, int framenum)
{
    QT3INFO *qtinfo=NULL;
    GINFO *ginfo=NULL;

    qtinfo = (QT3INFO *)gx->gref;
    if (qtinfo && qtinfo->valid)
    {
        ginfo = (GINFO *) galloc(sizeof(GINFO));
        if (ginfo)
        {
            Rect bounds;
            GetMovieBox(qtinfo->movie, &bounds);

            memset(ginfo, 0, sizeof(GINFO));
            ginfo->signature        = GMAKEID('Q','T','3',' ');   /* Gimex type */
            ginfo->size             = sizeof(GINFO);            /* Length of GINFO */
            ginfo->version          = GIMEX_VERSION;
            ginfo->framenum         = framenum;    /* Current frame */
            ginfo->width            = RECT_WIDTH(bounds);
            ginfo->height           = RECT_HEIGHT(bounds);
            ginfo->bpp              = 32;
            ginfo->originalbpp      = 24;
            ginfo->subtype          = 0;/* Internal format sub-type 0-default */
            ginfo->packed           = 1;  /* Type of packing on original image. 0 none, 1 run, n other */
            ginfo->quality          = 100;  /* Quality of lossy packing 0..100 */
            ginfo->alphabits        = 0;
            ginfo->redbits          = 8;
            ginfo->greenbits        = 8;
            ginfo->bluebits         = 8;
			ginfo->fps              = qtinfo->fps;
        }
    }

    return(ginfo);
}

int GCALL QT3_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes)
{
    QT3INFO *qtinfo;
    int ok=0;
    Rect bounds;

    qtinfo = (QT3INFO *)gx->gref;
    if (qtinfo->valid)
    {
        GetMovieBox(qtinfo->movie, &bounds);

//            SetMovieTimeValue(qtinfo->movie, ginfo->framenum * GetMovieDuration(qtinfo->movie)/(gx->frames-1));
        SetMovieTimeValue(qtinfo->movie, QT3gettimefromframe(qtinfo, ginfo->framenum));

        InvalidateMovieRegion(qtinfo->movie, NULL);
        MoviesTask(qtinfo->movie, 0);

        /* Copy the movie to dest */
        {
            int i;
            int j;
            int *d;
            int *s;

            for (i=0; i<ginfo->height; ++i)
            {
                d = (int*)(dest + rowbytes*i);
                s = (int*)(qtinfo->buffer + qtinfo->rowbytes*i);

                for (j=0; j<ginfo->width; ++j)
                {
                    *d = *s;
					if (ginfo->alphabits)
						((ARGB*)d)->a = 255 - ((ARGB*)s)->a; /* Invert the alpha to make it opaque */
					else
						((ARGB*)d)->a = 255;
                    s++;
                    d++;
                }
            }
        }
        ok = 1;
    }

    return(ok);
}

int GCALL QT3_close(GINSTANCE *gx)
{
    int ok=0;
    QT3INFO *qtinfo;

    if (gx)
    {
        qtinfo = (QT3INFO *)gx->gref;
        if (qtinfo)
        {
            if (qtinfo->valid)
            {
                if (qtinfo->buffer) gfree(qtinfo->buffer);
                DisposeGWorld(qtinfo->gw);
                DisposeMovie(qtinfo->movie);
                ExitMovies();
//                QT3_TerminateQTML();
            }
            ok = gfree(qtinfo);
        }

        ok = ok && gfree((void *) gx);
    }
    return(ok);
}

