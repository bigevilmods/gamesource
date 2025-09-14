/*H************************************************************************************************/
/*!

    \File
        AptDefine.h

    \Description
        This file defines all the public macros that are used inside core Apt and are also used in aptaux libraries
        to determine various behavours of Apt.

    \Notes
        None.

    \Copyright
        (c) 2005 Electronic Arts Inc.

    \Version
        1.0        03/07/04 (ONA) First Version
        1.1        04/07/06 Exposing the defines only if they are not defined in game config.
                            This helps avoid redefinition warnings. EATech#103274
        1.2        01/31/08 (RB) Fix for warning about unused params in APT_VALUE_GC_NEW_DELETE_OPERATORS.
        1.3        03/05/08 (RB) Split PS3 APT_INLINE macro into SN and non-SN sections for PS3 because "visibility" attribute doesn't exist for SNC.
        1.4        03/18/08 (RB) Made APT_INC and APT_DEC thread-safe. Defined private macros _APT_INC, etc.
        1.5        04/02/08 (RB) Moved some defines into _AptInternalDefines.h. Deleted
                                 APT_ASSERT_SIMULATION_THREAD and APT_SET_SIMULATION_THREAD.
        1.6        05/15/08 (ASK) Changed the conditions under which APT_ASSERT, APT_DEBUGPRINT macros are defined.
                                  Added new macro APT_ASSERTM to pass a message along with ASSERT
        1.7        07/29/08 (RB) Changed APT_ASSERTM so that the Msg param lets you pass non-literal strings
        1.8        08/19/08 ($senzee) Changed APT_ASSERT so that they call new AptAssert function which inserts
                                       the ActionScript callstack before calling the assert callback.
        1.9        03/06/09 (RB) Use APT_ASSERTM in APT_VALUE_GC_NEW_DELETE_OPERATORS instead of APT_ASSERT, to prevent VC compiler warning.
        2.0        04/15/09 (RB) Removed macros starting with APT_DEBUGGER_ from this file because they don't need to be public.
        2.1        07/07/09 (RB) Moved the following macros from this file into _AptInternalDefines.h since they don't need to be public:
                                 APT_DEFINE_OBJECT_METRICS, APT_DEFINE_OBJECT_METRICS_EXPLICIT, APT_DEFINE_CUSTOM_METRICS,
                                 APT_MALLOC_ARRAY, APT_FREE_ARRAY, APT_MALLOC_BLOCK, APT_FREE_BLOCK (and "tracked" version of macros).
                                 Also moved APT_RI_NEW_DELETE_OPERATORS, APT_RU and APT_ARRAYSIZE.
        2.2        09/22/09 (MZ) Removed APT_ALTERNATE_INPUT macro because it doesn't need to be public
        2.3        10/30/09 (RB) Moved the following macros from this file into _AptInternalDefines.h since they don't need to be public:
                                 APT_VALUE_NEW_DELETE_OPERATORS
                                 APT_VALUE_GC_NEW_DELETE_OPERATORS
*/
/************************************************************************************************H*/

#ifndef __APTDEFINE_H__
#define __APTDEFINE_H__

// This file uses some types from EABase, such as size_t and uint32_t
#include "EABase\eabase.h"
// FIFA_BEGIN | SENDBACK | mwebb | 2015 - 05 - 26 | Adding Frostbite support
#include "Module.h"
// FIFA_END

    // This is common place where all the different #defines in whole Apt code are defined.
    // you can either turn these on from the xml build files by defining them on command line passed to
    // compiler or you can just turn them on here.

#if !defined(APT_DO_PROFILE)
    //#define APT_DO_PROFILE                            1
#endif

#if !defined(APT_ALLOCATION_TRACKING)
    //#define   APT_ALLOCATION_TRACKING                 1
#endif
#if !defined(APT_XML_EXTENDED_MEMORY_DUMP_SUPPORTED)
    //#define   APT_XML_EXTENDED_MEMORY_DUMP_SUPPORTED  1   /* Provides a unique value to each allocated AptValue, this value should not be duplicated even across multiple dumps. */
