/******************************************************************************/
/*!
EACore
Copyright 2004 Electronic Arts Inc.

\file
\ingroup    Debug
\brief      This provides a "game" for Apt / AptAuxEAGLREAL to run in.
            This is not intended to be a perfect environment, just enough to be
            able to test the Apt libraries.
*/
/******************************************************************************/


/*** Includes *****************************************************************/
#include "Apt.h"

#if defined(DO_COVERAGE)
    #include "TestAptExt.h"
    #include "TestAptExt.cpp"
#endif

// PS2 specific Includes
#if defined(EA_PLATFORM_PLAYSTATION2)
// tlconfig uses latest compiler of 3.x series so we have upgrade to latest toolchain.
#define PS2TOOLCHAINVERSION "302"
#include    <sif.h>
#include    <sifrpc.h>
#include    <sifdev.h>
#include    <malloc.h>
#include    <libcdvd.h>
#include    <eeregs.h>

// turn on the following #define to include aptxmlimpl related support in viewer.
// please read readme-aptxmlimpl.txt for more information
//#define USE_APTXMLIMPL 1
#if defined(USE_APTXMLIMPL)
#include "dirtyxml.h"   // for xml testing.
#endif

// RenderMethods...
#include "dump_aptrmps2.h"

#endif // #if defined (ps2)

#if defined(EA_PLATFORM_PSP)
    // PSP specific includes
    #define PSPTOOLCHAINVERSION "023"

    #include <sys/types.h>
    #include <cstdlib>
    #include <cassert>
    #include <cstring>
    #include <cmath>
    #include <psptypes.h>
    #include <libgu.h>
    #include <libgum.h>
    //#include <libemu.h>

    // --- image reference file name
    #define APTVIEWERPSP_DAT    "AptViewerPSP.dat"

    // --- image list file name
    #define APTVIEWERPSP_LST    "AptViewerPSP.lst"

    // --- use undocumented function to get cpu milliseconds
    #if defined(PSP_EMU)
        #ifdef CPU_getcycle
            #undef CPU_getcycle
        #endif
        #define CPU_getcycle    sceEmuGetPerformanceTimer
    #endif


    #include "wchar.h"
    #include <moduleexport.h>
    #include <kernel.h> // For the thread initialization code
    SCE_MODULE_INFO( AptViewerEAGLREAL, 0, 1, 1 );
    int sce_newlib_heap_kb_size = 20 * 1024;
    #define DWORD unsigned int
    void    LoadPrxModules();
    void StartMainThread( int argc, char **argv);
    void ViewerPrint(const char *szFormat, ...);
#endif  // if defined (EA_PLATFORM_PSP)

// NGC specific includes
#if defined (EA_PLATFORM_GAMECUBE)
#include <dolphin.h>
#include "screenshot.h"
// RenderMethods...
//#include "dump_aptrmgc.h"
#endif

// PC specific Includes
#if defined (EA_PLATFORM_WINDOWS)
#include "eagl/support/dlopen.h"
// RenderMethods...
#include "dump_aptrmpc.h"
#endif

#if defined(EA_PLATFORM_XBOX)
// RenderMethods...
#include "dump_aptrmxbox.h"
#endif

// Standard Includes
#include <string.h>
#include <math.h>

// Required Package Headers
#include "real.h"
#include "realgraph.h"
#include "eagl/runtime/eagl.h"
#include "eagl/runtime/tar.h"
#include "eagl/runtime/state.h"
#include "eagl/runtime/model.h"
#include "eagl/runtime/fontdriver.h"
#include "eagl/runtime/singledraw.h"
#include "coreallocator/icoreallocator_interface.h" // Added to support FontFusion3

#if defined(APT_USE_SOUND_OBJECT)
#if !defined( PSP_EMU )     // sound not enabled for PSP emu yet
#include "snd/snd.h"
#endif
#endif

// Finally, Apt Includes.
#include "Apt.h"
#include "AptAuxEAGLREAL.h"
#include "ffapt.h"

#if defined(EA_PLATFORM_PSP)
    #include <string.h>
    #include <math.h>
#endif

// TibProfiler Integration.
#if defined(TIBPROFILER)
#include "TibProfilerIntegration.h"
#endif

/*** Using ********************************************************************/
using namespace EAGL;

/*** Macros *******************************************************************/


/*** Interface ****************************************************************/


/*** Constants ****************************************************************/

// PS2 specific Constants
#if defined(EA_PLATFORM_PLAYSTATION2)
//#define DISK_BOOT  1
#define     CONTROLLER1     0
#define     CONTROLLER2     4
#define     SCREEN_WIDTH 640.f
#define     SCREEN_HEIGHT 480.f
#define     VIEWPORT_NEAR_Z 65536.f
#define     VIEWPORT_FAR_Z 0.f
#if defined(DISK_BOOT)  // PS2 DVD Rom Boot.
#define     BASEDIR     "cdrom0:\\"
#define     MEMSIZE     24*1024*1024
#else                   // Dev Kit Network Boot
#define     BASEDIR     "host0:"
#define     MEMSIZE     100*1024*1024
#endif

#elif defined(EA_PLATFORM_PSP)
    // PSP Specific Constants
    #define     CONTROLLER1         0
    #define     CONTROLLER2         4
    #define     BASEDIR             ""
    #define     MEMSIZE             18*1024*1024
    #define     SCREEN_WIDTH        ((float)SCEGU_SCR_WIDTH)
    #define     SCREEN_HEIGHT       ((float)SCEGU_SCR_HEIGHT)
    //#define       VIEWPORT_FAR_Z      65535.f
    //#define       VIEWPORT_NEAR_Z     0.f
    #define     VIEWPORT_FAR_Z      65535.f             // correct one to display geom
    #define     VIEWPORT_NEAR_Z     0.f         // correct one to display geom

// XBOX Specific Constants
#elif defined(EA_PLATFORM_XBOX)
#define     CONTROLLER1     0
#define     CONTROLLER2     1
#define     BASEDIR     "d:\\"
#define     MEMSIZE     50*1024*1024
#define     SCREEN_WIDTH 640.f
#define     SCREEN_HEIGHT 480.f
#define     VIEWPORT_NEAR_Z 0.f
#define     VIEWPORT_FAR_Z 1.f

// PC Specific Constants
#elif defined(EA_PLATFORM_WINDOWS)
#define     CONTROLLER1     0
#define     CONTROLLER2     1
#define     MAXVERTEXBUFFERS    1024
#define     BASEDIR     ""
#define     MEMSIZE     128*1024*1024   // 128
#define     SCREEN_WIDTH 640.f      // 640 testing  854
#define     SCREEN_HEIGHT 480.f
#define     VIEWPORT_NEAR_Z 0.f
#define     VIEWPORT_FAR_Z 1.f

// NGC Specific Constants
#elif defined(EA_PLATFORM_GAMECUBE)
#define     CONTROLLER1     0
#define     CONTROLLER2     4
#define     BASEDIR     ""
#define     MEMSIZE     20*1024*1024
#define     SCREEN_WIDTH 640.f
#define     SCREEN_HEIGHT 448.f
#define     VIEWPORT_NEAR_Z 0.001f
#define     VIEWPORT_FAR_Z 0.999f

#endif // End Console specific Constants

// Global Constants
#define CONTROLLER_UP       0x001
#define CONTROLLER_DOWN     0x002
#define CONTROLLER_LEFT     0x004
#define CONTROLLER_RIGHT    0x008
#define CONTROLLER_CROSS    0x010
#define CONTROLLER_SQUARE   0x020
#define CONTROLLER_TRIANGLE 0x040
#define CONTROLLER_CIRCLE   0x080
#define CONTROLLER_START    0x100
#define CONTROLLER_SELECT   0x200
#define CONTROLLER_L1       0x400
#define CONTROLLER_R1       0x800
#define CONTROLLER_L2       0x1000
#define CONTROLLER_R2       0x2000
#define CONTROLLER_A1       0x4000      // for analog stick left
#define CONTROLLER_A2       0x8000      // for analog stick right

//#define EXTRA_KEY_MESSAGES 1


#if defined(XBOX_FASTCAP)
#include <xbdm.h>
#include "AptValue\AptValue.h"
#include "DogmaAllocator.h"
#include "AptValueGCAllocator.h"
#endif

/*** Variables ****************************************************************/

// Basics...
int                 gnFramesToRun;
int                  gbFPS = 1;
int                  gbRunProgram = 1;
int                  gbSaveScreenShot = 0;
bool                 gbPartialGarbageCollectEveryFrame =  false;
bool                 gbIgnoreRealTime                  =  false;
int                  gnRunFrames                       =  -1;
int                  gnRunFrames1                       =  -1;
int                  gnCurrFramesInc = 0 ;
bool                gbNoShutDown = false ;
bool                gbDoAptResetInBetween = false ;
char                * gszFileBase = NULL ;

#if defined(APT_USE_SOUND_OBJECT)
// disable sound for PSP emulator
#if !defined( PSP_EMU )
char    *gpSNDheap;
#endif
#endif

// Local Objects...
AptAuxFontList       gFonts;

// EAGL / REAL Stuff.
EAGL::RenderContext *pDisp;
EAGL::ViewPort      *pView;
#if defined(EA_PLATFORM_XBOX) || defined(EA_PLATFORM_WINDOWS) || defined(EA_PLATFORM_PLAYSTATION2)
DynamicLoader       *gprmLoader; // Loader used on Consoles.
#endif

// ICoreAllocator stuff - used by FontFusion
#define FONTFUSION_USE_COREALLOCATOR
class AptEAGLRealAllocator: public EA::Allocator::ICoreAllocator
{
public:
    virtual void *Alloc(size_t size, const char *name, unsigned int flags)
    {
        return MEM_alloc(name,size,MB_LOW);
    }

    virtual void *Alloc(size_t size, const char *name, unsigned int flags,
        unsigned int align, unsigned int alignOffset = 0)
    {
        return MEM_alloc(name,size,MB_LOW);
    }

    virtual void Free(void *block, size_t size=0)
    {
        MEM_free(block);
    }
};

extern "C" {
void AptAuxEAGLREAL_DebugPrint(const char *szFormat, ...);
}

/*** Implementation ***********************************************************/

void * operator new (size_t size)
{
    void * block;
    block = (void*)MEM_alloc("object", size, MB_LOW);
    MEM_fill(block,0xcbcbcbcb,size);
    return block;
}

void operator delete(void*p)
{
    if(p!=0)
    {
        MEM_fill(p, 0xcbcbcbcb, MEM_size(p));
        MEM_free(p);
    }
}

// newly added as now arrays inside AptAnimationPoolData are now runtime allocated
// and there has to be [] new/delete operators defined as it is not using APT_NEW_DELETE_GC_OPERATOR
void * operator new[] (size_t size)
{
  void * block;
  block = (void*)MEM_alloc("object", size, MB_LOW);
  MEM_fill(block,0xcbcbcbcb,size);
  return block;
}

void operator delete[](void*p)
{
  if(p!=0)
  {
    MEM_fill(p, 0xcbcbcbcb, MEM_size(p));
    MEM_free(p);
  }
}

void *myNew(size_t s, const char *n)
{
#if defined(EA_DEBUG)
  EAGL_ASSERT(MEM_validate(), "MEM_validation failure.");
#endif
  void *p = MEM_alloc(n, s, MB_LOW);
  MEM_fill(p, 0xcbcbcbcb, s);

  return p;
}

void myDelete(void *p,size_t)
{
#if defined(EA_DEBUG)
  EAGL_ASSERT(MEM_validate(), "MEM_validation failure.");
#endif

  MEM_free(p);
}

void setupSND(void)
{
#if defined(APT_USE_SOUND_OBJECT)
#if defined( PSP_EMU )
    #if defined( DEBUG_PSP )
        AptAuxEAGLREAL_DebugPrint("<warn msg='sound functions disabled for PSP_EMU'/>\n");
    #endif
#else
    // changed to this code to work with newer snd packages.
    gpSNDheap = (char *)MEM_alloc("SND", 400*1024, MB_LOW|MB_ALIGN);
    Snd::System::VectorToReal5();
    Snd::Memory::SetHeap(Snd::DEVICE_MAIN, gpSNDheap, 400*1024);
    Snd::System::SetSndLoadsIopModules(0);
    Snd::System::Init();
/*

    SNDSYS_vectortoreal();
    gpSNDheap = (char *)MEM_alloc("SND", 400*1024, MB_LOW|MB_ALIGN);

    SNDSYSOPTS sso;
    SNDSYS_getopts(&sso);
    sso.set.loadiopmodules = 0;
    SNDSYS_setopts(&sso);

    SNDSYS_init(gpSNDheap, MEM_size(gpSNDheap));
*/
#endif
#else
    AptAuxEAGLREAL_DebugPrint("APT-WARNING-Sound is compiled out, define APT_USE_SOUND_OBJECT if you wish to compile in functionality.\n");
#endif
}

