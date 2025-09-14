// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_CORE_FILESYS_CACHEDRIVER_H
#define RW_CORE_FILESYS_CACHEDRIVER_H

#include "EABase/eabase.h"
#include "rw/core/filesys/incrementalinstaller.h"
#include "rw/core/filesys/detail/installdriver.h"

#if defined(EA_PLATFORM_PS3) || defined(EA_PLATFORM_XENON)

    #include "rw/core/filesys/async.h"
    #include "rw/core/filesys/detail/installdriver.h"

    namespace rw {
    namespace core {
    namespace filesys {

    /// \internal
    struct CacheDriverOptions
    {
        CacheDriverOptions()
            : srcPath(NULL)
            , copyPriority(-256)
            , threadPriority(EA::Thread::kThreadPriorityMin)
            , version(0)
            , clear(false)
            , titleID(NULL)
            , files(NULL)
            , updateCallback(NULL)
            , errorCallback(NULL)
            , fileCallback(NULL)
            , doneCallback(NULL)
            , idleTime(250)
        {
        }

        const char *srcPath;
        int copyPriority;
        int threadPriority;
        uint32_t version;
        bool clear;
        const char *titleID;
        const char **files;
        FileCopierUpdateCallback updateCallback;
        FileCopierErrorCallback errorCallback;
        FileCopierFileCallback fileCallback;
        FileCopierDoneCallback doneCallback;
        uint32_t idleTime;
    };

    /// \internal
    class CacheDriver : public detail::InstallDriver
    {
    public:

        static void *operator new(size_t size);
        static void operator delete(void *p);

        CacheDriver( const CacheDriverOptions &options );
        virtual ~CacheDriver();

        virtual void Start();
        virtual void Stop();
        virtual void Terminate(bool waitForEnd);
        virtual bool IsTerminating() const;

        void Clear();
        void Mount();
        bool Mounted() const { return mMounted; };
        const char *GetPath() const { if(mPath[0] == 0) return 0; else return mPath; }

    private:

        // Non-copyable
        CacheDriver( const CacheDriver & );
        const CacheDriver& operator=( const CacheDriver & );

        // The instance is needed for the callback functions
        static CacheDriver* sInstance;


        static bool TestCallback(const char *srcFilename, const char *dstFilename);
        static void DoneCallback();

        bool XenonCacheMounted() const;
        void ValidateCache();
        bool IsCacheAvailable() const;

        CacheDriverOptions mOptions;
        char mPath[ rw::core::filesys::FILESYS_MAX_PATH ];
        FileCopier *mFileCopier;
        bool mMounted;
        bool mValidated;
        bool mDone;
    };

    }}} // namespace
#endif  // if defined(EA_PLATFORM_PS3) || defined(EA_PLATFORM_XENON)


#endif
