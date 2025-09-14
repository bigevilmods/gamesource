// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_CORE_FILESYS_INCREMENTAL_INSTALLER_H
#define RW_CORE_FILESYS_INCREMENTAL_INSTALLER_H

#include "EABase/eabase.h"

#if !defined(EA_PLATFORM_PS3) && !defined(EA_PLATFORM_XENON)
#error Incremental Installer is only supported on PS3 & XENON
#endif

#include "rw/core/filesys/async.h"
#include "rw/core/filesys/filecopier.h"
#include "rw/core/filesys/detail/installdriver.h"


#if defined EA_PLATFORM_PS3
    #include <sysutil/sysutil_gamecontent.h>
#endif


namespace rw {
namespace core {
namespace filesys {

/**
    \brief These are the possible values that are given to the HandleErrorCallback callback function.
*/
enum IncrementalInstallerError
{
    /// No error, we're good to go
    IncrementalInstallerError_None,

    /// The write target is full
    IncrementalInstallerError_NotEnoughSpace,

    /// The data is corrupt
    IncrementalInstallerError_CorruptData,

    /// The file has the wrong version number.
    IncrementalInstallerError_WrongVersion,
};


/**
    \brief This is the callback to handle errors. If the error is IncrementalInstallerError_NotEnoughSpace the second
    parameter will indicate how much space is required (in kilobytes).
*/
typedef void (*IncrementalInstaller_HandleErrorCallback)( IncrementalInstallerError error, int spaceRequiredInKB );


/**
    \brief This is the callback that is called when a message box should be shown to the user
            that the application is going to install game data.
*/
typedef void (*IncrementalInstaller_InstallCallback)();


/**
    \brief This is the callback that is called just before the preInstallFiles are installed.
            The application must use this to create the installation progress bar.
*/
typedef void (*IncrementalInstaller_BeginInstallCallback)();


/**
    \brief This is the callback that is called while the preInstallFiles are installed.
            The application must use this to update the installation progress bar.
*/
typedef void (*IncrementalInstaller_UpdateInstallCallback)( float factor ); // factor = [0.0, 1.0]


/**
    \brief This is the callback that is called just after the preInstallFiles are installed.
            The application must use this to destroy the installation progress bar.
*/
typedef void (*IncrementalInstaller_EndInstallCallback)();


/**
    \brief This is the callback that is called when the incremental installer is done, at which
            time the application should destroy the incremental installer singleton.
*/
typedef void (*IncrementalInstaller_DoneCallback)();

typedef const char* (*GetMessageString_Callback)();

/**
    \brief The maximum number of languages that are supported by the
            incremental installer.
*/
const int INCREMENTAL_INSTALLER_LANGUAGE_NUM = 20;

/**
    \brief These are the options with which the incremental installer can be configured.
            Note that some options are only available on some platforms.
*/
struct IncrementalInstallerOptions
{
    IncrementalInstallerOptions();

    // Cache
    const char *cacheSrcPath;                                           ///< The path from where to copy the cache data
    int cacheCopyPriority;                                              ///< The (RwFileSystem) copy priority with which the cache files are copied
    int cacheThreadPriority;                                            ///< The cache file copier thread priority for the background worker thread
    uint32_t cacheVersion;                                              ///< The version of the cache, if this doesn't match what is on disk the disk is cleared
    bool cacheClear;                                                    ///< If true the cache is cleared
    const char **cacheFiles;                                            ///< The files that are cached, this list MUST be NULL terminated
    const char *cacheTitleID;                                           ///< The title ID for the cache
    uint32_t cacheIdleTime;                                             ///< The idle time of the device before caching start copying data
    FileCopierUpdateCallback cacheUpdateCallback;                       ///< \see IncrementalInstaller_UpdateInstallCallback
    FileCopierErrorCallback cacheErrorCallback;                         ///< The file copier error callback for the cache files \see FileCopierErrorCallback
    FileCopierFileCallback  cacheFileCallback;                          ///< The file copier file callback for the cache files \see FileCopierFileCallback

    // Install
#if defined EA_PLATFORM_PS3
    const char *installSrcPath;                                         ///< The path from where to copy the install data
    const char *installDestPath;                                        ///< The path where the files will be installed to.  A common choice would be to use the game's TitleId, which is guaranteed to be unique.
    int installCopyPriority;                                            ///< The (RwFileSystem) copy priority with which the install files are copied
    int installThreadPriority;                                          ///< The install file copier thread priority for the background worker thread
    uint32_t installVersion;                                            ///< The version of the install data, if this doesn't match the handleErrorCallback is called with IncrementalInstallerError_WrongVersion
    const char *contentInstallSrcPath;                                  ///< The path from where to copy content files
    const char **contentInstallFiles;                                   ///< The files are the 'content', this list MUST be NULL terminated. Content are the icon files that go into the Playstation 3 menu.
    const char **preInstallFiles;                                       ///< The files that are pre-installed, this list MUST be NULL terminated. Pre-installed files are the files for which the user will see the install progress bar. If this list != NULL, then the beginInstallCallback, updateInstallCallback and endInstallCallback must be valid function pointers
    const char **postInstallFiles;                                      ///< The files that are post-installed, this list MUST be NULL terminated. Post-installed files are the files that will be copied to the persistent harddrive space in the background, hence the user will not see a progressbar for this.
    uint32_t postInstallIdleTime;                                       ///< The idle time of the device before caching start copying data


