/*H************************************************************************************************/
/*!

    \File
        AptInteger.h

    \Description
        Simple description of main purpose of file.

    \Notes
        None.

    \Copyright
        (c) 2004 Electronic Arts Inc.

    \Version
        1.0        03/06/04 (ONA) First Version

*/
/************************************************************************************************H*/

#ifndef __APTINTEGER_H__
#define __APTINTEGER_H__

#include "AptValue/AptValue.h"
// FIFA_BEGIN | SENDBACK | mwebb | 2015 - 05 - 26 | Adding Frostbite support
#include "Module.h"

//class AptInteger : public AptValueNoGC
class FB_APT_API AptInteger : public AptValueNoGC
// FIFA_END
{
protected:
    //  The user can't call new and delete, need to call Create() and Destroy()
    static void *operator new(size_t size);
    static void operator delete(void *p, size_t size);
    static void *operator new[](size_t size);
    static void operator delete[](void *p);

public:
    APT_DECLARE_OBJECT_METRICS

    //! Return the value of this object as a float
    APT_INLINE int      GetInt() const
    {
        return  (mnValue);
    }

    //! public form of the constructor will use the Float pool if enabled.
    static AptInteger * Create(const int nValue);
APT_ACCESS_INTERNAL:

    //! Clear the pool if enabled.
    static void         ClearPool();



    //! Override from AptValue
    virtual void        DeleteThis()
    {
        Destroy();
    }

    //! Override from AptValue
    virtual void        ForceDelete()
    {
        Destroy();
    }

    //! Either deletes the object or returns it to the pool, depending on how we are configured.
    void                Destroy();


protected:

    //! hide the destructor
    virtual ~AptInteger()
    {
        //  Do nothing...
    }

private:

    union
    {
        int             mnValue;
        AptInteger *    mpNextFree;
    };

    AptInteger();

    APT_INLINE explicit AptInteger(const int nValue):
        AptValueNoGC(AptVFT_Integer),
        mnValue(nValue)
    {
        //  Do nothing...
    }

    static AptInteger * spFirstFree;
};

#endif  //  __APTINTEGER_H__

/*H*************************************************************************************************
    EOF
*************************************************************************************************H*/

