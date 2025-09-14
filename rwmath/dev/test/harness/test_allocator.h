// (c) 2008 Criterion Software Limited and its licensors. All Rights Reserved

#ifndef TEST_ALLOCATOR_INCLUDED
#define TEST_ALLOCATOR_INCLUDED

#include "coreallocator/icoreallocator_interface.h"
#include "PPMalloc/EAGeneralAllocatorDebug.h"

class TestAllocator : public EA::Allocator::ICoreAllocator
{
public:

    TestAllocator();    
    void* Alloc(size_t size, const char *name, unsigned int flags, unsigned int align, unsigned int alignOffset);
    void* Alloc(size_t size, const char *name, unsigned int flags);
    
    void Free(void *block, size_t size = 0);
    void Setup();
    void Teardown();

private:
    unsigned int mNumAllocationsAllowedAfterTeardown;

    EA::Allocator::GeneralAllocatorDebug *mpHeap;
};

#endif
