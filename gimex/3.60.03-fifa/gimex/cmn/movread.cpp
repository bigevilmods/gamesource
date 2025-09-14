// (c) 1994-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*               MOV - Macintosh QuickTime Raw Gimex                */
/*                                                                  */
/*              by FrANK G. Barchard & Kaz Okuda, EAC               */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History:                                       */
/* ------- ----   -- --------                                       */
/*    1.00 950421 FB Raw QuickTime Movie based on HSI Gimex code    */
/*    1.01 950512 FB Renamed to MOV, ported to SGI                  */
/*    1.02 950529 FB ginstance/unflattened/export in progress       */
/*    1.03 950605 FB export flatten raw                             */
/*    1.04 950606 FB STSD Sample Description proper structure       */
/*    1.05 950710 FB subtype 1 for non-flat                         */
/*    1.06 950717 FB 16 and 32 bit import/export (sans alpha)       */
/*    1.07 950814 FB alpha bits 8 for 32 bit (import)               */
/*    1.08 950824 FB import unflattened,stco frames,export -s fps   */
/*    1.09 950831 FB unflatten working, subtype 1 for raw           */
/*    1.10 950831 FB unflatten seek working                         */
/*    1.11 950901 FB huge exporter                                  */
/*    1.12 951115 FB Wing4 size guess                               */
/*    1.13 960208 FB fps calc duration                              */
/*    1.14 970606 FB is allow header before data                    */
/*    1.15 970616 KO VFW dcmprs, better frame counting/sizing, 8bit */
/*    1.16 970617 KO Fixed bug in decode, buffers previous frame    */
/*    1.17 970708 KO Decodes format w. no header & incorrect size.  */
/*    1.18 970714 KO Corrected output header(s/c) and info bits read*/
/*    1.19 970716 KO info/is work on unknown fmts, fixed header info*/
/*    1.20 970829 KO checks for invalid framecount                  */
/*    1.21 970902 KO uses QTW from Apple to decode compressed files */
/*    1.22 970909 KO Added call to QTTerminate on close             */
/*    1.23 970916 FB added check that QTTerminate is necessary      */
/*    1.24 970918 KO Reordered QT shutdown functions                */
/*    1.25 971111 KO Cleaned up looking for error in opening movie  */
/*    1.26 971208 FB QTTerminate optional                           */
/*    1.27 971209 KO frees mov->movie after dispose                 */
/*    1.28 990217 KO Crashed if invalid file header                 */
/*    1.29 990512 KO Moved QTW code into qt3read.c, ignore 'wide'.  */
/*    1.30 990722 FB MOVstrstr dont go off end                      */
/*    1.31 990803 FB ginfo->fps                                     */
/*    1.32 000404 FB about->movie                                   */
/*    1.33 001121 FB export zero frames                             */
/*    1.34 021002 FB remove qt extension to allow openasqt for qt3  */
/*    1.35 030226 FB bump 8 bpp up to 32 bpp on export              */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* 32 bit                                                           */
/* 8 bit compressed                                                 */
/* Reentrant                                                        */
/* import Only                                                      */
/* Line buffered                                                    */
/* Files: movread.cpp movwrite.cpp movgimex.h                       */
/* fps is used but for backwards compatibility, if its 0, subtype   */
/* is tried on export for setting frame rate. ie -s15.              */
/* On import, fps is set. subtype is used to flattened flag.        */
/* Subtype on import:                                               */
/*   0 if flattened (proper)                                        */
/*   1 if not flat                                                  */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Format Notes:                                                    */
/* -------------                                                    */
/* QuickTime Raw Movies are exported by Movie Player with           */
/* Non-Apple playable and Self Contained options on.                */
/* They are in a chunk format that has hierarchy.  See              */
/* Inside Machintosh: QuickTime                                     */
/*                                                                  */
/* This is a dump of the header at the start of a quicktime:        */
/*                                                                  */
/*  00000: 00821408 6d646174 00000000 00000000  ....mdat........    */
/*                                                                  */
/* This is a dump of the header at the end of a quicktime:          */
/*                                                                  */
/* 821400:                   00000349 6d6f6f76          ...Imoov    */
/* 821410: 0000006c 6d766864 00000000 abbd979d  ...lmvhd....+=..    */
/* 821420: abbd97f5 00000258 000005ad 00010000  +=.u...X...-....    */
/* 821430: 00ff0000 00000000 00000000 00010000  ................    */
/* 821440: 00000000 00000000 00000000 00010000  ................    */
/* 821450: 00000000 00000000 00000000 40000000  ............@...    */
/* 821460: 00000000 00000000 00000000 00000000  ................    */
/* 821470: 000005ad 00000000 00000002 000002bd  ...-...........=    */
/* 821480: 7472616b 0000005c 746b6864 0000000f  trak...\tkhd....    */
/* 821490: abbd8a51 abbd97f5 00000001 00000000  +=.Q+=.u........    */
/* 8214a0: 000005ad 00000000 00000000 00000000  ...-............    */
/* 8214b0: 00000000 00010000 00000000 00000000  ................    */
/* 8214c0: 00000000 00010000 00000000 00000000  ................    */
/* 8214d0: 00000000 40000000 01400000 00f00000  ....@....@...p..    */
/* 8214e0: 00000024 65647473 0000001c 656c7374  ...$edts....elst    */
/* 8214f0: 00000000 00000001 000005ad 00000000  ...........-....    */
/* 821500: 00010000 00000235 6d646961 00000020  .......5mdia...     */
/* 821510: 6d646864 00000000 abbd979d abbd97f5  mdhd....+=..+=.u    */
/* 821520: 0000000f 00000025 00000000 0000003a  .......%.......:    */
/* 821530: 68646c72 00000000 6d686c72 76696465  hdlr....mhlrvide    */
/* 821540: 6170706c 40000000 00020023 19417070  appl@......#.App    */
/* 821550: 6c652056 6964656f 204d6564 69612048  le Video Media H    */
/* 821560: 616e646c 65720000 01d36d69 6e660000  andler...Sminf..    */
/* 821570: 0014766d 68640000 00010040 80008000  ..vmhd.....@....    */
/* 821580: 80000000 00396864 6c720000 00006468  .....9hdlr....dh    */
/* 821590: 6c72616c 69736170 706c4000 00010002  lralisappl@.....    */
/* 8215a0: 00261841 70706c65 20416c69 61732044  .&.Apple Alias D    */
/* 8215b0: 61746120 48616e64 6c657200 00002464  ata Handler...$d    */
/* 8215c0: 696e6600 00001c64 72656600 00000000  inf....dref.....    */
/* 8215d0: 00000100 00000c61 6c697300 00000100  .......alis.....    */
/* 8215e0: 00015a73 74626c00 00006673 74736400  ..Zstbl...fstsd.    */
/* 8215f0: 00000000 00000100 00005672 61772000  ..........Vraw .    */
/* 821600: 00000000 00000100 01000161 70706c00  ...........appl.    */
/* 821610: 00000000 0003ff01 4000f000 48000000  ........@.p.H...    */
/* 821620: 48000000 00000000 01044e6f 6e650000  H.........None..    */
/* 821630: 00000000 00000000 00000000 00000000  ................    */
/* 821640: 00000000 00000000 000018ff ff000000  ................    */
/* 821650: 18737474 73000000 00000000 01000000  .stts...........    */
/* 821660: 25000000 01000000 1c737473 63000000  %........stsc...    */
/* 821670: 00000000 01000000 01000000 01000000  ................    */
/* 821680: 01000000 14737473 7a000000 00000384  .....stsz.......    */
/* 821690: 00000000 25000000 a4737463 6f000000  ....%...$stco...    */
/* 8216a0: 00000000 25000000 08000384 08000708  ....%...........    */
/* 8216b0: 08000a8c 08000e10 08001194 08001518  ................    */
/* 8216c0: 0800189c 08001c20 08001fa4 08002328  ....... ...$..#(    */
/* 8216d0: 080026ac 08002a30 08002db4 08003138  ..&,..*0..-4..18    */
/* 8216e0: 080034bc 08003840 08003bc4 08003f48  ..4<..8@..;D..?H    */
/* 8216f0: 080042cc 08004650 080049d4 08004d58  ..BL..FP..IT..MX    */
/* 821700: 080050dc 08005460 080057e4 08005b68  ..P\..T`..Wd..[h    */
/* 821710: 08005eec 08006270 080065f4 08006978  ..^l..bp..et..ix    */
/* 821720: 08006cfc 08007080 08007404 08007788  ..l...p...t...w.    */
/* 821730: 08007b0c 08007e90 08000000 18756474  .............udt    */
/* 821740: 61000000 0c574c4f 43003200 14000000  a....WLOC.2.....    */
/* 821750: 00                 .       (ðL                           */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */
#include <string.h>
#include "gimex.h"
#include "movgimex.h"

