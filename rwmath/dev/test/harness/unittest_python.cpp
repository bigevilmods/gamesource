// (c) Electronic Arts. All Rights Reserved.
#include "testconfig.h"
#include "unittest.h"
#include "unittest_python.h"
#include "unittest_printf.h"

#ifndef __SPU__
    #include <stdarg.h>
#endif

#define PREFIX "PYTHON_OUTPUT> "

void UnitTestPythonOutput::OutputClassName(const char *className)
{
    Strcpy( mTypeAndClass, className );
    mType = mTypeAndClass;

    // classNames are of the form "host_vpu::Matrix44".  We search
    // for the first ':' to determine how to split the string.
    int i;
    for(i=0;mTypeAndClass[i] !=':';i++)
    {
    }

    // Null terminate type string
    mTypeAndClass[i] = '\0';

    mClass = &mTypeAndClass[i+2];
}

void UnitTestPythonOutput::OutputBenchmarkResult(const char * testIdentifier, const char *functionName, double timeValue)
{
    Printf(PREFIX "{ 'type':'%s', 'class':'%s', 'id':'%s', 'function':'%s', 'value':%f },\n", mType, mClass, testIdentifier, functionName, timeValue);
}

void UnitTestPythonOutput::OutputMessage(const char* message) 
{ 
    Printf(message); 
}


void UnitTestPythonOutput::OutputInitial()
{
    Printf(PREFIX "[\n");
}


void UnitTestPythonOutput::OutputFinal()
{
    Printf(PREFIX " ]\n");
}

