/*H************************************************************************************************/
/*!

    \File
        AptScriptFunction.cpp

    \Description
        Simple description of main purpose of file.

    \Notes
        None.

    \Copyright
        (c) 2004 Electronic Arts Inc.

    \Version
        1.0        03/07/04 (ONA) First Version
        2.0         6/15/04 (ASK) AptScriptFunctionBase was added in Apt 0.16.00 version.
                                  It was added to implement DefineFunction2 tag in new swf specification.
                                  AptScriptFunction is now derived from AptScriptFunctionBase which is almost same as old AptScriptFunction.
                                  Only it does not contain pointer to AptAction_DefineFunction/2 and it is moved to
                                  AptScriptFunction/2 classes. Everything else is almost the same.
                                  GC functions are moved to AptScriptFunctionBase.
        3.0       09/15/04  (DC)  Rearranged objects so that the objects can take care of themselves. This was necessary because
                                  anywhere that called a function needed to know how to initialize each type of script function.
                                  This also centralizes registers and local scoping into the script function classes.
        3.1       11/01/04 (DC)   Made a separate AptValue type (VFT) for each Script function object, this is because the
                                  garbage collector needs to be able to figure out what the size of an object is by its type.
        3.2       02/13/07 (RB)   EATech 105415: Added pOverrideSuper param to SetupBeforeExecution().
        3.3       08/14/08 ($senzee) Added GetClassName(), SetClassName() for opt callstack scoping support
        3.4       07/09/09 (RB)   Fixed VC linker warning LNK4221 by moving APT_DEFINE_OBJECT_METRICS here from AptFrameStack.cpp and AptNativeFunction.cpp.
*/
/************************************************************************************************H*/

#include "AptScriptFunction.h"
#include "AptFrameStack.h"
#include "AptCIH.h"
#include "_Apt.h"
#include "AptObject/AptGlobalExtensionObject.h"
#include "AptObject/AptGlobalObject.h"
#include "AptObject/AptNativeFunction.h"
#include "MainInline.h"
#include "AptDebugger/AptDebugger.h"

APT_DEFINE_OBJECT_METRICS(AptScriptFunctionBase, AptObject);
APT_DEFINE_OBJECT_METRICS(AptScriptFunction1, AptScriptFunctionBase);
APT_DEFINE_OBJECT_METRICS(AptScriptFunction2, AptScriptFunctionBase);
APT_DEFINE_OBJECT_METRICS(AptScriptFunctionByteCodeBlock, AptScriptFunctionBase);
APT_DEFINE_OBJECT_METRICS(AptFrameStack, AptValueWithHash);
APT_DEFINE_OBJECT_METRICS(AptNativeFunction, AptObject);

AptValue**  AptScriptFunctionBase::spRegBlockBase = NULL;
AptValue**  AptScriptFunctionBase::spRegBlockCurrentFrameBase  = NULL;
int         AptScriptFunctionBase::snRegBlockCurrentFrameCount = 0;
int         AptScriptFunctionBase::snRegisterBlockSize = 0;
AptFrameStack*  AptScriptFunctionBase::spFrameStack = NULL;
const int   AptScriptFunctionBase::MAX_REGISTERS_IN_FUNCTION = 256; // Defined by Flash format Spec.



/*F************************************************************************************************/
/*!
    \Function    AptScriptFunctionBase::InitializeStaticData

    \Description
        This initializes the static data needed to run.

        This must be called before any actionscript can be run.

    Insert your long description here.

    \Input      pInitParams       - Initialization parameters specified by the game.

    \Output
        None.

    \Notes
        None.

    \Version    1.0        09/13/04 (DC) Bringin' it to ya

*/
/************************************************************************************************F*/
void AptScriptFunctionBase::InitializeStaticData(const AptInitParmsT & pInitParams)
{
    snRegisterBlockSize = pInitParams.iRegArraySize;
    // $bug 1010 Global new and delete operators used by Apt - replaced it to use APT_MALLOC_ARRAY - 1/2
    spRegBlockBase = APT_MALLOC_ARRAY(AptValue*, snRegisterBlockSize);

    spRegBlockCurrentFrameBase  = spRegBlockBase;

    for(int i = 0; i<snRegisterBlockSize; i++)
    {
        spRegBlockBase[i] = gpUndefinedValue;
    }

    snRegBlockCurrentFrameCount = 0;
}


