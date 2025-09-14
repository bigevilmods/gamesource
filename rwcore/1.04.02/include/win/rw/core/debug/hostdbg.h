// (c) Electronic Arts. All Rights Reserved.

/// \file
/// Windows host debugging macros

#ifndef RW_HOSTDBG_H
#define RW_HOSTDBG_H

// Can't use Win32's DebugBreak since we have to avoid including windows.h
#if defined(_MSC_VER)
#define rwBREAK() __debugbreak()
#elif defined(__GNUC__)
#define rwBREAK() __asm__("int3" )
#else
#error Unsupported platform
#endif /* !defined(__GNUC__) */

#endif /* !RW_HOSTDBG_H */
