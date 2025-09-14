/*H************************************************************************************************/
/*!

    \File
        AptObject.h

    \Description
        Implements "Object" from Actionscript and is basis for most classes in Apt that can be
        instantiated as Objects in Flash.


    Background on Objects, Prototypes and constructors and __proto__.

    Object.prototype - refers to the implementation of the class, i.e. its members and such. These
                    are different then the items in the Object hash.

    Object.__proto__ - refers to the prototype (implementation) of the super class. These
                    are generally read only in flash from the sub class. (although Apt doesn't care.)
                    In any case When searching for members we start at the local scope and chain to
                    each __proto__ and then its __proto__ to see if any object in the chain has the
                    member.


    Object.prototype.__constructor__ - This is the pointer to the Super Class Constructor object. i.e.
                    the script that is executed when the super object constructor is called (super()).
                    This is accessable in Actionscript 2.0+, Flash 7+ only. It cannot be used directly
                    in flash 6 and/or Actionscript 1.0.



                        +---------------+               +---------------+
                        |               |   prototype   |               |
                        |  Base Object  +-------------->+  Base Object  |
                        |               |               |   prototype   |
                        +-------^-------+               +-------^-------+
                               /-\                             /-\
                                |                               |
                                | __constructor__               |
                                |                               | __proto__
                                |                               |
                                |                               |
                                |            +------------------+------------------------------------+
                                |            |                                                       |
                                |            |                                                       |
                                |            |                                                       |
                                |            |                                                       |
                                +======+==== | =================================================+    |
                                       |     |                                                  |    |
                                       |     |                                                  |    |
                                       |     |                                                  |    |
                                       |     |                                                  |    |
                                       |     |                                                  |    |
    +---------------+               +--+-----+------+       +---------------+               +---+----+------+
    |               |   prototype   |               |       |               |   prototype   |               |
    |  Sub Object   +-------------->+   Sub Object  |       |  Sub Object   +-------------->+   Sub Object  |
    |               |               |   prototype   |       |               |               |   prototype   |
    +-------^-------+               +--------^------+       +---------------+               +---------------+
           /-\                              /-\
            |                                |
            |                                |
            |                                |
            | __constructor__                | __proto__
            +==========================+     |
                                       |     |
                                       |     |
                                       |     |
    +---------------+               +--+-----+------+
    |               |   prototype   |               |
    |  SubSubObject +-------------->+  SubSubObject |
    |               |               |   prototype   |
    +---------------+               +---------------+



    \Notes
        None.

    \Copyright
        (c) 2004 Electronic Arts Inc.

    \Version
        1.0        03/07/04 (ONA) First Version
        2.1        08/14/08 ($senzee) Added GetClassName(), SetClassName() for call stack info
        2.2        10/30/09 (RB)  Replaced usage of APT_VALUE_GC_NEW_DELETE_OPERATORS with
                                  plain operator new/delete declarations.
*/
/************************************************************************************************H*/

#ifndef __APTOBJECT_H__
#define __APTOBJECT_H__

#include "AptValue/AptValue.h"
#include "AptNativeHash.h"
// FIFA_BEGIN | SENDBACK | mwebb | 2015 - 05 - 26 | Adding Frostbite support
#include "Module.h"
// FIFA_END

//  Initial size of the hash-table
#define APT_OBJECTHASHSIZE  8


//##############################################################################
//
// Class AptValueWithHash is a base class for all garbage collected items that
// have a hash, it implements the basic hash functions (which were previously
// duplicated for every object)
#include "AptNativeHash.h"

class AptValueWithHash : public AptValueGC
{
APT_ACCESS_INTERNAL:
    APT_DECLARE_OBJECT_METRICS

    AptValueWithHash(AptVirtualFunctionTable_Indices eType, int nHashSize) : AptValueGC(eType), mNativeHash(nHashSize)
    {
    };

    virtual AptNativeHash * GetNativeHashVirtual();
    virtual bool            ContainsNativeHashVirtual() const;
    virtual void            RegisterReferences();
    virtual void            DestroyGCPointers();