void setupEAGL(void)
{
    // Allocate the device
    EAGL::Device::SetNewOverride(myNew);
    EAGL::Device::SetDeleteOverride(myDelete);
    EAGL::Device *pDev = new EAGL::Device;

#if defined(EA_PLATFORM_PLAYSTATION2)
    pDev->EXTENSION_OBJ.SetDMABufferLength(800*1024);
    pDev->EXTENSION_OBJ.SetTextureBufferLength(80*1024);
#endif

#if defined (EA_PLATFORM_WINDOWS)
    pDev->EXTENSION_OBJ.SetMaxNumVertexBuffers(MAXVERTEXBUFFERS);
    pDev->EXTENSION_OBJ.SetMaxNumIndexBuffers(MAXVERTEXBUFFERS);
#endif

    bool success = pDev->Init();
    (void) success;
    EAGL_ASSERT(success,"Failed device initialization");

    // Generate a RenderContext to render into
    pDisp = pDev->NewRenderContext();

#if defined(EA_PLATFORM_WINDOWS)
    pDisp->EXTENSION_OBJ.SetRenderContextType( EAGL::RC_WINDOW );
#endif

    pDisp->SetSize(SCREEN_WIDTH, SCREEN_HEIGHT);
    pDisp->SetFrontBufferDepth(32);
    pDisp->SetBackBufferDepth(32);

#if !defined(EA_PLATFORM_PSP)
    // WARNING: SetZBufferDepth() not available on PSP - the depth of the Z buffer is fixed.
    pDisp->SetZBufferDepth(16);
#endif

#if defined (EA_PLATFORM_GAMECUBE)
    pDisp->EXTENSION_OBJ.SetFifoSize (1024*1024);
    pDisp->EXTENSION_OBJ.SetVolatileBufferSize(512*1024);

#endif

    // Set the video mode
    pDisp->SetupFrameBuffers();

#if DISK_BOOT
    pDisp->SetSyncToVBL(true);
#else
    pDisp->SetSyncToVBL(false);
#endif

    // Set up the viewport we will draw into
    pView = pDisp->NewViewPort();

    // Set the shape of the viewport.
    // The first 4 parameters are the screen coordinates
    // of the window, and the last two are the range of the
    // Z-buffer to use for this window.  This allows the
    // Z-buffer to be chopped up into regions in overlapping
    // viewports to control precision allocation.
    // Also note that a larger Z is nearer on this platform.
    pView->SetShape(0,0,SCREEN_WIDTH,SCREEN_HEIGHT,VIEWPORT_NEAR_Z,VIEWPORT_FAR_Z);

#if defined(EA_PLATFORM_PLAYSTATION2)
    pView->SetGuardBandScale(4.f);
#endif


//  pView->SetOrthographicScreenSpace(5, 500);
    pView->SetOrthographicScreenSpace(1, 10.f);

    // Setup the viewport clear colour
    pView->SetBackgroundColour(0x00000000);

#if defined(EA_PLATFORM_XBOX)
    pDisp->EXTENSION_OBJ.SetMultiSampleMode(MSM_NONE);
#endif

    // Set up a view
    EAGL::Transform tx;
    tx.BuildIdentity();
#if defined (EA_PLATFORM_PSP)
    tx.BuildTranslate(0.f, 0.f, 0.f);
#else
    tx.BuildTranslate(0.f, 0.f, -5.f);
#endif
    pView->SetViewMatrix(tx.m);

    FONT_installdriver(&EAGLdriver);
    FONT_installdriverextensions(&EAGLdriverext);
    FONT_init();

    FONT_setstatef(FONTstand, FONT_Z, powf(2, 16) - 1);
#if defined (EA_PLATFORM_WINDOWS)
    // funny thing is input want to be initialized only after main window get created, which make some kind of sense
    KEY_init();
    MOUSE_init();
    MOUSE_setbounds(0, 0, -10, (int)SCREEN_WIDTH, (int)SCREEN_HEIGHT, 10);                // F#563    11
    REAL_VERIFY(INPUT_init() == 1);
#endif
    // clear screen
    for (int i=0; i<3; i++)
    {
        pDisp->BeginFrame();
        pView->BeginView();
        pView->ClearViewPort();
        pView->EndView();
        pDisp->EndFrame();
    }

#if defined(EA_PLATFORM_XBOX) || defined(EA_PLATFORM_WINDOWS) || defined(EA_PLATFORM_PLAYSTATION2)
    gprmLoader = new DynamicLoader(rendermethod_o, rendermethod_o_len);
#endif
}

void _saveScreenShot()
{

}

static AptAnalogStickInfo gLastAnalogStickInfo1;
static AptAnalogStickInfo gLastAnalogStickInfo2;
float absf(float val) { return (val < 0.f) ? val * -1.f : val; }

#if defined(EA_PLATFORM_XBOX)
void _getInput(int nController, int &nPadUpEvent, int &nPadDownEvent)
{
    const int XBOX_CONTROLLER_THRESHOLD = 32;
    const XBGAMEPAD *pPad = PAD_getdataptr(nController);
    static int nPrevPadState = 0;
    int nPadState = 0;

    nPadUpEvent = nPadDownEvent = 0;
    if (pPad->wLastButtons & XINPUT_GAMEPAD_DPAD_UP) nPadState |= CONTROLLER_UP;
    if (pPad->wLastButtons & XINPUT_GAMEPAD_DPAD_DOWN) nPadState |= CONTROLLER_DOWN;
    if (pPad->wLastButtons & XINPUT_GAMEPAD_DPAD_LEFT) nPadState |= CONTROLLER_LEFT;
    if (pPad->wLastButtons & XINPUT_GAMEPAD_DPAD_RIGHT) nPadState |= CONTROLLER_RIGHT;
    if (pPad->wLastButtons & XINPUT_GAMEPAD_START) nPadState |= CONTROLLER_START;
    if (pPad->wLastButtons & XINPUT_GAMEPAD_BACK) nPadState |= CONTROLLER_SELECT;
    if (pPad->bLastAnalogButtons[XINPUT_GAMEPAD_A] > XBOX_CONTROLLER_THRESHOLD) nPadState |= CONTROLLER_CROSS;
    if (pPad->bLastAnalogButtons[XINPUT_GAMEPAD_B] > XBOX_CONTROLLER_THRESHOLD) nPadState |= CONTROLLER_CIRCLE;
    if (pPad->bLastAnalogButtons[XINPUT_GAMEPAD_X] > XBOX_CONTROLLER_THRESHOLD) nPadState |= CONTROLLER_SQUARE;
    if (pPad->bLastAnalogButtons[XINPUT_GAMEPAD_Y] > XBOX_CONTROLLER_THRESHOLD) nPadState |= CONTROLLER_TRIANGLE;
    if (pPad->bLastAnalogButtons[XINPUT_GAMEPAD_LEFT_TRIGGER] > XBOX_CONTROLLER_THRESHOLD) nPadState |= CONTROLLER_L1;
    if (pPad->bLastAnalogButtons[XINPUT_GAMEPAD_RIGHT_TRIGGER] > XBOX_CONTROLLER_THRESHOLD) nPadState |= CONTROLLER_R1;
    if (pPad->bLastAnalogButtons[XINPUT_GAMEPAD_WHITE] > XBOX_CONTROLLER_THRESHOLD) nPadState |= CONTROLLER_L2;
    if (pPad->bLastAnalogButtons[XINPUT_GAMEPAD_BLACK] > XBOX_CONTROLLER_THRESHOLD) nPadState |= CONTROLLER_R2;

    // added for analog stick support
    if (absf(pPad->fRX - gLastAnalogStickInfo2.fXAxisValue) > 0.00001f || absf(pPad->fRY - gLastAnalogStickInfo2.fYAxisValue) > 0.00001f)
    {
        gLastAnalogStickInfo2.fXAxisValue = pPad->fRX;
        gLastAnalogStickInfo2.fYAxisValue = pPad->fRY;
        gLastAnalogStickInfo2.nController = 0;
        gLastAnalogStickInfo2.nSide = AptInputType_RightAnalogStick;
        nPadState |= CONTROLLER_A1;
        AptAddToInputAnalogQueue(gLastAnalogStickInfo2);
    }

    if (absf(pPad->fLX - gLastAnalogStickInfo1.fXAxisValue) > 0.00001f || absf(pPad->fLY - gLastAnalogStickInfo1.fYAxisValue) > 0.00001f)
    {
        gLastAnalogStickInfo1.fXAxisValue = pPad->fLX;
        gLastAnalogStickInfo1.fYAxisValue = pPad->fLY;
        gLastAnalogStickInfo1.nController = 0;
        gLastAnalogStickInfo1.nSide = AptInputType_LeftAnalogStick;
        nPadState |= CONTROLLER_A2;
        AptAddToInputAnalogQueue(gLastAnalogStickInfo1);
    }

    nPadUpEvent = (nPadState ^ nPrevPadState) & nPrevPadState;

    nPadDownEvent = (nPadState ^ nPrevPadState) & nPadState;

    nPrevPadState = nPadState;
}

#elif defined(EA_PLATFORM_PLAYSTATION2)
void _getInput(int nController, int &nPadUpEvent, int &nPadDownEvent)
{
    const CONTROLLER *pPad = PAD_getdataptr(nController);
    static int nPrevPadState = 0;
    int nPadState = 0;
    const CONTROLLER_STICKINFO *pStickValues;


    if (pPad->DigitalButtonFlags & PS_UP) nPadState |= CONTROLLER_UP;
    if (pPad->DigitalButtonFlags & PS_DOWN) nPadState |= CONTROLLER_DOWN;
    if (pPad->DigitalButtonFlags & PS_LEFT) nPadState |= CONTROLLER_LEFT;
    if (pPad->DigitalButtonFlags & PS_RIGHT) nPadState |= CONTROLLER_RIGHT;
    if (pPad->DigitalButtonFlags & PS_START) nPadState |= CONTROLLER_START;
    if (pPad->DigitalButtonFlags & PS_SELECT) nPadState |= CONTROLLER_SELECT;
    if (pPad->DigitalButtonFlags & PS_CROSS) nPadState |= CONTROLLER_CROSS;
    if (pPad->DigitalButtonFlags & PS_CIRCLE) nPadState |= CONTROLLER_CIRCLE;
    if (pPad->DigitalButtonFlags & PS_SQUARE) nPadState |= CONTROLLER_SQUARE;
    if (pPad->DigitalButtonFlags & PS_TRIANGLE) nPadState |= CONTROLLER_TRIANGLE;
    if (pPad->DigitalButtonFlags & PS_L1) nPadState |= CONTROLLER_L1;
    if (pPad->DigitalButtonFlags & PS_R1) nPadState |= CONTROLLER_R1;
    if (pPad->DigitalButtonFlags & PS_L2) nPadState |= CONTROLLER_L2;
    if (pPad->DigitalButtonFlags & PS_R2) nPadState |= CONTROLLER_R2;

    if((pStickValues = PAD_getstickinfo(nController)))
    {
        float fx = (float)(pStickValues->rx - 127) / 255.f;
        float fy = -(float)(pStickValues->ry - 127) / 255.f;
        if (absf(fx - gLastAnalogStickInfo2.fXAxisValue) > 0.00001f || absf(fy - gLastAnalogStickInfo2.fYAxisValue) > 0.00001f)
        {
            gLastAnalogStickInfo2.fXAxisValue = fx;
            gLastAnalogStickInfo2.fYAxisValue = fy;
            gLastAnalogStickInfo2.nController = 0;
            gLastAnalogStickInfo2.nSide = AptInputType_RightAnalogStick;
            nPadState |= CONTROLLER_A1;
            AptAddToInputAnalogQueue(gLastAnalogStickInfo2);
        }
        fx = (float)(pStickValues->lx - 127) / 255.f;
        fy = -(float)(pStickValues->ly - 127) / 255.f;
        if (absf(fx - gLastAnalogStickInfo1.fXAxisValue) > 0.00001f || absf(fy - gLastAnalogStickInfo1.fYAxisValue) > 0.00001f)
        {
            gLastAnalogStickInfo1.fXAxisValue = fx;
            gLastAnalogStickInfo1.fYAxisValue = fy;
            gLastAnalogStickInfo1.nController = 0;
            gLastAnalogStickInfo1.nSide = AptInputType_LeftAnalogStick;
            nPadState |= CONTROLLER_A2;
            AptAddToInputAnalogQueue(gLastAnalogStickInfo1);
        }
    }

    nPadUpEvent = (nPadState ^ nPrevPadState) & nPrevPadState;
    nPadDownEvent = (nPadState ^ nPrevPadState) & nPadState;

    nPrevPadState = nPadState;
}

