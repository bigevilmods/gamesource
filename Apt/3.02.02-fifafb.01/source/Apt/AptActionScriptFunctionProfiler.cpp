/******************************************************************************/
/*!
    AptActionScriptFunctionProfiler
    Copyright 2007 Electronic Arts Inc.

    \file       AptActionScriptFunctionProfiler.cpp
    \ingroup    Apt
    \brief      Class to profile and log action script function calls.

    \Version    1.0        07/10/09  (RB)  Added date comment to existing revision
    \Version    1.1        07/10/09  (RB)  Prepend /app_home/ to ps3 output log name
    \Version    1.2        07/22/09  (DG)  Changes made to be ps3sdk 280.001-SNC280-lite compatible
*/
/******************************************************************************/

/*** Documentation ************************************************************/

/******************************************************************************/
/*! \class  AptActionScriptFunctionProfiler

    \brief  Class to profile and log action script function calls.
*/
/******************************************************************************/

/*** Includes *****************************************************************/

#include "AptActionScriptFunctionProfiler.h"

#if defined (APT_DO_AS_PROFILE)

#if defined (EA_PLATFORM_PS3) && !defined(__SNC__)
    #include "ppu_intrinsics.h"
#endif

#include "_AptInternalDefines.h"
#include "stdio.h"      // for string functions

/*** Using ********************************************************************/

/*** Macros *******************************************************************/

#define PROFILE_EVENT( type, name ) if( gAptFuncs.pfnActionScriptProfileEvent ) gAptFuncs.pfnActionScriptProfileEvent( type, name );

/*** Interface ****************************************************************/

/*** Variables ****************************************************************/

/*** Implementation ***********************************************************/

/******************************************************************************/
/*! AptActionScriptFunctionProfiler :: AptActionScriptFunctionProfiler

    \brief  AptActionScriptFunctionProfiler default constructor.

    \param          - none.
    \return         - void.
*/
/******************************************************************************/
AptActionScriptFunctionProfiler::AptActionScriptFunctionProfiler():
    mLogFilePath(NULL),
    mLogBuffer(NULL),
    mLogBufferEnd(NULL),
    mLogPosition(NULL),
    mLogBufferSize(200 * 1024),
    mThreshold(100 * 1000),
    mFrameStartTime(0),
    mFunctionEndTime(0),
    mProfileLogCount(0),
    mEnabled(false),
    mLastFunctionStartPosition(NULL),
    mIgnoreNextFunctionEnd(false)
{
}

/******************************************************************************/
/*! AptActionScriptFunctionProfiler :: ~AptActionScriptFunctionProfiler

    \brief  AptActionScriptFunctionProfiler destructor.

    \param          - none.
    \return         - void.
*/
/******************************************************************************/
// FIFA_BEGIN | SENDBACK | cburns | 02-10-2015 | Fix static deinitialization of Apt
AptActionScriptFunctionProfiler::~AptActionScriptFunctionProfiler()
{
    Shutdown();
}

/******************************************************************************/
/*! AptActionScriptFunctionProfiler :: Shutdown

\brief  Free all dynamic memory (since this is effectively a static variable)

\param          - none.
\return         - void.
*/
/******************************************************************************/
void AptActionScriptFunctionProfiler::Shutdown()
{
    ShutdownBuffer();

    if (mLogFilePath)
    {
        APT_FREE_BLOCK(mLogFilePath, MAX_FILENAME_LENGTH);
        mLogFilePath = NULL;
    }
}
// FIFA_END

/******************************************************************************/
/*! AptActionScriptFunctionProfiler :: InitializeBuffer

    \brief  Allocate profiling buffer if profiling enabled.

    \param          - none.
    \return         - void.
*/
/******************************************************************************/
void AptActionScriptFunctionProfiler::InitializeBuffer()
{
    if (mEnabled && (mLogBuffer == NULL))
    {
        mLogBuffer = reinterpret_cast<char8_t *>(APT_MALLOC_BLOCK(mLogBufferSize));
        mLogPosition = mLogBuffer;
        mLogBufferEnd = mLogBuffer + mLogBufferSize;
    }
}

