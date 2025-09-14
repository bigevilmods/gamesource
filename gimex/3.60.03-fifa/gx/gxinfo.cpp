// (c) 2003-2004 Electronic Arts Inc.

#include "gxinfo.h"

// extended ginfo type for PSD files to tell if a layer is invisible or not.

struct GINFOPSD
{
    GINFO ginfo;
    int blendmethod;
    int visibleflag;
    int opacitylevel;

// PAULDING START
    int canvasw;
    int canvash;
// PAULDING END
};

//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// bit depth conversion function for GSH.

static unsigned short GINFO_GSHWpixel8To5(const ARGB *aRgb8)
{
    unsigned int a;
    unsigned int r;
    unsigned int g;
    unsigned int b;
    unsigned int argb;

    if (aRgb8->a<0xe0) // 3444 - only 3 bits of alpha significant
    {
        a = ((int)aRgb8->a*7+128)/255;
        r = ((int)aRgb8->r*15+128)/255;
        g = ((int)aRgb8->g*15+128)/255;
        b = ((int)aRgb8->b*15+128)/255;
        argb = (0<<15)|(a<<12)|(r<<8)|(g<<4)|b;
    }
    else    // 555
    {
        r = (int)aRgb8->r>>(8-5);
        g = (int)aRgb8->g>>(8-5);
        b = (int)aRgb8->b>>(8-5);

        argb = (1<<15)|(r<<10)|(g<<5)|b;
    }
    return((unsigned short) argb);
}


static char *GXid2str(char *buf, unsigned int id)
{
    int i;
    int n;

    for (i=0,n=0; i<4; ++i)
        if ((id>>(24-i*8))&0xff)
            buf[n++] = (char) (id>>(24-i*8));
    buf[n] = 0;
    return(buf);
}

static void padprint(int x,char *sep,int y,char *trail,int len)
{
    char buf[512];

    if (trail)
    {
        sprintf(buf,"%d%s%d%s",x,sep,y,trail);
        printf("%*s",-len,buf);
    }
    else
    {
        sprintf(buf,"%d%s",x,sep);
        printf("%*s",len,buf);
    }
}

#if 0
/*
Example Text Palette

Colour #       ARGB Hex   ARGB 8            RGB 6         RGB 555    PSX
------------   --------   ---------------   -----------   --------   ----
00   0 ($00) - ffff0000   255 255   0   0    63   0   0   31  0  0   $001f
     1 ($01) - ffc6c6c6   255 198 198 198    49  49  49   24 24 24   $6318
     2 ($02) - ffffce00   255 255 206   0    63  51   0   31 25  0   $033f
     3 ($03) - ffff9c00   255 255 156   0    63  39   0   31 19  0   $027f
     4 ($04) - ffff6300   255 255  99   0    63  24   0   31 12  0   $019f
     5 ($05) - ffffff00   255 255 255   0    63  63   0   31 31  0   $03ff
     6 ($06) - ffff9c9c   255 255 156 156    63  39  39   31 19 19   $4e7f
     7 ($07) - ffffff9c   255 255 255 156    63  63  39   31 31 19   $4fff
     8 ($08) - ffff3100   255 255  49   0    63  12   0   31  6  0   $00df
     9 ($09) - ffff9c31   255 255 156  49    63  39  12   31 19  6   $1a7f
    10 ($0a) - ffff6331   255 255  99  49    63  24  12   31 12  6   $199f
    11 ($0b) - ffffff31   255 255 255  49    63  63  12   31 31  6   $1bff
    12 ($0c) - ffefefef   255 239 239 239    59  59  59   29 29 29   $77bd
    13 ($0d) - ffffcece   255 255 206 206    63  51  51   31 25 25   $673f
    14 ($0e) - ffffce31   255 255 206  49    63  51  12   31 25  6   $1b3f
    15 ($0f) - ffdedede   255 222 222 222    55  55  55   27 27 27   $6f7b

01  16 ($10) - ffffce9c   255 255 206 156    63  51  39   31 25 19   $4f3f
    17 ($11) - ffff9c63   255 255 156  99    63  39  24   31 19 12   $327f
    18 ($12) - ffff6363   255 255  99  99    63  24  24   31 12 12   $319f
    19 ($13) - ffff3131   255 255  49  49    63  12  12   31  6  6   $18df
    20 ($14) - ffffff63   255 255 255  99    63  63  24   31 31 12   $33ff
    21 ($15) - ffffffce   255 255 255 206    63  63  51   31 31 25   $67ff
    22 ($16) - ff000000   255   0   0   0     0   0   0    0  0  0   $0000
    23 ($17) - ff000000   255   0   0   0     0   0   0    0  0  0   $0000
    24 ($18) - ff000000   255   0   0   0     0   0   0    0  0  0   $0000
    25 ($19) - ff000000   255   0   0   0     0   0   0    0  0  0   $0000
    26 ($1a) - ff000000   255   0   0   0     0   0   0    0  0  0   $0000
    27 ($1b) - ff000000   255   0   0   0     0   0   0    0  0  0   $0000
    28 ($1c) - ff000000   255   0   0   0     0   0   0    0  0  0   $0000
    29 ($1d) - ff000000   255   0   0   0     0   0   0    0  0  0   $0000
    30 ($1e) - ff223344   255  34  51  68     8  12  17    4  6  8   $20c4
    31 ($1f) - ff000000   255   0   0   0     0   0   0    0  0  0   $0000
*/
#endif