/*F************************************************************************************************/
/*!
    \Function    AptScriptFunctionBase::ShutdownStaticData

    \Description
        Deletes static data created in InitializeStaticData

        This also ASSERTS certain conditions (i.e. that all functions were cleaned up properly)

    Insert your long description here.

    \Input
        None.

    \Output
        None.

    \Notes
        None.

    \Version    1.0        09/15/04 (DC) Bringin' it to ya

*/
/************************************************************************************************F*/
void AptScriptFunctionBase::ShutdownStaticData()
{
    APT_ASSERT(spRegBlockCurrentFrameBase == spRegBlockBase);
    APT_ASSERT(snRegBlockCurrentFrameCount == 0);

#if defined(EA_DEBUG)
    for(int i = 0; i<snRegisterBlockSize; i++)
    {
        APT_ASSERT(spRegBlockBase[i] == gpUndefinedValue);
    }
#endif
    //delete[] spRegBlockBase;        // (LEO) $bug782
    // $bug 1010 Global new and delete operators used by Apt - replaced it to use APT_FREE_ARRAY - 2/2
    APT_FREE_ARRAY(spRegBlockBase, AptValue*, snRegisterBlockSize) ;

    spRegBlockBase = NULL;
    spRegBlockCurrentFrameBase = NULL;
}

/*F************************************************************************************************/
/*!
    \Function    *AptScriptFunctionBase::PushStaticData

    \Description
        This needs to be called by any entity before initiating an new actionscript sequence.

        This only needs to be called by the entry points (actions, events etc) this is not called
        internally when recursing into actionscript functions, only when begining a new action
        script sequence.

    \Input
        None.

    \Output
        void*  - An internally useful value that needs to be passed into the
                 PopStaticData function upon completion of the actionscript sequence.

    \Notes
        None.

    \Version    1.0        09/15/04 (DC) Bringin' it to ya

*/
/************************************************************************************************F*/
void *AptScriptFunctionBase::PushStaticData()
{
    APT_ASSERT(spRegBlockBase);
    APT_ASSERT(spRegBlockCurrentFrameBase);
    AptValue ** pSaveBase = spRegBlockCurrentFrameBase;
    spRegBlockCurrentFrameBase += snRegBlockCurrentFrameCount;
    snRegBlockCurrentFrameCount = 0;
    return (void*)pSaveBase;
}

/*F************************************************************************************************/
/*!
    \Function    AptScriptFunctionBase::PopStaticData

    \Description
    This needs to be called by any entity after finising an new actionscript sequence.

    This only needs to be called by the entry points (actions, events etc) this is not called
    internally when recursing into actionscript functions, only when finishing a new action
    script sequence.

    Insert your long description here.

    \Input
        pPushValue - value returned from PushStaticData

    \Output
        None.

    \Notes
        None.

    \Version    1.0        09/15/04 (DC) Bringin' it to ya

*/
/************************************************************************************************F*/
void AptScriptFunctionBase::PopStaticData(void * pPushValue)
{
    APT_ASSERT(spRegBlockBase);
    // APT_ASSERT(spFrameStack == NULL); Note cannot assert this because Actions can recurse!
    AptValue ** pSaveBase = (AptValue**)pPushValue;

    APT_ASSERT(pSaveBase >= spRegBlockBase && pSaveBase <= spRegBlockCurrentFrameBase);

    // Clean up Registers
    for(int i = 0; i < snRegBlockCurrentFrameCount; i++)
    {
        AptValue * pValue = spRegBlockCurrentFrameBase[i];
        spRegBlockCurrentFrameBase[i] = gpUndefinedValue;
        APT_DEC(pValue);
    }

    snRegBlockCurrentFrameCount = spRegBlockCurrentFrameBase - pSaveBase;
    spRegBlockCurrentFrameBase  = pSaveBase;
}

