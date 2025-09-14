// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_CORE_VERSION_H
#define RW_CORE_VERSION_H

#if defined _MSC_VER
#pragma once
#endif

// Define the major, minor and patch versions.
// This information is updated with each release.

//! This define indicates the major version number for the core package.
//! \sa RW_CORE_VERSION
#define RW_CORE_VERSION_MAJOR   1
//! This define indicates the minor version number for the core package.
//! \sa RW_CORE_VERSION
#define RW_CORE_VERSION_MINOR   4
//! This define indicates the patch version number for the core package.
//! \sa RW_CORE_VERSION
#define RW_CORE_VERSION_PATCH   2

/*!
 * This is a utility macro that users may use to create a single version number
 * that can be compared against RW_CORE_VERSION.
 *
 * For example:
 *
 * \code
 *
 * #if RW_CORE_VERSION > RW_CORE_CREATE_VERSION_NUMBER( 1, 1, 0 )
 * printf("Core version is greater than 1.1.0.\n");
 * #endif
 *
 * \endcode
 */
#define RW_CORE_CREATE_VERSION_NUMBER( major_ver, minor_ver, patch_ver ) \
    ((major_ver) * 1000000 + (minor_ver) * 1000 + (patch_ver))

/*!
 * This macro is an aggregate of the major, minor and patch version numbers.
 * \sa RW_CORE_CREATE_VERSION_NUMBER
 */
#define RW_CORE_VERSION \
    RW_CORE_CREATE_VERSION_NUMBER( RW_CORE_VERSION_MAJOR, RW_CORE_VERSION_MINOR, RW_CORE_VERSION_PATCH )

#endif // RW_CORE_VERSION_H
