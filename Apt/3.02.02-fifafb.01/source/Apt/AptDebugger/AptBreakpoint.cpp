/******************************************************************************/
/*!
    Copyright 2010 Electronic Arts Inc.

    \file       AptBreakpoint

    \brief      Breakpoint implementation
*/
/******************************************************************************/


/*** Include files ********************************************************************************/
#include "Apt.h"
#include "_Apt.h"
#include "AptTarget.h"
#include "AptDebugger/AptBreakpoint.h"

#ifdef APT_DEBUGGER_ENABLE

/*** Namespace ****************************************************************/

/*** Macros *******************************************************************/

/*** Constants ****************************************************************/

/*** Declarations *************************************************************/
extern bool IsValidInstruction(const unsigned char* entry, unsigned int offset);
extern bool IsBreakpointInstruction(const unsigned char* entry, unsigned int offset);
extern bool AptFileNameCompare(const char *str1, const char *str2);

/*** Variables ****************************************************************/
APT_DEFINE_OBJECT_METRICS_EXPLICIT(AptBreakpoint, NULL);

/*** Implementation ***********************************************************/

/*** AptBreakpoint ***********************************************************/

/******************************************************************************/
/*!
    AptBreakpoint::AptBreakpoint()
    \brief  Constructor of AptBreakpoint
*/
/******************************************************************************/
AptBreakpoint::AptBreakpoint()
: mOffset(0)
, mIsActivated(false)
, mHitCount(0)
, mInstructionCode(AptActionInvalid)
{
    mFileName[0] = '\0';
    for (int i = 0; i < TIME_STAMP_SIZE; ++i)
    {
        mTimeStamp[i] = 0;
    }
}

/******************************************************************************/
/*!
    AptBreakpoint::~AptBreakpoint()
    \brief  Destructor of AptBreakpoint
*/
/******************************************************************************/
AptBreakpoint::~AptBreakpoint()
{
}

/******************************************************************************/
/*!
    AptBreakpoint::Create
    \brief  Create a new breakpoint with specified file name, offset and
            instruction code

    \param  fileName        Indicate the file name which contains the breakpoint.
    \param  offset          Offset in byte-code stream where the breakpoint is.
    \param  timeStamp       Time stamp of host APT file.

    \return Return true if success. Otherwise return false

    \note   File name length cannot exceed MAX_FILE_NAME_LENGTH.
*/
/******************************************************************************/
bool AptBreakpoint::Create(const char* fileName, unsigned int offset, char* timeStamp)
{
    strncpy(mFileName, fileName, MAX_FILE_NAME_LENGTH);
    mOffset = offset;
    if ( NULL != timeStamp) // If timeStamp == NULL which means this one is a temp breakpoint.
    {
        for (int i = 0; i < TIME_STAMP_SIZE; ++i)
        {
            mTimeStamp[i] = timeStamp[i];
        }
    }
    mInstructionCode = AptActionInvalid;
    mIsActivated = false;
    mHitCount = 0;
    return true;
}

/******************************************************************************/
/*!
    AptBreakpoint::Destroy()
    \brief  Destroy the breakpoint, internal state will be reset
    \note   Always, when anyone decide to destroy any breakpoint includes temp breakpoints
            User should call deactivate function to remove all breakpoints firstly.
*/
/******************************************************************************/
void AptBreakpoint::Destroy()
{
    APT_ASSERT(false == mIsActivated);
    mFileName[0] = '\0';
    mOffset = 0;
    mIsActivated = false;
    mHitCount = 0;
    mInstructionCode = 0xFF;
}

#endif
