#pragma once

#include <RecorderDLL.h>
#include <LiveSession5.h>
#include <LiveSessionDLL.h>

enum _exportRecordFile
{
	RECORDER_EXPORT_OK = 0,
	RECORDER_CONNECTION_FAIL,
	RECORDER_NOT_EXIST_EXPORT_FILE,
};


class CGroupInfo;
class CChannelInfo;
class CChannelManager;
class NotificationReceiver;
struct VCAM_STREAM_URL;
typedef struct _RECORD_TIME
{
	UINT year;
	UINT month;
	UINT day;
	UINT hour;
	UINT minute;
	UINT second;
	_RECORD_TIME( VOID )
	{
		year = 0;
		month = 0;
		day = 0;
		hour = 0;
		minute = 0;
		second = 0;
	}
	_RECORD_TIME( const _RECORD_TIME& clone )
	{
		year = clone.year;
		month = clone.month;
		day = clone.day;
		hour = clone.hour;
		minute = clone.minute;
		second = clone.second;
	}
	_RECORD_TIME& operator = ( const _RECORD_TIME& clone )
	{
		year = clone.year;
		month = clone.month;
		day = clone.day;
		hour = clone.hour;
		minute = clone.minute;
		second = clone.second;
		return (*this);
	}
} RECORD_TIME;

typedef struct _RECORD_TIME_INFO
{
	RECORD_TIME startTime;
	RECORD_TIME endTime;
	_RECORD_TIME_INFO( VOID ) {}
	_RECORD_TIME_INFO( const _RECORD_TIME_INFO& clone )
	{
		startTime = clone.startTime;
		endTime = clone.endTime;
	}
	_RECORD_TIME_INFO& operator = ( const _RECORD_TIME_INFO& clone )
	{
		startTime = clone.startTime;
		endTime = clone.endTime;
		return (*this);
	}
} RECORD_TIME_INFO;

typedef std::vector<CString> VCamUUIDList;
typedef std::map<CString,IStreamReceiver5*> PBStreamReceiverList; //CString -> group UUID or vcam UUID, IStreamReceiver5 -> playback streamer
typedef std::multimap<CString,IStreamReceiver5*> RLStreamReceiverList; //CString -> group UUID or vcam UUID, IStreamReceiver5 -> relay streamer


class RecorderIF
{
public:
	RecorderIF( BOOL bRunAsRecorder=FALSE );
	~RecorderIF( VOID );

	//BOOL	IsConnected( RS_SERVER_INFO_T *serverInfo );
	//BOOL	Reconnect( RS_SERVER_INFO_T *serverInfo );

	VOID	KillRelayStream( VOID );
	VOID	KillPlayBackStream( VOID );

/////////////////////////////////// VMS_RECORDER ������� ///////////////////////////////////////////////
	///////////////////////  DEVICE  /////////////////////////
	//Ư�� ���ڴ��� ���� ��ϵ� ī�޶� ��ϸ� ��ȸ�Ѵ�.
	BOOL	GetDeviceList( RS_SERVER_INFO_T *serverInfo, RS_DEVICE_INFO_SET_T *deviceInfoList );
	//������ ������ �ִ� ī�޶� ��ϰ� ���� Client�� ������ �ִ� group�� ī�޶� ������ ���Ͽ� Ʋ���� FALSE�� �����Ѵ�.
	BOOL	CheckDeviceList( RS_SERVER_INFO_T *serverInfo, CGroupInfo *group );

	//�׷쳻�� �����ִ� ī�޶� ������ Recorder�� �ϰ� ����Ѵ�.
	BOOL	AddDevice( CGroupInfo *group );
	//ä�θ�ϳ��� �����ִ� ī�޶� ������ Recorder�� �ϰ� ����Ѵ�.
	BOOL	AddDevice( std::vector<CChannelInfo*> *chennelInfos );
	//���� ī�޶� ������ Recorder�� ����Ѵ�.
	BOOL	AddDevice( CChannelInfo *chInfo );

	//�׷쳻�� �����ִ� ī�޶� ������ Recorder�κ��� �ϰ� �����Ѵ�.
	BOOL	RemoveDevice( CGroupInfo *group );
	//ä�θ�ϳ��� �����ִ� ī�޶� ������ Recorder�κ��� �ϰ� �����Ѵ�.
	BOOL	RemoveDevice( std::vector<CChannelInfo*> *chennelInfos );
	//���� ī�޶� ������ Recorder�κ��� �����Ѵ�.
	BOOL	RemoveDevice( CChannelInfo *chInfo );

