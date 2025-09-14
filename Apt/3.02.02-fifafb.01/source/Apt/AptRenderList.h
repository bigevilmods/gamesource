/*H************************************************************************************************/
/*!

    \file    AptRenderList.h

    \brief         Contains simple interface for generating RenderLists for buffered rendering.


        (c) 2005 Electronic Arts Inc.

    \version    1.0        06/22/07 ($senzee) First Version
    \version    1.1        10/30/07 (ASK) Added enum for effect operations - AptEffectOperation
*/
/************************************************************************************************H*/

#ifndef _Apt_AptRenderList_h
#define _Apt_AptRenderList_h

#if defined(_MSC_VER)
#pragma once
#endif

#include "AptRenderable.h"
#include "_Apt.h"
#include APT_INC_THREAD_FUTEX_H

class AptRenderableGeometry;
class AptRenderItem;

enum AptEffectOperation
{
    AptEffectOperation_Unapply = -1,
    AptEffectOperation_None = 0,
    AptEffectOperation_Apply = 1
};

class AptRenderList
{
public:
    static void Initialize();
    static void Stop();
    static void Shutdown();

    static bool Open();
    static void Close();

    static void StartRender();
    static void Render(unsigned uLevelMask);
    static void EndRender();

    static void Add(const AptRenderInfo &info, const AptRenderItem *pRI, IAptRenderable *pRenderable, AptRenderableGeometry *pBounds = 0);

    static void Destroy(AptFile *pFile);

    static void Restart();

private:
    static APT_THREAD_NAMESPACE::Futex mRestartMutex;

};

#endif // _Apt_AptRenderList_h
