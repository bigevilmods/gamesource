// (c) Electronic Arts. All Rights Reserved.

/// \file
/// Console Device class definition.

#ifndef RW_CORE_DEBUG_CONSOLEDEVICE_H
#define RW_CORE_DEBUG_CONSOLEDEVICE_H
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
  Debug Console output device.
  Sends output to the debugger console.
*/
class ConsoleDevice : public Device
{
public:
    ConsoleDevice() : Device(0, Device::FLAG_NONE) {}
    explicit ConsoleDevice(unsigned int flags) : Device(0, flags) {}
    ConsoleDevice(unsigned int bufSize, unsigned int flags)  : Device(bufSize, flags) {}
    virtual ~ConsoleDevice() {}

private:
    virtual void Output(const void *data, size_t dataLen);

    // Prevent default generation of these functions by not defining them
    ConsoleDevice(const ConsoleDevice& rhs);            // copy constructor
    ConsoleDevice& operator=(const ConsoleDevice& rhs); // assignment operator
};

} // namespace debug
} // namespace core
} // namespace rw

#endif
