/*F************************************************************************************************/
/*!
\File       TestAptExt.h

\Description

\Notes
None.

\Version    1.0        03/03/05 (DUG) First Version

*/
/************************************************************************************************F*/

#ifndef _TestAptExt_H_
#define _TestAptExt_H_

#include "AptExtObject.h"

#define APTEXT_MYOBJECT_NAME        ("TestAptExtObject")
#define APTEXT_MYOBJECT_NUMFNCS     (5)

class TestAptExt : public AptExtObject
{
public:
    TestAptExt()
        : AptExtObject(APTEXT_MYOBJECT_NUMFNCS)
    {}

    virtual ~TestAptExt() {}


    virtual const char *GetName(void) { return (APTEXT_MYOBJECT_NAME); }
    virtual void Initialize(void);

    // declare C++ extension functions
    APTEXT_FUNCTION_DECL(PingPongFunction)
    APTEXT_FUNCTION_DECL(LotrFunction)
    APTEXT_FUNCTION_DECL(CalculateFunction)
    APTEXT_FUNCTION_DECL(OnEnterFunction)
    APTEXT_FUNCTION_DECL(UnRegisterFunction)
};


#define APTEXT_MYBIGOBJECT_NAME        ("TestBigAptExtObject")
#define APTEXT_MYBIGOBJECT_NUMFNCS     (0)

class BigAptExt : public AptExtObject
{
public:
    BigAptExt()
        : AptExtObject(APTEXT_MYBIGOBJECT_NUMFNCS)
    {}

    virtual ~BigAptExt() {}



    virtual const char *GetName(void) { return (APTEXT_MYBIGOBJECT_NAME); }
    virtual void Initialize(void)
    {

    }

private:
    // Lets fatten this one up, shall we? (Make sure Apt can handle it)
    uint32_t lots_O_Fat[50];
};

#endif // _TestAptExt_H_
