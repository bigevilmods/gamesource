// (c) 2004-2005 Criterion Software Limited and its licensors. All Rights Reserved
/**
\file
Declaration of rw::core::controller::IDeviceEffect class
\deprecated
*/

#ifndef RW_CORE_CONTROLLER_IDEVICEEFFECT_H
#define RW_CORE_CONTROLLER_IDEVICEEFFECT_H

#if defined _MSC_VER
#pragma once
#endif

#include "rw/core/controller/deviceeffect.h"

namespace rw
{
    namespace core
    {
        namespace controller
        {
            /// \brief IDeviceEffect has been superceded by DeviceEffect
            /// and is deprecated.  It is kept here for backwards-
            /// compatibility purposes, but may be removed in a future
            /// release.
            /// \deprecated
            typedef DeviceEffect IDeviceEffect;
        } // namespace controller
    } // namespace core
} // namespace rw

#endif    // RW_CORE_CONTROLLER_IDEVICEEFFECT_H
