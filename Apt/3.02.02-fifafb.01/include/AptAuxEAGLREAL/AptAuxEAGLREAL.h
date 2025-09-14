#ifndef __APTAUX_EAGLREAL_H
#define __APTAUX_EAGLREAL_H
/*!
\Module AptAuxEAGLREAL
*/
//@{

#if defined(EA_PLATFORM_PLAYSTATION2) || defined(EA_PLATFORM_WINDOWS) || defined(EA_PLATFORM_XBOX) || defined(EA_PLATFORM_PSP)
#define EAGL_MODEL_RENDER 1
#endif

#include "realsystem.h"
#include "realmem.h"
#include "realmath.h"
#include "realcodec.h"
#include "realfile.h"
#include "realgraph.h"
#include "realshape.h"
#include "realinput.h"
#include "realfont.h"

#if defined(APT_USE_SOUND_OBJECT)
#if !defined( PSP_EMU )
    #include "snd/snd.h"
#endif
#endif

#include "eagl/runtime/eagl.h"
#include "eagl/runtime/tar.h"
#include "eagl/runtime/state.h"
#include "eagl/runtime/model.h"
#include "eagl/runtime/singledraw.h"
#include "../apt/apt.h"


typedef struct AptAuxInitParms_t {
    int iLevelInfoSize; /// Size for gLevelInfo Array.
    int iMaxStringCount;
    int iMaxDelayedReleaseValues; // Size of gpDelayedReleasePointers Array
    AptAuxInitParms_t () : iLevelInfoSize(64), iMaxStringCount(256),iMaxDelayedReleaseValues(64) {};
}AptAuxInitParmsT;

struct AptAuxFontList
{
    int             nFonts;
    unsigned short *aszFontNamesEnglish[16];
    unsigned short *aszFontNamesJapanese[16];
    void*           pData[16];
    int             nDataSize[16];
};

#define APTAUX_MAXCHARACTERS        512
#define APTAUX_MAXTEXPERLEVEL       256
#define APTAUX_MAXSOUNDSPERLEVEL    256

enum AptAuxCharacterType
{
    AptAuxCharacterType_None = 0,
    AptAuxCharacterType_Geometry = 1,
    AptAuxCharacterType_Texture = 2
};

typedef void *AptAuxIDToChar;

struct AptAuxSoundInfo
{
#if defined(APT_USE_SOUND_OBJECT)
    int nHandle;
    int nCharID;
    int nPatchID;
#endif
};

struct AptAuxSoundBankInfo
{
#if defined(APT_USE_SOUND_OBJECT)
    void *pData;
    int nHandle;
    int nSounds;
    AptAuxSoundInfo *aSounds;
#endif
};

struct AptAuxLayerInfo
{
    int bInUse;
    void *pBigData;
    int bBigDataToBeFreed;
    char szBaseName[128];
    char *pGeometryCopy__EAGL_HACK;
    char *pGeometryCopy__Base;
    EAGL::DynamicLoader *pGEOData;
#if defined(APT_USE_SOUND_OBJECT)
    AptAuxSoundBankInfo soundData;
#endif
    int nHighestGeometryIndex;
    void *pUserData;
};

extern const AptAuxInitParmsT gDefaultAptAuxParams;
void AptAuxEAGLREAL_SetupCallbacks(void);
void AptAuxEAGLREAL_InitializeFonts(AptAuxFontList *pFonts);
void AptAuxEAGLREAL_ShutdownFonts(void);
void AptAuxEAGLREAL_Initialize(EAGL::ViewPort *pView, EAGL::RenderContext *pContext, char *szFilePrefix, AptAuxFontList *pFonts, const AptAuxInitParmsT *pInitParams = &gDefaultAptAuxParams);
AptAuxLayerInfo *AptAuxEAGLREAL_ResolveData(void *pFileData, const char *szBaseFilename, void **ppAptData, void **ppConstData);
void AptAuxEAGLREAL_ReleaseLayerInfo(AptAuxLayerInfo *pLevel);
void AptAuxEAGLREAL_TimerInit(void);
void AptAuxEAGLREAL_TimerStartFrame(void);
unsigned int AptAuxEAGLREAL_TimerGetTicks(void);
float AptAuxEAGLREAL_TimerGetTime(void);
void AptAuxEAGLREAL_FinishAsyncLoads(void);
void AptAuxEAGLREAL_CleanUpOldStrings(void);
void AptAuxEAGLREAL_TransformPoint(float fInX, float fInY, float *pfOutX, float *pfOutY);

