/*H************************************************************************************************/
/*!

    \File
        AptGC.cpp

    \Description
        This source contains the methods definition to manage the Garbage Collection.

    \Notes
        None.

    \Copyright
        (c) 2004 Electronic Arts Inc.


    \Version    1.0        02/19/04 (ONA) First Version
    \Version    1.1        11/01/04 (DC)  Added support for Dogma Allocator. This is conditionally compiled
                                          for the moment, but those conditionals we likely be removed
                                          in due time.
    \Version    1.2        12/05/07 (ASK) Added new functions that only replace AptCIHs that are GC-marked with
                                          with undefined. This improve the performance of CleanRemList
                                          (RemoveReferences) a lot.
                                          Also added some comments in function headers.
    \Version    1.3        01/31/08 (RB)  Added missing function prototypes, or made functions static where possible.
    \Version    1.4        03/18/08 (RB)  Made XMLReferenceRegistrationCb static.
*/
/************************************************************************************************H*/

#include "_Apt.h"
#include "AptTarget.h"
#include "AptAnimationTarget.h"
#include "AptGlobal.h"
#include "AptValue/AptValueVector.h"
#include "AptValue/AptBoolean.h"
#include "AptValue/AptInteger.h"
#include "AptValue/AptFloat.h"
#include "MainInline.h"

#include "AptValueGCAllocator.h"

#include "AptProfile.h"
extern AptValueGC_PoolManager  * gpGCPoolManager;

void RemoveReferences(AptValue ** pAllocatedValues, int32_t nItemsInArray);
#if defined(APT_XML_MEMORY_DUMP_SUPPORTED)
void PrintObjectMapXML(const char * psDumpName);
#endif // APT_XML_MEMORY_DUMP_SUPPORTED

// FIFA_BEGIN | SENDBACK | nmeyer | 2019-08-16 | Fix bug in APT GC where removing references to an APT object can result in deleting memory, thus altering the allocation list
#include <EASTL/hash_map.h>
typedef eastl::hash_map<AptValue *, void *> allocMap;
allocMap g_pAllocatedValues;
// FIFA_END

/*F************************************************************************************************/
/*!
    Initializes the Garbage Collection.

    \Input      None.

    \Output     None.

    \Notes
        None.

    \Version
        1.0        02/19/04 (ONA) First Version

*/
/************************************************************************************************F*/
void    AptGC::Initialize()
{

}

/*F************************************************************************************************/
/*!
    \Function    AptGC::sReferenceRegistrationCb

    \Description
        Callback for marking the GC objects in first phase of garbage collection.

    \Input      pFromRef          - object from where this function is called.
    \Input      pToRef            - object for which this function is called.
    \Input      pReferenceName    - any reference name for debugging purpose.
    \Input      bFlag             - flags indicating where pToRef is.

    \Output
        None.

    \Notes
        None.

    \Version    1.0        09/28/04 (DC) Bringin' it to ya
    \Version    1.1        12/05/07 (ASK) Added check so that non-GC objects will not get marked.
                                          This was added as a part of CleanRemList optimization

*/
/************************************************************************************************F*/
void    AptGC::sReferenceRegistrationCb(const AptValue * pFromRef, AptValue * &pToRef, const char * pReferenceName, int32_t bFlag)
{
    if(!pToRef->getGCMark())
    {
        if (pToRef->IsGarbageCollected())
        {
            APT_ASSERT(pToRef->mValueBitfield.mbIsAllocated && "Reached deallocated item in Garbage Collection list!");
            pToRef->setGCMark(true);
            pToRef->RegisterReferences();
        }
    }
}

