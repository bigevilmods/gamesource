// (c) Electronic Arts. All Rights Reserved.

/// \file
/// A type used as a unique identifier, globally unique identifier.

#ifndef RW_GUID_HPP
#define RW_GUID_HPP

// ***********************************************************************************************************
// Includes


// ***********************************************************************************************************
// Forward Declarations


// ***********************************************************************************************************
// Typedefs


// ***********************************************************************************************************
// Defines + Enums + Consts

#include <rw/core/base/ostypes.h>

// ***********************************************************************************************************
// Global Variables


// ***********************************************************************************************************
// External Functions Prototypes


// ***********************************************************************************************************
// Structs + Unions + Classes

// ***********************************************************************************************************
//                                                RwGUID CLASS
// ***********************************************************************************************************

/**
128 Bit globally Unique Identifier.
\member m_data1 Used internally to store the first 4 bytes of the GUID.
\member m_data2 Used internally to store the next 2 bytes of the GUID.
\member m_data3 Used internally to store the next 2 bytes of the GUID.
\member m_data4 Used internally to store the last 8 bytes of the GUID.
*/
class RwGUID
{
public:
    RwGUID();
    explicit RwGUID(const RwChar *str);
    explicit RwGUID(const uint8_t *ptr);
    void Zero();
    void EndianSwap();
    RwBool operator == ( const RwGUID & guid ) const;
    RwBool operator < ( const RwGUID & guid ) const;
    RwGUID& operator++();
    void ToString(RwChar *buffer) const;
private:
    uint32_t m_data1;
    uint16_t m_data2;
    uint16_t m_data3;
    uint8_t  m_data4[8];
};

#endif // RW_GUID_HPP