#endif
#if !defined(APT_INC_DEC_MESSAGES)
    //#define   APT_INC_DEC_MESSAGES                    1
#endif
#if !defined(APT_ENABLE_DEBUG_REPORT)
    //#define   APT_ENABLE_DEBUG_REPORT                    1
#endif

// this will use the new class initialization order fix where parent class constructors for movie clip get
//called before child class constructors for movie clips get called
#if !defined(APT_USE_NEW_CLASS_INIT_ORDER)
    //#define APT_USE_NEW_CLASS_INIT_ORDER                1
#endif

//! AptCXForm uses two AptColorHelper objects to maintain the color transform.
//! By default, AptColorHelper represents colors as 4 bytes (ARGB) in the range of 0-255.
//! However, you can achieve much more accurate emulation of Flash color transformations by defining "APT_USE_FLASH_COLOR_RANGE".
//! By doing this, the color "scale" values range from 0.0 to 100.0, and the "translate" values range from -255.0 to 255.0,
//! just like Flash.  However, when APT_USE_FLASH_COLOR_RANGE is defined, the color components are represented by 4 floats,
//! so performance may differ from the default.
#if !defined(APT_USE_FLASH_COLOR_RANGE)
    #define APT_USE_FLASH_COLOR_RANGE 1
#endif

#if defined(EA_DEBUG)
    // Any of these can be defined in Release if you want, but are only on by default in debug builds.

    // Disabling this one to make our memory allocations more in line on both Debug and Opt
    //#if !defined(APT_VERIFY_NON_GC_ALLOC_SIZES)
    //    #define APT_VERIFY_NON_GC_ALLOC_SIZES             1  // Adds 4 bytes per NonGC allocation.
    //#endif
    #if !defined(DOGMA_EXTRA_MEMORY_COUNTERS)
        #define DOGMA_EXTRA_MEMORY_COUNTERS               1
    #endif
    #if !defined(APT_XML_MEMORY_DUMP_SUPPORTED)
        #define APT_XML_MEMORY_DUMP_SUPPORTED             1   // Allows XML based dumps of all Apt objects. Requires APT_USE_REGISTER_CALLBACKS.
    #endif                                                    // Object ID's are unique within a given dump, but may be duplicated in another dump. */
    #if !defined(ALLOW_PLAY_INPUTS)
        #define ALLOW_PLAY_INPUTS                           1  // allows to play inputs
    #endif
    #if !defined(ALLOW_SAVE_INPUTS)
        #define ALLOW_SAVE_INPUTS                           1  // allows to save inputs
    #endif
#endif

// the apt debug report is not super useful without extra memory counters, so make sure they are appropriately active
#if defined(APT_ENABLE_DEBUG_REPORT) && !defined(DOGMA_EXTRA_MEMORY_COUNTERS)
    #define DOGMA_EXTRA_MEMORY_COUNTERS               1    
#endif
        #define ALLOW_PLAY_INPUTS                           1  // allows to play inputs
        #define ALLOW_SAVE_INPUTS                           1  // allows to save inputs

#if defined(XBOX_FASTCAP)
    #define DOGMA_EXTRA_MEMORY_COUNTERS               1
#endif

// Mouse code in Apt is compiled out unless told no to.  PC uses the mouse by default.
#if defined (EA_PLATFORM_WINDOWS)
    #if !defined(APT_USE_MOUSE)
        #define   APT_USE_MOUSE         1
    #endif
#endif

#if defined (DO_COVERAGE)
    #define   APT_USE_BUTTONS           1
#endif


    // Added - for 0.18.03 version
    // these #defines decide whether code related to those actionscript objects will be compiled in or not.
    // Class definitions/constructors-destructors will still be there but all other functionality will be compiled out
    // in debug mode it will assert if #define is not defined and AS object is used.
#if !defined(APT_USE_DATE_OBJECT)
    //#define     APT_USE_DATE_OBJECT                 1
#endif
#if !defined(APT_USE_XML_OBJECT)
    #define     APT_USE_XML_OBJECT                  1
