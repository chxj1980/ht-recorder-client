#pragma once
#include "afxwin.h"


// SiteDlg ��ȭ �����Դϴ�.

class SiteDlg : public CDialogEx
{
	DECLARE_DYNAMIC(SiteDlg)

public:
	SiteDlg(CWnd* pParent = NULL);   // ǥ�� �������Դϴ�.
	virtual ~SiteDlg();

// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_SITEDLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

	DECLARE_MESSAGE_MAP()
private:
	CEdit m_editSiteUuid;
	CEdit m_editSiteName;
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedButtonGenerateSiteUuid();
private:
	
};
