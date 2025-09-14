// (c) Electronic Arts. All Rights Reserved.

/// \file
/// SPU thread management

#ifndef RW_SPUTHREAD_HPP
#define RW_SPUTHREAD_HPP

// ***********************************************************************************************************
// Includes

#include <sys/event.h>
#include <sys/spu_thread.h>
#include <sys/event.h>

#include <rw/core/base/ostypes.h>
#include <rw/core/debug/debug.h>

#include <sys/paths.h>
#include <sdk_version.h>
#include <spu_image.h>

// ***********************************************************************************************************
// Defines

#define SPUTHREAD_CHECKRETURN(x)                                                      \
do {                                                                                  \
    rwVERBOSE((#x "\n"));                                                             \
    int32_t returnValue = x;                                                          \
    if (returnValue != CELL_OK)                                                       \
    {                                                                                 \
        rwMESSAGE(("Failed: " #x ". Return value = %d.", returnValue));               \
        rwRETURN(FALSE);                                                              \
    }                                                                                 \
    else                                                                              \
    {                                                                                 \
        rwVERBOSE(("succeeded\n"));                                                   \
    }                                                                                 \
} while (0)

namespace rw
{

// ***********************************************************************************************************
// Forward Declarations


// ***********************************************************************************************************
// Typedefs


// ***********************************************************************************************************
// Enums + Consts


// ***********************************************************************************************************
// Global Variables


// ***********************************************************************************************************
// External Functions Prototypes


// ***********************************************************************************************************
// Structs + Unions + Classes

// ***********************************************************************************************************
//                                                SpuThread Definition
// ***********************************************************************************************************

class SpuThread
{
public:
    // SPU event queues
    sys_event_queue_t sendQueue;
    sys_event_port_t ppuSendPort;
    sys_event_queue_t receiveQueue;

    // SPU thread group
    sys_spu_thread_group_t threadGroup;

    // SPU thread
    sys_spu_thread_t thread;

    // SPU elf image
    sys_spu_image threadImage;

    // spu_printf handler thread and queue
    sys_event_queue_t spuPrintfSendQueue;

    RwBool InitializeSPUs();

    RwBool InitializeThreadGroup(const char* groupName, int32_t priority);

    RwBool InitializeThread(const char* elfName, const char* threadName, uint32_t arg1, uint64_t arg2);

    RwBool Initialize(const char* elfName, const char* groupName, const char* threadName, int32_t priority);
    RwBool Initialize(const char* elfName, const char* groupName, const char* threadName, int32_t priority, uint32_t arg1, uint64_t arg2);

    RwBool InitializeSpuSendEventQueue(uint32_t port,
                                       uint32_t key = SYS_EVENT_QUEUE_LOCAL,
                                       int32_t size = 127);

    RwBool InitializeSpuReceiveEventQueue(uint32_t queueNum,
                                          uint32_t port = SYS_EVENT_PORT_LOCAL,
                                          int32_t size = 127);

    RwBool InitializeSpuPrintfEventQueue();

    RwBool InitializeSpuPrintfHandlerThread();

    RwBool StartThreadGroup();

    RwBool WaitForThreadGroup(int32_t& exitCause, int32_t& exitStatus);

    RwBool Release();
};

// Inline Functions

} //namespace rw

#endif // RW_SPUTHREAD_HPP