// ----------------------------------------------------------------------------
#elif defined(EA_PLATFORM_PSP)
//void _getInput( int &nPadUpEvent, int &nPadDownEvent )
void _getInput(int nController, int &nPadUpEvent, int &nPadDownEvent)
{
/*
    static int nPrevPadState = 0;

    // read pad state
    //int nPadState = PAD_state(0);
    int nPadState   = sceEmuPadRead();
    int nPress      = nPadState & ~nPrevPadState;

#if defined( PSP_EMU )
    #if defined( DEBUG_PAD )
    if( nPadState ) AptAuxEAGLREAL_DebugPrint( "<_getInput>\n" );

    if( nPrevPadState | nPadState | nPress )
    {
        AptAuxEAGLREAL_DebugPrint( "\t<info msg='_getInput: padst: prev(%d) curr(%d) press(%d)'/>\n", nPrevPadState, nPadState, nPress );
    }
    #endif
#endif

    // nothing to do
    if ( nPress & PSP_BUTTON_SELECT )
    {
#if defined( PSP_EMU )
    #if defined( DEBUG_PAD )
        AptAuxEAGLREAL_DebugPrint( "\t<info msg='_getInput: nothing to do!'/>\n" );
    #endif
#endif

        return ;
    }

    if ( nPadState & PSP_BUTTON_START )     {   nPadState |= CONTROLLER_START;
#if defined( DEBUG_PAD )
        AptAuxEAGLREAL_DebugPrint("\t<info msg='BUTTON_START'/>\n" );
#endif
    }

    if ( nPadState & PSP_BUTTON_SELECT )    {   nPadState |= CONTROLLER_SELECT;
#if defined( DEBUG_PAD )
        AptAuxEAGLREAL_DebugPrint("\t<info msg='BUTTON_SELECT'/>\n" );
#endif
    }

    if ( nPadState & PSP_BUTTON_UP )    {   nPadState |= CONTROLLER_UP;
#if defined( DEBUG_PAD )
        AptAuxEAGLREAL_DebugPrint("\t<info msg='BUTTON_UP'/>\n" );
#endif
    }

    if ( nPadState & PSP_BUTTON_DOWN )  {   nPadState |= CONTROLLER_DOWN;
#if defined( DEBUG_PAD )
        AptAuxEAGLREAL_DebugPrint("\t<info msg='BUTTON_DOWN'/>\n" );
#endif
    }

    if ( nPadState & PSP_BUTTON_LEFT )  {   nPadState |= CONTROLLER_LEFT;
#if defined( DEBUG_PAD )
        AptAuxEAGLREAL_DebugPrint("\t<info msg='BUTTON_LEFT'/>\n" );
#endif
    }

    if ( nPadState & PSP_BUTTON_RIGHT ) {   nPadState |= CONTROLLER_RIGHT;
#if defined( DEBUG_PAD )
        AptAuxEAGLREAL_DebugPrint("\t<info msg='BUTTON_RIGHT'/>\n" );
#endif
    }

    if ( nPadState & PSP_BUTTON_TRIANGLE )  {   nPadState |= CONTROLLER_TRIANGLE;
#if defined( DEBUG_PAD )
        AptAuxEAGLREAL_DebugPrint("\t<info msg='BUTTON_TRIANGLE'/>\n" );
#endif
    }

    if ( nPadState & PSP_BUTTON_SQUARE )    {   nPadState |= CONTROLLER_SQUARE;
#if defined( DEBUG_PAD )
        AptAuxEAGLREAL_DebugPrint("\t<info msg='BUTTON_SQUARE'/>\n" );
#endif
    }

    if ( nPadState & PSP_BUTTON_CIRCLE )    {   nPadState |= CONTROLLER_CIRCLE;
#if defined( DEBUG_PAD )
        AptAuxEAGLREAL_DebugPrint("\t<info msg='BUTTON_CIRCLE'/>\n" );
#endif
    }

    if ( nPadState & PSP_BUTTON_X )         {   nPadState |= CONTROLLER_CROSS;
#if defined( DEBUG_PAD )
        AptAuxEAGLREAL_DebugPrint("\t<info msg='BUTTON_X'/>\n" );
#endif
    }

    if ( nPadState & PSP_BUTTON_L1 )        {   nPadState |= CONTROLLER_L1;
#if defined( DEBUG_PAD )
        AptAuxEAGLREAL_DebugPrint("\t<info msg='BUTTON_L1'/>\n" );
#endif
    }

    if ( nPadState & PSP_BUTTON_R1 )        {   nPadState |= CONTROLLER_R1;
#if defined( DEBUG_PAD )
        AptAuxEAGLREAL_DebugPrint("\t<info msg='BUTTON_R1'/>\n" );
#endif
    }

#if defined( PSP_EMU )
    // --- only for the emulator... probably not available in the hardware
    if ( nPadState & PSP_BUTTON_L2 )    {   nPadState |= CONTROLLER_L2;
    #if defined( DEBUG_PAD )
        AptAuxEAGLREAL_DebugPrint("\t<info msg='BUTTON_L2'/>\n" );
    #endif
    }

    if ( nPadState & PSP_BUTTON_R2 )    {   nPadState |= CONTROLLER_R2;
    #if defined( DEBUG_PAD )
        AptAuxEAGLREAL_DebugPrint("\t<info msg='BUTTON_R2'/>\n" );
    #endif
    }
#endif  // PS_EMU

    if ( nPadState & SCEEMU_PAD_UP )    {   nPadState |= CONTROLLER_UP;
    #if defined( DEBUG_PAD )
        AptAuxEAGLREAL_DebugPrint("\t<info msg='PAD_UP'/>\n" );
    #endif
    }

    if ( nPadState & SCEEMU_PAD_DOWN )  {   nPadState |= CONTROLLER_DOWN;
    #if defined( DEBUG_PAD )
        AptAuxEAGLREAL_DebugPrint("\t<info msg='PAD_DOWN'/>\n" );
    #endif
    }

    if ( nPadState & SCEEMU_PAD_LEFT )  {   nPadState |= CONTROLLER_LEFT;
    #if defined( DEBUG_PAD )
        AptAuxEAGLREAL_DebugPrint("\t<info msg='PAD_LEFT'/>\n" );
    #endif
    }

    if ( nPadState & SCEEMU_PAD_RIGHT ) {   nPadState |= CONTROLLER_RIGHT;
    #if defined( DEBUG_PAD )
        AptAuxEAGLREAL_DebugPrint("\t<info msg='PAD_RIGHT'/>\n" );
    #endif
    }

    // TODO: add analog input?
    if ( nPadState & SCEEMU_PAD_A ) {   nPadState |= CONTROLLER_CIRCLE;
    #if defined( DEBUG_PAD )
        AptAuxEAGLREAL_DebugPrint("\t<info msg='PAD_A (circle)'/>\n" );
    #endif
    }

    if ( nPadState & SCEEMU_PAD_B ) {   nPadState |= CONTROLLER_CROSS;
    #if defined( DEBUG_PAD )
        AptAuxEAGLREAL_DebugPrint("\t<info msg='PAD_B (cross)'/>\n" );
    #endif
    }

    nPadUpEvent     = (nPadState ^ nPrevPadState) & nPrevPadState;
    nPadDownEvent   = (nPadState ^ nPrevPadState) & nPadState;

    nPrevPadState   = nPadState;

#if defined( PSP_EMU )
    #if defined( DEBUG_PAD )
    if( nPadUpEvent | nPadDownEvent | nPadState )
    {
        AptAuxEAGLREAL_DebugPrint("\t<info msg='_getInput: st(%d) evt: Up(%d) Dn(%d)'/>\n", nPadState, nPadUpEvent, nPadDownEvent );
    }
    if( nPadState ) AptAuxEAGLREAL_DebugPrint("<_getInput/>\n" );
    #endif
#endif
*/
/*
    const CONTROLLER *pPad = PAD_getdataptr(nController);
    static int nPrevPadState = 0;
    int nPadState = 0;

    if (pPad->DigitalButtonFlags & PS_UP)       nPadState |= CONTROLLER_UP;
    if (pPad->DigitalButtonFlags & PS_DOWN)     nPadState |= CONTROLLER_DOWN;
    if (pPad->DigitalButtonFlags & PS_LEFT)     nPadState |= CONTROLLER_LEFT;
    if (pPad->DigitalButtonFlags & PS_RIGHT)    nPadState |= CONTROLLER_RIGHT;
    if (pPad->DigitalButtonFlags & PS_START)    nPadState |= CONTROLLER_START;
    if (pPad->DigitalButtonFlags & PS_SELECT)   nPadState |= CONTROLLER_SELECT;
    if (pPad->DigitalButtonFlags & PS_CROSS)    nPadState |= CONTROLLER_CROSS;
    if (pPad->DigitalButtonFlags & PS_CIRCLE)   nPadState |= CONTROLLER_CIRCLE;
    if (pPad->DigitalButtonFlags & PS_SQUARE)   nPadState |= CONTROLLER_SQUARE;
    if (pPad->DigitalButtonFlags & PS_TRIANGLE) nPadState |= CONTROLLER_TRIANGLE;
    if (pPad->DigitalButtonFlags & PS_L1)       nPadState |= CONTROLLER_L1;
    if (pPad->DigitalButtonFlags & PS_R1)       nPadState |= CONTROLLER_R1;
    if (pPad->DigitalButtonFlags & PS_L2)       nPadState |= CONTROLLER_L2;
    if (pPad->DigitalButtonFlags & PS_R2)       nPadState |= CONTROLLER_R2;

    nPadUpEvent = (nPadState ^ nPrevPadState) & nPrevPadState;
    nPadDownEvent = (nPadState ^ nPrevPadState) & nPadState;

    nPrevPadState = nPadState;
*/
}   // end of _getInput for PSP

// ----------------------------------------------------------------------------

#elif defined(EA_PLATFORM_GAMECUBE)
void _getInput(int nController, int &nPadUpEvent, int &nPadDownEvent)
{
    const GCPADStatus *pPadsData = PAD_getdataptr(nController);
    const GCPADStatus *pPad =  pPadsData + nController;
    static int nPrevPadState = 0;
    int nPadState = 0;


    if (pPad->button & PAD_BUTTON_UP) nPadState |= CONTROLLER_UP;
    if (pPad->button & PAD_BUTTON_DOWN) nPadState |= CONTROLLER_DOWN;
    if (pPad->button & PAD_BUTTON_LEFT) nPadState |= CONTROLLER_LEFT;
    if (pPad->button & PAD_BUTTON_RIGHT) nPadState |= CONTROLLER_RIGHT;
    if (pPad->button & PAD_BUTTON_START) nPadState |= CONTROLLER_START;

    if (pPad->button & PAD_BUTTON_A) nPadState |= CONTROLLER_CROSS;
    if (pPad->button & PAD_BUTTON_B) nPadState |= CONTROLLER_CIRCLE;
    if (pPad->button & PAD_BUTTON_X) nPadState |= CONTROLLER_SQUARE;
    if (pPad->button & PAD_BUTTON_Y)    nPadState |= CONTROLLER_TRIANGLE;
    if (pPad->triggerLeft > 0) nPadState |= CONTROLLER_L1;
    if (pPad->triggerRight > 0) nPadState |= CONTROLLER_R1;

    nPadUpEvent = (nPadState ^ nPrevPadState) & nPrevPadState;
    nPadDownEvent = (nPadState ^ nPrevPadState) & nPadState;

    if (absf(pPad->stickX - gLastAnalogStickInfo2.fXAxisValue) > 0.00001f || absf(pPad->stickY - gLastAnalogStickInfo2.fYAxisValue) > 0.00001f)
    {
        gLastAnalogStickInfo2.fXAxisValue = pPad->stickX;
        gLastAnalogStickInfo2.fYAxisValue = pPad->stickY;
        gLastAnalogStickInfo2.nController = 0;
        gLastAnalogStickInfo2.nSide = AptInputType_LeftAnalogStick;
        nPadState |= CONTROLLER_A2;
        AptAddToInputAnalogQueue(gLastAnalogStickInfo2);
    }

    if (absf(pPad->substickX - gLastAnalogStickInfo1.fXAxisValue) > 0.00001f || absf(pPad->substickY - gLastAnalogStickInfo1.fYAxisValue) > 0.00001f)
    {
        gLastAnalogStickInfo1.fXAxisValue = pPad->substickX;
        gLastAnalogStickInfo1.fYAxisValue = pPad->substickY;
        gLastAnalogStickInfo1.nController = 0;
        gLastAnalogStickInfo1.nSide = AptInputType_RightAnalogStick;
        nPadState |= CONTROLLER_A1;
        AptAddToInputAnalogQueue(gLastAnalogStickInfo1);
    }


    nPrevPadState = nPadState;
}

#endif // _getInput

#if 0
/* Bits for the cmd field below */
#define T2K_GRID_FIT        0x0001
#define T2K_SCAN_CONVERT    0x0002
#define T2K_RETURN_OUTLINES 0x0004
#define T2K_CODE_IS_GINDEX  0x0008 /* Otherwise it is the charactercode */
#define T2K_USE_FRAC_PEN    0x0010
#define T2K_SKIP_SCAN_BM    0x0020 /* Everything works as normal, however we do _not_ generate the actual bitmap */
#define T2K_TV_MODE         0x0040 /* Ideal for TV if you use integer metrics, and gray-scale (please turn off T2K_GRID_FIT) */
#define T2K_NAT_GRID_FIT    0x0080 /* Enables native TrueType hint/gridding support */
#define T2K_LCD_MODE        0x0100 /* Ideal for LCD screens */
#define T2K_Y_ALIGN         0x0200 /* Ideal with T2K_LCD_MODE, and T2K_TV_MODE */

#define T2K_TV_MODE_2  ( T2K_TV_MODE  | T2K_Y_ALIGN )

#define T2K_LCD_MODE_2 ( T2K_LCD_MODE | T2K_Y_ALIGN )
#define T2K_LCD_MODE_3 ( T2K_LCD_MODE | T2K_GRID_FIT )
#define T2K_LCD_MODE_4 ( T2K_LCD_MODE | T2K_NAT_GRID_FIT )
extern int HACK_FF_AntialiasMode;
extern int HACK_FF_Scale;
#endif

