// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_UNITTEST_UNITTEST_TEXT_H
#define RW_MATH_UNITTEST_UNITTEST_TEXT_H

#include "EABase/eabase.h"
#include "unittest_output.h"

class UnitTestTextOutput : public IUnitTestOutput
{
    friend class UnitTestOutputManager;

public:
    virtual void OutputInitial() {};
    virtual void OutputClassName(const char *className);
    virtual void OutputTestResult(bool result, const char *functionName, const char *fileName, int lineNumber);
    virtual void OutputBenchmarkResult(const char* testIdentifier, const char* functionName, double timeValue);
    virtual void OutputTotals(unsigned int totalTests, unsigned int totalPassed);
    virtual void OutputClassTotals(const char *, unsigned int, unsigned int)
    {
    };
    virtual void OutputMessage(const char* message);
    virtual void OutputFinal() {};

protected:
    UnitTestTextOutput() {};

private:
};

#endif

