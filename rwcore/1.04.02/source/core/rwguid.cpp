// (c) Electronic Arts. All Rights Reserved.

/// \file
/// Definition of RwGUID

// ***********************************************************************************************************
// Includes
#include "rwsharedheaders.hpp"
#include <limits.h>
#include <stdio.h>
#include "rw/core/guid.h"

// ***********************************************************************************************************
// Typedefs


// ***********************************************************************************************************
// Defines + Enums + Consts


// ***********************************************************************************************************
// Static Variables + Static Data Member Definitions


// ***********************************************************************************************************
// Structs + Unions + Classes


// ***********************************************************************************************************
// Static Functions

// ***********************************************************************************************************
// External Functions


// ***********************************************************************************************************
// Class Member Functions

// ***********************************************************************************************************
//                                                RwGUID CLASS
// ***********************************************************************************************************
/**
Default constructor.
*/
RwGUID::RwGUID()
{
    rwFUNCTION();
    rwRETURNVOID();
}

/**
Construct RwGUID from array of 16 uint8_t.
\param ptr Pointer to an array of 16 uint8_t.
*/
RwGUID::RwGUID(const uint8_t *ptr)
{
    rwFUNCTION();

    rwASSERT(ptr);

    m_data1 = *(reinterpret_cast<const uint32_t*>(&ptr[0]));
    m_data2 = *(reinterpret_cast<const uint16_t*>(&ptr[4]));
    m_data3 = *(reinterpret_cast<const uint16_t*>(&ptr[6]));

    RwMemcpy(&m_data4[0], &ptr[8], 8);

    rwRETURNVOID();
}

/**
Constructs a GUID from a string, the following formats are supported,
{18337D35-3702-406C-A3A6-B8BD6BB76E4E}
{18337D353702406CA3A6B8BD6BB76E4E}
18337D35-3702-406C-A3A6-B8BD6BB76E4E
18337D353702406CA3A6B8BD6BB76E4E

We assume that the string as 32 characters without -,{,}

\param str A pointer to a string containing the guid.
*/
RwGUID::RwGUID(const RwChar * str)
{
    rwFUNCTION();
    // We elinimate the dashes and convert the string in chunks of 8 characters to
    // convert into uint32_t
    RwChar part[9];
    int32_t i = 0;
    int32_t p = 0;
    uint32_t tempInt;

    if (str[p] == '{')
    {
        p++;
    }

    i = 0;
    part[8] = '\0';
    // Read the first data member (uint32_t)
    while (i != 8)
    {
        if (str[p] != '-')
        {
            part[i++] = str[p];
        }
        ++p;
    }
    // Convert into an int
    sscanf(part, "%x", &m_data1);

    i = 0;
    part[4] = '\0';
    // Read the second data member (uint16_t)
    while (i != 4)
    {
        if (str[p] != '-')
        {
            part[i++] = str[p];
        }
        ++p;
    }
    // Convert into an int
    sscanf(part, "%x", &tempInt);
    m_data2 = static_cast<uint16_t>(tempInt);

    i = 0;
    part[4] = '\0';
    // Read the third data member (uint16_t)
    while (i != 4)
    {
        if (str[p] != '-')
        {
            part[i++] = str[p];
        }
        ++p;
    }
    // Convert into an int
    sscanf(part, "%x", &tempInt);
    m_data3 = static_cast<uint16_t>(tempInt);

    i = 0;
    part[2] = 0;
    // Read the third data member (uint16_t)
    while (i != 8)
    {
        if (str[p] != '-')
        {
            part[0] = str[p++];
            part[1] = str[p];
            // Convert into an int
            sscanf(part, "%x", &tempInt);
            m_data4[i] = static_cast<uint8_t>(tempInt);
            ++i;
        }
        ++p;
    }

    rwRETURNVOID();
}


/**
Initialize RwGUID to 0.
*/
void
RwGUID::Zero()
{
    rwFUNCTION();
    m_data1 = 0;
    m_data2 = 0;
    m_data3 = 0;
    m_data4[0] = 0;
    m_data4[1] = 0;
    m_data4[2] = 0;
    m_data4[3] = 0;
    m_data4[4] = 0;
    m_data4[5] = 0;
    m_data4[6] = 0;
    m_data4[7] = 0;

    rwRETURNVOID();
}

/**
Endian swap an RwGUID in place.
*/
void
RwGUID::EndianSwap()
{
    rwFUNCTION();

    RWENDIANSWAP32(m_data1);
    RWENDIANSWAP16(m_data2);
    RWENDIANSWAP16(m_data3);

    rwRETURNVOID();
}

/**
Equality operator.
\param guid A reference to a RwGUID.
\return TRUE if lhs is equal to rhs otherwise FALSE.
*/
RwBool
RwGUID::operator == ( const RwGUID & guid ) const
{
    rwFUNCTION();

    const uint32_t *srcData1 = reinterpret_cast<const uint32_t *>(this);
    const uint32_t *srcData2 = reinterpret_cast<const uint32_t *>(&guid);

    rwRETURN((RwBool)((srcData1[0] == srcData2[0])
          && (srcData1[1] == srcData2[1])
          && (srcData1[2] == srcData2[2])
          && (srcData1[3] == srcData2[3])));
}

/**
Less that operator.
\param guid A reference to a RwGUID.
\return TRUE if lhs is considered less than rhs otherwise FALSE.
*/
RwBool
RwGUID::operator < ( const RwGUID & guid ) const
{
    rwFUNCTION();

    const uint32_t *srcData1 = reinterpret_cast<const uint32_t *>(this);
    const uint32_t *srcData2 = reinterpret_cast<const uint32_t *>(&guid);

    if (srcData1[0] != srcData2[0])
    {
        rwRETURN ((RwBool)(srcData1[0] < srcData2[0]));
    }
    else
    {
        if (srcData1[1] != srcData2[1])
        {
            rwRETURN ((RwBool)(srcData1[1] < srcData2[1]));
        }
        else
        {
            if (srcData1[2] != srcData2[2])
            {
                rwRETURN ((RwBool)(srcData1[2] < srcData2[2]));
            }
            else
            {
                rwRETURN ((RwBool)(srcData1[3] < srcData2[3]));
            }
        }
    }
}
/**
Prefix operator, useful if autogenerating a fake guid, /note this in no way ensures that the guid is unique.
\return Returns a refernce to a RwGUID.
*/
RwGUID&
RwGUID::operator++()
{
    rwFUNCTION();

    uint32_t *srcData = reinterpret_cast<uint32_t *>(this);

    srcData[3] += 1;

    if (srcData[3] == 0)
    {
        srcData[2] += 1;

        if (srcData[2] == 0)
        {
            srcData[1] += 1;

            if (srcData[1] == 0)
            {
                srcData[0] += 1;
            }
        }
    }

    rwRETURN(*this);
}
/**
Converts the RwGUID to a string of the format {DC3C1D5B-D860-4F3F-845E-BE626C123195}.
\param buffer A pointer to a buffer into which the string is written.
*/
void
RwGUID::ToString(RwChar *buffer) const
{
    rwFUNCTION();
    sprintf(buffer,"{%x-%x-%x-0%x%x-%x%x%x%x%x%x}", m_data1, m_data2, m_data3, m_data4[0], m_data4[1],
        m_data4[2], m_data4[3], m_data4[4], m_data4[5], m_data4[6], m_data4[7]);
    rwRETURNVOID();
}
