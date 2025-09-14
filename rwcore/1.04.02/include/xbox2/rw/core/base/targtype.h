// (c) Electronic Arts. All Rights Reserved.

/// \file
/// Defines Xbox2 target-specific types and macros.

#ifndef RW_TARGTYPE_H
#define RW_TARGTYPE_H

#define rwSIZEOF_CHAR 8

#define rwCHAR_IS_SIGNED 1

#define rwTARGET_ENDIAN rwBIENDIAN

#define rwTARGET_XBOX2 1

#define rwMATRIXALIGNMENT 16

#define RWGXBOX2_ALIGN16(_O) RWALIGN(_O, 16)

#define rwTARGET_POINTERSIZE 4

#define rwTARGET_GLOBALPOINTERSIZE rwTARGET_POINTERSIZE

#define rwTARGET_POINTERALIGNMENT 4

#endif /* !RW_TARGTYPE_H */
