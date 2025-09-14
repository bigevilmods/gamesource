// (c) 1994-2007 Electronic Arts Inc.

/* ABSTRACT */
/*------------------------------------------------------------------*/
/*                                                                  */
/*                       EXE - Alchemy Gimex                        */
/*                                                                  */
/*                      by Frank Barchard, EAC                      */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History:                                       */
/* ------- ----   -- --------                                       */
/*    1.00 000328 FB jpeg2000 using external tool based on clp      */
/*    1.01 000826 FB about disable if tools not found               */
/*    1.02 000901 FB export                                         */
/*    1.03 000908 FB disable about tristate                         */
/*    1.04 001205 FB redid types without exe                        */
/*    1.05 001208 FB unix build, but crippled                       */
/*    1.06 010303 FB os2 icon removed                               */
/*    1.07 030506 FB pdf added                                      */
/*    1.08 040113 FB pcd added                                      */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Module Notes:                                                    */
/* -------------                                                    */
/* non-reentrant import, due to subtype global                      */
/* Files: exeread.cpp exewrite.cpp exegimex.h                       */
/* writes intermediate files and spawns a tool                      */
/*                                                                  */
/* ADEX {43}.................. --A   Gem VDI Image {73}........ --g */
/* Adobe PDF {add-27}......... --d   GIF {add-32}............... -g */
/* Adobe Photoshop {add-29}...---p   GOES {77}................. --G */
/* Alias PIX / Vivid IMG {46}. --I   Histogram {79}............. -H */
/* Alpha BMP {47}.............. -M   Hitachi Raster {81}....... --h */
/* Autologic {48}............. --a   HP PCL {82}................ -P */
/* AVHRR {49}................. --R   HP RTL {87}............... --r */
/* Binary (BIF) {51}........... -B   HP-48sx {92}.............. --H */
/* Calcomp {54}............... --l   HSI JPEG {93}............. --j */
/* CALS {56}.................. --c   HSI Palette {94}........... -l */
/* Core IDC {57}.............. --B   HSI Raw {95}............... -r */
/* Cubicomp {59}.............. --P   IBM Picture Maker {96}.... --i */
/* Dr. Halo CUT {61}.......... --C   IFF/ILBM {97}.............. -i */
/* Enc. PostScript {62}........ -e   Imaging Technology........---M */
/* Epson Stylus {add-30}...... --K   Img Software Set {98}..... --Q */
/* ER Mapper Raster {64}...... --m   Intergraph {add-37}.......---r */
/* Erdas LAN/GIS {66}......... --e   JEDMICS CCITT4............---E */
/* Fargo Primera {68}......... --k   Jovian VI {99}............ --J */
/* First Publisher {70}....... --F   JPEG {add-38}.............. -j */
/* FOP {71}................... --f   Lumena CEL {103}.......... --L */
/* Macintosh PICT {104}........ -m   Sharp GPB.................---G */
/* MacPaint {106}............. --t   Spaceward {add-43}........---s */
/* MTV {107}.................. --M   Spot Image {132}.......... --S */
/* Multi-Image Palette {108}... -L   Stork {134}................ -K */
/* OS/2 BMP {110}.............. -O   Sun Icon {136}............ --N */
/* OS/2 Icon {add-999}........ --O   Sun Raster {137}........... -s */
/* PCPaint/Pictor {111}........ -A   Targa {139}................ -a */
/* PCX {113}................... -p   TIFF {141}................. -t */
/* PDS {117}.................. --p   US Patent Image {add-44}..---P */
/* PNG {add-41}...............---n   Utah RLE {145}............ --u */
/* Portable BitMap (PBM) {122}. -k   Verity Image Format {147}. --E */
/* Puzzle {124}............... --U   VITec {148}................ -T */
/* Q0 {125}................... --q   Windows BMP {150}.......... -w */
/* QDV {126}.................. --D   WPG {153}.................. -W */
/* QRT Raw {127}.............. --T   XBM {154}................. --b */
/* RIX {128}................... -R   XPM {156}................. --x */
/* Scodl {129}................ --s   XWD {159}................. --w */
/* SGI Image {131}............. -n                                  */
/*                                                                  */
/* alchemy pages.gif -U ---U bob.pdf --d                            */
/*                                                                  */
/* john.rle                                                         */
/* 00000: 52cc0000 00000001 00010903 08000800  RL..............     */
/* john.ico                                                         */
/* 00000: 43494e00 00008000 8000a400 00004000  CIN.......$...@.     */
/* john.ras                                                         */
/* 00000: 59a66a95 00000100 00000100 00000018  Y&j.............     */
/* john.eps                                                         */
/* 00000: c5d0d3c6 d4000100 fd140600 00000000  EPSFT...........     */
/* john.mac                                                         */
/* 00000: 00000002 ffffffff ffffffff 00000000  ................     */
/* john.xbm                                                         */
/* 00000: 23646566 696e6520 6a6f686e 5f776964  #define john_wid     */
/* pages.pdf   													    */
/* 00000: 25504446 2d312e30 0d0a25ae 999499af  %PDF-1.0..%..../     */
/* img0001.pcd   												    */
/* 00000: ffffffff ffffffff ffffffff ffffffff  ................     */
/* 00010: ffffffff ffffffff ffffffff ffffffff  ................     */
/* 00020: 07070707 00000000 00000000 57575757  ............WWWW     */
/* 00030: 05050505 00000000 00000000 0a0a0a0a  ................     */
/* 00040: 00000000 00000000 09090909 03030303  ................     */
/* 00050: 16161616 09090909 1c1c1c1c 00000000  ................     */
/* 00060: 00000000 01010101 00000000 00000000  ................     */
/* 00070: 00000000 00000000 00000000 00000000  ................     */
/*                                                                  */
/*------------------------------------------------------------------*/
/* END ABSTRACT */

