// (c) Electronic Arts. All Rights Reserved.

/// \file
/// Device class implementation

#include "rw/core/debug/device.h"

#include "rw/core/debug/manager.h"
#include "rw/core/resource/resourceallocator.h"

#include <string.h>

namespace rw {
namespace core {
namespace debug {

// Device::sMsgBufLen is the length of the buffer for an output
// string from Device::Print.  Its size can be changed by calling
// Device::SetBufLen before the new device is created.  Once a
// device has been created, there is no way to change the size
// of its MsgBuffer.
//lint -e{956}  // lint seems to be confused here...  This is a static member of the device class
size_t Device::sMsgBufLen = 4096;

/**
  \brief Unbuffered device constructor.

  Devices created with this constructor will not buffer their output.
  If you want the device to buffer its output, use Device::Device(unsigned int bufLen, unsigned int flags)

  \sa Device::Device, Device::OutputData
*/
Device::Device() :
mEnabled(true),
mFormatter(NULL),
mMsgBuffer(NULL),
mMsgBufLen(0),
mOutBuffer(NULL),
mOutBufP(NULL),
mOutBufLen(0),
mCriticalSection()
{
    rwFUNCTION();
    DeviceInit(0, FLAG_NONE);
    rwRETURNVOID();
}

/**
  \brief Unbuffered device constructor.

  Devices created with this constructor will not buffer their output.
  If you want the device to buffer its output, use Device::Device(unsigned int bufLen, unsigned int flags)

  \sa Device::Device, Device::OutputData
*/
Device::Device(unsigned int flags) :
mEnabled(true),
mFormatter(NULL),
mMsgBuffer(NULL),
mMsgBufLen(0),
mOutBuffer(NULL),
mOutBufP(NULL),
mOutBufLen(0),
mCriticalSection()
{
    rwFUNCTION();

    // make sure the correct constructor was called
    rwASSERT(flags == FLAG_NONE || flags == FLAG_THREADSAFE);
    DeviceInit(0, flags);
    rwRETURNVOID();
}

/**
  \brief Buffered device constructor.
  \param bufLen length of data buffer.

  If the bufLen parameter is non-zero, the device will buffer its output
  until the buffer is filled or flushed (see Device::Flush).

  \sa Device::Device, Device::OutputData, Device::Flush
*/
Device::Device(size_t bufLen, unsigned int flags) :
mEnabled(true),
mFormatter(NULL),
mMsgBuffer(NULL),
mMsgBufLen(0),
mOutBuffer(NULL),
mOutBufP(NULL),
mOutBufLen(0),
mCriticalSection()
{
    rwFUNCTION();
    DeviceInit(bufLen, flags);
    rwRETURNVOID();
}

void Device::DeviceInit(size_t bufLen, unsigned int flags)
{
    rwFUNCTION();
    mMsgBufLen = sMsgBufLen;
    mMsgBuffer = static_cast<char *>(Manager::GetAllocator()->Allocate(
        ResourceDescriptor(mMsgBufLen, sizeof(uint32_t)),
        RW_DEFAULT_NAMESTR).GetMemoryResource());
    if(bufLen != 0)
    {
        // Allocate the data buffer, if the user asked for one.
        mOutBufLen = bufLen;
        mOutBuffer = static_cast<char *>(Manager::GetAllocator()->Allocate(
            ResourceDescriptor(mOutBufLen, sizeof(uint32_t)),
            RW_DEFAULT_NAMESTR).GetMemoryResource());
        mOutBufP = mOutBuffer;
    }

#ifdef EA_DEBUG
    mFormatter = Manager::GetInstance()->GetSystemFormatter();
#else
    mFormatter = Manager::GetFormatter();
#endif

    if (flags & FLAG_THREADSAFE)
    {
        mCriticalSection.Create();
    }
    rwRETURNVOID();
}

/**
  \brief Destructor.
*/
Device::~Device()
{
    rwFUNCTION();
    if(mOutBuffer != NULL)
        Manager::GetAllocator()->Free(mOutBuffer);

    Manager::GetAllocator()->Free(mMsgBuffer);
    mMsgBuffer = NULL;
    mFormatter = NULL;

    mCriticalSection.Destroy();
    rwRETURNVOID();
}

/**
  \brief Flush this device.
  This function flushes any data in the data buffer to the devices'
  Output method.  The device must have been created with a data buffer
  (See Device::Device(unsigned int))

  \sa Device::OutputData, Device::Device
*/
void Device::Flush()
{
    rwFUNCTION();
    FlushOutputBuffer();
    rwRETURNVOID();
}

/**
  \brief Set the length of the message buffer (not the data buffer)
  for all devices created after this one.

  Existing devices will be unaffected, only devices created after
  calling this function will use the new message buffer size.  Note
  that the message buffer is NOT the same as the output buffer.
  When the devices' print method is called, the maximum length of
  the output string is limited to the message buffer length.

  \sa Device::Print

  \param bufLen buffer length to set.
*/
void Device::SetBufLen(size_t bufLen)
{
    rwFUNCTION();
    sMsgBufLen = bufLen;
    rwRETURNVOID();
}

/**
  \brief String output function.

  This function prints a message to an internal message buffer, and
  then calls OutputData with that string.  The maximum length of
  the formatted message is determined by the SetBufLen method when
  the device was created.  The default value for the length of the
  message buffer is 4K.

  \sa Device::SetBufLen

  \param channel Channel where message came from.
  \param format \e  printf style format for message.
  \param args arguments for the format.
*/
void Device::Print(Channel *channel, const char *format, va_list args)
{
    rwFUNCTION();
    mCriticalSection.Enter();

    if(mFormatter != NULL)
    {
        bool result = mFormatter->Format(mMsgBuffer, mMsgBufLen, channel, format, args);

        if(result)
        {
            // Note that the NULL-terminator is NOT sent.  If the device
            // needs to have a NULL-terminator, it should append one.
            OutputData(mMsgBuffer, strlen(mMsgBuffer));
        }
    }

    mCriticalSection.Leave();
    rwRETURNVOID();
}

/**
  \brief data output function.

  This function is called by client code or the print function to
  send data to the device.  If the device is not buffered, the data
  (or message) is immediately output on the device.  If the device
  is buffered, the data (or message) is stored in the buffer until
  the buffer has filled up or is flushed (Device::Flush)

  \sa Device::Flush

  \param data data to output
  \param dataLen length of data to output
*/
void Device::OutputData(const void *data, size_t dataLen)
{
    rwFUNCTION();
    if((mOutBuffer != NULL) && (mOutBufP != NULL))
    {
        // see if the current message fits in the buffer
        // sanity check first
        if(dataLen > mOutBufLen)
        {
            // There is more data to be output than will fit into the buffer,
            // so flush any data that already exists in the buffer and then
            // the new data to the device.
            FlushOutputBuffer();
            Output(data, dataLen);
        }
        else
        {
            // The devices' buffer has enough room to store this data, so
            // copy to the data buffer and return without outputting it.
            size_t bytesLeft = mOutBufLen - static_cast<size_t>(mOutBufP - mOutBuffer);
            if(dataLen > bytesLeft)
            {
                // There is not enough room in the output buffer for the
                // new data, so flush what is already there and put the new
                // data into the start of the buffer.
                FlushOutputBuffer();
            }
            memcpy(mOutBufP, data, dataLen);
            mOutBufP += dataLen;
        }
    }
    else
    {
        // This device does not have a data buffer,
        // so send it to the output immediately.
        Output(data, dataLen);
    }
    rwRETURNVOID();
}

// Private function
void Device::FlushOutputBuffer()
{
    rwFUNCTION();
    if((mOutBuffer != NULL) && (mOutBufP != NULL))
    {
        Output(mOutBuffer, static_cast<unsigned int>(mOutBufP-mOutBuffer));
        mOutBufP = mOutBuffer;
    }
    rwRETURNVOID();
}

/**
  \brief Set the Formatter used for this device.
  \param formatter Formatter to set.
*/
void Device::SetFormatter(IFormatter *formatter)
{
    rwFUNCTION();
    mFormatter = formatter;
    rwRETURNVOID();
}

/**
  \brief Device message buffer accessor.

  Return the buffer used by the device to compose its message strings.
  \return a pointer to the device message buffer.
*/
char *Device::GetBuffer() const
{
    rwFUNCTION();
    rwRETURN(mMsgBuffer);
}

void *Device::operator new(size_t size)
{
    rwFUNCTION();

    rw::IResourceAllocator *allocator = Manager::GetAllocator();

    rwASSERTM(allocator != NULL,
        ("Set the debug Manager allocator (using Manager::CreateInstance()) before creating any debug devices"));

    rwRETURN(allocator->Allocate(ResourceDescriptor(size, sizeof(uint32_t)),
        RW_RESOURCE_ALLOCATION_DESC("rw::core::debug::Device")).GetMemoryResource());
}

void Device::operator delete(void *p)
{
    rwFUNCTION();

    rw::IResourceAllocator *allocator = Manager::GetAllocator();

    rwASSERTM(allocator != NULL,
        ("debug Manager has no allocator."));

    if(p != NULL)
        allocator->Free(p);

    rwRETURNVOID();
}

}
}
}
