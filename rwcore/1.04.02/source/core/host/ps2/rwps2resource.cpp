// (c) Electronic Arts. All Rights Reserved.
/*************************************************************************************************************

    File: rwps2resource.cpp

    Purpose: A short description of the file.

 */

/*************************************************************************************************************
 Includes
 */
#include "rwsharedheaders.hpp"
#include "rw/core/base/ps2.h"
#include "rw/core/ps2resource.h"

/*************************************************************************************************************
 Defines
 */
/*
\todoc
*/
#define rw_MEM_RESOURCE_LOCK(lockId) \
    { sizeof(lockId) / sizeof((lockId)[0]), lockId }

/*************************************************************************************************************
 Enums
 */

/*************************************************************************************************************
 Typedef Enums
 */

/*************************************************************************************************************
 Typedef Structs + Unions
 */
typedef struct rw_MemResourceLock rw_MemResourceLock;

/*************************************************************************************************************
 Typedef Function Pointers
 */

/*************************************************************************************************************
 Structs + Unions
 */
/*
\todoc
*/
struct rw_MemResourceLock
{
    uint32_t size;
    uint32_t *lockId;
};

/*************************************************************************************************************
 Static Variables
 */
/*
\todoc
*/
#ifdef EA_DEBUG
static uint32_t rw_ResourceLockId[RWRESOURCE_NUM];
/*
\todoc
*/
static uint32_t rw_SPRLockId[rwSPR_SIZE];
static uint32_t rw_VU0MemLockId[rwVU0MEM_SIZE];
static uint32_t rw_VU1MemLockId[rwVU1MEM_SIZE];
static uint32_t rw_GSMemLockId[rwGSMEM_SIZE];
/*
\todoc
*/
static const rw_MemResourceLock rw_MemResourceLockId[RWMEMRESOURCE_NUM] = {
    rw_MEM_RESOURCE_LOCK(rw_SPRLockId),
    rw_MEM_RESOURCE_LOCK(rw_VU0MemLockId),
    rw_MEM_RESOURCE_LOCK(rw_VU1MemLockId),
    rw_MEM_RESOURCE_LOCK(rw_GSMemLockId)
};
#endif

/*************************************************************************************************************
 Global Variables
 */

/*************************************************************************************************************
 Function prototypes
 */

/*************************************************************************************************************
 Functions
 */
/*
\todoc
*/
#ifdef EA_DEBUG
void
RwResourceStart(void)
{
    uint32_t i;
    uint32_t j;

    rwFUNCTION();
    for (i = 0; i < RWRESOURCE_NUM; i++)
    {
        rw_ResourceLockId[i] = 0;
    }
    for (i = 0; i < RWMEMRESOURCE_NUM; i++)
    {
        for (j = 0; j < rw_MemResourceLockId[i].size; j++)
        {
            rw_MemResourceLockId[i].lockId[j] = 0;
        }
    }

    rwRETURNVOID();
}

/*
\todoc
*/
void
RwResourceLock(RwResource resource, uint32_t id)
{
    rwFUNCTION();
    rwASSERT(0 != id);
    rwASSERT(0 == rw_ResourceLockId[resource]);
    rw_ResourceLockId[resource] = id;

    rwRETURNVOID();
}

/*
\todoc
*/
void
RwResourceUse(RwResource resource, uint32_t id)
{
    rwFUNCTION();
    rwASSERT(0 != id);
    rwASSERT(id == rw_ResourceLockId[resource]);

    rwRETURNVOID();
}

/*
\todoc
*/
void
RwResourceUnlock(RwResource resource, uint32_t id)
{
    rwFUNCTION();
    rwASSERT(0 != id);
    rwASSERT(id == rw_ResourceLockId[resource]);
    rw_ResourceLockId[resource] = 0;

    rwRETURNVOID();
}

/*
\todoc
*/
void
RwMemResourceLock(RwMemResource resource, uint32_t id, uint32_t low, uint32_t high)
{
    uint32_t i;

    rwFUNCTION();
    rwASSERT(0 != id);
    rwASSERT(low < high);
    rwASSERT(high <= rw_MemResourceLockId[resource].size);
    for (i = low; i < high; i++)
    {
        rwASSERT(0 == rw_MemResourceLockId[resource].lockId[i]);
        rw_MemResourceLockId[resource].lockId[i] = id;
    }

    rwRETURNVOID();
}

/*
\todoc
*/
void
RwMemResourceUse(RwMemResource resource, uint32_t id, uint32_t low, uint32_t high)
{
    uint32_t i;

    rwFUNCTION();
    rwASSERT(0 != id);
    rwASSERT(low < high);
    rwASSERT(high <= rw_MemResourceLockId[resource].size);
    for (i = low; i < high; i++)
    {
        rwASSERT(id == rw_MemResourceLockId[resource].lockId[i]);
    }

    rwRETURNVOID();
}

/*
\todoc
*/
void
RwMemResourceUnlock(RwMemResource resource, uint32_t id, uint32_t low, uint32_t high)
{
    uint32_t i;

    rwFUNCTION();
    rwASSERT(0 != id);
    rwASSERT(low < high);
    rwASSERT(high <= rw_MemResourceLockId[resource].size);
    for (i = low; i < high; i++)
    {
        rwASSERT(id == rw_MemResourceLockId[resource].lockId[i]);
        rw_MemResourceLockId[resource].lockId[i] = 0;
    }

    rwRETURNVOID();
}
#endif
