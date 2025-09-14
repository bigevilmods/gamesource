/******************************************************************************/
/*!
    Copyright 2010 Electronic Arts Inc.

    \file       AptDebugger

    \brief      Apt debugger implementation
*/
/******************************************************************************/

/*** Include files ********************************************************************************/

#include "Apt.h"
#include "_Apt.h"
#include "AptTarget.h"
#include "_AptActions.h"
#include "AptDebugger/AptDebugger.h"
#include "AptDebugger/IDebuggerListener.h"
#include "AptDebugger/AptBreakpoint.h"
#include "MainInline.h"

#include APT_INC_THREAD_H
#include APT_INC_THREAD_FUTEX_H

#ifdef APT_DEBUGGER_ENABLE


/*** Namespace ****************************************************************/

/*** Macros *******************************************************************/

/*** Constants ****************************************************************/

/*** Declarations *************************************************************/
extern bool AptFileNameCompare(const char *str1, const char *str2);

/*** Variables ****************************************************************/

AptDebugger* AptDebugger::sInstance = NULL;

// Synchronization mutex
APT_THREAD_NAMESPACE::Futex gDebuggerMutex;

APT_DEFINE_OBJECT_METRICS_EXPLICIT(AptDebugger, NULL);

/*** Implementation ***********************************************************/

/******************************************************************************/
/*!
    AptGetDebugger
    \brief  Return pointer to the instance of IDebugger

    \return Return pointer to the instance of IDebugger
*/
/******************************************************************************/
IDebugger* AptGetDebugger()
{
    return AptDebugger::GetInstance();
}

/******************************************************************************/
/*!
    IsValidInstruction
    \brief  Check if specified byte-code stream offset is a valid instruction

    \param  entry       Byte-code stream start address of the file
    \param  offset      Target place where will be checked in offset

    \return Return true if is a normal instruction; otherwise return false
*/
/******************************************************************************/
bool IsValidInstruction(const unsigned char* entry, unsigned int offset)
{
    if ( NULL == entry )
    {
        return false;
    }

    const unsigned char*    address = entry + offset;
    if ( *address < LastAptAction )
    {
        return true;
    }
    return false;
}

/******************************************************************************/
/*!
    IsBreakpointInstruction
    \brief  Check if specified byte-code stream offset is a breakpoint instruction

    \param  entry       Byte-code stream start address of the file
    \param  offset      Target place where will be checked in offset

    \return Return true if is a breakpoint; otherwise return false
*/
/******************************************************************************/
bool IsBreakpointInstruction(const unsigned char* entry, unsigned int offset)
{
    if ( NULL == entry )
    {
        return false;
    }

    const unsigned char*    address = entry + offset;
    if ( AptActionBreakpoint == *address )
    {
        return true;
    }
    return true;
}

/******************************************************************************/
/*!
    IsTimeStampMatch
    \brief  Check if target file's time stamp match the file is running.

    \param  fileName   - File name
    \param  timeStamp  - Time stamp of host APT file.

    \return  If match return true, else return false.
*/
/******************************************************************************/
bool IsTimeStampMatch(const char* fileName, char* timeStamp)
{
    AptAnimationFile* file = AptAnimationFile::Cast(gpCurrentTargetSim->GetLoader()->findFile(fileName));
    bool isTimeStampCorrect = true;
    for (int i = 0; i < file->GetTimeStampSize(); ++i)
    {
        if (file->GetTimeStamp()[i] != timeStamp[i])
        {
            isTimeStampCorrect =  false;
            break;
        }
    }
    return isTimeStampCorrect;
}

/*** AptDebugger    ***********************************************************/

/******************************************************************************/
/*!
    AptDebugger::AptDebugger
    \brief  AptDebugger constrcutor
*/
/******************************************************************************/
AptDebugger::AptDebugger()
: mBreakPointsCount(0)
, mTempBreakpointCount(0)
, mListener(NULL)
, mState(DEBUGGER_STATE_RUN)
{
    for (int i = 0; i < MAX_BREAKPOINTS; ++i)
    {
        mBreakpoint[i]  = &mBreakpointBuffer[i];
    }

    for (int i = 0; i < MAX_TEMP_BREAKPOINTS; ++i)
    {
        mTempBreakpoint[i] = &mBreakpointBuffer[MAX_BREAKPOINTS + i];
    }
}

/******************************************************************************/
/*!
    AptDebugger::AptDebugger
    \brief  AptDebugger destructor
*/
/******************************************************************************/
AptDebugger::~AptDebugger()
{
}

/******************************************************************************/
/*!
    AptDebugger::Initialize
    \brief  Initialize AptDebugger. Singleton instantiation function

    \return Return true if success. Otherwise return false
*/
/******************************************************************************/
bool AptDebugger::Initialize()
{
    APT_ASSERT(NULL == sInstance);

    if ( NULL == sInstance )
    {
        sInstance = new AptDebugger();
    }

    return true;
}

/******************************************************************************/
/*!
    AptDebugger::Shutdown
    \brief  Shutdown AptDebugger. Singleton destruction function

    \return Return true if success. Otherwise return false
*/
/******************************************************************************/
bool AptDebugger::Shutdown()
{
    if ( NULL != sInstance )
    {
        delete sInstance;
        sInstance = NULL;
    }

    return true;
}

/******************************************************************************/
/*!
    AptDebugger::GetInstance
    \brief  AptDebugger singleton instance function

    \return Return non-zero if success. Otherwise return NULL.
*/
/******************************************************************************/
AptDebugger* AptDebugger::GetInstance()
{
    return sInstance;
}

