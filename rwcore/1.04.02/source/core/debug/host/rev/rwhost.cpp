// (c) Electronic Arts. All Rights Reserved.

/// \file
/// host namespace implementation for Wii.

#include <stddef.h>

#include "rw/core/debug/host.h"
#include "rw/core/base/ostypes.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <revolution/db.h>

namespace rw
{
namespace core
{
namespace debug
{
namespace host
{

int Open(const char *RWUNUSED(path), unsigned int RWUNUSED(mode))
{
    rwFUNCTION();
#if 1
    rwRETURN(0);
#else
    int openMode = 0;

    if((mode & HOST_READONLY) != 0)
        openMode |= O_RDONLY;
    if((mode & HOST_WRITEONLY) != 0)
        openMode |= O_WRONLY;
    if((mode & HOST_READWRITE) != 0)
        openMode |= O_RDWR;
    if((mode & HOST_APPEND) != 0)
        openMode |= O_APPEND;
    if((mode & HOST_CREATE) != 0)
    {
        openMode |= O_CREAT;
    }
    if((mode & HOST_TRUNCATE) != 0)
        openMode |= O_TRUNC;

    rwRETURN(static_cast<int>(open(path, openMode)));
#endif
}

void Close(int RWUNUSED(handle))
{
    rwFUNCTION();
    rwRETURNVOID();
}

size_t Read(int RWUNUSED(handle), void *RWUNUSED(p), size_t RWUNUSED(count))
{
    rwFUNCTION();
    rwRETURN(0);
}

size_t Write(int RWUNUSED(handle), const void *RWUNUSED(p), size_t RWUNUSED(count))
{
    rwFUNCTION();
    rwRETURN(0);
}


void PrintDebugString(const char *s)
{
    rwFUNCTION();
    fputs(s, stderr);
    rwRETURNVOID();
}

void BreakToDebugger()
{
    rwFUNCTION();
    OSHalt("Break");
    rwRETURNVOID();
}

}
}
}
}