/*F************************************************************************************************/
/*!
    \Function    AptScriptFunctionBase::AptScriptFunctionBase

    \Description
        Constructor for AptScriptFunctionBase

    \Input      *pCreatorFunction - This is the function from which this is defined.
    \Input      *pFunctionAnim    - This is the character to which the function is tied.
    \Input      *pCurCIH          - This is the CIH to which the function is tied.

    \Output     -

    \Notes
        It does almost the same work as older AptScriptFunction used to do, only it does not
        contain pointer to AptAction_DefineFunction/2.

    \Version    1.0        06/16/04 (ASK) First Version
                2.0        08/04/04 (LEO)
                    -We now keep track of the parent animation pointer.      $BUG253 LEO    44/47
                2.3        12/21/04 (LEO)
                    -Removed the mpParentAnim pointer
                2.4        04/05/08 JP
                    -Component classes used to be registered to the calling movie clips instead to the global space. That meant that when
                     the movie was unloaded it remained zombie because of the component. The new bRunningInitActions flag is set to true in
                     ExecuteInitActions. With the flag set to true, the component classes are now associated to the global space instead
                     of the calling movie clip
*/
/************************************************************************************************F*/
AptScriptFunctionBase::AptScriptFunctionBase(AptVirtualFunctionTable_Indices eType, AptScriptFunctionBase* pCreatorFunction, AptCIH *pCurCIH, bool bNeedsPrototype):
    AptObject(eType),
    mpCIH( pCurCIH ),
    mpParentAnim ( NULL),
    mpCreatorScope(NULL),
    mnFrameStackReserve(0)
{
    APT_ASSERT(spRegBlockBase);

    if( pCreatorFunction )
    {
        pCreatorFunction->CreatingNestedFunction();
        mpCreatorScope = spFrameStack;
        APT_INCSAFE(mpCreatorScope);
    }

    if (pCurCIH->isCIH() && !gAptActionInterpreter.bRunningInitActions)
    {
        // We have a calling movie clip and we are not running init actions: associate to the calling movie clip
        mpParentAnim = mpCIH->GetRootAnimation();
    }
    else
    {
        // We don't have a calling movie clip or we are running init actions
        mpParentAnim = _AptGetAnimationAtLevel(0);
    }

    // Take care of Reference counting.
    APT_INCSAFE(mpCIH);
    // ParentAnim can be null if we are associating to global space instead of a movie clip: a zombie will be avoided
    if(mpParentAnim)
    {
        APT_INC(mpParentAnim);
        mpParentAnim->IncZombieCount();
    }

    // every script function is an instance of the built-in Function class. - Mike Meyer
    Set__Proto__(gpFunctionPrototype);

    // every function has a unique prototype. (In Flash, too.  This is not an Apt quirk)
    if(bNeedsPrototype)
    {
        // Only true Script Functions can have prototypes.
        AptPrototype *pConstructorPrototype = new AptPrototype();
        mNativeHash.SetPrototype(pConstructorPrototype);
        pConstructorPrototype->GetNativeHashVirtual()->Set__Proto__(gpObjectPrototype);
    }
}


/*F************************************************************************************************/
/*!
    \Function    AptScriptFunctionBase::AptScriptFunctionBase

    \Description
        Similar to a copy constructor.

        This copies over the information that should be duplicated from the previous script function.

    Insert your long description here.

    \Input      pFrameStack       -
    \Input      *pFunctionAnim    -
    \Input      *pCurCIH          -

    \Output     A                 -

    \Notes
        None.

    \Version    1.0        09/14/04 (DC) Bringin' it to ya
                1.1        12/21/04 (LEO)
                        -Removed the mpParentAnim pointer

*/
/************************************************************************************************F*/
AptScriptFunctionBase::AptScriptFunctionBase(AptVirtualFunctionTable_Indices eType, AptScriptFunctionBase* pOrigFunc, AptCIH *pCurCIH ):
AptObject(eType),
    mpCIH( pCurCIH ),
    mpParentAnim ( NULL),
    mnFrameStackReserve(0)
{
    APT_ASSERT(spRegBlockBase);
    APT_ASSERT(pOrigFunc);

    mpCreatorScope = pOrigFunc->mpCreatorScope;
    APT_INCSAFE(mpCreatorScope);

    if (mpCIH->isCIH() && !gAptActionInterpreter.bRunningInitActions)
    {
        mpParentAnim = mpCIH->GetRootAnimation();
    }
    else
    {
        mpParentAnim = _AptGetAnimationAtLevel(0);
    }

    // Take care of Reference counting.
    APT_INCSAFE(mpCIH);
    if(mpParentAnim)
    {
        APT_INC(mpParentAnim);
        mpParentAnim->IncZombieCount();
    }

    // Take the prototype from the original Function.
    AptValue *pTemp = pOrigFunc->mNativeHash.GetPrototype();
    mNativeHash.SetPrototype(pTemp);
    // Also Take the __proto__
    pTemp = pOrigFunc->mNativeHash.Get__Proto__();
    mNativeHash.Set__Proto__(pTemp);
}


