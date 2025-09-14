#include "_Apt.h"

#include "MainInline.h"
#if defined APT_USE_BUTTONS
/*F************************************************************************************************/
/*!
    PreDestroy the object, mainly to keep compatibility with old __dec behavior.

    \Input      None.

    \Output     None.

    \Notes
        None.

    \Version
        1.0        03/17/04 (ONA) First Version

*/
/************************************************************************************************F*/
void    AptCharacterButtonInst::PreDestroy()
{
    mDisplayList.PreDestroy();
}
#endif
