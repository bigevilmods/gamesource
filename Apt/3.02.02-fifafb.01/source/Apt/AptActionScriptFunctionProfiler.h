/******************************************************************************/
/*!
    AptActionScriptFunctionProfiler
    Copyright 2007 Electronic Arts Inc.

    \file       ActionScriptFunctionProfiler.h
    \ingroup    Apt
    \brief      Class to profile and log action script function calls.
    \Notes
        This was first developed by Mike Balfour and Lance Jackson on Madden 08 team.
        This profiles actionscript functions. User can use this even in release mode.
        Turn it on in AptDefine.h (APT_DO_AS_PROFILE). This profiler is not a C++ code level
        profiler, but an actionscript profiler. It includes overhead of calling AS functions,
        (CallFucntion), finding variables from correct scope (getVariable) etc.
        AS Profiler uses AptActionSetup structure to save information inside profile log.
        The profiling is done on a frame level. When AptUpdate() is entered, frame logging
        starts. And frame logging ends when AptUpdate exits. When LogFrameEnd() is called
        at that time, it checks for the threshholdMS value and decides whether to output
        the log. Currently log is outputted in .gpftxt format. It is a simple text file
        that looks like following. But user can open this file in GenProf viewer which shows
        this file in a view similar to xbperf view.
        //--------------------- Frame Start ----------------------
        991 start AptRun-Actions
        1123 start   assetLoaded
        1362 start     onAllAssetsLoaded
        1474 start       init
        1967 end
        1994 end
        1998 end
        2020 end
        //--------------------- Frame End ------------------------

    \Version    1.0        05/15/07 (ASK) First Version
    \Version    1.1        07/10/09 (RB)  Increase MAX_FILENAME_LENGTH to 48
*/
/******************************************************************************/

/*** Include guard ************************************************************/

#ifndef APT_ACTIONSCRIPTFUNCTIONPROFILER_H
#define APT_ACTIONSCRIPTFUNCTIONPROFILER_H

/*** Includes *****************************************************************/
#include "Apt.h"
#include "_AptInternalDefines.h"

/*** Macros *******************************************************************/

/*** Interface ****************************************************************/

// --- Constants

// --- Forward declarations

#if defined (APT_DO_AS_PROFILE)

class AptActionScriptFunctionProfiler
{
    public:
        AptActionScriptFunctionProfiler();
        ~AptActionScriptFunctionProfiler();

        // FIFA_BEGIN | SENDBACK | cburns | 02-10-2015 | Fix static deinitialization of Apt
        void        Shutdown();
        // FIFA_END

        void        Start();
        void        Stop();
        void        InitializeBuffer();
        void        ShutdownBuffer();
        void        InitializeFileRootBuffer();
        bool8_t     IsProfiling() const {return mEnabled;}

// FIFA_BEGIN | SENDBACK | clong | May 08, 2015 | Make function profiler use microseconds instead of milliseconds
//        uint32_t    GetThreshold() const {return mThreshold;}
//        void        SetThreshold(uint32_t threshold) {mThreshold = threshold;}
        uint64_t    GetThreshold() const {return mThreshold;}
        void        SetThreshold(uint64_t threshold) {mThreshold = threshold;}
// FIFA_END
        uint32_t    GetBufferSize() const {return mLogBufferSize;}
        void        SetBufferSize(uint32_t bufferSize);

        void        SetLogFilePath(const char* path);

        void        LogFrameStart();
        void        LogFrameEnd();

        void        LogFunctionStart(const char8_t *name, bool bNativeFunc);
        void        LogFunctionEnd(const char8_t *name);

        void        LogBlockStart(const char8_t* name);
        void        LogBlockEnd();

        void        OutputLog();

    private:
        enum OpCode_t
        {
            OPCODE_INVALID = 0,
            OPCODE_FUNCTION_START,
            OPCODE_FUNCTION_END,
            OPCODE_BLOCK_START,
            OPCODE_BLOCK_END,
            OPCODE_FRAME_START,
            OPCODE_FRAME_END,
			OPCODE_NATIVE_FUNCTION_START,
			OPCODE_NATIVE_FUNCTION_END,
        };

        // helper function to write opcode and timestamp into buffer
        void _LogOpCodeAndTimestamp(OpCode_t code);

// FIFA_BEGIN | SENDBACK | clong | May 08, 2015 | Make function profiler use microseconds instead of milliseconds
//        // __mftb returns a counter that's roughly 50 MHz.  So if we divide our times by 50, we roughly get microseconds.
//        // (Checked against GetTickCount()).
//        static const uint32_t FRAME_TIME_TO_MICROSECONDS = 50;
// FIFA_END
        static const uint32_t MAX_LOG_ENTRY_LENGTH = 128;
        static const uint32_t MAX_FILENAME_LENGTH = 128;

        static const uint32_t OPCODE_SIZE = sizeof(OpCode_t);
        static const uint32_t TIMESTAMP_SIZE = sizeof(uint32_t);

        char       *mLogFilePath;

        char8_t    *mLogBuffer;
        char8_t    *mLogBufferEnd;
        char8_t    *mLogPosition;
        uint32_t    mLogBufferSize;

// FIFA_BEGIN | SENDBACK | clong | May 08, 2015 | Make function profiler use microseconds instead of milliseconds
//        uint32_t    mThreshold;
//        uint32_t    mFrameStartTime;
//        uint32_t    mFunctionEndTime;
        uint64_t    mThreshold;
        uint64_t    mFrameStartTime;
        uint64_t    mFunctionEndTime;
// FIFA_END
        uint32_t    mProfileLogCount;

        bool8_t     mEnabled;

        char8_t    *mLastFunctionStartPosition;
        bool8_t     mIgnoreNextFunctionEnd;
};

/*** Implementation ***********************************************************/

/******************************************************************************/

#endif // defined (APT_DO_AS_PROFILE)

#endif // ACTIONSCRIPTFUNCTIONPROFILER_H
