/*H************************************************************************************************/
/*!

\File
AptAuxEAGLREAL.cpp

\Description
This is an "Aux" implementation for Apt using the EAGL/REAL core libraries. This basically takes care
of most of the rendering and loading and such makeing game integration of apt much easier for teams
that use the EAGL/real core libraries.


\Notes
This file still has a number of globals / arrays that should be rearranged to have a more dynamic
system.

Also this has some quirks with initialization / destruction whereby there are no mirror functions.
I.e. AptAuxInitialize has no AptAuxDestoy... but we move along for now.

\Copyright
(c) 2003 Electronic Arts Inc.

\Version
1.0        04/05/06 (DC) Comment Creation
1.1     2004/07/19  (rrv:3) correction to text scaling factor, autosize left (bugs# 576-578)

*/
/************************************************************************************************H*/

/*!
\addtogroup g_referenceguide        Reference Guides
*/
//@{
/*!
\Module AptAuxEAGLREAL
*/
//@}

#include "Apt.h"

#include "AptAuxEAGLREAL.h"
#include "AptValueFactory.h"
#include "math.h"
#include "timer.h"
#include "ffapt.h"
#if defined(EA_PLATFORM_PLAYSTATION2)
    #include <sifdev.h>
    #include <eekernel.h>
    #include <libscf.h>
#elif defined(EA_PLATFORM_PSP)
    #include "eagl/runtime/model.h"
    #include "eagl/runtime/texturerc.h"
    #include "eagl/runtime/profiler.h"
    #include "eagl/runtime/rendermethod.h"
    #include "eagl/runtime/transform.h"

    #include "eagl/support/base.h"
    #include "eagl/support/printmessage.h"
    #include "eagl/support/filterediterator.h"
    #include "eagl/support/dlopen.h"

#elif defined(EA_PLATFORM_XBOX)
    #include <xgraphics.h>
#endif

#if defined(EA_PLATFORM_WINDOWS)
// for some reason, pc need to include dlopen.h explicitly
#include "eagl/runtime/eagl.h"
#include "eagl/support/dlopen.h"
#endif

using namespace EAGL;

const AptAuxInitParmsT   gDefaultAptAuxParams;
const AptAuxInitParmsT   *pCurrentAptAuxParams;
extern float gAptFFScaleFactorX;
extern float gAptFFScaleFactorY;
extern int gAptFFScaleAtThisPtSize;

static int gnMaskColour;
static int gnTarNumberForBatchDraw;
static EAGL::ViewPort *gpEAGLView;
static EAGL::RenderContext *gpEAGLRenderContext;
#if defined(EA_PLATFORM_PLAYSTATION2)
static COORD4 gfColourModulator __attribute__((aligned (16))); // ARGB
static COORD4 gfColourTranslate __attribute__((aligned (16))); // ARGB
static DrawClipTextured *gpImmedTexturedMethod;
static DrawClipGouraud *gpImmedGouraudMethod;
#elif defined(EA_PLATFORM_PSP)
COORD4 gfColourModulator __attribute__((aligned (16))); // ARGB
COORD4 gfColourTranslate __attribute__((aligned (16))); // ARGB
// fusionchange - added the masking enabled flag so that we can signal to rendermethods that masking is turned on.
int gnMaskingEnabled;
DrawTextured *gpImmedTexturedMethod;
DrawGouraud *gpImmedGouraudMethod;
#else
static COORD4 gfColourModulator; // ARGB
static COORD4 gfColourTranslate;
static DrawTextured *gpImmedTexturedMethod;
static DrawGouraud *gpImmedGouraudMethod;
#endif
static TAR *gapImmedCurTARs[16];
static int gnCurImmedTAR = 0;
static SHAPE *gpEmptyShape = NULL;
static GeoPrimState *gpImmedGeoPrimState;
static float colourScale[4];
static float colourTranslate[4];
static MATRIX4 vertexMatrix;
static MATRIX4 vertexMatrixIntegralTranslate;

// 2004/23/04 DC -- Array Size now pulled from AptAuxInitParmsT
static AptAuxLayerInfo *gLevelInfo; /* Size is AptAuxInitParmsT::iLevelInfoSize */
#if defined(APT_USE_SOUND_OBJECT)
#if !defined( PSP_EMU )
static SNDPLAYOPTS gSPO;
#endif
#endif
static AptAuxFontList *gpFonts;
static char gszFilePrefix[256];
static bool gbNeverClearZ;

#if defined(ALLOW_SAVE_INPUTS) || defined(ALLOW_PLAY_INPUTS) || defined(DO_COVERAGE)

static int fdSavedInputs = -1;
void      *gpSavedInputBuffer = NULL;
int        gnSavedInputBufferSize = 0;

#if defined(EA_PLATFORM_WINDOWS) // Includes files needed for file IO.
#include <fcntl.h>
#include <io.h>
#include <sys/stat.h>
#endif

#endif

#define DEADSTRINGPERFRAME (512)
struct DeadStringPool
{
    int nDeadStrings;
    SHAPE *aDeadStrings[DEADSTRINGPERFRAME];
    DeadStringPool(){nDeadStrings = 0;}
};

void* FontTextureMemPoolAddr = 0;
int FontTextureMemPoolClassID = 0;
// since the dead string can't be deleted after it's not used
// because eagl/hardware may still using it.
// wait DEADSTRINGPOOLS to delete it
#define DEADSTRINGPOOLS (3)
static DeadStringPool aDeadStringPools[DEADSTRINGPOOLS];
static int nCurrentDSPool = 0;

//static Model *apMaskModels[16];       // not needed now as there is a common array
//static MATRIX4 aMaskModelsTransforms[16]; // not needed now as there is a common array
static int nMaskModels;


struct AllocatedString ;
// added this enum to denote which type of asset is this. used in storing masking information
typedef enum AptAuxEAGLREAL_RenderObjType
{
    AptAuxEAGLREAL_RenderObjType_Geom = 0,
    AptAuxEAGLREAL_RenderObjType_String
};

// this stores information about a geometry mask.
struct AptAuxEAGLREAL_GeomMaskInfo
{
    Model * pMaskModel ;
    MATRIX4 MaskModelTransform ;
};

// an array of AptAuxEAGLREAL_MaskInfo is created to store all the masks information.
// used in commonDraw function.
struct AptAuxEAGLREAL_MaskInfo
{
    AptAuxEAGLREAL_RenderObjType eType;
    union
    {
        AptAuxEAGLREAL_GeomMaskInfo GeomMaskInfo ;
        AllocatedString * StringInfo ;
    };
};

static AptAuxEAGLREAL_MaskInfo aMaskInfo[16] ;

//fusionchange - different for psp
#if defined(EA_PLATFORM_PSP)
float _gDepth = -1.0f;
#else
float _gDepth = 3.f;
#endif

EAGL::AlphaTestMethod _atm = ATM_GREATER;
int _atmvalue = 6;
static void _drawStringRaw(AllocatedString *pString) ;
void CommonDraw(void * pRenderObject, AptAuxEAGLREAL_RenderObjType eType, AptMaskRenderOperation eMaskOperation);


/*************************************************************************************************\
    This is the rather quirky ARGB / RGBA / xxxx conversion area. New systems should add
    interpreters here.

\*************************************************************************************************/
#if defined(EA_PLATFORM_PLAYSTATION2)
/// Color Defined abgr8888
#define EAGL_COLOUR(ARGB) (((ARGB & 0x00ff0000) >> 17) \
                          |  (((ARGB & 0xff) >> 1) << 16) \
                          |  (((ARGB & 0x0000ff00) >> 1) & 0xff00) \
                          |  (((ARGB & 0xff000000) >> 1) & 0xff000000))

#define EAGL_GOURAUDCOLOUR(ARGB) (((ARGB & 0x00ff0000) >> 16) \
                          |  (((ARGB & 0xff)) << 16) \
                          |  (((ARGB & 0x0000ff00)) & 0xff00) \
                          |  (((ARGB & 0xff000000) >> 1) & 0xff000000))

#define EAGL_MAKEARGB(a, r, g, b) ((((a+1)>>1) << 24) | (((b)>>1) << 16) | (((g)>>1) << 8) | ((r)>>1))
#define EAGL_MAKEGOURAUDARGB(a, r, g, b) ((((a+1)>>1) << 24) | (b << 16) | (g << 8) | (r))

#elif defined(EA_PLATFORM_XBOX)
/// Color Defined argb8888
#define EAGL_COLOUR(ARGB) (ARGB)
#define EAGL_GOURAUDCOLOUR EAGL_COLOUR
#define EAGL_MAKEARGB(a, r, g, b) (((a)<<24) | ((r) << 16) | ((g) << 8) | (b))
#define EAGL_MAKEGOURAUDARGB EAGL_MAKEARGB

#elif defined(EA_PLATFORM_WINDOWS)
/// Color Defined argb8888
#define EAGL_COLOUR(ARGB) (ARGB)
#define EAGL_GOURAUDCOLOUR EAGL_COLOUR
#define EAGL_MAKEARGB(a, r, g, b) (((a)<<24) | ((r) << 16) | ((g) << 8) | (b))
#define EAGL_MAKEGOURAUDARGB EAGL_MAKEARGB

#elif defined(EA_PLATFORM_GAMECUBE)
/// Color Defined rgba8888
// (LEO) $BUG677 NGC pink screens when using masks (2)
#define EAGL_COLOUR(ARGB) (((ARGB & 0xff000000) >> 24) \
                        |  (((ARGB & 0xff) << 8)) \
                        |  (((ARGB & 0x0000ff00) << 8)) \
                        |  (((ARGB & 0x00ff0000) << 8)))
#define EAGL_GOURAUDCOLOUR EAGL_COLOUR
#define EAGL_MAKEARGB(a, r, g, b) (((r)<<24) | ((g) << 16) | ((b) << 8) | (a))

#define EAGL_MAKEGOURAUDARGB EAGL_MAKEARGB

#elif defined(EA_PLATFORM_PSP)
/// Color Defined abgr8888: [ARGB] -> [ABGR]
#define EAGL_COLOUR(ARGB) ( ((ARGB & 0xff000000))       |   \
                            ((ARGB & 0x00ff0000) >>16)  |   \
                            ((ARGB & 0x0000ff00))       |   \
                            ((ARGB & 0x000000ff) <<16)  )
//
#define EAGL_MAKEARGB(a, r, g, b) ( (a<<24) | (b<<16) | ((g << 8) | (r) )
//
#define EAGL_GOURAUDCOLOUR      EAGL_COLOUR
#define EAGL_MAKEGOURAUDARGB    EAGL_MAKEARGB
//

#endif // EAGL_COLOUR*


/*F************************************************************************************************/
/*!
Allocate memory (and feel dirty afterwards... jeez enough #defines people?).
if APT_ALLOCATION_TRACKING is defined it will tag the allocation with the filename / line number
of the allocating party. Elsewise it just tags the memory with an allocation number.


\Input

\Output

\Notes

\Version
1.0        05/04/04 (DC) Comment creation.
1.1        07/22/04 (DC) Removed header size added on XBOX and NGC... no idea why it was there, but
                         we don't use it and we run fine without it... Also cleaned up the process
                         a little.
*/
/************************************************************************************************F*/
int nAllocCount = 0;
#if defined(APT_ALLOCATION_TRACKING)
void *MemAlloc(size_t nSize, const char *szFile, int nLine)
#else
void *MemAlloc(size_t nSize)
#endif
{
#if defined(EA_PLATFORM_XBOX)
    char szBuf[256];
    #if defined(APT_ALLOCATION_TRACKING)
        const char *p = strrchr(szFile, '\\');
        if (p)
            p++;
        else p =
            szFile;
        sprintf(szBuf, "Apt%d:%s:%d", nAllocCount++, szFile, nLine);
    #else
        sprintf(szBuf, "Apt%d", nAllocCount++);
    #endif

    return MEM_allocalign(szBuf, nSize, ALIGNSIZE, 0, MB_LOW);

#elif defined(EA_PLATFORM_GAMECUBE)
    return MEM_allocalign("Apt", nSize, 32, 0, MB_LOW);
#elif defined(EA_PLATFORM_PSP)
    #if defined (EA_DEBUG)
        char szBuf[256];

        #if defined(APT_ALLOCATION_TRACKING)
            sprintf(szBuf, "Apt(%d):%d:%s:%d", nAllocCount++, nSize, szFile, nLine);

        #else
            sprintf(szBuf, "Apt:%d:%d", nSize, MB_LOW );

        #endif
        return MEM_alloc( szBuf, nSize, MB_LOW);
    #else
        return MEM_alloc( "Apt", nSize, MB_LOW);
    #endif
#else
    return MEM_alloc("Apt", nSize, MB_LOW);
#endif
}

/*F************************************************************************************************/
/*!
wrapper for free function (mirror of MemAlloc). Nothing special here folks.

\Input

\Output

\Notes

\Version
1.0        05/04/04 (DC) Comment creation.

*/
/************************************************************************************************F*/
void MemFree(void *pData)
{
    MEM_free(pData);
}

#if defined(APT_ALLOCATION_TRACKING)
#define APTAUX_MALLOC_BLOCK( size )  MemAlloc((size), __FILE__, __LINE__)
#else
#define APTAUX_MALLOC_BLOCK( size )  MemAlloc((size))
#endif
#define APTAUX_FREE_BLOCK( p, size ) MemFree((p))

extern "C" {

/*F************************************************************************************************/
/*!
default implementation of gAptFuncs.pfnDebugPrint callback function. can be overridden by game team.

\Input

\Output

\Notes

\Version
1.0        05/04/04 (DC) Comment creation.

*/
/************************************************************************************************F*/
void AptAuxEAGLREAL_DebugPrint(const char *szFormat, ...)
{
    // keeping the old function for 0.15.05 release. We will come back fix this later.
    va_list args;
    char szBuf[4096];

    va_start(args, szFormat);
#if defined (_MSC_VER)
    _vsnprintf(szBuf,sizeof(szBuf)/sizeof(szBuf[0]), szFormat, args);
#elif defined (__GNUC__)
    vsnprintf(szBuf,sizeof(szBuf)/sizeof(szBuf[0]), szFormat, args);
#else
    vsprintf(szBuf, szFormat, args);
#endif
    va_end(args);
    printf("%s", szBuf);
#if defined(EA_PLATFORM_XBOX) || defined(EA_PLATFORM_WINDOWS)
    OutputDebugString(szBuf);
#endif

}

/*F************************************************************************************************/
/*!
wrapper for callback function (for game teams) to receive assertFail notifications. This provides
the message, file and line typical of an assert statement.

\Input

\Output

\Notes

\Version
1.0        05/04/04 (DC) Comment creation.

*/
/************************************************************************************************F*/
void _assertFail(const char *msg, const char *file, unsigned int line)
{
    gAptFuncs.pfnAssertFail(msg, file, line);
}

}
/*F************************************************************************************************/
/*!
Default implementation of the _assertFail Function.

\Input

\Output

\Notes

\Version
1.0        05/04/04 (DC) Comment creation.

*/
/************************************************************************************************F*/
void AssertFail(const char *szExpression, const char *szFile, unsigned int nLine)
{
    AptAuxEAGLREAL_DebugPrint("ASSERT FAILED '%s' at %s:%d\n", szExpression, szFile, nLine);

    while(1)
    {
    };
}

/*F************************************************************************************************/
/*!
default implementation of gAptFuncs.pfnDebugSetScreenGrabPending callback function. can be overridden
by game team.

\Input

\Output

\Notes

\Version
1.0        05/04/04 (DC) Comment creation.

*/
/************************************************************************************************F*/
static void DebugSetScreenGrabPending(const char *szScreenGrabName)
{
}

/*F************************************************************************************************/
/*!
default implementation of gAptFuncs.pfnDebugAddSavedInput callback function. can be overridden
by game team.

\Input

\Output

\Notes

\Version
1.0        05/04/04 (DC) Comment creation.

*/
/************************************************************************************************F*/
static void DebugAddSavedInput(AptSavedInputRecord *pRecord, int nSize)
{
#if defined(ALLOW_SAVE_INPUTS) || defined(DO_COVERAGE)

    assert(fdSavedInputs != -1);

    #if defined(EA_PLATFORM_PLAYSTATION2)
        sceWrite(fdSavedInputs, pRecord, nSize);
    #elif defined(EA_PLATFORM_WINDOWS)
        _write(fdSavedInputs, pRecord, nSize);
    #elif defined(EA_PLATFORM_XBOX)
        fwrite(pRecord, nSize,1,(FILE*)fdSavedInputs);
    #endif

#endif
}
/*F************************************************************************************************/
/*!
Sets us up to record SavedInputs to the specified File.

\Input

\Output

\Notes


\Version
1.0        05/04/04 (DC) Comment creation.

*/
/************************************************************************************************F*/
void AptAuxEAGLREAL_RecordInputs(const char * fileName)
{
#if defined(ALLOW_SAVE_INPUTS) || defined(DO_COVERAGE)

#if defined(EA_PLATFORM_PLAYSTATION2)
    if (fdSavedInputs == -1)
    {
        fdSavedInputs = sceOpen(fileName, SCE_RDWR | SCE_TRUNC | SCE_CREAT);
        assert(fdSavedInputs != -1);
    }
#elif defined(EA_PLATFORM_WINDOWS)
    if (fdSavedInputs == -1)
    {
        fdSavedInputs = _open(fileName, _O_CREAT|_O_BINARY|_O_SEQUENTIAL|_O_WRONLY , _S_IREAD | _S_IWRITE  );
        assert(fdSavedInputs != -1);
    }
#elif defined(EA_PLATFORM_XBOX)
    if (fdSavedInputs == -1)
    {
        fdSavedInputs = (int)fopen(fileName, "w");
        assert(fdSavedInputs != -1);
    }
#endif

    AptDebugEnableSavedInputs(true);
#endif
}
/*F************************************************************************************************/
/*!
Sets us up to play the inputs from the specified file.

\Input

\Output

\Notes


\Version
1.0        05/04/04 (DC) Comment creation.

*/
/************************************************************************************************F*/
void AptAuxEAGLREAL_PlayInputs(const char * fileName)
{
#if defined(ALLOW_PLAY_INPUTS) || defined(DO_COVERAGE)
    assert( FILE_exists(fileName) );

    gnSavedInputBufferSize = FILE_size(fileName);

#if defined(APT_ALLOCATION_TRACKING)
    gpSavedInputBuffer = gAptFuncs.pfnMemAlloc(gnSavedInputBufferSize+1, __FILE__, __LINE__);
#else
    gpSavedInputBuffer = gAptFuncs.pfnMemAlloc(gnSavedInputBufferSize+1);
#endif

    if( FILE_loadat( fileName,gpSavedInputBuffer,0)!=gnSavedInputBufferSize)
    {
        printf("\r\rFile Load error!!!!\r\r");
    }

    AptDebugPlaySavedInputs((AptSavedInputRecord*) gpSavedInputBuffer, gnSavedInputBufferSize);
#endif
}




// following are couple of functions/variables related to memory and cycles printing
static bool bPrintProfileAndMemDump = false ;

// this function gets called from viewer if there is command line option as --dumpstats
void AptAuxEAGLREAL_SetDumpFlag(bool bFlag)
{
    bPrintProfileAndMemDump = bFlag ;
}

#if defined (APT_GATHER_MEMORY_METRICS)
char pszMemDumpFileName[128] = "DynamicMem.xml" ;

// this function gets called from viewer if there is command line option as --memdumpfile
void AptAuxEAGLREAL_SetMemDumpFileName(const char * pszFileName)
{
    if (pszFileName)
    {
        strcpy(pszMemDumpFileName, pszFileName) ;
    }
}

// this function gets called from viewer if there is command line option as --cyclesdumpfile
char * AptAuxEAGLREAL_GetMemDumpFileName()
{
    return (pszMemDumpFileName) ;
}
#endif


#if defined (APT_PROF_ENABLED) || defined (APT_DO_PROFILE)
char pszProfileDumpFileName[128] = "Cycles.xml";

#if defined(EA_PLATFORM_XBOX) || defined (EA_PLATFORM_WINDOWS)
FILE *pProfileFile = NULL;
#elif defined(EA_PLATFORM_PLAYSTATION2)
int32_t pProfileFile = 0;
#endif

#if defined (EA_PLATFORM_XBOX) || defined (EA_PLATFORM_WINDOWS)
#define APTAUX_FILESTRINGWRITE(BUFF, FILE) fputs(BUFF, FILE)
#elif defined (EA_PLATFORM_PLAYSTATION2)
#define APTAUX_FILESTRINGWRITE(BUFF, FILE) sceWrite(FILE, BUFF, strlen(BUFF))
#else
#define APTAUX_FILESTRINGWRITE(BUFF, FILE) gAptFuncs.pfnDebugPrint("%s", BUFF);
#endif


void AptAuxEAGLREAL_WriteProfileData(const char *szFormat, ...)
{
    // keeping the old function for 0.15.05 release. We will come back fix this later.
    va_list args;
    char szBuf[4096];

    va_start(args, szFormat);
#if defined (_MSC_VER)
    _vsnprintf(szBuf,sizeof(szBuf)/sizeof(szBuf[0]), szFormat, args);
#elif defined (__GNUC__)
    vsnprintf(szBuf,sizeof(szBuf)/sizeof(szBuf[0]), szFormat, args);
#else
    vsprintf(szBuf, szFormat, args);
#endif
    va_end(args);

    APTAUX_FILESTRINGWRITE(szBuf, pProfileFile);
}
#endif

/*F************************************************************************************************/
/*!
    \Function    AptAuxEAGLREAL_PrintProfileInfo

    \Description
        Function that can be called from viewer to print profile info.

    \Input      bToFile           - true if you want to print it to file.
    \Input      pszProfileFileName- name of the file to which profile data will be written.

    \Output
        None.

    \Notes
        Appropriate suffix will be added to name of the file for each sku.

    \Version    1.0        08/02/05 (ASK) First Version

*/
/************************************************************************************************F*/
void  AptAuxEAGLREAL_PrintProfileInfo(bool bToFile, char * pszProfileFileName)
{
#if defined (APT_PROF_ENABLED) || defined (APT_DO_PROFILE)
    if (bToFile && pszProfileFileName && strlen(pszProfileFileName))
    {
        // print the memory information to CSV file
        char szFileName[128];
        char szFileName1[128];
    #if defined (EA_PLATFORM_XBOX) || defined (EA_PLATFORM_WINDOWS)
        sprintf(szFileName, "d:\\%s", pszProfileFileName) ;
        sprintf(szFileName1, "d:\\%s", "AptCyclesFileDone.txt") ;

        pProfileFile = fopen(szFileName,"wt");
        if(pProfileFile)
    #elif defined (EA_PLATFORM_PLAYSTATION2)
        sprintf(szFileName, "%s%s", "host0:", pszProfileFileName) ;
        sprintf(szFileName1, "%s%s", "host0:", "AptCyclesFileDone.txt") ;

        pProfileFile = sceOpen(szFileName, SCE_RDWR | SCE_TRUNC | SCE_CREAT);
        if(pProfileFile >= 0) // surprisingly some times sceopens returns 0 even if file is successfully opened.
    #endif
        {
            extern void APT_PROFILE_WriteProfile();
            APT_PROFILE_WriteProfile();

            #if defined (EA_PLATFORM_XBOX) || defined (EA_PLATFORM_WINDOWS)
            // write code to dump stuff to file. following is just example.
            fflush(pProfileFile);
            fclose(pProfileFile);

            //so by creating this file AptCyclesFileDone.txt we are indicating the script that we are done with writing the
            // cycles data file. So script can copy the cycles data file.
            pProfileFile = fopen(szFileName1,"wt");
            fflush(pProfileFile);
            fclose(pProfileFile);

            #elif defined (EA_PLATFORM_PLAYSTATION2)
            // write code to dump stuff to file. following is just example.
            sceClose(pProfileFile);

            //so by creating this file AptCyclesFileDone.txt we are indicating the script that we are done with writing the
            // cycles data file. So script can copy the cycles data file.

            pProfileFile = sceOpen(szFileName1, SCE_RDWR | SCE_TRUNC | SCE_CREAT);
            sceClose(pProfileFile);

            #endif
        }
    }
    return;
#endif // #if defined (APT_PROF_ENABLED)
}

