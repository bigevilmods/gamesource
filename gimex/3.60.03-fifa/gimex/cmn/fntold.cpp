// (c) 1997-2007 Electronic Arts Inc.
/*------------------------------------------------------------------*/
/*  Local Functions                                                 */
/*------------------------------------------------------------------*/

typedef struct SHPHeader_t
{
    /* SHAPE HEADER structure */

    char    type;           /* type for this shape */
    char    linkShape[3];   /* link to next shape */
    char    width[2];       /* width of shape bitmap */
    char    height[2];      /* height of shape bitmap */
    char    centerX[2];     /* center point for shape */
    char    centerY[2];     /* center point for shape */
    char    shapeX[2];      /* position of shape */
    char    shapeY[2];      /* position of shape */
} SHPHeader;

typedef struct FNTPixOffset_t
{
    /* PIXEL OFFSET TYPE structure */

    char    y[2];           /* number of vertical pixels to top of character */
    char    x[2];           /* number of horizontal pixels to top of character  char    x[2]; */
} FNTPixOffset;

typedef struct FNTHeader_t
{
    /* FONT FILE HEADER structure */
    /* *** convert the first letter to lower case */

    char            FileType[4];    /* 'FNTP' for PSX, 'FNTI' for pc, etc... */
    unsigned char   First;          /* first character defined in ascii */
    unsigned char   Last;           /* last character defined in ascii (last-first+1 = total) */
    unsigned char   MaxWidth;       /* widest character in font in pixels */
    unsigned char   MaxHeight;      /* tallest character in font in pixels */
    char            Space;          /* default space between characters in pixels */
    char            YInc;           /* vertical size of characters in pixels */
    char            BaseLine;       /* distance from top left to baseline of font in pixels */
    char            DefaultWidth;   /* default width of undefined characters *** ??? */
    char            FileLength[4];  /* length of entire font file in bytes */
    char            Flags[2];       /* font flags (*** see font.doc for description) */
    char            Width[2];       /* offset to width table from start of file in bytes */
    char            Height[2];      /* offset to height table from start of file in bytes */
    char            XInc[2];        /* offset to xinc table from start of file in bytes */
    char            XOffset[2];     /* offset to xoffset table from start of file in bytes */
    char            YOffset[2];     /* offset to yoffset table from start of file in bytes */
    char            Shape[4];       /* offset to shape data for font from start of file in bytes */
} FNTHeader;

typedef struct FNTInfo_t
{
    /* GIMEX FONT INFO structure (this structure is stored in the gref field */
    /* of a ginstance */

    FNTHeader*  header;         /* pointer to font header */
    char*       widthTable;     /* pointer to width table for font */
    char*       heightTable;    /* pointer to height table for font */
    char*       xIncTable;      /* pointer to xInc table for font */
    char*       xOffsetTable;   /* pointer to xOffset table for font */
    char*       yOffsetTable;   /* pointer to yOffset table for font */
    SHPHeader*  shapeHeader;    /* pointer to shape header for font */
    char*       shapeData;      /* pointer to shape data for font */
} FNTInfo;

typedef struct FNTWriteInfo_t
{
    /* GIMEX FONT INFO structure (this structure is stored in the gref field */
    /* of a ginstance */

    FNTHeader*  header;         /* pointer to font header */
    int         maxXInc;        /* maximum xinc value in font */
    int         bpp;            /* destination bpp for font (1,4,8) */
    char*       widthTable;     /* pointer to width table for font */
    char*       heightTable;    /* pointer to height table for font */
    char*       xIncTable;      /* pointer to xInc table for font */
    char*       yIncTable;      /* pointer to yInc table for font (not saved to disk) */
    char*       xOffsetTable;   /* pointer to xOffset table for font */
    char*       yOffsetTable;   /* pointer to yOffset table for font */
    char**      characterList;  /* pointer to list of buffers holding single characters */
    char*       characterRowBytes; /* pointer to rowbytes table for characters in character list */
} FNTWriteInfo;



static int OLDGetRGBBits(void)
{
    switch (FNTTYPE)
    {
        case FNTTYPE_FFN:
            return 8;

		default:
            ;
    }
    return 0;
} /* OLDGetRGBBits */