/******************************************************************************/
/*! AptActionScriptFunctionProfiler :: ShutdownBuffer

    \brief  Shutdown profiling buffer without changing profiling state.

    \param          - none.
    \return         - void.
*/
/******************************************************************************/
void AptActionScriptFunctionProfiler::ShutdownBuffer()
{
    if (mLogBuffer)
    {
        APT_FREE_BLOCK(mLogBuffer, mLogBufferSize);
    }

    mLogBuffer = NULL;
    mLogBufferEnd = NULL;
    mLogPosition = NULL;
}

/******************************************************************************/
/*! AptActionScriptFunctionProfiler :: Start

    \brief  Start profiling/logging action script function calls.

    \param          - none.
    \return         - void.
*/
/******************************************************************************/
void AptActionScriptFunctionProfiler::Start()
{
    mEnabled = true;

    if( NULL == mLogFilePath )
    {
        // Set the default file path for the current platform.
#if defined(EA_PLATFORM_XENON)
        const char* rootDirectory = "game:\\";
#elif defined(EA_PLATFORM_XBOXONE)                 // You can see the files created with this: "C:\Program Files (x86)\Microsoft Durango XDK\bin\xbdir.exe" XG:\ /X/title,
        const char* rootDirectory = "g:\\";         // and copy files to your hard drive with this: "C:\Program Files (x86)\Microsoft Durango XDK\bin\xbcp.exe" /X/title XG:\SomeFile.txt C:\Temp,
#elif defined(EA_PLATFORM_PS3)
        const char* rootDirectory = "/app_home/";   // /app_home/ works only from runs started by the debugger or TargetManager.
#elif defined(EA_PLATFORM_PS4)
        const char* rootDirectory = "/hostapp/";    // /hostapp/ is where the "file serving directory" is set. /app0/ is the working directory. When launched from the VS integrated debugger, it's whatever the working directory is set to be. /host/C:\... is a path that maps to a PC drive.
#else
        const char* rootDirectory = "";
#endif
    
        SetLogFilePath(rootDirectory);
    }

    InitializeBuffer();
}

/******************************************************************************/
/*! AptActionScriptFunctionProfiler :: Stop

    \brief  Stop profiling/logging action script function calls.

    \param          - none.
    \return         - void.
*/
/******************************************************************************/
void AptActionScriptFunctionProfiler::Stop()
{
    mEnabled = false;
    ShutdownBuffer();
}

/******************************************************************************/
/*! AptActionScriptFunctionProfiler :: SetBufferSize

    \brief  Update the profile log buffer size.

    \param          - profile log buffer size in bytes.
    \return         - void.
*/
/******************************************************************************/
void AptActionScriptFunctionProfiler::SetBufferSize(uint32_t bufferSize)
{
    if (mLogBufferSize != bufferSize)
    {
        // Release buffer memory if size changed
        if (mLogBuffer)
        {
            APT_FREE_BLOCK(mLogBuffer, mLogBufferSize);
            mLogBuffer = NULL;
            mLogBufferEnd = NULL;
            mLogPosition = NULL;
        }

        // Stop profiling to avoid thrashing memory allocations
        mEnabled = false;
        mLogBufferSize = bufferSize;
    }
}

/******************************************************************************/
/*! 
    \brief  Sets the path to use for the action script profiler log.

    \param          - path to use for the log file.
    \return         - void.
*/
/******************************************************************************/
void AptActionScriptFunctionProfiler::SetLogFilePath(const char* path)
{
    if (NULL == mLogFilePath)
    {
        mLogFilePath = reinterpret_cast<char *>(APT_MALLOC_BLOCK(MAX_FILENAME_LENGTH));
        mLogFilePath[0] = '\0';
    }

    strncpy(mLogFilePath, path, MAX_FILENAME_LENGTH);
    mLogFilePath[MAX_FILENAME_LENGTH - 1] = '\0';
}

