// (c) 2007 - 2009 Electronic Arts

#include "SimpleProfiler.h"

#include <EAAssert/eaassert.h>
#include <EASTL/map.h>
#include <EASTL/string.h>
#include <stdio.h>


namespace fpa {
	
#ifdef EA_FILESYSTEM_PROFILING

// If defined prints out a lot of data while running.
// This can be used to find bugs in the profiler itself.
// #define SIMPLE_PROFILER_VERBOSE

SimpleProfiler::SimpleProfiler()
    : mActive( true )
    , mReadOffset( 0 )
    , mWriteOffset( 0 )
{
    mTimer.SetUnits( EA::StdC::Stopwatch::kUnitsMicroseconds );
    mTimer.Start();

    mActiveTag = "NONE";
}


SimpleProfiler::~SimpleProfiler()
{

}


void SimpleProfiler::Clear()
{
    mReadOffset = 0;
    mWriteOffset = 0;
    mBuffer.clear();
}


void SimpleProfiler::Enable()
{
    mActive = true;
}


void SimpleProfiler::Disable()
{
    mActive = false;
}


void SimpleProfiler::SetTag( const char *tag )
{
    if( mActive )
    {
        EA_ASSERT( strlen( tag ) < 128 );
        WriteInfoType( TAG_INFO );
        WriteString( tag );
    }
}


void SimpleProfiler::Callback( rw::core::filesys::ProfilerEvent type, const char *devicename, void *param, void *context )
{
    EA::Thread::AutoFutex autoFutex(mFutex);

    // Prevent compiler warning 'unused argument'
    (void)devicename;
    (void)param;
    (void)context;

    if( mActive )
    {
        uint64_t time = mTimer.GetElapsedTime();

        switch( type )
        {
        // ProfilerEventOpen: We do not need this event
        case rw::core::filesys::ProfilerEventOpen:
            break;

        // ProfilerEventOpenComplete will write away the file path in
        // combination with it's file handle (and parent's file handle).
        // The file handle is a unique file handle (also across multiple
        // devices), so we will use the handle to look up the file names
        // in later events.
        case rw::core::filesys::ProfilerEventOpenComplete:
            {
                rw::core::filesys::ProfilerEventOpenCompleteStruct *peocs = reinterpret_cast< rw::core::filesys::ProfilerEventOpenCompleteStruct * >( param );

                // Make sure the file exists
                if( peocs->handle )
                {
                    // Strip out all the separator filename (used by BIG)
                    int lastSeparator = 0;
                    {
                        int offset = 0;
                        while( peocs->filePath[ offset ] != 0 )
                        {
                            if( peocs->filePath[ offset ] == '|' ||
                                peocs->filePath[ offset ] == ':' )
                            {
                                lastSeparator = offset;
                            }

                            if(peocs->filePath[ offset ] == ':' && (peocs->filePath[ offset+1 ] == '\\' || peocs->filePath[ offset+1 ] == '/'))
                            {
                                lastSeparator = offset + 1;
                            }

                            ++offset;
                        }

                        if( lastSeparator != 0 )
                        {
                            ++lastSeparator;
                        }
                    }

                    #if defined EA_PLATFORM_PS3
                        const char *app_home = strstr(peocs->filePath, "app_home");
                        if(app_home)
                        {
                            lastSeparator = (app_home - peocs->filePath + 9);
                        }
                    #endif

                    WriteInfoType( OPEN_COMPLETE_INFO );
                    WriteIntPtr( peocs->handle );
                    WriteIntPtr( reinterpret_cast< intptr_t >( peocs->devFileHandle ) );
                    WriteString( &peocs->filePath[ lastSeparator ] );
                }
                else
                {
                    // The file could not be opened... this can be used to warn the user
                    WriteInfoType( NO_SUCH_FILE_INFO );
                    WriteString( peocs->filePath );
                }
            }
            break;

        // ProfilerEventClose: We do not need this event
        case rw::core::filesys::ProfilerEventClose:

            break;

        // ProfilerEventCloseComplete will write away the handle of the file
        // that is closed. This will be used in the save function to remove
        // the entry from the lookup table.
        case rw::core::filesys::ProfilerEventCloseComplete:
            {
                EA_ASSERT_MSG( param != 0, "File handle is invalid!" );
                WriteInfoType( CLOSE_COMPLETE_INFO );
                WriteIntPtr( reinterpret_cast< intptr_t >( param ) );
            }
            break;

        // ProfilerEventRead & ProfilerEventWrite: either a read or a write
        // was started, but because these have the same profile data coming in
        // from the RwFileSystem we share the code (we only change writing
        // the correct InfoType.)
        case rw::core::filesys::ProfilerEventRead:
        case rw::core::filesys::ProfilerEventWrite:
            {
                rw::core::filesys::ProfilerEventReadStruct *eventReadStruct = static_cast< rw::core::filesys::ProfilerEventReadStruct * >(param);

                WriteInfoType( type == rw::core::filesys::ProfilerEventRead ? READ_INFO : WRITE_INFO );
                WriteIntPtr( eventReadStruct->handle );
                WriteUint64( time );
                WriteUint64( eventReadStruct->ofs );
                WriteUint64( eventReadStruct->nbytes );
                WriteIntPtr( reinterpret_cast<intptr_t>(eventReadStruct->raddr) );
            }
            break;

        // ProfilerEventReadComplete & ProfilerEventWriteComplete: either a
        // read or a write completed, but because these have the same profile
        // data coming in from the RwFileSystem we share the code (we only
        // change writing the correct InfoType.)
        case rw::core::filesys::ProfilerEventReadComplete:
        case rw::core::filesys::ProfilerEventWriteComplete:
            {
                rw::core::filesys::ProfilerEventReadCompleteStruct *eventReadCompleteStruct = static_cast< rw::core::filesys::ProfilerEventReadCompleteStruct * >(param);

                WriteInfoType( type == rw::core::filesys::ProfilerEventReadComplete ? READ_COMPLETE_INFO : WRITE_COMPLETE_INFO );
                WriteIntPtr( eventReadCompleteStruct->handle );
                WriteUint64( time );
                WriteString( devicename );
                WriteIntPtr( reinterpret_cast<intptr_t>(eventReadCompleteStruct->buffer) );
            }
            break;

        // ProfilerEventDeviceReadStart & ProfilerEventDeviceWriteStart: either
        // a read or a was started on the device, but because these have the
        // same profile data coming in from the RwFileSystem we share the code
        // (we only change writing the correct InfoType.)
        case rw::core::filesys::ProfilerEventDeviceReadStart:
        case rw::core::filesys::ProfilerEventDeviceWriteStart:
            {
                rw::core::filesys::ProfilerEventReadOrWriteStruct *perow = static_cast< rw::core::filesys::ProfilerEventReadOrWriteStruct * >(param);

                WriteInfoType( type == rw::core::filesys::ProfilerEventDeviceReadStart ? READ_START_INFO : WRITE_START_INFO );
                WriteIntPtr( perow->handle );
                WriteUint64( time );
                WriteUint64( perow->location );
                WriteIntPtr( reinterpret_cast<intptr_t>(perow->buffer) );
                WriteIntPtr(static_cast<intptr_t>(perow->priority));
            }
            break;

        // ProfilerEventDeviceReadEnd & ProfilerEventDeviceWriteEnd: either
        // a read or a was started on the device, but because these have the
        // same profile data coming in from the RwFileSystem we share the code
        // (we only change writing the correct InfoType.)
        case rw::core::filesys::ProfilerEventDeviceReadEnd:
        case rw::core::filesys::ProfilerEventDeviceWriteEnd:
            {
                rw::core::filesys::ProfilerEventReadOrWriteStruct *perow = static_cast< rw::core::filesys::ProfilerEventReadOrWriteStruct * >(param);

                WriteInfoType( type == rw::core::filesys::ProfilerEventDeviceReadEnd ? READ_END_INFO : WRITE_END_INFO );
                WriteIntPtr( perow->handle );
                WriteUint64( time );
                WriteIntPtr( reinterpret_cast<intptr_t>(perow->buffer) );
            }
            break;

        // ProfilerEventSeek: We do not need this event
        case rw::core::filesys::ProfilerEventSeek:
            break;

        // ProfilerEventDeviceSeekStart: A seek was started within either a
        // read or write operation.
        case rw::core::filesys::ProfilerEventDeviceSeekStart:
            {
                rw::core::filesys::ProfilerEventSeekStruct2 *pess = static_cast< rw::core::filesys::ProfilerEventSeekStruct2 * >(param);

                WriteInfoType( SEEK_START_INFO );
                WriteIntPtr( pess->handle );
                WriteUint64( time );
                WriteIntPtr( reinterpret_cast<intptr_t>(pess->buffer) );
            }
            break;

        // ProfilerEventDeviceSeekEnd: A seek was ended within either a
        // read or write operation.
        case rw::core::filesys::ProfilerEventDeviceSeekEnd:
            {
                rw::core::filesys::ProfilerEventSeekStruct2 *pess = static_cast< rw::core::filesys::ProfilerEventSeekStruct2 * >(param);

                WriteInfoType( SEEK_END_INFO );
                WriteIntPtr( pess->handle );
                WriteUint64( time );
                WriteIntPtr( reinterpret_cast<intptr_t>(pess->buffer) );
            }
            break;

        // ProfilerEventBigFileOpen: A big file was opened. We write it's
        // handle out, because we can use this to match a big file to a
        // normal file in the ProfilerEventOpenComplete. In the
        // ProfilerEventOpenComplete we write the file's parent handle which
        // is the same as the handle we find here.
        case rw::core::filesys::ProfilerEventBigFileOpen:
            {
                rw::core::filesys::ProfilerEventBigFileOpenStruct *bigfileOpenStruct = static_cast< rw::core::filesys::ProfilerEventBigFileOpenStruct * >(param);

                WriteInfoType( BIGFILE_OPEN_INFO );
                WriteString( bigfileOpenStruct->bigfileName );
                WriteString( bigfileOpenStruct->fileName );
                WriteIntPtr( reinterpret_cast< intptr_t >( bigfileOpenStruct->voidPtr ) );
            }
            break;

        // ProfilerEventBigFileClose will write away the handle of the file
        // that is closed. This will be used in the save function to remove
        // the entry from the lookup table.
        case rw::core::filesys::ProfilerEventBigFileClose:
            {
                WriteInfoType( BIGFILE_CLOSE_INFO );
                WriteIntPtr( reinterpret_cast< intptr_t >( param ) );
            }
            break;

        case rw::core::filesys::ProfilerEventAdditionalFileDetail:
            {
                rw::core::filesys::ProfilerEventAdditionalFileDetailStruct *eventStruct = static_cast< rw::core::filesys::ProfilerEventAdditionalFileDetailStruct * >(param);

                WriteInfoType( ADDITIONAL_FILE_DATA );
                WriteIntPtr( eventStruct->handle );
                WriteIntPtr( reinterpret_cast<intptr_t>(eventStruct->buffer) );
                WriteUint64( eventStruct->fileId );
                WriteString( eventStruct->fileName );
                WriteUint32( eventStruct->fileType );
            }
            break;
        // In any other case (most likely because RwFileSystem has been changed)
        // we don't recognize the profile event type and 'gracefully' crash the
        // application. If it reaches this point, someone needs to update this
        // SimpleProfiler implementation.
        default:
            EA_FAIL();
            break;
        }
    }
}


// A lot of the events give a partial view of when an operation started/ended,
// for example ReadStart/ReadEnd. Because we need to know the time it took the
// read to complete we need to keep track of this. The Entry structure stores
// enough information so that we can figure out how long certain aspects took.
// Here is an overview of how the times work out:
//
// ----------------------- time line
// ^  ^  ^ ^       ^  ^
// 1  2  3 4       5  6
//
// 1 = read/write operation start
// 2 = device's seek start
// 3 = device's seek end
// 4 = device's read/write start
// 5 = device's read/write end
// 6 = read/write operation start
//
//
// Then there are two duration fields: readOrWriteDuration & seekDuration.
// For the profiler we are only interested in the time between 1
// (read/write operation start) and 6 (read/write operation start).
// But the RwFileSystem may decide to carve up a large read or write
// operation into multiple smaller reads/writes, this will trigger more events
// of type 2,3,4 and 5. Therefore we need to keep track of the total amount of
// time spent on seeking and writing, for the encompassing value 1 to 6. The
// two duration fields: readOrWriteDuration & seekDuration take care of this.
//
// Finally there is some read/write info we need regarding location/size.
// readOrWriteOffset is the offset from within a file where the data is read/written
// readOrWriteSize is the amount of data that was written (as requested by the
// application, the OS may decide to read/write more, because it needs to be block
// aligned)
// readOrWriteLocation is the physical location on the disk for the operation to
// occur, this is used by the File Profile Analyzer tool to figure out good/bad
// disc layouts.
struct Entry
{
    uint64_t    totalStartTime;
    uint64_t    totalEndTime;