#if defined(_MSC_VER)
#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "gimex.h"
#include "exegimex.h"
#include "bmpgimex.h"

#define EXETYPES 7
#define EXE_RAS 0
#define EXE_RLE 1
#define EXE_EPS 2
#define EXE_MAC 3
#define EXE_XBM 4
#define EXE_PDF 5
#define EXE_PCD 6

static int EXEsubtype=0;

static struct
{
    const char *shorttypestr;
    const char *longtypestr;
    int canimport;
} EXEtypetbl[EXETYPES]=
{
    "ras","Sun Raster",1,
    "rle","Utah RLE",1,
    "eps","Encapsulated Postscript",0,
    "mac","MacPaint",1,
    "xbm","X Bitmap",0,
    "pdf","Adobe PDF",0,
    "pcd","PhotoCD",0,
};

static int GCALL EXE_type(GSTREAM *g)
{
    char buf[128];
    int ok=0;
    int i=-1;
    gseek(g, 0);
    ok = gread(g,buf,128);
    if (ok)
    {
        if (ggetm(buf,2)==0x52cc)
            i = EXE_RLE;
//        else if (ggetm(buf,4)==0x43494e00)
//            i = EXE_ICO;
        else if (ggetm(buf,4)==0x59a66a95)
            i = EXE_RAS;
        else if (ggetm(buf,4)==0xc5d0d3c6)
            i = EXE_EPS;
        else if (ggetm(buf,4)==0x25504446)  // %PDF
            i = EXE_PDF;
        else if ((ggetm(buf,4)==0x00000002)
              && (ggetm(buf+4,4)==0xffffffff)
              && (ggetm(buf+4,4)==0xffffffff)
              && (ggetm(buf+4,4)==0x00000000))
            i = EXE_MAC;
        else if (ggetm(buf,4)==0x23646566)
        {
            int j;
            for (j=0; j<32-4; ++j)
            {
                if (ggetm(buf+j,4)==0x5f776964) // _wid
                {
                    i = EXE_XBM;
                    break;
                }
            }
        }
        else if (ggetm(buf,4)==0xffffffff && 
  			    (ggetm(buf+0x48,4)==0x08080808 || ggetm(buf+0x48,4)==0x09090909)) // orientation is 0x08 or 0x09
        {
            i = EXE_PCD;
        }
    }
    return(i);
}

/****************************************************************/
/*  Information Functions                                       */
/****************************************************************/

GABOUT * GCALL EXE_about(void)
{
    GABOUT *info;
    int i;

    info = (GABOUT *) galloc(sizeof(GABOUT));
    if (info)
    {
        memset(info, 0, sizeof(GABOUT));

        info->signature       = GMAKEID('.','E','X','E');
        info->size            = sizeof(GABOUT);
        info->version         = GIMEX_VERSION;
#if defined(_MSC_VER)
        info->canimport       = 1;    /* supports importing */
        info->canexport       = 1;    /* supports exporting */
        info->importpacked    = 1;    /* supports importing packed */
        info->exportpacked    = 1;    /* supports exporting packed */
        info->import8         = 0;    /* supports importing 8 bit indexed */
        info->export8         = 0;    /* supports exporting 8 bit indexed */
        info->import32        = 1;    /* supports importing 32 bit direct rgb */
        info->export32        = 1;    /* supports exporting 32 bit direct rgb */
#endif
        info->multiframe      = 0;    /* supports multiple frames */
        info->multifile       = 0;    /* format requires additional files or resource fork */
        info->multisize       = 1;    /* supports each frame different size */
        info->framebuffer     = 1;    /* module requires memory to buffer entire frame */
        info->external        = 1;    /* module uses external resources */
        info->dotsubtype      = 1;    /* subtype based on extension */
        info->usesfile        = 1;    /* module is file based */
        info->maxframename    = 0;    /* max frame name character */
        info->defaultquality  = 75;   /* default pack quality */
        info->mactype[0]      = GMAKEID( '.','E','X','E' );   /* mac file system types used */
        for (i=0; i<EXETYPES; ++i)
            sprintf(info->extensions[i], ".%s", EXEtypetbl[i].shorttypestr);    /* extensions */
        strcpy(info->authorstr,     "Frank Barchard");      /* author */
        strcpy(info->versionstr,    "1.08");                /* version # */

        strcpy(info->shorttypestr,  EXEtypetbl[EXEsubtype].shorttypestr); /* type */
        strcpy(info->wordtypestr,   EXEtypetbl[EXEsubtype].shorttypestr);                 /* wordtype */
        strcpy(info->longtypestr,   EXEtypetbl[EXEsubtype].longtypestr);            /* longtype */

        {
            int canimport = EXEtypetbl[EXEsubtype].canimport;

            info->canimport       = canimport;    /* supports importing */
            info->importpacked    = canimport;    /* supports importing packed */
            info->import32        = canimport;    /* supports importing 32 bit direct rgb */
        }

#if defined(_MSC_VER)
        {
            static int plugintested = 0;

            if (!plugintested)
            {
                plugintested = 1;
                if (!SearchPath(NULL,"alchemy.exe",NULL,0,NULL,NULL))
                {
                    plugintested = -1;
                }
            }
            if (plugintested<0)
            {
                info->canimport       = 0;    /* supports importing */
                info->importpacked    = 0;    /* supports importing packed */
                info->import8         = 0;    /* supports importing 8 bit indexed */
                info->import32        = 0;    /* supports importing 32 bit direct rgb */
                info->canexport       = 0;    /* supports exporting */
                info->exportpacked    = 0;    /* supports exporting packed */
                info->export8         = 0;    /* supports exporting 8 bit indexed */
                info->export32        = 0;    /* supports exporting 32 bit direct rgb */
            }
        }
#endif

    }
    return(info);
}

