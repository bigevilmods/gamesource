/*H************************************************************************************************/
/*!

    \File
        AptFloat.h

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

#ifndef __APTFLOAT_H__
#define __APTFLOAT_H__

#include "AptValue/AptValue.h"
// FIFA_BEGIN | SENDBACK | mwebb | 2015 - 05 - 26 | Adding Frostbite support
#include "Module.h"

//class AptFloat : public AptValueNoGC
class FB_APT_API AptFloat : public AptValueNoGC
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
    APT_INLINE float    GetFloat() const
    {
        return  (mfValue);
    }

    //! public form of the constructor will use the Float pool if enabled.
    static AptFloat *   Create(const float fValue);
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
    virtual ~AptFloat()
    {

    }

private:
    union
    {
        float       mfValue;
        AptFloat *  mpNextFree;
    };

    AptFloat();

    APT_INLINE explicit AptFloat(const float fValue):
        AptValueNoGC(AptVFT_Float),
        mfValue(fValue)
    {
        //  Do nothing...
    }

    static AptFloat *   spFirstFree;
};

#endif  //  __APTFLOAT_H__

/*H*************************************************************************************************
    EOF
*************************************************************************************************H*/