/******************************************************************************/
/*! AptActionScriptFunctionProfiler :: LogFrameStart

    \brief  Log the start of a frame.

    \param          - none.
    \return         - void.
*/
/******************************************************************************/
void AptActionScriptFunctionProfiler::LogFrameStart()
{
    if (mEnabled)
    {
        // Reset to beginning of log buffer
        mLogPosition = mLogBuffer;
        mFunctionEndTime = 0;

// FIFA_BEGIN | SENDBACK | clong | May 08, 2015 | Make function profiler use microseconds instead of milliseconds
// DELETE THESE COMMENTS when removing tags, they no longer apply, we use a portable system now
//        // This works for X360/PS3, but needs to be revisited for other platforms
//        //mFrameStartTime = static_cast<uint32_t>(__mftb());
// FIFA_END
        mFrameStartTime = gAptFuncs.pfnGetCurrentTime();

        APT_ASSERT(mLogPosition);
        APT_ASSERT(mLogBufferEnd);
        if ((mLogPosition + OPCODE_SIZE) < mLogBufferEnd)
        {
            // Write frame start opcode to buffer
            *mLogPosition = OPCODE_FRAME_START;
            mLogPosition += OPCODE_SIZE;
        }
    }
	PROFILE_EVENT( OPCODE_FRAME_START, NULL );
}

/******************************************************************************/
/*! AptActionScriptFunctionProfiler :: LogFrameEnd

    \brief  Log the end of a frame.

    \param          - none.
    \return         - void.
*/
/******************************************************************************/
void AptActionScriptFunctionProfiler::LogFrameEnd()
{
    if (mEnabled && mLogBuffer)
    {
        APT_ASSERT(mLogPosition);
        APT_ASSERT(mLogBufferEnd);
        if ((mLogPosition + OPCODE_SIZE) < mLogBufferEnd)
        {
            // Write frame end opcode to buffer
            *mLogPosition = OPCODE_FRAME_END;
            mLogPosition += OPCODE_SIZE;
        }

        if (mFunctionEndTime > mThreshold)
        {
            // Threshold exceeded - write out profile log
            OutputLog();
        }
    }
	PROFILE_EVENT( OPCODE_FRAME_END, NULL );
}

/******************************************************************************/
/*! AptActionScriptFunctionProfiler :: LogFunctionStart

    \brief  Log the start of an action script function call.

    \param  name    - function name
    \return         - void.
*/
/******************************************************************************/
void AptActionScriptFunctionProfiler::LogFunctionStart(const char8_t *name, bool bNativeFunc)
{
    if (mEnabled && mLogBuffer)
    {
        //uint32_t nameSize = EA::StdC::Strlen(name) + 1;
        uint32_t nameSize = static_cast<uint32_t>(strlen(name) + 1);

        APT_ASSERT(mLogPosition);
        APT_ASSERT(mLogBufferEnd);
        if ((mLogPosition + OPCODE_SIZE + TIMESTAMP_SIZE + nameSize) < mLogBufferEnd)
        {
            // save last OPCODE_FUNCTION_START position to "rewind" the buffer if needed
            mLastFunctionStartPosition = mLogPosition;

            _LogOpCodeAndTimestamp(OPCODE_FUNCTION_START);

            // Write function name to buffer
            //EA::StdC::StringnCopy(mLogPosition, name, nameSize);
            strncpy(mLogPosition, name, nameSize);
            mLogPosition += nameSize;
        }
    }

	PROFILE_EVENT( bNativeFunc ? OPCODE_NATIVE_FUNCTION_START : OPCODE_FUNCTION_START, name );
}

