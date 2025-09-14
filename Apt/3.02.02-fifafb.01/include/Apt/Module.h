// FIFA_BEGIN | SENDBACK | mwebb | 2015 - 05 - 26 | Adding Frostbite support
// (c) Electronic Arts.  All Rights Reserved.

#pragma once

#if defined(FB_APT_EXPORTS)
	#define FB_APT_API __declspec(dllexport)
#elif !defined(FB_STATIC_LIBS)
	#define FB_APT_API __declspec(dllimport)
#else
	#define FB_APT_API
#endif



#define FB_apt_API FB_APT_API