/* various signatures */
// RAS: 59a66a95
// RLE: 52cc
// ICO: 43494e00
// EPS: c5d0d3c6
// PDF: 25504446
// MAC: 00000002 ffffffff ffffffff 00000000
// XBM: 23646566 696e6520 6a6f686e 5f776964  #define john_wid
// PCD: 0X48 08080808 or 09090909 (orientation)

int GCALL EXE_is(GSTREAM *g)
{
    int ok=0;
    int type=0;
    type = EXE_type(g);
    if (type>=0)
    {
        EXEsubtype=type;
        ok = 80;
    }
    return(ok);
}

/****************************************************************/
/*  Import Functions                                            */
/****************************************************************/

int GCALL EXE_open(GINSTANCE **gxptr, GSTREAM *g, const char *pathname,bool framecountflag)
{
    int ok=0;
#if defined(_MSC_VER)
    char sysline[512];
    FILE *stream;

    EXEsubtype = EXE_type(g); // set global import subtype
    if (EXEsubtype>=0)
    {
        sprintf(sysline,"%s.err",pathname);
        stream = freopen(sysline, "w", stderr);

        sprintf(sysline,"alchemy %s -w -o gxtemp.bmp>%s.lst",pathname,pathname);
        system(sysline);

        stream = freopen("con:", "w", stderr);

        sprintf(sysline,"gxtemp.bmp");

        g = gopen(sysline);
        if (g)
        {
            ok = BMP_open(gxptr,g,sysline,0);
            if (ok)
                (*gxptr)->gref = (void *) pathname;
        }
    }
#endif
    return(ok);
}

GINFO * GCALL EXE_info(GINSTANCE *gx, int framenum)
{
    GINFO *ginfo=0;

#if defined(_MSC_VER)
    const char *pathname;
    GSTREAM *g;

    ginfo = BMP_info(gx,framenum);
    ginfo->packed = 1;

    pathname = (const char *) gx->gref;
    g = gopen(pathname);
    if (g)
    {
        ginfo->quality = 101-((ginfo->width*ginfo->height*3)/(int)glen(g));
        ginfo->signature = GMAKEID('.','E','X','E');
        ginfo->subtype = EXE_type(g);
        gclose(g);
    }
#endif
    return(ginfo);
}

int GCALL EXE_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes)
{
    int ok=0;

#if defined(_MSC_VER)
    int packed=ginfo->packed;
    int subtype=ginfo->subtype;

    ginfo->packed = 0;
    ginfo->subtype = 0;

    ok = BMP_read(gx,ginfo,dest,rowbytes);

    ginfo->packed = packed;
    ginfo->subtype = subtype;
#endif
    return(ok);
}

int GCALL EXE_close(GINSTANCE *gx)
{
    int ok=0;

#if defined(_MSC_VER)
    if (gx)
    {

        if (gx->gstream)
        {
            gclose(gx->gstream);
            gx->gstream = NULL;
        }
        ok = gfree((void*)gx);

        {
            const char *pathname;
            char sysline[512];

            pathname = (const char *) gx->gref;

            sprintf(sysline,"%s.err",pathname);
            remove(sysline);
            sprintf(sysline,"%s.lst",pathname);
            remove(sysline);
            sprintf(sysline,"gxtemp.bmp");
            remove(sysline);
        }
    }
#endif
    return(ok);
}


