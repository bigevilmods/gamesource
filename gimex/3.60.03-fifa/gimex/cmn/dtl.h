// (c) 1996-2007 Electronic Arts Inc.
/*----------------------------------------------------------------------\\ */
/* some miscelleanous defines                                           \\ */
/*----------------------------------------------------------------------\\ */
#define UBYTE   unsigned char
#define BYTE    char
#define UWORD   unsigned short
#define WORD    short
#define ULONG   unsigned int
#define LONG    int

/*----------------------------------------------------------------------\\ */
/* macros                                                               \\ */
/*----------------------------------------------------------------------\\ */
#define GetClutID(x,y) (((x >> 4) & 0x3f) | (y << 6))
#define GetTextureID(x1,y1,mode,xprate) ((((mode) & 3) << 7) | \
            (((xprate) & 3) << 5) | (((x1) >> 6) & 0x0f) | \
            (((y1) >> 4) & 0x10))


/*----------------------------------------------------------------------\\ */
/* Function Prototypes                                                  \\ */
/*----------------------------------------------------------------------\\ */

extern short cdecl InitializeDTL(UWORD BoardBase);
void cdecl SetDispXY(UWORD x, UWORD y);
void cdecl SetVideoMode(UWORD mode);
void cdecl SetDisplayMask(UWORD Mode);
void cdecl SetDrawClipTop(UWORD x, UWORD y);
void cdecl SetDrawClipBot(UWORD x, UWORD y);
void cdecl SetDrawXYLoc(UWORD x, UWORD y);
void cdecl PSXADSBlockFill(UWORD x, UWORD y, UWORD w, UWORD h,  ULONG BGR);
void cdecl SetTextureID(UWORD TextureID, UWORD dflag, UWORD tdflag);
void cdecl DrawSync(void);

/*----------------------------------------------------------------------\\ */
/* primitive bit settings                                               \\ */
/*----------------------------------------------------------------------\\ */
/* actually shadeing in texture mask (not flag for shadeing in texture) */
void SetShadeTex(UWORD tswitch);
void SetSemiTrans(UWORD tswitch);

/*----------------------------------------------------------------------\\ */
/* 3 point polygons (triangle) prototypes                               \\ */
/*----------------------------------------------------------------------\\ */
void Poly_F3(UWORD x0, UWORD y0, UWORD x1, UWORD y1,
    UWORD x2, UWORD y2, ULONG BGR);
void Poly_FT3(UWORD x0, UWORD y0, UWORD x1, UWORD y1,
    UWORD x2, UWORD y2, UWORD u0, UWORD v0,
    UWORD u1, UWORD v1, UWORD u2, UWORD v2,
    ULONG RGB, UWORD clutID, UWORD textureID);
void Poly_G3(UWORD x0, UWORD y0, UWORD x1, UWORD y1,
    UWORD x2, UWORD y2, ULONG RGB0, ULONG RGB1,
    ULONG RGB2);
void Poly_GT3(UWORD x0, UWORD y0, UWORD x1, UWORD y1,
    UWORD x2, UWORD y2, UWORD u0, UWORD v0,
    UWORD u1, UWORD v1, UWORD u2, UWORD v2,
    ULONG RGB0, ULONG RGB1, ULONG RGB2,
    UWORD clutID, UWORD textureID);

/*----------------------------------------------------------------------\\ */
/* 4 point polygons (quads) prototypes                                  \\ */
/*----------------------------------------------------------------------\\ */
void cdecl Poly_F4(UWORD x0, UWORD y0, UWORD x1, UWORD y1,
    UWORD x2, UWORD y2, UWORD x3, UWORD y3,
    ULONG BGR);

void Poly_FT4(UWORD x0, UWORD y0, UWORD x1, UWORD y1,
    UWORD x2, UWORD y2, UWORD x3, UWORD y3,
    UWORD u0, UWORD v0, UWORD u1, UWORD v1,
    UWORD u2, UWORD v2, UWORD u3, UWORD v3,
    ULONG RGB, UWORD clutID, UWORD textureID);
void Poly_G4(UWORD x0, UWORD y0, UWORD x1, UWORD y1,
    UWORD x2, UWORD y2, UWORD x3, UWORD y3,
    ULONG RGB0, ULONG RGB1, ULONG RGB2, ULONG RGB3);
void Poly_GT4(UWORD x0, UWORD y0, UWORD x1, UWORD y1,
    UWORD x2, UWORD y2, UWORD x3, UWORD y3,
    UWORD u0, UWORD v0, UWORD u1, UWORD v1,
    UWORD u2, UWORD v2, UWORD u3, UWORD v3,
    ULONG RGB0, ULONG RGB1, ULONG RGB2, ULONG RGB3,
    UWORD clutID, UWORD textureID);


/*----------------------------------------------------------------------\\ */
/* lines                                                                \\ */
/*----------------------------------------------------------------------\\ */
void Line_F2(UWORD x0, UWORD y0, UWORD x1, UWORD y1, ULONG RGB);
void Line_G2(UWORD x0, UWORD y0, UWORD x1, UWORD y1,
    ULONG RGB0, ULONG RGB1);

/*----------------------------------------------------------------------\\ */
/* Tiles                                                                \\ */
/*----------------------------------------------------------------------\\ */
void Tile(UWORD x0, UWORD y0, UWORD w, UWORD h, ULONG RGB);

/*----------------------------------------------------------------------\\ */
/* Sprites                                                              \\ */
/*----------------------------------------------------------------------\\ */
void Sprite8(UWORD x0, UWORD y0, UWORD u0, UWORD v0,
    UWORD clutID, ULONG RGB);
void Sprite16(UWORD x0, UWORD y0, UWORD u0, UWORD v0,
    UWORD clutID, ULONG RGB);
void Sprite(UWORD x0, UWORD y0, UWORD w, UWORD h,
    UWORD u0, UWORD v0, UWORD clutID, ULONG RGB);



/*----------------------------------------------------------------------\\ */
/* Load images to vram                                                  \\ */
/*----------------------------------------------------------------------\\ */
void cdecl DTLloadimage(UWORD x0, UWORD y0, UWORD w, UWORD h, void *dataptr);

