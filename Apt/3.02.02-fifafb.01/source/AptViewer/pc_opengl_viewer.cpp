/*H************************************************************************************************/
/*!
    \File           pc_opengl_viewer.cpp
    \Description    Apt viewer for the PC
    \Notes          None.
    \Copyright      (c) 2005 Electronic Arts Inc.

    \Version        2.0    12/14/06 (JP) Upgraded with AptDebugger
    \Version        2.1    06/17/09 (RB) Replaced Mesa with wgl
    \Version        2.1    09/23/09 (RB) Don't print leaked pointers for DO_COVERAGE
*/
/************************************************************************************************H*/

//#define EXTRA_KEY_MESSAGES 1
#include "Apt.h"
#include "AptAuxPCOpenGL.h"

#if defined(DO_COVERAGE)
#include "TestAptExt.h"
#include "TestAptExt.cpp"
#endif

#include <windows.h>
#include <wininet.h>
//#include <gl/glut.h>      // ASK
#include "glut.h"
#include <assert.h>
#include <stdio.h>
#include <crtdbg.h>
#include <io.h>
#include <direct.h>
#include <AptValueFactory.h>
#include <AptValueHelper.h>
#include <AptValue\AptValue.h>

#include "growable_array.h"
#if defined(DO_COVERAGE)
#pragma warning(disable:4100 4505) // unref'd formal parameter and inline function removed
#endif

#pragma warning(disable:4509) // SEH with dtor

#include "coreallocator/icoreallocator_interface.h" // Added for FontFusion3
#include "ffapt.h" // Added for FontFusion3


double oldFrameTimes[10];
int nFramePos;
LARGE_INTEGER lastFrameTime;
double frequency;
int bFPS = 1;
int bOnScreenTrace = 0;
int bGremlins = 0;
int bSaveInputs = 0;
int bPlayingInputs = 0;
int bDoScreenCapture = 0;
int bDoUnresolveCheck = 0;
#if defined(DO_COVERAGE)
int bCaptureDone = 0;

#include "Apt.h"
#include "AptValue/AptValue.h"
#include "TestAptExt.h"
// These are used for testing purposes in the testsuite.
AptExtObject * gpTestExtObj=NULL;
AptExtObject * gpBigExtObj = NULL;

#endif

int bModeWireframe;
int nCaptureLength;
int nCurCaptureLength;
int nLastCapturedFrame = -1;
FILE *capturefile;
static char gszScreenGrabName[_MAX_PATH];
static char szStringURL[_MAX_PATH];
int gnListBase = 1;

int gnWindowWidth = 640;
int gnWindowHeight = 480;

char _aOnScreenLog[10][128];
#define numberof(_) (sizeof(_)/sizeof((_)[0]))

#if defined(DO_COVERAGE)
HGLRC  hglrc;
GLubyte *osmesaBuf;
#endif

AptAuxFontList gFonts;

// ICoreAllocator stuff - used by FontFusion
#define FONTFUSION_USE_COREALLOCATOR
class AptOpenGLAllocator: public EA::Allocator::ICoreAllocator
{
public:
    virtual void *Alloc(size_t size, const char *name, unsigned int flags,
        unsigned int align, unsigned int alignOffset = 0)
    {
        return malloc(size);
    }

    virtual void Free(void *block, size_t size=0)
    {
        free(block);
    }

    virtual void *Alloc(size_t size, const char *name, unsigned int flags)
    {
        return malloc(size);
    }

};

/*F************************************************************************************************/
/*!
    \Function       _freeFontTable
    \Description    Releases the fonts
    \Input
    \Output         None.
    \Notes          None.
    \Version        1.0
*/
/************************************************************************************************F*/

static void _freeFontTable(void)
{
    for (int i = 0; i < gFonts.nFonts; i++)
    {
        free(gFonts.pData[i]);
        gAptFuncs.pfnMemFreeSize(gFonts.aszFontNamesEnglish[i], 256);
        gAptFuncs.pfnMemFreeSize(gFonts.aszFontNamesJapanese[i], 256);
    }
}

/*F************************************************************************************************/
/*!
    \Function       resizeWindowToFitAnim
    \Description    Resizes the window to fit the animation
    \Input
    \Output         None.
    \Notes          None.
    \Version        1.0
*/
/************************************************************************************************F*/

static void resizeWindowToFitAnim(void)
{
#if !defined(DO_COVERAGE)
    int nAnimWidth, nAnimHeight;
    AptGetAnimationSize(&nAnimWidth, &nAnimHeight);
    if (nAnimWidth <= 0) return;
    while (nAnimWidth % 4) nAnimWidth++; // for glReadPixels to work easily
    glutReshapeWindow(nAnimWidth, nAnimHeight);
#endif
}

/*F************************************************************************************************/
/*!
    \Function       noMessageBoxAssert
    \Description    Assertion that doesn't use a message box
    \Input
    \Output         None.
    \Notes          None.
    \Version        1.0
*/
/************************************************************************************************F*/

static void noMessageBoxAssert(const char *szMessage, const char *szFile, unsigned int nLine)
{
    char szBuf[32768];
    strcpy(szBuf, "\nAssertion failure:\n");
    gAptFuncs.pfnDebugPrint(szBuf);
    sprintf(szBuf, "%s(%d) : Condition \"%s\"\n\n", szFile, nLine, szMessage);
    gAptFuncs.pfnDebugPrint(szBuf);

    // now, intentially deref null to get a shutdown via exception handler
    int *pNull = 0;
    *pNull = 0;
}

/*F************************************************************************************************/
/*!
    \Function       captureSetScreenGrabPending
    \Description    Stores the screen name
    \Input
    \Output         None.
    \Notes          None.
    \Version        1.0
*/
/************************************************************************************************F*/

static void captureSetScreenGrabPending(const char *szName)
{
    strcpy(gszScreenGrabName, szName);
}
#if defined(APT_USE_SOUND_OBJECT)

/*F************************************************************************************************/
/*!
    \Function       captureLoadSound
    \Description    Loads a sound
    \Input
    \Output         None.
    \Notes          None.
    \Version        1.0
*/
/************************************************************************************************F*/

static AptAssetSound captureLoadSound(AptAnimationUserData, int nID, const char * szName)
{
    char szBuf[256];
    sprintf(szBuf, "%d.wav", nID);
    return strdup(szBuf);
}

/*F************************************************************************************************/
/*!
    \Function       captureStartSound
    \Description    Starts a sound
    \Input
    \Output         None.
    \Notes          None.
    \Version        1.0
*/
/************************************************************************************************F*/

static void captureStartSound(AptAssetSound sound, const char * szName)
//static void captureStartSound(AptAssetSound sound)
{
    gAptFuncs.pfnDebugPrint("[ starting sound ] -- '%s'\n", (char *)sound);
}

/*F************************************************************************************************/
/*!
    \Function       captureFreeSound
    \Description    Ends a sound
    \Input
    \Output         None.
    \Notes          None.
    \Version        1.0
*/
/************************************************************************************************F*/

static void captureFreeSound(AptAssetSound sound)
{
    gAptFuncs.pfnMemFree(sound);
}
#endif

/*F************************************************************************************************/
/*!
    \Function       onScreenDebugPrint
    \Description    Shows the debug information in the screen
    \Input
    \Output         None.
    \Notes          None.
    \Version        1.0
*/
/************************************************************************************************F*/

static void onScreenDebugPrint(const char *szFormat, ...)
{
    va_list args;
    char szBuf[8192];

    va_start(args, szFormat);
    _vsnprintf(szBuf, 8192, szFormat, args);
    va_end(args);
    OutputDebugString(szBuf);
    printf("%s", szBuf);

    for (int i = 1; i < numberof(_aOnScreenLog); i++)
    {
        memcpy(_aOnScreenLog[i - 1], _aOnScreenLog[i], numberof(_aOnScreenLog[0]));
    }
    strncpy(_aOnScreenLog[numberof(_aOnScreenLog) - 1], szBuf, numberof(_aOnScreenLog[0]));
    char *p = &_aOnScreenLog[numberof(_aOnScreenLog) - 1][strlen(_aOnScreenLog[numberof(_aOnScreenLog) - 1]) - 1];
    if (*p == '\n') *p = 0;
}

/*F************************************************************************************************/
/*!
    \Function       text3f
    \Description    Shows a line of text in the screen
    \Input
    \Output         None.
    \Notes          None.
    \Version        1.0
*/
/************************************************************************************************F*/

static void text3f(float x, float y, float z, char *string, void *default_font = GLUT_BITMAP_9_BY_15)
{
#if !defined(DO_COVERAGE)
    int i;
    glPushAttrib(GL_ENABLE_BIT);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_TEXTURE_2D);
