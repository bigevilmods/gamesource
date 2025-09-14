// (c) Electronic Arts. All Rights Reserved.

// Downloadable content PS3 example.
//
// This example shows how downloadable content could work on the PS3.
// One approach is to pass the AsyncOp::FOPEN_DRM flag.  The downside
// to this approach is that a lot of code may have to be touched to
// add the flag.  Another approach, which is taken by this example,
// is to add a new virtual device which is derived from the library's
// PS3Host1FileDeviceDriver and overrides the Open() function.  This
// device's Open function adds the FOPEN_DRM flag and then calls the
// library's PS3FileDeviceDriver, which respects the DRM flag.
// Note that at this time, this example isn't verified to be working.
// Please contact us (use the RW Core Users mailing list) to get the
// most up-to-date information on how this should work.  Once we have
// a shipping game using this technology, the example will be updated
// with code you can copy and paste into your own DLC-enabled game.

#define EA_ASSERT_ENABLED

#include <rw/core/filesys/manager.h>
#include <rw/core/filesys/driverimpl.h>

#include <EAAssert/eaassert.h>

#include <EAStdC/EAAlignment.h>
#include <EAStdC/EAString.h>

#include <cell/sysmodule.h>
#include <cell/fs/cell_fs_file_api.h>
#include <cell/fs/cell_fs_errno.h>

// Network library modules
#include <np/drm.h>
#include <np/common.h>

#include <sysutil/sysutil_common.h>
#include <sysutil/sysutil_game_common.h>
#include <sysutil/sysutil_gamecontent.h>

#include <string.h>
#include <stdlib.h>

using namespace rw::core;

namespace
{
    //-----------------------------------------------------------------------------
    // Allocator
    //-----------------------------------------------------------------------------

    class Allocator : public EA::Allocator::ICoreAllocator
    {
    public:

        virtual void *Alloc(size_t size, const char *, unsigned int);
        virtual void *Alloc(size_t size, const char *, unsigned int, unsigned int alignment, unsigned int);
        virtual void Free(void *block, size_t);
    };

    void *Allocator::Alloc(size_t size, const char *, unsigned int)
    {
        return Alloc(
            size,
            NULL,
            0,
    #ifdef EA_PLATFORM_PS3
            16,
            0
    #else
            0,
            0
    #endif
            );

    }

    void *Allocator::Alloc(size_t size, const char *, unsigned int, unsigned int alignment, unsigned int)
    {
        // Implementation ripped from
        // rwcore/1.00.00/source/core/memory_drv/rwmemoryaligned.cpp,
        // RwMallocAlignedGeneric
        if (0 == alignment)
        {
            alignment = 1;
        }

        /* Over allocate to allow for storage of base address */
        void *mem = malloc(size + sizeof(void *) + sizeof(size_t) + (alignment - 1));
        if (!mem)
        {
            return NULL;
        }

        void *alignedMem = EA::StdC::AlignUp((uint8_t *)mem + sizeof(void *) + sizeof(size_t), alignment);

        /* Store base address */
        *((void **)alignedMem - 1) = mem;

        /* Store size */
        *((size_t *)((void **)alignedMem - 1) - 1) = size;
        return alignedMem;
    }

    void Allocator::Free(void *block, size_t unused = 0)
    {
        (void)unused;

        if (block)
        {
            void *base = *((void **)block - 1);

            free(base);
        }
    }

    Allocator gAllocator;

    uint8_t NpPool[SCE_NP_MIN_POOL_SIZE];
}

void* operator new[](size_t, const char*, int, unsigned, const char*, int)
{
    EA_FAIL();

    return (void *)0;
}

// Fill this in with your actual license.  This one is from FIFA St. 3
SceNpDrmKey myDrmLicense = {
{
    0xEA,0x00,0x00,0x0E,0xBA,0xAE,0xDA,0xFE,
    0x0A,0x0E,0xDA,0x00,0x00,0x00,0x00,0xEA }
};

const char *gGameTitleId = "ABAB22335";
char DRM_CACHE_DIRECTORY[CELL_GAME_PATH_MAX];

