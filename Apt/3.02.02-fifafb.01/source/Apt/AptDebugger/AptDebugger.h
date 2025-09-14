/******************************************************************************/
/*!
    Copyright 2010 Electronic Arts Inc.

    \file       AptDebugger.h

    \brief      Apt debugger implementation
*/
/******************************************************************************/



#ifndef _AptDebugger_h
#define _AptDebugger_h

/*** Include files ********************************************************************************/
#include "AptDebugger/IDebugger.h"
#include "AptDebugger/AptBreakpoint.h"
#include "AptDebugger/AptDebugCallStack.h"

/*** Defines **************************************************************************************/

/*** Macros ***************************************************************************************/

/*** Type Definitions *****************************************************************************/
class IDebuggerListener;

template <typename T> class AptSharedPtr;
struct AptAnimationFile;

/******************************************************************************/
/*!
    \brief  Apt script debugger

    Provide breakpoint, execution control interfaces to allow user debug action
    script at runtime.
*/
/******************************************************************************/
class AptDebugger : public IDebugger
{
public:
    APT_NEW_DELETE_OPERATORS
	APT_DECLARE_OBJECT_METRICS

    // Debugger initialization
    static bool                 Initialize();
    // Shutdown debugger
    static bool                 Shutdown();
    // Singleton instance function
    static AptDebugger*         GetInstance();
    // Get instruction type by offset passed as parameter
    static unsigned char        GetInstructionType(unsigned int offset);
    // Register a debugger event listener
    void                        RegisterListener(IDebuggerListener* listener);
    // Unregister debugger event listener
    void                        UnregisterListener();
    // Add a breakpoint in debugger
    void                        AddBreakpoint(const char* fileName, unsigned int offset, char* timeStamp);
    // Remove a specified breakpoint. (Deactivate)
    void                        RemoveBreakpoint(const char* fileName, unsigned int offset);
    // Remove all breakpoints (Deactivate all)
    void                        RemoveAllBreakpoints();
    // Deactivate the breakpoint, deactivate means it's not working until the script code is loaded.
    void                        DeactivateBreakpoint(AptBreakpoint& breakpoint, bool isTempBreakpoint);
    // Activate the breakpoint
    void                        ActivateBreakpoint(AptBreakpoint& breakpoint, bool isTempBreakpoint);
    // Trace a message
    void                        Trace(const char* message);
    // Return breakpoint's information by file name and address
    IBreakpoint*                GetBreakpoint(const char* fileName, unsigned int offset);
    // Return activated breakpoint number.
    int                         GetBreakpointsNumber() const;
    // Return breakpoint's information by index
    IBreakpoint*                GetBreakpoint(int index) const;
    // Step into function
    void                        StepInto(unsigned int endOffset[], int offsetCount);

    // Step over function
    void                        StepOver(unsigned int endOffset[], int offsetCount);

    // Step out function
    void                        StepOut();

    // Continue execution
    void                        Continue();

    // Return call stack instance
    ICallStack*                 GetCallStack();

    // Get APT file name by instruction address.
    const char*                 GetFileNameByAddress(unsigned int address);

    // Get APT data by file name.
    unsigned int                GetAptDataByFileName(const char* fileName);


    /////////////////////////////////////////////////
    // Internal functions
    /////////////////////////////////////////////////

    // Breakpoint handler function
    unsigned char               RaiseBreakpoint(unsigned char* instructionStream, AptCIH* context);

    // Event handler for file loaded.
    void                        AnimationFileLoaded(AptAnimationFile* file);

    // Event handler for file unloaded.
    void                        AnimationFileUnloaded(AptAnimationFile* file);

    // Push call stack. If it is a script action return true else return false
    bool                        PushCallStack(AptValue* pCur, const char* functionName, const unsigned char* functionOffset, AptValue* thisObject);

    // Pop call stack.
    void                        PopCallStack(AptValue* pCur);

    // Set target FrameStack into call stack.
    void                        SetTopFrameStack(AptFrameStack* frameStack);

    // Get time stamp of specific file name.
    bool                        GetTimeStamp(const char* fileName, char* buf);

