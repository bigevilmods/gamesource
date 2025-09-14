// (c) Electronic Arts. All Rights Reserved.

/**
\file
File system loading and saving.
*/

#ifndef RW_CORE_FILESYS_LOADSAVE_H
#define RW_CORE_FILESYS_LOADSAVE_H

#include "EABase/eabase.h"
#include "rw/core/filesys/config.h"

namespace rw {
namespace core {
namespace filesys {
    RWCORELIB_API void Save(const char *filePath, const void *addr, uint64_t bytes);
    RWCORELIB_API bool Delete(const char *filePath);
    RWCORELIB_API bool Move(const char *currentName, const char *newName);
}}}

#endif
/* RWPUBLICEND */
