// (c) Electronic Arts. All Rights Reserved.

/**
\file
Common include file for Windows.h/Xtl.h that alters warning levels and disables certain (large) portions of these API headers.
*/

#ifndef PLATFORM_SDK_H
#define PLATFORM_SDK_H

#include "EABase/eabase.h"

#if defined EA_PLATFORM_MICROSOFT
    #if defined EA_PLATFORM_XENON
        #pragma warning(push, 0)
        #define NOD3D
        #define NONET
        #include <Xtl.h>
        #pragma warning(pop)
    #else
        #pragma warning(push, 0)
        #ifndef WIN32_LEAN_AND_MEAN
        #   define WIN32_LEAN_AND_MEAN
        #endif
        #include <Windows.h>
        #pragma warning(pop)
    #endif
#endif

///////////////////////////////////////////////////////////////////////////////
// EA_WINAPI_FAMILY_PARTITION
//
// If this isn't defined after EABase/eabase.h, then define it to resolve to 1
// for DESKTOP as that version of EABase doesn't know anything about the new winapis.
// 
#if !defined(EA_WINAPI_FAMILY_PARTITION)
    #define EA_WINAPI_FAMILY_PARTITION(partition) partition
#endif

#if !defined(EA_WINAPI_PARTITION_DESKTOP)
    #if defined(EA_PLATFORM_WINDOWS)
        #define EA_WINAPI_PARTITION_DESKTOP 1
    #else
        #define EA_WINAPI_PARTITION_DESKTOP 0
    #endif
#endif

#endif