static int OLDGetAlphaBits(void)
{
    switch (FNTTYPE)
    {
        case FNTTYPE_FFN:
            return 0;

		default:
            ;
    }
    return 0;
} /* FNTGetAlphaBits */


static int GetShapeRowBytes(int width, int bpp)
{
    int bits = width*bpp;
    int rowBytes;

#if FNTTYPE == FNTTYPE_FFN
    /* pc is 8 bit aligned, no min */
    rowBytes = (bits + 7)/8;
#elif FNTTYPE == FNTTYPE_3FN
    /* 3do is 32 bit aligned, 8 byte min */
    rowBytes = gmax((bits + 31)/32 * 4, 8);
#else
    /* psx and sat use default rowbyte calc */
    /* default is 32 bit aligned, no min */
    rowBytes = (bits + 31)/32 * 4;
#endif

    return rowBytes;
}


static int OLDGetRowBytes(int width, int shapeType)
{
    /* returns zero if there is a problem, otherwise returns the rowbytes */
    /* for the shape based on shape type and width of shape */

    int rowBytes = 0;
    int bits;
    int bpp;

    switch (shapeType)
    {
        case 0x03:
        case 0x7a:
            bpp = 4;
            break;

        case 0x01:
        case 0x79:
            bpp = 1;

        default:
            bpp = 1;
    }

    bits = width*bpp;

#if FNTTYPE == FNTTYPE_FFN
    /* pc is 8 bit aligned, no min */
    rowBytes = (bits + 7)/8;
#elif FNTTYPE == FNTTYPE_3FN
    /* 3do is 32 bit aligned, 8 byte min */
    rowBytes = gmax((bits + 31)/32 * 4, 8);
#else
    /* psx and sat use default rowbyte calc */
    /* default is 32 bit aligned, no min */
    rowBytes = (bits + 31)/32 * 4;
#endif

    return rowBytes;
} /* OLDGetRowBytes */


static int OLDGetBpp(FNTInfo* info)
{
    int bpp = 0;
    unsigned char shapeType = (unsigned char)FNTGETFIELD(info->shapeHeader->type);

    switch (shapeType)
    {
        case 0x01:
        case 0x79:
            bpp = 1;
            break;

        case 0x03:
        case 0x7a:
            bpp = 4;
            break;
    }
    return bpp;
} /* OLDGetBpp */


static int OLDGetFrameCount(FNTHeader* header)
{
    /* returns number of characters in font */
    return FNTGETFIELD(header->Last) - FNTGETFIELD(header->First) + 1;
}

static int OLDGetDefinedFrameCount(FNTInfo* info)
{
    /* returns number of defined frames in font */
    int characterCount = OLDGetFrameCount(info->header);
    int i;
    int frameCount = 0;

    if (info->widthTable)
	{
		for (i = 0; i < characterCount; i++)
	    {
			if (info->widthTable[i] > 0)
	            frameCount++;
	    }
	}
    return frameCount;
}

static int OLDCalcFrameNum(FNTInfo* info, int gimexIndex)
{
    /* returns the character index for the gimex frame */
    int index = 0;
    int definedCount = 0;   /* number of defined frames found so far */

    while (definedCount < gimexIndex)
    {
        if (info->widthTable[index] != 0)
            definedCount++;

        index++;
    }

    /* now check find the next undefined character */
    while (info->widthTable[index] == 0)
        index++;

    return index;
}

static int OLDWidth(FNTInfo* info, int framenum)
{
    if (info->widthTable)
        return info->widthTable[framenum];
    else
        return FNTGETFIELD(info->header->DefaultWidth);
}

static int OLDHeight(FNTInfo* info, int framenum)
{
    if (info->heightTable)
        return info->heightTable[framenum];
    else
        return FNTGETFIELD(info->header->YInc);
}

static int OLDXOffset(FNTInfo* info, int framenum)
{
    if (info->xOffsetTable)
        return info->xOffsetTable[framenum];
    else
        return 0;
}

static int OLDYOffset(FNTInfo* info, int framenum)
{
    if (info->yOffsetTable)
        return info->yOffsetTable[framenum];
    else
        return 0;
}