    APT_INLINE AptValue *              Lookup( const AptNativeString * const pKey) const;
    APT_INLINE void                    Set(const AptNativeString * const pKey, AptValue * const pValue);

protected:

    virtual ~AptValueWithHash() {};
    AptNativeHash           mNativeHash;

    // AptNativeHash needs to be able to access protected methods.
    // Had to add class keyword for psp compiler.
    friend                  class AptNativeHash;

public:
	// FIFA_BEGIN | SENDBACK | mwebb | 2015 - 05 - 26 | Adding Frostbite support
    //unsigned int GetNumAptValues();
    //const AptValue* GetKeyValuePair(unsigned int uKeyIndex, AptNativeString& keyName);
	FB_APT_API unsigned int GetNumAptValues();
	FB_APT_API const AptValue* GetKeyValuePair(unsigned int uKeyIndex, AptNativeString& keyName);
	// FIFA_END

};

//##############################################################################
//
// Class AptValueWithHash is a base class for all global garbage collected,
// (but non-reference counted) items that  have a hash, it implements the basic
// hash functions (which were previously duplicated for every object).
class AptValueGlobalWithHash : public AptValueWithHash
{
APT_ACCESS_INTERNAL:
    // DC - Added Metric defines
    APT_DECLARE_OBJECT_METRICS

    AptValueGlobalWithHash(AptVirtualFunctionTable_Indices eType, int nHashSize) :
      AptValueWithHash(eType, nHashSize)
      {
      };

      // This Optimizes global objects by doing nothing on Add/Release
#if defined(APT_INC_DEC_MESSAGES)
      virtual void    AddRef(const char * szFuncName, const char * szFileName, int nLineNumber)  { }   //  Do nothing...
      virtual void    Release(const char * szFuncName, const char * szFileName, int nLineNumber) { }   //  Do nothing...
#else
      virtual void    AddRef()  { }   //  Do nothing...
      virtual void    Release() { }   //  Do nothing...
#endif

};
/*------------------------------------------------------------------------------------------------------*\

\*------------------------------------------------------------------------------------------------------*/

//################################################################################################
// AptPrototype -- This implements classes in Actionscript. They are placed into prototypes.
// and hierarchy chains are built with these object.
class AptPrototype : public AptValueWithHash
{
APT_ACCESS_INTERNAL:
    static void *operator new(size_t size);
    static void operator delete(void *p, size_t size);

    APT_DECLARE_OBJECT_METRICS

    AptPrototype() :
            AptValueWithHash(AptVFT_Prototype, APT_OBJECTHASHSIZE),
            mp__constructor__(NULL)
    {
        SetAllowDelayedDeletion(false);
    }

    virtual void DestroyGCPointers();
    virtual void RegisterReferences();

    virtual const char *GetClassName() const;
    virtual void        SetClassName(const char *name);

    virtual AptValue *  objectMemberLookup(AptValue * const pContext, const AptNativeString * const pName) const;
    virtual bool        objectMemberSet(AptValue * const pContext, const AptNativeString * const pName, AptValue * const pValue);

    APT_INLINE
    AptValue *GetSuperConstructor( ) const
    {
        return mp__constructor__;
    }

    void SetSuperConstructor(AptValue * pNewSuperConstructor );

protected:
    virtual ~AptPrototype()
    {
        //  nothing to do...
    }

private:
    AptValue              *mp__constructor__;
#if defined(APT_RECORD_SCOPE_INFO)
    AptNativeString        mClassName;
#endif

private:
    // Do not call. Garbage collected objects cannot be created as C++ arrays.
    static void *operator new[](size_t size);
    static void operator delete[](void *p);
};


//################################################################################################
// AptObject -- The basics of a dynamic script object.
class AptObject : public AptValueWithHash
{
APT_ACCESS_INTERNAL:
	// FIFA_BEGIN | SENDBACK | mwebb | 2015 - 05 - 26 | Adding Frostbite support
    //static void *operator new(size_t size);
    //static void operator delete(void *p, size_t size);
	FB_APT_API static void *operator new(size_t size);
	FB_APT_API static void operator delete(void *p, size_t size);
	// FIFA_END

