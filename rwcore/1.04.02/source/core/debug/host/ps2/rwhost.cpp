// (c) Electronic Arts. All Rights Reserved.

/// \file
/// host namespace implementation for PS2.

#include "rw/core/base/ostypes.h"
#include "rw/core/debug/debug.h"

#include <sifdev.h>
#include <limits.h>
#include <string.h>
#include <stdio.h>
#if defined __GNUC__ && __GNUC__ >=3
#include <alloca.h>
#endif

#include "rw/core/debug/host.h"

namespace rw
{
namespace core
{
namespace debug
{
namespace host
{

int Open(const char *path, unsigned int mode)
{
    rwFUNCTION();
    int openMode = 0;

    if((mode & HOST_READONLY) != 0)
        openMode |= SCE_RDONLY;
    if((mode & HOST_WRITEONLY) != 0)
        openMode |= SCE_WRONLY;
    if((mode & HOST_READWRITE) != 0)
        openMode |= SCE_RDWR;
    if((mode & HOST_APPEND) != 0)
        openMode |= SCE_APPEND;
    if((mode & HOST_CREATE) != 0)
        openMode |= SCE_CREAT;
    if((mode & HOST_TRUNCATE) != 0)
        openMode |= SCE_TRUNC;

    // check if path starts with host0:, if not, prepend it
    char *actualPath = const_cast<char *>(path);
    const char *HOSTPATH = "host0:";
    if(strnicmp(HOSTPATH, path, 6) != 0)
    {
        // allocate space on stack to hold new path
        actualPath = static_cast<char *>(alloca(strlen(path)+6));
        // create new path
        strcpy(actualPath, HOSTPATH);
        strcat(actualPath, path);
    }

    rwRETURN(sceOpen(actualPath, openMode));
}

void Close(int handle)
{
    rwFUNCTION();
    sceClose(handle);
    rwRETURNVOID();
}

unsigned int Read(int handle, void *p, unsigned int count)
{
    rwFUNCTION();
    rwRETURN(sceRead(handle, p, count));
}

unsigned int Write(int handle, const void *p, unsigned int count)
{
    rwFUNCTION();
    rwRETURN(sceWrite(handle, p, count));
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
    rwBREAK();
    rwRETURNVOID();
}

}
}
}
}
