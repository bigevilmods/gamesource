// (c) Electronic Arts. All Rights Reserved.

/**
\file
Declaration of the install driver. There are multiple derivations from this class for different platforms.
*/

#ifndef RW_CORE_FILESYS_DETAIL_INSTALLDRIVER_H
#define RW_CORE_FILESYS_DETAIL_INSTALLDRIVER_H

#include "EABase/eabase.h"

#if 0
    #define VERBOSE(message) printf message
#else
    #define VERBOSE(message)
#endif

namespace rw {
namespace core {
namespace filesys {
namespace detail {

/*! \internal
*/
class InstallDriver
{
public:

    InstallDriver() {};
    virtual ~InstallDriver() {};

    // Starts the install driver (causing it to start installing)
    virtual void Start() = 0;

    // Stops the install driver (causing it to stop installing)
    virtual void Stop() = 0;

    // Terminates the install driver (causing it to stop installing), where waitForEnd will wait in this function until it's terminated
    virtual void Terminate(bool waitForEnd) = 0;

    // Determines if it's currently terminating the install driver
    virtual bool IsTerminating() const = 0;

protected:

    static bool CreateVersionFile( const char *path, uint32_t version );

    static bool ValidateVersion( const char *path, uint32_t version );
    static bool ValidateFiles( const char *path, const char **filenames );
};

}}}} // namespace

#endif
