// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_UNITTEST_UNITTEST_SPUPROXY_H
#define RW_MATH_UNITTEST_UNITTEST_SPUPROXY_H

#include "EABase/eabase.h"
#include "unittest.h"
#include "unittest_output.h"
#include "unittest_spuproxy_enums.h"


class UnitTestSPUProxy : public IUnitTestOutput
{
    friend class UnitTestOutputManager;

public:
    virtual void OutputInitial();
    virtual void OutputClassName(const char *className);
    virtual void OutputTestResult(bool result, const char *functionName, const char *fileName, int lineNumber);
    virtual void OutputBenchmarkResult(const char* testIdentifier, const char* functionName, double timeValue);
    virtual void OutputTotals(unsigned int totalTests, unsigned int totalPassed);
    virtual void OutputClassTotals(const char *className, unsigned int totalTests, unsigned int totalPassed);
    virtual void OutputMessage(const char* message);
    virtual void OutputFinal();

protected:
    UnitTestSPUProxy() {}
};

#endif

