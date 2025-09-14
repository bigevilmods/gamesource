// (c) Electronic Arts. All Rights Reserved.

/**
\file
Define the major, minor and patch versions.
*/

#ifndef RW_CORE_FILESYS_VERSION_H
#define RW_CORE_FILESYS_VERSION_H

// Define the major, minor and patch versions.
// This information is updated with each release.

//! This define indicates the major version number for the filesys package.
//! \sa RW_FILESYS_VERSION
#define RW_FILESYS_VERSION_MAJOR   1
//! This define indicates the minor version number for the filesys package.
//! \sa RW_FILESYS_VERSION
#define RW_FILESYS_VERSION_MINOR   25
//! This define indicates the patch version number for the filesys package.
//! \sa RW_FILESYS_VERSION
#define RW_FILESYS_VERSION_PATCH   1

/*!
 * This is a utility macro that users may use to create a single version number
 * that can be compared against RW_FILESYS_VERSION.
 *
 * For example:
 *
 * \code
 *
 * #if RW_FILESYS_VERSION > RW_FILESYS_CREATE_VERSION_NUMBER( 1, 1, 0 )
 * printf("Filesys version is greater than 1.1.0.\n");
 * #endif
 *
 * \endcode
 */
#define RW_FILESYS_CREATE_VERSION_NUMBER( major_ver, minor_ver, patch_ver ) \
    ((major_ver) * 1000000 + (minor_ver) * 1000 + (patch_ver))

/*!
 * This macro is an aggregate of the major, minor and patch version numbers.
 * \sa RW_FILESYS_CREATE_VERSION_NUMBER
 */
#define RW_FILESYS_VERSION \
    RW_FILESYS_CREATE_VERSION_NUMBER( RW_FILESYS_VERSION_MAJOR, RW_FILESYS_VERSION_MINOR, RW_FILESYS_VERSION_PATCH )

#endif // RW_FILESYS_VERSION_H
