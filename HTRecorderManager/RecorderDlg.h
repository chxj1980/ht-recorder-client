#pragma once
#include "afxwin.h"


// RecorderDlg ��ȭ �����Դϴ�.

class RecorderDlg : public CDialogEx
{
	DECLARE_DYNAMIC(RecorderDlg)

public:
	RecorderDlg(CString siteUuid, CWnd* pParent = NULL);   // ǥ�� �������Դϴ�.
	virtual ~RecorderDlg();

// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_RECORDERDLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

	DECLARE_MESSAGE_MAP()
private:
	CString m_strSiteUuid;
	CEdit m_editRecorderUuid;
	CEdit m_editRecorderName;
	CEdit m_editRecorderAddress;
	CEdit m_editRecorderUsername;
	CEdit m_editRecorderPassword;
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedButtonGenerateUuid();
};