/*F************************************************************************************************/
/*!
    Destructor

    \Notes
        None.

    \Version
        1.0        03/07/04 (ONA) First Version

*/
/************************************************************************************************F*/
AptScriptFunctionBase::~AptScriptFunctionBase()
{
    mpCIH = NULL;
    mpParentAnim = NULL;
    mpCreatorScope = NULL;
}

//-------------------------------------------------
// This is called to create the frame stack (duh).
// Right now it uses the mnFrameStackReserve if present as the reserve size to reduce
// hash expands, but the seize prediction can easily be made to be much smarted if
// we have the time to spend on it.
void AptScriptFunctionBase::CreateFrameStack()
{
    if(mnFrameStackReserve)
    {
        spFrameStack = new AptFrameStack(mpCreatorScope, mnFrameStackReserve);
    }
    else
    {
        spFrameStack = new AptFrameStack(mpCreatorScope);
    }
#ifdef APT_DEBUGGER_ENABLE
    // This set operation only used in get locals of user ordered level in call stack.
    AptDebugger::GetInstance()->SetTopFrameStack(spFrameStack);
#endif
    APT_INC(spFrameStack);
}


//-------------------------------------------------
// Event called when a function is defining another function.
// In this case we have to create our frame stack so that it can be the parent
// of the created function.
void AptScriptFunctionBase::CreatingNestedFunction()
{
    if(spFrameStack == NULL)
    {
        // We have to create the Frame Stack if a nested function is to be created
        CreateFrameStack();
    }
}

/*F************************************************************************************************/
/*!
    \Function    AptScriptFunctionBase::GetClassName()

    \Description
        Returns the name of the class this AptScriptFunction represents.

    \Version    1.0        08/14/08 ($senzee) First Version
*/
/************************************************************************************************F*/
const char *AptScriptFunctionBase::GetClassName() const
{
#if defined(APT_RECORD_SCOPE_INFO)
    return mClassName.GetBuffer();
#else
    return 0;
#endif
}

/*F************************************************************************************************/
/*!
    \Function    AptScriptFunctionBase::SetClassName()

    \Description
        Sets the name of the class this AptScriptFunction represents.

    \Version    1.0        08/14/08 ($senzee) First Version
*/
/************************************************************************************************F*/
void AptScriptFunctionBase::SetClassName(const char *name)
{
#if defined(APT_RECORD_SCOPE_INFO)
    if (mClassName.IsEmpty() && name && *name)
    {
        mClassName = name;
    }
    AptPrototype *pConstructorPrototype = static_cast<AptPrototype *>(mNativeHash.GetPrototype());
    if (pConstructorPrototype)
        pConstructorPrototype->SetClassName(mClassName.GetBuffer());
#endif
}

/*F************************************************************************************************/
/*!
    \Function    AptScriptFunction::AptScriptFunction

    \Description
        Constructor for AptScriptFunction

    \Input      *pCreatorFunction - Function that called define function
    \Input      *_pFunction       - Define Function structure pointer (from the big file)
    \Input      *pFunctionAnim    - Animation we are associated with
    \Input      *pCurCIH          - CIH we are associated with

    \Output     -

    \Notes
        Initializes the pFunction and then calls AptScriptFunctionBase constructor to do remaining initialization.

    \Version    1.0        06/16/04 (ASK) First Version

*/
/************************************************************************************************F*/
AptScriptFunction1::AptScriptFunction1(AptScriptFunctionBase*           pCreatorFunction,
                                       const AptAction_DefineFunction * _pFunction,
                                       AptCIH *                         pCurCIH):
    AptScriptFunctionBase(AptVFT_ScriptFunction1, pCreatorFunction, pCurCIH, true),
    mpFunction(_pFunction)
{
}

