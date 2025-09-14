// (c) Electronic Arts. All Rights Reserved.
#include "testconfig.h"
#include "unittest.h"
#include "unittest_text.h"
#include "EABase/eabase.h"
#include "unittest_printf.h"

#ifndef __SPU__
    #include <stdarg.h>
#endif

void UnitTestTextOutput::OutputTestResult(bool result, const char *functionName, const char *fileName, int lineNumber)
{
    if(result)
    {
#ifdef RW_MATH_UNITTEST_SHOW_PASSED
        Printf("%80s\t[passed]\n", functionName);
#endif
    }
    else
    {
        Printf("%80s\t[failed] <----\n%s(%d).\n", functionName, fileName, lineNumber);
    }
        
}

void UnitTestTextOutput::OutputTotals(unsigned int totalTests, unsigned int totalPassed)
{
#if !defined __SPU__
    Printf("-------------------------------------------------------------------------------\n"
           "Test Passed [%d] / Total Test [%d]\n"
           "-------------------------------------------------------------------------------\n",
           totalPassed, totalTests);
#else
    Printf("%s%s%d!%s%d\n", RW_MATH_UNITTEST_DATA_OUTPUT_TAG, RW_MATH_UNITTEST_DATA_OUTPUT_PASSED_TAG, totalPassed,
                                                              RW_MATH_UNITTEST_DATA_OUTPUT_TOTAL_TAG, totalTests);    
#endif
}


void UnitTestTextOutput::OutputClassName(const char *className)
{
    Printf("-------------------------------------------------------------------------------\n"
           "class %73s\n"
           "-------------------------------------------------------------------------------\n",
           className);
}

void UnitTestTextOutput::OutputBenchmarkResult(const char * /*testIdentifier*/, const char *functionName, double timeValue)
{
    Printf("%80s\t[%7f] MicroSecond\n", functionName, timeValue);
}

void UnitTestTextOutput::OutputMessage(const char* message) 
{ 
    Printf(message); 
}