/******************************************************************************/
/*! AptActionScriptFunctionProfiler :: LogFunctionEnd

    \brief  Log the end of an action script function call.

    \param  name    - function name
    \return         - void.
*/
/******************************************************************************/
void AptActionScriptFunctionProfiler::LogFunctionEnd(const char8_t* name)
{
    if (mEnabled && !mIgnoreNextFunctionEnd)
    {
        if ((mLogPosition + OPCODE_SIZE + TIMESTAMP_SIZE) < mLogBufferEnd)
        {
            _LogOpCodeAndTimestamp(OPCODE_FUNCTION_END);
        }
    }
    // we just passed the function end, so clear mIgnoreNextFunctionEnd
    mIgnoreNextFunctionEnd = false;

	PROFILE_EVENT( OPCODE_FUNCTION_END, name );
}

/******************************************************************************/
/*! AptActionScriptFunctionProfiler :: LogBlockStart

    \brief  Log the start of an action script block of code.

    \param  blockName - name of the block of code
    \return           - void.
*/
/******************************************************************************/
void AptActionScriptFunctionProfiler::LogBlockStart(const char8_t* blockName)
{
    if (mEnabled && mLogBuffer)
    {
        uint32_t nameSize = static_cast<uint32_t>(strlen(blockName) + 1);

        if ((mLogPosition + OPCODE_SIZE + TIMESTAMP_SIZE + nameSize) < mLogBufferEnd)
        {
            APT_ASSERT(mLastFunctionStartPosition);

            // rewind buffer to stomp the last OPCODE_FUNCTION_START
            // this makes the surrounding AptUtil.profileBlockStart()
            // call not to be logged
            mLogPosition = mLastFunctionStartPosition;

            _LogOpCodeAndTimestamp(OPCODE_BLOCK_START);

            // Write block name to buffer
            strncpy(mLogPosition, blockName, nameSize);
            mLogPosition += nameSize;

            // skip next function end
            // (the idea is that the name of the API doesn't end up logged, just the block name.)
            mIgnoreNextFunctionEnd = true;
        }
    }
	PROFILE_EVENT( OPCODE_BLOCK_START, blockName );
}

/******************************************************************************/
/*! AptActionScriptFunctionProfiler :: LogBlockEnd

    \brief  Log the end of an action script block of code.

    \return         - void.
*/
/******************************************************************************/
void AptActionScriptFunctionProfiler::LogBlockEnd()
{
    if (mEnabled)
    {
        APT_ASSERT(mLogPosition);
        APT_ASSERT(mLogBufferEnd);
        if ((mLogPosition + OPCODE_SIZE + TIMESTAMP_SIZE) < mLogBufferEnd)
        {
            APT_ASSERT(mLastFunctionStartPosition);

            // rewind buffer to stomp the last OPCODE_FUNCTION_START
            // this makes the surrounding AptUtil.profileBlockEnd()
            // call not to be logged
            mLogPosition = mLastFunctionStartPosition;

            _LogOpCodeAndTimestamp(OPCODE_BLOCK_END);

            // skip next function end
            // (the idea is that the name of the API doesn't end up logged, just the block name.)
            mIgnoreNextFunctionEnd = true;
        }
    }
	PROFILE_EVENT( OPCODE_BLOCK_END, NULL );
}