#if defined(EA_PLATFORM_PLAYSTATION2) || defined(EA_PLATFORM_XBOX) || defined(EA_PLATFORM_GAMECUBE)
void _doInput()
{
#define APT_KEY_EVENTS(sk, dk, c) if (nPadDownEvent & sk) { AptAddToInputQueue((AptInputType)dk, AptInputState_Pressed, c); } \
                                  else if (nPadUpEvent & sk) { AptAddToInputQueue((AptInputType)dk, AptInputState_Released, c); }
    int nPadUpEvent;
    int nPadDownEvent;

    PAD_update();
    _getInput(0, nPadUpEvent, nPadDownEvent);


#if 0
    const XBGAMEPAD *pPad = PAD_getdataptr(0);
    unsigned int nPadState = 0;
    const int XBOX_CONTROLLER_THRESHOLD = 32;
    if (pPad->wLastButtons & XINPUT_GAMEPAD_DPAD_UP) nPadState |= CONTROLLER_UP;
    if (pPad->wLastButtons & XINPUT_GAMEPAD_DPAD_DOWN) nPadState |= CONTROLLER_DOWN;
    if (pPad->wLastButtons & XINPUT_GAMEPAD_DPAD_LEFT) nPadState |= CONTROLLER_LEFT;
    if (pPad->wLastButtons & XINPUT_GAMEPAD_DPAD_RIGHT) nPadState |= CONTROLLER_RIGHT;
    if (pPad->wLastButtons & XINPUT_GAMEPAD_START) nPadState |= CONTROLLER_START;
    if (pPad->wLastButtons & XINPUT_GAMEPAD_BACK) nPadState |= CONTROLLER_SELECT;
    if (pPad->bLastAnalogButtons[XINPUT_GAMEPAD_A] > XBOX_CONTROLLER_THRESHOLD) nPadState |= CONTROLLER_CROSS;
    if (pPad->bLastAnalogButtons[XINPUT_GAMEPAD_B] > XBOX_CONTROLLER_THRESHOLD) nPadState |= CONTROLLER_CIRCLE;
    if (pPad->bLastAnalogButtons[XINPUT_GAMEPAD_X] > XBOX_CONTROLLER_THRESHOLD) nPadState |= CONTROLLER_SQUARE;
    if (pPad->bLastAnalogButtons[XINPUT_GAMEPAD_Y] > XBOX_CONTROLLER_THRESHOLD) nPadState |= CONTROLLER_TRIANGLE;
    if (pPad->bLastAnalogButtons[XINPUT_GAMEPAD_LEFT_TRIGGER] > XBOX_CONTROLLER_THRESHOLD) nPadState |= CONTROLLER_L1;
    if (pPad->bLastAnalogButtons[XINPUT_GAMEPAD_RIGHT_TRIGGER] > XBOX_CONTROLLER_THRESHOLD) nPadState |= CONTROLLER_R1;
    if (nPadState & (CONTROLLER_R1|CONTROLLER_L1))
    {
        if (nPadState & CONTROLLER_R1)
        {
            HACK_FF_Scale = 1;
            if (nPadState & CONTROLLER_UP)
            {
                HACK_FF_AntialiasMode = T2K_SCAN_CONVERT;
            }
            else if (nPadState & CONTROLLER_LEFT)
            {
                HACK_FF_AntialiasMode = T2K_SCAN_CONVERT | T2K_TV_MODE;
            }
            else if (nPadState & CONTROLLER_DOWN)
            {
                HACK_FF_AntialiasMode = T2K_SCAN_CONVERT | T2K_NAT_GRID_FIT;
            }
            else if (nPadState & CONTROLLER_RIGHT)
            {
                HACK_FF_AntialiasMode = T2K_SCAN_CONVERT | T2K_TV_MODE_2;
            }
        }
        else if (nPadState & CONTROLLER_L1)
        {
            if (nPadState & CONTROLLER_UP)
            {
                HACK_FF_AntialiasMode = T2K_SCAN_CONVERT | T2K_GRID_FIT;
                HACK_FF_Scale = 1;
            }
            else if (nPadState & CONTROLLER_LEFT)
            {
                HACK_FF_AntialiasMode = T2K_SCAN_CONVERT | T2K_Y_ALIGN;
                HACK_FF_Scale = 1;
            }
            else if (nPadState & CONTROLLER_DOWN)
            {
                HACK_FF_AntialiasMode = T2K_SCAN_CONVERT | T2K_Y_ALIGN | T2K_NAT_GRID_FIT;
                HACK_FF_Scale = 1;
            }
            else if (nPadState & CONTROLLER_RIGHT)
            {
                HACK_FF_AntialiasMode = T2K_SCAN_CONVERT | T2K_Y_ALIGN | T2K_GRID_FIT;
                HACK_FF_Scale = 1;
            }
        }
        FF.flushCache();
        AptDeallocAllAssetString();
    }
#endif

    APT_KEY_EVENTS(CONTROLLER_CROSS, AptInputType_PadCross, AptInputController_Gamepad0);
    //APT_KEY_EVENTS(CONTROLLER_CROSS, 'x', AptInputController_Gamepad0);
    APT_KEY_EVENTS(CONTROLLER_CROSS, AptInputType_MouseButton0, AptInputController_Gamepad0);

    APT_KEY_EVENTS(CONTROLLER_SQUARE, AptInputType_PadSquare, AptInputController_Gamepad0);
    //APT_KEY_EVENTS(CONTROLLER_SQUARE, 's', AptInputController_Gamepad0);

    APT_KEY_EVENTS(CONTROLLER_TRIANGLE, AptInputType_PadTriangle, AptInputController_Gamepad0);
    //APT_KEY_EVENTS(CONTROLLER_TRIANGLE, 't', AptInputController_Gamepad0);

    APT_KEY_EVENTS(CONTROLLER_CIRCLE, AptInputType_PadCircle, AptInputController_Gamepad0);
    //APT_KEY_EVENTS(CONTROLLER_CIRCLE, 'c', AptInputController_Gamepad0);

    APT_KEY_EVENTS(CONTROLLER_UP, AptInputType_Up, AptInputController_Gamepad0);
    APT_KEY_EVENTS(CONTROLLER_DOWN, AptInputType_Down, AptInputController_Gamepad0);
    APT_KEY_EVENTS(CONTROLLER_LEFT, AptInputType_Left, AptInputController_Gamepad0);
    APT_KEY_EVENTS(CONTROLLER_RIGHT, AptInputType_Right, AptInputController_Gamepad0);
    APT_KEY_EVENTS(CONTROLLER_START, AptInputType_PadStart, AptInputController_Gamepad0);

    // added as per suggestion from CAFE.
    APT_KEY_EVENTS(CONTROLLER_L1, AptInputType_PadL1, AptInputController_Gamepad0);
    APT_KEY_EVENTS(CONTROLLER_R1, AptInputType_PadR1, AptInputController_Gamepad0);
    APT_KEY_EVENTS(CONTROLLER_L2, AptInputType_PadL2, AptInputController_Gamepad0);
    APT_KEY_EVENTS(CONTROLLER_R2, AptInputType_PadR2, AptInputController_Gamepad0);


    if (nPadDownEvent & CONTROLLER_START)
    {
        gbRunProgram = 0;
    }

    if (nPadDownEvent & CONTROLLER_SELECT)
    {
        gbSaveScreenShot = 1;
    }
#if defined(APT_XML_MEMORY_DUMP_SUPPORTED)
    if (nPadDownEvent & CONTROLLER_L1)
    {
        extern void PrintObjectMapXML(const char*);
        PrintObjectMapXML("Requested");
    }
#endif
#if 0 && defined(APT_DO_PROFILE)
    if (nPadDownEvent & CONTROLLER_L1)
    {
        extern void APT_PROFILE_PrintProfile();
        //APT_PROFILE_PrintProfile();
        AptAuxEAGLREAL_PrintProfileInfo(true, "profileDump.xml");
    }
#endif

}


// code given by chan wilson for adding controller events.
static int nControllerList[8];

void _checkControllers()
{
    for (int i = 0; i < 4; i++)
    {
        if (nControllerList[i] != (int)PAD_active(i))
        {
            nControllerList[i] = PAD_active(i);
            if (PAD_active(i))
            {
                AptAddToInputQueue(AptInputType_Connected, AptInputState_Pressed, (AptInputController)(AptInputController_Gamepad0 + i));
            }
            else
            {
                AptAddToInputQueue(AptInputType_Disconnected, AptInputState_Pressed, (AptInputController)(AptInputController_Gamepad0 + i));
            }
        }
    }
}


#endif
// -----------------------------------------------------------------------------
#if defined(EA_PLATFORM_PSP)
void _doInput()
{
#if defined(DEBUG_PAD)
    #define APT_KEY_EVENTS(sk, dk, c) if (nPadDownEvent & sk) {                                             \
                                        AptAddToInputQueue((AptInputType)dk, AptInputState_Pressed, c);     \
                                        AptAuxEAGLREAL_DebugPrint("\t<info msg='==(%d:%d):Dn(%d):[0x%x]'/>\n", sk, dk, nPadDownEvent, (sk & nPadDownEvent));  \
                                        }                                                                   \
                                        else if (nPadUpEvent & sk) {                                        \
                                        AptAddToInputQueue((AptInputType)dk, AptInputState_Released, c);    \
                                        AptAuxEAGLREAL_DebugPrint("\t<info msg='==(%d:%d):Up(%d):[0x%x]'/>\n", sk, dk, nPadUpEvent, (sk & nPadUpEvent));  \
                                        }
#else
    #define APT_KEY_EVENTS(sk, dk, c) if (nPadDownEvent & sk) { AptAddToInputQueue((AptInputType)dk, AptInputState_Pressed, c); }       \
                                        else if (nPadUpEvent & sk) { AptAddToInputQueue((AptInputType)dk, AptInputState_Released, c); }
#endif

    int nPadUpEvent;
    int nPadDownEvent;

    PAD_update();
    _getInput(0, nPadUpEvent, nPadDownEvent);

#if defined( PSP_EMU )
    #if defined( DEBUG_PAD )
    if( nPadUpEvent | nPadDownEvent )
    {
        AptAuxEAGLREAL_DebugPrint("<doInput>\n" );
        AptAuxEAGLREAL_DebugPrint("\t<info msg='_doInput: PadEvent: Up( %d ) Down( %d )'/>\n", nPadUpEvent, nPadDownEvent );
    }
    #endif
#endif

    APT_KEY_EVENTS(CONTROLLER_CROSS, AptInputType_PadCross, AptInputController_Gamepad0);
    APT_KEY_EVENTS(CONTROLLER_CROSS, 'x', AptInputController_Gamepad0);
    APT_KEY_EVENTS(CONTROLLER_CROSS, AptInputType_MouseButton0, AptInputController_Gamepad0);

    APT_KEY_EVENTS(CONTROLLER_SQUARE, AptInputType_PadSquare, AptInputController_Gamepad0);
    APT_KEY_EVENTS(CONTROLLER_SQUARE, 's', AptInputController_Gamepad0);

    APT_KEY_EVENTS(CONTROLLER_TRIANGLE, AptInputType_PadTriangle, AptInputController_Gamepad0);
    APT_KEY_EVENTS(CONTROLLER_TRIANGLE, 't', AptInputController_Gamepad0);

    APT_KEY_EVENTS(CONTROLLER_CIRCLE, AptInputType_PadCircle, AptInputController_Gamepad0);
    APT_KEY_EVENTS(CONTROLLER_CIRCLE, 'c', AptInputController_Gamepad0);

    APT_KEY_EVENTS(CONTROLLER_UP,   AptInputType_Up, AptInputController_Gamepad0);
    APT_KEY_EVENTS(CONTROLLER_DOWN, AptInputType_Down, AptInputController_Gamepad0);
    APT_KEY_EVENTS(CONTROLLER_LEFT, AptInputType_Left, AptInputController_Gamepad0);
    APT_KEY_EVENTS(CONTROLLER_RIGHT, AptInputType_Right, AptInputController_Gamepad0);

    // added as per suggestion from CAFE.
    APT_KEY_EVENTS(CONTROLLER_L1, AptInputType_PadL1, AptInputController_Gamepad0);
    APT_KEY_EVENTS(CONTROLLER_R1, AptInputType_PadR1, AptInputController_Gamepad0);
    APT_KEY_EVENTS(CONTROLLER_L2, AptInputType_PadL2, AptInputController_Gamepad0);
    APT_KEY_EVENTS(CONTROLLER_R2, AptInputType_PadR2, AptInputController_Gamepad0);

    if (nPadDownEvent & CONTROLLER_START)
    {
        //gbRunProgram = 0;
    }

#if defined( PSP_EMU )
    #if defined( DEBUG_PAD )
    if( nPadUpEvent | nPadDownEvent )
    {
        AptAuxEAGLREAL_DebugPrint("<doInput/>\n");
    }
    #endif
#endif

}
void _checkControllers()
{
    //AptAddToInputQueue(AptInputType_Connected, AptInputState_Pressed, (AptInputController)(AptInputController_Gamepad0 + i));
    //AptAddToInputQueue( AptInputType_Connected, AptInputState_Pressed, (AptInputController)(AptInputController_Gamepad0));
    return ;
}
#endif
///////////////////////////////////////////////////////////////////////////////////////////////////////////

#if defined (EA_PLATFORM_WINDOWS)
// default mouse button state is on
int gMouseButton0 = 0;
// store the last keymap
unsigned char gKeyMap[256];     // changed to unsigned because of repeat rate problem reported by CAFE team. bug#244
int gMouseX,gMouseY, gMouseZ;
void _getInput()
{
    int i;

    MOUSESTRUCT         *mstate;
    mstate = MOUSE_getdata();

    // We only update if the mouse is within the Apt renderable protion of the screen
    if((mstate->x >= 0 && mstate->x <= SCREEN_WIDTH) && (mstate->y >= 0 && mstate->y <= SCREEN_HEIGHT))
    {
        if(gMouseX != mstate->x || gMouseY != mstate->y)
        {
            gMouseX=mstate->x;
            gMouseY=mstate->y;
            AptSetMousePosition(gMouseX , gMouseY);
        }
        if(gMouseZ != mstate->z)                        // (LEO) this is for the mouse wheel.
        {
            if(mstate->z > gMouseZ)
                AptAddToInputQueue((AptInputType)2, AptInputState_WheelUp, AptInputController_Mouse);
            else
                AptAddToInputQueue((AptInputType)2, AptInputState_WheelDown, AptInputController_Mouse);
            MOUSE_setstate(INPUT_STATE_Z, 0);           // Reset to zero so we don't max out our wheel values
            gMouseZ = mstate->z;
        }

        if((mstate->buttons[0]!=0)&& (gMouseButton0==0))
        {
            gMouseButton0 = 1;
            AptAddToInputQueue(AptInputType_MouseButton0, AptInputState_Pressed, AptInputController_Mouse);
        }
        else if((mstate->buttons[0]==0)&& (gMouseButton0==1))
        {
            gMouseButton0 = 0;
            AptAddToInputQueue(AptInputType_MouseButton0, AptInputState_Released, AptInputController_Mouse);
        }
    }

    KEYSTRUCT *kstate =KEY_getdata();
    static int bAptResetCalled = 0 ;
    for(i=0;i<256;i++)
    {
        if(kstate->keymap[i]!=gKeyMap[i])
        {
            int key = convertscancode(i);
            if(key == VK_ESCAPE)
            {
                gbRunProgram = 0;
                break;
            }
            // added these shortcuts to test AptReset
            if(key == 0x3c00)       // F2
            {
                if (bAptResetCalled == 0)
                {
                    AptReset() ;
                    bAptResetCalled = 1 ;
                }
                break;
            }

            if(key == 0x3d00)       // F3
            {
                AptLoadAnimation("", "_level0") ;
                bAptResetCalled = 0 ;
                break;
            }
            if(key == 0x3e00)       // F4
            {
                bAptResetCalled = 0 ;
                AptLoadAnimation(gszFileBase, "_level0") ;
                break;
            }

            if(kstate->keymap[i])
            {
                AptAddToInputQueue((AptInputType)key, AptInputState_Pressed, AptInputController_Keyboard);
            }else
            {
                AptAddToInputQueue((AptInputType)key, AptInputState_Released, AptInputController_Keyboard);
            }
            gKeyMap[i] = kstate->keymap[i];
        }
    }
}
void _doInput()
{
    _getInput();
}

// just return from this function.
// this has to be defined as it gets called from main loop.
void _checkControllers()
{
    return ;
}

#endif

void _loadFontFiles()
{
    int nSize;
    char *gpFontFile = (char *)FILE_loadsizez("fonts.txt", &nSize, MB_LOW);
    if (gpFontFile == 0)
    {
        printf("\n\n\nWarning: no 'fonts.txt' file found! Font rendering disabled.\n\n\n");
        gFonts.nFonts = 0;
        return;
    }
    char *p = gpFontFile;
    //printf("Fonts getting loaded are %s \n", p) ;
    char *lastPtr = p;
    int i;
    for (i = 0; ; i++)
    {
        while (*p != '\n' && *p != '\r' && (p - gpFontFile) < nSize)
        {
            p++;
        }
        if ((p - gpFontFile) == nSize) break;
        *p = 0;
        p++;
        gFonts.pData[gFonts.nFonts] = FILE_loadsize(lastPtr,&(gFonts.nDataSize[gFonts.nFonts]),MB_LOW);
        gFonts.nFonts++;
        while (*p == '\n' || *p == '\r')
        {
            *p++ = 0;
        }
        lastPtr = p;
    }

    printf("%d fonts loaded\n", gFonts.nFonts);

    assert(gFonts.nFonts <= (int)(sizeof(gFonts.pData)/sizeof(gFonts.pData[0])));
    MEM_free(gpFontFile) ;
}