static int OLDXInc(FNTInfo* info, int framenum)
{
    if (info->xIncTable)
        return info->xIncTable[framenum];
    else
    {
        int xoffset = OLDXOffset(info, framenum);
        int width   = OLDWidth(info, framenum);
        int space   = FNTGETFIELD(info->header->Space);

        return xoffset + width + space;
    }
}


static void OLDReadTable(GSTREAM* g, char** table, int tableOffset, int frameCount)
{
    char* tablePtr = NULL;

    if (tableOffset > 0 && frameCount > 0)
    {
        if (gseek(g, tableOffset))
        {
            tablePtr = (char *) galloc(frameCount);
            if (tablePtr)
            {
                if (!gread(g, tablePtr, frameCount))
                {
                    gfree(tablePtr);
                    tablePtr = NULL;
                }
            }
        }
    }
    *table = tablePtr;
} /* OLDReadTable */


static void OLDReadShape(GSTREAM* g, FNTInfo* info)
{
    unsigned char shapeType;
    int  width;
    int  height;
    int  rowBytes;

    /* read shape header into header buffer */
    gseek(g, FNTGETFIELD(info->header->Shape));

    info->shapeHeader = (SHPHeader*) galloc(sizeof(SHPHeader));
    if (info->shapeHeader)
    {
        gread(g, info->shapeHeader, sizeof(SHPHeader));

        /* determine height and rowbytes using shapeHeader */
        shapeType = (unsigned char)FNTGETFIELD(info->shapeHeader->type);
        width     = FNTGETFIELD(info->shapeHeader->width);
        height    = FNTGETFIELD(info->shapeHeader->height);

        rowBytes  = OLDGetRowBytes(width, shapeType);

        /* read shape into shapeData buffer */
        gseek(g, FNTGETFIELD(info->header->Shape) + sizeof(SHPHeader));

        info->shapeData = (char *) galloc(rowBytes * height);
        if (info->shapeData)
            gread(g, info->shapeData, rowBytes * height);
    }
} /* OLDReadShape */


static void OLDReadLine(char* src, char* dst, int rowBytes, GINFO* ginfo)
{
    int column;

    rowBytes = rowBytes;

    switch (ginfo->originalbpp)
    {
        case 1:
            switch (FNTTYPE)
            {
                case FNTTYPE_FFN:
                {
                    for (column = 0; column < ginfo->width; column++)
                    {
                        *dst = (char)((*src >> (7 - (column % 8))) & 0x01);

                        dst++;
                        if ((column+1) % 8 == 0)
                            src++;
                    }
                    break;
                }

				default:
                    ;
            }
            break;

        case 4:
            switch (FNTTYPE)
            {
                case FNTTYPE_FFN:
                {
                    for (column = 0; column < ginfo->width; column++)
                    {
                        char data;
                        if (column % 2 == 0)
                            data = (char)((*src & 0xf0) >> 4);
                        else
                        {
                            data = (char)(*src & 0x0f);
                            src++;
                        }
                        *dst = data;
                        dst++;
                    }
                    break;
                }

                case FNTTYPE_3FN:
                {
                    for (column = 0; column < ginfo->width; column++)
                    {
                        char data;
                        if (column % 2 == 0)
                            data = (char)((*src & 0xf0) >> 4);
                        else
                        {
                            data = (char)(*src & 0x0f);
                            src++;
                        }
                        *dst = data;
                        dst++;
                    }
                    break;
                }

				default:
                    ;
            }
            break;

        case 8:
            ;
    }
}



/*------------------------------------------------------------------*/
/*  Import Functions                                                */
/*------------------------------------------------------------------*/

