// (c) Electronic Arts. All Rights Reserved.
/* RWPUBLIC */
/*************************************************************************************************************

    File: rwps2resource.h

    Purpose: A short description of the file.

 */

#ifndef RW_PS2RESOURCE_H
#define RW_PS2RESOURCE_H

/* RWOLDPUBLIC */
/*************************************************************************************************************
 Includes
 */
#include <rw/core/base/ostypes.h>
#include <rw/core/debug/debug.h>

/*************************************************************************************************************
 Defines
 */
/*
\todoc
*/
#define rwRESOURCEID_GRAPHICSDMA          (0x00000001)
#define rwRESOURCEID_GRAPHICSDISPATCH     (0x00000002)
#define rwRESOURCEID_GRAPHICSENGINE       (0x00000003)
#define rwRESOURCEID_GRAPHICSCAMERA       (0x00000004)
#define rwRESOURCEID_GRAPHICSTEXTURECACHE (0x00000005)
#define rwRESOURCEID_GRAPHICSVERTEXBUFFER (0x00000006)
#define rwRESOURCEID_PHYSICS              (0x00000007)
/*
\todoc
*/
#ifndef EA_DEBUG
#define RwResourceStart()
#define RwResourceLock(resource, id)
#define RwResourceUse(resource, id)
#define RwResourceUnlock(resource, id)
#define RwMemResourceLock(resource, id, low, high)
#define RwMemResourceUse(resource, id, low, high)
#define RwMemResourceUnlock(resource, id, low, high)
#endif

/*************************************************************************************************************
 Enums
 */
/*
\todoc
*/
enum RwResource
{
    RWRESOURCE_VU0,
    RWRESOURCE_DMA0,
    RWRESOURCE_DMA1,
    RWRESOURCE_DMA2,
    RWRESOURCE_NUM,
    RWRESOURCE_NA,
    RWRESOURCE_FORCEENUMSIZEINT = RWFORCEENUMSIZEINT
};
/*
\todoc
*/
enum RwMemResource
{
    RWMEMRESOURCE_SPR,
    RWMEMRESOURCE_VU0,
    RWMEMRESOURCE_VU1,
    RWMEMRESOURCE_GS,
    RWMEMRESOURCE_NUM,
    RWMEMRESOURCE_NA,
    RWMEMRESOURCE_FORCEENUMSIZEINT = RWFORCEENUMSIZEINT
};

/*************************************************************************************************************
 Typedef Enums
 */
/*
\todoc
*/
typedef enum RwResource RwResource;
/*
\todoc
*/
typedef enum RwMemResource RwMemResource;

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
 Function prototypes
 */
#ifdef EA_DEBUG
extern void RwResourceStart(void);
extern void RwResourceLock(RwResource resource, uint32_t id);
extern void RwResourceUse(RwResource resource, uint32_t id);
extern void RwResourceUnlock(RwResource resource, uint32_t id);
extern void RwMemResourceLock(RwMemResource resource, uint32_t id, uint32_t low, uint32_t high);
extern void RwMemResourceUse(RwMemResource resource, uint32_t id, uint32_t low, uint32_t high);
extern void RwMemResourceUnlock(RwMemResource resource, uint32_t id, uint32_t low, uint32_t high);
#endif

/* RWOLDPUBLICEND */

#endif /* RW_RESOURCE_H */
/* RWPUBLICEND */
