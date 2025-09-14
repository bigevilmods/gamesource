/*H************************************************************************************************/
/*!

    \File    Apt\_AptInternalDefines.h

    \Description
        Defines internal Apt macros that affect how Apt gets built.

    \Notes
        None.

    \Copyright
        (c) 2008 Electronic Arts Inc.

    \Version    1.0        04/01/08 (RB)  First Version
    \Version    1.1        08/14/08 ($senzee) Added APT_USE_UTILITY define for the AptUtil functions
    \Version    1.2        08/19/08 ($senzee) Added SNPRINTF macro from AptActionScriptProfiler.cpp to
                                    support call stacks in the assert.
    \Version    1.3        02/03/09 (RB)  APT_USE_UTILITY defined by default
    \Version    1.4        04/17/09 (RB)  Added APT_DEBUGGER_ENABLE (undefined by default)
    \Version    1.5        07/07/09 (RB)  Added memory metrics macros and the APT_MALLOC-family macros.
                                          Added APT_RI_NEW_DELETE_OPERATORS, APT_RU and APT_ARRAYSIZE.
    \Version    1.6        09/22/09 (MZ)  Added APT_ALTERNATE_INPUT macro.
    \Version    1.7        09/23/09 (MZ)  Added APT_USE_FILTERS macro.
    \Version    1.8        11/02/09 (MZ)  Added APT_TRACK_IMPLEMENTED_INTERFACES to enable/disable interface tracking for AptObjects.
*/
/************************************************************************************************H*/


#ifndef _Apt__AptInternalDefines_h
#define _Apt__AptInternalDefines_h

#if defined(_MSC_VER)
#pragma once
#endif

/*** Include files ********************************************************************************/

#include <EABase/eabase.h>
#include "AptDefine.h"

// These intrinsics are required for the APT_PREFETCH macro
#if defined(EA_PLATFORM_PS3)
    #include <ppu_intrinsics.h>
#elif defined(EA_PLATFORM_XENON)
    #include <xtl.h>
#endif

//-----------------------------------------------------------------------------------------------
//
// Internal macros. To change the default definitions of these macros, edit this file or define
// them via your compiler's command line.
//
//-----------------------------------------------------------------------------------------------

// 2004/23/04 DC -- Added #define for the array size. This way if it needs to expand in the future
//                  the game team can define APT_MAX_FILES_LOADED=xxx with the new desired max
//                  value. Bug #465
//                  Note that this could theoretically be a dynamic value, but then we would end up
//                  allocating it a: from the heap and b: each frame rendered. not very efficient...
//                  This is just used to set the size of an array on the (C++) stack. See Apt.cpp,
//                  _AptInternalRender and _AptInternalUpdate.
#if !APT_MAX_FILES_LOADED
#   define APT_MAX_FILES_LOADED 128
#endif

//-----------------------------------------------------------------------------------------------
//
// The following macros can be defined by the Apt.build file, if you are using nant to build.
//
//-----------------------------------------------------------------------------------------------

// The macros below are #defined by default, and their default value is 1.
// If you want to disable the associated feature, edit this file and
// #define the value as 0 instead. Or, #define the macro as 0 on the compiler
// command line. Do NOT just undefine the macro.

#if !defined(APT_USE_UTILITY)
#   define APT_USE_UTILITY                          1   // Compiles in support for the AptUtil class
#endif

#if !defined(APT_USE_FILTERS)
    #define     APT_USE_FILTERS                     1   // Compiles in support for the flash.filters.* classes
#endif

#if defined(APT_USE_FILTERS)
	#define APT_FILTER_ALIGNED(variable_type, variable) \
		EA_ALIGNED(variable_type, variable, 16)			// We need to align the in-memory buffers for the filters to be stricter than the default
#endif													// alignment so that it works properly on all platforms

#if !defined(APT_POOL_STRINGS)
    #define     APT_POOL_STRINGS                    1   // This keeps deallocated strings to re-allocate to new callers (saves CPU, costs mem)
#endif

#if !defined(APT_POOL_INTEGERS)
    #define     APT_POOL_INTEGERS                   1   // This keeps deallocated ints to re-allocate to new callers (saves CPU, costs mem)
#endif

#if !defined(APT_POOL_FLOATS)
    #define     APT_POOL_FLOATS                     1   // This keeps deallocated floats to re-allocate to new callers (saves CPU, costs mem)
#endif