/*F************************************************************************************************/
/*!
    \Function    AptScriptFunction1::AptScriptFunction1

    \Description
        Pretty much a copy constructor.

    Insert your long description here.

    \Input      pOrigFunc         -
        NO END BRACKET

    \Output     A                 -

    \Notes
        None.

    \Version    1.0        09/15/04 (DC) Bringin' it to ya

*/
/************************************************************************************************F*/
AptScriptFunction1::AptScriptFunction1(AptScriptFunction1             * pOrigFunc,
                                       AptCIH *                         pCurCIH):
    AptScriptFunctionBase(AptVFT_ScriptFunction1, pOrigFunc, pCurCIH),
    mpFunction(pOrigFunc->mpFunction)
{
}

/*F************************************************************************************************/
/*!
    \Function    AptScriptFunction2::AptScriptFunction2

    \Description
        Constructor for AptScriptFunction2

    \Input      *_pFunction       - Function that called define function
    \Input      *_pFrameStack     - Define Function 2 structure pointer (from the big file)
    \Input      *pFunctionAnim    - Animation we are associated with
    \Input      *pCurCIH          - CIH we are associated with

    \Output                         -

    \Notes
        Initializes the pFunction and then calls AptScriptFunctionBase constructor to do remaining initialization.

    \Version    1.0        06/16/04 (ASK) First Version

*/
/************************************************************************************************F*/
AptScriptFunction2::AptScriptFunction2(AptScriptFunctionBase*           pCreatorFunction,
                                       const AptAction_DefineFunction2 *_pFunction,
                                       AptCIH *                         pCurCIH) :
    AptScriptFunctionBase(AptVFT_ScriptFunction2, pCreatorFunction, pCurCIH, true),
    mpFunction(_pFunction)
{
}

AptScriptFunction2::AptScriptFunction2(AptScriptFunction2             * pOrigFunc,
                                       AptCIH *                         pCurCIH) :
    AptScriptFunctionBase(AptVFT_ScriptFunction2, pOrigFunc, pCurCIH),
    mpFunction(pOrigFunc->mpFunction)
{
}

/*F************************************************************************************************/
/*!
    \Function    AptScriptFunction1::~AptScriptFunction1

    \Description
        Destructor for AptScriptFunction1

    \Input
        None.

    \Output                      -

    \Notes
        None.

    \Version    1.0        06/16/04 (ASK) First Version

*/
/************************************************************************************************F*/
AptScriptFunction1::~AptScriptFunction1()
{
    mpFunction = NULL;
}

/*F************************************************************************************************/
/*!
    \Function    AptScriptFunction2::~AptScriptFunction2

    \Description
        Destructor for AptScriptFunction2

    \Input
        None.

    \Output     A                 -

    \Notes
        None.

    \Version    1.0        06/16/04 (ASK) First Version

*/
/************************************************************************************************F*/
AptScriptFunction2::~AptScriptFunction2()
{
    mpFunction = NULL;
}


/*F************************************************************************************************/
/*!
    \Function    AptScriptFunctionBase::PreDestroy

    \Description
        Just sets the mpFuncAnim to Null

    \Input
        None.

    \Output
        None.

    \Notes
        None.

    \Version    1.0        06/16/04 (ASK) First Version

*/
/************************************************************************************************F*/
void    AptScriptFunctionBase::PreDestroy()
{
    // commented as it is not to be set to null at this place
    //mpParentAnim = NULL;     // $BUG253 LEO    45/47
    //mpFuncAnim = NULL;
}
/*F************************************************************************************************/
/*!
    \Function    AptScriptFunctionBase::RegisterReferences

    \Description
        Registers all garbage collected references with global macro.

    Insert your long description here.

    \Input
        None.

    \Output
        None.

    \Notes
        None.

    \Version    1.0        09/15/04 (DC) Bringin' it to ya

*/
/************************************************************************************************F*/
void AptScriptFunctionBase::RegisterReferences()
{
    // APT_ASSERT(spFrameStack == NULL); // Can't ASSERT this because we can get for Zombie Prints.
    if( APT_REFERENCES_REGISTERED(this) )
        return;
    AptObject::RegisterReferences();

    APT_REGISTER_REFERENCE_SAFE(mpCreatorScope,    "ParentScope", APT_REFREG_IS_APTVALUE);
    APT_REGISTER_REFERENCE_SAFE(mpCIH,             "CIH", APT_REFREG_IS_APTCIH);
    APT_REGISTER_REFERENCE_SAFE(mpParentAnim,      "ParentAnim", APT_REFREG_IS_APTCIH);
    return;
}

