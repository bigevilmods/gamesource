// (c) Electronic Arts. All Rights Reserved.

#include "EABase/eabase.h"

namespace rw {
namespace core {
namespace filesys {

size_t StrlcpyUTF16ToUTF8(char8_t* pDest, size_t nDestLength, const char16_t* pSrc, size_t nSrcLength)
{
    size_t destCount = 0;

    while (nSrcLength-- > 0)
    {
        uint32_t c = *pSrc++;   // Deal with surrogate characters

        // Encode as UTF-8
        if (c < 0x00000080u)
        {
            if (c == 0) // Break on NULL char, even if explicit length was set
                break;

            if (pDest && destCount + 1 < nDestLength)
            {
                *pDest++ = static_cast<char8_t>(c);
            }
            destCount += 1;
        }
        else if (c < 0x00000800u)
        {
            if (pDest && destCount + 2 < nDestLength)
            {
                *pDest++ = static_cast<char8_t>((c >> 6) | 0xc0);
                *pDest++ = static_cast<char8_t>((c | 0x80) & 0xbf);
            }
            destCount += 2;
        }
        else if (c < 0x00010000u)
        {
            if (pDest && destCount + 3 < nDestLength)
            {
                *pDest++ = static_cast<char8_t>((c >> 12) | 0xe0);
                *pDest++ = static_cast<char8_t>(((c >>  6) | 0x80) & 0xbf);
                *pDest++ = static_cast<char8_t>((c | 0x80) & 0xbf);
            }
            destCount += 3;
        }
        else if (c < 0x00200000u)
        {
            if (pDest && destCount + 4 < nDestLength)
            {
                *pDest++ = static_cast<char8_t>((c >> 18) | 0xf0);
                *pDest++ = static_cast<char8_t>(((c >> 12) | 0x80) & 0xbf);
                *pDest++ = static_cast<char8_t>(((c >>  6) | 0x80) & 0xbf);
                *pDest++ = static_cast<char8_t>((c | 0x80) & 0xbf);
            }
            destCount += 4;
        }
        else
        {
            const uint32_t kIllegalUnicodeChar = 0x0000fffd;
            if (pDest && destCount + 3 < nDestLength)
            {
                *pDest++ = static_cast<char8_t>((kIllegalUnicodeChar >> 12) | 0xe0);
                *pDest++ = static_cast<char8_t>(((kIllegalUnicodeChar >>  6) | 0x80) & 0xbf);
                *pDest++ = static_cast<char8_t>((kIllegalUnicodeChar | 0x80) & 0xbf);
            }
            destCount += 3;
        }
    }

    if (pDest && nDestLength != 0)
        *pDest = 0;

    return destCount;
}

} } }
