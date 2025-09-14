// (c) Electronic Arts. All Rights Reserved.

#include "EABase/eabase.h"
#include "unittest_printf.h"
#include <stdio.h>

#if defined EA_PLATFORM_CTR
#include <nn/os.h>
#endif

#ifndef __SPU__

#include <stdarg.h>

#if defined EA_PLATFORM_XENON
#include <xtl.h>
#elif defined EA_PLATFORM_WINDOWS
#include <Windows.h>
#endif



void Printf(const char* format, ...)
{
    va_list arg;
    va_start(arg, format);

#if defined EA_PLATFORM_WINDOWS
    char buffer[2048];
    vsprintf(buffer, format, arg);
    OutputDebugStringA(buffer);
    vprintf(format, arg);
#elif defined EA_PLATFORM_ANDROID
    char buffer[2048];
    vsprintf(buffer, format, arg);
    printf("%s", buffer);
#elif defined EA_PLATFORM_CTR
	nn::dbg::detail::VPrintf(format, arg);
#else
    vprintf(format, arg);
#endif

    va_end(arg);
}


#endif
