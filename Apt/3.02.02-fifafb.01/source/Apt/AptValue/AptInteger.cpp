/*H************************************************************************************************/
/*!

    \File
        AptInteger.cpp

    \Description
        Simple description of main purpose of file.

    \Notes
        None.

    \Copyright
        (c) 2004 Electronic Arts Inc.

    \Version    1.0     03/06/04 (ONA) First Version
    \Version    1.1     06/26/09 (RB)  Lock needed when user doesn't create/destroy via AptValueFactory
    \Version    1.2     10/27/09 (RB)  Moved body of new and delete operators into this file.

*/
/************************************************************************************************H*/

#include "_Apt.h"
#include "AptValue/AptInteger.h"
#include "AptValue/AptValueVector.h"
#include "AptGlobal.h"
#include "MainInline.h"


AptInteger *    AptInteger::spFirstFree = NULL;

APT_DEFINE_OBJECT_METRICS(AptInteger, AptValueNoGC);


/*F************************************************************************************************/
/*!
    \Function    operator new

    \Description class-specific operator new

    \Input      size - size of object in bytes; passed by compiler

    \Returns    newly-created object allocated via non-garbage-collecting Dogma allocator

    \Version    1.0        10/27/09 (RB)  Moved from public header
*/
/************************************************************************************************F*/
void *AptInteger::operator new(size_t size)
{
    return APT_NONGC_NEW(size);
}

/*F************************************************************************************************/
/*!
    \Function    operator delete

    \Description class-specific operator delete

    \Input      p    - pointer to object to delete; passed by compiler
    \Input      size - size of object in bytes; passed by compiler

    \Returns    void

    \Version    1.0        10/27/09 (RB)  Moved from public header
*/
/************************************************************************************************F*/
void AptInteger::operator delete(void *p, size_t size)
{
    APT_NONGC_DELETE(p, size);
}


/*F************************************************************************************************/
/*!
    \Function    operator new[]

    \Description class-specific operator new[]

    \Input      size - size of allocation in bytes; passed by compiler

    \Returns    pointer to array of newly-created objects allocated via non-garbage-collecting Dogma allocator

    \Version    1.0        10/27/09 (RB)  Moved from public header
*/
/************************************************************************************************F*/
void *AptInteger::operator new[](size_t size)
{
    return AptNonGCAllocSaveSize( APT_ALLOC_PARAMS(size) );
}


/*F************************************************************************************************/
/*!
    \Function    operator delete[]

    \Description class-specific operator delete[]

    \Input      p    - pointer to array of object to delete; passed by compiler
    \Input      size - size of object in bytes; passed by compiler

    \Returns    void

    \Version    1.0        10/27/09 (RB)  Moved from public header
*/
/************************************************************************************************F*/
void AptInteger::operator delete[](void *p)
{
    AptValueNoGC::VerifyAptValueNoGC();
    AptNonGCFreeSavedSize(p);
}


/*F************************************************************************************************/
/*!
Insert your long description here.

\Input      bValue            -

\Output     AptBoolean *      -

\Notes
None.

\Version
            1.0        03/31/04 (ONA) First Version
            1.1        06/26/09 (RB)  Lock needed when caller isn't AptValueFactory
*/
/************************************************************************************************F*/
AptInteger *    AptInteger::Create(const int nValue)
{
    #if APT_AUTOLOCK_VALUES
    AptUpdateAutoLock lock;
    #else
    AptAssertIsSimulationThread();
    #endif

    #if APT_POOL_INTEGERS
    if (spFirstFree)
    {
        AptInteger *    pNewInt = spFirstFree; // Re-read inside the lock.
        if(pNewInt)
        {
            //  The type should be Integer
            APT_ASSERT(pNewInt->getVtblIndex() == AptVFT_Integer);
            //  The refCount should be equal to zero
            APT_ASSERT(pNewInt->getRefCount() == 0);

            // May not have been deleted from the deferred vector. Set Release At End flag.
            pNewInt->SetReleaseAtEnd();

            //  Push the value on the release vector
            //  So it will be recycled if possible
            gpValuesToRelease->PushValue(pNewInt);

            spFirstFree = pNewInt->mpNextFree;

            pNewInt->mnValue = nValue;

            return  (pNewInt);
        }
    }
    #endif

    return  (new AptInteger(nValue));
}

/*F************************************************************************************************/
/*!
Insert your long description here.

\Input      None.

\Output     None.

\Notes
None.

\Version
            1.0        03/31/04 (ONA) First Version
            1.1        06/26/09 (RB)  Lock needed when caller isn't AptValueFactory
*/
/************************************************************************************************F*/
void    AptInteger::Destroy()
{
    #if APT_AUTOLOCK_VALUES
    AptUpdateAutoLock lock;
    #endif

    #if APT_POOL_INTEGERS
    {
        //  Add to the free pool
        mpNextFree = spFirstFree;
        spFirstFree = this;
    }
    #else
    {
        delete this
    }
    #endif
}

/*F************************************************************************************************/
/*!
    Insert your long description here.

    \Input      None.

    \Output     None.

    \Notes
        None.

    \Version
        1.0        03/31/04 (ONA) First Version

*/
/************************************************************************************************F*/
void    AptInteger::ClearPool()
{
    #if APT_POOL_INTEGERS
    {
        while (spFirstFree != NULL)
        {
            AptInteger *    pNext = spFirstFree->mpNextFree;
            spFirstFree->DestroyGCPointers();
            delete  spFirstFree;    //  The only place where we do a real destruction
            spFirstFree = pNext;
        }
    }
    #endif
}

/*H*************************************************************************************************
    EOF
*************************************************************************************************H*/