	//�׷쳻�� �����ִ� ī�޶� ������ Recorder�κ��� �ϰ� �����Ѵ�.
	BOOL	RemoveDeviceEx( CGroupInfo *group );
	//ä�θ�ϳ��� �����ִ� ī�޶� ������ Recorder�κ��� �ϰ� �����Ѵ�.
	BOOL	RemoveDeviceEx( std::vector<CChannelInfo*> *chennelInfos );
	//���� ī�޶� ������ Recorder�κ��� �����Ѵ�.
	BOOL	RemoveDeviceEx( CChannelInfo *chInfo );



	//�׷쳻�� �����ִ� ī�޶��� ���������� ��ȸ�Ѵ�.
	BOOL	CheckDeviceStatus( CGroupInfo *group, RS_DEVICE_STATUS_SET_T *deviceStatusList );
	//ä�θ�ϳ��� �����ִ� ī�޶��� ���������� ��ȸ�Ѵ�.
	BOOL	CheckDeviceStatus( std::vector<CChannelInfo*> *chennelInfos, RS_DEVICE_STATUS_SET_T *deviceStatusList );
	//���� ī�޶��� ���������� ��ȸ�Ѵ�.
	BOOL	CheckDeviceStatus( CChannelInfo *chInfo, RS_DEVICE_STATUS_SET_T *deviceStatusList );

	///////////////// RECORDING PRE-SETUP
	//ī�޶� �׷쿡 ���� ���� ������ ������ �����´�.
	BOOL	GetRecordingScheduleList( CGroupInfo *group, RS_RECORD_SCHEDULE_SET_T *recordShcedList );
	//ä�θ�Ͽ� ���� ���� ������ ������ �����´�.
	BOOL	GetRecordingScheduleList( std::vector<CChannelInfo*> *chInfos, RS_RECORD_SCHEDULE_SET_T *recordShcedList );
	//���� ī�޶� ���� ���� ������ ������ �����´�.
	BOOL	GetRecordingScheduleList( CChannelInfo *chInfo, RS_RECORD_SCHEDULE_SET_T *recordShcedList );

	//�׷쳻�� �����ִ� ī�޶� ���� ���� ������ ������ �����Ѵ�.
	/*
	schedType : 0-NONE 
	            1-�׻� ���ڵ�
		        2-�̺�Ʈ ���ڵ�
				3-ADAPTIVE ���ڵ� (�̺�Ʈ �߻� �ÿ��� pre, post �ð��� �����Ͽ� ��� �������� �����ϰ� �� �ܿ��� Ű�����Ӹ� ����)
	audio : ����� ���忩��
	*/
	BOOL	UpdateRecordingSchedule( CGroupInfo *group, UINT schedType, BOOL audio, 
									 UINT bitSun, UINT bitMon, UINT bitTue, UINT bitWed, UINT bitThu, UINT bitFri, UINT bitSat, 		
									 UINT preRecordingTime, UINT postRecordingTime );
	//ä�� ��ϳ��� �����ִ� ī�޶� ���� ���� ������ ������ �����Ѵ�.
	BOOL	UpdateRecordingSchedule( std::vector<CChannelInfo*> *chInfos, UINT schedType, BOOL audio, 
									 UINT bitSun, UINT bitMon, UINT bitTue, UINT bitWed, UINT bitThu, UINT bitFri, UINT bitSat, 		
									 UINT preRecordingTime, UINT postRecordingTime );
	//���� ī�޶� ���� ���� ������ ������ �����Ѵ�.
	BOOL	UpdateRecordingSchedule( CChannelInfo *chInfo, UINT schedType, BOOL audio, 
									 UINT bitSun, UINT bitMon, UINT bitTue, UINT bitWed, UINT bitThu, UINT bitFri, UINT bitSat, 
									 UINT preRecordingTime, UINT postRecordingTime );

	//�׷쳻 ī�޶�� ������ �ټ��� ���ڴ��� ���� overwrite ������ enable/disable �Ѵ�.
	BOOL	SetRecordingOverwrite( CGroupInfo *group, BOOL onoff );
	//ä�θ�ϳ� ī�޶�� ������ �ټ��� ���ڴ��� ���� overwrite ������ enable/disable �Ѵ�.
	BOOL	SetRecordingOverwrite( std::vector<CChannelInfo*> *chInfos, BOOL onoff );
	//Ư�� ���ڴ��� ���� overwrite ������ enable/disable �Ѵ�.
	BOOL	SetRecordingOverwrite( RS_SERVER_INFO_T *serverInfo, BOOL onoff );

