/*H************************************************************************************************/
/*!

    \File    Apt\AptTarget.h

    \Description
        Simple description of main purpose of file.

    \Notes
        None.

    \Copyright
        (c) 2005 Electronic Arts Inc.

    \Version    1.0        10/07/05 (LEO) First Version

*/
/************************************************************************************************H*/


#ifndef _Apt_AptTarget_h
#define _Apt_AptTarget_h

#if defined(_MSC_VER)
#pragma once
#endif

/*** Include files ********************************************************************************/
#include "_Apt.h"

/*** Defines **************************************************************************************/

/*** Macros ***************************************************************************************/

    #define APT_SETUP_SIM_TARGET_STATES(_)              \
        AptTarget *pPrevTargetSim = gpCurrentTargetSim; \
        gpCurrentTargetSim = (AptTarget *)_;            \
        gpCurrentTargetRender = (AptTarget *)_;

    #define APT_TEARDOWN_SIM_TARGET_STATES              \
        gpCurrentTargetSim = pPrevTargetSim;            \
        gpCurrentTargetRender = pPrevTargetSim;

    #define APT_SETUP_RENDER_TARGET_STATES(_)           \
        AptTarget *pPrevTargetRender = gpCurrentTargetRender; \
        gpCurrentTargetRender = (AptTarget *)_;         \
        gpCurrentTargetSim = (AptTarget *)_;

    #define APT_TEARDOWN_RENDER_TARGET_STATES           \
        gpCurrentTargetRender = pPrevTargetRender;      \
        gpCurrentTargetSim = pPrevTargetRender;


/*** Type Definitions *****************************************************************************/
//! This class maintains an instance of Apt
class AptTarget
{
public:
    APT_NEW_DELETE_OPERATORS
    APT_DECLARE_OBJECT_METRICS

    /*! \Constructor */
    AptTarget(const AptInitParmsT *aptInitParms);

    /*! \Constructor */
    AptTarget(const AptTargetInitParams *aptInitParms);

    /*! \Deconstructor */
    ~AptTarget();

    /*! \Shutsdown and deletes all internal members */
    void Shutdown();

    /*! \Resets all internal states */
    void Reset();

    /*! \Returns the AnimationTarget */
    APT_INLINE AptAnimationTarget *GetAnimationTarget()
    {
        return mpPool;
    }

    /*! \Returns the AptLoader for this target */
    APT_INLINE AptLoader *GetLoader()
    {
        return mpLoader;
    }

    /*! \Returns the AptLinker for this target */
    APT_INLINE AptLinker *GetLinker()
    {
        return mpLinker;
    }

    /*! \Returns the next AptTarget in the list */
    APT_INLINE AptTarget *GetNext()
    {
        return mpNext;
    }

    /*! \Returns the previous target in the list */
    APT_INLINE AptTarget *GetPrevious()
    {
        return mpPrev;
    }

    /*! \Sets the next item in the target list */
    APT_INLINE void SetNext(AptTarget *pNew)
    {
        mpNext = pNew;
    }

    /*! \Sets the previous target in the target list */
    APT_INLINE void SetPrevious(AptTarget *pNew)
    {
        mpPrev = pNew;
    }

    APT_INLINE AptBCRenderTreeManager *GetRenderManager()
    {
        return mpRTMgr;
    }
private:
    //don't copy
    AptTarget(const AptTarget&);
    AptTarget& operator=(const AptTarget&);

    AptTargetInitParams mInitParms;
    AptAnimationTarget  *mpPool;
    AptLoader           *mpLoader;
    AptLinker           *mpLinker;
    AptTarget           *mpNext;
    AptTarget           *mpPrev;

    AptBCRenderTreeManager  *mpRTMgr;
};

/*** Variables ************************************************************************************/

extern uint32_t             nTargetsCreated;

/*** Functions ************************************************************************************/

#endif // _Apt_AptTarget_h
