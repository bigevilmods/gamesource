// (c) Electronic Arts. All Rights Reserved.
#include "stdarg.h"
#include "testconfig.h"
#include <math.h>
#include "EAStdC/EAStopwatch.h"

#ifdef _MSC_VER
    #pragma warning(disable:4312) // warning C4312: 'type cast' : conversion from 'int' to 'int *' of greater size
#endif

#if defined __SPU__
    #include <spu_printf.h>
#endif

namespace rw {
namespace math {
namespace vpu {

    class VecFloat;

}}}


#include "unittest.h"

UnitTestOutputManager& UnitTest::mOutputManager = UnitTestOutputManager::OpenInstance();

void UnitTest::ShowFinalResult()
{
    mOutputManager.OutputTotals(sTotalTestCount, sTotalTestPassed);
}

bool UnitTest::PassedAllTests()
{
    return UnitTest::sTotalTestPassed == UnitTest::sTotalTestCount;
}

UnitTest::UnitTest() :
    mTimerStart(0),
    mTimerStop(0),
    mLoopCount(0),
    mTickPerSecond(0),
    mTestCount(0),
    mTestPassed(0)
{
    mTickPerSecond = EA::StdC::Stopwatch::GetCPUFrequency();
}

UnitTest::~UnitTest()
{
    ShowResult();
}

void UnitTest::ShowResult()
{
    mOutputManager.OutputClassTotals(mClassName, mTestCount, mTestPassed);

    sTotalTestCount += mTestCount;
    sTotalTestPassed += mTestPassed;
}

void UnitTest::ValidateTest(bool valid, const char* filename, const int linenum)
{
    mTestCount++;
    if(valid)
        mTestPassed++;

    mOutputManager.OutputTestResult(valid, mFunctionName, filename, linenum);
}

void UnitTest::StartTimer(unsigned int loopCount)
{
    mLoopCount = loopCount;
    mTimerStart = EA::StdC::Stopwatch::GetCPUCycle();
}

void UnitTest::StopTimer()
{
    mTimerStop = EA::StdC::Stopwatch::GetCPUCycle();

    uint64_t tickCount = mTimerStop - mTimerStart;

    double timeValue = (double)tickCount / (double)mTickPerSecond / (double)mLoopCount * 1000000;

    mOutputManager.OutputBenchmarkResult(mTestIdentifier, mFunctionName, timeValue);
}

bool UnitTest::IsEqualWithEpsilon(float a, float b, float epsilon)
{
    if (fabs(a - b) <= epsilon)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool UnitTest::IsEqualWithEpsilon(double a, double b, double epsilon)
{
    if (fabs(a - b) <= epsilon)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool UnitTest::IsEqualWithinPercent(float expected, float actual, float percent)
{
    if (expected == 0.0f) 
        return actual == 0.0f;

    if (fabs(actual - expected)/expected <= percent / 100.0f)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool UnitTest::IsEqualWithinPercent(double expected, double actual, float percent)
{
    if (expected == 0.0) 
        return actual == 0.0;

    if (fabs(actual - expected)/expected <= percent / 100.0f)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void* UnitTest::operator new( size_t size )
{
    void* mem = malloc(size + 16 + 16);

    // get aligned address
    intptr_t alignMem = ((intptr_t)mem + 16) & ~0xF;
    // store the original ptr value
    *(intptr_t*)alignMem = (intptr_t)mem;

    void* object = (void*)(alignMem + 16);

    return object;
}

void UnitTest::operator delete( void* object )
{
    // get the original aligned address
    intptr_t alignMem = (intptr_t)object - 16;

    // get the original ptr value
    void* mem = (void*)(*(intptr_t*)alignMem);
    free(mem);
}

#ifdef _MSC_VER
#pragma warning(default:4312) // warning C4312: 'type cast' : conversion from 'int' to 'int *' of greater size
#endif


void Strcpy( char *dest, const char *src)
{
    do
    {
        *(dest++) = *(src++);
    } while (*src);
    *dest = *src;
}

void UnitTest::SetClassName(const char* className)
{
    Strcpy( mClassName, className );

    mOutputManager.OutputClassName(mClassName);
}

void UnitTest::SetTestIdentifier(const char* identifier)
{
    Strcpy( mTestIdentifier, identifier );
}

#if !defined __SPU__
void UnitTest::SetFunctionName(const char* functionNameFormat, ...)
{
    va_list ap;
    va_start(ap, functionNameFormat);
    vsprintf(mFunctionName, functionNameFormat, ap);
    va_end(ap);
}
#endif

uint32_t    UnitTest::sTotalTestCount = 0;
uint32_t    UnitTest::sTotalTestPassed = 0;

#if defined EA_PLATFORM_XENON

#include "xtl.h"
#include "tracerecording.h"
#pragma comment( lib, "tracerecording.lib" )

// If an error is detected while trying to record trace results, this flag will be set to true.
BOOL gAbortTracing = FALSE;

void StartTraceCapture(char *traceName)
{
    if( !gAbortTracing )
    {
        // Start recording trace info
        char buffer[512];
        sprintf( buffer, "d:\\%s_trace.bin", traceName );
        BOOL recordTraceResults = XTraceStartRecording( buffer );
        if( !recordTraceResults )
        {
            DWORD errorCode = GetLastError();
            sprintf( buffer, "Error: Unable to begin recording of trace results (error %d).\n", errorCode);
            UnitTestOutputManager& outputManager = UnitTestOutputManager::OpenInstance();
            outputManager.OutputMessage(buffer);
            gAbortTracing = TRUE;
        }
    }
}

void StopTraceCapture()
{
    if( !gAbortTracing )
    {
        if( !XTraceStopRecording() )
        {
            UnitTestOutputManager& outputManager = UnitTestOutputManager::OpenInstance();
            outputManager.OutputMessage("Error: Unable to complete recording of trace results.\n");
        }

        // For now, there is no way to uniqueify the trace name in
        // StartTraceCapture.  So to avoid overwriting trace info,
        // gAbortTracing is set to true.  That ensures only one trace record is
        // created.  This limitation could be easily removed if we want to
        // record multiple traces.
        gAbortTracing = TRUE;
    }
}

#endif


template<>
const char *GetTypeName<float>()
{
    return "float";
}

template<>
const char *GetTypeName<double>()
{
    return "double";
}

template<>
const char *GetTypeName<rw::math::vpu::VecFloat>()
{
    return "VecFloat";
}

const char *GetBoolString(bool value)
{
    if(value)
    {
        return "true";
    }
    else
    {
        return "false";
    }
}

