// (c) Electronic Arts. All Rights Reserved.

/// \file
/// Device class definition

#ifndef RW_CORE_DEBUG_DEVICE_H
#define RW_CORE_DEBUG_DEVICE_H
#include <rw/core/base/ostypes.h>

#if defined(_MSC_VER)
#pragma once
#endif

#include <stdarg.h>

#include "detail/debugcriticalsection.h"

namespace rw
{
namespace core
{
namespace debug
{

class Channel;
class IFormatter;
/**
  \brief Debug device abstract class.

  This abstract class provides an interface for sending output to
  user-defined devices.  Since this is an abstract class, in order
  to use it, a class must be derived from this class and the Output
  method implemented.  Derived classes can be implemented to capture
  output to files, debuggers, memory buffers, communication devices,
  etc. Two implementations of devices are included with the RwCore
  library: FileDevice and ConsoleDevice.

  Please note: instances of this class are NOT THREADSAFE by default.
  If you need a thread-safe device, pass rw::core::debug::Debug::FLAG_THREADSAFE
  to the constructor when the object is created.
*/
class Device
{
public:

    /**
      \brief Device flags
    */
    enum
    {
        FLAG_NONE = 0x0,        ///< output is not threadsafe
        FLAG_THREADSAFE = 0x1   ///< enable threadsafe output
    };

    Device();
    explicit Device(unsigned int flags);
    Device(size_t bufLen, unsigned int flags);
    virtual ~Device();

    /**
     enable this device.  This flag is ignored by the
     implementation.  Callers should test this flag before
     outputting data to a device.
    */
    void Enable()
    {
        rwFUNCTION();
        mEnabled = true;
        rwRETURNVOID();
    }

    /**
     disable this device.  This has no effect on the devices'
     functionality.  disabled devices can be used just the
     same as enabled devices.  Callers should test the Enable
     method before outputting data to a device.
    */
    void Disable()
    {
        rwFUNCTION();
        mEnabled = false;
        rwRETURNVOID();
    }

    /**
     \brief Check if device is enabled.
     \return \e  true if device is enabled, \e  false if not.
    */
    bool IsEnabled() const
    {
        rwFUNCTION();
        rwRETURN(mEnabled);
    }

    /**
     call this to send data to the device.  If the device has
     a data buffer, it will be not be output until the buffer is
     full or it is flushed.  It is virtual so derived classes
     can override it if they wish.
    */
    virtual void OutputData(const void *data, size_t dataLen);

    /**
     Flush the data buffer.  If the device has no data buffer,
     every time some data is sent to the device with OutputData,
     it is flushed automatically.  It is virtual so derived classes
     can override it if they wish.
    */
    virtual void Flush();

    void SetFormatter(IFormatter *formatter);

    char *GetBuffer() const;

    void Print(Channel *channel, const char *format, va_list args);
    /**
      \brief derived class interface

      These are generally only for derived classes so they can have
      exclusive access to the device Critical Section.
    */
    void EnterCritical()
    {
        rwFUNCTION();
        mCriticalSection.Enter();
        rwRETURNVOID();
    }

    /**
      \brief derived class interface

      These are generally only for derived classes so they can have
      exclusive access to the device Critical Section.
    */
    void LeaveCritical()
    {
        rwFUNCTION();
        mCriticalSection.Leave();
        rwRETURNVOID();
    }

    static void *operator new(size_t);
    static void operator delete(void *);

    static void SetBufLen(size_t bufLen);

private:
    void DeviceInit(size_t msgBufLen, unsigned int flags);
    void FlushOutputBuffer();

    /**
      Derived classes need to implement the Output method.
      This method should output the data to the device. Note
      that message strings will not necessarily be NULL-
      terminated, so if your device needs to have strings
      NULL-terminated, that character must be sent separately.
    */
    virtual void Output(const void *data, size_t dataLen) = 0;

    /**
      Is the device enabled?  This flag is ignored internally, so
      calling OutputData on a disabled device will behave the same
      as calling OutputData on an enabled device.
    */
    bool mEnabled;

    /**
      Formatter used by this device.
    */
    IFormatter *mFormatter;

    /**
      Message formatting buffer.  This buffer is used by the Print
      method to hold the formatted output string.
    */
    char *mMsgBuffer;

    /**
      Length of the buffer.  It cannot be changed after the device
      has been created, but it can be changed by calling SetBufLen
      before the device is created.
    */
    size_t mMsgBufLen;

    /**
      Output buffer.  If the device is unbuffered (the default),
      this is NULL.  The size of this buffer can only be specified
      when the device is constructed.
    */
    char *mOutBuffer;

    /**
      Pointer into output buffer
    */
    char *mOutBufP;

    /**
      Length of this buffer
    */
    size_t mOutBufLen;

    /**
      Devices' critical section
    */
    rw::core::debug::detail::DebugCriticalSection mCriticalSection;

    static size_t sMsgBufLen; // Length of this buffer (global user setting).

    // Prevent default generation of these functions by not defining them
    Device(const Device& rhs);              // copy constructor
    Device& operator=(const Device& rhs);   // assignment operator
};

} // namespace debug
} // namespace core
} // namespace rw

#endif // RW_CORE_DEBUG_DEVICE_H