/* #if defined(__WATCOMC__) || defined(WIN32) */
#if 0

#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include "qtw.h"
#define __MSWINDOWS__

/*
 * Include bmpread.c replacing the file input with a read from a file buffer
 * bmpread only uses gread and gseek, no gtell.
 */
#define BMPCLIPBOARD 1

/* Used by bmpread.c */
#define gread       MOVmemread
#define gtell(g) 0
#define gseek       MOVmemseek

/* Defined by bmpread.c */
#define isbmp       MOVisbmp
#define aboutbmp    MOVaboutbmp
#define openbmp     MOVopenbmp
#define infobmp     MOVinfobmp
#define readbmp     MOVreadbmp
#define closebmp    MOVclosebmp

#include "bmpread.cpp"

#undef BMPCLIPBOARD
#undef gread
#undef gtell
#undef gseek
#undef isbmp
#undef aboutbmp
#undef openbmp
#undef infobmp
#undef readbmp
#undef closebmp

#endif


/****************************************************************/
/*  Internal Functions                                          */
/****************************************************************/

char *MOVstrnstr(char *s, char *str, int n)
{
    int len=strlen(str);

    while (n>=len)
    {
        if (!strncmp(s,str,len))
            return(s);
        ++s;
        --n;
    }
    return((char *)0);
}

static int MOVsizematch(int width, int height, int bpp, int flen)
{
    int frames;
    int ok=0;
    GPOS framesize;

    framesize = width*bpp/8*height;

    frames = (int)((flen-8)/framesize);
    if ((flen-8)==frames*framesize)
        ok = 1;
    return(ok);
}

