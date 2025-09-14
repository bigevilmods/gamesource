/*H************************************************************************************************/
/*!

    \file    AptExtObjectRegistry.cpp

    \brief   Encapsulates methods for registering and unregistering
             Apt Extension Objects.


    (c) 2008 Electronic Arts Inc.

    \version    1.0        02/06/08 (RB)  First version
*/
/************************************************************************************************H*/

/*** Include files ********************************************************************************/

#include "SDK/AptExtObjectRegistry.h"
#include "AptObject/AptGlobalObject.h"
#include "AptObject/AptGlobalExtensionObject.h"
#include "_Apt.h"
#include "MainInline.h"

/*** Defines **************************************************************************************/

/*** Macros ***************************************************************************************/

/*** Type Definitions *****************************************************************************/

/*** Function Prototypes **************************************************************************/

/*** Variables ************************************************************************************/

// Private variables
// Public variables

/*** Public Functions *****************************************************************************/


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
void AptExtObjectRegistry::Register( AptExtObject *pExtObject )
{
    AptNativeString sObjName(pExtObject->GetName());

    // If the name contains '.' then we must build
    // up the namespace as a tree of Objects. This
    // is what Flash does when you declare a class
    // in ActionScript with a fully-qualified name.
    // For each scope (denoted by '.') in sObjName, create a nested object under _global.
    AptObject* pScope = gpGlobalGlobalObject;
    int32_t i = _ForEachScope( sObjName, &pScope, _CreateScope );

    // conveniently, i is 1 + the last index of '.' within sObjName
    if ( i > 0 )
    {
        // We are registering a class within a namespace.
        // Skip the namespace letters and parse out
        // just the class name.
        sObjName = sObjName.Mid(i);
    }
    else
    {
        // We are registering at the global scope,
        // not within a namespace.
        pScope = gpGlobalExtensionObject;
    }

    // if this assert fires, the object has already been registered
    APT_ASSERT(NULL == pScope->Lookup(&sObjName));
    pScope->Set(&sObjName, pExtObject);
}


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
void AptExtObjectRegistry::UnRegister( const char8_t* szName )
{
    APT_ASSERT(szName != NULL);

    AptNativeString sObjName(szName);

    // find the parent scope of the given class
    AptObject* pScope = gpGlobalGlobalObject;
    int32_t i = _ForEachScope( sObjName, &pScope, _GetNextScope );

    // conveniently, i is 1 + the last index of '.' within sObjName
    if ( i > 0 )
    {
        // We are unregistering a class within a namespace.
        // Skip the namespace letters and parse out
        // just the class name.
        sObjName = sObjName.Mid(i);
    }
    else
    {
        // We are registering at the global scope,
        // not within a namespace.
        pScope = gpGlobalExtensionObject;
    }

    // remove the AptExtObject from the scope
    AptValue * pExtObject = pScope->Lookup(&sObjName);
    APT_ASSERT(NULL != pExtObject);

    // Note sim thread was locked in AptUnRegisterExtension
    _APT_INC(pExtObject);
    pScope->GetNativeHashVirtual()->Unset(&sObjName);
    _APT_DEC(pExtObject);

    if ( i > 0 )
    {
        // tear down the namespace scopes that we set up when the object was registered
        AptNativeString sFullName(szName);
        _UnsetEmptyChild( gpGlobalGlobalObject, sFullName, 0, sFullName.Find('.')  );
    }
}


/*** Private Functions ****************************************************************************/


/*F************************************************************************************************/
/*!
    \Function    _ForEachScope

    \Description
                Given a scope such as "foo.bar.MyClass", this method performs the action  'pActionFunction' on
                each part of the namespace.  The action is either to create or get the next-deepest scope.

    \Input:  sName:   A scope name like "foo.bar.MyClass".
    \Input:  ppScope: Starting scope and place to store the return value.
    \Input:  pActionFunction:  Either _CreateScope or _GetNextScope.

    \Output: ppScope. The return value goes in here.

    \Version    1.0        02/06/08 (RB) First Version

    \Notes
       For example, if the action is to create:
       - First we create the scope "foo" in the global namespace
       - We determine the next-deepest scope, which is "bar". So we create the scope "bar" within "foo".
       - We determine the next-deepest scope, which is "MyClass".  We create that within "bar".
       - Note, a namespace in Apt is an Object. The result is a nested hierarchy of Objects, "foo.bar",
         which contains the AptExtObject "MyClass".
       - Note that the creation function (see _CreateScope below) handles already-existing scopes. So it's
         OK to create "foo.bar.MyClass" and then also create "foo.bar.YourClass"-- the sub-scope "bar"
         is only created once and we just add items to its nativehash.
       - We return the newly-created scope foo.bar as *ppScope.

       If the action is to get the next-deepest scope:
       - First we lookup "foo" in the global namespace
       - Next we lookup "bar" within "foo"
       - We return foo.bar as *ppScope.

*/
/************************************************************************************************F*/
int32_t AptExtObjectRegistry::_ForEachScope( const AptNativeString& sName, AptObject** ppScope, ScopeFuncPtr pActionFunction )
{
    int32_t i=0;
    for ( int32_t j=sName.Find('.'); (j >= 0) && *ppScope; j=sName.Find('.',j+1) )
    {
        // Get the next part of the namespace and
        // create or delete an Object in it.
        AptNativeString sSubStr = sName.Mid(i, j-i);
        AptObject* pNewScope = pActionFunction( sSubStr, *ppScope );
        *ppScope = pNewScope;
        i = j+1;
    }
    return *ppScope ? i : 0;
}