static int OLDFNT_open(GINSTANCE** gx, GSTREAM* g, const char* pathname,bool framecountflag)
{
    int frameCount = 0;     /* holds number of possible characters (last-first+1) in font */
    int gimexFrames = 0;    /* holds number of defined characters in font */

    FNTInfo* info = (FNTInfo*)galloc(sizeof(FNTInfo));

    pathname = NULL; /* remove bogus warning */

    if (info)
    {
        FNTHeader* header = (FNTHeader*) galloc(sizeof(FNTHeader));
        memset(info, 0, sizeof(FNTInfo));
        if (header)
        {
            GINSTANCE* instance = (GINSTANCE*)galloc(sizeof(GINSTANCE));
            memset(header, 0, sizeof(FNTHeader));
            if (instance)
            {
                memset(instance, 0, sizeof(GINSTANCE));

                /* read the known size of header to get number of fonts */
                gseek(g,(GPOS)0);
                gread(g, (char*)header, sizeof(FNTHeader));
                frameCount = OLDGetFrameCount(header);
                gfree(header);

                /* now allocate the real space for the header, remembering */
                /* to add enough room for the pixOffset table */

                header = (FNTHeader *) galloc(sizeof(FNTHeader) + frameCount*4);
                gseek(g,(GPOS)0);
                gread(g, (char*)header, sizeof(FNTHeader) + frameCount*4);
                info->header = header;

                /* from info in header determine what tables exist, allocate */
                /* memory, and read into coresponding pointer */
                OLDReadTable(g, &(info->widthTable),   FNTGETFIELD(header->Width),   frameCount);
                OLDReadTable(g, &(info->heightTable),  FNTGETFIELD(header->Height),  frameCount);
                OLDReadTable(g, &(info->xIncTable),    FNTGETFIELD(header->XInc),    frameCount);
                OLDReadTable(g, &(info->xOffsetTable), FNTGETFIELD(header->XOffset), frameCount);
                OLDReadTable(g, &(info->yOffsetTable), FNTGETFIELD(header->YOffset), frameCount);

                /* read shape data, user will usually want the entire font, */
                /* reading it all now is faster and doesn't require much memory */
                OLDReadShape(g, info);

                memset(instance, 0, sizeof(GINSTANCE));
                instance->frames  = frameCount;
                instance->signature = OLDSIGNATURE;
                instance->size = sizeof(GINSTANCE);
                instance->gstream = g;
                instance->gref    = (char*)info;

                gimexFrames = OLDGetDefinedFrameCount(info);
            }
            *gx = instance;
        }
        else
            gfree(info);
    }

    return gimexFrames;
} /* openfnt */