GPOS MOVseekchunk(GSTREAM *g,unsigned int id)
{
    char buf[8];
    int clen;
    GPOS cpos=0;
    unsigned int cid=0;
    int  ok=1;
    int iterations=5;

    while (ok && iterations)
    {
        iterations--;
        cpos = gtell(g);
        ok = gread(g,buf,8);
        if (ok)
        {
            clen = ggetm(buf+0,4);
            cid  = ggetm(buf+4,4);
            if (cid==id)
            {
                ok = gseek(g,gtell(g)-8);
                break;
            }
            cpos += clen;
            ok = 0;
            if (cpos>=0 && cpos<glen(g))
                ok = gseek(g,cpos);
        }
    }
    if (!ok || !iterations)
        cpos = -1;
    return(cpos);
}


int MOVseekframe(GSTREAM *g, MOVINFO *mov, int framenum)
/*
 * Seeks to the frame and returns the size of the frame
 */
{
    GPOS framesize;
    GPOS offset;
    int lastspc;
    int nextspc;
    int framechunk;
    int lastchunk;
    int nextchunk;
    int  frame;
    int  chunks;
    int  numberofentries;
    int  i;


    if (mov->chunkoffsettbl && mov->sampletochunk && mov->samplesizetbl)
    {
        lastspc    = 0;
        framechunk = -1;
        frame      = -1;
        chunks     = MOVGETFIELD((mov->chunkoffsettbl)->NumberOfEntries);
        numberofentries = MOVGETFIELD((mov->sampletochunk)->NumberOfEntries);
        for(i=0;i<numberofentries;i++)
        {
            nextchunk = MOVGETFIELD((mov->sampletochunk)->Table[i].FirstChunk);
            nextspc   = MOVGETFIELD((mov->sampletochunk)->Table[i].SamplesPerChunk);
            while ((framechunk+1 < nextchunk-1) && (frame < framenum))
            {
                frame += lastspc;
                framechunk++;
            }
            if (frame >= framenum) break;
            lastspc = nextspc;
            lastchunk = nextchunk;
        }

        while ((framechunk < chunks-1) && (frame < framenum))
        {
            frame += lastspc;
            framechunk++;
        }

        if (frame < framenum)
            return(0);

        /*
         * Now - 'frame' is the last frame in chunk 'framechunk'.
         *       There are 'lastspc' frames in 'framechunk'.
         *       The frame 'framenum' is in 'framechunk'.
         */

        /*
         * So far all movies found are offset from the beginning of the file so
         * I will go with that.
         */
        offset = MOVGETFIELD((mov->chunkoffsettbl)->Table[framechunk]);

        /* Figure how many frames came before this one in the chunk and add their sizes
         * to the offset */
        if (MOVGETFIELD((mov->samplesizetbl)->SampleSize) > 0)  /* All same size */
        {
            framesize = MOVGETFIELD((mov->samplesizetbl)->SampleSize);
            offset += (framenum - (frame-lastspc+1)) * framesize;
        }
        else
        {
            for (i = frame-lastspc+1; i < framenum; i++)
            {
                offset += MOVGETFIELD((mov->samplesizetbl)->Table[i]);
            }
            framesize = MOVGETFIELD((mov->samplesizetbl)->Table[framenum]);
        }
    }
    else
    {
        framesize = mov->width*mov->height*mov->bpp/8;
        offset = mov->mdatpos+8+framenum*framesize;
    }

    gseek(g,offset);

    return((int)framesize);
}

int MOVdecoderaw(GSTREAM *g, char *dest, int size, GINFO *ginfo, int rowbytes)
{
    char *linebuf;
    char *lineptr;
    int bpp;
    int bpl;
    int y;
    int x;
    int ok=0;
    ARGB *d;

    bpp = ginfo->originalbpp;
    bpl = ginfo->width*bpp/8;

    linebuf = (char *)galloc(bpl);

    if (linebuf)
    {
        ok = 1;

        for (y=0; y<ginfo->height && ok; ++y)
        {
            d = (ARGB *) (dest+rowbytes*y);

            ok = gread(g,linebuf,bpl);
            lineptr = linebuf;

            switch (bpp)
            {
                case 16:
                    for (x=0; x<ginfo->width; ++x)
                    {
                        unsigned int r;
                        unsigned int g;
                        unsigned int b;
                        unsigned int p;

                        p = ggetm(lineptr,2);
                        r = (p>>10)&31;
                        g = (p>>5)&31;
                        b = (p>>0)&31;

                        d->a = 255;
                        d->r = (unsigned char)((r*255+16)/31);
                        d->g = (unsigned char)((g*255+16)/31);
                        d->b = (unsigned char)((b*255+16)/31);
                        ++d;
                        lineptr += 2;
                    }
                    break;


                default:
                case 24:
                    for (x=0; x<ginfo->width; ++x)
                    {
                        d->a = 255;
                        d->r = lineptr[0];
                        d->g = lineptr[1];
                        d->b = lineptr[2];
                        ++d;
                        lineptr += 3;
                    }
                    break;

                case 32:
                    for (x=0; x<ginfo->width; ++x)
                    {
                        d->a = 255-lineptr[0];  // Quicktime inverts alpha
                        d->r = lineptr[1];
                        d->g = lineptr[2];
                        d->b = lineptr[3];
                        ++d;
                        lineptr += 4;
                    }
                    break;
            }
        }
    }
    gfree(linebuf);
    return(ok);
}