void AptAuxEAGLREAL_SetProfileDumpFileName(const char * pszFileName)
{
#if defined (APT_PROF_ENABLED) || defined (APT_DO_PROFILE)
    if (pszFileName)
    {
        strcpy(pszProfileDumpFileName, pszFileName) ;
    }
#endif
}

char * AptAuxEAGLREAL_GetProfileDumpFileName()
{
#if defined (APT_PROF_ENABLED) || defined (APT_DO_PROFILE)
    return (pszProfileDumpFileName) ;
#else
    return NULL ;
#endif
}



#if defined(APT_GATHER_MEMORY_METRICS)
/*F************************************************************************************************/
/*!
    \Function    AptAuxEAGLREAL_PrintMemoryAllocationsInfo

    \Description
        Function that can be called to print memory statistics from viewer code

    \Input      bToFile           - if true save it to file.
    \Input      pszMemoryFileName - name of memory dump file name

    \Output
        None.

    \Notes
        Appropriate prefix will be added to file name according to sku

    \Version    1.0        08/02/05 (ASK) First Version

*/
/************************************************************************************************F*/
void AptAuxEAGLREAL_PrintMemoryAllocationsInfo(bool bToFile, char * pszMemoryFileName)
{
    if (bToFile && pszMemoryFileName && strlen(pszMemoryFileName))
    {
        // print the memory information to CSV file
#if defined (EA_PLATFORM_XBOX) || defined (EA_PLATFORM_WINDOWS)
        char szFileName[128] ;
        char szFileName1[128] ;

        sprintf(szFileName, "d:\\%s", pszMemoryFileName) ;
        sprintf(szFileName1, "d:\\%s", "AptMemFileDone.txt") ;

        FILE *pFile = fopen(szFileName,"wt");
        if(pFile)
        {
            // write code to dump stuff to file. following is just example.
            char szBuf[256];

            size_t TotalBytesUsedNonGC = gpNonGCPoolManager->GetTotalBytesUsed();
            size_t TotalBytesUsedGC = gpGCPoolManager->GetTotalBytesUsed();

            sprintf(szBuf, "Total GC: %d\n", TotalBytesUsedGC);
            fputs(szBuf, pFile);

            sprintf(szBuf, "Total NonGC: %d\n", TotalBytesUsedNonGC);
            fputs(szBuf, pFile);
            sprintf(szBuf, "Total %d", (TotalBytesUsedGC + TotalBytesUsedNonGC));
            fputs(szBuf, pFile);
            fflush(pFile);
            fclose(pFile);

            // do not delete next code as that code create a empty file indicating script that it is done
            // with writing the mem dump file.
            FILE *pFile = fopen(szFileName1,"wt");
            if (pFile)
            {
                fflush(pFile);
                fclose(pFile);
            }
        }
#endif
#if defined (EA_PLATFORM_PLAYSTATION2)
        char szFileName[128] ;
        char szFileName1[128] ;

        sprintf(szFileName, "%s%s", "host0:", pszMemoryFileName) ;
        sprintf(szFileName1, "%s%s", "host0:", "AptMemFileDone.txt") ;

        int nfFile = sceOpen(szFileName, SCE_RDWR | SCE_TRUNC | SCE_CREAT);
        if(nfFile >= 0)
        {
            // write code to dump stuff to file. following is just example.
            char szBuf[256];

            size_t TotalBytesUsedNonGC = gpNonGCPoolManager->GetTotalBytesUsed();
            size_t TotalBytesUsedGC = gpGCPoolManager->GetTotalBytesUsed();

            sprintf(szBuf, "Total GC: %d\n", TotalBytesUsedGC);
            sceWrite(nfFile, szBuf, strlen(szBuf) );

            sprintf(szBuf, "Total NonGC: %d\n", TotalBytesUsedNonGC);
            sceWrite(nfFile, szBuf, strlen(szBuf) );
            sprintf(szBuf, "Total %d", (TotalBytesUsedGC + TotalBytesUsedNonGC));
            sceWrite(nfFile, szBuf, strlen(szBuf) );
            sceClose(nfFile);

            // do not delete next code as that code create a empty file indicating script that it is done
            // with writing the mem dump file.
            int nfFile = sceOpen(szFileName1, SCE_RDWR | SCE_TRUNC | SCE_CREAT);
            if (nfFile >= 0)
            {
                sceClose(nfFile);
            }
        }

#endif
    }
    return ;
}

void AptAuxEAGLREAL_PrintMemMetricsRecursive(AptMemoryMetrics * pTemp) ;

#undef GetClassName

void AptAuxEAGLREAL_PrintMemMetricsRecursive(AptMemoryMetrics * pTemp)
{
    while(pTemp)
    {
        AptAuxEAGLREAL_DebugPrint("<item name=\"%s\" TotalCount=\"%d\" CurrentCount=\"%d\" CurrentBytes=\"%d\" BytesPerAlloc=\"%d\" MaxConcurrentBytes=\"%d\">\n",
            pTemp->GetClassName(),
            pTemp->GetTotalAllocationCount(),
            pTemp->GetCurrentAllocationCount(),
            pTemp->GetBytesCurrentlyAllocated(),
            pTemp->GetCurrentAllocationCount() ? pTemp->GetBytesCurrentlyAllocated() / pTemp->GetCurrentAllocationCount() : 0,
            pTemp->GetMaxBytesAllocatedAtAnyOneTime()
            );

        AptAuxEAGLREAL_PrintMemMetricsRecursive(AptMemoryMetrics::GetChildInstance(pTemp));
        AptAuxEAGLREAL_DebugPrint("</item>\n");
        pTemp = AptMemoryMetrics::GetSiblingInstance(pTemp);
    }
}


void AptAuxEAGLREAL_PrintMemoryAllocationsInfo1()
{
#if defined(APT_GATHER_MEMORY_METRICS)
    AptMemoryMetrics * pTemp = AptMemoryMetrics::GetFirstInstance();

    AptAuxEAGLREAL_DebugPrint("Apt-Memory allocations per Object------------------------------\n");
    AptAuxEAGLREAL_DebugPrint("Object,Total Count,Current Count,Current Bytes,Bytes/allocation,Max Concurrent Bytes\n");
    AptAuxEAGLREAL_PrintMemMetricsRecursive(pTemp);

    AptAuxEAGLREAL_DebugPrint("---------------------------------------------------------------\n");
#endif
    return ;
}



#endif  // #if defined(APT_GATHER_MEMORY_METRICS)

/*F************************************************************************************************/
/*!
\Function    AptAuxEAGLREAL_PlaySavedInputsDone

\Description
Callback implementation for PlaySavedInputsDone

\Input      DoneFlag          -
\Input      pszFailCause      -

\Output
None.

\Notes
None.

\Version    1.0        08/02/05 (ASK) First Version

*/
/************************************************************************************************F*/
void AptAuxEAGLREAL_PlaySavedInputsDone(bool DoneFlag, const char * pszFailCause)
{
    if(DoneFlag)
    {
        APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, "Playing of saved inputs done sucessfully\n");
    }
    else
    {
        APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, "Playing of saved inputs failed!!!!\n");
        APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, "%s\n", pszFailCause);
    }

    if (bPrintProfileAndMemDump)
    {
#if defined (APT_GATHER_MEMORY_METRICS)
        AptAuxEAGLREAL_PrintMemoryAllocationsInfo(true, AptAuxEAGLREAL_GetMemDumpFileName()) ;
        AptAuxEAGLREAL_PrintMemoryAllocationsInfo1() ;
#endif

#if defined (APT_PROF_ENABLED) || defined (APT_DO_PROFILE)
        AptAuxEAGLREAL_PrintProfileInfo(true, AptAuxEAGLREAL_GetProfileDumpFileName()) ;
#endif
    }
}

void AptAuxEAGLREAL_HandleZombie(bool bZombieVectorFull, bool bRemoving, const char *pszMovieclipName, const char *pszFileName)
{
}

/*F************************************************************************************************/
/*!
searches a list of models (shapes) for the nID passed in. This list is basically a list of
geometry files in the .big file.

\Input

\Output

\Notes

This is a linear search with an atoi() call each compare... thus it can become PAINFULLY slow as
the number geometry files grows. Luckily it is only called once, but if fine number grow I can see
this needing some optimization eventually.

\Version
1.0        05/04/04 (DC) Comment creation.

*/
/************************************************************************************************F*/
static Model *_findModel(AptAuxLayerInfo *pLevel, int nID)
{
    DynamicLoader *pLoader = pLevel->pGEOData;

    if (pLoader == 0) return 0;

    void *addr;
    int nIndex = 0;
    while (pLoader->GetNextAddr(EAGL::DynamicLoader::ModelType, nIndex, addr))
    {
        Model *pModel = (Model *)addr;
        const char *szName = pModel->GetName();
        int nModelID = -1;

        nModelID = atoi(szName);
        if (nModelID == nID)
        {
            if (nID > pLevel->nHighestGeometryIndex) pLevel->nHighestGeometryIndex = nID;
            return pModel;
        }
    }
    return 0;
}
/*F************************************************************************************************/
/*!
This basically gets a pointer to each of the shapes in the big file.

\Input

\Output

\Notes

\Version
1.0        05/04/04 (DC) Comment creation.

*/
/************************************************************************************************F*/
static void _getModelPointers(AptAuxLayerInfo *pLevel, Model **apModels, int nSize)
{
    memset(apModels, 0, nSize * sizeof(Model *));

    DynamicLoader *pLoader = pLevel->pGEOData;
    assert(pLoader);

    void *addr;
    int nIndex = 0;
    while (pLoader->GetNextAddr(EAGL::DynamicLoader::ModelType, nIndex, addr))
    {
        Model *pModel = (Model *)addr;
        const char *szName = pModel->GetName();
        int nModelID = -1;

        nModelID = atoi(szName);
        assert(nModelID < nSize);
        apModels[nModelID] = pModel;
    }
}
/*F************************************************************************************************/
/*!
default implementation of gAptFuncs.pfnLoadGeometry callback function. can be overridden
by game team. It just loads the shape data from the big file.

\Input

\Output

\Notes

\Version
1.0        05/04/04 (DC) Comment creation.

*/
/************************************************************************************************F*/
static void _loadGeometry(const char *szBaseName, AptAuxLayerInfo *pLevel)
{
    char szFilename[256];
    strcpy(szFilename, szBaseName);

    void *pData;
    int nOffset, nImageSize;

    strcat(szFilename, ".o");
    const char *bFound = BIG_locateentryz(pLevel->pBigData, szFilename, 0, &nOffset, &nImageSize);
    if (!bFound) return;    // no geoms in big

    // 128 is for alignment, +1 is to work around an EAGL bug that we're scared to take a patch to fix at this point
#if defined(APT_ALLOCATION_TRACKING)
    pLevel->pGeometryCopy__Base = (char *) gAptFuncs.pfnMemAlloc(nImageSize + 128 + 1, __FILE__, __LINE__);
#else
    pLevel->pGeometryCopy__Base = (char *) gAptFuncs.pfnMemAlloc(nImageSize + 128 + 1);
#endif
    unsigned int temp = (unsigned int)pLevel->pGeometryCopy__Base;
    temp +=  0x7F;
    temp &= ~0x7F; // Align to 128 byte boundary. DC -- changed to simpler math.

    pLevel->pGeometryCopy__EAGL_HACK = (char *)temp;

    memcpy(pLevel->pGeometryCopy__EAGL_HACK, (char *)pLevel->pBigData + nOffset, nImageSize);

    // DC -- removed unneeded memset. Replaced with setting last byte to zero. (memcpy above init's the rest).
    pLevel->pGeometryCopy__EAGL_HACK[nImageSize + 1] = '\0';

#if defined(EA_PLATFORM_GAMECUBE)
    DCFlushRange(pLevel->pGeometryCopy__EAGL_HACK, nImageSize);
#elif defined(EA_PLATFORM_PLAYSTATION2)
    FlushCache(WRITEBACK_DCACHE);
#endif
//  pData = (char *)pLevel->pBigData + nOffset;
    pData = pLevel->pGeometryCopy__EAGL_HACK;
    DynamicLoader *pLoader = new DynamicLoader(pData, nImageSize);
    pLevel->pGEOData = pLoader;
}
#if defined(APT_USE_SOUND_OBJECT)
/*F************************************************************************************************/
/*!
default implementation of gAptFuncs.pfnLoadSoundBank callback function. can be overridden
by game team. Load the sound data from the big file.

\Input

\Output

\Notes

\Version
1.0        05/04/04 (DC) Comment creation.

*/
/************************************************************************************************F*/
static void _loadSoundBank(const char *szBaseName, AptAuxLayerInfo *pLevel)
{
#if !defined( PSP_EMU )     // sound not enabled for PSP emu yet
    char szFilename[256];
    int nRetVal;
    strcpy(szFilename, szBaseName);
    strcat(szFilename, ".asf");

    pLevel->soundData.pData = BIG_locatez(pLevel->pBigData, szFilename);
    if (pLevel->soundData.pData == NULL)
        return;

    nRetVal = SNDbankadd(&pLevel->soundData.nHandle, pLevel->soundData.pData);
    assert(nRetVal >= 0);

    SNDplaysetdef(&gSPO);

    strcat(szFilename, "_lut");
    unsigned char *pASFLUT = (unsigned char *)BIG_locate(pLevel->pBigData, szFilename);
    assert(pASFLUT && pASFLUT[0] == 'A' && pASFLUT[1] == 'S' && pASFLUT[2] == 'F' && pASFLUT[3] == 'T');

    pLevel->soundData.nSounds = *(int *)(pASFLUT + 4);
    pLevel->soundData.aSounds = (AptAuxSoundInfo *)(pASFLUT + 8);
#endif
}
#endif
/*F************************************************************************************************/
/*!
default implementation of gAptFuncs.pfnLoadTexture callback function. can be overridden
by game team. Load the texture data from the big file.

\Input

\Output

\Notes

\Version
1.0        05/04/04 (DC) Comment creation.

*/
/************************************************************************************************F*/
static void *loadTexture(AptAnimationUserData userData, int nID)
{
    AptAuxLayerInfo *pLevel = (AptAuxLayerInfo *)userData;
    char szBuf[256];
    sprintf(szBuf, "%s%s", pLevel->szBaseName, SHAPE_EXT);
    void *pShapeFile = (unsigned char *)BIG_locate(pLevel->pBigData, szBuf);
    int i;

    for (i=0; i<SHAPE_count(pShapeFile); i++)
    {
        char szName[16];
        SHAPE *pShape = SHAPE_pointer(pShapeFile, i);
        int nFrameID = atoi(SHAPE_name(pShapeFile, i, szName));
        if (nID == nFrameID)
        {
            return pShape;
        }
    }

    return NULL;
}
/*F************************************************************************************************/
/*!
default implementation of gAptFuncs.pfnFreeTexture callback function. can be overridden
by game team. This does nothing in the default implementation.

\Input

\Output

\Notes

\Version
1.0        05/04/04 (DC) Comment creation.

*/
/************************************************************************************************F*/
static void freeTexture(AptAssetTexture texture)
{
}
/*F************************************************************************************************/
/*!
default implementation of gAptFuncs.pfnbindTexture callback function. can be overridden
by game team. This sets the given texture to the given shape.

\Input

\Output

\Notes

\Version
1.0        05/04/04 (DC) Comment creation.

*/
/************************************************************************************************F*/
static void bindTexture(AptAnimationUserData userData, int nID, AptAssetTexture texture)
{
    char szGeoName[256];
    char szTexName[256];
    Model::var_iterator begin, end, iter;

    AptAuxLayerInfo *pLevel = (AptAuxLayerInfo *)userData;
#if defined(EA_PLATFORM_PLAYSTATION2) || defined(EA_PLATFORM_XBOX)
    sprintf(szGeoName, "ScaleMatrix::%d::TextureMatrix", nID);                  // Changed for EAGL-4.08.07 upgrade: XBOX now uses ScaleMatrix
#elif defined(EA_PLATFORM_GAMECUBE) || defined(EA_PLATFORM_WINDOWS)
    sprintf(szGeoName, "Matrix::%d::TextureMatrix", nID);
#elif defined(EA_PLATFORM_PSP)
    sprintf(szGeoName, "Matrix::%d::TextureMatrix", nID);
#else
#error "define texture matrix string here"
#endif
    sprintf(szTexName, "%d", nID);

    // $bug759 - MCB - Perform NULL check to make sure the object passed in has geometry.
    if(pLevel->pGEOData == NULL)
    {
        return;
    }

    Model *apModels[APTAUX_MAXCHARACTERS];
    _getModelPointers(pLevel, apModels, APTAUX_MAXCHARACTERS);

    SHAPE *shape = (SHAPE *)texture;
    int nTexWidth2 = SHAPE_width(shape);
    int nTexHeight2 = SHAPE_height(shape);
    nTexWidth2 = 2 << (int)(logf((float)(nTexWidth2-1)) / logf(2.f));
    nTexHeight2 = 2 << (int)(logf((float)(nTexHeight2-1)) / logf(2.f));
    MATRIX4 scaleMatrix = {{1.f / nTexWidth2, 0, 0, 0,
        0, 1.f / nTexHeight2, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1}};

    // fix it up yo
    for (int i = 0; i < APTAUX_MAXCHARACTERS; i++)
    {
        Model *pModel = apModels[i];
        if (!pModel) continue;

        begin = pModel->GetVars().find_first(szGeoName);
        end = pModel->GetVars().find_end(szGeoName);

        //setup the TextureMatrix
        for (iter = begin; iter != end; ++iter)
        {
            int N;
            const MATRIX4 *pMatrix;
            (*iter).GetValue(pMatrix,N);

            // scale matrix by width/height
            MATRIX4 out;
            MATRIX4_mult(pMatrix, &scaleMatrix, &out);

            MATRIX4 *pNonConstMatrix = const_cast<MATRIX4*>(pMatrix);
            *pNonConstMatrix = out;
        }

        pModel->SetTexture(szTexName, shape);
    }
}

/*F************************************************************************************************/
/*!
default implementation of gAptFuncs.pfnGetExternVariable callback function. can be overridden
by game team. This typically executes an external command (not supported in viewer).

\Input

\Output

\Notes

\Version
1.0        05/04/04 (DC) Comment creation.

*/
/************************************************************************************************F*/
static void command(const char *szCommand, const char *szParams)
{
    char szBuf[4096];
    sprintf(szBuf, "external command: %s('%s')\n", szCommand, szParams);
    AptAuxEAGLREAL_DebugPrint(szBuf);
}
/*F************************************************************************************************/
/*!
returns the size of the passed file. (Assuming it exists).

\Input

\Output

\Notes
    This function has no defined way to handle a non-existent file. So Caller must verify first!

\Version
1.0        05/04/04 (DC) Comment creation.

*/
/************************************************************************************************F*/
static void *_loadFileSize(const char *szName, int *pnSize)
{
    char szBuf[256]; // Possible Overflow warning! gszFilePrefix itself maxes at 256 chars!
    sprintf(szBuf, "%s%s", gszFilePrefix, szName);
    return FILE_loadsizez(szBuf, pnSize, MB_ALIGN);
}
/*F************************************************************************************************/
/*!
returns true if the stream is in UTF8 format.

\Input

\Output

\Notes

\Version
1.0        05/04/04 (DC) Comment creation.

*/
/************************************************************************************************F*/
static bool isUTF8File(char* stream)
{
    // signature of the UTF-8,
    // I am not sure if this sig is global standard, or just internal sig for ms notepad
    unsigned char *pData =(unsigned char *)stream;
    return pData[0] == 0xef && pData[1] == 0xbb && pData[2] == 0xbf;
}
/*F************************************************************************************************/
/*!
returns true if the stream is in UCS2 (Unicode) format.

\Input

\Output

\Notes

\Version
1.0        05/04/04 (DC) Comment creation.

*/
/************************************************************************************************F*/
static bool isUCS2File(char* stream)
{
    // signature of the Unicode (16bit),
    // I am not sure if this sig is global standard, or just internal sig for ms notepad
    unsigned short*pData =(unsigned short *)stream;
    return pData[0] == 0xfeff;
}
/*F************************************************************************************************/
/*!
returns the number of characters in a UTF8 Buffer. (not of direct correlation to buffer size in
bytes)

\Input

\Output

\Notes

\Version
1.0        05/04/04 (DC) Comment creation.

*/
/************************************************************************************************F*/
static int UTF8Size(unsigned short* buf, int buflen)
{
    int total=0;
    for (int i=0;i<buflen;i++)
    {
        if (buf[i] <= 0x7f)
        {
            total++;
        }
        else if (buf[i] <= 0x7ff)
        {
            total+=2;
        }
        else
        {
            total+=3;
        }
    }
    return total;
}
/*F************************************************************************************************/
/*!
Converts a string from UCS2 to UTF8.

\Input

\Output

\Notes
    Warning, Buffer overflow much? Caller be warned!

\Version
1.0        05/04/04 (DC) Comment creation.

*/
/************************************************************************************************F*/
static void UCS2ToUTF8(unsigned short* src, int buflen,char* dst)
{
    if (buflen == -1)
    {
        unsigned short *p = src;
        buflen = 0;
        while (*p != 0)
        {
            p++;
            buflen++;
        }
        buflen++;
    }
    for(int i=0;i<buflen;i++)
    {
        if (src[i] <= 0x7f)
        {   //0000 0000-0000 007F   0xxxxxxx
            *dst = (char)(src[i]);
            ++dst;
        }
        else if (src[i] <= 0x7ff)
        {   //0000 0080-0000 07FF   110xxxxx 10xxxxxx
            *dst = 0xc0 | ((src[i] & 0x07c0)>> 6);
            ++dst;
            *dst = 0x80| (src[i] & 0x003f);
            ++dst;
        }
        else
        {   //0000 0800-0000 FFFF   1110xxxx 10xxxxxx 10xxxxxx
            *dst = 0xe0 | ((src[i] & 0xf000)>> 12);
            ++dst;
            *dst = 0x80 | ((src[i] & 0x0fc0)>> 6);
            ++dst;
            *dst = 0x80| (src[i] & 0x003f);
            ++dst;
        }
    }
}