/******************************************************************************/
/*!
    AptDebugger::RegisterListener
    \brief  Register a debug event listener.

    \param  listener        Pointer to a Callback function which will handle
                            the debug events.

    \note   Currently, only one listener is supported.
*/
/******************************************************************************/
void AptDebugger::RegisterListener(IDebuggerListener* listener)
{
    APT_ASSERT(NULL == mListener);
    APT_THREAD_NAMESPACE::AutoFutex lock(gDebuggerMutex);
    mListener = listener;
}

/******************************************************************************/
/*!
    AptDebugger::UnregisterListener
    \brief  Unregister debug event listener.
*/
/******************************************************************************/
void AptDebugger::UnregisterListener()
{
    APT_ASSERT(NULL != mListener);
    APT_THREAD_NAMESPACE::AutoFutex lock(gDebuggerMutex);
    mListener = NULL;
}

/******************************************************************************/
/*!
    AptDebugger::AddBreakpoint
    \brief  Add a new breakpoint at the place which is specified by fileName
            and offset.

    \param  fileName        Indicate the file name which contains the breakpoint.
    \param  offset          Offset in byte-code stream where the breakpoint is.
    \param  timeStamp       Time stamp of host APT file.
*/
/******************************************************************************/
void AptDebugger::AddBreakpoint(const char* fileName, unsigned int offset, char* timeStamp)
{
    APT_THREAD_NAMESPACE::AutoFutex lock(gDebuggerMutex);

    BreakpointInfo * info = GetBreakpointInfo(fileName, offset);
    if ( NULL != info)
    {
        // If the breakpoint already exists, just send back an add message
        if ( NULL != mListener )
        {
            mListener->BreakpointAdd(info->mBreakpoint.GetFileName(), info->mBreakpoint.GetOffset());
            if (info->mBreakpoint.IsActivated())
            {
                // If the breakpoint is active, also send an activate message
                mListener->BreakpointActivate(info->mBreakpoint.GetFileName(), info->mBreakpoint.GetOffset());
            }
        }
        return;
    }

    if ( mBreakPointsCount < MAX_BREAKPOINTS )
    {
        APT_ASSERT(!mBreakpoint[mBreakPointsCount]->mIsValid);
        mBreakpoint[mBreakPointsCount]->mBreakpoint.Create(fileName, offset, timeStamp);
        mBreakpoint[mBreakPointsCount]->mIsValid = true;
        mBreakPointsCount++;
        if( NULL != mListener)
        {
            mListener->BreakpointAdd(fileName,offset);
        }
        if ( NULL != gpCurrentTargetSim )
        {
            ActivateBreakpoint(mBreakpoint[mBreakPointsCount - 1]->mBreakpoint, false);
        }
    }
}

/******************************************************************************/
/*!
    AptDebugger::GetTimeStamp
    \brief  Get time stamp of specific file name.

    \param  fileName        File name.
    \param  buf             Result bytes buffer

    \return  If the file exist return true, else return false;
*/
/******************************************************************************/
bool AptDebugger::GetTimeStamp(const char* fileName, char* buf)
{
    AptAnimationFile* file = AptAnimationFile::Cast(gpCurrentTargetSim->GetLoader()->findFile(fileName));
    if (NULL != file)
    {
        for (int i = 0; i < file->GetTimeStampSize(); ++i)
        {
            buf[i] = file->GetTimeStamp()[i];
        }
        return true;
    }
    else
    {
        for (int i = 0; i < file->GetTimeStampSize(); ++i)
        {
            buf[i] = 0;
        }
        return false;
    }
}

/******************************************************************************/
/*!
    AptDebugger::RemoveBreakpoint
    \brief  Remove a breakpoint at the place which is specified by fileName
            and offset.

    \param  fileName        Indicate the file name which contains the breakpoint.
    \param  offset          Offset in byte-code stream where the breakpoint is.
*/
/******************************************************************************/
void AptDebugger::RemoveBreakpoint(const char* fileName, unsigned int offset)
{
    APT_THREAD_NAMESPACE::AutoFutex lock(gDebuggerMutex);

    for ( int i = 0; i < mBreakPointsCount; i++ )
    {
        if ( AptFileNameCompare(mBreakpoint[i]->mBreakpoint.GetFileName(), fileName) && mBreakpoint[i]->mBreakpoint.GetOffset() == offset )
        {
            if ( mBreakpoint[i]->mBreakpoint.IsActivated() )
            {
                DeactivateBreakpoint(mBreakpoint[i]->mBreakpoint, false);
            }
            mBreakpoint[i]->mIsValid = false;
            mBreakpoint[i]->mBreakpoint.Destroy();
            // Move the unused slot to the end of list, keep the breakpoint list order.
            BreakpointInfo*  current = mBreakpoint[i];
            for ( int j = i; j < mBreakPointsCount - 1; j++ )
            {
                mBreakpoint[j] = mBreakpoint[j + 1];
            }
            mBreakpoint[mBreakPointsCount - 1] = current;
            mBreakPointsCount--;
            if(mListener != NULL)
            {
                mListener->BreakpointRemove(fileName, offset);
            }
            break;
        }
    }
}

/******************************************************************************/
/*!
    AptDebugger::RemoveAllBreakpoints
    \brief  Remove all user breakpoints exclude temp breakpoint.
*/
/******************************************************************************/
void AptDebugger::RemoveAllBreakpoints()
{
    APT_THREAD_NAMESPACE::AutoFutex lock(gDebuggerMutex);
    for (int i = mBreakPointsCount - 1; i >= 0; --i)
    {
        const char* fileName = mBreakpoint[i]->mBreakpoint.GetFileName();
        const int FILE_NAME_BUFFER_SIZE = 256;
        APT_ASSERT(strlen(fileName) < 256 - 1); // Exclude '\0' at the end of string
        char savedFileName[FILE_NAME_BUFFER_SIZE] = {0};
        strcpy(savedFileName, fileName); // FileName stored in breakpoint, breakpoint will be destroyed later
                                         // So have to store it's copy, mainly used in function "RemoveBreakpoint".
        unsigned int offset  = mBreakpoint[i]->mBreakpoint.GetOffset();
        RemoveBreakpoint(savedFileName, offset);
    }
}

