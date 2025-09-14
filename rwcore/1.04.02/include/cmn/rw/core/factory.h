// (c) Electronic Arts. All Rights Reserved.

#ifndef PUBLIC_RW_CORE_FACTORY_H
#define PUBLIC_RW_CORE_FACTORY_H

/// \file
/// Generic templated object factory.

#include "rw/core/base/ostypes.h"
#include "rw/core/stltypes.h"
#include "rw/core/memory/memoryfunctions.h"
#include "rw/core/sharedglobal/sharedglobal.h"

namespace rw
{

// ***********************************************************************************************************
//                                                Producer CLASS
// ***********************************************************************************************************

/**
Abstract base class used as an interface to create products from within an abstract factory.

\par Template_params
PRODUCT_T The type of product object created by the factory.
PARAM_T   The type of parameter to pass to the producers  Create function.

A specializations of this class is used as a default implementation of the producer type template parameter
of the abstract  rw::Factory class.

\see rw::Factory
*/
template<typename PRODUCT_T, typename PARAM_T>
class Producer
{
public:

    RWMEMORYFUNCTIONS_NEWANDDELETE();

    /**
    Factory creates products of this type.
     */
    typedef PRODUCT_T                       product_type;

    /**
    The Create method of \see creator_type is passed a parameter of this type.
     */
    typedef PARAM_T                         param_type;

    /**
    Create a product of type PRODUCT_T.
    \param param Argument of type PARAM_T passed to product creator.

    \return Newly created object of type PRODUCT_T.
    */
    virtual product_type    Create(param_type param = param_type()) = 0;
};

/**
A Generic factory

Generic Factory for creating objects of type PRODUCT_T
based on a type identifier CLASSID_T. The PRODUCT's creation
method is passed a parameter of type PARAM_T when it is invoked.

\par
It is the caller's responsibility to delete any creator objects
which are registered with the factory. \see Factory::ClearRegistry
only clear's the registries record of registrations, and does not delete
the registered creators.

\par Example_usage:

A factory for creating objects of types inherited from Base,
by name. The objects are created with an int parameter. Note that
the following code does \em not delete the registered creators and
therefore leaks memory!

\code
typedef Factory<Base*, string, int> Factory;

class Inherted1Creator : public Factory::creator_type
{
    Base *CreateObject(int value)
    {
        return new Inherited1(value);
    }
}

class Inherted2Creator : public Factory::creator_type
{
    Base *CreateObject(int value)
    {
        return new Inherited2(value);
    }
}

void RegisterMoreTypes()
{
    Factory factory; // monostate can be instanced each time it is needed.

    factory.RegisterCreator("inherted2", new Inherited2Creator);
}

int main()
{
    Factory factory;

    factory.RegisterCreator("inherted1", new Inherited1Creator);

    RegisterMoreTypes();

    Base *obj = factory.Create("inherited1", 1);
    Inherted2 = factory.Create("inherited2", 2);

    factory.ClearRegistry();
}
\endcode

\note The class is implemented as a MONOSTATE pattern. Each instance
of a particular factory specialisation which you create (either dynamically
or on the stack) shares the same registry.
*/
template < typename PRODUCT_T, typename CLASSID_T, typename PARAM_T, uint32_t GLOBAL_ID,
           typename PRODUCER_T = Producer<PRODUCT_T, PARAM_T>, uint32_t ID = 0 >
// TODO: What was ID for, should specialize it so we get old behaviour if ID = 0?

class Factory
{
public:

    /**
    Factory creates products of this type.
    */
    typedef PRODUCT_T                       product_type;

    /**
    Product creation occurs through the Create method of this type.
    */
    typedef PRODUCER_T                       creator_type;

    /**
    Products are idendified by instances of this type.
    */
    typedef CLASSID_T                       classid_type;

    /**
    The Create method of \see creator_type is passed a parameter of this type.
    */
    typedef PARAM_T                         param_type;

    /**
    The factory's registry of product identifiers and their creators in stored in an
    object of this type.
    */
    typedef RwMap<CLASSID_T, PRODUCER_T*>    registry_type;

    /**
    Structure to hold all the data that needs to be shared.
    */
    struct SharedData
    {
        SharedData()
            : m_target("none")
        {
            // empty
        }

        registry_type m_registry;
        RwString m_target;
    };

    /**
    The factory's own type.
    */
    typedef rw::Factory<PRODUCT_T, CLASSID_T, PARAM_T, GLOBAL_ID, PRODUCER_T, ID> self_type;

    /**
    Default constructor.
    */
    Factory()
    {
        rwFUNCTION();
        rwRETURNVOID();
    }

    /**
    Default destructor.
    */
    virtual ~Factory()
    {
        rwFUNCTION();
        rwRETURNVOID();
    }

    /**
    Create a product given its class identifier and a parameter to use in its construction.

    \param type Type of product to create.
    \param params Parameter to pass to registered producer's Create method.

    \return New product.
    */
    product_type
    Create(classid_type type, param_type params)
    {
        product_type obj = NULL;
        typename registry_type::iterator it;

        rwFUNCTION();

        it = Registry().find(type);
        if (it != Registry().end())
        {
            obj = (*it).second->Create(params);
        }

        rwRETURN(obj);
    }

