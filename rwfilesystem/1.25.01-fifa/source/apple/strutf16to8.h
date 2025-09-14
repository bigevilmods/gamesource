// (c) Electronic Arts. All Rights Reserved.

#ifndef RWFS_STRUTF16TO8_H
#define RWFS_STRUTF16TO8_H

namespace rw {
namespace core {
namespace filesys {

size_t StrlcpyUTF16ToUTF8(char8_t* pDest, size_t nDestLength, const char16_t* pSrc, size_t nSrcLength = (size_t)-1);

}}}

#endif

