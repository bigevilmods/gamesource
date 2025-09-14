/*H************************************************************************************************/
/*!

    \File    AptDebugHelper.h

    \Description
        This is a helper class that exposes debug data

    \Notes
        None.

    \Copyright
        (c) 2018 Electronic Arts Inc.

    \Version    1.0        09/03/18 (GM) First Version

*/
/************************************************************************************************H*/


#ifndef _AptDebugHelper_h
#define _AptDebugHelper_h

/*** Include files ********************************************************************************/
#include "EASTL/bonus/ring_buffer.h"
#include "EASTL/fixed_vector.h"

/*** Defines **************************************************************************************/

/*** Macros ***************************************************************************************/

/*** Type Definitions *****************************************************************************/

/*** Variables ************************************************************************************/

/*** Functions ************************************************************************************/

// Forward Declarations


class FB_APT_API AptDebugHelper
{
public:
	typedef void (*pushBinaryCbFunc) (uint8_t* dataValue, size_t dataSize);

    // Singleton instance function
	static bool						Initialize();
	static bool 					Shutdown();
    static AptDebugHelper *         GetInstance();
	
	void							AddTrace(const AptNativeString & aptTraceStr);
	void							SetPushBinaryCB(pushBinaryCbFunc pushBinaryCB) { mPushBinaryCb = pushBinaryCB; }

private:
									AptDebugHelper();				//!< Constructor
									~AptDebugHelper();				//!< Destructor

	static AptDebugHelper*			sInstance;						//!< Singleton instance

	pushBinaryCbFunc				mPushBinaryCb;

};  // end of AptDebugHelper


#endif // _AptDebugHelper_h