/******************************************************************************/
/*!
    AptBreakpoint::Activate
    \brief  Activate breakpoint, place breakpoint instruction and backup
            original instruction.

    \param  breakpoint        AptBreakpoint to activate.
    \param  isTempBreakpoint  If this is a temp breakpoint.
*/
/******************************************************************************/
void AptDebugger::ActivateBreakpoint(AptBreakpoint& breakpoint, bool isTempBreakpoint)
{
    // Since when activate a user/temp breakpoint, you can find there is maybe
    // already a breakpoint at the same position. In this case the original operation code we got this time
    // must be "0xba"(AptActionBreakpoint). So here, checking whether the mentioned case happened
    AptAnimationFile* file = AptAnimationFile::Cast(gpCurrentTargetSim->GetLoader()->findFile(breakpoint.GetFileName()));
    BreakpointInfo* breakpointInfo = NULL;
    if (isTempBreakpoint)
    {
        breakpointInfo = GetBreakpointInfo(breakpoint.GetFileName(), breakpoint.GetOffset());
        unsigned char operationCode;
        if (NULL != breakpointInfo && breakpointInfo->mBreakpoint.IsActivated())
        {
            operationCode = breakpointInfo->mBreakpoint.GetInstructionCode();
            // The position has already been set a breakpoint before, so we should get the real
            // original operation code from tempBreakpointList or breakpointList.
            breakpoint.SetInstructionCode(operationCode);
            // We don't need to set AptActionBreakpoint at the same position.
        }
        else
        {
            unsigned char* baseStream = static_cast<unsigned char*>(file->GetAptData());
            unsigned char* address = baseStream + breakpoint.GetOffset();
            breakpoint.SetInstructionCode(*address);
            // In this case, the position should be set a AptActionBreakpoint instead the original operation code
            *address = AptActionBreakpoint;
        }
        breakpoint.SetActivated(true);
    }
    else
    {
        if ( NULL != file)
        {
            if ( IsTimeStampMatch( breakpoint.GetFileName(), breakpoint.GetTimeStamp() ) )
            {
                unsigned char* baseStream = static_cast<unsigned char*>(file->GetAptData());
                if ( NULL != baseStream && IsValidInstruction(baseStream, breakpoint.GetOffset()))
                {
                    // It is common that both temp breakpoint and user breakpoints are set at the same position.
                    // So when user remove any breakpoint, the original operation we stored before will work again.
                    // In this case the other breakpoint won't work which is not we want.
                    // In order to keep the breakpoint when remove one of these two breakpoints, we should store
                    // both of them in two breakpoint info arrays separately.
                    breakpointInfo = GetTempBreakpointInfo(breakpoint.GetFileName(), breakpoint.GetOffset());
                    unsigned char operationCode;
                    if (NULL != breakpointInfo)
                    {
                        APT_ASSERT(breakpointInfo->mBreakpoint.IsActivated());
                        operationCode = breakpointInfo->mBreakpoint.GetInstructionCode();
                        breakpoint.SetInstructionCode(operationCode);
                    }
                    else
                    {
                        unsigned char* baseStream = static_cast<unsigned char*>(file->GetAptData());
                        unsigned char* address = baseStream + breakpoint.GetOffset();
                        breakpoint.SetInstructionCode(*address);
                        *address = AptActionBreakpoint;
                    }
                    breakpoint.SetActivated(true);
                    if ( NULL != mListener )
                    {
                        // Fire activate breakpoint successfully.
                        mListener->BreakpointActivate(breakpoint.GetFileName(), breakpoint.GetOffset());
                    }
                }
            }
            else
            {
                if ( NULL != mListener )
                {
                    // time stamp doesn't match.
                    mListener->BreakpointDeactivate(breakpoint.GetFileName(), breakpoint.GetOffset(), IDebuggerListener::FILE_OVERDUE);
                }
            }
        }
        else
        {
            if ( NULL != mListener )
            {
                // Since file is not loaded or file does not exist.
                mListener->BreakpointDeactivate(breakpoint.GetFileName(), breakpoint.GetOffset(), IDebuggerListener::FILE_NOT_LOADED);
            }
        }
    }
}

/******************************************************************************/
/*!
    AptDebugger::DeactivateBreakpoint
    \brief  Deactivate the breakpoint, remove breakpoint instruction
            and put the original instruction back.

    \param breakpoint        AptBreakpoint to deactivate.
    \param isTempBreakpoint  If it is a temp breakpoint.
*/
/******************************************************************************/
void AptDebugger::DeactivateBreakpoint(AptBreakpoint& breakpoint, bool isTempBreakpoint)
{
    AptFilePtr file = gpCurrentTargetSim->GetLoader()->findFile(breakpoint.GetFileName());
    DeactivateBreakpoint(AptAnimationFile::Cast(file), breakpoint, isTempBreakpoint);
}

