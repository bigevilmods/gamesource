/*H************************************************************************************************/
/*!

    \File    AptProf.cpp

    \Description
        Contains AptProfileSample class definition, along with
        APT_PROFILE_Init and APT_PROFILE_Shutdown functions.

    \Notes
        The Apt profile callbacks allow you to implement customized profile-reporting behavior
        programmatically, as an alternative to using the profiling abilities of your compiler.
        Please see AptProf.h for details.

    \Copyright
        (c) 2007 Electronic Arts Inc.

    \Version    1.0        08/01/05 (LEO) First Version
    \Version    1.01       03/12/07 (RB)  Removed APT_PROFILE_PrintProfile, APT_PROFILE_Purge, etc.
                                          Removed dependency on Timer package.
                                          Invoke new callbacks pfnProfilerInit, pfnProfilerStartSample, etc.
                                          Added dummy callbacks in case user does not provide any.
    \Version    1.1        01/31/08 (RB)  Added missing function prototypes.

*/
/************************************************************************************************H*/
#if defined(APT_DO_PROFILE)

/*** Include files ********************************************************************************/
#include "EABase/EABase.h"

#include "AptProf.h"
#include "Apt.h"


/*** Defines **************************************************************************************/

/*** Macros ***************************************************************************************/

/*** Type Definitions *****************************************************************************/

/*** Function Prototypes **************************************************************************/

/*** Variables ************************************************************************************/

// Private variables

// Public variables


/*** Private Functions ****************************************************************************/


// These are empty implementations of profile sample start/stop callbacks

static void APT_ProfilerStartSample_Null( const char * ) {}
static void APT_ProfilerStopSample_Null() {}


/*** Public Functions *****************************************************************************/

/*F************************************************************************************************/
/*!
    \Function    AptProfileSample::AptProfileSample

    \Description
        Constructor for the profiler.  This starts the profile for the scope this object is created in.

    Insert your long description here.

    \Input      name              -

    \Output     A                 -

    \Notes
        None.

    \Version    1.0        08/01/05 (LEO) First Version

*/
/************************************************************************************************F*/
AptProfileSample::AptProfileSample( const char* name )
{
    gAptFuncs.pfnProfilerStartSample( name );
}

/*F************************************************************************************************/
/*!
    \Function    AptProfileSample::~AptProfileSample

    \Description
        Deconstructor for the profiler.  This ends the profile for the scope this object was created in.

    \Input
        None.

    \Output     A                 -

    \Notes
        None.

    \Version    1.0        08/01/05 (LEO) First Version

*/
/************************************************************************************************F*/
AptProfileSample::~AptProfileSample( )
{
    gAptFuncs.pfnProfilerStopSample();
}
/*
//! Simple helper functions to write out spaces and xml end tags
inline void PrintSpace(int32_t nSpace)
{
    for(int32_t i = 0; i < nSpace; i++)
    {
        pfnProfilePrint(" ");
    }
}

inline void PrintCloseTags(int32_t nCurrentDepth, int32_t nNewDepth)
{
    for(int32_t i = nCurrentDepth; i > nNewDepth; i--)
    {
        PrintSpace(i);
        pfnProfilePrint("</Item>\n");
    }
}
*/
/*F************************************************************************************************/
/*!
    \Function    DisplayProfileData

    \Description
        Displays the profile log to the tty

    \Input      iter              -

    \Output
        None.

    \Notes
        None.

    \Version    1.0        08/01/05 (LEO) First Version

*/
/************************************************************************************************F*/
//void OutPutProfileData(EA::Profile::ProfileIterator* iter)
//{
//    int32_t nDepth = iter->Get_Current_Depth();// - 1;
//
//    if(nDepth == 0)
//    {
//        pfnProfilePrint(
//            "<Metric Name=\"Apt\" InclusiveValue=\"%f\" Units=\"MilliSeconds\" >\n",
//            1000.0*((double_t)iter->Get_Current_Total_Time()/(double_t)EA::Timer::get_cycles_per_second())
//        );
//        nProfileLogCounter = nDepth;
//        return;
//    }
//
//    if(nDepth <= nProfileLogCounter)
//    {
//        pfnProfilePrint(" />\n");
//        if(nDepth < nProfileLogCounter - 1)
//        {
//            PrintCloseTags(nProfileLogCounter - 1, nDepth);
//        }
//        PrintSpace(nDepth);
//        if(nDepth < nProfileLogCounter)
//        {
//            pfnProfilePrint("</Item>\n");
//            PrintSpace(nDepth);
//        }
//    }
//    else if(nProfileLogCounter == 0)
//    {
//        PrintSpace(nDepth);
//    }
//    else if(nDepth > nProfileLogCounter)
//    {
//        pfnProfilePrint(" >\n");
//        PrintSpace(nDepth);
//    }
//
//    nProfileLogCounter = nDepth;
//    pfnProfilePrint
//        (
//        "<Item Calls=\"%i\" Cycles=\"%lu\" TimeMS=\"%f\" AvgTimeMS=\"%f\" Depth=\"%i\" Function=\"%s\"",
//        iter->Get_Current_Total_Calls(),
//        (long unsigned int)iter->Get_Current_Total_Time(),
//        1000.0*((double_t)iter->Get_Current_Total_Time()/(double_t)EA::Timer::get_cycles_per_second()),
//        (1000.0*((double_t)iter->Get_Current_Total_Time()/(double_t)EA::Timer::get_cycles_per_second()))/(double_t)iter->Get_Current_Total_Calls(),
//        nDepth,
//        iter->Get_Current_Name()
//        );
//}
#else

// Must have prototypes for these
void APT_PROFILE_Init(void);
void APT_PROFILE_Shutdown(void);

#endif  //#if defined(APT_DO_PROFILE)
/*F************************************************************************************************/
/*!
    \Function    APT_PROFILE_init

    \Description
        Resets profiler and resets the global Apt profile

    \Input
        None.

    \Output
        None.

    \Notes
        None.

    \Version    1.0        08/01/05 (LEO) First Version

*/
/************************************************************************************************F*/
void APT_PROFILE_Init(void)
{
#if defined(APT_DO_PROFILE)
    if ( gAptFuncs.pfnProfilerInit )
    {
        gAptFuncs.pfnProfilerInit();
    }
    if ( gAptFuncs.pfnProfilerStartSample == NULL || gAptFuncs.pfnProfilerStopSample == NULL )
    {
        // disable profile sampling because the user did not provide a callback
        gAptFuncs.pfnProfilerStartSample = APT_ProfilerStartSample_Null;
        gAptFuncs.pfnProfilerStopSample = APT_ProfilerStopSample_Null;
    }
#endif // APT_DO_PROFILE
}

/*F************************************************************************************************/
/*!
    \Function    APT_PROFILE_shutdown

    \Description
        Deletes the global apt profile

    \Input
        None.

    \Output
        None.

    \Notes
        None.

    \Version    1.0        08/02/05 (LEO) First Version

*/
/************************************************************************************************F*/
void APT_PROFILE_Shutdown(void)
{
#if defined(APT_DO_PROFILE)
    if ( gAptFuncs.pfnProfilerShutdown )
    {
        gAptFuncs.pfnProfilerShutdown();
    }
#endif // APT_DO_PROFILE
}

