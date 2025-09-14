// (c) Electronic Arts. All Rights Reserved.

/// \file
/// IDs for RW platform components. Mainly used on object types for arenas.

#ifndef RW_COMPONENTIDS_H
#define RW_COMPONENTIDS_H

#include <rw/core/base/ostypes.h>

/*************************************************************************************************************
 Includes
 */

/*************************************************************************************************************
 Defines
 */

#define RWMAKEOBJECTTYPE(_comp, _obj) ((((_comp) & 0xff) << 16) | ((_obj) & 0xff))

/*************************************************************************************************************
 Enums
 */

enum rwComponentID
{
    rwCOMPONENTID_NA        = 0x00,

    rwCOMPONENTID_CORE      = 0x01,
    rwCOMPONENTID_GRAPHICS  = 0x02,
    rwCOMPONENTID_AUDIO     = 0x03,
    rwCOMPONENTID_PHYSICS   = 0x04,
    rwCOMPONENTID_NETWORK   = 0x05,
    rwCOMPONENTID_AI        = 0x06,
    rwCOMPONENTID_ANIMATION = 0x07,
    rwCOMPONENTID_COLLISION = 0x08,
    rwCOMPONENTID_PLATFORM  = 0x09,

    rwCOMPONENTID_FORCEENUMSIZEINT = RWFORCEENUMSIZEINT
};

/*************************************************************************************************************
 Typedef Enums
 */

typedef enum rwComponentID rwComponentID;

/*************************************************************************************************************
 Typedef Structs + Unions
 */

/*************************************************************************************************************
 Typedef Function Pointers
 */

/*************************************************************************************************************
 Structs + Unions
 */

/*************************************************************************************************************
 Global Variables
 */

/*************************************************************************************************************
 Internal Functions Prototypes
 */

/*************************************************************************************************************
 External Functions Prototypes
 */

#endif /* RW_COMPONENTIDS_H */
