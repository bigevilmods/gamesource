// (c) Electronic Arts. All Rights Reserved.

// This file contains an example of how the user the RwFileSystem
// IncrementalInstaller.
//
// All the code that could be used directly (e.g. copy/paste) is prefixed with
// "Example_" (this code is located near the bottom of this file.)

#include <EABase/eabase.h>

#if defined EA_PLATFORM_XENON
#pragma warning(push,3)
#pragma warning(disable: 4365)
#include <xtl.h>
#pragma warning(pop)
#endif

#ifdef EA_PLATFORM_PS3
#include <cell/gcm.h>
#include <cell/sysmodule.h>
#include <cell/cell_fs.h>
#include <cell/pad.h>
#include <sysutil/sysutil_common.h>
#include <sysutil/sysutil_msgdialog.h>
#include <sysutil/sysutil_sysparam.h>
#include <sdk_version.h>

#define HOST_SIZE (1024*1024)

#include "gamedata.h"

#endif

#include <EAAssert/eaassert.h>
#include <PPMalloc/EAGeneralAllocatorDebug.h>

#include <rw/core/filesys/async.h>
#include <rw/core/filesys/directory.h>
#include <rw/core/filesys/filecopier.h>
#include <rw/core/filesys/manager.h>
#include <rw/core/filesys/incrementalinstaller.h>

#include <new>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// This allocator is used for all memory allocations
class Allocator : public EA::Allocator::ICoreAllocator
{
public:

    Allocator() : mHeap(mHeapBuffer + mHeapAlignment - reinterpret_cast<int>(mHeapBuffer) % mHeapAlignment, mHeapSize, false) {}
    void* Alloc(size_t size, const char *name, unsigned int flags, unsigned int align, unsigned int alignOffset) { return mHeap.MallocAligned(size, align, alignOffset); }
    void* Alloc(size_t size, const char *name, unsigned int flags) { return mHeap.Malloc(size); }
    void Free(void *block, size_t = 0) { mHeap.Free(block); }

private:

    static const int mHeapAlignment = 128;
    static const int mHeapSize = 1 * 1024 * 1024;
    EA::Allocator::GeneralAllocatorDebug mHeap;
    uint8_t mHeapBuffer[mHeapSize + mHeapAlignment];
} gAllocator;

void* operator new[](size_t, const char*, int, unsigned, const char*, int)
{
    EA_FAIL();

    return (void *)0;
}

// Generate the data that we install (we generate it, because it makes no sense
// to add about 80 MB of stub data to source control.
void GenerateDataFile( const char *filePath, int n )
{
    // Test if the file exists first, because writing some of the larger data
    // takes up some time that we don't want running the test case multiple
    // times.
    FILE *file = fopen( filePath, "r" );
    if( file )
    {
        fclose( file );
    }
    else
    {
        printf( "Generating Data File: \"%s\"\n", filePath );
        FILE *file = fopen( filePath, "w+" );
        EA_ASSERT( file );
        if( file )
        {
            char buffer[ 128 ];
            for( int i = 0; i < n; ++i )
            {
                sprintf( buffer, "%d\n", i );
               fputs( buffer, file );
            }
            fclose( file );
        }
        else
        {
            printf( "FAILED\n" );
        }
    }
}


// Generate the data files
void GenerateData( const char *path, const char *prefix)
{
    const size_t BUFFER_SIZE = rw::core::filesys::FILESYS_MAX_PATH;
    char buffer[ BUFFER_SIZE ];

    for( int i = 0; i < 4; ++i )
    {
        sprintf( buffer, "%s\\%s%d.dat", path, prefix, i );
        GenerateDataFile( buffer, 1024 * 1024 );
    }
}


