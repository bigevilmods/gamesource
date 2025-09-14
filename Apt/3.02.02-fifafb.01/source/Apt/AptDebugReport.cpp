/*H************************************************************************************************/
/*!

    \File
        AptDebugReport.cpp

    \Description
        Track and report information about Apt's memory usage

    \Notes
        This is not available for access outside of Apt

    \Copyright
        (c) 2013 Electronic Arts Inc.

    \Version
        1.0        04/20/13 (bmanson) First Version
*/
/************************************************************************************************H*/
#if defined(APT_ENABLE_DEBUG_REPORT)

/*** Includes ******************************************************************/
#include "AptCIH.h"
#include "AptDebugReport.h"
#include "DogmaAllocator.h"
#include "AptValue/AptValue.h"
#include "stdio.h"      // for string functions
#include "EASTL/vector.h"
#include "EASTL/sort.h"

/*** Macros *******************************************************************/

/*** Interface ****************************************************************/

namespace EA
{
namespace Apt
{
namespace Debug
{

/*** Variables ****************************************************************/

/*** Implementation ***********************************************************/

/******************************************************************************/
/*! 
    AptReport::AptReport

    \brief  constructor for report objects.

    \param          - none.
    \return         - none

*/
/******************************************************************************/
AptReport::AptReport(DOGMA_PoolManager* nonGCPool, DOGMA_PoolManager* gcPool) : mNonGCPool(nonGCPool), mGCPool(gcPool), mTrackingEnabled(false), mNumStrings(0), mAllocations(NULL), mCountedAllocs(NULL)
{
    APT_ASSERT(MAX_TRACK_APTVALUE >= AptVFT_NumVFTs);

    mAllocations.clear();
    mCountedAllocs.clear();

    memset(mCIHList, 0, sizeof(AptCIH*) * MAX_TRACK_CIH);
    memset(mAptValueList, 0, sizeof(unsigned int) * MAX_TRACK_APTVALUE);
    memset(mAptValueLabelList, 0, sizeof(char) * MAX_TRACK_APTVALUE_LABEL_STRLEN * MAX_TRACK_APTVALUE);

    strncpy(mAptValueLabelList[AptVFT_StringValue], "StringVal", MAX_TRACK_APTVALUE_LABEL_STRLEN);
    strncpy(mAptValueLabelList[AptVFT_Property], "Property", MAX_TRACK_APTVALUE_LABEL_STRLEN);
    strncpy(mAptValueLabelList[AptVFT_None], "None", MAX_TRACK_APTVALUE_LABEL_STRLEN);
    strncpy(mAptValueLabelList[AptVFT_Register], "Register", MAX_TRACK_APTVALUE_LABEL_STRLEN);
    strncpy(mAptValueLabelList[AptVFT_Boolean], "Boolean", MAX_TRACK_APTVALUE_LABEL_STRLEN);
    strncpy(mAptValueLabelList[AptVFT_Float], "Float", MAX_TRACK_APTVALUE_LABEL_STRLEN);
    strncpy(mAptValueLabelList[AptVFT_Integer], "Integer", MAX_TRACK_APTVALUE_LABEL_STRLEN);
    strncpy(mAptValueLabelList[AptVFT_Lookup], "Lookup", MAX_TRACK_APTVALUE_LABEL_STRLEN);
    strncpy(mAptValueLabelList[AptVFT_NativeFunction], "Native Function", MAX_TRACK_APTVALUE_LABEL_STRLEN);
    strncpy(mAptValueLabelList[AptVFT_FrameStack], "FrameStack", MAX_TRACK_APTVALUE_LABEL_STRLEN);
    strncpy(mAptValueLabelList[AptVFT_Extern], "Extern", MAX_TRACK_APTVALUE_LABEL_STRLEN);
    strncpy(mAptValueLabelList[AptVFT_CharacterInstHandle], "CIH", MAX_TRACK_APTVALUE_LABEL_STRLEN);
    strncpy(mAptValueLabelList[AptVFT_Sound], "Sound", MAX_TRACK_APTVALUE_LABEL_STRLEN);
    strncpy(mAptValueLabelList[AptVFT_Array], "Array", MAX_TRACK_APTVALUE_LABEL_STRLEN);
    strncpy(mAptValueLabelList[AptVFT_Math], "Math", MAX_TRACK_APTVALUE_LABEL_STRLEN);
    strncpy(mAptValueLabelList[AptVFT_Key], "Key", MAX_TRACK_APTVALUE_LABEL_STRLEN);
    strncpy(mAptValueLabelList[AptVFT_Global], "Global", MAX_TRACK_APTVALUE_LABEL_STRLEN);
    strncpy(mAptValueLabelList[AptVFT_ScriptColour], "ScriptColour", MAX_TRACK_APTVALUE_LABEL_STRLEN);
    strncpy(mAptValueLabelList[AptVFT_Object], "Object", MAX_TRACK_APTVALUE_LABEL_STRLEN);
    strncpy(mAptValueLabelList[AptVFT_Prototype], "Prototype", MAX_TRACK_APTVALUE_LABEL_STRLEN);
    strncpy(mAptValueLabelList[AptVFT_Date], "Date", MAX_TRACK_APTVALUE_LABEL_STRLEN);
    strncpy(mAptValueLabelList[AptVFT_MovieClip], "MovieClip", MAX_TRACK_APTVALUE_LABEL_STRLEN);
    strncpy(mAptValueLabelList[AptVFT_Mouse], "Mouse", MAX_TRACK_APTVALUE_LABEL_STRLEN);
    strncpy(mAptValueLabelList[AptVFT_XmlNode], "XmlNode", MAX_TRACK_APTVALUE_LABEL_STRLEN);
    strncpy(mAptValueLabelList[AptVFT_Xml], "Xml", MAX_TRACK_APTVALUE_LABEL_STRLEN);
    strncpy(mAptValueLabelList[AptVFT_XmlAttributes], "XmlAttributes", MAX_TRACK_APTVALUE_LABEL_STRLEN);
    strncpy(mAptValueLabelList[AptVFT_LoadVars], "LoadVars", MAX_TRACK_APTVALUE_LABEL_STRLEN);
    strncpy(mAptValueLabelList[AptVFT_TextFormat], "TextFormat", MAX_TRACK_APTVALUE_LABEL_STRLEN);
    strncpy(mAptValueLabelList[AptVFT_Extension], "Extension", MAX_TRACK_APTVALUE_LABEL_STRLEN);
    strncpy(mAptValueLabelList[AptVFT_GlobalExtension], "GlobalExtension", MAX_TRACK_APTVALUE_LABEL_STRLEN);
    strncpy(mAptValueLabelList[AptVFT_Stage], "Stage", MAX_TRACK_APTVALUE_LABEL_STRLEN);
    strncpy(mAptValueLabelList[AptVFT_Error], "Error", MAX_TRACK_APTVALUE_LABEL_STRLEN);
    strncpy(mAptValueLabelList[AptVFT_StringObject], "String Object", MAX_TRACK_APTVALUE_LABEL_STRLEN);
    strncpy(mAptValueLabelList[AptVFT_ScriptFunction1], "ScriptFunction1", MAX_TRACK_APTVALUE_LABEL_STRLEN);
    strncpy(mAptValueLabelList[AptVFT_ScriptFunction2], "ScriptFunction2", MAX_TRACK_APTVALUE_LABEL_STRLEN);
    strncpy(mAptValueLabelList[AptVFT_ScriptFunctionByteCodeBlock], "ScriptFunctionByteCode", MAX_TRACK_APTVALUE_LABEL_STRLEN);
    strncpy(mAptValueLabelList[AptVFT_CIHNone], "CIHNone", MAX_TRACK_APTVALUE_LABEL_STRLEN);
    strncpy(mAptValueLabelList[AptVFT_MovieClipLoader], "MovieClipLoader", MAX_TRACK_APTVALUE_LABEL_STRLEN);
    strncpy(mAptValueLabelList[AptVFT_AptUtil], "AptUtil", MAX_TRACK_APTVALUE_LABEL_STRLEN);
    strncpy(mAptValueLabelList[AptVFT_AltInput], "AltInput", MAX_TRACK_APTVALUE_LABEL_STRLEN);
}

/******************************************************************************/
/*! 
    AptReport::~AptReport

    \brief  destructor for report objects.

    \param          - none.
    \return         - none

*/
/******************************************************************************/
AptReport::~AptReport()
{
    // nothing for now
}

/******************************************************************************/
/*! 
    AptReport::StartTrack

    \brief  start tracking an eastring (aptnativestring)

    \param          - EAStringC - which to track
    \return         - none

*/
/******************************************************************************/
void AptReport::StartTrack(const EAStringC* val) 
{
    if(val)
    {
        //APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, "StartTrack [%5"PRId64"]\t%016llx\t%s", mNumStrings, val, val->c_str());
        mNumStrings++;
    }
}

/******************************************************************************/
/*! 
    AptReport::EndTrack

    \brief  end tracking an eastringc

    \param          - val - which to stop tracking
    \return         - none

*/
/******************************************************************************/
void AptReport::EndTrack(const EAStringC* val)
{
    if(val)
    {
        //APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, "EndTrack   [%5"PRId64"]\t%016llx\t%s", mNumStrings, val, val->c_str());
        APT_ASSERT(mNumStrings > 0);
        mNumStrings--;
    }
}

/******************************************************************************/
/*! 
    AptReport::StartTrack

    \brief  start tracking an aptvalue

    \param          - AptValue - which to track
    \return         - none

*/
/******************************************************************************/
void AptReport::StartTrack(AptValue* val)
{ 
    if(val && mTrackingEnabled)
    {
        // Determines how detailed we get when storing the current callstack. We always store at 
        // least the first one.
        const int TRACK_CALLSTACK_DEPTH = 3;

        AptVirtualFunctionTable_Indices idx = val->getVtblIndex();
        if(idx >= 0 && idx < AptVFT_NumVFTs)
        {
            mAptValueList[idx]++;

            if (idx == AptVFT_Object 
                || idx == AptVFT_Array
                //|| idx == AptVFT_ScriptFunction2
                )
            {
                eastl::string origin = "";
                
                int count = gAptOptCallStack->GetItemCount() - 1;
                if (count < 0)
                {
                    count = 0;
                }
                const AptStackItem item = gAptOptCallStack->GetItem(count);
                origin.sprintf("{%s}  (%s::%s::%s)", mAptValueLabelList[idx], item.file, item.scope, item.function);

                // Let's show the last 3 functions in the callstack
                for (int i = 1; i < TRACK_CALLSTACK_DEPTH && (count - i) >= 0; i++)
                {
                    const AptStackItem loopitem = gAptOptCallStack->GetItem(count - i);
                    origin.append_sprintf("\t(%s::%s::%s)", loopitem.file, loopitem.scope, loopitem.function);
                }
                int64_t address = (int64_t)val;
                mAllocations[address] = origin;
            }
        }
         else
        {
            APT_ASSERT(false);
        }
    }
}

/******************************************************************************/
/*! 
    AptReport::EndTrack

    \brief  end tracking an aptvalue

    \param          - val - which to stop tracking
    \return         - none

*/
/******************************************************************************/
void AptReport::EndTrack(const AptValue* val)
{ 
    if(val && mTrackingEnabled)
    {
        AptVirtualFunctionTable_Indices idx = val->getVtblIndex();
        if(idx >= 0 && idx < AptVFT_NumVFTs)
        {
            mAptValueList[idx]--;

            
            if (idx == AptVFT_Object 
                || idx == AptVFT_Array
                || idx == AptVFT_ScriptFunction2
                )
            {
                int64_t address = (int64_t)val;
                mAllocations.erase(address);
            }

        }
        else
        {
            APT_ASSERT(false);
        }
    }
}


void AptReport::PrintOwners(AptValue* aptValue, const eastl::vector<AptValue*> &alreadyProcessed, int depth/*=0*/, int maxDepth/*=3*/) const
{
    if(depth == maxDepth)
    {
        return;
    }

    AptValue * checkValue = NULL;

    eastl::string indent = "    ";

    for(int i=0; i < depth; i++)
    {
        indent.append_sprintf("  ");
    }

    if (aptValue)
    {
        AptHashItem * internalIter = NULL;

        checkValue = gpGCPoolManager->GetFirstAptValue();
        while (checkValue)
        {
            eastl::vector<AptValue*>::const_iterator iter = alreadyProcessed.begin();
            eastl::vector<AptValue*>::const_iterator iterEnd = alreadyProcessed.end();
            bool alreadySeen = false;

            for(; iter != iterEnd; ++iter)
            {
                if((*iter) == checkValue)
                {
                    alreadySeen = true;
                    break;
                }
            }

            if (checkValue->getVtblIndex() != AptVFT_CIHNone)
            {
                if(checkValue->getVtblIndex() == AptVFT_Array)
                {
                    AptArray* arrayVal = checkValue->c_array();

                    for(int index=0; index < arrayVal->length(); index++)
                    {
                        AptValue* val = arrayVal->get(index);

                        if(val == aptValue)
                        {
                            if(alreadySeen)
                            {
                                APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, "%sCIRCULAR REFERENCE at index %d",indent.c_str(), index);
                            }
                            else
                            {
                                APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, "%sIn an array at index %d",indent.c_str(), index);
                            }

                            alreadyProcessed.push_back(aptValue);

                            if(!alreadySeen)
                            {
                                PrintOwners(checkValue, alreadyProcessed, depth+1, maxDepth);
                            }

                        }
                    }
                }
                else
                {
                    internalIter = checkValue->GetNativeHashVirtual() == NULL ? NULL : checkValue->GetNativeHashVirtual()->GetFirstItem();
                    while (internalIter)
                    {
                        if (internalIter->mValue == aptValue)
                        {
                            if(alreadySeen)
                            {
                                APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, "%sCIRCULAR REFERENCE as '%s' by a(n) %s with refcount %d",indent.c_str(), internalIter->Key.c_str(), mAptValueLabelList[checkValue->getVtblIndex()], checkValue->getRefCount());
                            }
                            else
                            {
                                APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, "%sReferenced as '%s' by a(n) %s with refcount %d",indent.c_str(), internalIter->Key.c_str(), mAptValueLabelList[checkValue->getVtblIndex()], checkValue->getRefCount());
                            }

                            alreadyProcessed.push_back(aptValue);

                            if(!alreadySeen)
                            {
                                PrintOwners(checkValue, alreadyProcessed, depth+1, maxDepth);
                            }
                        }
                        internalIter = checkValue->GetNativeHashVirtual()->GetNextItem(internalIter);
                    }
                }

            }
            checkValue = gpGCPoolManager->GetNextAptValue(checkValue);
        }
    }
}

