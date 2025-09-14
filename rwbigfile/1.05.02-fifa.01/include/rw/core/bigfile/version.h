// (c) Electronic Arts. All Rights Reserved.

/**
\file
Define the major, minor and patch versions.
*/

#ifndef RW_BIGFILE_VERSION_H
#define RW_BIGFILE_VERSION_H

#if defined _MSC_VER
#pragma once
#endif

// Define the major, minor and patch versions.
// This information is updated with each release.

//! This define indicates the major version number for the bigfile package.
//! \sa RW_BIGFILE_VERSION
#define RW_BIGFILE_VERSION_MAJOR   1
//! This define indicates the minor version number for the bigfile package.
//! \sa RW_BIGFILE_VERSION
#define RW_BIGFILE_VERSION_MINOR   5
//! This define indicates the patch version number for the bigfile package.
//! \sa RW_BIGFILE_VERSION
#define RW_BIGFILE_VERSION_PATCH   2

/*!
 * This is a utility macro that users may use to create a single version number
 * that can be compared against RW_BIGFILE_VERSION.
 *
 * For example:
 *
 * \code
 *
 * #if RW_BIGFILE_VERSION > RW_BIGFILE_CREATE_VERSION_NUMBER( 1, 1, 0 )
 * printf("Bigfile version is greater than 1.1.0.\n");
 * #endif
 *
 * \endcode
 */
#define RW_BIGFILE_CREATE_VERSION_NUMBER( major_ver, minor_ver, patch_ver ) \
    ((major_ver) * 1000000 + (minor_ver) * 1000 + (patch_ver))

/*!
 * This macro is an aggregate of the major, minor and patch version numbers.
 * \sa RW_BIGFILE_CREATE_VERSION_NUMBER
 */
#define RW_BIGFILE_VERSION \
    RW_BIGFILE_CREATE_VERSION_NUMBER( RW_BIGFILE_VERSION_MAJOR, RW_BIGFILE_VERSION_MINOR, RW_BIGFILE_VERSION_PATCH )

#endif // RW_BIGFILE_VERSION_H