    // Get target temp breakpoint call stack depth.
    int                         GetTargetBreakpointDepth();

    // Get root character animation instance which used to get file name and other useful information.
    AptCharacterAnimationInst*  GetRootCharacterInstance(AptCIH* context);

    // To know if AS is under stop state.
    bool                        IsStopped();

    static const unsigned int INVALID_OFFSET = 0xFFFFFFFF;

private:
    struct BreakpointInfo
    {
                                BreakpointInfo() : mIsValid(false) {}
        AptBreakpoint           mBreakpoint;
        bool                    mIsValid;
    };


    enum DebuggerState
    {
        DEBUGGER_STATE_RUN,                                         // Run script until hit another breakpoint
        DEBUGGER_STATE_STEP_INTO,                                   // Execute only one instruction, step in if next instruction is function
        DEBUGGER_STATE_STEP_OVER,                                   // Execute one instruction, step over if next instruction is function
        DEBUGGER_STATE_STEP_OUT,                                    // Execute script until current function returns
        DEBUGGER_STATE_STOPPED,                                     // Stopped
    };

    enum Config
    {
        MAX_BREAKPOINTS      = 16,
        MAX_TEMP_BREAKPOINTS = 16,
        MAX_LOADED_FILE_COUNT = 256
    };

                                AptDebugger();                      //!< Constructor
                                ~AptDebugger();                     //!< Destructor

                                AptDebugger(const AptDebugger& debugger);
    AptDebugger&                operator=(const AptDebugger& debugger);

    void                        CreateTempBreakpoint(
                                    const char* fileName,
                                    const unsigned char* entry,
                                    unsigned char* instructionStream
                                );                                  //!< Create a temp breakpoint
    BreakpointInfo*             GetBreakpointInfo(const char* fileName, unsigned int offset);
    BreakpointInfo*             GetTempBreakpointInfo(const char* fileName, unsigned int offset);

    // Add temp breakpoints in debugger
    void                        AddTempBreakpoints(const char* fileName, unsigned int offset[], int offsetCount);
    // Remove all temp breakpoints
    void                        RemoveAllTempBreakpoints();
    // Get next instruction offset by function type and the offset where invoke the function.
    unsigned int                ComputeNextInstructionOffsetForFunction(unsigned char functionType, unsigned int offset);

    // Deactivate the breakpoint, only used internally
    void                        DeactivateBreakpoint(AptAnimationFile* file, AptBreakpoint& breakpoint, bool isTempBreakpoint);



    static AptDebugger*         sInstance;                          //!< Singleton instance

    // We have limitation for the number of breakpoint now. It's MAX_BREAKPOINTS_NUM.
    // All of the breakpoints are allocated as one block memory while AptDebugger is
    // initialized. There is a flag to see if the breakpoint is used or not.
    // mBreakpoint is the breakpoint list what we used.
    // mTempBreakpoint is for the temp breakpoint.
    // mBreakpointBuffer is the real breakpoint instance array. mBreakpoint and mTempBreakpoint
    // are all point to element of this array.
    BreakpointInfo*             mBreakpoint[MAX_BREAKPOINTS]; //!< Breakpoint array
    BreakpointInfo*             mTempBreakpoint[MAX_TEMP_BREAKPOINTS]; //!< Temp breakpoint, used internally
    BreakpointInfo              mBreakpointBuffer[MAX_BREAKPOINTS + MAX_TEMP_BREAKPOINTS]; //!< The real breakpoint instance list

    int                         mBreakPointsCount;                 //!< Instantiated breakpoints number
    int                         mTempBreakpointCount;              //!< Instantiated temp breakpoints number
    int                         mTargetCallStackDepth;              //!< Temp breakpoint call stack depth which is used by resurisive situration

    IDebuggerListener*          mListener;                          //!< Debugger event listener

    DebuggerState               mState;                             //!< Debugger state

    AptDebugCallStack           mCallStack;                         //!< Call stack instance

};


/*** Variables ************************************************************************************/

/*** Functions ************************************************************************************/

#endif // _AptDebugger_h