    uint64_t    seekStartTime;
    uint64_t    seekEndTime;

    uint64_t    readOrWriteStartTime;
    uint64_t    readOrWriteEndTime;

    uint64_t    readOrWriteDuration;
    uint64_t    seekDuration;

    uint64_t    readOrWriteOffset;
    uint64_t    readOrWriteSize;
    uint64_t    readOrWriteLocation;
    int         priority;
};


struct SimpleProfiler_AdditionalFileData
{
    eastl::string mFileName;
    uint64_t      mFileId;
    uint32_t      mFileType;
};


void SimpleProfiler::Save( const char *filepath )
{
    EA::Thread::AutoFutex autoFutex(mFutex);

    typedef eastl::map< intptr_t, eastl::pair< eastl::string, eastl::string > > t_HandleToFileNameLut;
    t_HandleToFileNameLut handleToFileNameLut;

    typedef eastl::map< intptr_t, eastl::string > t_BigHandleToFileNameLut;
    t_BigHandleToFileNameLut bigHandleToFileNameLut;

    typedef eastl::map< uint64_t, Entry > t_HandleToEntryLut;
    t_HandleToEntryLut handleToEntryLut;

    typedef eastl::map< uint64_t, SimpleProfiler_AdditionalFileData > t_AdditionalFileDataLut;
    t_AdditionalFileDataLut additionalFileDataLut;

    // Check to see whether output should be routed to a file... if not route it to stdout
    bool realFile = false;
    FILE *file = NULL;
    {
        if( filepath )
        {
            file = fopen( filepath, "w+" );
            realFile = file != NULL;
        }

        if( file == NULL )
        {
            file = stdout;
        }
    }

    // Print the XML header
    fprintf( file, "<SimpleProfiler>\n" );
    fprintf( file, "<Header version=\"0.1\" platform=\"%s\"/>\n", EA_PLATFORM_NAME );
    fprintf( file, "<Events>\n" );

    // Local vars, used in the while loop
    InfoType infoType;
    intptr_t handle;
    char string1[ 1024 ];
    char string2[ 1024 ];
    intptr_t intptr1;
    uint64_t time;
    uint64_t size;
    uint64_t location;
    uint64_t offset;
    intptr_t buffer;
    int priority;

    // While we didn't reach the end of the record buffer
    while( mReadOffset < mWriteOffset )
    {
        ReadInfoType( &infoType );

        switch( infoType )
        {
            case ADDITIONAL_FILE_DATA:
                {
                    uint64_t fileId;
                    uint32_t fileType;

                    ReadIntPtr( &handle );
                    ReadIntPtr( &buffer );
                    ReadUint64( &fileId );
                    ReadString( string1 );
                    ReadUint32( &fileType );

                    t_AdditionalFileDataLut::insert_return_type ret;
                    uint64_t key = CreateKey(handle, buffer);
                    ret = additionalFileDataLut.insert( key );
                    if( ret.second )
                    {
                        ret.first->second.mFileName = eastl::string( string1 );
                        ret.first->second.mFileId = fileId;
                        ret.first->second.mFileType = fileType;
                    }
                }
                break;
        // Change the active tag
        case TAG_INFO:
            {
                // Clear the string (this is neccesary because of the way we copy the tag into the active tag)
                ReadString( string1 );
                mActiveTag = string1;

                #ifdef SIMPLE_PROFILER_VERBOSE
                    printf( "TAG_INFO: tag = %s\n", mActiveTag.c_str() );
                #endif
            }
            break;

        // We tried to open a file that doesn't exist
        case NO_SUCH_FILE_INFO:
            ReadString( string1 );
            #ifdef SIMPLE_PROFILER_VERBOSE
                    printf( "NO_SUCH_FILE_INFO: filePath = %s\n", string1 );
            #endif
            fprintf( file, "<OpenFail file=\"%s\"/>", string1 );
            break;

        // The open was completed.
        // The handle will be added to the lookup tables, which can then be used for find the
        // filename for each operation.
        case OPEN_COMPLETE_INFO:
            {
                ReadIntPtr( &handle );
                ReadIntPtr( &intptr1 );
                ReadString( string1 );

                #ifdef SIMPLE_PROFILER_VERBOSE
                    printf( "OPEN_COMPLETE_INFO: handle = 0x%0p, filePath = %s, intptr_t = 0x%p\n", handle, string1, intptr1 );
                #endif

                // Insert handle to filename
                EA_ASSERT( handle != 0  );
                t_HandleToFileNameLut::insert_return_type ret;
                ret = handleToFileNameLut.insert( handle );

                // Make sure we could insert the filename... if this fails that
                // either means that the handle is not unique or that a file
                // with that handle has neven been closed... which should not
                // happen, but it somehow did. If we were to ignore this, the
                // profiler would still work, but the filenames for the profile
                // data would be wrong.
                if( ret.second )
                {
                    EA_ASSERT( string1[ 0 ] != 0 );
                    ret.first->second.first = eastl::string( string1 );

                    // Find handle to entry (for BIG)
                    t_BigHandleToFileNameLut::iterator iter = bigHandleToFileNameLut.find( intptr1 );
                    if( iter != bigHandleToFileNameLut.end() )
                    {
                        EA_ASSERT( iter->second[ 0 ] != 0 );
                        ret.first->second.second = eastl::string( iter->second );
                    }
                }
            }
            break;

        // The close was complete.
        // The handle will be removed from the lookup table, this ensures that
        // the next open can use this handle again.
        case CLOSE_COMPLETE_INFO:
            {
                ReadIntPtr( &handle );
                EA_ASSERT( handle != 0 );

                #ifdef SIMPLE_PROFILER_VERBOSE
                    printf( "CLOSE_COMPLETE_INFO: handle = 0x%0p\n", handle );
                #endif


                t_HandleToFileNameLut::iterator iter = handleToFileNameLut.find( handle );
                if( iter != handleToFileNameLut.end() )
                {
                    handleToFileNameLut.erase( iter );
                }
            }
            break;

        // The read/write started
        // This event is triggerd only once and is matched with the
        // READ/WRITE_COMPLETE_INFO. Within these events multiple
        // READ/WRITE_START_INFO, READ/WRITE_END_INFO, SEEK_START_INFO
        // and SEEK_END_INFO may occur.
        case READ_INFO:
        case WRITE_INFO:
            {
                ReadIntPtr( &handle );
                ReadUint64( &time );
                ReadUint64( &offset );
                ReadUint64( &size );
                ReadIntPtr( &buffer );

                #ifdef SIMPLE_PROFILER_VERBOSE
                    printf( "%s: handle = 0x%0p, time = %llu, offset = %llu, size = %llu\n", infoType == READ_INFO ? "READ_INFO" : "WRITE_INFO", handle, time, offset, size );
                #endif

                // Insert handle to entry
                t_HandleToEntryLut::insert_return_type ret;
                uint64_t key = CreateKey(handle, buffer);
                ret = handleToEntryLut.insert( key );
                if( ret.second )
                {
                    ret.first->second.totalStartTime = time;
                    ret.first->second.readOrWriteOffset = offset;
                    ret.first->second.readOrWriteSize = size;

                    // Because internally the RwFileSystem driver may respawn read/write events... we need to accumulate these values
                    ret.first->second.seekDuration = 0;
                    ret.first->second.readOrWriteDuration = 0;
                }
            }
            break;

        // The read/write is complete.
        // This is the point where we output the profile event data.
        case READ_COMPLETE_INFO:
        case WRITE_COMPLETE_INFO:
            {
                ReadIntPtr( &handle );
                ReadUint64( &time );
                ReadString( string1 );
                ReadIntPtr( &buffer );

                #ifdef SIMPLE_PROFILER_VERBOSE
                    printf( "%s: handle = 0x%0p, time = %llu\n", infoType == READ_COMPLETE_INFO ? "READ_COMPLETE_INFO" : "WRITE_COMPLETE_INFO", handle, time );
                #endif

                // Erase handle to entry
                uint64_t key = CreateKey(handle, buffer);
                t_HandleToEntryLut::iterator iter = handleToEntryLut.find( key );
                if( iter != handleToEntryLut.end() )
                {
                    iter->second.totalEndTime = time;

                    t_HandleToFileNameLut::iterator filenameIter = handleToFileNameLut.find( handle );
                    if( filenameIter != handleToFileNameLut.end() )
                    {
                        if( infoType == READ_COMPLETE_INFO )
                        {
                            fprintf( file, "<Read" );
                        }
                        else
                        {
                            fprintf( file, "<Write" );
                        }

                        fprintf( file, " time=\"%016llu\"", iter->second.totalStartTime );
                        fprintf( file, " seekTime=\"%016llu\"", iter->second.seekDuration );
                        fprintf( file, " %sTime=\"%016llu\"", infoType == READ_COMPLETE_INFO ? "read" : "write", iter->second.readOrWriteDuration );
                        fprintf( file, " actualTime=\"%016llu\"", iter->second.totalEndTime - iter->second.totalStartTime );
                        fprintf( file, " location=\"%016llu\"", iter->second.readOrWriteLocation );
                        fprintf( file, " %sOffset=\"%016llu\"", infoType == READ_COMPLETE_INFO ? "read" : "write", iter->second.readOrWriteOffset );
                        fprintf( file, " %sSize=\"%016llu\"", infoType == READ_COMPLETE_INFO ? "read" : "write", iter->second.readOrWriteSize );
                        fprintf( file, " priority=\"%d\"", iter->second.priority);

                        fprintf( file, " tag=\"%s\"", mActiveTag.c_str() );
                        fprintf( file, " device=\"%s\"", string1 );

                        t_AdditionalFileDataLut::iterator additionalFileDataIter = additionalFileDataLut.find( key );
                        if( additionalFileDataIter != additionalFileDataLut.end() )
                        {
                            if( filenameIter != handleToFileNameLut.end() )
                            {
                                fprintf( file, " bigfile=\"%s\"", filenameIter->second.first.c_str() );
                            }
                            else
                            {
                                fprintf( file, " bigfile=\"unknown\"" );
                            }

                            // If the addional file data's name is valid, use it. Otherwise use the Id
                            if( additionalFileDataIter->second.mFileName.length() )
                            {
                                fprintf( file, " file=\"%s (%d)\"", additionalFileDataIter->second.mFileName.c_str(), additionalFileDataIter->second.mFileType );
                            }
                            else
                            {
                                uint64_t fileId = additionalFileDataIter->second.mFileId;
                                fprintf( file, " file=\"0x%8X%8X (%d)\"", (unsigned int)(fileId >> 32),(uint32_t)fileId & 0xFFFFFFFF, additionalFileDataIter->second.mFileType );
                            }
                        }
                        else
                        {
                            if( filenameIter != handleToFileNameLut.end() )
                            {
                                fprintf( file, " bigfile=\"%s\"", filenameIter->second.second.c_str() );
                                fprintf( file, " file=\"%s\"", filenameIter->second.first.c_str() );
                            }
                        }

                        fprintf( file, "/>\n" );


                        handleToEntryLut.erase( iter );
                    }
                }
            }
            break;

        // The read/write started...
        // we look up the handle and add the start time and location to the entry
        case READ_START_INFO:
        case WRITE_START_INFO:
            {
                ReadIntPtr( &handle );
                ReadUint64( &time );
                ReadUint64( &location );
                ReadIntPtr( &buffer );
                intptr_t prio;
                ReadIntPtr(&prio);
                priority = static_cast<int>(prio);

                #ifdef SIMPLE_PROFILER_VERBOSE
                    printf( "%s: handle = 0x%0p, time = %llu, location = %llu, priority = %d\n", infoType == READ_START_INFO ? "READ_START_INFO" : "WRITE_START_INFO", handle, time, location, static_cast<int>(priority) );
                #endif

                // Find handle to entry
                uint64_t key = CreateKey(handle, buffer);
                t_HandleToEntryLut::iterator iter = handleToEntryLut.find(key);
                if( iter != handleToEntryLut.end() )
                {
                    iter->second.readOrWriteStartTime = time;
                    iter->second.readOrWriteLocation = location;
                    iter->second.priority = priority;
                }
            }
            break;

        // The read ended... we look up the handle and add the end time to the entry,
        // we also sum up the duration of the read because (as described above)
        // large reads may be broken up by the RwFileSystem into multiple smaller reads
        case READ_END_INFO:
        case WRITE_END_INFO:
            {
                ReadIntPtr( &handle );
                ReadUint64( &time );
                ReadIntPtr( &buffer );

                #ifdef SIMPLE_PROFILER_VERBOSE
                    printf( "%s: handle = 0x%0p, time = %llu\n", infoType == READ_END_INFO ? "READ_END_INFO" : "WRITE_END_INFO", handle, time );
                #endif

                // Find handle to entry
                uint64_t key = CreateKey(handle, buffer);
                t_HandleToEntryLut::iterator iter = handleToEntryLut.find(key);
                if( iter != handleToEntryLut.end() )
                {
                    iter->second.readOrWriteEndTime = time;
                    iter->second.readOrWriteDuration += iter->second.readOrWriteEndTime - iter->second.readOrWriteStartTime;
                }
            }
            break;

        // The seek started... we look up the handle and add the start time to the entry
        case SEEK_START_INFO:
            {
                ReadIntPtr( &handle );
                ReadUint64( &time );
                ReadIntPtr( &buffer );

                #ifdef SIMPLE_PROFILER_VERBOSE
                    printf( "SEEK_START_INFO: handle = 0x%0p, time = %llu, buffer = %p\n", handle, time, buffer );
                #endif

                // Find handle to entry
                uint64_t key = CreateKey(handle, buffer);
                t_HandleToEntryLut::iterator iter = handleToEntryLut.find(key);
                if( iter != handleToEntryLut.end() )
                {
                    iter->second.seekStartTime = time;
                }
            }
            break;

        // The seek ended... we look up the handle and add the end time to the entry,
        // we also sum up the duration of the seek time, because (as described above)
        // large reads may be broken up by the RwFileSystem into multiple smaller reads
        case SEEK_END_INFO:
            {
                ReadIntPtr( &handle );
                ReadUint64( &time );
                ReadIntPtr( &buffer );

                #ifdef SIMPLE_PROFILER_VERBOSE
                    printf( "SEEK_END_INFO: handle = 0x%0p, time = %llu\n", handle, time );
                #endif

                // Find handle to entry
                uint64_t key = CreateKey(handle, buffer);
                t_HandleToEntryLut::iterator iter = handleToEntryLut.find(key);
                if( iter != handleToEntryLut.end() )
                {
                    iter->second.seekEndTime = time;
                    iter->second.seekDuration += iter->second.seekEndTime - iter->second.seekStartTime;
                }
            }
            break;

        // A bigfile is opened, we add the big file's handle to the lookup table
        // so that we can link the big file to a normal file in the
        // OPEN_COMPLETE_INFO case.
        case BIGFILE_OPEN_INFO:
            {
                ReadString( string1 );
                ReadString( string2 );
                ReadIntPtr( &intptr1 );

                #ifdef SIMPLE_PROFILER_VERBOSE
                    printf( "BIGFILE_OPEN_INFO: bigfileName = %s, fileName = %s, parenthandle = 0x%0p\n", string1, string2, intptr1 );
                #endif

                t_BigHandleToFileNameLut::insert_return_type ret;
                ret = bigHandleToFileNameLut.insert( intptr1 );
                if( ret.second )
                {
                    ret.first->second = eastl::string( string1 );
                }
            }
            break;

        // A bigfile is was closed, we remove the big file's handle from the
        // lookup table so that it can be reused in the BIGFILE_OPEN_INFO.
        case BIGFILE_CLOSE_INFO:
            {
                ReadIntPtr( &intptr1 );

                #ifdef SIMPLE_PROFILER_VERBOSE
                    printf( "BIGFILE_OPEN_INFO: voidPtr = 0x%p\n", intptr1 );
                #endif

                t_BigHandleToFileNameLut::iterator iter = bigHandleToFileNameLut.find( intptr1 );
                if( iter != bigHandleToFileNameLut.end() )
                {
                    bigHandleToFileNameLut.erase( iter );
                }
            }
            break;

        // Some number came up we didn't recognize.
        // On a working version of this software, this is most likely caused by memory
        // corruption in the calling application. In any case... to hopefully not crash
        // the application we force the end of profile data saving.
        default:
            EA_FAIL();
            mReadOffset = mWriteOffset;     // Make sure we do not read further
            break;
        }
    }

    // Print the XML footer
    fprintf( file, "</Events>\n" );
    fprintf( file, "</SimpleProfiler>\n" );

    // If we wrote to a real file (not to the output stream) then we close
    if( realFile )
    {
        fclose( file );
    }

    // Clear the profile buffer for the next run
    Clear();
}


void SimpleProfiler::Read( void *buffer, size_t size )
{
    memcpy( buffer, &mBuffer[ static_cast<eastl::vector<char>::size_type>(mReadOffset) ], size );
    mReadOffset += size;
}


void SimpleProfiler::Write( const void *buffer, size_t size )
{
    const char* buf = static_cast<const char*>(buffer);

    for(size_t i=0; i < size; ++i)
    {
        mBuffer.push_back(buf[i]);
    }

    mWriteOffset += size;
}


void SimpleProfiler::ReadInfoType( InfoType *value )
{
    char c;
    Read( &c, sizeof( char ) );
    *value = static_cast< InfoType >( c );
}


void SimpleProfiler::WriteInfoType( InfoType value )
{
    char c = static_cast< char >( value );
    Write( &c, sizeof( char ) );
}


void SimpleProfiler::ReadUint32( uint32_t *value )
{
    Read( value, sizeof( uint32_t ) );
}


void SimpleProfiler::WriteUint32( uint32_t value )
{
    Write( &value, sizeof( uint32_t ) );
}


void SimpleProfiler::ReadUint64( uint64_t *value )
{
    Read( value, sizeof( uint64_t ) );
}


void SimpleProfiler::WriteUint64( uint64_t value )
{
    Write( &value, sizeof( uint64_t ) );
}


void SimpleProfiler::ReadIntPtr( intptr_t *value )
{
    Read( value, sizeof( intptr_t ) );
}


void SimpleProfiler::WriteIntPtr( intptr_t value )
{
    Write( &value, sizeof( intptr_t ) );
}


void SimpleProfiler::ReadString( char *string )
{
    size_t di = 0;
    char c;

    do
    {
        Read( &c, 1 );
        string[ di++ ] = c;
    }
    while( c != 0 );
}


void SimpleProfiler::WriteString( const char *string )
{
    size_t length = strlen( string );
    Write( string, length + 1 );
}

#endif
} // namespace
