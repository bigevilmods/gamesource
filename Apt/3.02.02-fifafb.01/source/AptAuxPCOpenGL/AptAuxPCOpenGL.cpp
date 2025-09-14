/*!
\addtogroup g_referenceguide        Reference Guides
*/
//@{
/*!
\Module AptAuxPCOpenGL
*/
//@}

#include "Apt.h"
#include "AptAuxPCOpenGL.h"

#include "AptValueFactory.h"

#include <windows.h>
#include <crtdbg.h>
#include <gl/gl.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "bigfile.h"
#include "timer.h"
#include "math.h"
#include "ffapt.h"

#define APTAUX_UNREF(_) ((void)_)

// nvidia ogl stuff
#ifndef APT_FALLBACK_RENDERING
//#define GLH_EXT_SINGLE_FILE
//#include <glh_extensions.h>
#endif

//#define PRETEND_GAME

static float colourScale[4];
static float colourTranslate[4];
static float vertexMatrix[16];
static float vertexMatrixIntegralTranslate[16];
static AptAuxLayerInfo gLevelInfo[128];
static int gnActiveMasks;
static FILE *savedInputsFile;
AptAuxFontList *gpFonts;
static AptSavedInputRecord *gaSavedInputs;
static int gnSavedInputFileSize;
static int gnPlaySavedInputsToFrame;
struct LoadParams
{
    char szName[256];
    AptAuxLayerInfo *pLevel;
    AptFilePtr pAsyncLoadContext;
    void *pMainData;
    void *pConstTable;
};
static HANDLE hLoadThreadsMutex;
static LoadParams *aFinishedLoads[256];
static int nFinishedLoads = 0;
static bool gbVerifyUnresolve = false;
static bool gbUnitTestDoRandomLoading = false;

// these declarations let us use glext functionality without requiring glext.h
typedef void (APIENTRY * ActiveTextureARBFunc) (GLenum target);
typedef void (APIENTRY * ClientActiveTextureARBFunc) (GLenum target);
typedef void (APIENTRY * MultiTexCoord2fARBFunc) (GLenum target, GLfloat s, GLfloat t);
typedef void (APIENTRY * MultiTexCoord2fvARBFunc) (GLenum target, const GLfloat *v);
static const int GL_TEXTURE0_ARB = 0x84C0;
static const int GL_TEXTURE1_ARB = 0x84C1;
static const int GL_MAX_TEXTURE_UNITS_ARB = 0x84E2;
static const int GL_COMBINE_ARB = 0x8570;
static const int GL_COMBINE_RGB_ARB = 0x8571;
static const int GL_COMBINE_ALPHA_ARB = 0x8572;
static const int GL_SOURCE0_RGB_ARB = 0x8580;
static const int GL_SOURCE1_RGB_ARB = 0x8581;
static const int GL_CONSTANT_ARB = 0x8576;
static const int GL_PRIMARY_COLOR_ARB = 0x8577;
static const int GL_PREVIOUS_ARB = 0x8578;
static const int GL_OPERAND0_RGB_ARB = 0x8590;
static const int GL_OPERAND1_RGB_ARB = 0x8591;
static const int GL_SOURCE1_ALPHA_ARB = 0x8589;
static const int  GL_SOURCE0_ALPHA_ARB = 0x8588;
static const int  GL_OPERAND0_ALPHA_ARB = 0x8598;
static const int  GL_OPERAND1_ALPHA_ARB = 0x8599;

ActiveTextureARBFunc        glActiveTextureARB;
ClientActiveTextureARBFunc  glClientActiveTextureARB;
MultiTexCoord2fARBFunc      glMultiTexCoord2fARB;
MultiTexCoord2fvARBFunc glMultiTexCoord2fvARB;

// == REAL FILE_loadsize
static void *_loadFileSize(const char *szFile, int *pnSize)
{
    assert(szFile);
    FILE *in = fopen(szFile, "rb");

    int size;
    void *pData;

    if (!in)
    {
        char szError[256];
        sprintf(szError, "Couldn't open file: '%s'. (Did you build it first?)", szFile);
        OutputDebugString(szError);
#ifndef PRETEND_GAME
//      MessageBox(NULL, szError, "Error", MB_ICONEXCLAMATION | MB_OK);
#endif
        return NULL;
    }

    fseek(in, 0, SEEK_END);
    size = ftell(in);
    pData = malloc(size);
    rewind(in);
    fread(pData, size, 1, in);
    fclose(in);

    *pnSize = size;
    return pData;
}

static void *_loadFile(const char *szFile)
{
    int size;
    return _loadFileSize(szFile, &size);
}

static void _saveFile(const char *szFile, void *pData, int nSize)
{
    FILE *out = fopen(szFile, "wb");
    assert(out);
    fwrite(pData, nSize, 1, out);
    fclose(out);
}


#if defined(APT_ALLOCATION_TRACKING)
void *mymalloc(unsigned int nSize, const char *szFile, int nLine)
#else
void *mymalloc(unsigned int nSize)
#endif
{
    void *pTemp = malloc(nSize);
    return pTemp;
}

#if defined(APT_ALLOCATION_TRACKING)
#define APTAUX_MALLOC_BLOCK( size )  mymalloc((size), __FILE__, __LINE__)
#else
#define APTAUX_MALLOC_BLOCK( size )  mymalloc((size))
#endif
#define APTAUX_FREE_BLOCK( p, size ) free((p))


static void command(const char *szCommand, const char *szParams)
{
    // if you take this out, be prepared inputmask test will fail
    if(strcmp(szCommand,"setinputroot")==0)
    {
        if(strcmp(szParams,"void")==0)
        {
            AptSetInputRoot(0);
        }else
        {
            AptSetInputRoot(szParams);
        }
    }
    else if (strcmp(szCommand, "do_random_loading") == 0) // required for "randomloader" test case
    {
        gbUnitTestDoRandomLoading = 1;
    }
    else if (strcmp(szCommand, "CreateLeaks") == 0)      // Required for "CreateLeaks" Test case
    {
        // Create a bunch of stuff that won't be deleted.
        APTAUX_MALLOC_BLOCK(0x4);
        APTAUX_MALLOC_BLOCK(0x10);
        APTAUX_MALLOC_BLOCK(0x40);
        APTAUX_MALLOC_BLOCK(0x80);
        APTAUX_MALLOC_BLOCK(0xFF);
        APTAUX_MALLOC_BLOCK(0x100);
        APTAUX_MALLOC_BLOCK(0x101);
        APTAUX_MALLOC_BLOCK(0x500);
        APTAUX_MALLOC_BLOCK(0x1003);
        APTAUX_MALLOC_BLOCK(0x4001);
    }
    gAptFuncs.pfnDebugPrint("external command: %s('%s')\n", szCommand, szParams);
}

bool isUTF8File(char* stream)
{
    // signature of the UTF-8,
    // I am not sure if this sig is global standard, or just internal sig for ms notepad
    unsigned char *pData =(unsigned char *)stream;
    return pData[0] == 0xef && pData[1] == 0xbb && pData[2] == 0xbf;
}
bool isUCS2File(char* stream)
{
    // signature of the Unicode (16bit),
    // I am not sure if this sig is global standard, or just internal sig for ms notepad
    unsigned char *pData =(unsigned char *)stream;
    return pData[0] == 0xff && pData[1] == 0xfe;
}
int UTF8Size(unsigned short* buf, int buflen)
{
    int total=0;
    for(int i=0;i<buflen;i++)
    {
        if(buf[i] <= 0x7f)
        {
            total++;
        }else
        if(buf[i] <= 0x7ff)
        {
            total+=2;
        }else
        {
            total+=3;
        }
    }
    return total;
}