#endif
#if !defined(APT_USE_STAGE_OBJECT)
    //#define     APT_USE_STAGE_OBJECT                1
#endif
#if !defined(APT_USE_BUTTONS)
    #define     APT_USE_BUTTONS                     1
#endif
#if !defined(APT_USE_REPEATED_INPUT_OPTI)
    #define     APT_USE_REPEATED_INPUT_OPTI         1
#endif

    // Added - for 0.19.00 version ($fr719)
    // Extended the changes made above, allowing more features to be compiled out.
    // If you do not want the feature, comment out the #define below so it will not be compiled into the library.
#if !defined(APT_USE_SOUND_OBJECT)
    #define     APT_USE_SOUND_OBJECT                1   // this removes all support for Sound objects.
#endif
#if !defined(APT_3D)
    #define     APT_3D                              1   // Provides _z, _zscale, _xrotation, _yrotation properties for movieclips
#endif
#if !defined(APT_RENDER_FLAGS)
    //#define     APT_RENDER_FLAGS                    1   // provides _renderflags property for movieclip.
#endif
    // This define will turn on code that uses ZID to be passed to custom control instead of strings.
    // 3 new callbacks will be added because of this - pfnCreateCustomControlZid, pfnUpdateCustomControlZid and pfnDestroyCustomControlZid
    // pfnCreateCustomControlZid is called only ones for a customcontrol movieclip when it is firt time found as custom control
    // Aux lib should create required data structure for that custom control and should return
    // the void pointer to it. pfnDestroyCustomControlZid will be called from AptRenderItemcustomcontrol destructor.
    // pfnUpdateCustomControlZid will be called AptCIH::ProcessCustomControls
#if !defined(APT_CUSTOM_CONTROL_USE_ZID)
    #define     APT_CUSTOM_CONTROL_USE_ZID          1
#endif


//------------------------------------------------------------------------------
// DC - Added Debugging Macros to help standardize debug code.

#define APT_DEBUG_NONE     0 /* No Debugging */
#define APT_DEBUG_MINIMUM  1 /* Minimal Debugging Information */
#define APT_DEBUG_NORMAL   2 /* Normal Debugging Information  */
#define APT_DEBUG_CUSTOM   3 /* Used for Temporary / Custom Tests during development. Completed tasks should use other levels. */
#define APT_DEBUG_MAXIMUM  4 /* Internal Testing, Little or no concern for memory / time */

// APT_DEBUG_LEVEL - Defaults to Normal in a debug build, none, in a release build
#if !defined (APT_DEBUG_LEVEL)
    #if defined(EA_DEBUG)
        #define APT_DEBUG_LEVEL  (APT_DEBUG_NORMAL)
    #else
        #define APT_DEBUG_LEVEL  (APT_DEBUG_NONE)
    #endif
#endif


// Added new level APT_DEBUG_MSG_ASSERT_LVL that can be used to log ASSERTs in release mode.
#define APT_DEBUG_MSG_NONE_LVL      0
#define APT_DEBUG_MSG_ASSERT_LVL    1
#define APT_DEBUG_MSG_DEFAULT_LVL   2
#define APT_DEBUG_MSG_NORMAL_LVL    APT_DEBUG_MSG_DEFAULT_LVL
#define APT_DEBUG_MSG_WARNING_LVL   3
#define APT_DEBUG_MSG_DEBUG_LVL     4

// value of APT_CURR_DEBUG_MSG_LVL now decides in what conditions APT_ASSERT and APT_DEBUGPRINT are defined.
// Previously it used to be APT_DEBUG_LEVEL, but there is lot of extra code added for APT_DEBUG_LEVEL >= APT_DEBUG_NORMAL
// so it is hard to use in release mode to turn on ASSERTs.
#if !defined(APT_CURR_DEBUG_MSG_LVL)
#if defined(EA_DEBUG)
#define APT_CURR_DEBUG_MSG_LVL      APT_DEBUG_MSG_NORMAL_LVL
// FIFA_BEGIN | SENDBACK | mwebb | Mar 6 2015 | FIFA 16 change
//#elif defined(EA_FINAL)
//// In release/ship mode this should be set to APT_DEBUG_MSG_NONE_LVL so it will not generate any code for ASSERTs, APT_DEBUGPRINT
//// but to turn on asserts in release mode, set it to minimum of APT_DEBUG_MSG_ASSERT_LVL
//#define APT_CURR_DEBUG_MSG_LVL      APT_DEBUG_MSG_NONE_LVL
// FIFA_END
#else
// In release/ship mode this should be set to APT_DEBUG_MSG_NONE_LVL so it will not generate any code for ASSERTs, APT_DEBUGPRINT
// but to turn on asserts in release mode, set it to minimum of APT_DEBUG_MSG_ASSERT_LVL
#define APT_CURR_DEBUG_MSG_LVL      APT_DEBUG_MSG_NONE_LVL
#endif
#endif