// This function is called in the file callback, to make sure that the installed
// file contains the data that we installed.
void ValidateData( const char *filename )
{
    FILE *file = fopen( filename, "r" );

    if( file )
    {
        const size_t BUFFER_SIZE = 32;
        char buffer[ BUFFER_SIZE ];

        int i = 0;
        while( false == feof( file ) )
        {
            if( fgets( buffer, BUFFER_SIZE, file ) )
            {
                EA_ASSERT_MSG( i == atoi(buffer), "unexpected data value found" );
                ++i;
            }
        }

        fclose( file );
    }
}


#ifdef EA_PLATFORM_PS3
    // In this example we register the EXITGAME callback, which is used to detect
    // if the user turns off the console while we are running. This will cause
    // proper abortion of the program. Note that since proper termination is not
    // a feature of the incremental installer, the game still has responsibility to
    // terminate the incremental installer at that time.
    #define GAMEDATA_SAMPLE_SYSUTIL_CALLBACK_SLOT_EXITGAME    (0)

    // This is the callback that is called when the user turns of the console (it
    // will be called for different reasons as well, but we are not interested in
    // those for the sake of this example.)
    static void sysutil_callback( uint64_t status, uint64_t param, void * userdata )
    {
        (void)param;
        (void)userdata;

        switch(status)
        {
        case CELL_SYSUTIL_REQUEST_EXITGAME:
            // We want to quit... so set the quit flag
            printf("sysutil_callback -> CELL_SYSUTIL_REQUEST_EXITGAME \n");
			
			// We have got request to stop we tell the installer to stop and then call _Exit
			// to kill this process
			rw::core::filesys::IncrementalInstaller::GetInstance()->Stop();
			_Exit(0);
            break;

        default:
            break;
        }
    }


    // Initialize all the required Cell modules.
    int init()
    {
        int ret = 0;

        ret = cellSysmoduleLoadModule( CELL_SYSMODULE_IO );
        if ( ret != CELL_OK )
        {
            printf( "ERROR : cellSysmoduleLoadModule( CELL_SYSMODULE_IO ) = 0x%x\n", ret );
            return ret;
        }

        ret = cellSysmoduleLoadModule( CELL_SYSMODULE_GCM );
        if ( ret != CELL_OK ) {
            printf( "ERROR : cellSysmoduleLoadModule( CELL_SYSMODULE_GCM ) = 0x%x\n", ret );
            return ret;
        }

        ret = cellSysmoduleLoadModule( CELL_SYSMODULE_FS );
        if ( ret != CELL_OK )
        {
            printf( "ERROR : cellSysmoduleLoadModule( CELL_SYSMODULE_FS ) = 0x%x\n", ret );
            return ret;
        }

        ret = cellSysmoduleLoadModule( CELL_SYSMODULE_SYSUTIL_GAME );
        if ( ret != CELL_OK )
        {
            printf( "ERROR : cellSysmoduleLoadModule( CELL_SYSMODULE_FS ) = 0x%x\n", ret );
            return ret;
        }

        return ret;
    }


    // Exit all the required Cell modules
    int exit()
    {
        int ret = 0;

        ret = cellSysmoduleUnloadModule( CELL_SYSMODULE_FS );
        if ( ret != CELL_OK )
        {
            printf( "ERROR : cellSysmoduleUnloadModule( CELL_SYSMODULE_FS ) = 0x%x\n", ret );
            return ret;
        }

        ret = cellSysmoduleUnloadModule( CELL_SYSMODULE_IO );
        if ( ret != CELL_OK )
        {
            printf( "ERROR : cellSysmoduleUnloadModule( CELL_SYSMODULE_IO ) = 0x%x\n", ret );
            return ret;
        }

        return ret;
    }
#endif // EA_PLATFORM_PS3

void Example_Main();