/******************************************************************************/
/*! AptActionScriptFunctionProfiler :: OutputLog

    \brief  Write out a log of the current profile data.

    \param          - none.
    \return         - void.
*/
/******************************************************************************/
void AptActionScriptFunctionProfiler::OutputLog()
{
    if (mEnabled && mLogBuffer)
    {
        char8_t *bufferPosition = mLogBuffer;
        int32_t  callStackDepth = 0;
        bool8_t  parsingBuffer = true;
        char8_t  dataBuffer[MAX_LOG_ENTRY_LENGTH];
        char8_t  filename[MAX_FILENAME_LENGTH];
        uint32_t fileOffset = 0;
        void    *fileHandle = NULL;

        // Create log file
        //EA::StdC::Snprintf(filename, MAX_FILENAME_LENGTH, "ActionScriptProf%03d.gpftxt", ++mProfileLogCount);
        SNPRINTF(filename, MAX_FILENAME_LENGTH, "%sActionScriptProf%03d.gpftxt", mLogFilePath, ++mProfileLogCount);
        fileHandle = gAptFuncs.pfnSyncOpenFile(filename);

        APT_ASSERT(mLogPosition);
        APT_ASSERT(mLogBufferEnd);
        while (parsingBuffer && (bufferPosition < mLogPosition))
        {
            // Read opcode from buffer
            char8_t opcode = *bufferPosition;
            bufferPosition += OPCODE_SIZE;

            switch (opcode)
            {
                case OPCODE_FUNCTION_START:
                case OPCODE_BLOCK_START:
                {
                    // Read time from buffer
                    // This may need to be optimized (can be misaligned read)
                    uint32_t startTime = *(reinterpret_cast<uint32_t *>(bufferPosition));
                    bufferPosition += TIMESTAMP_SIZE;

                    // Read name from buffer
                    char8_t *name = bufferPosition;
                    //bufferPosition += EA::StdC::Strlen(name) + 1;
                    bufferPosition += strlen(name) + 1;

                    // Create string for profile log entry
                    APT_ASSERT(callStackDepth >= 0);
                    int32_t dataSize = SNPRINTF(dataBuffer, MAX_LOG_ENTRY_LENGTH, "%9d start %s\n", startTime, name);

                    // if it ran out of room, don't use the negative dataSize!!!
                    // Also note that dataBuffer will not have an '\0' in it!
                    if (dataSize < 0)
                    {
                        dataSize = MAX_LOG_ENTRY_LENGTH;
                    }

                    // Write profile log entry to file
                    gAptFuncs.pfnSyncWriteToFile(fileHandle, fileOffset, dataBuffer, dataSize);
                    fileOffset += dataSize;

                    callStackDepth++;

                    break;
                }

                case OPCODE_FUNCTION_END:
                case OPCODE_BLOCK_END:
                {
                    // Read time from buffer
                    // This may need to be optimized (can be misaligned read)
                    uint32_t endTime = *(reinterpret_cast<uint32_t *>(bufferPosition));
                    bufferPosition += TIMESTAMP_SIZE;

                    // Create string for profile log entry
                    //uint32_t dataSize = EA::StdC::Snprintf(dataBuffer, MAX_LOG_ENTRY_LENGTH, "%9d end\n", endTime);
                    int32_t dataSize = SNPRINTF(dataBuffer, MAX_LOG_ENTRY_LENGTH, "%9d end\n", endTime);

                    if (dataSize < 0)
                    {
                        dataSize = MAX_LOG_ENTRY_LENGTH;
                    }

                    // Write profile log entry to file
                    gAptFuncs.pfnSyncWriteToFile(fileHandle, fileOffset, dataBuffer, dataSize);
                    fileOffset += dataSize;

                    // Update callstack depth
                    callStackDepth--;
                    APT_ASSERT(callStackDepth >= 0);
                    break;
                }

                case OPCODE_FRAME_START:
                {
                    // Create string for profile log entry
                    //uint32_t dataSize = EA::StdC::Snprintf(dataBuffer, MAX_LOG_ENTRY_LENGTH, "//--------------------- Frame Start ----------------------\n");
                    int32_t dataSize = SNPRINTF(dataBuffer, MAX_LOG_ENTRY_LENGTH, "//--------------------- Frame Start ----------------------\n");

                    if (dataSize < 0)
                    {
                        dataSize = MAX_LOG_ENTRY_LENGTH;
                    }

                    // Write profile log entry to file
                    gAptFuncs.pfnSyncWriteToFile(fileHandle, fileOffset, dataBuffer, dataSize);
                    fileOffset += dataSize;
                    break;
                }

                case OPCODE_FRAME_END:
                {
                    // Create string for profile log entry
                    //uint32_t dataSize = EA::StdC::Snprintf(dataBuffer, MAX_LOG_ENTRY_LENGTH, "//--------------------- Frame End ------------------------\n");
                    int32_t dataSize = SNPRINTF(dataBuffer, MAX_LOG_ENTRY_LENGTH, "//--------------------- Frame End ------------------------\n");

                    if (dataSize < 0)
                    {
                        dataSize = MAX_LOG_ENTRY_LENGTH;
                    }

                    // Write profile log entry to file
                    gAptFuncs.pfnSyncWriteToFile(fileHandle, fileOffset, dataBuffer, dataSize);
                    fileOffset += dataSize;

                    // Stop parsing at end of frame for now
                    // Multi-frame log support pending
                    parsingBuffer = false;
                    break;
                }

                case OPCODE_INVALID:
                default:
                {
                    // Invalid opcode found - abort
                    parsingBuffer = false;

                    // Create string for error message
                    //uint32_t dataSize = EA::StdC::Snprintf(dataBuffer, MAX_LOG_ENTRY_LENGTH, "// !!!! INVALID OPCODE - ABORTING !!!!\n");
                    int32_t dataSize = SNPRINTF(dataBuffer, MAX_LOG_ENTRY_LENGTH, "// !!!! INVALID OPCODE - ABORTING !!!!\n");

                    if (dataSize < 0)
                    {
                        dataSize = MAX_LOG_ENTRY_LENGTH;
                    }

                    // Write error message to file
                    gAptFuncs.pfnSyncWriteToFile(fileHandle, fileOffset, dataBuffer, dataSize);
                    fileOffset += dataSize;
                    break;
                }
            }

            // sanity check (we had some problems with writing past the end of arrays and clobbering things)
            // if somehow we are point somewhere before where we began, then something went horribly wrong.
            APT_ASSERT((bufferPosition >= mLogBuffer));
        }

        // Check for buffer overrun
        if (parsingBuffer)
        {
            // Create string for error message
            //uint32_t dataSize = EA::StdC::Snprintf(dataBuffer, MAX_LOG_ENTRY_LENGTH, "// !!!! OUT OF BUFFER SPACE !!!!\n");
            int32_t dataSize = SNPRINTF(dataBuffer, MAX_LOG_ENTRY_LENGTH, "// !!!! OUT OF BUFFER SPACE !!!!\n");

            // Write error message to file
            gAptFuncs.pfnSyncWriteToFile(fileHandle, fileOffset, dataBuffer, dataSize);
            fileOffset += dataSize;
        }

        // Close log file
        gAptFuncs.pfnSyncCloseFile(fileHandle);
    }
}