    APT_DECLARE_OBJECT_METRICS

        AptObject(AptVirtualFunctionTable_Indices eType, int nSize = APT_OBJECTHASHSIZE) :
            AptValueWithHash(eType, nSize),
            //mpaImplementedObjects(0),
            mnImplementedObjects(0),
            mbHasClass(0),
            mbIsInMainInst(0)
    {
    }

    // DC - Made Common Interface to "hasClass" functions from AptValue. (This is now virtual)
	// FIFA_BEGIN | SENDBACK | mwebb | 2015 - 05 - 26 | Adding Frostbite support
    //virtual void SetHasClass(int bHasClass)
	FB_APT_API virtual void SetHasClass(int bHasClass)
	// FIFA_END
    {
        mbHasClass = (bHasClass) ? 1u : 0u;
    }

    // DC - Made Common Interface to "hasClass" functions from AptValue. (This is now virtual)
	// FIFA_BEGIN | SENDBACK | mwebb | 2015 - 05 - 26 | Adding Frostbite support
    //virtual bool GetHasClass() const
	FB_APT_API  virtual bool GetHasClass() const
	// FIFA_END
    {
        return mbHasClass != 0u;
    }

	// FIFA_BEGIN | SENDBACK | mwebb | 2015 - 05 - 26 | Adding Frostbite support
    //inline void setInMainInst(int bInMainInst)
	FB_APT_API inline void setInMainInst(int bInMainInst)
	// FIFA_END
    {
        mbIsInMainInst = (bInMainInst) ? 1u : 0u;
    }

	// FIFA_BEGIN | SENDBACK | mwebb | 2015 - 05 - 26 | Adding Frostbite support
    //inline int  getInMainInst() const
	FB_APT_API inline int  getInMainInst() const
	// FIFA_END
    {
        return static_cast<int>(mbIsInMainInst);
    }

	// FIFA_BEGIN | SENDBACK | mwebb | 2015 - 05 - 26 | Adding Frostbite support
	//virtual AptValue *  objectMemberLookup(AptValue * const pContext, const AptNativeString * const pName) const;


	//virtual void  RegisterReferences();
	//virtual void  DestroyGCPointers();


	//APT_INLINE void        Set__Proto__(AptValue * const pValue);
	//APT_INLINE void        SetPrototype(AptValue * const pValue);

	//void        SetImplementedObjects(AptArray* paImplementedObjects, int nNumObjects);
	//AptArray*   GetImplementedObjects(int * nOutNumObjects) const;
	//bool        DoesImplementObject(AptValue  * pObject) const;

	//AptObject * Clone(bool bRecursive = false) const;
    FB_APT_API virtual AptValue *  objectMemberLookup(AptValue * const pContext, const AptNativeString * const pName) const;


    FB_APT_API virtual void  RegisterReferences();
    FB_APT_API virtual void  DestroyGCPointers();


    FB_APT_API APT_INLINE void        Set__Proto__(AptValue * const pValue);
    FB_APT_API  APT_INLINE void        SetPrototype(AptValue * const pValue);

    FB_APT_API  void        SetImplementedObjects(AptArray* paImplementedObjects, int nNumObjects);
    FB_APT_API AptArray*   GetImplementedObjects(int * nOutNumObjects) const;
    FB_APT_API bool        DoesImplementObject  (AptValue  * pObject) const;

    FB_APT_API AptObject * Clone(bool bRecursive = false) const;
	// FIFA_END
protected:

    virtual ~AptObject()
    {
    }

    //AptValue     ** mpaImplementedObjects;    // (LEO) Removed this, ImplementedObjects is now in the NativeHash as an array
    uint32_t        mnImplementedObjects:8; // Max 256 implemented Interfaces
    uint32_t        mbHasClass:1;
    uint32_t        mbIsInMainInst:1;

private:
    // Do not call. Garbage collected objects cannot be created as C++ arrays.
    static void *operator new[](size_t size);
    static void operator delete[](void *p);
};


#endif  //  __APTOBJECT_H__

/*H*************************************************************************************************
    EOF
*************************************************************************************************H*/

