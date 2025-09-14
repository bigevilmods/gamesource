// (c) Electronic Arts. All Rights Reserved.

#include <EAAssert/eaassert.h>
#include <EAStdC/EAString.h>
#include <EAStdC/EASprintf.h>
#include "rw/core/filesys/async.h"

namespace rw {
namespace core {
namespace filesys {

#if defined(EA_PLATFORM_PS3)

/*!
    This function convert a portable device name (e.g. hdd0:) to a native
    PS3 file name (/dev_hdd0).  It also converts any backslashes in the
    name to forward slashes.  Recognised device names are host0, host1,
    hdd0, hdd1, and bdvd.  Note that if the device name is not recognised,
    the output buffer will not be touched and if asserts are enabled, an
    assert will be thrown.

    \importlib rwfilesystem

    \return A pointer to the destination buffer pointer, so the result may be
    chained to other string manipulation functions (eg: EA::StdC::Strlcat).

    \param HostNameBuf A buffer to copy the file name into.  Must be
    FILESYS_MAX_PATH characters in size.
    \param RwFileName The original file name that may have a portable device
    name and/or backslashes.
*/
    //
char *ConvertHostNameToNativeName(char *HostNameBuf, const char *RwFileName)
{
    EA_ASSERT(HostNameBuf != NULL);
    EA_ASSERT(RwFileName != NULL);

    const int MOUNTTBLSIZE = 5;
    
    struct HostToMount
    {
        const char *host;
        const char *mountPoint;
        int hostNameLen;
    } HostToMountTable[MOUNTTBLSIZE] =
    {
        { "host1:", "/app_home/", 6},
        { "bdvd:",  "/dev_bdvd/", 5},
        { "hdd0:",  "/dev_hdd0/", 5},
        { "hdd1:",  "/dev_hdd1/", 5},
        { "host0:", "/app_home/", 6}, // legacy name
    };

    // first, see if this path already is already in a recognized native format
    const size_t mountPointNameLen(9);

    bool isUnix = (RwFileName[0] == '/');
    for (int i = 0; i < MOUNTTBLSIZE; ++i)
    {
        if (!EA::StdC::Strncmp(RwFileName, HostToMountTable[i].mountPoint, mountPointNameLen))
        {
            isUnix = true;
            break;
        }
    }

    if (isUnix)
    {
        // got a match, just copy the name as-is into the HostNameBuf
        char *d = HostNameBuf;

        while (*RwFileName)
        {
            // during the copy, check for any backslash
            // characters and convert them to forward-slash.
            *d++ = *RwFileName++;
            if (*(d - 1) == '\\')
                *(d - 1) = '/';
        }
        *d = '\0';

        return HostNameBuf;
    }

    // The name uses a portable (non-native) path.
    char *d = HostNameBuf;
    for (int i = 0; i < MOUNTTBLSIZE; ++i)
    {
        if (!EA::StdC::Strncmp(RwFileName, HostToMountTable[i].host, HostToMountTable[i].hostNameLen))
        {
            // skip over device name (Host1:)
            const char *s = RwFileName + HostToMountTable[i].hostNameLen;

            if ((*s == '/') || (*s == '\\'))            // skip initial directory slash
                ++s;

            EA::StdC::Snprintf(d, FILESYS_MAX_PATH, "%s%s", HostToMountTable[i].mountPoint, s);
            while (*d)
            {
                if (*d == '\\')
                    *d = '/';
                ++d;
            }

            return HostNameBuf;
        }
    }

    return NULL;
}
#endif

}}}
