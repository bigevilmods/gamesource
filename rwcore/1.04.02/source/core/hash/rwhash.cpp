// (c) Electronic Arts. All Rights Reserved.

/// \file
/// A hash function implementation

/*!
 * Fowler/Noll/Vo hash code
 *
 ***
 *
 * Fowler/Noll/Vo hash
 *
 * The basis of this hash algorithm was taken from an idea sent
 * as reviewer comments to the IEEE POSIX P1003.2 committee by:
 *
 *      Phong Vo (http://www.research.att.com/info/kpv/)
 *      Glenn Fowler (http://www.research.att.com/~gsf/)
 *
 * In a subsequent ballot round:
 *
 *      Landon Curt Noll (http://www.isthe.com/chongo/)
 *
 * improved on their algorithm.  Some people tried this hash
 * and found that it worked rather well.  In an EMail message
 * to Landon, they named it the ``Fowler/Noll/Vo'' or FNV hash.
 *
 * FNV hashes are designed to be fast while maintaining a hashValue.
 * collision rate. The FNV speed alhashValue.s one to quickly hash lots
 * of data while maintaining a reasonable collision rate.  See:
 *
 *      http://www.isthe.com/chongo/tech/comp/fnv/index.html
 *
 * for more details as well as other forms of the FNV hash.
 ***
 *
 * Please do not copyright this code.  This code is in the public domain.
 *
 * LANDON CURT NOLL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO
 * EVENT SHALL LANDON CURT NOLL BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
 * USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 * By:
 *    chongo <Landon Curt Noll> /\oo/\
 *      http://www.isthe.com/chongo/
 *
 * Share and Enjoy!    :-)
 */

// ***********************************************************************************************************
// Includes
#include "rwsharedheaders.hpp"
#include <limits.h>
#include <stdlib.h>

#include "rw/core/hash/hash.h"


// ***********************************************************************************************************
// Typedefs


// ***********************************************************************************************************
// Defines + Enums + Consts

namespace
{

// 32 bit magic FNV-1 prime
const uint32_t rw_32_PRIME(0x01000193u);

// 64 bit magic FNV-1 prime
const uint64_t rw_64_PRIME(RWUINT64CONST(0x100000001b3));

} // namespace

namespace rw
{

const uint32_t RWHASH32_INIT(0x811c9dc5u);
const uint64_t RWHASH64_INIT(RWUINT64CONST(0xcbf29ce484222325));

// ***********************************************************************************************************
// Static Variables + Static Data Member Definitions


// ***********************************************************************************************************
// Structs + Unions + Classes


// ***********************************************************************************************************
// Static Functions


// ***********************************************************************************************************
// External Functions

/**
Performs a 32 bit Fowler/Noll/Vo hash on a 32 bit unsigned int.

\param integer      The 32 bit unsigned int to hash.
\param hashValue    Previous hash value.

\note   To use the recommended 32 bit FNV-1 hash, use RWHASH32_INIT as the hash value
        argument.

\return 32 bit hash of the buffer.
*/
uint32_t
RwHash32UInt32(uint32_t integer,
               uint32_t hashValue)
{
    rwFUNCTION();

    // FNV-1 hash each byte in the int, do this in an endian portable way
    // for each byte multiply the previous hash value by the 32 bit FNV magic prime mod 2^32
    //               and xor the bottom with the current byte
    hashValue *= rw_32_PRIME;
    hashValue ^= (uint32_t)(integer & 0xff);
    integer >>= 8;
    hashValue *= rw_32_PRIME;
    hashValue ^= (uint32_t)(integer & 0xff);
    integer >>= 8;
    hashValue *= rw_32_PRIME;
    hashValue ^= (uint32_t)(integer & 0xff);
    integer >>= 8;
    hashValue *= rw_32_PRIME;
    hashValue ^= (uint32_t)(integer & 0xff);

    rwRETURN(hashValue);
}


/**
Performs a 32 bit Fowler/Noll/Vo hash on a buffer.

\param buffer       Start of buffer to hash.
\param length       Length in bytes of the buffer to hash.
\param hashValue    Previous hash value.

\note   To use the recommended 32 bit FNV-1 hash, use RWHASH32_INIT as the hash value
        argument.

\return 32 bit hash of the buffer.
*/
uint32_t
RwHash32Buffer(const uint8_t *buffer,
               size_t length,
               uint32_t hashValue)
{
    const uint8_t *bufferEnd = buffer + length;        // beyond end of buffer

    rwFUNCTION();
    rwASSERT(buffer != NULL);
    rwASSERT(length > 0);

    // FNV-1 hash each byte in the buffer
    while (buffer < bufferEnd)
    {
        // multiply by the 32 bit FNV magic prime mod 2^32
        hashValue *= rw_32_PRIME;

        // xor the bottom with the current byte
        hashValue ^= (uint32_t)(*buffer++);
    }

    rwRETURN(hashValue);
}


/**
Perform a 32 bit Fowler/Noll/Vo hash on a string.

\param str          String to hash.
\param hashValue    Previous hash value.

\note   To use the recommended 32 bit FNV-1 hash, use RWHASH32_INIT as the hash value
        argument.

\return 32 bit hash of the string.
*/
uint32_t
RwHash32String(const RwChar *str,
               uint32_t hashValue)
{
    const uint8_t *s = (const uint8_t *)str;    // unsigned string

    rwFUNCTION();
    rwASSERT(str != NULL);

    // FNV-1 hash each byte in the buffer
    while (*s)
    {
        // multiply by the 32 bit FNV magic prime mod 2^32
        hashValue *= rw_32_PRIME;

        // xor the bottom with the current byte
        hashValue ^= (uint32_t)(*s++);
    }

    rwRETURN(hashValue);
}


/**
Perform a 64 bit Fowler/Noll/Vo hash on a buffer.

\param buffer       Start of buffer to hash.
\param length       Length in bytes of the buffer to hash.
\param hashValue    Previous hash value.

\note   To use the recommended 64 bit FNV-1 hash, use RWHASH64_INIT as the hash value
        argument.

\return 64 bit hash of the buffer.
*/
uint64_t
RwHash64Buffer(const uint8_t *buffer,
               size_t length,
               uint64_t hashValue)
{
    const uint8_t *bufferEnd = buffer + length;        // beyond end of buffer

    rwFUNCTION();
    rwASSERT(buffer != NULL);
    rwASSERT(length > 0);

    // FNV-1 hash each byte of the buffer
    while (buffer < bufferEnd)
    {
        // multiply by the 64 bit FNV magic prime mod 2^64
        hashValue *= rw_64_PRIME;

        // xor the bottom with the current byte
        hashValue ^= (uint64_t)(*buffer++);
    }

    rwRETURN(hashValue);
}


/**
Perform a 64 bit Fowler/Noll/Vo hash on a char array string.

\param str          String to hash.
\param hashValue    Previous hash value.

\note   To use the recommended 64 bit FNV-1 hash, use RWHASH64_INIT as the hash value
        argument.

\return 64 bit hash of the string.
*/
uint64_t
RwHash64String(const RwChar *str,
               uint64_t hashValue)
{
    const uint8_t *s = (const uint8_t *)str;    // unsigned string

    rwFUNCTION();
    rwASSERT(str != NULL);

    // FNV-1 hash each byte of the string
    while (*s)
    {
        // multiply by the 64 bit FNV magic prime mod 2^64
        hashValue *= rw_64_PRIME;

        // xor the bottom with the current byte
        hashValue ^= (uint64_t)(*s++);
    }

    rwRETURN(hashValue);
}

} // namespace rw

// ***********************************************************************************************************
// Class Member Functions