//------------------------------------------------------------------------------
// APT_ASSERT macro

// FIFA_BEGIN | SENDBACK | mwebb | 2015 - 05 - 26 | Adding Frostbite support
//void AptAssert(const char *statement, const char *message, const char *file, int line);
FB_APT_API void AptAssert(const char *statement, const char *message, const char *file, int line);
// FIFA_END

#if (APT_CURR_DEBUG_MSG_LVL >= APT_DEBUG_MSG_ASSERT_LVL)

    #define APT_ASSERT(_Statement) {                                            \
        if (!(_Statement))                                                      \
        {                                                                       \
    AptAssert(#_Statement, NULL, __FILE__, __LINE__);                   \
}                                                                       \
}

#define APT_ASSERTM(_Statement, _Msg) {                                 \
    if (!(_Statement))                                                  \
{                                                                       \
    AptAssert(#_Statement, _Msg, __FILE__, __LINE__);                   \
        }                                                                       \
    }

#define APT_FAIL(_Msg) AptAssert(NULL, _Msg, __FILE__, __LINE__);


    #define NOT_REACHED                     0

#else

    #define APT_ASSERT(_Statement)          /* Nothing */
#define APT_ASSERTM(_Statement, _Msg)  /* Nothing */
#define APT_FAIL(_Statement)           /* Nothing */

#endif

//------------------------------------------------------------------------------
// Basic Typedefs
#ifndef TYPE_DEFINED
#define TYPE_DEFINED
typedef unsigned int    uint32_t;
typedef signed int      int32_t;
#endif

//------------------------------------------------------------------------------
// APT_INLINE
#if defined(EA_DEBUG) || !defined(APTLIBRARY)
    #undef APT_ENABLE_INLINE
#else
    #define APT_ENABLE_INLINE
#endif

#if !defined(APT_ENABLE_INLINE)

    //! In Debug builds or external inclusions, don't inline anything.
    #define APT_FORCE_INLINE
    #define APT_INLINE

#else // defined(EA_DEBUG) || !defined(APTLIBRARY)

    // these are the default values, they will be overridden as needed below.
    #define APT_FORCE_INLINE    inline
    #define APT_INLINE          inline

    // This mess defines force inline macros differently per platform as necessary.
    #if defined(_MSC_VER)
        // Undef the default value and give it a compiler specific value.
        #undef  APT_INLINE
        #define APT_INLINE    /*__declspec(dllexport)*/ inline
        #undef  APT_FORCE_INLINE
        #define APT_FORCE_INLINE    /*__declspec(dllexport)*/  __forceinline
    #endif

    #if defined(EA_PLATFORM_PS3)
        #if !defined(EA_COMPILER_SN)
        #undef  APT_FORCE_INLINE
        #define APT_FORCE_INLINE    inline __attribute__((always_inline)) __attribute__((used)) /*__attribute__ ((visibility("default")))*/
        #undef  APT_INLINE
        #define APT_INLINE          inline __attribute__((used)) /*__attribute__ ((visibility("default")))*/
        #else
        #undef  APT_FORCE_INLINE
        #define APT_FORCE_INLINE    inline __attribute__((always_inline)) __attribute__((used))
        #undef  APT_INLINE
        #define APT_INLINE          inline __attribute__((used))
    #endif
    #endif

	#if defined(EA_COMPILER_CLANG)
		#undef  APT_FORCE_INLINE
		#define APT_FORCE_INLINE    inline __attribute__((always_inline)) __attribute__((used))
		#undef  APT_INLINE
		#define APT_INLINE          inline __attribute__((used))
	#endif

