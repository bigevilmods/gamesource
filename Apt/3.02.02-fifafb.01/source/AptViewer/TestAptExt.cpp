/*F************************************************************************************************/
/*!
\File       TestAptExt.cpp

\Description

\Notes
None.

\Version    1.0        03/03/05 (DUG) First Version

*/
/************************************************************************************************F*/


#include "Apt.h"
#include "AptDefine.h"
#include "AptValue/AptValue.h"
#include "TestAptExt.h"
#include "AptValueFactory.h"
//#include "..\Apt\MainInline.h"


void TestAptExt::Initialize(void)
{
    // initialize C++ extension functions
    APTEXT_FUNCTION_INIT(PingPongFunction)
    APTEXT_FUNCTION_INIT(LotrFunction)
    APTEXT_FUNCTION_INIT(CalculateFunction)
    APTEXT_FUNCTION_INIT(OnEnterFunction)
    APTEXT_FUNCTION_INIT(UnRegisterFunction)
}

// in actionscript, PingPongFunction takes 1 parameter and checks to see
// if it is equal to 5.  it returns true or false.
APTEXT_FUNCTION(TestAptExt, PingPongFunction)
{
    // get the parameter
    AptValue *pParam = GetParam(0);
    AptValue *pRetVal = NULL;
    bool8_t bIsEqualTo5 = false;

    // is it an integer
    APT_ASSERT(pParam != NULL);
    if (pParam->isInteger())
    {
        int32_t iParam = pParam->toInteger();
        bIsEqualTo5 = (iParam == 5);
    }

    // create the return value
    pRetVal = AptBoolean::Create(bIsEqualTo5);
    return pRetVal;
}

// in actionscript, LotrFunction takes an object as a parameter.  It looks
// up a count member in the object, and returns another object with count value
// in it.  If the object passed in does not have the count, it returns the
// undefined object
APTEXT_FUNCTION(TestAptExt, LotrFunction)
{
    // get the parameter
    AptValue *pInputObject = GetParam(0);
    AptValue *pRetVal = NULL;

    // lookup the count
    APT_ASSERT(pInputObject != NULL);
    AptNativeString sCount("m_sCount");
    AptValue *pCountValue = GetVariable(pInputObject, &sCount);
    if (pCountValue != NULL)
    {
        // we found the count value, so create our return object and
        // add the count to it.
        pRetVal = CreateNewAptObject(1);
        SetVariable(pRetVal, &sCount, pCountValue);
    }
    else
    {
        // we didn't find the count value, so return undefined
        pRetVal = GetUndefinedValue();
    }

    return pRetVal;
}

// in actionscript, CalculateFunction takes 3 parameters.  It calculates
// the result depending operation value which is passed, and returns the result
// If the operation is illegal, it returns the
// undefined object
APTEXT_FUNCTION(TestAptExt, CalculateFunction)
{
    // get the parameter
    AptValue *pValue1 = GetParam(0);
    AptValue *pValue2 = GetParam(1);
    AptValue *pOperation = GetParam(2);
    AptValue *pRetVal = NULL;

    // lookup the count
    APT_ASSERT(pValue1 != NULL && pValue2 != NULL && pOperation != NULL);

    AptNativeString sOperation;
    pOperation->toString(sOperation);

    if(!sOperation.CompareNoCase("Add"))
    {
        pRetVal = AptValueFactory::CreateInteger(pValue1->toInteger() + pValue2->toInteger());
    }
    else if(!sOperation.CompareNoCase("Subtract"))
    {
        pRetVal = AptValueFactory::CreateInteger(pValue1->toInteger() - pValue2->toInteger());
    }
    else if(!sOperation.CompareNoCase("Multiply"))
    {
        pRetVal = AptValueFactory::CreateInteger(pValue1->toInteger() * pValue2->toInteger());
    }
    else if(!sOperation.CompareNoCase("Divide"))
    {
        APT_ASSERT(pValue2->toInteger() != 0)
        pRetVal = AptValueFactory::CreateInteger(pValue1->toInteger() / pValue2->toInteger());
    }
    else
    {
        pRetVal = GetUndefinedValue();
    }
    return pRetVal;

}

//
APTEXT_FUNCTION(TestAptExt, OnEnterFunction)
{
    APT_DEBUGPRINT(APT_DEBUG_NORMAL, "inside on enter frame");
    return GetUndefinedValue();
}

APTEXT_FUNCTION(TestAptExt, UnRegisterFunction)
{
    AptUnRegisterExtension(APTEXT_MYOBJECT_NAME);
    return GetUndefinedValue();
}