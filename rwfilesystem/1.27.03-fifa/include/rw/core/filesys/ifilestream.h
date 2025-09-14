// (c) Electronic Arts. All Rights Reserved.

/**
\file
Header file of rw::core::filesys::IFileStream and its subclass DefaultFileStream.
*/

#ifndef RW_CORE_FILESYS_IFILESTREAM_H
#define RW_CORE_FILESYS_IFILESTREAM_H

#include "EABase/eabase.h"
#include "EAAssert/eaassert.h"

#include "rw/core/filesys/async.h"

namespace rw {
namespace core {
/**
    \namespace rw::core::filesys
    \brief Contains filesystem interface and device drivers for performing I/O from memory, file and other devices.
*/
namespace filesys {

/**
  \brief The IFileStream interface is a standard FILE/fopen like interface
  for performing synchronous reads from memory, file, or any
  other device that can be read from in this way.
  The IFileStream is primarily intended for use in tools
  that require synchronous read capabilities.
*/
class RWCORELIB_API IFileStream
{
public:

    /**
      \brief The orgin for seek operations.
    */
    enum SeekOrigin
    {
        SEEKORIGIN_CURRENT,         ///< Seek from the current position of the file pointer.
        SEEKORIGIN_BEGINNING,       ///< Seek from the start of the file
        SEEKORIGIN_END,             ///< Seek from the end of the file
        SEEKORIGIN_FORCEENUMSIZEINT = 0xFFFFFFFF        ///< \internal
    };

    virtual ~IFileStream() {}

    /**
      \brief Read count bytes into buffer from the current location in the file stream
      \param buffer destination for read data
      \param count  number of bytes to read

      \importlib rwfilesystem

      \return actual number of bytes read, 0 on failure
    */
    virtual uint64_t  Read(void * buffer, size_t count)  = 0;

    /**
      \brief Write count bytes from buffer to file stream at the current file stream location
      \param buffer source data to be written
      \param count  number of bytes to write

      \importlib rwfilesystem

      \return actual number of bytes written, 0 on failure
    */
    virtual uint64_t  Write(const void * buffer, size_t count)              = 0;

    /**
      \brief Returns the current location within the file stream

      \importlib rwfilesystem

      \return current location within the file stream
    */
    virtual uint64_t  Tell()                                                = 0;

    /**
      \brief Seek to a new location in the file stream
      If successful, Seek returns 0, otherwise it returns a nonzero value.

      \param offset offset to seek to from the seek origin
      \param origin IFileStream::SeekOrigin value specifying where to seek from

      \importlib rwfilesystem

      \return negative on failure, 0 on success
    */
    virtual int Seek(int64_t offset, SeekOrigin origin)                    = 0;

    /**
      \brief Returns the size of referenced file in bytes

      \importlib rwfilesystem

      \return size of file in bytes
    */
    virtual uint64_t  Size()                                                = 0;
};

/**
  \brief The DefaultFileStream provides a synchronous version of the
  AsyncOp class, suitable for use in tools.

  File I/O in games should be asynchronous, making this class a poor
  choice for in-game use.

  DefaultFileStream does provide additional buffering over what is
  provided by the underlying AsyncOp device in use.
  \importlib rwfilesystem
*/
class RWCORELIB_API DefaultFileStream : public IFileStream
{
 public:
    /*! \brief Default constructor
    */
    DefaultFileStream();

    /*! \brief Open a file.

    Refer to AsyncOp::Open for a discussion of the parameters.
    \param filename - path to file to open
    \param modeflags - creation or open mode AsyncOp::OpenFlags flags
    \return 0 if the file was opened successfully, -1 if the open failed.
    */
    int               Open(const char * filename, unsigned int modeflags);

    /*! \brief Close the file
    */
    void              Close();

    // IFileStream interface
    virtual ~DefaultFileStream();
    virtual uint64_t  Read(void * buffer, size_t count);
    virtual uint64_t  Write(const void * buffer, size_t count);
    virtual uint64_t  Tell();
    virtual int       Seek(int64_t offset, IFileStream::SeekOrigin origin);
    virtual uint64_t  Size();

private:

    rw::core::filesys::AsyncOp  mFileOp;
    rw::core::filesys::Handle  *mFileHandle;
    uint64_t                     mPos;
};

}}}

#endif