	//Ư�� ���ڴ��� ���� overwrite ������ �����´�.
	BOOL	GetRecordingOverwrite( RS_SERVER_INFO_T *serverInfo, BOOL *onoff );

	//�׷쳻 ī�޶�� ������ �ټ��� ���ڴ��� retentiontime�� enable/disable�ϰ�, enable�ÿ� �ش� ����� �����Ѵ�.
	BOOL	SetRecordingRetentionTime( CGroupInfo *group, BOOL enable, UINT year, UINT month, UINT week, UINT day );
	//ä�θ�ϳ� ī�޶�� ������ �ټ��� ���ڴ��� retentiontime�� enable/disable�ϰ�, enable�ÿ� �ش� ����� �����Ѵ�.
	BOOL	SetRecordingRetentionTime( std::vector<CChannelInfo*> *chInfos, BOOL enable, UINT year, UINT month, UINT week, UINT day );
	//Ư�� ���ڴ��� retentiontime�� enable/disable�ϰ�, enable�ÿ� �ش� ����� �����Ѵ�.
	BOOL	SetRecordingRetentionTime( RS_SERVER_INFO_T *serverInfo, BOOL enable, UINT year, UINT month, UINT week, UINT day );


	//Ư�� ���ڴ��� retentiontime ���� �����´�.
	BOOL	GetRecordingRetentionTime( RS_SERVER_INFO_T *serverInfo, BOOL *enable, UINT *year, UINT *month, UINT *week, UINT *day );


	BOOL	GetDiskInfo( RS_SERVER_INFO_T *serverInfo, RS_DISK_INFO_SET_T *diskInfoList );
	
	//�׷쳻 ī�޶�� ������ �ټ��� ���ڴ��� �����ִ� Ư�� ��ũ(strVolumeSerial)�� recordingSize��ŭ�� ��������� �Ҵ��Ѵ�.
	//BOOL	ReserveDiskSpace( CGroupInfo *group, CString strVolumeSerial, UINT64 recordingSize );
	//ä�θ�ϳ� ī�޶�� ������ �ټ��� ���ڴ��� �����ִ� Ư�� ��ũ(strVolumeSerial)�� recordingSize��ŭ�� ��������� �Ҵ��Ѵ�.
	//BOOL	ReserveDiskSpace( std::vector<CChannelInfo*> *chInfos, CString strVolumeSerial, UINT64 recordingSize );
	//Ư�� ���ڴ��� �����ִ� Ư�� ��ũ(strVolumeSerial)�� recordingSize��ŭ�� ��������� �Ҵ��Ѵ�.
	BOOL	ReserveDiskSpace( RS_SERVER_INFO_T *serverInfo, CString strVolumeSerial, UINT64 recordingSize );

	//Ư�� ���ڴ��� �Ҵ�� ī�޶��� ���� ��ġ(��ũ��ġ)�� ���� �Ѵ�. 
	//�ι�° �Ķ����[std::map<CString,VCamUUIDList> *vcamUUIDList]�� 
	//CString�� Ư�� ��ũ�� volumeSerial�̸�, VCamUUIDList�� ī�޶� ���� UUID vector�̴�.
	BOOL	GetDiskPolicy( RS_SERVER_INFO_T *serverInfo, std::map<CString,VCamUUIDList> *vcamUUIDList );

	//ī�޶� ���� ��ũ �Ҵ��� �ϳ��� ���ڴ����� ������� �Ѵ�. - CGroupInfo �Ǵ� std::vector<CChannelInfo*>�� ī�޶�� ��� ������ ���ڴ���
	//�׷쿡 ���� ī�޶� Ư�� ��ũ�� �Ҵ� - ��ũ �Ҵ翡 �����ϰų� strVolumeSerial�� ������ ��ũ�� ã�� ���� ��� FALSE ����
	BOOL	UpdateDiskPolicy( CGroupInfo *group, CString strVolumeSerial );

