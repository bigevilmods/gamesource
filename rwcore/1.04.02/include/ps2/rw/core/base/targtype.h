// (c) Electronic Arts. All Rights Reserved.

/// \file
/// Defines PS2 target-specific types and macros.

#ifndef RW_TARGTYPE_H
#define RW_TARGTYPE_H

#define rwSIZEOF_CHAR 8

#define rwCHAR_IS_SIGNED 1

#define rwTARGET_ENDIAN rwLIENDIAN

#define rwTARGET_PS2 1

#if !defined(rwMATRIXALIGNMENT)
#define rwMATRIXALIGNMENT 16
#endif // defined(rwMATRIXALIGNMENT)

#define rwTARGET_POINTERSIZE 4

#define rwTARGET_GLOBALPOINTERSIZE rwTARGET_POINTERSIZE

#define rwTARGET_POINTERALIGNMENT 4

#endif /* !RW_TARGTYPE_H */
