// (c) Electronic Arts. All Rights Reserved.

/// \file
/// Manager class implementation

#include "rw/core/debug/manager.h"

#include "rw/core/base/ostypes.h"
#include "rw/core/debug/debug.h"
#include "rw/core/debug/host.h"
#include "rw/core/debug/channel.h"
#include "rw/core/debug/device.h"
#include "rw/core/debug/consoledevice.h"
#include "rw/core/debug/formatter.h"
#include "rw/core/debug/asserthandler.h"
#include "rw/core/resource/resourceallocator.h"
#include "rw/core/resource/resourceallocatorregistry.h"

#include "rw/core/string.h"

namespace rw {
namespace core {
namespace debug {

/// allocator used to allocate elements of the debug system.

//lint -e{956}  // Non const, non volatile static or external variable (intended)
rw::IResourceAllocator *Manager::sAllocator = NULL;

#ifndef EA_DEBUG

IFormatter *Manager::sFormatter = NULL;

/**
  \brief initialize the debug system

  CreateInstance can only be called once, if called twice
  it will assert. CreateInstance must be called before
  before calling GetInstance.

  \param allocator IResourceAllocator to use for the entire debug system
*/
Manager *Manager::CreateInstance(rw::IResourceAllocator *allocator)
{
    rwFUNCTION();
    sAllocator = allocator;
    sFormatter = new Formatter;

    rwRETURN(NULL);
}

/**
  \brief shut down the debug system.

  This will destroy the debug manager instance.
  Calling GetInstance() after calling DestroyInstance()
  will assert.
*/
void Manager::DestroyInstance()
{
    rwFUNCTION();
    delete sFormatter;
    rwRETURNVOID();
}

/**
  \brief retrieve the debug system IResourceAllocator
*/
rw::IResourceAllocator *Manager::GetAllocator()
{
    rwFUNCTION();
    rwRETURN(sAllocator);
}

#else   // EA_DEBUG is defined

/// singleton instance.
//lint -e{956}  // Non const, non volatile static or external variable (intended)
Manager *Manager::sInstance = NULL;

/// mutex for the debug Manager
//lint -e{956, 1935, 1937}  // Non const, non volatile static or external variable (intended)
rw::core::debug::detail::DebugCriticalSection Manager::sMutex;

PrintOpts Manager::sPrintOpts;

/**
  name of the system channel.
*/
const char *SYSTEMCHANNEL = "system";


/**
  format string for asserts.
*/
const char *ASSERTFORMAT = "%s(%d): Assert %s in function %s:";

// ------------------------------------------------------------
static void DebugManagerMessageCallback(const char * channelname,const char * file , uint32_t line, const char * format,va_list args)
{
    rwFUNCTION();
    Channel * channel = NULL;
    // Ensure we can access the debug manager
    rw::core::debug::Manager * manager = rw::core::debug::Manager::GetInstance();
    if(manager == NULL)
    {
        rwRETURNVOID();
    }

    // ensure the user provided a valid output channel
    if(channelname == NULL)
    {
        channel = manager->GetSystemChannel();
    }
    else
    {
        channel = manager->GetChannel(channelname);
    }

    // Once we have the channel output the information
    if(channel != NULL)
    {
        channel->EnterCritical();
        channel->SetInfo(file,line);
        channel->Vprint(format,args);
        channel->Print("\n");
        channel->LeaveCritical();
    }
    rwRETURNVOID();
}
// ------------------------------------------------------------
static void DebugManagerChannelCallback(Channel * channel,const char * file , uint32_t line,const char * format, va_list args)
{
    rwFUNCTION();
    // allow users to print on the system channel by passing
    // in null for the channel object
    if(channel == NULL)
    {
        rw::core::debug::Manager * manager = rw::core::debug::Manager::GetInstance();
        if(manager == NULL)
        {
            rwRETURNVOID();
        }
        channel = manager->GetSystemChannel();
    }
    if(channel != NULL)
    {
        channel->EnterCritical();
        channel->SetInfo(file,line);
        channel->Vprint(format,args);
        channel->Print("\n");
        channel->LeaveCritical();
    }
    rwRETURNVOID();
}
// ------------------------------------------------------------
static bool DebugManagerPrintAssertCallback(const char * condition, const char * file,
                                     const char * function , unsigned int line   ,
                                     const char * format   , va_list      argList,
                                     unsigned int flags)
{
    rwFUNCTION();
    rwRETURN(rw::core::debug::Manager::AssertFailed(condition,file,function,line,format,argList,flags));
}


///////////////////////////////////////////////////////////////////////////
//
// singleton functions
//

/**
  \brief Create the debug Manager singleton instance.
  \param allocator the allocator that the debug system will use for internal allocations.
  \return the instance created.
*/
Manager *Manager::CreateInstance(rw::IResourceAllocator *allocator)
{
    rwFUNCTION();

    sMutex.Create();

    rwASSERTM(sInstance == NULL, ("instance already exists."));

    sAllocator = allocator;
    sInstance = new Manager();
    sInstance->Init();

    // Hook up the debug manager to the RW debugging hooks
    SetDebugMessageFunction(DebugManagerMessageCallback);
    SetDebugMessageChannelFunction(DebugManagerChannelCallback);
    SetDebugPrintAssertFunction(DebugManagerPrintAssertCallback);

    rwRETURN(sInstance);
}

/**
  \brief Return the debug Manager singleton instance.
  \return the singleton instance.
*/
Manager *Manager::GetInstance()
{
    rwFUNCTION();

#if defined(RWCROSS)

    // permit lazy initialization in cross builds, to avoid having each DLL
    // need to initialize the debug manager explicitly.  Still not reference-
    // counted, so the caller will still need to call DestroyInstance before
    // program termination if they want to completely release all resources
    // before exiting.
    if (sInstance == NULL)
        CreateInstance(&rw::GetDefaultAllocator());

#else

    // in native, where DLLs should not be a concern, simply assert that the 
    // caller has created the debug manager instance as part of the global
    // initialization sequence.
    rwASSERTM(sInstance != NULL,
        ("instance does not exist, call Manager::CreateInstance() first."));
#endif

    rwRETURN(sInstance);
}


/**
  \brief Destroy the debug Manager singleton instance.

  Call this function to shut down the debug Manager.
*/
void Manager::DestroyInstance()
{
    rwFUNCTION();
    rwASSERTM(sInstance != NULL,
        ("instance does not exist, call Manager::CreateInstance() first."));

    // Manager has its own delete operator
    delete sInstance;
    sInstance = NULL;
    sAllocator = NULL;

    sMutex.Destroy();
    rwRETURNVOID();
}

///////////////////////////////////////////////////////////////////////////
//
// static functions
//


/**
  \brief Allocator accessor.
  \return the allocator used by the debug Manager.
*/
rw::IResourceAllocator *Manager::GetAllocator()
{
    rwFUNCTION();
    rwRETURN(sAllocator);
}


/**
  \brief Assert handler.

  This function gets called when an assert fails. It is usually called by the
  rwASSERTM macro but can be called directly when necessary. It does not
  need the actual debug Manager singleton instance. This means asserts can be
  triggered any time, even from static constructors that get called before
  main() is executed.

  \param condition the condition that triggered the assert.
  \param file the source file where the assert was triggered.
  \param function the function where the assert was triggered.
  \param line the line number where the assert was triggered.
  \param format a user supplied message that provides more information about the assert, in \c printf() format.
  \param args a va_list containing optional parameters for the format.
  \param flags 0 or a combination of the AssertFlags bit-flags.
  \return true: system must abort, false: assert was ignored.
*/
bool Manager::AssertFailed(const char *condition, const char *file,
                           const char *function, unsigned int line,
                           const char *format, va_list args, unsigned int flags)
{
    rwFUNCTION();

    if(sInstance != NULL &&                    // Check if the debug manager not created yet
       sInstance->mAssertHandler != NULL &&    // and the assert handler not set.
       sInstance->mSystemChannel != NULL &&    // In rare cases, asserts that occur during
       sInstance->mSystemDevice != NULL)       // destruction of the system, these may be null.
    {
        const bool shouldAbort = sInstance->mAssertHandler->AssertFailed(
            condition, file, function, line, format, args, flags);

        sMutex.Enter();

        for(unsigned int i=0; i<MAXCHANNELS; i++)
        {
            if(sInstance->mChannels[i] != NULL)
                sInstance->mChannels[i]->Flush();
        }

        sMutex.Leave();

        rwRETURN(shouldAbort);
    }

    // no instance or no assert handler, use HAL debugging functions
    static const size_t BUFSIZE=2048;
    char msgBuf[BUFSIZE];

    msgBuf[BUFSIZE-1] = '\0';
    RwSnprintf(msgBuf, BUFSIZE - 1, ASSERTFORMAT, file, line, condition, function);
    host::PrintDebugString(msgBuf);
    if(format != NULL)
    {
        RwVsnprintf(msgBuf, BUFSIZE - 1, format, args);
        host::PrintDebugString(msgBuf);
    }

    rwRETURN((flags & ASSERTFLAG_FATAL) != 0);
}


/**
  \brief Sets the assert handler
  \param assertHandler Pointer to a custom IAssertHandler interface
*/
void Manager::SetAssertHandler(IAssertHandler *assertHandler)
{
    rwFUNCTION();
    if (mAssertHandlerOwner)
    {
        delete mAssertHandler;
        mAssertHandler = NULL;
        mAssertHandlerOwner = false;
    }
    mAssertHandler = assertHandler;
    rwRETURNVOID();
}


/**
  \brief System Abort function.

  This function can always be called to abort the system after printing
  a message to the debug system channel. If the debug Manager singleton
  has not been created yet, it will use the Hal to print and abort.

  \param message the message to print before aborting.
  \sa HaltSystem
*/
void Manager::Abort(const char *message)
{
    rwFUNCTION();
    // If the actual singleton instance exists, use the system
    // channel. Else use the low-level Hal functions that
    // are always available.
    if(sInstance != NULL)
    {
        sInstance->GetSystemChannel()->Print(message);
        sInstance->HaltSystem();
    }
    else
    {
        host::PrintDebugString(message);
        host::BreakToDebugger();
    }
    rwRETURNVOID();
}

///////////////////////////////////////////////////////////////////////////
//
// new / delete operators
//

/**
  \brief debug Manager new operator.
  \param size size of block to allocate
  \param name name of block to allocator
  \param flags flags to pass to allocation
*/
void *Manager::operator new(size_t size)
{
    rwFUNCTION();
    rwASSERTM(sizeof(Manager) == size,
        ("new called with incorrect size"));
    rwRETURN(sAllocator->Allocate(
        ResourceDescriptor(size, 4),     // magic number 4 is used instead of sizeof(uint32_t) to avoid 64bit safety warnings
        RW_RESOURCE_ALLOCATION_DESC("rw::core::debug::Manager")).GetMemoryResource());
}

/**
  \brief debug Manager delete operator
  \param p block to delete
*/
void Manager::operator delete(void *p)
{
    rwFUNCTION();
    sAllocator->Free(p);
    rwRETURNVOID();
}

///////////////////////////////////////////////////////////////////////////
//
// constructor / destructor
//


/**
  \brief debug Manager constructor.
*/
Manager::Manager() :
    mDisabledChannelCount(0),
    mSystemDevice(NULL),
    mSystemChannel(NULL),
    mAssertHandler(NULL),
    mAssertHandlerOwner(false),
    mSystemFormatter(NULL)
{
    rwFUNCTION();
    for(unsigned int i=0; i<MAXCHANNELS; ++i)
    {
        mChannels[i] = NULL;
        mDisabledChannelList[i][0] = '\0';
    }
    rwRETURNVOID();
}


/**
  \brief debug manager destructor.
*/
Manager::~Manager()
{
    rwFUNCTION();
    delete mSystemChannel;
    mSystemChannel = NULL;
    delete mSystemDevice;
    mSystemDevice = NULL;
    delete mSystemFormatter;
    mSystemFormatter = NULL;
    if (mAssertHandlerOwner)
    {
        delete mAssertHandler;
        mAssertHandlerOwner = false;
    }
    mAssertHandler = NULL;
    rwRETURNVOID();
}

///////////////////////////////////////////////////////////////
//
// accessors
//


/**
  \brief Set the default print options.
  \param opts a PrintOpts structure containing the options to set.
  \sa PrintOpts.
*/
void Manager::SetDefaultPrintOpts(const PrintOpts &opts)
{
    rwFUNCTION();
    sMutex.Enter();

    sPrintOpts = opts;

    sMutex.Leave();
    rwRETURNVOID();
}

/**
  \brief Get the default Print options.
  \return a PrintOpts structure containing the default print options.
  \sa PrintOpts.
*/
PrintOpts Manager::GetDefaultPrintOpts()
{
    rwFUNCTION();
    rwRETURN(sPrintOpts);
}

///////////////////////////////////////////////////////////////
//
// other public functions
//


/**
  \brief Halt the system.

  This function flushes all the channels and halts the system. This is the
  default function to call when the system has to be aborted. If the debug Manager
  singleton instance has not been created yet, the Abort function can be
  called.
  \sa Abort.
*/
void Manager::HaltSystem() const
{
    rwFUNCTION();
    sMutex.Enter();

    for(unsigned int i=0; i<MAXCHANNELS; i++)
    {
        if(mChannels[i] != NULL)
            mChannels[i]->Flush();
    }

    host::BreakToDebugger();

    sMutex.Leave();
    rwRETURNVOID();
}

/**
  \brief Register a new channel with the debug Manager.

  If the channel has already been registered, it will not be registered again.

  \param channel the Channel to register.
  \return true: channel successfully registered, true: registration failed.
  \note this function gets called automatically by the default Channel constructor.
*/
bool Manager::RegisterChannel(Channel *channel)
{
    rwFUNCTION();
    sMutex.Enter();

    // first see if the channel has already been registered
    unsigned int i;
    for(i=0; i<MAXCHANNELS; ++i)
    {
        if(mChannels[i] == channel)
        {
            sMutex.Leave();
            rwRETURN(false);
        }
    }

    // look for the first free slot and assign it
    for(i=0; i<MAXCHANNELS; ++i)
    {
        if(mChannels[i] == 0)
        {
            mChannels[i] = channel;

            // now see if this channel is in the list of channels
            // to be disabled.
            for(unsigned int j=0; j<mDisabledChannelCount; ++j)
            {
                if(strcmp(mDisabledChannelList[j], channel->GetName()) == 0)
                {
                    channel->Disable();
                    break;
                }
            }

            sMutex.Leave();
            rwRETURN(true);
        }
    }

    sMutex.Leave();

//    RWDEBUG_ABORT("out of channels.");

#ifdef EA_DEBUG
    rw::core::debug::Manager::AssertFailed("User Abort", __FILE__, RW_CURRENT_FUNCTION, __LINE__, "out of channels.", NULL, rw::core::debug::ASSERTFLAG_FATAL);
#endif

    rwRETURN(false);
}

/**
  \brief Delete a channel from the debug Manager

  This effectively un-registers a channel from the debug Manager.

  \param channel channel to delete.
  \return true: the channel has been successfully deleted, false: deletion failed.
  \note this function get called automatically by the default Channel destructor.
*/
bool Manager::DeleteChannel(const Channel *channel)
{
    rwFUNCTION();
    bool ret = false;
    sMutex.Enter();

    for(unsigned int i=0; i<MAXCHANNELS; ++i)
    {
        if(mChannels[i] == channel)
        {
            mChannels[i] = NULL;
            ret = true;
            break;
        }
    }

    sMutex.Leave();

#ifdef EA_DEBUG
    if (!ret)
    {
        rw::core::debug::Manager::AssertFailed("User Abort",
            __FILE__, RW_CURRENT_FUNCTION, __LINE__,
            "channel not found.", NULL,
            rw::core::debug::ASSERTFLAG_FATAL);
    }
#endif

    rwRETURN(ret);
}

/**
  \brief Disable a channel by name.

  This function will disable a channel given it's name. If the channel
  does not exist, the name will be stored in a list and when channels are
  registered, it will check the list of names of channels that have been
  disabled. If it exists in the list, the channel will be disabled. The idea
  behind this mechanism is that during init time a config file containing names
  of channels to be disabled will be read. The user can now specify which channels
  to disable to reduce debug output clutter.

  \param name name of the channel to disable.
*/
void Manager::DisableChannel(const char *name)
{
    rwFUNCTION();
    sMutex.Enter();

    // see if this is an existing channel
    Channel *channel = GetChannel(name);
    if(channel != NULL)
        channel->Disable();
    else
    {
        // not found, add to the list of channels to disable
        // when they are added
        rwASSERTM(mDisabledChannelCount < MAXCHANNELS,
            ("too many channels."));

        bool found = false;
        for(unsigned int i=0; (i<mDisabledChannelCount) && (!found); ++i)
        {
            if(strcmp(name, mDisabledChannelList[i]) == 0)
                found = true;
        }

        if(!found)
            strncpy(mDisabledChannelList[mDisabledChannelCount++], name, MAXNAMELEN);
    }

    sMutex.Leave();
    rwRETURNVOID();
}

/**
  \brief Get a channel based on the channel name.
  \param name name of the channel to find.
  \return the channel who's name matches the given name or NULL if not found.
*/
Channel *Manager::GetChannel(const char *name) const
{
    rwFUNCTION();
    sMutex.Enter();

    for(unsigned int i=0; i<MAXCHANNELS; ++i)
    {
        if((mChannels[i] != NULL) && (strcmp(mChannels[i]->GetName(), name) == 0))
        {
            sMutex.Leave();
            rwRETURN(mChannels[i]);
        }
    }

    sMutex.Leave();

    rwRETURN(NULL);
}

///////////////////////////////////////////////////////////////////////////
//
// private functions
//

/**
  \brief Initialise the debug Manager.

  These are initialisation functions that need to be called \e after the
  constructor has been called and the instance has been created.
*/
void Manager::Init()
{
    rwFUNCTION();
    mAssertHandler = new AssertHandler;
    mAssertHandlerOwner = true;
    mSystemFormatter = new Formatter;
    mSystemDevice = new ConsoleDevice(0, Device::FLAG_THREADSAFE);
    mSystemChannel = new Channel(SYSTEMCHANNEL, mSystemDevice);
    rwRETURNVOID();
}

void *IAssertHandler::operator new(size_t size)
{
    rwFUNCTION();
    rwRETURN(Manager::GetAllocator()->Allocate(
        ResourceDescriptor(size, 4),     // magic number 4 is used instead of sizeof(uint32_t) to avoid 64bit safety warnings
        RW_RESOURCE_ALLOCATION_DESC("rw::core::debug::IAssertHandler")).GetMemoryResource());
}

void IAssertHandler::operator delete(void *p)
{
    rwFUNCTION();
    Manager::GetAllocator()->Free(p);
    rwRETURNVOID();
}

#endif // EA_DEBUG

void *IFormatter::operator new(size_t size)
{
    rwFUNCTION();
    rwRETURN(Manager::GetAllocator()->Allocate(
        ResourceDescriptor(size, 4),     // magic number 4 is used instead of sizeof(uint32_t) to avoid 64bit safety warnings
        RW_RESOURCE_ALLOCATION_DESC("rw::core::debug::IFormatter")).GetMemoryResource());
}

void IFormatter::operator delete(void *p)
{
    rwFUNCTION();
    Manager::GetAllocator()->Free(p);
    rwRETURNVOID();
}


}   // namespace debug
}   // namespace core
}   // namespace rw