	//ä�ο� ���� ī�޶� Ư�� ��ũ�� �Ҵ� - ��ũ �Ҵ翡 �����ϰų� strVolumeSerial�� ������ ��ũ�� ã�� ���� ��� FALSE ����
	BOOL	UpdateDiskPolicy( std::vector<CChannelInfo*> *chInfos, CString strVolumeSerial );

	//���� ī�޶� Ư�� ��ũ�� �Ҵ� - ��ũ �Ҵ翡 �����ϰų� strVolumeSerial�� ������ ��ũ�� ã�� ���� ��� FALSE ����
	BOOL	UpdateDiskPolicy( CChannelInfo *chInfo, CString strVolumeSerial );

	//��ϵ� ��� ī�޶� Ư�� ��ũ�� �Ҵ� - ��ũ �Ҵ翡 �����ϰų� strVolumeSerial�� ������ ��ũ�� ã�� ���� ��� FALSE ����
	BOOL	UpdateDiskPolicy( CChannelManager *chMgr, CString strVolumeSerial );

/////////////////////////////////// VMS_RECORDER �� VMS_VIEWER ������ ///////////////////////////////////////////////
	///////////////// RELAY
	//BOOL	GetRelayInfo( VOID *xmlData );
	BOOL	StartRelay( CGroupInfo *group );
	BOOL	StartRelay( CString groupUUID, std::vector<CChannelInfo*> *chInfos );
	BOOL	StartRelay( CChannelInfo *chInfo );
	
	BOOL	UpdateRelay( CGroupInfo *group );
	BOOL	UpdateRelay( CString groupUUID, std::vector<CChannelInfo*> *chInfos );
	BOOL	UpdateRelay( CChannelInfo *chInfo );

	BOOL	StopRelay( CGroupInfo *group );
	BOOL	StopRelay( CString groupUUID, std::vector<CChannelInfo*> *chInfos );
	BOOL	StopRelay( CChannelInfo *chInfo );

	///////////////// RECORDING
	//�׷쿡 ���� ī�޶��� ���ڵ��� �����쿡 ���� �۵��� �غ� �Ǿ����� Ȯ��
	BOOL	IsRecording( CGroupInfo *group, RS_RECORDING_STATUS_SET_T *recordingStatusList );
	//ä�� ��Ͽ� ���� ī�޶��� ���ڵ��� �����쿡 ���� �۵��� �غ� �Ǿ����� Ȯ��
	BOOL	IsRecording( std::vector<CChannelInfo*> *chInfos, RS_RECORDING_STATUS_SET_T *recordingStatusList );
	//���� ī�޶� ���� ���ڵ��� �����쿡 ���� �۵��� �غ� �Ǿ����� Ȯ��
	BOOL	IsRecording( CChannelInfo *chInfo, RS_RECORDING_STATUS_SET_T *recordingStatusList );

	//�׷쿡 ���� ī�޶��� ���ڵ� ����
	BOOL	StartRecordingRequest( CGroupInfo *group );
	//ä�� ��Ͽ� ���� ī�޶��� ���ڵ� ����
	BOOL	StartRecordingRequest( std::vector<CChannelInfo*> *chInfos );
	//���� ī�޶� ���� ���ڵ� ����(�������� �����ϰų� ���ڵ����̸� FALSE ����)
	BOOL	StartRecordingRequest( CChannelInfo *chInfo );

	//�׷쿡 ���� ī�޶��� ���ڵ� ����
	BOOL	StopRecordingRequest( CGroupInfo *group );
	//ä�� ��Ͽ� ���� ī�޶��� ���ڵ� ����
	BOOL	StopRecordingRequest( std::vector<CChannelInfo*> *chInfos );
	//���� ī�޶� ���� ���ڵ� ����(��������� �����ϰų� ���ڵ� ���� �ƴϸ� FALSE ����)
	BOOL	StopRecordingRequest( CChannelInfo *chInfo );

	//�׷쿡 ���� ī�޶󿬰��� ���ڵ� ���� �ϰ� ���ڵ� ����(���ڵ忡 ��ϵ� ��� ī�޶� ���ڵ��� ������)
	BOOL	StartRecordingAll( CGroupInfo *group );
	//ä�θ�Ͽ� ���� ī�޶󿬰��� ���ڵ� ���� �ϰ� ���ڵ� ����(���ڵ忡 ��ϵ� ��� ī�޶� ���ڵ��� ������)
	BOOL	StartRecordingAll( std::vector<CChannelInfo*> *chInfos );
	//Ư�� ���ڵ忡 ��ϵ� ī�޶� ���� �ϰ� ���ڵ� ����
	BOOL	StartRecordingAll( RS_SERVER_INFO_T *serverInfo );