/******************************************************************************/
/*!
    AptDebugger::DeactivateBreakpoint
    \brief  Deactivate the breakpoint, remove breakpoint instruction
            and put the original instruction back, only used internally.

    \param file              Apt animation file.
    \param breakpoint        AptBreakpoint to deactivate.
    \param isTempBreakpoint  If it is a temp breakpoint.
*/
/******************************************************************************/
void AptDebugger::DeactivateBreakpoint(AptAnimationFile* file, AptBreakpoint& breakpoint, bool isTempBreakpoint)
{
    APT_ASSERT( NULL != file )
    // It is common that both temp breakpoint and user breakpoints are set at the same position.
    // So when user remove any breakpoint, the original operation we stored before will work again.
    // In this case the other breakpoint won't work which is not we want.
    // In order to prevent this case happen, we should operate specially.
    BreakpointInfo* breakpointInfo = NULL;
    if (isTempBreakpoint == true)
    {
        APT_ASSERT(breakpoint.IsActivated());
        breakpointInfo = GetBreakpointInfo(breakpoint.GetFileName(), breakpoint.GetOffset());
        // If there is no user breakpoint at the same position we just need to remove the breakpoint
        // and restore the original operation code.
        if (NULL == breakpointInfo || !breakpointInfo->mBreakpoint.IsActivated())
        {
            unsigned char* baseStream = static_cast<unsigned  char*>(file->GetAptData());
            unsigned char* address = baseStream + breakpoint.GetOffset();
            *address = breakpoint.GetInstructionCode();
        }
        // If breakpointInfo != NULL and the corresponding user breakpoint has been activated
        // which means both of temp and user breakpoint at the same position.
        // In this case, if we still restore the original operation code, then the user breakpoint
        // will not work. So here we just make the temp breakpoint inactivate is enough.
        breakpoint.SetActivated(false);
    }
    else
    {
        breakpointInfo = GetTempBreakpointInfo(breakpoint.GetFileName(), breakpoint.GetOffset());
        if(NULL == breakpointInfo)
        {
            if (breakpoint.IsActivated())
            {
                unsigned char* baseStream = static_cast<unsigned char*>(file->GetAptData());
                unsigned char* address = baseStream + breakpoint.GetOffset();
                *address = breakpoint.GetInstructionCode();
            }
        }
        else
        {
            // Almost all the situation is the same as above comment, but normally, temp breakpoint
            // must be activated if exist.
            APT_ASSERT(breakpointInfo->mBreakpoint.IsActivated());
        }
        breakpoint.SetActivated(false);
        if (NULL != mListener)
        {
            mListener->BreakpointDeactivate(breakpoint.GetFileName(), breakpoint.GetOffset(), IDebuggerListener::NORMAL_DEACTIVATE);
        }
    }
}

/******************************************************************************/
/*!
    AptDebugger::Trace
    \brief  Send a message from an ActionScript trace

    \param message           The message being traced
*/
/******************************************************************************/
void AptDebugger::Trace(const char* message)
{
    if (NULL != mListener)
    {
        mListener->Trace(message);
    }
}

/******************************************************************************/
/*!
    AptDebugger::RemoveAllTempBreakpoints
    \brief  Remove all temp breakpoints.
*/
/******************************************************************************/
void AptDebugger::RemoveAllTempBreakpoints()
{
    APT_THREAD_NAMESPACE::AutoFutex lock(gDebuggerMutex);
    for (int i = mTempBreakpointCount - 1; i >= 0; --i)
    {
        DeactivateBreakpoint(mTempBreakpoint[i]->mBreakpoint, true);
        mTempBreakpoint[i]->mIsValid = false;
        mTempBreakpoint[i]->mBreakpoint.Destroy();
        --mTempBreakpointCount;
    }
}

/******************************************************************************/
/*!
    AptDebugger::GetBreakpoint
    \brief  Return an instance of breakpoint which is specified by fileName
            and offset.

    \param  fileName        Indicate the file name which contains the breakpoint.
    \param  offset          Offset in byte-code stream where the breakpoint is.

    \return Return non-zero if breakpoint is found. Otherwise return NULL.
*/
/******************************************************************************/
IBreakpoint* AptDebugger::GetBreakpoint(const char* fileName, unsigned int offset)
{
    for ( int i = 0; i < mBreakPointsCount; i++ )
    {
        if ( AptFileNameCompare(mBreakpoint[i]->mBreakpoint.GetFileName(), fileName) && mBreakpoint[i]->mBreakpoint.GetOffset() == offset )
        {
            return &mBreakpoint[i]->mBreakpoint;
        }
    }
    return NULL;
}

/******************************************************************************/
/*!
    AptDebugger::GetBreakpointsNumber
    \brief  Return all instantiated breakpoints number

    \return Number of instantiated breakpoints.
*/
/******************************************************************************/
int AptDebugger::GetBreakpointsNumber() const
{
    return mBreakPointsCount;
}

/******************************************************************************/
/*!
    AptDebugger::GetBreakpoint
    \brief  Return an instance of breakpoint by index.

    \param  index       Index of the breakpoint information which will be return.

    \return Return non-zero if breakpoint is found. Otherwise return NULL.

    \note   index must be less than number of instantiated breakpoints number.
*/
/******************************************************************************/
IBreakpoint* AptDebugger::GetBreakpoint(int index) const
{
    if ( index >= 0 && index < mBreakPointsCount )
    {
        return &mBreakpoint[index]->mBreakpoint;
    }
    else
    {
        return NULL;
    }
}

