// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_UNITTEST_UNITTEST_OUTPUT_H
#define RW_MATH_UNITTEST_UNITTEST_OUTPUT_H

#include "EABase/eabase.h"


class IUnitTestOutput
{
public:
    virtual ~IUnitTestOutput() {};

    virtual void OutputInitial() = 0;
    virtual void OutputClassName(const char *className) = 0;
    virtual void OutputTestResult(bool result, const char *functionName, const char *fileName, int lineNumber) = 0;
    virtual void OutputBenchmarkResult(const char *testIdentifier, const char* functionName, double timeValue) = 0;
    virtual void OutputTotals(unsigned int totalTests, unsigned int totalPassed) = 0;
    virtual void OutputClassTotals(const char *className, unsigned int totalTests, unsigned int totalPassed) = 0;
    virtual void OutputMessage(const char* message) = 0;
    virtual void OutputFinal() = 0;
};

class UnitTestOutputManager
{
public:
    ~UnitTestOutputManager();

    //Singleton
    static UnitTestOutputManager& OpenInstance()
    {
        static UnitTestOutputManager manager;
        return manager;
    }

    IUnitTestOutput* GetOutput(const char *target);
    bool AddOutput(const char *target);
    bool ParseCommandLineArguments(int argc, char **argv);

    void OutputInitial();
    void OutputClassName(const char *className);
    void OutputTestResult(bool result, const char *functionName, const char *fileName, int lineNumber);
    void OutputBenchmarkResult(const char *testIdentifier, const char* functionName, double timeValue);
    void OutputTotals(unsigned int totalTests, unsigned int totalPassed);
    void OutputClassTotals(const char *className, unsigned int totalTests, unsigned int totalPassed);
    void OutputMessage(const char* message);
    void OutputFinal();

protected:
    UnitTestOutputManager() {};


private:
    struct UnitTestOutputList
    {
        UnitTestOutputList(IUnitTestOutput *newPtr = NULL, const char* target = NULL);
        ~UnitTestOutputList()
        {
            if(mPtr) delete mPtr;
        };

        IUnitTestOutput *mPtr;
        const char *mTargetName;

        UnitTestOutputList *mNext;
    };

    void AddToList(IUnitTestOutput *output, const char *target);

    UnitTestOutputList *mOutputListHead;
};        

#endif