//  glColor3f(((float) rand()) / RAND_MAX, ((float) rand()) / RAND_MAX, ((float) rand()) / RAND_MAX);

    glRasterPos3f(x,y,z);
    for (i=0; string[i]; i++)
        glutBitmapCharacter(default_font,string[i]);

//  glEnable(GL_DEPTH_TEST);
    glPopAttrib();
#endif
}

/*F************************************************************************************************/
/*!
    \Function       reshapeFunc
    \Description    Resizes the window with the animation size
    \Input
    \Output         None.
    \Notes          None.
    \Version        1.0
*/
/************************************************************************************************F*/

static void reshapeFunc(int nWindowWidth, int nWindowHeight)
{
    int nAnimWidth, nAnimHeight;

    gnWindowWidth = nWindowWidth;
    gnWindowHeight = nWindowHeight;
    AptGetAnimationSize(&nAnimWidth, &nAnimHeight);

    if (nAnimWidth <= 0) return;

    glViewport((nWindowWidth - nAnimWidth) / 2, (nWindowHeight - nAnimHeight) / 2, nAnimWidth, nAnimHeight);
    glMatrixMode (GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, nAnimWidth, nAnimHeight, 0, -500, 500);

    glMatrixMode(GL_MODELVIEW);
}

/*F************************************************************************************************/
/*!
    \Function       writeTarga
    \Description    Creates a targa file
    \Input
    \Output         None.
    \Notes          None.
    \Version        1.0
*/
/************************************************************************************************F*/

static void writeTarga(const char *filename, const GLubyte *buffer, int width, int height)
{
   FILE *f = fopen( filename, "wb" );
   if (f) {
      int i, x, y;
      const GLubyte *ptr = buffer;
      fputc (0x00, f);  /* ID Length, 0 => No ID    */
      fputc (0x00, f);  /* Color Map Type, 0 => No color map included   */
      fputc (0x02, f);  /* Image Type, 2 => Uncompressed, True-color Image */
      fputc (0x00, f);  /* Next five bytes are about the color map entries */
      fputc (0x00, f);  /* 2 bytes Index, 2 bytes length, 1 byte size */
      fputc (0x00, f);
      fputc (0x00, f);
      fputc (0x00, f);
      fputc (0x00, f);  /* X-origin of Image    */
      fputc (0x00, f);
      fputc (0x00, f);  /* Y-origin of Image    */
      fputc (0x00, f);
      fputc (width & 0xff, f);      /* Image Width  */
      fputc ((width>>8) & 0xff, f);
      fputc (height & 0xff, f);     /* Image Height */
      fputc ((height>>8) & 0xff, f);
      fputc (0x18, f);      /* Pixel Depth, 0x18 => 24 Bits */
      fputc (0x20, f);      /* Image Descriptor */
      for (y=height-1; y>=0; y--) {
         for (x=0; x<width; x++) {
            i = (y*width + x) * 4;
            fputc(ptr[i+2], f); /* write blue */
            fputc(ptr[i+1], f); /* write green */
            fputc(ptr[i], f);   /* write red */
         }
      }
   }
}

GrowableArray<char*> *pvaStrings;

/*F************************************************************************************************/
/*!
    \Function       StringsURLDlgProc
    \Description    Dialog that selects the localization strings URL
    \Input
    \Output         None.
    \Notes          None.
    \Version        1.0
*/
/************************************************************************************************F*/

/*
BOOL CALLBACK StringsURLDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
    switch(Message)
    {
        case WM_INITDIALOG:

        return TRUE;
        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                case IDOK:
                    EndDialog(hwnd, IDOK);
                break;
                case IDCANCEL:
                    EndDialog(hwnd, IDCANCEL);
                break;
            }
        break;
        default:
            return FALSE;
    }
    return TRUE;
}*/

/*F************************************************************************************************/
/*!
    \Function       _setStringsURL
    \Description    Selects the localization strings URL
    \Input
    \Output         None.
    \Notes          None.
    \Version        1.0
*/
/************************************************************************************************F*/

static void _setStringsURL()
{
/*  int ret = DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_ABOUT), hwnd, AboutDlgProc);
    if(ret == IDOK)
    {
        MessageBox(hwnd, "Dialog exited with IDOK.", "Notice", MB_OK | MB_ICONINFORMATION);
    }
    else if(ret == IDCANCEL)
    {
        MessageBox(hwnd, "Dialog exited with IDCANCEL.", "Notice",
            MB_OK | MB_ICONINFORMATION);
    }
    else if(ret == -1)
    {
        MessageBox(hwnd, "Dialog failed!", "Error",
            MB_OK | MB_ICONINFORMATION);
    }*/
}
#define DIE_READING() \
    do { pvaStrings->set_to_zero(); gAptFuncs.pfnDebugPrint("badly formed strings result\n"); goto doublebreak; } while(0)

/*F************************************************************************************************/
/*!
    \Function       _freeLocalizedStrings
    \Description    Frees the localization strings
    \Input
    \Output         None.
    \Notes          None.
    \Version        1.0
*/
/************************************************************************************************F*/

static void _freeLocalizedStrings()
{
    if (!pvaStrings) return;

    for (unsigned int i = 0; i < pvaStrings->length(); i++)
    {
        if (pvaStrings->operator[](i))
        {
            free(pvaStrings->operator [](i));
            pvaStrings->operator [](i) = 0;
        }
    }
    pvaStrings->set_to_zero();
    delete pvaStrings;
    pvaStrings = 0;
}

/*F************************************************************************************************/
/*!
    \Function       _slurpLocalizedStrings
    \Description    Downloads the localization strings
    \Input
    \Output         None.
    \Notes          None.
    \Version        1.0
*/
/************************************************************************************************F*/

static void _slurpLocalizedStrings()
{
    pvaStrings = new GrowableArray<char *>;

    HINTERNET hInternet = InternetOpen("Apt(1.0;p)", INTERNET_OPEN_TYPE_DIRECT, 0, 0, 0);
    HINTERNET hFile = InternetOpenUrl(hInternet, szStringURL, 0, 0, INTERNET_FLAG_NO_UI, 0);
    char szBuf[1024]; // this buffer has to be big enough to hold the largest "$%d=%s&"
    szBuf[sizeof(szBuf) - 1] = 0;
    int nBufOffset = 0;
    DWORD nNumBytes;

    while (InternetReadFile(hFile, &szBuf[nBufOffset], (sizeof(szBuf) - 1) - nBufOffset, &nNumBytes))
    {
        szBuf[nBufOffset + nNumBytes] = 0;
        if (nNumBytes == 0)
        {
            if (strlen(szBuf) == 0)
            {
                break;
            }
        }
        //gAptFuncs.pfnDebugPrint("Read: '%s' (%d bytes)\n", &szBuf[nBufOffset], nNumBytes);

        char *pCur = szBuf;
        for (;;)
        {
            char *pAmp = strchr(pCur, '&');
            if (pAmp == 0)
            {
                if (nNumBytes > 0)
                {
                    nBufOffset = strlen(pCur);
                    memmove(szBuf, pCur, nBufOffset);
                    break;
                }
            }
            else
            {
                *pAmp = 0;
            }
            char *pEquals = strchr(pCur, '=');
            if (pEquals == 0) DIE_READING();
            *pEquals = 0;
            if (pCur[0] != '$') DIE_READING();
            int nIndex = atoi(&pCur[1]);
            pvaStrings->ensure(nIndex);
            pvaStrings->operator[](nIndex) = strdup(pEquals + 1);

            if (pAmp)
            {
                pCur = pAmp + 1;
            }
            else
            {
                *pCur = 0;
                break;
            }
        }
    }
doublebreak:
    InternetCloseHandle(hFile);
    InternetCloseHandle(hInternet);
}

/*F************************************************************************************************/
/*!
    \Function       _shutdownCapture
    \Description    Shutdown with or without a crash
    \Input
    \Output         None.
    \Notes          None.
    \Version        1.0
*/
/************************************************************************************************F*/

static void _shutdownCapture(int bCrashed)
{
    if (!bCrashed)
    {
        // if we crashed, don't shutdown again
        // it might cause another crash if we're in a bad state, which would mean a messagebox from windows, which stops the tests

        int bPlaying = AptDebugIsPlaying(0);
        if (!bPlaying)
        {
            fprintf(capturefile, "shutting down with movie stopped.\n");
        }
        AptShutdown(true);
        AptAuxPCOpenGL_Shutdown();

        _freeFontTable();
        _freeLocalizedStrings();
        AptAllocatorShutdown();
    }

    if (bCrashed && bGremlins)
    {
        MessageBox(NULL, "An exception was encountered. The viewer will now shutdown. Please save \"capture.txt\" and \"saved.inputs\".", "Exception encountered!", MB_ICONEXCLAMATION | MB_OK);
    }
    PostQuitMessage(0);
#if defined(DO_COVERAGE)
    bCaptureDone = 1;
    wglDeleteContext(hglrc);
    free(osmesaBuf);
#endif
#if defined(EA_DEBUG)
    _CrtDumpMemoryLeaks();
#endif

    fclose(capturefile);
}

