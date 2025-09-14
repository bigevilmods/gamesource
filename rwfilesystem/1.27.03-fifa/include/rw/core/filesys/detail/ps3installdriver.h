// (c) Electronic Arts. All Rights Reserved.

/**
\file
Implementation of the Playstation 3 install driver.
*/

#ifndef RW_CORE_FILESYS_PS3_INSTALL_DRIVER_H
#define RW_CORE_FILESYS_PS3_INSTALL_DRIVER_H

#include "EABase/eabase.h"
#include "EAAssert/eaassert.h"
#include "rw/core/filesys/async.h"
#include "rw/core/filesys/detail/installdriver.h"
#include "rw/core/filesys/filecopier.h"
#include "rw/core/filesys/incrementalinstaller.h"

#include "eathread/eathread_thread.h"

#include <cell/sysmodule.h>
#include <sys/memory.h>
#include <sys/return_code.h>
#include <sysutil/sysutil_sysparam.h>
#include <sysutil/sysutil_common.h>


namespace rw {
namespace core {
namespace filesys {

/**
  \brief The PS3InstallDriver class is the Playstation 3 implementation for the install driver
*/
class PS3InstallDriver : public detail::InstallDriver
{
public:

    static void *operator new(size_t size);
    static void operator delete(void *p);

    PS3InstallDriver( const IncrementalInstallerOptions &options );
    virtual ~PS3InstallDriver();

    virtual void Start();
    virtual void Stop();
    virtual void Terminate(bool waitForEnd);
    virtual bool IsTerminating() const;

    void Init();

    const char *GetInstallPath() const {return &mInstallPath[0]; }

private:

    // Non-copyable
    PS3InstallDriver( const PS3InstallDriver & );
    const PS3InstallDriver& operator=( const PS3InstallDriver & );

    // The instance is needed for the callback functions
    static PS3InstallDriver* sInstance;

    // Different callbacks
    static bool FileCopierTestCallback(const char *srcFilename, const char *dstFilename );
    static void FileCopierUpdateCallback(const char *srcFilename, const char *dstFilename, uint64_t currentBytesCopied, uint64_t totalBytesCopied );
    static void FileCopierFileCallback(const char *srcFilename, const char *dstFilename );
    static void FileCopierDoneCallback();

    // Game data methods
    void CalculateSizes( const char **files, const char *srcPath, uint64_t *installSizeKB, bool isBootedFromDvd );
    static void WaitCallback( int iButton, void* pVoid );
    bool Initialize();
    void UpdateState();

    //bool Validate();

    #ifdef EA_FILESYSTEM_VERBOSE
        static void PrintCellGameDataStatGet( CellGameDataStatGet *get );
    #endif

    // Members
    IncrementalInstallerOptions mOptions;
    char mContentPath[ FILESYS_MAX_PATH ];
    char mInstallPath[ FILESYS_MAX_PATH ];
    FileCopier *mFileCopier;

    enum State
    {
        INSTALL_STATE_INVALID,

        CONTENT_INSTALL_BEGIN,
        CONTENT_INSTALL_UPDATE,
        CONTENT_INSTALL_END,

        PRE_INSTALL_BEGIN,
        PRE_INSTALL_UPDATE,
        PRE_INSTALL_END,

        POST_INSTALL_BEGIN,
        POST_INSTALL_UPDATE,
        POST_INSTALL_END,
    };

    State mState;

    uint64_t mContentInstallSizeKB;
    uint64_t mPreInstalledSizeKB;
    uint64_t mPreInstallSizeKB;
    uint64_t mPostInstallSizeKB;

    IncrementalInstallerError mError;
    int      miMissingSpaceKB;
    int      miTotalRequiredKB;

    bool    mbShowedWillInstall;
    bool    mbWaitForCallback;
    bool    mbQuitRequested;

    const char* GetMakeSpaceMsgString(int iSizeMB) const;
    const char* GetMakeSpaceForTropheyMsgString(int iSizeMB) const;
    const char* GetCorruptGameDataMsgString() const;
    static bool ShowWillInstall();
};

}}} // namespace

#endif
