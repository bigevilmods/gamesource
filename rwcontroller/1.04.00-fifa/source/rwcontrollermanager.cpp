// (c) 2004-2005 Criterion Software Limited and its licensors. All Rights Reserved
/**
\file
Implements rw::core::controller::Manager.
*/
#include "rw/core/controller/deviceinfo.h"
#include "rw/core/controller/devicestate.h"
#include "rw/core/controller/manager.h"

#include "EAAssert/eaassert.h"

// Include the Manager implementation class declarations
#if defined(EA_PLATFORM_WINDOWS)
#include "pc/private_rwllmanager.h"
#elif defined(EA_PLATFORM_XBOXONE)
#include "capilano/private_rwllmanager.h"
#elif defined(EA_PLATFORM_PS4)
#include "kettle/private_rwllmanager.h"
#endif

namespace rw {
namespace core {
namespace controller {

Manager *Manager::sInstance = NULL;
EA::Allocator::ICoreAllocator *Manager::sAllocator = NULL;
bool Manager::mEnableDeadZoneFilter = true;

void Manager::CreateInstance(const Options &options)
{
    EA_ASSERT_MSG(sInstance == NULL, "instance already exists.");

    sAllocator = options.mAllocator;
    sInstance = new Manager(options);
}

Manager* Manager::GetInstance()
{
    EA_ASSERT_MSG(sInstance != NULL, "instance does not exist, call Manager::CreateInstance() first.");

    return sInstance;
}

void Manager::DestroyInstance()
{
    EA_ASSERT_MSG(sInstance != NULL, "instance does not exist, call Manager::CreateInstance() first.");

    delete sInstance;
    sInstance = NULL;
    sAllocator = NULL;
}

void Manager::Update()
{
    pImpl->Update();
}

const DeviceInfo* Manager::GetDeviceInfo(unsigned int deviceNumber) const
{
    return pImpl->GetDeviceInfo(deviceNumber);
}

const DeviceState* Manager::GetDeviceState(unsigned int deviceNumber) const
{
    return pImpl->GetDeviceState(deviceNumber);
}

DeviceEffect* Manager::GetDeviceEffect(unsigned int deviceNumber) const
{
    return pImpl->GetDeviceEffect(deviceNumber);
}

unsigned int Manager::GetNumControllers() const
{
    return pImpl->GetNumControllers();
}

#if defined(EA_PLATFORM_PS3)
uint32_t Manager::TMPGetButtonData() const
{
    return pImpl->TMPGetButtonData();
}

bool Manager::GetInterceptState() const
{
    return pImpl->GetInterceptState();
}

uint32_t Manager::TMPGetCapabilityData(unsigned int deviceNumber) const
{
	return pImpl->TMPGetCapabilityData(deviceNumber);
}

uint32_t Manager::TMPGetClassTypeData(unsigned int deviceNumber) const
{
	return pImpl->TMPGetClassTypeData(deviceNumber);
}

bool Manager::TMPGetPadPeriphInfo(CellPadPeriphInfo& cellPadPeriphInfo) const
{
	return pImpl->TMPGetPadPeriphInfo(cellPadPeriphInfo);
}
#elif defined(EA_PLATFORM_PS4)
const EA::Controller::IEAController* Manager::GetController(int index)
{
	return pImpl->GetController(index);
}
#elif defined(EA_PLATFORM_XBOXONE)
uint64 Manager::GetGamePadID(int index)
{
	return pImpl->GetGamePadID(index);
}

void Manager::SetNullPads(bool enable)
{
    pImpl->SetNullPads(enable);
}

#endif

Manager::Manager(const Options &options)
{
    pImpl = new LLManager(options);
}

Manager::~Manager()
{
    delete pImpl;
}

void *Manager::operator new(size_t size)
{
    return sAllocator->Alloc(size, "rw::core::controller::Manager", EA::Allocator::MEM_PERM);
}

void Manager::operator delete(void *ptr)
{
    sAllocator->Free(ptr);
}

Options::Options(EA::Allocator::ICoreAllocator *coreAllocator) 
: mAllocator(coreAllocator)
#if defined EA_PLATFORM_WINDOWS
  , mUseXInput(false)
  , mUseForeground(true)
  , mWnd(0)
#endif
{}

Options::Options(rw::core::debug::Channel*, EA::Allocator::ICoreAllocator *coreAllocator) 
: mAllocator(coreAllocator)
#if defined EA_PLATFORM_WINDOWS
  , mUseXInput(false)
  , mUseForeground(true)
  , mWnd(0)
#endif
{}

#if defined(EA_PLATFORM_XBOXONE)
void Manager::SimulateButtonPush(unsigned int deviceNumber, const int* buttonIds, unsigned int numButtons, unsigned int numFrames)
{
	pImpl->SimulateButtonPush(deviceNumber, buttonIds, numButtons, numFrames);
}
#endif

}}}