void _reshapeViewport()
{
    int nAnimWidth, nAnimHeight;
    float fXOffset = 0.f, fYOffset = 0.f;

    AptGetAnimationSize(&nAnimWidth, &nAnimHeight);

    pView->SetShape(0, 0, static_cast<float>(SCREEN_WIDTH), static_cast<float>(SCREEN_HEIGHT), VIEWPORT_NEAR_Z, VIEWPORT_FAR_Z);
    pView->SetOrthographicScreenSpace(1, 10.f);

    if (nAnimWidth <= 0 || nAnimHeight <= 0) return;
#if defined(EA_PLATFORM_WINDOWS)
    // for hit test reason, I don't want to move the animation to center of the window
    fXOffset = 0.f;
    fYOffset = 0.f;
    pView->SetShape(fXOffset, fYOffset, static_cast<float>(nAnimWidth), static_cast<float>(nAnimHeight), VIEWPORT_NEAR_Z, VIEWPORT_FAR_Z);
#elif defined(EA_PLATFORM_GAMECUBE)
    // DC - I want to center the viewport (cause I'm cool like dat!)
    fXOffset    = (SCREEN_WIDTH  - nAnimWidth) * 0.5f;
    fYOffset    = (SCREEN_HEIGHT - nAnimHeight)* 0.5f;
#elif defined(EA_PLATFORM_PSP)
    fXOffset    = (SCREEN_WIDTH  - nAnimWidth)  * 0.5f;
    fYOffset    = (SCREEN_HEIGHT - nAnimHeight) * 0.5f;
#else
    fXOffset = (SCREEN_WIDTH - nAnimWidth) * 0.5f;
    fYOffset = (SCREEN_HEIGHT - nAnimHeight) * 0.5f;
//  pView->SetShape(fXOffset, fYOffset, static_cast<float>(nAnimWidth), static_cast<float>(nAnimHeight), VIEWPORT_NEAR_Z, VIEWPORT_FAR_Z);
#endif

    // Set up a view (centre the anim)
    EAGL::Transform tx;
    tx.BuildIdentity();

#if defined(EA_PLATFORM_PSP)
    //tx.BuildTranslate(fXOffset, fYOffset, -1.f);
    //no z transform for psp
    tx.BuildTranslate(fXOffset - 0.5f, fYOffset - 0.5f, 0.0f);
#else
    tx.BuildTranslate(fXOffset - 0.5f, fYOffset - 0.5f, -5.f);
#endif

    pView->SetViewMatrix(tx.m);
}

void _drawFPS()
{
#if !defined(EA_PLATFORM_PSP)
    static float oldFrameTimes[10];
    static int nFramePos = 0;
    static char szBuf[32];
    static float fTime = 0;

    oldFrameTimes[nFramePos++] = AptAuxEAGLREAL_TimerGetTime();
    nFramePos %= sizeof(oldFrameTimes)/sizeof(oldFrameTimes[0]);

    if (!gbFPS)
        return;

    float avg = 0.f;
    for (unsigned i=0; i < sizeof(oldFrameTimes)/sizeof(oldFrameTimes[0]); i++)
    {
        avg += oldFrameTimes[i];
    }
    avg = (float)(sizeof(oldFrameTimes)/sizeof(oldFrameTimes[0])) / avg;

    fTime += 1.f / avg;
    if (fTime > 4.f)
    {
        printf("current fps: %d  freemem %d\n", (int)avg,MEM_totalunused());
        fTime = 0.f;
    }

#if !DISK_BOOT
    sprintf(szBuf, "fps: %d", (int)avg);
    FONT_setstate(FONTstand, FONT_COLOUR, CA_BLACK);
    FONT_drawtext(FONTstand, 11, 21, szBuf);
    FONT_setstate(FONTstand, FONT_COLOUR, CA_GREEN);
    FONT_drawtext(FONTstand, 10, 20, szBuf);
#endif

#else       // for PSP
//---------------------------------------------------------------------------
#define OLDFRAMESZ  10

#if defined( PSP_EMU )
    #define NUMERATOR   10240.0         // OLDFRAMESZ * 1024

#else
    #define NUMERATOR   10.0            // OLDFRAMESZ

#endif

    static  float   oldFrameTimes[] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
    static  float   avg         = 0.0;

    static  int     nFramePos   = 0;
    static  int     nFrameCnt   = 0;

    static  char    szBuf[32];

    // --- global sentinel
    if (!gbFPS) return;

#if defined( PSP_EMU )

    #if defined( DEBUG_FPS )
    // --- get current
    float fTime = AptAuxEAGLREAL_TimerGetTime();
    oldFrameTimes[nFramePos++] = fTime;

    #else
    oldFrameTimes[nFramePos++] = AptAuxEAGLREAL_TimerGetTime();

    #endif

#else
    oldFrameTimes[nFramePos++] = AptAuxEAGLREAL_TimerGetTime();

#endif

    // --- wrap index
    if( nFramePos == OLDFRAMESZ )
    {
        nFramePos = 0;
    }

    float sum = 0.0;

    for (unsigned i=0; i<OLDFRAMESZ; i++)
    {
        sum += oldFrameTimes[i];
    }

    // --- calc avg per frame
    if(sum>0.0)
    {
        //avg = ((float)OLDFRAMESZ)/sum*1024.0;
        avg = NUMERATOR/sum;
    }


    if(nFramePos==0)
    {
        nFrameCnt++;
        if(nFrameCnt>OLDFRAMESZ)
        {
            nFrameCnt       = (int) avg;
            gnFramesToRun   = (nFrameCnt<<4);
            nFrameCnt       = 0;

#if defined( DEBUG_FPS )
        #if defined( PSP_EMU )
            long cpucy  = CPU_getcycle();
            long unumem = MEM_totalunused();
            sceEmuPrintf("<info msg='at(%010d) sum(%06.01f) fps(%06.02f) cnt(%04d) tm(%6.1f) freemem(%d)'/>\n", cpucy, sum, avg, gnFramesToRun, fTime, unumem );

        #else
            printf("current fps: %d cnt: %d  freemem %d\n", (int)avg, gnFramesToRun, MEM_totalunused());

        #endif
#endif

        }

        //fTime = 0.f;
    }

#if !DISK_BOOT
    memset(szBuf,0,sizeof(szBuf));
    sprintf(szBuf, "fps: %d", (int)avg);
    FONT_setstate(FONTstand, FONT_COLOUR, CA_BLACK);
    FONT_drawtext(FONTstand, 11, 21, szBuf);
    FONT_setstate(FONTstand, FONT_COLOUR, CA_GREEN);
    FONT_drawtext(FONTstand, 10, 20, szBuf);
#endif

#endif      // end of big if def.
}
#if defined (EA_PLATFORM_GAMECUBE)
void* MyAlloc(u32 size)
{
    return MEM_alloc("screenshot buffer",size,MB_HIGH);
}
void MyFree( void* block)
{
    MEM_free(block);
}
#endif
int memprt = 0;

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
char *gpFFMemory = 0;
int gnFFMemClass = -1;
int nHighWaterFF = 0;
int nCurFF = 0;
void *_allocateMemory(size_t s)
{
    nCurFF += s;
    if (nCurFF > nHighWaterFF)
    {
        nHighWaterFF = nCurFF;
    }
    return MEM_alloc("ff", s, gnFFMemClass);
}
void _freeMemory(void *p)
{
    nCurFF -= MEM_size(p);
    MEM_free(p);
}
#define MYMIN(a,b) (((int)(a)<(int)(b)) ? (a) : (b))
void *_reallocateMemory(void *p, size_t newSize)
{
    nCurFF += newSize;
    if (nCurFF > nHighWaterFF)
    {
        nHighWaterFF = nCurFF;
    }
    nCurFF -= MEM_size(p);
    void *n = MEM_alloc("ff", newSize, gnFFMemClass);
    memcpy(n, p, MYMIN(MEM_size(p), newSize));
    MEM_free(p);
    return n;
}

// XXX - ASK
int iFreeMem, iFreeMem1 ;

void printFreeMem()
{
    int iTemp = MEM_totalunused() ;
    if (iTemp != iFreeMem)
    {
        //printf("freemem - %d difference = %d\n", MEM_totalunused(), iTemp - iFreeMem);
        AptAuxEAGLREAL_DebugPrint("freemem - %d difference = %d\n", MEM_totalunused(), iTemp - iFreeMem);
        iFreeMem = iTemp ;
    }
}



bool gbNewScreenReadyToBeRendered = false;

/*F************************************************************************************************/
/*!
Set a flag that we can use to know when a frame was rendered.

\Input

\Output

\Notes

\Version
1.0        05/04/04 (DC) Comment creation.

*/
/************************************************************************************************F*/
static void SetNewScreenReadyToBeRendered(const char *szScreenGrabName)
{
    gbNewScreenReadyToBeRendered = true;
}

/*F************************************************************************************************/
/*!
Take a screenShot, (doesn't do anything yet)

\Input

\Output

\Notes

\Version
1.0        05/04/04 (DC) Comment creation.

*/
/************************************************************************************************F*/
static void DebugGetScreenShot(const char *szScreenGrabName)
{

}



void PrintMovieclipInfo(AptMovieclipInformation *mcInfo)
{
    AptAuxEAGLREAL_DebugPrint("Apt-MovieclipInformation------------------------------\n");
    AptAuxEAGLREAL_DebugPrint("No. of Animations   ---- %d\n", mcInfo->nAnimations) ;
    AptAuxEAGLREAL_DebugPrint("No. of sprites      ---- %d\n", mcInfo->nAnimations) ;
    AptAuxEAGLREAL_DebugPrint("No. of shapes       ---- %d\n", mcInfo->nShapes) ;
    AptAuxEAGLREAL_DebugPrint("No. of Buttons      ---- %d\n", mcInfo->nButtons) ;
    AptAuxEAGLREAL_DebugPrint("No. of Static Text  ---- %d\n", mcInfo->nStaticText) ;
    AptAuxEAGLREAL_DebugPrint("No. of Dynamic Text ---- %d\n", mcInfo->nDynamicText) ;
    AptAuxEAGLREAL_DebugPrint("No. of Morphs       ---- %d\n", mcInfo->nMorph) ;
    AptAuxEAGLREAL_DebugPrint("------------------------------------------------------\n");

    return ;
}

#if defined(APT_GATHER_MEMORY_METRICS)
void PrintMemMetricsRecursive(AptMemoryMetrics * pTemp)
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

        PrintMemMetricsRecursive(AptMemoryMetrics::GetChildInstance(pTemp));
        AptAuxEAGLREAL_DebugPrint("</item>\n");
        pTemp = AptMemoryMetrics::GetSiblingInstance(pTemp);
    }
}
#endif

void PrintMemoryAllocationsInfo()
{
#if defined(APT_GATHER_MEMORY_METRICS)
    AptMemoryMetrics * pTemp = AptMemoryMetrics::GetFirstInstance();

    AptAuxEAGLREAL_DebugPrint("Apt-Memory allocations per Object------------------------------\n");
    AptAuxEAGLREAL_DebugPrint("Object,Total Count,Current Count,Current Bytes,Bytes/allocation,Max Concurrent Bytes\n");
    PrintMemMetricsRecursive(pTemp);

    AptAuxEAGLREAL_DebugPrint("---------------------------------------------------------------\n");
#endif
    return ;
}

/*F************************************************************************************************/
/*!
    \Function    PrintDebugInformation

    \Description
        Prints different type of debugging information.

    \Input
        None.

    \Output
        None.

    \Notes
        None.

    \Version    1.0        09/14/04 (ASK) First Version

*/
/************************************************************************************************F*/
void PrintDebugInformation()
{
    // turn on various calls in this function to print various debugging information

    AptMovieclipInformation mcInfo ;
    AptGetMovieclipInfo(&mcInfo) ;
    PrintMovieclipInfo(&mcInfo) ;

    PrintMemoryAllocationsInfo() ;

}


//---------------------------------------------------

void viewerExtracted(char * szFileBase) ;