static GINFO* OLDFNT_info(GINSTANCE* gx, int framenum)
{
    /* given the framenum, calc index into to font tables (width, height, etc.) */
    /* and return a completed GINFO structure, if framenum is bad return NULL */

    GINFO* ginfo  = NULL;
    FNTInfo* info = (FNTInfo*)gx->gref;

    framenum = OLDCalcFrameNum(info, framenum); /* returns character index from gimexframenum */

    /* check framenum for validity */
    /* NOTE: framenum 0 will return the first defined character (ie: ASCI 32) */

    if (framenum < OLDGetFrameCount(info->header))
    {
        int rgbBits   = OLDGetRGBBits();
        int alphaBits = OLDGetAlphaBits();

        /* get info needed to create ginfo block from header and tables */
        int width  = OLDWidth(info, framenum);
        int height = OLDHeight(info, framenum);

        /* get font fields need for centerpoint and hotspot calculations */
        int xinc     = OLDXInc(info, framenum);
        int yinc     = FNTGETFIELD(info->header->YInc);
        int xoffset  = OLDXOffset(info, framenum);
        int yoffset  = OLDYOffset(info, framenum);
        /*int baseline = FNTGETFIELD(info->header->BaseLine);*/

        /* center point is the (left, baseline) coordinate for character, */
        int centerx = -xoffset;
        int centery = -yoffset;
        int hotSpotCount = 2;

        int bpp = OLDGetBpp(info);

        /* create ginfo block and fill info */
        ginfo = (GINFO*) galloc(sizeof(GINFO));
        if (ginfo)
        {
            memset(ginfo, 0, sizeof(GINFO));

            ginfo->signature        = OLDSIGNATURE; /* Type of file */
            ginfo->size             = sizeof(GINFO);/* Length of GINFO */
            ginfo->version          = 180;
            ginfo->framenum         = framenum;     /* Current frame num */
            ginfo->width            = width;        /* Width of bitmap in pixels */
            ginfo->height           = height;       /* Height of bitmap in pixels */
            ginfo->subtype          = 0;            /* subtype */
            ginfo->packed           = 0;
            ginfo->quality          = 100;          /* Quality of lossy packing 0..100 */
            ginfo->alphabits        = alphaBits;    /* Number of bits in alpha channel */
            ginfo->redbits          = rgbBits;      /* Number of bits in red channel */
            ginfo->greenbits        = rgbBits;      /* Number of bits in grreen channel */
            ginfo->bluebits         = rgbBits;      /* Number of bits in blue channel */
            ginfo->centerx          = centerx;      /* Center Point relative to upper left corner */
            ginfo->centery          = centery;
            ginfo->defaultx         = 0;            /* shape Coordinate Point */
            ginfo->defaulty         = 0;
            ginfo->numhotspots      = hotSpotCount; /* Number of Hot Spots defined */

            ginfo->bpp              = 8;            /* Bits per pixel (8 or 32) */
            ginfo->originalbpp      = bpp;

            /* build grayscale colour table with bpp*bpp or 2 entries */
            {
                int colours = (bpp > 1) ? bpp*bpp : 2;
                int i;
                for (i = 0; i< colours; ++i)
                {
                    ginfo->colourtbl[i].a = (unsigned char)(i*255 / (colours-1));
                    ginfo->colourtbl[i].r = (unsigned char)(i*255 / (colours-1));
                    ginfo->colourtbl[i].g = (unsigned char)(i*255 / (colours-1));
                    ginfo->colourtbl[i].b = (unsigned char)(i*255 / (colours-1));
                }
                ginfo->numcolours  = colours;  /* Number of colours in original indexed palette */
                ginfo->startcolour = 0;
            }

            /* store frame name based on first character defined and framenum */
            {
                int index = framenum + FNTGETFIELD(info->header->First);
                ginfo->framename[0] = 'c';
                ginfo->framename[1] = 'h';
                ginfo->framename[2] = 'r';
                ginfo->framename[3] = (char)index;;
            }

            /* store hotspot values */
            ginfo->hotspottbl[0][0] = xinc;     /* x value */
            ginfo->hotspottbl[0][1] = centery;  /* y value */
            ginfo->hotspottbl[1][0] = centerx;  /* x value */
            ginfo->hotspottbl[1][1] = yinc;     /* y value */
        }
    }
    return ginfo;
} /* infofnt */


static int OLDFNT_read(GINSTANCE* gx, GINFO* ginfo, char* dst, int rowBytes)
{
    FNTInfo* info = (FNTInfo*)gx->gref;
    char*    src = NULL;
    int      row;

    int shapeRowBytes = OLDGetRowBytes(
        FNTGETFIELD(info->shapeHeader->width),
        FNTGETFIELD(info->shapeHeader->type));

    int framenum = ginfo->framenum;

    /* get base pointer to shapeData */
    {
        /* determine x,y offset - stored in table directly after font info */
        int xyOffset = FNTGETMEM((char*)(info->header) + sizeof(FNTHeader) + framenum*4, 4);
        int x = xyOffset & 0x0000ffff;
        int y = xyOffset >> 16;

        src = info->shapeData + (y * shapeRowBytes) + (x*ginfo->originalbpp / 8);
    }

    for (row = 0; row < ginfo->height; row++)
    {
        /* read data from src into dst buffer using correct conversion */
        OLDReadLine(src, dst, rowBytes, ginfo);
        dst += rowBytes;
        src += shapeRowBytes;
    }

    return 1;
} /* readfnt */


static int OLDFNT_close(GINSTANCE* gx)
{
    if (gx)
    {
        FNTInfo* info = (FNTInfo*)gx->gref;
        if (info)
        {
            if (info->header)       gfree((void*)(info->header));
            if (info->widthTable)   gfree((void*)(info->widthTable));
            if (info->heightTable)  gfree((void*)(info->heightTable));
            if (info->xIncTable)    gfree((void*)(info->xIncTable));
            if (info->xOffsetTable) gfree((void*)(info->xOffsetTable));
            if (info->yOffsetTable) gfree((void*)(info->yOffsetTable));
            if (info->shapeHeader)  gfree((void*)(info->shapeHeader));
            if (info->shapeData)    gfree((void*)(info->shapeData));

            gfree((void*)info);
        }
        gfree((void*)gx);
    }
    return 1;
} /* closefnt */