/******************************************************************************/
/*!
    AptDebugger::GetBreakpointInfo
    \brief  Return a breakpoint's information which is specified by fileName
            and offset.

    \param  fileName        Indicate the file name which contains the breakpoint.
    \param  offset          Offset in byte-code stream where the breakpoint is.

    \return Return pointer to a instance of BreakpointInfo.
*/
/******************************************************************************/
AptDebugger::BreakpointInfo* AptDebugger::GetBreakpointInfo(const char* fileName, unsigned int offset)
{
    for ( int i = 0; i < mBreakPointsCount; i++ )
    {
        if ( AptFileNameCompare(mBreakpoint[i]->mBreakpoint.GetFileName(), fileName) && mBreakpoint[i]->mBreakpoint.GetOffset() == offset )
        {
            return mBreakpoint[i];
        }
    }
    return NULL;
}

/******************************************************************************/
/*!
    AptDebugger::GetTempBreakpointInfo
    \brief  Return a  temp breakpoint's information which is specified by fileName
            and offset.

    \param  fileName        Indicate the file name which contains the breakpoint.
    \param  offset          Offset in byte-code stream where the breakpoint is.

    \return Return pointer to a instance of BreakpointInfo.
*/
/******************************************************************************/
AptDebugger::BreakpointInfo* AptDebugger::GetTempBreakpointInfo(const char* fileName, unsigned int offset)
{
    for (int i = 0; i < mTempBreakpointCount; ++i)
    {
        if ( AptFileNameCompare(mTempBreakpoint[i]->mBreakpoint.GetFileName(), fileName) && mTempBreakpoint[i]->mBreakpoint.GetOffset() == offset )
        {
            return mTempBreakpoint[i];
        }
    }
    return NULL;
}

/******************************************************************************/
/*!
    AptDebugger::RaiseBreakpoint
    \brief  Breakpoint hit event handler, this function is called when Apt
            meet a breakpoint when executing.

    \param  instructionStream   Pointer to current instruction (breakpoint instruction).
                                The breakpoint instruction is the previous byte.
    \param  context             Runtime environment context

    \return Original instruction code before the breakpoint is placed.
*/
/******************************************************************************/
unsigned char AptDebugger::RaiseBreakpoint(unsigned char* instructionStream, AptCIH* context)
{
    APT_ASSERT(NULL != instructionStream && NULL != context);

    const char* fileName = GetFileNameByAddress(uintptr_t(instructionStream));
    const unsigned char* entry = reinterpret_cast<unsigned char*>(GetAptDataByFileName(fileName));

    instructionStream--;

    APT_ASSERT(uintptr_t(instructionStream) >= uintptr_t(entry));

    unsigned int offset = uintptr_t(instructionStream) - uintptr_t(entry);

    BreakpointInfo* hitBreakPoint = NULL;
    // Handle temp breakpoint first

    hitBreakPoint = GetTempBreakpointInfo(fileName, offset);


    // If breakpoint can't be found in temp breakpoint list, then search specific breakpoint list
    if (hitBreakPoint == NULL)
    {
        hitBreakPoint = GetBreakpointInfo(fileName, offset);
    }

    APT_ASSERT(NULL != hitBreakPoint);
    APT_ASSERT(hitBreakPoint->mIsValid && hitBreakPoint->mBreakpoint.IsActivated());

    unsigned char originalInstruction = hitBreakPoint->mBreakpoint.GetInstructionCode();
    hitBreakPoint->mBreakpoint.SetHitCount(hitBreakPoint->mBreakpoint.GetHitCount() + 1);
    mState = DEBUGGER_STATE_STOPPED;

    // TODO: Get the real call stack here.
    mCallStack.SetCurrentOffset(offset);
    mCallStack.SetCurrentFileName(fileName);

    char callStackString[AptDebugCallStack::MAX_CALL_STACK_CONTENT_LENGTH] = {0};
    mCallStack.GetCallStackAsString(callStackString, AptDebugCallStack::MAX_CALL_STACK_CONTENT_LENGTH);

    if ( NULL != mListener )
    {
        mListener->BreakpointHit(fileName, offset, callStackString);
    }
    APT_ASSERT(DEBUGGER_STATE_STOPPED != mState);
    return originalInstruction;
}

/******************************************************************************/
/*!
    AptDebugger::AnimationFileLoaded
    \brief  Event handler for file loaded. Place breakpoint if necessary.

    \param  file        Pointer to a AptFile instance which is just loaded
*/
/******************************************************************************/
void AptDebugger::AnimationFileLoaded(AptAnimationFile* file)
{
    for (int i = mBreakPointsCount - 1; i >= 0; --i)
    {
        APT_ASSERT(NULL != mBreakpoint[i]);
        if (AptFileNameCompare( mBreakpoint[i]->mBreakpoint.GetFileName(), file->GetName()))
        {
            ActivateBreakpoint(mBreakpoint[i]->mBreakpoint, false);
        }
    }
}

/******************************************************************************/
/*!
    AptDebugger::AnimationFileUnloaded
    \brief  Event handler for file unloaded. Remove breakpoint if necessary.

    \param  file        Pointer to a AptFile instance which is just unloaded
*/
/******************************************************************************/
void AptDebugger::AnimationFileUnloaded(AptAnimationFile* file)
{
    for (int i = mBreakPointsCount - 1; i >= 0; --i)
    {
        APT_ASSERT(NULL != mBreakpoint[i]);
        if (AptFileNameCompare( mBreakpoint[i]->mBreakpoint.GetFileName(), file->GetName()))
        {
            DeactivateBreakpoint(file, mBreakpoint[i]->mBreakpoint, false);
        }
    }

    for (int i = mTempBreakpointCount - 1; i >= 0; --i)
    {
        APT_ASSERT(NULL != mTempBreakpoint[i]);
        if (AptFileNameCompare( mBreakpoint[i]->mBreakpoint.GetFileName(), file->GetName()))
        {
            DeactivateBreakpoint(file, mTempBreakpoint[i]->mBreakpoint, true);
        }
    }
}