// Setup our rendering example
#if defined (EA_PLATFORM_PSP)
int viewerMain( SceSize _ArgNum, void* _ArgData )
#else
void viewerMain(int argc, char **argv)
#endif
{

#if defined(EA_PLATFORM_PSP)
    //  int argc = (int) _ArgNum ;
    //int argc = 1 ;
    //char ** argv = (char ** )_ArgData ;

    int argc = _ArgNum / sizeof(char*);
    char** argv = static_cast<char**>(_ArgData);

    MEM_initsize(MEMSIZE);
#endif
#if !defined( PSP_EMU )
    char pSavedInputsFileName[260];
    bool bSaveInputs    = false;
#endif
    bool bPlayingInputs = false;
    AptEAGLRealAllocator FontFusionAllocator;

    THREAD_init();
    TIMER_init();
    PRINT_init();
    PRINT_setdevicestate(PRINT_DEBUGGER, 1);

#if defined(EA_PLATFORM_PLAYSTATION2)
    PS2_PAD_CONFIGOPTS ppcfg;
    PAD_getconfigopts(&ppcfg);
    ppcfg.loadiopmodules = false;
    PAD_setconfigopts(&ppcfg);
    gDiscOpts.maxpathlength = 60;
    gDiscOpts.usedisc = SCECdCD;
#endif

#if defined(EA_PLATFORM_PSP)
    LoadPrxModules();

    static int nControllerList[8];
    PAD_init();
    SYNCTASK_del((int (*)(int, int))PAD_update);
#endif

#if defined (EA_PLATFORM_PLAYSTATION2) || defined (EA_PLATFORM_XBOX) || defined (EA_PLATFORM_GAMECUBE)
    PAD_init();
    SYNCTASK_del((int (*)(int, int))PAD_update);
    for (int i = 0; i < 4; i++)
    {
        PAD_update();
        nControllerList[i] = PAD_active(i);
    }
#endif


#if defined(EA_PLATFORM_PSP)
    char * pTemp1 = NULL ;
    pTemp1 = (char *)MEM_alloc("tempalloc", 100, MB_LOW);
    MEM_free(pTemp1) ;

    FILESYSOPTS fso;
    fso.size = sizeof(fso);
    FILESYS_getopts(&fso);
    fso.malloc = MEM_alloc;
    fso.free = MEM_free;
    FILESYS_setopts(&fso);
    FILESYS_init(32);
    ASYNCFILE_init(16, MB_HIGH);

#else
    FILESYS_setmemcallbacks(MEM_alloc,MEM_free);
    FILESYS_init(0,0,0);
    ASYNCFILE_init(16, MB_HIGH);
#endif
#if DISK_BOOT
    int nBigHandle;
    FILESYS_setcurrentpath("cd:");
    FILESYS_addbigsync("cd:\\aptdata.big", MB_HIGH, 50, &nBigHandle);
#endif

    setupSND();
    setupEAGL();

    char *szFileBase        = "main"; // Default FileName.
    // --- length of file name string
    int   nFileNameLength = strlen(szFileBase);

    for (int i = 1; i < argc; i++)
    {
        if (stricmp(argv[i], "--saveinputs") == 0)
        {
            bSaveInputs = true;
            argv[i][0] = 0;
            if( argv[i+1] == NULL || argv[i+1][0] == '\0')
            {
                printf("Error: Command line Malformed!\n --saveinputs must be followed by a file name!\n\n");
                return;
            }

            // Grab the file name.
            strcpy(pSavedInputsFileName, BASEDIR);
            strcat(pSavedInputsFileName, argv[i+1]);
            i++;
        }
        else if (stricmp(argv[i], "--playinputs") == 0)
        {
            bPlayingInputs = true;
            argv[i][0] = 0;
            if( argv[i+1] == NULL || argv[i+1][0] == '\0')
            {
                printf("Error: Command line Malformed!\n --playinputs must be followed by a file name!\n\n");
                return;
            }


            // Grab the file name.
            strcpy(pSavedInputsFileName, BASEDIR);
            strcat(pSavedInputsFileName, argv[i+1]);
            i++;
        }
        else if (stricmp(argv[i], "--memdumpfile") == 0)
        {
            argv[i][0] = 0;
            if( argv[i+1] == NULL || argv[i+1][0] == '\0')
            {
                printf("Error: Command line Malformed!\n --memdumpfile must be followed by a file name!\n\n");
                return;
            }
            if (strlen(argv[i+1]) > 128)
            {
                printf("Error: Command line Malformed!\n --memdumpfile name cannot be bigger than 128!\n\n");
                return;
            }

            // Grab the file name.
#if defined (APT_GATHER_MEMORY_METRICS)
            AptAuxEAGLREAL_SetMemDumpFileName(argv[i+1]) ;
#endif
            i++;
        }
        else if (stricmp(argv[i], "--cycledumpfile") == 0)
        {
            argv[i][0] = 0;
            if( argv[i+1] == NULL || argv[i+1][0] == '\0')
            {
                printf("Error: Command line Malformed!\n --cycledumpfile must be followed by a file name!\n\n");
                return;
            }
            if (strlen(argv[i+1]) > 128)
            {
                printf("Error: Command line Malformed!\n --cycledumpfile name cannot be bigger than 128!\n\n");
                return;
            }

            // Grab the file name.
            AptAuxEAGLREAL_SetProfileDumpFileName(argv[i+1]) ;
            i++;
        }
        else if (stricmp(argv[i], "--dumpstats") == 0)
        {
            AptAuxEAGLREAL_SetDumpFlag(true) ;
        }
        else if (stricmp(argv[i], "--playinputsto") == 0)
        {
            ; // Do nothing. This feature is not supported (even on PCOpenGL).
        }
        else if (stricmp(argv[i], "--gctest") == 0)
        {
            gbPartialGarbageCollectEveryFrame = true;
        }
        else if (stricmp(argv[i], "--ignoretime") == 0) // This ignores real time, and always advances a frame. Useful for profiling or during XML dumps.
        {
            gbIgnoreRealTime = true;
        }
        else if (stricmp(argv[i], "--NoShutDown") == 0) // This ignores real time, and always advances a frame. Useful for profiling or during XML dumps.
        {
            gbNoShutDown = true ;
        }
        else if (stricmp(argv[i], "--DoAptResetInBetween") == 0) // This ignores real time, and always advances a frame. Useful for profiling or during XML dumps.
        {
            gbDoAptResetInBetween = true;
        }

        else if (stricmp(argv[i], "--runframes") == 0) // This tells the app to shutdown after x frames (should be combined with --ignoretime)
        {
            if( argv[i+1] == NULL || argv[i+1][0] == '\0')
            {
                printf("Error: Command line Malformed!\n --runframes must be followed by the number of frames!\n\n");
                return;
            }

            gnRunFrames = atoi(argv[i+1]);
            gnRunFrames1 = gnRunFrames ;
            i++;
        }
#if defined(APT_XML_MEMORY_DUMP_SUPPORTED)
        else if (stricmp(argv[i], "--xmldump") == 0)
        {
            extern bool gbDumpOnLoadAnimation;
            gbDumpOnLoadAnimation = true;
        }
#endif
        else
        {
            szFileBase      = argv[i]; // Grab FileName.
            nFileNameLength = strlen(szFileBase);
        }
    }
//#endif  // PSP_EMU

    // List Files? No Problem, we Loop.
    char *p = NULL;
    char *lastPtr = p;
    int   nSize = 0 ;
    char * gpTestFile = NULL ;
    if ( ( nFileNameLength > 4) && (strcmp(szFileBase + nFileNameLength - 4, ".lst") == 0))
    {
        gpTestFile = (char *)FILE_loadsizez(szFileBase, &nSize, MB_LOW);
        if (gpTestFile == 0)
        {
            printf("\n\n\nWarning: no '%s' file found! Nothing to do...\n\n\n", szFileBase);
            return;
        }
        p = gpTestFile;
        lastPtr = p;
        int i;
        for (i = 0; ; i++)
        {
            while (*p != '\n' && *p != '\r' && (p - gpTestFile) < nSize)
            {
                p++;
            }
            if ((p - gpTestFile) == nSize) break;
            *p = 0;
            p++;
            gbRunProgram = 1 ;
            if (gbNoShutDown)
            {
                szFileBase = lastPtr ;
                break ;
            }
            else
            {
                viewerExtracted(lastPtr) ;
            }
            while (*p == '\n' || *p == '\r')
            {
                *p++ = 0;
            }
            lastPtr = p;
        }
        if (!gbNoShutDown)
            return ;
    }

#if !defined( PSP_EMU )
    _loadFontFiles();

    gpFFMemory = (char *)MEM_alloc("FontFusion mess", 120000, MB_LOW);

    gnFFMemClass = MEMCLASS_getfree();

    MEMCLASS_create(gnFFMemClass, "FontFusion", gpFFMemory, 120000);

    //FFInitStub(_allocateMemory,_freeMemory,_reallocateMemory);
    FusionMemObject::sSetAllocator(&FontFusionAllocator);


#if defined(TIBPROFILER)
    uint32_t uBufferSizeInBytes = 0x00800000;
    void*    pProfilingBuffer = MEM_alloc("APT_PROFILErBuffer", uBufferSizeInBytes);
    AptProfileInitialize(pProfilingBuffer, uBufferSizeInBytes);
#endif

    AptAuxEAGLREAL_Initialize(pView, pDisp, BASEDIR, &gFonts);
#endif  // PSP_EMU

    // DC - Added Allocator calls.
    AptAllocatorInitialize();

#if defined(USE_APTXMLIMPL)
    // define USE_APTXMLIMPL if you want to include AptXmlImpl
    // you also need to do changes to AptViewer.build to incclude Aptxmlimpl library.
#if defined (EA_PLATFORM_PLAYSTATION2)
    CDirtyXmlImpl xmlImpl ;
    AptSetXMLImplementor(&xmlImpl);
#endif
#endif // end of xml part

    // Set This up so we can use the info later.
    gAptFuncs.pfnDebugSetScreenGrabPending = SetNewScreenReadyToBeRendered;

    AptInitParmsT initParams;
    initParams.iZombieVectorSize = 32 ;
    initParams.iCallStackDepth = 64 ;
    initParams.iLookupArraySize = 2048 ;
#if defined(EA_DEBUG)
    initParams.bPrintZombieDump = true;
#endif
    initParams.iLookupArraySize = 2028;
    //AptOptEnable(APT_OPT_SPRSTACK) ;
    AptInitialize(&initParams);
#if defined(DO_COVERAGE)
    AptExtObject * pExtObj = new TestAptExt();
    AptRegisterExtension(pExtObj);
#endif

#if !defined( PSP_EMU )
    if(bPlayingInputs)
    {
        extern void AptAuxEAGLREAL_PlayInputs(const char * fileName);
        AptAuxEAGLREAL_PlayInputs(pSavedInputsFileName);
    }
    if(bSaveInputs)
    {
        extern void AptAuxEAGLREAL_RecordInputs(const char * fileName);
        AptAuxEAGLREAL_RecordInputs(pSavedInputsFileName);
    }
#endif
    gszFileBase = szFileBase ;
    AptLoadAnimation(szFileBase, "_level0");

    AptSetValidFocusButton();
    AptAuxEAGLREAL_TimerInit();

#if defined(TIBPROFILER)
    AptProfileStart(1, 1);
#endif

#if defined(XBOX_FASTCAP)
    {
        HRESULT temp = DmStartProfiling("E:\\AptViewerEAGLREAL\\fastcap.txt", 0x02000000); // 32MB seems to be enough, we can change the buffer size later if we need more.
        if(temp != XBDM_NOERR)
        {
            char tempString[512];
            sprintf(tempString, "#########  DmStartProfiling Error %X!!!!!!", temp);
            OutputDebugString(tempString);
        }
    }
#endif

    while (gbRunProgram)
    {
        if (gbPartialGarbageCollectEveryFrame)
        {
            AptPartialGarbageCollection();
        }

        //nPreloadTesing ++ ;
        //if (nPreloadTesing == 10)
        //{
        //    AptCancelPreloadedAnimation("preload1") ;
        //    AptCancelPreloadedAnimation("preload2") ;
        //    AptCancelPreloadedAnimation("number1") ;
        //}
        //if (nPreloadTesing == 20)
        //{
        //    AptCancelPreloadedAnimation("preload1") ;
        //}

        //PrintDebugInformation() ;

        printFreeMem() ;        // can be used for tracking how memory is changing.
        SYNCTASK_run(0);
        _doInput();             // Get Input for User For this Frame.

#if defined (EXTRA_KEY_MESSAGES)
    // $hack add extra 8 key messages to check if Apt slows down or not.
        for (int extrakeymsg=0; extrakeymsg < 8; extrakeymsg ++)
            AptAddToInputQueue((AptInputType)AptInputType_PadL2, AptInputState_Pressed, (AptInputController)(AptInputController_Gamepad0));
//          AptAddToInputQueue(AptInputType_MouseButton0, AptInputState_Pressed, AptInputController_Mouse);
    // end of $hack
#endif
        _checkControllers() ;   // Submit user input to Apt Input Queue.
        _reshapeViewport();     // reshape screen to animation size
        AptAuxEAGLREAL_TimerStartFrame();

        AptAuxEAGLREAL_FinishAsyncLoads();
#if defined (EA_PLATFORM_PSP)
        gbIgnoreRealTime = true ;
#endif
        if(gbIgnoreRealTime)
        {
            AptUpdate( 17 );
        }
        else
        {
            AptUpdate(AptAuxEAGLREAL_TimerGetTicks());
        }

        // If Playing back inputs, only render new frames!!! This cuts test time by about 90%...
        if(!bPlayingInputs || gbNewScreenReadyToBeRendered)
        {
            // Start this frame
            pDisp->BeginFrame();
            // Start using this viewport
            pView->BeginView();
            #if defined(EA_PLATFORM_PLAYSTATION2)
                pDisp->EXTENSION_OBJ.SetRenderMask(0x00000000);
            #endif
            // Clear to background colour
            pView->ClearViewPort();

            #if defined(EA_PLATFORM_PLAYSTATION2)
                pDisp->EXTENSION_OBJ.SetRenderMask(0xff000000);
            #endif

            AptRender(16);  // hardcoded for now

            _drawFPS();
            pView->EndView();
            pDisp->EndFrame();
        }

        AptAuxEAGLREAL_CleanUpOldStrings();

        if (gbSaveScreenShot)
        {
            _saveScreenShot();
        }

        if(memprt)
        {
            MEM_print();
            memprt = 0;
        }

        printFreeMem() ;

#if defined(TIBPROFILER)
        if(gbNewScreenReadyToBeRendered)
        {
            //printf("==-=-=-=-=-=-=-=-=-=- AptProfileprocess ==-=-=-=-=-=-=-=-=-=-  ");
            AptProfileProcess();
            AptProfileStart(1, 1);
        }
#endif

        if(gbNewScreenReadyToBeRendered)
        {
            DebugGetScreenShot(""); // TODO: add better support for screenshots.
        }

        // If a number of frames was given to run, shutdown when that number has been reached.
        if(gnRunFrames != -1)
        {
            if(gnRunFrames==0)
            {
                //printf("\r -- Exiting because we re done playing what was requested.");
                gbRunProgram = false;
            }
            gnRunFrames--;
        }

        if (gnRunFrames == 0 && gbNoShutDown && p != NULL && lastPtr != NULL)
        {
            while (*p == '\n' || *p == '\r')
            {
                *p++ = 0;
            }
            lastPtr = p;
            while (*p != '\n' && *p != '\r' && (p - gpTestFile) < nSize)
            {
                p++;
            }
            if ((p - gpTestFile) == nSize) break;
            *p = 0;
            p++;
            if (gbDoAptResetInBetween)
            {
                AptReset() ;
            }
            gnRunFrames = gnRunFrames1 ;
            AptLoadAnimation(lastPtr, "_level0") ;

        }

        //gnCurrFramesInc ++ ;
        //if ((gnCurrFramesInc % 300) == 0 && gnCurrFramesInc < 301)
        //{
        //    AptLoadAnimation("mainBE.swf", "_level0") ;
        //}
        //else if ((gnCurrFramesInc % 600) == 0 && gnCurrFramesInc < 601)
        //{
        //    AptLoadAnimation("main.swf", "_level0") ;
        //}
        //else if ((gnCurrFramesInc % 900) == 0 && gnCurrFramesInc < 901)
        //{
        //    AptReset() ;
        //    AptLoadAnimation("main.swf", "_level0") ;
        //}

        //if (gnCurrFramesInc > 2200)
        //{
        //    gnCurrFramesInc = 0 ;
        //}


        gbNewScreenReadyToBeRendered = false;
    }

#if defined(XBOX_FASTCAP)

    FILE *pFile = fopen("d:\\dynmem.txt","wt");
    if(pFile)
    {
        char szBuf[256];

        size_t TotalBytesUsedNonGC = gpNonGCPoolManager->GetTotalBytesUsed();
        size_t TotalBytesUsedGC = gpGCPoolManager->GetTotalBytesUsed();


        sprintf(szBuf, "Total GC: %d\n", TotalBytesUsedGC);
        fputs(szBuf, pFile);

        sprintf(szBuf, "Total NonGC: %d\n", TotalBytesUsedNonGC);
        fputs(szBuf, pFile);
        sprintf(szBuf, "Total GC + NonGC: %d", (TotalBytesUsedGC + TotalBytesUsedNonGC));
        fputs(szBuf, pFile);


    }

    fflush(pFile);
    fclose(pFile);
#endif

#if defined(DO_COVERAGE)
    AptUnRegisterExtension(pExtObj);
#endif

    AptShutdown();
    AptAuxEAGLREAL_ShutdownFonts();
    for(int i=0;i<gFonts.nFonts;i++)
    {
        MEM_free(gFonts.pData[i]);
    }

#if defined(XBOX_FASTCAP)
    {
        HRESULT temp = DmStopProfiling();
        if(temp != XBDM_NOERR)
        {
            char tempString[512];
            sprintf(tempString, "#########  DmStopProfiling Error %X!!!!!!", temp);
            OutputDebugString(tempString);
        }
        // Need to wait for it to finish writing the file or it will be incomplete!
        // I personally don't know of an event driven way to do this at the moment...
        Sleep(10000);
    }
#endif

#if defined(TIBPROFILER)
    AptProfileStop();
    AptProfileShutdown();
    MEM_free(pProfilingBuffer);
#endif

    // DC - Added Allocator calls.
    AptAllocatorShutdown();

//  we are suppose to call this function
//  but unfortunately, there seems to be a bug inside this function, so don't call it for now.
//  MEMCLASS_purge(gnFFMemClass);
    MEMCLASS_remove(gnFFMemClass);

#if defined(EA_PLATFORM_XBOX)
    {
        XLaunchNewImage(NULL, 0);
    }
#else
#if defined (EA_PLATFORM_PSP)
    return 0 ;
#endif
#endif
}