/* convert 32 bit ARGB entry to PSX 16 bit entry */
/* with transparency and tranluciency */

static unsigned short GINFO_pixel8To5(ARGB *aRgb8)
{
    unsigned int a;
    unsigned int r;
    unsigned int g;
    unsigned int b;
    unsigned int aRgb5;

    a = aRgb8->a;
    r = ((int)aRgb8->r*31+128)/255;
    g = ((int)aRgb8->g*31+128)/255;
    b = ((int)aRgb8->b*31+128)/255;

    aRgb5 = (b<<10)|(g<<5)|r;

    if (a<25*255/100)           /* transparent */
    {
        aRgb5 = 0;
    }
    else if (a>=75*255/100)     /* opaque */
    {
        if (!aRgb5)
            aRgb5 = (1<<10);    /* black remap to blue */
    }
    else                        /* translucient */
    {
        aRgb5 |= 0x8000;
    }
    return((unsigned short) aRgb5);
}



int GINFO_palettehistogram(char *f,GINFO *ginfo,int *histogram)
{
    int i;
    int ok=0;

    if (ginfo->bpp<=8)
    {
        for (i=0; i<ginfo->width*ginfo->height; ++i)
        {
            ++histogram[(int) *(unsigned char *)f];
            ++f;
        }
        ok = 1;
        for (i=ginfo->numcolours; i<256; ++i)
        {
            if (histogram[i])
            {
                ok = i;    // ohoh.. colours outside the palette
                break;
            }
        }
    }
    return(ok);
}



void GINFO_showabout(GABOUT *about)
{
    int j;
    char idbuf[8];

    if (OPTverbose==2)
    {
        printf("Format Version:    %s\n",about->versionstr);
        printf("gimex.dll Version: %.2f\n",(float)about->version/100.0f);
        printf("Short Name:    %s\n",about->shorttypestr);
        printf("Long Name:     %s\n",about->longtypestr);
        printf("Extensions:    ");
        for (j=0; j<MAXEXTENSIONS; ++j)
            if (about->extensions[j][0])
                printf("%s",about->extensions[j]);
        printf("\n");
        printf("Features:      ");

        if ((about->import8 || about->export8) && (about->import32 || about->export32))
            gprintf("8/32 ");
        else if ((about->import8 || about->export8))
            gprintf("8 ");
        else if ((about->import32 || about->export32))
            gprintf("32 ");
        else
            gprintf("- ");
        if (about->canimport && about->canexport)
            gprintf("import/export ");
        else if (about->canimport)
            gprintf("import only ");
        else if (about->canexport)
            gprintf("export only ");
        else
            gprintf(" ");

        if (about->multiframe)
        {
            if (about->movie)
                gprintf("movie");
            else if (about->font)
                gprintf("font");
            else if (about->mipmap)
                gprintf("texture");
            else
                gprintf("pages");
        }
        else
            gprintf("image");

        if (about->file64)
            gprintf("64");

        gprintf("\n");
    }
    else if (OPTverbose>=3)
    {
        printf("ABOUT %s\n",about->longtypestr);
        printf("version          %.2f\n",(float)about->version/100.0f);
        printf("canimport    %8d        canexport    %8d\n",about->canimport,about->canexport);
        printf("importpacked %8d        exportpacked %8d\n",about->importpacked,about->exportpacked);
        printf("import8      %8d        export8      %8d\n",about->import8,about->export8);
        printf("import32     %8d        export32     %8d\n",about->import32,about->export32);
        printf("multiframe   %8d        multifile    %8d\n",about->multiframe,about->multifile);
        printf("framebuffer  %8d        external     %8d\n",about->framebuffer,about->external);
        for (j=0; j<MAXMACTYPES; ++j)
            if (about->mactype[j])
                printf("mactype[%d]       %s\n",j,GXid2str(idbuf,about->mactype[j]));
        for (j=0; j<MAXEXTENSIONS; ++j)
            if (about->extensions[j][0])
                printf("extensions[%d]    %s\n",j,about->extensions[j]);
        printf("authorstr        %s\n",about->authorstr);
        printf("versionstr       %s\n",about->versionstr);
        printf("shorttypestr     %s\n",about->shorttypestr);
        printf("wordtypestr      %s\n",about->wordtypestr);
        printf("longtypestr      %s\n",about->longtypestr);
    }
}