void init()
{
    int ret = CELL_OK;

    cellSysutilRegisterCallback(0, NULL, NULL);

    ret = cellSysmoduleLoadModule( CELL_SYSMODULE_IO );
    EA_ASSERT(ret == CELL_OK);

    ret = cellSysmoduleLoadModule( CELL_SYSMODULE_FS );
    EA_ASSERT(ret == CELL_OK );

    ret = cellSysmoduleLoadModule( CELL_SYSMODULE_SYSUTIL_NP );
    EA_ASSERT(ret == CELL_OK );

    ret = cellSysmoduleLoadModule( CELL_SYSMODULE_SYSUTIL_GAME );
    EA_ASSERT(ret == CELL_OK );

    ret = sceNpInit(SCE_NP_MIN_POOL_SIZE, NpPool);
    EA_ASSERT(ret == CELL_OK);

    CellGameContentSize size;

    ret = cellGameDataCheck( CELL_GAME_GAMETYPE_GAMEDATA,
        gGameTitleId, &size);

    if (ret == CELL_GAME_RET_NONE)
    {
        CellGameSetInitParams init;
        memset(&init, 0, sizeof(init));

        EA_STRLCPY_VERIFY(init.title, "DLC Simple Sample", CELL_GAME_SYSP_TITLE_SIZE);


        EA_STRLCPY_VERIFY(init.titleId, gGameTitleId, CELL_GAME_SYSP_TITLEID_SIZE);


        EA_STRLCPY_VERIFY(init.version, "01.00", CELL_GAME_SYSP_VERSION_SIZE);



        char tmpContentInfoPath[CELL_GAME_PATH_MAX];
        char tmpUserdirPath[CELL_GAME_PATH_MAX];
        ret = cellGameCreateGameData(&init, tmpContentInfoPath, tmpUserdirPath);
        EA_ASSERT( ret == CELL_OK );
    }

    char contentInfoPath[CELL_GAME_PATH_MAX];

    ret = cellGameContentPermit(contentInfoPath, DRM_CACHE_DIRECTORY);

    EA_ASSERT( ret == CELL_OK );

    filesys::Options opts(&gAllocator);
    filesys::Manager::CreateInstance(opts);
}

void shutdown()
{
    filesys::Manager::DestroyInstance();

    sceNpTerm();
}

void *LoadFile(const char *filename, unsigned int openflags, size_t *size)
{
    filesys::AsyncOp op;

    op.Open(filename, openflags);
    filesys::Handle *hFile = op.GetResultHandle();

    EA_ASSERT(hFile != NULL);

    uint64_t file_size = filesys::GetSize(hFile);

    void *buffer = malloc(static_cast<size_t>(file_size));

    op.Read(hFile, buffer, 0, file_size);
    uint64_t nread = op.GetResultSize();

    EA_ASSERT(nread == file_size);

    op.Close(hFile);
    op.Wait();

    *size = file_size;

    return buffer;
}

bool CheckFileForDrm(const char *name)
{
    // This function can be used to look the file name up in 
    // a dictionary to see whether it should be opened encrypted
    // or not.  This example says to just assume any file in
    // the DRM cache directory should be tried.
    return !EA::StdC::Strncmp(name, DRM_CACHE_DIRECTORY, 31/*strlen(BASE_DIRECTORY)*/);
}

int main()
{
    init();

    filesys::Manager *fsMgr = filesys::Manager::GetInstance();

    const char *file_name = "packed.big";
    char path[filesys::FILESYS_MAX_PATH];

    sprintf(path, "%s/%s.edat", DRM_CACHE_DIRECTORY, file_name);

    int ret = sceNpDrmIsAvailable(&myDrmLicense, path);
    EA_ASSERT(ret == CELL_OK);

    size_t expectedSize(0), fSize(0);

    // Load the file from the local directory.
    void *refFile = LoadFile(file_name, filesys::AsyncOp::FOPEN_READONLY, &expectedSize);

    if (filesys::Exists(path))
    {
        rw::core::filesys::gHddDriverImpl.UseDRM(true, &myDrmLicense,
            CheckFileForDrm);

        fsMgr->AddSearchLocation(DRM_CACHE_DIRECTORY,
            filesys::Manager::SEARCHPATH_ADD_TO_HEAD);

        // Now load the file again, this time it'll be picked up from the
        // GameData cache location if you have installed it there
        void *pFile = LoadFile(file_name,
            filesys::AsyncOp::FOPEN_READONLY, &fSize);

        fsMgr->RemoveSearchLocation(DRM_CACHE_DIRECTORY);

        EA_ASSERT(pFile != NULL);
        EA_ASSERT(fSize == expectedSize);
        EA_ASSERT(!memcmp(refFile, pFile, fSize));
    }
    else
    {
        printf("The data file for this example is not present on the target PS3.  It must be installed there first\n");
    }

    free(refFile);

    shutdown();

    return 0;
}
