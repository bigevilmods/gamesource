// (c) Electronic Arts. All Rights Reserved.

/// \file
/// \deprecated.  Use EASTL's String class intead of this one.
/// Implementation of C++ string stream allocator for wide character streams

#include "rwsharedheaders.hpp"

#include <EAStdC/EAString.h>

#include "rw/core/string/wstringconfig.h"
#include "rw/core/string/wstring.h"

using namespace rw::core;

#define SIZEOFDATAHEADER sizeof(short)*2
#define SIZEOFREFDATAHEADER sizeof(short)*3

/**
constructor
\note Default string allocator class, dynamicly allocates all memory from the core allocator
 supplied in the constructor. There is 4 bytes of overhead allocated at the start of the
 string buffer to cache the capacity and length of the string
*/
WStringCoreAllocator::WStringCoreAllocator(rw::IResourceAllocator *allocator)
: mCoreAllocator(allocator)
{
    rwFUNCTION();
    rwRETURNVOID();
}
/**
Does nothing
*/
void WStringCoreAllocator::Modify(char16_t**)
{
    rwFUNCTION();
    rwRETURNVOID();
}
/**
\brief allocates memory for a string of n characters
\param n number of characters
\return a pointer to the allocated memory
*/
char16_t* WStringCoreAllocator::Allocate(size_t n)
{
    rwFUNCTION();
    Data *pData = static_cast<Data*>(mCoreAllocator->Allocate(
        ResourceDescriptor((n+SIZEOFDATAHEADER)*2, sizeof(uint32_t)),
        RW_DEFAULT_NAMESTR).GetMemoryResource()); // Two bytes per char16_t type
    pData->mCapacity = static_cast<uint16_t>(n);
    pData->mLength = 0;
    rwRETURN(pData->mBuffer);
}
/**
\brief deallocates the memory

*/
void WStringCoreAllocator::Deallocate(char16_t* p)
{
    rwFUNCTION();
    if (p != WString::mEmpty)
    {
        Data *pData = reinterpret_cast<Data*> ((char *)p-SIZEOFDATAHEADER);
        mCoreAllocator->Free(pData);
    }
    rwRETURNVOID();
}
/**
Returns STRING_TYPE_ALLOCATED
*/
uint32_t WStringCoreAllocator::GetAllocatorType() const
{
    rwFUNCTION();
    rwRETURN(WString::STRING_TYPE_ALLOCATED);
}
/**
\brief assigns one String to another
*/
void WStringCoreAllocator::Assign(WString& s1, char16_t** , const WString& s2, char16_t* p2)
{
    rwFUNCTION();
    s1.Assign(p2, p2 + s2.GetLength());
    rwRETURNVOID();
}
/**
It always returns false.
*/
int32_t WStringCoreAllocator::Compare(const char16_t* , const char16_t* )
{
    rwFUNCTION();
    rwRETURN(false);
}
/**
Sets the length of the Char8 type String
*/
void WStringCoreAllocator::SetLength(char16_t *p, size_t n)
{
    rwFUNCTION();
    Data *pData = reinterpret_cast<Data*> ((char *)p-SIZEOFDATAHEADER);
    pData->mLength = static_cast<uint16_t>(n);
    rwRETURNVOID();
}
/**
Returns the length of the char16 type string
*/
size_t WStringCoreAllocator::GetLength(const char16_t *p) const
{
    rwFUNCTION();
    if (p == WString::mEmpty)
    {
        rwRETURN(0);
    }

    const Data *pData = reinterpret_cast<const Data*> ((const char *)p-SIZEOFDATAHEADER);
    rwRETURN(pData->mLength);
}
/**
Returns the Capacity of the string
*/
size_t WStringCoreAllocator::GetCapacity(const char16_t *p) const
{
    rwFUNCTION();
    if (p == WString::mEmpty)
    {
        rwRETURN(0);
    }

    const Data *pData = reinterpret_cast<const Data*> ((const char *)p-SIZEOFDATAHEADER);
    rwRETURN(static_cast<size_t>(pData->mCapacity-1));
}
/**
constructor
\note Ref-counted string allocator class, dynamicly allocates all memory from the core
 allocator supplied in the constructor. There is 6 bytes of overhead allocated at
 the start of the string buffer to cache the capacity and length of the string and to
 store the current reference count.
*/
WStringRefAllocator::WStringRefAllocator(rw::IResourceAllocator *allocator)
: mCoreAllocator(allocator)
{
    rwFUNCTION();
    rwRETURNVOID();
}