/******************************************************************************/
/*!
    AptDebugger::AddTempBreakpoint
    \brief  Create several temp breakpoints (breakpioints array).

    \param  fileName        Indicate the file name which contains the breakpoint.
    \param  offset          Offsets array in byte-code stream where the breakpoints are.
    \param  offsetCount     Offsets count
*/
/******************************************************************************/
void AptDebugger::AddTempBreakpoints(const char* fileName, unsigned int offset[], int offsetCount)
{
    APT_THREAD_NAMESPACE::AutoFutex lock(gDebuggerMutex);
    AptAnimationFile* file = AptAnimationFile::Cast(gpCurrentTargetSim->GetLoader()->findFile(fileName));
    if (NULL != file)
    {
        int tempBreakpointCountPre = mTempBreakpointCount;
        for (int i = 0; i < offsetCount; ++i)
        {
            if (offset[i] == INVALID_OFFSET)
            {
                // Offset INVALID_OFFSET(0xFFFFFFFF) is no sense, maybe codes has reached the end of function
                // or end of time-frame codes.
                continue;
            }
            // Since all temp breakpoint should have already been cleared.
            APT_ASSERT(GetTempBreakpointInfo(fileName, offset[i]) == NULL || GetTempBreakpointInfo(fileName, offset[i])->mIsValid == false)

            if ( mTempBreakpointCount < MAX_TEMP_BREAKPOINTS )
            {
                mTempBreakpoint[tempBreakpointCountPre + i]->mBreakpoint.Create(fileName, offset[i]);
                mTempBreakpoint[tempBreakpointCountPre + i]->mIsValid = true;
                ActivateBreakpoint(mTempBreakpoint[tempBreakpointCountPre + i]->mBreakpoint, true);
                ++mTempBreakpointCount;
            }
        }
    }
}

/******************************************************************************/
/*!
    AptDebugger::StepInto
    \brief  Execute instructions until research the end offset, if there is a
            function, step into it and stop execution

    \param  endOffset     Offset array
    \param  offsetCount   Offsets count
*/
/******************************************************************************/
void AptDebugger::StepInto(unsigned int endOffset[], int offsetCount)
{
    RemoveAllTempBreakpoints();
    if (offsetCount > 0)
    {
        AddTempBreakpoints(mCallStack.GetCurrentFileName(), endOffset, offsetCount);
        mTargetCallStackDepth = mCallStack.GetDepth();
    }
    mState = DEBUGGER_STATE_STEP_INTO;
}

/******************************************************************************/
/*!
    AptDebugger::StepOver
    \brief  Execute instructions until research the end offset, doesn't step
            into function.

    \param  endOffset    Offset array
    \param  offsetCount  Offsets count
*/
/******************************************************************************/
void AptDebugger::StepOver(unsigned int endOffset[], int offsetCount)
{
    RemoveAllTempBreakpoints();
    if (offsetCount > 0)
    {
        AddTempBreakpoints(mCallStack.GetCurrentFileName(), endOffset, offsetCount);
        mTargetCallStackDepth = mCallStack.GetDepth();
    }
    mState = DEBUGGER_STATE_STEP_OVER;
}

/******************************************************************************/
/*!
    AptDebugger::StepOut

    \brief  Execute instructions until current function finished.
*/
/******************************************************************************/
void AptDebugger::StepOut()
{
    RemoveAllTempBreakpoints();
    mTargetCallStackDepth = mCallStack.GetDepth() - 1;
    mState = DEBUGGER_STATE_STEP_OUT;
}

/******************************************************************************/
/*!
    AptDebugger::Continue

    \brief  Continue execution
*/
/******************************************************************************/
void AptDebugger::Continue()
{
    RemoveAllTempBreakpoints();
    mState = DEBUGGER_STATE_RUN;
}

/******************************************************************************/
/*!
    AptDebugger::GetCallStack

    \brief  Return the call stack instance.

    \return Reference to the call stack instance.
*/
/******************************************************************************/
ICallStack* AptDebugger::GetCallStack()
{
    return &mCallStack;
}

/******************************************************************************/
/*!
    AptDebugger::PushCallStack

    \brief  Push function to call stack

    \param  context         Apt runtime environment context.
    \param  functionName    Function name
    \param  charPtrAddress  Instruction address
    \param  thisObject      "this" object

    \return If the function is script function return true and push success else return false
*/
/******************************************************************************/
bool AptDebugger::PushCallStack(AptValue* context, const char* functionName,  const unsigned char* charPtrAddress, AptValue* thisObject)
{
    if (!context)
    {
        // We were not passed a function, so just abort
        return false;
    }

    unsigned int address = uintptr_t(charPtrAddress);
    if (context->isScriptFunction())
    {
        // It must be a scriptFunction, no native function will be pushed into stack.
        const char* hostFileName;
        unsigned int functionOffset;
        if (INVALID_OFFSET != address)
        {
            hostFileName = GetFileNameByAddress(address);
            unsigned int hostFileBaseStreamByte = GetAptDataByFileName(hostFileName);
            functionOffset = address - 1 - hostFileBaseStreamByte;
        }
        else
        {
            hostFileName = "Event/ Timer/ Component constructor";
            functionOffset = INVALID_OFFSET;
        }
        AptScriptFunctionBase* functionBase = context->c_scriptfunction();
        const char* functionFileName = GetFileNameByAddress(uintptr_t(functionBase->GetByteCodeBase()));
        unsigned int functionFileBaseStream = GetAptDataByFileName(functionFileName);
        unsigned int functionFirstOffset = uintptr_t(functionBase->GetByteCodeBase()) - functionFileBaseStream;
        mCallStack.Push(hostFileName, functionName, functionOffset, functionBase, thisObject);
        if (mState == DEBUGGER_STATE_STEP_INTO)
        {
            unsigned int offset[1] = {functionFirstOffset};
            AddTempBreakpoints(functionFileName, offset, 1);
        }
        return true;
    }
    else
    {
        if (!strcmp(functionName, "Frame time"))
        {
            mCallStack.Push("Frame time", "Frame time", INVALID_OFFSET, context, thisObject);
        }
    }
    return false;
}

