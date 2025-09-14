// (c) Electronic Arts. All Rights Reserved.

/// \file
/// Declaration of the rw::HashedString class

#ifndef PUBLIC_RW_CORE_HASHEDSTRING_H
#define PUBLIC_RW_CORE_HASHEDSTRING_H

#include "rw/core/base/ostypes.h"
#include "rw/core/stltypes.h"

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


// ***********************************************************************************************************
// Structs + Unions + Classes

// ***********************************************************************************************************
//                                                RwHashedString CLASS
// ***********************************************************************************************************

/**
Class for storing a string, and a hash of that string, which allows for fast string comparisons.
Because the hash is not unique, comparison between two instances with a matching hash results
in a normal string comparison. Comparison between two instances with non-matching hashes is however
much faster, being the result of a single integer comparison.

\para
Relative to the time taken to compare two identical strings, tests indicate that it takes approximately
1.33 times longer to calculate the hash for one of the strings.
Comparison between strings with the same hash incurs the extra hash comparison.
Comparison between strings with a different hash is a constant time operation.
*/
class RwHashedString
{
public:
    // Construction & Destruction
    RwHashedString();
    explicit RwHashedString(const RwChar *str);
    explicit RwHashedString(const RwString &str);
    
    // Public Enums, Consts, Typedefs and Static variables

    // Interface Implementations

    // Methods
    inline uint32_t GetHash() const;
    inline const RwString &GetString() const;

    void SetString(const RwChar *str);
    void SetString(const RwString &str);

    inline bool operator==(const RwHashedString &other) const;
    inline bool operator!=(const RwHashedString &other) const;

    inline bool operator<(const RwHashedString &other) const;
    inline bool operator<=(const RwHashedString &other) const;
    inline bool operator>(const RwHashedString &other) const;
    inline bool operator>=(const RwHashedString &other) const;

    inline RwHashedString &operator=(const char *str);
    inline RwHashedString &operator=(const RwString &str);

    // Methods used for comparison against externally stored strings
    static uint32_t Hash(const RwChar *str);
    static uint32_t Hash(const RwChar *str, size_t len);

    inline bool Equals(uint32_t hash, const RwChar *str) const;

protected:
    // Virtual Functions

private:
    // Private Enums, Consts, Typedefs and Static variables

    // Helper Member Functions

    // Member Variables

    uint32_t mHash;
    RwString mString;
};

// Inline Functions

/**
Accessor for the hash value of the string.

\return The hash of the string.
*/
uint32_t RwHashedString::GetHash() const
{
    rwFUNCTION();
    rwRETURN(mHash);
}


/**
Accessor for the string value.

\return The string value.
*/
const RwString &RwHashedString::GetString() const
{
    rwFUNCTION();
    rwRETURN(mString);
}


/**
Tests two RwHashedStrings, using their hashes for faster comparison of dissimilar strings.

\param other The other RwHasedString to compare against.

\return true if both instances have the same string, false otherwise.
*/
bool RwHashedString::operator==(const RwHashedString &other) const
{
    rwFUNCTION();
    rwRETURN(mHash == other.mHash && mString == other.mString);
}


/**
Tests two RwHashedStrings for inequality, using their hashes for faster comparison of dissimilar strings.

\param other The other RwHasedString to compare against.

\return false if both instances have the same string, true otherwise.
*/
bool RwHashedString::operator!=(const RwHashedString &other) const
{
    rwFUNCTION();
    rwRETURN(mHash != other.mHash || mString != other.mString);
}


/**
Tests two RwHashedStrings according to RwString::operator< between their string values.

\param other The other RwHasedString to compare against.

\return true if this string value is less than other's string value, false otherwise.
*/
bool RwHashedString::operator<(const RwHashedString &other) const
{
    rwFUNCTION();
    rwRETURN(mString < other.mString);
}


/**
Tests two RwHashedStrings according to RwString::operator<= between their string values.

\param other The other RwHasedString to compare against.

\return true if this string value is less than or equal to other's string value, false otherwise.
*/
bool RwHashedString::operator<=(const RwHashedString &other) const
{
    rwFUNCTION();
    rwRETURN(mString <= other.mString);
}


/**
Tests two RwHashedStrings according to RwString::operator> between their string values.

\param other The other RwHasedString to compare against.

\return true if this string value is greater than other's string value, false otherwise.
*/
bool RwHashedString::operator>(const RwHashedString &other) const
{
    rwFUNCTION();
    rwRETURN(mString > other.mString);
}


/**
Tests two RwHashedStrings according to RwString::operator>= between their string values.

\param other The other RwHasedString to compare against.

\return true if this string value is greater than or equal to other's string value, false otherwise.
*/
bool RwHashedString::operator>=(const RwHashedString &other) const
{
    rwFUNCTION();
    rwRETURN(mString >= other.mString);
}


/**
Assigns a new string value, and forces a recalculation of the hash.

\param str The NULL terminated new string value.

\return A reference to this.
*/
RwHashedString &RwHashedString::operator=(const char *str)
{
    rwFUNCTION();
    SetString(str);
    rwRETURN(*this);
}

/**
Assigns a new string value, and forces a recalculation of the hash.

\param str The new string value.

\return A reference to this.
*/
RwHashedString &RwHashedString::operator=(const RwString &str)
{
    rwFUNCTION();
    SetString(str);
    rwRETURN(*this);
}

/**
Tests for equality against an externally stored string. The hash must have been calculated using
one of the the Hash() methods, and the string must not have been modified since.

\param hash The hash of the test string, as returned by Hash(str).
\param str The test string value.

\return true if the strings match, false otherwise.
*/
bool RwHashedString::Equals(uint32_t hash, const RwChar *str) const
{
    rwFUNCTION();
    rwRETURN(mHash == hash && mString == str);
}

} // namespace rw

#endif // PUBLIC_RW_CORE_HASHEDSTRING_H