#endif // #if defined(EA_DEBUG) || !defined(APTLIBRARY)


//------------------------------------------------------------------------------
// APT_ACCESS_INTERNAL: Used to stop external access to internal values.
// FIFA_BEGIN | DIVERGENCE | mwebb | 2015 - 05 - 26 | Fix linker errors because the 'protected' version isn't being exported from the library.
//#if defined(APTLIBRARY)
//    #define APT_ACCESS_INTERNAL public
// #else
//     #define APT_ACCESS_INTERNAL protected
// #endif
#define APT_ACCESS_INTERNAL public
// FIFA_END


//! Utility interface. Game can optionally implement this API to receive notifications when user specified
//! thresholds are exceeded. When they have been exceeded, the game can assert / halt / or increase the limit
//! the choice is up to the game. Warning, these functions will be called for EVERY allocation over the
//! given threshold, be warned.
class IAptMemoryLimitCallback{
public:
    virtual ~IAptMemoryLimitCallback() {}

    //! Provided so that one can set limits on a specific class (probably not a common task)
    virtual void ClassMemoryLimitExceeded(class AptMemoryMetrics* pClassTracker) = 0;
};



// GetClassName is a windows define. Any game code that includes Apt headers will always have window headers included.
// So, we are undefining it here to keep the precompiler from replacing it everywhere we use it.
#if EA_PLATFORM_WINDOWS
#ifdef GetClassName
#undef GetClassName
#endif  // GetClassName
#endif  // EA_PLATFORM_WINDOWS


//! Used for memory tracking.
class AptMemoryMetrics{
public:
    //-----------------------------------------------------------------------------------------------------------------
    // Static functions.

    //! Gets the first AptClassMemoryMetrics instance. Useful for walking and printing
    static AptMemoryMetrics *GetFirstInstance();
    //! Gets the next AptClassMemoryMetrics instance
    static AptMemoryMetrics *GetSiblingInstance(AptMemoryMetrics *pCurrent);
    //! Gets the next AptClassMemoryMetrics instance
    static AptMemoryMetrics *GetChildInstance(AptMemoryMetrics *pCurrent);

    //! Sets the callback for memory limit callback. It's functions are called when the configured limits are exceeded
    static void              SetLimitCallback( const IAptMemoryLimitCallback * pCallback);

    //! Initializes the hierarchies.
    static void              Initialize();

    //! Resets All Counters
    static void ResetAll();

    //-----------------------------------------------------------------------------------------------------------------
    // Instance functions.

    //! Get the total number of instances of this class allocated since AptInitialize
    uint32_t    GetTotalAllocationCount();
    //! Get the Number of items currently allocated
    uint32_t    GetCurrentAllocationCount();
    //! Get the number of bytes this class currently takes up in memory
    size_t      GetBytesCurrentlyAllocated();
    //! Get the maximum count of these objects allocated at any one instant.
    size_t      GetMaxBytesAllocatedAtAnyOneTime();
    //! Get the name of the class this metric is for
    const char *GetClassName();

    //! Calls IAptMemoryLimitCallback::ClassMemoryLimitExceeded() when the passed limit is exceeded.
    void        SetClassMemoryLimit(uint32_t uMaxBytes);

    //! Constructs the object and inserts it into the static list of all metric objects.
    explicit AptMemoryMetrics(const char * szClassName, AptMemoryMetrics * pParent);

    //! called when memory is allocated
    void OnMemoryAllocated(size_t nAllocSize);
    //! called when memory is deallocated
    void OnMemoryDeallocated(size_t nAllocSize);

private:
    //! note that all members must be atomically read / written.
    void Reset();


    //-----------------------------------------------------------------------------------------------------------------
    // Static members.
    static AptMemoryMetrics         *spHead;
    static IAptMemoryLimitCallback  *spCallback;
    static bool                      sInitialized;

