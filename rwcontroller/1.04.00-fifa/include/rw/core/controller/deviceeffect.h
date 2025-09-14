// (c) 2004-2005 Criterion Software Limited and its licensors. All Rights Reserved
/**
\file
Declaration of rw::core::controller::DeviceEffect class
*/

#ifndef RW_CORE_CONTROLLER_DEVICEEFFECT_H
#define RW_CORE_CONTROLLER_DEVICEEFFECT_H

#if defined _MSC_VER
#pragma once
#endif

#if !defined RWCORELIB_API
 #if 0 // defined EA_DLL && !defined RWCROSS
  #define RWCORELIB_API __declspec(dllimport)
 #else
  #define RWCORELIB_API
 #endif
#endif

#include "EABase/eabase.h"

namespace rw
{
    namespace core
    {
        namespace controller
        {

            /**
            \brief Controller vibration is enabled and disabled through this class
            */
            class RWCORELIB_API DeviceEffect
            {
            public:
                /**
                 \brief Effect information.
                */
                struct Info
                {
                    /**
                    \brief Set of ranges of magnitudes.
                    */
                    enum Magnitudes
                    {
                        MIN_MAGNITUDE = 0,      ///< Minimum magnitude
                        MAX_MAGNITUDE = 65535   ///< Maximum magnitude
                    };

                    /**
                    \brief Set of flags to control the playing of effects.
                    */
                    enum PlayControl
                    {
                        STOP,               ///< Stop effect
                        START,              ///< Start effect
                        START_IF_PLAYING    ///< Start if playing
                    };

                    float MagnitudeLeft;      ///< The magnitude of the left motor if it exists
                    float MagnitudeRight;     ///< The magnitude of the right motor if it exists
                    PlayControl StartPlaying; ///< Whether the effect should start or stop playing
                };

                // constructor and destructor
                explicit DeviceEffect(unsigned int DeviceId);
                ~DeviceEffect();

                static void *operator new(size_t);
                static void operator delete(void *);

                /**
                \brief Returns whether there is an effect currently playing
                */
                bool IsPlaying() const { return mPlaying; }

                /**
                \brief Starts the effect
                \importlib rwcontroller
                */
                void Start();

                /**
                \brief Stops the effect
                \importlib rwcontroller
                */
                void Stop();

                /**
                \brief Sets new info for the effect. This will restart the effect
                with the new info settings if it was playing.
                \param pInfo the info needed to setup the effect
                \importlib rwcontroller
                */
                void SetInfo(const Info *pInfo);

            private:
                unsigned int    mDeviceID;
                bool            mPlaying;
                float           mMagnitudeLeft;
                float           mMagnitudeRight;
            };

        } // namespace controller
    } // namespace core
} // namespace rw

#endif    // RW_CORE_CONTROLLER_IDEVICEEFFECT_H
