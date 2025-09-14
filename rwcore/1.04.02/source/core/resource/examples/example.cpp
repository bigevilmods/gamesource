// (c) Electronic Arts. All Rights Reserved.
/*************************************************************************************************************

File: example.cpp

Purpose: Demonstrate writing your own allocator from IResourceAllocator and using it in the standard pattern
with some standard Rw object.

*/

// ***********************************************************************************************************
// Includes
#include <cstdio>


#include <rw/core/bittable/bittable.h>

using namespace rw;

// ***********************************************************************************************************
// Typedefs

// ***********************************************************************************************************
// Defines + Enums + Consts

// ***********************************************************************************************************
// Structs + Unions + Classes

namespace
{
    class MyAllocator : public IResourceAllocator
    {
    public:
        MyAllocator()
        {
            rwFUNCTION();
            rwRETURNVOID();
        }

        // reset the cached values stored by MyAllocator
        void Reset()
        {
            rwFUNCTION();
            rwRETURNVOID();
        }

    protected:
        // Implementation routines used by the IResourceAllocator Allocate and
        // Free methods
        Resource AllocateImplementation(const ResourceDescriptor &resourceDescriptor, const char8_t *RWUNUSED(name))
        {
            rwFUNCTION();
            rwRETURN(Resource(RwMallocAligned(resourceDescriptor.GetSize(), resourceDescriptor.GetAlignment())));
        }

        void FreeImplementation(const Resource &resource)
        {
            rwFUNCTION();
            RwFreeAligned(resource.GetMemoryResource());
            rwRETURNVOID();
        }

    };

    // ***********************************************************************************************************
    // Static Variables + Static Data Member Definitions

    // ***********************************************************************************************************
    // Static Functions

} // unnamed namespace


// ***********************************************************************************************************
// External Functions

int main(int argc, char *argv[])
{
    rwFUNCTION();

    MyAllocator allocator;
    uint32_t numRows = 3, numCols = 3;
    printf("Allocating memory for BitTable of %d x %d\n", numRows, numCols);

    ResourceDescriptor rd = BitTable::GetResourceDescriptor(numRows, numCols);
    Resource resource = allocator.Allocate(rd, RW_DEFAULT_NAMESTR);

    printf("Initializing allocated BitTable\n");
    BitTable *pTable = BitTable::Initialize(resource, numRows, numCols);

    printf("Freeing memory\n");
    allocator.Free(resource);

    printf("Done\n");

    rwRETURN(0);
}

// ***********************************************************************************************************
// Class Member Functions

