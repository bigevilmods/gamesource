/*H************************************************************************************************/
/*!

    \File    AptMemoryMetricsImpl.h

    \Description
        Encapsulates implementation of private AptMemoryMetrics methods.
        Lets us have a private GetMetrics method (returning non-const pointer),
        Initialize and InitializeItem, without exposing it to the user.

    \Notes
        None.

    \Copyright
        (c) 2008 Electronic Arts Inc.

    \Version    1.0        07/22/08 (RB)  First Version

*/
/************************************************************************************************H*/

/*** Include guard ************************************************************/

#ifndef APTMEMORYMETRICSIMPL_H
#define APTMEMORYMETRICSIMPL_H

/*** Includes *****************************************************************/

#include "AptMemoryMetrics.h"

/*** Macros *******************************************************************/

/*** Interface ****************************************************************/

/// Pimpl implementation class for AptHighwaterMemoryMetrics
struct AptHighwaterMemoryMetricsImpl : public AptHighwaterMemoryMetrics
{
    static int32_t DefaultOverflowCallback(const AptHighwaterMemoryMetrics& metrics, void*, void* );
    static void InitializeItem( AptHighwaterMemoryMetrics::Type type, int32_t initialMaxCount, int32_t estimatedBytesPerItem, const char* helpfulHint, int32_t defaultPolicy = (int32_t) POLICY_APT_WILL_ASSERT );
    static void Initialize();
    static AptHighwaterMemoryMetrics* GetMetrics( Type type );
};

/*** Implementation ***********************************************************/

/******************************************************************************/



#endif  //  APTMEMORYMETRICSIMPL_H