/*F************************************************************************************************/
/*!
    \Function    AptGC::CleanUnreachable

    \Description
        This will delete any object not reachable by a "root" object.

    This just calls RegisterReferences on all root objects then AptRegisterGlobalReferences.
    These should do the job of recursively marking all of the objects that they touch. At the end
    of this we call DestroyGCPointers on all un-marked objects, after this we delete them. Note that
    we need to call DestroyGCPointers on all objects before we start deleting any of them this is
    because DestroyGCPointers will likely touch other objects that are going to be deleted.
    Note also that we have to prevent the AptValue release function from deleting Garbage collected
    objects while we call the Destroy GC objects, this is for the same reason, if we do not
    DestroyGCPointers will start touching objects previously deleted.

    Remember, Having two forms of deletion at the same time, (reference counting and garbage
    collecting) can cause issues that we need to be aware of. Also complicating the matter is that
    some objects are solely reference counted, others are solely garbage collected. Just be sure to
    clean up all of your references in DestroyGCPointers and don't delete any objects that are GC'd
    until all of the references are cleaned and you will be fined.

    \Input
        None.

    \Output
        None.

    \Notes
        None.

    \Version    1.0        09/09/04 (DC) Bringin' it to ya
    \Version    1.1        11/01/04 (DC) Added support for internal dogma allocator.
    \Version    1.2        12/16/05 (ASK) #1026 - Redundant code in AptGC.cpp's AptGC::CleanUnreachable(). if(!pObject->getGCMark())
*/
/************************************************************************************************F*/
void    AptGC::CleanUnreachable()
{
#if defined (EA_DEBUG) && !defined(DO_COVERAGE)
    int         nNumDeleted = 0;
#endif
    APT_PROFILE_BEGIN( APT_GC_CLEAN_UNREACHABLE, "AptGC::CleanUnreachable" );

    gpValuesToRelease->ReleaseValues();

    //PrintObjectMapXML("BeforeGC");

    // Step 1.
    {
        //  Now find all the roots and mark them recursively
        AptValue *  pObject = gpGCPoolManager->GetFirstAptValue();

        // Setup the callback if you are using one.
        void (*pOldValue) (const AptValue * pFromRef, AptValue * &pToRef, const char * pReferenceName, int32_t bFlag) = AptValue::sReferenceRegistrationCb;
        AptValue::sReferenceRegistrationCb = sReferenceRegistrationCb;

        while (pObject != NULL)
        {
#if (APT_DEBUG_LEVEL >= APT_DEBUG_MAXIMUM)
            APT_ASSERT( pObject->getRefCount() > 0);
#endif
            if (pObject->getGCRoot() != 0)
            {
                if (pObject->getGCMark() == false)
                {
                    pObject->setGCMark(true);
                    pObject->RegisterReferences();
                }
            }
            pObject = gpGCPoolManager->GetNextAptValue(pObject);
        }

        //  Mark from all the other traceable objects
        AptRegisterGlobalReferences();

        // return the original Callback if needed.
        AptValue::sReferenceRegistrationCb = pOldValue;
    }

    // Step 2.
    {
        AptValue *  pObject = gpGCPoolManager->GetFirstAptValue();

        // Clean up all references on objects that are about to be deleted. Note that this
        // can theoretically cause a number of marked objects to be deleted immediately (RefCounts=0).
        bool bOldVal = AptValue::sbSuspendRefcountDeletions; // Disable Defered vector for the process.
        AptValue::sbSuspendRefcountDeletions = true;

        while (pObject != NULL)
        {
            if (pObject->getGCMark() == false)
            {
                //  We need to sweep the object
                pObject->PreDestroy();
                pObject->DestroyGCPointers();
            }

            // No need to cache the next apt value, this one isn't being deleted yet.
            pObject = gpGCPoolManager->GetNextAptValue(pObject);
        }

        AptValue::sbSuspendRefcountDeletions = bOldVal;

    }

    // Step 3.
    // Sweep all the unmarked objects
    {
        AptValue *  pObject = gpGCPoolManager->GetFirstAptValue();
        while (pObject != NULL)
        {
            if (pObject->getGCMark() == false)
            {
#if (APT_DEBUG_LEVEL >= APT_DEBUG_MAXIMUM)
                APT_ASSERT(pObject->getRefCount() == 0);
#endif

#if defined (EA_DEBUG) && !defined(DO_COVERAGE)
                ++nNumDeleted;
#endif
                // EATech incident #108117  (JP) 2/2
                AptValue *  pNext = gpGCPoolManager->GetNextAptValue(pObject);
                pObject->DeleteThis();
                pObject = pNext;
            }
            else
            {
#if (APT_DEBUG_LEVEL >= APT_DEBUG_MAXIMUM)
                APT_ASSERT( pObject->getRefCount() > 0 );
#endif

                // Clear it back out to prepare for the next GC run.
                pObject->setGCMark(false);

                pObject = gpGCPoolManager->GetNextAptValue(pObject);
            }
        }
    }

    gpValuesToRelease->ReleaseValues();

#if defined (EA_DEBUG) && !defined(DO_COVERAGE)
    if (nNumDeleted)
    {
        // Don't let this message screw up the test suite..
        APT_DEBUGPRINT(APT_DEBUG_MSG_DEBUG_LVL, "Apt-GC---------------------- Sweeping of %d objects\n", nNumDeleted);
    }
#endif

    //  At the end of the garbage collection, we also release the base type pools
    AptInteger::ClearPool();
    AptFloat::ClearPool();
    StringPool::ClearTemporaryPool();
    APT_PROFILE_END( APT_GC_CLEAN_UNREACHABLE );
}


/*F************************************************************************************************/
/*!
    \Function    AptGC::CleanAll

    \Description
        Deletes all Apt Objects.

    Note that we still need to destroy GC pointers to clean up objects that are only reference
    counted (and not GC'd).

    \Input
        None.

    \Output
        None.

    \Notes
        None.

    \Version    1.0        09/09/04 (DC) Bringin' it to ya

*/
/************************************************************************************************F*/
void    AptGC::CleanAll()
{
    AptValue *  pObject;
    int nNumDeleted = 0;

    gpValuesToRelease->ReleaseValues();


    //PrintObjectMapXML("BeforeCleanAll");

    {
        pObject = gpGCPoolManager->GetFirstAptValue();

        // Clean up all references on objects that are about to be deleted. Note that this
        // can theoretically cause a number of marked objects to be deleted immediately (RefCounts=0).
        bool bOldVal = AptValue::sbSuspendRefcountDeletions; // Disable Defered vector for the process.
        AptValue::sbSuspendRefcountDeletions = true;

        while (pObject != NULL)
        {
            //  We need to sweep the object
            pObject->PreDestroy();
            pObject->DestroyGCPointers();
            pObject = gpGCPoolManager->GetNextAptValue(pObject);
            nNumDeleted ++;
        }

        AptValue::sbSuspendRefcountDeletions = bOldVal;
    }


    gpValuesToRelease->ReleaseValues();

    pObject = gpGCPoolManager->GetFirstAptValue();

    while ( pObject != NULL)
    {
        #if (APT_DEBUG_LEVEL >= APT_DEBUG_NORMAL)
            // This is is necessary on debug builds to avoid asserts.
            pObject->SetDestroyedGC();
        #endif

        // EATech incident #108117  (JP) 1/2
        AptValue *  pNext = gpGCPoolManager->GetNextAptValue(pObject);
        pObject->DeleteThis();
        pObject = pNext;
    }

    gpValuesToRelease->ReleaseValues();

#if defined (EA_DEBUG) && !defined(DO_COVERAGE)
    // Don't print these messages on coverage builds, it screws up the tes suite.
    if (nNumDeleted)
    {
        // Don't let this message screw up the test suite..
        APT_DEBUGPRINT(APT_DEBUG_MSG_DEBUG_LVL, "Apt-GC-CleanAll------------- Cleaned %d objects\n", nNumDeleted);
    }
#endif

    // At the end of the garbage collection, we also release the base type pools
    AptInteger::ClearPool();
    AptFloat::ClearPool();
    StringPool::ClearTemporaryPool();
}


