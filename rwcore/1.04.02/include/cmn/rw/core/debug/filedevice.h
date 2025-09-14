// (c) Electronic Arts. All Rights Reserved.

/// \file
/// File Device class definition.

#ifndef RW_CORE_DEBUG_FILEDEVICE_H
#define RW_CORE_DEBUG_FILEDEVICE_H
#include <rw/core/base/ostypes.h>
#include <rw/core/debug/device.h>

#if defined(_MSC_VER)
#pragma once
#endif

namespace rw
{
namespace core
{
namespace debug
{

/**
  \brief debug Device that outputs to a file on the host system.
*/
class FileDevice : public Device
{
public:

    /**
       \brief Output mode
    */
    enum OpenMode
    {
        OPEN_MODE_TEXT,     ///< output in text mode
        OPEN_MODE_BINARY    ///< output in binary mode
    };

    FileDevice(const char *fileName, OpenMode mode = OPEN_MODE_TEXT,
        unsigned int flags = 0);
    FileDevice(const char *fileName, size_t bufSize,
        OpenMode mode = OPEN_MODE_TEXT, unsigned int flags = 0);

    virtual ~FileDevice();

    virtual void Flush();

private:
    void Init(const char *fileName, OpenMode mode);
    virtual void Output(const void *data, size_t dataLen);

    int mHandle;

    // Prevent default generation of these functions by not defining them
    FileDevice(const FileDevice& rhs);              // copy constructor
    FileDevice& operator=(const FileDevice& rhs);   // assignment operator
};

} // namespace core
} // namespace debug
} // namespace rw

#endif