/*F************************************************************************************************/
/*!
default implementation of gAptFuncs.pfnGetBytesTotal callback function. can be overridden
by game team. returns the size in bytes of the passed type of file. This callback is used by
action-script to check on a load status of certain objects.

\Input

\Output

\Notes
This function will be called because of execution of getBytesTotal for movieclip, XML, loadVars or Sound.
Out of these XML case is handled by the XML interface exposed by Apt. Game teams either fill up
this function with their code or they can redefine this callback function

\Version
1.0        05/04/04 (DC) Comment creation.

*/
/************************************************************************************************F*/
int  getBytesTotal(const char * szFileName, AptGetBytesEnum eGetBytes)
{
    gAptFuncs.pfnDebugPrint("getBytesTotal - '%s' '%d\n", szFileName, eGetBytes);
    int iRet = 0 ;
    switch(eGetBytes)
    {
    case AptGetBytesEnum_MovieClip:
        {
            // for movieclip option aux library should keep a integer field in its Level structure
            // that keeps track of file size.
            // here iterate thru all the files loaded and then compare the szFileName with filename stored in
            // that structure and return the filesize.
            char szFilename1[256];
            char szBaseFilename[256];
//          unsigned int i;
            assert(strlen(szFileName) < 250);

            const char *pSlash;
            // not strrchr because we have to look for both / and \ (and there's no strrcspn).
            for (pSlash = &szFileName[strlen(szFileName) - 1]; pSlash >= szFileName; pSlash--)
            {
                if (*pSlash == '/' || *pSlash == '\\')
                {
                    break;
                }
            }
            if (pSlash >= szFileName)
            {
                strcpy(szBaseFilename, pSlash+1);
            }
            else
            {
                strcpy(szBaseFilename, szFileName);
            }

            sprintf(szFilename1, "%s%s.big", gszFilePrefix, szFileName);
            if (FILE_exists(szFilename1))
                iRet = FILE_size(szFilename1) ;
        }
        break ;
    case AptGetBytesEnum_Sound:
        {
        }
        break ;
    case AptGetBytesEnum_XML:
        {
            // this should be handled by the interface implementation, so this function will not be
            // executed with this option.
        }
        break ;
    case AptGetBytesEnum_LoadVars:
        {
            // It comes here when LoadVars.load or LoadVars.sendAndLoad command gets executed.
            // loadvars functionality is totally game dependent. It all depends on how games are responding to
            // loadVariables callback function.
            // some games might connect to database to get variables and then return them to Apt
            // some games might load a text file containing url-encoded variables and values.
            // So games teams here have to decide about how they would return the value to this callback
            // for loadVars option.
        }
        break ;
    }

    return iRet ;
}

/*F************************************************************************************************/
/*!
default implementation of gAptFuncs.pfnGetBytesLoaded callback function. can be overridden
by game team. returns the number of bytes successfully loaded so far. This callback is used by
action-script to check on a load status of certain objects.

\Input

\Output

\Notes
This function will be called because of execution of getBytesTotal for movieclip, XML, loadVars or Sound.
Out of these XML case is handled by the XML interface exposed by Apt. Game teams either fill up
this function with their code or they can redefine this callback function

\Version
1.0        05/04/04 (DC) Comment creation.

*/
/************************************************************************************************F*/
int  getBytesLoaded(const char * szFileName, AptGetBytesEnum eGetBytes)
{
    gAptFuncs.pfnDebugPrint("getBytesLoaded - '%s' '%d\n", szFileName, eGetBytes);
    int iRet = 0 ;
    switch(eGetBytes)
    {
    case AptGetBytesEnum_MovieClip:
        {
            // Game Teams can Overwrite with real values, for now we return that all files are done.
            return getBytesTotal(szFileName, eGetBytes);
        }
        break ;
    case AptGetBytesEnum_Sound:
        {
            // Game Teams can Overwrite with real values, for now we return that all files are done.
            return getBytesTotal(szFileName, eGetBytes);
        }
        break ;
    case AptGetBytesEnum_XML:
        {
            // this should be handled by the interface implementation, so this function will not be
            // executed with this option.
        }
        break ;
    case AptGetBytesEnum_LoadVars:
        {
            // It comes here when LoadVars.load or LoadVars.sendAndLoad command gets executed.
            // loadvars functionality is totally game dependent. It all depends on how games are responding to
            // loadVariables callback function.
            // some games might connect to database to get variables and then return them to Apt
            // some games might load a text file containing url-encoded variables and values.
            // So games teams here have to decide about how they would return the value to this callback
            // for loadVars option.

        }
        break ;
    }

    return iRet ;
}


/*F************************************************************************************************/
/*!
default implementation of gAptFuncs.pfnSendVariables callback function. can be overridden
by game team. This handles "send" functionality in action script.

\Input

\Output

\Notes
parameter bIsSendAndLoad denotes whether this call was from LoadVars.send or LoadVars.sendAndLoad.
If call is from sendAndLoad then there will be a successive call to pfnLoadVariables with NULL parameter.
This function should keep track of that and use that in pfnLoadVariables function to give a proper response
back to Apt.

\Version
1.0        05/04/04 (DC) Comment creation.

*/
/************************************************************************************************F*/
void sendVariables(const char * szUrl, const char * szTarget, const char *szMethod, const char *szProp, int bIsSendAndLoad)
{
    gAptFuncs.pfnDebugPrint("sendVariables : '%s' at target '%s ----%s ----%s '\n", szProp, szTarget, szMethod, szUrl);
    return ;
}

/*F************************************************************************************************/
/*!
    default implementation of gAptFuncs.pfnLoadVariables callback function. can be overridden
    by game team. This handles "sendAndLoad" functionality in action script.

    \Input

    \Output

    \Notes
        this call is preceeded by a call to pfnSendVariables.
        This happens in case of LoadVars.sendAndLoad. Apt will first call pfnSendVariables
        and then it will call this function (loadVariablesNULL).
        In this case auxiliary library is expected to send response back to Apt.
        We introduced another pfnLoaVariablesNULL function that gets called exclusively from LoadVars.sendAndLoad
        to avoid confusion about NULL parameter. It is not much different
        except that it will not have any input parameter. We added a extra flag to
        pfnSendVariables function to denote whether there will be a call to pfnLoadVariableNULL.

    \Version
        1.0        05/04/04 (DC) Comment creation.
        1.1        01/11/05 (LEO)
                -updated to use APT_MALLOC_BLOCK

*/
/************************************************************************************************F*/
static AptValue *loadVariablesNULL()
{
    // this call was precedded by a call to pfnSendVariables, and so send the response back to Apt depending on game requirements
    // Here we(Apt Dev team)  are just adding this case to the function so that function will return valid value.
    // users can use this block and write code to send response back to Apt
    gAptFuncs.pfnDebugPrint("loadVariables: called with NULL parameters\n");

    AptValue * pValue = AptValueFactory::CreateString("");
    return pValue;
}

/*F************************************************************************************************/
/*!
default implementation of gAptFuncs.pfnLoadVariables callback function. can be overridden
by game team. This handles "load" functionality in action script.

\Input

\Output

\Notes

\Version
1.0        05/04/04 (DC) Comment creation.

*/
/************************************************************************************************F*/
static AptValue *loadVariables(const char *szCommand)
{
    int size;
    char* pData =(char*)_loadFileSize(szCommand,&size);
    AptValue * pValue;
    if (!pData)
    {
        gAptFuncs.pfnDebugPrint("couldn't loadVariables '%s'\n", szCommand);

        pValue = AptValueFactory::CreateString("");
        return pValue;
    }

    if (isUTF8File(pData))
    {
        // UTF8
        // remove the first 3 bytes sig. and plus a ending zero

        char *pRet = (char *) APTAUX_MALLOC_BLOCK( (size -3 + 1) );
        memcpy(pRet,pData+3,size -3);
        pRet[size -3] = 0;
        pValue = AptValueFactory::CreateString(pRet);
        APTAUX_FREE_BLOCK(pRet, (size-3+1));
    }
    else if (isUCS2File(pData))
    {
        // UCS2
        int sizeInUTF8 = UTF8Size((unsigned short*)(pData+2),(size-2)/2);

        char *pRet = (char *) APTAUX_MALLOC_BLOCK( (sizeInUTF8 + 1) );

        UCS2ToUTF8((unsigned short*)(pData+2),(size-2)/2 , pRet);
        pRet[sizeInUTF8] = 0;
        pValue = AptValueFactory::CreateString(pRet);
        APTAUX_FREE_BLOCK(pRet, (sizeInUTF8 + 1));

    }
    else
    {
        // ansi
        char *pRet = (char *) APTAUX_MALLOC_BLOCK( (size -3 + 1) );
        memcpy(pRet,pData,size-3);
        pRet[size-3] = 0;
        pValue = AptValueFactory::CreateString(pRet);
        APTAUX_FREE_BLOCK(pRet, (size -3 + 1));
    }


    gAptFuncs.pfnMemFreeSize(pData, size);
    return pValue;
}

/*F************************************************************************************************/
/*!
default implementation of gAptFuncs.pfnGetExternVariable callback function. can be overridden
by game team.

\Input

\Output

\Notes

\Version
1.0        05/04/04 (DC) Comment creation.

*/
/************************************************************************************************F*/
static AptValue *getVariableDefault(const char *szVar)
{
    char *szString = "";
    AptValue * pValue;

    if (strcmp(szVar, "in_apt") == 0)
    {
        szString = "1";
    }

    else if (strcmp(szVar, "gPlatform") == 0)
    {

        #if defined (EA_PLATFORM_PLAYSTATION2)
            char szPlatform[20] = "PS2" ;   //_APTVIEWER
        #elif defined (EA_PLATFORM_PSP)
            char szPlatform[20] = "PSP" ;   //_APTVIEWER
        #elif defined (EA_PLATFORM_XBOX)
            char szPlatform[20] = "XBOX" ;  //_APTVIEWER
        #elif defined (EA_PLATFORM_WINDOWS)
            char szPlatform[20] = "PC" ;    //_APTVIEWER
        #elif defined (EA_PLATFORM_GAMECUBE)
            char szPlatform[20] = "GC" ;    //_APTVIEWER
        #endif


        pValue = AptValueFactory::CreateString(szPlatform);
        return pValue;
    }


    pValue = AptValueFactory::CreateString(szString);
    return pValue;

}
#if defined(APT_USE_SOUND_OBJECT)
/*F************************************************************************************************/
/*!
default implementation of gAptFuncs.pfnloadSound callback function. can be overridden
by game team. This loads a sound based on the passed ID.

\Input

\Output

\Notes

\Version
1.0        05/04/04 (DC) Comment creation.
2.0         06/29/04 (ASK) Added extra szName parameter as per Nascar request in 0.16.00 release.
            We are right now ignoring the parameter
*/
/************************************************************************************************F*/
static AptAssetSound loadSound(AptAnimationUserData userData, int nID, const char * szName)
{
#if !defined( PSP_EMU )     // sound not yet enabled for PSP emu
    // we are ignoring szName parameter, but it can be used if required.
    AptAuxLayerInfo *pLevel = (AptAuxLayerInfo *)userData;
    int i;

    // find the requested sound
    for (i=0; i<pLevel->soundData.nSounds; i++)
    {
        if (pLevel->soundData.aSounds[i].nCharID == nID)
        {
            break;
        }
    }
    if (i == pLevel->soundData.nSounds)
    {
#if defined(EA_DEBUG)
        gAptFuncs.pfnDebugPrint("%s(%d): Imported sound not found!\n", __FILE__, __LINE__);
#else
        gAptFuncs.pfnDebugPrint("AptAuxEAGLREAL(%d): Imported sound not found!\n",__LINE__);
#endif
        return 0;
    }

    // XXX nHandle should be removed from pLevel probably??
    pLevel->soundData.aSounds[i].nHandle = pLevel->soundData.nHandle;

    return (AptAssetSound)&pLevel->soundData.aSounds[i];
#else
    return (AptAssetSound)(0);

#endif
}
/*F************************************************************************************************/
/*!
default implementation of gAptFuncs.pfnFreeSound callback function. can be overridden
by game team.

\Input

\Output

\Notes

\Version
1.0        05/04/04 (DC) Comment creation.

*/
/************************************************************************************************F*/
static void freeSound(AptAssetSound sound)
{

}
/*F************************************************************************************************/
/*!
default implementation of gAptFuncs.pfnStartSound callback function. can be overridden
by game team.

\Input

\Output

\Notes

\Version
1.0        05/04/04 (DC) Comment creation.

*/
/************************************************************************************************F*/
static void startSound(AptAssetSound sound, const char * szName)
{
#if !defined( PSP_EMU )     // sound not yet enabled for PSP emu
    AptAuxSoundInfo *pSound = (AptAuxSoundInfo *)sound;

    SNDSYS_entercritical();
    SNDBANK_play(pSound->nHandle, pSound->nPatchID, &gSPO);
    SNDSYS_leavecritical();
#endif
}
/*F************************************************************************************************/
/*!
default implementation of gAptFuncs.pfnStartSoundStream callback function. can be overridden
by game team.

\Input

\Output

\Notes

\Version
1.0        05/04/04 (DC) Comment creation.

*/
/************************************************************************************************F*/
static void startSoundStream(AptAnimationUserData userData, int nID)
{
#if !defined( PSP_EMU )     // sound not yet enabled for PSP emu
    AptAuxLayerInfo *pLevel = (AptAuxLayerInfo *)userData;
    int i;

    // find the requested sound (negative ID for streams)
    for (i=0; i<pLevel->soundData.nSounds; i++)
    {
        if (pLevel->soundData.aSounds[i].nCharID == -nID)
        {
            break;
        }
    }
    assert(i != pLevel->soundData.nSounds);

    SNDSYS_entercritical();
    SNDBANK_play(pLevel->soundData.aSounds[i].nHandle, pLevel->soundData.aSounds[i].nPatchID, &gSPO);
    SNDSYS_leavecritical();
#endif
}
#endif
/*F************************************************************************************************/
/*!
default implementation of gAptFuncs.pfnSetBackgroundColour callback function. can be overridden
by game team.

\Input

\Output

\Notes

\Version
1.0        05/04/04 (DC) Comment creation.

*/
/************************************************************************************************F*/
static void SetBackgroundColour(unsigned int nColour)
{
    gpEAGLView->SetBackgroundColour(EAGL_GOURAUDCOLOUR(nColour));
}

/*F************************************************************************************************/
/*!
default implementation of gAptFuncs.pfnSetVertexMatrix callback function. can be overridden
by game team. All drawing uses the current vertex matrix, and Apt maintains a vertex matrix stack,
(in AptRenderingContext) so no need to worry about saving old matrix for later use.

\Input

\Output

\Notes

\Version
1.0        05/04/04 (DC) Comment creation.

*/
/************************************************************************************************F*/
#if !defined(APT_3D)
static void SetVertexMatrix(AptMatrix *pMatrix)
#else
static void SetVertexMatrix(AptMath::Mat44T *pMatrix)
#endif
{
#if !defined(APT_3D)
    memset(&vertexMatrix, 0, sizeof(vertexMatrix));
    vertexMatrix.m[0] = pMatrix->a;
    vertexMatrix.m[1] = pMatrix->b;
    vertexMatrix.m[4] = pMatrix->c;
    vertexMatrix.m[5] = pMatrix->d;
    vertexMatrix.m[10] = 1.f;
    vertexMatrix.m[12] = pMatrix->tx;
    vertexMatrix.m[13] = pMatrix->ty;
    vertexMatrix.m[15] = 1.f;
    vertexMatrixIntegralTranslate = vertexMatrix;
    vertexMatrixIntegralTranslate.m[12] = (float)((int)vertexMatrix.m[12]);
    vertexMatrixIntegralTranslate.m[13] = (float)((int)vertexMatrix.m[13]);
    vertexMatrixIntegralTranslate.m[14] = (float)((int)vertexMatrix.m[14]);
#else
    memset(&vertexMatrix, 0, sizeof(vertexMatrix));
    vertexMatrix.m[0] = pMatrix->m[AptMath::MAT_A];
    vertexMatrix.m[1] = pMatrix->m[AptMath::MAT_B];
    vertexMatrix.m[2] = pMatrix->m[AptMath::MAT_C];
    vertexMatrix.m[4] = pMatrix->m[AptMath::MAT_D];
    vertexMatrix.m[5] = pMatrix->m[AptMath::MAT_E];
    vertexMatrix.m[6] = pMatrix->m[AptMath::MAT_F];
    vertexMatrix.m[8] = pMatrix->m[AptMath::MAT_G];
    vertexMatrix.m[9] = pMatrix->m[AptMath::MAT_H];
    vertexMatrix.m[10] = pMatrix->m[AptMath::MAT_I];
    vertexMatrix.m[12] = pMatrix->m[AptMath::MAT_X];
    vertexMatrix.m[13] = pMatrix->m[AptMath::MAT_Y];
    vertexMatrix.m[14] = pMatrix->m[AptMath::MAT_Z];
    vertexMatrix.m[15] = 1.f;
    vertexMatrixIntegralTranslate = vertexMatrix;
    vertexMatrixIntegralTranslate.m[12] = (float)((int)vertexMatrix.m[12]);
    vertexMatrixIntegralTranslate.m[13] = (float)((int)vertexMatrix.m[13]);
    vertexMatrixIntegralTranslate.m[14] = (float)((int)vertexMatrix.m[14]);
#endif
}
/*F************************************************************************************************/
/*!
    This transforms a point by the current vertexMatrix.

\Input

\Output

\Notes
    Right now this function is not called anywhere in apt or apt aux...

\Version
1.0        05/04/04 (DC) Comment creation.

*/
/************************************************************************************************F*/
void AptAuxEAGLREAL_TransformPoint(float fInX, float fInY, float *pfOutX, float *pfOutY)
{
    COORD3 v = { fInX, fInY, 0 };
    COORD3 result;
    MATRIX4_vect3mult3(&v, &vertexMatrix, &result);
    *pfOutX = result.x;
    *pfOutY = result.y;
}
/*F************************************************************************************************/
/*!
default implementation of gAptFuncs.pfnSetColourTransform callback function. can be overridden
by game team. All drawing uses the current color transform, and Apt maintains a color transform stack,
(in AptRenderingContext) so no need to worry about saving old colors for later use.


\Input

\Output

\Notes

\Version
1.0        05/04/04 (DC) Comment creation.

*/
/************************************************************************************************F*/
static void SetColourTransform(AptCXForm *pCXForm)
{
    pCXForm->scale.CopyToFloatArray4(colourScale);
    pCXForm->translate.CopyToFloatArray4(colourTranslate);
}
/*F************************************************************************************************/
/*!
transforms the passed color by the current color transform (set by pfnSetColourTransform).

\Input

\Output     the transformed color.

\Notes

\Version
1.0        05/04/04 (DC) Comment creation.
2.0        06/10/04 (LEO) 489 NGC masking not working, FIFA devtrack #1682 fixed

*/
/************************************************************************************************F*/
static unsigned int _transformColour(unsigned int nColour)
{
#if defined(EA_PLATFORM_GAMECUBE)
    float a = (nColour >> 24) / 255.f;
#else
    float a = (nColour >> 24) / 100.f;
#endif
    float r = ((nColour >> 16) & 0xff) / 255.f;
    float g = ((nColour >> 8) & 0xff) / 255.f;
    float b = (nColour & 0xff) / 255.f;
    a *= colourScale[0];
    r *= colourScale[1];
    g *= colourScale[2];
    b *= colourScale[3];
    a += colourTranslate[0];
    r += colourTranslate[1];
    g += colourTranslate[2];
    b += colourTranslate[3];
#if defined(EA_PLATFORM_GAMECUBE)
    nColour = ((unsigned int)(r * 255.f) << 24)
        | ((unsigned int)(g * 255.f) << 16)
        | ((unsigned int)(b * 255.f) << 8)
        | ((unsigned int)(a * 255.f));
#else
    nColour = ((unsigned int)(a *100.f) << 24)
        | ((unsigned int)(r * 255.f) << 16)
        | ((unsigned int)(g * 255.f) << 8)
        | ((unsigned int)(b * 255.f));
#endif
    return nColour;
}

/*F************************************************************************************************/
/*!
default implementation of gAptFuncs.pfnLoadRenderingUnit callback function. can be overridden
by game team. This loads a rendering unit file (a.k.a. a crap load of triangles). These are in the
.big file and are referenced by their ID value (typically 1.ea3 or 234.eac etc).

\Input

\Output

\Notes

\Version
1.0        05/04/04 (DC) Comment creation.

*/
/************************************************************************************************F*/
static AptAssetRenderingUnit loadRenderingUnit(AptAnimationUserData userData, int nID)
{
    AptAuxLayerInfo *pLevel = (AptAuxLayerInfo *)userData;
    return _findModel(pLevel, nID);
}
/*F************************************************************************************************/
/*!
default implementation of gAptFuncs.pfnFreeRenderingUnit callback function. can be overridden
by game team. This Frees a previously loaded rendering unit file. see loadRenderingUnit;

\Input

\Output

\Notes

\Version
1.0        05/04/04 (DC) Comment creation.

*/
/************************************************************************************************F*/
static void freeRenderingUnit(AptAssetRenderingUnit renderingUnit)
{
    assert(renderingUnit);
}

/*F************************************************************************************************/
/*!
default implementation of gAptFuncs.pfnCustomControlRender callback function. can be overridden
by game team If custom controls are used. This allows game teams to render their own (possibly 3d)
controls. Apt only manages these.

\Input

\Output

\Notes

\Version
1.0        05/04/04 (DC) Comment creation.
1.1        03/18/05 (DC) Added arguments for nested MAsk support (igored in EAGLREAL viewer)
1.20       11/05/07 $senzee Changed signature in accordance with Apt 2.05.00 API.  This is a
                     superficial change, since both the mask operation and mask depth can be obtained from the
                     AptRenderInfo structure.
*/
/************************************************************************************************F*/
static void customControlRender(char *szType,
                                char *szTarget,
                                AptAssetRenderingUnit renderingUnit,
                                const char * szCustomProperties,
                                const AptRenderInfo *info)
{
//  gAptFuncs.pfnDebugPrint("custom control render: '%s' at target '%s'\n", szType, szTarget);

    static char szMyBuf[512] ;
//  for(int i = 0 ; i < 512; i++) szMyBuf[i] = 0 ;
    if (strcmp(szMyBuf, szCustomProperties) != 0)
    {
        strcpy(szMyBuf, szCustomProperties) ;
        gAptFuncs.pfnDebugPrint("custom control render: '%s' at target '%s' with properties as %s\n", szType, szTarget, szCustomProperties);
    }

//  AptAuxEAGLREAL_DebugPrint("custom control RU (%p): '%s' at target '%s'\n", renderingUnit, szType, szTarget);
}