/*F************************************************************************************************/
/*!
    \Function       displayFunc
    \Description    Shows lines of text with information
    \Input
    \Output         None.
    \Notes          None.
    \Version        1.0
*/
/************************************************************************************************F*/

static void displayFunc(void)
{
    if (nCaptureLength == 0)
    {
        resizeWindowToFitAnim();

        {
            extern uint32_t gnBackgroundColor;
            float a = (gnBackgroundColor >> 24) / 255.f;
            float r = ((gnBackgroundColor >> 16) & 0xff) / 255.f;
            float g = ((gnBackgroundColor >> 8) & 0xff) / 255.f;
            float b = (gnBackgroundColor & 0xff) / 255.f;
            glClearColor(r, g, b, a);
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        AptRender(16);
    }
    else
    {
        if (nCaptureLength - nCurCaptureLength > 0)
        {
            if (gszScreenGrabName[0])
            {
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

                AptRender(16) ;

                if(bDoScreenCapture)
                {

                    #if defined(DO_COVERAGE)
                        char szBuf[1024];
                        sprintf(szBuf, "frame%s.tga", gszScreenGrabName);
                        writeTarga(szBuf, osmesaBuf, gnWindowWidth, gnWindowHeight);
                    #else
                        int nSize = gnWindowWidth * gnWindowHeight * 4;
                        unsigned char *pData = (unsigned char *)malloc(nSize);
                        glReadPixels(0, 0, gnWindowWidth, gnWindowHeight, GL_RGBA, GL_UNSIGNED_BYTE, pData);
                        char szBuf[1024];
                        sprintf(szBuf, "frame%s.tga", gszScreenGrabName);
                        writeTarga(szBuf, pData, gnWindowWidth, gnWindowHeight);
                        free(pData);
                    #endif

                }

                gszScreenGrabName[0] = 0;
                nCurCaptureLength++;

            }
            else
            {
                return;
            }
            if (nCaptureLength == nCurCaptureLength)
            {
                _shutdownCapture(0);
                return;
            }
        }
    }

    LARGE_INTEGER cur;
    QueryPerformanceCounter(&cur);
    double val;
    val = (cur.QuadPart - lastFrameTime.QuadPart);
    val /= frequency;
    lastFrameTime = cur;
    oldFrameTimes[nFramePos++] = val;
    nFramePos %= sizeof(oldFrameTimes)/sizeof(oldFrameTimes[0]);

    if (bFPS)
    {
        glViewport(0, 0, gnWindowWidth, gnWindowHeight);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, gnWindowWidth, gnWindowHeight, 0, -1, 1);
        glMatrixMode(GL_MODELVIEW);

        double avg = 0.f;
        for (int i = 0; i < sizeof(oldFrameTimes)/sizeof(oldFrameTimes[0]); i++)
        {
            avg += oldFrameTimes[i];
        }
        avg /= (double)(sizeof(oldFrameTimes)/sizeof(oldFrameTimes[0]));
        char szBuf[256];
        sprintf(szBuf, "fps: %.1f", 1.f / (float)avg);
        glColor3f(0,0,0);
        text3f(11, 21, 0, szBuf);
        glColor3f(0,1,0);
        text3f(10, 20, 0, szBuf);

        reshapeFunc(gnWindowWidth, gnWindowHeight);
    }

    if (bOnScreenTrace)
    {
        for (int i = 0; i < numberof(_aOnScreenLog); i++)
        {
            glColor3f(0,0,0);
            text3f(11, gnWindowHeight - (numberof(_aOnScreenLog) - i) * 20 + 1, 0, _aOnScreenLog[i], GLUT_BITMAP_8_BY_13);
            glColor3f(0,1,0);
            text3f(10, gnWindowHeight - (numberof(_aOnScreenLog) - i) * 20, 0, _aOnScreenLog[i], GLUT_BITMAP_8_BY_13);
        }
    }

    if (bGremlins)
    {
        glColor3f(0,0,0);
        text3f(11, 41, 0, "Gremlins enabled");
        glColor3f(0,1,0);
        text3f(10, 40, 0, "Gremlins enabled");
    }

#if !defined(DO_COVERAGE)
    glutSwapBuffers();
#endif
}

/*F************************************************************************************************/
/*!
    \Function       closeFunction
    \Description    Releases all
    \Input
    \Output         None.
    \Notes          None.
    \Version        1.0
*/
/************************************************************************************************F*/

static void closeFunc(void)
{
/*  AptFreeAssetAnimation("selector");
    AptFreeAssetAnimation("leagteam");
    AptFreeAssetAnimation("listbox");*/


#if defined(DO_COVERAGE)
    // Make sure that they can be unregistered.
    AptUnRegisterExtension(gpTestExtObj);
    AptUnRegisterExtension(gpBigExtObj);
#endif

// This have to be called BEFORE the shutdown
    AptShutdown();
    AptAuxPCOpenGL_Shutdown();

    _freeFontTable();
    _freeLocalizedStrings();
    AptAllocatorShutdown();
#if defined(EA_DEBUG)
    _CrtDumpMemoryLeaks();
#endif
    PostQuitMessage(0);
}

/*F************************************************************************************************/
/*!
    \Function       doGremlins
    \Description    Random input
    \Input
    \Output         None.
    \Notes          None.
    \Version        1.0
*/
/************************************************************************************************F*/

static void doGremlins()
{
    if (bGremlins)
    {
        extern unsigned int AptRand();
        int nNumInputs = AptRand() % 3;
        for (int i = 0; i < nNumInputs; i++)
        {
            if (AptRand() % 2)
            {
                int nController = AptRand() % AptInputController_NumControllers;
                if (AptRand() % 2 == 0)
                {
                    // make pad inputs really popular, relative to rest of the "input space"
                    // and only do pad on ps2/ngc/xbox
                    //printf("XXX %d, %d\n", nState % 2, nState);
                    int nInput = AptRand() % 10 + AptInputType_PadSelect;

#if 1 // crappy fifa hack
                    if (nInput == AptInputType_PadCross)
                    {
                        nInput = 'X';
                    }
                    if (nInput == AptInputType_PadTriangle)
                    {
                        nInput = 'T';
                    }
                    if (nInput == AptInputType_PadSquare)
                    {
                        nInput = 'S';
                    }
#endif

                    AptAddToInputQueue((AptInputType)nInput,
                        (AptInputState)(AptRand() % AptInputState_NumStates),
                        (AptInputController)nController);
                }
                else
                {
                    AptAddToInputQueue((AptInputType)(AptRand() % AptInputType_NumInputs),
                        (AptInputState)(AptRand() % AptInputState_NumStates),
                        (AptInputController)(nController));
                }
            }
            else
            {
                int nW, nH;
                AptGetAnimationSize(&nW, &nH);
                if (nW && nH)
                {
                    AptSetMousePosition(AptRand()%nW, AptRand()%nH);
                }
                else
                {
                    AptSetMousePosition(AptRand(), AptRand());
                }
            }
        }
    }
}

/*F************************************************************************************************/
/*!
    \Function       PrintMovieclipInfo
    \Description    Shows movie clip related information
    \Input
    \Output         None.
    \Notes          None.
    \Version        1.0
*/
/************************************************************************************************F*/

void PrintMovieclipInfo(AptMovieclipInformation *mcInfo)
{
    gAptFuncs.pfnDebugPrint("Apt-MovieclipInformation------------------------------\n");
    gAptFuncs.pfnDebugPrint("No. of Animations   ---- %d\n", mcInfo->nAnimations) ;
    gAptFuncs.pfnDebugPrint("No. of sprites      ---- %d\n", mcInfo->nAnimations) ;
    gAptFuncs.pfnDebugPrint("No. of shapes       ---- %d\n", mcInfo->nShapes) ;
    gAptFuncs.pfnDebugPrint("No. of Buttons      ---- %d\n", mcInfo->nButtons) ;
    gAptFuncs.pfnDebugPrint("No. of Static Text  ---- %d\n", mcInfo->nStaticText) ;
    gAptFuncs.pfnDebugPrint("No. of Dynamic Text ---- %d\n", mcInfo->nDynamicText) ;
    gAptFuncs.pfnDebugPrint("No. of Morphs       ---- %d\n", mcInfo->nMorph) ;
    gAptFuncs.pfnDebugPrint("------------------------------------------------------\n");

    return ;
}

