// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_UNITTEST_UNITTEST_H
#define RW_MATH_UNITTEST_UNITTEST_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "EABase/eabase.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "unittest_output.h"
#include "unittest_printf.h"

#if defined EA_COMPILER_METROWERKS
    // Make sure EA_COMPILER_METROWERKS is checked first.  For some reason it
    // looks like the Metrowerks compiler defines __GNUC__ so we need to check
    // for EA_COMPILER_METROWERKS first.
    #define UNIT_TEST_DECLARE_NO_INLINE_PREFIX
    #define UNIT_TEST_DECLARE_NO_INLINE_POSTFIX __attribute__((never_inline))
#elif (__GNUC__ >= 3)
    // Used to ensure test classes are not
    #define UNIT_TEST_DECLARE_NO_INLINE_PREFIX
    #define UNIT_TEST_DECLARE_NO_INLINE_POSTFIX __attribute__((noinline))
#elif defined _MSC_VER
    #define UNIT_TEST_DECLARE_NO_INLINE_PREFIX __declspec(noinline)
    #define UNIT_TEST_DECLARE_NO_INLINE_POSTFIX
#elif (__GNUC__ < 3)
    // GCC 2, do nothing
    #define UNIT_TEST_DECLARE_NO_INLINE_PREFIX
    #define UNIT_TEST_DECLARE_NO_INLINE_POSTFIX
#else
    #error Unknown compiler
#endif


class UnitTest
{
public:

    UnitTest();

    virtual ~UnitTest();

    virtual void Test() = 0;

    void SetClassName(const char* className);

    void SetTestIdentifier(const char* identifier);

#if !defined __SPU__
    void SetFunctionName(const char* functionNameFormat, ...);
#endif

    void ValidateTest(bool valid, const char* filename=__FILE__, const int linenum= __LINE__);

    void StartTimer(unsigned int loopCount);
    void StopTimer();

    void ShowResult();

    static void ShowFinalResult();

    static bool PassedAllTests();

    void *operator new( size_t size );

    void operator delete( void* object );

protected:

    bool IsEqualWithEpsilon(float a, float b, float epsilon = 0.001);
    bool IsEqualWithEpsilon(double a, double b, double epsilon = 0.001);

    bool IsEqualWithinPercent(float expected, float actual, float percent = 0.00001);
    bool IsEqualWithinPercent(double expected, double actual, float percent  = 0.00001);
protected:
    char        mClassName[512];        // the name of class currently being tested
    char        mTestIdentifier[512];   // the identifier of class currently being tested
    char        mFunctionName[1024];    // the name of function currently being tested

    uint64_t mTimerStart;            // start of timing test
    uint64_t mTimerStop;             // end of timing test
    uint64_t mLoopCount;             // number of loops timing test went through
    uint64_t mTickPerSecond;         // number of tick per second

    unsigned int mTestCount;
    unsigned int mTestPassed;

    static uint32_t sTotalTestCount;
    static uint32_t sTotalTestPassed;

    static UnitTestOutputManager& mOutputManager;
};

// Returns the name of a type.
// e.g. GetTypeName<float>() returns "float".
template<class T>
const char *GetTypeName();

// Returns "true" or "false" depending on 'value'.
const char *GetBoolString(bool value);

#define Validate(condition) \
    ( ValidateTest(condition, __FILE__, __LINE__) )


// The unit test framework implements strcpy because the SPU standard strcpy is huge.
void Strcpy( char *dest, const char *src);

#if defined EA_PLATFORM_XENON
// Functions for doing trace captures.
// At the moment these functions only do something useful on Xenon.

void InitializeTraceCaptureSupport();
// 'traceName' is a unique name for the trace.  The name will be used to create a filename
// for outputting trace results to, so only use characters that are valid characters in
// a filename.
void StartTraceCapture(char *traceName);
void StopTraceCapture();
#endif

#if defined __SPU__

#define EVENT_SPRINTF_PORT 2U
#define EVENT_EXIT_PORT 4U

#define spu_sprintf(buffer, fmt, args...) \
	_spu_call_event_va_arg(EVENT_SPRINTF_PORT<<EVENT_PORT_SHIFT, buffer, fmt, ## args)

#define spu_exit() \
	_spu_call_event_va_arg(EVENT_EXIT_PORT<<EVENT_PORT_SHIFT, (char*)42/*dummy*/)

#define SetFunctionName(classNameFormat, args...) \
{ \
    spu_sprintf( mFunctionName, classNameFormat, ## args ); \
}

#include <spu_printf.h>
#define Printf(fmt, args...) spu_printf(fmt, ## args)

#endif


class AlignedFloatBuffer
{
public:
    AlignedFloatBuffer(float *floatData, size_t sizeOfFloatData)
    {
        uintptr_t unalignedBuffer = reinterpret_cast<uintptr_t>(&mBuffer[0]);
        uintptr_t buffer = (unalignedBuffer + 15) & ~15;
        mData = reinterpret_cast<float *>(buffer);
        memcpy(mData, floatData, sizeOfFloatData);
    }

    const float* GetFloatData()
    {
        return mData;
    }

private:
    const static size_t BUFFER_SIZE = 512;
    char mBuffer[BUFFER_SIZE];
    float *mData;
};

void Strcpy( char *dest, const char *src);

#endif