static bool customControlUpdate(AptAssetRenderingUnit renderingUnit)
{
    return false;
}
/*F************************************************************************************************/
/*!
This looks like it clears out the current shape with the current color mask in preperation for
drawing. TODO: Look into this and provide better description.
This is called by the CommonDraw routine.

\Input

\Output

\Notes

\Version
1.0        05/04/04 (DC) Comment creation.

*/
/************************************************************************************************F*/
static void _clearZToDepth(float fDepth)
{
    unsigned int nColour = EAGL_GOURAUDCOLOUR(gnMaskColour);
    gpImmedGeoPrimState->SetTextureEnable(false);
    gpImmedGeoPrimState->SetDepthTestMethod(DTM_ALWAYS);
    // fusionchange - for PSP, we need the following to enable proper setting of the Z plane.
#if defined(EA_PLATFORM_PSP)
    EAGL::RenderContextExtension* pRCExt = &EAGL::Device::Get()->GetCurrentRenderContext()->EXTENSION_OBJ;
    //nColour = nColour & 0x40ffffff;
    pRCExt->SetPixelMask(0xffffffff);
    gpImmedGeoPrimState->EXTENSION_OBJ.SetZWritesEnable(true);
    gpImmedGeoPrimState->SetAlphaTestEnable(false);
#else
    gpImmedGeoPrimState->EXTENSION_OBJ.SetZWritesEnable(true);
#endif
#if defined(EA_PLATFORM_PLAYSTATION2)
    gpImmedGeoPrimState->EXTENSION_OBJ.SetDestinationAlphaMode(false);
    gpImmedGeoPrimState->EXTENSION_OBJ.SetDestinationAlphaTest(false);
#endif

    gpImmedGouraudMethod->SetState(*gpImmedGeoPrimState);

    // can't set view to identity, so we instead set inverse transform on model, assuming that we're ortho view with no rot
    EAGL::Transform inv;
    MATRIX4 tx;

    tx = gpEAGLView->GetViewMatrix();
    inv.m = tx;
    inv.Inverse();
    inv.m.m[14] = 0;        // really want a 2d inverse, so do this because we're lame

    gpImmedGouraudMethod->SetModelMatrix(inv.m);
    gpImmedGouraudMethod->Begin(PT_TRIANGLELIST);

    COORD4 c = { 0, 0, fDepth, 1.f };
    float w;
    float h;
    float unused;

    gpEAGLView->GetShape(unused, unused, w, h, unused, unused);
    c.x = 0.f;
    c.y = 0.f;
    gpImmedGouraudMethod->AddVertex(c, nColour);
    c.x = w;
    c.y = 0.f;
    gpImmedGouraudMethod->AddVertex(c, nColour);
    c.x = w;
    c.y = h;
    gpImmedGouraudMethod->AddVertex(c, nColour);
    c.x = 0.f;
    c.y = 0.f;
    gpImmedGouraudMethod->AddVertex(c, nColour);
    c.x = w;
    c.y = h;
    gpImmedGouraudMethod->AddVertex(c, nColour);
    c.x = 0.f;
    c.y = h;
    gpImmedGouraudMethod->AddVertex(c, nColour);

    gpImmedGouraudMethod->End();
    gpImmedGouraudMethod->Flush();
}

/*F************************************************************************************************/
/*!
This looks like it sets up a primative for EAGL. TODO: Look into this and provide better description.
This is called by the CommonDraw routine.

\Input

\Output

\Notes

\Version
1.0        05/04/04 (DC) Comment creation.

*/
/************************************************************************************************F*/
static void _setDepthMethod(Model *pModel, EAGL::DepthTestMethod eDTM)
{
    const EAGL::Model::VariableTable &vartable = pModel->GetVars();
    EAGL::Model::VariableTable::iterator begin, end, iter;

    begin = vartable.find_first("GeoPrimState::State");
    end = vartable.find_end("GeoPrimState::State");
    int numEntries;

    EAGL::GeoPrimState *retrievedState = NULL;

    for (iter = begin; iter != end; ++iter)
    {
        if ((*iter).GetValue(retrievedState,numEntries))
        {
            retrievedState->SetDepthTestMethod(eDTM);
#if !defined(EA_PLATFORM_PSP)
            retrievedState->EXTENSION_OBJ.SetZWritesEnable(true);
#endif
#if defined(EA_PLATFORM_PLAYSTATION2)
            retrievedState->EXTENSION_OBJ.SetDestinationAlphaMode(false);
            retrievedState->EXTENSION_OBJ.SetDestinationAlphaTest(false);
#endif
        }
    }
}

/*F************************************************************************************************/
/*!
Hook for game team to skip the gpEAGLView->ClearViewPort(EAGL::CLEAR_Z) at the end of the draw routine.
This effects the operation of the CommonDraw Routine.

\Input

\Output

\Notes

\Version
1.0        05/04/04 (DC) Comment creation.

*/
/************************************************************************************************F*/
void AptAuxEAGLREAL_NeverClearZ(bool bNeverClear)
{
    gbNeverClearZ = bNeverClear;
}

#define DRAW_BOUNDING_BOX       0
#if(DRAW_BOUNDING_BOX)
void drawDebugBoundingBox(Model *pObj, unsigned int cColor)
{
    unsigned int CLR = EAGL_GOURAUDCOLOUR(cColor);
    COORD3 bBoxMin;
    COORD3 bBoxMax;
    pObj->GetBoundingBox(bBoxMin, bBoxMax);

    COORD4 p1 = {bBoxMin.x, bBoxMax.y, 0.f, 1.f};
    COORD4 p2 = {bBoxMax.x, bBoxMax.y, 0.f, 1.f};
    COORD4 p3 = {bBoxMax.x, bBoxMin.y, 0.f, 1.f};
    COORD4 p4 = {bBoxMin.x, bBoxMin.y, 0.f, 1.f};

    gpImmedGeoPrimState->SetTextureEnable(false);
    gpImmedGouraudMethod->SetState(*gpImmedGeoPrimState);
    gpImmedGouraudMethod->SetModelMatrix(vertexMatrix);
    gpImmedGouraudMethod->Begin(PT_LINELIST);
    gpImmedGouraudMethod->AddVertex(p1, CLR);
    gpImmedGouraudMethod->AddVertex(p2, CLR);
    gpImmedGouraudMethod->AddVertex(p2, CLR);
    gpImmedGouraudMethod->AddVertex(p3, CLR);
    gpImmedGouraudMethod->AddVertex(p3, CLR);
    gpImmedGouraudMethod->AddVertex(p4, CLR);
    gpImmedGouraudMethod->AddVertex(p4, CLR);
    gpImmedGouraudMethod->AddVertex(p1, CLR);
    gpImmedGouraudMethod->End();
    gpImmedGouraudMethod->Flush();
}
#endif

/*F************************************************************************************************/
/*!
Draws a shape object from Apt. This should be in the ea3 format. Eventually this filters down into
the commonDraw Routine. This has to be aware of minor platform differences, especially in RGBA quirks.

\Input

\Output

\Notes

\Version
1.0        05/04/04 (DC) Comment creation.
2.0        06/10.04 (LEO) Removed the XBOX Alpha bug fix, because the AptGouraud rendermethod was fixed in EAGL.
2.1        03/18/05 (DC) Added arguments for nested MAsk support (igored in EAGLREAL viewer)
2.20       11/05/07 $senzee Changed signature in accordance with Apt 2.05.00 API.  This is a
                     superficial change, since both the mask operation and mask depth can be obtained from the
                     AptRenderInfo structure.

*/
/************************************************************************************************F*/
static void drawRenderingUnit(AptAssetRenderingUnit renderingUnit, const AptRenderInfo *info)
{
    if (info->meMaskOper == AptMaskRenderOperation_None)
    {
        Model *pModel = (Model *) renderingUnit;
        gfColourModulator.x = colourScale[1]; // R
        gfColourModulator.y = colourScale[2]; // G
        gfColourModulator.z = colourScale[3]; // B
#if defined(EA_PLATFORM_GAMECUBE)
        gfColourModulator.w = colourScale[0] > 1.f ? 1.f : colourScale[0]; // A
#elif defined(EA_PLATFORM_PSP)
        gfColourModulator.w = colourScale[0] > 1.f ? 1.f : colourScale[0]; // A
#else
        gfColourModulator.w = colourScale[0]; // A
#endif
        gfColourTranslate.x = colourTranslate[1]; // R
        gfColourTranslate.y = colourTranslate[2]; // G
        gfColourTranslate.z = colourTranslate[3]; // B
        gfColourTranslate.w = colourTranslate[0]; // A

        pModel->Draw(vertexMatrix);
        #if(DRAW_BOUNDING_BOX)                      // (LEO) when defined, all objects will have a boundary box drawn to it..
        {
            drawDebugBoundingBox(pModel, 0xffff00CC);
        }
        #endif
    }
    else
    {
        // if it is a mask operation then instead of keeping separate track of masks for geometry and strings
        // keep a common array of both AptAuxEAGLREAL_GeomMaskInfo and call commonDraw that does same work
        // as following blovk of code was doing for geometry.
        CommonDraw((void *)renderingUnit, AptAuxEAGLREAL_RenderObjType_Geom, info->meMaskOper);
    }
}
/*F************************************************************************************************/
/*!
this gets called from drawRenderingUnit or drawStringFF in case if mask operation is not None
it keeps track of all the masks in a combined array of masks and then draws them accordingly.
drawing operation for geometry is same as it was previously in drawRenderingUnit function.
extra code is added for case if mask is a string-texture.

\Input

\Output

\Notes
previous comment states:
    Right now for 0.11.00 release - this code is not working fine if there are consecutive pairs of
    masks placed in a swf file. if a buffer layer having some geometry/text/texture which is not a
    part of any mask or being masked then masking works fine. Individual cases of masking placed on
    to stage with no other masks on stage also work fine. This has to be fixed when we upgrade to
    EAGL 4.8.7 with new rendermethods.

\Version
1.0        05/04/04 (DC) Comment creation.

*/
/************************************************************************************************F*/
void CommonDraw(void * pRenderObject, AptAuxEAGLREAL_RenderObjType eType, AptMaskRenderOperation eMaskOperation)
{
    Model *pModel = (Model *)pRenderObject;
    if (eMaskOperation == AptMaskRenderOperation_Add)
    {
        aMaskInfo[nMaskModels].eType = eType ;
        if (eType == AptAuxEAGLREAL_RenderObjType_Geom)
        {
            aMaskInfo[nMaskModels].GeomMaskInfo.pMaskModel = (Model *) pRenderObject ;
            aMaskInfo[nMaskModels].GeomMaskInfo.MaskModelTransform = vertexMatrix ;
        }
        else
        {
            aMaskInfo[nMaskModels].StringInfo = (AllocatedString *) pRenderObject ;
        }
        nMaskModels++;
    }
    else
    {
        int i;
        for (i = 0; i < nMaskModels; i++)
        {
            void * pTempRenderObj = (void *) aMaskInfo[i].GeomMaskInfo.pMaskModel ; // it might be of string type also.
            if (pTempRenderObj == pRenderObject)
            {
                memmove(&aMaskInfo[i], &aMaskInfo[i+1], sizeof(aMaskInfo[i]) * (nMaskModels - i));
                break;
            }
        }
        assert(nMaskModels != 0); // There are no Mask Models!
        assert(i != nMaskModels);
        if(nMaskModels > 0)       // Prevent crash in Release if this happens (We hit the assert in Debug)
            nMaskModels--;
    }

    if (nMaskModels > 0)
    {
        int i;
        {
#if defined(EA_PLATFORM_PLAYSTATION2)
        gpEAGLRenderContext->EXTENSION_OBJ.SetRenderMask(0xffffffff);
#elif defined(EA_PLATFORM_XBOX)
        gpEAGLRenderContext->EXTENSION_OBJ.SetColorWriteEnable(CWM_NONE);
#elif defined(EA_PLATFORM_WINDOWS)
        gpEAGLRenderContext->EXTENSION_OBJ.SetColorWriteEnable(0);
#elif defined(EA_PLATFORM_GAMECUBE)
        gpEAGLRenderContext->EXTENSION_OBJ.SetColourWritesEnable(0);
#endif

        gfColourModulator.x = 1.f;
        gfColourModulator.y = 1.f;
        gfColourModulator.z = 1.f;
        gfColourModulator.w = 1.f;
        gfColourTranslate.x = 0.f;
        gfColourTranslate.y = 0.f;
        gfColourTranslate.z = 0.f;
        gfColourTranslate.w = 0.f;
        }
        _clearZToDepth(_gDepth); // actually 3 "farther" than 0 + modelview
        for (i = 0; i < nMaskModels; i++)
        {
            if (aMaskInfo[i].eType == AptAuxEAGLREAL_RenderObjType_Geom)
            {
                // this code is kept same as it was previously before masking of dynamic text was not supported.
                pModel = (Model *) aMaskInfo[i].GeomMaskInfo.pMaskModel ;

                // fusionchange - for PSP, _setDepthMethod does not work, as we remove all model symbols
                // with a call to DynamicLoader::Release() immediately after load. Thus, we have to
                // override the DTM inside of the rendermethod itself using the gnMaskingEnabled flag.
#if defined(EA_PLATFORM_PSP)
                gnMaskingEnabled = 1;
#else
                _setDepthMethod(pModel, DTM_ALWAYS);
#endif

                pModel->Draw(aMaskInfo[i].GeomMaskInfo.MaskModelTransform);

                // fusionchange - added the gnMaskingEnabled flag; see above for details.
#if defined(EA_PLATFORM_PSP)
                gnMaskingEnabled = 0;
#else
    #if defined(PS2)
                _setDepthMethod(pModel, DTM_GEQUAL);
#else
                _setDepthMethod(pModel, DTM_LEQUAL);
#endif
#endif
            }
            else
            {
                // this is string
                gpImmedGeoPrimState->SetTextureEnable(true);
                gpImmedGeoPrimState->SetDepthTestMethod(DTM_ALWAYS);
                gpImmedTexturedMethod->SetState(*gpImmedGeoPrimState);

                _drawStringRaw(aMaskInfo[i].StringInfo);

#if defined(EA_PLATFORM_PLAYSTATION2)
                gpImmedGeoPrimState->SetDepthTestMethod(DTM_GEQUAL);
#else
                gpImmedGeoPrimState->SetDepthTestMethod(DTM_LEQUAL);
#endif

                gpImmedGeoPrimState->SetTextureEnable(false);
                gpImmedTexturedMethod->SetState(*gpImmedGeoPrimState);

            }
        }
        {

#if defined(EA_PLATFORM_PLAYSTATION2)
        gpEAGLRenderContext->EXTENSION_OBJ.SetRenderMask(0x00000000);
#elif defined(EA_PLATFORM_XBOX)
        gpEAGLRenderContext->EXTENSION_OBJ.SetColorWriteEnable(CWM_ALL);
#elif defined(EA_PLATFORM_WINDOWS)
        gpEAGLRenderContext->EXTENSION_OBJ.SetColorWriteEnable(D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE | D3DCOLORWRITEENABLE_ALPHA);
#elif defined(EA_PLATFORM_GAMECUBE)
        gpEAGLRenderContext->EXTENSION_OBJ.SetColourWritesEnable(1);
#elif defined(EA_PLATFORM_PSP)
        gpEAGLRenderContext->EXTENSION_OBJ.SetPixelMask(0x00000000);
#endif

        }
    }
    else
    {
        if (!gbNeverClearZ)
        {
            gpEAGLView->ClearViewPort(EAGL::CLEAR_Z);
        }
    }

    #if(DRAW_BOUNDING_BOX)                      // (LEO) when defined, all objects will have a boundary box drawn to it..
    {
        drawDebugBoundingBox(pModel, 0xffff00CC);
    }
    #endif
}





/*F************************************************************************************************/
/*!
TODO: Look into this and provide better description.

\Input

\Output

\Notes

\Version
1.0        05/04/04 (DC) Comment creation.

*/
/************************************************************************************************F*/
static SHAPE *_createShape(int nWidth, int nHeight, int nShapeType, bool bMipMap,int MB_FLAG)
{
    int nShapeDataSize = 0;
    SHAPE *pShape = NULL;
    int nClutBpp = 0;
    int nNumMipmaps = 0;
    int nBpp = 0;

    if (bMipMap)
    {
        nNumMipmaps = intlog2(MIN(nWidth, nHeight));
#if defined(EA_PLATFORM_PLAYSTATION2)
        // minimum size for mipmap is 8xN or Nx8 on the PS2
        nNumMipmaps -= 3;
#endif
    }

    switch (nShapeType & 0x7f)
    {
        case SHP_P_8:
#if defined(EA_PLATFORM_GAMECUBE)
            nClutBpp = 15;
#else
            nClutBpp = 32;
#endif
            nBpp = 8;
        break;
        case SHP_P_4:
#if defined(EA_PLATFORM_GAMECUBE)
            nClutBpp = 15;
#else
            nClutBpp = 32;
#endif
            nBpp = 4;
        break;
        case XSH_A_8:
            nBpp = 8;
        break;

        case FSH_ARGB_1555: // == case XSH_ARGB_1555:
        case SSH_ARGB_1555:
        case FSH_RGB_565: // == case XSH_RGB_565:
        case GSH_RGB_565:
        case FSH_ARGB_4444: // == case XSH_ARGB_4444:
        case GSH_AL_88:
            nBpp = 16;
        break;

        case SHP_ARGB_8888:
            nBpp = 32;
        break;

        // error: unknown shape type
        default:
            gAptFuncs.pfnDebugPrint("error: unknown shape type '%d'\n", nShapeType);
            //printf("error: unknown shape type '%d'\n", nShapeType);
            assert(0);
        break;
    }
#if defined(EA_PLATFORM_GAMECUBE)
    // there is a bug in real before 5.18
    // if I pass 15 bit as 4th paramter, real can't calc the clutsize correctly, and lead to a
    // crash in release build. Margolin, Pavel will fix it in 5.18
    // for now, here is the work around.
    nShapeDataSize = SHAPE_createsize(nWidth, nHeight, nBpp, 16, nNumMipmaps, 0, METALBINSIZE);
#else
    nShapeDataSize = SHAPE_createsize(nWidth, nHeight, nBpp, nClutBpp, nNumMipmaps, 0, METALBINSIZE);
#endif

    pShape = (SHAPE *)MEM_allocalign("FONTSHAPE", nShapeDataSize + sizeof(SHAPE), 128, sizeof(SHAPE), FontTextureMemPoolClassID | MB_FLAG);
    assert(pShape != NULL);
    memset(pShape, 0, nShapeDataSize);

    SHAPE_createat(pShape, nWidth, nHeight, nBpp, nClutBpp, nNumMipmaps, 0, METALBINSIZE);
#if defined(EA_PLATFORM_XBOX)
    pShape->swizzled = 1;
#elif defined(EA_PLATFORM_PLAYSTATION2)
    char *szMetal = (char *)SHAPE_metalbin(pShape);
    szMetal[4] = '2';
    szMetal[5] = '4';
    szMetal[6] = '0';
#endif
    pShape->type = nShapeType;

    return pShape;
}

#if 0
struct BadSizes
{
    int nSourceWidth, nSourceHeight;
    int nFixedWidth, nFixedHeight;
};

static BadSizes aBadSizes[] =
{
    { 64, 512,  /* to */    128, 512 },
    { 64, 256,  /* to */    128, 256 },
    { 64, 128,  /* to */    128, 128 },
    { 64, 16,   /* to */    64, 32 },
    { 64, 8,    /* to */    64, 32 },
    { 64, 4,    /* to */    64, 32 },
    { 64, 2,    /* to */    64, 32 },
    { 64, 1,    /* to */    64, 32 },
    { 32, 512,  /* to */    128, 512 },
    { 32, 256,  /* to */    128, 256 },
    { 32, 128,  /* to */    128, 128 },
    { 32, 64,   /* to */    64, 64 },
    { 32, 8,    /* to */    32, 16 },
    { 32, 4,    /* to */    32, 16 },
    { 32, 2,    /* to */    32, 16 },
    { 32, 1,    /* to */    32, 16 },
    { 16, 512,  /* to */    128, 512 },
    { 16, 32,   /* to */    32, 32 },
};
#endif // #if 0

#if defined(EA_PLATFORM_PLAYSTATION2)
    #define     SYS_BCDToDecimal(_x_)   (((((_x_) & 0xF0 )>> 4) * 10) + ((_x_) & 0x0F))
#endif