void UCS2ToUTF8(unsigned short* src, int buflen,char* dst)
{
    char* ptr = dst;
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
        if(src[i] <= 0x7f)
        {   //0000 0000-0000 007F   0xxxxxxx
            *dst = (char)(src[i]);
            ++dst;
        }else
        if(src[i] <= 0x7ff)
        {   //0000 0080-0000 07FF   110xxxxx 10xxxxxx
            *dst = 0xc0 | ((src[i] & 0x07c0)>> 6);
            ++dst;
            *dst = 0x80| (src[i] & 0x003f);
            ++dst;
        }else
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
void UTF8ToUCS2(char* src,int len, wchar_t* dst)
{
    unsigned char* pSrc = (unsigned char*)src;
    wchar_t* pDst = dst;
    while((char*)pSrc<src+len)
    {
        if(*pSrc<=127)      //0000 0000-0000 007F   0xxxxxxx
        {
            *pDst = *pSrc;
            ++pDst;
            ++pSrc;
        }else if((*pSrc&0xe0) == 0xc0)  //0000 0080-0000 07FF   110xxxxx 10xxxxxx
        {
            assert((pSrc[1] & 0xc0) == 0x80);
            *pDst = ((pSrc[0] & 0x1f) <<6) | (pSrc[1] & 0x3f);
            ++pDst;
            pSrc+=2;
        }else if((*pSrc&0xf0) == 0xe0)
        {
            assert((pSrc[1] & 0xc0) == 0x80);
            assert((pSrc[2] & 0xc0) == 0x80);
            *pDst = ((pSrc[0] & 0x0f) <<12) | ((pSrc[1] & 0x3f) << 6) | (pSrc[2] & 0x3f);
            ++pDst;
            pSrc+=3;
        }else
        {
            assert(0&&"UCS4 is not supported");
        }
    }
}


// This function will be called because of execution of getBytesTotal for
// movieclip, XML, loadVars or Sound.
// Out of these XML case is handled by the XML interface exposed by Apt.
// Game teams either fill up this function with their code or they can redefine this callback function
int  getBytesTotal(const char * szFileName, AptGetBytesEnum eGetBytes)
{

    int iRet = 0 ;
    switch(eGetBytes)
    {
    case AptGetBytesEnum_MovieClip:
        {
            // for movieclip option aux library should keep a integer field in its Level structure
            // that keeps track of file size.
            // here iterate thru all the files loaded and then compare the szFileName with filename stored in
            // that structure and return the filesize.

            char szBaseFilename[256];
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

            strcat(szBaseFilename, ".big");
            FILE *in = fopen(szBaseFilename, "rb");
            int size;

            if (!in)
            {
                break ;
            }

            fseek(in, 0, SEEK_END);
            size = ftell(in);
            fclose(in);

            iRet = size;
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

    gAptFuncs.pfnDebugPrint("getBytesTotal  - '%s' Type:'%d Size:0x%X\n", szFileName, eGetBytes, iRet);
    return iRet ;
}


// This function will be called because of execution of getBytesLoaded for
// movieclip, XML, loadVars or Sound.
// Out of these XML case is handled by the XML interface exposed by Apt.
// Game teams either fill up this function with their code or they can redefine this callback function
int  getBytesLoaded(const char * szFileName, AptGetBytesEnum eGetBytes)
{
    int iRet = 0 ;
    switch(eGetBytes)
    {
    case AptGetBytesEnum_MovieClip:
        {
            return getBytesTotal(szFileName, eGetBytes);
        }
        break ;
    case AptGetBytesEnum_Sound:
        {
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

    gAptFuncs.pfnDebugPrint("getBytesLoaded - '%s' Type:'%d Size:0x%X\n", szFileName, eGetBytes, iRet);
    return iRet ;
}



// parameter bIsSendAndLoad denotes whether this call was from LoadVars.send or LoadVars.sendAndLoad.
// If call is from sendAndLoad then there will be a successive call to pfnLoadVariables withh NULL parameter.
// This function should keep track of that and use that in pfnLoadVariables function to give a proper response
// back to Apt.
void sendVariables(const char * szUrl, const char * szTarget, const char *szMethod, const char *szProp, int bIsSendAndLoad)
{
    gAptFuncs.pfnDebugPrint("sendVariables : '%s' at target '%s ----%s ----%s '\n", szProp, szTarget, szMethod, szUrl);
    return ;
}


// this call is precedded by a call to pfnSendVariables.
// This happens in case of LoadVars.sendAndLoad. Apt will first call pfnSendVariables
// and then it will call this function (loadVariablesNULL).
// In this case auxiliary library is expected to send response back to Apt.
// We introduced another pfnLoaVariablesNULL function that gets called exclusively from LoadVars.sendAndLoad
// to avoid confusion about NULL parameter. It is not much different
// except that it will not have any input parameter. We added a extra flag to
// pfnSendVariables function to denote whether there will be a call to pfnLoadVariableNULL.
static AptValue *loadVariablesNULL()
{
    // this call was precedded by a call to pfnSendVariables, and so send the response back to Apt depending on game requirements
    // Here we(Apt Dev team)  are just adding this case to the function so that function will return valid value.
    // users can use this block and write code to send response back to Apt
    gAptFuncs.pfnDebugPrint("loadVariables: called with NULL parameters\n");
    AptValue * pValue = AptValueFactory::CreateString("");
    return pValue;

}

static AptValue *loadVariables(const char *szCommand)
{
    /*
    if (strcmp(szCommand, "a") == 0)
    {
        char *z = "a=7&bcd=99&xyz=&=&aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa=zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz&lastone=55";
        char *data = (char *)malloc(strlen(z) + 1);
        strcpy(data, z);
        return data;
    }
    */
    int size;
    AptValue * pValue = NULL;
    char* pData =(char*)_loadFileSize(szCommand,&size);
    if (!pData)
    {
        pValue = AptValueFactory::CreateString("");
        return pValue;
    }
    char* pRet = 0;
    if(isUTF8File(pData))
    {
        // UTF8
        // remove the first 3 bytes sig. and plus a ending zero
        pRet = (char *) APTAUX_MALLOC_BLOCK( (size -3 + 1) );
        memcpy(pRet,pData+3,size -3);
        pRet[size -3] = 0;
        pValue = AptValueFactory::CreateString(pRet);
        APTAUX_FREE_BLOCK(pRet, (size-3+1));
    }else if(isUCS2File(pData))
    {
        // UCS2
        int sizeInUTF8 = UTF8Size((unsigned short*)(pData+2),(size-2)/2);
        pRet = (char *) APTAUX_MALLOC_BLOCK( (sizeInUTF8 + 1) );
        UCS2ToUTF8((unsigned short*)(pData+2),(size-2)/2 , pRet);
        pRet[sizeInUTF8] = 0;
        pValue = AptValueFactory::CreateString(pRet);
        APTAUX_FREE_BLOCK(pRet, (sizeInUTF8 + 1));

    }else
    {
        // ansi
        pRet = (char *) APTAUX_MALLOC_BLOCK( (size + 1) );
        memcpy(pRet,pData,size);
        pRet[size] = 0;
        pValue = AptValueFactory::CreateString(pRet);
        APTAUX_FREE_BLOCK(pRet, (size + 1));

    }

    free(pData);
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
1.0         05/04/04 (DC)       Comment creation.
1.1         12/21/2004 (DUG)    Replaced macros for memory allocation by CreateString().

*/
/************************************************************************************************F*/
static AptValue *getVariableDefault(const char *szVar)
{
    char szString[4096];
    AptValue * pValue = NULL;

    if (strcmp(szVar, "in_apt") == 0)
    {
        strcpy(szString, "1");
    }
    else if (strcmp(szVar, "gPlatform") == 0)
    {

        #if defined (PS2)
            char szPlatform[20] = "PS2" ;   //_APTVIEWER
        #elif defined (PSP)
            char szPlatform[20] = "PSP" ;   //_APTVIEWER
        #elif defined (XBOX)
            char szPlatform[20] = "XBOX" ;  //_APTVIEWER
        #elif defined (PC)
            char szPlatform[20] = "PC" ;    //_APTVIEWER
        #elif defined (NGC)
            char szPlatform[20] = "GC" ;    //_APTVIEWER
        #else
            char szPlatform[20] = "" ;    //_APTVIEWER
        #endif

        pValue = AptValueFactory::CreateString(szPlatform);
        return pValue;
    }
    else
    {
        // This is needed for extern test in testsuite.
        sprintf(szString, "<value of '%s'>", szVar);
    }

    pValue = AptValueFactory::CreateString(szString);
    return pValue;
}

void debugPrint(const char *szFormat, ...)
{
    va_list args;
    char szBuf[8192];

    va_start(args, szFormat);
    _vsnprintf(szBuf, 8192, szFormat, args);
    va_end(args);
    OutputDebugString(szBuf);
    printf("%s", szBuf);
}

void debugSetScreenGrabPending(const char *szScreenGrabName)
{
}

void debugAddSavedInput(AptSavedInputRecord *pRecord, int nSize)
{
    if (!savedInputsFile)
    {
        savedInputsFile = fopen("saved.inputs", "wb");
        assert(savedInputsFile);
    }

    fwrite(pRecord, nSize, 1, savedInputsFile);
}


#define APTAUX_RESOLVE(_) { APT_ASSERT((uintptr_t)_ < 0xfffff); if ((_) != 0) *((char **)(&_)) = ((char *)((uintptr_t)pBase) + ((uintptr_t)_)); }


static void _loadGeometry(const char *szBaseName, AptAuxLayerInfo *pLevel)
{
    char szFilename[256];
    strcpy(szFilename, szBaseName);
    strcat(szFilename, ".geo");

    AptAuxGeoInfo *pInfo = pLevel->pGEOData = (AptAuxGeoInfo *) BIG_locate(pLevel->pBigData, szFilename);

    assert(pLevel->pGEOData && "Couldn't find .geo file");

    if (strcmp(pInfo->szMagic, "Apt PCGL geometry file\032") == 0)
    {
        pInfo->szMagic[0] = 'R';
        void *pBase = pInfo;
        APTAUX_RESOLVE(pInfo->aShapes);
        for (int i = 0; i < pInfo->nShapeIDs; i++)
        {
            // Insert the geometry piece into the global ID->data translation table
            assert(pInfo->aShapes[i].nID < APTAUX_MAXCHARSPERLEVEL);
            assert(pLevel->aIDToChar[pInfo->aShapes[i].nID].eType == AptAuxCharacterType_None);
            pLevel->aIDToChar[pInfo->aShapes[i].nID].eType = AptAuxCharacterType_Geometry;
            pLevel->aIDToChar[pInfo->aShapes[i].nID].pData = &pInfo->aShapes[i];

            // Resolve the rest of the stuff
            APTAUX_RESOLVE(pInfo->aShapes[i].apUnits);
            for (int j = 0; j < pInfo->aShapes[i].nUnits; j++)
            {
                APTAUX_RESOLVE(pInfo->aShapes[i].apUnits[j]);
                APTAUX_RESOLVE(pInfo->aShapes[i].apUnits[j]->prims.pData);
//              assert(pInfo->aShapes[i].apUnits[j]->prims.nPrims < 200);   // debug
            }
        }
    }
    else
    {
        assert(strcmp(pInfo->szMagic, "Rpt PCGL geometry file\032") == 0); // if it's not what we were expecting, it better be the resolved version
    }
}


#pragma pack(push, 1)
typedef struct
{
   char  idlength;
   char  colourmaptype;
   char  datatypecode;
   short int colourmaporigin;
   short int colourmaplength;
   char  colourmapdepth;
   short int x_origin;
   short int y_origin;
   short width;
   short height;
   char  bitsperpixel;
   char  imagedescriptor;
} TGAHEADER;
#pragma pack(pop)

static AptAssetTexture loadTexture(void *pUserData, int nID)
{
    AptAuxLayerInfo *pLevel = (AptAuxLayerInfo *)pUserData;
    int y=0, x=0;
    char szBuf[MAX_PATH];
    sprintf(szBuf, "%d.tga", nID);
    unsigned char *pData = (unsigned char *)BIG_locate(pLevel->pBigData, szBuf);

    if (!pData) return NULL;

    unsigned char *pFrameBuffer;
    TGAHEADER *pHeader = (TGAHEADER *)pData;
    unsigned int nTexID;

    // worst tga loading code ever
    pFrameBuffer = pData + sizeof(TGAHEADER);

    glGenTextures(1, &nTexID);
    glBindTexture(GL_TEXTURE_2D, nTexID);

    // tga is bgra, we want rgba. oh, and it's upside down
    for (y = 0; y < pHeader->height; y++)
    {
        for (x = 0; x < pHeader->width; x++)
        {
            unsigned char temp = pFrameBuffer[y*pHeader->width*4+x*4+0];
            pFrameBuffer[y*pHeader->width*4+x*4+0] = pFrameBuffer[y*pHeader->width*4+x*4+2];
            pFrameBuffer[y*pHeader->width*4+x*4+2] = temp;
        }
    }
    for (y = 0; y < pHeader->height/2; y++)
    {
        unsigned char *pSrc = &pFrameBuffer[y*pHeader->width*4];
        unsigned char *pDest = &pFrameBuffer[((pHeader->height)-y-1)*pHeader->width*4];
        for (x = 0; x < pHeader->width*4; x++)
        {
            unsigned char temp = *pSrc;
            *pSrc = *pDest;
            *pDest  = temp;
            pSrc++; pDest++;
        }
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, pHeader->width, pHeader->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pFrameBuffer);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    return (AptAssetTexture)nTexID;
}

static void freeTexture(AptAssetTexture texture)
{
    unsigned int nTexID = (unsigned int)texture;
    glDeleteTextures(1, &nTexID);
    texture = NULL;
}

static void bindTexture(void *pUserData, int nID, AptAssetTexture texture)
{
    AptAuxLayerInfo *pLevel = (AptAuxLayerInfo *)pUserData;
    //  assert(pLevel->aIDToChar[nID].eType == AptAuxCharacterType_None);
    pLevel->aIDToChar[nID].eType = AptAuxCharacterType_Texture;
    pLevel->aIDToChar[nID].pData = texture;

    // fix scale for textures
    for (int i = 0; i < APTAUX_MAXCHARSPERLEVEL; i++)
    {
        if (pLevel->aIDToChar[i].eType != AptAuxCharacterType_Geometry) continue;
        AptAuxShape *pShape = (AptAuxShape *) pLevel->aIDToChar[i].pData;
        for (int iUnit = 0; iUnit < pShape->nUnits; iUnit++)
        {
            AptAuxUnit *pUnit = pShape->apUnits[iUnit];
            switch (pUnit->eType)
            {
                case AptAuxUnitType_TriTiled:
                case AptAuxUnitType_TriClipped:
                {
                    int nTexID = (int)pLevel->aIDToChar[pUnit->triClipped.nBitmapCharID].pData;
                    if ((int)texture == nTexID)
                    {
                        pUnit->triClipped.nTextureID = nTexID;
                        int nWidth, nHeight;
                        glBindTexture(GL_TEXTURE_2D, nTexID);
                        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &nWidth);
                        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &nHeight);

                        glPushMatrix();
                        glLoadIdentity();
                        glScalef(1.f/nWidth, 1.f/nHeight, 1);
                        glMultMatrixf(pUnit->triClipped.afMatrix);
                        glGetFloatv(GL_MODELVIEW_MATRIX, pUnit->triClipped.afMatrix);
                        glPopMatrix();
                    }
                }
                break;
            }
        }
    }


}

static void setColourTransform(AptCXForm *pCXForm)
{
    pCXForm->scale.CopyToFloatArray4(colourScale);
    pCXForm->translate.CopyToFloatArray4(colourTranslate);
}

#if defined (APT_3D)
#include "AptStd/AptMath.h"
static void setVertexMatrix(AptMath::Mat44T *pMatrix)
{
    memset(vertexMatrix, 0, sizeof(vertexMatrix));
    vertexMatrix[0] = pMatrix->m[AptMath::MAT_A];
    vertexMatrix[1] = pMatrix->m[AptMath::MAT_B];
    vertexMatrix[2] = pMatrix->m[AptMath::MAT_C];
    vertexMatrix[4] = pMatrix->m[AptMath::MAT_D];
    vertexMatrix[5] = pMatrix->m[AptMath::MAT_E];

    vertexMatrix[6] = pMatrix->m[AptMath::MAT_F];
    vertexMatrix[8] = pMatrix->m[AptMath::MAT_G];
    vertexMatrix[9] = pMatrix->m[AptMath::MAT_H];


    vertexMatrix[10] = pMatrix->m[AptMath::MAT_I];
    vertexMatrix[12] = pMatrix->m[AptMath::MAT_X];
    vertexMatrix[13] = pMatrix->m[AptMath::MAT_Y];
    vertexMatrix[14] = pMatrix->m[AptMath::MAT_Z];
    vertexMatrix[15] = 1.f ;

    memcpy(vertexMatrixIntegralTranslate, vertexMatrix, sizeof(vertexMatrix));
}
#else
static void setVertexMatrix(AptMatrix *pMatrix)
{
    memset(vertexMatrix, 0, sizeof(vertexMatrix));
    vertexMatrix[0] = pMatrix->a;
    vertexMatrix[1] = pMatrix->b;
    vertexMatrix[4] = pMatrix->c;
    vertexMatrix[5] = pMatrix->d;
    vertexMatrix[10] = 1.f;
    vertexMatrix[12] = pMatrix->tx;
    vertexMatrix[13] = pMatrix->ty;
    vertexMatrix[15] = 1.f;
    memcpy(vertexMatrixIntegralTranslate, vertexMatrix, sizeof(vertexMatrix));
#if 0
    vertexMatrixIntegralTranslate[12] = (float)((int)vertexMatrix[12]);
    vertexMatrixIntegralTranslate[13] = (float)((int)vertexMatrix[13]);
    vertexMatrixIntegralTranslate[14] = (float)((int)vertexMatrix[14]);
#endif
}

#endif
#if defined(APT_USE_SOUND_OBJECT)
/*F************************************************************************************************/
/*!
    \Function    loadSound

    \Description
        Insert function's purpose here

    Insert your long description here.

    \Input      *pUserData        -
    \Input      nID               -
    \Input      szName            -

    \Output     AptAssetSound     -

    \Notes
        Added an extra parameter to function prototype.

    \Version    1.0        06/29/04 (ASK) First Version

*/
/************************************************************************************************F*/
static AptAssetSound loadSound(void *pUserData, int nID, const char * szName)
{
    // we are ignoring szName parameter, but it can be used if required.
    AptAuxLayerInfo *pLevel = (AptAuxLayerInfo *)pUserData;
    char szBuf[MAX_PATH];
    sprintf(szBuf, "%d.wav", nID);
    AptAssetSound sound = (AptAssetSound)BIG_locate(pLevel->pBigData, szBuf);
    return sound;
}

static void startSoundStream(void *pUserData, int nID)
{
    AptAuxLayerInfo *pLevel = (AptAuxLayerInfo *)pUserData;
    char szBuf[MAX_PATH];
    sprintf(szBuf, "stream_%04d.wav", nID);
    AptAssetSound sound = (AptAssetSound)BIG_locate(pLevel->pBigData, szBuf);
    sndPlaySound((char *)sound, SND_MEMORY | SND_ASYNC | SND_NOSTOP);
}

static void startSound(AptAssetSound sound, const char * szName)
//static void startSound(AptAssetSound sound)
{
    if (sound == NULL)
        return;

    sndPlaySound((char *)sound, SND_MEMORY | SND_ASYNC | SND_NOSTOP);
}

static void freeSound(AptAssetSound sound)
{
    //assert(sound);
}
#endif

uint32_t gnBackgroundColor = 0;
static void setBackgroundColour(unsigned int nColour)
{
    gnBackgroundColor = nColour;
}

static void myassertmsg(const char *szMessage, const char *szDescription, const char *szFile, unsigned int nLine)
{
    char szBuf[32768];
    OutputDebugString("\nAssertion failure:\n");
    sprintf(szBuf, "%s(%d) : Condition \"%s\" : Description: \"%s\"\n\n", szFile, nLine, szMessage, szDescription);
    OutputDebugString(szBuf);
    sprintf(szBuf, "Assert failure: \"%s\"\nat line %d in '%s'. Description: '%s'", szMessage, nLine, szFile, szDescription);
    MessageBox(NULL, szBuf, "Assert failure", MB_OK | MB_ICONEXCLAMATION);
}

static void myassert(const char *szMessage, const char *szFile, unsigned int nLine)
{
    char szBuf[32768];
    OutputDebugString("\nAssertion failure:\n");
    sprintf(szBuf, "%s(%d) : Condition \"%s\"\n\n", szFile, nLine, szMessage);
    OutputDebugString(szBuf);
    sprintf(szBuf, "Assert failure: \"%s\"\nat line %d in '%s'", szMessage, nLine, szFile);
    MessageBox(NULL, szBuf, "Assert failure", MB_OK | MB_ICONEXCLAMATION);
}

static void printGLErrors()
{
    int nError;
    for(;;)
    {
        nError = glGetError();
        if (nError == GL_NO_ERROR) break;
        debugPrint("glGetError: 0x%04lX\n", nError);
    }
}

static AptAssetRenderingUnit loadRenderingUnit(void *pUserData, int nID)
{
    AptAuxLayerInfo *pLevel = (AptAuxLayerInfo *)pUserData;
    assert(pLevel->aIDToChar[nID].eType == AptAuxCharacterType_Geometry);
    return (AptAssetRenderingUnit *)pLevel->aIDToChar[nID].pData;
}

static void freeRenderingUnit(AptAssetRenderingUnit renderingUnit)
{
    assert(renderingUnit);
}

/*
 * LEO-05.24.04 : This is code to draw end caps to phat line to support the drawing of phat lines in AptAuxPCOpenGL.  xPos and yPos represent
 * an end point of the line and the radius represents the thickness of the line.
 *
static void drawCircle(float xPos, float yPos, float radius)
{
    float x, y, d;
    x = 0.0f;
    y = radius;
    d = 3.0f - 2.0f * radius;
    while(x <= y)
    {
        glVertex3f( xPos + x, yPos + y, 0.0f);
        glVertex3f( xPos + y, yPos + x, 0.0f);
        glVertex3f( xPos + y, yPos + -x, 0.0f);
        glVertex3f( xPos + x, yPos + -y, 0.0f);
        glVertex3f( xPos + -x, yPos + -y, 0.0f);
        glVertex3f( xPos + -y, yPos + -x, 0.0f);
        glVertex3f( xPos + -y, yPos + x, 0.0f);
        glVertex3f( xPos + -x, yPos + y, 0.0f);
        if(d < 0)
        {
            d = d + 4.0f * x + 6.0f;
        }
        else
        {
            d = d + 4.0f * (x - y) + 10.0f;
            y = y - 1.0f;
        }
        x = x + 1.0f;
    }
}
*/

//static void drawRenderingUnit(AptAssetRenderingUnit renderingUnit, AptMaskRenderOperation eMaskOperation, int nMaskDepth)
static void drawRenderingUnit(AptAssetRenderingUnit renderingUnit, const AptRenderInfo *pRenderInfo)
{
    AptMaskRenderOperation eMaskOperation = pRenderInfo->meMaskOper;
    int nMaskDepth = pRenderInfo->mnMaskDepth;

    setVertexMatrix((AptMath::Mat44T *)&pRenderInfo->mTransform.Pos44);
    pRenderInfo->mTransform.vColorMul4.CopyToFloatArray4(colourScale);
    pRenderInfo->mTransform.vColorAdd4.CopyToFloatArray4(colourTranslate);

    printGLErrors();
    int iUnit;
    assert(renderingUnit);
    AptAuxShape *pShape = (AptAuxShape *) renderingUnit;
    if (eMaskOperation == AptMaskRenderOperation_None)
    {
        // Render everything normally
        for (iUnit = 0; iUnit < pShape->nUnits; iUnit++)
        {
            AptAuxUnit *pUnit = pShape->apUnits[iUnit];
            switch (pUnit->eType)
            {
            case AptAuxUnitType_TriSolid:
                {
                    float a = (pUnit->triSolid.nColour >> 24) / 255.f;
                    float r = ((pUnit->triSolid.nColour >> 16) & 0xff) / 255.f;
                    float g = ((pUnit->triSolid.nColour >> 8) & 0xff) / 255.f;
                    float b = (pUnit->triSolid.nColour & 0xff) / 255.f;

                    // Pre-transform the color since this is a solid fill
                    a *= colourScale[0];
                    r *= colourScale[1];
                    g *= colourScale[2];
                    b *= colourScale[3];
                    a += colourTranslate[0];
                    r += colourTranslate[1];
                    g += colourTranslate[2];
                    b += colourTranslate[3];

                    printGLErrors();
                    glPushMatrix();
                    glMultMatrixf(vertexMatrix);
                    printGLErrors();

                    glActiveTextureARB(GL_TEXTURE0_ARB);
                    glDisable(GL_TEXTURE_2D);
                    glActiveTextureARB(GL_TEXTURE1_ARB);
                    glDisable(GL_TEXTURE_2D);

                    glColor4f(r, g, b, a);
                    glAlphaFunc(GL_ALWAYS, 0.0f);

                    glVertexPointer(2, GL_FLOAT, 0, pUnit->prims.pData);

                    // This is probably faster than disabling the client state entirely (although someone
                    // would have to run tests and this would be system-dependent TODO)
                    glClientActiveTextureARB(GL_TEXTURE0_ARB);
                    glTexCoordPointer(2, GL_FLOAT, 0, pUnit->prims.pData);
                    glClientActiveTextureARB(GL_TEXTURE1_ARB);
                    glTexCoordPointer(2, GL_FLOAT, 0, pUnit->prims.pData);

                    glActiveTextureARB(GL_TEXTURE0_ARB);
                    glDrawArrays(GL_TRIANGLES, 0, pUnit->prims.nPrims * 3);

                    glPopMatrix();

                    printGLErrors();
                }
                break;
            case AptAuxUnitType_TriTiled:
            case AptAuxUnitType_TriClipped:
                {
                    float a = (pUnit->triSolid.nColour >> 24) / 255.f;
                    float r = ((pUnit->triSolid.nColour >> 16) & 0xff) / 255.f;
                    float g = ((pUnit->triSolid.nColour >> 8) & 0xff) / 255.f;
                    float b = (pUnit->triSolid.nColour & 0xff) / 255.f;

                    a *= colourScale[0];
                    r *= colourScale[1];
                    g *= colourScale[2];
                    b *= colourScale[3];

                    printGLErrors();
                    glPushMatrix();
                    glMultMatrixf(vertexMatrix);
                    printGLErrors();

                    glActiveTextureARB(GL_TEXTURE0_ARB);
                    glMatrixMode(GL_TEXTURE);
                    glPushMatrix();
                    printGLErrors();

                    int nTexID = (int)pUnit->triClipped.nTextureID;

                    glBindTexture(GL_TEXTURE_2D, nTexID);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, pUnit->eType == AptAuxUnitType_TriTiled ? GL_REPEAT : GL_CLAMP);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, pUnit->eType == AptAuxUnitType_TriTiled ? GL_REPEAT : GL_CLAMP);

                    glMultMatrixf(pUnit->triTiled.afMatrix);

                    glEnable(GL_TEXTURE_2D);

                    // Bind crap to unit 1
                    // TODO: figure out if we can just leave this at 0 without getting weird behavior (i.e. white square)
                    glActiveTextureARB(GL_TEXTURE1_ARB);
                    glBindTexture(GL_TEXTURE_2D, nTexID);
                    glEnable(GL_TEXTURE_2D);

                    glColor4f(r, g, b, a);
                    float afSwizzle[4] = {colourTranslate[1], colourTranslate[2], colourTranslate[3], colourTranslate[0]};
                    glActiveTextureARB(GL_TEXTURE1_ARB);
                    glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, afSwizzle);
                    glAlphaFunc(GL_GEQUAL, afSwizzle[3]);

                    glVertexPointer(2, GL_FLOAT, 0, pUnit->prims.pData);

                    glClientActiveTextureARB(GL_TEXTURE0_ARB);
                    glTexCoordPointer(2, GL_FLOAT, 0, pUnit->prims.pData);
                    glClientActiveTextureARB(GL_TEXTURE1_ARB);
                    glTexCoordPointer(2, GL_FLOAT, 0, pUnit->prims.pData);

                    glActiveTextureARB(GL_TEXTURE0_ARB);
                    glDrawArrays(GL_TRIANGLES, 0, pUnit->prims.nPrims * 3);

                    glActiveTextureARB(GL_TEXTURE0_ARB);
                    glPopMatrix();
                    glMatrixMode(GL_MODELVIEW);

                    glPopMatrix();

                    printGLErrors();
                }
                break;
            case AptAuxUnitType_LineSolid:
                {
                    float a = (pUnit->lineSolid.nColour >> 24) / 255.f;
                    float r = ((pUnit->lineSolid.nColour >> 16) & 0xff) / 255.f;
                    float g = ((pUnit->lineSolid.nColour >> 8) & 0xff) / 255.f;
                    float b = (pUnit->lineSolid.nColour & 0xff) / 255.f;

                    a *= colourScale[0];
                    r *= colourScale[1];
                    g *= colourScale[2];
                    b *= colourScale[3];
                    a += colourTranslate[0];
                    r += colourTranslate[1];
                    g += colourTranslate[2];
                    b += colourTranslate[3];

                    glActiveTextureARB(GL_TEXTURE0_ARB);
                    glDisable(GL_TEXTURE_2D);
                    glActiveTextureARB(GL_TEXTURE1_ARB);
                    glDisable(GL_TEXTURE_2D);

                    glPushMatrix();
                    glMultMatrixf(vertexMatrix);

                // this doesn't really do what we want. will need to emulate thick lines via quads
                //  glLineWidth((float)nWidth);

                    glColor4f(r, g, b, a);

// disabled draw arrays so that mesa works for lines. using mesa for regression testing because it's software rendering and is the same on all "hardware"
// mesa _appears_ to have a bug in draw arrays for lines with stride 2.
/*                  glVertexPointer(2, GL_FLOAT, 0, pUnit->prims.pData);
                    glTexCoordPointer(2, GL_FLOAT, 0, pUnit->prims.pData);
                    glDrawArrays(GL_LINES, 0, pUnit->prims.nPrims * 2);
*/
                    glBegin(GL_LINES);
                    for (int nPrim = 0; nPrim < pUnit->prims.nPrims; nPrim++)
                    {
                        glVertex3f(pUnit->prims.pData[nPrim * 4], pUnit->prims.pData[nPrim * 4 + 1], 0.f);
                        glVertex3f(pUnit->prims.pData[nPrim * 4 + 2], pUnit->prims.pData[nPrim * 4 + 3], 0.f);
                    }
                    glEnd();
/*
 * LEO-05.24.04 : The following Code is for rendering fat lines.  We have decided to leave this commented out.
                    for (int nPrim = 0; nPrim < pUnit->prims.nPrims; nPrim++)
                    {
                        glBegin(GL_QUADS);
                            drawCircle(pUnit->prims.pData[nPrim * 4], pUnit->prims.pData[nPrim * 4 + 1], 5.5);
                        glEnd();
                        glBegin(GL_QUADS);
                            drawCircle(pUnit->prims.pData[nPrim * 4 + 2], pUnit->prims.pData[nPrim * 4 + 3], 5.5);
                        glEnd();
                    }
*/

//

                    glPopMatrix();
                    printGLErrors();
                }
                break;
            }
        }
    }
    else
    {
        // Mask operation
        printGLErrors();

        // Fail all incoming fragments
        glEnable(GL_STENCIL_TEST);
//      glDisable(GL_STENCIL_TEST);
        glStencilFunc(GL_NEVER, 0, 0xFF);

        // Write to all stencil planes
        glStencilMask(0xFF);
        // No color write (not needed because stencil always fails)
//      glColorMask(FALSE, FALSE, FALSE, FALSE);

        if (eMaskOperation == AptMaskRenderOperation_Add)
        {
            // Don't care what stencil was in before
            // Always inc that way we don't have to change the func
            glStencilOp(GL_INCR, GL_INCR, GL_INCR);
            gnActiveMasks++;
        }
        else
        {
            assert(gnActiveMasks > 0);
            glStencilOp(GL_DECR, GL_DECR, GL_DECR);
            gnActiveMasks--;
        }

        printGLErrors();
        // Actually draw the primitives; simplified drawing code since we don't care about texturing etc.
        for (iUnit = 0; iUnit < pShape->nUnits; iUnit++)
        {
            AptAuxUnit *pUnit = pShape->apUnits[iUnit];
            switch (pUnit->eType)
            {
            // Masking does NOT take into account the alpha test so we clump all tris together
            case AptAuxUnitType_TriSolid:
            case AptAuxUnitType_TriTiled:
            case AptAuxUnitType_TriClipped:
                {
                    printGLErrors();
                    glPushMatrix();
                    glMultMatrixf(vertexMatrix);
                    printGLErrors();

                    glActiveTextureARB(GL_TEXTURE0_ARB);
                    glDisable(GL_TEXTURE_2D);
                    glActiveTextureARB(GL_TEXTURE1_ARB);
                    glDisable(GL_TEXTURE_2D);

                    glAlphaFunc(GL_ALWAYS, 0.0f);

                    glColor4f(1.0f, 0.8f, 0.8f, 1.0f);

                    glVertexPointer(2, GL_FLOAT, 0, pUnit->prims.pData);

                    // This is probably faster than disabling the client state entirely
                    glClientActiveTextureARB(GL_TEXTURE0_ARB);
                    glTexCoordPointer(2, GL_FLOAT, 0, pUnit->prims.pData);
                    glClientActiveTextureARB(GL_TEXTURE1_ARB);
                    glTexCoordPointer(2, GL_FLOAT, 0, pUnit->prims.pData);

                    glActiveTextureARB(GL_TEXTURE0_ARB);
                    glDrawArrays(GL_TRIANGLES, 0, pUnit->prims.nPrims * 3);

/*
                    // Enable this for polygon outlines
                    glActiveTextureARB(GL_TEXTURE0_ARB);
                    glDisable(GL_TEXTURE_2D);
                    glActiveTextureARB(GL_TEXTURE1_ARB);
                    glDisable(GL_TEXTURE_2D);
                    glColor3f(0.0f, 0.0f, 0.0f);
                    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                    glVertexPointer(2, GL_FLOAT, 0, pUnit->prims.pData);
                    glTexCoordPointer(2, GL_FLOAT, 0, pUnit->prims.pData);
                    glDrawArrays(GL_TRIANGLES, 0, pUnit->prims.nPrims * 3);
                    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
//*/

                    glPopMatrix();

                    printGLErrors();
                }
                break;
            case AptAuxUnitType_LineSolid:
                {
                    glActiveTextureARB(GL_TEXTURE0_ARB);
                    glDisable(GL_TEXTURE_2D);
                    glActiveTextureARB(GL_TEXTURE1_ARB);
                    glDisable(GL_TEXTURE_2D);

                    glAlphaFunc(GL_ALWAYS, 0.0f);

                    glPushMatrix();
                    glMultMatrixf(vertexMatrix);

                    glColor4f(1.0f, 0.8f, 0.8f, 1.0f);

                    glVertexPointer(2, GL_FLOAT, 0, pUnit->prims.pData);

                    glClientActiveTextureARB(GL_TEXTURE0_ARB);
                    glTexCoordPointer(2, GL_FLOAT, 0, pUnit->prims.pData);
                    glClientActiveTextureARB(GL_TEXTURE1_ARB);
                    glTexCoordPointer(2, GL_FLOAT, 0, pUnit->prims.pData);

                    glActiveTextureARB(GL_TEXTURE0_ARB);
                    glDrawArrays(GL_LINES, 0, pUnit->prims.nPrims * 2);

                    glPopMatrix();
                    printGLErrors();
                }
                break;
            }
        }

        printGLErrors();
        // Disable writing to stencil, re-enable color
        glStencilMask(0);
        glStencilFunc(GL_LESS, 0, 0xFF);
//      glColorMask(TRUE, TRUE, TRUE, TRUE);

        // Remove the test if no masks left
        if (gnActiveMasks == 0)
            glDisable(GL_STENCIL_TEST);
        printGLErrors();
//      debugPrint("Active masks: %d\n", gLevelInfo[nLevel].nActiveMasks);
    }
}

//static void customControlRender(char *szType, char *szTarget, AptAssetRenderingUnit renderingUnit)
//static void customControlRender(char *szType,
//                                char *szTarget,
//                                AptAssetRenderingUnit renderingUnit,
//                                const char * szCustomProperties,
//                                AptMaskRenderOperation eMaskOperation,
//                                int nMaskDepth)
static void customControlRender(char *szType, char *szTarget, AptAssetRenderingUnit renderingUnit,
                                const char *szCustomProperties, const AptRenderInfo *pRenderInfo)

{

    setVertexMatrix((AptMath::Mat44T *)&pRenderInfo->mTransform.Pos44);
    pRenderInfo->mTransform.vColorMul4.CopyToFloatArray4(colourScale);
    pRenderInfo->mTransform.vColorAdd4.CopyToFloatArray4(colourTranslate);

    AptMaskRenderOperation eMaskOperation = pRenderInfo->meMaskOper;
    int nMaskDepth = pRenderInfo->mnMaskDepth;

//  gAptFuncs.pfnDebugPrint("custom control render: '%s' at target '%s'\n", szType, szTarget);

    static char szMyBuf[512] ;
//  for(int i = 0 ; i < 512; i++) szMyBuf[i] = 0 ;
    if (strcmp(szMyBuf, szCustomProperties) != 0)
    {
        strcpy(szMyBuf, szCustomProperties) ;
        gAptFuncs.pfnDebugPrint("custom control render: '%s' at target '%s' with properties as %s\n", szType, szTarget, szCustomProperties);
    }


    //gAptFuncs.pfnDebugPrint("cur vertex matrix:\n%f %f %f %f\n%f %f %f %f\n%f %f %f %f\n%f %f %f %f\n",
    //  vertexMatrix[0], vertexMatrix[1], vertexMatrix[2], vertexMatrix[3],
    //  vertexMatrix[4], vertexMatrix[5], vertexMatrix[6], vertexMatrix[7],
    //  vertexMatrix[8], vertexMatrix[9], vertexMatrix[10], vertexMatrix[11],
    //  vertexMatrix[12], vertexMatrix[13], vertexMatrix[14], vertexMatrix[15]);
    //gAptFuncs.pfnDebugPrint("cur cxform:\n  scale: %f %f %f %f\n  translate: %f %f %f %f\n",
    //  colourScale[0], colourScale[1], colourScale[2], colourScale[3],
    //  colourTranslate[0], colourTranslate[1], colourTranslate[2], colourTranslate[3]);
}

static bool customControlUpdate(AptAssetRenderingUnit renderingUnit)
{
    return false;
}

void AptAuxPCOpenGL_GetRealTimeClock(AptSysClock *pClock, bool bLocal)
{
    SYSTEMTIME systime;
    if(bLocal)
        GetLocalTime(&systime);
    else
        GetSystemTime(&systime);

    pClock->Second = systime.wSecond;
    pClock->Minute = systime.wMinute;
    pClock->Hour   = systime.wHour;
    pClock->Date   = systime.wDay;
    pClock->Day    = systime.wDayOfWeek;
    pClock->Month  = systime.wMonth - 1;
    pClock->Year   = systime.wYear;
    pClock->Hundredths = systime.wMilliseconds;
}

const int FFB_MAXSTRINGS = 200;
struct AllocatedString
{
    int bUsed;
    unsigned int nTexID;
    unsigned int nColour;
    float fLeft, fTop, fRight, fBottom;
    float fTexWidth, fTexHeight;
    float fTextWidth, fTextHeight;
    float fYAscender;
    AptStringAlignment eAlignment;
    AptStringAlignment eBoxAlignment;
    unsigned int nBackColor;
    unsigned int nBorderColor;
    int bBackground;
    int bBorder;
    int nMaxScroll;
#if defined(EA_DEBUG)
    char            szText[32];
#endif
};

AllocatedString allocStrings[FFB_MAXSTRINGS];
int             nAllocStrings = 0;


AptAssetString allocateStringFF(AptAllocateStringParameters *pASP)
{
    if (gpFonts->nFonts == 0) return 0;

//#if !defined (APT_DECOUPLED_RENDERING)
    if(!(pASP->eFlags & (APT_TEXTFIELD_FUPDATE | APT_TEXTFIELD_DIRTY | APT_TEXTFIELD_TEXTCOLOR)))
    {
        AllocatedString *pStrPtr = (AllocatedString *)(pASP->pCurrString);
        if(!(pASP->eFlags & (APT_TEXTFIELD_ASCHANGE)))
        {
            pStrPtr->bUsed = 1;
            pStrPtr->bBackground = pASP->bBackground;           // None of these properties require a texture recreate
            pStrPtr->bBorder = pASP->bBorder;
            pStrPtr->nBackColor = pASP->nBackColor;
            pStrPtr->nBorderColor = pASP->nBorderColor;
        }
        pASP->nMaxScroll = pStrPtr->nMaxScroll;
        pASP->fTextWidth = pStrPtr->fTextWidth;
        pASP->fTextHeight = pStrPtr->fTextHeight;
        pASP->fStrLen = 1000;
        return pASP->pCurrString;
    }
//#endif
    /// If we get passed here, one of the following flags were set
        //APT_TEXTFIELD_FUPDATE,            // Force update
        //APT_TEXTFIELD_TEXTCHANGE,         // Text in field changed
        //APT_TEXTFIELD_TEXTWIDTH,          // _textWidth property changed
        //APT_TEXTFIELD_WORDWRAP,           // WordWrap flag set
        //APT_TEXTFIELD_HEIGHT,             // _height changed
        //APT_TEXTFIELD_WIDTH               // _width changed
    int i = 0;

    float fWidth = (pASP->x1 - pASP->x0);
    float fHeight = (pASP->y1 - pASP->y0);
    int nWidth = (int) fWidth;
    int nHeight = (int) fHeight;
    int nFontIndex = 0;

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
    }   // end of extra if added check for szFonsName

#if defined(EA_DEBUG)
    if (i == gpFonts->nFonts)
    {
        nFontIndex = 0;
        UCS2ToUTF8(gpFonts->aszFontNamesEnglish[0], -1, szUTF8Name);
        gAptFuncs.pfnDebugPrint("couldn't find font '%s', using '%s' in place\n", pASP->szFontName, szUTF8Name);
    }
#endif

    // Get memory for the texture
    wchar_t UCS2[64*1024];

    int nLen = strlen(pASP->szString);
    unsigned char *pSrc = (unsigned char *)pASP->szString;
    wchar_t* pDst = UCS2;
    UTF8ToUCS2((char *)pASP->szString,nLen+1,UCS2);
    assert(pDst < UCS2 + 64*1024);
    assert(sizeof(wchar_t)==sizeof(uint16_t));
    // Case for autoSize flag set.. we need to check if we need to recalculate the texture size
    if(pASP->eBoxAlignment != AptStringAlignment_None)
    {
        // Now set alignment.
        FF.calcRenderStringSize( nFontIndex, (uint16_t*)UCS2, (int)pASP->fFontHeight, nWidth, nHeight,
            2, 0, 0, pASP->bWordWrap, pASP->nFontStyle, (int)fWidth, (CFontFusion::Alignment)pASP->eAlignment, pASP->nLineOffset );
        pASP->y1 = (float)nHeight;
        fWidth   = (float)nWidth;  // DC -- Must update these vars for later use.
        fHeight  = (float)nHeight; // DC -- Must update these vars for later use.
    }

    // DC -- Can't create texture sizes until after the final text size is known!
    // This was previously done before autosizing!
    int nTexWidth = 2 << (int) (logf(fWidth - 1) / logf(2.0f));
    int nTexHeight = 2 << (int) (logf(fHeight - 1) / logf(2.0f));
    unsigned char *pData = new unsigned char[nTexWidth * nTexHeight];
    memset(pData, 0, sizeof(unsigned char) * nTexWidth * nTexHeight);

    // Find an unused string
    int nString;
    AllocatedString *pString = NULL;
    assert(nAllocStrings < FFB_MAXSTRINGS);
    for (nString = 0; nString < FFB_MAXSTRINGS; nString++)
    {
        if (!allocStrings[nString].bUsed)
        {
            pString = &allocStrings[nString];
            break;
        }
    }

    nAllocStrings++;

#if defined(EA_DEBUG)
    strncpy(pString->szText, pASP->szString, 32);
#endif
    glGenTextures(1, &pString->nTexID);
    glBindTexture(GL_TEXTURE_2D, pString->nTexID);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    //gAptFuncs.pfnDebugPrint("renderString: '%s' (%.1f)\n", szString, fFontHeight);
    unsigned int nTextWidth, nTextHeight;
    float fYAscender = 0.f;

    pASP->nMaxScroll = FF.renderString(nFontIndex, (uint16_t*)UCS2, (int)pASP->fFontHeight, nTexWidth, nTexHeight, pData, &nTextWidth, &nTextHeight, &fYAscender, 2, 0, 0, pASP->bWordWrap, pASP->nFontStyle, (int)nWidth, nHeight, (CFontFusion::Alignment)pASP->eAlignment, pASP->nLineOffset);
    pASP->fTextWidth = (float)nTextWidth;
    pASP->fTextHeight = (float)nTextHeight;
    pASP->fStrLen = 1000;

    // $bug898 PDS 06/21/05 autosize not correct
    pString->fLeft  = pASP->x0;
    pString->fRight = pASP->x1;
    switch (pASP->eAlignment)
    {
    case AptStringAlignment_Center:
        pString->fLeft  = (pASP->x0 + pASP->x1) / 2 - fWidth / 2;
        pString->fRight = (pASP->x0 + pASP->x1) / 2 + fWidth / 2;
        if (!pASP->bWordWrap)
        {
            pASP->x0 = pString->fLeft;
            pASP->x1 = pString->fRight;
        }
        break;
    case AptStringAlignment_Right:
        pString->fLeft = pASP->x1 - fWidth;
        pASP->x0 = pString->fLeft;
        break;
    case AptStringAlignment_Left:
        pString->fRight = pASP->x0 + fWidth;
        pASP->x1 = pString->fRight;
        break;
    }

    // Copy stuff we need for drawing
    pString->bUsed = 1;
    pString->nColour = pASP->nColour;
    //pString->fLeft = pASP->x0; // $bug898 PDS 06/21/05 autosize not correct
    pString->fTop = pASP->y0 + 2;
    //pString->fRight = pASP->x1; // $bug898 PDS 06/21/05 autosize not correct
    pString->fBottom = pASP->y1;
    pString->fTexWidth = (float)nTexWidth;
    pString->fTexHeight = (float)nTexHeight;
    pString->eAlignment = pASP->eAlignment;
    pString->eBoxAlignment = pASP->eBoxAlignment;
    pString->bBackground = pASP->bBackground;
    pString->bBorder = pASP->bBorder;
    pString->nBackColor = pASP->nBackColor;
    pString->nBorderColor = pASP->nBorderColor;
    pString->fTextWidth = (float)nTextWidth;
    pString->fTextHeight = (float)nTextHeight;
    pString->fYAscender = fYAscender;

    // lazy conversion to lum/alpha, where we get only 0-126 grey back
    unsigned char *pData2 = new unsigned char[nTexWidth * nTexHeight * 2];
    memset(pData2, 0, sizeof(unsigned char) * nTexWidth * nTexHeight * 2);
    for (i = 0; i < nTexWidth * nTexHeight; i++)
    {
        pData2[i*2] = 255;
        pData2[i*2+1] = pData[i] << 1;
    }

//  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE8_ALPHA8, nTexWidth, nTexHeight, 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, pData2);

#if 0
    FILE *poo = fopen("poo.raw", "wb");
    fwrite(pData, nTexWidth*nTexHeight, 1, poo);
    fclose(poo);
#endif

    delete[] pData;
    delete[] pData2;

    return &allocStrings[nString];
}

