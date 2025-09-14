// (c) Electronic Arts. All Rights Reserved.

/// \file
/// host namespace implementation for PC.

#include <fcntl.h>
#include <sys/stat.h>
#include <io.h>
#include <stdio.h>      // fprintf, stdout

#pragma warning(push)
#pragma warning(disable: 4365) // conversion from 'LONG64' to 'unsigned __int64', signed/unsigned mismatch
#include <windows.h>
#pragma warning(pop)


#include "rw/core/debug/host.h"
#include "rw/core/base/ostypes.h"
#include "rw/core/debug/debug.h"

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
    int permission = 0;

    if((mode & HOST_READONLY) != 0)
        openMode |= _O_RDONLY;
    if((mode & HOST_WRITEONLY) != 0)
        openMode |= _O_WRONLY;
    if((mode & HOST_READWRITE) != 0)
        openMode |= _O_RDWR;
    if((mode & HOST_APPEND) != 0)
        openMode |= _O_APPEND;
    if((mode & HOST_CREATE) != 0)
    {
        openMode |= _O_CREAT;
        permission = _S_IWRITE;
    }
    if((mode & HOST_TRUNCATE) != 0)
        openMode |= O_TRUNC;
    if((mode & HOST_BINARY) != 0)
        openMode |= _O_BINARY;

    if(permission != 0)
        rwRETURN(static_cast<int>(_open(path, openMode, permission)));
    else
        rwRETURN(static_cast<int>(_open(path, openMode)));
}

void Close(int handle)
{
    rwFUNCTION();
    _close(handle);
    rwRETURNVOID();
}

size_t Read(int handle, void *p, size_t count)
{
    rwFUNCTION();
    rwRETURN(static_cast<size_t>(_read(handle, p, static_cast<unsigned int>(count))));
}

size_t Write(int handle, const void *p, size_t count)
{
    rwFUNCTION();
    rwRETURN(static_cast<size_t>(_write(handle, p, static_cast<unsigned int>(count))));
}

void PrintDebugString(const char *s)
{
    rwFUNCTION();
    if (IsDebuggerPresent() != 0)
    {
#if defined(EA_PLATFORM_XBOXONE)
		OutputDebugStringA(s);
#else
        OutputDebugString(s);
#endif
    }
    else
    {
        fputs(s, stderr);
    }
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