/*F************************************************************************************************/
/*!
default implementation of gAptFuncs.pfnGetRealTimeClock callback function. can be overridden
by game team. Returns the system time.

\Input

\Output

\Notes

\Version
1.0        05/04/04 (DC) Comment creation.

*/
/************************************************************************************************F*/
void AptAuxEAGLREAL_GetRealTimeClock(AptSysClock *pClock, bool bLocal)
{
#if defined(EA_PLATFORM_PLAYSTATION2)
    sceCdCLOCK  rtc;
    sceCdReadClock(&rtc);
    if(bLocal)
        sceScfGetLocalTimefromRTC(&rtc);
    else
        sceScfGetGMTfromRTC(&rtc);

    // no vblank interrupt while initializing clock
    //DisableIntc(INTC_VBLANK_E);
    pClock->Second = SYS_BCDToDecimal(rtc.second);
    pClock->Minute = SYS_BCDToDecimal(rtc.minute);
    pClock->Hour = SYS_BCDToDecimal(rtc.hour);
    pClock->Date = SYS_BCDToDecimal(rtc.day);
    pClock->Month = SYS_BCDToDecimal(rtc.month) - 1;
    pClock->Year = SYS_BCDToDecimal(rtc.year) + 2000;
    pClock->Day = 0;
    pClock->Hundredths = 0;
    //EnableIntc(INTC_VBLANK_E);
#endif
#if defined(EA_PLATFORM_GAMECUBE)
    bool bIntsEnabled;
    OSCalendarTime td;
    OSTicksToCalendarTime(OSGetTime(), &td);
    bIntsEnabled = OSDisableInterrupts();
    pClock->Second = td.sec;
    pClock->Minute = td.min;
    pClock->Hour = td.hour;
    pClock->Year = td.year;
    pClock->Month = td.mon - 1;
    pClock->Date = td.mday;
    pClock->Day = td.wday;
    pClock->Hundredths = td.msec;
    OSRestoreInterrupts(bIntsEnabled);
#endif
#if defined(EA_PLATFORM_XBOX)
    SYSTEMTIME systime;

    if(bLocal)
        GetLocalTime(&systime);
    else
        GetSystemTime(&systime);
    pClock->Second = systime.wSecond;
    pClock->Minute = systime.wMinute;
    pClock->Hour = systime.wHour;
    pClock->Date = systime.wDay;
    pClock->Day = systime.wDayOfWeek;
    pClock->Month = systime.wMonth - 1;
    pClock->Year = systime.wYear;
    pClock->Hundredths = systime.wMilliseconds;
#endif
#if defined(EA_PLATFORM_WINDOWS)
    SYSTEMTIME systime;

    if(bLocal)
        GetLocalTime(&systime);
    else
        GetSystemTime(&systime);

    pClock->Second = systime.wSecond;
    pClock->Minute = systime.wMinute;
    pClock->Hour = systime.wHour;
    pClock->Date = systime.wDay;
    pClock->Day = systime.wDayOfWeek;
    pClock->Month = systime.wMonth - 1;
    pClock->Year = systime.wYear;
    pClock->Hundredths = systime.wMilliseconds;
#endif
}
/*F************************************************************************************************/
/*!
creates a texture with the specified color that is at least the size of the passed in width and
height (although possibly much larger than them).

\Input

\Output

\Notes

\Version
1.0        05/04/04 (DC) Comment creation.

*/
/************************************************************************************************F*/
SHAPE *AptAuxEAGLREAL_CreateFontTexture(int *pnWidth, int *pnHeight, unsigned int nARGBColour,int MB_FLAG)
{
    int i;
#if !defined(EA_PLATFORM_GAMECUBE) && !defined(EA_PLATFORM_PLAYSTATION2)  && !defined(EA_PLATFORM_PSP)
    int j;
#endif
    int nType = SHP_P_8;

#if defined (EA_PLATFORM_PLAYSTATION2)
    int nW = (*pnWidth + 7)/8*8;
    int nH = (*pnHeight + 7)/8*8;
#else
    int nW = 2 << (int) (logf(static_cast<float>(*pnWidth) - 1) / logf(2.0f));
    int nH = 2 << (int) (logf(static_cast<float>(*pnHeight) - 1) / logf(2.0f));
#endif
#if 0 && defined(EA_PLATFORM_PLAYSTATION2)
    for (i = 0; i < (int)(sizeof(aBadSizes)/sizeof(aBadSizes[0])); i++)
    {
        if (nW == aBadSizes[i].nSourceWidth && nH == aBadSizes[i].nSourceHeight)
        {
            nW = aBadSizes[i].nFixedWidth;
            nH = aBadSizes[i].nFixedHeight;
#if defined(EA_DEBUG)
            printf("upsizing yucky sized texture: %dx%d -> %dx%d\n", *pnWidth, *pnHeight, nW, nH);
#endif
            break;
        }
    }

    if (i == (int)(sizeof(aBadSizes)/sizeof(aBadSizes[0])))
    {
        nW = *pnWidth;
        nH = *pnHeight;
    }
#endif
    SHAPE *pShape = _createShape(nW, nH, nType, false,MB_FLAG);

    if (nType == SHP_P_8)
    {
        SHAPE *pClut = SHAPE_clut(pShape);
#if defined(EA_PLATFORM_GAMECUBE)
        int nColours = 256;
        unsigned short *clutdata = (unsigned short *) SHAPE_data(pClut);
        short  color = (nARGBColour & 0x00f00000) >> 12;                // Bug 372 change
        color |= (nARGBColour & 0x0000f000) >> 8;
        color |= (nARGBColour & 0x000000f0) >> 4;

        for (i=0; i<nColours; ++i)
        {
            clutdata[i] = color | (i/32)<<13;
        }
        DCFlushRange(clutdata,256*sizeof(unsigned short));
#elif defined (EA_PLATFORM_PSP)
        int nColours = 256;
        unsigned int *clutdata = (unsigned int *) SHAPE_data(pClut);
        // ARGB -> ABGR
        unsigned int color = (unsigned int) EAGL_COLOUR( nARGBColour );

        #if defined(DEBUG_AUX)
        //sceEmuPrintf("<info msg='clut: color(0x%08x:0x%08x) W(%d) H(%d)'/>\n", nARGBColour, color, nW, nH );
        #endif

        for (i=0; i<nColours; ++i)
        {
            clutdata[i] = color + (((unsigned int)i) << 24);
        }
#elif defined (EA_PLATFORM_PLAYSTATION2)
        int nColours = 128;
        ARGB *clutdata = (ARGB *) SHAPE_data(pClut);
        for (i=0; i<nColours; ++i)
        {
            *((unsigned int *)&clutdata[i]) = EAGL_COLOUR((unsigned int)(nARGBColour & 0xffffff) + (((unsigned int)i) << 24));
        }
#else
        int nStepSize = 2;
        int nColours = 127;
        ARGB *clutdata = (ARGB *) SHAPE_data(pClut);
        for (i=0, j=0; i<nColours; ++i, j+=nStepSize)
        {
            *((unsigned int *)&clutdata[i]) = EAGL_COLOUR((unsigned int)(nARGBColour & 0xffffff) + (((unsigned int)j) << 24));
        }
#endif
    }

    *pnWidth = nW;
    *pnHeight = nH;

    return pShape;
}

struct AllocatedString
{
    SHAPE *pShape;
    float fLeft, fTop, fRight, fBottom;
    float fTextWidth, fTextHeight;
    float fTexWidth, fTexHeight;
    float fYAscender;
    AptStringAlignment eAlignment;
    AptStringAlignment eBoxAlignment;
    unsigned int nBackColor;
    unsigned int nBorderColor;
    unsigned int nPtSize:8;
    unsigned int bBackground:1;
    unsigned int bBorder:1;
    unsigned int bUsed:1;
    unsigned int bHasBeenDrawn:1; // DC Added Flag to see if allocated string has been drawn at least once.

    unsigned int nMaxScroll;
    // $FR#598 CSB - Added extra parameters for Font Style, Indent, LeftMargin, and RightMargin 23/36
    unsigned int nFontStyle;
    int          nIndent;
    int          nLeftMargin;
    int          nRightMargin;
#if defined  (EA_DEBUG)
    char            szText[32];
#endif
    // $FR#598 CSB - Setting Font Style, Indent, LeftMargin, and RightMargin 24/36
    AllocatedString() : pShape(0), fLeft(0), fTop(0), fRight(0), fBottom(0), fTextWidth(0), fTextHeight(0), fTexWidth(0), fTexHeight(0), fYAscender(0),
        eAlignment(AptStringAlignment_Left), eBoxAlignment(AptStringAlignment_Left), nBackColor(0), nBorderColor(0), nPtSize(0), bBackground(0), bBorder(0), bUsed(0),
        bHasBeenDrawn(0), nMaxScroll(0), nFontStyle(0x00000000), nIndent(0), nLeftMargin(0), nRightMargin(0)
        {}
};
#if defined(EA_PLATFORM_WINDOWS)
SHAPE* From8BitsTo16Bits(SHAPE* srcShape)
{
    int width = SHAPE_width(srcShape);
    int height = SHAPE_height(srcShape);

    SHAPE* sixteenbit = _createShape(width,height,FSH_ARGB_4444,false,MB_LOW);
    unsigned short* destdata = (unsigned short*)SHAPE_data(sixteenbit);
    char* srcdata = (char*)SHAPE_data(srcShape);
    SHAPE* srcClut = SHAPE_clut(srcShape);
    unsigned int *srcclutdata = (unsigned int *) SHAPE_data(srcClut);
    int ii,jj;
    for(ii=0;ii<height;ii++)
    for(jj=0;jj<width;jj++)
    {
        unsigned char srcIndex= *(srcdata+width*ii+jj);
        unsigned int srcColor = srcclutdata[srcIndex];
        unsigned short destColor =  ((srcColor&0xf0000000)>>16) |
                                    ((srcColor&0x00f00000)>>12) |
                                    ((srcColor&0x0000f000)>>8) |
                                    ((srcColor&0x000000f0)>>4);
        *(destdata+width*ii+jj) = destColor;
    }
    return sixteenbit;

}
#endif

#if defined(EA_PLATFORM_GAMECUBE)
SHAPE* From8BitsTo4Bits(SHAPE* srcShape)
{
    int width = SHAPE_width(srcShape);
    int height = SHAPE_height(srcShape);

    SHAPE* fourbit = _createShape(width,height,SHP_P_4,false,MB_LOW);
    char* destdata = (char*)SHAPE_data(fourbit);
    char* srcdata = (char*)SHAPE_data(srcShape);
    int ii,jj;
    int picth=width/2;
    for(ii=0;ii<height;ii++)
    for(jj=0;jj<picth;jj++)
    {
        *(destdata+picth*ii+jj) = ((*((srcdata+width*ii)+jj*2)&0xf0)|(*((srcdata+width*ii)+jj*2+1)>>4));
    }
    SHAPE* destClut = SHAPE_clut(fourbit);
    SHAPE* srcClut = SHAPE_clut(srcShape);

    int nColours = 16;
    unsigned short *destclutdata = (unsigned short *) SHAPE_data(destClut);
    unsigned short *srcclutdata = (unsigned short *) SHAPE_data(srcClut);
    for (ii=0; ii<nColours; ++ii)
    {
        destclutdata[ii] = srcclutdata[ii*16+15];
    }
    DCFlushRange(destclutdata,16*sizeof(unsigned short));
    return fourbit;
}
#endif

#if defined(EA_PLATFORM_PSP)
SHAPE* From8BitsTo16Bits(SHAPE* srcShape)
{
    int width = SHAPE_width(srcShape);
    int height = SHAPE_height(srcShape);

    SHAPE* fourbit = _createShape(width,height,SHP_P_4,false,MB_LOW);
    char* destdata = (char*)SHAPE_data(fourbit);
    char* srcdata = (char*)SHAPE_data(srcShape);
    int ii,jj;
    int picth=(width)/2;
    for(ii=0;ii<height;ii++)
    for(jj=0;jj<picth;jj++)
    {
        *(destdata+picth*ii+jj) = ((((*((srcdata+width*ii)+jj*2+1))*2&0xf0))|(((*((srcdata+width*ii)+jj*2))*2>>4)));
    }
    SHAPE* destClut = SHAPE_clut(fourbit);
    SHAPE* srcClut = SHAPE_clut(srcShape);

    int nColours = 16;
    unsigned int *destclutdata = (unsigned int *) SHAPE_data(destClut);
    unsigned int *srcclutdata = (unsigned int *) SHAPE_data(srcClut);
    for (ii=0; ii<nColours; ++ii)
    {
        destclutdata[ii] = srcclutdata[ii*8+7];
    }
    return fourbit;
}
#endif


#if defined(EA_PLATFORM_PLAYSTATION2)
SHAPE* From8BitsTo4Bits(SHAPE* srcShape)
{
    int width = SHAPE_width(srcShape);
    int height = SHAPE_height(srcShape);

    SHAPE* fourbit = _createShape(width,height,SHP_P_4,false,MB_LOW);
    char* destdata = (char*)SHAPE_data(fourbit);
    char* srcdata = (char*)SHAPE_data(srcShape);
    int ii,jj;
    int picth=(width)/2;
    for(ii=0;ii<height;ii++)
    for(jj=0;jj<picth;jj++)
    {
        *(destdata+picth*ii+jj) = ((((*((srcdata+width*ii)+jj*2+1))*2&0xf0))|(((*((srcdata+width*ii)+jj*2))*2>>4)));
    }
    SHAPE* destClut = SHAPE_clut(fourbit);
    SHAPE* srcClut = SHAPE_clut(srcShape);

    int nColours = 16;
    unsigned int *destclutdata = (unsigned int *) SHAPE_data(destClut);
    unsigned int *srcclutdata = (unsigned int *) SHAPE_data(srcClut);
    for (ii=0; ii<nColours; ++ii)
    {
        destclutdata[ii] = srcclutdata[ii*8+7];
    }
    return fourbit;
}
#endif

// (LEO) 06.08.04 FR521 Make FFB_MAXSTRINGS dynamic
AllocatedString *   allocStrings = NULL;
int             nAllocStrings = 0;

#define BORDER_SIZE 2

/*F************************************************************************************************/
/*!
default implementation of gAptFuncs.pfnAllocateString callback function. can be overridden
by game team. This basically takes string parameters, creates a texture, and draws the text onto
the texture. Note that we use Font fusion to take care of the actual font rendering. This
only takes place if the textField is "Dirty" i.e. the first time, and then only when an attribute
changes that requires a redrawing of the field.

\Input

\Output

\Notes

\Version
1.0        05/04/04 (DC) Comment creation.

*/
/************************************************************************************************F*/
static AptAssetString allocateStringFF(AptAllocateStringParameters *pASP)
{
    if (gpFonts->nFonts == 0)
    {
        return 0;
    }

// The macro APT_DECOUPLED_RENDERING no longer exists.
//#if !defined (APT_DECOUPLED_RENDERING)

    //// If flag is NONE or ASCHANGE, do nothing.  The reason we don't have to do anything on an ASCHANGE is because the size should be the same as before
    if(!(pASP->eFlags & (APT_TEXTFIELD_FUPDATE | APT_TEXTFIELD_DIRTY | APT_TEXTFIELD_TEXTCOLOR)))
    {
        AllocatedString *pStrPtr = (AllocatedString *)(pASP->pCurrString);
        if(!(pASP->eFlags & (APT_TEXTFIELD_ASCHANGE)))
        {
            pStrPtr->bUsed          = 1;
            pStrPtr->bBackground    = pASP->bBackground;        // None of these properties require a texture recreate
            pStrPtr->bBorder        = pASP->bBorder;
            pStrPtr->nBackColor     = pASP->nBackColor;
            pStrPtr->nBorderColor   = pASP->nBorderColor;
        }
        pASP->nMaxScroll    = pStrPtr->nMaxScroll;
        pASP->fTextWidth    = pStrPtr->fTextWidth;
        pASP->fTextHeight   = pStrPtr->fTextHeight;
        pASP->fStrLen       = 1000;

        return pASP->pCurrString;
    }
//#endif //APT_DECOUPLED_RENDERING

    /// If we get passed here, one of the following flags were set
        //APT_TEXTFIELD_FUPDATE,            // Force update
        //APT_TEXTFIELD_TEXTCHANGE,         // Text in field changed
        //APT_TEXTFIELD_TEXTWIDTH,          // _textWidth property changed
        //APT_TEXTFIELD_WORDWRAP,           // WordWrap flag set
        //APT_TEXTFIELD_HEIGHT,             // _height changed
        //APT_TEXTFIELD_WIDTH               // _width changed

    int i = 0;
    int nBoxWidth = 0;
    int nBoxHeight = 0;

    float xScaleFactor = pASP->fFontHeight < gAptFFScaleAtThisPtSize ? 1.0f : gAptFFScaleFactorX;
    float yScaleFactor = pASP->fFontHeight < gAptFFScaleAtThisPtSize ? 1.0f : gAptFFScaleFactorY;

    // adjust/scale values for Height & Width of Text
    float fWidth    = (pASP->x1 - pASP->x0) * xScaleFactor;
    float fHeight   = (pASP->y1 - pASP->y0) * yScaleFactor;

    int nTextWidth  = (int) fWidth;
    nBoxWidth       = nTextWidth;
    int nTextHeight = (int) fHeight;
    int nFontIndex  = 0;

    char szUTF8Name[256];
    if(!pASP->szFontName)
    {
        // dynamic text field was created and Animation does not have any Font
        // character in it so use defualt.
        nFontIndex = 0 ;
    }
    else
    {
        for (i = 0; i < gpFonts->nFonts; i++)
        {
            UCS2ToUTF8(gpFonts->aszFontNamesEnglish[i], -1, szUTF8Name);
            if (strcmp(szUTF8Name, pASP->szFontName) == 0)
            {
                nFontIndex = i;
                break;
            }
            UCS2ToUTF8(gpFonts->aszFontNamesJapanese[i], -1, szUTF8Name);
            if (strcmp(szUTF8Name, pASP->szFontName) == 0)
            {
                nFontIndex = i;
                break;
            }
        }
    }
#if defined(EA_DEBUG)
    if (i == gpFonts->nFonts)
    {
        nFontIndex = 0;
        UCS2ToUTF8(gpFonts->aszFontNamesEnglish[0], -1, szUTF8Name);
        //gAptFuncs.pfnDebugPrint("couldn't find font '%s', using '%s' instead\n", pASP->szFontName, szUTF8Name);
    }
#endif
    // Find an unused string

    int nString;

    AllocatedString *pString = NULL;

    assert(nAllocStrings < pCurrentAptAuxParams->iMaxStringCount);
    for (nString = 0; nString < pCurrentAptAuxParams->iMaxStringCount; nString++)
    {
        if (!allocStrings[nString].bUsed)
        {
            pString = &allocStrings[nString];
            break;
        }
    }
    nAllocStrings++;
    pString->nPtSize = (int)pASP->fFontHeight;

#if defined(EA_DEBUG)
    strncpy(pString->szText, pASP->szString, 32);
#endif


    int nLen = strlen(pASP->szString);

    unsigned short *UCS2 = (unsigned short* ) APTAUX_MALLOC_BLOCK( sizeof(unsigned short) * (nLen+1) );
    unsigned char*  pSrc = (unsigned char *)pASP->szString;
    unsigned short* pDst = UCS2;

/*
Below is a slightly confusing Conversion process. Be not afraid. We are simply converting
from UTF-8 Values to Unicode indexes. What does that mean? well in order to try to maintain
compatibility with ASCII, the UTF-8 encoding of Unicode values only take up as many bytes
as they need. The signal to how many bytes to use is how many sequential bits are set at
the end of the first byte.

if (      (str[0] & 0b1000.00000 ) == 0b0000.00000) // 0 sequential set bits.
{
                  ------ str[0] --------
    unicode bits: 0 x6 x5 x4 x3 x2 x1 x0
}
else if ( (str[0] & 0b1110.00000 ) == 0b1100.00000) // 2 sequential set bits.
{
                  ------- str[0] --------   ------ str[1] -------
    unicode bits: 1  1  0 x10 x9 x8 x7 x6 . 1 0 x5 x4 x3 x2 x1 x0
}
else if ( (str[0] & 0b1111.00000 ) == 0b1110.00000) // 3 sequential set bits.
{
                    -------- str[0] ---------   ----- str[1] ----------  ------ str[2] --------
    unicode value:  1 1 0 x16 x15 x14 x13 x12 . 1 0 x11 x10 x9 x8 x7 x6 . 1 0 x5 x4 x3 x2 x1 x0
}

Note that there is a form that allows 4 byte charactes, (USC-4) that has 4 sequential set bits,
but we do not currently support it in Apt.

*/

    // UCS-4 range (hex.)           UTF-8 octet sequence (binary)
    while ((char*)pSrc<pASP->szString+nLen)
    {
        // Convert from ASCII. (Grab one byte.)
        if (*pSrc<=127)     //0000 0000-0000 007F   0xxxxxxx
        {
            *pDst = *pSrc;
            ++pDst;
            ++pSrc;
        }
        // (Grab two bytes)
        //      This          |       To
        //0000 0080-0000 07FF | 110xxxxx 10xxxxxx
        else if ((*pSrc&0xe0) == 0xc0)
        {
            assert((pSrc[1] & 0xc0) == 0x80);
            *pDst = ((pSrc[0] & 0x1f) <<6) | (pSrc[1] & 0x3f);
            ++pDst;
            pSrc+=2;
        }
        else if ((*pSrc&0xf0) == 0xe0)
        {
            assert((pSrc[1] & 0xc0) == 0x80);
            assert((pSrc[2] & 0xc0) == 0x80);
            *pDst = ((pSrc[0] & 0x0f) <<12) | ((pSrc[1] & 0x3f) << 6) | (pSrc[2] & 0x3f);
            ++pDst;
            pSrc+=3;
        }
        else if ((*pSrc&0xf8) == 0xF0)
        {
            assert(false && "USC-4 (4 byte characters) are not supported in Apt at this time.");
        }
        else
        {
            assert(false && "Invalid Unicode Sequence Encountered");
        }
    }
    *pDst++ = 0;
    assert((char*)pSrc == pASP->szString+nLen);
    unsigned int nTextureWidth, nTextureHeight;

    int nPointSize = (int) pASP->fFontHeight ;
    if (nPointSize == 0)
        nPointSize = 1 ;            // we cannot pass 0 as point size to AptFF.

    // Case for autoSize flag set.. we need to check if we need to recalculate the texture size
    if(pASP->eBoxAlignment != AptStringAlignment_None)
    {
        // Now set alignment.
        FF.calcRenderStringSize(
                            nFontIndex,
                            UCS2,
                            (int)nPointSize,
                            nTextWidth,                 // out: max width
                            nTextHeight,                // out: max height
                            BORDER_SIZE,
                            0, 0,
                            pASP->bWordWrap,
                            // $FR#598 CSB - Adding parameter for Font Style 25/36
                            pASP->nFontStyle,
                            nBoxWidth,
                            (CFontFusion::Alignment)pASP->eBoxAlignment,
                            pASP->nLineOffset
                            );

        // Wrapped text fields do not change width!
        // (1.1:rrv:1/3) adjust by scale factors
        pASP->x1    = pASP->x0 + ((float)( pASP->bWordWrap ? nBoxWidth : nTextWidth ) / xScaleFactor);
        pASP->y1    = pASP->y0 + ((float)nTextHeight / yScaleFactor);

        // (1.1:rrv:1/3) adjust by scale factors
        nTextWidth  = (int)(((float)(nTextWidth)  / xScaleFactor) );
        nTextHeight = (int)(((float)(nTextHeight) / yScaleFactor) );
    }

    // $bug898 PDS [ #898 ] Shift of text and keyboard issue
    // If wrap is on don't change the nBoxWidth
    nBoxWidth = nTextWidth = pASP->bWordWrap ? nBoxWidth : nTextWidth;

    nBoxHeight = nTextHeight;

    // save width and height floats
    pString->fTextWidth     = (float)nTextWidth;
    pString->fTextHeight    = (float)nTextHeight;

    // texture may be resized, so save these here
    // (1.1:rrv:1/3) relocated from below
    fWidth  = pString->fTextWidth;
    fHeight = pString->fTextHeight;

    // set texture size
    nTextureWidth           = nTextWidth;
    nTextureHeight          = nTextHeight;

    assert( pString->pShape == NULL);

    #if defined(EA_PLATFORM_XBOX)
        pString->pShape = AptAuxEAGLREAL_CreateFontTexture((int*)&nTextureWidth, (int*)&nTextureHeight, pASP->nColour, MB_LOW);
    #else
        pString->pShape = AptAuxEAGLREAL_CreateFontTexture((int*)&nTextureWidth, (int*)&nTextureHeight, pASP->nColour, MB_HIGH);
    #endif

    // $internal709 - MCB - Changed the parameter passed in for alignment. Use eBoxAlignment if it's set to anything other than 'none' or it's wordwrapped.
    pASP->nMaxScroll = FF.renderString(
                                nFontIndex,                                     // nFont
                                UCS2,                                           // wszString
                                nPointSize,                                     // nPointSize
                                nTextureWidth,                                  // nTextureWidth    (in)
                                nTextureHeight,                                 // nTextureHeight   (in)
                                (unsigned char *)SHAPE_data(pString->pShape),   // pData
                                (unsigned int*)&nTextWidth,                     // pnTextWidth      (out)
                                (unsigned int*)&nTextHeight,                    // pnTextHeight     (out)
                                &pString->fYAscender,                           // pfYAscender      (out)
                                BORDER_SIZE,                                    // nBorderSize
                                0,0,                                            // nX, nY
                                pASP->bWordWrap,                                // bWrap
                                // $FR#598 CSB - Adding parameter for Font Style 26/36
                                pASP->nFontStyle,
                                nBoxWidth,                                      // nBoxWidth
                                nBoxHeight,                                     // nBoxHeight
                                // $bug898 PDS [ #898 ] Shift of text and keyboard issue
                                (pASP->eBoxAlignment == AptStringAlignment_None || pASP->bWordWrap) ?
                                    (CFontFusion::Alignment)pASP->eAlignment : (CFontFusion::Alignment)AptStringAlignment_None,
                                pASP->nLineOffset                               // nLineOffset
                                );
    // (1.1:rrv:2/3) adjust sizing by scale factors
    nTextWidth  = (int)(((float)(nTextWidth)  / xScaleFactor) );
    nTextHeight = (int)(((float)(nTextHeight) / yScaleFactor) );

    // adjust lineoffset
    if( pASP->nLineOffset > pASP->nMaxScroll )
    {
        pASP->nLineOffset = pASP->nMaxScroll;
    }

    APTAUX_FREE_BLOCK(UCS2, sizeof(unsigned short) * (nLen+1));

    // watch out to return the text width not the texture width!! (or box width) Which was updated by the renderString
    pASP->fTextWidth    = (float)nTextWidth;

    // watch out to return the text height not the texture height!! (or box height) Which was updated by the renderString
    pASP->fTextHeight   = (float)nTextHeight;

    // nice round number
    pASP->fStrLen       = 1000;

    // Copy things we need for drawing font texture
    pString->bUsed      = 1;

    // Warning These need to include border sizes
    pString->fLeft      = (float)((int)(pASP->x0)) ;
    pString->fTop       = (float)((int)(pASP->y0)) ;

    // For some reason we are much more likely to overflow in X...
    pString->fRight     = (float)((int)(pString->fLeft + ((pASP->x1 - pASP->x0) * xScaleFactor)));

    // For some reason Font fusion tells us we need more room then we do in Y, this alleviates that.
    pString->fBottom    = (float)((int)(pString->fTop  + ((pASP->y1 - pASP->y0) * yScaleFactor))-BORDER_SIZE);

    // copy corresponding
    pString->eAlignment    = pASP->eAlignment;
    pString->eBoxAlignment = pASP->eBoxAlignment;
    pString->bBackground   = pASP->bBackground;
    pString->bBorder       = pASP->bBorder;
    pString->nBackColor    = pASP->nBackColor;
    pString->nBorderColor  = pASP->nBorderColor;
    pString->fTexWidth     = static_cast<float>(2 << (int) (logf(fWidth - 1) / logf(2.0f)));    // These are the actual texture sizes
    pString->fTexHeight    = static_cast<float>(2 << (int) (logf(fHeight - 1) / logf(2.0f)));   // These are the actual texture sizes
    pString->nMaxScroll    = pASP->nMaxScroll;

#if defined(EA_PLATFORM_XBOX)
    {
        unsigned char *unswizzled   = NULL;
        unsigned char *swizzled     = (unsigned char *)SHAPE_data(pString->pShape);

        int pitch   = SHAPE_rowbytes(pString->pShape);
        int bpp     = SHAPE_depth(pString->pShape);

        unswizzled  = (unsigned char *) MEM_alloc("swizzle temp", pitch * pString->pShape->height, FontTextureMemPoolClassID | MB_ALIGN|MB_HIGH);
        memcpy(unswizzled, swizzled, MEM_size(unswizzled));

        XGSwizzleRect(unswizzled, 0, NULL, swizzled, pString->pShape->width, pString->pShape->height, NULL, bpp / 8);

        // TODO workaround until EAGL 4.08.06
        //  wbinvd := Write-Back and Invalidate Cache
        __asm {
                wbinvd
        };

        MEM_free(unswizzled);
    }
#endif
#if defined(EA_PLATFORM_GAMECUBE)
    {
        SHAPE* fourbit = From8BitsTo4Bits(pString->pShape);
        int shapesize = SHAPE_clonesize(fourbit);
        SHAPE* swizzed = (SHAPE *)MEM_allocalign("GcFontShpSwizzled", shapesize + sizeof(SHAPE), 32, sizeof(SHAPE), FontTextureMemPoolClassID|MB_LOW);
        assert(swizzed != NULL);
        SHAPE_cloneat(swizzed,fourbit);
        SHAPE_swizzleat(swizzed,fourbit);
        MEM_free(pString->pShape);
        MEM_free(fourbit);
        pString->pShape = swizzed;
        DCFlushRange(swizzed, SHAPE_size(swizzed));
        SHAPE* clut =SHAPE_clut(swizzed);
        if(clut)
        {
            DCFlushRange(clut, SHAPE_size(clut));
        }
    }
#endif
#if defined(EA_PLATFORM_PLAYSTATION2)
    {
        SHAPE* fourbit = From8BitsTo4Bits(pString->pShape);
        MEM_free(pString->pShape);
        pString->pShape = fourbit;
    }
#endif
#if defined(EA_PLATFORM_WINDOWS) || defined(EA_PLATFORM_PSP)
    {
        SHAPE* sixteenbit = From8BitsTo16Bits(pString->pShape);
        MEM_free(pString->pShape);
        pString->pShape = sixteenbit;
    }
#endif

    return &allocStrings[nString];
}