int GINFO_showpalette(GINFO *ginfo, int *histogram, bool outsidecolours, const char *OPTpalrange, int verbose)
{
    int i,j=0;
    int ok=1;
    char textpal[512];
    unsigned int rgb16;
    RANGER *r=0;
    EXPR *expr=0;
    char palrange[SPITMAXLINE];
    double fnext;
    int numcolours = ginfo->numcolours;
    if (verbose>=3 && outsidecolours)
        numcolours = 256;

	int histogramzero[256]={ 0,};
	if (!histogram)
		histogram = histogramzero;

    /* header */

    if (OPTpalrange[0])
    {
        strcpy(palrange,OPTpalrange);
    }
    else
    {
        sprintf(palrange,"0..%d",numcolours-1);
    }

    ok = 0;
    expr = EXPR_open();
    if (expr)
    {
        double n=(double)numcolours;
        double rmin;
        EXPR_set(expr,"len",n);
        EXPR_set(expr,"colours",n);
        EXPR_set(expr,"colors",n);
        EXPR_set(expr,"numcolours",n);
        EXPR_set(expr,"numcolors",n);
        EXPR_set(expr,"n",n);
        EXPR_set(expr,"last",n-1);

        EXPR_set(expr,"bpp",(double)ginfo->originalbpp);
        EXPR_set(expr,"startcolour",(double)ginfo->startcolour);
        EXPR_set(expr,"framenum",(double)ginfo->framenum);

        r = RANGER_open(expr, palrange, 0.0, n-1);
        if (r)
        {
            ok = RANGER_info(r,&rmin,NULL,NULL);
            if (rmin<0.0)
                ok = 0;
        }
    }

    if (ok)
    {
        if (OPTverbose>=4)
        {
            printf("Colour #       ARGB Hex   ARGB 8             RGB 6      RGB 555     5A3  USAGE\n");
            printf("------------   --------   ---------------    --------   --------    ---- -----\n");
        }
        else if (OPTverbose>=2)
        {
            printf("Colour #       ARGB Hex   ARGB 8             Usage\n");
            printf("------------   --------   ---------------    -----\n");
        }

        j = -1;
        while (RANGER_next(r,&fnext))
        {
            i = (int) fnext;
            if (OPTverbose==1)
            {
                if (j>=0)
                    printf(",\n");

                sprintf(textpal,"%d,%d,%d,%d",
                    ginfo->colourtbl[i].a,ginfo->colourtbl[i].r,ginfo->colourtbl[i].g,ginfo->colourtbl[i].b);
            }
            else
            {
                if (j<0) j = i;
                if (i/16!=j/16)
                    printf("\n");

                if (i&15)   sprintf(textpal,"   ");
                else        sprintf(textpal,"%02d ",i/16);

                printf("%s",textpal);

                if (OPTverbose>=4)
                {
//                    if (ginfo->signature==GMAKEID('S','H','P','G'))
                        rgb16 = GINFO_GSHWpixel8To5(ginfo->colourtbl+i);
//                    else
//                        rgb16 = GINFO_pixel8To5(ginfo->colourtbl+i);

                    sprintf(textpal,"%3d ($%02x) - %02x%02x%02x%02x   %3d %3d %3d %3d    %2d %2d %2d   %2d %2d %2d   $%04x %d\n",
                        i,i,
                        ginfo->colourtbl[i].a,ginfo->colourtbl[i].r,ginfo->colourtbl[i].g,ginfo->colourtbl[i].b,
                        ginfo->colourtbl[i].a,ginfo->colourtbl[i].r,ginfo->colourtbl[i].g,ginfo->colourtbl[i].b,
                        ginfo->colourtbl[i].r>>2,ginfo->colourtbl[i].g>>2,ginfo->colourtbl[i].b>>2,
                        ginfo->colourtbl[i].r>>3,ginfo->colourtbl[i].g>>3,ginfo->colourtbl[i].b>>3,
                        rgb16,histogram[i]);
                }
                else
                {
                    sprintf(textpal,"%3d ($%02x) - %02x%02x%02x%02x   %3d %3d %3d %3d    %d\n",
                        i,i,
                        ginfo->colourtbl[i].a,ginfo->colourtbl[i].r,ginfo->colourtbl[i].g,ginfo->colourtbl[i].b,
                        ginfo->colourtbl[i].a,ginfo->colourtbl[i].r,ginfo->colourtbl[i].g,ginfo->colourtbl[i].b,
                        histogram[i]);
                }
            }

            j = i;
            printf("%s",textpal);
        }
    }

    if (r)
        RANGER_close(r);
    if (expr)
        EXPR_close(expr);

    return(ok);
}

#define CENTERVIEW 7
#define COMMENTVIEW 14

