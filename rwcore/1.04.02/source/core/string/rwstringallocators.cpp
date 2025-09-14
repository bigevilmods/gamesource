// (c) Electronic Arts. All Rights Reserved.

/// \file
/// \deprecated.  Use EASTL's String class intead of this one.
/// Implementation of C++ string stream allocator

#include "rwsharedheaders.hpp"
#include "rw/core/string/stringconfig.h"
#include "rw/core/string/string.h"


using namespace rw::core;
/**
constructor
\note Default string allocator class, dynamicly allocates all memory from the core allocator
 supplied in the constructor. There is 4 bytes of overhead allocated at the start of the
 string buffer to cache the capacity and length of the string
*/
StringCoreAllocator::StringCoreAllocator(rw::IResourceAllocator *allocator)
: mCoreAllocator(allocator)
{
    rwFUNCTION();
    rwRETURNVOID();
}
/**
Modify function does nothing
*/
void StringCoreAllocator::Modify(char8_t**)
{
    rwFUNCTION();
    rwRETURNVOID();
}
/**
\brief allocates memory for a string of n characters
\param n number of characters
\return a pointer to the allocated memory
*/
char8_t* StringCoreAllocator::Allocate(size_t n)
{
    rwFUNCTION();
    Data *pData = static_cast<Data*>(mCoreAllocator->Allocate(
        ResourceDescriptor(n+4, 4),   // magic number 4 is used instead of sizeof(uint32_t) to avoid 64bit safety warnings
        RW_DEFAULT_NAMESTR).GetMemoryResource());
    pData->mCapacity = static_cast<uint16_t>(n);
    pData->mLength = 0;
    rwRETURN(pData->mBuffer);
}
/**
\brief deallocates the memory

*/
void StringCoreAllocator::Deallocate(char8_t* p)
{
    rwFUNCTION();
    if (p != String::mEmpty)
    {
        Data *pData = reinterpret_cast<Data*>(p-4);
        mCoreAllocator->Free(pData);
    }
    rwRETURNVOID();
}
/**
Returns STRING_TYPE_ALLOCATED
*/
uint32_t StringCoreAllocator::GetAllocatorType() const
{
    rwFUNCTION();
    rwRETURN(String::STRING_TYPE_ALLOCATED);
}
/**
\brief assigns one String to another
*/
void StringCoreAllocator::Assign(String& s1, char8_t** , const String& s2, char8_t* p2)
{
    rwFUNCTION();
    s1.Assign(p2, p2 + s2.GetLength());
    rwRETURNVOID();
}
/**
It always returns false.
*/
int32_t StringCoreAllocator::Compare(const char8_t* , const char8_t* )
{
    rwFUNCTION();
    rwRETURN(false);
}
/**
Sets the length of the Char8 type String
*/
void StringCoreAllocator::SetLength(char8_t *p, size_t n)
{
    rwFUNCTION();
    Data *pData = reinterpret_cast<Data*> (p-4);
    pData->mLength = static_cast<uint16_t>(n);
    rwRETURNVOID();
}
/**
Returns the length of the char8 type string
*/
size_t StringCoreAllocator::GetLength(const char8_t *p) const
{
    rwFUNCTION();
    if (p == String::mEmpty)
    {
        rwRETURN(0);
    }

    const Data *pData = reinterpret_cast<const Data*> (p-4);
    rwRETURN(pData->mLength);
}
/**
Returns the Capacity of the string
*/
size_t StringCoreAllocator::GetCapacity(const char8_t *p) const
{
    rwFUNCTION();
    if (p == String::mEmpty)
    {
        rwRETURN(0);
    }

    const Data *pData = reinterpret_cast<const Data*> (p-4);
    rwRETURN(static_cast<size_t>(pData->mCapacity-1));
}
/**
Constructor
\note Ref-counted string allocator class, dynamicly allocates all memory from the core
 allocator supplied in the constructor. There is 6 bytes of overhead allocated at
 the start of the string buffer to cache the capacity and length of the string and to
 store the current reference count.
*/
StringRefAllocator::StringRefAllocator(rw::IResourceAllocator *allocator)
: mCoreAllocator(allocator)
{
    rwFUNCTION();
    rwRETURNVOID();
}
/**
If there are multiple strings referencing to this data, Modify function makes a copy of the string
*/
void StringRefAllocator::Modify(char8_t** p)
{
    rwFUNCTION();
    if (*p != String::mEmpty)
    {
        Data *pData = reinterpret_cast<Data*> (*p-6);
        // if we have multiple strings referencing this data, make a copy
        if (pData->mRefCount > 1)
        {
            Data *pNewData = static_cast<Data*>(mCoreAllocator->Allocate(ResourceDescriptor(
                static_cast<size_t>(pData->mCapacity+6), 4),    // magic number 4 is used instead of sizeof(uint32_t) to avoid 64bit safety warnings
                RW_DEFAULT_NAMESTR).GetMemoryResource());
            STRING_MEMCOPY(pNewData, pData, static_cast<unsigned int>(pData->mCapacity + 6));
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
char8_t* StringRefAllocator::Allocate(size_t n)
{
    rwFUNCTION();
    Data *pData = static_cast<Data*>(mCoreAllocator->Allocate(
        ResourceDescriptor(n+6, 4), // magic number 4 is used instead of sizeof(uint32_t) to avoid 64bit safety warnings
        RW_DEFAULT_NAMESTR).GetMemoryResource());
    pData->mRefCount = 1;
    pData->mCapacity = static_cast<uint16_t>(n);
    pData->mLength = 0;
    rwRETURN(pData->mBuffer);
}
/**
Deallocates memory
*/
void StringRefAllocator::Deallocate(char8_t* p)
{
    rwFUNCTION();
    if (p != String::mEmpty)
    {
        Data *pData = reinterpret_cast<Data*> (p-6);
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
uint32_t StringRefAllocator::GetAllocatorType() const
{
    rwFUNCTION();
    rwRETURN(String::STRING_TYPE_REFCOUNT);
}
/**
Assigns one String to another
*/
void StringRefAllocator::Assign(String& , char8_t** p1, const String& , char8_t* p2)
{
    rwFUNCTION();
    Deallocate(*p1);
    Data *pData = reinterpret_cast<Data*> (p2-6);
    pData->mRefCount++;
    *p1 = p2;
    rwRETURNVOID();
}
/**
Always returns false
*/
int32_t StringRefAllocator::Compare(const char8_t* , const char8_t* )
{
    rwFUNCTION();
    rwRETURN(false);
}
/**
Sets the length of the string
*/
void StringRefAllocator::SetLength(char8_t *p, size_t n)
{
    rwFUNCTION();
    Data *pData = reinterpret_cast<Data*> (p-6);
    pData->mLength = static_cast<uint16_t>(n);
    rwRETURNVOID();
}
/**
Gets the lengths of the string
*/
size_t StringRefAllocator::GetLength(const char8_t *p) const
{
    rwFUNCTION();
    if (p == String::mEmpty)
    {
        rwRETURN(0);
    }

    const Data *pData = reinterpret_cast<const Data*> (p-6);
    rwRETURN(pData->mLength);
}
/**
Returns the capacity of the string
*/
size_t StringRefAllocator::GetCapacity(const char8_t *p) const
{
    rwFUNCTION();
    if (p == String::mEmpty)
    {
        rwRETURN(0);
    }

    const Data *pData = reinterpret_cast<const Data*> (p-6);
    rwRETURN(pData->mCapacity);
}
/**
Constructor
\note Const string allocator class, this is intended to be used for read-only strings
 the buffer to use is passed in the constructor, any calls to the
 Modify/Assign/Allocate/Deallocate functions will assert, GetLength and GetCapacity will
 return the string length
 */
StringConstAllocator::StringConstAllocator()
{
    rwFUNCTION();
    rwRETURNVOID();
}
/**
Does nothing as the string is constant
*/
void StringConstAllocator::Modify(char8_t** )
{
    rwFUNCTION();
    // not allowed
    //RWDEBUG_ABORT("Can't modify const string");
    rwRETURNVOID();
}
/**
Does nothing as the string is constant
*/
char8_t* StringConstAllocator::Allocate(size_t )
{
    rwFUNCTION();
    // not allowed
    //RWDEBUG_ABORT("Can't alloc const string");

    //lint -e{527} unreachable code
    rwRETURN(NULL);
}
/**
Deallocate the memory
Does nothing now.
*/
void StringConstAllocator::Deallocate(char8_t* )
{
    rwFUNCTION();
    // we will allow the deallocate as it will be called when the string is freed
    rwRETURNVOID();
}
/**
Returns STRING_TYPE_CONST
*/
uint32_t StringConstAllocator::GetAllocatorType() const
{
    rwFUNCTION();
    rwRETURN(String::STRING_TYPE_CONST);
}
/**
Does nothing as the string is constant
*/
void StringConstAllocator::Assign(String& , char8_t** , const String& , char8_t* )
{
    rwFUNCTION();
    // not allowed
    //RWDEBUG_ABORT("Can't alloc const string");
    rwRETURNVOID();
}
/**
Always returns false
*/
int32_t StringConstAllocator::Compare(const char8_t* , const char8_t* )
{
    rwFUNCTION();
    rwRETURN(false);
}
/**
Does nothing as the string is constant
*/
void StringConstAllocator::SetLength(char8_t*, size_t)
{
    rwFUNCTION();
    // don't do anything
    rwRETURNVOID();
}
/**
Returns string length
*/
size_t StringConstAllocator::GetLength(const char8_t* p) const
{
    rwFUNCTION();
    rwRETURN(EA::StdC::Strlen(p));

}
/**
Returns string capacity
*/
size_t StringConstAllocator::GetCapacity(const char8_t *p) const
{
    rwFUNCTION();
    rwRETURN(EA::StdC::Strlen(p));
}
/**
constructor
\note Fixed string allocator class, this is a base class for the StringSizedAllocator template
 that will create an in-place buffer to use for the string. The Allocate/Deallocate
 functions will assert and the GetCapacity needs to be implemented by the template.
*/
StringFixedAllocator::StringFixedAllocator()
{
    rwFUNCTION();
    rwRETURNVOID();
}
/**
Does nothing as String is fixed
*/
void StringFixedAllocator::Modify(char8_t**)
{
    rwFUNCTION();
    rwRETURNVOID();
}
/**
Does nothing as String is fixed
*/
char8_t* StringFixedAllocator::Allocate(size_t)
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
void StringFixedAllocator::Deallocate(char8_t* )
{
    rwFUNCTION();
    // we will allow the deallocate as it will be called when the string is freed
    rwRETURNVOID();
}
/**
Returns STRING_TYPE_FIXED
*/
uint32_t StringFixedAllocator::GetAllocatorType() const
{
    rwFUNCTION();
    rwRETURN(String::STRING_TYPE_FIXED);
}
/**
Assigns one string to another
*/
void StringFixedAllocator::Assign(String& s1, char8_t** , const String& s2, char8_t* p2)
{
    rwFUNCTION();
    s1.Assign(p2, p2 + s2.GetLength());
    rwRETURNVOID();
}
/**
Always returns false
*/
int32_t StringFixedAllocator::Compare(const char8_t* , const char8_t* )
{
    rwFUNCTION();
    rwRETURN(false);
}
/**
Does nothing as String is fixed
*/
void StringFixedAllocator::SetLength(char8_t*, size_t)
{
    rwFUNCTION();
    // don't do anything
    rwRETURNVOID();
}
/**
Returns the string length
*/
size_t StringFixedAllocator::GetLength(const char8_t *p) const
{
    rwFUNCTION();
    rwRETURN(EA::StdC::Strlen(p));
}
/**
Returns the string capacity
*/
size_t StringFixedAllocator::GetCapacity(const char8_t *p) const
{
    rwFUNCTION();
    rwRETURN(EA::StdC::Strlen(p));
}