    /**
    Create a product given its class identifier.

    \param type Type of product to create.

    \return New product.
    */
    product_type
    Create(classid_type type)
    {
        product_type obj = NULL;
        typename registry_type::iterator it;

        rwFUNCTION();

        it = Registry().find(type);
        if (it != Registry().end())
        {
            obj = (*it).second->Create();
        }

        rwRETURN(obj);
    }

    /**
    Register a creation object for products of the given type.

    \param type       Type identifier of product to create.
    \param  creator   Creation object which will create the product.

    \return true if already registered, false otherwise.
    */
    bool
    RegisterCreator(classid_type type, creator_type* creator)
    {
        bool found;

        rwFUNCTION();

        found = UnregisterCreator(type);
        Registry()[type] = creator;

        rwRETURN(found);
    }

    /**
    Register a creation object for products of the given type.
    If the type is already registered, do not replace the existing registration.

    \param type       Type identifier of product to create.
    \param  creator   Creation object which will create the product.

    \return true if already registered, false otherwise.
    */
    bool
    RegisterCreatorUnique(classid_type type, creator_type* creator)
    {
        rwFUNCTION();

        bool found;

        if (Registered(type))
        {
            rwRETURN(true);
        }

        found = UnregisterCreator(type);
        Registry()[type] = creator;

        rwRETURN(found);
    }

    /**
    Register a creation object for products of the given collection of types.
    This allows a single creator object to be registered for distinct types.

    \param types      Collection of type identifiers of product to create.
    \param creator    Creation object which will create the product.

    \return true if already registered, false otherwise.
    */
    bool
    RegisterCreatorForClasses(const RwVector<classid_type> &types, creator_type* creator)
    {
        bool found = false;
        RwVector<classid_type> classplatformpairs;

        rwFUNCTION();

        for (typename RwVector<classid_type>::const_iterator it = types.begin();
             it != types.end();
             it++)
        {
            found = UnregisterCreator(*it);
            Registry()[*it] = creator;
        }

        rwRETURN(found);
    }

    /**
    Register a creation object for products of the given collection of types.
    This allows a single creator object to be registered for distinct types.
    If any of the types is already registered, do not replace the existing registration.

    \param types      Collection of type identifiers of product to create.
    \param creator    Creation object which will create the product.

    \return true if already registered, false otherwise.
    */
    bool
    RegisterCreatorUniqueForClasses(const RwVector<classid_type> &types, creator_type* creator)
    {
        bool found = false;
        RwVector<classid_type> classplatformpairs;

        rwFUNCTION();

        for (typename RwVector<classid_type>::const_iterator it = types.begin();
             it != types.end();
             it++)
        {
            if (Registered(*it))
            {
                continue;
            }

            found = UnregisterCreator(*it);
            Registry()[*it] = creator;
        }

        rwRETURN(found);
    }

    /**
    Remove a type of product from the factory.

    \param type Type of product to remove.

    \return true if already registered, false otherwise.
    */
    bool
    UnregisterCreator(const classid_type &type)
    {
        bool found = false;
        typename registry_type::iterator it;

        rwFUNCTION();

        it = Registry().find(type);
        while (it != Registry().end())
        {
            found = true;
            Registry().erase(it);
            it = Registry().find(type);
        }

        rwRETURN(found);
    }

    /**
    Remove multiple types of product from the factory.

    \param types Collection of types of product to remove.

    \return true if already registered, false otherwise.
    */
    bool
    UnregisterCreatorForClasses(const RwVector<classid_type> &types)
    {
        bool found = false;
        RwVector<classid_type> classplatformpairs;

        rwFUNCTION();

        for (typename RwVector<classid_type>::const_iterator it = types.begin();
             it != types.end();
             it++)
        {
            found = UnregisterCreator(*it);
        }

        rwRETURN(found);
    }

    /**
    Ask if the factory knows how to create a given kind of product.

    \param type Type of product to query.

    \return true if factory can create this product, false otherwise.
    */
    bool
    Registered(classid_type type)
    {
        typename registry_type::iterator it = Registry().find(type);

        rwFUNCTION();

        if ( it != Registry().end() )
        {
            rwRETURN(true);
        }

        rwRETURN(false);
    }

    /**
    Retrieves a pointer to the Creater for a given kind of product.

    \param type Type of product to query.

    \return either a pointer to the creator, or NULL if one has not been registered.
    */
    creator_type*
    GetCreator(classid_type type)
    {
        typename registry_type::iterator it = Registry().find(type);

        rwFUNCTION();

        if ( it != Registry().end() )
        {
            rwRETURN((*it).second);
        }

        rwRETURN(NULL);
    }

    /**
    Remove all products from the factory.
    */
    void
    ClearRegistry()
    {
        rwFUNCTION();

        Registry().clear();

        rwRETURNVOID();
    }

    /**
    Accesses the string specifying which target this factory's creators are registered for.

    \return Target associated with this factory.
    */
    RwString &
    Target()
    {
        rwFUNCTION();

        rwRETURN(Data().m_target);
    }

    /**
    Accessor of monostate registry holder.

    \return Reference to this factory's registry of products and creators.
    */
    virtual registry_type &
    Registry() const
    {
        rwFUNCTION();

        rwRETURN(Data().m_registry);
    }


    virtual SharedData &
    Data() const
    {
        static rw::SharedGlobal<SharedData, GLOBAL_ID> sRegistry;
        rwFUNCTION();

        rwRETURN(sRegistry.Data());
    }
};

} // namespace rw

#endif // PUBLIC_RW_CORE_FACTORY_H