#ifdef __MSWINDOWS__ /* The following functions are for windows use only */

    typedef OSErr (QTAPI *QTInitializeType) ( LPLONG );
    typedef OSErr (QTAPI *OpenMovieFileType) ( LPCSTR, MovieFile FAR *, UINT );
    typedef OSErr (QTAPI *NewMovieFromFileType) ( Movie FAR *, MovieFile, LPINT, LPSTR, UINT, Boolean FAR *);
    typedef TimeValue (QTAPI *GetMovieDurationType) ( Movie );
    typedef PicHandle (QTAPI *GetMoviePictType) ( Movie, TimeValue );
    typedef DIBHandle (QTAPI *PictureToDIBType) ( PicHandle );
    typedef VOID (QTAPI *DisposePictureType) ( PicHandle );
    typedef OSErr (QTAPI *CloseMovieFileType) ( MovieFile );
    typedef VOID (QTAPI *DisposeMovieType) ( Movie );
    typedef TimeScale (QTAPI *GetMovieTimeScaleType) ( Movie );
    typedef Track (QTAPI *GetMovieIndTrackTypeType) ( Movie, LONG, OSType, LONG );
    typedef PicHandle (QTAPI *GetTrackPictType) ( Track, TimeValue);
    typedef void (QTAPI *QTTerminateType) ( void );
    typedef OSErr (QTAPI *GetMoviesErrorType) ( void );

    static QTInitializeType         pQTInitialize;
    static OpenMovieFileType        pOpenMovieFile;
    static NewMovieFromFileType     pNewMovieFromFile;
    static GetMovieDurationType     pGetMovieDuration;
    static GetMoviePictType         pGetMoviePict;
    static PictureToDIBType         pPictureToDIB;
    static DisposePictureType       pDisposePicture;
    static CloseMovieFileType       pCloseMovieFile;
    static DisposeMovieType         pDisposeMovie;
    static GetMovieTimeScaleType    pGetMovieTimeScale;
    static GetMovieIndTrackTypeType pGetMovieIndTrackType;
    static GetTrackPictType         pGetTrackPict;
    static QTTerminateType          pQTTerminate;

    static char *MOVrfilebuf=0;
    static char *MOVrfileptr=0;
    static HANDLE MOVrhandle=0;

    static HINSTANCE MOVloadlibrary()
    {
        HINSTANCE hlib;

        hlib = LoadLibrary(MOVLIBRARYFILENAME);
        if (hlib != NULL)
        {
            pQTInitialize =         (QTInitializeType) GetProcAddress(hlib, "QTInitialize");
            pOpenMovieFile =        (OpenMovieFileType) GetProcAddress(hlib, "OpenMovieFile");
            pNewMovieFromFile =     (NewMovieFromFileType) GetProcAddress(hlib, "NewMovieFromFile");
            pGetMovieDuration =     (GetMovieDurationType) GetProcAddress(hlib, "GetMovieDuration");
            pGetMoviePict =         (GetMoviePictType) GetProcAddress(hlib, "GetMoviePict");
            pPictureToDIB =         (PictureToDIBType) GetProcAddress(hlib, "PictureToDIB");
            pDisposePicture =       (DisposePictureType) GetProcAddress(hlib, "DisposePicture");
            pCloseMovieFile =       (CloseMovieFileType) GetProcAddress(hlib, "CloseMovieFile");
            pDisposeMovie =         (DisposeMovieType) GetProcAddress(hlib, "DisposeMovie");
            pGetMovieTimeScale =    (GetMovieTimeScaleType) GetProcAddress(hlib, "GetMovieTimeScale");
            pGetMovieIndTrackType = (GetMovieIndTrackTypeType) GetProcAddress(hlib, "GetMovieIndTrackType");
            pGetTrackPict =         (GetTrackPictType) GetProcAddress(hlib, "GetTrackPict");
            pQTTerminate =          (QTTerminateType) GetProcAddress(hlib, "QTTerminate");
        }

        return(hlib);
    }

    static int MOVfreelibrary(HINSTANCE hlib)
    {
        return((int)FreeLibrary(hlib));
    }

    static int MOVdecompressbegin(MOVINFO *mov, const char *fname)
    {
        int ok = 0;
        MovieFile mfMovie;

        if (mov->compression == MOVRAWFCC)
        {
            ok = 1;
        }
        else
        {
            mov->library = (void*)MOVloadlibrary();
            if (mov->library)
            {
                if (!pQTInitialize (NULL))
                {
                    mov->movie = galloc(sizeof(Movie));
                    if (mov->movie)
                    {
                        pOpenMovieFile (fname, &mfMovie, OF_READ);
                        pNewMovieFromFile ((Movie*)mov->movie, mfMovie, NULL, NULL, 0, NULL);
                        pCloseMovieFile (mfMovie);
                        ok = 1;
                    }
                    else
                    {
                        mov->movie = 0;
                    }
                }
                else
                {
                    MOVfreelibrary((HINSTANCE)mov->library);
                    mov->library = 0;
                }
            }
        }

        return(ok);
    }

    static int MOVdecompressend(MOVINFO *mov)
    {
        if (mov->library)
        {
            if (mov->movie)
            {
                pDisposeMovie((Movie)mov->movie);
                gfree(mov->movie);
                mov->movie = 0;
            }
            if (pQTTerminate) pQTTerminate();
            MOVfreelibrary((HINSTANCE)mov->library);
        }
        return(1);
    }

    DIBHandle MOVseekdib(MOVINFO *mov, int frame, int frames)
    {
        TimeValue time;
        DIBHandle dib=0;
        PicHandle pic;
        /*Track trkVideo; */

        time = pGetMovieDuration(*(Movie*)mov->movie);

        /* */
        /* Used to use this to get only the video track image (No text) But this would */
        /* Would not look the same as playing the movie, so simplified to just get the frame */
        /* */
        /*trkVideo = pGetMovieIndTrackType(*(Movie*)mov->movie, 1, VideoMediaType, movieTrackMediaType); */
        /*if (trkVideo) */
        /*{ */
        /*    pic = pGetTrackPict (trkVideo, frame * time\(frames-1)); */

            pic = pGetMoviePict (*(Movie*)mov->movie, frame * time/(frames-1));
            if (pic)
            {
                dib = pPictureToDIB(pic);
                pDisposePicture(pic);
            }
        /*} */

        return(dib);
    }

    static int MOVmemread(GSTREAM *g, void *buf, int size)
    {
        int ok=0;

        if (MOVrfileptr)
        {
            if (memcpy(buf, MOVrfileptr, size))
            {
                MOVrfileptr+=size;
                ok = size;
            }
        }
        return(ok);
    }

    static int MOVmemseek(GSTREAM *g, int offset)
    {
        MOVrfileptr = MOVrfilebuf + offset;
        return(offset);
    }

    int GCALL MOV_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes)
    {
        DIBHandle dib;
        GINSTANCE *gxbmp;
        GINFO *ginfobmp;
        MOVINFO *mov;
        int framesize;
        int ok=0;

        mov = (MOVINFO *)gx->gref;

        if (mov->valid)
        {

            if (mov->compression == MOVRAWFCC)
            {
                framesize = MOVseekframe(gx->gstream, mov, ginfo->framenum);
                ok = MOVdecoderaw(gx->gstream, dest, framesize, ginfo, rowbytes);
            }
            else
            {
                dib = MOVseekdib(mov, ginfo->framenum, gx->frames);
                if (dib)
                {
                    /* Use BMPread to read the ginfo and dest */
                    MOVrhandle = dib;
                    MOVrfilebuf = GlobalLock(MOVrhandle);
                    MOVrfileptr = MOVrfilebuf;

                    if (MOVopenbmp(&gxbmp, 0, "dummy.bmp"))
                    {
                        ginfobmp = MOVinfobmp(gx,1);
                        if (ginfobmp)
                        {
                            ok = MOVreadbmp(gxbmp, ginfobmp, dest, rowbytes);
                            gfree(ginfobmp);
                        }
                        MOVclosebmp(gxbmp);
                    }

                    GlobalUnlock(MOVrhandle);
                    GlobalFree(MOVrhandle);
                    MOVrfilebuf = 0;
                    MOVrfileptr = 0;
                }
            }
        }

        return(ok);
    }