	//�׷쿡 ���� ī�޶󿬰��� ���ڵ� ���� �ϰ� ���ڵ� ����(���ڵ忡 ��ϵ� ��� ī�޶� ���ڵ��� ������)
	BOOL	StopRecordingAll( CGroupInfo *group );
	//�׷쿡 ���� ī�޶󿬰��� ���ڵ� ���� �ϰ� ���ڵ� ����(���ڵ忡 ��ϵ� ��� ī�޶� ���ڵ��� ������)
	BOOL	StopRecordingAll( std::vector<CChannelInfo*> *chInfos );
	//Ư�� ���ڵ忡 ��ϵ� ī�޶� ���� �ϰ� ���ڵ� ����
	BOOL	StopRecordingAll( RS_SERVER_INFO_T *serverInfo );

	//���� ���ڵ忡�� ���ڵ��ǰ� �ִ� ī�޶�(�׷�)�� ���ڵ� ���� ����
	BOOL	DeleteRecordingData( CGroupInfo *group );
	//���� ���ڵ忡�� ���ڵ��ǰ� �ִ� ī�޶�(ä��)�� ���ڵ� ���� ����
	BOOL	DeleteRecordingData( std::vector<CChannelInfo*> *chInfos );
	//���� ���ڵ忡�� ���ڵ��ǰ� �ִ� ���� ī�޶��� ���ڵ� ���� ����
	BOOL	DeleteRecordingData( CChannelInfo *chInfo );

	///////////////// PLAYBACK STREAM
	//CALENDAR SEARCH
	//�׷쳻�� �����ִ� ī�޶��� ��/��/��/��/���� �ð������� �����´�.
	BOOL	GetTimeIndex( CGroupInfo *group, UINT year, std::map<CChannelInfo*,CPtrArray> *time );
	//ä�� ��ϳ��� �����ִ� ī�޶��� ��/��/��/��/���� �ð������� �����´�.
	BOOL	GetTimeIndex( std::vector<CChannelInfo*> *chInfos, UINT year, std::map<CChannelInfo*,CPtrArray> *time );

	//�׷쳻�� �����ִ� ī�޶�� ������ ����(Recording) �ð������� �����´�.
	BOOL	GetDayIndex( CGroupInfo *group, UINT year, UINT month, UINT day );
	//ä�θ�ϳ��� �����ִ� ī�޶�� ������ ����(Recording) �ð������� �����´�.
	BOOL	GetDayIndex( std::vector<CChannelInfo*> *chInfos, UINT year, UINT month, UINT day );
	//���� ī�޶�� ������ ����(Recording) �ð������� �����´�.
	BOOL	GetDayIndex( CChannelInfo *chInfo, UINT year, UINT month, UINT day );
	//���� ī�޶�� ������ ����(Recording) �ð������� �����´�.-���糯¥�κ��� ���ϴ� �Ⱓ(�ϴ���)�� ������ �´�.
	BOOL	GetPeriodIndex( CChannelInfo *chInfo, int period );
	//���� ī�޶�� ������ ����(Recording) �ð������� �����´�.-����ð����κ��� ���ϴ� �Ⱓ(�д���)�� ������ �´�.
	BOOL RecorderIF::GetRecentMinIndex( CChannelInfo *chInfo, CTime curTime, UINT min );
	//���� ī�޶�� ������ ����(Recording) �ð������� �����´�.-����ð����κ��� ���ϴ� �Ⱓ(�д���)�� ������ �´�.
	int RecorderIF::GetRecordedTimeExport( CChannelInfo *chInfo, CTime startTime, CTime endTime);

/*
	//�׷쳻�� �����ִ� ī�޶��� �������� �����´�.
	BOOL	GetYearIndex( CGroupInfo *group, UINT year, std::map<CChannelInfo*,PlayBackTimeInfo> *month );
	//ä�θ�ϳ��� �����ִ� ī�޶��� �������� �����´�.
	BOOL	GetYearIndex( std::vector<CChannelInfo*> *chInfos, UINT year, std::map<CChannelInfo*,PlayBackTimeInfo> *month );

	//�׷쳻�� �����ִ� ī�޶��� �������� �����´�.
	BOOL	GetMonthIndex( CGroupInfo *group, UINT year, UINT month, std::map<CChannelInfo*,PlayBackMonthInfo> *day );
	//ä�θ�ϳ��� �����ִ� ī�޶��� �������� �����´�.
	BOOL	GetMonthIndex( std::vector<CChannelInfo*> *chInfos, UINT year, UINT month, std::map<CChannelInfo*,PlayBackMonthInfo> *day );

	//�׷쳻�� �����ִ� ī�޶��� �ð������� �����´�.
	BOOL	GetDayIndex( CGroupInfo *group, UINT year, UINT month, UINT day, std::map<CChannelInfo*,PlayBackDayInfo> *hour );
	//ä�θ�ϳ��� �����ִ� ī�޶��� �ð������� �����´�.
	BOOL	GetDayIndex( std::vector<CChannelInfo*> *chInfos, UINT year, UINT month, UINT day, std::map<CChannelInfo*,PlayBackDayInfo> *hour );

	//�׷쳻�� �����ִ� ī�޶��� �������� �����´�.
	BOOL	GetHourIndex( CGroupInfo *group, UINT year, UINT month, UINT day, UINT hour, std::map<CChannelInfo*,PlayBackHourInfo> *minute );
	//ä�θ�ϳ��� �����ִ� ī�޶��� �������� �����´�.
	BOOL	GetHourIndex( std::vector<CChannelInfo*> *chInfos, UINT year, UINT month, UINT day, UINT hour, std::map<CChannelInfo*,PlayBackHourInfo> *minute );
*/


