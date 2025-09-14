// (c) Electronic Arts. All Rights Reserved.

/**
\file
Path functionality.
*/

#ifndef RW_CORE_FILESYS_PATH_H
#define RW_CORE_FILESYS_PATH_H

#include "rw/core/filesys/config.h"

namespace rw {
namespace core {
namespace filesys {
    /*!
        \brief Returns the parent directory of filePath. In most cases, returns all characters up to but not including
        the last directory separator.
        Will return an empty string if given a path root (C:\, /, or \\UNC\Path)
    */
    RWCORELIB_API const char *GetPathDirectory(const char *filePath, char *directoryPath, size_t directoryPathCapacity);

    /*!
        \brief Returns the file name and extension from filePath. Will return NULL
        if the path does not end with a file name.
    */
    RWCORELIB_API const char *GetPathFileName(const char *filePath);

    /*!
        \brief Returns the extension of the file specified by filePath. If the
        file contains no extension, NULL will be returned.
        It is expected that filePath is not a directory file (not C:\file.txt\ for example)
    */
    RWCORELIB_API const char *GetPathFileNameExtension(const char *filePath);

    /*!
        \brief GetPathRoot gets the root directory of a specific path.
        Note: GetPathRoot does not check if the root is valid or not.
        Specifically, it will see the root of "\\MachineName" as "\\MachineName"
        Note: GetPathRoot will not set a trailing directory separator with UNC paths.

        pathDestination is set to:
        - Empty string: if path is relative
        - "/": if path is a UNIX root (Note: A path is considered a UNIX root even on a Windows platform, even if it starts with a backslash)
        - "C:\"
        - "\\Network\ShareFolder (UNC path)
        \return pathDestination or NULL if pathDestinationCapacity is not big enough.
    */
    RWCORELIB_API const char *GetPathRoot(const char *pathSource, char *pathDestination, size_t pathDestinationCapacity);

    /*!
        \brief Converts all the directory separators in pathSource to the directory separator
        of the current platform.
        pathSource and pathDestination can point to the same location.
    */
    RWCORELIB_API const char *PathNormalize(const char *pathSource, char *pathDestination, size_t pathDestinationCapacity);

    /*!
        \brief PathCombine tries to append pathSuffix onto pathPrefix
        If pathPrefix is not a drive reference ("C:" or "D:") and does not end with a valid directory separator,
        a directory separator is appended to pathPrefix before concatenation.
        
        Note: If pathSuffix includes a root (C:\ or \\networklocation), pathSuffix is returned.
              If pathPrefix is empty, pathSuffix is returned
    */
    RWCORELIB_API const char *PathCombine(const char *pathPrefix, const char *pathSuffix, char *pathDestination, size_t pathDestinationCapacity);

    /*!
        \brief PathSimplify attempts to rewind the "." and ".." directories in a path string.

        PathSimplify will add a trailing directory separator only if the input path ends with a directory separator
    */
    RWCORELIB_API const char *PathSimplify(const char *pathSource, char *pathDestination, size_t pathDestinationCapacity);

    /*!
        \brief PathIsRooted returns true if path contains an absolute path.

        Note: PathIsRooted does not check or the root is valid. For example
        \\MachineName is still considered to be rooted (even though it is an invalid
        UNC path).
    */
    RWCORELIB_API bool PathIsRooted(const char *path);

    /*!
        \brief Returns the directory separator used by the current platform
    */
    RWCORELIB_API char GetDirectorySeparator();

    /*!
        \brief Checks if value is one of the possible directory separators.
        Note: Does not check for platform-specific seperators. It will return true
        for both (/) and (\) on all systems.
    */
    RWCORELIB_API bool IsDirectorySeparator(char value);

    /*!
        \brief Flags used to tune the behaviour of the 'MatchesWildcardPattern' functions below. 
			   These flags are a subset of the pattern matching flags enum in EAIO.  This accounts
			   for the discontinous numbering.
    */
    enum MatchFlags
    {
        MatchFlag_Pathname   = 0x01,  /// Wildcard don't can't see past directory separator chars.
        MatchFlag_Period     = 0x04,  /// A leading '.' is matched only explicitly and not by * or ?.
        MatchFlag_LeadingDir = 0x08,  /// Match a leading directory and allow ignoring after it.
        MatchFlag_DosPath    = 0x40,  /// Force the path to be interpreted as a DOS path (e.g. '\') instead of native path.
        MatchFlag_UnixPath   = 0x80   /// Force the path to be interpreted as a Unix path (e.g. '/') instead of native path.
    };

    /*!
        \brief Default Match Flag settings.
    */
#ifdef EA_PLATFORM_WINDOWS
    const int MatchFlags_Default = MatchFlag_DosPath;
#else
    const int MatchFlags_Default = MatchFlag_UnixPath;
#endif

    /*!
        \brief Performs a wildcard pattern match against the specified string.
    */
    RWCORELIB_API bool MatchesWildcardPattern(const char *string, const char *pattern, int matchflags = MatchFlags_Default);

    /*!
        \brief Performs a case insensitive wildcard pattern match against the specified string.
    */
    RWCORELIB_API bool NoCaseMatchesWildcardPattern(const char *string, const char *pattern, int matchflags = MatchFlags_Default);

}}}

#endif

