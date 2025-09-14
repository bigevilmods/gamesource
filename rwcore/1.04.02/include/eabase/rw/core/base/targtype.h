// (c) Electronic Arts. All Rights Reserved.

/// \file
/// Defines Windows target-specific types and macros.

#ifndef RW_TARGTYPE_H
#define RW_TARGTYPE_H

#include <EABase/eabase.h>

#define rwSIZEOF_CHAR 8
#define rwCHAR_IS_SIGNED 1

#define rwTARGET_ENDIAN rwLIENDIAN

#define rwTARGET_WIN 1

#define rwMATRIXALIGNMENT 16

#define rwTARGET_POINTERSIZE EA_PLATFORM_PTR_SIZE

#define rwTARGET_GLOBALPOINTERSIZE rwTARGET_POINTERSIZE

#if rwTARGET_POINTERSIZE==8
#define rwTARGET_POINTERALIGNMENT 8
#else
#define rwTARGET_POINTERALIGNMENT 4
#endif

#endif /* !RW_TARGTYPE_H */
