/*H************************************************************************************************/
/*!

\File
UnitMain.cpp

\Description
An implementation of main() for our unit tester, This is a standalone application.

\Notes

\Copyright
(c) 2003 Electronic Arts Inc.

\Version
1.0        04/05/06 (DC) Comment Creation

*/
/************************************************************************************************H*/

#if defined(UNIT_TESTING)
#include "Apt.h"
#include "_Apt.h"

#include "unit/unit.h"

#if 1 // define(PC)

#include "stdio.h"
#include <windows.h>

int main( int argc, char** argv );
// Windows entry function.
int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpCmdLine,int nCmdShow)
{
    main(__argc,__argv);
}

#endif



/*F************************************************************************************************/

/************************************************************************************************F*/
static void LoadAnimation(const char *szBaseName, AptFilePtr pAsyncLoadContext)
{

}
/*F************************************************************************************************/

/************************************************************************************************F*/
static void LoadAnimationCompleted(const char *szBaseName, const char * szTargetName)
{
}
/*F************************************************************************************************/

/************************************************************************************************F*/
static void FreeAnimation(void *pUserHandle)
{

}
/*F************************************************************************************************/

/************************************************************************************************F*/
static void FreeConstantTable(void *pConstTable)
{
}
/*F************************************************************************************************/

/************************************************************************************************F*/
void AssertFail(const char *szExpression, const char *szFile, unsigned int nLine)
{
    printf("ASSERT FAILED '%s' at %s:%d\n", szExpression, szFile, nLine );
}
/*F************************************************************************************************/

/************************************************************************************************F*/
static void command(const char *szCommand, const char *szParams)
{
    char szBuf[4096];
    sprintf(szBuf, "external command requested: %s('%s')\n", szCommand, szParams);
    printf(szBuf);
}
/*F************************************************************************************************/

/************************************************************************************************F*/
static char *getVariableCrap(const char *szVar)
{
    return "getVariableCrap";
}
/*F************************************************************************************************/

/************************************************************************************************F*/
static char *loadVariablesNULL()
{
    return "Dummy String for null";
}

/*F************************************************************************************************/

/************************************************************************************************F*/
static char *loadVariables(const char *szCommand)
{
    return "Dummy String";
}
/*F************************************************************************************************/

/************************************************************************************************F*/
static AptAssetSound loadSound(AptAnimationUserData userData, int nID)
{
    return (AptAssetSound)NULL;
}
/*F************************************************************************************************/

/************************************************************************************************F*/
static void freeSound(AptAssetSound sound)
{

}
/*F************************************************************************************************/

/************************************************************************************************F*/
static void startSound(AptAssetSound sound, const char * szName)
{
}
/*F************************************************************************************************/

/************************************************************************************************F*/
static void startSoundStream(AptAnimationUserData userData, int nID)
{

}
/*F************************************************************************************************/

/************************************************************************************************F*/
static void SetBackgroundColour(unsigned int nColour)
{

}
/*F************************************************************************************************/

/************************************************************************************************F*/
static void SetVertexMatrix(AptMatrix *pMatrix)
{

}
/*F************************************************************************************************/

/************************************************************************************************F*/
static void *loadTexture(AptAnimationUserData userData, int nID)
{
    return NULL;
}
/*F************************************************************************************************/

/************************************************************************************************F*/
static void freeTexture(AptAssetTexture texture)
{
}
/*F************************************************************************************************/

/************************************************************************************************F*/
static void bindTexture(AptAnimationUserData userData, int nID, AptAssetTexture texture)
{

}
/*F************************************************************************************************/

/************************************************************************************************F*/
static void SetColourTransform(AptCXForm *pCXForm)
{

}
/*F************************************************************************************************/

/************************************************************************************************F*/
static AptAssetRenderingUnit loadRenderingUnit(AptAnimationUserData userData, int nID)
{
    return (AptAssetRenderingUnit) NULL;
}
/*F************************************************************************************************/

/************************************************************************************************F*/
static void freeRenderingUnit(AptAssetRenderingUnit renderingUnit)
{
    assert(renderingUnit);
}

/*F************************************************************************************************/

/************************************************************************************************F*/
static void drawRenderingUnit(AptAssetRenderingUnit renderingUnit, AptMaskRenderOperation eMaskOperation)
{

}

/*F************************************************************************************************/

