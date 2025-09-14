// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_CORE_FILESYS_ANDROID_DRIVERIMPL_H
#define RW_CORE_FILESYS_ANDROID_DRIVERIMPL_H

#include "unix/rw/core/filesys/driverimpl.h"

#include "rw/core/filesys/device.h"

namespace rw {
namespace core {
namespace filesys {
    /*! \brief Android Package DeviceDriver implementation
     *
     * To open a file in a .apk file you should prefix the path with "appbundle:/".
     *
     * e.g.
     *      "appbundle:/myfile.txt"
     *  
     * Before trying to open a file in the .apk file you need to initialize
     * rwfilesystem from Java.  Doing so provides rwfilesystem with a reference
     * to the Java AssetManager as well as the JVM itself.
     *
     * You will need to startup rwfilesystem and pass the AssetManager:
     *
     *   rwfilesystem.Startup(getAssets());
     * 
     * When the application is exitting you should also call shutdown to free up
     * references to Java objects held by the filesystem.
     *
     *   rwfilesystem.Shutdown();
     *
     * This DeviceDriver allows files to be read from an Android Package (.apk)
     * file.  Due to the limited API exposed by the Android API's, this driver
     * only implements a subset of the normal driver features.  In particular,
     * the driver does not allow for random seeking in the file.  Please read
     * the documentation for each of the methods for more details.
     *
     */
    class AppBundleDeviceDriver : public DeviceDriver
    {
    public:
        AppBundleDeviceDriver();

        virtual ~AppBundleDeviceDriver() {};

        virtual void *          Open(const char *name, unsigned int oflags, Handle **ppdfh);
        virtual void            Close(void * h);
        using                   DeviceDriver::Close;
        virtual unsigned int    Read(void * h, void *buf, unsigned int bufsize, DeviceDriver *dd, void *dfh);

        /*! \brief This method is currently unsupported. */
        virtual unsigned int    Write(void * h, const void *buf, unsigned int bufsize, DeviceDriver *dd, void *dfh);

        /*! \brief Seeks within a file.
         *
         * The seek operation is currently very limited.  Seek's may be to the
         * beginning of the file or to the current location in the file (i.e. a
         * nop).  Any other Seek operations are unsupported.
         *
         * Note: It could be possible to implement an arbitrary seek in a file,
         * but this would be very inefficient, so it is currently unsupported.
         */
        virtual uint64_t        Seek(void * h, uint64_t offset, int whence, DeviceDriver *dd, void *dfh);

        /*! \brief Returns the size of the file.
         *
         * Note: This method may only be called if the file that was opened was
         * stored uncompressed in the package file.  If the file was stored
         * compressed then this method will Assert at runtime or return an
         * incorrect value.
         */
        virtual uint64_t        GetSize(void * h);
        using                   DeviceDriver::GetSize;


        /*! \brief This method is currently unsupported. */
        virtual bool            Resize(void * h, uint64_t newsize);

        /*! \brief This method is currently unsupported. */
        virtual bool            Delete(const char *filePath);

        /*! \brief This method is currently unsupported. */
        virtual bool            Move(const char *currentName, const char *newName);

        /*! \brief This method is currently unsupported. */
        virtual void *          FindBegin(const char *filePattern, FindData *findData);

        /*! \brief This method is currently unsupported. */
        virtual bool            FindNext(void *h, FindData *findData);

        /*! \brief This method is currently unsupported. */
        virtual bool            FindEnd(void *h);

        /*! \brief This method is currently unsupported. */
        virtual bool            DirectoryCreate(const char *directory);

        /*! \brief This method is currently unsupported. */
        virtual bool            DirectoryRemove(const char *directory);

        /*! \brief This method is currently unsupported. */
        virtual bool            GetNameFromHandle(void *handle, char *name, size_t nameBufferSize);
    };

    extern AppBundleDeviceDriver gAppBundleDeviceDriverImpl;
}}}

#endif
