/*H************************************************************************************************/
/*!

\File
AptVersion.cpp

\Description
This file is parsed and updated automatically during the build
process with the new buid number and date.

\Notes
None.

\Copyright
(c) 2005 Electronic Arts Inc.

\Version    1.0        01/04/05 (MCB) First Version

*/
/************************************************************************************************H*/

#include "_Apt.h"

#include "MainInline.h"

const char *AptDebugVersionString()
{
    // XXX note that this string is sneakily formatted for a lame sed script in makedevrelease.pl
    return
        "Apt-3.02.02" // version
        " built on "
        "5/23/2014 2:30:37 PM" // date
        ;
}