void viewerExtracted(char * szFileBase)
{
    //MEM_print() ;
    //int iFreeMemory = MEM_totalunused() ;
    //printf("---------------freemem - %d\n", iFreeMemory);

#if defined( PSP_EMU )
    #if defined( DEBUG_PSP )
    int tticks = 0;
    AptAuxEAGLREAL_DebugPrint("<viewerExtracted tag='%s\.%d'>\n", __FILE__, __LINE__ );
    #endif
#endif

#if !defined( PSP_EMU )
    _loadFontFiles();

    gpFFMemory = (char *)MEM_alloc("FontFusion mess", 120000, MB_LOW);
    gnFFMemClass = MEMCLASS_getfree();
    MEMCLASS_create(gnFFMemClass, "FontFusion", gpFFMemory, 120000);
    //FFInitStub(_allocateMemory,_freeMemory,_reallocateMemory);
    AptEAGLRealAllocator FontFusionAllocator;
    FusionMemObject::sSetAllocator(&FontFusionAllocator);

    AptAuxEAGLREAL_Initialize(pView, pDisp, BASEDIR, &gFonts);

    // DC - Added Allocator calls.
    AptAllocatorInitialize();

#endif


#if defined(USE_APTXMLIMPL)
#if defined (EA_PLATFORM_PLAYSTATION2)
    CDirtyXmlImpl xmlImpl ;
    AptSetXMLImplementor(&xmlImpl);
#endif
#endif
    // added in 19- it sets and unsets APT_OPT_SPRSTACK flag to use scratch pads on ps2.
    //if (!AptOptIsEnabled(APT_OPT_SPRSTACK))
    //{
    //    AptOptEnable(APT_OPT_SPRSTACK) ;
    //}
    //else
    //{
    //    AptOptDisable(APT_OPT_SPRSTACK) ;
    //}
    gszFileBase = szFileBase ;
    AptInitParmsT initParams;
    initParams.iLookupArraySize = 2048 ;
    AptInitialize(&initParams);
    AptLoadAnimation(szFileBase, "_level0");
    AptSetValidFocusButton();
    AptAuxEAGLREAL_TimerInit();

    int nFramesToRun = 700 ;
    if(gnRunFrames != -1)
        nFramesToRun = gnRunFrames ;

    //int gbAptResetDone = 0 ;  // uncomment this if you want to test AptReset.

    while (gbRunProgram)
    {
        printFreeMem() ;        // can be used for tracking how memory is changing.
        SYNCTASK_run(0);
        _doInput();
        _checkControllers() ;
        // reshape screen to animation size
        _reshapeViewport();
        AptAuxEAGLREAL_TimerStartFrame();

        // Start this frame
        pDisp->BeginFrame();

        // Start using this viewport
        pView->BeginView();

#if defined(EA_PLATFORM_PLAYSTATION2)
        pDisp->EXTENSION_OBJ.SetRenderMask(0x00000000);
#endif
        // Clear to background colour
        pView->ClearViewPort();

#if defined(EA_PLATFORM_PLAYSTATION2)
        pDisp->EXTENSION_OBJ.SetRenderMask(0xff000000);
#endif

        AptAuxEAGLREAL_FinishAsyncLoads();
        AptUpdate(AptAuxEAGLREAL_TimerGetTicks());

        AptRender(16);  // hardcoded for now

        _drawFPS();

        pView->EndView();
        pDisp->EndFrame();

        AptAuxEAGLREAL_CleanUpOldStrings();

        if (gbSaveScreenShot)
            _saveScreenShot();
        if(memprt)
        {
            MEM_print();
            memprt = 0;
        }
        printFreeMem() ;
        // added to run test files automatically without user input.
        // it does not test user input and actioscript related to
        // but while test files are executing, we can press input if want to test something specific.
        nFramesToRun-- ;
        if (!nFramesToRun)
        {
            // uncomment this if you want to test AptReset. This will call AptReset and run the same animation
            // again for nFramesToRun frames.
            //if (gbAptResetDone == 0)
            //{
            //    gbAptResetDone = 1 ;
            //    AptReset() ;
            //    AptLoadAnimation(szFileBase, "_level0");
            //    nFramesToRun = 700 ;
            //}
            //else
            {
                gbRunProgram = 0 ;
            }
        }
    }
    AptShutdown();

#if !defined( PSP_EMU )
    AptAuxEAGLREAL_ShutdownFonts();
    for(int i=0;i<gFonts.nFonts;i++)
    {
        MEM_free(gFonts.pData[i]);
    }
    // this was done previously in AptAuxEAGLREAL_ShutdownFonts() but now done here
    // so fonts will get deleted.
    gFonts.nFonts = 0 ;

    // DC - Added Allocator calls.
    AptAllocatorShutdown();

//  we are suppose to call this function
//  but unfortunately, there seems to be a bug inside this function, so don't call it for now.
    //MEMCLASS_purge(gnFFMemClass);
    MEMCLASS_remove(gnFFMemClass);
    // even if MEMCLASS_remove is called, found out that memory is still not freed so
    // manually deleting it.
    MEM_free(gpFFMemory) ;
#endif
    //printf("***************freemem - %d\n", MEM_totalunused());
#if defined(EA_PLATFORM_XBOX)
    {
        XLaunchNewImage(NULL, 0);
    }
#else
    return;
#endif
}




#if defined(EA_PLATFORM_PLAYSTATION2)

// these are generated by something like:
// xxd -i libsd.irx | sed "s/\[\]/\[\] __attribute__((aligned(128)))/" > dump_libsd.irx.h
// there's a dump.bat in test\iop that does all of them.
#include <dump_sio2man.irx.h>
#include <dump_mtapman.irx.h>
#include <dump_padman.irx.h>
#include <dump_libsd.irx.h>
#include <dump_sdrdrv.irx.h>
#include <dump_snddrv.irx.h>
#include <dump_mcman.irx.h>
#include <dump_mcserv.irx.h>

void loadModule(void *pData, int nSize, char *name = 0)
{
    // Memory allocation on IOP
    void *iopaddr = sceSifAllocIopHeap(nSize);
    sceSifDmaData sd;

    printf("loading data for '%s' from %p, size %d\n", name ? name : "<unknown>", pData, nSize);

    // Send module data to IOP
    sd.data = (unsigned int) pData;
    sd.addr = (unsigned int) iopaddr;
    sd.size = nSize;
    sd.mode = 0;
    sceSifSetDma(&sd, 1);

    int retryCnt = 0;
    while(retryCnt++ < 10)
    {
        // Load module buffer
        if (sceSifLoadModuleBuffer(iopaddr, 0, NULL) < 0)
        {
            printf("couldn't load iop module! (%s)\n", name ? name : "<unknown>");
            printf("Is your PS2 flashed to toolchain version " PS2TOOLCHAINVERSION "?\n");
        }
        else
        {
            break;
        }
    }

    // Free unneeded memory in EE/IOP
    sceSifFreeIopHeap(iopaddr);
}

void _skuInit()
{
#if DISK_BOOT
    // reboot IOP, replace default modules, reinitialize
    sceSifInitRpc(0);
    sceCdInit(SCECdINIT);
    sceCdMmode(SCECdCD);
    while ( !sceSifRebootIop("cdrom0:\\IOPRP" PS2TOOLCHAINVERSION ".IMG;1") )
        ; // !!
    while( !sceSifSyncIop() )
        ; // !!
    sceSifInitRpc(0);
    sceCdInit(SCECdINIT);
    sceCdMmode(SCECdCD);
    sceCdDiskReady(0);
    sceFsReset();
#endif  // DISK_BOOT

    sceSifInitRpc(0);
    sceSifInitIopHeap();

    // simple way of not having go to disk for iop files
#define LOAD_MODULE(_) loadModule(_, _##_len, #_)
    LOAD_MODULE(sio2man_irx);
    LOAD_MODULE(mtapman_irx);
    LOAD_MODULE(padman_irx);
    LOAD_MODULE(libsd_irx);
    LOAD_MODULE(sdrdrv_irx);
    LOAD_MODULE(snddrv_irx);
    LOAD_MODULE(mcman_irx);
    LOAD_MODULE(mcserv_irx);
#undef LOAD_MODULE
}

#elif defined(EA_PLATFORM_PSP)
//
//static char               disp_list[0x40000];
// matrix_stack must match the values specified in sceGumSetMatrixStack (below)
//static scePspFMATRIX4 matrix_stack[8+8+32+8];
//
// ----------------------------------------------------------------------------
void _skuInit()
{




#if defined( DEBUG_PSP )
    AptAuxEAGLREAL_DebugPrint("<_skuInit/>\n");
#endif

// none of this is needed!
#if 0
    // function initializes emulator system; must be called before ANY other library function
    sceEmuInit();

    // function initializes graphics library; must be called before ANY other libgu function
    sceGuInit();

    // function sets the display list.
    //  SCEGU_IMMEDIATE: creates the display list while also drawing
    //      sceGuStart( SCEGU_IMMEDIATE, disp_list, sizeof(disp_list) );
    //  SCEGU_CALL: only the display list is created; the created list performs drawing using "sceGuCallList()"
    sceGuStart( SCEGU_CALL, disp_list, sizeof(disp_list) );

    // set drawing buffer: (SCEGU_PF5551=pixel format PF5551), (SCEGU_VRAM_TOP=0), (SCEGU_VRAM_WIDTH=512)
    //sceGuDrawBuffer( SCEGU_PF5551, SCEGU_VRAM_BP_0, SCEGU_VRAM_WIDTH );
    sceGuDrawBuffer( SCEGU_PF8888, SCEGU_VRAM_BP_0, SCEGU_VRAM_WIDTH );

    // set display buffer: (SCEGU_SCR_WIDTH=480), (SCEGU_SCR_HEIGHT=272), (SCEGU_VRAM_BP_1=SCEGU_VRAM_TOP+SCEGU_VRAM_BUFSIZE), (SCEGU_VRAM_WIDTH=512)
    sceGuDispBuffer( SCEGU_SCR_WIDTH, SCEGU_SCR_HEIGHT, SCEGU_VRAM_BP_1, SCEGU_VRAM_WIDTH );

    // set depth buffer: (SCEGU_VRAM_BP_2=SCEGU_VRAM_TOP+(SCEGU_VRAM_BUFSIZE*2)), (SCEGU_VRAM_WIDTH=512)
    sceGuDepthBuffer( SCEGU_VRAM_BP_2, SCEGU_VRAM_WIDTH );

    // sets the offset of the vertex coordinates with the screen coordinate system
    //  used when converting coordinates from the screen coordinate system to the drawing coordinate system
    //  (SCEGU_SCR_OFFSETX=((4096-SCEGU_SCR_WIDTH)/2), SCEGU_SCR_OFFSETY=((4096-SCEGU_SCR_HEIGHT)/2)
    sceGuOffset( SCEGU_SCR_OFFSETX, SCEGU_SCR_OFFSETY );

    // sets the viewport with the screen coordinate system;
    //  it is used to convert from the clip coordinate system to the screen coordinate system;
    //  the origin of the clip coord system becomes the X and Y position of the screen coord system
    //  the width and height of the X and Y coordinates of the clip coord system become the width
    //  and height of the X & Y coordinates of the screen coord system
    //  sceGuViewport( X, Y, SCEGU_SCR_WIDTH, SCEGU_SCR_HEIGHT );
    //sceGuViewport( 1024, 1024, SCEGU_SCR_WIDTH, SCEGU_SCR_HEIGHT );
    sceGuViewport( 2048, 2048, SCEGU_SCR_WIDTH, SCEGU_SCR_HEIGHT );

    // sets the depth range
    //  sceGuDepthRange( minimum, maximum );
    //sceGuDepthRange(0, 65535);
    //sceGuDepthRange( 0x1000, 0xe000 );
    //sceGuDepthRange( 10000, 50000 );
    sceGuDepthRange(0, 65535);

    // set texture mode
    //sceGuTexMode(SCEGU_PF5551, 0, SCEGU_SINGLE_CLUT, SCEGU_TEXBUF_NORMAL);
    //sceGuTexMode(SCEGU_PF4444, 0, SCEGU_SINGLE_CLUT, SCEGU_TEXBUF_NORMAL);
    sceGuTexMode(SCEGU_PF8888, 0, SCEGU_SINGLE_CLUT, SCEGU_TEXBUF_NORMAL);

    // disables the feature specified by the function parameter; everything is disabled by default
    //  see documentation for full list of functional capabilities that can be dis/enabled (libgu reference, p.30)
    //  see function "sceGuEnable()" for further info (libgu reference, p.27)
    //sceGuDisable( SCEGU_BLEND );

    // sets scissoring area within the screen coord system; area must be within 480x272
    sceGuScissor( 0, 0, SCEGU_SCR_WIDTH, SCEGU_SCR_HEIGHT );

    // enable scissoring functionality (libgu reference, pp.27,30)
    sceGuEnable( SCEGU_SCISSOR_TEST );

    // sets depth test parameter
    //  SCEGU_NEVER     -> draw no pixels
    //  SCEGU_ALWAYS    -> draw all pixels
    //  SCEGU_NOTEQUAL  -> draw pixel when pixel depth <> depth buffer value
    //  SCEGU_EQUAL     -> draw pixel when pixel depth == depth buffer value
    //  SCEGU_GREATER   -> draw pixel when pixel depth >  depth buffer value
    //  SCEGU_LESS      -> draw pixel when pixel depth <  depth buffer value
    //  SCEGU_LEQUAL    -> draw pixel when pixel depth <= depth buffer value
    //  SCEGU_GEQUAL    -> draw pixel when pixel depth >= depth buffer value
    //sceGuDepthFunc( SCEGU_ALWAYS );
    sceGuDepthFunc( SCEGU_GREATER );

    // enable depth test functionality
    sceGuEnable( SCEGU_DEPTH_TEST );

    // specify front face for back face culling
    //  SCEGU_CW        -> align vertices clockwise for back face culling
    //  SCEGU_CCW       -> align vertices counter-clockwise for back face culling
    sceGuFrontFace( SCEGU_CW );

    // enable: reverse patch's normal direction
    //sceGuEnable( SCEGU_NORMAL_REVERSE_PATCH );

    // disable texture mapping
    //sceGuDisable( SCEGU_TEXTURE );

    // set shading mode:
    //  SCEGU_FLAT      -> flat shading
    //  SCEGU_SMNOOTH   -> Gouraurd shading
    sceGuShadeModel( SCEGU_SMOOTH );

    // enable: dither and lighting
    sceGuEnable( SCEGU_DITHER );
    sceGuEnable( SCEGU_LIGHTING );
    sceGuEnable( SCEGU_TEXTURE );

    // set exponent of the specular reflection light
    //sceGuSpecular( 75.0f );
    sceGuSpecular( 10.0f );

    // sets buffer to matrix stack used by libgum
    //  specifies how many stacks are to be used for each matrix (projection, view, world, texture)
    //  must match the values specified in "scePspFMATRIX4 matrix_stack[8+8+32+8];" (above)
    sceGumSetMatrixStack( matrix_stack, 8, 8, 32, 8 );

    // set color value to use when clearing the drawing buffer (alpha is ignored)
    //sceGuClearColor( 0xff645a1e );
    //sceGuClearColor( 0xff000000 );
    sceGuClearColor( 0xff000000 );

    // sets depth value for clearing the depth buffer
    sceGuClearDepth( 0 );

    // sets stencil value to clear
    sceGuClearStencil( 0 );

    // sets current matrix stack to...
    //  SCEGU_MATRIX_WORLD
    //sceGumMatrixMode( SCEGU_MATRIX_WORLD );

    // ends the display list
    //  in mode SCEGU_IMMEDIATE: adds a cmd that generates a drawing end interrupt
    //  in mode SCEGU_CALL     : adds a return cmd
    sceGuFinish();

    // detects: the end of drawing, signal interruption, end of read, and completion of a display list transfer
    //  sceGuSync( mode, block );
    //      where mode :=
    //                  SCEGU_SYNC_FINISH   : finish drawing
    //                  SCEGU_SYNC_SIGNAL   : signal interrupt
    //                  SCEGU_SYNC_END      : finish read
    //                  SCEGU_SYNC_LIST     : display list transfer completed
    //      and block :=
    //                  SCEGU_SYNC_WAIT     : block mode
    //                  SCEGU_SYNC_NOWAIT   : non-block mode
    //  in block mode, the block is maintained until the end.
    //  in non-block mode, the current status is returned (0=complete;1=in progress)
    sceGuSync( SCEGU_SYNC_FINISH, SCEGU_SYNC_WAIT );
#endif

}