// (<native>::Extension::GetTeamInfo)\t(../nba/node/nav/CriticalPath::(null)::_UpdateTeamInfo)\t(../nba/node/nav/CriticalPath::(null)::call)
void AptReport::PrintProblemDependencies(const char * origin) const
{
    // step 1, find the AptValue from within all our allocations.
    AptValue * problemValue = NULL;

    AllocationMap::const_iterator iter = mAllocations.begin();
    AllocationMap::const_iterator iterEnd = mAllocations.end();

    APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, "Searching for items allocated from [%s]", origin);    
    for(; iter != iterEnd; ++iter)
    {
        eastl::string thisOrigin = (*iter).second;

        if (thisOrigin.compare(origin) == 0)
        {
            // FOUND!
            // Man, this is safe as houses.
            problemValue = (AptValue*)((*iter).first);
            AptVirtualFunctionTable_Indices idx = problemValue->getVtblIndex();
            APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, "  Found a(n) %s allocated here with refcount %d.", mAptValueLabelList[idx], problemValue->getRefCount());
            
            eastl::vector<AptValue*> start;
            PrintOwners(problemValue, start);
        }
    }


}

void AptReport::PrintTracked(bool printRaw, bool printOwners, bool onlyPrintDelta)
{
    // Raw [memory, source] data
    if (printRaw)
    {
        AllocationMap::const_iterator iter = mAllocations.begin();
        AllocationMap::const_iterator iterEnd = mAllocations.end();

        for(; iter != iterEnd; ++iter)
        {
            int64_t address = (*iter).first;
            eastl::string origin = (*iter).second;

            APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, "[%" PRId64 "]\t[%s]\n", address, origin.c_str());
        }
    }
    // [source, count] display.
    else
    {

        // TODO - instead of counting them here, I could count them as they come in,
        // in the StartTrack and EndTrack.
        mLastCountedAllocs = mCountedAllocs;
        mCountedAllocs.clear();

        int counter;
        eastl::string tmpOrigin;

        AllocationMap::const_iterator allocIter = mAllocations.begin();
        AllocationMap::const_iterator allocIterEnd = mAllocations.end();

        for(; allocIter != allocIterEnd; ++allocIter)
        {
            tmpOrigin = (*allocIter).second;

            AllocationCountMap::const_iterator found = mCountedAllocs.find(tmpOrigin);

            if(found == mCountedAllocs.end())
            {
                counter = 1;
            }
            else
            {
                counter = mCountedAllocs[tmpOrigin]+1;
            }

            mCountedAllocs[tmpOrigin] = counter;
        }

        // Now go through the counted elements
        AllocationCountMap::const_iterator countedIter = mCountedAllocs.begin();
        AllocationCountMap::const_iterator countedIterEnd = mCountedAllocs.end();

        eastl::vector<AllocationCount> countVector;

        for(; countedIter != countedIterEnd; ++countedIter)
        {
            bool addToVector = !onlyPrintDelta;

            // Find it in the previous list of allocations
            AllocationCountMap::iterator item = mLastCountedAllocs.find((*countedIter).first);

            // If we found it
            if(item != mLastCountedAllocs.end())
            {
                // And the new refcount is bigger than the old one
                if((*countedIter).second > (*item).second)
                {
                    // Display this!
                    addToVector = true;
                }
            }

            if(addToVector)
            {
                countVector.push_back(eastl::make_pair((*countedIter).first, (*countedIter).second));
            }
        }

        eastl::vector<AllocationCount>::iterator sortIter = countVector.begin();
        eastl::vector<AllocationCount>::iterator sortIterEnd = countVector.end();
        AllocationCountSortByString compare;

        eastl::sort(sortIter, sortIterEnd, compare);

        eastl::vector<AllocationCount>::const_iterator sortedIter = countVector.begin();
        eastl::vector<AllocationCount>::const_iterator sortedIterEnd = countVector.end();

        for(; sortedIter != sortedIterEnd; ++sortedIter)
        {
            eastl::string origin = (*sortedIter).first;
            int count = (*sortedIter).second;
            APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, "%d\t[%s]\n", count, origin.c_str());
            if(printOwners)
            {
                PrintProblemDependencies(origin.c_str());
            }
        }

    }
}
void AptReport::ClearTracked()
{
    mAllocations.clear();
}
/******************************************************************************/
/*! 
    AptReport::StartTrack

    \brief  start tracking a CIH

    \param          - cih - which to track
    \return         - none

*/
/******************************************************************************/
void AptReport::StartTrack(const AptCIH* cih)
{
    int idx = FindUnusedCIHTrackIndex();
    if( idx >= 0 && idx < MAX_TRACK_CIH)
    {
        mCIHList[idx] = cih;
    }
    else
    {
        APT_ASSERT(false);
    }
}