#if !defined(APT_USE_MATH_OBJECT)
    #define     APT_USE_MATH_OBJECT                 1   // Compiles in support for Math class
#endif

#if !defined(APT_USE_SCRIPTCOLOUR_OBJECT)
    #define     APT_USE_SCRIPTCOLOUR_OBJECT         1   // Compiles in support for ScriptColour class
#endif

#if !defined(APT_USE_LOADVARS_OBJECT)
    #define     APT_USE_LOADVARS_OBJECT             1   // Compiles in support for LoadVars class
#endif

    // Added - for 0.19.00 version ($fr719)
    // Extended the changes made above, but only removed the methods, not the actual classes themselves.
    // The objects for these types below can still be created and used, just the methods are compiled out.
    // If you don't use the methods associated with these objects, comment out the #define below so it will not be compiled into the library.
#if !defined(APT_USE_ARRAY_METHODS)
    #define     APT_USE_ARRAY_METHODS               1   // Compiles in Array methods such as splice, join, sort, etc.
#endif

#if !defined(APT_USE_STRING_METHODS)
    #define     APT_USE_STRING_METHODS              1   // Compiles in String methods such as charAt, substr, toUpperCase, etc.
#endif

#if !defined(APT_TRACK_IMPLEMENTED_INTERFACES)
    #define     APT_TRACK_IMPLEMENTED_INTERFACES    1   // Support using interfaces with ActionScript's instanceof operator and casting.
#endif                                                  // (Has no effect on using regular classes with instanceof or casting.)

// The macros below are NOT #defined by default.
// If you want to enable the associated feature, edit this file and
// #define the value.  (To be consistent with the macros above,
// if you #define these macros then you should set the value as 1.)

#if !defined(APT_PRINT_INPUT)
    //#define   APT_PRINT_INPUT                     1
#endif

#if !defined(APT_GATHER_MOVIECLIP_METRICS)
    //#define   APT_GATHER_MOVIECLIP_METRICS        1
#endif

#if !defined(APT_RECORD_SCOPE_INFO)
//#   define APT_RECORD_SCOPE_INFO                  1
#endif

#if !defined(APT_EQUALITY_TYPECHECK_ASSERT)
//#   define APT_EQUALITY_TYPECHECK_ASSERT          1
#endif

#if !defined(APT_DEBUGGER_ENABLE)
//#   define APT_DEBUGGER_ENABLE                    1
#endif

#if !defined(APT_ALTERNATE_INPUT)
    //#define     APT_ALTERNATE_INPUT               1   // This flag enables code for alternate input
#endif

#if !defined(APT_GATHER_MEMORY_METRICS)
    //#define APT_GATHER_MEMORY_METRICS             1   /* Gathers detailed metrics on Apt memory usage. */
#endif

#if !defined(APT_ENABLE_ZOMBIE_OUTPUT)
    #if !defined(EA_FINAL)
        #define APT_ENABLE_ZOMBIE_OUTPUT            1   // Enables zombie output to the TTY window
    #endif
#endif

//
// This macro is #defined for debug builds, and undefined otherwise
//

#if !defined(EA_FINAL)
	#define APT_ENABLE_METRICS						1
	#define APT_DO_AS_PROFILE						1
#endif

// Some teams may wish to send RenderList allocations to a separate place.  This can be set in this
// header, or else defined via build flags (package.Apt.UseRenderListAllocators)
#if !defined(APT_USE_RENDERLIST_ALLOCATORS)
   #define APT_USE_RENDERLIST_ALLOCATORS            0
#endif

// Some teams need to adjust the way internal Apt RenderBuffer allocations work, and route them
// instead through a Temporary allocation system.  I don't think all teams need/want this, so I've
// made it easy for them to define.  They can either modify this file directly, or defined via
// build flags (package.Apt.UseTemporaryAllocators)
#if !defined(APT_USE_TEMPORARY_ALLOCATORS)
    #define APT_USE_TEMPORARY_ALLOCATORS            0
#endif


//------------------------------------------------------------------------------
// APT_DEBUGPRINT macro

extern void AptDebuggerPrint(int32_t nMsgLvl, const char *szFormat, ...);

#if (APT_CURR_DEBUG_MSG_LVL >= APT_DEBUG_MSG_ASSERT_LVL)
#define APT_DEBUGPRINT                  AptDebuggerPrint
#else

