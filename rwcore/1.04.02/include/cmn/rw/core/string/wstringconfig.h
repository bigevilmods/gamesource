// (c) Electronic Arts. All Rights Reserved.

/// \file
/// \deprecated Use EASTL's string class instead.
///
/// Wide String configuration options

#ifndef RW_CORE_STRING_WSTRINGCONFIG_H
#define RW_CORE_STRING_WSTRINGCONFIG_H


#define STRING_MEMCOPY           EA::StdC::Memcpy
#define STRING_MEMMOVE           EA::StdC::Memmove
#define WSTRING_MEMFILL(d,s,c)   EA::StdC::MemFill16(d,s,static_cast<unsigned int>(c))
#define STRING_MEMCOMPARE        EA::StdC::Memcmp
#define WSTRING_STRINGLENGTH     EA::StdC::Strlen
#define WSTRING_VSNPRINTF        EA::StdC::Vsnprintf
#define WSTRING_TOUPPER          EA::StdC::Strupr
#define WSTRING_TOLOWER          EA::StdC::Strlwr

#endif // RW_CORE_STRING_STRING_H
