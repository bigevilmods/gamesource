/*H************************************************************************************************/
/*!

    \file    AptMemoryMetrics.h

    \brief   Data structures encapsulating runtime memory metrics.


    (c) 2007 Electronic Arts Inc.

    \version    1.0        07/22/08 (RB)  First version
*/
/************************************************************************************************H*/
#ifndef __AptMemoryMetrics_h
#define __AptMemoryMetrics_h

#include <EABase/eabase.h>

struct AptInitParmsT;

/// This struct is used to show "highwater" metrics for various fixed-size internal buffers in Apt.
/// Although the user is expected to pass in reasonable values for AptInitParmsT, which in turn
/// determine the size of these internal buffers, it is hard for the user to know if the size is
/// sufficient.
///
/// Consequently, buffer-overruns (a.k.a. memory stomping) can occur if Apt silently exceeds the
/// bounds of some internal array. In debug builds, this condition somtimes fires an assert. But
/// in many cases the behavior in release builds differs, and there is nothing to indicate an overflow.
/// We would like more assurance that overflow conditions are not occuring in release.
///
/// Enter AptHighwaterMemoryMetrics. About 16 different internal, fixed-size buffers are associated
/// with so-called "highwater memory metrics" in Apt.  Each buffer is represented by an enum value,
/// from AptHighwaterMemoryMetrics::Type.  If an overflow is detected, instead of simply asserting,
/// we invoke the "overflow callback" (see below) and pass it the struct corresponding to the
/// buffer being overrun.  Within the callback implementation, the user can look at the Type enum
/// to see which buffer is involved; any of the fields such as mInitialMaxCount and mHelpfulHint
/// may be printed to the TTY; and the user can decide whether or not to modify Apt's default
/// response (which is typically to invoke APT_ASSERT).
///
/// At any time, including after Apt shuts down, the user has the option of calling
/// AptHighwaterMemoryMetrics::GetMetrics, to loop through all Types and see if any of them have
/// potentially overflowed or to see if mHighwaterMaxCount ever approached mInitialMaxCount.
///
/// Please see http://docs.ea.com/Apt:Collecting_Apt_Metrics#Highwater_Memory_Metrics for further
/// usage documention.
///
struct AptHighwaterMemoryMetrics
{
    /// Apt maintains an internal array of AptHighwaterMemoryMetrics structs indexed by this Type enum.
    enum Type
    {
        //
        // Buffers corresponding to AptInitParmsT fields
        //
        TYPE_BUTTONSET,
        TYPE_INPUTSET,
        TYPE_LISTENERSET,
        TYPE_ACTIONPOOL,
        TYPE_INTERVALS,
        TYPE_BUTTON_INSTANCES,
        TYPE_MOVIECLIP_INSTANCES,
        TYPE_QUEUED_INPUTS,
        TYPE_DATASTACK_DEPTH,
        TYPE_CALLSTACK_DEPTH,
        TYPE_REGISTERARRAY,
        TYPE_LOOKUPARRAY,
        TYPE_ZOMBIEVECTOR,

        //
        // Buffers corresponding to internal Apt data structures
        //
        TYPE_FREECUSTCTRLS,
        TYPE_INTERVAL_PARAMS,
        TYPE_MAX
    };

    enum Policy
    {
        POLICY_APT_WILL_ASSERT =                        0x0001,
        //POLICY_APT_WILL_ALLOCATE_MORE_MEMORY =          0x0002, //reserved
    };

    Type           mType;               /// Type of metric. Unique among all instances of this struct.
    int32_t        mInitialMaxCount;    /// The maximum size of the fixed-size buffer associated with this metric (in terms of number of items, not bytes).
    int32_t        mHighwaterMaxCount;  /// For some buffers we can keep track of how high the item-count ever goes; if this goes over mInitialMaxCount you know an overflow occurred.
    int32_t        mEstimatedBytesPerItem; /// A rough estimate of the memory cost per item for the associated buffer. For example, this is sizeof(void*) for an array of void*.
    int32_t        mDefaultPolicy;      /// A bitwise 'or' combination of the Policy enum. Tells Apt what to do if an overflow occurs.
    const char8_t* mHelpfulHint;       /// Some text explaining the situation if an overflow occurs; useful when implementing sOverflowCallback.

    static class IAptHighwaterMemoryOverflowCallback* sOverflowCallback;  /// see IAptHighwaterMemoryOverflowCallback below


    /******************************************************************************/
    /*! AptHighwaterMemoryMetrics :: GetMetrics

        \brief  Gets a single metrics record

        \param  type    - type of metrics to get
        \return         - const pointer to metrics record

        Call this method at any time to query the highwater metrics gathered so far.
        Typically the user would want to call this after Apt shutdown to see if
        the actual highwater value (mHighwaterMaxCount) ever actually came close
        to mInitialMaxCount. If the highwater value is always really low,
        and certainly if there was an overflow, the user can make an informed decision
        about changing one of the buffer sizes.
    */
    /******************************************************************************/
    static const AptHighwaterMemoryMetrics* GetMetrics( Type type );


    /******************************************************************************/
    /*! AptHighwaterMemoryMetrics :: CompareAptInitParams

        \brief  Compares user's current Apt Init Params with the gathered highwater values

        \param  userParams     - User's Apt Init Params that were passed to AptInitialize.
                                 If NULL, we use the global gAptInitParms (see _Apt.h)
        \param  returnedParams - New, suggested Apt Init Params. NOTE: Not guaranteed to be optimal!
        \return                - true if params need to change; false if everythings OK for now

        Call this to compare your AptInitParams with the highwater counters Apt has measured so far.
        This function returns an AptInitParmsT struct whose values are enlarged to account for any
        detected overflow. NOTE: The new sizes are just a best-guess attempt at a better value,
        not a precise optimal value, nor even guaranteed not to overflow next time!  It might take
        a few iterations to find a good size.
        This is just for convenience. You can call it before or after AptShutdown.
    */
    /******************************************************************************/
    static bool CompareAptInitParams(const AptInitParmsT* userParams, AptInitParmsT * returnedParams);


    /******************************************************************************/
    /*! AptHighwaterMemoryMetrics :: GetTypeAsString

        \brief  Converts Type enum to a string

        \param  type           - Type to convert
        \return                - friendly string name
    */
    /******************************************************************************/
    static const char* GetTypeAsString( Type type );
};

/// This callback is invoked whenever an overflow condition is detected for the buffer associated with this metric.
///
/// The user can choose to examine the AptHighwaterMemoryMetrics struct passed in (to print out any of the struct's
/// fields, for example).
///
/// The return value MUST be a bitwise combination of Policy enum values. The implementation can just return
/// metrics.mDefaultPolicy in most cases-- that will typically tell Apt to assert.
/// NOTE: This means if 0 is returned, Apt will NOT assert.
class IAptHighwaterMemoryOverflowCallback
{
public:
    virtual ~IAptHighwaterMemoryOverflowCallback() {}

    virtual int32_t OnOverflowDetected(
        const AptHighwaterMemoryMetrics& metrics,  /// metric info associated with this overflow condition
        const void* collection,  /// Optional raw pointer to internal Apt buffer that is getting overrun. WARNING: Just for looking, not for touching. Might be NULL if we can't provide it.
        const void* item         /// Raw pointer to item being added at the time of overflow. Do not touch. Might be NULL if we can't provide it.
    ) = 0;
};

#endif // __AptMemoryMetrics_h
