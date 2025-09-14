// (c) Electronic Arts. All Rights Reserved.

/// \file
/// Header file for hashing support

#ifndef RW_HASH_HPP
#define RW_HASH_HPP
#include <rw/core/base/ostypes.h>

// ***********************************************************************************************************
// Includes


// ***********************************************************************************************************
// Defines

namespace rw
{

/**
Initial hash value for 32 bit unsigned int hashing function
*/
extern const uint32_t RWHASH32_INIT;

/**
Initial hash value for 64 bit unsigned int hashing function
*/
extern const uint64_t RWHASH64_INIT;


// ***********************************************************************************************************
// Forward Declarations


// ***********************************************************************************************************
// Typedefs


// ***********************************************************************************************************
// Enums + Consts


// ***********************************************************************************************************
// Global Variables


// ***********************************************************************************************************
// External Functions Prototypes


uint32_t
RwHash32UInt32(uint32_t integer,
               uint32_t hashValue);

uint32_t
RwHash32Buffer(const uint8_t *buffer,
               size_t length,
               uint32_t hashValue);

uint32_t
RwHash32String(const RwChar *str,
               uint32_t hashValue);


uint64_t
RwHash64Buffer(const uint8_t *buffer,
               size_t length,
               uint64_t hashValue);

uint64_t
RwHash64String(const RwChar *str,
               uint64_t hashValue);


// ***********************************************************************************************************
// Structs + Unions + Classes

} // namespace rw

#endif // RW_HASH_HPP