/*F************************************************************************************************/
/*!
returns the shape object from a text field (AssetString) object.

\Input

\Output

\Notes  Currently this function is not called anywhere in apt or apt aux.

\Version
1.0        05/04/04 (DC) Comment creation.

*/
/************************************************************************************************F*/
SHAPE *AptAuxEAGLREAL_GetSHAPEFromString(AptAssetString string)
{
    return ((AllocatedString *)string)->pShape;
}

/*F************************************************************************************************/
/*!
TODO: get more detailed information on this function.

\Input

\Output

\Notes  Currently this function is not called anywhere in apt or apt aux.

\Version
1.0        05/04/04 (DC) Comment creation.

*/
/************************************************************************************************F*/
unsigned int AptAuxEAGLREAL_StartBatchDraw(SHAPE *shape, bool strip, bool ignoreZ)
{
    assert(gnTarNumberForBatchDraw == -1);
    gnTarNumberForBatchDraw = gnCurImmedTAR++;
    TAR *pTAR = gapImmedCurTARs[gnTarNumberForBatchDraw];
    gnCurImmedTAR %= sizeof(gapImmedCurTARs)/sizeof(gapImmedCurTARs[0]);

    pTAR->SwapShape(shape);
    pTAR->SetClampMode(CM_CLAMP);

    gpImmedGeoPrimState->SetTextureEnable(true);
    gpImmedTexturedMethod->SetTexture(pTAR);
    if (ignoreZ)
    {
        gpImmedGeoPrimState->SetDepthTestMethod(DTM_ALWAYS);
    }
    else
    {
    #if defined(EA_PLATFORM_PLAYSTATION2)
        gpImmedGeoPrimState->SetDepthTestMethod(DTM_GEQUAL);
    #else
        gpImmedGeoPrimState->SetDepthTestMethod(DTM_LEQUAL);
    #endif
    }
    gpImmedTexturedMethod->SetState(*gpImmedGeoPrimState);
    gpImmedTexturedMethod->SetModelMatrix(vertexMatrixIntegralTranslate);

#if defined(EA_PLATFORM_PLAYSTATION2)
    {
        pTAR->EXTENSION_OBJ.SetMinU(0.00f);
        pTAR->EXTENSION_OBJ.SetMaxU(.99f);
        pTAR->EXTENSION_OBJ.SetClampH(PS2CM_REGION_CLAMP);
        pTAR->EXTENSION_OBJ.SetMinV(0.00f);
        pTAR->EXTENSION_OBJ.SetMaxV(.99f);
        pTAR->EXTENSION_OBJ.SetClampV(PS2CM_REGION_CLAMP);
    }
#elif defined(EA_PLATFORM_PSP)
    {
        pTAR->EXTENSION_OBJ.SetClampU(PSP_CM_CLAMP);
        pTAR->EXTENSION_OBJ.SetClampV(PSP_CM_CLAMP);
    }
#endif

    gpImmedTexturedMethod->Begin(strip ? PT_TRIANGLESTRIP : PT_TRIANGLELIST);

    return _transformColour(0xffffffff);
}
/*F************************************************************************************************/
/*!
TODO: get more detailed information on this function.

\Input

\Output

\Notes  Currently this function is not called anywhere in apt or apt aux.

\Version
1.0        05/04/04 (DC) Comment creation.

*/
/************************************************************************************************F*/
void AptAuxEAGLREAL_EndBatchDraw()
{
    assert(gnTarNumberForBatchDraw >= 0);
    gpImmedTexturedMethod->End();
    gpImmedTexturedMethod->Flush();
    gapImmedCurTARs[gnTarNumberForBatchDraw]->SwapShape(0);
    gnTarNumberForBatchDraw = -1;
}
/*F************************************************************************************************/
/*!
TODO: get more detailed information on this function.

\Input

\Output

\Notes  Currently this function is not called anywhere in apt or apt aux.

\Version
1.0        05/04/04 (DC) Comment creation.

*/
/************************************************************************************************F*/
void AptAuxEAGLREAL_AddStringVertex(COORD4 *c, unsigned int clr, COORD4 *t)
{
    gpImmedTexturedMethod->AddVertex(*c, clr, *t);
}

float _gZ = 0.f ;

/*F************************************************************************************************/
/*!
Renders us some Text y'all. Basically we create the object, assign the texture and conditionally
draw a border around it.

\Input

\Output

\Notes

\Version
1.0        05/04/04 (DC) Comment creation.

*/
/************************************************************************************************F*/
static void _drawStringRaw(AllocatedString *pString)
{
    assert(gnTarNumberForBatchDraw == -1);
    assert(pString->bUsed);

    TAR *pTAR = gapImmedCurTARs[gnCurImmedTAR++];
    gnCurImmedTAR %= sizeof(gapImmedCurTARs)/sizeof(gapImmedCurTARs[0]);

    pTAR->SwapShape(pString->pShape);
    pTAR->SetClampMode(CM_CLAMP);

    gpImmedTexturedMethod->SetTexture(pTAR);
    gpImmedTexturedMethod->SetModelMatrix(vertexMatrixIntegralTranslate);

    ////////////////////////////////////////////////////////////////////////
    // START -- (1.1:rrv:3/3) adjust sizing by scale factors (delta values)
    ////////////////////////////////////////////////////////////////////////
    unsigned int CLR = _transformColour(0xffffffff);
    //CLR = 0xffffffff;

    // calculate string width & height
    float w = (pString->fRight  - pString->fLeft);
    float h = (pString->fBottom - pString->fTop);

    // calculate delta values for text box border coordinates
    //  if string-points-size < scaling-reference-points-size, delta is zero
    //  else, use delta value
    //  note that if the scale factor is 0.5, the value is the same as the numerator
    float deltaW = (float) pString->nPtSize < gAptFFScaleAtThisPtSize ? 0.0f : (w/gAptFFScaleFactorX)-w;
    float deltaH = (float) pString->nPtSize < gAptFFScaleAtThisPtSize ? 0.0f : (h/gAptFFScaleFactorY)-h;

    // p1-4 are text box border coords
    COORD4 p1 = {pString->fLeft,  pString->fTop,    0.f, 1.f};
    COORD4 p2 = {(pString->fRight+deltaW),  pString->fTop,              0.f, 1.f};  //adjust with delta!
    COORD4 p3 = {(pString->fRight+deltaW),  (pString->fBottom+deltaH),  0.f, 1.f};  //adjust with delta!
    COORD4 p4 = {pString->fLeft,            (pString->fBottom+deltaH),  0.f, 1.f};  //adjust with delta!

    COORD4 c = { 0, 0, _gZ, 1.f };      // changed z value from 1.0 to 0.0 for dynamic text masking
    COORD4 t = { 0, 0, 1.f, 1.f };

    #if defined(EA_PLATFORM_PLAYSTATION2)
    {
        pTAR->EXTENSION_OBJ.SetMinU(0.00f);
        pTAR->EXTENSION_OBJ.SetMaxU(.99f);
        pTAR->EXTENSION_OBJ.SetClampH(PS2CM_REGION_CLAMP);
        pTAR->EXTENSION_OBJ.SetMinV(0.00f);
        pTAR->EXTENSION_OBJ.SetMaxV(.99f);
        pTAR->EXTENSION_OBJ.SetClampV(PS2CM_REGION_CLAMP);
    }
    #elif defined(EA_PLATFORM_PSP)
    {
        pTAR->EXTENSION_OBJ.SetClampU(PSP_CM_CLAMP);
        pTAR->EXTENSION_OBJ.SetClampV(PSP_CM_CLAMP);
    }
    #endif

    if(pString->bBackground)
    {
        // Draw background
        CLR = EAGL_GOURAUDCOLOUR(pString->nBackColor);
        gpImmedGeoPrimState->SetTextureEnable(false);
        gpImmedGouraudMethod->SetState(*gpImmedGeoPrimState);
        gpImmedGouraudMethod->SetModelMatrix(vertexMatrix);
        gpImmedGouraudMethod->Begin(PT_TRIANGLESTRIP);
        gpImmedGouraudMethod->AddVertex(p1, CLR);
        gpImmedGouraudMethod->AddVertex(p2, CLR);
        gpImmedGouraudMethod->AddVertex(p3, CLR);
        gpImmedGouraudMethod->AddVertex(p4, CLR);
        gpImmedGouraudMethod->AddVertex(p1, CLR);
        gpImmedGouraudMethod->End();
        gpImmedGouraudMethod->Flush();
    }

    CLR = _transformColour(0xffffffff);
    gpImmedTexturedMethod->Begin(PT_TRIANGLESTRIP);

    // bounds for text
    c.x = pString->fLeft;
    c.y = pString->fTop;
    t.x = 0.f;
    t.y = 0.f;
    gpImmedTexturedMethod->AddVertex(c, CLR, t);

    c.x = pString->fRight + deltaW;
    c.y = pString->fTop;
    t.x = w / pString->fTexWidth;
    t.y = 0.0f;
    gpImmedTexturedMethod->AddVertex(c, CLR, t);

    c.x = pString->fLeft;
    c.y = pString->fBottom + deltaH;
    t.x = 0.0f;
    t.y = h / pString->fTexHeight;
    gpImmedTexturedMethod->AddVertex(c, CLR, t);

    c.x = pString->fRight + deltaW;
    c.y = pString->fBottom + deltaH;
    t.x = w / pString->fTexWidth;
    t.y = h / pString->fTexHeight;
    gpImmedTexturedMethod->AddVertex(c, CLR, t);

    ////////////////////////////////////////////////////////////////////////
    // END -- (1.1:rrv:3/3) adjust sizing by scale factors (delta values)
    ////////////////////////////////////////////////////////////////////////

    gpImmedTexturedMethod->End();
    gpImmedTexturedMethod->Flush();

    if(pString->bBorder)
    {
        // Draw border
        CLR = EAGL_GOURAUDCOLOUR(pString->nBorderColor);
        gpImmedGeoPrimState->SetTextureEnable(false);
        gpImmedGouraudMethod->SetState(*gpImmedGeoPrimState);
        gpImmedGouraudMethod->SetModelMatrix(vertexMatrix);
        gpImmedGouraudMethod->Begin(PT_LINELIST);
        gpImmedGouraudMethod->AddVertex(p1, CLR);
        gpImmedGouraudMethod->AddVertex(p2, CLR);
        gpImmedGouraudMethod->AddVertex(p2, CLR);
        gpImmedGouraudMethod->AddVertex(p3, CLR);
        gpImmedGouraudMethod->AddVertex(p3, CLR);
        gpImmedGouraudMethod->AddVertex(p4, CLR);
        gpImmedGouraudMethod->AddVertex(p4, CLR);
        gpImmedGouraudMethod->AddVertex(p1, CLR);
        gpImmedGouraudMethod->End();
        gpImmedGouraudMethod->Flush();
    }
}

/*F************************************************************************************************/
/*!
TODO: Get more detailed information on this function.

\Input

\Output

\Notes  Currently this function is not called anywhere in apt or apt aux.

\Version
1.0        05/04/04 (DC) Comment creation.

*/
/************************************************************************************************F*/
void AptAuxEAGLREAL_DrawStringIgnoreZ(AptAssetString string)
{
    AllocatedString *pString = (AllocatedString *)string;
    gpImmedGeoPrimState->SetTextureEnable(true);
    gpImmedGeoPrimState->SetDepthTestMethod(DTM_ALWAYS);
    gpImmedTexturedMethod->SetState(*gpImmedGeoPrimState);
    _drawStringRaw(pString);
}

/*F************************************************************************************************/
/*!
default implementation of gAptFuncs.pfnDrawString callback function. can be overridden
by game team. This is called by Apt to get a pre rendered text field object drawn on the screen.

\Input

\Output

\Notes

\Version
1.0        05/04/04 (DC) Comment creation.
1.1        03/18/05 (DC) Added arguments for nested MAsk support (igored in EAGLREAL viewer)
1.20       11/05/07 $senzee Changed signature in accordance with Apt 2.05.00 API.  This is a
                     superficial change, since both the mask operation and mask depth can be obtained from the
                     AptRenderInfo structure.
*/
/************************************************************************************************F*/
static void drawStringFF(AptAssetString string, const AptRenderInfo *info)
{
    AllocatedString *pString = (AllocatedString *)string;
    gpImmedGeoPrimState->SetTextureEnable(true);
#if defined(EA_PLATFORM_PLAYSTATION2) || defined (EA_PLATFORM_PSP)
    gpImmedGeoPrimState->SetDepthTestMethod(DTM_GEQUAL);
#else
    gpImmedGeoPrimState->SetDepthTestMethod(DTM_LEQUAL);
#endif
    gpImmedTexturedMethod->SetState(*gpImmedGeoPrimState);

    // now check the masking operation
    if (info->meMaskOper == AptMaskRenderOperation_None)
    {
        _drawStringRaw(pString);
    }
    else
    {
        // call commonDraw that adds/subtract this string from a common array of masks.
        // and draws the string as a mask if required.
        CommonDraw((void *)pString, AptAuxEAGLREAL_RenderObjType_String, info->meMaskOper) ;
    }

    pString->bHasBeenDrawn = 1;
}
/*F************************************************************************************************/
/*!
default implementation of gAptFuncs.pfnDeallocateString callback function. can be overridden
by game team. TODO: Figure out what the heck the gapImmedCurTARs and such is doing.

\Input

\Output

\Notes

\Version
1.0        05/04/04 (DC) Comment creation.

*/
/************************************************************************************************F*/
static void deallocateStringFF(AptAssetString string, unsigned int eFlags)
{

// The macro APT_DECOUPLED_RENDERING no longer exists.
//#if !defined (APT_DECOUPLED_RENDERING)
    if(!(eFlags & (APT_TEXTFIELD_FUPDATE | APT_TEXTFIELD_DIRTY | APT_TEXTFIELD_TEXTCOLOR)))
    {
        return;
    }
//#endif //APT_DECOUPLED_RENDERING

    DeadStringPool* pCurrentPool = aDeadStringPools + nCurrentDSPool;
    AllocatedString *pString = (AllocatedString *)string;
    for (int i = 0; i < (int)(sizeof(gapImmedCurTARs)/sizeof(gapImmedCurTARs[0])); ++i)
    {
        gapImmedCurTARs[i]->SwapShape(gpEmptyShape);
    }

    // DC - Added check to see if string has been drawn before adding it to this list.
    // if the string has not been drawn yet, just delete it now.
    if(pString->bHasBeenDrawn)
    {
    assert(pCurrentPool->nDeadStrings < DEADSTRINGPERFRAME);
    pCurrentPool->aDeadStrings[(pCurrentPool->nDeadStrings)++] = pString->pShape;
    }
    else
    {
        if( pString->pShape != NULL && (void *)pString->pShape != (void *)-1)
        {
            MEM_free( pString->pShape);
        }
    }

    pString->pShape = 0;
    pString->bUsed = 0;
    nAllocStrings--;
}

void           **gpDelayedReleasePointers; // Array of length pCurrentAptAuxParams->iMaxStringCount
int              gnNumDelayedReleasePointers = 0; // Currne number of elements in the Array.

/*F************************************************************************************************/
/*!
Adds the given pointer to a list of values to be deleted at the end of the frame. This is useful for
batching deletions.

\Input

\Output

\Notes

\Version
1.0        05/04/04 (DC) Comment creation.

*/
/************************************************************************************************F*/
void AddDelayedReleasePointer(void * ptr)
{
    // $bug697 - MCB - Changed assert to compare against the iMaxDelayedReleaseValues value instead of iMaxString value
    assert( gnNumDelayedReleasePointers < pCurrentAptAuxParams->iMaxDelayedReleaseValues && "Increase the size of AptAuxInit Param -> iMaxDelayedReleaseValues at initialization");
    gpDelayedReleasePointers[gnNumDelayedReleasePointers++] = ptr;
}
/*F************************************************************************************************/
/*!
Batch Deletes the values in the Delayed Release Array.

\Input

\Output

\Notes

\Version
1.0        05/04/04 (DC) Comment creation.

*/
/************************************************************************************************F*/
void FreeDelayedReleasePointers()
{
    for(int x=0; x<gnNumDelayedReleasePointers; x++)
    {
        gAptFuncs.pfnMemFree( gpDelayedReleasePointers[x] );
        gpDelayedReleasePointers[x] = NULL;
    }
    gnNumDelayedReleasePointers = 0;
}

void AptAuxEAGLREAL_TimerInit(void)
{
    Timer_Init();
}
void AptAuxEAGLREAL_TimerStartFrame(void)
{
    // This is called here just to get it done every frame. This may not be the best permenant home for this,
    // but can live here for now.
    FreeDelayedReleasePointers();

    Timer_StartFrame();
}
/*F************************************************************************************************/
/*!
returns the number of milli-seconds since the last call to AptAuxEAGLREAL_TimerStartFrame. The
returned value is passed to AptUpdate each frame.

\Input

\Output

\Notes

\Version
1.0        05/04/04 (DC) Comment creation.

*/
/************************************************************************************************F*/
unsigned int AptAuxEAGLREAL_TimerGetTicks(void)
{
    return Timer_GetTicks();
}
/*F************************************************************************************************/
/*!
returns the number of seconds since the last call to AptAuxEAGLREAL_TimerStartFrame. Assuming we
get through a frame at least once a second, this should always return zero.

\Input

\Output

\Notes

\Version
1.0        05/04/04 (DC) Comment creation.

*/
/************************************************************************************************F*/
float AptAuxEAGLREAL_TimerGetTime(void)
{
    return Timer_GetTime();
}

struct AsyncLoads
{
    int bValid;
    AptFilePtr pAsyncLoadContext;
    char szBaseFilename[256];
    int nLoadHandle;
    void * pFileBuffer ;

