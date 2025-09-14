// (c) Electronic Arts. All Rights Reserved.

/// \file
/// File Device class implementation.

#include "rw/core/debug/filedevice.h"

#include "rw/core/base/ostypes.h"
#include "rw/core/debug/debug.h"
#include "rw/core/debug/host.h"

#define INVALID_FILE_HANDLE (-1)

namespace rw {
namespace core {
namespace debug {

/**
  \brief Constructor that allows the caller to specify the buffer size
  \param fileName the name of the file to open for output.
  \param mode the opening mode (text or binary).
  \param flags Device creation flags
*/
FileDevice::FileDevice(const char *fileName, OpenMode mode, unsigned int flags) :
    Device(0, flags),
    mHandle(0)
{
    rwFUNCTION();
    Init(fileName,mode);
    rwRETURNVOID();
}


/**
  \brief Constructor that allows the caller to specify the buffer size
  \param fileName the name of the file to open for output.
  \param bufSize size of the file buffer
  \param mode the opening mode (text or binary).
  \param flags Device creation flags
*/
FileDevice::FileDevice(const char *fileName, size_t bufSize, OpenMode mode, unsigned int flags) :
    Device(bufSize, flags),
    mHandle(0)
{
    rwFUNCTION();
    Init(fileName,mode);
    rwRETURNVOID();
}

/**
  \internal initialization routine
  \param fileName the name of the file to open for output
  \param mode the opening mode (text or binary)
*/
void FileDevice::Init(const char *fileName, OpenMode mode)
{
    rwFUNCTION();
    unsigned int hostFlags = host::HOST_CREATE|host::HOST_TRUNCATE|host::HOST_WRITEONLY;

    if (mode == OPEN_MODE_BINARY)
    {
        hostFlags |= host::HOST_BINARY;
    }

    mHandle = host::Open(fileName, hostFlags);

    //lint -e{1025} Function not found (ignored due to unique macro)
    //lint -e{1703} Function arbitrarily selected
    rwWARNING(mHandle >= 0,
        ("cannot open file %s, device disabled", fileName));

    if(mHandle < 0)
    {
        Disable();
    }
    rwRETURNVOID();
}

/**
  \brief Destructor.
*/
FileDevice::~FileDevice()
{
    rwFUNCTION();

    // file handle might be invalid if file open fails on device creation
    if(mHandle >= 0)
    {
        FileDevice::Flush();
    }

    this->EnterCritical();

    if(mHandle >= 0)
    {
        host::Close(mHandle);
        mHandle = INVALID_FILE_HANDLE;
    }

    this->LeaveCritical();
    rwRETURNVOID();
}


/**
  \brief FileDevice internal output function.
  
  The output function is the core of a device. The FileDevice::Output
  method writes out the output data to the opened file.

  \param data data to write to the file
  \param dataLen number of bytes to write to the file from the data buffer
*/
void FileDevice::Output(const void *data, size_t dataLen)
{
    rwFUNCTION();
    this->EnterCritical();

    rwASSERTM(mHandle >= 0, ("illegal file handle"));

    host::Write(mHandle, data, dataLen);

    this->LeaveCritical();
    rwRETURNVOID();
}

// NOTE: FileDevices remain open after calling the flush.
void FileDevice::Flush()
{
    rwFUNCTION();
    this->EnterCritical();

    Device::Flush();

    this->LeaveCritical();
    rwRETURNVOID();
}

}
}
}