#if defined(EA_DEBUG) || defined(APT_ENABLE_ZOMBIE_OUTPUT)
#define APT_DEBUGPRINT              AptDebuggerPrint
#else
                                    // This weird-looking code makes it build in ps3 opt
#define APT_DEBUGPRINT              true ? (void)0 : AptDebuggerPrint
#endif

#endif

// If we have zombie output enabled then we want to see the zombie xml outputs
#if defined(APT_ENABLE_ZOMBIE_OUTPUT)
#define APT_XML_MEMORY_DUMP_SUPPORTED 1
#endif

//-----------------------------------------------------------------------------------------------
//
// These internal macros should NOT be edited, nor defined on the compiler command line.
//
//-----------------------------------------------------------------------------------------------

inline void noop() {}

#define APT_THREADHELPER_LOCK(_)    //(_).Lock()
#define APT_THREADHELPER_UNLOCK(_)  //(_).Unlock()

#define APT_ASSERT_RENDER_THREAD      noop


#define APT_UNREF(_) ((void)_)


#define NATIVE_MEMBER_FUNCTION_DISPATCH(_name_) \
    if (!psMethod_##_name_) { \
    psMethod_##_name_ = new AptNativeFunction(sMethod_##_name_); \
    psMethod_##_name_->setGCRoot(1); \
    _APT_INC(psMethod_##_name_); \
    } \
    return psMethod_##_name_;

#define NATIVE_MEMBER_FUNCTION_DESTROY(_name_) \
    if (psMethod_##_name_) { _APT_DEC(psMethod_##_name_); \
    psMethod_##_name_ = 0; }


//------------------------------------------------------------------------------
// DC - Added Macros to make reference registration easier to modify in the future.
#define APT_REFERENCES_REGISTERED(pVar)       (false) /* Here for custom debugging use. Not used by default. */

// These flags are used by the callbacks to indecate which base type the pointer pVar is
#define APT_REFREG_IS_APTVALUE      0x00000000
#define APT_REFREG_IS_APTCIH        0x00000001
#define APT_REFREG_IS_DISPLAYLIST   0x00000002

#define APT_REGISTER_REFERENCE_SAFE(pVar, sName, nFlageroo)    \
if(pVar)                                            \
{                                                   \
    AptValue::sReferenceRegistrationCb(this, (AptValue *&)pVar, sName, nFlageroo);\
}

#define APT_REGISTER_REFERENCE(pVar, sName, nFlageroo)         \
{                                                   \
    AptValue::sReferenceRegistrationCb(this, (AptValue *&)pVar, sName, nFlageroo);\
}

#define APT_REGISTER_REFERENCE_ANONYMOUS(pVar, sName, nFlageroo)     \
{                                                         \
    AptValue::sReferenceRegistrationCb(NULL, (AptValue *&)pVar, sName, nFlageroo);\
}

#define APT_REGISTER_REFERENCE_ANONYMOUS_SAFE(pVar, sName, nFlageroo)\
if(pVar)                                                  \
{                                                         \
    AptValue::sReferenceRegistrationCb(NULL, (AptValue *&)pVar, sName, nFlageroo);\
}

#define APT_REGISTER_REFERENCE_FROM(pFrom, pVar, sName, nFlageroo)     \
{                                                           \
    AptValue::sReferenceRegistrationCb(pFrom, (AptValue *&)pVar, sName, nFlageroo); \
}

#define APT_REGISTER_REFERENCE_FROM_SAFE(pFrom, pVar, sName, nFlageroo)\
if(pVar)                                                    \
{                                                           \
    AptValue::sReferenceRegistrationCb(pFrom, (AptValue *&)pVar, sName, nFlageroo); \
}

#if !defined(SNPRINTF) && ((defined(EA_PLATFORM_SONY) && defined(EA_PLATFORM_CONSOLE)) || defined(EA_PLATFORM_PS3)) // EA_PLATFORM_SONY not defined on PS3 until EABase 2.00.29
#define SNPRINTF snprintf
#else
#define SNPRINTF _snprintf
#endif

// Used for floating-point comparison in AptMath methods
#define APT_EPSILON (0.001f)

// These defines should not be used directly, they are here for the allocation macros only.
#if defined (APT_GATHER_MEMORY_METRICS)
    //! This must be done in a single source file for each class in order to init the metrics variables.
    #define APT_DEFINE_OBJECT_METRICS(className, parentClass) AptMemoryMetrics className::s_MemoryMetrics(#className, &(parentClass::s_MemoryMetrics))
    //! This must be done in a single source file for each class in order to init the metrics variables.
    #define APT_DEFINE_OBJECT_METRICS_EXPLICIT(className, parentTracker) AptMemoryMetrics className::s_MemoryMetrics(#className, parentTracker)
    //! This must be done in a single source file for each class in order to init the metrics variables.
    #define APT_DEFINE_CUSTOM_METRICS(MetricName, parentTracker)   AptMemoryMetrics MetricName( #MetricName, parentTracker)

#else
    #define APT_DEFINE_OBJECT_METRICS(className, parentTracker)
    #define APT_DEFINE_OBJECT_METRICS_EXPLICIT(className, parentTracker)
    #define APT_DEFINE_CUSTOM_METRICS(className, parentTracker)
#endif // #if defined (APT_GATHER_MEMORY_METRICS)

//
// Wrappers for malloc and free which use Dogma. The way these work is affected
// by the macros APT_GATHER_MEMORY_METRICS, APT_ALLOCATION_TRACKING, and
// APT_VERIFY_NON_GC_ALLOC_SIZES:
//
// - If APT_GATHER_MEMORY_METRICS is defined, then each APT_MALLOC_... call will
//   keep track of memory metrics--bytes allocated per class or struct--via the
//   ON_MEM_ALLOCATED mechanism.

// - If APT_ALLOCATION_TRACKING is defined, then we track the file and line
//   number where each allocation happened.

// - If APT_VERIFY_NON_GC_ALLOC_SIZES is defined (which is mutually-exclusive
//   with respect to APT_ALLOCATION_TRACKING) then we record the size of each
//   allocation in a small header at the front of each allocation.
//
#if defined(APT_ALLOCATION_TRACKING)

    #define APT_MALLOC_ARRAY(      type, count) ((type *)(ON_MEM_ALLOCATED(   AptAnonymousAllocations, sizeof(type) * (count)) gpNonGCPoolManager->TrackedAllocate(sizeof(type) * (count), __FILE__, __LINE__)))
    #define APT_FREE_ARRAY(   ptr, type, count)          (ON_MEM_DEALLOCATED( AptAnonymousAllocations, sizeof(type) * (count)) gpNonGCPoolManager->TrackedDeallocate( (void*)ptr, sizeof(type)*count ))
    #define APT_MALLOC_BLOCK(      bytes)                (ON_MEM_ALLOCATED(   AptAnonymousAllocations, bytes ) gpNonGCPoolManager->TrackedAllocate(bytes, __FILE__, __LINE__))
    #define APT_FREE_BLOCK(   ptr, bytes)                (ON_MEM_DEALLOCATED( AptAnonymousAllocations, bytes ) gpNonGCPoolManager->TrackedDeallocate( (void*)ptr, bytes ))
#if APT_USE_RENDERLIST_ALLOCATORS
    #define APT_MALLOC_RENDERLIST( bytes)                (ON_MEM_ALLOCATED(   AptAnonymousAllocations, bytes ) gAptFuncs.pfnRenderListAlloc(bytes, __FILE__, __LINE__))
    #define APT_FREE_RENDERLIST(   ptr,  bytes)          (ON_MEM_DEALLOCATED( AptAnonymousAllocations, bytes ) gAptFuncs.pfnRenderListFreeSize((void*)ptr, bytes))
#else
    #define APT_MALLOC_RENDERLIST( bytes)                (APT_MALLOC_BLOCK(   bytes))
    #define APT_FREE_RENDERLIST(   ptr,  bytes)          (APT_FREE_BLOCK(     ptr, bytes))
#endif
#if APT_USE_TEMPORARY_ALLOCATORS
    #define APT_MALLOC_TEMP(      bytes)                 (ON_MEM_ALLOCATED(   AptAnonymousAllocations, bytes ) gAptFuncs.pfnTempAlloc(bytes, __FILE__, __LINE__))
    #define APT_FREE_TEMP(   ptr, bytes)                 (ON_MEM_DEALLOCATED( AptAnonymousAllocations, bytes ) gAptFuncs.pfnTempFreeSize( (void*)ptr, bytes ))
#endif

    #define APT_MALLOC_ARRAY_TRACKER( tracker,     type, count) ((type *)(ON_MEM_ALLOCATED(   tracker, sizeof(type) * (count)) gpNonGCPoolManager->TrackedAllocate(sizeof(type) * (count), __FILE__, __LINE__)))
    #define APT_FREE_ARRAY_TRACKER(   tracker,ptr, type, count)          (ON_MEM_DEALLOCATED( tracker, sizeof(type) * (count)) gpNonGCPoolManager->TrackedDeallocate( (void*)ptr, sizeof(type)*count ))
    #define APT_MALLOC_BLOCK_TRACKER( tracker,     bytes)                (ON_MEM_ALLOCATED(   tracker, bytes ) gpNonGCPoolManager->TrackedAllocate(bytes, __FILE__, __LINE__))
    #define APT_FREE_BLOCK_TRACKER(   tracker,ptr, bytes)                (ON_MEM_DEALLOCATED( tracker, bytes ) gpNonGCPoolManager->TrackedDeallocate( (void*)ptr, bytes ))

#elif defined(APT_VERIFY_NON_GC_ALLOC_SIZES)

    #define APT_MALLOC_ARRAY(    type, count) ((type *)(ON_MEM_ALLOCATED(   AptAnonymousAllocations, sizeof(type) * (count)) gpNonGCPoolManager->SizeVerifyAllocate(sizeof(type) * (count))))
    #define APT_FREE_ARRAY( ptr, type, count)          (ON_MEM_DEALLOCATED( AptAnonymousAllocations, sizeof(type) * (count)) gpNonGCPoolManager->SizeVerifyDeallocate((void*) ptr, sizeof(type)*count ))
    #define APT_MALLOC_BLOCK(    bytes)                (ON_MEM_ALLOCATED(   AptAnonymousAllocations, bytes ) gpNonGCPoolManager->SizeVerifyAllocate( bytes ))
    #define APT_FREE_BLOCK( ptr, bytes)                (ON_MEM_DEALLOCATED( AptAnonymousAllocations, bytes ) gpNonGCPoolManager->SizeVerifyDeallocate( (void*)ptr, bytes ))
#if APT_USE_RENDERLIST_ALLOCATORS
    #define APT_MALLOC_RENDERLIST( bytes)              (ON_MEM_ALLOCATED(   AptAnonymousAllocations, bytes ) gAptFuncs.pfnRenderListAlloc(bytes))
    #define APT_FREE_RENDERLIST( ptr,  bytes)          (ON_MEM_DEALLOCATED( AptAnonymousAllocations, bytes ) gAptFuncs.pfnRenderListFreeSize((void*)ptr, bytes))
#else
    #define APT_MALLOC_RENDERLIST( bytes)              (APT_MALLOC_BLOCK(   bytes))
    #define APT_FREE_RENDERLIST( ptr,  bytes)          (APT_FREE_BLOCK  (   ptr, bytes))
#endif
#if APT_USE_TEMPORARY_ALLOCATORS
    #define APT_MALLOC_TEMP(      bytes)               (ON_MEM_ALLOCATED(   AptAnonymousAllocations, bytes ) gAptFuncs.pfnTempAlloc(bytes))
    #define APT_FREE_TEMP(   ptr, bytes)               (ON_MEM_DEALLOCATED( AptAnonymousAllocations, bytes ) gAptFuncs.pfnTempFreeSize( (void*)ptr, bytes ))
#endif

    #define APT_MALLOC_ARRAY_TRACKER( tracker,     type, count) ((type *)(ON_MEM_ALLOCATED(   tracker, sizeof(type) * (count)) gpNonGCPoolManager->SizeVerifyAllocate(sizeof(type) * (count))))
    #define APT_FREE_ARRAY_TRACKER(   tracker,ptr, type, count)          (ON_MEM_DEALLOCATED( tracker, sizeof(type) * (count)) gpNonGCPoolManager->SizeVerifyDeallocate((void*) ptr, sizeof(type)*count ))
    #define APT_MALLOC_BLOCK_TRACKER( tracker,     bytes)                (ON_MEM_ALLOCATED(   tracker, bytes ) gpNonGCPoolManager->SizeVerifyAllocate( bytes ))
    #define APT_FREE_BLOCK_TRACKER(   tracker,ptr, bytes)                (ON_MEM_DEALLOCATED( tracker, bytes ) gpNonGCPoolManager->SizeVerifyDeallocate( (void*)ptr, bytes ))

#else

    // we're not tracking allocations nor recording the size of each

    #define APT_MALLOC_ARRAY(    type, count) ((type *)(ON_MEM_ALLOCATED(   AptAnonymousAllocations, sizeof(type) * (count)) gpNonGCPoolManager->Allocate(sizeof(type) * (count))))
    #define APT_FREE_ARRAY( ptr, type, count)          (ON_MEM_DEALLOCATED( AptAnonymousAllocations, sizeof(type) * (count)) gpNonGCPoolManager->Deallocate((void*) ptr, sizeof(type)*count ))
    #define APT_MALLOC_BLOCK(    bytes)                (ON_MEM_ALLOCATED(   AptAnonymousAllocations, bytes ) gpNonGCPoolManager->Allocate( bytes ))
    #define APT_FREE_BLOCK( ptr, bytes)                (ON_MEM_DEALLOCATED( AptAnonymousAllocations, bytes ) gpNonGCPoolManager->Deallocate( (void*)ptr, bytes ))
#if APT_USE_RENDERLIST_ALLOCATORS
    #define APT_MALLOC_RENDERLIST( bytes)              (ON_MEM_ALLOCATED(   AptAnonymousAllocations, bytes ) gAptFuncs.pfnRenderListAlloc(bytes))
    #define APT_FREE_RENDERLIST( ptr,  bytes)          (ON_MEM_DEALLOCATED( AptAnonymousAllocations, bytes ) gAptFuncs.pfnRenderListFree((void*)ptr))
#else
    #define APT_MALLOC_RENDERLIST( bytes)              (APT_MALLOC_BLOCK(   bytes))
    #define APT_FREE_RENDERLIST( ptr,  bytes)          (APT_FREE_BLOCK(     ptr, bytes))
#endif
#if APT_USE_TEMPORARY_ALLOCATORS
    #define APT_MALLOC_TEMP(      bytes)               (ON_MEM_ALLOCATED(   AptAnonymousAllocations, bytes ) gAptFuncs.pfnTempAlloc(bytes))
    #define APT_FREE_TEMP(   ptr, bytes)               (ON_MEM_DEALLOCATED( AptAnonymousAllocations, bytes ) gAptFuncs.pfnTempFree( (void*)ptr ))
#endif

    #define APT_MALLOC_ARRAY_TRACKER( tracker,     type, count) ((type *)(ON_MEM_ALLOCATED(   tracker, sizeof(type) * (count)) gpNonGCPoolManager->Allocate(sizeof(type) * (count))))
    #define APT_FREE_ARRAY_TRACKER(   tracker,ptr, type, count)          (ON_MEM_DEALLOCATED( tracker, sizeof(type) * (count)) gpNonGCPoolManager->Deallocate((void*) ptr, sizeof(type)*count ))
    #define APT_MALLOC_BLOCK_TRACKER( tracker,     bytes)                (ON_MEM_ALLOCATED(   tracker, bytes ) gpNonGCPoolManager->Allocate( bytes ))
    #define APT_FREE_BLOCK_TRACKER(   tracker,ptr, bytes)                (ON_MEM_DEALLOCATED( tracker, bytes ) gpNonGCPoolManager->Deallocate( (void*)ptr, bytes ))

#endif // #if defined(APT_ALLOCATION_TRACKING)

//-----------------------------------------------------------------------------------------------
// This defaults to same allocator for now, but it is possible to give Non Garbage collected Apt
// Values a separate allocator in the future if desired.
#define APT_RI_NEW_DELETE_OPERATORS     APT_NEW_DELETE_OPERATORS

#define APT_ARRAYSIZE(_) ((int)(sizeof(_) / sizeof(_[0])))

#define APT_RU(_item_) if (bUnresolve) APT_UNRESOLVE(_item_) else APT_RESOLVE(_item_)

// For use with floating point operations
#define EPSILON                 0.0001f
#define FLOAT_EQUALS(a,b)       (fabsf(a - b) < EPSILON)
#define FLOAT_NOT_EQUALS(a,b)   (fabsf(a - b) >= EPSILON)

#if defined(EA_PLATFORM_XENON)
    #define APT_PREFETCH(_off, _base) __dcbt(_off, _base)
#elif defined(EA_PLATFORM_PS3)
    #if defined(__SNC__)
        #define APT_PREFETCH(_off, _base) __builtin_dcbt(_base, _off);
    #else
        #define APT_PREFETCH(_off, _base) __asm__ ("dcbt %0, %1" : /*no result*/ : "b%" (_off), "r" (_base) : "memory");
    #endif
#else
    #define APT_PREFETCH(_off, _base) /* do nothing */
#endif

#endif // _Apt__AptInternalDefines_h

