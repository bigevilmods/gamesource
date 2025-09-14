// (c) Electronic Arts. All Rights Reserved.

/// \file
/// Host namespace definition.

#ifndef RW_CORE_DEBUG_HOST_H
#define RW_CORE_DEBUG_HOST_H

#include <eabase/eabase.h>

#if !defined(EA_PLATFORM_REVOLUTION)
#include <sys/types.h>  // size_t
#endif

#if defined(_MSC_VER)
#pragma once
#endif

// documentation provided in the header since there are separate implementations
// for each platform.

/*
  The purpose of these functions is to provide host file I/O functions.
  This allows the client to read and write files on the host system, usually
  the PC the dev. kit is connected to at the time of execution or the local
  hard drive of the dev. kit. In most cases these functions will map directly
  to the HAL file I/O functions but if specialised host I/O functions are
  available, these will be used instead.
*/
namespace rw
{
    namespace core
    {
        namespace debug
        {
            namespace host
            {
                /**
                  since Host directly maps to Hal file functions,
                  these enums have to have the same values as the corresponding
                  ones in Hal. See include/rw/core/hal/hal.h
                  file open mode flags.
                  \see rw::core::debug::host::Open

                  \enum HOST_READONLY open file for read-only.
                  \enum HOST_WRITEONLY open file for write-only.
                  \enum HOST_READWRITE open file for read-write.
                  \enum HOST_APPEND open existing file and append.
                  \enum HOST_CREATE create a file if it doesn't already exist.
                  \enum HOST_TRUNCATE open an existing file and truncate it to 0.
                  \enum HOST_BINARY open file for binary.
                */
                enum HostMode
                {
                    HOST_READONLY   = 1,
                    HOST_WRITEONLY  = 2,
                    HOST_READWRITE  = 4,
                    HOST_APPEND     = 8,
                    HOST_CREATE     = 16,
                    HOST_TRUNCATE   = 32,
                    HOST_BINARY     = 64
                };
                /**
                  \brief open a file on the host platform.
                  \param path file path to open.
                  \param mode mode to open file with, this is a combination of HostMode flags.
                  \return a handle to the file, or a negative integer when failed to open file.
                */
                int Open(const char *path, unsigned int mode);
                /**
                  \brief close a file on the host platform.
                  \param handle handle of file to close.
                */
                void Close(int handle);
                /**
                   \brief read data from an open file.
                   \param handle handle of the file to read from.
                   \param p buffer to read to.
                   \param count number of bytes to read.
                   \return the number of bytes read or a negative integer when read failed.
                */
                size_t Read(int handle, void *p, size_t count);
                /**
                  \brief write data to an open file.
                  \param handle handle of the file to write to.
                  \param p buffer to write from.
                  \param count number of bytes to write.
                  \return the number of bytes written or a negative integer when write failed.
                */
                size_t Write(int handle, const void *p, size_t count);

                void PrintDebugString(const char *s);

                void BreakToDebugger();
            } // namespace host
        } // namespace debug
    } // namespace core
} // namespace rw

#endif // RW_CORE_DEBUG_HOST_H