//void drawStringFF(AptAssetString string, AptMaskRenderOperation eMaskOperation, int nMaskDepth)
void drawStringFF(AptAssetString string, const AptRenderInfo *pRenderInfo)
{
    AptMaskRenderOperation eMaskOperation = pRenderInfo->meMaskOper;
    int nMaskDepth = pRenderInfo->mnMaskDepth;

    setVertexMatrix((AptMath::Mat44T *)&pRenderInfo->mTransform.Pos44);
    pRenderInfo->mTransform.vColorMul4.CopyToFloatArray4(colourScale);
    pRenderInfo->mTransform.vColorAdd4.CopyToFloatArray4(colourTranslate);

    AllocatedString *pString = (AllocatedString *)string;

    assert(pString->bUsed);
//  debugPrint("render: %s %f %f\n", pString->szText);

    // There is definitely some optimization that can go in here, especially for state management.  Pretty slow.

    float a = 1.f;
    float r = 1.f;
    float g = 1.f;
    float b = 1.f;

    float p1[2] = {pString->fLeft, pString->fTop};
    float p2[2] = {pString->fRight, pString->fTop};
    float p3[2] = {pString->fRight, pString->fBottom};
    float p4[2] = {pString->fLeft, pString->fBottom};
    float w = pString->fRight - pString->fLeft;
    float h = pString->fBottom - pString->fTop;
    int nBorderBuff = 2;
    if(pString->bBackground || pString->bBorder)
    {
        // Generate text box border coords based on eBoxAlignment
        if(pString->eBoxAlignment == AptStringAlignment_Center)
        {
            p1[0] = pString->fLeft + ((w - pString->fTextWidth) / 2.0f) - nBorderBuff;
            p2[0] = pString->fRight - ((w - pString->fTextWidth) / 2.0f) + nBorderBuff;
            p3[0] = pString->fRight - ((w - pString->fTextWidth) / 2.0f) + nBorderBuff;
            p4[0] = pString->fLeft + ((w - pString->fTextWidth) / 2.0f) - nBorderBuff;
            p3[1] = pString->fTop + pString->fTextHeight;
            p4[1] = pString->fTop + pString->fTextHeight;
        }
        else if(pString->eBoxAlignment == AptStringAlignment_Right)
        {
            p1[0] = pString->fLeft + (w - pString->fTextWidth) - nBorderBuff;
            p2[0] = pString->fRight + nBorderBuff;
            p3[0] = pString->fRight + nBorderBuff;
            p4[0] = pString->fLeft + (w - pString->fTextWidth) - nBorderBuff;
            p3[1] = pString->fTop + pString->fTextHeight;
            p4[1] = pString->fTop + pString->fTextHeight;

        }
        else if(pString->eBoxAlignment == AptStringAlignment_Left)
        {
            p1[0] = pString->fLeft - nBorderBuff;
            p2[0] = pString->fRight - (w - pString->fTextWidth) + nBorderBuff;
            p3[0] = pString->fRight - (w - pString->fTextWidth) + nBorderBuff;
            p4[0] = pString->fLeft - nBorderBuff;
            p3[1] = pString->fTop + pString->fTextHeight;
            p4[1] = pString->fTop + pString->fTextHeight;
        }
    }
    glPushMatrix();
    glMultMatrixf(vertexMatrixIntegralTranslate);
    if(pString->bBackground)
    {
        // Draw background
        r = ((pString->nBackColor >> 16) & 0xff) / 255.f;
        g = ((pString->nBackColor >> 8) & 0xff) / 255.f;
        b = (pString->nBackColor & 0xff) / 255.f;
        a *= colourScale[0];
        r *= colourScale[1];
        g *= colourScale[2];
        b *= colourScale[3];
        a += colourTranslate[0];
        r += colourTranslate[1];
        g += colourTranslate[2];
        b += colourTranslate[3];

        glActiveTextureARB(GL_TEXTURE0_ARB);
        glDisable(GL_TEXTURE_2D);
        glColor4f(r, g, b, a);

        glBegin(GL_QUADS);
            glVertex2f(p1[0], p1[1]);
            glVertex2f(p2[0], p2[1]);
            glVertex2f(p3[0], p3[1]);
            glVertex2f(p4[0], p4[1]);
        glEnd();
    }

    r = ((pString->nColour >> 16) & 0xff) / 255.f;
    g = ((pString->nColour >> 8) & 0xff) / 255.f;
    b = (pString->nColour & 0xff) / 255.f;
    a *= colourScale[0];
    r *= colourScale[1];
    g *= colourScale[2];
    b *= colourScale[3];
    a += colourTranslate[0];
    r += colourTranslate[1];
    g += colourTranslate[2];
    b += colourTranslate[3];

    glColor4f(r, g, b, a);
    glActiveTextureARB(GL_TEXTURE0_ARB);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, pString->nTexID);
    glActiveTextureARB(GL_TEXTURE1_ARB);
    glDisable(GL_TEXTURE_2D);