	//PLAYBACK
	//�׷쳻�� �����ִ� ī�޶��� PlayBack�� ��û�Ѵ�.
	BOOL	StartPlayback( CGroupInfo *group, UINT year, UINT month, UINT day, UINT hour, UINT minute, UINT second=0 );
	//ä�θ�ϳ��� �����ִ� ī�޶��� PlayBack�� ��û�Ѵ�.
	BOOL	StartPlayback( CString groupUUID, std::vector<CChannelInfo*> *chInfos, UINT year, UINT month, UINT day, UINT hour, UINT minute, UINT second=0 );
	//����ī�޶� ���� PlayBack�� ��û�Ѵ�.
	BOOL	StartPlayback( CChannelInfo *chInfo, UINT year, UINT month, UINT day, UINT hour, UINT minute, UINT second=0 );

	//�׷쳻�� �����ִ� ī�޶��� PlayBack�� �����Ѵ�.
	BOOL	StopPlayback( CGroupInfo *group );
	//ä�θ�ϳ��� �����ִ� ī�޶��� PlayBack�� �����Ѵ�.
	BOOL	StopPlayback( CString groupUUID, std::vector<CChannelInfo*> *chInfos );
	//���� ī�޶� ���� PlayBack�� �����Ѵ�.
	BOOL	StopPlayback( CChannelInfo *chInfo );

	//PLAYBACK CONTROL
	//�׷쳻�� �����ִ� ī�޶��� Play�� ������Ѵ�.(pause���� �Ǵ� forward/backward replay ���¿���)
	BOOL	ControlPlay( CGroupInfo *group );
	//ä�θ�ϳ��� �����ִ� ī�޶��� Play�� ������Ѵ�.(pause���� �Ǵ� forward/backward replay ���¿���)
	BOOL	ControlPlay( CString groupUUID, std::vector<CChannelInfo*> *chInfos );
	//���� ī�޶��� Play�� ������Ѵ�.(pause���� �Ǵ� forward/backward replay ���¿���)
	BOOL	ControlPlay( CChannelInfo *chInfo );

	//�׷쳻�� �����ִ� ī�޶��� Play�� �����Ѵ�.
	BOOL	ControlStop( CGroupInfo *group );
	//ä�θ�ϳ��� �����ִ� ī�޶��� Play�� �����Ѵ�.
	BOOL	ControlStop( CString groupUUID, std::vector<CChannelInfo*> *chInfos );
	//���� ī�޶��� Play�� �����Ѵ�.
	BOOL	ControlStop( CChannelInfo *chInfo );

	//�׷쳻�� �����ִ� ī�޶��� Play�� �ӽ������Ѵ�.
	BOOL	ControlPause( CGroupInfo *group );
	//ä�θ�ϳ��� �����ִ� ī�޶��� Play�� �ӽ������Ѵ�.
	BOOL	ControlPause( CString groupUUID, std::vector<CChannelInfo*> *chInfos );
	//���� ī�޶��� Play�� �ӽ������Ѵ�.
	BOOL	ControlPause( CChannelInfo *chInfo );

