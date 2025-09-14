// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_MATH_UNITTEST_UNITTEST_SPUPROXY_ENUMS_H
#define RW_MATH_UNITTEST_UNITTEST_SPUPROXY_ENUMS_H

enum UnitTestOutputMethod
{
    UTOM_OutputInitial,
    UTOM_OutputClassName,
    UTOM_OutputTestResult,
    UTOM_OutputBenchmarkResult,
    UTOM_OutputTotals,
    UTOM_OutputClassTotals,
    UTOM_OutputMessage,
    UTOM_OutputFinal,

    UTOM_ForceSize = 0xffffffff
};

#endif

