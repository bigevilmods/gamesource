// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_UNITTEST_UNITTEST_CSV_H
#define RW_MATH_UNITTEST_UNITTEST_CSV_H

#include "EABase/eabase.h"
#include "unittest_output.h"

#if !defined __SPU__
    #include <stdio.h>
#endif

class UnitTestCSVFileOutput : public IUnitTestOutput
{
    friend class UnitTestOutputManager;

public:
#if !defined __SPU__
    virtual ~UnitTestCSVFileOutput() { CloseResultsFile(); } 
#endif

    virtual void OutputInitial();
    virtual void OutputClassName(const char *)
    {
    };
    virtual void OutputTestResult(bool result, const char *functionName, const char *fileName, int lineNumber);
    virtual void OutputBenchmarkResult(const char* testIdentifier, const char* functionName, double timeValue);
    virtual void OutputClassTotals(const char *className, unsigned int totalTests, unsigned int totalPassed);
    virtual void OutputTotals(unsigned int, unsigned int)
    {
    };
    virtual void OutputMessage(const char* message);
    virtual void OutputFinal();

protected:
#if !defined __SPU__
    UnitTestCSVFileOutput(const char *fileName)
    {
        InitializeResultsFile(fileName);
    };
#else
    UnitTestCSVFileOutput(const char *)
    {
    };
#endif

private:
#if !defined __SPU__
    void Printf(const char* format, ...);

    void InitializeResultsFile(const char *fileName);
    void CloseResultsFile();

    FILE *mResultsFile;
#endif
};


#endif
