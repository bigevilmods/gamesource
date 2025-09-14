// (c) Electronic Arts. All Rights Reserved.

/// \file
/// Memory profile function interface.

#ifndef RW_MEMORYPROFILEFUNCTIONS_HPP
#define RW_MEMORYPROFILEFUNCTIONS_HPP

// ***********************************************************************************************************
// Includes

// ***********************************************************************************************************
// Defines
namespace rw
{
namespace internal
{
namespace memory
{

// ***********************************************************************************************************
// Forward Declarations


// ***********************************************************************************************************
// Typedefs


// ***********************************************************************************************************
// Enums + Consts


// ***********************************************************************************************************
// Global Variables


// ***********************************************************************************************************
// External Functions Prototypes


// ***********************************************************************************************************
// Structs + Unions + Classes


// ***********************************************************************************************************
//                                                Class Definition
// ***********************************************************************************************************

class Profile
{
public:
    typedef void (*PushStackFunc)();
    typedef void (*PopStackFunc)();
    typedef void (*WriteFunc)(const RwChar *string);
    typedef void (*DisableFunc)();

    Profile()
    {
        rwFUNCTION();
        m_pushStackFunc();
        rwRETURNVOID();
    }

    ~Profile()
    {
        rwFUNCTION();
        m_popStackFunc();
        rwRETURNVOID();
    }

    void
    Write(const RwChar *string = "")
    {
        rwFUNCTION();
        m_writeFunc(string);
        rwRETURNVOID();
    }

    static void Disable()
    {
        rwFUNCTION();
        m_disableFunc();
        rwRETURNVOID();
    }

    static PushStackFunc    m_pushStackFunc;
    static PopStackFunc     m_popStackFunc;
    static WriteFunc        m_writeFunc;
    static DisableFunc      m_disableFunc;
};

} // namespace memory

} // namespace internal

} // namespace rw

#endif // RW_MEMORYPROFILEFUNCTIONS_HPP