#else /* Not windows */

    static int MOVdecompressbegin(MOVINFO *mov, const char *fname)
    {
        if (mov->compression == MOVRAWFCC)
            return(1);
        else
            return(0);
    }

    static int MOVdecompressend(MOVINFO *mov)
    {
        if (mov->compression == MOVRAWFCC)
            return(1);
        else
            return(0);
    }

    int GCALL MOV_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes)
    {
        int ok=0;
        int framesize;
        MOVINFO *mov;

        mov = (MOVINFO *)gx->gref;

        if (mov->compression == MOVRAWFCC)
        {
            framesize = MOVseekframe(gx->gstream, mov, ginfo->framenum);
            ok = MOVdecoderaw(gx->gstream, dest, framesize, ginfo, rowbytes);
        }

        return(ok);
    }

#endif


ARGB *MOVconvertcolourtable(MOVCTABStruct *incolours, int numcolours)
{

#define MOV_RED(colour)   (colour>>4 & 0xFFFF)
#define MOV_GREEN(colour) (colour>>2 & 0xFFFF)
#define MOV_BLUE(colour)  (colour    & 0xFFFF)

    ARGB *outcolours=0;
    int i;

    outcolours = (ARGB *) galloc(numcolours * sizeof(ARGB));
    if (outcolours)
    {
        for (i=0; i < numcolours; i++)
        {
            outcolours[i].a = 255;
            outcolours[i].r = (unsigned char)(((unsigned int)MOV_RED(MOVGETFIELD(incolours->Table[i])) * 255 + 32768)/65536);
            outcolours[i].g = (unsigned char)(((unsigned int)MOV_GREEN(MOVGETFIELD(incolours->Table[i])) * 255 + 32768)/65536);
            outcolours[i].b = (unsigned char)(((unsigned int)MOV_BLUE(MOVGETFIELD(incolours->Table[i])) * 255 + 32768)/65536);
        }
    }
    return(outcolours);
}

/****************************************************************/
/*  Information Functions                                       */
/****************************************************************/