void AptAuxEAGLREAL_DrawStringIgnoreZ(AptAssetString string);
void AptAuxEAGLREAL_NeverClearZ(bool bNeverClear);

SHAPE *AptAuxEAGLREAL_GetSHAPEFromString(AptAssetString string);
SHAPE *AptAuxEAGLREAL_CreateFontTexture(int *pnWidth, int *pnHeight, unsigned int nARGBColour, int MB_FLAG);
unsigned int AptAuxEAGLREAL_StartBatchDraw(SHAPE *shape, bool strip, bool ignoreZ);
void AptAuxEAGLREAL_AddStringVertex(COORD4 *c, unsigned int clr, COORD4 *t);
void AptAuxEAGLREAL_EndBatchDraw();

/// This function sets the gloabal variable of the FontTextureMemoryPoolSize.  It doesn't do the allocation.
/// This function has several uses:
/// -# Call it before call AptAuxEAGLREAL_Initialize - this gives a chance to cleint to tell how large the pool should be.
/// -# Call it before call AptAuxEAGLREAL_ReallocFontTextureMemoryPool - this allow client to specify how large should the pool to resized to.
void AptAuxEAGLREAL_SetFontTextureMemoryPoolSize(int size);

// added as request from def jam team -FR #348 Access to custom control vertexMatrix
MATRIX4* AptAuxEAGLREAL_GetCustomControlMatrix();

//-----------------------------------------------------------
// added for Bug [ #460 ] Apt no longer lets you control where to a load happens.
// following 4 functions allow user of AptAuxEAGLREAL to control where big files will be loaded.

/// Allows user to set the memory area where big file will be loaded.
/// Please look in REAL core documentation for Predefined memory location classes.
/// Default is set to MB_LOW | MB_ALIGN
void AptAuxEAGLREAL_SetAsyncLoadIntoMemFlags(int nFlags);

/// Returns the memory location/area where big files will be loaded.
/// Please look in REAL core documentation for Predefined memory location classes.
int AptAuxEAGLREAL_GetAsyncLoadIntoMemFlags();

/// Allows user to set the memory location where decompression on big files can take place
/// if it is required.
/// Default is set to MB_HIGH | MB_ALIGN
/// Please look in REAL core documentation for Predefined memory location classes.
void AptAuxEAGLREAL_SetAsyncDecompressIntoMemFlags(int nFlags);

/// Returns the memory location where big file decompression will take place.
/// Please look in REAL core documentation for Predefined memory location classes.
int AptAuxEAGLREAL_GetAsyncDecompressIntoMemFlags();
//--------------------------------------------------------------


///Allows user to change whether or not allocated regions for files are tagged. Tagging memory does
///not directly pose any problems just that the memory for the loaded files must be preallocated, and
///to do that we must call blocking file information functions to get the size (these can cause the
///UI to be unresponsive for up to a second.and add to the total load time). Thus on release builds
///it is recommended that tagging be disabled.
void AptAuxEAGLREAL_SetAsyncLoadUseTag(bool newStatus);

///Returns whether Memory tagging is enabled.
bool AptAuxEAGLREAL_GetAsyncLoadUseTag() ;

/// Added to give ability to play saved inputs.
void AptAuxEAGLREAL_PlayInputs(const char* fileName);
void AptAuxEAGLREAL_RecordInputs(const char* fileName);
void AptAuxEAGLREAL_PlaySavedInputsDone(bool bSuccess, const char * pszFailCause);

void AptAuxEAGLREAL_SetDumpFlag(bool bFlag) ;

#if defined (APT_GATHER_MEMORY_METRICS)
void AptAuxEAGLREAL_PrintMemoryAllocationsInfo(bool bToFile, char * pszMemoryFileName) ;
void AptAuxEAGLREAL_SetMemDumpFileName(const char * pszFileName)    ;
char * AptAuxEAGLREAL_GetMemDumpFileName()  ;
#endif


void  AptAuxEAGLREAL_PrintProfileInfo(bool bToFile, char * pszProfileFileName) ;
void AptAuxEAGLREAL_SetProfileDumpFileName(const char * pszFileName) ;
char * AptAuxEAGLREAL_GetProfileDumpFileName()  ;


//@}

#endif

