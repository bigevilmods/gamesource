// (c) 2000-2007 Electronic Arts Inc.
// WavCompCtrl.h : header file
//

#if !defined(AFX_WAVCOMPCTRL_H__65297295_3466_11D3_B3A2_00500463EC33__INCLUDED_)
#define AFX_WAVCOMPCTRL_H__65297295_3466_11D3_B3A2_00500463EC33__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CWavCompCtrl window

class CWavCompCtrl : public CStatic
{
// Construction
public:
	CWavCompCtrl();
	void SetImageA(int nWidth, int nHeight, ImageType image, bool rgb=true);
	void SetImageB(int nWidth, int nHeight, ImageType image, bool rgb=true);
	void RenderImage(CDC &dc, int nWidth, int nHeight, ImageType &image, CRect destrect);
	void SetMode(bool rgbmode) {m_bRgbMode = rgbmode;}
	void SetBrowse(bool browse);

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWavCompCtrl)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CWavCompCtrl();

	// Generated message map functions
protected:
	int m_nWidthA, m_nHeightA, m_nWidthB, m_nHeightB;
	ImageType m_ImageA, m_ImageB;
	bool m_bRgbMode, m_bBrowsing;
	//{{AFX_MSG(CWavCompCtrl)
	afx_msg void OnPaint();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WAVCOMPCTRL_H__65297295_3466_11D3_B3A2_00500463EC33__INCLUDED_)
