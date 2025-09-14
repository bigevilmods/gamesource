/*H************************************************************************************************/
/*!

    \File
        AptBoolean.h

    \Description
        Declares AptBoolean class

    \Notes
        None.

    \Copyright
        (c) 2004 Electronic Arts Inc.

    \Version  1.0   03/06/04 (ONA) First Version
    \Version  1.1   06/08/09 (RB)  Moved body of Create method into header because it's marked inline.
    \Version  1.2   10/27/09 (RB)  Replaced APT_VALUE_NEW_DELETE_OPERATORS with actual new/delete declaration.
*/
/************************************************************************************************H*/

#ifndef __APTBOOLEAN_H__
#define __APTBOOLEAN_H__

#include "AptValue/AptValue.h"
// FIFA_BEGIN | SENDBACK | mwebb | 2015 - 05 - 26 | Adding Frostbite support
#include "Module.h"

//class AptBoolean : public AptValueNoGC
class FB_APT_API AptBoolean : public AptValueNoGC
// FIFA_END
{
protected:
    //  The user can't call new and delete, need to call Create() and Destroy()
    static void *operator new(size_t size);
    static void operator delete(void *p, size_t size);
    static void *operator new[](size_t size) ;
    static void operator delete[](void *p);

public:
    APT_DECLARE_OBJECT_METRICS

    //! Return the boolean value of thie object.
    APT_INLINE bool GetBool() const;

    static AptBoolean * Create(const bool bValue);

APT_ACCESS_INTERNAL:
    AptBoolean() : AptValueNoGC(AptVFT_Boolean)
    {
        mbValue = false;
        setIsDefined(1);
        setRefCount(MAX_REFCOUNT);
    }

    static void Initialize();

    static void Shutdown();

    // This Optimizes global objects by doing nothing on Add/Release
#if defined(APT_INC_DEC_MESSAGES)
    virtual void    AddRef(const char * szFuncName, const char * szFileName, int nLineNumber)  { }   //  Do nothing...
    virtual void    Release(const char * szFuncName, const char * szFileName, int nLineNumber) { }   //  Do nothing...
#else
    virtual void    AddRef()  { }   //  Do nothing...
    virtual void    Release() { }   //  Do nothing...
#endif

protected:

    //! Hide the destructor
    APT_INLINE virtual ~AptBoolean()
    {
        // Do Nothing
    }

private:
    bool                mbValue;
    static AptBoolean   *spBooleanFalse;
    static AptBoolean   *spBooleanTrue;

    AptBoolean(bool bVal) : AptValueNoGC(AptVFT_Boolean)
    {
        mbValue = bVal;
        setIsDefined(1);
        setRefCount(MAX_REFCOUNT);
    }
};

#endif  //  __APTBOOLEAN_H__

/*H*************************************************************************************************
    EOF
*************************************************************************************************H*/

