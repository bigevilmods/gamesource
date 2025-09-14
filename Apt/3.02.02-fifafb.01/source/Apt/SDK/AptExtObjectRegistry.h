/*H************************************************************************************************/
/*!

    \file    AptExtObjectRegistry.h

    \brief   Encapsulates methods for registering and unregistering
             Apt Extension Objects.


    (c) 2008 Electronic Arts Inc.

    \version    1.0        02/06/08 (RB)  First version
*/
/************************************************************************************************H*/

#ifndef __AptExtObjectRegistry_h
#define __AptExtObjectRegistry_h

#include <EABase/eabase.h>
#include "AptExtObject.h"


/// This class is a singleton, never instantiated, used internally only by Apt.cpp in the
/// AptRegisterExtension and AptUnRegisterExtension API functions.
class AptExtObjectRegistry
{
public:

    /*F************************************************************************************************/
    /*!
        \Function    Register

        \Description
            Registers an Apt C++ extension object

        \Input      *pExtObject - Pointer to the object to register.

        \Output
            None.

        \Notes
            Works with instantiable AptExtObjects. Class name can include a namespace.

        \Version    1.0        02/06/08 (RB) First Version
    */
    /************************************************************************************************F*/
    static void Register( class AptExtObject *pExtObject );


    /*F************************************************************************************************/
    /*!
        \Function    UnRegister

        \Description
            UnRegisters an Apt C++ extension object

        \Input      *szName     - Name of class to unregister

        \Output
            None.

        \Version    1.0        02/06/08 (RB) First Version
    */
    /************************************************************************************************F*/
    static void UnRegister( const char8_t* szName );

private:

    typedef AptObject* (*ScopeFuncPtr)(const AptNativeString&, AptObject*);

    static int32_t    _ForEachScope( const AptNativeString& sObjName, AptObject** ppScope, ScopeFuncPtr pActionFunction );
    static AptObject* _CreateScope( const AptNativeString& sName, AptObject* pScope );
    static AptObject* _GetNextScope( const AptNativeString& sName, AptObject* pScope );
    static bool       _UnsetEmptyChild( AptObject* pScope, const AptNativeString& sFullName, int32_t i, int32_t j );

private:
    AptExtObjectRegistry();
    AptExtObjectRegistry( const AptExtObjectRegistry& );
    AptExtObjectRegistry& operator=( const AptExtObjectRegistry& );
};


#endif //__AptExtObjectRegistry_h