GABOUT * GCALL MOV_about(void)
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
        info->canexport       = 1;    /* supports exporting */
        info->importpacked    = 1;    /* supports importing packed */
        info->exportpacked    = 0;    /* supports exporting packed */
        info->import8         = 1;    /* supports importing 8 bit indexed */
        info->export8         = 0;    /* supports exporting 8 bit indexed */
        info->import32        = 1;    /* supports importing 32 bit direct rgb */
        info->export32        = 1;    /* supports exporting 32 bit direct rgb */
        info->multiframe      = 1;    /* supports multiple frames */
        info->multifile       = 0;    /* format requires additional files or resource fork */
        info->multisize       = 0;    /* supports each frame different size */
        info->framebuffer     = 0;    /* module requires memory to buffer entire frame */
        info->external        = 0;    /* module uses external resources */
        info->usesfile        = 1;    /* module is file based */
        info->maxframename    = 0;    /* max frame name character */
        info->movie           = 1;    /* movie file format */
        info->file64          = 1;    /* this format supports 64 bit file sizes */
        info->defaultquality  = 100;  /* default pack quality */
        info->mactype[0]      = GMAKEID('M','o','o','V');   /* mac file system types used */
        strcpy(info->extensions[0], ".mov");                /* extensions */
        strcpy(info->extensions[1], ".moov");               /* extensions */
        strcpy(info->authorstr,     "FrANK Barchard & Kaz Okuda"); /* authors */
        strcpy(info->versionstr,    "1.35");                /* version # */
        strcpy(info->shorttypestr,  "MOV");                 /* type */
        strcpy(info->wordtypestr,   "QuickTime");           /* wordtype */
        strcpy(info->longtypestr,   "QuickTime RAW");     /* longtype */
    }
    return(info);
}

/* check for reasonable header: */
/* has 8 byte chunk (len/id): */
/*  00000: 00821408 6d646174 00000000 00000000  ....mdat........    */
/*  00000: 00000301 6d6f6f76 0000006c 6d766864  ....moov...lmvhd    */
/*  00000: 00000008 77696465 01356008 6d646174  ....wide.5`.mdat    */

int GCALL MOV_is(GSTREAM *g)
{
    unsigned char movheader[8];
    int ok=0;

    gseek(g,(GPOS)0);
    if (gread(g,movheader,8))
    {
        if ( ggetm(movheader+4,4)==GMAKEID('w','i','d','e') )
        {
            /* There is an empty (8byte) 'wide' chunk to skip at the beginning of the file */
            gseek(g, 8);
        }
        else
        {
            gseek(g, 0);
        }
    }

    if (gread(g,movheader,8))
    {
        if ((ggetm(movheader+4,4)==GMAKEID('m','d','a','t'))
         || (ggetm(movheader+4,4)==GMAKEID('m','o','o','v')))
        {
            GINSTANCE *gx;
            if (MOV_open(&gx, g, "name", 0))
            {
                ok = 90;
                MOV_close(gx);
            }
        }
    }
    return(ok);
}

/****************************************************************/
/*  Import Functions                                            */
/****************************************************************/