/******************************************************************************/
/*!
    AptDebugger::SetTopFrameStack

    \brief  Set FrameStack into call stack

    \param  frameStack  frameStack need to be push

    \note   Since frameStack doesn't exist in every function for there is possible for
            a function which hasn't any local variable. Further more, when we call function
            AptDebugCallStack::Push the FrameStack is unavailable at that time, so these are
            why we have to add this function instead of put these codes into function AptDebugCallStack::Push.
*/
/******************************************************************************/
void AptDebugger::SetTopFrameStack(AptFrameStack* frameStack)
{
    mCallStack.SetTopFrameStack(frameStack);
}

/******************************************************************************/
/*!
    AptDebugger::PopCallStack

    \brief   Pop function from call stack, If current state is step out, step in or step over
             Should take add breakpoint when out of function into consideration.

    \param   context  Apt runtime environment context.
*/
/******************************************************************************/
void AptDebugger::PopCallStack(AptValue* context)
{
    if (!context)
    {
        // We were not passed a function, so just abort
        return;
    }
    if (context->isScriptFunction())
    {
        if ( mState == DEBUGGER_STATE_STEP_OUT  ||
             mState == DEBUGGER_STATE_STEP_INTO ||
             mState == DEBUGGER_STATE_STEP_OVER )
        {
            AptScriptFunctionBase* functionBase = context->c_scriptfunction();
            AptCharacterAnimationInst* characterInstance = GetRootCharacterInstance(functionBase->mpCIH);

            AptNativeString fileName = mCallStack.GetTop()->GetFileName();
            unsigned int fileBaseStreamByte;
            if ( !strcmp(fileName.c_str(), "Frame time") || !strcmp(fileName.c_str(), "Event/ Timer/ Component constructor"))
            {
                fileBaseStreamByte = uintptr_t(AptAnimationFile::Cast(characterInstance->mpFile)->GetAptData());
            }
            else
            {
                fileBaseStreamByte = GetAptDataByFileName(fileName.c_str());
            }
            unsigned int functionOffset = mCallStack.GetTop()->GetEntranceOffset();
            if (functionOffset != INVALID_OFFSET)
            {
                unsigned int targetBreakpointOffset = functionOffset;
                functionOffset += fileBaseStreamByte;
                unsigned char functionType;

                // Here means there is no breakpoint in original position.
                functionType = GetInstructionType(functionOffset);

                if (functionType == AptActionBreakpoint)
                {
                    // Judge whether the function itself has been set a breakpoint.
                    for (int i = 0; i < mBreakPointsCount; ++i)
                    {
                        if (AptFileNameCompare(mBreakpoint[i]->mBreakpoint.GetFileName(), fileName)
                            && mBreakpoint[i]->mBreakpoint.GetOffset() == targetBreakpointOffset
                            && mBreakpoint[i]->mIsValid)
                        {
                            functionType = mBreakpoint[i]->mBreakpoint.GetInstructionCode();
                            break;
                        }
                    }

                    for (int i = 0; i < mTempBreakpointCount; ++i)
                    {
                        if (AptFileNameCompare(mTempBreakpoint[i]->mBreakpoint.GetFileName(), fileName)
                            && mTempBreakpoint[i]->mBreakpoint.GetOffset() == targetBreakpointOffset
                            && mTempBreakpoint[i]->mIsValid)
                        {
                            functionType = mTempBreakpoint[i]->mBreakpoint.GetInstructionCode();
                            break;
                        }
                    }
                }

                APT_ASSERT(functionType <= LastAptAction);
                // Now we get what the function type it is.

                targetBreakpointOffset = ComputeNextInstructionOffsetForFunction(functionType, targetBreakpointOffset);


                unsigned int offsetArray[1] = {targetBreakpointOffset};
                switch(mState)
                {
                case DEBUGGER_STATE_STEP_OVER:
                    if (mCallStack.GetDepth() - 1 < mTargetCallStackDepth)
                    {
                        RemoveAllTempBreakpoints();
                        AddTempBreakpoints(fileName, offsetArray, 1);
                    }
                    break;
                case DEBUGGER_STATE_STEP_OUT:
                    if (mCallStack.GetDepth() - 1 <= mTargetCallStackDepth)
                    {
                        RemoveAllTempBreakpoints();
                        AddTempBreakpoints(fileName, offsetArray, 1);
                    }
                    break;
                case DEBUGGER_STATE_STEP_INTO:
                    if (mCallStack.GetDepth() - 1 < mTargetCallStackDepth)
                    {
                        RemoveAllTempBreakpoints();
                        AddTempBreakpoints(fileName, offsetArray, 1);
                    }
                    break;
                default:
                    // It can't be here.
                    APT_ASSERT(false);
                }
            }
        }
    }
    mCallStack.Pop();
    if (0 == mCallStack.GetDepth())
    {
        // Here means current frame has end, there is no sense to step in/step out/step over.
        mState = DEBUGGER_STATE_RUN;
    }
}

/******************************************************************************/
/*!
    AptDebugger::GetTargetBreakpointDepth

    \brief   Return target call stack depth for step over.
    \return  Target temp breakpoint depth
*/
/******************************************************************************/
int AptDebugger::GetTargetBreakpointDepth()
{
    return mTargetCallStackDepth;
}