#undef GetClassName

/*F************************************************************************************************/
/*!
    \Function       PrintMemMetricsRecursive
    \Description    Recursive function for allocation related information
    \Input
    \Output         None.
    \Notes          None.
    \Version        1.0
*/
/************************************************************************************************F*/

#if defined(APT_GATHER_MEMORY_METRICS)
void PrintMemMetricsRecursive(AptMemoryMetrics * pTemp)
{
    while(pTemp)
    {
        gAptFuncs.pfnDebugPrint("<item name=\"%s\" TotalCount=\"%d\" CurrentCount=\"%d\" CurrentBytes=\"%d\" BytesPerAlloc=\"%d\" MaxConcurrentBytes=\"%d\">\n",
            pTemp->GetClassName(),
            pTemp->GetTotalAllocationCount(),
            pTemp->GetCurrentAllocationCount(),
            pTemp->GetBytesCurrentlyAllocated(),
            pTemp->GetCurrentAllocationCount() ? pTemp->GetBytesCurrentlyAllocated() / pTemp->GetCurrentAllocationCount() : 0,
            pTemp->GetMaxBytesAllocatedAtAnyOneTime()
            );

        PrintMemMetricsRecursive(AptMemoryMetrics::GetChildInstance(pTemp));
        gAptFuncs.pfnDebugPrint("</item>\n");
        pTemp = AptMemoryMetrics::GetSiblingInstance(pTemp);
    }
}
#endif

/*F************************************************************************************************/
/*!
    \Function       PrintMemoryAllocationsInfo
    \Description    Prints allocation related information
    \Input          None.
    \Output         None.
    \Notes          None.
    \Version        1.0
*/
/************************************************************************************************F*/

void PrintMemoryAllocationsInfo()
{
#if defined(APT_GATHER_MEMORY_METRICS)
    AptMemoryMetrics * pTemp = AptMemoryMetrics::GetFirstInstance();

    gAptFuncs.pfnDebugPrint("Apt-Memory allocations per Object------------------------------\n");
    PrintMemMetricsRecursive(pTemp);
    gAptFuncs.pfnDebugPrint("---------------------------------------------------------------\n");
#endif
    return ;
}


/*F************************************************************************************************/
/*!
    \Function       PrintDebugInformation

    \Description
        Prints different type of debugging information.

    \Input
        None.

    \Output
        None.

    \Notes
        None.

    \Version        1.0        09/14/04 (ASK) First Version

*/
/************************************************************************************************F*/
void PrintDebugInformation()
{
    // turn on various calls in this function to print various debugging information

    AptMovieclipInformation mcInfo ;
    AptGetMovieclipInfo(&mcInfo, true) ;
    //PrintMovieclipInfo(&mcInfo) ;

    //PrintMemoryAllocationsInfo() ;
}

/*F************************************************************************************************/
/*!
    \Function       UpdateFunction
    \Description    Updates apt
    \Input
    \Output
    \Notes
    \Version        1.0
*/
/************************************************************************************************F*/

DWORD WINAPI  UpdateFunction(void *pNotUsed)
{
    //AptSetSimulationThreadID();
    while(true)
    {
        unsigned int ticks = AptAuxPCOpenGL_TimerGetTicks();
        if(ticks > 250)
            ticks = 16;
        AptUpdate(ticks*5);
        AptAuxPCOpenGL_FinishAsyncLoads();
        Sleep(0);
    }
    return 0;

}

/*F************************************************************************************************/
/*!
    \Function       idleFunc
    \Description    What to do when no pending events
    \Input
    \Output
    \Notes
    \Version        1.0
*/
/************************************************************************************************F*/

static unsigned int gnCurrFrames = 0 ;
static void idleFunc(void)
{
    AptAuxPCOpenGL_TimerStartFrame();

    doGremlins();

    AptAuxPCOpenGL_FinishAsyncLoads();
    if (nCaptureLength > 0)
    {
        AptUpdate(1);
    }
    else if (bPlayingInputs)
    {
        AptUpdate(500);
    }
    else
    {

#if defined (EXTRA_KEY_MESSAGES)
// hack add extra 8 key messages to check if Apt slows down or not.
        for (int extrakeymsg=0; extrakeymsg < 8; extrakeymsg ++)
            AptAddToInputQueue((AptInputType)AptInputType_PadL2, AptInputState_Pressed, (AptInputController)(AptInputController_Gamepad0));
// end of hack
#endif

        // testing partial garbage collection
        //static int nPartialGC = 0 ;
        //nPartialGC++ ;
        //if ((nPartialGC % 100) == 0)
        //{
        //    AptPartialGarbageCollection();
        //}

        // turn this on if you want to print debuggin information
        // like no. of movieclips information, memory allocatios etc.
        PrintDebugInformation() ;

#if 0
        static bool multiThread =false;

        if(!multiThread)
        {
            multiThread = true;
            HANDLE hUpdateThread = CreateThread(NULL, 0, UpdateFunction, 0, CREATE_SUSPENDED , NULL);
            //SetThreadPriority(hUpdateThread, THREAD_PRIORITY_IDLE);
            HANDLE hRenderThread = GetCurrentThread();
            //SetThreadPriority(hRenderThread, THREAD_PRIORITY_ABOVE_NORMAL );
            ResumeThread(hUpdateThread);
        }

        if(0)
        {
            for(int iii = 0; iii < 2048; iii++)
            {
                AptUpdate(16);
                AptAuxPCOpenGL_FinishAsyncLoads();
            }

            for(int iii = 0; iii < 2048; iii++)
            {
                displayFunc();
                Sleep(15);
            }

            if(0)
            {
                multiThread = true;
                HANDLE hUpdateThread = CreateThread(NULL, 0, UpdateFunction, 0, 0, NULL);
                //HANDLE hRenderThread = CreateThread(NULL, 0, RenderFunction, 0, 0, NULL);
                //Sleep(10000);
            }
        }
#else
        {
            unsigned int ticks = AptAuxPCOpenGL_TimerGetTicks();
            if(ticks > 250)
                ticks = 16;
            AptUpdate(16);

            gnCurrFrames ++ ;

            //if ((gnCurrFrames % 300) == 0 && gnCurrFrames < 301)
            //{
            //    AptLoadAnimation("mainBE.swf", "_level0") ;
            //}
            //else if ((gnCurrFrames % 600) == 0 && gnCurrFrames < 601)
            //{
            //    AptLoadAnimation("main.swf", "_level0") ;
            //}
            //else if ((gnCurrFrames % 900) == 0 && gnCurrFrames < 901)
            //{
            //    AptReset() ;
            //    AptLoadAnimation("main.swf", "_level0") ;
            //}

            //if (gnCurrFrames > 1500)
            //{
            //    gnCurrFrames = 0 ;
            //}


            //if ((gnCurrFrames % 2300) == 0 )
            //{
            //    AptReset() ;
            //    AptLoadAnimation("main.swf", "_level0") ;
            //}

        }
#endif

        //gAptFuncs.pfnDebugPrint("ticks: %d\n", AptAuxPCOpenGL_TimerGetTicks());
    }

#if !defined(DO_COVERAGE)
    glutPostRedisplay();
#endif
}

/*F************************************************************************************************/
/*!
    \Function       motionFunc
    \Description    Treats the mouse event
    \Input          mouse position
    \Output
    \Notes
    \Version        1.0
*/
/************************************************************************************************F*/

static void motionFunc(int x, int y)
{
    // We only update if the mouse is within the Apt render able portion of the screen
    if((x >= 0 && x <= gnWindowWidth) && (y >= 0 && y <= gnWindowHeight))
    {
        AptSetMousePosition(x, y);
    }
}

/*F************************************************************************************************/
/*!
    \Function       mouseFunc
    \Description    Treats the mouse button event
    \Input          button pressed and mouse position
    \Output
    \Notes
    \Version        1.0
*/
/************************************************************************************************F*/

static void mouseFunc(int nButton, int nState, int x, int y)
{
    if (nButton == 0)
    {
        if (nState == 0)
        {
            AptAddToInputQueue(AptInputType_MouseButton0, AptInputState_Pressed, AptInputController_Mouse);
        }
        else if (nState == 1)
        {
            AptAddToInputQueue(AptInputType_MouseButton0, AptInputState_Released, AptInputController_Mouse);
        }
    }
}

/*F************************************************************************************************/
/*!
    \Function       keyboardFunc
    \Description    Treats the key down event
    \Input          key code and mouse position
    \Output
    \Notes
    \Version        1.0
*/
/************************************************************************************************F*/

static AptAnalogStickInfo gLastAnalogStickInfo ;

