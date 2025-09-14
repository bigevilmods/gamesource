// (c) Electronic Arts. All Rights Reserved.

#ifndef RW_CORE_CODEC_CONFIG_H
#define RW_CORE_CODEC_CONFIG_H

#if defined(EA_DLL) && defined(EA_COMPILER_MSVC)
#   if !defined(RWCODEC_API)
#       define RWCODEC_API __declspec(dllimport)
#   endif
#else
#   define RWCODEC_API
#endif

#endif