//  glEnable(GL_BLEND);
//  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBegin(GL_TRIANGLE_STRIP);
    {
        glTexCoord2f(0.0f, 0.0f);
        glVertex2f(pString->fLeft, pString->fTop);
        glTexCoord2f(w / pString->fTexWidth, 0.0f);
        glVertex2f(pString->fRight, pString->fTop);
        glTexCoord2f(0.0f, h / pString->fTexHeight);
        glVertex2f(pString->fLeft, pString->fBottom);
        glTexCoord2f(w / pString->fTexWidth, h / pString->fTexHeight);
        glVertex2f(pString->fRight, pString->fBottom);

    }
    glEnd();

    if(pString->bBorder)
    {
        // Draw border
        r = ((pString->nBorderColor >> 16) & 0xff) / 255.f;
        g = ((pString->nBorderColor >> 8) & 0xff) / 255.f;
        b = (pString->nBorderColor & 0xff) / 255.f;
        a *= colourScale[0];
        r *= colourScale[1];
        g *= colourScale[2];
        b *= colourScale[3];
        a += colourTranslate[0];
        r += colourTranslate[1];
        g += colourTranslate[2];
        b += colourTranslate[3];

        glActiveTextureARB(GL_TEXTURE0_ARB);
        glDisable(GL_TEXTURE_2D);
        glColor4f(r, g, b, a);

        glBegin(GL_LINE_LOOP);
            glVertex2f(p1[0], p1[1]);
            glVertex2f(p2[0], p2[1]);
            glVertex2f(p3[0], p3[1]);
            glVertex2f(p4[0], p4[1]);
        glEnd();
    }
    glPopMatrix();
}