/******************************************************************************/
/*! 
    AptReport::EndTrack

    \brief  end tracking a CIH

    \param          - cih - which to stop tracking
    \return         - none

*/
/******************************************************************************/
void AptReport::EndTrack(const AptCIH* cih)
{
    int idx = FindCIHTrackIndex(cih);
    if( idx >= 0 && idx < MAX_TRACK_CIH)
    {
        mCIHList[idx] = NULL;
    }
}

/******************************************************************************/
/*! 
    AptReport::Print

    \brief  make a report of all various Apt subsystems

    \param          - none
    \return         - none

*/
/******************************************************************************/
void AptReport::Print(int flags)
{
    const int FLAG_GC_POOL = 0x01;
    const int FLAG_NON_GC_POOL = 0x01 << 1;
    const int FLAG_ACTIVE_CIH = 0x01 << 2;
    const int FLAG_APT_VALUES = 0x01 << 3;
    const int FLAG_NATIVE_STRINGS = 0x01 << 4;
    const int FLAG_TRACKED_ALLOCS = 0x01 << 5;
    const int FLAG_OWNERS = 0x01 << 6;
    const int FLAG_DELTA_ONLY = 0x01 << 7;

    if(mNonGCPool && (flags & FLAG_GC_POOL))
    {       
        APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, "*****************************************\n");
        APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, "               NON-GC Dogma Pool         \n");
        APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, "*****************************************\n");
        APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, "\n   Num Allocs\t     Bytes\n");

        size_t runningTotalBytes = 0;
        size_t runningTotalAllocs = 0;
        for(int bucket = 1; bucket <= 64; bucket++)
        {
            size_t alloc = mNonGCPool->GetMaxAllocatedInBucket(bucket);
            APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, "   %10d\t%10d\n", alloc, alloc * bucket);
            runningTotalBytes += (alloc * bucket);
            runningTotalAllocs += alloc;
        }
        APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, "======> Non-GC Internal Total: allocs:%d\tbytes:%d\n", runningTotalAllocs, runningTotalBytes);
        APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, "======> Non-GC External Total: bytes:%d\n", mNonGCPool->GetExternalBytesUsed());
        APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, "\n\n");
    }
    
    if(mGCPool && (flags & FLAG_NON_GC_POOL))
    {
        APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, "*****************************************\n");
        APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, "               GC Dogma Pool             \n");
        APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, "*****************************************\n");
        APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, "\n   Num Allocs\t     Bytes\n");        

        size_t runningTotalBytes = 0;
        size_t runningTotalAllocs = 0;
        for(int bucket = 1; bucket <= 24; bucket++)
        {
            size_t alloc = mGCPool->GetMaxAllocatedInBucket(bucket);
            APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, "   %10d\t%10d\n", alloc, alloc * bucket);
            runningTotalBytes += (alloc * bucket);
            runningTotalAllocs += alloc;
        }
        APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, "======> GC Internal Total: allocs:%d\tbytes:%d\n", runningTotalAllocs, runningTotalBytes);
        APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, "======> GC External Total: bytes:%d\n", mGCPool->GetExternalBytesUsed());
        APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, "\n\n"); 
        // todo: num bytes outside of dogma
    }

    if (flags & FLAG_ACTIVE_CIH)
    {
        PrintActiveCIH();
        APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, "\n\n");
    }

    if (flags & FLAG_APT_VALUES)
    {
        APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, "*****************************************\n");
        APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, "               Apt Values                \n");
        APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, "*****************************************\n");        

        for(int i = AptVFT_xxx; i < AptVFT_NumVFTs; i++)
        {
            if(mAptValueList[i] > 0)
            {
                APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, " -- %s (%d)\n", mAptValueLabelList[i], mAptValueList[i]);
            }
        }

        APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, "\n\n"); 
    }

    if (flags & FLAG_NATIVE_STRINGS)
    {
        APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, "*****************************************\n");
        APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, "            Apt Native Strings           \n");
        APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, "*****************************************\n");
        
        APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, "Number of Strings:%" PRIu64 "\n", mNumStrings);

        APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, "\n\n"); 
    }

    if (flags & FLAG_TRACKED_ALLOCS)
    {
        bool showOwners = (flags & FLAG_OWNERS) != 0;
        bool deltaOnly = (flags & FLAG_DELTA_ONLY) != 0;
        PrintTracked(false, showOwners, deltaOnly);
        //PrintTracked(true);
        // (<native>::Extension::GetTeamInfo)\t(../nba/node/nav/CriticalPath::(null)::_UpdateTeamInfo)\t(../nba/node/nav/CriticalPath::(null)::call)
        //PrintProblemDependencies("{14}(<native>::Extension::GetTeamInfo)\t(../nba/node/nav/CriticalPath::(null)::_UpdateTeamInfo)\t(../nba/node/nav/CriticalPath::(null)::call)");
        //PrintProblemDependencies("{19}(main::(null)::CreateEventFocusChanged)\t(main::Object::FocusOnIndex)\t(main::Object::Focus)");
        

    }
}


