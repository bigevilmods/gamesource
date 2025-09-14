// (c) 1994-2007 Electronic Arts Inc.

#include "shpread.h"

// for getenv
#include <stdlib.h>

/****************************************************************/
/*  Information Functions                                       */
/****************************************************************/

#if SHPMAKECODE==3
#define DOES32 0
#else
#define DOES32 1
#endif

GABOUT * GCALL SHP_about(void)
{
    GABOUT *info;

    info = (GABOUT *) galloc(sizeof(GABOUT));
    if (info)
    {
        memset(info,0,sizeof(GABOUT));
        info->signature         = GMAKEID('S','h','p',SHPSIGNATURE&255);
        info->size              = sizeof(GABOUT);
        info->version           = GIMEX_VERSION;
        info->canimport         = 1;    /* supports importing */
        info->canexport         = 1;    /* supports exporting */
        info->importpacked      = 3;    /* supports importing packed */
        info->exportpacked      = 3;    /* supports exporting packed */
        info->import8           = 1;    /* supports importing 8 bpp indexed */
        info->export8           = 1;    /* supports exporting 8 bpp indexed */
        info->import32          = DOES32; /* supports importing 32 bpp direct rgb */
        info->export32          = DOES32; /* supports exporting 32 bpp direct rgb */
        info->multiframe        = SHPMULTIFRAME; /* supports multiple frames */
        info->multifile         = 0;    /* format requires additional files or resource fork */
        info->multisize         = 1;    /* supports each frame different size */
        info->framebuffer       = 1;    /* module requires memory to buffer entire frame */
        info->external          = 0;    /* module uses external resources */
        info->usesfile          = 1;    /* module is file based */
        info->mipmap            = 1;    /* module supports mipmaps */
        info->maxframename      = 512-8;/* max frame name character 512 - 1 null -1 (-1) -4 shapeid - 1 null */
        info->defaultquality    = 75;   /* default pack quality for MG */
        info->firstextension    = 1;    /* use first extension for filenames */
#if SHPMAKECODE/TYPEMAX==STREAMINGSHAPE
        info->movie           = 1;      /* movie file format */
#endif

        info->mactype[0]      = SHPMACTYPE0; /* mac file system types used */
        info->mactype[1]      = SHPMACTYPE1;
        info->mactype[2]      = SHPMACTYPE2;
        info->mactype[3]      = SHPMACTYPE3;

		bool removeext=false;
		if (getenv("GIMEX_DEPRECIATENEWSHAPE") || getenv("GIMEX_DISABLENEWSHAPE") || getenv("GIMEX_DNS"))
			removeext = true;

		if (removeext)
		{
			strcpy(info->extensions[0], SHPEXTENSION1);     /* extensions */
	        strcpy(info->extensions[1], SHPEXTENSION2);
		    strcpy(info->extensions[2], SHPEXTENSION3);
		}
		else
		{
			strcpy(info->extensions[0], SHPEXTENSION0);     /* extensions */
			strcpy(info->extensions[1], SHPEXTENSION1);
			strcpy(info->extensions[2], SHPEXTENSION2);
			strcpy(info->extensions[3], SHPEXTENSION3);
		}


        strcpy(info->authorstr,     "F Barchard");      /* authors */
        sprintf(info->versionstr,   "%.2f",SHP_VERSION/100.0f); /* version # */
        strcpy(info->shorttypestr,  SHPSHORTTYPE);      /* type */
        strcpy(info->wordtypestr,   SHPWORDTYPE);       /* wordtype */
        strcpy(info->longtypestr,   SHPLONGTYPE );      /* longtype */
    }
    return(info);
}

#undef DOES32