int GCALL MOV_open(GINSTANCE **gxptr, GSTREAM *g, const char *pathname,bool framecountflag)
{
    GINSTANCE *gx = NULL;
    MOVINFO *mov = NULL;
    int  ok=0;
    int  frames;
    int  chunks=0;
    int  samples;
    int  lastchunk;
    int  width;
    int  height;
    int  bpp;
    int  fps;
    int  subtype;
    int  numberofentries;
    int  i;
    char buf[8];
    char *moov = NULL;
    char *trak;
    MOVSTCOStruct *chunkoffsettbl=0;
    MOVSTSCStruct *sampletochunk=0;
    MOVSTSZStruct *samplesizetbl=0;
    MOVSTSSStruct *keyframes=0;
    ARGB          *colourtbl=0;
    int           numcolours=0;
    char *fieldptr=0;
    int clen;
    int tlen;
    GPOS cpos;
    GPOS mdatpos;
    GPOS mdatlen;
    int compression = MOVRAWFCC;
    int filestart = 0;

    gx = (GINSTANCE *) galloc(sizeof(GINSTANCE));
    if (gx)
    {
        memset(gx, 0, sizeof(GINSTANCE));
        *gxptr = gx;
        gx->signature=GMAKEID('M','O','O','V');
        gx->size=sizeof(GINSTANCE);
        gx->gstream = g;
        gx->framenum = -1;  /* keep track of which frame were pointing at */

        /* Check for and ignore a 'wide' chunk at the start of the file */
        {
            char buf[8];
            gseek(g,filestart);
            if (gread(g,buf,8))
            {
                if ( ggetm(buf+4,4)==GMAKEID('w','i','d','e') )
                    filestart = 8;
            }
        }

        gseek(g, filestart);
        gx->gref = galloc(sizeof(MOVINFO));
        if (gx->gref)
        {
            mov = (MOVINFO *) gx->gref;
            memset((void *) mov,0,sizeof(MOVINFO));

            mdatpos = MOVseekchunk(g,GMAKEID('m','d','a','t'));
            if (mdatpos>=0)
            {
                gseek(g,mdatpos);
                ok = gread(g,buf,8);
                mdatlen = ggetm(buf,4);
                if (mdatlen == 0)
                    mdatlen = glen(g);
            }
            else
            {
                mdatpos = 0;
                mdatlen = glen(g);
            }

            gseek(g,filestart);
            cpos = MOVseekchunk(g,GMAKEID('m','o','o','v'));

            width = 320;
            height = 240;
            bpp = 32;
            subtype = 0;
            moov = 0;
            chunkoffsettbl = 0;
            frames = 1;
            fps = 15;
            if ((cpos>=0) && (cpos<glen(g)))
            {
                gseek(g,cpos);
                ok = gread(g,buf,8);
                clen = ggetm(buf,4);    /* moov header chunk len */
                moov = (char *) galloc(clen);
                gread(g,moov,clen-8);

                trak = 0;
                tlen = 0;

                /* Find the movie trak description atom */
                do
                {
                    trak = MOVstrnstr(moov+tlen,"trak",clen-tlen);
                    if (trak)
                    {
                        tlen += ggetm(trak-4,4);
                        fieldptr = MOVstrnstr(trak,"hdlr",tlen); /* search for hdlr chunk */
                    }
                }
                while (trak &&
                       (MOVGETFIELD(((MOVHDLRStruct *)(fieldptr-4))->ComponentSubType) != GMAKEID('v','i','d','e')));

                if (!trak) /* No data handler found */
                {
                    gfree(mov);
                    gfree(moov);
                    gfree(gx);
                    *gxptr = 0;
                    return(0);
                }
                tlen = ggetm(trak-4,4);
                if (tlen>clen)
                    tlen = clen;

                fieldptr = MOVstrnstr(trak,"tkhd",tlen); /* search for tkhd chunk */
                if (fieldptr)
                {
                    width  = ggetm(fieldptr+80,2);
                    height = ggetm(fieldptr+84,2);
                }

                fieldptr = MOVstrnstr(trak,"stsd",tlen); /* search for stsd chunk */
                if (fieldptr)
                {
                    compression = MOVGETFIELD(((MOVSTSDStruct *)(fieldptr-4))->STSD.CodecType);
                    bpp = MOVGETFIELD(((MOVSTSDStruct *)(fieldptr-4))->STSD.Depth);
                }

                fieldptr = MOVstrnstr(trak,"stco",tlen); /* search for Chunk-offset atom */
                if (fieldptr)
                {
                    chunkoffsettbl = (MOVSTCOStruct *)(fieldptr-4);
                    chunks = MOVGETFIELD(((MOVSTCOStruct *)(fieldptr-4))->NumberOfEntries);
                }

                fieldptr = MOVstrnstr(trak,"stsz",tlen); /* search for Sample size table atom */
                if (fieldptr)
                {
                    samplesizetbl = (MOVSTSZStruct *)(fieldptr-4);
                }

                fieldptr = MOVstrnstr(trak,"stss",tlen); /* search for Sync sample table atom */
                if (fieldptr)
                {
                    keyframes = (MOVSTSSStruct *)(fieldptr-4);
                }

                fieldptr = MOVstrnstr(trak,"mdhd",tlen); /* search for Sync sample table atom */
                if (fieldptr)
                {
                    fps = MOVGETFIELD(((MOVMDHDStruct *)(fieldptr-4))->TimeScale);
                    if (fps<0 || fps>1000) fps = 15;
                }

                fieldptr = MOVstrnstr(trak,"stsc",tlen); /* search for Sample-to-chunk atom */
                if (fieldptr)
                {
                    sampletochunk = (MOVSTSCStruct *)(fieldptr - 4);
                    frames = 0;
                    numberofentries = MOVGETFIELD(((MOVSTSCStruct *)(fieldptr-4))->NumberOfEntries);
                    samples = MOVGETFIELD(((MOVSTSCStruct *)(fieldptr-4))->Table[0].SamplesPerChunk);
                    lastchunk = MOVGETFIELD(((MOVSTSCStruct *)(fieldptr-4))->Table[0].FirstChunk);
                    for(i=1;i<numberofentries;i++)
                    {
                        frames += samples * (MOVGETFIELD(((MOVSTSCStruct *)(fieldptr-4))->Table[i].FirstChunk) - lastchunk);
                        samples   = MOVGETFIELD(((MOVSTSCStruct *)(fieldptr-4))->Table[i].SamplesPerChunk);
                        lastchunk = MOVGETFIELD(((MOVSTSCStruct *)(fieldptr-4))->Table[i].FirstChunk);
                    }

                    if (lastchunk <= chunks)
                    {
                        frames += samples * (chunks - lastchunk + 1);
                    }
                }
                else
                {
                    frames = (int)((mdatlen-8)/(width*height*bpp/8));
                }

                if (( frames > (mdatlen-8)/(width*height*bpp/8)) && (compression == MOVRAWFCC) )
                {
                    /* Too many frames */
                    frames = (int)((mdatlen-8)/(width*height*bpp/8));
                }


                fieldptr = MOVstrnstr(moov,"ctab",clen); /* search for colour table */
                if (fieldptr)
                {
                    numcolours = (int)MOVGETFIELD(((MOVCTABStruct *)(fieldptr-4))->NumberOfEntries)+1;
                    colourtbl = MOVconvertcolourtable((MOVCTABStruct *)(fieldptr-4), numcolours);
                }

            }
            else
            {
                clen = (int)(glen(g)-filestart);
                if (MOVsizematch(320,240,24,clen))
                {
                    width = 320;
                    height = 240;
                    bpp = 24;
                }
                else if (MOVsizematch(320,240,32,clen))
                {
                    width = 320;
                    height = 240;
                    bpp = 32;
                }
                else if (MOVsizematch(320-32,240-32,32,clen))
                {
                    width = 320-32;
                    height = 240-32;
                    bpp = 32;
                }
                else if (MOVsizematch(320-32,240-32,24,clen))
                {
                    width = 320-32;
                    height = 240-32;
                    bpp = 24;
                }
                else if (MOVsizematch(320,200,32,clen))
                {
                    width = 320;
                    height = 200;
                    bpp = 32;
                }
                else if (MOVsizematch(320,200,24,clen))
                {
                    width = 320;
                    height = 200;
                    bpp = 24;
                }
                else if (MOVsizematch(128,80,24,clen))
                {
                    width = 128;
                    height = 80;
                    bpp = 24;
                }
                else if (MOVsizematch(320,164,24,clen))
                {
                    width = 320;
                    height = 164;
                    bpp = 24;
                }
                else
                {
                    width = 320;
                    height = 240;
                    bpp = 24;
                }

                frames = (int)((glen(g)-8)/(width*height*bpp/8));
                subtype = 1;
            }

            mov->width = width;
            mov->height = height;
            mov->bpp = bpp;
            mov->subtype = subtype;
            mov->mdatpos = (int)mdatpos;
            mov->movheader = moov;
            mov->compression = compression;
            mov->chunkoffsettbl = chunkoffsettbl;
            mov->samplesizetbl  = samplesizetbl;
            mov->sampletochunk  = sampletochunk;
            mov->keyframes      = keyframes;
            mov->colourtbl      = colourtbl;
            mov->numcolours     = numcolours;
            mov->fps            = fps;
            mov->valid          = MOVdecompressbegin(mov, pathname);
            ok = ok && mov->valid;

            if (ok) ok = gx->frames = frames;
        }
    }

    if (!ok)
    {
        if (gx->gref) gfree(gx->gref);
        if (gx) gfree(gx);
        if (moov) gfree(moov);
    }

    return(ok);
}