/******************************************************************************/
/*! 
    AptReport::GetNumAptValues

    \brief  Return how many AptValues of a particular type are currently tracked

    \param          - none
    \return         - none

*/
/******************************************************************************/
int AptReport::GetNumAptValues(int valueType) const
{
    APT_ASSERT(valueType >= AptVFT_xxx && valueType < AptVFT_NumVFTs);
    return mAptValueList[valueType];
}

/******************************************************************************/
/*! 
    AptReport::PrintActiveCIH

    \brief  make a report of active CIH clips

    \param          - none
    \return         - none

*/
/******************************************************************************/
void AptReport::PrintActiveCIH() const
{
    struct CIHHash
    {
        unsigned int mHash;
        int mRefCount;
        const AptNativeString* mLabel;

        inline CIHHash():mHash(0), mRefCount(0), mLabel(NULL) {}
    };
    
    CIHHash* listCIH = new CIHHash[MAX_TRACK_CIH];
    for(int i = 0; i < MAX_TRACK_CIH; i++)
    {
        if(mCIHList[i] != NULL)
        {
            const EAStringC* string = &mCIHList[i]->GetTrackLabel();
            if(string != NULL)
            {
                unsigned int hash = GetCIHHash(string);
                bool foundIt = false;
                int firstOpenBucket = -1;
                for(int j = 0; j < MAX_TRACK_CIH; j++)
                {
                    if(firstOpenBucket < 0 && listCIH[j].mHash == 0)
                    {
                        firstOpenBucket = j;
                    }
                    else if(listCIH[j].mHash == hash)
                    {
                        listCIH[j].mRefCount++;
                        foundIt = true;
                        break;
                    }
                }
            
                if(!foundIt && firstOpenBucket >= 0)
                {
                    listCIH[firstOpenBucket].mHash = hash;
                    listCIH[firstOpenBucket].mLabel = string;
                    listCIH[firstOpenBucket].mRefCount++;
                }
                else
                {
                    // no more buckets remaining, need to increase MAX_TRACK_CIH
                    APT_ASSERT(foundIt);
                }
            }
        }
    }

    APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, "*****************************************\n");
    APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, "               ACTIVE CIHs               \n");
    APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, "*****************************************\n");    

    for(int i = 0; i < MAX_TRACK_CIH; i++)
    {
        if(listCIH[i].mRefCount > 0)
        {
            APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, " -- %s (%d)\n", listCIH[i].mLabel->c_str(), listCIH[i].mRefCount);
        }
    }

    delete [] listCIH;
}