    // Callback functions
    IncrementalInstaller_HandleErrorCallback preInstallHandleErrorCallback;  ///< \see IncrementalInstaller_ErrorCallback
    IncrementalInstaller_InstallCallback preInstallCallback;            ///< \see IncrementalInstaller_InstallCallback
    IncrementalInstaller_BeginInstallCallback preInstallBeginCallback;  ///< \see IncrementalInstaller_BeginInstallCallback
    IncrementalInstaller_UpdateInstallCallback preInstallUpdateCallback;///< \see IncrementalInstaller_UpdateInstallCallback
    IncrementalInstaller_EndInstallCallback preInstallEndCallback;      ///< \see IncrementalInstaller_EndInstallCallback
    FileCopierUpdateCallback installUpdateCallback;                     ///< The file copier file callback for the cache files \see FileCopierFileCallback
    FileCopierErrorCallback installErrorCallback;                       ///< \see FileCopierErrorCallback
    FileCopierFileCallback  installFileCallback;                        ///< The file copier file callback for the cache files \see FileCopierFileCallback

    /// The game data parameters that need to be set
    CellGameSetInitParams gameDataSystemFileParam;
    char titleLang[INCREMENTAL_INSTALLER_LANGUAGE_NUM][CELL_GAME_TITLE_SIZE];

    GetMessageString_Callback installWillInstallString;
    GetMessageString_Callback installMakeSpaceString;
    GetMessageString_Callback installMakeSpaceForTrophies;
    GetMessageString_Callback installCorruptGameDataString;

    bool    mbFakeFreeSpace;
    bool    mbIsBootedFromDvd;
    bool    mbShowDialogMessages;
    int     miFakeFreeSpaceKB;
    int     miCareerDataSizeKB;
    int     miTropheySizeKB;

#endif

    IncrementalInstaller_DoneCallback doneCallback;                     ///< When the installer is done (content files, pre-install files and post-install files)
};



/**
    \brief The incremental installer is the code that allows for caching and installing (PS3 only) files.
        The incremental installer is called 'incremental' because it has different phases of installation.

        On the PS3 it has three different phases: pre-install, post-install, cache. The pre-install phase
        will have to show the user an installation progress bar. The post-install phase is installing data
        in the background while the game is running. Both the pre-install and post-install phase install
        data onto the persistent partion of the PS3 harddrive. This means that the data will still be there
        even after the console has been playing other games. The third phase is the cache phase, for which
        there actually is no use on PS3, but people wanted it anyway.

        On the XENON there is only one phase: cache. The cache installs the files to the cache partition.
        This partition is not persistent and hence may be cleared.
*/
class IncrementalInstaller
{
public:

    static void *operator new(size_t size);
    static void *operator new(size_t, void *ptr) { return ptr; }
    static void operator delete(void *p);

    /**
        \brief Create the incremental installer instance
    */
    static IncrementalInstaller *CreateInstance( const IncrementalInstallerOptions &options );

    /**
        \brief Get the incremental installer instance
    */
    static IncrementalInstaller *GetInstance();

    /**
        \brief Destroy the incremental installer instance
    */
    static void DestroyInstance();

    /**
        \brief Start the incremental installing
    */
    void Start();

    /**
        \brief Stop the incremental installing
    */
    void Stop();

    /**
        \brief Terminates the incremental installer
    */
    void Terminate(bool waitForEnd);

    /**
        \brief Determines if it's currently terminating the incremental installer,
        where waitForEnd will wait in this function until it's terminated
    */
    bool IsTerminating() const;

    /**
        \brief You MUST call this function when the disc is ejected!
    */
    void OnDiskEject();

    /**
        \brief You MUST call this function when the disc is recovered!
    */
    void OnDiskRecover();

    /**
        \brief Call this if you want to mount the cache immediately
    */
    void MountCache();

    /**
        \brief Call this to know what the cache path is.
    */
    const char *GetCachePath() const;

    /**
        \brief Call this to know what the install path is. However, you must
        call the Start() function first the path cannot be hard coded and is
        not valid until after the incremental install has been started.  If you
        just want to get the cache path for your title, use
        cellGameDataCheckCreate2() in the PS3 SDK.  Refer to the DLC example
        for a demonstration of this technique.
    */
    const char *GetInstallPath() const;

    /**
        \brief Does a valid cache drive exist?
    */
    bool CacheIsMounted() const;

private:

    static IncrementalInstaller* sInstance;

    static void InstallDriverDone();

    // Non-copyable
    IncrementalInstaller( const IncrementalInstaller& );
    const IncrementalInstaller& operator=( const IncrementalInstaller& );

    // Not constructable/destructable except from CreateInstance/DestroyInstance
    IncrementalInstaller( const IncrementalInstallerOptions &options );
    ~IncrementalInstaller();

    void UpdateState();

    IncrementalInstallerOptions mOptions;

#if defined(EA_PLATFORM_PS3)
    IncrementalInstaller_DoneCallback mUserDoneCallback;
#endif

    detail::InstallDriver *mCacheDriver;

    detail::InstallDriver *mInstallDriver;
    bool mInstallDriverIsStarted;

    enum State
    {
        BEGIN,

#if defined(EA_PLATFORM_PS3)
        INSTALL_DRIVER,
#endif
        CACHE_DRIVER,

        END,
    };

    State mState;
#ifdef EA_PLATFORM_PS3
    char mInstallPath[FILESYS_MAX_PATH];
#endif
};


}}}

#endif