/*F************************************************************************************************/
/*!
    \Function    _CreateScope

    \Description
        This method creates a nested scope (named sName) within a parent scope (pScope).
        We use this method as an "action" passed as pActionFunction to _ForEachScope above.

    \Input:  sName:  The sub-part of a namespace (like "bar"); it won't contain any '.' and it won't be empty.
    \Input:  pScope: Scope to create the new scope in.

    \Output: The new, nested scope is returned.

    \Version    1.0        02/06/08 (RB) First Version
*/
/************************************************************************************************F*/
AptObject* AptExtObjectRegistry::_CreateScope( const AptNativeString& sName, AptObject* pScope )
{
    APT_ASSERT( sName.Size() > 0 );
    AptValue* pNewScope = pScope->Lookup(&sName);
    if ( !pNewScope )
    {
        pNewScope = new AptObject(AptVFT_Object);
        pScope->Set(&sName, pNewScope);
    }
    else
    {
        // If namespace collides with an existing class or movieclip name, then
        // that's OK, but it sure better be an Object.
        APT_ASSERT( pNewScope->isObject() );
    }
    return (pNewScope && pNewScope->isObject()) ? static_cast<AptObject*>(pNewScope) : NULL;
}


/*F************************************************************************************************/
/*!
    \Function    _GetNextScope

    \Description
        This method looks up the scope (named sName) within a parent scope (pScope).
        We use this method as an "action" passed as pActionFunction to _ForEachScope above.

    \Input:  sName:  The sub-part of a namespace (like "bar"); it won't contain any '.' and it won't be empty.
    \Input:  pScope: Scope to look in.

    \Output: The scope is returned if it's found; otherwise NULL.

    \Version    1.0        02/06/08 (RB) First Version
*/
/************************************************************************************************F*/
AptObject* AptExtObjectRegistry::_GetNextScope( const AptNativeString& sName, AptObject* pScope )
{
    AptValue* pNextScope = pScope ? pScope->Lookup(&sName) : NULL;
    return (pNextScope && pNextScope->isObject()) ? static_cast<AptObject*>(pNextScope) : NULL;
}


/*F************************************************************************************************/
/*!
    \Function    _UnsetEmptyChild

    \Description
        This is a recursive method that tears down the namespace hierarchy we created when an AptExtObject class
        was registered.

    \Input    pScope:     The scope to start in.
    \Input    sFullName:  The fully-qualified name of an AptExtObject class. It is constant during recursion. We
                          operate on child scopes by looking at substrings of sFullName.
                          The substring range is given by i and j. So, the substring given by sFullName[i .. j] is the
                          child scope we're interested in.

    \Output: ppScope. The return value goes in here.

    \Version    1.0        02/06/08 (RB) First Version

    \Notes
        This works fine even when part of the namespace has been created by script.  For example, if the script
        declares an object such as foo.yahoo.Junk before we register a single AptExtObject called foo.bar.MyWidget,
        then we only tear down the namespace foo.bar.*. We wouldn't touch foo.yahoo. So "foo" would remain even after
        this method completes.

*/
/************************************************************************************************F*/
bool AptExtObjectRegistry::_UnsetEmptyChild( AptObject* pScope, const AptNativeString& sFullName, int32_t i, int32_t j )
{
    //check that the substring start index isn't out of bounds
    if ( pScope && (i < static_cast<int32_t>( sFullName.GetLength() )) )
    {
        //check that the substring end index isn't out of bounds either
        if ( j < 0 )
            j = sFullName.GetLength();

        // get the substring that is the child scope,
        // and lookup that child scope.
        AptNativeString sChildName = sFullName.Mid( i, j-i );
        AptValue* pChild = pScope->Lookup( &sChildName );
        if ( pChild )
        {
            APT_ASSERT(pChild->isObject());
            if ( pChild->isObject() )
            {
                do
                {
                    // If the child scope has no children itself, then
                    // it can be deleted (we just need to call Unset).
                    // This is how the empty namespace hierarchy gets deleted.
                    if ( !pChild->GetNativeHashVirtual()->GetFirstItem() )
                    {
                        pScope->GetNativeHashVirtual()->Unset( &sChildName );
                        return true;
                    }

                    // Otherwise, if the child scope has children, then recurse and try
                    // to delete empty sub-scopes.
                }
                while( _UnsetEmptyChild( static_cast<AptObject*>(pChild), sFullName, j+1, sFullName.Find( '.', j+1 ) ) );
            }
        }
    }
    return false;
}
