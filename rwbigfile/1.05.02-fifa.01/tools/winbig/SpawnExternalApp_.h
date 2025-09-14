// (c) Electronic Arts. All Rights Reserved.
#pragma once


// CSpawnExternalApp_ dialog

class CSpawnExternalApp_ : public CDialog
{
	DECLARE_DYNAMIC(CSpawnExternalApp_)

public:
	CSpawnExternalApp_(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSpawnExternalApp_();

// Dialog Data
	enum { IDD = IDD_SPAWN_EXTERNAL_APP };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	void SetFileNameToDisplay(CString strFileName);

	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnInitDialog();

private:
	CString m_strFileNameToDisplay;
};
