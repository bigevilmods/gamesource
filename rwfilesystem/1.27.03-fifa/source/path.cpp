// (c) Electronic Arts. All Rights Reserved.

#include "EAAssert/eaassert.h"
#include "rw/core/filesys/path.h"
#include "rw/core/filesys/constants.h"
#include "EAStdC/EAString.h"
#include "EAStdC/EAMemory.h"
#include "EAStdC/EACType.h"

namespace rw {
namespace core {
namespace filesys {

#if defined EA_PLATFORM_MICROSOFT
static const char PREFERRED_DIRECTORY_SEPARATOR = '\\';
#else
static const char PREFERRED_DIRECTORY_SEPARATOR = '/';
#endif

static const char VOLUME_SEPARATOR = ':';
static const char FILE_EXTENSION_SEPARATOR = '.';
static const char DIRECTORY_SEPARATORS[] = { '\\', '/' };
static const size_t NUM_DIRECTORY_SEPARATORS = sizeof DIRECTORY_SEPARATORS / sizeof DIRECTORY_SEPARATORS[0];


static inline bool IsFileExtensionSeparator(char c)
{
    return c == FILE_EXTENSION_SEPARATOR;
}

static inline char IdentityTransform(char c)
{
    return c;
}

static inline char CaseTransform(char c)
{
    return static_cast<char>(EA::StdC::Tolower(c));
}

/*!
    \brief
    Note: Does not guarantee that the root length includes the trailing
    directory separator
*/
static size_t GetUNCRootLength(const char *path, size_t pathLength)
{
    bool encounteredSeparator = false;
    size_t lastIndex;
    for (lastIndex = 2; lastIndex < pathLength; ++lastIndex)
    {
        if (IsDirectorySeparator(path[lastIndex]))
        {
            if (encounteredSeparator)
                break;

            encounteredSeparator = true;
        }
    }

    return lastIndex;
}


static size_t GetRootLength(const char *path, size_t pathLength)
{
    EA_ASSERT(path != NULL);

    if (pathLength == 0)
        return 0;

    // "\foo", "/foo" -> "foo"
    if (pathLength > 0 && IsDirectorySeparator(path[0]))
    {
        // "\\x\y\foo" -> "foo"
        // "\\x\y", "\\x" -> ""
        if (pathLength > 1 && IsDirectorySeparator(path[1]))
        {
            return GetUNCRootLength(path, pathLength);
        }
        
#if defined (EA_PLATFORM_PS3)
        // PS3 Special Case: Specifically, in order to support the PS3 device names (/app_home),
        // the path root has to be the first directory after the directory separator.
        else if (pathLength > 1)
        {
            const char *afterFirstSep = path + 1;
            const char *nextSep = afterFirstSep;
            while (*nextSep != '\0' && !IsDirectorySeparator(*nextSep))
                nextSep++;
            if (*nextSep == '\0')
            {
                // This is a path that is similar to /app_home (i.e. without a trailing separator)
                // Simply select the last character instead
                nextSep--;
            }

            ssize_t length = nextSep - path + 1;
            return static_cast<size_t>(length);
        }
#endif

        return 1;
    }

    // "C:foo" -> "foo"
    // "C:\" -> ""
    // "C:\foo" -> "foo"
    // "memmap:\foo"
    // "memmap:/foo"
    const char *device = EA::StdC::Strnchr(path, VOLUME_SEPARATOR, pathLength);

    if (pathLength > 1 && device != NULL)
    {
        if (IsDirectorySeparator(*(device + 1)))
            device++;

        ptrdiff_t indexOfDevice = device - path;

        return static_cast<size_t>(indexOfDevice + 1); // length = index + 1
    }

    return 0;
}


static inline const char *SetEmptyDirectory(char *directory)
{
    directory[0] = 0;
    return directory;
}

static const char *GetFromLastDirectorySeparator(const char *filePath)
{
    ssize_t fileNameLength = static_cast<ssize_t>(EA::StdC::Strlen(filePath));
    const char *position = filePath + fileNameLength - 1;

    for (ssize_t i = fileNameLength - 1; i >= 0; i--)
    {
        if (IsDirectorySeparator(*position))
        {
            break;
        }

        position--;
    }

    // Move past the separator
    position++;
    return position;
}

static bool IsDotDirectory(const char *directoryName)
{
    if (IsFileExtensionSeparator(*directoryName))
    {
        directoryName++;

        if (IsFileExtensionSeparator(*directoryName))
            directoryName++;

        if (*directoryName == 0 || IsDirectorySeparator(*directoryName))
            return true;
    }

    return false;
}

char GetDirectorySeparator()
{
    return PREFERRED_DIRECTORY_SEPARATOR;
}

bool IsDirectorySeparator(char value)
{
    for (size_t i = 0; i < NUM_DIRECTORY_SEPARATORS; ++i)
        if (value == DIRECTORY_SEPARATORS[i])
            return true;
    return false;
}

bool PathIsRooted(const char *path)
{
    // GetRootLength checks for Unix(/), UNC(\\) and Windows (C:\). It will
    // return an integer >0 if a root exists.
    size_t rootLength = GetRootLength(path, EA::StdC::Strlen(path));
    return rootLength > 0;
}

const char *GetPathDirectory(const char *filePath, char *directoryPath, size_t directoryPathCapacity)
{
    size_t pathLength = EA::StdC::Strlen(filePath);
    EA_ASSERT(directoryPathCapacity > pathLength);
    EA_ASSERT(pathLength >= 1);
    
    ssize_t rootLength = static_cast<ssize_t>(GetRootLength(filePath, pathLength));

    ssize_t lastSeparator = static_cast<ssize_t>(pathLength - 1);
    for (; lastSeparator > rootLength; --lastSeparator)
        if (IsDirectorySeparator(filePath[lastSeparator]))
            break;

    if (lastSeparator < 0 || lastSeparator < rootLength)
        return SetEmptyDirectory(directoryPath);

    EA::StdC::Memcpy(directoryPath, filePath, static_cast<size_t>(lastSeparator));

    directoryPath[lastSeparator] = 0;
    return PathNormalize(directoryPath, directoryPath, directoryPathCapacity);
}


const char *GetPathFileName(const char *filePath)
{
    ssize_t fileNameLength = static_cast<ssize_t>(EA::StdC::Strlen(filePath));
    for (ssize_t i = fileNameLength - 1; i >= 0; --i)
        if (IsDirectorySeparator(filePath[i]))
            return filePath + i + 1;

    // If no directory separators were found, then filePath must be a file itself.
    if (fileNameLength > 0)
        return filePath;
    else
        return 0;
}

const char *GetPathFileNameExtension(const char *filePath)
{
    const char *fromLastSeparator = GetFromLastDirectorySeparator(filePath);

    if (!IsDotDirectory(fromLastSeparator))
        return EA::StdC::Strrchr(fromLastSeparator, FILE_EXTENSION_SEPARATOR);
    else
        return NULL;
}

const char *GetPathRoot(const char *pathSource, char *pathDestination, size_t pathDestinationCapacity)
{
    EA_ASSERT_MSG(pathSource != NULL, "Source string cannot be null.");
    EA_ASSERT_MSG(pathDestination != NULL, "Destination string cannot be null.");

    size_t rootLength = GetRootLength(pathSource, EA::StdC::Strlen(pathSource));

    if (rootLength >= pathDestinationCapacity)
        return NULL;

    EA::StdC::Strncpy(pathDestination, pathSource, rootLength);
    pathDestination[rootLength] = '\0';
    
    return PathNormalize(pathDestination, pathDestination, pathDestinationCapacity);
}

const char *PathCombine(const char *pathPrefix, const char *pathSuffix, char *pathDestination, size_t pathDestinationCapacity)
{
    EA_ASSERT_MSG(pathPrefix != NULL, "Path prefix cannot be null.");
    EA_ASSERT_MSG(pathSuffix != NULL, "Path suffix cannot be null.");
    EA_ASSERT_MSG(pathDestination != NULL, "Path destination cannot be null.");

    size_t prefixLength = EA::StdC::Strlen(pathPrefix);
    size_t suffixLength = EA::StdC::Strlen(pathSuffix);
    EA_ASSERT(prefixLength + suffixLength + 1 < pathDestinationCapacity);

    size_t prefixLengthToCopy = prefixLength;
    size_t suffixStartIndex = 0;

    // Case: Suffix is rooted or Prefix is empty
    //       then return Suffix
    
    bool suffixIsRoot = PathIsRooted(pathSuffix);

    if (suffixIsRoot || (prefixLength == 0))
    {
        EA_STRLCPY_VERIFY(pathDestination, pathSuffix, pathDestinationCapacity);
        return PathNormalize(pathDestination, pathDestination, pathDestinationCapacity);
    }

    // TODO: Use GetRootLength
    // Case: Prefix is a drive reference.
    bool prefixIsDriveReference = prefixLength == 2 && EA::StdC::Isalpha(pathPrefix[0]) && pathPrefix[1] == ':';

    if (!prefixIsDriveReference)
    {
        // Case: Prefix ends with a directory separator
        //       Ignore these characters (because one will be added later)
        while (IsDirectorySeparator(pathPrefix[prefixLengthToCopy - 1]))
            prefixLengthToCopy--;
    }

    // Case: Suffix starts with directory separator
    //       Ignore these characters (because one will already have been added)
    while (IsDirectorySeparator(pathSuffix[suffixLength - suffixStartIndex]))
        suffixStartIndex++;

    // Copy both strings into pathDestination
    EA::StdC::Memmove(pathDestination, pathPrefix, prefixLength + 1);
    pathDestination[prefixLengthToCopy] = PREFERRED_DIRECTORY_SEPARATOR;
    
    EA_STRLCPY_VERIFY(pathDestination + prefixLengthToCopy + 1, pathSuffix + suffixStartIndex, pathDestinationCapacity - prefixLengthToCopy - 1); //+1 for dir seperator

    return PathNormalize(pathDestination, pathDestination, pathDestinationCapacity);
}


const char *PathNormalize(const char *sourcePath, char *destinationPath, size_t destinationPathCapacity)
{
    const size_t sourceLength = EA::StdC::Strlen(sourcePath);
    EA_ASSERT(sourceLength < destinationPathCapacity);
    (void)destinationPathCapacity;

    size_t i = 0;
    for (; i < sourceLength; ++i)
        destinationPath[i] = IsDirectorySeparator(sourcePath[i]) ? PREFERRED_DIRECTORY_SEPARATOR : sourcePath[i];
    destinationPath[i] = 0;

    return destinationPath;
}


static inline bool MustRewindDirectory(const char *path)
{
    return path[0] == '.' && path[1] == '.';
}


static inline bool MustSkipDirectory(const char *path)
{
    return path[0] == '.' && (IsDirectorySeparator(path[1]) || path[1] == 0);
}


static inline char *RewindDirectory(char * const start, char *current)
{
    // For cases like "C:\temp\", this will bail early as current will point past the 
    // separator.
    if (*current == 0 && IsDirectorySeparator(*(current - 1)))
        current -= 2;

    for (; ; --current)
        if (IsDirectorySeparator(*current))
        {
            ++current;
            break;
        }

    return (current < start) ? start : current;
}


const char *PathSimplify(const char *pathSource, char *pathDestination, size_t pathDestinationCapacity)
{
    // The path source is copied into this temporary buffer as we destructively modify it. 
    // According to the interface, pathDestination and pathSource are allowed to alias and 
    // so modifying pathDestination destructively could lead to unexpected results.
    
    char source[FILESYS_MAX_PATH] = {};
    EA_STRLCPY_VERIFY(source, pathSource, EAArrayCount(source));

    size_t pathSourceLength = EA::StdC::Strlen(source);
    EA_ASSERT(pathSourceLength < pathDestinationCapacity);
 
    const ssize_t rootLength = static_cast<ssize_t>(GetRootLength(source, pathSourceLength));
    if (rootLength > 0)
        EA::StdC::Memcpy(pathDestination, source, static_cast<size_t>(rootLength));
    ssize_t toIndex = rootLength < 0 ? 0 : rootLength;
    char *to = pathDestination + toIndex;
    char * const start = to;
    const char *from = source + toIndex;
    const char * const end = source + pathSourceLength;

    for (; from < end;)
    {
        if (MustRewindDirectory(from))
        {
            // If we have a situation like "..\" or "C:.." we cannot perform
            // a simplification operation so we just copy the periods.
            // However, C:\.., C:\..\..\, etc.. are valid operations and should result
            // in C:\,
            if (to == start)
            {
                if (rootLength == 0)
                {
                    *to++ = '.';
                    *to++ = '.';
                    *to++ = PREFERRED_DIRECTORY_SEPARATOR;
                }
            }
            else
            {
                to = RewindDirectory(start, to);
            }

            from += 3;
            *to = 0;
        }
        else if (MustSkipDirectory(from))
        {
            // Delete the directory separator before the "." since we are staying on the same directory
            if (to != start)
            {
                to--;
                *to = 0;
            }
            // Unless this is the first character (i.e. in a situation like .\foo)
            else
            {
                if (rootLength == 0)
                {
                    *to = '.';
                    to++;
                }
                else if (*(from - 1) == ':')
                {
                    // this will come up when attempting to get the current working 
                    // directory by ommiting the leading slash:
                    // e.g. C:.\\dir
                    
                    // check if the next character is a directory seperator, if so,
                    // skip over it as well to get C:dir
                    if (IsDirectorySeparator(*(from + 1)))
                    {
                        ++from;
                    }
                }
                else
                {
                    // This will occur when the "." is encountered here:
                    // D:\.\dir for example. In this case, ignore the previous backslash
                    to--;
                    *to = 0;
                }

            }

            from++;
            *to = 0;
        }
        else
        {
            *to = IsDirectorySeparator(*from) ? PREFERRED_DIRECTORY_SEPARATOR : *from;

            to++;
            from++;

            *to = 0;
        }
    }

    *to = 0;
    return PathNormalize(pathDestination, pathDestination, pathDestinationCapacity);
}


template<typename T>
static bool MatchesWildcardPatternInternal(const char *string, const char *pattern, T transform, int matchflags)
{
    const char cSeparator = (matchflags & MatchFlag_DosPath) ? '\\' : '/';
    const char* const stringBegin = string;
    char test;

    for (;;)
    {
        const char s = transform(*string);
        const char p = transform(*pattern);

        // If the pattern and the string match exact.
        if (s == p) 
        {
            if (s == 0)
                return true;

            ++string;
            ++pattern;
            continue;
        }

        // If the pattern contains a '?' match any single character.
        if (*string && p == '?')
        {
            ++string;
            ++pattern;
            continue;
        }

        // If the pattern contains a wildcard...
        if (p == '*') 
        {
            char c = *pattern;
            
            // move past wildcards in the pattern...
            while(c == '*')
                c = *++pattern;

            if((*string == '.') && 
                    (matchflags & MatchFlag_Period) &&
                    ((string == stringBegin) || ((matchflags & MatchFlag_Pathname) && (string[-1] == cSeparator))))
            {
                return false;
            }

            // If we hit the end of the pattern...
            if(c == '\0')
            {
                if(matchflags & MatchFlag_Pathname)
                    return (matchflags & MatchFlag_LeadingDir) || !EA::StdC::Strchr(string, cSeparator);
                else
                    return true;
            }
            // If we hit '*' and the flag is set that we don't see past directory seperators and we've hit a directory seperator in the pattern...  
            else if((c == cSeparator) && (matchflags & MatchFlag_Pathname))
            {
                string = EA::StdC::Strchr(string, cSeparator);

                if(string == NULL)
                    return false;            
            }

            // If we haven't hit the end of the string...
            while((test = *string) != '\0') 
            {                
                if(MatchesWildcardPatternInternal(string, pattern, transform, (matchflags & ~MatchFlag_Period)))
                    return true;

                // bail if we hit a directory separator and that feature is enabled.
                if((test == cSeparator) && (matchflags & MatchFlag_Pathname))
                    break;

                ++string;
            }

            return false;
        }

        return false;
    }
}



bool MatchesWildcardPattern(const char *string, const char *pattern, int matchflags)
{
    return MatchesWildcardPatternInternal(string, pattern, IdentityTransform, matchflags);
}


bool NoCaseMatchesWildcardPattern(const char *string, const char *pattern, int matchflags)
{
    return MatchesWildcardPatternInternal(string, pattern, CaseTransform, matchflags);
}
    


}}}