/******************************************************************************/
/*!
    AptDebugger::ComputeNextInstructionOffsetForFunction

    \brief   Compute next instruction offset by function type and current function offset.
             Here you can find this function just add one or two offset on previous offset.
             Just in order to calculate which offset is the end of statement of calling a function,
             and return the result.
    \param   functionType  Function type.
    \param   offset        Offset where call the function

    \return  Next first instruction offset when out of function.
*/
/******************************************************************************/
unsigned int AptDebugger::ComputeNextInstructionOffsetForFunction(unsigned char functionType, unsigned int offset)
{
    unsigned int targetBreakpointOffset = offset;
    switch(functionType)
    {
    case AptActionCallMethod:
        targetBreakpointOffset++;
        break;
    case AptActionDictCallFuncPop:
        targetBreakpointOffset += 2;
        break;
    case AptActionDictCallFuncSetVar:
        targetBreakpointOffset += 2;
        break;
    case AptActionDictCallMethodPop:
        targetBreakpointOffset += 2;
        break;
    case AptActionDictCallMethodSetVar:
        targetBreakpointOffset += 2;
        break;
    case AptActionCallFuncAndPop:
        targetBreakpointOffset++;
        break;
    case AptActionCallFuncSetVar:
        targetBreakpointOffset++;
        break;
    case AptActionCallMethodPop:
        targetBreakpointOffset++;
        break;
    case AptActionCallMethodSetVar:
        targetBreakpointOffset++;
        break;
    case AptActionCallFunction:
        targetBreakpointOffset++;
        break;
    case AptActionNewObject:
        targetBreakpointOffset++;
        break;
    case AptActionNewMethod:
        targetBreakpointOffset++;
        break;
    case AptActionInitArray:
        targetBreakpointOffset++;
        break;
    case AptActionInitObject:
        targetBreakpointOffset++;
        break;
    default:
        // It can't be here.
        APT_ASSERT(false);
    }
    return targetBreakpointOffset;
}

/******************************************************************************/
/*!
    AptDebugger::GetInstructionType

    \brief   Get instruction type by offset passed as parameter.
    \param   Instruction offset in assembly codes.
    \return  Action type code.
*/
/******************************************************************************/
unsigned char AptDebugger::GetInstructionType(unsigned int offset)
{
    return *reinterpret_cast<unsigned char*>(offset);
}

/******************************************************************************/
/*!
    AptDebugger::IsStopped

    \brief   To know whether AS is under stop state.
    \return  If is stop-state return true else return false.
*/
/******************************************************************************/
bool AptDebugger::IsStopped()
{
    if (mState == DEBUGGER_STATE_STOPPED)
    {
        return true;
    }
    else
    {
        return false;
    }
}

/******************************************************************************/
/*!
    AptDebugger::GetRootCharacterInstance

    \brief   Get root character animation instance which used to get file name
             and other useful information.
    \param   context  Runtime environment context.

    \return  Root character animation instance.
*/
/******************************************************************************/
AptCharacterAnimationInst* AptDebugger::GetRootCharacterInstance(AptCIH* context)
{
    while(NULL != context && !context->GetCharacterInst()->IsAnimationInst())
    {
        context = context->GetDisplayListParent();
    }
    APT_ASSERT(NULL != context);

    return static_cast<AptCharacterAnimationInst*> (context->GetCharacterInst()) ;
}

/******************************************************************************/
/*!
    AptDebugger::GetFileNameByAddress

    \brief   Get APT file name by instruction address.

    \param   address  Instruction address.

    \return  File name.
*/
/******************************************************************************/
const char* AptDebugger::GetFileNameByAddress(unsigned int address)
{
    AptFilePtr loadedFile[MAX_LOADED_FILE_COUNT];

    gpCurrentTargetSim->GetLoader()->GetFileVector(loadedFile, MAX_LOADED_FILE_COUNT);
    AptAnimationFile* targetFile = NULL;
    for (int i = 0; i < MAX_LOADED_FILE_COUNT && NULL != loadedFile[i]; ++i)
    {
        AptFilePtr file = loadedFile[i];
        if (file->GetFileType() == AptFileType_Animation)
        {
            AptAnimationFile* animFile = AptAnimationFile::Cast(file);
            unsigned int loadedFileStream = uintptr_t(animFile->GetAptData());
            if( address >= loadedFileStream )
            {
                if( !targetFile || loadedFileStream > uintptr_t(targetFile->GetAptData()) )
                {
                    targetFile = animFile;
                }
            }
        }
    }
    APT_ASSERT(NULL != targetFile);
    return targetFile->GetName().c_str();
}

/******************************************************************************/
/*!
    AptDebugger::GetAptDataByFileName

    \brief   Get Apt data by file name.

    \param   fileName  Apt file name.

    \return  Apt data
*/
/******************************************************************************/
unsigned int AptDebugger::GetAptDataByFileName(const char* fileName)
{
    AptAnimationFile* file = AptAnimationFile::Cast(gpCurrentTargetSim->GetLoader()->findFile(fileName));
    APT_ASSERT(NULL != file);
    return uintptr_t(file->GetAptData());
}
#else

/******************************************************************************/
/*!
    AptGetDebugger
    \brief  Return pointer to the instance of IDebugger, flag APT_DEBUGGER_ENABLE off

    \return It's non-sense to use this function when flag APT_DEBUGGER_ENABLE off, so always return NULL
*/
/******************************************************************************/
IDebugger* AptGetDebugger()
{
    return NULL;
}

#endif