    //-----------------------------------------------------------------------------------------------------------------
    // Instance Members.
    uint32_t    muTotalAllocationCount;
    uint32_t    muCurrentAllocationCount;
    uint32_t    muBytesCurrentlyAllocated;
    uint32_t    muMaxBytesAllocatedAtAnyOneTime;
    uint32_t    muMemoryLimit;
    const char *mszName;

    AptMemoryMetrics           *mpParent;
    AptMemoryMetrics           *mpNextSibling;
    AptMemoryMetrics           *mpFirstChild;

    // Prevent use of default constructor. (function not implemented)
    AptMemoryMetrics();
};


//-----------------------------------------------------------------------------------------------
// These defines should not be used directly, they are here for the allocation macros only.
#if defined (APT_GATHER_MEMORY_METRICS)

    #define ON_MEM_ALLOCATED(           tracker, nSize) tracker.OnMemoryAllocated(    nSize),
    #define ON_MEM_DEALLOCATED(         tracker, nSize) tracker.OnMemoryDeallocated(  nSize),

    //! This must be done in a the object definition for each class in order to declare the metrics variables.
    #define APT_DECLARE_OBJECT_METRICS static AptMemoryMetrics s_MemoryMetrics;

    //! This must be done in a the object definition for each class in order to declare the metrics variables.
    #define APT_DECLARE_CUSTOM_METRICS(MetricName) extern AptMemoryMetrics MetricName;

#else

    #define ON_MEM_ALLOCATED(           tracker, nSize)
    #define ON_MEM_DEALLOCATED(         tracker, nSize)

    #define APT_DECLARE_OBJECT_METRICS
    #define APT_DECLARE_CUSTOM_METRICS(MetricName)

#endif // #if defined (APT_GATHER_MEMORY_METRICS)

//-----------------------------------------------------------------------------------------------
class DOGMA_PoolManager;
class AptValueGC_PoolManager;

// FIFA_BEGIN | SENDBACK | mwebb | 2015 - 05 - 26 | Adding Frostbite support
//AptValueGC_PoolManager* GetGCPoolManager();
//DOGMA_PoolManager* GetNonGCPoolManager();
FB_APT_API AptValueGC_PoolManager* GetGCPoolManager();
FB_APT_API DOGMA_PoolManager* GetNonGCPoolManager();
// FIFA_END
void PrintAptDebugReport(int flags);
void ClearAptDebugReport();
void EnableAptDebugTracking(bool enable);
int DebugGetNumAptValueTracked(int type);

// @deprecated, but still available in case you really need it.  Please
// use the above Getters instead.
extern class DOGMA_PoolManager*         gpNonGCPoolManager;
extern class AptValueGC_PoolManager*    gpGCPoolManager;

APT_DECLARE_CUSTOM_METRICS(AptAnonymousAllocations)

// This is used some places to conditionally add FILe, LINE, it's dirty I admit, but it's probably better
// then #defines everywhere...
#if defined APT_ALLOCATION_TRACKING
#define APT_ALLOC_PARAMS(size)  size,__FILE__,__LINE__
#else
#define APT_ALLOC_PARAMS(size)  size
#endif

#if defined(APT_ALLOCATION_TRACKING)

    // These are used by the array allocators.
    void *AptNonGCAllocSaveSize(size_t nSize, const char *szName, int nLine);
    void AptNonGCFreeSavedSize(void* p);

    #define APT_NONGC_NEW(         bytes)                (ON_MEM_ALLOCATED(   s_MemoryMetrics, bytes ) GetNonGCPoolManager()->TrackedAllocate(bytes, __FILE__, __LINE__))
    #define APT_NONGC_DELETE( ptr, bytes)                (ON_MEM_DEALLOCATED( s_MemoryMetrics, bytes ) GetNonGCPoolManager()->TrackedDeallocate( (void*)ptr, bytes ))
    #define APT_GC_NEW(            bytes)                (ON_MEM_ALLOCATED(   s_MemoryMetrics, bytes ) GetGCPoolManager()->TrackedAllocateAptValueGC(bytes, __FILE__, __LINE__))
    #define APT_GC_DELETE(    ptr, bytes)                (ON_MEM_DEALLOCATED( s_MemoryMetrics, bytes ) GetGCPoolManager()->TrackedDeallocateAptValueGC( (AptValueGC*)ptr, bytes ))

