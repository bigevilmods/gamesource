// (c) Electronic Arts. All Rights Reserved.

/// \file
/// host namespace implementation for Xbox2.

#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <malloc.h> // alloca

#include <xtl.h>
#include <xbdm.h>

#include "rw/core/debug/host.h"
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

    // check if path starts with D:\ or net:\ if not, prepend it
    char *actualPath = const_cast<char *>(path);
    const char *HOSTPATH = "D:\\";
#if !EA_FINAL
    const char *HOSTPATH_DEVKIT = "E:\\";
#endif
    const char *REMOTEPATH = "net:\\";
    if((_strnicmp(HOSTPATH, path, 3) != 0) 
#if !EA_FINAL
    && (_strnicmp(HOSTPATH_DEVKIT, path, 3) != 0)
#endif
    && (_strnicmp(REMOTEPATH, path, 5) != 0))
    {
        // allocate space on stack to hold new path
        actualPath = static_cast<char *>(alloca(strlen(path)+4));
        // create new path
        strcpy(actualPath, HOSTPATH);
        strcat(actualPath, path);
    }

    if(permission != 0)
        rwRETURN(static_cast<int>(_open(actualPath, openMode, permission)));
    else
        rwRETURN(static_cast<int>(_open(actualPath, openMode)));
}

void Close(int handle)
{
    rwFUNCTION();
    _close(handle);
    rwRETURNVOID();
}

unsigned int Read(int handle, void *p, unsigned int count)
{
    rwFUNCTION();
    rwRETURN(static_cast<unsigned int>(_read(handle, p, count)));
}

unsigned int Write(int handle, const void *p, unsigned int count)
{
    rwFUNCTION();
    rwRETURN(static_cast<unsigned int>(_write(handle, p, count)));
}

void PrintDebugString(const char *s)
{
    rwFUNCTION();
    OutputDebugString(s);
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
