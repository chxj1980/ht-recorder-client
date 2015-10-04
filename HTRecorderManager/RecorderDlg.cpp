// RecorderDlg.cpp : ���� �����Դϴ�.
//

#include "stdafx.h"
#include "HTRecorderManager.h"
#include "RecorderDlg.h"
#include "afxdialogex.h"
#include "GUIDGenerator.h"
#include "RecorderDAO.h"

// RecorderDlg ��ȭ �����Դϴ�.

IMPLEMENT_DYNAMIC(RecorderDlg, CDialogEx)

RecorderDlg::RecorderDlg(CString siteUuid, CWnd* pParent /*=NULL*/)
	: CDialogEx(RecorderDlg::IDD, pParent)
	, m_strSiteUuid(siteUuid)
{

}

RecorderDlg::~RecorderDlg()
{
}

void RecorderDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_RECORDER_UUID, m_editRecorderUuid);
	DDX_Control(pDX, IDC_EDIT_RECORDER_NAME, m_editRecorderName);
	DDX_Control(pDX, IDC_EDIT_RECORDER_ADDRESS, m_editRecorderAddress);
	DDX_Control(pDX, IDC_EDIT_RECORDER_USERNAME, m_editRecorderUsername);
	DDX_Control(pDX, IDC_EDIT_RECORDER_PASSWORD, m_editRecorderPassword);
}


BEGIN_MESSAGE_MAP(RecorderDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &RecorderDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &RecorderDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_BUTTON_GENERATE_UUID, &RecorderDlg::OnBnClickedButtonGenerateUuid)
END_MESSAGE_MAP()


// RecorderDlg �޽��� ó�����Դϴ�.


void RecorderDlg::OnBnClickedOk()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	CString uuid;
	CString name;
	CString address;
	CString username;
	CString password;
	m_editRecorderUuid.GetWindowTextW(uuid);
	m_editRecorderName.GetWindowTextW(name);
	m_editRecorderAddress.GetWindowTextW(address);
	m_editRecorderUsername.GetWindowTextW(username);
	m_editRecorderPassword.GetWindowTextW(password);

	if (uuid.GetLength() < 1 || name.GetLength() < 1 || address.GetLength() < 1 || username.GetLength() < 1 || password.GetLength() < 1)
		return;

	SITE_T site;
	wcscpy(site.uuid, (LPWSTR)(LPCWSTR)m_strSiteUuid);

	RECORDER_T recorder;
	wcscpy(recorder.uuid, (LPWSTR)(LPCWSTR)uuid);
	wcscpy(recorder.name, (LPWSTR)(LPCWSTR)name);
	wcscpy(recorder.address, (LPWSTR)(LPCWSTR)address);
	wcscpy(recorder.username, (LPWSTR)(LPCWSTR)username);
	wcscpy(recorder.pwd, (LPWSTR)(LPCWSTR)password);

	RecorderDAO dao;
	dao.CreateRecorder(&site, &recorder);

	CDialogEx::OnOK();
}


void RecorderDlg::OnBnClickedCancel()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	CDialogEx::OnCancel();
}


void RecorderDlg::OnBnClickedButtonGenerateUuid()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	m_editRecorderUuid.SetWindowText(GuidGenerator::generate_widechar_string_guid());
}