static float fFontScale = 0.1f;
static void keyboardFunc(unsigned char key, int x, int y)
{
    if (nCaptureLength != 0) return;

    if (key == VK_ESCAPE)
    {
        closeFunc();
    }
    else if (key == ' ' || key == 13)
    {
        AptAddToInputQueue(AptInputType_MouseButton0, AptInputState_Pressed, AptInputController_Keyboard);
    }
    else if (key == 23) // ctrl-w
    {
        bModeWireframe = !bModeWireframe;
        if (bModeWireframe)
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
        else
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
    }
    else if (key == 6) // ctrl-f
    {
        bFPS = !bFPS;
    }
    // this is to test analog stick input
    else if (key == 7) // ctrl-g
    {
        // code changed for analog stick so commenting
        //gLastAnalogStickInfo.fXAxisValue = 7.f ;
        //gLastAnalogStickInfo.fYAxisValue = 0.1234f ;
        //gLastAnalogStickInfo.nController = 0 ;
        //gLastAnalogStickInfo.nSide = 0 ;
        //AptAddToInputQueue((AptInputType)AptInputType_LeftAnalogStick, AptInputState_Pressed, AptInputController_Gamepad0);
    }
    // this is to test analog stick input
    else if (key == 8) // ctrl-h
    {
        // code changed for analog stick so commenting
        //gLastAnalogStickInfo.fXAxisValue = 0.2f ;
        //gLastAnalogStickInfo.fYAxisValue = 12.0f ;
        //gLastAnalogStickInfo.nController = 1 ;
        //gLastAnalogStickInfo.nSide = 1 ;
        //AptAddToInputQueue((AptInputType)AptInputType_RightAnalogStick, AptInputState_Pressed, AptInputController_Gamepad1);
    }
    // this is to test alternate input code
    else if (key == 9) // ctrl-i
    {
#if defined (APT_ALTERNATE_INPUT)
        // works with alternateinput1.swf file in examples folder
        AptAltInput myInput;
        //strcpy(myInput.sEvent, "AltInputEvent1") ;
        myInput.sEvent = "AltInputEvent1" ;
        myInput.pValue = AptValueFactory::CreateInteger(100) ;
        AptAddAlternateInput(&myInput) ;
#endif
    }
    // this is to test alternate input code
    else if (key == 10) // ctrl-j
    {
#if defined (APT_ALTERNATE_INPUT)
        // works with alternateinput1.swf file in examples folder
        AptAltInput myInput;
        //strcpy(myInput.sEvent, "AltInputEvent2") ;
        myInput.sEvent = "AltInputEvent2" ;
        myInput.pValue = AptValueFactory::CreateFloat(123.45) ;
        AptAddAlternateInput(&myInput) ;
#endif
    }


    else if (key == 20) // ctrl-t
    {
        bOnScreenTrace = !bOnScreenTrace;
    }
    else if (key == 18 || key == 19) // ctrl-r || ctrl-s
    {
        if (key == 19)
        {
            _setStringsURL();
        }
        _freeLocalizedStrings();
        _slurpLocalizedStrings();
        // refresh current movies
        AptDeallocAllAssetString();
    }

    if (key >= 32 && key <= 126)
    {
        AptAddToInputQueue((AptInputType)key, AptInputState_Pressed, AptInputController_Keyboard);
    }
}

/*F************************************************************************************************/
/*!
    \Function       specialKeyFunction
    \Description    Treats the key down event
    \Input          key code and mouse position
    \Output
    \Notes
    \Version        1.0
*/
/************************************************************************************************F*/

static void specialKeyFunc(int key, int x, int y)
{
    if (key == GLUT_KEY_LEFT) AptAddToInputQueue(AptInputType_Left, AptInputState_Pressed, AptInputController_Keyboard);
    else if (key == GLUT_KEY_RIGHT) AptAddToInputQueue(AptInputType_Right, AptInputState_Pressed, AptInputController_Keyboard);
    else if (key == GLUT_KEY_DOWN) AptAddToInputQueue(AptInputType_Down, AptInputState_Pressed, AptInputController_Keyboard);
    else if (key == GLUT_KEY_UP) AptAddToInputQueue(AptInputType_Up, AptInputState_Pressed, AptInputController_Keyboard);
    else if (key == GLUT_KEY_F1) AptAddToInputQueue(AptInputType_PadSelect, AptInputState_Pressed, AptInputController_Gamepad0);
    else if (key == GLUT_KEY_F2) AptAddToInputQueue(AptInputType_PadStart, AptInputState_Pressed, AptInputController_Gamepad0);
    else if (key == GLUT_KEY_F3) AptAddToInputQueue(AptInputType_PadCross, AptInputState_Pressed, AptInputController_Gamepad0);
    else if (key == GLUT_KEY_F4) AptAddToInputQueue(AptInputType_PadCircle, AptInputState_Pressed, AptInputController_Gamepad0);
    else if (key == GLUT_KEY_F5) AptAddToInputQueue(AptInputType_PadTriangle, AptInputState_Pressed, AptInputController_Gamepad0);
    else if (key == GLUT_KEY_F6) AptAddToInputQueue(AptInputType_PadSquare, AptInputState_Pressed, AptInputController_Gamepad0);
    else if (key == GLUT_KEY_F7) AptAddToInputQueue(AptInputType_PadL1, AptInputState_Pressed, AptInputController_Gamepad0);
    else if (key == GLUT_KEY_F8) AptAddToInputQueue(AptInputType_PadR1, AptInputState_Pressed, AptInputController_Gamepad0);
    else if (key == GLUT_KEY_F9) AptAddToInputQueue(AptInputType_PadL2, AptInputState_Pressed, AptInputController_Gamepad0);
    else if (key == GLUT_KEY_F10) AptAddToInputQueue(AptInputType_PadR2, AptInputState_Pressed, AptInputController_Gamepad0);

}

/*F************************************************************************************************/
/*!
    \Function       AptValueFactoryTestFunc
    \Description    Tests different AptValues
    \Input
    \Output
    \Notes
    \Version        1.0
*/
/************************************************************************************************F*/

#if defined(DO_COVERAGE)
void AptValueFactoryTestFunc()
{
    AptValue * pInt1 = AptValueFactory::CreateInteger(1);
    AptValue * pInt2 = AptValueFactory::CreateInteger(1);
    AptValue * pString = AptValueFactory::CreateString("APT");
    AptValue * pString2 = AptValueFactory::CreateString("55");
    AptValue * pString3 = AptValueFactory::CreateStringFormatted("here%d",15);
    AptValue * pBoolean = AptValueFactory::CreateBoolean(true);
    AptValue * pBoolean2 = AptValueFactory::CreateBoolean(false);
    AptValue * pFloat = AptValueFactory::CreateFloat(5.f);
    AptValue * pFloat2 = AptValueFactory::CreateFloat(222.222f);
    AptValue * pUndefined = AptValueFactory::CreateUndefined();

    AptValue *pArrayorig[3];
    pArrayorig[0] = pInt1;
    pArrayorig[1] = pString;
    pArrayorig[2] = pBoolean;
    AptValue * pArray = AptValueFactory::CreateArray(3, pArrayorig);
    AptNativeString str;
    pString->toString(str);
    APT_ASSERT(pInt1->toInteger() == 1);
    APT_ASSERT(pInt1->toBool()    == true);
    pString->toString(str);
    APT_ASSERT(!str.Compare("APT"));
    pString2->toString(str);
    APT_ASSERT(!str.Compare("55"));
    APT_ASSERT(pString2->toInteger() == 55);
    pString3->toString(str);
    APT_ASSERT(!str.Compare("here15"));

    APT_ASSERT(pBoolean->toBool() == true);
    APT_ASSERT(pBoolean->toInteger() == 1);
    APT_ASSERT(pBoolean2->toBool() == false);
    APT_ASSERT(pBoolean2->toInteger() == 0);
    APT_ASSERT(pFloat->toFloat()   == 5.f);
    APT_ASSERT(pFloat->toInteger() == 5);
    APT_ASSERT(pFloat2->toFloat() == 222.222f);

    char * pfName = "_level0/myfunc";
    char * pRetValue = NULL;
    char * pObject = "_level0";
    AptNativeString param1, param2;
    pInt1->toString(param1);
    pInt2->toString(param2);
    //AptObject * pObject = new AptObject(AptVFT_Object, 1);
    AptCallFunctionOpti(pfName, pRetValue, NULL, 2, pInt1, pInt2);

    return ;
}
#endif


/*F************************************************************************************************/
/*!
    \Function       AptValueHelperTestFunc
    \Description    Tests different AptValues
    \Input
    \Output
    \Notes
    \Version        1.0
*/
/************************************************************************************************F*/

