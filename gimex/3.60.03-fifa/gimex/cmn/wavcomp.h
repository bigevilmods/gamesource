// (c) 2000-2007 Electronic Arts Inc.
// WavComp.h : main header file for the WAVCOMP application
//

#if !defined(AFX_WAVCOMP_H__6529728B_3466_11D3_B3A2_00500463EC33__INCLUDED_)
#define AFX_WAVCOMP_H__6529728B_3466_11D3_B3A2_00500463EC33__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CWavCompApp:
// See WavComp.cpp for the implementation of this class
//

class CWavCompApp : public CWinApp
{
public:
	CWavCompApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWavCompApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CWavCompApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WAVCOMP_H__6529728B_3466_11D3_B3A2_00500463EC33__INCLUDED_)
