// SiteDlg.cpp : ���� �����Դϴ�.
//

#include "stdafx.h"
#include "HTRecorderManager.h"
#include "SiteDlg.h"
#include "afxdialogex.h"
#include "GUIDGenerator.h"
#include "SiteDAO.h"
#include "StringUtil.h"


// SiteDlg ��ȭ �����Դϴ�.

IMPLEMENT_DYNAMIC(SiteDlg, CDialogEx)

SiteDlg::SiteDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(SiteDlg::IDD, pParent)
{

}

SiteDlg::~SiteDlg()
{
}

void SiteDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_SITE_UUID, m_editSiteUuid);
	DDX_Control(pDX, IDC_EDIT_SITE_NAME, m_editSiteName);
}


BEGIN_MESSAGE_MAP(SiteDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &SiteDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &SiteDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_BUTTON_GENERATE_SITE_UUID, &SiteDlg::OnBnClickedButtonGenerateSiteUuid)
END_MESSAGE_MAP()


// SiteDlg �޽��� ó�����Դϴ�.


void SiteDlg::OnBnClickedOk()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	CString site_uuid;
	CString site_name;
	m_editSiteUuid.GetWindowTextW(site_uuid);
	m_editSiteName.GetWindowTextW(site_name);

	if (site_uuid.GetLength() < 1 || site_name.GetLength() < 1)
		return;

	SITE_T site;
	wcscpy(site.uuid, (LPWSTR)(LPCWSTR)site_uuid);
	wcscpy(site.name, (LPWSTR)(LPCWSTR)site_name);

	SiteDAO dao;
	dao.CreateSite(&site);

	CDialogEx::OnOK();
}


void SiteDlg::OnBnClickedCancel()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	CDialogEx::OnCancel();
}


void SiteDlg::OnBnClickedButtonGenerateSiteUuid()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	m_editSiteUuid.SetWindowText(GuidGenerator::generate_widechar_string_guid());
}