//bool deallocateStringFF(AptAssetString string, unsigned int eFlags)
void deallocateStringFF(AptAssetString string, unsigned int eFlags)
{
//#if !defined (APT_DECOUPLED_RENDERING)
    if(!(eFlags & (APT_TEXTFIELD_FUPDATE | APT_TEXTFIELD_DIRTY | APT_TEXTFIELD_TEXTCOLOR)))
    {
        return;
    }
//#endif

    AllocatedString *pString = (AllocatedString *)string;
    pString->bUsed = 0;
    glDeleteTextures(1, &pString->nTexID);
    nAllocStrings--;
    return;
}


void AptAuxPCOpenGL_PlaySavedInputs(const char *szFilename, unsigned int nPlayTo)
{
    gaSavedInputs = (AptSavedInputRecord *)_loadFileSize(szFilename, &gnSavedInputFileSize);
    gnPlaySavedInputsToFrame = nPlayTo;
}

void AptAuxPCOpenGL_PlaySavedInputsDone(bool bSuccess, const char * pszFailCause)
{
    if(bSuccess)
    {
        APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, "Playing of saved inputs done sucessfully\n");

    }
    else
    {
        APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, "Playing of saved inputs failed!!!!\n");
        APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, "%s\n", pszFailCause);
    }
}