/*F************************************************************************************************/
/*!
    \Function    AptScriptFunctionBase::DestroyGCPointers

    \Description
        Does work of removing references to GC objects in script function

    \Input
        None.

    \Output
        None.

    \Notes
        None.

    \Version    1.0        06/16/04 (ASK) First Version

*/
/************************************************************************************************F*/
void    AptScriptFunctionBase::DestroyGCPointers()
{
    APT_ASSERT(mpCIH);
    APT_ASSERT(mpParentAnim);

    APT_DECSAFE(mpCreatorScope);
    mpCreatorScope = NULL;
    APT_DECSAFE(mpCIH);
    mpCIH = NULL;
    if (mpParentAnim != NULL)
    {
        mpParentAnim->DecZombieCount();          // $BUG LEO    14
        APT_DEC(mpParentAnim) ;
    }
    mpParentAnim = NULL ;
    AptObject::DestroyGCPointers();
}



/*F************************************************************************************************/
/*!
    \Function    AptScriptFunctionBase::SetupBeforeExecution

    \Description
        Generic place to put code to be executed before each function call.

    \Input      pContext          -

    \Output
        None.

    \Notes
        None.

    \Version    1.0        09/10/04 (DC) Bringin' it to ya

*/
/************************************************************************************************F*/
void        AptScriptFunctionBase::SetupBeforeExecution(AptScriptFunctionState *pState, AptValue* pContext, AptValue*, AptValue* )
{
    //APT_DEBUGPRINT(" --ENTER ---------------Base:%8.8X--Max:%4.4d\n",spRegBlockCurrentFrameBase, snRegBlockCurrentFrameCount);

    /* Swap out the Current scope. */
    pState->mpFrameStack = spFrameStack;
    spFrameStack         = NULL;

}
/*F************************************************************************************************/
/*!
    \Function    AptScriptFunctionBase::CleanupAfterExecution

    \Description
        Generic place to put code to be executed after each function call.

    Insert your long description here.

    \Input
        None.

    \Output
        None.

    \Notes
        None.

    \Version    1.0        09/10/04 (DC) Bringin' it to ya

*/
/************************************************************************************************F*/
void        AptScriptFunctionBase::CleanupAfterExecution(AptScriptFunctionState *pState )
{
    //APT_DEBUGPRINT(" --EXIT  ---------------Base:%8.8X--Max:%4.4d\n",spRegBlockCurrentFrameBase, snRegBlockCurrentFrameCount);

    // Time to Clean out the Frame Stack.
    if(spFrameStack)
    {
        mnFrameStackReserve = spFrameStack->GetNativeHashVirtual()->GetHashSize();
        APT_DEC(spFrameStack);
    }
    /* Replace the Previous scope. */
    spFrameStack = pState->mpFrameStack;
}

/*F************************************************************************************************/
/*!
    \Function    AptScriptFunctionBase::GetRegisterValue

    \Description
        returns the value of the register at the given index.  (undefined if not there)

    \Input      nIndex            - index of register we are interested in.

    \Output     AptValue *        - value at that register (or undefined value)

    \Notes
        None.

    \Version    1.0        09/10/04 (DC) Bringin' it to ya

*/
/************************************************************************************************F*/
AptValue * AptScriptFunctionBase::GetRegisterValue(int nIndex)
{
    APT_ASSERT(nIndex < MAX_REGISTERS_IN_FUNCTION);
    APT_ASSERT(nIndex < ( snRegisterBlockSize - (spRegBlockCurrentFrameBase-spRegBlockBase))); // Don't overFlow the register Array
    APT_ASSERT(spRegBlockCurrentFrameBase[nIndex]); // Should never be NULL

    // Don't modify snRegBlockCurrentFrameCount unless we set a value.

    //APT_DEBUGPRINT(" --GETREG--%2.2d:%8.8X--Base:%8.8X--Max:%4.4d\n",nIndex, spRegBlockCurrentFrameBase[nIndex], spRegBlockCurrentFrameBase, snRegBlockCurrentFrameCount);

    return spRegBlockCurrentFrameBase[nIndex];
}


