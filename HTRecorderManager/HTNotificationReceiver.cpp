#include "stdafx.h"
#include "HTNotificationReceiver.h"
#include "OutputWnd.h"
#include "StatusLogDAO.h"

HTNotificationReceiver::HTNotificationReceiver(void)
	: m_wndEventListWindow(NULL)
{

}

HTNotificationReceiver::~HTNotificationReceiver(void)
{


}

void HTNotificationReceiver::SetRecorderAddress(wchar_t * address)
{
	memset(m_strRecorderAddress, 0x00, sizeof(m_strRecorderAddress));
	wcscpy(m_strRecorderAddress, address);
}

wchar_t * HTNotificationReceiver::GetRecorderAddress(void)
{
	return m_strRecorderAddress;
}

VOID HTNotificationReceiver::SetEventListWindow(CWnd * wnd)
{
	m_wndEventListWindow = wnd;
}

CWnd* HTNotificationReceiver::GetEventListWindow(void)
{
	return m_wndEventListWindow;
}

void HTNotificationReceiver::OnConnectionStop(RS_CONNECTION_STOP_NOTIFICATION_T * notification)
{
	CString strMsg;
	strMsg = GetRecorderAddress();
	strMsg += TEXT(" ���ڴ� ������ �ٸ� �����ڰ� �α����Ͽ����ϴ�.");
	if (m_wndEventListWindow)
		((COutputWnd*)(m_wndEventListWindow))->AddString2OutputEvent(strMsg);

	STATUS_LOG_T log;
	wcscpy(log.uuid, m_strRecorderAddress);
	log.level = LEVEL_INFO;
	log.category = CATEGORY_SYSTEM;
	log.resid = 0;
	wcscpy(log.contents, (LPWSTR)(LPCWSTR)strMsg);

	StatusLogDAO dao;
	dao.CreateStatusLog(&log);
}

void HTNotificationReceiver::OnRecordingStorageFull(RS_STORAGE_FULL_NOTIFICATION_T *notification)
{
	CString strMsg;
	strMsg = GetRecorderAddress();
	strMsg += TEXT(" ���ڴ� ������ ���� ������ �����մϴ�.");
	if (m_wndEventListWindow)
		((COutputWnd*)(m_wndEventListWindow))->AddString2OutputEvent(strMsg);

	STATUS_LOG_T log;
	wcscpy(log.uuid, m_strRecorderAddress);
	log.level = LEVEL_CIRITICAL;
	log.category = CATEGORY_STORAGE;
	log.resid = 0;
	wcscpy(log.contents, (LPWSTR)(LPCWSTR)strMsg);

	StatusLogDAO dao;
	dao.CreateStatusLog(&log);
}

void HTNotificationReceiver::OnReservedStorageFull(RS_STORAGE_FULL_NOTIFICATION_T *notification)
{
	CString strMsg;
	strMsg = GetRecorderAddress();
	strMsg += TEXT(" ���ڴ� ������ ���� ���� ������ �����մϴ�.");
	if (m_wndEventListWindow)
		((COutputWnd*)(m_wndEventListWindow))->AddString2OutputEvent(strMsg);

	STATUS_LOG_T log;
	wcscpy(log.uuid, m_strRecorderAddress);
	log.level = LEVEL_CIRITICAL;
	log.category = CATEGORY_STORAGE;
	log.resid = 0;
	wcscpy(log.contents, (LPWSTR)(LPCWSTR)strMsg);

	StatusLogDAO dao;
	dao.CreateStatusLog(&log);
}

void HTNotificationReceiver::OnOverwritingError(RS_OVERWRITE_ERROR_NOTIFICATION_T *notification)
{
	CString strMsg;
	strMsg = GetRecorderAddress();
	strMsg += TEXT(" ���ڴ� ������ ����⿡ �����Ͽ����ϴ�.");
	if (m_wndEventListWindow)
		((COutputWnd*)(m_wndEventListWindow))->AddString2OutputEvent(strMsg);

	STATUS_LOG_T log;
	wcscpy(log.uuid, m_strRecorderAddress);
	log.level = LEVEL_CIRITICAL;
	log.category = CATEGORY_STORAGE;
	log.resid = 0;
	wcscpy(log.contents, (LPWSTR)(LPCWSTR)strMsg);

	StatusLogDAO dao;
	dao.CreateStatusLog(&log);
}