#if defined(APT_XML_MEMORY_DUMP_SUPPORTED)

bool gbDumpOnLoadAnimation = false;

static void XMLReferenceRegistrationCb(const AptValue * pFromRef, AptValue * &pToRef, const char * pReferenceName, int32_t bFlag)
{
    // All calls print message.
    APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, "<REFV2 FROM=\"%u\" TO=\"%u\" NAME=\"%s\"/>\n",
        pFromRef ? pFromRef->GetUniqueID() : 0xffffffff, pToRef->GetUniqueID(),  pReferenceName);
}
#endif // defined(APT_XML_MEMORY_DUMP_SUPPORTED)
/*F************************************************************************************************/
/*!
    Prints the entire memory .

    \Input      pTraceableObject  - The non-GC object to trace from.

    \Output     None.

    \Notes
    Define APT_XML_MEMORY_DUMP_SUPPORTED to use this function.

    \Version
    1.0        03/19/04 (ONA) First Version

*/
/************************************************************************************************F*/
static AptValue *gpRefernceValue = NULL;
static AptValue *gpRefernceValueReplace = NULL;
static int32_t  pnRefCount = 0;

/*F************************************************************************************************/
/*!
    \Function    ReferenceReplaceCb

    \Description
        Goes through all values in Apt and replaces a given reference with something else

    This function can be used to find all instances of one reference and replace it with anything other
    reference you want.  This is used by the AptValue::sReferenceRegistrationCb call back mechanism.  It uses
    gpRefernceValue as the value to check and gpRefernceValueReplace as the value to replace it with.

    \Input      pFromRef          - object from where this function is called.
    \Input      pToRef            - object for which this function is called.
    \Input      pReferenceName    - any reference name for debugging purpose.
    \Input      bFlag             - flags indicating where pToRef is.

    \Output
        None.

    \Notes
        None.

    \Version    1.0        04/05/05 (LEO) First Version
    \Version    1.1        05/19/05 (DC)  Now inc's Replacement value and dec's replaced value to ensure
                                            valid reference counts.

*/
/************************************************************************************************F*/
static void ReferenceReplaceCb(const AptValue * pFromRef, AptValue * &pToRef, const char * pReferenceName, int32_t bFlag)
{
    if(pToRef != gpRefernceValue)
        return;

    //pnRefCount--;
    APT_ASSERT(bFlag != APT_REFREG_IS_DISPLAYLIST);
#if !defined(EA_DEBUG)
    if(bFlag == APT_REFREG_IS_DISPLAYLIST)
    {
        APT_DEBUGPRINT(APT_DEBUG_MSG_WARNING_LVL, "THIS SHOULD NOT HAPPEN, PLEASE CONTACT APT TEAM");
        return;
    }
#endif

    // We are removing the reference, dec the object so that at the end we know we got them all.
    // But make sure not to dec the object until we are done with it.
    AptValue* pHoldCIH = pToRef;

    if(pToRef->isCIH(true))
    {
        if(gpRefernceValueReplace == NULL)
        {
            if(pFromRef == NULL || !pFromRef->isScriptFunction())
            {
                pToRef = (bFlag == APT_REFREG_IS_APTCIH) ? gpUndefinedCIH : (AptValue *)gpUndefinedValue;
            }
            else if(pFromRef != NULL && pFromRef->isScriptFunction())
            {
                if(pFromRef->c_scriptfunction()->mpParentAnim == pToRef)
                {
                    pToRef->c_cih(true)->DecZombieCount();
                }
                pToRef = gpUndefinedCIH;
            }
        }
        else
        {
            if(pFromRef == NULL || !pFromRef->isScriptFunction())
            {
                if(bFlag == APT_REFREG_IS_APTCIH && !gpRefernceValueReplace->isCIH())
                {
                    pToRef = gpUndefinedCIH;        // Can't do it!
                }
                else
                {
                    pToRef = (AptValue *)gpRefernceValueReplace;
                    APT_INC(gpRefernceValueReplace);
                    pnRefCount--;
                }
            }
            else if(pFromRef != NULL && pFromRef->isScriptFunction())
            {
                if((pFromRef->c_scriptfunction()->mpParentAnim == pToRef) && pToRef->c_cih(true)->GetCIHState() != AptCIH::AptCIHState_Zombie)
                {
                    pToRef->c_cih(true)->DecZombieCount();
                    pToRef = (!gpRefernceValueReplace->isCIH()) ? gpUndefinedCIH : (AptValue *)gpRefernceValueReplace;
                    if(pToRef == gpRefernceValueReplace)
                    {
                        APT_INC(gpRefernceValueReplace);
                    }
                }
            }
        }
    }
    else
    {
        if(gpRefernceValueReplace == NULL)
        {
            pToRef = (AptValue *)gpUndefinedValue;
        }
        else
        {
            pToRef = (AptValue *)gpRefernceValueReplace;
            APT_INC(gpRefernceValueReplace);
        }

    }
    APT_ASSERT(pnRefCount >= 0);
    if(pToRef != pHoldCIH)
    {
        APT_DEC(pHoldCIH);              // Decrement the object we replaced.
    }
    else
    {
        // IF we didn't replace anything, make sure we had a good reason.
        APT_ASSERT(pToRef->c_cih(true)->GetCIHState() == AptCIH::AptCIHState_Zombie);
    }

#if (APT_DEBUG_LEVEL >= APT_DEBUG_NORMAL)
    for(int32_t i = 0; i < GetTargetSim()->GetAnimationTarget()->GetDelayedReleaseListSize(); i++)
    {
        if(GetTargetSim()->GetAnimationTarget()->GetDelayedReleaseList()[i] != NULL)
        {
            if(GetTargetSim()->GetAnimationTarget()->GetDelayedReleaseList()[i]->isCIH())
            {
                APT_ASSERT(GetTargetSim()->GetAnimationTarget()->GetDelayedReleaseList()[i]->GetDisplayListNext() == NULL && GetTargetSim()->GetAnimationTarget()->GetDelayedReleaseList()[i]->GetDisplayListPrevious()  == NULL);
            }
        }
    }
#endif
}