	//�׷쳻�� �����ִ� ī�޶��� �ӽ������� �����Ѵ�.
	BOOL	ControlResume( CGroupInfo *group );
	//ä�θ�ϳ��� �����ִ� ī�޶��� �ӽ������� �����Ѵ�.
	BOOL	ControlResume( CString groupUUID, std::vector<CChannelInfo*> *chInfos );
	//���� ī�޶��� Play�� �ӽ������� �����Ѵ�.
	BOOL	ControlResume( CChannelInfo *chInfo );

	//�׷쳻�� �����ִ� ī�޶��� ������ ������� forward play�Ѵ�. �⺻������ key frame ������ �����Ѵ�.
	BOOL	ControlFowardPlay( CGroupInfo *group, UINT speed );
	//ä�θ�ϳ��� �����ִ� ī�޶��� ������ ������� forward play�Ѵ�. �⺻������ key frame ������ �����Ѵ�.
	BOOL	ControlFowardPlay( CString groupUUID, std::vector<CChannelInfo*> *chInfos, UINT speed );
	//���� ī�޶��� ������ ������� forward play�Ѵ�. �⺻������ key frame ������ �����Ѵ�.
	BOOL	ControlFowardPlay( CChannelInfo *chInfo, UINT speed );

	//�׷쳻�� �����ִ� ī�޶��� ������ ������� backward play�Ѵ�. �⺻������ key frame ������ �����Ѵ�.
	BOOL	ControlBackwardPlay( CGroupInfo *group, UINT speed );
	//ä�θ�ϳ��� �����ִ� ī�޶��� ������ ������� backward play�Ѵ�. �⺻������ key frame ������ �����Ѵ�.
	BOOL	ControlBackwardPlay( CString groupUUID, std::vector<CChannelInfo*> *chInfos, UINT speed );
	//���� ī�޶��� ������ ������� backward play�Ѵ�. �⺻������ key frame ������ �����Ѵ�.
	BOOL	ControlBackwardPlay( CChannelInfo *chInfo, UINT speed );


	//�׷쳻�� �����ִ� ī�޶��� ���念���� playback ������ġ�� ������ �ð��� ��ġ�� �̵��Ѵ�.
	BOOL	ControlJump( CGroupInfo *group, UINT year, UINT month, UINT day, UINT hour, UINT minute, UINT second );
	//ä�θ�ϳ��� �����ִ� ī�޶��� ���念���� playback ������ġ�� ������ �ð��� ��ġ�� �̵��Ѵ�.
	BOOL	ControlJump( CString groupUUID, std::vector<CChannelInfo*> *chInfos, UINT year, UINT month, UINT day, UINT hour, UINT minute, UINT second );
	//���� ī�޶��� ���念���� playback ������ġ�� ������ �ð��� ��ġ�� �̵��Ѵ�.
	BOOL	ControlJump( CChannelInfo *chInfo, UINT year, UINT month, UINT day, UINT hour, UINT minute, UINT second );

	//�׷쳻�� �����ִ� ī�޶��� ���念���� playback ù ������ġ�� �ð����� �̵��Ѵ�.
	BOOL	ControlGoToFirst( CGroupInfo *group );//, UINT *year, UINT *month, UINT*day, UINT *hour, UINT *minute, UINT *second );
	//ä�θ�ϳ��� �����ִ� ī�޶��� ���念���� playback ù ������ġ�� �ð����� �̵��Ѵ�.
	BOOL	ControlGoToFirst( CString groupUUID, std::vector<CChannelInfo*> *chInfos);//, UINT *year, UINT *month, UINT *day, UINT *hour, UINT *minute, UINT *second );
	//���� ī�޶��� ���念���� playback ù ������ġ�� �ð����� �̵��Ѵ�.
	BOOL	ControlGoToFirst( CChannelInfo *chInfo );

