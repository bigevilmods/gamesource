/*H************************************************************************************************/
/*!

    \File
        AptBoolean.cpp

    \Description
        Defines the AptBoolean class. Note, only two instances ever exist: one for true, and one for false.

    \Notes
        None.

    \Copyright
        (c) 2004 Electronic Arts Inc.

    \Version    1.0     03/06/04 (ONA) First Version
    \Version    1.1     10/27/09 (RB)  Moved body of new and delete operators into this file.

*/
/************************************************************************************************H*/

#include "AptValue/AptBoolean.h"
#include "_AptInternalDefines.h"
#include "MainInline.h"

#if !defined(APT_ENABLE_INLINE)
#include "AptValue/AptBoolean.inl"
#endif

AptBoolean *    AptBoolean::spBooleanFalse  = NULL;
AptBoolean *    AptBoolean::spBooleanTrue   = NULL;

APT_DEFINE_OBJECT_METRICS(AptBoolean, AptValueNoGC);


/*F************************************************************************************************/
/*!
    \Function    operator new

    \Description class-specific operator new

    \Input      size - size of object in bytes; passed by compiler

    \Returns    newly-created object allocated via non-garbage-collecting Dogma allocator

    \Version    1.0        10/27/09 (RB)  Moved from public header
*/
/************************************************************************************************F*/
void *AptBoolean::operator new(size_t size)
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
void AptBoolean::operator delete(void *p, size_t size)
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
void *AptBoolean::operator new[](size_t size)
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
void AptBoolean::operator delete[](void *p)
{
    AptValueNoGC::VerifyAptValueNoGC();
    AptNonGCFreeSavedSize(p);
}

/*F************************************************************************************************/
/*!
    \Function    AptBoolean::Initialize

    \Description
        Sets up the global true false AptBooleans

    \Input
        None.

    \Output
        None.

    \Notes
        None.

    \Version    1.0        07/20/05 (LEO) First Version

*/
/************************************************************************************************F*/
void AptBoolean::Initialize()
{
    if(spBooleanFalse == NULL || spBooleanTrue == NULL)
    {
        spBooleanFalse = new AptBoolean(false);
        spBooleanTrue  = new AptBoolean(true);
    }
}

/******************************************************************************/
/*!
    \brief  Simple factory function to create an AptBoolean

    \param  value   bool

    \return         - Corresponding AptBoolean
*/
/******************************************************************************/
AptBoolean * AptBoolean::Create(const bool value)
{
    return value ? spBooleanTrue : spBooleanFalse;
}

/*F************************************************************************************************/
/*!
    \Function    AptBoolean::Shutdown

    \Description
        Delete the global true false AptBooleans

    \Input
        None.

    \Output
        None.

    \Notes
        None.

    \Version    1.0        07/20/05 (LEO) First Version

*/
/************************************************************************************************F*/
void AptBoolean::Shutdown()
{
    if(spBooleanFalse != NULL)
    {
        spBooleanFalse->ForceDelete();
        spBooleanFalse = NULL;
    }
    if(spBooleanTrue != NULL)
    {
        spBooleanTrue->ForceDelete();
        spBooleanTrue = NULL;
    }
}

/*H*************************************************************************************************
    EOF
*************************************************************************************************H*/