void showframeinfo(GINFO *ginfo,int *histogram, bool outsidecolours, int OPTcenter, int OPTcomment, int verbose)
{
    int j;
    char idbuf[8];
    int shapesize;
    int numhotspots=gmin(GIMEX_HOTSPOTTBL_SIZE,ginfo->numhotspots);
    int bpp;
    char bpc[512];
    char compressionbuf[8];
    char *compressionstr="";
    char subtypebuf[512];
    char *subtypestr="";
    GINFOPSD * ginfopsd = (GINFOPSD *) ginfo;


    bpp = ginfo->originalbpp;
    if (bpp == 15)
        bpp = 16;
    shapesize = ginfo->framesize;
    if (!shapesize)
        shapesize = (ginfo->width*bpp+7)/8*ginfo->height;

    if (ginfo->alphabits)
    {
        sprintf(bpc,"%d%d%d%d",ginfo->alphabits,ginfo->redbits,ginfo->greenbits,ginfo->bluebits);
    }
    else
    {
        sprintf(bpc,"%d%d%d",ginfo->redbits,ginfo->greenbits,ginfo->bluebits);
    }

    if (ginfo->packed==0)
    {
        if (OPTverbose>=3)
            compressionstr = "";//"Unpacked";
        else
            compressionstr = "";
    }
    else
    {
        switch (ginfo->signature&0xffffff00)
        {
            case GMAKEID('S','H','P',0):
            case GMAKEID('S','S','F',0):
            case GMAKEID('S','h','p',0):
            case GMAKEID('S','s','f',0):
            case GMAKEID('.','D','D','S')&0xffffff00:
            case GMAKEID('.','X','P','R')&0xffffff00:
            case GMAKEID('.','3','P','R')&0xffffff00:
            case GMAKEID('.','G','T','F')&0xffffff00:

                switch (ginfo->packed)
                {
                    case 2: compressionstr = "Refpack"; break;
                    case 3:
                        if (ginfo->alphabits>=4)
                            compressionstr = "DXT3";
                        else if(ginfo->alphabits == 3)
                            compressionstr = "DXT5";
                        else
                            compressionstr = "DXT1";
                        break;
                    case 4: compressionstr = "MPeg"; break;

                    default:
                        if (ginfo->packed>=0x20000000)
                        {
                            compressionbuf[0] = (char)('\'');
                            compressionbuf[1] = (char)((ginfo->packed>>24)&255);
                            compressionbuf[2] = (char)((ginfo->packed>>16)&255);
                            compressionbuf[3] = (char)((ginfo->packed>>8)&255);
                            compressionbuf[4] = (char)((ginfo->packed)&255);
                            compressionbuf[5] = (char)('\'');
                            compressionbuf[6] = (char)(0);
                            compressionstr = compressionbuf;
                        }
                        else
                            compressionstr = "Packed";
                        break;
                }
                break;

            case GMAKEID('.','A','V','I')&0xffffff00:
                switch (ginfo->packed)
                {
                    case 1: compressionstr = "RLE"; break;
                    default:
                        if (ginfo->packed>=0x20000000)
                        {
                            compressionbuf[0] = (char)('\'');
                            compressionbuf[1] = (char)((ginfo->packed>>24)&255);
                            compressionbuf[2] = (char)((ginfo->packed>>16)&255);
                            compressionbuf[3] = (char)((ginfo->packed>>8)&255);
                            compressionbuf[4] = (char)((ginfo->packed)&255);
                            compressionbuf[5] = (char)('\'');
                            compressionbuf[6] = (char)(0);
                            compressionstr = compressionbuf;
                        }
                        else
                            compressionstr = "Packed";
                        break;
                }
                break;

            case GMAKEID('.','G','I','F')&0xffffff00:
            case GMAKEID('.','P','N','G')&0xffffff00:
            case GMAKEID('.','V','A','N')&0xffffff00:
                compressionstr = "LZ";
                break;

            case GMAKEID('G','I','M','X')&0xffffff00:
                compressionstr = "Huff";
                break;

            case GMAKEID('S','3','T','C')&0xffffff00:
                if (ginfo->alphabits>=4)
                    compressionstr = "DXT3";
                else if(ginfo->alphabits ==3)
                    compressionstr = "DXT5";
                else
                    compressionstr = "DXT1";
                break;

            case GMAKEID('.','V','Q','T')&0xffffff00:
                compressionstr = "VQ";
                break;

            case GMAKEID('.','M','V','I')&0xffffff00:
            case GMAKEID('M','D','E','C')&0xffffff00:
            case GMAKEID('0','0','U','V')&0xffffff00:
            case GMAKEID('T','G','V','i')&0xffffff00:
            case GMAKEID('.','D','C','T')&0xffffff00:
            case GMAKEID('C','B','R','A')&0xffffff00:
            case GMAKEID('.','S','T','R')&0xffffff00:
            case GMAKEID('M','O','O','V')&0xffffff00:
            case GMAKEID('Q','T','3',' ')&0xffffff00:
                compressionstr = "Video";
                break;

            case GMAKEID('M','A','D','C')&0xffffff00:
                switch (ginfo->packed)
                {
                    case 1: compressionstr = "Madcow"; break;
                    default:
                        if (ginfo->packed>=0x20000000)
                        {
                            compressionbuf[0] = (char)('\'');
                            compressionbuf[1] = (char)((ginfo->packed>>24)&255);
                            compressionbuf[2] = (char)((ginfo->packed>>16)&255);
                            compressionbuf[3] = (char)((ginfo->packed>>8)&255);
                            compressionbuf[4] = (char)((ginfo->packed)&255);
                            compressionbuf[5] = (char)('\'');
                            compressionbuf[6] = (char)(0);
                            compressionstr = compressionbuf;
                        }
                        else
                            compressionstr = "Madcow";
                        break;
                }
                break;

            case GMAKEID('M','P','E','G')&0xffffff00:
            case GMAKEID('G','R','I','F')&0xffffff00:
            case GMAKEID('.','I','P','U')&0xffffff00:
                compressionstr = "MPeg";
//                if (OPTverbose<=2)
                {
                    switch (ginfo->subtype>>6)
                    {
                        default:
                        case 0: compressionstr = "MPeg";   break;
                        case 1: compressionstr = "MPeg I"; break;
                        case 2: compressionstr = "MPeg P"; break;
                        case 3: compressionstr = "MPeg B"; break;
                    }
                }
                break;

            case GMAKEID('.','S','C','T')&0xffffff00:
            case GMAKEID('.','T','G','A')&0xffffff00:
            case GMAKEID('.','Q','T','3')&0xffffff00:
            case GMAKEID('A','L','I','S')&0xffffff00:
            case GMAKEID('.','P','C','X')&0xffffff00:
            case GMAKEID('.','R','L','A')&0xffffff00:
            case GMAKEID('.','S','G','I')&0xffffff00:
            case GMAKEID('.','S','I','M')&0xffffff00:
            case GMAKEID('.','A','N','M')&0xffffff00:
            case GMAKEID('P','I','C','T')&0xffffff00:
            case GMAKEID('I','L','B','M')&0xffffff00:
            case GMAKEID('.','B','M','P')&0xffffff00:
                compressionstr = "RLE";
                break;

            case GMAKEID('J','P','E','G')&0xffffff00:
            case GMAKEID('I','J','P','G')&0xffffff00:
            case GMAKEID('.','J','P','2')&0xffffff00:
                compressionstr = "JPeg";
                break;
       }
    }

    subtypestr = subtypebuf;
    subtypebuf[0] = 0;

    switch (ginfo->signature&0xffffff00)
    {
        case GMAKEID('S','H','P',0):
        case GMAKEID('S','S','F',0):
        case GMAKEID('S','h','p',0):
        case GMAKEID('S','s','f',0):
        case GMAKEID('.','X','P',0):
        case GMAKEID('.','3','P',0):
            if (ginfo->subtype&      0x1) strcat(subtypebuf,"nopal ");
            if (ginfo->subtype&      0x2) strcat(subtypebuf,"noshp ");
            if (ginfo->subtype&      0x4) strcat(subtypebuf,"nohot ");
            if (ginfo->subtype&      0x8) strcat(subtypebuf,"nocomment ");
            if (ginfo->subtype&     0x10) strcat(subtypebuf,"noanim ");
            if (ginfo->subtype&     0x20) strcat(subtypebuf,"notpage ");
            if (ginfo->subtype&     0x40) strcat(subtypebuf,"reserved64? ");
            if (ginfo->subtype&     0x80) strcat(subtypebuf,"reserved128? ");
            if (ginfo->subtype&    0x100) strcat(subtypebuf,"namedpal ");
            if (ginfo->subtype&    0x200) strcat(subtypebuf,"nocollapse ");
            if (ginfo->subtype&    0x400) strcat(subtypebuf,"nosort ");
            if (ginfo->subtype&    0x800) strcat(subtypebuf,"rotated ");
            if (ginfo->subtype&   0x1000) strcat(subtypebuf,"transposed ");
            if (ginfo->subtype&   0x2000) strcat(subtypebuf,"swizzled ");
            if (ginfo->subtype&   0x4000) strcat(subtypebuf,"csm2 ");
            if (ginfo->subtype&   0x8000) strcat(subtypebuf,"noname ");
            if (ginfo->subtype&  0x10000) strcat(subtypebuf,"dot3 ");
            if (ginfo->subtype&  0x20000) strcat(subtypebuf,"embm ");
            if (ginfo->subtype&  0x40000) strcat(subtypebuf,"cubemap ");
            if (ginfo->subtype&  0x80000) strcat(subtypebuf,"mipmont ");
            if (ginfo->subtype& 0x100000) strcat(subtypebuf,"nodxtcollapse ");
            if (ginfo->subtype& 0x200000) strcat(subtypebuf,"metalbin ");
            if (ginfo->subtype&0x1000000) strcat(subtypebuf,"opaque ");
            if (ginfo->subtype&0x2000000) strcat(subtypebuf,"transparent ");
            if (ginfo->subtype&0x4000000) strcat(subtypebuf,"halfalpha ");
            break;

        case GMAKEID('F','N','T',0):
        case GMAKEID('F','n','t',0):
            if (ginfo->subtype&     0x1) strcat(subtypebuf,"antialias ");
            if (ginfo->subtype&     0x2) strcat(subtypebuf,"dropshadow ");
            if (ginfo->subtype&     0x4) strcat(subtypebuf,"outline ");
            if (ginfo->subtype&     0x8) strcat(subtypebuf,"spaced ");
            if (ginfo->subtype& 0x20000) strcat(subtypebuf,"jis ");
            if (ginfo->subtype& 0x40000) strcat(subtypebuf,"char16 ");
            if (ginfo->subtype& 0x80000) strcat(subtypebuf,"powerof2 ");
            if (ginfo->subtype&0x100000) strcat(subtypebuf,"maxwidth256 ");
            break;


        case GMAKEID('.','B','M','P')&0xffffff00:
            if (ginfo->subtype==2) subtypestr = "Win95";
            else if (ginfo->subtype==3) subtypestr = "WinNT";
            else if (ginfo->subtype==4) subtypestr = "Win3.1+565";
            break;

        case GMAKEID('I','L','B','M')&0xffffff00:
            if (ginfo->subtype==1) subtypestr = "pbm";
            break;

        case GMAKEID('G','I','M','X')&0xffffff00:
        case GMAKEID('M','P','E','G')&0xffffff00:
            if (ginfo->subtype&0x00ffff00)
                sprintf(subtypebuf,"gop%d,%d",(ginfo->subtype>>16)&255,(ginfo->subtype>>8)&255);
            break;

        case GMAKEID('M','D','E','C')&0xffffff00:
            if (ginfo->subtype&    1) strcat(subtypebuf,"xa ");
            if (ginfo->subtype&    2) strcat(subtypebuf,"ref ");
            if (ginfo->subtype&    4) strcat(subtypebuf,"dct ");
            if (ginfo->subtype&    8) strcat(subtypebuf,"alpha ");
            break;

        case GMAKEID('G','R','I','F')&0xffffff00:
            if (ginfo->subtype==1) subtypestr = "idec";
            break;
        case GMAKEID('8','B','P','S')&0xffffff00:
            if(ginfo->size == sizeof(GINFOPSD))
                if(!ginfopsd->visibleflag)
                    subtypestr = "invisible";
            break;
        default:
            break;
    }

    switch (verbose+OPTcenter*CENTERVIEW+OPTcomment*COMMENTVIEW)
    {
        case 0:
            break;

        case 1:
        case 2:

            if (verbose>=2)
            {
                char framebuf[512];

                if (ginfo->framename[0])
                    sprintf(framebuf," #%d '%-s':",ginfo->framenum,ginfo->framename);
                else
                    sprintf(framebuf," #%d:",ginfo->framenum);
                printf("%-12s",framebuf);
            }
            {
                char textbuf[512];

                if (ginfo->originalbpp<=8)
                {
                    sprintf(textbuf,"%dx%dx%d",ginfo->width,ginfo->height,ginfo->originalbpp);
                    printf("%-11s",textbuf);
                    sprintf(textbuf," (%s clut),",bpc);
                    printf("%-13s",textbuf);
                }
                else
                {
                    sprintf(textbuf,"%dx%dx%s,",ginfo->width,ginfo->height,bpc);
                    printf("%-24s",textbuf);
                }

            }

            printf("Position ");
            padprint(ginfo->defaultx,",",ginfo->defaulty,",",9);
            if (verbose>=2)
            {
                printf("Size ");
                padprint(shapesize,", ",0,(char *)0,9);
            }
            printf("%s\n",compressionstr);
            break;

        case 3:

            if (ginfo->framename[0])
                printf(" Frame #%d '%s':\n",ginfo->framenum,ginfo->framename);
            else
                printf(" Frame #%d:\n",ginfo->framenum);

            printf("  width:     %8d         defaultx:  %8d\n",ginfo->width,ginfo->defaultx);
            printf("  height:    %8d         defaulty:  %8d\n",ginfo->height,ginfo->defaulty);
            if (ginfo->subtype<=8)
                printf("  bpp:       %8d         subtype:   %8d %s\n",ginfo->originalbpp,ginfo->subtype,subtypestr);
            else
            {
                char subnumstr[SPITMAXLINE];
                sprintf(subnumstr,"0x%x",ginfo->subtype);
                printf("  bpp:       %8d         subtype: %10s %s\n",ginfo->originalbpp,subnumstr,subtypestr);
            }

            if (ginfo->numcolours || ginfo->bpp<=8)
                printf("  colours:   %8d         ",ginfo->numcolours);
            else
                printf("  colours:          -         ");
            if (ginfo->packed)
                printf("packed:  %10d %s\n",ginfo->packed,compressionstr);
            else
                printf("packed:           -\n");

            if (ginfo->alphabits)
                printf("  bits/channel:  %d%d%d%d         ",ginfo->alphabits,ginfo->redbits,ginfo->greenbits,ginfo->bluebits);
            else
                printf("  bits/channel:   %d%d%d         ",ginfo->redbits,ginfo->greenbits,ginfo->bluebits);

            if (!ginfo->packed || (ginfo->packed==1 && ginfo->quality==100))
                printf("quality:          -\n");
            else if (ginfo->framesize && ginfo->quality && ginfo->quality!=100)
                printf("framesize: %8d (q%d)\n",ginfo->framesize,ginfo->quality);
            else if (ginfo->framesize)
                printf("framesize: %8d \n",ginfo->framesize);
            else
                printf("quality:   %8d \n",ginfo->quality);

            if (ginfo->fps || ginfo->dpi)
                printf("  fps:         %6.2f         dpi:          %5.1f\n",ginfo->fps,ginfo->dpi);


            for (j=0; j<numhotspots; ++j)
            {
                bool isfont=(ginfo->signature&0xffffff00)==GMAKEID('F','N','T',0)
                         || (ginfo->signature&0xffffff00)==GMAKEID('F','n','t',0);
                printf("  hotspottbl[%d]:%6d,%4d ",j,ginfo->hotspottbl[j][0],ginfo->hotspottbl[j][1]);

                if (isfont)
                {
                    int centerx=ginfo->hotspottbl[1][0];
                    int centery=ginfo->hotspottbl[0][1];

                    if (j==0)
                        printf("advancex %d, yoffset %d",ginfo->hotspottbl[0][0],-ginfo->hotspottbl[0][1]);
                    else if (j==1)
                        printf("xoffset %d, advancey %d",-ginfo->hotspottbl[1][0],ginfo->hotspottbl[1][1]);
                    else if (j==2)
                        printf("ascent %d, descent %d",ginfo->hotspottbl[2][0],ginfo->hotspottbl[2][1]);
                    else if (j>=3)
                    {
                        int currentcode;
                        sscanf(ginfo->framename,"%x",&currentcode);
                        printf("kern %04x %04x (%c%c) %d",
                        ginfo->hotspottbl[j][0],currentcode,
                        ginfo->hotspottbl[j][0],currentcode,
                        ginfo->hotspottbl[j][1]);
                    }
                }

                printf("\n");

            }


            if (ginfo->comment[0])
            {
                printf("  comment:       '%s'\n",ginfo->comment);
            }

            {
                GINFOOLDSHP *ginfoshp = (GINFOOLDSHP *)ginfo;

                if ((ginfo->signature==GMAKEID('.','T','I','M') || ginfo->signature==GMAKEID('S','H','P','P'))
                  && ginfo->size==sizeof(GINFOOLDSHP)
                 && ginfo->bpp<=8)
                {
                    printf("  clutx:     %8d         cluty:     %8d \n",ginfoshp->clutx,ginfoshp->cluty);
                }

                if ((ginfo->signature==GMAKEID('I','L','B','M')
                || ((ginfo->signature&~255)==GMAKEID('S','H','P',0))
                || ((ginfo->signature&0x00ffffff)==GMAKEID(0,'S','S','F')))
                  && ginfo->size==sizeof(GINFOOLDSHP))
                {
                    for (j=0; j<ginfoshp->numanims; ++j)
                    {
                        printf("  palanim[%d]:  dir %2d, speed %2d, %d..%d\n",j,
                          ginfoshp->palanim[j].dir,
                          ginfoshp->palanim[j].speed,
                          ginfoshp->palanim[j].start,
                          ginfoshp->palanim[j].end);
                    }
                }
            }

            {
                GINFOPSD *ginfopsd = (GINFOPSD *)ginfo;

                if (ginfo->signature==GMAKEID('8','B','P','S')
                  && ginfo->size==sizeof(GINFOPSD))
                {
                    printf("  blendmethod:  '%4s'        opacity:   %8d \n",GXid2str(idbuf,ginfopsd->blendmethod),ginfopsd->opacitylevel);
                    printf("  canvasw:   %8d         canvash:   %8d \n",ginfopsd->canvasw,ginfopsd->canvash);
                }
            }
            break;

        case 4:
        case 5:
        case 6:

            printf("\nGINFO #%d--------------------\n",ginfo->framenum);

            printf("signature:       %4s         packed:    %10d \n",GXid2str(idbuf,ginfo->signature),ginfo->packed);
            printf("size:        %8d         quality:     %8d \n",ginfo->size,             ginfo->quality);
            printf("framenum:    %8d         alphabits:   %8d \n",ginfo->framenum,         ginfo->alphabits);
            printf("width:       %8d         redbits:     %8d \n",ginfo->width,            ginfo->redbits);
            printf("height:      %8d         greenbits:   %8d \n",ginfo->height,           ginfo->greenbits);
            printf("bpp:         %8d         bluebits:    %8d \n",ginfo->bpp,              ginfo->bluebits);
            printf("originalbpp: %8d         centerx:     %8d \n",ginfo->originalbpp,      ginfo->centerx);
            printf("startcolour: %8d         centery:     %8d \n",ginfo->startcolour,      ginfo->centery);
            printf("numcolours:  %8d         defaultx:    %8d \n",ginfo->numcolours,       ginfo->defaultx);
            printf("subtype:     %8d         defaulty:    %8d \n",ginfo->subtype,          ginfo->defaulty);
            printf("framesize:   %8d         numhotspots: %8d \n",ginfo->framesize,        ginfo->numhotspots);

            printf("framename:      '%s'\n",ginfo->framename);
            for (j=0; j<numhotspots; ++j)
            {
                printf("hotspottbl[%d].x:%8d\n",j,ginfo->hotspottbl[j][0]);
                printf("hotspottbl[%d].y:%8d\n",j,ginfo->hotspottbl[j][1]);
            }
            if (ginfo->signature==GMAKEID('S','H','P','P'))
            {
                printf("clutx:       %8d         cluty:       %8d \n",((GINFOOLDSHP *) ginfo)->clutx,((GINFOOLDSHP *) ginfo)->cluty);
            }
            if ( (ginfo->signature&~0xFF) == GMAKEID('S','H','P',0) )
            {
                int semitrans=0;
                int abr;
                char code;

                abr = (((GINFOOLDSHP *)ginfo)->tpage>>5) & 0x3;
                code = ((GINFOOLDSHP *)ginfo)->code;

                if ((code == 0) && (abr == 0))  /* Default */
                {
                    semitrans = 0;
                }
                else if (code & 0x2)            /* Not opaque */
                {
                    switch(abr)
                    {
                        case 0: /* 50\50 */
                            semitrans = 1;
                            break;
                        case 1: /* add */
                            semitrans = 3;
                            break;
                        case 3: /* subtract */
                            semitrans = 4;
                            break;
                    }
                }
                else                            /* Opaque */
                {
                    semitrans = 2;
                }
                printf("semitrans:   %8d\n", semitrans);

            }
            printf("fps:           %6.2f         dpi:            %5.1f\n",ginfo->fps,ginfo->dpi);
            if (ginfo->comment[0])
            {
                printf("  comment:       '%s'\n",ginfo->comment);
            }
            break;

        case CENTERVIEW+0:
        case CENTERVIEW+1:
        case CENTERVIEW+2:
            printf("    ");
            padprint(ginfo->centerx,",",ginfo->centery,",",12);
            printf("\n");
            break;

        case CENTERVIEW+3:
        case CENTERVIEW+4:
        case CENTERVIEW+5:
        case CENTERVIEW+6:
        {
            char framebuf[8];

            printf("    ");
            padprint(ginfo->centerx,",",ginfo->centery,",",12);

            if (ginfo->framename[0])
                sprintf(framebuf," '%-s':",ginfo->framename);
            else
                sprintf(framebuf," #%d:",ginfo->framenum);
            printf("//%-9s",framebuf);

            padprint(ginfo->width,"x",ginfo->height,",",9);
            padprint(ginfo->originalbpp," bpp, ",0,(char *)0,8);

            printf("Def ");
            padprint(ginfo->defaultx,",",ginfo->defaulty,",",9);
            if (verbose>=2)
            {
                printf("Size ");
                padprint(shapesize,", ",0,(char *)0,9);
            }

            if (ginfo->packed==0)
                printf("Unpacked");
            else
                printf("Packed");
            printf("\n");
            break;
        }

        case COMMENTVIEW+0:
        case COMMENTVIEW+1:
        case COMMENTVIEW+2:
        case COMMENTVIEW+3:
        case COMMENTVIEW+4:
        case COMMENTVIEW+5:
        case COMMENTVIEW+6:
            if (ginfo->comment[0])
            {
                if (OPTverbose>=2)
                    printf("    ");
                printf("'%-s'\n",ginfo->comment);
            }
            break;
    }
}

