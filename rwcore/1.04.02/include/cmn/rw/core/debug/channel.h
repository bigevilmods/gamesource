// (c) Electronic Arts. All Rights Reserved.

/// \file
/// Channel class definition

#ifndef RW_CORE_DEBUG_CHANNEL_H
#define RW_CORE_DEBUG_CHANNEL_H
#include <rw/core/base/ostypes.h>
#include <rw/core/debug/manager.h>

#if defined(_MSC_VER)
#pragma once
#endif

#include <stdarg.h>

#include "detail/debugcriticalsection.h"

namespace rw
{
    namespace core
    {
        namespace debug
        {
            class Device;


            /**
              \brief debug Channel class.

              This class contains the debug Channel definition. each debug message is sent to a channel, each channel has
              one more more devices where the information gets sent to. The constructor takes a pointer to a device as one
              of it's parameters, this is the default device for this channel. Additional devices can be added and removed
              using the \e  AddDevice() and \e  RemoveDevice() member functions.

              Please note: instances of this class are NOT THREADSAFE by default.  If you need a thread-safe channel, pass
              rw::core::debug::Channel::FLAG_THREADSAFE to the constructor when the object is created.
            */
            class Channel
            {
            public:
                /**
                  \enum MAXDEVICES maximum number of devices that a channel can be associated with.
                */
                enum
                {
                    MAXDEVICES = 16
                };

                /**
                  \enum FLAG_THREADSAFE threadsafe flag for use with debug::Channel object
                */
                enum
                {
                    FLAG_THREADSAFE = 0x1
                };

                static void *operator new(size_t);
                static void operator delete(void *);

                Channel(const char *name, Device *device = NULL, unsigned int flags = 0);
                ~Channel();

                const char *GetName() const;
                const char *GetFileName() const;
                unsigned int GetLine() const;
                void SetInfo(const char *fileName, unsigned int line);
                bool HaveInfo() const;

                const PrintOpts &GetPrintOpts() const;
                void SetPrintOpts(const PrintOpts &opts);

                void Enable();
                void Disable();
                bool IsEnabled() const;

                bool AddDevice(Device *device);
                bool RemoveDevice(const Device *device);

                //lint -e{1916}
                void Print(const char *format, ...);
                void Vprint(const char *format, va_list args);

                void OutputData(const void *data, unsigned int dataLen);

                void Flush();

                void EnterCritical()
                { mCriticalSection.Enter(); }

                void LeaveCritical()
                { mCriticalSection.Leave(); }

                Device *GetFirstDevice();
                Device *GetNextDevice();

            private:
                unsigned int mDeviceCount;          // number of devices attached to this channel
                unsigned int mIndex;                // iterator index for GetFirstDevice() / GetNextDevice()
                bool mEnabled;                      // is this channel enabled or not
                bool mHaveInfo;                     // has SetInfo() been called
                char mName[debug::MAXNAMELEN];      // name of this channel

                Device *mDeviceList[MAXDEVICES];    // list of devices associated with this channel

                const char *mFileName;              // file name used by RWDEBUG_PRINT macro (see SetInfo function)
                unsigned int mLine;                 // ditto for line number

                detail::DebugCriticalSection mCriticalSection; // Critical Section for this channel

                // last data member due to odd size
                PrintOpts mPrintOpts;           // print options for this channel

                // Prevent default generation of these functions by not defining them
                Channel(const Channel& rhs);            // copy constructor
                Channel& operator=(const Channel& rhs); // assignment operator
            };
        } // namespace debug
    } // namespace core
} // namespace rw

#endif // RW_CORE_DEBUG_CHANNEL_H
