// (c) 2007 - 2009 Electronic Arts

#ifndef RW_CORE_FILESYS_EXAMPLE_SIMPLEPROFILER_H
#define RW_CORE_FILESYS_EXAMPLE_SIMPLEPROFILER_H

#include <eathread/eathread_futex.h>
#include <EABase/eabase.h>
#include <EAAssert/eaassert.h>
#include <EASTL/string.h>
#include <EASTL/vector.h>
#include <rw/core/filesys/profiler.h>
#include <EAStdC/EAStopwatch.h>

namespace fpa {

/*!
    \brief A Simple Profiler for the file system.

    This profiler can be used by applications to collect file system profile
    data. The data that it outputs by calling the Save method can be loaded
    by the File Profile Analyzer.

    Because the profile does not want to have impact on the I/O of the
    application, all profile data is gathered in a buffer. This buffer is
    properly saved to a profile data file (or default output) by calling the
    Save method.
*/
#ifdef EA_FILESYSTEM_PROFILING
class SimpleProfiler : public rw::core::filesys::FileSysProfiler
{
public:

    SimpleProfiler();
    ~SimpleProfiler();

    /*!
        \brief Reserve space for the profile buffer (so it will take less time for memory allocations)
    */
    void Reserve(uint32_t reserveSize)
    {
        mBuffer.reserve(reserveSize);
    }

    /*!
        \brief Enable recording profile information.

        (By default the Simple Profiler
        is enabled.) You can use this to temporarily enable/disable the
        profiler logging.
        \see Disable
    */
    void Enable();


    /*!
        \brief Disable recording profile information.

        (By default the Simple Profiler
        is enabled.) You can use this to temporarily enable/disable the
        profiler logging.
        \see Enable
    */
    void Disable();


    /*!
        Clears the profile data record buffer.
    */
    void Clear();


    /*!
        \brief Sets the tag for the profile events.

        The tag is a null terminated string (the recommended length is 4 characters). 
        The tag can be used in the profile data to identify certain sections of the code. For example
        one could have a "GUI" section "CARS", "SND", etc. This tag will
        show up in the File Profile Analyzer tool.
    */
    void SetTag( const char *tag );


    /*!
        \brief Saves the profile data.

        This saves the profile data to either a file or the default output
        stream. If the path == 0, then the default output stream is used. If
        the path != 0 then the path is the path to a filename to where the
        profile data will be saved.
    */
    void Save( const char *path = 0 );



    /*!
        \brief The callback that will be called by the RwFileSystem.

        This callback will be called by the RwFileSystem for each profile event
        that it recognizes.
    */
    void Callback( rw::core::filesys::ProfilerEvent type, const char *devicename, void *param, void *context );


private:

    uint64_t CreateKey( intptr_t handle, intptr_t buffer )
    {
        return (static_cast<uint64_t>(buffer) << 32) + static_cast<uint64_t>(handle);
    }

    // This infotype is written to the profile data buffer, so that the save
    // function knows what data to expect.
    enum InfoType
    {
        // SimpleProfiler
        TAG_INFO = 0,
        NO_SUCH_FILE_INFO,

        // RwFileSystem
        OPEN_COMPLETE_INFO,
        CLOSE_COMPLETE_INFO,
        READ_INFO,
        READ_COMPLETE_INFO,
        READ_START_INFO,
        READ_END_INFO,
        WRITE_INFO,
        WRITE_COMPLETE_INFO,
        WRITE_START_INFO,
        WRITE_END_INFO,
        SEEK_START_INFO,
        SEEK_END_INFO,

        // RwBigFile
        BIGFILE_OPEN_INFO,
        BIGFILE_CLOSE_INFO,

        // Additional application specific file data
        ADDITIONAL_FILE_DATA,
        //
        INFOTYPE_COUNT,
    };

    // Because we write the type as a single char, we have to be sure it fits.
    // If this assertion fires, update the Read/Write InfoType methods
    EA_COMPILETIME_ASSERT( INFOTYPE_COUNT < 256 );

    void Read( void *buffer, size_t size );
    void Write( const void *buffer, size_t size );

    void ReadInfoType( InfoType *value );
    void WriteInfoType( InfoType value );

    void ReadUint32( uint32_t *value );
    void WriteUint32( uint32_t value );

    void ReadUint64( uint64_t *value );
    void WriteUint64( uint64_t value );

    void ReadIntPtr( intptr_t *value );
    void WriteIntPtr( intptr_t value );

    void ReadString( char *string );
    void WriteString( const char *string );

    EA::StdC::Stopwatch mTimer;
    eastl::vector<char> mBuffer;
    bool mActive;
    eastl::string mActiveTag;

    size_t mReadOffset;
    size_t mWriteOffset;

    EA::Thread::Futex mFutex;

    SimpleProfiler(const SimpleProfiler&);
    SimpleProfiler& operator=(const SimpleProfiler&);
};
#endif

} // namespace

#endif