/******************************************************************************/
/*! AptActionScriptFunctionProfiler :: _LogOpCodeAndTimestamp

    \brief  Logs an opcode and its timestamp.
            Note: This function modifies mLogPosition. A precondition is that
            there's enough space in the buffer to write to.

    \param  code    - opcode to log
    \return         - void.
*/
/******************************************************************************/
void AptActionScriptFunctionProfiler::_LogOpCodeAndTimestamp(OpCode_t code)
{
    APT_ASSERT((mLogPosition + OPCODE_SIZE + TIMESTAMP_SIZE) < mLogBufferEnd);

    // Calculate frame-relative end time
// FIFA_BEGIN | SENDBACK | clong | May 08, 2015 | Make function profiler use microseconds instead of milliseconds
//    // This works for X360/PS3, but needs to be revisited for other platforms
//    uint32_t curTime = gAptFuncs.pfnGetCurrentTime();
//    mFunctionEndTime = (curTime - mFrameStartTime) / FRAME_TIME_TO_MICROSECONDS;
    mFunctionEndTime = gAptFuncs.pfnGetCurrentTime() - mFrameStartTime;
// FIFA_END

    // Write function end opcode to buffer
    *mLogPosition = code;
    mLogPosition += OPCODE_SIZE;

    // Write ending time to buffer
    // This may need to be optimized (can be misaligned write)
    uint32_t *timeEntry = reinterpret_cast<uint32_t *>(mLogPosition);
    *timeEntry = mFunctionEndTime;
    mLogPosition += TIMESTAMP_SIZE;
}

#endif // defined (APT_DO_AS_PROFILE)