#elif defined(APT_VERIFY_NON_GC_ALLOC_SIZES)

    // These are used by the array allocators.
    void *AptNonGCAllocSaveSize(size_t nSize);
    void AptNonGCFreeSavedSize(void* p);

    #define APT_NONGC_NEW(         bytes)              (ON_MEM_ALLOCATED(   s_MemoryMetrics, bytes ) GetNonGCPoolManager()->SizeVerifyAllocate(bytes))
    #define APT_NONGC_DELETE( ptr, bytes)              (ON_MEM_DEALLOCATED( s_MemoryMetrics, bytes ) GetNonGCPoolManager()->SizeVerifyDeallocate( (void*)ptr, bytes ))
    #define APT_GC_NEW(            bytes)              (ON_MEM_ALLOCATED(   s_MemoryMetrics, bytes ) GetGCPoolManager()->AllocateAptValueGC(bytes))
    #define APT_GC_DELETE(    ptr, bytes)              (ON_MEM_DEALLOCATED( s_MemoryMetrics, bytes ) GetGCPoolManager()->DeallocateAptValueGC( (AptValueGC*)ptr, bytes ))

#else

    // These are used by the array allocators.
    void *AptNonGCAllocSaveSize(size_t nSize);
    void AptNonGCFreeSavedSize(void* p);

    #define APT_NONGC_NEW(         bytes)              (ON_MEM_ALLOCATED(   s_MemoryMetrics, bytes ) GetNonGCPoolManager()->Allocate(bytes))
    #define APT_NONGC_DELETE( ptr, bytes)              (ON_MEM_DEALLOCATED( s_MemoryMetrics, bytes ) GetNonGCPoolManager()->Deallocate( (void*)ptr, bytes ))
    #define APT_GC_NEW(            bytes)              (ON_MEM_ALLOCATED(   s_MemoryMetrics, bytes ) GetGCPoolManager()->AllocateAptValueGC(bytes))
    #define APT_GC_DELETE(    ptr, bytes)              (ON_MEM_DEALLOCATED( s_MemoryMetrics, bytes ) GetGCPoolManager()->DeallocateAptValueGC( (AptValueGC*)ptr, bytes ))

#endif // #if defined(APT_ALLOCATION_TRACKING)


//-----------------------------------------------------------------------------------------------
// Custom new and delete operators for Apt classes
//
#define APT_VALUE_NEW_DELETE_OPERATORS                                                          \
        static void *operator new(size_t size)           { return APT_NONGC_NEW(size); }        \
        static void operator delete(void *p, size_t size){ APT_NONGC_DELETE(p, size);  }        \
        static void *operator new[](size_t size)         { return AptNonGCAllocSaveSize( APT_ALLOC_PARAMS(size) ); }\
        static void operator delete[](void *p)           { AptValueNoGC::VerifyAptValueNoGC(); AptNonGCFreeSavedSize(p); }

#define APT_VALUE_GC_NEW_DELETE_OPERATORS                                                       \
        static void *operator new(size_t size)           { return APT_GC_NEW(size); }           \
        static void operator delete(void *p, size_t size){ APT_GC_DELETE(p, size);  }           \
        static void *operator new[](size_t)              { APT_FAIL("Garbage collected Objects should never be created in Arrays!!!"); return (void*)0x00000000 ; }        \
        static void operator delete[](void *)           { AptValueGC::VerifyAptValueGC(); }


//-----------------------------------------------------------------------------------------------
#include "DogmaAllocator.h"


#define APT_NEW_DELETE_OPERATORS                                                                \
    public:                                                                                     \
        static void *operator new(size_t size)           { return APT_NONGC_NEW(size); }        \
        static void operator delete(void *p, size_t size){ APT_NONGC_DELETE(p, size);  }        \
        static void *operator new[](size_t size)         { return AptNonGCAllocSaveSize( APT_ALLOC_PARAMS(size) ); }\
        static void operator delete[](void *p)           { AptNonGCFreeSavedSize(p); }          \
        static void *operator new (size_t t, void *p)    { return p; }