/**
If there are multiple strings referencing to this data, Modify function makes a copy of the string
*/
void WStringRefAllocator::Modify(char16_t** p)
{
    rwFUNCTION();
    if (*p != WString::mEmpty)
    {
        Data *pData = reinterpret_cast<Data*> ((char *)(*p)-SIZEOFREFDATAHEADER);
        //Data *pData = reinterpret_cast<Data*> (*p-SIZEOFREFDATAHEADER);
        // if we have multiple strings referencing this data, make a copy
        if (pData->mRefCount > 1)
        {
            Data *pNewData = static_cast<Data*>(mCoreAllocator->Allocate(ResourceDescriptor(
                static_cast<size_t>(pData->mCapacity+SIZEOFREFDATAHEADER)*2, sizeof(uint32_t)),
                RW_DEFAULT_NAMESTR).GetMemoryResource()); //Two bytes per char16_t type
            STRING_MEMCOPY(pNewData, pData, static_cast<unsigned int>(pData->mCapacity + SIZEOFREFDATAHEADER)*2);
            pNewData->mRefCount = 1;
            pData->mRefCount--;
            *p = pNewData->mBuffer;
        }
    }
    rwRETURNVOID();
}
/**
Allocates memeory for a string of n characters
*/
char16_t* WStringRefAllocator::Allocate(size_t n)
{
    rwFUNCTION();
    Data *pData = static_cast<Data*>(mCoreAllocator->Allocate(
        ResourceDescriptor((n+SIZEOFREFDATAHEADER)*2, sizeof(uint32_t)),
        RW_DEFAULT_NAMESTR).GetMemoryResource()); //Two bytes per char16_t type
    pData->mRefCount = 1;
    pData->mCapacity = static_cast<uint16_t>(n);
    pData->mLength = 0;
    rwRETURN(pData->mBuffer);
}
/**
Deallocates memory
*/
void WStringRefAllocator::Deallocate(char16_t* p)
{
    rwFUNCTION();
    if (p != WString::mEmpty)
    {
        Data *pData = reinterpret_cast<Data*> ((char *)p-SIZEOFREFDATAHEADER);
        pData->mRefCount--;
        if (pData->mRefCount == 0)
        {
            mCoreAllocator->Free(pData);
        }
    }
    rwRETURNVOID();
}
/**
Returns STRING_TYPE_REFCOUNT
*/
uint32_t WStringRefAllocator::GetAllocatorType() const
{
    rwFUNCTION();
    rwRETURN(WString::STRING_TYPE_REFCOUNT);
}
/**
Assigns one String to another
*/
void WStringRefAllocator::Assign(WString& , char16_t** p1, const WString& , char16_t* p2)
{
    rwFUNCTION();
    Deallocate(*p1);
    Data *pData = reinterpret_cast<Data*> ((char *)p2-SIZEOFREFDATAHEADER);
    pData->mRefCount++;
    *p1 = p2;
    rwRETURNVOID();
}
/**
Always returns false
*/
int32_t WStringRefAllocator::Compare(const char16_t* , const char16_t* )
{
    rwFUNCTION();
    rwRETURN(false);
}
/**
Sets the length of the string
*/
void WStringRefAllocator::SetLength(char16_t *p, size_t n)
{
    rwFUNCTION();
    Data *pData = reinterpret_cast<Data*> ((char *)p-SIZEOFREFDATAHEADER);
    pData->mLength = static_cast<uint16_t>(n);
    rwRETURNVOID();
}
/**
Gets the lengths of the string
*/
size_t WStringRefAllocator::GetLength(const char16_t *p) const
{
    rwFUNCTION();
    if (p == WString::mEmpty)
    {
        rwRETURN(0);
    }

    const Data *pData = reinterpret_cast<const Data*> ((const char *)p-SIZEOFREFDATAHEADER);
    rwRETURN(pData->mLength);
}
/**
Returns the capacity of the string
*/
size_t WStringRefAllocator::GetCapacity(const char16_t *p) const
{
    rwFUNCTION();
    if (p == WString::mEmpty)
    {
        rwRETURN(0);
    }

    const Data *pData = reinterpret_cast<const Data*> ((const char *)p-SIZEOFREFDATAHEADER);
    rwRETURN(pData->mCapacity);
}
/**
constructor
\note Const string allocator class, this is intended to be used for read-only strings
 the buffer to use is passed in the constructor, any calls to the
 Modify/Assign/Allocate/Deallocate functions will assert, GetLength and GetCapacity will
 return the string length
*/
WStringConstAllocator::WStringConstAllocator()
{
    rwFUNCTION();
    rwRETURNVOID();
}
/**
Does nothing as the string is constant
*/
void WStringConstAllocator::Modify(char16_t** )
{
    rwFUNCTION();
    // not allowed
    //RWDEBUG_ABORT("Can't modify const string");
    rwRETURNVOID();
}
/**
Does nothing as the string is constant
*/
char16_t* WStringConstAllocator::Allocate(size_t )
{
    rwFUNCTION();
    // not allowed
    //RWDEBUG_ABORT("Can't alloc const string");

    //lint -e{527} unreachable code
    rwRETURN(NULL);
}
/**
Deallocate the memory.
Does nothing now.
*/
void WStringConstAllocator::Deallocate(char16_t* )
{
    rwFUNCTION();
    // we will allow the deallocate as it will be called when the string is freed
    rwRETURNVOID();
}
/**
Returns STRING_TYPE_CONST
*/
uint32_t WStringConstAllocator::GetAllocatorType() const
{
    rwFUNCTION();
    rwRETURN(WString::STRING_TYPE_CONST);
}
/**
Does nothing as the string is constant
*/
void WStringConstAllocator::Assign(WString& , char16_t** , const WString& , char16_t* )
{
    rwFUNCTION();
    // not allowed
    //RWDEBUG_ABORT("Can't alloc const string");
    rwRETURNVOID();
}
/**
Always returns false
*/
int32_t WStringConstAllocator::Compare(const char16_t* , const char16_t* )
{
    rwFUNCTION();
    rwRETURN(false);
}
/**
Does nothing as the string is constant
*/
void WStringConstAllocator::SetLength(char16_t*, size_t)
{
    rwFUNCTION();
    // don't do anything
    rwRETURNVOID();
}
/**
Returns string length
*/
size_t WStringConstAllocator::GetLength(const char16_t* p) const
{
    rwFUNCTION();
    rwRETURN(EA::StdC::Strlen(p));
}
/**
Returns string capacity
*/
size_t WStringConstAllocator::GetCapacity(const char16_t *p) const
{
    rwFUNCTION();
    rwRETURN(EA::StdC::Strlen(p));

}
/**
constructor
\note  Fixed string allocator class, this is a base class for the WStringSizedAllocator template
 that will create an in-place buffer to use for the string. The Allocate/Deallocate
 functions will assert and the GetCapacity needs to be implemented by the template.
*/
WStringFixedAllocator::WStringFixedAllocator()
{
    rwFUNCTION();
    rwRETURNVOID();
}
/**
Does nothing as String is fixed
*/
void WStringFixedAllocator::Modify(char16_t**)
{
    rwFUNCTION();
    rwRETURNVOID();
}
/**
Does nothing as String is fixed
*/
char16_t* WStringFixedAllocator::Allocate(size_t)
{
    rwFUNCTION();
    // not allowed
    //RWDEBUG_ABORT("Can't alloc fixed string");

    //lint -e{527} unreachable code
    rwRETURN(NULL);
}
/**
Deallocate the memory.
Does nothing now.
*/
void WStringFixedAllocator::Deallocate(char16_t* )
{
    rwFUNCTION();
    // we will allow the deallocate as it will be called when the string is freed
    rwRETURNVOID();
}
/**
Returns STRING_TYPE_FIXED
*/
uint32_t WStringFixedAllocator::GetAllocatorType() const
{
    rwFUNCTION();
    rwRETURN(WString::STRING_TYPE_FIXED);
}
/**
Assigns one string to another
*/
void WStringFixedAllocator::Assign(WString& s1, char16_t** , const WString& s2, char16_t* p2)
{
    rwFUNCTION();
    s1.Assign(p2, p2 + s2.GetLength());
    rwRETURNVOID();
}
/**
Always returns false
*/
int32_t WStringFixedAllocator::Compare(const char16_t* , const char16_t* )
{
    rwFUNCTION();
    rwRETURN(false);
}
/**
Does nothing as String is fixed
*/
void WStringFixedAllocator::SetLength(char16_t*, size_t)
{
    rwFUNCTION();
    // don't do anything
    rwRETURNVOID();
}
/**
Returns the string length
*/
size_t WStringFixedAllocator::GetLength(const char16_t *p) const
{
    rwFUNCTION();
    rwRETURN(EA::StdC::Strlen(p));

}
/**
Returns the string capacity
*/
size_t WStringFixedAllocator::GetCapacity(const char16_t *p) const
{
    rwFUNCTION();
    rwRETURN(EA::StdC::Strlen(p));
}