void GINFO_showaboutex(GABOUT *about,const char *fname,int ok)
{
    int j;
    char idbuf[8];

    printf("is%s     %8d\n",about->shorttypestr,ok);

    if (GIMEXabout)
    {
        printf("GIMEXDLL ABOUT --------------------\n");
        printf("version          %.2f\n",(float)GIMEXabout->version/100.0f);
        printf("author           %s\n",GIMEXabout->author);
        printf("comment          %s\n",GIMEXabout->comment);
    }

    printf("GIMEX ABOUT '%s'--------------------\n",fname);
    printf("version          %.2f\n",(float)about->version/100.0f);
    printf("canimport    %8d        canexport    %8d\n",about->canimport,about->canexport);
    printf("importpacked %8d        exportpacked %8d\n",about->importpacked,about->exportpacked);
    printf("import8      %8d        export8      %8d\n",about->import8,about->export8);
    printf("import32     %8d        export32     %8d\n",about->import32,about->export32);
    printf("multiframe   %8d        multifile    %8d\n",about->multiframe,about->multifile);
    printf("framebuffer  %8d        external     %8d\n",about->framebuffer,about->external);
    for (j=0; j<MAXMACTYPES; ++j)
        if (about->mactype[j])
            printf("mactype[%d]       %s\n",j,GXid2str(idbuf,about->mactype[j]));
    for (j=0; j<MAXEXTENSIONS; ++j)
        if (about->extensions[j][0])
            printf("extensions[%d]    %s\n",j,about->extensions[j]);
    printf("authorstr        %s\n",about->authorstr);
    printf("versionstr       %s\n",about->versionstr);
    printf("shorttypestr     %s\n",about->shorttypestr);
    printf("wordtypestr      %s\n",about->wordtypestr);
    printf("longtypestr      %s\n",about->longtypestr);
}