/*F************************************************************************************************/
/*!
    \Function    ReplaceReferences

    \Description
        Used to replace all references of pCIH with pNewObj

    \Input      *pValue           - pointer to object that needs to be replaced.
    \Input      *pNewObj          - pointer to new object
    \Input      ** pAllocatedValues - pointer to liner array of all GC values.
    \Input      nItemsInArray       - number of items in linear array.

    \Output
        None.

    \Notes
        Not used any more - after adding RemoveReferences function.

    \Version    1.0        04/05/05 (LEO) First Version

*/
/************************************************************************************************F*/
static int32_t snUseNewWay = 1;
static int32_t nTemp1 = 0;
static int32_t nTemp2 = 0;
void AptGC::ReplaceReferences(AptValue *pValue, AptValue *pNewObj, AptValue ** pAllocatedValues, int32_t nItemsInArray)
{
    gpRefernceValue = pValue;
    gpRefernceValueReplace = pNewObj;
    pnRefCount = pValue->getRefCount();
    uint32_t nTempRefCount = pnRefCount;

    AptValue *pObject = gpGCPoolManager->GetFirstAptValue();
    void (*pOldValue) (const AptValue * pFromRef, AptValue * &pToRef, const char * pReferenceName, int32_t bFlag) = AptValue::sReferenceRegistrationCb;
    AptValue::sReferenceRegistrationCb = ReferenceReplaceCb;


    // first try to go through various pools in animationpooldata and then see if there is need to go thru all objects.
    GetTargetSim()->GetAnimationTarget()->RemoveCIHReferences();
    AptRegisterGlobalReferences();

    nTemp1 = 0;
    nTemp2 = 0;

    if (pValue->getRefCount() > 1)
    {
    if (snUseNewWay == 0 || pAllocatedValues == NULL)
    {
#if defined(EA_DEBUG)       // For Debugging, run through everything to make sure we don't have a ref count issue
        while (pObject != NULL)
#else
        while (pObject != NULL && pnRefCount > 0)
#endif
        {
            pObject->RegisterReferences();
            pObject = gpGCPoolManager->GetNextAptValue(pObject);
            nTemp1++;
            if (pValue->getRefCount() == 1)
            {
                break ;
            }
        }
    }
    else
    {
        //AptValue ** pAllocatedValues = gpGCPoolManager->GetAllAllocatedAptValues();
        for (int32_t nArrayIndex=0; nArrayIndex < (int32_t) nItemsInArray; nArrayIndex++)
        {
            pObject = pAllocatedValues[nArrayIndex];
            if (pObject != NULL && pObject->mValueBitfield.mbIsAllocated)
            {
                pObject->RegisterReferences();
                nTemp2++;
                if (pValue->getRefCount() == 1 || pnRefCount == 0)
                {
                    break ;
                }
            }
            if (pObject == pValue)
            {
                pAllocatedValues[nArrayIndex] = NULL;
            }
        }
        //gpGCPoolManager->ReleaseAllocatedAptValuesArray(pAllocatedValues);
    }
    }


    //GetTargetSim()->GetAnimationTarget()->RemoveCIHReferences();
    //AptRegisterGlobalReferences();

    //APT_DEBUGPRINT(APT_DEBUG_MSG_DEBUG_LVL, "####### Replaced obj has [%i] references and [%i] found\n", nTempRefCount, nTempRefCount - pnRefCount);

    if(pNewObj != NULL)
    {
        pNewObj->setRefCount(pNewObj->getRefCount() + (nTempRefCount - pnRefCount));            // Set ref count to the replaced Object's ref count
    }

    gpRefernceValue = NULL;
    gpRefernceValueReplace = NULL;
    pnRefCount = 0;

    // return the original Callback if needed.
    AptValue::sReferenceRegistrationCb = pOldValue;
}