static void hardwareInit(float fWidth, float fHeight)
{
//  char szExtensions[4096];
//  strcpy(szExtensions, (const char *) glGetString(GL_EXTENSIONS));
//  for (char *p = szExtensions; *p; p++)
//      if (*p == ' ') *p = '\n';
#ifndef APT_FALLBACK_RENDERING
    char *szExtensions = (char *) glGetString(GL_EXTENSIONS);
    assert(strstr(szExtensions, "GL_ARB_multitexture"));
    assert(strstr(szExtensions, "GL_ARB_texture_env_combine"));
    int nTextureUnits;
    glGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB, &nTextureUnits);
    assert(nTextureUnits >= 2);

//  glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 0.0f, 0.0f);
    glActiveTextureARB = (ActiveTextureARBFunc) wglGetProcAddress("glActiveTextureARB");
    glClientActiveTextureARB = (ClientActiveTextureARBFunc) wglGetProcAddress("glClientActiveTextureARB");
    glMultiTexCoord2fARB = (MultiTexCoord2fARBFunc) wglGetProcAddress("glMultiTexCoord2fARB");
    glMultiTexCoord2fvARB = (MultiTexCoord2fvARBFunc) wglGetProcAddress("glMultiTexCoord2fvARB");

    // Build the following: (RGBa*S + T)alpha + Fb(1-alpha)
    // I think this will emulate what Flash does (except for negative scale/translate)...
    // <g> where:
    //      RGBA is the texture data
    //      S is the transform scale (passed in through incoming fragment color)
    //      T is the transform translation (passed in through constant)
    //      alpha is the blend factor (resulting from the operations inside the first parentheses)
    //      Fb is what's in the frame buffer right now
    // Stage 1: rgba * Cc
    glActiveTextureARB(GL_TEXTURE0_ARB);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
    glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_MODULATE);
    glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_ALPHA_ARB, GL_MODULATE);

    // Arg0
    glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_TEXTURE);
    glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND0_RGB_ARB, GL_SRC_COLOR);
    glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_ARB, GL_TEXTURE);
    glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA_ARB, GL_SRC_ALPHA);

    // Arg1
    glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE1_RGB_ARB, GL_PRIMARY_COLOR_ARB);
    glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND1_RGB_ARB, GL_SRC_COLOR);
    glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA_ARB, GL_PRIMARY_COLOR_ARB);
    glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA_ARB, GL_SRC_ALPHA);

    // Stage 2: prev + T
    glActiveTextureARB(GL_TEXTURE1_ARB);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