int main(int, char *argv[] )
{
    char directory[ rw::core::filesys::FILESYS_MAX_PATH ];
#ifdef EA_PLATFORM_XENON
    sprintf( directory, "GAME:" );
#else
    // Get the executable dir, so that we know where to install the data.
    char *lastBackSlash = strrchr( argv[ 0 ], '\\' );
    if( lastBackSlash )
    {
        *lastBackSlash = 0;
        strcpy( directory, argv[ 0 ] );
        *lastBackSlash = '\\';
    }
    else
    {
        printf( "Something went wrong with extracting the path to generate the data to\n" );
        return 1;
    }
#endif


#ifdef EA_PLATFORM_PS3
    // Initialize all the required Cell modules
    if( init() != CELL_OK )
    {
        printf( "ERROR : init -> failed to initialize\n" );
        return 1;
    }

    void *host_addr = memalign(1024*1024, HOST_SIZE);

    int ret = 0;

    if((ret = cellGcmInit(64*1024, HOST_SIZE, host_addr)) != CELL_OK){
        printf("cellGcmInit Error ret = 0x%08x\n", ret);
        return -1;
    }

    if (initDisplay() != 0)
        return -1;

    initShader();

    setDrawEnv();

    if (setRenderObject())
        return -1;

    ret = cellPadInit( 1 );
    if ( ret != 0 ) {
        printf( "cellPadInit() = 0x%x\n", ret );
        return ret;
    }

    // Register the callback for when the user turns off his machine
    if( cellSysutilRegisterCallback( GAMEDATA_SAMPLE_SYSUTIL_CALLBACK_SLOT_EXITGAME, sysutil_callback, NULL ) != CELL_OK )
    {
        printf( "ERROR : Failed to register the GAMEDATA_SAMPLE_SYSUTIL_CALLBACK_SLOT_EXITGAME callback\n" );
        return 1;
    }

    setRenderState();

    /* 1st time */
    setRenderTarget(frame_index);

#endif

    // Create the RwFileSystem manager
    rw::core::filesys::Options options( &gAllocator );
    rw::core::filesys::Manager::CreateInstance( options );

    // Generate the (stub) data
    GenerateData( directory, "cache" );
    GenerateData( directory, "preinstall");
    GenerateData( directory, "postinstall");

    // Call the example code
    Example_Main();

    // Clean up the RwFileSystem manager
    rw::core::filesys::Manager::DestroyInstance();

#ifdef EA_PLATFORM_PS3
    // Terminate the required Cell modules
    if( exit() != CELL_OK )
    {
        printf( "ERROR : exit -> did not exit cleanly\n" );
    }
#endif

    printf( "Exit...\n" );
    return 0;
}








///////////////////////////////////////////////////////////////////////////////
// Incremental Installer example code
//
// The code below contains a fairly complete incremental installer code, which
// you can use to create your own incremental installer integration.
//
// If you have locale specific files you may want to use techniques suchs as
// the following:
//
// const int LANGUAGE_AUDIO_FILE_INDEX = 0;
// static const char *gCacheFiles[] =
// {
//      "englishAudio.big", // Will be replaced for other languages
//      NULL,
// };
//
// Then in some function:
//
// if( language == LANGUAGE_SPANISH )
// {
//      static const char *spanishAudio = "spanishAudio.big";
//      gCacheFiles[ LANGUAGE_AUDIO_FILE_INDEX ] = spanishAudio;
// }
//
// This will effectively make the cache file point to the correct file.
//
//
// Error handling. Because error handling is not part of the incremental
// installer, the application is required to properly handle the errors.
// Hence this example will not show you how to handle the errors, as there
// may be some application specific requirements. However, this example does
// contain some of the default string that can be used for the languages that
// one generally encounters.
//
//
///////////////////////////////////////////////////////////////////////////////