/*F************************************************************************************************/
/*!
    \Function    AptScriptFunctionBase::SetRegisterValue

    \Description
        Sets the value of the register at the given index to the given value.

        This increments and decrements as needed.

    \Input      nIndex            - index to set
    \Input      pNewValue         - value to set.

    \Output
        None.

    \Notes
        None.

    \Version    1.0        09/10/04 (DC) Bringin' it to ya

*/
/************************************************************************************************F*/
void    AptScriptFunctionBase::SetRegisterValue(int nIndex, AptValue * pNewValue)
{
    APT_ASSERT(pNewValue);
    APT_ASSERT(nIndex < MAX_REGISTERS_IN_FUNCTION);
    APT_ASSERT(nIndex < ( snRegisterBlockSize - (spRegBlockCurrentFrameBase-spRegBlockBase))); // Don't overFlow the register Array

    // forced crash added with NCAA CL 405983
#if defined(APT_FORCE_CRASHES)
    if (nIndex >= (snRegisterBlockSize - (spRegBlockCurrentFrameBase-spRegBlockBase)))
    {
		#if _MSC_VER
		__debugbreak();
		#elif defined(__clang__) || defined(__GNUC__)
		__builtin_trap();
		#else
		bool* willTrashMemory = (bool*)NULL;
		*willTrashMemory = true;
		#endif
    }
#endif

    if( (nIndex+1) > snRegBlockCurrentFrameCount)
        snRegBlockCurrentFrameCount = nIndex+1;

    //APT_DEBUGPRINT(" --SETREG--%2.2d:%8.8X--Base:%8.8X--Max:%4.4d\n",nIndex, pNewValue, spRegBlockCurrentFrameBase, snRegBlockCurrentFrameCount);

    AptValue * pOldValue = spRegBlockCurrentFrameBase[nIndex];
    spRegBlockCurrentFrameBase[nIndex] = pNewValue;
    APT_INC(pNewValue); // Always APT_INC the new value first (incase they are same value and at refcount=1)
    APT_DEC(pOldValue);
}