GINFO * GCALL MOV_info(GINSTANCE *gx, int framenum)
{
    MOVINFO *mov;
    GINFO *ginfo=0;

    mov = (MOVINFO *)gx->gref;

    if (framenum < gx->frames)
        ginfo = (GINFO *) galloc(sizeof(GINFO));

    if (ginfo)
    {
        memset(ginfo,0,sizeof(GINFO));

        /* template for ginfo */

        ginfo->signature        = GMAKEID('M','O','O','V');   /* Gimex type */
        ginfo->size             = sizeof(GINFO);            /* Length of GINFO */
        ginfo->version          = GIMEX_VERSION;
        ginfo->framenum         = framenum;    /* Current frame */
        ginfo->width            = mov->width;  /* Width of bitmap in pixels */
        ginfo->height           = mov->height; /* Height of bitmap in pixels */
        ginfo->subtype          = mov->subtype;/* Internal format sub-type 0-default */
        ginfo->packed           = mov->compression==GMAKEID('r','a','w',' ')?0:mov->compression;  /* Type of packing on original image. 0 none, 1 run, n other */
        ginfo->quality          = 100;  /* Quality of lossy packing 0..100 */
        ginfo->alphabits        = (mov->bpp==32)?8:0;  /* Number of bits in alpha channel */
        ginfo->redbits          = (mov->bpp==32)?8:(mov->bpp>8)?(mov->bpp/3):0;    /* Number of bits in red channel */
        ginfo->greenbits        = (mov->bpp==32)?8:(mov->bpp>8)?(mov->bpp/3):0;    /* Number of bits in grreen channel */
        ginfo->bluebits         = (mov->bpp==32)?8:(mov->bpp>8)?(mov->bpp/3):0;    /* Number of bits in blue channel */
        ginfo->centerx          = 0;    /* Center Point relative to upper left corner */
        ginfo->centery          = 0;
        ginfo->defaultx         = 0;    /* Default Coordinate Point */
        ginfo->defaulty         = 0;
        ginfo->numhotspots      = 0;    /* Number of Hot Spots defined */
        ginfo->bpp              = (mov->bpp<=8)?8:32;   /* Bits per pixel (8 or 32) */
        ginfo->originalbpp      = mov->bpp;        /* Bits per pixel in original image (1 to 32) */
        ginfo->numcolours       = mov->numcolours; /* Number of colours in original indexed palette */
        ginfo->fps              = (float) mov->fps; /* Frame rate */

        if (mov->colourtbl)
            memcpy(ginfo->colourtbl, mov->colourtbl, mov->numcolours*sizeof(ARGB));
    }

    return(ginfo);
}

/*
 * readmov is defined above so as to include all of the windows specific code in
 * one ifdef.
 */

int GCALL MOV_close(GINSTANCE *gx)
{
    int ok=0;
    MOVINFO *mov;

    if (gx)
    {
        mov = (MOVINFO *)gx->gref;
        if (mov)
        {
            if (mov->movheader) gfree(mov->movheader);
            if (mov->colourtbl) gfree(mov->colourtbl);
            ok = MOVdecompressend(mov);
            gfree(gx->gref);
        }

        ok = ok && gfree((void *) gx);
    }
    return(ok);
}

