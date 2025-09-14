/*H************************************************************************************************/
/*!

    \File
        AptActionInterpreter.inl

    \Description
        Simple description of main purpose of file.

    \Notes
        None.

    \Copyright
        (c) 2004 Electronic Arts Inc.

    \Version
        1.0        04/02/04 (ONA) First Version

*/
/************************************************************************************************H*/

#ifndef __APTACTIONINTERPRETER_INL__
#define __APTACTIONINTERPRETER_INL__

#include "_AptActions.h"

/*F************************************************************************************************/
/*!
    Insert your long description here.

    \Input      nPos              - 

    \Output     AptValue *        - 

    \Notes
        None.

    \Version
        1.0        04/02/04 (ONA) First Version

*/
/************************************************************************************************F*/
AptValue *    AptActionInterpreter::stackAt(const int32_t nPos) const
{
    return  (stack.At(nPos));
}

/*F************************************************************************************************/
/*!
    Insert your long description here.

    \Input      None.

    \Output     AptValue *        - 

    \Notes
        None.

    \Version
        1.0        04/02/04 (ONA) First Version

*/
/************************************************************************************************F*/
AptValue *    AptActionInterpreter::stackGetPop()
{
    return  (stack.GetPop());
}

/*F************************************************************************************************/
/*!
    Insert your long description here.

    \Input      pValue            - 

    \Output     None.

    \Notes
        None.

    \Version
        1.0        04/02/04 (ONA) First Version

*/
/************************************************************************************************F*/
void    AptActionInterpreter::stackPush(AptValue * const pValue)
{
    APT_ASSERT(pValue);
    APT_ASSERT(pValue->isLookup() == false);
    APT_ASSERT(pValue->isRegister() == false);
    APT_ASSERT(stack.GetSize() < stack.GetCapacity());

    stack.Push(pValue);
}
/*F************************************************************************************************/
/*!
Insert your long description here.

\Input      pValue            - 

\Output     None.

\Notes
None.

\Version
1.0        04/02/04 (ONA) First Version

*/
/************************************************************************************************F*/
void    AptActionInterpreter::stackPushNoInc(AptValue * const pValue)
{
    APT_ASSERT(pValue);
    APT_ASSERT(pValue->isLookup() == false);
    APT_ASSERT(pValue->isRegister() == false);
    APT_ASSERT(stack.GetSize() < stack.GetCapacity());

    stack.PushNoInc(pValue);
}
/*F************************************************************************************************/
/*!
Insert your long description here.

\Input      pValue            - 

\Output     None.

\Notes
None.

\Version
1.0        04/02/04 (ONA) First Version

*/
/************************************************************************************************F*/
void    AptActionInterpreter::stackPopNoDec()
{
    stack.PopNoDec();
}

/*F************************************************************************************************/
/*!
Insert your long description here.

\Input      pValue            - 

\Output     None.

\Notes
None.

\Version
1.0        04/02/04 (ONA) First Version

*/
/************************************************************************************************F*/
void    AptActionInterpreter::stackPopAndPush(const int32_t nCountToPop, AptValue * const pValue)
{
    APT_ASSERT(pValue);
    APT_ASSERT(pValue->isLookup() == false);
    APT_ASSERT(pValue->isRegister() == false);
    stack.PopAndPush(nCountToPop, pValue);
}

/*F************************************************************************************************/
/*!
    Insert your long description here.

    \Input      None.

    \Output     None.

    \Notes
        None.

    \Version
        1.0        04/02/04 (ONA) First Version

*/
/************************************************************************************************F*/
void    AptActionInterpreter::stackPop()
{
    stack.Pop();
}

/*F************************************************************************************************/
/*!
    Insert your long description here.

    \Input      nItems            - 

    \Output     None.

    \Notes
        None.

    \Version
        1.0        04/02/04 (ONA) First Version

*/
/************************************************************************************************F*/
void    AptActionInterpreter::stackPop(const int32_t nItems)
{
    return  (stack.Pop(nItems));
}

/*F************************************************************************************************/
/*!
    Insert your long description here.

    \Input      nItems            - 

    \Output     None.

    \Notes
        None.

    \Version
        1.0        04/02/04 (ONA) First Version

*/
/************************************************************************************************F*/
void    AptActionInterpreter::stackSafePop(const int32_t nItems)
{
    return  (stack.SafePop(nItems));
}

#endif  //  __APTACTIONINTERPRETER_INL__

/*H*************************************************************************************************
    EOF
*************************************************************************************************H*/

