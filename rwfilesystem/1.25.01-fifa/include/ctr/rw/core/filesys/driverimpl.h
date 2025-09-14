// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_CORE_FILESYS_INTERNAL_DEVICE_CTR_H
#define RW_CORE_FILESYS_INTERNAL_DEVICE_CTR_H

#include <string.h>

#include "rw/core/filesys/device.h"

namespace rw {
namespace core {
namespace filesys {

	class DeviceFileHandle;
/*! \brief CTR rwfilesystem DeviceDriver implementation

    Client code may use this as a base class or declare
    additional instances of it.  Write() is not implemented
    by this driver.

	Currently we do not support anything except the ROM FS, as there are several problems with the other archive drives.
	The flash memory is not done because we currently do not have access to the flash memory needed to test this.
	The SD Card archies cannot be put in because of their unique requirements, such as the need to presize any written files,
	and you cannot resize a file once you have created it.
*/
class CtrFileDeviceDriver : public DeviceDriver
{
public:
    CtrFileDeviceDriver() {}

    virtual ~CtrFileDeviceDriver() {}
	virtual void        Restore();
private:
    virtual bool        Init();
	

    virtual void *      Open(const char *name, unsigned int oflags, Handle **);
    virtual void        Close(void *h);
    virtual unsigned int Read(void *h, void *buf, unsigned int bufsize, DeviceDriver *, void *);
    virtual uint64_t    Seek(void *h, uint64_t offset, int whence, DeviceDriver *, void *);
    virtual uint64_t    GetSize(void *h);
	virtual bool            Resize(void * h, uint64_t newsize);
	virtual bool            Delete(const char *filePath);
	virtual bool Move(const char *currentName, const char *newName);
    virtual void*       FindBegin(const char *, FindData *);
    virtual bool        FindNext(void *, FindData *);
    virtual bool        FindEnd(void *);
	virtual bool DirectoryCreate(const char *directory);
	virtual bool DirectoryRemove(const char *directory);

    virtual bool        GetNameFromHandle(void *handle, char *name, size_t nameBufferSize);

	static void CreateNintendoFileReader(DeviceFileHandle * handle);
	static void DestroyNintendoFileReader(DeviceFileHandle * handle);
	static void FixSlash(char *Dest,const char *Src);
	void WCharBufferToCharBuffer(const wchar_t* src_wchar_buffer, char* dst_char_buffer );
};

extern CtrFileDeviceDriver gDeviceDriverImpl;

}
}
}

#endif // RW_CORE_FILESYS_INTERNAL_DEVICE_WII_H
