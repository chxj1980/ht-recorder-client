#include "stdafx.h"
#include "HTNotificationReceiver.h"
#include "OutputWnd.h"
#include "StatusLogDAO.h"
#include "HTNotificationReceiverFactory.h"

HTNotificationReceiver::HTNotificationReceiver(void)
	: m_wndEventListWindow(NULL)
{
	unsigned int thrdAddr;
	m_pollThread = (HANDLE)(::_beginthreadex(NULL, 0, &HTNotificationReceiver::PollProcess, this, 0, &thrdAddr));
}

HTNotificationReceiver::~HTNotificationReceiver(void)
{
	m_bPoll = FALSE;
	::WaitForSingleObject(m_pollThread, INFINITE);
	::CloseHandle(m_pollThread);
}

void HTNotificationReceiver::SetHTRecorder(HTRecorder * recorder)
{
	m_pHTRecorder = recorder;
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

unsigned __stdcall HTNotificationReceiver::PollProcess(VOID * param)
{
	HTNotificationReceiver * self = static_cast<HTNotificationReceiver*>(param);
	self->Poll();
	return 0;
}

void HTNotificationReceiver::Poll(void)
{
	m_bPoll = TRUE;
	while (m_bPoll)
	{
		if (wcslen(m_strRecorderAddress) > 0)
		{
			RS_DEVICE_INFO_SET_T devices;
			RS_DEVICE_STATUS_SET_T status;
			BOOL result = m_pHTRecorder->GetDeviceList(&devices);
			if (result)
			{
				result = m_pHTRecorder->CheckDeviceStatus(&devices, &status);
				if (result)
				{
					//status.validDeviceCount
					for (int index = 0; index < status.validDeviceCount; index++)
					{
						if (status.deviceStatusInfo[index].nIsConnected)
						{
							CString strMsg = TEXT("Device");
							strMsg += TEXT("[");
							strMsg += devices.deviceInfo[index].GetURL();
							strMsg += TEXT("] Associated with ");
							strMsg += GetRecorderAddress();
							strMsg += TEXT(" Recorder Server is Connected.");
							if (m_wndEventListWindow)
								((COutputWnd*)(m_wndEventListWindow))->AddString2OutputEvent(strMsg);
						}
						else
						{
							CString strMsg = TEXT("Device");
							strMsg += TEXT("[");
							strMsg += devices.deviceInfo[index].GetURL();
							strMsg += TEXT("] Associated with ");
							strMsg += GetRecorderAddress();
							strMsg += TEXT(" Recorder Server is Disconnected.");
							if (m_wndEventListWindow)
								((COutputWnd*)(m_wndEventListWindow))->AddString2OutputEvent(strMsg);
						}

						if (status.deviceStatusInfo[index].nIsRecordingError)
						{
							CString strMsg = TEXT("Device");
							strMsg += TEXT("[");
							strMsg += devices.deviceInfo[index].GetURL();
							strMsg += TEXT("] Associated with ");
							strMsg += GetRecorderAddress();
							strMsg += TEXT(" Recorder Server is Under Recording Error.");
							if (m_wndEventListWindow)
								((COutputWnd*)(m_wndEventListWindow))->AddString2OutputEvent(strMsg);
						}
					}
				}
			}
		}
		::Sleep(3000);
	}
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