// ----------------------------------------------------------------------------



#elif defined(EA_PLATFORM_XBOX)
void _skuInit()
{
    XInitDevices(0, NULL);
}
#elif defined(EA_PLATFORM_WINDOWS)
void _skuInit()
{
}
#elif defined(EA_PLATFORM_GAMECUBE)
void _skuInit()
{
    OSInit();
    DVDInit();
    VIInit();
    PADInit();
}
#endif

// ----------------------------------------------------------------------------
// --- workaround suggested by EAC
// ---  to deal with static linking of global instances of render methods
// ---  also,
/*
    There's a 'forcelink.txt' file that's generated when the render methods
    are built.  This file contains link commands to force these symbols to be
    ignored during the link.  Use it, or use this.
*/
// ----------------------------------------------------------------------------
#if defined( PSP_EMU )
    extern EAGL::RenderMethod rm_GouraudApt;
    extern EAGL::RenderMethod rm_TextureApt;
    ///extern EAGL::RenderMethod rm_Texture;
#endif
// ----------------------------------------------------------------------------


int main(int argc, char **argv)
{
    _skuInit();


#if defined(EA_PLATFORM_PSP)
    ViewerPrint("calling startmainthread\n") ;
    StartMainThread( argc, argv );
    return 0;
#endif

    // Must do this before anything else. Hey, we need memory!
    MEM_initsize(MEMSIZE);

#if defined(EA_PLATFORM_XBOX)
    // Watch out, XBox does not use command lines like everyone else...
    // This is a VERY basic command line parser.
    LAUNCH_DATA launchData;
    DWORD       dwLaunchDataType;
    XGetLaunchInfo(&dwLaunchDataType, &launchData); // Gets command line string

    char * defaultCommandLine[] = {"AptViewerEAGLREAL.xbe"};
    argc = 1;
    argv = defaultCommandLine;

    if (dwLaunchDataType == LDT_FROM_DEBUGGER_CMDLINE)
    {
        char* szCmdLine = ((LD_FROM_DEBUGGER_CMDLINE *)&launchData)->szCmdLine;
        int nStrlen     = strlen(szCmdLine);

        argc   = 2;
        for(int i = 0; i < nStrlen; i++)
        {
            if(szCmdLine[i] == ' ')
                argc++;
        }
        argv = new char * [argc];

        argv[0] = defaultCommandLine[0]; // argv[0] is the application name.
        argv[1] = &szCmdLine[0];         // argv[1] is the base of the argument string.
        for(int i = 0, arg=2; i < nStrlen; i++)
        {
            if(szCmdLine[i] == ' ')
            {
                argv[arg++] = &szCmdLine[i+1];
                szCmdLine[i] = '\0';
            }
        }
    }
#endif

#if defined(EA_PLATFORM_PSP)
    // --- PSP emulator does not support a command line argument, so use file as work-around
    #if defined( DEBUG_PSP )
        AptAuxEAGLREAL_DebugPrint("<info msg='viewerMain file: %s'/>\n", APTVIEWERPSP_DAT );
    #endif

    //viewerMain( APTVIEWERPSP_LST );
#else
    if(argc < 2)
    {
        printf("No Arguments found on command line.");
        printf("Viewer Requires file name to open!");
        return -1;
    }
    viewerMain(argc, argv);
#endif

#if defined(EA_PLATFORM_XBOX)
    // Delete memory we allocated for argument simulation.
    delete [] argv;
#endif

    return 0;
}
#if defined (EA_PLATFORM_WINDOWS)
int WINAPI WinMain(
  HINSTANCE hInstance,      // handle to current instance
  HINSTANCE hPrevInstance,  // handle to previous instance
  LPSTR lpCmdLine,          // command line
  int nCmdShow              // show state
)
{
    main(__argc,__argv);
}

#endif




#ifdef PSP
void StartMainThread(int argc, char** argv)
{
    SceUID main_thread = 0;

    //scratchSRAM removed with SN 0.7
    main_thread = sceKernelCreateThread( "main", viewerMain, SCE_KERNEL_USER_LOWEST_PRIORITY
        , 256*100, /*SCE_KERNEL_TH_USE_ScratchSRAM|*/SCE_KERNEL_TH_USE_VFPU, NULL );

    if ( ((unsigned int)main_thread) == SCE_KERNEL_ERROR_NO_MEMORY )
    {
        ViewerPrint( "<< Viewer >> no memory\n" );
    }
    else if ( ((unsigned int)main_thread) == SCE_KERNEL_ERROR_ILLEGAL_ATTR )
    {
        ViewerPrint( "<< Viewer >> illegal attr\n" );
    }
    else if ( ((unsigned int)main_thread) == SCE_KERNEL_ERROR_ILLEGAL_STACK_SIZE )
    {
        ViewerPrint( "<< Viewer >> illegal stack size\n" );
    }
    else if ( ((unsigned int)main_thread) == SCE_KERNEL_ERROR_ILLEGAL_CONTEXT )
    {
        ViewerPrint( "<< Viewer >> illegal context\n" );
    }
    else if ( ((unsigned int)main_thread) == SCE_KERNEL_ERROR_ILLEGAL_PRIORITY )
    {
        ViewerPrint( "<< Viewer >> illegal priority\n" );
    }
    else if ( ((unsigned int)main_thread) == SCE_KERNEL_ERROR_ILLEGAL_ENTRY )
    {
        ViewerPrint( "<< Viewer >> illegal entry\n" );
    }
    else if ( ((unsigned int)main_thread) == SCE_KERNEL_ERROR_SPAD_INUSE )
    {
        ViewerPrint( "<< Viewer >> spad inuse\n" );
    }
    else
    {
        int err = sceKernelStartThread( main_thread, argc * sizeof(char*), argv );

        if ( ((unsigned int)err) == SCE_KERNEL_ERROR_UNKNOWN_THID )
        {
            ViewerPrint( "<< Viewer >> unknown thid\n" );
        }
        else if ( ((unsigned int)err) == SCE_KERNEL_ERROR_ILLEGAL_THID )
        {
            ViewerPrint( "<< Viewer >> illegal thid\n" );
        }
        else if ( ((unsigned int)err) == SCE_KERNEL_ERROR_ILLEGAL_CONTEXT )
        {
            ViewerPrint( "<< Viewer >> illegal context\n" );
        }
        else if ( ((unsigned int)err) == SCE_KERNEL_ERROR_NOT_DORMANT )
        {
            ViewerPrint( "<< Viewer >> not dormant\n" );
        }
        else
        {
            ViewerPrint( "<< Viewer >> initial thread going to sleep, night night!\n" );

            sceKernelSleepThread();
        }
    }
}

const char* PRX_KMODULES[] =    // located in /prx/kmodule/
{
    "sc_sascore",

        /*
        "audiocodec",
        "ifhandle",
        "memab",
        "mpegbase",
        "pspnet_adhoc_auth",
        "videocodec",
        */
        "" // dummy terminator
};

const int NUM_PRX_KMODULES = sizeof( PRX_KMODULES ) / sizeof( PRX_KMODULES[ 0 ] ) - 1;

const char* PRX_MODULES[] =     // located in /prx/module/
{
    /*
    "libatrac3plus",
    "mpeg",
    "pspnet",
    "pspnet_inet",
    "pspnet_apctl",
    "pspnet_adhoc",
    "pspnet_adhocctl",
    */
    "" // dummy terminator
};

const int NUM_PRX_MODULES = sizeof( PRX_MODULES ) / sizeof( PRX_MODULES[ 0 ] ) - 1;

static SceUID load_module(const char *filename, SceKernelLMOption *lm_opt, SceKernelSMOption *sm_opt)
{
    SceUID mid;
    int ret;

    /* load module */
    mid = sceKernelLoadModule(filename, 0, lm_opt);
    if (mid < 0) {
        printf("fatal error : cannot load %s 0x%08x\n", filename, mid);
        return mid;
    }

    /* start module */
    if ((ret = sceKernelStartModule(mid, 0, NULL, NULL, sm_opt)) < 0) {
        printf("fatal error : cannot start %s 0x%08x\n", filename, ret);
        return ret;
    }

    return mid;
}

/* unload module */
static int unload_module(SceUID mid)
{
    int ret;

    /* stop module */
    if ((ret = sceKernelStopModule(mid, 0, NULL, NULL, NULL)) < 0) {
        printf("fatal error : cannot stop module 0x%08x\n", ret);
        return ret;
    }

    /* unload moudle */
    if ((ret = sceKernelUnloadModule(mid)) < 0) {
        printf("fatal error : cannot unload module 0x%08x\n", ret);
        return ret;
    }

    return 0;
}

void LoadPrxModules()
{
    int i = 0;
    char filepath[ 256 ];

    SceKernelLMOption mode;
    mode.size = sizeof( mode );
    mode.position = SCE_KERNEL_LMWO_POS_Low;
    mode.access   = SCE_KERNEL_LMWO_ACCESS_Noseek;

    const char* FILEIO_DRIVE = "host0:";

    //
    // load kernel modules
    //
    mode.mpidtext = mode.mpiddata = SCE_KERNEL_PRIMARY_KERNEL_PARTITION;
    for( i = 0; i < NUM_PRX_KMODULES; i++ )
    {
        //sprintf( filepath, "%sprx/kmodule/%s.prx", FILEIO_DRIVE, PRX_KMODULES[ i ] );
        sprintf( filepath, "%s%s.prx", FILEIO_DRIVE, PRX_KMODULES[ i ] );
        load_module( filepath, &mode, NULL );
    }

    //
    // load non-kernel modules
    //
    mode.mpidtext = mode.mpiddata = SCE_KERNEL_PRIMARY_USER_PARTITION;
    for( i = 0; i < NUM_PRX_MODULES; i++ )
    {
        //sprintf( filepath, "%sprx/module/%s.prx", FILEIO_DRIVE, PRX_MODULES[ i ] );
        sprintf( filepath, "%s%s.prx", FILEIO_DRIVE, PRX_MODULES[ i ] );
        load_module( filepath, &mode, NULL );
    }
}

/*
int main(int argc, char** argv)
{
    StartMainThread( argc, argv );
    return 0;
}
*/

//debug print
void ViewerPrint(const char *szFormat, ...)
{
    va_list args;
    char szBuf[4096];

    va_start(args, szFormat);
    vsprintf(szBuf, szFormat, args);
    va_end(args);
#if defined(XBOX) || defined(PC)
    OutputDebugString(szBuf);
#else
    printf("%s", szBuf);

#endif
}


#endif //PSP