/******************************************************************************/
/*! 
    AptReport::GetCIHHash

    \brief  compute a hash for lookup on the string

    \param          - string - pointer to a string that needs a hash
    \return         - hash code

*/
/******************************************************************************/
unsigned int AptReport::GetCIHHash(const EAStringC* str)
{
    const unsigned char* p = (const unsigned char*)str->c_str(); // To consider: limit p to at most 256 chars.
    unsigned int c, result = 2166136261U; // We implement an FNV-like string hash. 
    while((c = *p++) != 0) // Using '!=' disables compiler warnings.
        result = (result * 16777619) ^ c;
    return result;
}


/******************************************************************************/
/*! 
    AptReport::FindUnusedCIHTrackIndex

    \brief  which index is free to have a CIH inserted into it

    \param          - none
    \return         - index if found, -1 if no room left

*/
/******************************************************************************/
int AptReport::FindUnusedCIHTrackIndex() const
{
    int index = -1;
    for(int i = 0; i < MAX_TRACK_CIH; i++)
    {
        if(mCIHList[i] == NULL)
        {
            index = i;
            break;
        }
    }

    return index;
}

/******************************************************************************/
/*! 
    AptReport::FindCIHTrackIndex

    \brief  which index is a given CIH located at in our tracker?

    \param          - cih - which cih to find
    \return         - index if found, -1 if not found

*/
/******************************************************************************/
int AptReport::FindCIHTrackIndex(const AptCIH* cih) const
{
    int index = -1;
    for(int i = 0; i < MAX_TRACK_CIH; i++)
    {
        if(mCIHList[i] == cih)
        {
            index = i;
            break;
        }
    }

    return index;
}

} // end namespace Debug
} // end namespace Apt
} // end namespace EA

#endif // defined (APT_ENABLE_DEBUG_REPORT)
