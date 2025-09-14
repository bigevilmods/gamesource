// (c) Electronic Arts. All Rights Reserved.

#include <cstring>
#include "eastl/algorithm.h"

#include "bigfileindexentry.h"


BigFileIndexEntry::BigFileIndexEntry(const char *sourcePath, const char *destinationPath) :
    nameLen(0), pathLen(0), pathIndex(0), fileSize(0),
        zSize(0), hash(0), offset(0), flags(0)
{
    strncpy(srcpath, sourcePath, sizeof(srcpath));

    memset(savepath, 0, sizeof(savepath));
    memset(path, 0, sizeof(path));
    memset(name, 0, sizeof(name));

    strncpy(path, destinationPath, sizeof(path));
    eastl::replace(&path[0], &path[sizeof(path)], '\\', '/');
    strncpy(savepath, path, sizeof(savepath));

    char *lastDirectorySeparator = strrchr(path, '/');
    char *filename = NULL;

    if (lastDirectorySeparator)
    {
        *lastDirectorySeparator = 0;
        filename = lastDirectorySeparator + 1;

        strcpy(name, filename);
    }
    else
    {
        strcpy(name, destinationPath);
        strcpy(path, ".");
    }

    nameLen = strlen(name);
    pathLen = strlen(path);

    hash = CalculateHash(savepath);
}


uint64_t BigFileIndexEntry::CalculateHash(const char *str, uint64_t hash)
{
    const unsigned char *s = reinterpret_cast<const unsigned char *>(str);

    for (;;)
    {
        unsigned int c = (unsigned int) *s++;
        if (!c) break;
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }

    return hash;
}


