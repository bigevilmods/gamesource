// (c) Electronic Arts. All Rights Reserved.
/* RWPUBLIC */
/*************************************************************************************************************

 File: rwdiag.h

 Purpose:

 */

#ifndef RW_DIAG_H
#define RW_DIAG_H

/* RWOLDPUBLIC */

/*************************************************************************************************************
 Includes
 */
#include <rw/core/base/ostypes.h>

/*************************************************************************************************************
 Defines
 */

/*************************************************************************************************************
 Enums
 */

enum RwDiagFunction
{
    RWDIAGFUNCTION_NA = -1,
    RWDIAGFUNCTION_GIFPATH,
    RWDIAGFUNCTION_DMA,
    RWDIAGFUNCTION_DMAWAIT,
    RWDIAGFUNCTION_VU1STATUS,
    RWDIAGFUNCTION_VIF1STATUS,
    RWDIAGFUNCTION_VIF1FIFO,

    RWDIAGFUNCTION_FORCEENUMSIZEINT = RWFORCEENUMSIZEINT
};

/*************************************************************************************************************
 Typedef Enums
 */
/*
\todoc
*/
typedef enum RwDiagFunction RwDiagFunction;

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
extern void RwDiagOpen(RwDiagFunction function);
extern void RwDiagClose(void);

/* RWOLDPUBLICEND */

#endif /* RW_DIAG_H */
/* RWPUBLICEND */
