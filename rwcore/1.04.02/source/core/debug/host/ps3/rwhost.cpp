// (c) Electronic Arts. All Rights Reserved.

/// \file
/// host namespace implementation for PS3.

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
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

static char *mount_point = "/app_home/";

// convert user host file name to mount point name
static void PS3HostName(char *Hostnamebuf, const char *name)
{
    rwFUNCTION();
    const char *s = strchr(name,':');    // skip over device name (Host:)
    if (s)
        ++s;
    else
        s = name;
    if (*s=='/' || *s=='\\')            // skip initial directory slash
        ++s;

    char *d = Hostnamebuf;
    strcpy(d,mount_point);
    d += strlen(d);
    while (*s)
    {
        *d = *s;
        if (*d=='\\')
            *d = '/';
        ++d;
        ++s;
    }
    *d = '\0';
    rwRETURNVOID();
}

#define DEBUG_MAX_PATH 512
int Open(const char *path, unsigned int mode)
{
    rwFUNCTION();
    int openMode = 0;
    int permission = 0;

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
        // NOTE:  If this isn't done, the file will be opened and marked as read-only the first time it's created.
        //        Then, the next time the user attempts to open the file when it already exists and is read-only, the open() call below
        //        will fail with an EEXIST (-29) error.  To get around this, we open the file with rwx privileges.
        //        Best I can tell, this is an octal number equivalent to
        //        CELL_FS_S_IRWXU | CELL_FS_S_IRWXG | CELL_FS_S_IRWXO (read, write, execute for user, group, other)
        //        See fs_external.h in the PlayStation3GCC package.
        permission = 0777;
    }
    if((mode & HOST_TRUNCATE) != 0)
        openMode |= O_TRUNC;


    char Hostnamebuf[DEBUG_MAX_PATH];
    PS3HostName(Hostnamebuf, path);

    if (0 != permission)
    {
        rwRETURN(static_cast<int>(open(Hostnamebuf, openMode, permission)));
    }
    else
    {
        rwRETURN(static_cast<int>(open(Hostnamebuf, openMode)));
    }
}

void Close(int handle)
{
    rwFUNCTION();
    close(handle);
    rwRETURNVOID();
}

unsigned int Read(int handle, void *p, unsigned int count)
{
    rwFUNCTION();
    rwRETURN(static_cast<unsigned int>(read(handle, p, count)));
}

unsigned int Write(int handle, const void *p, unsigned int count)
{
    rwFUNCTION();
    rwRETURN(static_cast<unsigned int>(write(handle, p, count)));
}


void PrintDebugString(const char *s)
{
    rwFUNCTION();
#if defined(__SPU__)
    spu_printf("%s", s);
#else
    fputs(s, stderr);
#endif
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