/*F************************************************************************************************/
/*!
    \Function    RemoveReferenceRegistrationCb

    \Description
        Goes through all values in Apt and removes AptCIH references that have been GC-marked and are on
        the animation target's delayed release list

    \Input      pFromRef          - value from which function is called
    \Input      &pToRef           - value that needs to be replaced
    \Input      pReferenceName    - Reference name of object
    \Input      bFlag             - Flags to indicate from where/what type of operation is done.

    \Output
        None.

    \Notes
        This function is pretty much same as ReferenceReplaceCb but only works on AptCIHs that
        have been GC-marked and need to be deleted.

    \Version    1.0        12/05/07 (ASK) Optimized version that works on GCMark to bulk-delete references
*/
/************************************************************************************************F*/
static void RemoveReferenceRegistrationCb(const AptValue * pFromRef, AptValue * &pToRef, const char * pReferenceName, int32_t bFlag)
{
    if(pToRef->getGCMark() == false)
    {
        return;
    }

#if defined(EA_DEBUG)
    {
        // the only items that should be marked are items on the delayed release list
        AptCIH** listPtrForToRef = NULL;
        AptCIH** delayedReleaseList = AptAnimationTarget::GetDelayedReleaseList();
        int32_t  delayedReleaseListSize = AptAnimationTarget::GetDelayedReleaseListSize();
        for (int32_t idxReleaseList = 0; idxReleaseList < delayedReleaseListSize; idxReleaseList++)
        {
            if (pToRef == delayedReleaseList[idxReleaseList])
            {
                listPtrForToRef = &delayedReleaseList[idxReleaseList];
                break;
            }
        }

        APT_ASSERT(listPtrForToRef != NULL);
    }
#endif

    //pnRefCount--;
    APT_ASSERT(bFlag != APT_REFREG_IS_DISPLAYLIST);
#if !defined(EA_DEBUG)
    if(bFlag == APT_REFREG_IS_DISPLAYLIST)
    {
        APT_DEBUGPRINT(APT_DEBUG_MSG_WARNING_LVL, "THIS SHOULD NOT HAPPEN, PLEASE CONTACT APT TEAM");
        return;
    }
#endif

    // We are removing the reference, dec the object so that at the end we know we got them all.
    // But make sure not to dec the object until we are done with it.
    AptValue* pHoldCIH = pToRef;

    if(pToRef->isCIH(true))
    {
        if(gpRefernceValueReplace == NULL)
        {
            if(pFromRef == NULL || !pFromRef->isScriptFunction())
            {
                pToRef = (bFlag == APT_REFREG_IS_APTCIH) ? gpUndefinedCIH : (AptValue *)gpUndefinedValue;
            }
            else if(pFromRef != NULL && pFromRef->isScriptFunction())
            {
                if(pFromRef->c_scriptfunction()->mpParentAnim == pToRef)
                {
                    pToRef->c_cih(true)->DecZombieCount();
                }
                pToRef = gpUndefinedCIH;
            }
        }
        else
        {
            if(pFromRef == NULL || !pFromRef->isScriptFunction())
            {
                if(bFlag == APT_REFREG_IS_APTCIH && !gpRefernceValueReplace->isCIH())
                {
                    pToRef = gpUndefinedCIH;        // Can't do it!
                }
                else
                {
                    pToRef = (AptValue *)gpRefernceValueReplace;
                    APT_INC(gpRefernceValueReplace);
                    pnRefCount--;
                }
            }
            else if(pFromRef != NULL && pFromRef->isScriptFunction())
            {
                if((pFromRef->c_scriptfunction()->mpParentAnim == pToRef) && pToRef->c_cih(true)->GetCIHState() != AptCIH::AptCIHState_Zombie)
                {
                    pToRef->c_cih(true)->DecZombieCount();
                    pToRef = (!gpRefernceValueReplace->isCIH()) ? gpUndefinedCIH : (AptValue *)gpRefernceValueReplace;
                    if(pToRef == gpRefernceValueReplace)
                    {
                        APT_INC(gpRefernceValueReplace);
                    }
                }
            }
        }
    }
    else
    {
        if(gpRefernceValueReplace == NULL)
        {
            pToRef = (AptValue *)gpUndefinedValue;
        }
        else
        {
            pToRef = (AptValue *)gpRefernceValueReplace;
            APT_INC(gpRefernceValueReplace);
        }

    }
    APT_ASSERT(pnRefCount >= 0);
    if(pToRef != pHoldCIH)
    {
        if (pHoldCIH->getRefCount() == 1)
        {
            // get pointer to pToRef in the sapDelayedReleaseList so we can remove it from the list
            AptCIH** listPtrForToRef = NULL;
            AptAnimationCihList& delayedReleaseList = AptAnimationTarget::GetDelayedReleaseList();
            int32_t  delayedReleaseListSize = AptAnimationTarget::GetDelayedReleaseListSize();
            for (int32_t idxReleaseList = 0; idxReleaseList < delayedReleaseListSize; idxReleaseList++)
            {
                if (pHoldCIH == delayedReleaseList[idxReleaseList])
                {
                    listPtrForToRef = &delayedReleaseList[idxReleaseList];
                    break;
                }
            }

            APT_ASSERT(listPtrForToRef != NULL);

            *listPtrForToRef = NULL;

			// FIFA_BEGIN | SENDBACK | nmeyer | 2019-08-16 | Fix bug in APT GC where removing references to an APT object can result in deleting memory, thus altering the allocation list
			allocMap::iterator allocIter = g_pAllocatedValues.find(pHoldCIH);
			if (allocIter != g_pAllocatedValues.end())
			{
				*((uintptr_t *)allocIter->second) = 0;
			}
			// FIFA_END
        }

        APT_DEC(pHoldCIH);              // Decrement the object we replaced.
    }
    else
    {
        // IF we didn't replace anything, make sure we had a good reason.
        APT_ASSERT(pToRef->c_cih(true)->GetCIHState() == AptCIH::AptCIHState_Zombie);
    }

#if (APT_DEBUG_LEVEL >= APT_DEBUG_NORMAL)
    for(int32_t i = 0; i < GetTargetSim()->GetAnimationTarget()->GetDelayedReleaseListSize(); i++)
    {
        if(GetTargetSim()->GetAnimationTarget()->GetDelayedReleaseList()[i] != NULL)
        {
            if(GetTargetSim()->GetAnimationTarget()->GetDelayedReleaseList()[i]->isCIH())
            {
                APT_ASSERT(GetTargetSim()->GetAnimationTarget()->GetDelayedReleaseList()[i]->GetDisplayListNext() == NULL && GetTargetSim()->GetAnimationTarget()->GetDelayedReleaseList()[i]->GetDisplayListPrevious()  == NULL);
            }
        }
    }
#endif
}


