// (c) Electronic Arts. All Rights Reserved.

/// \file
/// SPU Thread management.

// ***********************************************************************************************************
// Includes
#include "rwsharedheaders.hpp"
#include "rw/core/spu.h"
#include "rw/core/sputhread.h"
#include <stdio.h>
#include <sys/process.h>
#include <sys/spu_initialize.h>
#include <sys/spu_thread_group.h>
#include <sys/interrupt.h>
#include <sys/spu_utility.h>
#include <spu_printf.h>


#if !defined(EVENT_PRINTF_PORT)
#define EVENT_PRINTF_PORT 1U
#endif // !defined(EVENT_PRINTF_PORT)

// ***********************************************************************************************************
// Defines

namespace
{

// ***********************************************************************************************************
// Typedefs


// ***********************************************************************************************************
// Enums + Consts


// ***********************************************************************************************************
// Structs + Unions + Classes


// ***********************************************************************************************************
// Static Variables


// ***********************************************************************************************************
// Static Functions

int32_t handleSpuPrintf(sys_spu_thread_t id, uint32_t arg_addr)
{
    rwFUNCTION();
    int32_t retval;
    char buffer[1024];
    // this should be snprintf but there doesn't appear to be a spu_thread_snprintf
    spu_thread_sprintf(buffer, id, arg_addr);
    retval = printf("spu %x: %s", id, buffer);
    rwRETURN(retval);
}

void spuPrintEventHandler(uint64_t arg) RWNORETURN;
void spuPrintEventHandler(uint64_t arg)
{
    rwFUNCTION();
    sys_event_queue_t* queue = reinterpret_cast<sys_event_queue_t*>(static_cast<uint32_t>(arg));
    rwASSERT(queue);

    while (true)
    {
        sys_event_t event;

        // Wait for event from SPU solver thread
        if (sys_event_queue_receive(*queue, &event, SYS_NO_TIMEOUT) != CELL_OK)
        {
            rwMESSAGE(("sys_event_queue_receive failed."));
            sys_ppu_thread_exit(0);
        }

        // Extract SPU event parameters
        sys_spu_thread_t source_thr_id = static_cast<sys_spu_thread_t>(event.data1);
        uint8_t spu_port               = static_cast<uint8_t>(event.data2 >> 32);
        uint32_t data0                 = static_cast<uint32_t>(event.data2 & 0xffffffff);
        uint32_t data1                 = static_cast<uint32_t>(event.data3);

        rwVERBOSE(("Received event from spu thread id = %p\t"
                   "spu port = 0x%x\tdata0 = 0x%x\tdata1 = 0x%x\n",
                    source_thr_id, spu_port, data0, data1));

        if ((spu_port != EVENT_PRINTF_PORT) && (data0 != 0))
        {
            rwMESSAGE(("event from unknown port received on spu_printf event queue - "
                       "event.source = %p\tevent.data1 = %p\tevent.data2 = %p\tevent.data3 = %p\n",
                       event.source, event.data1, event.data2, event.data3));
        }
        int32_t retVal = handleSpuPrintf(source_thr_id, data1);
        sys_spu_thread_write_spu_mb(source_thr_id, retVal);
    }
}

} // anonymous namespace

