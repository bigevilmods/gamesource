// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_CORE_FILESYS_CONFIG_H
#define RW_CORE_FILESYS_CONFIG_H

#include <EABase/eabase.h> //EA_WCHAR_UNIQUE
#include <EAStdC/internal/Config.h> //EASTDC_VERSION_N, EASTDC_API
#include <EAStdC/EAString.h> //EASTDC_UNICODE_CONST_CHAR_PTR_CAST, EASTDC_UNICODE_CHAR_CAST, EA_WCHAR_UNIQUE

#ifndef RWCORELIB_DLL
    #if defined(EA_DLL)
        #define RWCORELIB_DLL 1
    #else
        #define RWCORELIB_DLL 0
    #endif
#endif

// Added April 2014 for backwards compatibility with older versions of EAStdC
// Remove after April 2016
#if EASTDC_VERSION_N < 11700
    namespace EA
    {
        namespace StdC
        {
            inline EASTDC_API char8_t* Strnchr(const char8_t* pString, int c, size_t n)
            {
                while(n-- > 0)
                {
                    if(*pString == c)
                    {
                        return (char8_t*)pString;
                    }
                    if(*pString == '\0')
                    {
                        return NULL;
                    }
                    pString++;
                }
                return NULL;
            }

            inline EASTDC_API char16_t* Strnchr(const char16_t* pString, char16_t c, size_t n)
            {
                while(n-- > 0)
                {
                    if(*pString == c)
                    {
                        return (char16_t*)pString;
                    }
                    if(*pString == '\0')
                    {
                        return NULL;
                    }
                    pString++;
                }
                return NULL;
            }

            inline EASTDC_API char32_t* Strnchr(const char32_t* pString, char32_t c, size_t n)
            {
                while(n-- > 0)
                {
                    if(*pString == c)
                    {
                        return (char32_t*)pString;
                    }
                    if(*pString == '\0')
                    {
                        return NULL;
                    }
                    pString++;
                }
                return NULL;
            }
            
            #if defined(EA_WCHAR_UNIQUE) && EA_WCHAR_UNIQUE
                inline wchar_t* Strnchr(const wchar_t* pString, wchar_t c, size_t n)
                {
                    return reinterpret_cast<wchar_t *>(Strnchr(EASTDC_UNICODE_CONST_CHAR_PTR_CAST(pString), EASTDC_UNICODE_CHAR_CAST(c), n));
                }
            #endif
            
        }
    }
#endif
#ifndef EA_STRLCPY_VERIFY
    #if defined(EA_DEBUG)
        #define EA_STRLCPY_VERIFY(DestString, SourceString, DestCapacity) \
            EA_DISABLE_VC_WARNING(4127) \
            do { \
                size_t ret = EA::StdC::Strlcpy(DestString, SourceString, DestCapacity); \
                EA_UNUSED(ret); \
                EA_ASSERT(ret < DestCapacity); \
            } while(0) \
            EA_RESTORE_VC_WARNING()
    #else
        #define EA_STRLCPY_VERIFY(DestString, SourceString, DestCapacity) EA::StdC::Strlcpy(DestString, SourceString, DestCapacity)
    #endif
#endif

#ifndef EA_STRLCAT_VERIFY
    #if defined(EA_DEBUG)
        #define EA_STRLCAT_VERIFY(DestString, SourceString, DestCapacity) \
            EA_DISABLE_VC_WARNING(4127) \
            do { \
                size_t ret = EA::StdC::Strlcat(DestString, SourceString, DestCapacity); \
                EA_UNUSED(ret); \
                EA_ASSERT(ret < DestCapacity); \
            } while(0) \
            EA_RESTORE_VC_WARNING()
    #else
        #define EA_STRLCAT_VERIFY(DestString, SourceString, DestCapacity) EA::StdC::Strlcat(DestString, SourceString, DestCapacity)
    #endif
#endif

#ifndef RWCORELIB_API // If the build file hasn't already defined this to be dllexport...
    #if RWCORELIB_DLL 
        #if defined(_MSC_VER)
            #define RWCORELIB_API      __declspec(dllimport)
            #define RWCORELIB_LOCAL
        #elif defined(__CYGWIN__)
            #define RWCORELIB_API      __attribute__((dllimport))
            #define RWCORELIB_LOCAL
        #elif (defined(__GNUC__) && (__GNUC__ >= 4))
            #define RWCORELIB_API      __attribute__ ((visibility("default")))
            #define RWCORELIB_LOCAL    __attribute__ ((visibility("hidden")))
        #else
            #define RWCORELIB_API
            #define RWCORELIB_LOCAL
        #endif
    #else
        #define RWCORELIB_API
        #define RWCORELIB_LOCAL
    #endif
#endif

#endif