/*F************************************************************************************************/
/*!
    \Function    RemoveReferences

    \Description
        Used to remove all references to items in Animation Target's Delayed Release list.
        NOTE: Items in the delayed release list should first have their GC mark set to true.

    \Input      *pAllocatedValues - all possible objects that could have a reference
    \Input      nItemsInArray     - number of items in the array.

    \Output
        None.

    \Notes
        This gets called from AptAnimationTarget::CleanRemList() to replace all the references
        to AptCIHs that are getting deleted.

       - Basically all the GC object pointers are stored in a linera array
       - RegisterReferences is called on all those objects which recursively goes thru all the
         GC objects and call RemoveReferenceRegistrationCb thru APT_REGISTER_REFERENCE_XXX calls.
       - RemoveReferenceRegistrationCb only replaces AptCIHs that has GC-mark and ignore all other
         values.
       - This gives tremendous performance improvement for the operation of AptAnimationTarget::CleanRemList()
         especially when screen are shutting down and lot of AptCIHs are getting deleted but
         do not get deleted thru natural reference counting mechanism and needs to be replaced
         everywhere.
       - Previously we use to call ReplaceReferences on every AptCIH and it use to go thru all
         all GCed object for every AptCIHs that needs to be deleted.

    \Version    1.0        11/21/07 (tmp) First Version

*/
/************************************************************************************************F*/
void RemoveReferences(AptValue ** pAllocatedValues, int32_t nItemsInArray)
{
	// FIFA_BEGIN | SENDBACK | nmeyer | 2019-08-16 | Fix bug in APT GC where removing references to an APT object can result in deleting memory, thus altering the allocation list
	for (int32_t i = 0; i < nItemsInArray; ++i)
	{
		g_pAllocatedValues[pAllocatedValues[i]] = &pAllocatedValues[i];
	}
	// FIFA_END

    gpRefernceValue = NULL;
    gpRefernceValueReplace = NULL;

    void (*oldCallback) (const AptValue * pFromRef, AptValue * &pToRef, const char * pReferenceName, int32_t bFlag) = AptValue::sReferenceRegistrationCb;
    AptValue::sReferenceRegistrationCb = RemoveReferenceRegistrationCb;

    // first try to go through various pools in animationpooldata and then see if there is need to go thru all objects.
    GetTargetSim()->GetAnimationTarget()->RemoveCIHReferences();
    AptRegisterGlobalReferences();

    for (int32_t nArrayIndex=0; nArrayIndex < nItemsInArray; nArrayIndex++)
    {
        AptValue* pObject = pAllocatedValues[nArrayIndex];
        if (pObject != NULL && pObject->mValueBitfield.mbIsAllocated)
        {
            pObject->RegisterReferences();
        }
    }

	// FIFA_BEGIN | SENDBACK | nmeyer | 2019-08-16 | Fix bug in APT GC where removing references to an APT object can result in deleting memory, thus altering the allocation list
	g_pAllocatedValues.clear();
	// FIFA_END

    // return the original Callback if needed.
    AptValue::sReferenceRegistrationCb = oldCallback;
}


#if defined(APT_XML_MEMORY_DUMP_SUPPORTED)
void    PrintObjectMapXML(const char * psDumpName)
{
    AptValue *  pObject;
    static int  gnXMLDumpIndex=0;

    APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, "<DUMP NAME=\"%s\" INDEX=\"%d\" FORMAT=\"2\">\n", psDumpName, gnXMLDumpIndex++);

    // Setup the callback if you are using one.
    void (*pOldValue) (const AptValue * pFromRef, AptValue * &pToRef, const char * pReferenceName, int32_t bFlag) = AptValue::sReferenceRegistrationCb;
    AptValue::sReferenceRegistrationCb = XMLReferenceRegistrationCb;

    pObject = gpGCPoolManager->GetFirstAptValue();

    while (pObject != NULL)
    {
        if(pObject->isScriptFunction() || pObject->isCIH())             // We want the swf file name from which these are defined in
        {
            AptCharacterAnimationInst *pAnim = NULL;
            AptNativeString sDesc;
            if(pObject->isCIH())
            {
                AptCIH * pCIHObject = pObject->c_cih();
                AptCIH * pCIHRoot   = pCIHObject->GetRootAnimation();
                if(pCIHRoot->IsAnimationInst())
                {
                    pAnim = pCIHRoot->GetAnimationInst();
                }
                sDesc = pCIHObject->GetInstanceName();
            }
            else
            {
                AptScriptFunctionBase * pSF = pObject->c_scriptfunction();
                const char * pName = pSF->GetName();
                pAnim = pSF->mpParentAnim->GetAnimationInst();
                sDesc = pName ? pName : "";
            }
            APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, "<NODE ID=\"%u\" PTR=\"0x%X\" TYPE=\"%s\" REFCOUNT=\"%d\" GCROOT=\"%s\" FILENAME=\"%s.swf\" DISCRIPTION=\"%s\"/>\n",
                pObject->GetUniqueID(),
                pObject,
                AptGetTypeOfAptValue(pObject),
                pObject->getRefCount(),
                pObject->getGCRoot() ? "TRUE" : "FALSE",
                pAnim ? pAnim->mpFile.Get()->GetName().c_str() : "",
                sDesc.c_str());
        }
        else
        {
                APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, "<NODE ID=\"%u\" PTR=\"0x%X\" TYPE=\"%s\" REFCOUNT=\"%d\" GCROOT=\"%s\"/>\n",
                    pObject->GetUniqueID(),
                    pObject,
                    AptGetTypeOfAptValue(pObject),
                    pObject->getRefCount(),
                    pObject->getGCRoot() ? "TRUE" : "FALSE" );
        }
        pObject = gpGCPoolManager->GetNextAptValue(pObject);
    }

    pObject = gpGCPoolManager->GetFirstAptValue();

    while (pObject != NULL)
    {
        pObject->RegisterReferences();
        pObject = gpGCPoolManager->GetNextAptValue(pObject);
    }

    //  Mark from all the others traceables objects
    AptRegisterGlobalReferences();


    // return the original Callback if needed.
    AptValue::sReferenceRegistrationCb = pOldValue;

    APT_DEBUGPRINT(APT_DEBUG_MSG_DEFAULT_LVL, "</DUMP>\n");

}

