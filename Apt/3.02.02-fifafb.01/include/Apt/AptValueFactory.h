/*H************************************************************************************************/
/*!

\File
AptValueFactory.h

\Description
Generates various AptValues such as String, Integer, Float, Boolean, Array and Formatted String, which are garbage-collected.

\Notes
None.

\Copyright
(c) 2004 Electronic Arts Inc.

\Version
1.0     2004-12-20  (DUG)   First Version

*/
/************************************************************************************************H*/
#ifndef _AptValueFactory_h_
#define _AptValueFactory_h_

// Forward Declarations
#include "AptValue\AptValue.h"
// FIFA_BEGIN | SENDBACK | mwebb | 2015 - 05 - 26 | Adding Frostbite support
#include "Module.h"

//class AptValueFactory
class FB_APT_API AptValueFactory
// FIFA_END
{

public:

     static AptValue * CreateString(const char * szValue);
     static AptValue * CreateInteger(int nValue) ;
     static AptValue * CreateFloat(float fValue) ;
     static AptValue * CreateBoolean(bool bValue) ;
     static AptValue * CreateArray(int nElements, AptValue ** pAptValue) ;
     static AptValue * CreateArray() ;
     static AptValue * CreateObject() ;
     static AptValue * CreateStringFormatted(const char * szFormat, ...) ;
     static AptValue * CreateUndefined() ;

};  // end of AptValueFactory
#endif // _AptValueFactory_h_
