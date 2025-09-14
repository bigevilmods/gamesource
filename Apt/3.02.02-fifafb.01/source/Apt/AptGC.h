/*H************************************************************************************************/
/*!

    \File
        AptGC.h

    \Description
        This header contains the definition of the Garbage Collector.

    \Notes
        None.

    \Copyright
        (c) 2004 Electronic Arts Inc.

    \Version
        1.0        02/19/04 (ONA) First Version
        1.1        03/18/08 (RB)  Made ReplaceReferences and PrintZombieExternalReferenceMapXML static.
*/
/************************************************************************************************H*/

#ifndef __APTGC_H__
#define __APTGC_H__

#include "AptValue/AptValue.h"

class AptValue;

class AptGC
{
public:

    //  Prototypes for GC AptValue
    //  Prototypes for other traceable objects
    typedef int         (*GetObjectNumGCPointersCallback)(void * const pObject);
    typedef AptValue *  (*GetObjectGCPointerCallback)(void * const pObject, const int nIndex);

    static void Initialize();

    static bool         IsValueGarbageCollected(const AptValue * const pValue);

    static void         CleanUnreachable();
    static void         CleanAll();

    struct GCLinkedList
    {
        AptValue *  mPrev;
        AptValue *  mNext;
    };

    static void ReplaceReferences(AptValue *pValue, AptValue * pNewObj, AptValue ** pAllocatedValues = NULL, int32_t nItemsInArray = 0);
#if defined(APT_XML_MEMORY_DUMP_SUPPORTED)
    static void PrintZombieExternalReferenceMapXML(AptCIH *pParent);
#endif

private:
    struct RegisteredTraceObject;

    AptGC();
    explicit AptGC(const AptGC & Other);
    ~AptGC();
    AptGC & operator=(const AptGC & Other);

    static void sReferenceRegistrationCb(const AptValue * pFromRef, AptValue * &pToRef, const char * pReferenceName, int32_t bFlag);

#if defined(APT_XML_MEMORY_DUMP_SUPPORTED)
    // This function needs to be able to traverse the object list, it won't modify anything.
    friend void    PrintObjectMapXML(const char * psDumpName);
#endif

};

#endif  //  __APTGC_H__

/*H*************************************************************************************************
    EOF
*************************************************************************************************H*/

