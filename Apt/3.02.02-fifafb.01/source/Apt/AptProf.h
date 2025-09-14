/*H************************************************************************************************/
/*!

    \File   AptProf.h

    \Description
            Contains APT_PROFILE macros and AptProfileSample class declaration.

    \Notes
            The Apt runtime code can be timed using your favorite profiler if you
            do the following:
            - Define APT_DO_PROFILE via the compiler command-line.
            - Implement the pfnProfilerInit, pfnProfilerShutdown callbacks to
              setup/teardown your profiler.
            - Implement the pfnProfilerStartSample callback to begin timing.  This
              callback is invoked at the beginning of any Apt runtime function
              containing "APT_PROFILE" (which is used by the vast majority of the code in Apt.)
            - Implement the pfnProfilerStopSample callback to stop timing and store
              the sample somehow.  This callback is invoked whenever a function
              using APT_PROFILE returns.

            This allows you to implement customized profile-reporting behavior
            programmatically, as an alternative to using the profiling abilities of your compiler.

    \Copyright
            (c) 2007 Electronic Arts Inc.

    \Version    1.0        08/01/05 (LEO) First Version
    \Version    1.01       03/12/07 (RB)  Removed APT_PROFILE_PrintProfile, APT_PROFILE_Purge, etc.


*/
/************************************************************************************************H*/


#ifndef _AptProf_h
#define _AptProf_h

/*** Include files ********************************************************************************/

#if defined(APT_DO_PROFILE)
#include "AptDefine.h"
#endif

/*** Defines **************************************************************************************/

void APT_PROFILE_Init(void);

void APT_PROFILE_Shutdown(void);


#if defined(APT_DO_PROFILE)
#define APT_PROFILE(name)           AptProfileSample __profile(#name)
#else
#define APT_PROFILE(name)

#endif  // APT_DO_PROFILE


#define APT_PROFILE_TEST_ENABLE             1
#define APT_PROFILE_INTERPRETER_ENABLED     0
#define APT_PROFILE_RENDER_ENABLED          1
#define APT_PROFILE_INPUT_ENABLED           1
#define APT_PROFILE_LOOKUP_ENABLED          1
#define APT_PROFILE_ACTIONSCRIPT_ENABLED    1
#define APT_PROFILE_AUXILIARY_ENABLED       1

#if APT_PROFILE_TEST_ENABLE
#define APT_PROFILE_TEST(_name)      APT_PROFILE(_name)
#else
#define APT_PROFILE_TEST(_name)
#endif

#if APT_PROFILE_INTERPRETER_ENABLED
#define APT_PROFILE_INTERPRETER(_name)      APT_PROFILE(_name)
#else
#define APT_PROFILE_INTERPRETER(_name)
#endif

#if APT_PROFILE_RENDER_ENABLED
#define APT_PROFILE_RENDER(_name)           APT_PROFILE(_name)
#else
#define APT_PROFILE_RENDER(_name)
#endif

#if APT_PROFILE_INPUT_ENABLED
#define APT_PROFILE_INPUT(_name)            APT_PROFILE(_name)
#else
#define APT_PROFILE_INPUT(_name)
#endif

#if APT_PROFILE_LOOKUP_ENABLED
#define APT_PROFILE_LOOKUP(_name)           APT_PROFILE(_name)
#else
#define APT_PROFILE_LOOKUP(_name)
#endif

#if APT_PROFILE_ACTIONSCRIPT_ENABLED
#define APT_PROFILE_ACTIONSCRIPT(_name)         APT_PROFILE(_name)
#else
#define APT_PROFILE_ACTIONSCRIPT(_name)
#endif

#if APT_PROFILE_LOOKUP_ENABLED || APT_PROFILE_INTERPRETER_ENABLED
#define APT_PROFILE_INTERPRETER_LOOKUP(_name)           APT_PROFILE(_name)
#else
#define APT_PROFILE_INTERPRETER_LOOKUP(_name)
#endif

#if APT_PROFILE_LOOKUP_ENABLED || APT_PROFILE_INTERPRETER_ENABLED
#define APT_PROFILE_INPUT_LOOKUP(_name)     APT_PROFILE(_name)
#else
#define APT_PROFILE_INPUT_LOOKUP(_name)
#endif

#if APT_PROFILE_AUXILIARY_ENABLED
#define APT_PROFILE_AUXILIARY(_name)      APT_PROFILE(_name)
#else
#define APT_PROFILE_AUXILIARY(_name)
#endif

/*** Macros ***************************************************************************************/


/*** Type Definitions *****************************************************************************/
class   AptProfileSample
{
public:
    AptProfileSample( const char* name ) ;
    ~AptProfileSample( )  ;

    APT_NEW_DELETE_OPERATORS
    APT_DECLARE_OBJECT_METRICS
};

/*** Variables ************************************************************************************/

/*** Functions ************************************************************************************/

#endif // _AptProf_h
