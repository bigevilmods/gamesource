// (c) Electronic Arts. All Rights Reserved.

/// \file
/// Defines Windows target-specific types and macros.

#ifndef RW_TARGTYPE_H
#define RW_TARGTYPE_H

#define rwSIZEOF_CHAR 8

#define rwCHAR_IS_SIGNED 1

#define rwTARGET_ENDIAN rwLIENDIAN

#define rwTARGET_WIN 1

#define rwMATRIXALIGNMENT 16

#if defined(_WIN64) || defined(RWTARGETOS_WIN64)
// If we are targetting a Win64 platform, arenas must have 64-bit pointers
//
// As of June 2008, we will require arenas to be built specifically for a
// 64-bit runtime environment, meaning that 64-bit games won't be able to
// load arenas built for a 32-bit configuration.  Since this value may
// change depending on the environment in which it was built, instead of
// having a 64-bit cross-compile platform, the PC cross configuration will
// need to be rebuilt from clean to build assets targetting a 32- or 64-
// bit environment.  To build assets targetting a 32-bit platform, the cross
// compile will have to be done in 32-bit mode, and to build assets
// for a 64-bit platform, the cross-compile will also have to be done in
// 64-bit mode.
#define rwTARGET_POINTERSIZE 8
#else
#define rwTARGET_POINTERSIZE 4
#endif

#define rwTARGET_GLOBALPOINTERSIZE rwTARGET_POINTERSIZE

#if defined(_WIN64) || defined(RWTARGETOS_WIN64)
#define rwTARGET_POINTERALIGNMENT 8
#else
#define rwTARGET_POINTERALIGNMENT 4
#endif

#endif /* !RW_TARGTYPE_H */
