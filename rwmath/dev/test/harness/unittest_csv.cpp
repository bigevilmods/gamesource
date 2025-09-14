// (c) Electronic Arts. All Rights Reserved.
#include "testconfig.h"
#include "unittest_csv.h"

#if defined __SPU__
    #include <spu_printf.h>
    #define Printf(fmt, args...) spu_printf("%s" fmt, RW_MATH_UNITTEST_CSV_FILE_OUTPUT_TAG, ## args)
#else
    #include <stdio.h>
    #include <stdarg.h>
#endif


void UnitTestCSVFileOutput::OutputTestResult(bool result, const char *functionName, 
                                             const char *fileName, int lineNumber)
{
    if(result)
    {
#if defined RW_MATH_UNITTEST_SHOW_PASSED
        Printf("TEST,\"%s\",0,OK\n", functionName);
#endif
    }
    else
        Printf("TEST,\"%s\",1,\"%s(%d)\"\n", functionName, fileName, lineNumber);
}

void UnitTestCSVFileOutput::OutputClassTotals(const char *className, unsigned int totalTests, unsigned int totalPassed)
{
    Printf("TEST,\"Class %s\",%d,\"Total tests: %d, tests passed: %d\"\n", className, totalTests-totalPassed, totalTests, totalPassed);
}

void UnitTestCSVFileOutput::OutputBenchmarkResult(const char * /*testIdentifier*/, const char* functionName, double timeValue)
{
    Printf("BENCHMARK,\"%s\",%f,0,0,Benchmark\n", functionName, timeValue);
}

void UnitTestCSVFileOutput::OutputInitial()
{
#if !defined __SPU__
    Printf("<data format=\"csv\">\n<content>\n<![CDATA[");
#endif
}

void UnitTestCSVFileOutput::OutputFinal()
{
#if !defined __SPU__
    Printf("]]>\n</content>\n</data>\n");
#endif
}

void UnitTestCSVFileOutput::OutputMessage(const char* message)
{
    Printf("%s", message);
}

#if !defined __SPU__

void UnitTestCSVFileOutput::Printf(const char* format, ...)
{
    if(!mResultsFile)
        return;

    va_list arg;
    va_start(arg, format);

    vfprintf(mResultsFile, format, arg);
}

void UnitTestCSVFileOutput::InitializeResultsFile(const char *fileName)
{
    if(fileName)
        mResultsFile = fopen(fileName, "w");
}


void UnitTestCSVFileOutput::CloseResultsFile()
{
    if(!mResultsFile)
        return;

    fclose(mResultsFile);
}
#endif

