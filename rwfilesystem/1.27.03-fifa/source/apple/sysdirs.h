// (c) Electronic Arts. All Rights Reserved.

#ifndef RWFS_APPLE_SYSDIRS_H
#define RWFS_APPLE_SYSDIRS_H

#include "EABase/eabase.h"

namespace rw {
namespace core {
namespace filesys {

int appleGetTempDirectory(char16_t* pDirectory, uint32_t nMaxPermittedLength);
int appleGetTempDirectory(char8_t* pDirectory, uint32_t nMaxPermittedLength);
int appleGetHomeDirectory(char16_t* pDirectory, uint32_t nMaxPermittedLength);
int appleGetHomeDirectory(char8_t* pDirectory, uint32_t nMaxPermittedLength);
int appleGetAppName(char16_t* pAppName, uint32_t nMaxPermittedLength);
int appleGetAppName(char8_t* pAppName, uint32_t nMaxPermittedLength);

}}}

#endif
