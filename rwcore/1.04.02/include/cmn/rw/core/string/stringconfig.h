// (c) Electronic Arts. All Rights Reserved.

/// \file
/// \deprecated Use EASTL's string class instead.
///
/// String configuration options

#ifndef RW_CORE_STRING_STRINGCONFIG_H
#define RW_CORE_STRING_STRINGCONFIG_H

#define STRING_MEMCOPY          EA::StdC::Memcpy
#define STRING_MEMMOVE          EA::StdC::Memmove
#define STRING_MEMFILL          EA::StdC::Memset8
#define STRING_MEMCOMPARE       EA::StdC::Memcmp
#define STRING_STRINGLENGTH     EA::StdC::Strlen
#define STRING_VSNPRINTF        EA::StdC::Vsnprintf
#define STRING_TOUPPER          EA::StdC::Strupr
#define STRING_TOLOWER          EA::StdC::Strlwr


#endif // RW_CORE_STRING_STRING_H
