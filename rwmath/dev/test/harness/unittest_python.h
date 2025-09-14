// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_UNITTEST_UNITTEST_PYTHON_H
#define RW_MATH_UNITTEST_UNITTEST_PYTHON_H

#include "EABase/eabase.h"
#include "unittest_output.h"

class UnitTestPythonOutput : public IUnitTestOutput
{
    friend class UnitTestOutputManager;

public:
    virtual void OutputInitial();
    virtual void OutputClassName(const char *className);
    virtual void OutputTestResult(bool, const char *, const char *, int)
    {
    }
    virtual void OutputBenchmarkResult(const char* testIdentifier, const char* functionName, double timeValue);
    virtual void OutputTotals(unsigned int, unsigned int)
    {
    }
    virtual void OutputClassTotals(const char *, unsigned int, unsigned int)
    {
    };
    virtual void OutputMessage(const char* message);
    virtual void OutputFinal();

protected:
    UnitTestPythonOutput() {};

private:
    char        mTypeAndClass[512];        // the name of class currently being tested
    char*       mType;
    char*       mClass;

};


#endif

