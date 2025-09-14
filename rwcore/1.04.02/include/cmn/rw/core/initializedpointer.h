// (c) Electronic Arts. All Rights Reserved.

/// \file
/// Definition of InitializedPointer template class

#ifndef RW_INITIALIZEDPOINTER_HPP
#define RW_INITIALIZEDPOINTER_HPP

// ***********************************************************************************************************
// Includes

// ***********************************************************************************************************
// Defines

namespace rw
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
//                                                InitializedPointer CLASS
// ***********************************************************************************************************

/**
A template class that emulates a pointer, however all instances using the default constructor are guaranteed
to be initialized to NULL.
*/
template <typename T>
class InitializedPointer
{
public:
    // Construction & Destruction
    InitializedPointer();
    InitializedPointer(T *initialValue);
    ~InitializedPointer();

    // Public Enums, Consts, Typedefs and Static variables

    // Interface Implementations

    // Methods
    inline InitializedPointer &operator=(T *other);

    inline operator T *();

    inline T *operator->();

protected:
    // Virtual Functions

private:
    // Private Enums, Consts, Typedefs and Static variables

    // Helper Member Functions

    // Member Variables
    T *m_pointer;
};

// Inline Functions

/**
Constructs an InitializedPointer.
*/
template<typename T>
InitializedPointer<T>::InitializedPointer()
    : m_pointer(NULL)
{
    rwFUNCTION();
    rwRETURNVOID();
}


/**
Constructs an InitializedPointer.
*/
template<typename T>
InitializedPointer<T>::InitializedPointer(T *initialValue)
    : m_pointer(initialValue)
{
    rwFUNCTION();
    rwRETURNVOID();
}


/**
Destructs an InitializedPointer.
*/
template<typename T>
InitializedPointer<T>::~InitializedPointer()
{
    rwFUNCTION();
    rwRETURNVOID();
}


/**
Assignment operator for InitializedPointers.

\param other The new pointer.

\return A reference to this instance.
*/
template<typename T>
InitializedPointer<T> &InitializedPointer<T>::operator=(T *other)
{
    rwFUNCTION();
    m_pointer = other;
    rwRETURN(*this);
}


/**
Pointer cast operator for InitializedPointers.

\return The pointer.
*/
template<typename T>
InitializedPointer<T>::operator T *()
{
    rwFUNCTION();
    rwRETURN(m_pointer);
}


/**
-> overload operator for InitializedPointers.

\return The pointer.
*/
template<typename T>
T *InitializedPointer<T>::operator->()
{
    rwFUNCTION();
    rwRETURN(m_pointer);
}


} //namespace rw

#endif // RW_INITIALIZEDPOINTER_HPP