namespace rw
{

// ***********************************************************************************************************
// External Functions


// ***********************************************************************************************************
//                                                SpuThread Definition
// ***********************************************************************************************************

RwBool
SpuThread::InitializeSPUs()
{
    rwFUNCTION();
    int32_t retVal;

    // Initialize 1 SPU as raw, the rest for SPU threads
    rwVERBOSE(("sys_spu_initialize\n"));
    retVal = sys_spu_initialize(6, 1);
    if (retVal == EBUSY)
    {
        rwVERBOSE(("sys_spu_initialize: SPUs have already been initialized.\n"));
    }
    else if (retVal != CELL_OK)
    {
        rwMESSAGE(("sys_spu_initialize has failed (%d)", retVal));
        return FALSE;
    }

    rwRETURN(TRUE);
}


RwBool
SpuThread::InitializeThreadGroup(const char* groupName, int32_t priority)
{
    rwFUNCTION();

    // Create thread group
    sys_spu_thread_group_attribute_t groupAttr;
    groupAttr.name = groupName;
    groupAttr.nsize = strlen(groupAttr.name) + 1;
    groupAttr.type = SYS_SPU_THREAD_GROUP_TYPE_NORMAL;
    SPUTHREAD_CHECKRETURN(sys_spu_thread_group_create(&threadGroup, 1, priority, &groupAttr));

    rwRETURN(TRUE);
}

RwBool
SpuThread::InitializeThread(const char* elfName, const char* threadName, uint32_t arg1, uint64_t arg2)
{
    rwFUNCTION();

    sys_spu_thread_attribute_t threadAttr;

    // Open thread image
    SPUTHREAD_CHECKRETURN(sys_spu_image_open(&threadImage, elfName));

    // Set attributes
    threadAttr.name = threadName;
    threadAttr.nsize = strlen(threadName) + 1;
    sys_spu_thread_argument_t threadArg;
    threadArg.arg1 = SYS_SPU_THREAD_ARGUMENT_LET_32(arg1);
    threadArg.arg2 = SYS_SPU_THREAD_ARGUMENT_LET_64(arg2);

    threadAttr.option = SYS_SPU_THREAD_OPTION_NONE;

    // Initialize the SPU thread
    SPUTHREAD_CHECKRETURN(sys_spu_thread_initialize(&thread, threadGroup, 0,
        &threadImage, // extra parameter
        &threadAttr, &threadArg));

    rwRETURN(TRUE);
}

RwBool
SpuThread::Initialize(const char* elfName, const char* groupName, const char* threadName, int32_t priority)
{
    rwFUNCTION();
    rwRETURN(Initialize(elfName, groupName, threadName, priority, 0, 0));
}

RwBool
SpuThread::Initialize(const char* elfName, const char* groupName, const char* threadName,
                      int32_t priority, uint32_t arg1, uint64_t arg2)
{
    rwFUNCTION();

    rwRETURN(InitializeSPUs() &&
             InitializeThreadGroup(groupName, priority) &&
             InitializeThread(elfName, threadName, arg1, arg2));
}


RwBool
SpuThread::InitializeSpuSendEventQueue(uint32_t port, uint32_t key, int32_t size)
{
    rwFUNCTION();

    // Initialize the send event queue
    sys_event_queue_attribute_t queueAttr;
    queueAttr.attr_protocol = SYS_SYNC_PRIORITY;
    queueAttr.type = SYS_PPU_QUEUE;
    SPUTHREAD_CHECKRETURN(sys_event_queue_create(&sendQueue, &queueAttr, key, size));

    // Connect the SPU thread user event type to the send event queue
    rwVERBOSE(("Connecting SPU thread (%d) to the event port %d.\n", thread, port));
    SPUTHREAD_CHECKRETURN(sys_spu_thread_connect_event(thread, sendQueue, SYS_SPU_THREAD_EVENT_USER, port));

    rwRETURN(TRUE);
}

RwBool
SpuThread::InitializeSpuReceiveEventQueue(uint32_t spuQueueNum, uint32_t port, int32_t size)
{
    rwFUNCTION();

    // Initialize the received event queue
    sys_event_queue_attribute_t queueAttr;
    queueAttr.attr_protocol = SYS_SYNC_PRIORITY;
    queueAttr.type = SYS_SPU_QUEUE;
    SPUTHREAD_CHECKRETURN(sys_event_queue_create(&receiveQueue, &queueAttr, SYS_EVENT_QUEUE_LOCAL, size));

    // Connect the spu receive queue to a send port on the PPU
    SPUTHREAD_CHECKRETURN(sys_event_port_create(&ppuSendPort, port, SYS_EVENT_PORT_NO_NAME));
    SPUTHREAD_CHECKRETURN(sys_event_port_connect_local(ppuSendPort, receiveQueue));

    // Bind the receive event queue to the SPU thread
    rwVERBOSE(("Binding SPU thread (%d) to the event queue id %d.\n", thread, spuQueueNum));
    SPUTHREAD_CHECKRETURN(sys_spu_thread_bind_queue(thread, receiveQueue, spuQueueNum));

    rwRETURN(TRUE);
}


RwBool
SpuThread::InitializeSpuPrintfEventQueue()
{
    rwFUNCTION();

    // Initialize the send event queue
    sys_event_queue_attribute_t queueAttr;
    queueAttr.attr_protocol = SYS_SYNC_PRIORITY;
    queueAttr.type = SYS_PPU_QUEUE;
    SPUTHREAD_CHECKRETURN(sys_event_queue_create(&spuPrintfSendQueue, &queueAttr, SYS_EVENT_QUEUE_LOCAL, 127));

    rwVERBOSE(("Connecting SPU thread (%d) for spu_printf to the event port %d.\n", thread, 127));
    SPUTHREAD_CHECKRETURN(sys_spu_thread_connect_event(thread,
                                                       spuPrintfSendQueue,
                                                       SYS_SPU_THREAD_EVENT_USER,
                                                       EVENT_PRINTF_PORT));

    rwRETURN(TRUE);
}

RwBool
SpuThread::InitializeSpuPrintfHandlerThread()
{
    rwFUNCTION();

    rwVERBOSE(("Starting PPU thread to handle spu_printf events.\n"));
    sys_ppu_thread_t  spuPrintfThread;
    SPUTHREAD_CHECKRETURN(sys_ppu_thread_create(&spuPrintfThread,
                                                 spuPrintEventHandler,
                                                 reinterpret_cast<uint64_t>(&spuPrintfSendQueue), // data
                                                 3071, // lowest priority possible
                                                 2048, // stack size
                                                 0,    // ppu thread flags
                                                 "spu_printf_event_handler"));

    rwRETURN(TRUE);
}


RwBool
SpuThread::StartThreadGroup()
{
    rwFUNCTION();
    // Start the thread group.
    SPUTHREAD_CHECKRETURN(sys_spu_thread_group_start(threadGroup));
    rwRETURN(TRUE);
}


RwBool
SpuThread::WaitForThreadGroup(int32_t& exitCause, int32_t& exitStatus)
{
    rwFUNCTION();
    SPUTHREAD_CHECKRETURN(sys_spu_thread_group_join(threadGroup, &exitCause, &exitStatus));
    rwRETURN(TRUE);
}


RwBool
SpuThread::Release()
{
    rwFUNCTION();

    sys_spu_thread_group_terminate(threadGroup, 0);
    if (receiveQueue)
    {
        sys_event_queue_destroy(receiveQueue, 0);
    }
    if (ppuSendPort)
    {
        sys_event_port_disconnect(ppuSendPort);
        sys_event_port_destroy(ppuSendPort);
    }
    if (sendQueue)
    {
        sys_event_queue_destroy(sendQueue, 0);
    }
    if (threadGroup)
    {
        sys_spu_thread_group_destroy(threadGroup);
    }

    rwRETURN(TRUE);
}


} //namespace rw
