// (c) Electronic Arts. All Rights Reserved.

#ifdef __SPU__

#include "unittest_spuproxy.h"


#define EVENT_SPUPROXY_PORT 3U

#define spu_proxy_method(method, args...) \
    _spu_call_event_va_arg(EVENT_SPUPROXY_PORT<<EVENT_PORT_SHIFT, (char *)method, ## args)

void UnitTestSPUProxy::OutputInitial()
{
    spu_proxy_method( UTOM_OutputInitial );
}

void UnitTestSPUProxy::OutputClassName(const char *className)
{
    spu_proxy_method( UTOM_OutputClassName, className );
}

void UnitTestSPUProxy::OutputTestResult(bool result, const char *functionName, const char *fileName, int lineNumber)
{
    spu_proxy_method( UTOM_OutputTestResult, result, functionName, fileName, lineNumber );
}

void UnitTestSPUProxy::OutputBenchmarkResult(const char *testIdentifier, const char *functionName, double timeValue)
{
    spu_proxy_method( UTOM_OutputBenchmarkResult, testIdentifier, functionName, timeValue );
}

void UnitTestSPUProxy::OutputTotals(unsigned int totalTests, unsigned int totalPassed)
{
    spu_proxy_method( UTOM_OutputTotals, totalTests, totalPassed );
}

void UnitTestSPUProxy::OutputClassTotals(const char *className, unsigned int totalTests, unsigned int totalPassed)
{
    spu_proxy_method( UTOM_OutputClassTotals, className, totalTests, totalPassed );
}

void UnitTestSPUProxy::OutputMessage(const char* message)
{
    spu_proxy_method( UTOM_OutputMessage, message );
}

void UnitTestSPUProxy::OutputFinal()
{
    spu_proxy_method( UTOM_OutputFinal );
}

#endif
