// (c) Electronic Arts. All Rights Reserved.
/**
\file
Implements the save function.
*/

#include "rw/core/filesys/loadsave.h"
#include "rw/core/filesys/async.h"
#include "EAAssert/eaassert.h"

#include <stdio.h>

namespace rw {
namespace core {
namespace filesys {

/*!
    Save a contiguous buffer to a file.
    \param filePath file path.  If not fully-qualified, it will be stored in
    the current working directory location.
    \param addr memory address to begin save from
    \param bytes number of bytes to save.
    \importlib rwfilesystem
    \sa Manager::SetCWD
*/
void Save(const char *filePath, const void *addr, uint64_t bytes)
{
    AsyncOp op;
    
    op.Open(filePath,AsyncOp::FOPEN_CREATE | AsyncOp::FOPEN_OVERWRITE | AsyncOp::FOPEN_READWRITE);
    op.Wait();

    filesys::Handle *fh = op.GetResultHandle();
    EA_ASSERT_FORMATTED(fh != NULL, ("Error Opening file '%s' for writing", filePath));

    if (fh != NULL)
    {
        op.Write(fh, addr, 0, bytes);
        op.Wait();

        uint64_t nWritten = op.GetResultSize();
        EA_ASSERT(nWritten == bytes);
        (void)nWritten;

        op.Close(fh);
        op.Wait();
    }
}

}}}