int GCALL SHP_is(GSTREAM *g)
{
    unsigned char shpheader[32];
    int ok=0;

    gseek(g,(GPOS)0);
    if (gread(g,shpheader,32) == 32)
    {

#if SHPMULTIFRAME
        if (SHPSIGNATURE && ggetm(shpheader,4)==SHPSIGNATURE)
        {
            ok = 100;
        }
        else if (FNTSIGNATURE && ggetm(shpheader,4)==FNTSIGNATURE)
        {
            ok = 98; // lower than font or streaming shape
        }

// if chunk based, skip over audio chunks
#if SHPMAKECODE/TYPEMAX==STREAMINGSHAPE
        else
        {
            gseek(g,(GPOS)0);
            if ((SHPSIGNATURE||FNTSIGNATURE) && SSF_numchunks(g,SHPSIGNATURE,FNTSIGNATURE,4)) // max 4 chunks before we give up
                ok = 99; // so we can tell it had audio
            else
            {
                unsigned char nextheader[32];

                int flen = (int)glen(g);
                int next = ggetx(shpheader+4,4);
                int flags = ggetx(shpheader+1,3);

                if ((unsigned int)next < (unsigned int) flen - 16
					&& (flags&ELEMENT_FLAG_NEW_FORMAT))
                {
                    gseek(g,next);
                    if (gread(g,nextheader,32) == 32)
                    {
						int validity = SHPRisvalidtype(shpheader,SHPMAKECODE);
                        if (validity
                         && SHPRisvalidtype(nextheader,SHPMAKECODE)
                         && ggeti(shpheader+24,4)  // width
                         && ggeti(shpheader+28,4)) // height
                        {
                            ok = 20+validity;
                        }
                    }
                }
            }
        }
#endif

#else

        {
            int next;
            int flen;
            unsigned char nextheader[32];

            if (SHPSIGNATURE && ggetm(shpheader,4)==SHPSIGNATURE)
            {
                ok = 95;
            }
            else
            {
                flen = (int)glen(g);
                next = ggetx(shpheader+4,4);    // next field
                if ((unsigned int)next < (unsigned int) flen)
                {
                    gseek(g,next);
                    if (gread(g,nextheader,32) == 32)
                    {
                        if (SHPMAKECODE && SHPRisvalidtype(shpheader,SHPMAKECODE)
                         && SHPRisvalidtype(nextheader,SHPMAKECODE))
                        {
                            ok = 80;
                        }
                    }
                }
            }
        }
#endif

    }
    return(ok);
}

/****************************************************************/
/*  Import Functions                                            */
/****************************************************************/

int GCALL SHP_open(GINSTANCE **gx, GSTREAM *g, const char *pathname,bool framecountflag)
{
    GINSTANCE    *Result=0L;
    ShapeListPtr gShape=0L;
    int frames=0;

    if (SHPRReadShapeFile(g,&gShape,SHPMULTIFRAME,SHPMAKECODE,SHPSIGNATURE,FNTSIGNATURE,SHPMIPMAPDATAALIGNMENT))
    {
        Result=(GINSTANCE*)galloc(sizeof(GINSTANCE));
        if (Result)
        {
            frames=gShape->NumShape;
            memset(Result,0,sizeof(GINSTANCE));
            Result->frames=frames;
            Result->signature=SHPSIGNATURE;
            Result->size=sizeof(GINSTANCE);
            Result->gstream=g;
            Result->gref=(char *)gShape;
        }
        *gx=Result;
    }
    return(frames);
}

#define SIGNEXTEND(x) (int)(short)(x)

