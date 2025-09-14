#ifndef APTAUX_PC_OPENGL_H
#define APTAUX_PC_OPENGL_H

/*!
\Module AptAuxPCOpenGL
*/
//@{


struct AptAuxFontList
{
    int nFonts;
    unsigned short *aszFontNamesEnglish[16];
    unsigned short *aszFontNamesJapanese[16];
    void*   pData[16];
    int     nDataSize[16];
};

void AptAuxPCOpenGL_Initialize(int nWidth, int nHeight, AptAuxFontList *pFonts);
void AptAuxPCOpenGL_Shutdown();
void AptAuxPCOpenGL_TimerInit(void);
void AptAuxPCOpenGL_TimerStartFrame(void);
unsigned int AptAuxPCOpenGL_TimerGetTicks(void);
void AptAuxPCOpenGL_PlaySavedInputs(const char *szFilename, unsigned int nPlayTo);
void AptAuxPCOpenGL_PlaySavedInputsDone(bool bSuccess, const char * pszFailCause);

void AptAuxPCOpenGL_FinishAsyncLoads(void);
void AptAuxPCOpenGLDebug_VerifyUnresolve(bool bEnabled);

// added as request from def jam team -FR #348 Access to custom control vertexMatrix
float* AptAuxPCOpenGL_GetCustomControlMatrix();

// The reason this is defined outside apt is because Aux code doesn't read the .apt file.
// Whereas the _fileLoad on the .apt automatically allocates memory for all non-aux portions of characters,
// Aux code needs to keep track of characters and levels by itself in order to be able to manage resources
// properly.

#define APTAUX_MAXCHARSPERLEVEL         1024
#define APTAUX_MAXTEXPERLEVEL           1024
#define APTAUX_MAXSOUNDSPERLEVEL        1024

enum AptAuxUnitType
{
    AptAuxUnitType_None         = 0,
    AptAuxUnitType_TriSolid     = 1,
    AptAuxUnitType_TriLinear    = 2,
    AptAuxUnitType_TriRadial    = 3,
    AptAuxUnitType_TriTiled     = 4,
    AptAuxUnitType_TriClipped   = 5,
    AptAuxUnitType_LineSolid    = 6
};

struct AptAuxPrimList
{
    int     nPrims;
    float   *pData;
};

struct AptAuxUnitTriSolid
{
    unsigned int    nColour;
//  AptAuxUnitList  tris;
};

struct AptAuxIDToChar;

struct AptAuxUnitTriTiled
{
    unsigned int    nColour;
    float           afMatrix[16];
    unsigned int    nBitmapCharID;
    int             nTextureID;
//  AptAuxUnitList  tris;
};
typedef struct AptAuxUnitTriTiled AptAuxUnitTriClipped;

struct AptAuxUnitLineSolid
{
//  float           fWidth;
    unsigned int    nColour;
//  AptAuxUnitList  lines;
};

struct AptAuxUnit
{
//  int magic;
    AptAuxUnitType      eType;
    AptAuxPrimList      prims;
    union
    {
        AptAuxUnitTriSolid      triSolid;
        AptAuxUnitTriTiled      triTiled;
        AptAuxUnitTriClipped    triClipped;
        AptAuxUnitLineSolid     lineSolid;
    };
};

struct AptAuxShape
{
    int         nID;
    int         nUnits;
    AptAuxUnit  **apUnits;
};

struct AptAuxGeoInfo
{
    char            szMagic[24];
    int             nShapeIDs;
    AptAuxShape     *aShapes;
};

enum AptAuxCharacterType
{
    AptAuxCharacterType_None = 0,
    AptAuxCharacterType_Geometry = 1,
    AptAuxCharacterType_Texture = 2
};

struct AptAuxIDToChar
{
    AptAuxCharacterType eType;
    void                *pData;
};

struct AptAuxLayerInfo
{
    int bInUse;
    void *pBigData;
    AptAuxGeoInfo *pGEOData;
    AptAuxIDToChar  aIDToChar[APTAUX_MAXCHARSPERLEVEL];

    char szVerifyUnresolveName[256];
};

//@}

#endif