//  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_ADD);
    glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_ALPHA_ARB, GL_ADD);
//  glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_REPLACE);
//  glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_ALPHA_ARB, GL_REPLACE);

    // Arg0
    glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_PREVIOUS_ARB);
    glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND0_RGB_ARB, GL_SRC_COLOR);
    glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_ARB, GL_PREVIOUS_ARB);
    glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA_ARB, GL_SRC_ALPHA);

    // Arg1
    glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE1_RGB_ARB, GL_CONSTANT_ARB);
    glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND1_RGB_ARB, GL_SRC_COLOR);
    glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA_ARB, GL_CONSTANT_ARB);
    glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA_ARB, GL_SRC_ALPHA);

    // Stage 3: interpolate(previous, frame buffer, alpha) is done by the blendfunc
    printGLErrors();
#endif

    glClearColor(0,0,0,1);
//  glClearDepth(-1.0f);    // note that this is clamped to [0, 1]
    glClearDepth(0.0f); // note that this is clamped to [0, 1]
    glColorMask(TRUE, TRUE, TRUE, TRUE);
    glDepthMask(1);
    glShadeModel (GL_SMOOTH);
    glViewport (0, 0, (int)fWidth, (int)fHeight);
    glMatrixMode (GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, fWidth, fHeight, 0, -1, 1);

    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);          // Enable Alpha Blending (disable alpha testing)
    glEnable(GL_BLEND);

#ifdef APT_FALLBACK_RENDERING
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_GEQUAL);
#else
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);
    glStencilFunc(GL_LESS, 0, 0xFF);
    glClearStencil(0);
#endif

    glEnable(GL_ALPHA_TEST);
//  glAlphaFunc(GL_GEQUAL, 0.02f);

    glEnable(GL_LINE_SMOOTH);

    glColor4f(1, 1, 1, 1);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    printGLErrors();
}

DWORD WINAPI LoadThreadProc(LPVOID lpParameter)
{
    LoadParams *pLP = (LoadParams *)lpParameter;
    const char *pSlash = NULL;

    char szFilename[256];
    char szBaseFilename[256];
    assert(strlen(pLP->szName) < 250);

    // not strrchr because we have to look for both / and \ (and there's no strrcspn).
    for (pSlash = &pLP->szName[strlen(pLP->szName) - 1];
            pSlash >= pLP->szName;
            pSlash--)
    {
        if (*pSlash == '/' || *pSlash == '\\')
        {
            break;
        }
    }

    if (pSlash >= pLP->szName)
    {
        strcpy(szBaseFilename, pSlash+1);
    }
    else
    {
        strcpy(szBaseFilename, pLP->szName);
    }

    strcpy(szFilename, pLP->szName);
    strcat(szFilename, ".big");
    //Sleep(rand() % 10000);
    pLP->pLevel->pBigData = _loadFile(szFilename);
    if (pLP->pLevel->pBigData == 0)
    {
        assert(nFinishedLoads < sizeof(aFinishedLoads)/sizeof(aFinishedLoads[0]));
        aFinishedLoads[nFinishedLoads++] = pLP;
        return 0;
    }
    if (gbVerifyUnresolve)
    {
        strcpy(pLP->pLevel->szVerifyUnresolveName, szBaseFilename);
    }

    memset(pLP->pLevel->aIDToChar, 0, sizeof(pLP->pLevel->aIDToChar));

    _loadGeometry(szBaseFilename, pLP->pLevel);

    strcpy(szFilename, szBaseFilename);
    strcat(szFilename, ".apt");
    pLP->pMainData = BIG_locate(pLP->pLevel->pBigData, szFilename);

    strcpy(szFilename, szBaseFilename);
    strcat(szFilename, ".const");
    pLP->pConstTable = BIG_locate(pLP->pLevel->pBigData, szFilename);

    WaitForSingleObject(hLoadThreadsMutex, INFINITE);

    assert(nFinishedLoads < sizeof(aFinishedLoads)/sizeof(aFinishedLoads[0]));
    aFinishedLoads[nFinishedLoads++] = pLP;

    ReleaseMutex(hLoadThreadsMutex);
    return 0;
}

void AptAuxPCOpenGL_FinishAsyncLoads(void)
{
    WaitForSingleObject(hLoadThreadsMutex, INFINITE);
    for (int i = 0; i < nFinishedLoads; i++)
    {
        if (aFinishedLoads[i]->pLevel->pBigData)
        {
            AptCompleteAnimationAsyncLoad(aFinishedLoads[i]->pAsyncLoadContext, aFinishedLoads[i]->pMainData, aFinishedLoads[i]->pConstTable, aFinishedLoads[i]->pLevel);
        }
        delete aFinishedLoads[i];
    }
    nFinishedLoads = 0;
    ReleaseMutex(hLoadThreadsMutex);
}

void AptAuxPCOpenGL_LoadAnimation(const char *szBaseName, AptFilePtr pAsyncLoadContext)
{
    LoadParams *pLP = new LoadParams();
    int i = 0;

    strcpy(pLP->szName, szBaseName);
    for (i = 0; i < sizeof(gLevelInfo)/sizeof(gLevelInfo[0]); i++)
    {
        if (!gLevelInfo[i].bInUse)
        {
            pLP->pLevel = &gLevelInfo[i];
            gLevelInfo[i].bInUse = 1;
            break;
        }
    }
    assert(i != sizeof(gLevelInfo)/sizeof(gLevelInfo[0]));
    pLP->pAsyncLoadContext = pAsyncLoadContext;

    //CreateThread(NULL, 0, LoadThreadProc, pLP, 0, NULL);
    LoadThreadProc(pLP);
    //AptAuxPCOpenGL_FinishAsyncLoads();
}