void AptValueHelperTestFunc()
{
    AptValue * pInt1 = AptValueFactory::CreateInteger(1);
    AptValue * pInt2 = AptValueFactory::CreateInteger(1);
    AptValue * pString = AptValueFactory::CreateString("APT");
    AptValue * pString2 = AptValueFactory::CreateString("55");
    AptValue * pString3 = AptValueFactory::CreateStringFormatted("here%d",15);
    AptValue * pBoolean = AptValueFactory::CreateBoolean(true);
    AptValue * pBoolean2 = AptValueFactory::CreateBoolean(false);
    AptValue * pFloat = AptValueFactory::CreateFloat(5.f);
    AptValue * pFloat2 = AptValueFactory::CreateFloat(222.222f);
    AptValue * pUndefined = AptValueFactory::CreateUndefined();

    AptValue *pArrayorig[6];
    pArrayorig[0] = pInt1;
    pArrayorig[1] = pString;
    pArrayorig[2] = pBoolean;
    pArrayorig[3] = pFloat ;
    pArrayorig[4] = pUndefined ;
    pArrayorig[5] = pString2 ;

    AptValue * pArray = AptValueFactory::CreateArray(6, pArrayorig);

    // testing GetArrayMember functions
    int32_t int1 = 0 ;
    if (AptValueHelper::GetArrayMember(pArray, 0, &int1))
    {
        APT_ASSERT(int1 == 1) ;
    }

    char8_t szStr1[10] ;
    if (AptValueHelper::GetArrayMember(pArray, 1, szStr1, 10))
    {
        AptNativeString str1(szStr1) ;
        APT_ASSERT(!str1.Compare("APT")) ;
    }

    float_t fFloat1 = 0.0 ;
    if (AptValueHelper::GetArrayMember(pArray, 3, &fFloat1))
    {
        APT_ASSERT(fFloat1 == 5.0f) ;
    }

    return ;
}

/*F************************************************************************************************/
/*!
    \Function       specialKeyUpFunction
    \Description    Treats the released key event
    \Input          Released key code and mouse position
    \Output
    \Notes
    \Version        1.0
*/
/************************************************************************************************F*/

static void specialKeyUpFunc(int key, int x, int y)
{
    if (key == GLUT_KEY_LEFT) AptAddToInputQueue(AptInputType_Left, AptInputState_Released, AptInputController_Keyboard);
    else if (key == GLUT_KEY_RIGHT) AptAddToInputQueue(AptInputType_Right, AptInputState_Released, AptInputController_Keyboard);
    else if (key == GLUT_KEY_DOWN) AptAddToInputQueue(AptInputType_Down, AptInputState_Released, AptInputController_Keyboard);
    else if (key == GLUT_KEY_UP) AptAddToInputQueue(AptInputType_Up, AptInputState_Released, AptInputController_Keyboard);
    else if (key == GLUT_KEY_F1) AptAddToInputQueue(AptInputType_PadSelect, AptInputState_Released, AptInputController_Gamepad0);
    else if (key == GLUT_KEY_F2) AptAddToInputQueue(AptInputType_PadStart, AptInputState_Released, AptInputController_Gamepad0);
    else if (key == GLUT_KEY_F3) AptAddToInputQueue(AptInputType_PadCross, AptInputState_Released, AptInputController_Gamepad0);
    else if (key == GLUT_KEY_F4) AptAddToInputQueue(AptInputType_PadCircle, AptInputState_Released, AptInputController_Gamepad0);
    else if (key == GLUT_KEY_F5) AptAddToInputQueue(AptInputType_PadTriangle, AptInputState_Released, AptInputController_Gamepad0);
    else if (key == GLUT_KEY_F6) AptAddToInputQueue(AptInputType_PadSquare, AptInputState_Released, AptInputController_Gamepad0);
    else if (key == GLUT_KEY_F7) AptAddToInputQueue(AptInputType_PadL1, AptInputState_Released, AptInputController_Gamepad0);
    else if (key == GLUT_KEY_F8) AptAddToInputQueue(AptInputType_PadR1, AptInputState_Released, AptInputController_Gamepad0);
    else if (key == GLUT_KEY_F9) AptAddToInputQueue(AptInputType_PadL2, AptInputState_Released, AptInputController_Gamepad0);
    else if (key == GLUT_KEY_F10) AptAddToInputQueue(AptInputType_PadR2, AptInputState_Released, AptInputController_Gamepad0);
}

/*F************************************************************************************************/
/*!
    \Function       keyboardUpFunction
    \Description    Treats the released key event
    \Input          Released key code and mouse position
    \Output
    \Notes
    \Version        1.0
*/
/************************************************************************************************F*/


static void keyboardUpFunc(unsigned char key, int x, int y)
{
    if (key == ' ' || key == 13)
    {
        AptAddToInputQueue(AptInputType_MouseButton0, AptInputState_Released, AptInputController_Keyboard);
    }
    if (key >= 32 && key <= 126)
    {
        AptAddToInputQueue((AptInputType)key, AptInputState_Released, AptInputController_Keyboard);
    }
}

/*F************************************************************************************************/
/*!
    \Function       reportingFunction
    \Description    Shows a report string and saves it in the log file
    \Input
    \Output
    \Notes
    \Version        1.0
*/
/************************************************************************************************F*/

int reportingFunction( int reportType, char *userMessage, int *retVal )
{

#if defined(DO_COVERAGE)
    // want to print out consistent text so that tests can be compared,
    // therefore we don't want to print out the leaked pointers.
    static int dumping_leaks = 0;
    static const char begin_tag[] = "Dumping objects ->";
    static const char end_tag[] = "Object dump complete.";
    if ( (!dumping_leaks) && (strlen(userMessage) >= strlen(begin_tag)) && (!strncmp(userMessage,begin_tag,strlen(begin_tag))) )
    {
        dumping_leaks = 1;
    }
    else if ( (dumping_leaks) && (strlen(userMessage) >= strlen(end_tag)) && (!strncmp(userMessage,end_tag,strlen(end_tag))) )
    {
        dumping_leaks = 0;
    }

    if (capturefile && !dumping_leaks)
    {
        fprintf(capturefile, "%s", userMessage);
    }
#endif
    OutputDebugString(userMessage);

    return FALSE;
}

/*F************************************************************************************************/
/*!
    \Function       captureDebugPrint

    \Description    Apt function for a debug string

    \Input          Format string and variables to be shown

    \Output         Localized string

    \Notes

    \Version        1.0
*/
/************************************************************************************************F*/

static void captureDebugPrint(const char *szFormat, ...)
{
    va_list args;
    char szBuf[8192];

    va_start(args, szFormat);
    _vsnprintf(szBuf, 8192, szFormat, args);
    va_end(args);
    fprintf(capturefile, "%d: %s", AptDebugGetCurrentFrame(0), szBuf);
}

/*F************************************************************************************************/
/*!
    \Function       windowStatusFunc(
    \Description    Gives focus to the next window if there is capture
    \Input
    \Output         None
    \Notes          None
    \Version        1.0
*/
/************************************************************************************************F*/