GINFO * GCALL SHP_info(GINSTANCE *gx, int framenum)
{
    int             bpp;
    int             width;
    int             height;
//    int             centerx;
//    int             centery;
    int             shapex;
    int             shapey;
    int             colours=0;
    int             alphabits;
    int             rgbbits;
    int             startcolour;
    int             i;
    int             j;
    int             csm1=0;
    int             type=SHPMAKECODE;
    int             mode=DXT_DXT3_CODE;
    char            *D;
    char            *pal;
    GINFO           *ginfo=0;
    ShapeListPtr    gShape;
    GShapeInfo      *shpinfo;
    SHPHeaderStruct *HeaderPict=0;
    CLUTHeaderStruct *theClut;

    gShape=(ShapeListPtr)gx->gref;

    if (framenum<gShape->NumShape)
    {

        shpinfo = &gShape->ListShape[framenum];
        HeaderPict=(SHPHeaderStruct *)shpinfo->shape_Data;
        D = ((char*)HeaderPict + sizeof(SHPHeaderStruct));
        if (HeaderPict)
        {
            width    = SHPGETFIELD(HeaderPict->width);//&0x0fff;           /* Width of bitmap in pixels */
            height   = SHPGETFIELD(HeaderPict->height);//&0x0fff;          /* Height of bitmap in pixels */
            shapex   = SIGNEXTEND(SHPGETFIELD(HeaderPict->shapex)<<4)>>4; /* shape position x */
            shapey   = SIGNEXTEND(SHPGETFIELD(HeaderPict->shapey)<<4)>>4; /* shape position y */

            if (SHPGETFIELD(HeaderPict->flags)&TEXTURE_FLAG_TRANSPOSED) shpinfo->subtype |= SUBTRANSPOSE;
            if (SHPGETFIELD(HeaderPict->flags)&TEXTURE_FLAG_SWIZZLED)   shpinfo->subtype |= SUBSWIZZLE;
            if (SHPGETFIELD(HeaderPict->flags)&TEXTURE_FLAG_OPAQUE)     shpinfo->subtype |= SUBOPAQUE;
            if (SHPGETFIELD(HeaderPict->flags)&TEXTURE_FLAG_TRANSPARENT)shpinfo->subtype |= SUBTRANSPARENT;
            if (SHPGETFIELD(HeaderPict->flags)&TEXTURE_FLAG_HALF_ALPHA) shpinfo->subtype |= SUBHALFALPHA;
            if (SHPGETFIELD(HeaderPict->flags)&TEXTURE_FLAG_EMBM)       shpinfo->subtype |= SUBEMBM;
            if (SHPGETFIELD(HeaderPict->flags)&TEXTURE_FLAG_DOT3)       shpinfo->subtype |= SUBDOT3;
            if (SHPGETFIELD(HeaderPict->flags)&TEXTURE_FLAG_CUBE_MAP)   shpinfo->subtype |= SUBCUBEMAP;            
            if (SHPGETFIELD(HeaderPict->flags)&TEXTURE_FLAG_MIPMONT)    shpinfo->subtype |= SUBMIPMONT;            

            bpp = SHPRGetShpbpp(HeaderPict,type);
            if (bpp>8) shpinfo->subtype &= ~SUBNOPAL;

            switch (type)
            {
                case TYPEPSH:
                    alphabits = 1;
                    rgbbits = 5;
                    if (bpp==24)
                        rgbbits = 8;
                    break;

                case TYPEASH:
                    alphabits = 1;
                    rgbbits = 5;
                    break;

                case TYPEGSH:
				case TYPEGSS:
                    alphabits = 4;
                    rgbbits = 5;
                    break;

//                case TYPENSH:
                case TYPEMSH:
				case TYPEMSS:
                    alphabits = 1;
                    rgbbits = 8;
                    if (bpp==16)
                        rgbbits = 5;
                    break;

    	  	case TYPEXSH:
			case TYPEXSS:
			case TYPEZSH:
			case TYPEZSS:
			case TYPEYSH:
			case TYPEYSS:
                    alphabits = 8;
                    rgbbits = 8;
    				break;

                default:
                case TYPEVSH:
                case TYPEFSH:
				case TYPEFSS:
                    alphabits = 1;
                    rgbbits = 8;
                    break;
            }

            if (framenum < gx->frames)
                ginfo = (GINFO *) galloc(sizeof(GINFOSHP));

            if (ginfo)
            {
                memset(ginfo, 0, sizeof(GINFOSHP));

                /* template for ginfo */

                ginfo->signature        = SHPSIGNATURE;              /* type of file */
                ginfo->size             = sizeof(GINFOSHP);          /* Length of GINFO */
                ginfo->version          = GIMEX_VERSION;
                ginfo->framenum         = framenum;                  /* Current frame num */
                ginfo->width            = width;                     /* Width of bitmap in pixels */
                ginfo->height           = height;                    /* Height of bitmap in pixels */
                ginfo->subtype          = shpinfo->subtype;          /* subtype */
                ginfo->packed           = shpinfo->packed;           /* packed or not */
                ginfo->framesize        = shpinfo->framesize;        /* total data bytes for shape */
                ginfo->quality          = 100;          /* Quality of lossy packing 0..100 */
                ginfo->alphabits        = alphabits;    /* Number of bits in alpha channel */
                ginfo->redbits          = rgbbits;      /* Number of bits in red channel */
                ginfo->greenbits        = rgbbits;      /* Number of bits in grreen channel */
                ginfo->bluebits         = rgbbits;      /* Number of bits in blue channel */
//                ginfo->centerx          = centerx;      /* Center Point relative to upper left corner */
//                ginfo->centery          = centery;
                ginfo->defaultx         = shapex;       /* shape Coordinate Point */
                ginfo->defaulty         = shapey;
                ginfo->numhotspots      = 0;            /* Number of Hot Spots defined */

#if SHPMAKECODE/TYPEMAX==SHAPEFILE
                {
					if (gShape->ListShape[framenum].shape_Name)
						strcpy(ginfo->framename,gShape->ListShape[framenum].shape_Name);  /* Null terminated name of frame/image */

                    /* strip trailing spaces (exporter pads with spaces) */
                    {
                        int i;
                        for (i=3; i>=0; --i)
                        {
                            if (ginfo->framename[i]==' ')
                                ginfo->framename[i]=0;
                            else
                                break;
                        }
                    }
                }
#endif
                if(!SHPRGetShapeHotSpot(gShape,framenum,ginfo,type))
                {
                    if(ginfo)
                        gfree(ginfo);
                    return 0;
                }
//                SHPRGetShapeName(gShape,framenum,ginfo,type);
                SHPRGetShapeComment(gShape,framenum,ginfo,type);
                SHPRGetShapeMetal(gShape,framenum,ginfo,type);
                SHPRGetShapeDataBin(gShape,framenum,ginfo,type);

                /* 16,24 bpp true colour */
                if (bpp>8)
                {
                    ginfo->bpp              = 32;       /* Bits per pixel (8 or 32) */
                    ginfo->originalbpp      = bpp;
                    ginfo->numcolours  = 0;        /* Number of colours in original indexed palette */

                    if (bpp<=15)
                    {
                        if (type==TYPEGSH || type==TYPEGSS)      /* GSH 5a3 */
                        {
                            ginfo->alphabits = 3;
                            ginfo->redbits   = 5;
                            ginfo->greenbits = 5;
                            ginfo->bluebits  = 5;
                        }
                        else
                        {
                            ginfo->alphabits = 1;
                            ginfo->redbits   = 5;
                            ginfo->greenbits = 5;
                            ginfo->bluebits  = 5;
                        }
                    }
                    if(bpp == 12)
                    {
                        ginfo->alphabits = 0;
                        ginfo->redbits   = 2;
                        ginfo->greenbits = 8;
                        ginfo->bluebits  = 2;
                        ginfo->originalbpp = 12;
                    }
                    if (bpp==16)
                    {
                        if (type==TYPEFSH || type==TYPEXSH || type==TYPEXSS || type==TYPEFSS || type==TYPEZSH || type==TYPEZSS || type==TYPEYSH || type==TYPEYSS)/* FSH/XSH */
                        {
                            if ((HeaderPict->type&0x7f)==0x6d)  /* 4444 */
                            {
                                ginfo->alphabits = 4;
                                ginfo->redbits   = 4;
                                ginfo->greenbits = 4;
                                ginfo->bluebits  = 4;
                            }
                            else if ((HeaderPict->type&0x7f)==0x67)  /* 655 */
                            {
                                ginfo->alphabits = 0;
                                ginfo->redbits   = 6;
                                ginfo->greenbits = 5;
                                ginfo->bluebits  = 5;
                            }
                            else if ((HeaderPict->type&0x7f)==0x65)  /* IA8 */
                            {
                                ginfo->alphabits = 8;
                                ginfo->redbits   = 8;
                                ginfo->greenbits = 0;
                                ginfo->bluebits  = 0;
                            }
                            else if ((HeaderPict->type&0x7f)==0x68)  /* YUY2 */
                            {
                                ginfo->alphabits = 0;
                                ginfo->redbits   = 4;
                                ginfo->greenbits = 8;
                                ginfo->bluebits  = 4;
                            }
                            else
                            {
                                ginfo->alphabits = 0;
                                ginfo->redbits   = 5;
                                ginfo->greenbits = 6;
                                ginfo->bluebits  = 5;
                            }
                        }


                        else if (type==TYPEMSH || type == TYPEMSS)      /* MSH */
                        {
                            if ((HeaderPict->type&0x7f)==0x5a)  /* 4444 */
                            {
                                ginfo->alphabits = 4;
                                ginfo->redbits   = 4;
                                ginfo->greenbits = 4;
                                ginfo->bluebits  = 4;
                            }
                            else
                            {
                                ginfo->alphabits = 0;
                                ginfo->redbits   = 5;
                                ginfo->greenbits = 6;
                                ginfo->bluebits  = 5;
                            }
                        }
                        else if (type==TYPEGSH || type==TYPEGSS)      /* GSH */
                        {
                            if ((HeaderPict->type&0x7f)==0x13)  /* IA8 */
                            {
                                ginfo->alphabits = 8;
                                ginfo->redbits   = 8;
                                ginfo->greenbits = 0;
                                ginfo->bluebits  = 0;
                            }
                            else
                            {
                                ginfo->alphabits = 0;
                                ginfo->redbits   = 5;
                                ginfo->greenbits = 6;
                                ginfo->bluebits  = 5;
                            }
                        }

                    }
                    if (bpp>16)
                    {
                        if ((HeaderPict->type&0x7f)==0x66)  /* 6666 */
                        {
                            ginfo->alphabits = 6;
                            ginfo->redbits   = 6;
                            ginfo->greenbits = 6;
                            ginfo->bluebits  = 6;
                        }
                        else if ((HeaderPict->type&0x7f)==0x6a)  /* 2101010 */
                        {
                            ginfo->alphabits = 2;
                            ginfo->redbits   = 10;
                            ginfo->greenbits = 10;
                            ginfo->bluebits  = 10;
                        }
                        else
                        {
                            int bpc=gmin(8,bpp/3);
                            ginfo->redbits = ginfo->greenbits = ginfo->bluebits = bpc;
                            ginfo->alphabits = bpp-bpc*3;
                        }
                    }
                    mode = ggetm( &((((DXTHEADER *)(D))->code)) ,sizeof((((DXTHEADER *)(D))->code)));

                    if ((bpp==32 && ginfo->packed==3) || (((HeaderPict->type&0x7f) == 0x61) || ((HeaderPict->type&0x7f)==0x62))) // DXT3 = 4565
                    {

                        if(mode == DXT_DXT3_CODE || (HeaderPict->type&0x7f)==0x61)  // DXT3 packtype or DXT3 shapetype
                        {

                            ginfo->alphabits = 4;
                            ginfo->redbits   = 5;
                            ginfo->greenbits = 6;
                            ginfo->bluebits  = 5;
                        }
                        else
                        {
                            ginfo->alphabits = 3;
                            ginfo->redbits   = 5;
                            ginfo->greenbits = 6;
                            ginfo->bluebits  = 5;
                        }

                    }										// GSH dxt shape type               regular  dxt1 shape type
                    if ((bpp==16 && ginfo->packed==3) || (((HeaderPict->type&0x7f)==0x1e) || ((HeaderPict->type&0x7f)==0x60))) // DXT1 = 1565
                    {
                        ginfo->alphabits = 1;
                        ginfo->redbits   = 5;
                        ginfo->greenbits = 6;
                        ginfo->bluebits  = 5;
                    }
                }
                /* 8 grey or colour, with 16 or 24 bpp palette */
                else
                {
                    if (!bpp)        bpp    = 8;
                    ginfo->bpp              = 8;                        /* Bits per pixel (8 or 32) */
                    ginfo->originalbpp      = bpp;
                    theClut   = (CLUTHeaderStruct *) gShape->ListShape[framenum].shape_Clut;
                    colours   = (1 << bpp);

                    if (theClut)
                    {
                        int DataOffset = SHPGETFIELD(theClut->DataOffset);       // get the dataoffset field. Have to respect this in Shape2 for openas on fonts
                        colours = SHPRclutcolours(theClut,&startcolour,bpp,type);
                        // check for swizzled clut. PS2 has to be either
                        // CSM1 or CSM2.
                        csm1 = SHPGETFIELD(theClut->flags)&TEXTURE_FLAG_SWIZZLED;
                        if (type==TYPESSH && !csm1 && bpp==8)
                            ginfo->subtype |= SUBCSM2;
                        
                        ginfo->numcolours  = colours+startcolour;

                        pal = (char *) theClut + DataOffset;//sizeof(CLUTHeaderStruct);
                        switch (theClut->type)
                        {
                            case 0x22:  /* VSH 24 bpp (6 bit channels) */
                                ginfo->redbits = ginfo->greenbits = ginfo->bluebits = 6;
                                ginfo->alphabits = 0;
                                if (colours==256 && !ggeti(&pal[255*3+0],3))
                                    ginfo->alphabits = 1;

                                for (i=0; i<colours; ++i)
                                {
                                    ginfo->colourtbl[startcolour+i].a = (unsigned char)((i==255 && ginfo->alphabits)?0:255);
                                    ginfo->colourtbl[startcolour+i].r = (unsigned char)((pal[i*3+0]*255+32)/63);
                                    ginfo->colourtbl[startcolour+i].g = (unsigned char)((pal[i*3+1]*255+32)/63);
                                    ginfo->colourtbl[startcolour+i].b = (unsigned char)((pal[i*3+2]*255+32)/63);
                                }
                                break;


                            case 0x24:  /* OLDFSH 24 bpp */
                                ginfo->redbits = ginfo->greenbits = ginfo->bluebits = 8;
                                ginfo->alphabits = 0;

                                for (i=0; i<colours; ++i)
                                {
                                    ginfo->colourtbl[startcolour+i].a = (unsigned char)255;
									ginfo->colourtbl[startcolour+i].r = pal[i*3+0];
                                    ginfo->colourtbl[startcolour+i].g = pal[i*3+1];
                                    ginfo->colourtbl[startcolour+i].b = pal[i*3+2];
                                }
                                break;

                            case 0x2f:  /* FSH 24 bpp */
                                ginfo->redbits = ginfo->greenbits = ginfo->bluebits = 8;
                                ginfo->alphabits = 0;

                                for (i=0; i<colours; ++i)
                                {
                                    ginfo->colourtbl[startcolour+i].a = (unsigned char)255;
                                    ginfo->colourtbl[startcolour+i].b = pal[i*3+0];
                                    ginfo->colourtbl[startcolour+i].g = pal[i*3+1];
                                    ginfo->colourtbl[startcolour+i].r = pal[i*3+2];
                                }
                                break;

                            case 0x21:  /* SSH 32 bpp */
							case 0x3b:  /* MSH 32 bpp */
                                ginfo->redbits = ginfo->greenbits = ginfo->bluebits = ginfo->alphabits = 8;

                                for (i=0; i<colours; ++i)
                                {
                                    j = (csm1 && (i&31)>=8 && (i&31)<=23) ? i ^ 0x18 : i;

                                    ginfo->colourtbl[startcolour+i].r = pal[j*4+0];
                                    ginfo->colourtbl[startcolour+i].g = pal[j*4+1];
                                    ginfo->colourtbl[startcolour+i].b = pal[j*4+2];
                                    ginfo->colourtbl[startcolour+i].a = pal[j*4+3];
                                }
                                break;

                            case 0x32:  /* GSH 3555 bpp */
                                ginfo->alphabits = 3;
                                ginfo->redbits =   5;
                                ginfo->greenbits = 5;
                                ginfo->bluebits =  5;
                                for (i=0; i<colours; ++i)
                                {
                                    *(int *)&ginfo->colourtbl[startcolour+i]=GSHRpixel5To8(ggetx(pal+i*2,2));
                                }
                                break;

                            case 0x33:  /* GSH 3555 bpp */
                                ginfo->alphabits = 8;
                                ginfo->redbits =   8;
                                ginfo->greenbits = 8;
                                ginfo->bluebits =  8;
                                for (i=0; i<colours; ++i)
                                {
                                    ginfo->colourtbl[startcolour+i].a = pal[i*2+0];
                                    ginfo->colourtbl[startcolour+i].r = pal[i*2+1];
                                    ginfo->colourtbl[startcolour+i].g = pal[i*2+((colours*2+31)&~31)+0];
                                    ginfo->colourtbl[startcolour+i].b = pal[i*2+((colours*2+31)&~31)+1];
                                }
                                break;


                            case 0x2d:  /* FSH 1555 bpp */
                                ginfo->alphabits = 1;
                                ginfo->redbits =   5;
                                ginfo->greenbits = 5;
                                ginfo->bluebits =  5;
                                for (i=0; i<colours; ++i)
                                {
                                    *(int *)&ginfo->colourtbl[startcolour+i]=FSHRpixel5To8(ggetx(pal+i*2,2));
                                }
                                break;

                            case 0x20:  /* SSH 1555 bpp */
                            case 0x39:  /* MSH 1555 bpp */
                                ginfo->alphabits = 1;
                                ginfo->redbits =   5;
                                ginfo->greenbits = 5;
                                ginfo->bluebits =  5;
                                for (i=0; i<colours; ++i)
                                {
                                    j = i;
                                    if (csm1)
                                        if ((i&31)>=8 && (i&31)<=23)
                                            j ^= (8^16);

                                    *(int *)&ginfo->colourtbl[startcolour+i]=SSHRpixel5To8(ggetx(pal+j*2,2));
                                }
                                break;
                            case 0x30:  /* GSH IA8 clut */
                                ginfo->alphabits = 8;
                                ginfo->redbits =   8;
                                ginfo->greenbits = 0;
                                ginfo->bluebits =  0;
                                for (i=0; i<colours; ++i)
                                {
                                    *(int *)&ginfo->colourtbl[startcolour+i]=GSHRpixelIATo8(ggetx(pal+i*2,2));
                                }
                                break;


                            case 0x31:  /* GSH 565 bpp */
                            case 0x29:  /* FSH 565 bpp */
                                ginfo->alphabits = 0;
                                ginfo->redbits =   5;
                                ginfo->greenbits = 6;
                                ginfo->bluebits =  5;
                                for (i=0; i<colours; ++i)
                                {
                                    *(int *)&ginfo->colourtbl[startcolour+i]=FSHRpixel565To8(ggetx(pal+i*2,2));
                                }
                                break;

                            case 0x38:  /* MSH 565 bpp */
                                ginfo->alphabits = 0;
                                ginfo->redbits =   5;
                                ginfo->greenbits = 6;
                                ginfo->bluebits =  5;
                                for (i=0; i<colours; ++i)
                                {
                                    *(int *)&ginfo->colourtbl[startcolour+i]=MSHRpixel565To8(ggetx(pal+i*2,2));
                                }
                                break;

                            case 0x3a:  /* MSH 4444 */
                                ginfo->alphabits = 4;
                                ginfo->redbits =   4;
                                ginfo->greenbits = 4;
                                ginfo->bluebits =  4;
                                for (i=0; i<colours; ++i)
                                {
                                    *(int *)&ginfo->colourtbl[startcolour+i]=MSHRpixel4444To8(ggetx(pal+i*2,2));
                                }
                                break;

                            case 0x2a:  /* XSH/FSH 32 bpp */
                                {
                                    ginfo->alphabits = 8;
                                    ginfo->redbits =   8;
                                    ginfo->greenbits = 8;
                                    ginfo->bluebits =  8;
                                    for (i=0; i<colours; ++i)
                                    {
                                        ginfo->colourtbl[startcolour+i].b = pal[i*4+0];
                                        ginfo->colourtbl[startcolour+i].g = pal[i*4+1];
                                        ginfo->colourtbl[startcolour+i].r = pal[i*4+2];
                                        ginfo->colourtbl[startcolour+i].a = pal[i*4+3];
                                    }
                                }
                                break;

                            case 0x2e:  /* FSH 8555 bpp */
                                ginfo->alphabits = 8;
                                ginfo->redbits =   5;
                                ginfo->greenbits = 5;
                                ginfo->bluebits =  5;
                                for (i=0; i<colours; ++i)
                                {
                                    *(int *)&ginfo->colourtbl[startcolour+i]=FSHRpixel5To8(ggetx(pal+i*4,2));
                                    ginfo->colourtbl[startcolour+i].a = pal[i*4+3];
                                }
                                break;

                            case 0x2c:  /* FSH 8565 bpp */
                                ginfo->alphabits = 8;
                                ginfo->redbits =   5;
                                ginfo->greenbits = 6;
                                ginfo->bluebits =  5;
                                for (i=0; i<colours; ++i)
                                {
                                    *(int *)&ginfo->colourtbl[startcolour+i]=FSHRpixel565To8(ggetx(pal+i*4,2));
                                    ginfo->colourtbl[startcolour+i].a = pal[i*4+3];
                                }
                                break;

                            case 0x23:  /* PSH 16 bpp */
                                ginfo->alphabits = 1;
                                ginfo->redbits =   5;
                                ginfo->greenbits = 5;
                                ginfo->bluebits =  5;

                                for (i=0; i<colours; ++i)
                                {
                                    *(int *)&ginfo->colourtbl[startcolour+i]=PSHRpixel5To8(ggetx(pal+i*2,2));
                                }
                                break;

                            default:
                                break;
                        }
                    }
                    else    /* grey scale */
                    {
                        if ((HeaderPict->type&0x7f)==0x10)  /* GSH 4 bpp I4 */
                        {
                            ginfo->alphabits = 0;
                            ginfo->redbits   = 4;
                            ginfo->greenbits = 0;
                            ginfo->bluebits  = 0;
                            ginfo->numcolours  = colours;

                            for (i = 0; i < colours; ++i)
                            {
                                ginfo->colourtbl[i].a = (unsigned char)255;
                                ginfo->colourtbl[i].r = (unsigned char)(i*255/15);
                                ginfo->colourtbl[i].g = (unsigned char)(i*255/15);
                                ginfo->colourtbl[i].b = (unsigned char)(i*255/15);
                            }
                        }
                        else if ((HeaderPict->type&0x7f)==0x11)  /* GSH 8 bpp I8 */
                        {
                            ginfo->alphabits = 0;
                            ginfo->redbits   = 8;
                            ginfo->greenbits = 0;
                            ginfo->bluebits  = 0;
                            ginfo->numcolours  = colours;

                            for (i = 0; i < colours; ++i)
                            {
                                ginfo->colourtbl[i].a = (unsigned char)255;
                                ginfo->colourtbl[i].r = (unsigned char)i;
                                ginfo->colourtbl[i].g = (unsigned char)i;
                                ginfo->colourtbl[i].b = (unsigned char)i;
                            }
                        }
                        else if ((HeaderPict->type&0x7f)==0x12)  /* GSH 8 bpp IA4 */
                        {
                            ginfo->alphabits = 4;
                            ginfo->redbits   = 4;
                            ginfo->greenbits = 0;
                            ginfo->bluebits  = 0;
                            ginfo->numcolours  = colours;

                            for (i = 0; i < colours; ++i)
                            {
                                ginfo->colourtbl[i].a = (unsigned char)((i>>4)*255/15);
                                ginfo->colourtbl[i].r = (unsigned char)((i&15)*255/15);
                                ginfo->colourtbl[i].g = (unsigned char)((i&15)*255/15);
                                ginfo->colourtbl[i].b = (unsigned char)((i&15)*255/15);
                            }
                        }
                        else if ((HeaderPict->type&0x7f)==0x63)  /* XSH Luminance8 */
                        {
                            ginfo->alphabits = 0;
                            ginfo->redbits =   8;
                            ginfo->greenbits = 0;
                            ginfo->bluebits =  0;
                            ginfo->numcolours  = colours;
                            for (i=0; i<colours; ++i)
                            {
                                ginfo->colourtbl[i].a = (unsigned char)255;
                                ginfo->colourtbl[i].r = (unsigned char)i;
                                ginfo->colourtbl[i].g = (unsigned char)i;
                                ginfo->colourtbl[i].b = (unsigned char)i;
                            }
                        }
						else if ((HeaderPict->type&0x7f)==0x64)  /* XSH Alpha8 */
                        {
                            ginfo->alphabits = 8;
                            ginfo->redbits =   0;
                            ginfo->greenbits = 0;
                            ginfo->bluebits =  0;
                            ginfo->numcolours  = colours;
                            for (i=0; i<colours; ++i)
                            {
                                ginfo->colourtbl[i].b = 0xff;
                                ginfo->colourtbl[i].g = 0xff;
                                ginfo->colourtbl[i].r = 0xff;
                                ginfo->colourtbl[i].a = i;
                            }
                        }
                        else
                        {
                            ginfo->numcolours  = colours;
                            ginfo->alphabits   = 0;
                            if (colours>1)
                            {
                                for (i=0; i<colours; ++i)
                                {
                                    ginfo->colourtbl[i].a = 0xff;
                                    ginfo->colourtbl[i].r = (unsigned char)(i*255/(colours-1));
                                    ginfo->colourtbl[i].g = (unsigned char)(i*255/(colours-1));
                                    ginfo->colourtbl[i].b = (unsigned char)(i*255/(colours-1));
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return(ginfo);
}


int GCALL SHP_read(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes)
{
    ShapeListPtr    gShape=0L;
    SHPHeaderStruct *HeaderPict;
    int             framenum;
    int             ok=0;

    gShape=(ShapeListPtr)gx->gref;
    framenum=ginfo->framenum;
    if (framenum!=-1)
    {
        HeaderPict=(SHPHeaderStruct*)(gShape->ListShape[framenum].shape_Data);
        if (HeaderPict)
        {
            ok = SHPRReadBitmap(HeaderPict,dest,rowbytes,SHPMAKECODE);
        }
    }
    return(ok);
}


int GCALL SHP_close(GINSTANCE *gx)
{
    ShapeListPtr gShape=0L;

    if (gx)
    {
        gShape=(ShapeListPtr)gx->gref;
        SHPRDeleteShape(gShape);
        gfree((void*)gx);
    }
    return(1);
}

/* cleanup macros */

#undef SHP_about
#undef SHP_is
#undef SHP_open
#undef SHP_info
#undef SHP_read
#undef SHP_close
#undef SHP_wopen
#undef SHP_write
#undef SHP_wclose

#undef SHPSIGNATURE
#undef FNTSIGNATURE
#undef SHPEXTENSION0
#undef SHPEXTENSION1
#undef SHPEXTENSION2
#undef SHPEXTENSION3
#undef SHPMACTYPE0
#undef SHPMACTYPE1
#undef SHPMACTYPE2
#undef SHPMACTYPE3
#undef SHPSHORTTYPE
#undef SHPWORDTYPE
#undef SHPLONGTYPE
#undef SHPALIGNMENT
#undef SHPDATAALIGNMENT
#undef SHPMIPMAPDATAALIGNMENT
#undef SHPCLUTDATAALIGNMENT
#undef SHPPACKEDDATAALIGNMENT

#undef ggetx
#undef gputx
#undef SHPGETFIELD
#undef SHPPUTFIELD
#undef SHPALIGN
#undef SHPMIPMAPALIGN

#undef SHPMAKECODE