// Added this extra function in 0.15.00 for FR 380
void AptAuxPCOpenGL_LoadAnimationCompleted(const char *szBaseName, const char * szTargetName)
{
    // Do Nothing.
    // Do make a check of char * are valid or not before operating on them.
    // szTargetName can be a pointer to 0 length string.
}


void AptAuxPCOpenGL_FreeConstantTable(void *pConstantTable)
{
}

void AptAuxPCOpenGL_FreeAnimation(void *pUserHandle)
{
    AptAuxLayerInfo *pLevel = (AptAuxLayerInfo *)pUserHandle;

    if (gbVerifyUnresolve)
    {
        char szFilename[_MAX_PATH];
        int nSize;
        strcpy(szFilename, pLevel->szVerifyUnresolveName);
        strcat(szFilename, ".apt");
        void *pAptData = BIG_locatesize(pLevel->pBigData, szFilename, &nSize);
        sprintf(szFilename, "verifyunresolve_%s.apt", pLevel->szVerifyUnresolveName);
        _saveFile(szFilename, pAptData, nSize);

        strcpy(szFilename, pLevel->szVerifyUnresolveName);
        strcat(szFilename, ".const");
        void *pConstData = BIG_locatesize(pLevel->pBigData, szFilename, &nSize);
        sprintf(szFilename, "verifyunresolve_%s.const", pLevel->szVerifyUnresolveName);
        _saveFile(szFilename, pConstData, nSize);
    }

    pLevel->bInUse = 0;
    free(pLevel->pBigData);
}

void AptAuxPCOpenGL_TimerInit(void)
{
    Timer_Init();
}
void AptAuxPCOpenGL_TimerStartFrame(void)
{
    Timer_StartFrame();
}
unsigned int AptAuxPCOpenGL_TimerGetTicks(void)
{
    // hackahacka
    if (gbUnitTestDoRandomLoading)
    {
        static const char *aRandomLoadingFiles[] = {
            "load1",
            "load2",
            "load3"
        };
        extern unsigned int AptRand();
        if (AptRand() % 100 == 0)
        {
            static int snLastAction = -1;
            int nAction = AptRand() % 5;
            if (nAction == 0)
            {
                int nIndex = AptRand() % (sizeof(aRandomLoadingFiles)/sizeof(aRandomLoadingFiles[0]));
                AptLoadAnimation(aRandomLoadingFiles[nIndex], "_level0");
            }
            else if (nAction == 1)
            {
                AptShutdown();
                AptInitialize();
            }
            else if (nAction == 2)
            {
                AptLoadAnimation("", "_level0");
            }
            else if (nAction == 3)
            {
                for (int i = 0; i < sizeof(aRandomLoadingFiles)/sizeof(aRandomLoadingFiles[0]); i++)
                {
                    AptLoadAnimation(aRandomLoadingFiles[i], "_level0");
                }
            }
            else if (nAction == 4)
            {
                int nIndex = AptRand() % (sizeof(aRandomLoadingFiles)/sizeof(aRandomLoadingFiles[0]));
                AptLoadAnimation(aRandomLoadingFiles[nIndex], "_level0");
                if (AptRand() % 2) AptUpdate(100);
                AptShutdown();
                AptInitialize();
            }
            snLastAction = nAction;
        }
    }

    return Timer_GetTicks();
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
    float ftmp[4];
    glGetFloatv(GL_VIEWPORT, ftmp);
    return ftmp[3];
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
    float ftmp[4];
    glGetFloatv(GL_VIEWPORT, ftmp);
    return ftmp[2];
}


#if defined(APT_RENDER_FLAGS)
void AptAuxPCOpenGL_PushRenderFlags(const char *pszRenderFlags)
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

void AptAuxPCOpenGL_PopRenderFlags(const char *pszRenderFlags)
{
    if (pszRenderFlags && pszRenderFlags[0] != 0)
    {
        // pop the render flags from the stack.
        gAptFuncs.pfnDebugPrint("poping renderflags - %s\n", pszRenderFlags);
    }
}
#endif

// added following blank function for UI effects callback functions.
AptAssetEffect AptAuxPCOpenGL_CreateEffect(const AptEffectData &uiEffectData, AptValue * movieClip)
{
    return NULL;

}

void AptAuxPCOpenGL_DestroyEffect(AptAssetEffect uiEffect)
{

}

void AptAuxPCOpenGL_PushEffect(const AptRenderInfo &renderInfo)
{

}

void AptAuxPCOpenGL_PopEffect(const AptRenderInfo &renderInfo)
{

}

void AptAuxPCOpenGL_Initialize(int nWidth, int nHeight, AptAuxFontList *pFonts)
{
    gAptFuncs.pfnMemAlloc = mymalloc;
    gAptFuncs.pfnMemFree = free;
    gAptFuncs.pfnLoadAnimation = AptAuxPCOpenGL_LoadAnimation;
    gAptFuncs.pfnFreeConstantTable = AptAuxPCOpenGL_FreeConstantTable;
    gAptFuncs.pfnFreeAnimation = AptAuxPCOpenGL_FreeAnimation;
    gAptFuncs.pfnAssertFail = myassert;
    gAptFuncs.pfnAssertFailMsg = myassertmsg;
    gAptFuncs.pfnCommand = command;
    gAptFuncs.pfnSetExternVariable = command;
    gAptFuncs.pfnGetExternVariable = getVariableDefault;
    gAptFuncs.pfnLoadVariables = loadVariables;
    gAptFuncs.pfnDebugPrint = debugPrint;
    gAptFuncs.pfnDebugSetScreenGrabPending = debugSetScreenGrabPending;
    gAptFuncs.pfnDebugAddSavedInput = debugAddSavedInput;
    gAptFuncs.pfnLoadTexture = loadTexture;
    gAptFuncs.pfnFreeTexture = freeTexture;
    gAptFuncs.pfnBindTexture = bindTexture;
#if defined(APT_USE_SOUND_OBJECT)
    gAptFuncs.pfnLoadSound = loadSound;
    gAptFuncs.pfnFreeSound = freeSound;
    gAptFuncs.pfnStartSound = startSound;
    gAptFuncs.pfnStartSoundStream = startSoundStream;
#endif
    gAptFuncs.pfnLoadRenderingUnit = loadRenderingUnit;
    gAptFuncs.pfnDrawRenderingUnit = drawRenderingUnit;
    gAptFuncs.pfnFreeRenderingUnit = freeRenderingUnit;
    gAptFuncs.pfnSetColourTransform = setColourTransform;
    gAptFuncs.pfnSetVertexMatrix = setVertexMatrix;
    gAptFuncs.pfnSetBackgroundColour = setBackgroundColour;
    gAptFuncs.pfnCustomControlRender = customControlRender;
    gAptFuncs.pfnCustomControlUpdate = customControlUpdate;
    gAptFuncs.pfnAllocateString = allocateStringFF;
    gAptFuncs.pfnDrawString = drawStringFF;
    gAptFuncs.pfnDeallocateString = deallocateStringFF;
    gAptFuncs.pfnPointHitTest = NULL;
    gAptFuncs.pfnGetRealTimeClock = AptAuxPCOpenGL_GetRealTimeClock;
    //gAptFuncs.pfnCustomControlUpdate = NULL;

    gAptFuncs.pfnSendVariables = sendVariables;
    gAptFuncs.pfnLoadVariablesNULL = loadVariablesNULL;
    gAptFuncs.pfnGetBytesTotal = getBytesTotal ;
    gAptFuncs.pfnGetBytesLoaded = getBytesLoaded ;
    gAptFuncs.pfnLoadAnimationCompleted = AptAuxPCOpenGL_LoadAnimationCompleted;

    gAptFuncs.pfnGetStageHeight = getStageHeight;
    gAptFuncs.pfnGetStageWidth = getStageWidth;
    gAptFuncs.pfnPlaySavedInputsDone = AptAuxPCOpenGL_PlaySavedInputsDone;

#if defined(APT_RENDER_FLAGS)
    gAptFuncs.pfnPushRenderFlags = AptAuxPCOpenGL_PushRenderFlags ;
    gAptFuncs.pfnPopRenderFlags = AptAuxPCOpenGL_PopRenderFlags ;
#endif

    gAptFuncs.pfnCreateEffect = AptAuxPCOpenGL_CreateEffect;
    gAptFuncs.pfnDestroyEffect = AptAuxPCOpenGL_DestroyEffect;
    gAptFuncs.pfnPushEffect = AptAuxPCOpenGL_PushEffect;
    gAptFuncs.pfnPopEffect = AptAuxPCOpenGL_PopEffect;

    savedInputsFile = NULL;

    hLoadThreadsMutex = CreateMutex(NULL, 0, NULL);

    gpFonts = pFonts;
    FF.init(gpFonts->nFonts, (void **)gpFonts->pData,(int *)gpFonts->nDataSize);

    for (int i = 0; i < gpFonts->nFonts; i++)
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

    colourTranslate[0] = 0;
    colourTranslate[1] = 0;
    colourTranslate[2] = 0;
    colourTranslate[3] = 0;
    colourScale[0] = 1;
    colourScale[1] = 1;
    colourScale[2] = 1;
    colourScale[3] = 1;
    memset(vertexMatrix, 0, sizeof(vertexMatrix));
    vertexMatrix[0] = 1.f;
    vertexMatrix[5] = 1.f;
    vertexMatrix[10] = 1.f;
    vertexMatrix[15] = 1.f;

    hardwareInit((float)nWidth, (float)nHeight);

    if (gaSavedInputs)
    {
        AptDebugPlaySavedInputs(gaSavedInputs, gnSavedInputFileSize);
    }
    printGLErrors();
}

void AptAuxPCOpenGL_Shutdown()
{
    FF.purge();
    if (savedInputsFile)
    {
        fclose(savedInputsFile);
    }
    if (gaSavedInputs)
    {
        free(gaSavedInputs);
    }
    CloseHandle(hLoadThreadsMutex);
}

void AptAuxPCOpenGLDebug_VerifyUnresolve(bool bEnabled)
{
    gbVerifyUnresolve = bEnabled;
}

// added as request from def jam team -FR #348 Access to custom control vertexMatrix
float* AptAuxPCOpenGL_GetCustomControlMatrix()
{
    return vertexMatrix ; // return whatever is current vertex matrix
}

