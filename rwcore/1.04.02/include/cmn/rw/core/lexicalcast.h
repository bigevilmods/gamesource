// (c) Electronic Arts. All Rights Reserved.

/// \file
/// Simple boost::lexical_cast workalike for converting between string and numeric values.

#ifndef PUBLIC_RW_CORE_LEXICALCAST_H
#define PUBLIC_RW_CORE_LEXICALCAST_H

#include "rw/core/stltypes.h"
#include "rw/core/stringstream.h"

// ***********************************************************************************************************
// Forward Declarations

// ***********************************************************************************************************
// Typedefs

// ***********************************************************************************************************
// Defines + Enums + Consts

// ***********************************************************************************************************
// Global Variables

// ***********************************************************************************************************
// External Functions Prototypes

/**
Casts a FROM into a TO


Casts a FROM into a TO using an RwStringStream.
\par
Use it to convert a numerical value into a string, or vice versa:
\code
    int32_t  value = 1234;
    RwString intAsString = RwLexicalCast<RwString>(value);

    RwString realAsString = "3.1415";
    float32_t realvalue = RwLexicalCast<float32_t>(realvalue);
\endcode

\note All 8 bit types are treated as characters. If you want to convert an int8 into a string, you have to
cast it into an int16 or int32 first.
\code
    int8_t i = 65;
    RwString intAsString0 = RwLexicalCast<RwString>(i);          // gives "A"
    RwString intAsString1 = RwLexicalCast<RwString>((int32_t)i); // gives "65"
\endcode
*/
template <typename TO, typename FROM>
inline TO
RwLexicalCast(FROM f)
{
    rwFUNCTION();

    RwStringStream str;

    // By unsetting all the base fields, the stream will detect the base of a number according to its
    // initial characters, allowing "0777" to be read as octal and "0xff" to be read as hexadecimal.
    str.unsetf(RwIOS::basefield);

    TO o;
    str << f;
    str >> o;
    rwRETURN(o);
}

// ***********************************************************************************************************
// Structs + Unions + Classes

#endif // PUBLIC_RW_CORE_LEXICALCAST_H
