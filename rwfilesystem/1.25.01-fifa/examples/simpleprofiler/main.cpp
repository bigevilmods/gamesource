// (c) 2007 - 2009 Electronic Arts

#include <EABase/eabase.h>
#include <coreallocator/icoreallocator_interface.h>

#include <EAStdC/EAAlignment.h>
#include <rw/core/filesys/async.h>
#include <rw/core/filesys/loadsave.h>
#include <rw/core/filesys/manager.h>

#include "SimpleProfiler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <new>

#ifdef EA_PLATFORM_PS3
    #include <cell/sysmodule.h>
#endif

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

    void *alignedMem = EA::StdC::AlignAddressUp((uint8_t *)mem + sizeof(void *) + sizeof(size_t), alignment);

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

//-----------------------------------------------------------------------------
// Required for EASTL

void* operator new[](size_t size, const char* pName, int flags, unsigned debugFlags, const char* file, int line)
{
    (void)pName;
    (void)flags;
    (void)debugFlags;
    (void)file;
    (void)line;

    return gAllocator.Alloc( size, NULL, 0 );
}


void* operator new[](size_t size, size_t alignment, size_t alignmentOffset, const char* pName, int flags, unsigned debugFlags, const char* file, int line)
{
    (void)alignmentOffset;
    (void)pName;
    (void)flags;
    (void)debugFlags;
    (void)file;
    (void)line;

    return gAllocator.Alloc( size, NULL, 0, static_cast<unsigned int>(alignment), 0 );
}

void operator delete[]( void * p )
{
    return gAllocator.Free( p );
}

//-----------------------------------------------------------------------------


static void CreateData()
{
    size_t bufferSize = 1024 * 1024;
    void *buffer = gAllocator.Alloc(bufferSize, NULL, 0 );

    rw::core::filesys::AsyncOp ao;
    unsigned int modeflags = rw::core::filesys::AsyncOp::FOPEN_CREATE;
    if( rw::core::filesys::Exists( "data.dat" ) )
    {
        modeflags = rw::core::filesys::AsyncOp::FOPEN_OVERWRITE;
    }

    ao.Open( "data.dat", modeflags | rw::core::filesys::AsyncOp::FOPEN_READWRITE );
    ao.Wait();
    if( ao.GetResultHandle() )
    {
        ao.Write( ao.GetResultHandle(), buffer, 0, bufferSize );
    }

    ao.Close( ao.GetResultHandle() );
    ao.Wait();

    gAllocator.Free( buffer );
}

int main( int, char *argv[] )
{
    #if defined EA_PLATFORM_PS3
        int ret = 0;

        ret = cellSysmoduleLoadModule( CELL_SYSMODULE_IO );
        if ( ret != CELL_OK )
        {
            printf( "ERROR : cellSysmoduleLoadModule( CELL_SYSMODULE_IO ) = 0x%x\n", ret );
        }

        ret = cellSysmoduleLoadModule( CELL_SYSMODULE_FS );
        if ( ret != CELL_OK )
        {
            printf( "ERROR : cellSysmoduleLoadModule( CELL_SYSMODULE_FS ) = 0x%x\n", ret );
        }
    #endif

    (void)argv;

    rw::core::filesys::Options options( &gAllocator );
    rw::core::filesys::Manager::CreateInstance( options );

    fpa::SimpleProfiler profiler;
    rw::core::filesys::Manager::GetInstance()->SetEventProfiler( &profiler );
    profiler.SetTag( "MAIN" );

    const size_t BUFFER_SIZE = 64 * 1024;
    char *fileBuffer = static_cast< char * >( gAllocator.Alloc( BUFFER_SIZE, __FILE__, __LINE__ ) );

    CreateData();

    rw::core::filesys::AsyncOp ao;
    ao.Open( "data.dat" );
    ao.Wait();
    if( ao.GetResultHandle() )
    {
        uint64_t size = rw::core::filesys::GetSize( "data.dat" );
        uint64_t readOffset = 0;
        uint64_t readSize;
        while( size != 0 )
        {
            if( size > BUFFER_SIZE )
            {
                readSize = BUFFER_SIZE;
            }
            else
            {
                readSize = size;
            }

            ao.Read( ao.GetResultHandle(), fileBuffer, readOffset, readSize );
            ao.Wait();

            size -= readSize;
            readOffset += readSize;
        }

        ao.Close( ao.GetResultHandle() );
        ao.Wait();
    }

    rw::core::filesys::Delete( "data.dat" );

    gAllocator.Free( fileBuffer );

    #if defined EA_PLATFORM_PS3
        profiler.Save( "/app_home/profile.txt" );
    #elif defined EA_PLATFORM_XENON
        profiler.Save( "D:\\profile.txt" );
    #elif defined EA_PLATFORM_REVOLUTION
        profiler.Save();
    #else
        profiler.Save( "profile.xml" );
        //profiler.Save();
    #endif

    rw::core::filesys::Manager::DestroyInstance();

    #ifdef EA_PLATFORM_PS3
        // Unload modules
        ret = cellSysmoduleUnloadModule( CELL_SYSMODULE_FS );
        if ( ret != CELL_OK )
        {
            printf( "ERROR : cellSysmoduleUnloadModule( CELL_SYSMODULE_FS ) = 0x%x\n", ret );
        }

        ret = cellSysmoduleUnloadModule( CELL_SYSMODULE_IO );
        if ( ret != CELL_OK )
        {
            printf( "ERROR : cellSysmoduleUnloadModule( CELL_SYSMODULE_IO ) = 0x%x\n", ret );
        }
    #endif

    return 0;
}