const int32_t   MAX_ZOMBIE_TREE_LEVELS = 10;

static AptValue *ZombieTestRefernce = NULL;
static char     s_aTabs[MAX_ZOMBIE_TREE_LEVELS];
static int32_t  s_iLevel = 0;
static bool     s_bIsRef = false;


/*F************************************************************************************************/
/*!
    \Function    ZombieReferenceRegistrationCb

    \Description
        Prints out debugging info for all the external references found in the zombie AptCIH

    Insert your long description here.

    \Input      pFromRef          - object from where this function is called.
    \Input      pToRef            - object for which this function is called.
    \Input      pReferenceName    - any reference name for debugging purpose.
    \Input      bFlag             - flags indicating where pToRef is.

    \Output
        None.

    \Notes
        None.

    \Version    1.0        11/09/04 (LEO) First Version
                    - Updated for newer garbage collection stuff

*/
/************************************************************************************************F*/
static void ZombieReferenceRegistrationCb(const AptValue * pFromRef, AptValue * &pToRef, const char * pReferenceName, int32_t bFlag)
{
    if(pToRef == ZombieTestRefernce)
    {
        if(pFromRef == NULL)
        {
#if defined(DO_COVERAGE)
            APT_DEBUGPRINT(APT_DEBUG_MSG_DEBUG_LVL, "%\t\t%s<REFLEVEL%d FROM=\"omitted for coverage build\" TO=\"omitted for coverage build\" NAME=\"%s\"/>\n",
                s_aTabs,  s_iLevel + 1, pReferenceName);
#else
            APT_DEBUGPRINT(APT_DEBUG_MSG_DEBUG_LVL, "\t\t%s<REFLEVEL%d FROM=\"%u\" TO=\"%u\" NAME=\"%s\"/>\n",
                s_aTabs,  s_iLevel + 1, pFromRef ? pFromRef->GetUniqueID() : 0xffffffff, pToRef->GetUniqueID(),  pReferenceName);
#endif
        }
        else
        {
            const AptValue * pObject = pFromRef;
#if defined(DO_COVERAGE)
            APT_DEBUGPRINT(APT_DEBUG_MSG_DEBUG_LVL, "\t\t%s<REFLEVEL%d ID=\"omitted for coverage build\" PTR=\"omitted for coverage build\" TYPE=\"%s\" REFCOUNT=\"%d\" DESCRIPTION=\"%s\"%c>\n",
                s_aTabs, s_iLevel + 1,
                AptGetTypeOfAptValue(pObject),
                pObject->getRefCount(),
                pReferenceName,
                pObject->getGCRoot() ? '/' : ' ');
#else
            APT_DEBUGPRINT(APT_DEBUG_MSG_DEBUG_LVL, "\t\t%s<REFLEVEL%d ID=\"%u\" PTR=\"0x%X\" TYPE=\"%s\" REFCOUNT=\"%d\" DESCRIPTION=\"%s\"%c>\n",
                s_aTabs, s_iLevel + 1,
                pObject->GetUniqueID(),
                pObject,
                AptGetTypeOfAptValue(pObject),
                pObject->getRefCount(),
                pReferenceName,
                pObject->getGCRoot() ? '/' : ' ');
#endif
        }
        s_bIsRef = true;
    }
}

/*F************************************************************************************************/
/*!
    \Function     PrintLevelReferences

    \Description  Recursive function that prints out external references

    \Input        *pParent  -  Referenced AptValue

    \Output       None.

    \Notes        None.

    \Version      1.0      02/08/07 (JP) First Version
*/
/************************************************************************************************F*/

static void PrintLevelReferences(AptValue * pParent)
{
    for( AptValue * pObj = gpGCPoolManager->GetFirstAptValue(); pObj; pObj = gpGCPoolManager->GetNextAptValue(pObj))
    {
        if(pObj->getGCMark())
        {
            continue;
        }
        s_bIsRef = false;
        ZombieTestRefernce = pParent;
        s_aTabs[s_iLevel] = 0;
        pObj->RegisterReferences();

        if(s_bIsRef &&  !pObj->getGCRoot() && s_iLevel + 1 < MAX_ZOMBIE_TREE_LEVELS)
        {
            pObj->setGCMark(true);  // Don't take it again as root
            s_aTabs[s_iLevel++] = '\t';
            PrintLevelReferences(pObj); // Recursive enter to next level
            s_iLevel--;
        }
    }
    APT_DEBUGPRINT(APT_DEBUG_MSG_DEBUG_LVL, "\t%s</REFLEVEL%d>\n", s_aTabs, s_iLevel);
}