void Example_ErrorCallback(const char *srcFilename, const char *dstFilename, rw::core::filesys::FileCopierError error )
{
    switch( error )
    {
    case rw::core::filesys::FileCopierError_MissingFile:
        printf("Example_CacheFileCopierErrorCallback -> error = FileCopierError_MissingFile (%s)\n", srcFilename);
        break;

    case rw::core::filesys::FileCopierError_NotEnoughSpace:
        printf("Example_CacheFileCopierErrorCallback -> error = FileCopierError_NotEnoughSpace\n");
        break;

    default:
        EA_FAIL_MSG("Example_CacheFileCopierErrorCallback -> error = UNKNOWN ERROR, THIS SHOULDN'T BE POSSIBLE!\n");
        break;
    }
}

void Example_PreInstallHandleErrorCallback(rw::core::filesys::IncrementalInstallerError error, int /*extraInfo*/ )
{
    switch( error )
    {
    case rw::core::filesys::IncrementalInstallerError_NotEnoughSpace:
        printf("Example_PreInstallHandleErrorCallback -> error = IncrementalInstallerError_NotEnoughSpace\n");
        break;

    case rw::core::filesys::IncrementalInstallerError_CorruptData:
        printf("Example_PreInstallHandleErrorCallback -> error = IncrementalInstallerError_CorruptData\n");
        break;

    case rw::core::filesys::IncrementalInstallerError_WrongVersion:
        printf("Example_PreInstallHandleErrorCallback -> error = IncrementalInstallerError_WrongVersion\n");
        break;

    default:
        printf( "Example_PreInstallHandleErrorCallback -> error = %d\n", (int)error );
    }
}

void Example_PreInstallCallback()
{
    printf( "Example_PreInstallCallback -> Show the user a message that data will be installed\n" );
}

void Example_PreInstallBeginCallback()
{
    printf( "Example_PreInstallBeginCallback -> Create the progress bar\n" );
}

void Example_PreInstallUpdateCallback( float factor )
{
    printf( "Example_PreInstallUpdateCallback -> Update the progress bar to %f%%\n", factor * 100.0f );
}

void Example_preInstallEndCallback()
{
    printf( "Example_PreInstallEndCallback -> Destroy the progress bar\n" );
}

void Example_DoneCallback()
{
    printf( "Example_DoneCallback\n" );
}

void Example_CacheFileCallback(const char *srcFilename, const char *dstFilename)
{
    printf("Installed %s to %s\n", srcFilename, dstFilename);
}

const char *Example_WillInstallString()
{
    return "This example will install game data to the HDD. If the game data does not install correctly you will be prompted to install again. Do not switch off the system while the HDD access indicator is flashing";
}

const char *Example_MakeSpaceString()
{
    return "There is not enough available space in the HDD. To create data, at least %d MB more space is required. Exit the game and obtain the necessary space.";
}

const char *Example_MakeSpaceForTrophies()
{
    return "There is not enough available space in the HDD. To play the game, at least %d MB more space is required. Exit the game and obtain the necessary space.";
}

const char *Example_CorruptGameDataString()
{
    return "Game data is corrupt. Please exit the game and delete this game data.";
}

struct RenderThreadContext
{
    bool run;
};

#if defined(EA_PLATFORM_PS3)
intptr_t RenderThread(void *pContext)
{
    RenderThreadContext *rtContext = static_cast<RenderThreadContext *>(pContext);

    while (rtContext->run)
    {
        // Errors from cellSysutilCheckCallback not handled
        // (or even detected) at the moment

        cellSysutilCheckCallback();

        if (setRenderObject())
            break;

        // clear frame buffer
        cellGcmSetClearSurface(gCellGcmCurrentContext, CELL_GCM_CLEAR_Z|
            CELL_GCM_CLEAR_R|
            CELL_GCM_CLEAR_G|
            CELL_GCM_CLEAR_B|
            CELL_GCM_CLEAR_A);

        /* set draw command */
        cellGcmSetDrawArrays(gCellGcmCurrentContext, CELL_GCM_PRIMITIVE_TRIANGLES, 0, 3);


        /* start reading the command buffer */
        flip();
    }

    return 0;
}

#endif