    AsyncLoads() : bValid(0), nLoadHandle(-1)
    {
        szBaseFilename[0] = 0;
        pFileBuffer = 0 ;
    }

#if defined(APT_ALLOCATION_TRACKING)
    void *operator new(size_t size) { return gAptFuncs.pfnMemAlloc(size, __FILE__, __LINE__); }
    void *operator new[](size_t size) { return gAptFuncs.pfnMemAlloc(size, __FILE__, __LINE__); }
#else
    void *operator new(size_t size) { return gAptFuncs.pfnMemAlloc(size); }
    void *operator new[](size_t size) { return gAptFuncs.pfnMemAlloc(size); }
#endif
    void operator delete(void *p, size_t size) { gAptFuncs.pfnMemFreeSize(p, size); }
    void operator delete[](void *p) { gAptFuncs.pfnMemFree(p); }
};
static AsyncLoads *pAsyncLoads;
#define MAX_NUM_ASYNC_LOADS 32



#if 0
FILE *printFile;
static void printfunc(PRINTCHANNEL channel, const char *string)
{
    fprintf(printFile, "%s", string);
}
#endif


/*F************************************************************************************************/
/*!
default implementation of gAptFuncs.pfnLoadAnimationCompleted callback function. can be overridden
by game team. This does nothing at this point, games may use this event for something useful (i.e.
transitioning to the new movie?).

\Input

\Output

\Notes

\Version
1.0        05/04/04 (DC) Comment creation.

*/
/************************************************************************************************F*/
void LoadAnimationCompleted(const char *szBaseName, const char * szTargetName)
{
    // Do Nothing.
    // Do make a check of char * are valid or not before operating on them.
    // szTargetName can be a pointer to 0 length string.
}


int  gAptAuxEAGLREALAsyncLoadIntoMemFlags       = MB_LOW | MB_ALIGN;
int  gAptAuxEAGLREALAsyncDecompressIntoMemFlags = MB_HIGH | MB_ALIGN;
#if defined(EA_DEBUG)
bool gAptAuxEAGLREALAsyncLoadUseTag             = true;  // Uses tags by defualt in Debug version
#else
bool gAptAuxEAGLREALAsyncLoadUseTag             = false; // Turn off tagging by default on release builds
#endif

/*F************************************************************************************************/
/*!
    \Function    AptAuxEAGLREAL_SetAsyncLoadIntoMemFlags

    \Description
        Allows user to set the memory location where big files will be loaded.
        Default is set to MB_LOW | MB_ALIGN.

    \Input      nFlags            -

    \Output
        None.

    \Notes
        None.

    \Version    1.0        04/22/04 (ASK) First Version

*/
/************************************************************************************************F*/
void AptAuxEAGLREAL_SetAsyncLoadIntoMemFlags(int nFlags)
{
    gAptAuxEAGLREALAsyncLoadIntoMemFlags = nFlags ;
}


/*F************************************************************************************************/
/*!
    \Function    AptAuxEAGLREAL_GetAsyncLoadIntoMemFlags

    \Description
        Returns the memory flags/locations where big files will be loaded.


    \Input
        None.

    \Output     int               -

    \Notes
        None.

    \Version    1.0        04/22/04 (ASK) First Version

*/
/************************************************************************************************F*/
int AptAuxEAGLREAL_GetAsyncLoadIntoMemFlags()
{
    return gAptAuxEAGLREALAsyncLoadIntoMemFlags ;
}

/*F************************************************************************************************/
/*!
\Function    AptAuxEAGLREAL_SetAsyncLoadUseTag

\Description
Allows user to change whether or not allocated regions for files are tagged. Tagging memory does
not directly pose any problems just that the memory for the loaded files must be preallocated, and
to do that we must call blocking file information functions to get the size (these can cause the
UI to be unresponsive for up to a second.and add to the total load time). Thus on release builds
it is recommended that tagging be disabled.

\Input      nFlags            -

\Output
None.

\Notes
None.

\Version    1.0        04/22/04 (DC) First Version

*/
/************************************************************************************************F*/
void AptAuxEAGLREAL_SetAsyncLoadUseTag(bool newStatus)
{
    gAptAuxEAGLREALAsyncLoadUseTag = newStatus ;
}


/*F************************************************************************************************/
/*!
\Function    AptAuxEAGLREAL_GetAsyncLoadUseTag

\Description
Returns whether Memory tagging is enabled.


\Input
None.

\Output     bool               -  current value.

\Notes
None.

\Version    1.0        04/22/04 (DC) First Version

*/
/************************************************************************************************F*/
bool AptAuxEAGLREAL_GetAsyncLoadUseTag()
{
    return gAptAuxEAGLREALAsyncLoadUseTag ;
}


/*F************************************************************************************************/
/*!
    \Function    AptAuxEAGLREAL_SetAsyncDecompressIntoMemFlags

    \Description
        Allows user to set the memory location where decompression of big files (if needed)
        will be done.
        Default is set to MB_HIGH | MB_ALIGN

    Insert your long description here.

    \Input      nFlags            -

    \Output
        None.

    \Notes
        None.

    \Version    1.0        04/22/04 (ASK) First Version

*/
/************************************************************************************************F*/
void AptAuxEAGLREAL_SetAsyncDecompressIntoMemFlags(int nFlags)
{
    gAptAuxEAGLREALAsyncDecompressIntoMemFlags = nFlags ;
}


/*F************************************************************************************************/
/*!
    \Function    AptAuxEAGLREAL_GetAsyncDecompressIntoMemFlags

    \Description
        Returns where the big files will be decompressed if required.

    \Input
        None.

    \Output     int               -

    \Notes
        None.

    \Version    1.0        04/22/04 (ASK) First Version

*/
/************************************************************************************************F*/
int AptAuxEAGLREAL_GetAsyncDecompressIntoMemFlags()
{
    return gAptAuxEAGLREALAsyncDecompressIntoMemFlags ;
}
/*F************************************************************************************************/
/*!
default implementation of gAptFuncs.pfnLoadAnimation callback function. can be overridden
by game team. This initiates the ASync file transaction, one would override this if there was
something special to do to load the file.

\Input

\Output

\Notes

\Version
1.0        05/04/04 (DC) Comment creation.

*/
/************************************************************************************************F*/
static void LoadAnimation(const char *szBaseName, AptFilePtr pAsyncLoadContext)
{
    char szFilename[256];
    char szBaseFilename[256];
    unsigned int i;
    assert(strlen(szBaseName) < 250);

    const char *pSlash;
    // not strrchr because we have to look for both / and \ (and there's no strrcspn).
    for (pSlash = &szBaseName[strlen(szBaseName) - 1];
            pSlash >= szBaseName;
            pSlash--)
    {
        if (*pSlash == '/' || *pSlash == '\\')
        {
            break;
        }
    }

    if (pSlash >= szBaseName)
    {
        strcpy(szBaseFilename, pSlash+1);
    }
    else
    {
        strcpy(szBaseFilename, szBaseName);
    }
#if defined(APT_CAFE_2_XBOX_PATH_HACK)
    if(szBaseName[0] == '.' && szBaseName[1] == '.')
    {
        szBaseName++;
        szBaseName++;
        szBaseName++;
    }
#endif

    sprintf(szFilename, "%s%s.big", gszFilePrefix, szBaseName);

    for (i = 0; i < MAX_NUM_ASYNC_LOADS; i++)
    {
        if (!pAsyncLoads[i].bValid)
        {
            break;
        }
    }
    assert(i != MAX_NUM_ASYNC_LOADS);
    pAsyncLoads[i].pAsyncLoadContext = pAsyncLoadContext;
    strcpy(pAsyncLoads[i].szBaseFilename, szBaseFilename);

#if 0
    static int nAsyncCount = 0;
    nAsyncCount++;
    char szBuf[256];
    sprintf(szBuf, "d:\\dump_%d.csv", nAsyncCount);
    printFile = fopen(szBuf, "wb");
    PRINT_setdevicename(PRINT_USER_DEVICE, "userdevice", printfunc);
    PRINT_setdevicestate(PRINT_USER_DEVICE, 1);
    MEM_print();
    PRINT_restore();
    fclose(printFile);
#endif

    // start using Mem_alloc instead of filesys's own memory handling.
    // this way we can name Apt asyn file buffer, so it is easily identified.
    // instead of ASYNCFILE_load use ASYNCFILE_loadat. For this we need to allocate memory first and for that
    // we need to get the size of file first. this code might add some extra cycles but it is a feature request.
    // everything else related to asyncfile handling remains same.

    int nSize = 0 ;
    if (gAptAuxEAGLREALAsyncLoadUseTag && FILE_exists(szFilename))
    {
        nSize = FILE_size(szFilename) ;
        // same as gAptAuxEAGLREALAsyncLoadIntoMemFlags
        void * pBuffer = MEM_alloc("Apt-AsyncFileBuff", nSize, gAptAuxEAGLREALAsyncLoadIntoMemFlags);
        pAsyncLoads[i].pFileBuffer = pBuffer ;
        if (nSize)
        {
            pAsyncLoads[i].nLoadHandle = ASYNCFILE_loadat(szFilename, pBuffer, nSize);
        }
    }
    else
    {
        // if file not found do it older way..and let asyncfile module find later that file does not exists.
        pAsyncLoads[i].nLoadHandle = ASYNCFILE_load(szFilename, gAptAuxEAGLREALAsyncLoadIntoMemFlags);
    }
#if defined(EA_DEBUG)
    gAptFuncs.pfnDebugPrint("AptAuxEAGLREAL: starting load of '%s' as handle %d (FILE_size = %d bytes)\n", szFilename, pAsyncLoads[i].nLoadHandle, FILE_sizez(szFilename));
#endif
    pAsyncLoads[i].bValid = true;
}

/*F************************************************************************************************/
/*!
This basically grabs teh pieces from the big file (Const, apt, geometry, sounds) and stuff pointers
to them so that we can begin to use them.

\Input

\Output

\Notes

\Version
1.0        05/04/04 (DC) Comment creation.

*/
/************************************************************************************************F*/
AptAuxLayerInfo *AptAuxEAGLREAL_ResolveData(void *pFileData, const char *szBaseFilename, void **ppAptData, void **ppConstData)
{
    char szFilename[256];
    int i;
    AptAuxLayerInfo *pLevel = NULL;

    assert(ppAptData);
    assert(ppConstData);

    for (i = 0; i < pCurrentAptAuxParams->iLevelInfoSize ; i++)
    {
        if (!gLevelInfo[i].bInUse)
        {
            gLevelInfo[i].bInUse = 1;
            pLevel = &gLevelInfo[i];
            break;
        }
    }
    assert(i < pCurrentAptAuxParams->iLevelInfoSize);

    pLevel->bBigDataToBeFreed = 0;
    pLevel->pBigData = pFileData;
    strcpy(pLevel->szBaseName, szBaseFilename);

    _loadGeometry(szBaseFilename, pLevel);

#if defined(APT_USE_SOUND_OBJECT)
    if (gAptFuncs.pfnLoadSound == loadSound)
    {
        _loadSoundBank(szBaseFilename, pLevel);
    }
#endif

    strcpy(szFilename, szBaseFilename);
    strcat(szFilename, ".apt");
    *ppAptData = BIG_locate(pLevel->pBigData, szFilename);

    strcpy(szFilename, szBaseFilename);
    strcat(szFilename, ".const");
    *ppConstData = BIG_locate(pLevel->pBigData, szFilename);

    return pLevel;
}
/*F************************************************************************************************/
/*!
    This is called from the main game loop prior to AptUpdate. This moves files in the file list along
    their paths(state machines). when an animation load is complete it calls the
    AptCompleteAnimationAsyncLoad function for apt to handle it.

    \Input

    \Output

    \Notes

    \Version
        1.0        05/04/04 (DC) Comment creation.
        2.0        08/04/04 (LEO)
            -Added code to keep track of file sizes for Zombied animations

*/
/************************************************************************************************F*/
void AptAuxEAGLREAL_FinishAsyncLoads(void)
{
    unsigned int i;
    int nSize;

    for (i = 0; i < MAX_NUM_ASYNC_LOADS; i++)
    {
        if (pAsyncLoads[i].bValid)
        {
            int nStatus = ASYNCFILE_getstatus(pAsyncLoads[i].nLoadHandle);
            if (nStatus == ASYNCFILE_UNKNOWN)
            {
                pAsyncLoads[i].bValid = false;
            }
            else if (nStatus == ASYNCFILE_DONE)
            {
                void *pData = 0;
                nSize = 0;

                int nRet = ASYNCFILE_release(pAsyncLoads[i].nLoadHandle, &pData, &nSize);
                #if defined(EA_DEBUG)
                gAptFuncs.pfnDebugPrint("AptAuxEAGLREAL: finished load of handle %d, data = %p, read %d bytes\n", pAsyncLoads[i].nLoadHandle, pData, nSize);
                #endif
                #if defined(EA_DEBUG) || defined(DO_COVERAGE)
                pAsyncLoads[i].pAsyncLoadContext.SetFileSize(nSize);         // $BUG253 LEO    38/47
                #endif

                if (nRet == ASYNCFILE_DONE)
                {
                    // pData can be null or some valid pointer depending on what call has initiated
                    // the asyncload - if ASYNCFILE_loadat is used then ASYNCFILE_release function return the buffer
                    // address with nSize as zero so just look at size to check if load was successful or not.
                    // If load was initiated with ASYNCFILE_load then pData returned will be zero
                    if ( (nSize == 0) )     // && (!pData)
                    {
                        gAptFuncs.pfnDebugPrint("couldn't load '%s'!\n", pAsyncLoads[i].szBaseFilename);
                        // assert added to check if pData returned by ASYNCFILE_release is same as the one passed as parameter to
                        // ASYNCFILE_loadat function in loadanimation
                        // assert(pData == pAsyncLoads[i].pFileBuffer);
                        AptCompleteAnimationAsyncLoad(pAsyncLoads[i].pAsyncLoadContext, 0, 0, 0);
                        pAsyncLoads[i].bValid = false;
                        pAsyncLoads[i].pAsyncLoadContext = AptFilePtr(0);
                        if (pAsyncLoads[i].pFileBuffer)
                        {
                            // memory was allocated for ASYNC_loadat call because of File_exists and File_size were successful
                            // but still there is problem in reading the file so free that buffer
                            MEM_free(pAsyncLoads[i].pFileBuffer) ;
                            pAsyncLoads[i].pFileBuffer = 0 ;
                        }
                        continue;
                    }

                    if (unpacksizez(pData))
                    {
                        void *pOrigData = pData;
                        char szBuf[256];
                        sprintf(szBuf, "GUI::%s", pAsyncLoads[i].szBaseFilename);
                        nSize = unpacksize(pData);
                        #if defined(EA_DEBUG) || defined(DO_COVERAGE)              // $BUG253 LEO    39/47
                        pAsyncLoads[i].pAsyncLoadContext.SetFileSize(nSize);
                        #endif
                        pData = MEM_allocalign(szBuf, nSize, 128, 0, gAptAuxEAGLREALAsyncDecompressIntoMemFlags);
                        assert(pData != NULL);
                        unpack(pOrigData, pData);
#if defined(EA_PLATFORM_GAMECUBE)
                        DCFlushRange(pData, nSize);
#elif defined(EA_PLATFORM_PLAYSTATION2)
                        FlushCache(WRITEBACK_DCACHE);
#endif
                        MEM_free(pOrigData);
                    }
                    void *pAptData;
                    void *pConstData;
                    AptAuxLayerInfo *pLevel = AptAuxEAGLREAL_ResolveData(pData, pAsyncLoads[i].szBaseFilename, &pAptData, &pConstData);

                    pLevel->bBigDataToBeFreed = 1;
                    AptCompleteAnimationAsyncLoad(pAsyncLoads[i].pAsyncLoadContext, pAptData, pConstData, pLevel);
                }
                pAsyncLoads[i].pAsyncLoadContext = AptFilePtr(0);
                pAsyncLoads[i].bValid = false;
                pAsyncLoads[i].pFileBuffer = 0 ;
            }
        }
    }

}
/*F************************************************************************************************/
/*!
default implementation of gAptFuncs.pfnFreeConstantTable callback function. can be overridden
by game team. This can save some memory if we free the constant table after we complete loading of
the big file. This is however up to the game teams.

\Input

\Output

\Notes

\Version
1.0        05/04/04 (DC) Comment creation.

*/
/************************************************************************************************F*/
static void FreeConstantTable(void *pConstTable)
{
}
/*F************************************************************************************************/
/*!
called by free animation to free memory allocated to each level.

\Input

\Output

\Notes

\Version
1.0        05/04/04 (DC) Comment creation.

*/
/************************************************************************************************F*/
void AptAuxEAGLREAL_ReleaseLayerInfo(AptAuxLayerInfo *pLevel)
{
    DynamicLoader *pLoader = pLevel->pGEOData;

    if (pLoader)
    {
        delete pLoader;
        pLoader = 0;
        gAptFuncs.pfnMemFree(pLevel->pGeometryCopy__Base);
        pLevel->pGEOData = 0;
        pLevel->pGeometryCopy__EAGL_HACK = 0;
        pLevel->pGeometryCopy__Base = 0;
    }
#if defined(APT_USE_SOUND_OBJECT)
#if !defined( PSP_EMU )
    if (gAptFuncs.pfnLoadSound == loadSound && pLevel->soundData.pData)
    {
        SNDbankremove(pLevel->soundData.nHandle);
    }
#endif
#endif

    pLevel->nHighestGeometryIndex = 0;
    pLevel->bInUse = 0;
}
/*F************************************************************************************************/
/*!
default implementation of gAptFuncs.pfnFreeAnimation callback function. can be overridden
by game team. This is called when the animation is no longer needed (i.e. when the movie is unloaded.

\Input

\Output

\Notes

\Version
1.0        05/04/04 (DC) Comment creation.

*/
/************************************************************************************************F*/
static void FreeAnimation(void *pUserHandle)
{
    AptAuxLayerInfo *pLevel = (AptAuxLayerInfo *)pUserHandle;

#if defined(EA_PLATFORM_GAMECUBE) || defined(EA_PLATFORM_PLAYSTATION2)
    EAGL::RenderContextExtension::WaitForDrawsToComplete();
#endif

    AptAuxEAGLREAL_ReleaseLayerInfo(pLevel);


    if (pLevel->bBigDataToBeFreed)
    {
        // Due to EAGL quirks, we can't delete big data right now. wait until all data has
        // been released and batch delete all of the big files at one at the end of the frame.
        // (or at shutdown).
        AddDelayedReleasePointer(pLevel->pBigData);
        pLevel->bBigDataToBeFreed = 0;
        pLevel->pBigData = NULL;

    }
}

/*F************************************************************************************************/
/*!
    \Function    getStageHeight

    \Description
        Returns the Height of the Apt Stage in EAGL

    Insert your long description here.

    \Input
        None.

    \Output     float             -

    \Notes
        None.

    \Version    1.0        06/28/04 (LEO) First Version

*/
/************************************************************************************************F*/
float getStageHeight()   {
    float ftmp, height;//, width, height, nearZ, farZ;
    gpEAGLView->GetShape(ftmp, ftmp, ftmp, height, ftmp, ftmp);
    return height;
}

/*F************************************************************************************************/
/*!
    \Function    getStageWidth

    \Description
        Returns the Width of the Apt Stage in EAGL

    Insert your long description here.

    \Input
        None.

    \Output     float             -

    \Notes
        None.

    \Version    1.0        06/28/04 (LEO) First Version

*/
/************************************************************************************************F*/
float getStageWidth()   {
    float ftmp, width;//, width, height, nearZ, farZ;
    gpEAGLView->GetShape(ftmp, ftmp, width, ftmp, ftmp, ftmp);
    return width;
}

/*F************************************************************************************************/
/*!
Sets all apt callbacks to default values.

\Input

\Output

\Notes

\Version
1.0        05/04/04 (DC) Comment creation.

*/
/************************************************************************************************F*/

#if defined(APT_RENDER_FLAGS)
void AptAuxEAGLREAL_PushRenderFlags(const char *pszRenderFlags)
{
    if (pszRenderFlags && pszRenderFlags[0] != 0)
    {
        // aux library can maintain its own stack of render flags if needed and then push the new render flags on to stack
        // then later in draw code it can access the render flags from that stack and apply them as needed.
        // render flags strings can be any thing including name of shader or any other property.
        // Aux library will have to parse the renderflags own its own.
        gAptFuncs.pfnDebugPrint("pushing renderflags - %s\n", pszRenderFlags);
    }
}

void AptAuxEAGLREAL_PopRenderFlags(const char *pszRenderFlags)
{
    if (pszRenderFlags && pszRenderFlags[0] != 0)
    {
        // pop the render flags from the stack.
        gAptFuncs.pfnDebugPrint("poping renderflags - %s\n", pszRenderFlags);
    }
}
#endif



void AptAuxEAGLREAL_SetupCallbacks(void)
{
    gAptFuncs.pfnMemAlloc = MemAlloc;
    gAptFuncs.pfnMemFree = MemFree;
    gAptFuncs.pfnLoadAnimation = LoadAnimation;
    gAptFuncs.pfnFreeAnimation = FreeAnimation;
    gAptFuncs.pfnFreeConstantTable = FreeConstantTable;
#if (APT_DEBUG_LEVEL > APT_DEBUG_NONE)
    gAptFuncs.pfnAssertFail = AssertFail;
#else
    gAptFuncs.pfnAssertFail = NULL;
#endif
#if defined(APT_DO_PROFILE)
    gAptFuncs.pfnWriteProfileData = AptAuxEAGLREAL_WriteProfileData;
#endif
    gAptFuncs.pfnDebugPrint = AptAuxEAGLREAL_DebugPrint;
    gAptFuncs.pfnDebugSetScreenGrabPending = DebugSetScreenGrabPending;
    gAptFuncs.pfnDebugAddSavedInput = DebugAddSavedInput;
    gAptFuncs.pfnCommand = command;
    gAptFuncs.pfnSetExternVariable = command;
    gAptFuncs.pfnGetExternVariable = getVariableDefault;
    gAptFuncs.pfnLoadVariables = loadVariables;
#if defined(APT_USE_SOUND_OBJECT)
    gAptFuncs.pfnLoadSound = loadSound;
    gAptFuncs.pfnFreeSound = freeSound;
    gAptFuncs.pfnStartSound = startSound;
    gAptFuncs.pfnStartSoundStream = startSoundStream;
#endif
    gAptFuncs.pfnLoadTexture = loadTexture;
    gAptFuncs.pfnFreeTexture = freeTexture;
    gAptFuncs.pfnBindTexture = bindTexture;

    gAptFuncs.pfnSetColourTransform = SetColourTransform;
    gAptFuncs.pfnSetVertexMatrix = SetVertexMatrix;
    gAptFuncs.pfnSetBackgroundColour = SetBackgroundColour;
    gAptFuncs.pfnLoadRenderingUnit = loadRenderingUnit;
    gAptFuncs.pfnFreeRenderingUnit = freeRenderingUnit;
    gAptFuncs.pfnDrawRenderingUnit = drawRenderingUnit;
    gAptFuncs.pfnCustomControlRender = customControlRender;
    gAptFuncs.pfnPointHitTest = NULL;
    gAptFuncs.pfnGetRealTimeClock = AptAuxEAGLREAL_GetRealTimeClock;
    gAptFuncs.pfnCustomControlUpdate = customControlUpdate;

    gAptFuncs.pfnAllocateString = allocateStringFF;
    gAptFuncs.pfnDeallocateString = deallocateStringFF;
    gAptFuncs.pfnDrawString = drawStringFF;

    gAptFuncs.pfnSendVariables = sendVariables;
    gAptFuncs.pfnLoadVariablesNULL = loadVariablesNULL;
    gAptFuncs.pfnGetBytesTotal = getBytesTotal ;
    gAptFuncs.pfnGetBytesLoaded = getBytesLoaded ;
    gAptFuncs.pfnLoadAnimationCompleted = LoadAnimationCompleted ;
    gAptFuncs.pfnGetStageHeight = getStageHeight;
    gAptFuncs.pfnGetStageWidth = getStageWidth;
    gAptFuncs.pfnPlaySavedInputsDone = AptAuxEAGLREAL_PlaySavedInputsDone;

    gAptFuncs.pfnHandleZombieState = AptAuxEAGLREAL_HandleZombie;

#if defined(APT_RENDER_FLAGS)
    gAptFuncs.pfnPushRenderFlags = AptAuxEAGLREAL_PushRenderFlags ;
    gAptFuncs.pfnPopRenderFlags = AptAuxEAGLREAL_PopRenderFlags ;
#endif
}