/*F************************************************************************************************/
/*!
    \Function    AptScriptFunction2::SetupBeforeExecution

    \Description
        This does the script function 2 specific tasks prior to execution.

        Script function 2 objects can pre-load a number of values into registers. Script
        function 2 objects also get their own set of registers at each call level.

    \Input      *pContext         -

    \Output
        None.

    \Notes
        None.

    \Version    1.0        09/10/04 (DC) Bringin' it to ya

*/
/************************************************************************************************F*/
void        AptScriptFunction2::SetupBeforeExecution(
    AptScriptFunctionState *pState,
    AptValue *pContext,
    AptValue *pOverrideThis,
    AptValue *pOverrideSuper
)
{
    AptScriptFunctionBase::SetupBeforeExecution(pState, pContext, pOverrideThis, pOverrideSuper);

    pState->mpRegBlockPreviousFrameBase = spRegBlockCurrentFrameBase;
    spRegBlockCurrentFrameBase += snRegBlockCurrentFrameCount;
    snRegBlockCurrentFrameCount = 0;

#if APT_DEBUG_LEVEL >= APT_DEBUG_MAXIMUM
    mpRegBlockMyBase            = spRegBlockCurrentFrameBase;
#endif


    AptValue * pTemp = NULL ;
    int nStartReg = 1 ;

    if (mpFunction->getDF2Flag(DF2_PreloadThisFlag))
    {
        if( pOverrideThis )
        {
            // RB: 05-09-2006  The caller passed in a pointer for 'this' -- the object being created.
            // Therefore we must be inside a super constructor.
            // We want to make sure we assign an AptObject to 'this'.
            // It would be incorrect to assign an AptPrototype to 'this', which
            // is what the findChild() call does below.
            pTemp = pOverrideThis;
        }
        else
        {
            pTemp = mpCIH->findChild(StringPool::GetString(SC_this), NULL);
        }
        SetRegisterValue(nStartReg++, pTemp);
    }
    if (mpFunction->getDF2Flag(DF2_PreloadArgumentsFlag))
    {
        // we have not implemented the arguments object and also we ask users not to use
        // arguments are it adds extra object creation and extra checking
        // so if actionscript needs to preload the arguments object in a register then
        // we are just setting it to undefinedvalue. Actually we should assert here.
        APT_ASSERT( false && "Arguments Array is unsupported in Apt at this time.");
        SetRegisterValue(nStartReg++, gpUndefinedValue);
    }
    if (mpFunction->getDF2Flag(DF2_PreloadSuperFlag))
    {
        if ( pOverrideSuper )
        {
            pTemp = pOverrideSuper;
        }
        else
        {
            // DC -- Try to get super from the pContext, if not found there, then move on to the mpCIH.
            pTemp = pContext->findChild(StringPool::GetString(SC_super), NULL);
        }

        if( pTemp == NULL || !pTemp->getIsDefined() )
        {
            pTemp = mpCIH->findChild(StringPool::GetString(SC_super), NULL);
        }

        SetRegisterValue(nStartReg++, pTemp);

    }

    if (mpFunction->getDF2Flag(DF2_PreloadRootFlag))
    {
        AptNativeString pTmpStr = "_root";
        pTemp = mpCIH->findChild(&pTmpStr, NULL);

        SetRegisterValue(nStartReg++, pTemp);
    }

    if (mpFunction->getDF2Flag(DF2_PreloadParentFlag))
    {
        AptNativeString pTmpStr = "_parent";
        pTemp = mpCIH->findChild(&pTmpStr, NULL);
        if (pTemp == NULL)
        {
            pTemp = gpUndefinedValue ;
        }

        SetRegisterValue(nStartReg++, pTemp);
    }
    if (mpFunction->getDF2Flag(DF2_PreloadGlobalFlag))
    {
        SetRegisterValue(nStartReg++, gpGlobalGlobalObject);
    }
}
/*F************************************************************************************************/
/*!
    \Function    AptScriptFunction2::CleanupAfterExecution

    \Description
        Basically cleans up the registers we used and returns Register stack back to it's previous
        values.

    \Input
        None.

    \Output
        None.

    \Notes
        None.

    \Version    1.0        09/15/04 (DC) Bringin' it to ya

*/
/************************************************************************************************F*/
void        AptScriptFunction2::CleanupAfterExecution(AptScriptFunctionState *pState)
{
#if APT_DEBUG_LEVEL >= APT_DEBUG_MAXIMUM
    APT_ASSERT( mpRegBlockMyBase == spRegBlockCurrentFrameBase);
#endif
    AptScriptFunctionBase::CleanupAfterExecution(pState);

    // Clean up Registers
    for(int i = 0; i < snRegBlockCurrentFrameCount; i++)
    {
        AptValue * pValue = spRegBlockCurrentFrameBase[i];
        spRegBlockCurrentFrameBase[i] = gpUndefinedValue;
        APT_DEC(pValue);
    }

    snRegBlockCurrentFrameCount = spRegBlockCurrentFrameBase - pState->mpRegBlockPreviousFrameBase;
    spRegBlockCurrentFrameBase  = pState->mpRegBlockPreviousFrameBase;
}
/*F************************************************************************************************/
/*!
    \Function    AptScriptFunctionByteCodeBlock::AptScriptFunctionByteCodeBlock

    \Description
        Creates a script function object for a bytecode block.

        This prevents the duplication of setup / scoping code for these objects as the script
        function base provides all that is needed.

    \Input      *pBytecodeBase    -
        NO END BRACKET

    \Output     A                 -

    \Notes
        None.

    \Version    1.0        09/10/04 (DC) Bringin' it to ya

*/
/************************************************************************************************F*/
AptScriptFunctionByteCodeBlock::AptScriptFunctionByteCodeBlock( const uint8_t *pBytecodeBase,
                                                               int blockSize,
                               AptConstantPool constantPool,
                               const char * pName,
                               AptCIH *pCurCIH,
                               AptScriptFunctionBase * pCreatorFunction):
    AptScriptFunctionBase(AptVFT_ScriptFunctionByteCodeBlock, pCreatorFunction, pCurCIH, false),
    mpByteCodeBase(pBytecodeBase),
    mnByteCodeSize(blockSize),
    mpName(pName),
    mConstantPool(constantPool)
{

}

/*H*************************************************************************************************
    EOF
*************************************************************************************************H*/

