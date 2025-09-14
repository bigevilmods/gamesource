// (c) Electronic Arts. All Rights Reserved.

/// \file
/// Exception handling class to help in passing exceptions up through try / catch blocks.

// ***********************************************************************************************************
// Includes
#include "rwsharedheaders.hpp"
#include "rw/core/memory/memoryfunctions.h"

namespace rw
{

// ***********************************************************************************************************
// Typedefs


// ***********************************************************************************************************
// Defines + Enums + Consts


// ***********************************************************************************************************
// Static Variables + Static Data Member Definitions


// ***********************************************************************************************************
// Structs + Unions + Classes


// ***********************************************************************************************************
// Static Functions


// ***********************************************************************************************************
// External Functions


// ***********************************************************************************************************
// Class Member Functions

// ***********************************************************************************************************
//                                              rw::Exception CLASS
// ***********************************************************************************************************

/**
Construct an instance of an Exception object with a "Message" pointer, a "File" pointer, a line number and a
"Function" pointer.
\param msg message describing the exceptional circumstance
\param file source file where exception occurred
\param line line in source file where exception occurred
\param func name of function in which exception occurred
*/
Exception::Exception(const RwChar *msg, const RwChar *file,
                     const uint32_t line, const RwChar *func) throw()
    : m_pMsg(0), m_pFile(0), m_pFunc(0), m_Line((int32_t)line), m_pWhere(0)
{
    rwFUNCTION();

    m_pMsg = static_cast<RwChar*>(RwMalloc(RwStrlen(msg) + 1));
    RwStrcpy(m_pMsg, msg);

    m_pFile = static_cast<RwChar*>(RwMalloc(RwStrlen(file) + 1));
    RwStrcpy(m_pFile, file);

    m_pFunc = static_cast<RwChar*>(RwMalloc(RwStrlen(func) + 1));
    RwStrcpy(m_pFunc, func);

    rwRETURNVOID();
}

/**
Destroy an instance of an exception object.
*/
Exception::~Exception() throw()
{
    rwFUNCTION();

    if (m_pWhere != 0)
    {
        RwFree(m_pWhere);
    }

    if (m_pMsg != 0)
    {
        RwFree(m_pMsg);
    }

    if (m_pFile != 0)
    {
        RwFree(m_pFile);
    }

    if (m_pFunc != 0)
    {
        RwFree(m_pFunc);
    }

    rwRETURNVOID();
}

/**
Copy constructor for Exception class.

\param _Right Reference to an Exception.
*/
Exception::Exception(const Exception& _Right) throw() :
    m_pMsg(0), m_pFile(0), m_pFunc(0), m_Line(0), m_pWhere(0)
{
    rwFUNCTION();

    m_pMsg = static_cast<RwChar*>(RwMalloc(RwStrlen(_Right.m_pMsg) + 1));
    RwStrcpy(m_pMsg, _Right.m_pMsg);

    m_pFile = static_cast<RwChar*>(RwMalloc(RwStrlen(_Right.m_pFile) + 1));
    RwStrcpy(m_pFile, _Right.m_pFile);

    m_pFunc = static_cast<RwChar*>(RwMalloc(RwStrlen(_Right.m_pFunc) + 1));
    RwStrcpy(m_pFunc, _Right.m_pFunc);

    m_Line = _Right.m_Line;

    rwRETURNVOID();
}

/**
Copy operator for Exception class.

\param _Right Reference to an Exception.
\return A reference to an Exception.
*/
Exception&
Exception::operator=(const Exception& _Right) throw()
{
    rwFUNCTION();

    if (m_pMsg) RwFree(m_pMsg);

    m_pMsg = static_cast<RwChar*>(RwMalloc(RwStrlen(_Right.m_pMsg) + 1));
    RwStrcpy(m_pMsg, _Right.m_pMsg);

    if (m_pFile) RwFree(m_pFile);

    m_pFile = static_cast<RwChar*>(RwMalloc(RwStrlen(_Right.m_pFile) + 1));
    RwStrcpy(m_pFile, _Right.m_pFile);

    if (m_pFunc) RwFree(m_pFunc);

    m_pFunc = static_cast<RwChar*>(RwMalloc(RwStrlen(_Right.m_pFunc) + 1));
    RwStrcpy(m_pFunc, _Right.m_pFunc);

    m_Line = _Right.m_Line;

    rwRETURN(*this);
}

/**
Return a string describing the "Message" associated with an Exception object.
*/
const RwChar *
Exception::What() const throw()
{
    rwFUNCTION();

    if (m_pMsg)
    {
        rwRETURN(m_pMsg);
    }

    rwRETURN("Exception: Unknown Error.");
}

/**
Return a string describing whereabouts in the source code an Exception object was generated.
*/
const RwChar *
Exception::Where() throw()
{
    size_t    size;

    rwFUNCTION();

    size = RwStrlen(m_pFile) + 32 + RwStrlen(m_pFunc);

    if (m_pWhere != 0)
    {
        RwFree(m_pWhere);
    }

    m_pWhere = static_cast<RwChar *> (RwMalloc(size));

    sprintf(m_pWhere, "%s (%d) %s", m_pFile, m_Line, m_pFunc);

    rwRETURN(m_pWhere);
}

} // namespace rw