void Example_Main()
{
    // Note that the file lists have to be eiter static or global... or memory allocated,
    // in any case you have to make sure that the files list exists while the installer
    // is running.

    // Setup the incremental installer platform specific properties.
    static const char *titleID = "ABCD12345";
    rw::core::filesys::IncrementalInstallerOptions options;

#if defined(EA_PLATFORM_PS3) || defined(EA_PLATFORM_XENON)
    // The cache files, these files will be installed in the background
    // while the user is playing the game to the cache drive.
    static const char * cacheFiles[] =
    {
        "cache0.dat",
        "cache1.dat",
        "cache2.dat",
        "cache3.dat",
        "data.big",
        NULL                // You need to mark the end with NULL
    };

    options.cacheSrcPath = ".";
    options.cacheCopyPriority = -256;
    options.cacheThreadPriority = EA::Thread::kThreadPriorityMin;
    options.cacheVersion = 0x0100;
    options.cacheClear = true;
    options.cacheFiles = cacheFiles;
    options.cacheTitleID = titleID;
    options.cacheErrorCallback = Example_ErrorCallback;
    options.cacheFileCallback = Example_CacheFileCallback;
#endif

#if defined(EA_PLATFORM_PS3)
    // The content info files, these files are shown in the PS3 menu
    static const char * contentInstallFiles[] =
    {
        "ICON0.PNG",
        "ICON1.PAM",
        "PIC1.PNG",
        NULL
    };

    // The pre install files, these files will be installed while the user is
    // looking at an install progress bar. Note that the calling application
    // is responsible for creating the progress bar.
    static const char * preInstallFiles[] =
    {
        "preinstall0.dat",
        "preinstall1.dat",
        "preinstall2.dat",
        "preinstall3.dat",
        NULL                // You need to mark the end with NULL
    };

    // The post install files, these files will be installed in the background
    // while the user is playing the game.
    static const char * postInstallFiles[] =
    {
        "postinstall0.dat",
        "postinstall1.dat",
        "postinstall2.dat",
        "postinstall3.dat",
        NULL                // You need to mark the end with NULL
    };

    // These properties determine where on the PS3 the data will be
    // installed, this is required by the TRC. The information from
    // this struct will be displayed in the Playstation menu as well.
    strcpy( options.gameDataSystemFileParam.title, "PARAMSFO_TITLE" );
    // Note this NEEDS to be 4 characters followed by 5 digits
    strcpy( options.gameDataSystemFileParam.titleId, titleID );
    strcpy( options.gameDataSystemFileParam.version, "01.00" );

    strcpy( options.titleLang[CELL_GAME_PARAMID_TITLE_DEFAULT],     "SAMPLE GAME" );
    strcpy( options.titleLang[CELL_GAME_PARAMID_TITLE_JAPANESE],    "SAMPLE GAME JAPANESE" );
    strcpy( options.titleLang[CELL_GAME_PARAMID_TITLE_ENGLISH],     "SAMPLE GAME ENGLISH" );
    strcpy( options.titleLang[CELL_GAME_PARAMID_TITLE_FRENCH],      "SAMPLE GAME FRENCH" );
    strcpy( options.titleLang[CELL_GAME_PARAMID_TITLE_SPANISH],     "SAMPLE GAME SPANISH" );
    strcpy( options.titleLang[CELL_GAME_PARAMID_TITLE_GERMAN],      "SAMPLE GAME GERMAN" );
    strcpy( options.titleLang[CELL_GAME_PARAMID_TITLE_ITALIAN],     "SAMPLE GAME ITALIAN" );
    strcpy( options.titleLang[CELL_GAME_PARAMID_TITLE_DUTCH],       "SAMPLE GAME DUTCH" );
    strcpy( options.titleLang[CELL_GAME_PARAMID_TITLE_PORTUGUESE],  "SAMPLE GAME PORTUGUESE" );
    strcpy( options.titleLang[CELL_GAME_PARAMID_TITLE_RUSSIAN],     "SAMPLE GAME RUSSIAN" );
    strcpy( options.titleLang[CELL_GAME_PARAMID_TITLE_KOREAN],      "SAMPLE GAME KOREAN" );
    strcpy( options.titleLang[CELL_GAME_PARAMID_TITLE_CHINESE_T],   "SAMPLE GAME CHINESE_T" );
    strcpy( options.titleLang[CELL_GAME_PARAMID_TITLE_CHINESE_S],   "SAMPLE GAME CHINESE_S" );
    strcpy( options.titleLang[CELL_GAME_PARAMID_TITLE_FINNISH],     "SAMPLE GAME FINNISH" );
    strcpy( options.titleLang[CELL_GAME_PARAMID_TITLE_SWEDISH],     "SAMPLE GAME SWEDISH" );
    strcpy( options.titleLang[CELL_GAME_PARAMID_TITLE_DANISH],      "SAMPLE GAME DANISH" );
    strcpy( options.titleLang[CELL_GAME_PARAMID_TITLE_NORWEGIAN],   "SAMPLE GAME NORWEGIAN" );

    options.installSrcPath = ".";

    // Use the TitleId as the directory to copy files into - it's guaranteed to be unique
    options.installDestPath = titleID;
    options.installCopyPriority = -256;
    options.installThreadPriority = EA::Thread::kThreadPriorityMin;
    options.installVersion = 0x0100;
    options.contentInstallFiles = contentInstallFiles;
    options.preInstallFiles = preInstallFiles;
    options.postInstallFiles = postInstallFiles;

    options.preInstallHandleErrorCallback = Example_PreInstallHandleErrorCallback;
    options.preInstallCallback = Example_PreInstallCallback;
    options.preInstallBeginCallback = Example_PreInstallBeginCallback;
    options.preInstallUpdateCallback = Example_PreInstallUpdateCallback;
    options.preInstallEndCallback = Example_preInstallEndCallback;
    options.installErrorCallback = Example_ErrorCallback;

    options.installWillInstallString = Example_WillInstallString;
    options.installMakeSpaceString = Example_MakeSpaceString;
    options.installMakeSpaceForTrophies = Example_MakeSpaceForTrophies;
    options.installCorruptGameDataString = Example_CorruptGameDataString;
    options.mbIsBootedFromDvd = false;
    options.mbShowDialogMessages = true;

    options.mbFakeFreeSpace = false;
    options.miFakeFreeSpaceKB = 0;
    options.miCareerDataSizeKB = 0;
    options.miTropheySizeKB = 50 * 1024;    // Skate 2 used this much (50 MB)

#endif

    options.doneCallback = Example_DoneCallback;

#if defined(EA_PLATFORM_PS3)

    RenderThreadContext rtContext;
    rtContext.run = true;

    EA::Thread::Thread RenderThreadObj;

    RenderThreadObj.Begin(RenderThread, &rtContext);

#endif

    rw::core::filesys::IncrementalInstaller::CreateInstance(options);
    rw::core::filesys::IncrementalInstaller::GetInstance()->Start();

    EA::Thread::ThreadTime quitTime = EA::Thread::GetThreadTime() + 30*1000;

    // Give it 30 seconds of install time before quitting
    while ( quitTime > EA::Thread::GetThreadTime())
    {
#if defined(EA_PLATFORM_PS3)
        if ( pad_read() == 0 )
            break;
#endif
        EA::Thread::ThreadSleep(16);
    }

#if defined(EA_PLATFORM_PS3)
    rtContext.run = 0;

    while (RenderThreadObj.GetStatus() != EA::Thread::Thread::kStatusEnded)
    {
        EA::Thread::ThreadSleep(1);
    }
#endif

    rw::core::filesys::IncrementalInstaller::GetInstance()->Stop();

    rw::core::filesys::IncrementalInstaller::DestroyInstance();
}