	//�׷쳻�� �����ִ� ī�޶��� ���念���� playback ������ ������ġ�� �ð����� �̵��Ѵ�.
	BOOL	ControlGoToLast( CGroupInfo *group );//, UINT *year, UINT *month, UINT*day, UINT *hour, UINT *minute, UINT *second );
	//ä�θ�ϳ��� �����ִ� ī�޶��� ���念���� playback ������ ������ġ�� �ð����� �̵��Ѵ�.
	BOOL	ControlGoToLast( CString groupUUID, std::vector<CChannelInfo*> *chInfos );//, UINT *year, UINT *month, UINT *day, UINT *hour, UINT *minute, UINT *second );
	//���� ī�޶��� ���念���� playback ������ ������ġ�� �ð����� �̵��Ѵ�.
	BOOL	ControlGoToLast( CChannelInfo *chInfo );

	BOOL	ControlForwardStep( CGroupInfo *group );
	BOOL	ControlForwardStep( CString groupUUID, std::vector<CChannelInfo*> *chInfos );
	BOOL	ControlForwardStep( CChannelInfo *chInfo );

	BOOL	ControlBackwardStep( CGroupInfo *group );
	BOOL	ControlBackwardStep( CString groupUUID, std::vector<CChannelInfo*> *chInfos );
	BOOL	ControlBackwardStep( CChannelInfo *chInfo );

	//EXPORT
	//BOOL	StartExport( CGroupInfo *group );
	//BOOL	StartExport( CString groupUUID, std::vector<CChannelInfo*> *chInfos );
	BOOL	StartExport( CChannelInfo *chInfo, CString szFilePath, 
		UINT sYear, UINT sMonth, UINT sDay, UINT sHour, UINT sMinute, 
		UINT eYear, UINT eMonth, UINT eDay, UINT eHour, UINT eMinute,
		LPEXPORTMSGHANDLER pHandler = NULL, LPVOID pParam = NULL);

	//BOOL	StopExport( CGroupInfo *group );
	//BOOL	StopExport( CString groupUUID, std::vector<CChannelInfo*> *chInfos );
	BOOL	StopExport( CChannelInfo *chInfo );

	/*
	BOOL	PauseExport( CGroupInfo *group );
	BOOL	PauseExport( CString groupUUID, std::vector<CChannelInfo*> *chInfos );
	BOOL	PauseExport( CChannelInfo *chInfo );

	BOOL	ResumeExport( CGroupInfo *group );
	BOOL	ResumeExport( CString groupUUID, std::vector<CChannelInfo*> *chInfos );
	BOOL	ResumeExport( CChannelInfo *chInfo );
	*/

	//Set Permission
	void	SetRunAsRecorder( BOOL bRunAsRecorder ){ _bRunAsRecorder=bRunAsRecorder; };
	BOOL	GetRunAsRecorder(){ return _bRunAsRecorder; };


	Recorder * GetRecorder( RS_SERVER_INFO_T *serverInfo, UINT nRetryCount=0 );

private:
	Recorder * GetRecorder( CChannelInfo *chInfo, UINT nRetryCount=0 );
//	Recorder * GetRecorder( RS_SERVER_INFO_T *serverInfo, UINT nRetryCount=0 );

	static VOID GenerateQuasiMac( CString *vcamMacAddress );
	static UINT16 GetIndexOfMaxResolution( VCAM_STREAM_URL *vcamStreamUrl );
	static VOID GetDeviceUrls( CString *szVcamStreamUrl, CString *szIpAddress, CString *szProfileName, UINT16 &nRtspPort );
	static BOOL MakeDeviceInfo( CChannelInfo *chInfo, RS_DEVICE_INFO_T *devInfo );
	static BOOL MakeSchedInfo( CChannelInfo *chInfo, RS_RECORD_SCHEDULE_INFO_T *schedInfo, UINT schedType, BOOL audio,
							   UINT bitSun, UINT bitMon, UINT bitTue, UINT bitWed, UINT bitThu, UINT bitFri, UINT bitSat, 		
							   UINT preRecordingTime, UINT postRecordingTime );

	std::map<CString,Recorder*>							_recorderList;
	std::map<CString,CString>							_cameraList;

	CRITICAL_SECTION									_lockOfRecorder;
	CRITICAL_SECTION									_lockOfCamera;

	CRITICAL_SECTION									_lockOfPbReceiver;
	CRITICAL_SECTION									_lockOfRlReceiver;
	std::map<CString,PBStreamReceiverList>				_pbUUID;
	std::map<CString,RLStreamReceiverList>				_rlUUID;




	BOOL												_bRunAsRecorder;
	NotificationReceiver								*_notifier;

	CRITICAL_SECTION									_lockOfExpReceiver;
	ExportStreamReceiver								*_exportReceiver;
};