// (c) Electronic Arts. All Rights Reserved.

#include "EAAssert/eaassert.h"
#include "EAStdC/EAMemory.h"
#include "EAStdC/EASprintf.h"

#include "rw/core/filesys/async.h"
#include "rw/core/filesys/detail/ps3installdriver.h"
#include "rw/core/filesys/incrementalinstaller.h"

#include <cell/cell_fs.h>
#include <sysutil/sysutil_msgdialog.h>
#include <sdk_version.h>

using namespace EA::StdC;

namespace rw {
namespace core {
namespace filesys {

// Statics
PS3InstallDriver* PS3InstallDriver::sInstance = NULL;

bool PS3InstallDriver::FileCopierTestCallback(const char *srcFilename, const char *dstFilename )
{
    uint64_t srcSize = GetSize( srcFilename );
    uint64_t dstSize = GetSize( dstFilename );
    bool result = srcSize != dstSize || srcSize == 0 || dstSize == 0;
    VERBOSE( ("PS3InstallDriver::FileCopierTestCallback -> srcFilename = %s, dstFilename = %s, result = %d\n", srcFilename, dstFilename, result ? 1 : 0 ) );
    return result;
}


void PS3InstallDriver::FileCopierUpdateCallback(const char *srcFilename, const char *dstFilename, uint64_t currentBytesCopied, uint64_t totalBytesCopied )
{
    if( sInstance->mState == PRE_INSTALL_UPDATE )
    {
        sInstance->mPreInstalledSizeKB += currentBytesCopied / 1024;
        VERBOSE( ( "PS3InstallDriver::FileCopierUpdateCallback -> srcFilename = %s, dstFilename = %s, percentage = %f\n", srcFilename, dstFilename, (double)totalBytesCopied / (double)sInstance->mPreInstallSizeKB ) );
        sInstance->UpdateState();
    }
    else if( sInstance->mState == POST_INSTALL_UPDATE )
    {
        VERBOSE( ( "PS3InstallDriver::FileCopierUpdateCallback -> srcFilename = %s, dstFilename = %s, percentage = %f\n", srcFilename, dstFilename, (double)totalBytesCopied / (double)sInstance->mPostInstallSizeKB ) );
        sInstance->UpdateState();
    }

    if(sInstance->mOptions.installUpdateCallback)
        sInstance->mOptions.installUpdateCallback(srcFilename, dstFilename, currentBytesCopied, totalBytesCopied);
}


void PS3InstallDriver::FileCopierFileCallback(const char *srcFilename, const char *dstFilename )
{
    VERBOSE( ( "PS3InstallDriver::FileCopierFileCallback -> srcFilename = %s, dstFilename = %s\n", srcFilename, dstFilename ) );
}


void PS3InstallDriver::FileCopierDoneCallback()
{
    if( sInstance->mState == CONTENT_INSTALL_BEGIN || sInstance->mState == CONTENT_INSTALL_UPDATE )
    {
        sInstance->mState = CONTENT_INSTALL_END;
        sInstance->UpdateState();
    }
    else if( sInstance->mState == PRE_INSTALL_BEGIN || sInstance->mState == PRE_INSTALL_UPDATE )
    {
        sInstance->mState = PRE_INSTALL_END;
        sInstance->UpdateState();
    }
    else if( sInstance->mState == POST_INSTALL_BEGIN || sInstance->mState == POST_INSTALL_UPDATE )
    {
        sInstance->mState = POST_INSTALL_END;
        sInstance->UpdateState();
    }
}

void *PS3InstallDriver::operator new(size_t size)
{
    EA_ASSERT(sizeof(PS3InstallDriver) == size);
    return RWFILESYSTEM_ALLOCATE(size);
}


void PS3InstallDriver::operator delete(void *p)
{
    Manager::GetInstance()->Free( p );
}


PS3InstallDriver::PS3InstallDriver( const IncrementalInstallerOptions &options )
    : InstallDriver()
    , mOptions( options )
    , mFileCopier( NULL )
    , mState( INSTALL_STATE_INVALID )
    , mContentInstallSizeKB(0)
    , mPreInstalledSizeKB(0)
    , mPreInstallSizeKB(0)
    , mPostInstallSizeKB(0)
    , mError(IncrementalInstallerError_None)
    , miMissingSpaceKB(0)
    , miTotalRequiredKB(0)
{
    EA_ASSERT_MSG( mOptions.installSrcPath != NULL, "You must specify options.srcPath" );
    EA_ASSERT_MSG( mOptions.installDestPath != NULL, "You must specify options.installDestPath" );
    if (mOptions.contentInstallSrcPath == NULL)
        mOptions.contentInstallSrcPath = mOptions.installSrcPath;
    EA_ASSERT_MSG( mOptions.preInstallHandleErrorCallback != NULL, "You must specify options.handleErrorCallback" );
    if( mOptions.preInstallFiles != NULL )
    {
        EA_ASSERT_MSG( mOptions.preInstallCallback != NULL, "You must specify options.installCallback" );
        EA_ASSERT_MSG( mOptions.preInstallBeginCallback != NULL, "You must specify options.beginInstallCallback" );
        EA_ASSERT_MSG( mOptions.preInstallUpdateCallback != NULL, "You must specify options.updateInstallCallback" );
        EA_ASSERT_MSG( mOptions.preInstallEndCallback != NULL, "You must specify options.endInstallCallback" );
    }
    EA_ASSERT_MSG( mOptions.doneCallback != NULL, "You must specify options.doneCallback" );
    EA_ASSERT( mOptions.installWillInstallString != NULL );
    EA_ASSERT( mOptions.installMakeSpaceString != NULL );
    EA_ASSERT( mOptions.installMakeSpaceForTrophies != NULL );
    EA_ASSERT( mOptions.installCorruptGameDataString != NULL );

    EA_ASSERT( sInstance == NULL );
    sInstance = this;
}


PS3InstallDriver::~PS3InstallDriver()
{
    delete mFileCopier;
    sInstance = NULL;
}

void PS3InstallDriver::Init()
{
    FileCopierOptions fcoptions;
    fcoptions.copyPriority = mOptions.installCopyPriority;
    fcoptions.threadPriority = mOptions.installThreadPriority;
    fcoptions.srcIdleTime = mOptions.postInstallIdleTime;
    fcoptions.testCallback = FileCopierTestCallback;
    fcoptions.updateCallback = FileCopierUpdateCallback;
    fcoptions.doneCallback = FileCopierDoneCallback;
    fcoptions.errorCallback = mOptions.installErrorCallback;
    fcoptions.fileCallback = mOptions.installFileCallback;
    mFileCopier = new FileCopier( fcoptions );

    MemFill32(&mContentPath[0], 0, sizeof(mContentPath));
    MemFill32(&mInstallPath[0], 0, sizeof(mInstallPath));
    if( Initialize() )
    {
        UpdateState();
    }
    else
    {
        mOptions.doneCallback();
    }
}


void PS3InstallDriver::Start()
{
    if( mFileCopier )
    {
        mFileCopier->Start();
    }
}


void PS3InstallDriver::Stop()
{
    if( mFileCopier )
    {
        mFileCopier->Stop();
    }

    mbQuitRequested = true;
}


void PS3InstallDriver::Terminate(bool waitForEnd)
{
    if( mFileCopier )
    {
        mFileCopier->Terminate(waitForEnd);
    }
}


bool PS3InstallDriver::IsTerminating() const
{
    if( mFileCopier )
    {
        return mFileCopier->IsTerminating();
    }

    return false;
}


void PS3InstallDriver::CalculateSizes( const char **files, const char *srcPath, uint64_t *installSizeKB, bool isBootedFromDvd )
{
    char fullSrcFilename[ FILESYS_MAX_PATH ];
    const char *srcFileName;
    int index = 0;

    *installSizeKB = 0;

    if( files != NULL )
    {
        do
        {
           srcFileName = files[index++];
           if( srcFileName != NULL )
           {
                Snprintf( fullSrcFilename, sizeof( fullSrcFilename ), "%s/%s", srcPath, srcFileName );
                // Check for disc eject
                if (isBootedFromDvd)
                {
                    CellFsStat filestat;
                    int retValue = cellFsStat( "/dev_bdvd", &filestat );
                    if ( retValue != CELL_FS_SUCCEEDED )
                    {
                        return;
                    }
                }

                uint64_t srcSize = GetSize( fullSrcFilename );
                *installSizeKB += static_cast< int >( ( srcSize + 1023 ) / static_cast< uint64_t >( 1024 ) );
           }
        } while( srcFileName != NULL );
    }
}

void PS3InstallDriver::WaitCallback(int iButton, void* pVoid)
{
    if (iButton == CELL_MSGDIALOG_BUTTON_OK)
    {
        sInstance->mbWaitForCallback = false;
    }
}

bool PS3InstallDriver::ShowWillInstall()
{
    if (!sInstance->mOptions.mbShowDialogMessages)
        return sInstance->mbQuitRequested;

    const char* szMsg = sInstance->mOptions.installWillInstallString();
    sInstance->mbWaitForCallback = true;
    cellMsgDialogOpen2
        (
        CELL_MSGDIALOG_TYPE_SE_TYPE_ERROR | CELL_MSGDIALOG_TYPE_BG_VISIBLE | CELL_MSGDIALOG_TYPE_BUTTON_TYPE_OK | CELL_MSGDIALOG_TYPE_DISABLE_CANCEL_ON,
        szMsg,
        PS3InstallDriver::WaitCallback,
        NULL, NULL
        );

    while (sInstance->mbWaitForCallback && !sInstance->mbQuitRequested)
    {
        EA::Thread::ThreadSleep(100);
    }

    return sInstance->mbQuitRequested;
}

#ifdef EA_FILESYSTEM_VERBOSE
// Print some GameData statistics
void PS3InstallDriver::PrintCellGameDataStatGet( CellGameDataStatGet *get )
{
    printf("--------------------------------------------------------------------------------\n" );
    printf("hddFreeSizeKB 0x%x\n", get->hddFreeSizeKB);
    printf("isNewData: %d\n", get->isNewData );
    printf("contentInfoPath : %s\n", get->contentInfoPath );
    printf("gameDataPath    : %s\n", get->gameDataPath );
    printf("sizeKB : %d\n", get->sizeKB);
    printf("sysSizeKB : 0x%x\n", get->sysSizeKB);
    printf("%lld  %lld  %lld\n", get->st_atime, get->st_ctime, get->st_mtime );
    printf("\n" );

    printf( "PARAM.SFO:TITLE = [%s]\n", get->getParam.title );

    for( int i = 0; i < 12; i++ )
    {
        if( get->getParam.titleLang[ i ] )
        {
            printf( "PARAM.SFO:TITLE%02d = [%s]\n", i, get->getParam.titleLang[i] );
        }
    }

    printf("PARAM.SFO:TITLEID        = [%s]\n", get->getParam.titleId );
    printf("PARAM.SFO:VERSION        = [%s]\n", get->getParam.dataVersion );
    printf("PARAM.SFO:ATTRIBUTE      = [%d]\n", get->getParam.attribute );
    printf("PARAM.SFO:PARENTAL_LEVEL = [%d]\n", get->getParam.parentalLevel );
    printf("\n" );
    printf("--------------------------------------------------------------------------------\n" );
}
#endif // EA_FILESYSTEM_VERBOSE

#if defined(EA_ASSERT_ENABLED)

bool PS3InstallDriver::IsAlpha( char c ) const
{
    return ( c >= 'A' && c <= 'Z'  ) || ( c >= 'a' && c <= 'z'  );
}

bool PS3InstallDriver::IsDigit( char c ) const
{
    return ( c >= '0' && c <= '9'  );
}

#endif

bool PS3InstallDriver::Initialize()
{
    bool result = false;

#if defined(EA_ASSERT_ENABLED)
    // Make sure that the title Id is 4 times alpha character followed by 5
    // times a digit character, for example: "ABCD12345".
    bool validTitleId =
        IsAlpha( sInstance->mOptions.gameDataSystemFileParam.titleId[ 0 ] ) &&
        IsAlpha( sInstance->mOptions.gameDataSystemFileParam.titleId[ 1 ] ) &&
        IsAlpha( sInstance->mOptions.gameDataSystemFileParam.titleId[ 2 ] ) &&
        IsAlpha( sInstance->mOptions.gameDataSystemFileParam.titleId[ 3 ] ) &&
        IsDigit( sInstance->mOptions.gameDataSystemFileParam.titleId[ 4 ] ) &&
        IsDigit( sInstance->mOptions.gameDataSystemFileParam.titleId[ 5 ] ) &&
        IsDigit( sInstance->mOptions.gameDataSystemFileParam.titleId[ 6 ] ) &&
        IsDigit( sInstance->mOptions.gameDataSystemFileParam.titleId[ 7 ] ) &&
        IsDigit( sInstance->mOptions.gameDataSystemFileParam.titleId[ 8 ] );
    EA_ASSERT( validTitleId );
#endif

    mbShowedWillInstall = false;
    mbWaitForCallback = false;
    mbQuitRequested = false;

    mError = IncrementalInstallerError_None;
    miTotalRequiredKB = 0;

    // We are assuming the game application has already created a
    // render thread and that cellSysutilCheckCallback() is being
    // called on the render thread or some other thread at this
    // point.
    CellGameContentSize size;
    int gameContentSize = 0;

    int ret = cellGameDataCheck( CELL_GAME_GAMETYPE_GAMEDATA,
        mOptions.installDestPath, &size);

    bool needsInstall = false;
    bool installDirectoryExists = false;

    if (ret == CELL_GAME_RET_OK)
    {
        if (size.sizeKB != CELL_GAME_SIZEKB_NOTCALC)
        {
            gameContentSize = size.sizeKB;
        }
        else
        {
            // Obtain the size of game content since it could not be calculated from cellGameDataCheck,
            // which must be done before cellGameContentPermit is called.
            int sizeKB = 0;
            int sizeRet = cellGameGetSizeKB(&sizeKB);

            if (sizeRet == CELL_GAME_RET_OK)
            {
                gameContentSize = sizeKB;
            }
            else
            {
                VERBOSE(("cellGameContentPermit() Error: 0x%x\n", sizeRet));
            }
        }
        // Already installed.  Verify the files are there already.
        ret = cellGameContentPermit( mContentPath, mInstallPath ) ;
        installDirectoryExists = true;
        if(ret < 0)
        {
            VERBOSE(("cellGameContentPermit() Error: 0x%x\n", ret));
        }
        else
        {
            VERBOSE(("cellGameContentPermit() OK\n"));
            VERBOSE(("  contentInfoPath:[%s]\n", mContentPath));
            VERBOSE(("  usrdirPath:[%s]\n", mInstallPath));

            // Add searchpath to head so that all files will first be look for on the hdd
            Manager::GetInstance()->AddSearchLocation( mInstallPath, Manager::SEARCHPATH_ADD_TO_HEAD );
        }

        bool bValid = ValidateVersion( mInstallPath, mOptions.installVersion );
        if (!bValid)
        {
            mError = IncrementalInstallerError_WrongVersion;
        }
        else
        {
            bValid &= ValidateFiles( mContentPath, mOptions.contentInstallFiles );
            bValid &= ValidateFiles( mInstallPath, mOptions.preInstallFiles );

            if (bValid)
            {
                bValid &= ValidateFiles( mInstallPath, mOptions.postInstallFiles ); 

                if (bValid)
                {
                    // Post install is completed
                    mState = POST_INSTALL_END;
                    result = true;
                }
                else
                {
                    // The content & pre-install files are present and non-zero in
                    // size, so move on to the post-install files
                    mState = POST_INSTALL_BEGIN;
                    result = true;
                    needsInstall = true;
                }
            }
            else
            {
                // The content & pre-install files are not present or corrupt, so they should be re-installed
                mState = CONTENT_INSTALL_BEGIN;
                result = true;
                needsInstall = true;
            }
        }
    }
    else if (ret == CELL_GAME_RET_NONE)
    {
        needsInstall = true;
    }
    else
    {
        mError = IncrementalInstallerError_CorruptData;
    }

    if (mError == IncrementalInstallerError_None)
    {
        VERBOSE(("cellGameDataCheck() OK [new data]\n"));

        // Add up the sizes of the files to be installed.
        CalculateSizes( mOptions.contentInstallFiles, mOptions.contentInstallSrcPath, &mContentInstallSizeKB, mOptions.mbIsBootedFromDvd );
        CalculateSizes( mOptions.preInstallFiles, mOptions.installSrcPath, &mPreInstallSizeKB, mOptions.mbIsBootedFromDvd );
        CalculateSizes( mOptions.postInstallFiles, mOptions.installSrcPath, &mPostInstallSizeKB, mOptions.mbIsBootedFromDvd );

        miTotalRequiredKB = 1 + // for the version file
            mContentInstallSizeKB +
            mPreInstallSizeKB + 
            mPostInstallSizeKB +
            mOptions.miTropheySizeKB +
            mOptions.miCareerDataSizeKB;

        if (mContentInstallSizeKB == 0)
        {
            miTotalRequiredKB = 0;
        }
        else
        {
            miTotalRequiredKB -= gameContentSize;
        }
        // Do a file check if anything needs to be installed or space needs to be held for trophy and career DB.
        if (miTotalRequiredKB > 0)
        {
            VERBOSE( ("PS3InstallDriver::Initialize ->  hddFreeSizeKB = %d, requiredSizeInKb = %d, totalInstallSizeKB = %d, gameDataSizeKB = %d\n",
                size.hddFreeSizeKB, requiredSizeInKb, (int)mTotalInstallSizeKB, gameDataSizeKB ) );

            int hddFreeSizeKB = size.hddFreeSizeKB - size.sysSizeKB;
            if (mOptions.mbFakeFreeSpace)
            {
                hddFreeSizeKB = mOptions.miFakeFreeSpaceKB;
            }

            int needSizeInKB = hddFreeSizeKB - miTotalRequiredKB;

            if (needSizeInKB < 0)
            {
                mError = IncrementalInstallerError_NotEnoughSpace;
                miMissingSpaceKB = -needSizeInKB;
                result = false;
            }
        }

        // Carry on installing if all is good
        if (needsInstall && ret == CELL_GAME_RET_NONE && mError == IncrementalInstallerError_None)
        {
            VERBOSE(("cellGameDataCheck() OK [new data]\n"));

            // We have enough space, so get the temporary install paths.  Content
            // files and pre-game installation files will be placed here.  After
            // these files have finished copying, the temp paths will be changed
            // to the final paths and the post-install files will be copied.

            ret = installDirectoryExists 
                ? 0 
                : cellGameCreateGameData( &mOptions.gameDataSystemFileParam, mContentPath, mInstallPath );

            if(ret < 0)
            {
                VERBOSE(("cellGameCreateGameData() Error: 0x%x\n", ret));
            }
            else
            {
                VERBOSE(("tmp_contentInfoPath: %s\n", tmp_contentInfoPath));
                VERBOSE(("tmp_usrdirPath: %s\n", tmp_usrdirPath));
                VERBOSE(("cellGameCreateGameData() OK\n"));

                // Update the param.sfo strings.
                for(int i = CELL_GAME_PARAMID_TITLE_DEFAULT;
                    i < CELL_GAME_PARAMID_TITLE_NORWEGIAN;
                    ++i)
                {
                    ret = cellGameSetParamString( i, sInstance->mOptions.titleLang[i] );
                    if(ret < 0)
                    {
                        VERBOSE(("cellGameSetParamString() Error id:[%d] : 0x%x\n", i, ret));
                    }
                    else
                    {
                        VERBOSE(("cellGameSetParamString() OK id:[%d]\n", i));
                    }
                }

                if (!mbShowedWillInstall)
                {
                    mbShowedWillInstall = true;
                    ShowWillInstall();
                }

                if(CreateVersionFile(sInstance->mInstallPath, sInstance->mOptions.installVersion))
                {
                    result = true;
                    mState = CONTENT_INSTALL_BEGIN;
                    mOptions.preInstallCallback();
                }
            }
        }
    }

    if (mbQuitRequested)
    {
        // The user has asked to quit, so just suspend the thread
        EA::Thread::ThreadSleep(EA::Thread::kTimeoutNone);

        return result;
    }

    const char* szMsg = NULL;

    // If there was no error, create a version file and set the return value
    // to true.
    if (mError == IncrementalInstallerError_NotEnoughSpace)
    {
        // sample error message
//        cellGameContentErrorDialog( CELL_GAME_ERRDIALOG_NOSPACE, needSizeInKB, NULL) ;

        // Skate 2 error message

        int iMissingMB = miMissingSpaceKB/1024;
        if (0 != miMissingSpaceKB % 1024)
            ++iMissingMB;

        if (miTotalRequiredKB > mOptions.miTropheySizeKB + mOptions.miCareerDataSizeKB + 1024)
            szMsg = GetMakeSpaceMsgString(iMissingMB);
        else
            szMsg = GetMakeSpaceForTropheyMsgString(iMissingMB);

        if (mOptions.mbShowDialogMessages)
        {
            cellMsgDialogOpen2
                (
                CELL_MSGDIALOG_TYPE_SE_TYPE_ERROR | CELL_MSGDIALOG_TYPE_BG_VISIBLE | CELL_MSGDIALOG_TYPE_BUTTON_TYPE_NONE | CELL_MSGDIALOG_TYPE_DISABLE_CANCEL_ON,
                szMsg,
                WaitCallback,
                NULL, NULL);
        }
#if defined(EA_DEBUG)
        else
        {
            printf("%s\n", szMsg);
        }
#endif
        mOptions.preInstallHandleErrorCallback(mError, miMissingSpaceKB);

        mState = INSTALL_STATE_INVALID;
        result = false;
    }
    else if (mError == IncrementalInstallerError_CorruptData || mError == IncrementalInstallerError_WrongVersion)
    {
        // Skate 2 used cellGameDataExitBroken, but that function is not
        // available in 2.80, so we'll display our own dialog instead.

#if defined(EA_DEBUG)
        if (mError == IncrementalInstallerError_WrongVersion)
        {
            printf("Wrong version of installed data found.  Delete the installed data from the system menu and restart.\n");
        }
#endif

        // Tell the user to delete this install and restart the game
        szMsg = GetCorruptGameDataMsgString();

        if (mOptions.mbShowDialogMessages)
        {
            cellMsgDialogOpen2 (
                CELL_MSGDIALOG_TYPE_SE_TYPE_ERROR | CELL_MSGDIALOG_TYPE_BG_VISIBLE | CELL_MSGDIALOG_TYPE_BUTTON_TYPE_NONE | CELL_MSGDIALOG_TYPE_DISABLE_CANCEL_ON,
                szMsg,
                WaitCallback,
                NULL, NULL);
        }
#if defined(EA_DEBUG)
        else
        {
            printf("%s\n", szMsg);
        }
#endif
        mOptions.preInstallHandleErrorCallback(mError, 0);

        // we must not allow the game to continue as it would be picking up
        // invalid data from the cached/installed files.  The developer will
        // have to delete the old data from the PS3 system menu.
        EA::Thread::ThreadSleep(EA::Thread::kTimeoutNone);
    }

    return result;
}

void PS3InstallDriver::UpdateState()
{
    bool updateStateAgain;
    do
    {
        updateStateAgain = false;

        switch( mState )
        {
        case INSTALL_STATE_INVALID:
            EA_FAIL();
            break;

        case CONTENT_INSTALL_BEGIN:
            if( mOptions.contentInstallFiles != NULL )
            {
                VERBOSE( ( "PS3InstallDriver -> CONTENT_INSTALL_BEGIN\n" ) );
                mFileCopier->Copy( mOptions.contentInstallFiles, mOptions.contentInstallSrcPath, mContentPath );
                mState = CONTENT_INSTALL_UPDATE;
                updateStateAgain = true;
            }
            else
            {
                mState = PRE_INSTALL_BEGIN;
                updateStateAgain = true;
            }
            break;

        case CONTENT_INSTALL_UPDATE:
            VERBOSE( ( "PS3InstallDriver -> CONTENT_INSTALL_UPDATE\n" ) );
            break;

        case CONTENT_INSTALL_END:
            VERBOSE( ( "PS3InstallDriver -> CONTENT_INSTALL_END\n" ) );
            mState = PRE_INSTALL_BEGIN;
            updateStateAgain = true;
            break;

        case PRE_INSTALL_BEGIN:
            if( mOptions.preInstallFiles != NULL )
            {
                VERBOSE( ( "PS3InstallDriver -> PRE_INSTALL_BEGIN\n" ) );

                if( mOptions.preInstallBeginCallback )
                    mOptions.preInstallBeginCallback();

                mFileCopier->Copy( mOptions.preInstallFiles, mOptions.installSrcPath, mInstallPath );
                mState = PRE_INSTALL_UPDATE;
                updateStateAgain = true;
            }
            else
            {
                mState = PRE_INSTALL_END;
                updateStateAgain = true;
            }
            break;

        case PRE_INSTALL_UPDATE:
            VERBOSE( ( "PS3InstallDriver -> PRE_INSTALL_UPDATE\n" ) );
            if( mOptions.preInstallUpdateCallback )
            {
                float factor = mPreInstallSizeKB != 0 ?
                    (float)( ( mPreInstalledSizeKB * 100 ) / mPreInstallSizeKB ) / 100.0f
                    : 1.0f;
                mOptions.preInstallUpdateCallback( factor );
            }
            break;

        case PRE_INSTALL_END:
            VERBOSE( ( "PS3InstallDriver -> PRE_INSTALL_END\n" ) );
            {
                // finalize the install path.  Note that this needs to be
                // done regardless of whether any files were copied or not.
                // After this is done, the installed content/data will be
                // accessible to the game.
                int ret = cellGameContentPermit( mContentPath, mInstallPath ) ;
                if(ret < 0)
                {
                    VERBOSE(("cellGameContentPermit() Error: 0x%x\n", ret));
                }
                else
                {
                    VERBOSE(("cellGameContentPermit() OK\n"));
                    VERBOSE(("  contentInfoPath:[%s]\n", mContentPath));
                    VERBOSE(("  usrdirPath:[%s]\n", mInstallPath));

                    // Add searchpath to head so that all files will first be look for on the hdd
                    Manager::GetInstance()->AddSearchLocation( mInstallPath, Manager::SEARCHPATH_ADD_TO_HEAD );
                }
            }
            if( mOptions.preInstallEndCallback )
            {
                mOptions.preInstallEndCallback();
            }
            mState = POST_INSTALL_BEGIN;
            updateStateAgain = true;
            break;

        case POST_INSTALL_BEGIN:
            if( mOptions.postInstallFiles != NULL )
            {
                VERBOSE( ( "PS3InstallDriver -> POST_INSTALL_BEGIN\n" ) );
                mFileCopier->Copy( mOptions.postInstallFiles, mOptions.installSrcPath, mInstallPath );
                mState = POST_INSTALL_UPDATE;
                updateStateAgain = true;
            }
            else
            {
                mState = POST_INSTALL_END;
                updateStateAgain = true;
            }
            break;

        case POST_INSTALL_UPDATE:
            VERBOSE( ( "PS3InstallDriver -> POST_INSTALL_UPDATE\n" ) );
            break;

        case POST_INSTALL_END:
            VERBOSE( ( "PS3InstallDriver -> POST_INSTALL_END\n" ) );
            if( mOptions.doneCallback )
            {
                mOptions.doneCallback();
            }
            break;
        }
    } while(updateStateAgain);
}

const char* PS3InstallDriver::GetMakeSpaceForTropheyMsgString(int iSizeMB) const
{
    static char szMsgBuf[CELL_MSGDIALOG_STRING_SIZE+1];

    const char *szFormat = mOptions.installMakeSpaceForTrophies();

    Snprintf(szMsgBuf, CELL_MSGDIALOG_STRING_SIZE, szFormat, iSizeMB);
    return szMsgBuf;
}

const char* PS3InstallDriver::GetCorruptGameDataMsgString() const
{
    return mOptions.installCorruptGameDataString();
}

const char* PS3InstallDriver::GetMakeSpaceMsgString(int iSizeMB) const
{
    static char szMsgBuf[CELL_MSGDIALOG_STRING_SIZE+1];
    const char *szFormat = mOptions.installMakeSpaceString();

    Snprintf(szMsgBuf, CELL_MSGDIALOG_STRING_SIZE, szFormat, iSizeMB);
    return szMsgBuf;
}

}
}
}
