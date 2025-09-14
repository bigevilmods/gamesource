/*H************************************************************************************************/
/*!

    \File    AptGlobalExtensionObject.cpp

    \Description
        Simple description of main purpose of file.

    \Notes
        None.

    \Copyright
        (c) 2004 Electronic Arts Inc.

    \Version    1.0        04/02/04 (JSH) First Version

*/
/************************************************************************************************H*/


/*** Include files ********************************************************************************/

#include "AptObject/AptGlobalExtensionObject.h"
#include "MainInline.h"

#if !defined(APT_ENABLE_INLINE)
    #include "AptObject\AptGlobalExtensionObject.inl"
#endif

/*** Defines **************************************************************************************/

/*** Macros ***************************************************************************************/

/*** Type Definitions *****************************************************************************/

/*** Function Prototypes **************************************************************************/

/*** Variables ************************************************************************************/
APT_DEFINE_OBJECT_METRICS(AptGlobalExtensionObject, AptObject);
// Private variables

// Public variables

AptGlobalExtensionObject *gpGlobalExtensionObject = NULL;

/*** Private Functions ****************************************************************************/

/*** Public Functions *****************************************************************************/