#define NATIVE_MEMBER_FUNCTION_DECL(_name_) \
    static AptValue *sMethod_##_name_(AptValue *pThis, int nParams); \
    static AptNativeFunction *psMethod_##_name_;

#define NATIVE_MEMBER_FUNCTION(_class_,_name_) \
    AptNativeFunction *_class_::psMethod_##_name_ = 0; \
    AptValue *_class_::sMethod_##_name_(AptValue *pThis, int nParams)


//------------------------------------------------------------------------------
// DC - Added Increment / Decrement messages. Turn them on with APT_INC_DEC_MESSAGES
// NOTE: These macros are ONLY intended to be used from within the Apt runtime source code.
//       Users should use APT_INC and APT_DEC instead.
#if defined(__GNUC__)   && defined(APT_INC_DEC_MESSAGES)
    #define _APT_DEC(_)          (_)->Release(__PRETTY_FUNCTION__, __FILE__, __LINE__);
    #define _APT_DECSAFE(_)      if (_)  {   (_)->Release(__PRETTY_FUNCTION__, __FILE__, __LINE__); }
    #define _APT_INC(_)          (_)->AddRef(__PRETTY_FUNCTION__, __FILE__, __LINE__);
    #define _APT_INCSAFE(_)      if (_)  {   (_)->AddRef(__PRETTY_FUNCTION__, __FILE__, __LINE__);  }
#elif defined(_MSC_VER) && defined(APT_INC_DEC_MESSAGES)
    #define _APT_DEC(_)          (_)->Release(__FUNCTION__, __FILE__, __LINE__);
    #define _APT_DECSAFE(_)      if (_)  {   (_)->Release(__FUNCTION__, __FILE__, __LINE__); }
    #define _APT_INC(_)          (_)->AddRef(__FUNCTION__, __FILE__, __LINE__);
    #define _APT_INCSAFE(_)      if (_)  {   (_)->AddRef(__FUNCTION__, __FILE__, __LINE__);  }
#else
    #define _APT_DEC(_)          (_)->Release();
    #define _APT_DECSAFE(_)      if (_)  {   (_)->Release(); }
    #define _APT_INC(_)          (_)->AddRef();
    #define _APT_INCSAFE(_)      if (_)  {   (_)->AddRef();  }
#endif

#if !defined(APTLIBRARY)

// FIFA_BEGIN | SENDBACK | mwebb | 2015 - 05 - 26 | Adding Frostbite support
//extern void AptUpdateLock();   // Only to be called by APT_INC/APT_DEC.
//extern void AptUpdateUnlock();
FB_APT_API extern void AptUpdateLock();   // Only to be called by APT_INC/APT_DEC.
FB_APT_API extern void AptUpdateUnlock();
// FIFA_END

//
// For external users: These are the APT_INC and APT_DEC macros you should use. They are thread-safe.
//
#define APT_DEC(_)          { AptUpdateLock(); _APT_DEC(_);     AptUpdateUnlock(); }
#define APT_DECSAFE(_)      { AptUpdateLock(); _APT_DECSAFE(_); AptUpdateUnlock(); }
#define APT_INC(_)          { AptUpdateLock(); _APT_INC(_);     AptUpdateUnlock(); }
#define APT_INCSAFE(_)      { AptUpdateLock(); _APT_INCSAFE(_); AptUpdateUnlock(); }

#else

// Used internally when building the Apt runtime.
#define APT_DEC(_)          _APT_DEC(_)
#define APT_DECSAFE(_)      _APT_DECSAFE(_)
#define APT_INC(_)          _APT_INC(_)
#define APT_INCSAFE(_)      _APT_INCSAFE(_)

#endif // APTLIBRARY


#endif  //  __APTDEFINE_H__
/*H*************************************************************************************************
    EOF
*************************************************************************************************H*/