static void windowStatusFunc(int state)
{
    if (nCaptureLength > 0)
    {
        HWND hwnd = FindWindow("", "apt::pc_opengl_viewer");
        HWND stoleFrom = GetNextWindow(hwnd, GW_HWNDNEXT);
        SetWindowPos(hwnd, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
        SetFocus(stoleFrom);
    }
}

/*F************************************************************************************************/
/*!
    \Function       _getNextLine
    \Description    Reads a line in a file
    \Input          FILE *in - file pointer
                    char *szBuf - buffer that receives the line
    \Output         None
    \Notes          None
    \Version        1.0
*/
/************************************************************************************************F*/

static void _getNextLine(FILE *in, char *szBuf)
{
    int c;
    for (;;)
    {
        c = getc(in);
        if (feof(in)) return;
        int bEOL = 0;
        while (c == '\n')
        {
            bEOL = 1;
            c = getc(in);
            if (feof(in))
            {
                *szBuf = 0;
                return;
            }
        }
        if (bEOL)
        {
            ungetc(c, in);
            *szBuf = 0;
            return;
        }
        *szBuf++ = c;
    }
}

/*F************************************************************************************************/
/*!
    \Function       _allocLocalizedString
    \Description    Prepares a localized string
    \Input          String parameters
    \Output         Localized string
    \Notes
    \Version        1.0
*/
/************************************************************************************************F*/


//typedef AptAssetString (*AllocateString)(AptAllocateStringParameters *asp, unsigned int eFlags, AptAssetString pCurrString);//, int *pnNumLines, float *pfW, float *pfH, float *pfSL);
typedef AptAssetString (*AllocateString)(AptAllocateStringParameters *asp); //, unsigned int eFlags, AptAssetString pCurrString);//, int *pnNumLines, float *pfW, float *pfH, float *pfSL);

AllocateString pfnOriginalAllocateString = 0;

//AptAssetString _allocLocalizedString(AptAllocateStringParameters *asp, unsigned int eFlags, AptAssetString pCurrString)//, int *pnNumLines, float *pfW, float *pfH, float *pfSL)
AptAssetString _allocLocalizedString(AptAllocateStringParameters *asp)  //, unsigned int eFlags, AptAssetString pCurrString)//, int *pnNumLines, float *pfW, float *pfH, float *pfSL)
{
    char szBuf[64*1024];

    if (asp->szString[0] == '$')
    {
        char *pEnd;
        unsigned int nString = strtoul(&asp->szString[1], &pEnd, 10);
        if (pEnd == &asp->szString[1] ||                    // didn't look like a string id
            nString >= pvaStrings->length() ||      // number bigger than the size of our list
            pvaStrings->operator[](nString) == 0)   // not in the list
        {
            strcpy(szBuf, asp->szString);
        }
        else
        {
            strcpy(szBuf, pvaStrings->operator[](nString));
        }
    }
    else
    {
        strcpy(szBuf, asp->szString);
    }

    //return pfnOriginalAllocateString(asp, eFlags, pCurrString);//, pnNumLines, pfW, pfH, pfSL);
    return pfnOriginalAllocateString(asp) ;  //, eFlags, pCurrString);//, pnNumLines, pfW, pfH, pfSL);
}

#if defined(__cplusplus)
extern "C" {
#endif

typedef void* (*ALLOC_FUNC)(size_t);
typedef void  (*FREE_FUNC)(void*);
typedef void* (*REALLOC_FUNC)(void*,size_t);
//extern void FFInitStub(ALLOC_FUNC allocptr,FREE_FUNC freeptr,REALLOC_FUNC reallocptr);
#if defined(__cplusplus)
}
#endif


#if defined(DO_COVERAGE)

/*F************************************************************************************************/
/*!
    \Function       WndProc
    \Description    Your basic Windows WndProc used by the OpenGL context window that gets
                    created by CreateOpenGlContext.
    \Input          Whatever a WndProc takes
    \Output         LONG
    \Notes          This is only used for the testsuite, i.e. when DO_COVERAGE is #defined.
*/
/************************************************************************************************F*/
LONG WINAPI WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LONG        lRet = 1;
    PAINTSTRUCT ps;

    switch(uMsg) {
    case WM_CREATE:
        break;

    case WM_DESTROY:
        break;

    case WM_PAINT:
        BeginPaint(hWnd, &ps);
        EndPaint(hWnd, &ps);
        break;

    default:
        lRet = DefWindowProc (hWnd, uMsg, wParam, lParam);
        break;
    }

    return lRet;
}

/*F************************************************************************************************/
/*!
    \Function       CreateOpenGlContext
    \Description    Creates a context via wglCreateContext
    \Input          width and height of window
    \Output         HGLRC - a context that can be passed to wglMakeCurrent
    \Notes          This is only used for the testsuite, i.e. when DO_COVERAGE is #defined.
*/
/************************************************************************************************F*/
HGLRC CreateOpenGlContext( HINSTANCE hInst, HDC* hdc, int width, int height )
{
    static const char title[] = "apt::pc_opengl_viewer";
    WNDCLASS  wc;

    wc.style         = CS_HREDRAW | CS_VREDRAW;  // to redraw if moved
    wc.lpfnWndProc   = (WNDPROC)WndProc;         /* event handler */
    wc.cbClsExtra    = 0;                           /* no extra class data */
    wc.cbWndExtra    = 0;                           /* no extra window data */
    wc.hInstance     = hInst;                   /* instance */
    wc.hIcon         = LoadIcon(NULL, IDI_WINLOGO); /* load a default icon */
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW); /* load a default cursor */
    wc.hbrBackground = NULL;                        /* redraw our own bg */
    wc.lpszMenuName  = NULL;                        /* no menu */
    wc.lpszClassName = title;                       /* use a special class */

    if (!RegisterClass(&wc))
    {
        printf("Failed to register window for OpenGL context\n");
        return NULL;
    }

    HWND hWnd = CreateWindow(title, title,
        WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
        0, 0, width, height,
        NULL, NULL, hInst, NULL);

    if (hWnd == NULL)
    {
        printf( "Failed to create window for OpenGL context\n" );
        return NULL;
    }

    *hdc = GetDC( hWnd );

    PIXELFORMATDESCRIPTOR pfd;
    memset( &pfd, 0, sizeof( pfd ) );
    pfd.nSize = sizeof( pfd );
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;
    pfd.cDepthBits = 16;
    pfd.iLayerType = PFD_MAIN_PLANE;
    int i = ChoosePixelFormat(*hdc,&pfd);
    SetPixelFormat(*hdc, i, &pfd);

    return wglCreateContext( *hdc );
}

#endif // DO_COVERAGE


/*F************************************************************************************************/
/*!
    \Function       mainInternal
    \Description    Entry point
    \Input          WinMain parameters
    \Output         Error code
    \Notes          Reads font files, command tail parameters and load animation
    \Version        1.0
*/
/************************************************************************************************F*/


int mainInternal(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    int i;

    {
        LARGE_INTEGER freq;
    QueryPerformanceFrequency(&freq);
    frequency = (double)freq.QuadPart;
    }

#if defined(EA_DEBUG)
    _CrtSetReportMode( _CRT_WARN, _CRTDBG_MODE_FILE );
    _CrtSetReportFile( _CRT_WARN, _CRTDBG_FILE_STDOUT );
    _CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_FILE );
    _CrtSetReportFile( _CRT_ERROR, _CRTDBG_FILE_STDOUT );
    _CrtSetReportMode( _CRT_ASSERT, _CRTDBG_MODE_FILE );
    _CrtSetReportFile( _CRT_ASSERT, _CRTDBG_FILE_STDOUT );
    _CrtSetReportHook( reportingFunction );
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    int nFile = 0;
    for (i = 1; i < __argc; i++)
    {
        if (strncmp(__argv[i], "-c", 2) == 0)
        {
            nCaptureLength = atoi(&__argv[i][2]);
            __argv[i][0] = 0;
        }
        else if (stricmp(__argv[i], "--verifyunresolve") == 0)
        {
            bDoUnresolveCheck = 1;
            __argv[i][0] = 0;
        }
        else if (stricmp(__argv[i], "--gremlins") == 0)
        {
            bGremlins = 1;
            __argv[i][0] = 0;
        }
        else if (stricmp(__argv[i], "--stringurl") == 0)
        {
            __argv[i][0] = 0;
            i++;
            strcpy(szStringURL, __argv[i]);
            __argv[i][0] = 0;
        }
        else if (stricmp(__argv[i], "--doScreenCap") == 0)
        {
            bDoScreenCapture = 1; // turn on screen captures.
            __argv[i][0] = 0;
        }
        else if (stricmp(__argv[i], "--skipScreenCap") == 0)
        {
            bDoScreenCapture = 0; // turn on screen captures.
            __argv[i][0] = 0;
        }
        else if (stricmp(__argv[i], "--saveinputs") == 0)
        {
            bSaveInputs = 1;
            bDoScreenCapture = 1; // turn on screen captures.
            __argv[i][0] = 0;
        }
        else if (strncmp(__argv[i], "--playinputs", 12) == 0)
        {
            int nPlayUpTo = -1;
            if (strcmp(__argv[i], "--playinputsto") == 0)
            {
                i++;
                nPlayUpTo = atoi(__argv[i]);
                __argv[i][0] = 0;
            }
            else
            {
            __argv[i][0] = 0;
            }

            i++;
            assert(__argv[i]);

            AptAuxPCOpenGL_PlaySavedInputs(__argv[i], nPlayUpTo);

            bPlayingInputs = 1;

            __argv[i][0] = 0;
        }
        else
        {
            nFile = i;
        }
    }


#if defined(DO_COVERAGE)

    HDC hdc;
    hglrc = CreateOpenGlContext( hInstance, &hdc, gnWindowWidth, gnWindowHeight );
    if ( hglrc == NULL )
    {
        printf("Could not create OpenGL context\n");
        return 0;
    }

    /* Allocate the image buffer */
    osmesaBuf = (GLubyte *)malloc( gnWindowWidth * gnWindowHeight * 4 * sizeof(GLubyte) );
    if (!osmesaBuf) {
        printf("Alloc image buffer failed!\n");
        return 0;
    }

    //
    // The following code is commented out because we don't use Mesa any more; we use "wgl" functions
    // instead, such as wglMakeCurrent.
    // We haven't yet implemented any code that can copy the OpenGL window into an array of bytes
    // using GDI and/or "wgl" functions.
    //
    // ----> So for now, SCREEN CAPTURE IS NON-FUNCTIONAL <----
    //

    /* Bind the buffer to the context and make it current */
    //if (!OSMesaMakeCurrent( osmesaCtx, osmesaBuf, GL_UNSIGNED_BYTE, gnWindowWidth, gnWindowHeight )) {
    //  printf("OSMesaMakeCurrent failed!\n");
    //  return 0;
    //}
    if ( !wglMakeCurrent( hdc, hglrc ) )
    {
        printf("wglMakeCurrent failed!\n");
        return 0;
    }

