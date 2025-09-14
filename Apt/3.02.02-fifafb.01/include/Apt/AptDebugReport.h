/*H************************************************************************************************/
/*!

    \File
        AptDebugReport.h

    \Description
        Prints out the status of various memory related apt systems

    \Notes
        Not available for usage external to apt

    \Copyright
        (c) 2013 Electronic Arts Inc.

    \Version
        1.0        04/20/13 (bmanson) First Version
*/
/************************************************************************************************H*/

#ifndef __APTDEBUGREPORT_H__
#define __APTDEBUGREPORT_H__

#if defined(APT_ENABLE_DEBUG_REPORT)
/*** Includes *****************************************************************/
#include "EASTL/hash_map.h"
#include "EASTL/string.h"
#include "EASTL/vector.h"
/*** Macros *******************************************************************/

/*** Interface ****************************************************************/

class AptCIH;
class DOGMA_PoolManager;
class AptValue;
class EAStringC;
class AptNativeHash;

// --- Namespaces
namespace EA
{
namespace Apt
{
namespace Debug
{

// --- Constants

// --- Declarations

typedef eastl::hash_map<int64_t, eastl::string> AllocationMap;
typedef eastl::hash_map<eastl::string, int> AllocationCountMap;
typedef eastl::pair<eastl::string, int> AllocationCount;

// eastl::sort comparator class for sorting PlayerStat pairs
class AllocationCountSortByString
{

public:
    bool operator()(const AllocationCount& a, const AllocationCount& b)
    {
        return a.first.compare(b.first) > 0;
    }

    bool operator()(const AllocationCount* a, const AllocationCount* b)
    {
        return a->first.compare(b->first) > 0;
    }
};

class AptReport
{
    public:
        AptReport(DOGMA_PoolManager* nonGCPool = NULL, DOGMA_PoolManager* gcPool = NULL);
        virtual ~AptReport();
        
        void StartTrack(const AptCIH* cih);
        void EndTrack(const AptCIH* cih);

        void StartTrack(AptValue* val);
        void EndTrack(const AptValue* val);

        void StartTrack(const EAStringC* val);
        void EndTrack(const EAStringC* val);

        void Print(int flags);
        int GetNumAptValues(int valueType) const;

        void PrintTracked(bool printRaw, bool printOwners=false, bool onlyPrintDelta=false);
        void ClearTracked();
        void EnableTracking(bool enable) { mTrackingEnabled = enable; }
        void PrintOwners(AptValue* aptValue, const eastl::vector<AptValue*> &alreadyProcessed, int depth=0, int maxDepth=3) const;
        void PrintProblemDependencies(const char * origin) const;

    private:
        explicit AptReport(const AptReport & report);
        const AptReport& operator=(const AptReport & report);

        int FindCIHTrackIndex(const AptCIH* cih) const;
        int FindUnusedCIHTrackIndex() const;
        void PrintActiveCIH() const;
        static unsigned int GetCIHHash(const EAStringC* str);

        static const int MAX_TRACK_CIH = 8192;
        const AptCIH* mCIHList[MAX_TRACK_CIH];
        
        static const int MAX_TRACK_APTVALUE = 64;
        unsigned int mAptValueList[MAX_TRACK_APTVALUE];
        
        static const int MAX_TRACK_APTVALUE_LABEL_STRLEN = 64;
        char mAptValueLabelList[MAX_TRACK_APTVALUE][MAX_TRACK_APTVALUE_LABEL_STRLEN];

        bool mTrackingEnabled;

        uint64_t mNumStrings;

        AllocationMap mAllocations;
        AllocationCountMap mCountedAllocs;
        AllocationCountMap mLastCountedAllocs;

        DOGMA_PoolManager* mNonGCPool;
        DOGMA_PoolManager* mGCPool;
};

} // end namespace Debug
} // end namespace Apt
} // end namespace EA

#endif // defined APT_ENABLE_DEBUG_REPORT

#endif // __APTDEBUGREPORT_H__