void HTNotificationReceiver::OnConfigurationChanged(RS_CONFIGURATION_CHANGED_NOTIFICATION_T *notification)
{
	CString strMsg;
	strMsg = GetRecorderAddress();
	strMsg += TEXT(" ���ڴ� ������ ȯ�� ������ ����Ǿ����ϴ�.");
	if (m_wndEventListWindow)
		((COutputWnd*)(m_wndEventListWindow))->AddString2OutputEvent(strMsg);

	STATUS_LOG_T log;
	wcscpy(log.uuid, m_strRecorderAddress);
	log.level = LEVEL_INFO;
	log.category = CATEGORY_SYSTEM;
	log.resid = 0;
	wcscpy(log.contents, (LPWSTR)(LPCWSTR)strMsg);

	StatusLogDAO dao;
	dao.CreateStatusLog(&log);
}

void HTNotificationReceiver::OnPlaybackError(RS_PLAYBACK_ERROR_NOTIFICATION_T *notification)
{
	CString strMsg;
	strMsg = GetRecorderAddress();
	strMsg += TEXT(" ���ڴ� ������ ����� �����Ͽ����ϴ�.");
	if (m_wndEventListWindow)
		((COutputWnd*)(m_wndEventListWindow))->AddString2OutputEvent(strMsg);

	STATUS_LOG_T log;
	wcscpy(log.uuid, m_strRecorderAddress);
	log.level = LEVEL_WARNING;
	log.category = CATEGORY_PLAYBACK;
	log.resid = 0;
	wcscpy(log.contents, (LPWSTR)(LPCWSTR)strMsg);

	StatusLogDAO dao;
	dao.CreateStatusLog(&log);
}

void HTNotificationReceiver::OnDiskError(RS_DISK_ERROR_NOTIFICATION_T * notification)
{
	CString strMsg;
	strMsg = GetRecorderAddress();
	strMsg += TEXT(" ���ڴ� ������ �����ü(��ũ)�� ������ �߰ߵǾ����ϴ�.");
	if (m_wndEventListWindow)
		((COutputWnd*)(m_wndEventListWindow))->AddString2OutputEvent(strMsg);

	STATUS_LOG_T log;
	wcscpy(log.uuid, m_strRecorderAddress);
	log.level = LEVEL_WARNING;
	log.category = CATEGORY_STORAGE;
	log.resid = 0;
	wcscpy(log.contents, (LPWSTR)(LPCWSTR)strMsg);

	StatusLogDAO dao;
	dao.CreateStatusLog(&log);
}

void HTNotificationReceiver::OnKeyFrameMode(RS_KEY_FRAME_MODE_NOTIFICATION_T * notification)
{
	CString strMsg;
	strMsg = GetRecorderAddress();
	strMsg += TEXT(" ���ڴ� ������ �����ü(��ũ) �ӵ����Ϸ� ����, �����尡 Ű�������� ������� ����Ǿ����ϴ�.");
	if (m_wndEventListWindow)
		((COutputWnd*)(m_wndEventListWindow))->AddString2OutputEvent(strMsg);

	STATUS_LOG_T log;
	wcscpy(log.uuid, m_strRecorderAddress);
	log.level = LEVEL_WARNING;
	log.category = CATEGORY_STORAGE;
	log.resid = 0;
	wcscpy(log.contents, (LPWSTR)(LPCWSTR)strMsg);

	StatusLogDAO dao;
	dao.CreateStatusLog(&log);
}

void HTNotificationReceiver::OnBufferClean(RS_BUFFER_CLEAN_NOTIFICATION_T * notification)
{
	CString strMsg;
	strMsg = GetRecorderAddress();
	strMsg += TEXT(" ���ڴ� ������ �����ü(��ũ) ����ð� �������� ����, ������ ���� ���۸� ���� ���ο� ������ �õ��մϴ�.");
	if (m_wndEventListWindow)
		((COutputWnd*)(m_wndEventListWindow))->AddString2OutputEvent(strMsg);

	STATUS_LOG_T log;
	wcscpy(log.uuid, m_strRecorderAddress);
	log.level = LEVEL_WARNING;
	log.category = CATEGORY_STORAGE;
	log.resid = 0;
	wcscpy(log.contents, (LPWSTR)(LPCWSTR)strMsg);

	StatusLogDAO dao;
	dao.CreateStatusLog(&log);
}