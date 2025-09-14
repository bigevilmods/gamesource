// (c) Electronic Arts. All Rights Reserved.

/// \file
/// This file provides utility functions for manipulating strings.

#ifndef PUBLIC_RW_CORE_STRINGUTILS_H
#define PUBLIC_RW_CORE_STRINGUTILS_H

// ***********************************************************************************************************
// Defines

namespace rw
{

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

/**
Breaks a string into substrings at a specified token.
\par
Given a string break it into seperate strings that are determined by a token. For example
input="foo;bar;elephant;rhino" and token=';' give results=("foo", "bar", "elephant", "rhino").
\param result A vector to append the parts to.
\param input The string to tokenize.
\param token The seperator.
*/
inline void
TokenizeString(RwVector<RwString> &results, const RwString &input, RwChar token)
{
    rwFUNCTION();

    RwString::size_type start = 0;
    RwString::size_type end = 0;

    while (RwString::npos != (end = input.find_first_of(token, start)))
    {
        results.push_back(input.substr(start, end - start));
        start = end + 1;
    }

    end = input.length();
    if (start < end)
    {
        results.push_back(input.substr(start, end - start));
    }

    rwRETURNVOID();
}

/**
Concatenates all strings in vIn with a token in between strings.

For example
vIn = ("red", "green", "blue", "orange") with token=',' gives out="red,green,blue,orange"
\param out The string to append the strings to.
\param vIn A vector of strings to concatenate.
\param token To put inbetween the strings.
*/
inline void
ConcatenateStrings(RwString &out, const RwVector<RwString> &vIn, const RwChar token)
{
    rwFUNCTION();

    size_t size = vIn.size();

    for (uint32_t i = 0; i < size; i++)
    {
        out += vIn[i];

        if (i < (size - 1))
        {
            out += token;
        }
    }

    rwRETURNVOID();
}

// ***********************************************************************************************************
// Structs + Unions + Classes

// ***********************************************************************************************************
//                                                Class Definition
// ***********************************************************************************************************

} //namespace rw

#endif // PUBLIC_RW_CORE_STRINGUTILS_H