/*F************************************************************************************************/
/*!
    \Function    PrintExternalReferenceMapXML

    \Description
        Prints out debugging info for all the external references found in the zombie AptCIH

    \Input      *pParent          - pointer to movieclip

    \Output
        None.

    \Notes
        None.

    \Version    1.0        08/06/04 (LEO) First Version
                2.0        11/09/04 (LEO) Second Version
                    - Updated for newer garbage collection stuff
                2.1        02/08/07 (JP) Add recursive levels
*/
/************************************************************************************************F*/
void AptGC::PrintZombieExternalReferenceMapXML(AptCIH *pParent)
{
    // Setup the callback if you are using one.
    void (*pOldValue) (const AptValue * pFromRef, AptValue * &pToRef, const char * pReferenceName, int32_t bFlag) = AptValue::sReferenceRegistrationCb;
    AptValue::sReferenceRegistrationCb = ZombieReferenceRegistrationCb;

#if defined(DO_COVERAGE)
    APT_DEBUGPRINT(APT_DEBUG_MSG_DEBUG_LVL, "<EXTERNAL_REF_DUMP NAME=\"%s.swf\" ID=\"omitted for coverage build\" DESCRIPTION=\"%s\">\n", pParent->GetAnimationInst()->mpFile.Get()->GetName().c_str(), pParent->GetInstanceName().c_str());
#else
    APT_DEBUGPRINT(APT_DEBUG_MSG_DEBUG_LVL, "<EXTERNAL_REF_DUMP NAME=\"%s.swf\" ID=\"%u\" DESCRIPTION=\"%s\">\n", pParent->GetAnimationInst()->mpFile.Get()->GetName().c_str(), pParent->GetUniqueID(), pParent->GetInstanceName().c_str());
#endif

    for( AptValue * pObject = gpGCPoolManager->GetFirstAptValue(); pObject; pObject = gpGCPoolManager->GetNextAptValue(pObject))
    {
        if(pObject->isScriptFunction() && pObject->getRefCount() > 0 && pObject->c_scriptfunction()->mpParentAnim == pParent)
        {
            s_iLevel = 0;
            if(!pObject->getGCMark())  // Don't show it again if it was already shown in an upper level
            {
                const int32_t BUFFERSIZE = 128;
                int32_t bufferleft = BUFFERSIZE-1;
                char8_t buffer[BUFFERSIZE];
                buffer[0] = '\0';
                buffer[bufferleft] = '\0';
                for(uint32_t i = 0; i < pObject->c_scriptfunction()->GetNumArguments(); ++i)
                {
                    bool8_t isLastParam = (i+1 >= pObject->c_scriptfunction()->GetNumArguments());
                    bufferleft -= SNPRINTF(&buffer[BUFFERSIZE - bufferleft - 1], bufferleft, isLastParam ? "%s" : "%s, ", pObject->c_scriptfunction()->GetArgumentName(i));
                }
#if defined(DO_COVERAGE)
                APT_DEBUGPRINT(APT_DEBUG_MSG_DEBUG_LVL, "\t<REFLEVEL%d ID=\"omitted for coverage build\" PTR=\"omitted for coverage build\" TYPE=\"%s\" REFCOUNT=\"%d\" DESCRIPTION=\"%s\"%c>\n",
                APT_DEBUGPRINT(APT_DEBUG_MSG_DEBUG_LVL, "\t<REFLEVEL%d ID=\"omitted for coverage build\" PTR=\"omitted for coverage build\" TYPE=\"%s\" REFCOUNT=\"%d\" DESCRIPTION=\"%s(%s)\"%c>\n",
                    s_iLevel,
                    AptGetTypeOfAptValue(pObject),
                    pObject->getRefCount(),
                    pObject->c_scriptfunction()->GetName() == NULL ? "" : pObject->c_scriptfunction()->GetName(),
                    buffer,
                    pObject->getGCRoot() ? '/' : ' ');
#else
                APT_DEBUGPRINT(APT_DEBUG_MSG_DEBUG_LVL, "\t<REFLEVEL%d ID=\"%u\" PTR=\"0x%X\" TYPE=\"%s\" REFCOUNT=\"%d\" DESCRIPTION=\"%s(%s)\"%c>\n",
                    s_iLevel,
                    pObject->GetUniqueID(),
                    pObject,
                    AptGetTypeOfAptValue(pObject),
                    pObject->getRefCount(),
                    pObject->c_scriptfunction()->GetName() == NULL ? AptGetFunctionDebugName(pObject, "") : pObject->c_scriptfunction()->GetName(),
                    buffer,
                    pObject->getGCRoot() ? '/' : ' ');
#endif
                PrintLevelReferences(pObject);   // Recursively show the references
            }
            AptRegisterGlobalReferences();
        }
    }
    APT_DEBUGPRINT(APT_DEBUG_MSG_DEBUG_LVL, "</EXTERNAL_REF_DUMP>\n");

    for( AptValue * pObject = gpGCPoolManager->GetFirstAptValue(); pObject; pObject = gpGCPoolManager->GetNextAptValue(pObject))
    {
        pObject->setGCMark(false);
    }

    // return the original Callback if needed.
    AptValue::sReferenceRegistrationCb = pOldValue;
}
#endif


/*H*************************************************************************************************
EOF
*************************************************************************************************H*/