/************************************************************************************************F*/
static void customControlRender(char *szType, char *szTarget, AptAssetRenderingUnit renderingUnit, const char * szCustomProperties)
{

}
/*F************************************************************************************************/

/************************************************************************************************F*/
void AptAuxEAGLREAL_GetRealTimeClock(AptSysClock *pClock, bool bLocal)
{
#if defined(PS2)
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
#if defined(NGC)
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
#if defined(XBOX)
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
#if defined(PC)
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

/************************************************************************************************F*/
static AptAssetString allocateStringFF(AptAllocateStringParameters *pASP)
{
    return (AptAssetString)NULL;
}
/*F************************************************************************************************/

/************************************************************************************************F*/
static void deallocateStringFF(AptAssetString string, unsigned int eFlags)
{

}
/*F************************************************************************************************/

/************************************************************************************************F*/
static void drawStringFF(AptAssetString string, AptMaskRenderOperation eMaskOperation)
{
}
/*F************************************************************************************************/

/************************************************************************************************F*/
void sendVariables(const char * szUrl, const char * szTarget, const char *szMethod, const char *szProp, int bIsSendAndLoad)
{
    gAptFuncs.pfnDebugPrint("sendVariables : '%s' at target '%s ----%s ----%s '\n", szProp, szTarget, szMethod, szUrl);
    return ;
}
/*F************************************************************************************************/

/************************************************************************************************F*/
int  getBytesTotal(const char * szFileName, AptGetBytesEnum eGetBytes)
{
    return 0;
}
/*F************************************************************************************************/

/************************************************************************************************F*/
int  getBytesLoaded(const char * szFileName, AptGetBytesEnum eGetBytes)
{
    return 0;
}
/*F************************************************************************************************/

/************************************************************************************************F*/
static void DebugAddSavedInput(AptSavedInputRecord *pRecord, int nSize)
{
}
/*F************************************************************************************************/

/************************************************************************************************F*/
static void DebugSetScreenGrabPending(const char *szScreenGrabName)
{
}
/*F************************************************************************************************/

/************************************************************************************************F*/
int nAllocCount = 0;
#if defined(APT_ALLOCATION_TRACKING)
void *MemAlloc(size_t nSize, const char *szFile, int nLine)
#else
void *MemAlloc(size_t nSize)
#endif
{
#if defined(XBOX)
    nSize = MAX(nSize, sizeof(SHAPE));
    char szBuf[256];
#if defined(APT_ALLOCATION_TRACKING)
    const char *p = strrchr(szFile, '\\');
    if (p) p++;
    else p = szFile;
    sprintf(szBuf, "Apt%d:%s:%d", nAllocCount++, szFile, nLine);
#else
    sprintf(szBuf, "Apt%d", nAllocCount++);
#endif
    return MEM_allocalign(szBuf, nSize, ALIGNSIZE, sizeof(SHAPE), MB_LOW);
#elif defined(NGC)
    return MEM_allocalign("Apt", nSize + sizeof(SHAPE), 32, sizeof(SHAPE), MB_LOW);
#else
    return malloc( nSize);
#endif
}
/*F************************************************************************************************/

/************************************************************************************************F*/
void MemFree(void *pData)
{
    free(pData);
}
/*F************************************************************************************************/

/************************************************************************************************F*/
static void DebugPrint(const char *szFormat, ...)
{
    va_list args;
    char szBuf[4096];

    va_start(args, szFormat);
    vsprintf(szBuf, szFormat, args);
    va_end(args);
    printf("%s", szBuf);
#if defined(XBOX) || defined(PC)
    OutputDebugString(szBuf);
#endif
}
/*F************************************************************************************************/

/************************************************************************************************F*/
int main( int argc, char** argv )
{
    // Your task specific implementation code goes here. If your
    // unit test init code require command line arguments, be sure
    // to remove them from argv before calling DoTests(). Also be
    // sure to preserve any Unit related arguments

    gAptFuncs.pfnMemAlloc      = MemAlloc;
    gAptFuncs.pfnMemFree       = MemFree;
    gAptFuncs.pfnLoadAnimation = LoadAnimation;
    gAptFuncs.pfnFreeAnimation = FreeAnimation;
    gAptFuncs.pfnFreeConstantTable = FreeConstantTable;
    gAptFuncs.pfnAssertFail    = AssertFail;
    gAptFuncs.pfnDebugPrint    = DebugPrint;
    gAptFuncs.pfnDebugSetScreenGrabPending = DebugSetScreenGrabPending;
    gAptFuncs.pfnDebugAddSavedInput = DebugAddSavedInput;
    gAptFuncs.pfnCommand            = command;
    gAptFuncs.pfnSetExternVariable  = command;
    gAptFuncs.pfnGetExternVariable  = getVariableCrap;
    gAptFuncs.pfnLoadVariables      = loadVariables;

    gAptFuncs.pfnLoadSound          = loadSound;
    gAptFuncs.pfnFreeSound          = freeSound;
    gAptFuncs.pfnStartSound         = startSound;
    gAptFuncs.pfnStartSoundStream   = startSoundStream;
    gAptFuncs.pfnLoadTexture        = loadTexture;
    gAptFuncs.pfnFreeTexture        = freeTexture;
    gAptFuncs.pfnBindTexture        = bindTexture;
    gAptFuncs.pfnSetColourTransform = SetColourTransform;
    gAptFuncs.pfnSetVertexMatrix    = SetVertexMatrix;
    gAptFuncs.pfnSetBackgroundColour= SetBackgroundColour;
    gAptFuncs.pfnLoadRenderingUnit  = loadRenderingUnit;
    gAptFuncs.pfnFreeRenderingUnit  = freeRenderingUnit;
    gAptFuncs.pfnDrawRenderingUnit  = drawRenderingUnit;
    gAptFuncs.pfnCustomControlRender= customControlRender;
    gAptFuncs.pfnPointHitTest       = NULL;
    gAptFuncs.pfnGetRealTimeClock   = AptAuxEAGLREAL_GetRealTimeClock;
    gAptFuncs.pfnAllocateString     = allocateStringFF;
    gAptFuncs.pfnDeallocateString   = deallocateStringFF;
    gAptFuncs.pfnDrawString         = drawStringFF;
    gAptFuncs.pfnSendVariables      = sendVariables;
    gAptFuncs.pfnLoadVariablesNULL  = loadVariablesNULL;
    gAptFuncs.pfnGetBytesTotal      = getBytesTotal ;
    gAptFuncs.pfnGetBytesLoaded     = getBytesLoaded ;
    gAptFuncs.pfnLoadAnimationCompleted = LoadAnimationCompleted ;

    AptInitialize();

    // Now run the tests
    int iResults = EATest::DoTests( argc, argv );

    return iResults;
}

// A set of unit tests for our unit tester
#if 0 // (This is a sample unit test implementation.)

#if UNIT_TESTING
#include "unit/unit.h"

class TestUnitSuiteObject_Name : public EATest::TestSuite
{
public:
    virtual void Initialize()
    {
        // Initialize all member variables to known state here.

        // Setup the test suite name.
        SuiteName("[Insert Test Suite Name, i.e. AptString Tester]");

        // Tests can be registered with the EATEST_REGISTER macro...
        // Register as many as you want.

        // EATEST_REGISTER("[InsertTestName]", "[Insert Test Description]", TestUnitSuiteObject_Name, UnitTestFunction_Name);

        // EATEST_REGISTER_SETUP_TEARDOWN registers a testcase with test-specific setup &
        // teardown calls. The test-specific setup call is invoked immediately after the
        // suite-wide Setup() call, while the test-specific teardown call is invoked
        // immediately before the suite-wide Teardown() call.

        // EATEST_REGISTER_SETUP_TEARDOWN("[InsertTestName]", "[Insert Test Description]", TestUnitSuiteObject_Name, UnitTestFunction_Name, TestSpecificSetup_Name, TestSpecificTeardown_Name);
    }

    // Global TestSuite Setup Function, Typically used to allocate memory and or setup vars
    // needed for test cases.
    //void Setup()
    //{

    //}


    // Global TestSuite TearDown Function, Typically used to de-allocate memory and or destroy vars
    // used in test cases.
    //void Teardown()
    //{

    //}

    // Create Unit Test Functions. No need to name them anything specific.
    //void UnitTestFunction_Name(void)
    //{
    //    EATESTAssert(SetupTeardownTest == 1, "Unexpected count of setups");
    //}

    // Example Test Specific Setup function
    //void TestSpecificSetup(void)
    //{

    //}

    // Example Test Specific Teardown function
    //void TestSpecificTeardown(void)
    //{

    //}

} TestUnitTestsSingleton;

#endif // #if UNIT_TESTING

#endif // #if 0 (This is example code only)

#endif // #if UNIT_TESTING
