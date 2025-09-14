// (c) Electronic Arts. All Rights Reserved.

#include "EABase/eabase.h"
#include "testconfig.h"
#include "unittest.h"
#if defined __SPU__
#include "unittest_spuproxy.h"
#else
#include "unittest_csv.h"
#include "unittest_text.h"
#include "unittest_python.h"
#include <stdio.h>
#endif // __SPU__
#include <string.h>

#if defined EA_PLATFORM_PS3 && !defined __SPU__
    #include <sys/paths.h>
    const char *RW_MATH_UNITTEST_CSV_FILE_PATH = SYS_APP_HOME "/rwmath_test_results_ppu.csv";
#elif defined EA_PLATFORM_XENON 
    const char *RW_MATH_UNITTEST_CSV_FILE_PATH = "D:\\rwmath_test_results_xenon.csv";
#elif defined EA_PLATFORM_WINDOWS
    #if EA_PLATFORM_PTR_SIZE == 8
        const char *RW_MATH_UNITTEST_CSV_FILE_PATH = "rwmath_test_results_pc64.csv";
    #elif EA_PLATFORM_PTR_SIZE == 4
        const char *RW_MATH_UNITTEST_CSV_FILE_PATH = "rwmath_test_results_pc32.csv";
    #else
        #error unexpected pointer size
    #endif
#elif EA_PLATFORM_REVOLUTION
    const char *RW_MATH_UNITTEST_CSV_FILE_PATH = "";
#elif EA_PLATFORM_PS3_SPU
    const char *RW_MATH_UNITTEST_CSV_FILE_PATH = "";
#else
    const char *RW_MATH_UNITTEST_CSV_FILE_PATH = "rwmath_test_results_" EA_PLATFORM_NAME ".csv";
#endif


IUnitTestOutput* UnitTestOutputManager::GetOutput(const char *target)
{
    for(UnitTestOutputList *listElement = mOutputListHead; listElement; listElement = listElement->mNext)
    {
        if(!strcmp(listElement->mTargetName, target))
            return listElement->mPtr;
    }

    return NULL;
}    

bool UnitTestOutputManager::AddOutput(const char *target)
{
    if(!target || GetOutput(target))
        return false;

#if defined __SPU__
    if(!strcmp(target, "spuproxy"))
    {
        AddToList( new UnitTestSPUProxy(), target);
    }
#else
    if(!strcmp(target, "csvfile"))
    {
        AddToList( new UnitTestCSVFileOutput(RW_MATH_UNITTEST_CSV_FILE_PATH), target);
    }
    else if(!strcmp(target, "stdout"))
    {
        AddToList( new UnitTestTextOutput(), target);
    }
    else if(!strcmp(target, "python"))
    {
        AddToList( new UnitTestPythonOutput(), target);
    }
#endif // __SPU__
    else
    {
        return false;
    }

    return true;
}

#ifdef __SPU__
bool UnitTestOutputManager::ParseCommandLineArguments(int, char **)
{
    return false;
}
#else
bool UnitTestOutputManager::ParseCommandLineArguments(int argc, char **argv)
{
    const char outputPrefix[] = "--output-";
    const size_t outputPrefixStrLen = sizeof(outputPrefix) - 1;   // subtract 1 to account for '\0' terminator.
    // Loop over all the command line arguments
    for( int argIndex = 1; argIndex < argc; )
    {
        const char* curArg = argv[argIndex];
        if(strstr(curArg, outputPrefix) == curArg)
        {
            if(!AddOutput(curArg + outputPrefixStrLen))
            {
                printf("Unable to add output type '%s'.\n", curArg + outputPrefixStrLen);
                return false;
            }
        }
        else if(strcmp(curArg, "--spu-response-file")==0)
        {
            argIndex++;
#ifndef EA_PLATFORM_PS3
            printf("Unsupported option (--spu-response-file) for this platform.\n");
            return false;
#endif
        }
        else
        {
            return false;
        }

        argIndex++;
    }

    AddOutput("stdout");
    return true;
}
#endif

void UnitTestOutputManager::OutputInitial()
{
    for(UnitTestOutputList *listElement = mOutputListHead;listElement;listElement = listElement->mNext)
    {
        listElement->mPtr->OutputInitial();
    }
}

void UnitTestOutputManager::OutputFinal()
{
    for(UnitTestOutputList *listElement = mOutputListHead;listElement;listElement = listElement->mNext)
    {
        listElement->mPtr->OutputFinal();
    }
}

void UnitTestOutputManager::OutputClassName(const char *className)
{
    for(UnitTestOutputList *listElement = mOutputListHead;listElement;listElement = listElement->mNext)
    {
        listElement->mPtr->OutputClassName(className);
    }
}

void UnitTestOutputManager::OutputClassTotals(const char *className, unsigned int totalTests, unsigned int totalPassed)
{
    for(UnitTestOutputList *listElement = mOutputListHead;listElement;listElement = listElement->mNext)
    {
        listElement->mPtr->OutputClassTotals(className, totalTests, totalPassed);
    }
}

void UnitTestOutputManager::OutputTotals(unsigned int totalTests, unsigned int totalPassed)
{
    for(UnitTestOutputList *listElement = mOutputListHead;listElement;listElement = listElement->mNext)
    {
        listElement->mPtr->OutputTotals(totalTests, totalPassed);
    }
}

void UnitTestOutputManager::OutputMessage(const char* message)
{
    for(UnitTestOutputList *listElement = mOutputListHead;listElement;listElement = listElement->mNext)
    {
        listElement->mPtr->OutputMessage(message);
    }
}

void UnitTestOutputManager::OutputBenchmarkResult(const char * testIdentifier,
                                                  const char* functionName, 
                                                  double timeValue)
{
    for(UnitTestOutputList *listElement = mOutputListHead;listElement;listElement = listElement->mNext)
    {
        listElement->mPtr->OutputBenchmarkResult(testIdentifier, functionName, timeValue);
    }
}

void UnitTestOutputManager::OutputTestResult(bool result, 
                                             const char* functionName, 
                                             const char* fileName, 
                                             int lineNumber)
{
    for(UnitTestOutputList *listElement = mOutputListHead;listElement;listElement = listElement->mNext)
    {
        listElement->mPtr->OutputTestResult(result, functionName,  fileName, lineNumber);
    }
}

UnitTestOutputManager::~UnitTestOutputManager()
{
    while(mOutputListHead)
    {
        UnitTestOutputList *currentNode = mOutputListHead;
        mOutputListHead = currentNode->mNext;
        delete currentNode;
    }
}

void UnitTestOutputManager::AddToList(IUnitTestOutput *output, const char* target)
{
    UnitTestOutputList *newNode = new UnitTestOutputList(output, target);
    newNode->mNext = mOutputListHead;
    mOutputListHead = newNode;
}

UnitTestOutputManager::UnitTestOutputList::UnitTestOutputList(IUnitTestOutput *newPtr, const char *target) 
: mPtr(newPtr), mTargetName(target), mNext(NULL) 
{}

