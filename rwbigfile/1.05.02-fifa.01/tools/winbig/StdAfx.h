// (c) Electronic Arts. All Rights Reserved.
// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__8F105A90_DD69_4139_89F3_D707683B1BF5__INCLUDED_)
#define AFX_STDAFX_H__8F105A90_DD69_4139_89F3_D707683B1BF5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxcview.h>
#include <afxdisp.h>        // MFC Automation classes
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include "rw/core/bigfile/bigfile.h"
#include "coreallocator/icoreallocator_interface.h"

#include "rw/core/codec/refpack.h"
#include "rw/core/codec/chunkcodec.h"
#include "job_manager/job_scheduler.h"
#include "EABase/EABase.h"

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__8F105A90_DD69_4139_89F3_D707683B1BF5__INCLUDED_)