#if defined(EA_PLATFORM_GAMECUBE)
// if u have see linker complains about can't find rm_GouraudApt, possibly because you
// forget to link aptrm.a
extern EAGL::RenderMethod rm_GouraudApt;
extern EAGL::RenderMethod rm_TextureApt;

#elif defined(EA_PLATFORM_PSP)
    extern EAGL::RenderMethod rm_GouraudApt;
    extern EAGL::RenderMethod rm_TextureApt;
    //extern EAGL::RenderMethod rm_Texture;
#endif


#if 0
#define SmallestLargestBlock 300*1024
bool AptAuxEAGLREAL_GoDefrag()
{
    if(MEM_largestunused(FontTextureMemPoolClassID)<SmallestLargestBlock&&MEM_totalunused(FontTextureMemPoolClassID)>SmallestLargestBlock)
        return true;
    else
        return false;
    MEM_totalunused(FontTextureMemPoolClassID),MEM_largestunused(FontTextureMemPoolClassID);
}
#endif

#if defined (EA_PLATFORM_WINDOWS)
// pc using 16bit texture
int gFontTextureMemoryPoolSize = 2*1024*1024;
#elif defined (EA_PLATFORM_XBOX)
// xbox is 8 bit
int gFontTextureMemoryPoolSize = 2*1024*1024;
#elif defined (EA_PLATFORM_PLAYSTATION2)
// ps2 is 4 bit
int gFontTextureMemoryPoolSize = 512*1024;
#elif defined (EA_PLATFORM_PSP)
// psp is 32 bit
int gFontTextureMemoryPoolSize = 2*1024*1024;
#elif defined (EA_PLATFORM_GAMECUBE)
// ngc is 4 bit
int gFontTextureMemoryPoolSize = 512*1024;
#else
    #error platform unknown for TextureMemoryPoolSize!
// there is still other platform?
#endif
void AptAuxEAGLREAL_SetFontTextureMemoryPoolSize(int size)
{
    gFontTextureMemoryPoolSize = size;
}

void _allocFontTextureMemoryPool()
{
#if defined(APT_ALLOCATION_TRACKING)
    FontTextureMemPoolAddr = gAptFuncs.pfnMemAlloc(gFontTextureMemoryPoolSize, __FILE__, __LINE__);
#else
    FontTextureMemPoolAddr = gAptFuncs.pfnMemAlloc(gFontTextureMemoryPoolSize);
#endif
    FontTextureMemPoolClassID = MEMCLASS_getfree();
#if defined(EA_PLATFORM_GAMECUBE)
    int defaultAlign = 32;
    int align = 32;
#elif defined(EA_PLATFORM_WINDOWS)
    int defaultAlign = DEFAULTALIGN;
    int align = DEFAULTALIGN;
#elif defined(EA_PLATFORM_PLAYSTATION2)
    int defaultAlign = DEFAULTALIGN;
    int align = DEFAULTALIGN;
#elif defined(EA_PLATFORM_PSP)
    int defaultAlign    = DEFAULTALIGN;
    int align           = DEFAULTALIGN;
#elif defined(EA_PLATFORM_XBOX)
    int defaultAlign = ALIGNSIZE;
    int align = ALIGNSIZE;
#endif
    MEMCLASS_create(FontTextureMemPoolClassID,"AptFontTextureMemoryPool",FontTextureMemPoolAddr,gFontTextureMemoryPoolSize,defaultAlign,align,4,0,1,0);
}

/*F************************************************************************************************/
/*!
allocates font texture memory, and initializes it. After this it sets up the structures and
function pointers needed to render text later.

\Input

\Output

\Notes

\Version
1.0        05/04/04 (DC) Comment creation.

*/
/************************************************************************************************F*/
void AptAuxEAGLREAL_InitializeFonts(AptAuxFontList *pFonts)
{
    int i;

    _allocFontTextureMemoryPool();

    pAsyncLoads = new AsyncLoads[MAX_NUM_ASYNC_LOADS];

    gpFonts = pFonts;
    FF.init(gpFonts->nFonts, (void **)gpFonts->pData,(int *)gpFonts->nDataSize);

    for (i = 0; i < pFonts->nFonts; i++)
    {
#if defined(APT_ALLOCATION_TRACKING)
        pFonts->aszFontNamesEnglish[i] = (unsigned short *)gAptFuncs.pfnMemAlloc(256, __FILE__, __LINE__);
        pFonts->aszFontNamesJapanese[i] = (unsigned short *)gAptFuncs.pfnMemAlloc(256, __FILE__, __LINE__);
#else
        pFonts->aszFontNamesEnglish[i] = (unsigned short *)gAptFuncs.pfnMemAlloc(256);
        pFonts->aszFontNamesJapanese[i] = (unsigned short *)gAptFuncs.pfnMemAlloc(256);
#endif
        FF.getName(i, pFonts->aszFontNamesEnglish[i], 1033);
        FF.getName(i, pFonts->aszFontNamesJapanese[i], 1041);
    }

    for (i = 0; i < (int)(sizeof(gapImmedCurTARs)/sizeof(gapImmedCurTARs[0])); ++i)
    {
        gapImmedCurTARs[i] = new TAR();
    }
    int nWidth = 8;
    int nHeight = 8;
    gpEmptyShape = AptAuxEAGLREAL_CreateFontTexture(&nWidth, &nHeight, 0,MB_LOW);
#if defined(EA_PLATFORM_XBOX)
    {
        unsigned char *unswizzled = NULL;
        unsigned char *swizzled = (unsigned char *)SHAPE_data(gpEmptyShape);
        int pitch = SHAPE_rowbytes(gpEmptyShape);
        int bpp = SHAPE_depth(gpEmptyShape);

        unswizzled = (unsigned char *) MEM_alloc("swizzle temp", pitch * gpEmptyShape->height, FontTextureMemPoolClassID | MB_ALIGN|MB_HIGH);
        memcpy(unswizzled, swizzled, MEM_size(unswizzled));

        XGSwizzleRect(unswizzled, 0, NULL, swizzled, gpEmptyShape->width, gpEmptyShape->height, NULL, bpp / 8);

        MEM_free(unswizzled);
    }
#endif
#if defined(EA_PLATFORM_GAMECUBE)
    int shapesize = SHAPE_clonesize(gpEmptyShape);
    SHAPE* swizzed = (SHAPE *)MEM_allocalign("GcFontShpSwizzled", shapesize + sizeof(SHAPE), 32, sizeof(SHAPE), FontTextureMemPoolClassID|MB_LOW);
    assert(swizzed != NULL);
    SHAPE_cloneat(swizzed,gpEmptyShape);
    SHAPE_swizzleat(swizzed,gpEmptyShape);
    MEM_free(gpEmptyShape);
    gpEmptyShape = swizzed;
    DCFlushRange(swizzed, SHAPE_size(swizzed));
    SHAPE* clut =SHAPE_clut(swizzed);
    if(clut)
        DCFlushRange(clut, SHAPE_size(clut));
#endif
#if defined(EA_PLATFORM_WINDOWS) || defined (EA_PLATFORM_PSP)
    SHAPE* sixteenbits = From8BitsTo16Bits(gpEmptyShape);
    MEM_free(gpEmptyShape);
    gpEmptyShape = sixteenbits;
#endif

    assert(gpEmptyShape);
    for (i = 0; i < (int)(sizeof(gapImmedCurTARs)/sizeof(gapImmedCurTARs[0])); ++i)
    {
        gapImmedCurTARs[i]->SwapShape(gpEmptyShape);
    }

    DynamicLoader::RegisterVar("ColourModulator", reinterpret_cast<void *>(&gfColourModulator));
    DynamicLoader::RegisterVar("ColourTranslate", reinterpret_cast<void *>(&gfColourTranslate));

#if defined(EA_PLATFORM_GAMECUBE)
    DynamicLoader::RegisterVar("rm_GouraudApt", reinterpret_cast<void *>(&rm_GouraudApt));
    DynamicLoader::RegisterVar("rm_TextureApt", reinterpret_cast<void *>(&rm_TextureApt));

#elif defined(EA_PLATFORM_PSP)
    DynamicLoader::RegisterVar("rm_GouraudApt", reinterpret_cast<void *>(&rm_GouraudApt));
    DynamicLoader::RegisterVar("rm_TextureApt", reinterpret_cast<void *>(&rm_TextureApt));
    //DynamicLoader::RegisterVar("rm_Texture", reinterpret_cast<void *>(&rm_Texture));

    // fusionchange -
    // On PSP rendermethods have an extra property, which tells them when masking is enabled. This
    // causes depth testing to be disabled so that our mask models can render properly. Depth testing
    // is disabled by setting the depth test method to "ALWAYS".
    DynamicLoader::RegisterVar("EnableAptMask", reinterpret_cast<void *>(&gnMaskingEnabled));
    gnMaskingEnabled = 0;
#endif

    // initialize immediate mode stuff
    gpImmedGeoPrimState = new GeoPrimState();
    gpImmedGeoPrimState->SetShading(S_GOURAUD);
    gpImmedGeoPrimState->SetTransparencyMethod(TM_ALPHA);
    gpImmedGeoPrimState->SetAlphaBlendMode(ABM_BLEND) ; // ABM_BLEND    //ABM_OFF
#if !defined(EA_PLATFORM_PSP)
    gpImmedGeoPrimState->EXTENSION_OBJ.SetZWritesEnable(true);
#endif
    gpImmedGeoPrimState->SetDepthTestMethod(DTM_LEQUAL);    //DTM_ALWAYS    //DTM_LEQUAL
    gpImmedGeoPrimState->SetTextureEnable(false);
#if defined (EA_PLATFORM_PLAYSTATION2)
    gpImmedGouraudMethod = new DrawClipGouraud();
    gpImmedTexturedMethod = new DrawClipTextured();
#else
    gpImmedGouraudMethod = new DrawGouraud();
    gpImmedTexturedMethod = new DrawTextured();
#endif

#if defined(EA_PLATFORM_PSP)
    //gpImmedGouraudMethod->Purge();
#endif
    gpImmedGouraudMethod->Init();
#if defined(EA_PLATFORM_PSP)
    //gpImmedTexturedMethod->Purge();
#endif
    gpImmedTexturedMethod->Init();

    gfColourModulator.x = 1.f;
    gfColourModulator.y = 1.f;
    gfColourModulator.z = 1.f;
    gfColourModulator.w = 1.f;
}

/*F************************************************************************************************/
/*!
Interestingly enough the game team calls shutdown fonts, but not initialize fonts... (initialize
Fonts is called from AptAux_Initialize). That aside this mainly mirros the initialize function
by deallocating the memory it creates, and unregistering the data it registers. once this is
complete it moves on to do a bunch of other stuff that really may not belong here...

\Input

\Output

\Notes  This function cleans up after AptAuxInitialize as well as AptAuxEAGLREAL_InitializeFonts
        In the future we should have two clean up functions, to provide a more mirrored interface
        (make things more readable cleaner).

\Version
1.0        05/04/04 (DC) Comment creation.

*/
/************************************************************************************************F*/
void AptAuxEAGLREAL_ShutdownFonts(void)
{
    int i;
    for (i = 0; i < gpFonts->nFonts; i++)
    {
        gAptFuncs.pfnMemFreeSize(gpFonts->aszFontNamesEnglish[i], 256);
        gAptFuncs.pfnMemFreeSize(gpFonts->aszFontNamesJapanese[i], 256);
    }
    // this was added in somewhere 0.9.0 for multiple testcases when viewerextracted in eagl_Real_viewer was
    // added but there was a memory leak for font files in AptViewer (not in AptAuxEAGLREAL)
    // and so more than 50 tests could not be executed as free memory was going down.
    // so commented this out. hopefully this will not cause problem for teams using AptAuxEAGLREAL.
    // as this was set to 0, gFonts->nFonts was also set to zero and fonts (.ttf) were never deleted in viewer.
    // once again memeory leak for font files was in viewer and not in AptAuxEAGLREAL.
    //gpFonts->nFonts = 0 ;
    DynamicLoader::UnRegisterVar("ColourModulator");
    DynamicLoader::UnRegisterVar("ColourTranslate");

#if defined(EA_PLATFORM_GAMECUBE)
    DynamicLoader::UnRegisterVar("rm_GouraudApt");
    DynamicLoader::UnRegisterVar("rm_TextureApt");
#elif defined(EA_PLATFORM_PSP)
    DynamicLoader::UnRegisterVar("rm_GouraudApt");
    DynamicLoader::UnRegisterVar("rm_TextureApt");
    //DynamicLoader::UnRegisterVar("rm_Texture");
#endif

    delete gpImmedGeoPrimState;
    gpImmedGeoPrimState = 0;

#if defined(EA_PLATFORM_PLAYSTATION2)
    delete gpImmedGouraudMethod;
    gpImmedGouraudMethod = 0;
    delete gpImmedTexturedMethod;
    gpImmedTexturedMethod = 0;
#else
    delete gpImmedGouraudMethod;
    gpImmedGouraudMethod = 0;
    delete gpImmedTexturedMethod;
    gpImmedTexturedMethod = 0;
#endif

    // If there are any other files loading, cancel them.
    for (i = 0; i < MAX_NUM_ASYNC_LOADS; i++)
    {
        if (pAsyncLoads[i].bValid)
        {
            ASYNCFILE_cancel(pAsyncLoads[i].nLoadHandle);
        }
    }

    // Wait for the cancelations to complete.
    bool bSomeStillValid = true;
    while (bSomeStillValid)
    {
        AptAuxEAGLREAL_FinishAsyncLoads();
        SYNCTASK_run();
        bSomeStillValid = false;

        for (i = 0; i < MAX_NUM_ASYNC_LOADS; i++)
        {
            if (pAsyncLoads[i].bValid)
            {
                bSomeStillValid = true;
            }
        }
    }

    delete [] pAsyncLoads;
    pAsyncLoads = 0;

    for(int j=0;j<DEADSTRINGPOOLS*2;j++)
    {
        AptAuxEAGLREAL_CleanUpOldStrings();
    }

    for (i = 0; i < (int)(sizeof(gapImmedCurTARs)/sizeof(gapImmedCurTARs[0])); ++i)
    {
        gapImmedCurTARs[i]->SwapShape(0);
        delete gapImmedCurTARs[i];
        gapImmedCurTARs[i] = 0;
    }

    //MEMCLASS_purge(FontTextureMemPoolClassID);
    MEMCLASS_remove(FontTextureMemPoolClassID);
    FontTextureMemPoolClassID= 0;
    gAptFuncs.pfnMemFreeSize(FontTextureMemPoolAddr, gFontTextureMemoryPoolSize);
    FontTextureMemPoolAddr = 0;

    gpFonts = 0;
    gpEmptyShape = 0;

    FF.purge();

    // Free any other delayed reelease pointers accumulated during shutdown.
    FreeDelayedReleasePointers();

    // 2004/04/27 -- DC -- WARNING! due to design issues this memory is deallocated
    //                     in the AptAuxEAGLREAL_ShutdownFonts function. We plan on
    //                     cleaning up the interface to AptAux in the near future
    //                     and providing a cleaner method for this.
    assert(gLevelInfo);
    gAptFuncs.pfnMemFreeSize( gLevelInfo,  sizeof(AptAuxLayerInfo) * pCurrentAptAuxParams->iLevelInfoSize );
    gLevelInfo = NULL;

    assert(allocStrings);
    gAptFuncs.pfnMemFreeSize( allocStrings, sizeof(AllocatedString) * pCurrentAptAuxParams->iMaxStringCount );
    allocStrings = NULL;

    assert(gpDelayedReleasePointers);
    gAptFuncs.pfnMemFreeSize( gpDelayedReleasePointers, pCurrentAptAuxParams->iMaxDelayedReleaseValues*sizeof(void*) );
    gpDelayedReleasePointers = NULL;

#if defined(ALLOW_PLAY_INPUTS) || defined(DO_COVERAGE)
    if(gpSavedInputBuffer != NULL)
    {
        gAptFuncs.pfnMemFreeSize(gpSavedInputBuffer, gnSavedInputBufferSize);
    }
    gpSavedInputBuffer = NULL;
#endif

#if defined(ALLOW_SAVE_INPUTS) || defined(DO_COVERAGE)
    if (fdSavedInputs != -1)
    {
        #if defined(EA_PLATFORM_PLAYSTATION2)
            sceClose(fdSavedInputs);
        #elif defined(EA_PLATFORM_WINDOWS)
            _close(fdSavedInputs);
        #elif defined(EA_PLATFORM_XBOX)
            fflush((FILE*)fdSavedInputs);
            fclose((FILE*)fdSavedInputs);
        #endif
    }
    fdSavedInputs = -1;
#endif
}
/*F************************************************************************************************/
/*!
Setup the Apt Aux Library. This takes viewport and context info about our EAGL environment and
sets up all that is needed for Apt aux to get going.

\Input      pView    -
            pContext -
            szFilePrefix -
            pFonts  -
            pInitParams - Optional input parameter.
\Notes
This calls AptAuxEAGLREAL_InitializeFonts, but game has to call AptAuxEAGLREAL_ShutdownFonts...

\Version
1.0        07/15/03 (ONA) First Version

*/
/************************************************************************************************F*/
void AptAuxEAGLREAL_Initialize(EAGL::ViewPort *pView,
                               EAGL::RenderContext *pContext,
                               char *szFilePrefix,
                               AptAuxFontList *pFonts,
                               const AptAuxInitParmsT *pInitParams)
{
    pCurrentAptAuxParams = pInitParams;

    if (gAptFuncs.pfnMemAlloc == 0 || gAptFuncs.pfnMemFree == 0)
    {
        AptAuxEAGLREAL_SetupCallbacks();
    }

    // 2004/04/27 -- DC -- WARNING! due to design issues this memory is deallocated
    //                     in the AptAuxEAGLREAL_ShutdownFonts function. We plan on
    //                     cleaning up the interface to AptAux in the near future
    //                     and providing a cleaner method for this.
#if defined(APT_ALLOCATION_TRACKING)
    gLevelInfo = (AptAuxLayerInfo*)gAptFuncs.pfnMemAlloc( sizeof(AptAuxLayerInfo)*pInitParams->iLevelInfoSize, __FILE__, __LINE__);
#else
    gLevelInfo = (AptAuxLayerInfo*)gAptFuncs.pfnMemAlloc( sizeof(AptAuxLayerInfo)*pInitParams->iLevelInfoSize);
#endif
    assert(gLevelInfo);

    memset(gLevelInfo, 0, sizeof(AptAuxLayerInfo)*pInitParams->iLevelInfoSize) ;

    gpEAGLView = pView;
    gpEAGLRenderContext = pContext;
    strcpy(gszFilePrefix, szFilePrefix);

    gnTarNumberForBatchDraw = -1;

#if defined(EA_PLATFORM_WINDOWS)
    DeviceInfo* pDeviceInfo = EAGL::RenderContextExtension::GetDeviceInfo();  // RenderContext::EXTENSION_OBJ::GetDeviceInfo();
    if ((pDeviceInfo->devCaps.PrimitiveMiscCaps & D3DPMISCCAPS_COLORWRITEENABLE) == 0)
    {
        gnMaskColour = 0x0;
    }
    else
#endif
    {
        gnMaskColour = 0xffff00ff;
    }

    gbNeverClearZ = false;

    // (LEO) 06.08.04 FR521 Make FFB_MAXSTRINGS dynamic
    // This is Deleted in AptAuxEAGLREAL_ShutdownFonts for now.
    assert(allocStrings == NULL);
#if defined(APT_ALLOCATION_TRACKING)
    allocStrings = (AllocatedString*)gAptFuncs.pfnMemAlloc(pCurrentAptAuxParams->iMaxStringCount*sizeof(AllocatedString), __FILE__, __LINE__);
#else
    allocStrings = (AllocatedString*)gAptFuncs.pfnMemAlloc(pCurrentAptAuxParams->iMaxStringCount*sizeof(AllocatedString));
#endif
    memset(allocStrings, 0,pCurrentAptAuxParams->iMaxStringCount*sizeof(AllocatedString));

    // (DC)
    assert(gpDelayedReleasePointers == NULL);
#if defined(APT_ALLOCATION_TRACKING)
    gpDelayedReleasePointers = (void**)gAptFuncs.pfnMemAlloc(pCurrentAptAuxParams->iMaxDelayedReleaseValues*sizeof(void*), __FILE__, __LINE__);
#else
    gpDelayedReleasePointers = (void**)gAptFuncs.pfnMemAlloc(pCurrentAptAuxParams->iMaxDelayedReleaseValues*sizeof(void*));
#endif
    memset(gpDelayedReleasePointers, 0,pCurrentAptAuxParams->iMaxDelayedReleaseValues*sizeof(void*));

    if (gpFonts == 0 && pFonts != 0)
    {
        AptAuxEAGLREAL_InitializeFonts(pFonts);
    }
}
/*F************************************************************************************************/
/*!
deallocated strings are added to a dead List which is freed at the end of a frame or on shutdown.
Thus the game needs to call this every frame (or whenever they want their memory back...)

\Input

\Notes

\Version
1.0        07/15/03 (ONA) First Version

*/
/************************************************************************************************F*/
void AptAuxEAGLREAL_CleanUpOldStrings(void)
{
    int nExpiredPool = (nCurrentDSPool + 1) % DEADSTRINGPOOLS;
    DeadStringPool* pCurrentPool = aDeadStringPools + nExpiredPool;

    for (int i = 0; i < pCurrentPool->nDeadStrings; i++)
    {
        if(pCurrentPool->aDeadStrings[i] && (void *)pCurrentPool->aDeadStrings[i] != (void *)0xffffffff)
            SHAPE_destroy(pCurrentPool->aDeadStrings[i]);
    }
    pCurrentPool->nDeadStrings = 0;
    nCurrentDSPool = nExpiredPool;
}

// added as request from def jam team -FR #348 Access to custom control vertexMatrix
MATRIX4* AptAuxEAGLREAL_GetCustomControlMatrix()
{
    return &vertexMatrix ; // return whatever is current vertex matrix
}

