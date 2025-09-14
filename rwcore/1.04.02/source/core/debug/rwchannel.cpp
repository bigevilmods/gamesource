// (c) Electronic Arts. All Rights Reserved.

/// \file
/// Channel class implementation

#include "rw/core/debug/channel.h"
#include "rw/core/debug/device.h"

#include "rw/core/resource/resourceallocator.h"

#include <string.h>

namespace rw {
namespace core {
namespace debug {

/**
  \brief debug Channel new operator
  \param size size of the block to allocate.
*/
void *Channel::operator new(size_t nsize)
{
    rwFUNCTION();

    rw::IResourceAllocator *allocator = Manager::GetAllocator();

    rwASSERTM(allocator != NULL,
        ("Set the debug Manager allocator (using Manager::CreateInstance()) before creating any debug channels"));

    rwRETURN(allocator->Allocate(ResourceDescriptor(nsize, 4),  // magic number 4 is used instead of sizeof(uint32_t) to avoid 64bit safety warnings
        RW_RESOURCE_ALLOCATION_DESC("rw::core::debug::Channel")).GetMemoryResource());
}

/**
  \brief debug Channel delete operator
  \param p pointer to block to free.
*/
void Channel::operator delete(void *p)
{
    rwFUNCTION();
    rw::IResourceAllocator *allocator = Manager::GetAllocator();

    rwASSERTM(allocator != NULL,
        ("debug Manager has no allocator."));

    if(p != NULL)
        allocator->Free(p);
    rwRETURNVOID();
}

/**
  \brief debug Channel constructor.
  \param name name of the channel.
  \param device default device associated with this channel. This can be NULL
  in which case the channel is effectively disabled until a device is added
  using the \e  AddDevice() function.
*/
Channel::Channel(const char *name, Device *device, unsigned int flags) :
    mDeviceCount(0),
    mIndex(0),
    mEnabled(true),
    mHaveInfo(false),
    mFileName(NULL),
    mLine(0),
    mCriticalSection(),
    mPrintOpts()
{
    rwFUNCTION();
    strncpy(mName, name, MAXNAMELEN);

#ifdef EA_DEBUG
    Manager::GetInstance()->RegisterChannel(this);
    mPrintOpts = Manager::GetInstance()->GetDefaultPrintOpts();
#endif

    // If the caller asked for a thread-safe channel,
    // create a critical section now.
    if (flags & FLAG_THREADSAFE)
    {
        mCriticalSection.Create();
    }

    for(unsigned int i=0; i<MAXDEVICES; i++)
        mDeviceList[i] = NULL;

    if(device != NULL)
        AddDevice(device);
    rwRETURNVOID();
}

/**
  \brief debug Channel destructor.
*/
Channel::~Channel()
{
    rwFUNCTION();
#ifdef EA_DEBUG
    Manager::GetInstance()->DeleteChannel(this);
#endif

    mCriticalSection.Destroy();

    mFileName = NULL;
    rwRETURNVOID();
}

/**
  \brief Channel name accessor.
  \return the name of the channel.
*/
const char *Channel::GetName() const
{
    rwFUNCTION();
    rwRETURN(mName);
}


/**
  \brief File name accessor.
  \return the file name set by the caller. (__FILE__)
*/
const char *Channel::GetFileName() const
{
    rwFUNCTION();
    rwRETURN(mFileName);
}

/**
  \brief Line number accessor.
  \return the line number set by the caller. (__LINE__)
*/
unsigned int Channel::GetLine() const
{
    rwFUNCTION();
    rwRETURN(mLine);
}

/**
  \brief set the file name and line number for the current Print operation.

  This function is used by the RWDEBUG_PRINT macro to set the file name and
  line number where the debug print came from. Due to the implementation of
  this macro, it is not possible to pass these directly as parameters to the Print
  function and need to be set separately. Typically, user code will not call
  this function directly unless they are defining their own debug print macros.

  \param fileName name of the file. (__FILE__)
  \param line line numner. (__LINE__)
  \sa #rwMESSAGE
*/
void Channel::SetInfo(const char *fileName, unsigned int line)
{
    rwFUNCTION();
    mFileName = fileName;
    mLine = line;
    mHaveInfo = true;
    rwRETURNVOID();
}

/**
  \brief HaveInfo flag accessor.

  This function is used by the Print function to see if the SetInfo function has
  been called. If so, the file name and line number of the Print are available.
  Typically, user code will not call this function directly unless they are defining
  their own debug print macros.

  \return mHaveInfo member.
*/
bool Channel::HaveInfo() const
{
    rwFUNCTION();
    rwRETURN(mHaveInfo);
}

/**
  \brief Get the output options for this channel.
  \return a pointer to a debugOpts structure containing the options.
  \sa PrintOpts
*/
const PrintOpts &Channel::GetPrintOpts() const
{
    rwFUNCTION();
    rwRETURN(mPrintOpts);
}

/**
  \brief Set the output options for this channel.
  \param opts a pointer to a debugOpts structure containing the options to set.
  \sa PrintOpts
*/
void Channel::SetPrintOpts(const PrintOpts &opts)
{
    rwFUNCTION();
    mCriticalSection.Enter();
    mPrintOpts = opts;
    mCriticalSection.Leave();
    rwRETURNVOID();
}

/**
  \brief Enable debug Channel.
*/
void Channel::Enable()
{
    rwFUNCTION();
    mEnabled = true;
    rwRETURNVOID();
}

/**
  \brief Disable debug Channel.
*/
void Channel::Disable()
{
    rwFUNCTION();
    mEnabled = false;
    rwRETURNVOID();
}

/**
  \brief Enabled flag accessor.
  \return \e  true: channel enabled, \e  false: channel disabled
*/
bool Channel::IsEnabled() const
{
    rwFUNCTION();
    rwRETURN(mEnabled);
}

/**
  \brief Add a device to the channel.
  \param device a pointer to the device to add.
  \return \e  true if device successfully added, \e  false if operation failed.
  \sa Device
*/
bool Channel::AddDevice(Device *device)
{
    rwFUNCTION();

    mCriticalSection.Enter();

    for(unsigned int i=0; i<MAXDEVICES; i++)
    {
        if(mDeviceList[i] == 0)
        {
            mDeviceList[i] = device;
            mDeviceCount++;
            mCriticalSection.Leave();

            rwRETURN(true);
        }
    }

    rwCMESSAGE(SYSTEMCHANNEL, ("Channel::AddDevice: out of devices, ignored (maximum is %d)", MAXDEVICES));

    mCriticalSection.Leave();

    rwRETURN(false);
}

/**
  \brief Remove device from the channel.
  \param device a pointer to the device to remove
  \return \e  true if device successfully removed, \e  false if operation failed
  \sa Device
*/
bool Channel::RemoveDevice(const Device *device)
{
    rwFUNCTION();

    mCriticalSection.Enter();

    for(unsigned int i=0; i<MAXDEVICES; i++)
    {
        if(mDeviceList[i] == device)
        {
            mDeviceList[i] = NULL;
            mDeviceCount--;
            mCriticalSection.Leave();

            rwRETURN(true);
        }
    }

    rwCMESSAGE(SYSTEMCHANNEL, ("Channel::RemoveDevice: device not found"));

    mCriticalSection.Leave();

    rwRETURN(false);
}

//lint -e{1916}
/**
  \brief String output function.
  This function takes \e  printf() style arguments and sends the data to all the
  enabled devices associated with this channel.
  \note avoid calling this function directly, instead use one of the
  #rwMESSAGE, #rwCMESSAGE or #rwABORT macros.
*/
void Channel::Print(const char *format, ...)
{
    rwFUNCTION();

    if(!mEnabled || (mDeviceCount == 0))
        rwRETURNVOID();

    va_list args;
    va_start(args, format);

    Vprint(format, args);

    va_end(args);
    mHaveInfo = false;

    rwRETURNVOID();
}

/**
  \brief Binary data output function.
  This function sends the data to all the enabled devices associated with this
  channel.
  \note avoid calling this function directly
*/
void Channel::OutputData(const void *data, unsigned int dataLen)
{
    rwFUNCTION();

    if(!mEnabled || (mDeviceCount == 0))
        rwRETURNVOID();

    mCriticalSection.Enter();

    for(unsigned int i=0; i<MAXDEVICES; i++)
    {
        if((mDeviceList[i] != NULL) && (mDeviceList[i]->IsEnabled()))
            mDeviceList[i]->OutputData(data, dataLen);
    }

    mCriticalSection.Leave();
    rwRETURNVOID();
}

/**
  \param format printf style format string
  \param args variable argument list
*/
void Channel::Vprint(const char *format, va_list args)
{
    rwFUNCTION();

    if(!mEnabled || (mDeviceCount == 0))
        rwRETURNVOID();

    mCriticalSection.Enter();

    for(unsigned int i=0; i<MAXDEVICES; i++)
    {
        if((mDeviceList[i] != NULL) && (mDeviceList[i]->IsEnabled()))
        {
            mDeviceList[i]->Print(this, format, args);
        }
    }

    mCriticalSection.Leave();
    rwRETURNVOID();
}

/**
  \brief flush all devices associated with this channel
*/
void Channel::Flush()
{
    rwFUNCTION();
    mCriticalSection.Enter();

    for(unsigned int i=0; i<MAXDEVICES; i++)
    {
        if((mDeviceList[i] != NULL) && (mDeviceList[i]->IsEnabled()))
            mDeviceList[i]->Flush();
    }

    mCriticalSection.Leave();
    rwRETURNVOID();
}

/**
  \brief Get the first device in the device list.
  \return a pointer to the device or NULL if no devices found.
  \sa GetNextDevice
  \warning the device iteration mechanism is not thread safe, it uses
  an internal index that is used by GetNextDevice()
*/
Device *Channel::GetFirstDevice()
{
    rwFUNCTION();
    mIndex = 0;
    rwRETURN(GetNextDevice());
}

/**
  \brief Get the next device in the device list.
  \return a pointer to the device or NULL if no device found.
  \sa GetFirstDevice
  \warning the device iteration mechanism is not thread safe
*/
Device *Channel::GetNextDevice()
{
    rwFUNCTION();
    for(unsigned int i=mIndex; i<MAXDEVICES; i++)
    {
        if(mDeviceList[i] != NULL)
        {
            mIndex = i+1;
            rwRETURN(mDeviceList[i]);
        }
    }
    rwRETURN(NULL);
}

}
}
}
