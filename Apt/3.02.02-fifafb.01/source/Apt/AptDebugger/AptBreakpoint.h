/******************************************************************************/
/*!
    Copyright 2010 Electronic Arts Inc.

    \file       AptBreakpoint.h

    \brief      Breakpoint information wrapper class
*/
/******************************************************************************/



#ifndef _AptBreakpoint_h
#define _AptBreakpoint_h

/*** Include files ********************************************************************************/
#include "AptDebugger/IBreakpoint.h"

/*** Defines **************************************************************************************/

/*** Macros ***************************************************************************************/

/*** Type Definitions *****************************************************************************/
template <typename T> class AptSharedPtr;
struct AptFile;
typedef AptSharedPtr< AptFile > AptFilePtr;

/******************************************************************************/
/*!
    \brief  Saved breakpoint information
*/
/******************************************************************************/
class AptBreakpoint : public IBreakpoint
{
public:
    APT_NEW_DELETE_OPERATORS
	APT_DECLARE_OBJECT_METRICS
    // Constructor
                                AptBreakpoint();
    // Destructor
                                ~AptBreakpoint();

    // Returns file name where the breakpoint is placed
    inline const char*          GetFileName() const;

    // Returns the offset in bytecode stream where is breakpoint is placed.
    inline unsigned int         GetOffset() const;

    // Destroy the breakpoint, internal state will be reset
    void                        Destroy();

    // Create a new breakpoint
    bool                        Create(const char* fileName, unsigned int offset, char* timeStamp = NULL);

    // Returns true if this is a valid breakpoint
    inline bool                 IsActivated() const;

    // Set isActivate of the breakpoint
    inline void                 SetActivated(bool activateState);

    // Returns hit count of the breakpoint
    inline unsigned int         GetHitCount();

    // Set hit count of the breakpoint
    inline bool                 SetHitCount(unsigned int count);

    // Increase hit count of the breakpoint by 1
    inline bool                 IncreaseHitCount();

    // Returns the original instruction code of the breakpoint where it is placed
    inline unsigned char        GetInstructionCode() const;

    // Set instruction code of the breakpoint
    inline void                 SetInstructionCode(const unsigned char instructionCode);

    // Get time stamp of host APT file.
    inline char*                GetTimeStamp();

private:
    enum Config
    {
        MAX_FILE_NAME_LENGTH    = 256,
        TIME_STAMP_SIZE         = 16
    };

                                AptBreakpoint(const AptBreakpoint& instance);
    AptBreakpoint&              operator=(const AptBreakpoint& instance);

    char                        mFileName[MAX_FILE_NAME_LENGTH];    //!< File name where the breakpoint is placed
    unsigned int                mOffset;                            //!< Offset of the breakpoint
    char                        mTimeStamp[TIME_STAMP_SIZE];        //!< Time stamp of host APT file
    bool                        mIsActivated;                       //!< Flag for activated/deactivated state
    unsigned int                mHitCount;                          //!< Hit count of the breakpoint
    unsigned char               mInstructionCode;                   //!< Original instruction code
};

/******************************************************************************/
/*!
    AptBreakpoint::GetFileName
    \brief  Returns file name where the breakpoint is placed

    \return File name for this breakpoint if there is no error. Otherwise
            return a empty string.
*/
/******************************************************************************/
inline const char* AptBreakpoint::GetFileName() const
{
    return mFileName;
}

/******************************************************************************/
/*!
    AptBreakpoint::GetOffset
    \brief  Returns the offset in bytecode stream where is breakpoint is placed.

    \return Offset of the breakpoint in bytecode stream.
*/
/******************************************************************************/
inline unsigned int AptBreakpoint::GetOffset() const
{
    return mOffset;
}

/******************************************************************************/
/*!
    AptBreakpoint::IsActivated()
    \brief  Returns activated/deactivated state of the breakpoint.
            Activated means the breakpoint is working now.
            Deactivated means it's not working until the script code is loaded.

    \return Returns true if the breakpoint is activated.
*/
/******************************************************************************/
inline bool AptBreakpoint::IsActivated() const
{
    return mIsActivated;
}

/******************************************************************************/
/*!
    AptBreakpoint::SetActivated()
    \brief  Set activated/deactivated state of the breakpoint.
            Activated means the breakpoint is working now.
            Deactivated means it's not working until the script code is loaded.
*/
/******************************************************************************/
inline void AptBreakpoint::SetActivated(bool activateState)
{
    mIsActivated = activateState;
}

/******************************************************************************/
/*!
    AptBreakpoint::GetHitCount()
    \brief  Returns hit count of the breakpoint

    \return Returns hit count of the breakpoint
*/
/******************************************************************************/
inline unsigned int AptBreakpoint::GetHitCount()
{
    return mHitCount;
}

/******************************************************************************/
/*!
    AptBreakpoint::SetHitCount()
    \brief  Set hit count of the breakpoint

    \param  count       New value for the hit count.

    \return Returns true if success. Otherwise return false.
*/
/******************************************************************************/
inline bool AptBreakpoint::SetHitCount(unsigned int count)
{
    if ( !mIsActivated )
    {
        return false;
    }

    mHitCount = count;

    return true;
}

/******************************************************************************/
/*!
    AptBreakpoint::IncreaseHitCount()
    \brief  Increase hit count of the breakpoint by 1

    \return Returns true if success. Otherwise return false.
*/
/******************************************************************************/
inline bool AptBreakpoint::IncreaseHitCount()
{
    if ( !mIsActivated )
    {
        return false;
    }

    mHitCount++;

    return true;
}

/******************************************************************************/
/*!
    AptBreakpoint::GetInstructionCode()
    \brief  Returns the original instruction code of the breakpoint
            where it is placed

    \return Returns an Actions enumeration value.
*/
/******************************************************************************/
inline uint8_t AptBreakpoint::GetInstructionCode() const
{
    return mInstructionCode;
}

/******************************************************************************/
/*!
    AptBreakpoint::SetInstructionCode()
    \brief  Set the original instruction code of the breakpoint
            where it is placed
*/
/******************************************************************************/
inline void AptBreakpoint::SetInstructionCode(const unsigned char instructionCode)
{
    mInstructionCode = instructionCode;
}

/******************************************************************************/
/*!
    AptBreakpoint::GetTimeStamp
    \brief  Get time stamp of host APT file.

    \return  Return time stamp of host APT file.
*/
/******************************************************************************/
inline char* AptBreakpoint::GetTimeStamp()
{
    return mTimeStamp;
}
/*** Variables ************************************************************************************/

/*** Functions ************************************************************************************/

#endif // _AptBreakpoint_h
