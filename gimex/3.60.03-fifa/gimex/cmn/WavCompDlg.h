// (c) 2000-2007 Electronic Arts Inc.
/////////////////////////////////////////////////////////////////////////////
// CWavCompDlg dialog

// WavCompDlg.h : header file
//

#if !defined(AFX_WAVCOMPDLG_H__6529728D_3466_11D3_B3A2_00500463EC33__INCLUDED_)
#define AFX_WAVCOMPDLG_H__6529728D_3466_11D3_B3A2_00500463EC33__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "WavCompCtrl.h"
class CWavelet;

class CWavCompDlg : public CDialog
{
// Construction
public:
	CWavCompDlg(CWnd* pParent = NULL);	// standard constructor
	virtual ~CWavCompDlg();
	CWavelet * pWavelet;

// Dialog Data
	//{{AFX_DATA(CWavCompDlg)
	enum { IDD = IDD_WAVCOMP_DIALOG };
	CButton	m_Browse;
	CStatic	m_Ratio;
	CStatic	m_CompSize;
	CStatic	m_OrgSize;
	CSliderCtrl	m_YQuant;
	CSliderCtrl	m_PrQuant;
	CSliderCtrl	m_PbQuant;
	CListBox	m_ImageList;
	CWavCompCtrl	m_WavCompCtrl;
	int		m_PbVal;
	int		m_PrVal;
	int		m_YVal;
	BOOL	m_bSubDirs;
	int		m_MipMaps;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWavCompDlg)
	public:
	virtual void OnFinalRelease();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void FindAverageColor(int nWidth, int nHeight, unsigned char * pImage, unsigned char * pAlphaImage, unsigned char & blue, unsigned char & green, unsigned char & red, unsigned char & alpha);
	void RedrawImages();
	void SetControls();
	void CreateWctFilename(const char * directory, const char * filename, char * path);
	void SetUp();
	void SetCurrentImage();
	void Recompress();
	ImageType m_Image, m_AlphaImage, m_CompressedImage, m_CompressedAlphaImage;
	int m_nWidth, m_nHeight, m_nPixelDepth;
	void LoadImage(char * pFileName);
	char m_Directory[MAX_PATH];
	void EnumTGAs();
	void EnumDir(const char * pDirectory);
	HICON m_hIcon;
	int m_nYQuant, m_nPrQuant, m_nPbQuant, m_nAlphaQuant, m_nMipMaps;
	int m_nDefYQuant, m_nDefPrQuant, m_nDefPbQuant, m_nDefAlphaQuant, m_nDefMipMaps;
	int m_nRestoreYQuant, m_nRestorePrQuant, m_nRestorePbQuant, m_nRestoreAlphaQuant, m_nRestoreMipMaps;
	bool m_bBrowsing;
	CIgArray<CString,CString>m_Dirs;
	int m_nCurSel;

	// Generated message map functions
	//{{AFX_MSG(CWavCompDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnSelchangeImagelist();
	afx_msg void OnModeRgb();
	afx_msg void OnModeAlpha();
	afx_msg void OnMode();
	afx_msg void OnChangeYval();
	afx_msg void OnChangePrval();
	afx_msg void OnChangePbval();
	afx_msg void OnChangeDir();
	afx_msg void OnDestroy();
	afx_msg void OnDefaults();
	afx_msg void OnBatch();
	afx_msg void OnRestore();
	afx_msg void OnSubdirs();
	afx_msg void OnChangeMipmaps();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WAVCOMPDLG_H__6529728D_3466_11D3_B3A2_00500463EC33__INCLUDED_)
