// (c) 2004-2005 Criterion Software Limited and its licensors. All Rights Reserved
/**
\file
Implements rw::core::controller::DeviceInfo.
*/

#include "rw/core/controller/deviceinfo.h"
#include "rw/core/controller/devicestate.h"
#include "rw/core/controller/manager.h"

#include "EAStdC/EAMemory.h"

#include "EAAssert/eaassert.h"

namespace rw {
namespace core {
namespace controller {

DeviceInfo::ButtonCapability DeviceInfo::GetButtonCapability(unsigned int ButtonIndex) const
{
    EA_ASSERT_MSG(ButtonIndex < mNumberButtons, "ButtonIndex too large");
    EA_ASSERT_MSG(mButtonCapabilities != NULL, "mButtonCapabilities not allocated");

    return mButtonCapabilities[ButtonIndex];
}

void DeviceInfo::SetButtonCapability(unsigned int ButtonIndex, ButtonCapability newCapability)
{
    EA_ASSERT_MSG(ButtonIndex < mNumberButtons, "ButtonIndex too large");
    EA_ASSERT_MSG(mButtonCapabilities != NULL, "mButtonCapabilities not allocated");

    mButtonCapabilities[ButtonIndex] = newCapability;
}

DeviceInfo::DeviceInfo(unsigned int id)
: mDeviceID(id),
  mDeviceType(UNKNOWN),
  mNumberButtons(0),
  mNumberAxis(0),
  mButtonCapabilities(NULL)
#if defined(EA_PLATFORM_PS3)
  , vendorID(0)
  , productID(0)
#endif
{
#if defined(EA_PLATFORM_WINDOWS)
	memset(tszInstanceName,0,sizeof(tszInstanceName));
	memset(tszProductName,0,sizeof(tszProductName));	
#endif
}

DeviceInfo::~DeviceInfo()
{
    if (mButtonCapabilities != NULL)
    {
        EA::Allocator::ICoreAllocator *pAllocator = Manager::GetAllocator();
        pAllocator->Free(mButtonCapabilities);
        mButtonCapabilities = NULL;
    }
}

// Function to reset the type of device and the number of buttons/axis. This will
// free the old memory and allocate new memory for the button capabilities
void DeviceInfo::Reset(Type deviceType, unsigned int numButtons, unsigned int numAxis)
{
    mDeviceType = deviceType;

    // do not release the buffers unless this is a new device with a different
    // number of buttons and axes from the one that was previously connected.
    if (mDeviceType == UNKNOWN)
        return;

    // if the number of buttons changed, realloc the button capabilities array
    if (mNumberButtons != numButtons)
    {
        EA::Allocator::ICoreAllocator *pAllocator = Manager::GetAllocator();

        // free the old capabilities if needed
        if (mButtonCapabilities != NULL)
        {
            pAllocator->Free(mButtonCapabilities);
            mButtonCapabilities = NULL;
        }

        if (numButtons > 0)
        {
            mButtonCapabilities = static_cast<ButtonCapability*>(
                pAllocator->Alloc(sizeof(*mButtonCapabilities)*numButtons, "RwController Button Capabilities", EA::Allocator::MEM_PERM));
            EA::StdC::Memclear(mButtonCapabilities, sizeof(*mButtonCapabilities)*numButtons);
        }
    }

    mNumberButtons = numButtons;
    mNumberAxis = numAxis;
}

void *DeviceInfo::operator new(size_t size)
{
    return Manager::GetAllocator()->Alloc(size, "rw::core::controller::DeviceInfo", EA::Allocator::MEM_PERM);
}

void DeviceInfo::operator delete(void *p)
{
    Manager::GetAllocator()->Free(p);
}

}
}
}