#else

    glutInit(&__argc, __argv);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(gnWindowWidth, gnWindowHeight);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_STENCIL);
    glutCreateWindow("apt::pc_opengl_viewer");

    glutDisplayFunc(displayFunc);
    glutReshapeFunc(reshapeFunc);
    glutIdleFunc(idleFunc);
    glutMouseFunc(mouseFunc);
    glutMotionFunc(motionFunc);
    glutPassiveMotionFunc(motionFunc);
    glutKeyboardFunc(keyboardFunc);
    glutKeyboardUpFunc(keyboardUpFunc);
    glutSpecialFunc(specialKeyFunc);
    glutSpecialUpFunc(specialKeyUpFunc);

#endif
    // why doesn't this work?!
    //  glutCloseFunc(closeFunc);

    // find all .ttf files in the current directory. AptAuxPCOpenGL_Initialize will fill the internal names out for us
    gFonts.nFonts = 0;
    _finddata_t findinfo;
    long nHandle = _findfirst("*.ttf", &findinfo);
    long nRet = nHandle;
    for (;;)
    {
        if (nRet == -1) break;
        assert(gFonts.nFonts < sizeof(gFonts.pData)/sizeof(gFonts.pData[0]));

        FILE *in = fopen(findinfo.name, "rb");
        assert(in);
        fseek(in, 0, SEEK_END);
        gFonts.nDataSize[gFonts.nFonts] = ftell(in);
        gFonts.pData[gFonts.nFonts] = malloc(gFonts.nDataSize[gFonts.nFonts]);
        rewind(in);
        fread(gFonts.pData[gFonts.nFonts], gFonts.nDataSize[gFonts.nFonts], 1, in);
        fclose(in);
        gFonts.nFonts++;

        nRet = _findnext(nHandle, &findinfo);
    }
    _findclose(nHandle);

    nHandle = _findfirst("*.ffs", &findinfo);
    nRet = nHandle;
    for (;;)
    {
        if (nRet == -1) break;
        assert(gFonts.nFonts < sizeof(gFonts.pData)/sizeof(gFonts.pData[0]));

        FILE *in = fopen(findinfo.name, "rb");
        assert(in);
        fseek(in, 0, SEEK_END);
        gFonts.nDataSize[gFonts.nFonts] = ftell(in);
        gFonts.pData[gFonts.nFonts] = malloc(gFonts.nDataSize[gFonts.nFonts]);
        rewind(in);
        fread(gFonts.pData[gFonts.nFonts], gFonts.nDataSize[gFonts.nFonts], 1, in);
        fclose(in);

        gFonts.nFonts++;

        nRet = _findnext(nHandle, &findinfo);
    }
    _findclose(nHandle);

    // set up all the Apt callbacks

    //FFInitStub(malloc, free, realloc);
#define FONTFUSION_NEW 1
#ifdef FONTFUSION_NEW
    AptOpenGLAllocator FontFusionAllocator;
    FontFusionMemObject::SetAllocatorCallbacks(malloc, free, (FONTFUSION_REALLOC_CALLBACK)realloc);
#else
    AptOpenGLAllocator FontFusionAllocator;
    FusionMemObject::sSetAllocator(&FontFusionAllocator);
#endif

    AptAuxPCOpenGL_Initialize(gnWindowWidth, gnWindowHeight, &gFonts);

    if (gFonts.nFonts == 0)
    {
        char szBuf[512];
        char szMessage[512];
        _getcwd(szBuf, sizeof(szBuf)/sizeof(szBuf[0]));
        sprintf(szMessage, "WARNING: no fonts (*.ttf) found in animation directory ('%s'): dynamic text rendering disabled.", szBuf);
        MessageBox(0, szMessage, "apt::pc_opengl_viewer", 0);
    }

    gAptFuncs.pfnDebugPrint = onScreenDebugPrint;
    // added to test the support for analog stick.

    AptAllocatorInitialize();

    if (szStringURL[0] != 0)
    {
        pfnOriginalAllocateString = gAptFuncs.pfnAllocateString;
        gAptFuncs.pfnAllocateString = _allocLocalizedString;
        _slurpLocalizedStrings();
    }

    if (nCaptureLength || bGremlins)
    {
        // override print so we can capture trace stmts if we capturing a test suite
        capturefile = fopen("capture.txt", "wb");
        gAptFuncs.pfnDebugPrint = captureDebugPrint;
        gAptFuncs.pfnAssertFail = noMessageBoxAssert;
        AptSeedRand(0xabcdef); // this is depended upon by testsuite; tests will fail if it's changed
        if (nCaptureLength)
        {
            gAptFuncs.pfnDebugSetScreenGrabPending = captureSetScreenGrabPending;
            bSaveInputs = 1;
        }
#if defined(APT_USE_SOUND_OBJECT)
        gAptFuncs.pfnLoadSound = captureLoadSound;
        gAptFuncs.pfnFreeSound = captureFreeSound;
        gAptFuncs.pfnStartSound = captureStartSound;
#endif
    }

    /*  AptGetDefaultPoolSizes(&gPoolSizes);
    _filePoolSizeOverrides(&gPoolSizes);
    AptOverridePoolSizes(&gPoolSizes);*/

    // this was needed to be added to to get new buffered rendering to work in Apt 2.05.00
    // ea_aptaux_rna also does that.
    AptOptEnable(APT_OPT_SPRTREE);

    // initialize the main library now that the callbacks are set up
    AptInitParmsT initParams;
    initParams.iZombieVectorSize = 64;
    initParams.iCallStackDepth   = 64;
#if defined(EA_DEBUG) || defined(DO_COVERAGE)
    initParams.bPrintZombieDump = true;
#endif
    initParams.iLookupArraySize  = 0x400;
    AptInitialize(&initParams);

#if defined(DO_COVERAGE)
    gpTestExtObj = new TestAptExt();
    gpBigExtObj  = new BigAptExt();
    AptRegisterExtension(gpTestExtObj);
    AptRegisterExtension(gpBigExtObj);
    AptValueFactoryTestFunc();
#endif
    AptValueHelperTestFunc();
    AptDebugEnableSavedInputs(bSaveInputs);
    AptAuxPCOpenGLDebug_VerifyUnresolve(bDoUnresolveCheck ? true : false);

    /*
    AptFilePtr f = AptPreloadAnimation("load1");

    while (!AptIsFileLoaded(f))
    {
        AptUpdate(1000);
        AptAuxPCOpenGL_FinishAsyncLoads();
    }*/

    AptLoadAnimation(__argv[nFile], "_level0");

    AptSetValidFocusButton();
    resizeWindowToFitAnim();

    AptAuxPCOpenGL_TimerInit();

    onScreenDebugPrint("Ctrl-T: toggle onscreen traces\n");
    onScreenDebugPrint("Ctrl-F: toggle fps display\n");
#if defined(APT_USE_SOUND_OBJECT)
    onScreenDebugPrint("Ctrl-S: toggle sound\n");
#endif
    onScreenDebugPrint("Ctrl-W: toggle wireframe\n");
    onScreenDebugPrint("Ctrl-R: reload strings\n");
    onScreenDebugPrint("Ctrl-S: change strings url\n");

#if defined(DO_COVERAGE)
    for (; !bCaptureDone;)
    {
        idleFunc();
        displayFunc();
    }
#else

    glutMainLoop();

#endif
    return 0;
}

/*F************************************************************************************************/
/*!
    \Function       WinMain
    \Description    Entry point
    \Input          WinMain parameters
    \Output         Error code
    \Notes          Isolated to handle exceptions
    \Version        1.0
*/
/************************************************************************************************F*/

int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
#if defined(DO_COVERAGE)
__try
#endif
{
    return mainInternal(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
}
#if defined(DO_COVERAGE)
__except((nCaptureLength > 0) ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
{
    // if we're doing a capture, we don't want to crash with msgbox. log that we crashed and exit instead.
    fprintf(capturefile, "Fatal exception encountered!\n");
    onScreenDebugPrint("Apt Error: Fatal exception encountered!\n"); // Used by Autosmoke that runs testsuite every night
    _shutdownCapture(1);
}
    return -1;
#endif

}
