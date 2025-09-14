/*H************************************************************************************************/
/*!

    \File
        AptPubDebugCallStack.h

    \Description
        Exposes the debug call stack for tracking down Apt issues.

    \Notes
        Class is always defined but implementation will only return in
        case of EA_DEBUG

    \Copyright
        (c) 2007 Electronic Arts Inc.

    \Version
        1.0        02/13/07 (rstradling) First Version
*/
/************************************************************************************************H*/

#ifndef __APTPUBDEBUGCALLSTACK_H__
#define __APTPUBDEBUGCALLSTACK_H__
#include "eabase/eabase.h"


namespace EA
{
    namespace Apt
    {
        namespace Debug
        {
            class AptPubDebugCallStackC
            {
            public:
                static uint32_t _GetNumItems();
                static const char* _GetFunctionName(uint32_t Index);
            };
        }
    }
}
#endif // __APTPUBDEBUGCALLSTACK_H__
