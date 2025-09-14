/*H************************************************************************************************/
/*!

    \File    AptStackHelper.h

    \Description
        This is a helper class that exposes functions to manipulate the AptActionInterpreter stack

    \Notes
        None.

    \Copyright
        (c) 2011 Electronic Arts Inc.

    \Version    1.0        02/04/11 (MAH) First Version

*/
/************************************************************************************************H*/


#ifndef _AptStackHelper_h
#define _AptStackHelper_h

/*** Include files ********************************************************************************/


#include "eabase/eabase.h"

/*** Defines **************************************************************************************/

/*** Macros ***************************************************************************************/

/*** Type Definitions *****************************************************************************/

/*** Variables ************************************************************************************/

/*** Functions ************************************************************************************/

// Forward Declarations
class AptValue;


class AptStackHelper
{
public:
    static AptValue * GetStackAt( const int index );


};  // end of AptValueHelper


#endif // _AptStackHelper_h