#define BIGNUM 0x7fffffff

void initstats(int *stats)
{
    stats[0] =  BIGNUM;
    stats[1] =  BIGNUM;
    stats[2] = -BIGNUM;
    stats[3] = -BIGNUM;
    stats[4] =0;
    stats[5] =0;
}

void updatestats(int *stats,GINFO *ginfo)
{
    int x;
    int y;

    x = ((ginfo->defaultx&0x3fff) << (2+16)) >> (2+16);
    y = ((ginfo->defaulty&0x3fff) << (2+16)) >> (2+16);

    stats[0] = min(stats[0],x);
    stats[1] = min(stats[1],y);
    stats[2] = max(stats[2],x+ginfo->width);
    stats[3] = max(stats[3],y+ginfo->height);

    if (strstr(ginfo->framename,"_mm"))
        ++stats[4];
    else
        ++stats[5];
}

void showstats(int *stats,int psx)
{
    if (psx && stats[0]!=BIGNUM)
        printf("Bounding rectangle: (%d,%d..%d,%d) %dx%d.  %d textures, %d mipmaps\n",
          stats[0],stats[1],stats[2],stats[3],
          stats[2]-stats[0],stats[3]-stats[1],
          stats[5],stats[4]);
}

bool justshowpalette(int OPTverbose,int OPTbanner, const char *OPTpalrange)
{
    if (OPTverbose<=1 && !OPTbanner && strcmp(OPTpalrange,"-1"))
        return true;
    return false;
}

