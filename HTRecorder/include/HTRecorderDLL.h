#pragma once
#include <vector>
#include <map>
#include <ctime>
#include <sys/timeb.h>
#include <atlcoll.h>

#include "LiveSession5.h"
#include "LiveSessionDLL.h"

#if !defined(AFX_EXT_CLASS)
#define AFX_EXT_CLASS __declspec(dllimport)
#endif

using namespace ATL;

class CBitArray
{
public:
	CBitArray( size_t nBits=32 )
	{
		Resize(nBits);
	}

	virtual ~CBitArray( VOID ) {}

	VOID Assign( BYTE *pArray, size_t nSize )
	{
		Resize( nSize*8 );
		for( size_t i=0; i<nSize; i++ ) _bits[i] = pArray[i];
	}

	VOID ExclusiveOR( BYTE *pArray, size_t nSize )
	{
		size_t nCount = _bits.size();
		BYTE value;
		for( size_t i=0; i<nCount; i++ )
		{
			if( i<nSize )
			{
				value = _bits[i];
				_bits[i] = ( value | pArray[i] );
			}
		}
	}

	VOID Resize( size_t nBits )
	{
		_nCount = nBits;
		size_t nSize = (nBits / 8) + 1;
		_bits.resize( nSize, 0x00 );
	}

	VOID Reset( VOID )
	{
		if( _bits.empty() ) return;
		ZeroMemory( &_bits[0], _bits.size() );
	}

	BOOL Get( size_t nBit ) CONST
	{
		if( nBit>_nCount ) return FALSE;
		BYTE mask = 0x01 << (nBit % 8);
		return ( _bits[nBit/8] & mask );
	}

	VOID Set( size_t nBit, BOOL bVal=TRUE )
	{
		if( nBit>_nCount ) return;
		BYTE value = _bits[nBit/8];
		BYTE mask = 0x01 << (nBit % 8);
		_bits[nBit/8] = bVal ? ( value | mask ) : (value & ~mask);
	}
	size_t	GetCount( VOID ) CONST { return _nCount; }

protected:
	std::vector<BYTE> _bits;
	size_t _nCount;
};

static const size_t BUFFER_ADD_SIZE = 1024;
static const size_t LIMIT_BUFFER_SIZE = 300 * 1024;// 300K

class CBuffer
{
public:
	CBuffer(size_t nSize = 512)
		: m_nUsed(0)
		, m_pBuffer(NULL)
		, m_nStartPos(0)
		, m_nEndPos(0)
	{
		size_t size = nSize + 32;
		m_nSize = size;
		m_nInitSize = size;
		m_nLimitMove = size / 2;

		m_pBuffer = (CHAR*)malloc(size);
		if(m_pBuffer == NULL)
		{
			m_nSize = 0;
		}

		if(nSize > LIMIT_BUFFER_SIZE)
			m_nLimitSize = m_nInitSize * 2;
		else
			m_nLimitSize = LIMIT_BUFFER_SIZE;
	}

	~CBuffer( VOID )
	{
		if(m_pBuffer)
		{
			free(m_pBuffer);
			m_pBuffer = NULL;
		}
	}

	VOID Assign( CONST VOID *pData, size_t nDataSize )
	{
		if( pData )
		{
			if( !CheckBuffer(nDataSize + 32) ) return;
			memcpy( m_pBuffer, pData, nDataSize );
		}
		m_pBuffer[nDataSize]  = '\0';
		m_nUsed = nDataSize;
		m_nStartPos = 0;
		m_nEndPos = nDataSize;
	}

	VOID Append( CONST VOID *pData, size_t nDataSize )
	{
		if(pData)
		{
			if( !CheckBuffer(nDataSize + 32) ) return;
			CopyMemory(m_pBuffer + m_nEndPos, pData, nDataSize);
			m_pBuffer[ m_nEndPos + nDataSize ] = '\0';
		}
		m_nUsed += nDataSize;
		m_nEndPos += nDataSize;
	}
	VOID Erase( size_t nStart=0, size_t nCount=UINT_MAX )
	{
		if(nStart > m_nUsed) return;
		if(nCount == UINT_MAX) nCount = m_nUsed;

		size_t nEnd = m_nStartPos + nStart + nCount;

		// ����� ����ũ�⺸�� ũ�ٸ� ��� ������ ����. ���� �����͸� �̵�
		if(nEnd >= m_nEndPos)
		{
			m_nEndPos = m_nStartPos + nStart;
			m_nUsed = m_nEndPos - m_nStartPos;

			if(m_nStartPos == m_nEndPos)
			{
				m_nStartPos = 0;
				m_nEndPos = 0;
			}

			return;
		}

		// ����� ���ۺ��� �۴ٸ� �տ� �����͸� �̵���.
		if(nStart == 0)
		{
			// ���۰� ���� �տ� ��ġ���� ������.
			m_nStartPos += nCount;
			m_nUsed -= nCount;
			if(m_nStartPos > m_nLimitMove)
			{
				memmove(m_pBuffer, m_pBuffer + m_nStartPos, m_nUsed);
				m_nStartPos = 0;
				m_nEndPos = m_nUsed;
			}
		}
		else
		{
			// ���۰� �߰������� ������. �޸� �̵�
			memmove(m_pBuffer + m_nStartPos + nStart, m_pBuffer + nEnd, m_nUsed - nCount);
			m_nEndPos -= nCount;
			m_nUsed -= nCount;
		}

		if(m_nSize > m_nLimitSize)
		{
			size_t size = m_nUsed;
			size_t padding = size % 32;

			if(m_nUsed < size + padding)
			{
				char *pTemp = m_pBuffer;
				m_pBuffer = (char*)malloc(size + padding);

				memcpy(m_pBuffer, pTemp + m_nStartPos, m_nUsed);
				m_pBuffer[m_nUsed] = '\0';
				m_nStartPos = 0;
				m_nEndPos = m_nUsed;
				m_nSize = size + padding;

				free(pTemp);
			}
		}
	}
		
	size_t GetSize( VOID ) CONST
	{
		return m_nUsed;
	}

	size_t GetCapacity( VOID ) CONST
	{
		return m_nSize;
	}

	CHAR* GetPtr( VOID )
	{
		return m_pBuffer + m_nStartPos;
	}

	size_t SetReserve( size_t nSize )
	{
		if(nSize > m_nSize)
		{
			m_pBuffer = (char*)realloc(m_pBuffer, nSize);
			m_nSize = nSize;
		}

		return m_nSize;
	}

	VOID SetSize( size_t nSize )
	{
		if(nSize > m_nSize)
		{
			size_t size = nSize + BUFFER_ADD_SIZE;
			size_t padding = size % 32;

			m_pBuffer = (char*)realloc(m_pBuffer, size + padding);
			m_nSize = size + padding;
		}
		m_nUsed = nSize;
		m_nEndPos = m_nUsed;

		m_pBuffer[ m_nUsed ] = '\0';
	}

	VOID Init( size_t nSize )
	{
		if(m_pBuffer) free(m_pBuffer);
		size_t size = nSize + BUFFER_ADD_SIZE;
		size_t padding = size % 32;

		m_pBuffer = (char*)malloc(size);
		m_nUsed = 0;
		m_nEndPos = 0;
		m_nSize = size;
		m_pBuffer[0] = '\0';
	}

	BOOL CheckBuffer( size_t nSize )
	{
		size_t nTotal = m_nEndPos + nSize;
		if(nTotal > m_nSize)
		{
			size_t size = m_nSize + nTotal + BUFFER_ADD_SIZE;
			size_t padding = size % 32;

			m_pBuffer = (char*)realloc(m_pBuffer, size + padding);
			if(m_pBuffer == NULL)
			{
				m_nSize = 0;
				m_nUsed = 0;
				m_nEndPos = 0;
				m_nStartPos = 0;
				return FALSE;
			}

			m_nSize = size + padding;
			m_nLimitMove = size / 2;
		}
		return TRUE;
	}
private:
	char *m_pBuffer;
	size_t m_nSize;
	size_t m_nUsed;
	size_t m_nStartPos;
	size_t m_nEndPos;

	size_t m_nInitSize;
	size_t m_nLimitSize;
	size_t m_nLimitMove;
};

#define SIZEOF_ARRAY(x) sizeof(x)/sizeof(x[0])


#define RS_DEFAULT_SERVICE_ADDRESS	L"127.0.0.1"
#define RS_DEFAULT_USER				L"admin"
#define RS_DEFAULT_PASSWORD			L"admin"

#define RS_DEFAULT_SERVICE_PORT		11550	// Nautilus ���ڵ� ���� �⺻ Port

#define RS_FILE_VERSION				10300
#define RS_DB_VERSION				5

#define RS_MAX_RECORDER				(32)
#define	RS_MAX_DEVICE				(64)
#define	RS_MAX_DISK					(32)
#define	RS_MAX_SCHEDULE_INFO		(64)

#define	RS_MAX_PLAYBACK_CH			(64)
#define	RS_MAX_MONTH				(12)
#define	RS_MAX_DAY					(31)
#define	RS_MAX_HOUR					(24)
#define	RS_MAX_MINUTE				(60)

#define KBYTE						1024.0
#define MEGABYTE					1048576.0
#define GIGABYTE					1073741824.0

typedef	enum _RS_RL_FRAME_TYPE
{
	RS_RL_FRAME_NONE	= 0x00000000, 
	RS_RL_FRAME_VIDEO	= 0x00000001, 
	RS_RL_FRAME_AUDIO	= 0x00000002, 
	RS_RL_FRAME_META	= 0x00000004, 
	RS_RL_FRAME_ALL		= 0x00000007
} RS_RL_FRAME_TYPE;

typedef	enum	_RS_PB_FRAME_TYPE
{
	RS_PB_FRAME_NONE	= 0x00000000, 
	RS_PB_FRAME_VIDEO	= 0x00000001, 
	RS_PB_FRAME_AUDIO	= 0x00000002, 
	RS_PB_FRAME_META	= 0x00000004, 
	RS_PB_FRAME_ALL		= 0x00000007
} RS_PB_FRAME_TYPE;

typedef	enum	_RS_EP_FRAME_TYPE
{
	RS_EP_FRAME_NONE		= 0x00000000, 
	RS_EP_FRAME_VIDEO		= 0x00000001, 
	RS_EP_FRAME_AUDIO		= 0x00000002, 
	RS_EP_FRAME_VIDEO_AUDIO	= 0x00000003, 
	RS_EP_FRAME_META		= 0x00000004, 
	RS_EP_FRAME_ALL			= 0x00000007
} RS_EP_FRAME_TYPE;

typedef	enum	_RS_ERROR_TYPE
{
	RS_ERROR_ERROR						= 0,	// �Ʒ� Error�� �ش���� ���� ���
	RS_ERROR_NO_ERROR					= 1,	// No Error
	RS_ERROR_FAIL						= 2,	// �Ϲ� Error
	RS_ERROR_UnKnownRequest				= 3,	// XML command�� �ƴϰų� �Ű����� element�� ������ �ִ� ���
	RS_ERROR_NoDeviceStreamInformation	= 4,	// Device ������ �������� ���� ���
	RS_ERROR_UnknownModelInformation	= 5,	// Device�� �� ������ ���� ���
	RS_ERROR_DeviceDisconnected			= 6,	// Device ������ ������ ���
	RS_ERROR_ClientAcceptFull			= 7,	// ������ ���� ������ Control session�� ��� ���� ���
	RS_ERROR_RecordingAcceptFull		= 8,	// ������ ������ �ִ밪���� ���� Recording �õ��� ���
	RS_ERROR_StreamingAcceptFull		= 9,	// ������ ������ �ִ밪���� ���� Relay session�� ��û�� ���
	RS_ERROR_RecordingStorageFull		= 10,	// Recording �� Storage space�� ������ ���
	RS_ERROR_ReservedStorageFull		= 11,	// reserved space��ŭ recording �Ͽ����� overwriteing ������ off �� ���
	RS_ERROR_PoorStreamingStatus		= 12,	// ��Ʈ��ũ ���� ���� �������� frame ������ ���� ���
	RS_ERROR_AlreadyRecording			= 13,	// Recording ���� �� recording ������ �����ϴ� command�� ������ ���
	RS_ERROR_NoRecordingData			= 14,	// ��û�� �ð��� Recording data�� ���� ���
	RS_ERROR_Timeout					= 15,	// Timeout �� ���
	RS_ERROR_FileOpenFail				= 16,	// File open ������ ���
	RS_ERROR_UnknownRawFrame			= 17,	// Device���� ���ŵ� frame ������ ��ü �������� ��ȯ�� ������ ���
	RS_ERROR_UnknownAccount				= 18,	// ������ ��ϵ� ���� ������ ��ġ���� �ʴ� ���
	RS_ERROR_NeedSetupLogin				= 19,	// Setup �������� �α��� ���� �ʾҴµ� setup command�� ������ ���
	RS_ERROR_OtherSetupLogin			= 20,	// ���� �α��� ���� ���� ������ �ٽ� �α����Ͽ� 
												//  �α��� ���� session�� ����Ǵ� ���
	RS_ERROR_AlreadyRelaying			= 21,	// �̹� Relay ���� ���
												// (�ϳ��� Client���� ���� Device�� ���� �ߺ����� Relay ��û�� ���)
	RS_ERROR_InvalidXML					= 22,	// ��ȿ�� XML data�� �ƴ� ���
	RS_ERROR_DeviceStreamInfoFull		= 23,	// ������ ������ �ִ밪���� ���� Device�� ��� �õ��� ���
	RS_ERROR_AlreadyUsing				= 24,	// Device ������ �������� �ʰ� 
												// �ش� Device ������ �����ϴ� command�� ������ ���
	RS_ERROR_PlaybackAcceptFull			= 25,	// ������ ������ �ִ밪���� ���� Playback session�� ��û�� ���
	RS_ERROR_ExportAcceptFull			= 26,	// ������ ������ �ִ밪���� ���� Export session�� ��û�� ���
	RS_ERROR_UnsupportedVersion			= 27,	// Recording ���� �����̳� �ε��� ������ ���� �ʴ� ���
	RS_ERROR_FileHeaderParsingFail		= 28,	// Playback �� FileHeader parsing ������ ���
	RS_ERROR_BlockHeaderParsingFail		= 29,	// Playback �� BlockHeader parsing ������ ���
	RS_ERROR_StreamHeaderParsingFail	= 30,	// Playback �� StreamHeader parsing ������ ���
	RS_ERROR_FrameParsingFail			= 31,	// Playback �� Frame parsing ������ ���
	RS_ERROR_BrokenBlockWatermark		= 32,	// Recording data block�� watermark�� ������ ���
	RS_ERROR_BrokenFrameWatermark		= 33,	// Recording frame�� watermark�� ������ ���
	RS_ERROR_IOFail						= 34,	// �������� ��ũ I/O�� ������ ���
	RS_ERROR_EndOfRecording				= 35,	// ������ Recording data���� playback�� ���
	RS_ERROR_OverwritingError			= 36,	// ��ũ�� �и��Ǿ��ų� ���ϸ� ������ �����Ǿ 
												//  overwriting �Ұ����� ���
	RS_ERROR_HeterogeneousModelType		= 37,	// ���� Device�� ONVIF�� Hitron �ΰ��� Ÿ������ ��� ����� ���
	RS_ERROR_MAX
} RS_ERROR_TYPE;

// Device type
typedef	enum	_RS_DEVICE_TYPE
{
	RS_DEVICE_UNKNOWN		= 0,
	RS_DEVICE_NAUTILUS		= 1,
	RS_DEVICE_HITRON_CAMERA	= 2,
	RS_DEVICE_ONVIF_CAMERA	= 3,
	RS_DEVICE_HITRON_NVR	= 4,
	RS_DEVICE_HITRON_NVR_CAM= 5,
} RS_DEVICE_TYPE;

// Device Connection type
typedef	enum	_RS_DEVICE_CONNECT
{
	RS_DEVICE_CONNECT_T_RTPoverUDP			= 1,
	RS_DEVICE_CONNECT_T_RTPoverRTSP			= 2,
	RS_DEVICE_CONNECT_T_RTPoverRTSPoverHTTP	= 3,
	RS_DEVICE_CONNECT_T_RTPMulticast		= 4,
} RS_DEVICE_CONNECT;


// Recording schedule type
typedef	enum	_RS_RECORD_SCHEDULE
{
	RS_RECORD_SCHEDULE_T_NONE		= 0,	// No Recording
	RS_RECORD_SCHEDULE_T_ALWAYS		= 1,	// �׻� Recording
	RS_RECORD_SCHEDULE_T_EVENT		= 2,	// Event �߻��� Recording
	RS_RECORD_SCHEDULE_T_ADAPTIVE	= 3,	// ADAPTIVE Recording
											// (Event �߻��ÿ��� pre/post �ð��� �����Ͽ� ��� frame ����, 
											//  �׿ܿ��� key frame�� ����)
	RS_RECORD_SCHEDULE_T_MAX
} RS_RECORD_SCHEDULE;

// Recording type
typedef	enum	_RS_RECORDING_MODE
{
	RS_RECORDING_MODE_T_NORMAL		= 0, 
	RS_RECORDING_MODE_T_SPECIAL		= 1,
} RS_RECORDING_MODE;

// Playback Direction type
typedef	enum	_RS_PLAYBACK_DIRECTION
{
	RS_PLAYBACK_DIRECTION_T_NONE		= 0,
	RS_PLAYBACK_DIRECTION_T_FORWARD,
	RS_PLAYBACK_DIRECTION_T_BACKWARD
} RS_PLAYBACK_DIRECTION;

// Playback Speed type
typedef	enum	_RS_PLAYBACK_SPEED
{
	RS_PLAYBACK_SPEED_T_1X		= 1,
	RS_PLAYBACK_SPEED_T_2X		= 2,
	RS_PLAYBACK_SPEED_T_3X		= 3, 
	RS_PLAYBACK_SPEED_T_4X		= 4, 
	RS_PLAYBACK_SPEED_T_5X		= 5, 
	RS_PLAYBACK_SPEED_T_MAX		= 6
} RS_PLAYBACK_SPEED;

/////////////////////// SCHEDULE /////////////////////////
#define RS_RECORD_NONE					0x00					// ���ڵ� ���� ����
#define RS_RECORD_TIME					0x01					// ���� ���ڵ�
#define RS_RECORD_EVENT					0x02					// �̺�Ʈ ���ڵ�
#define RS_RECORD_ADAPTIVE_TIME_EVENT	0x03					// Adaptive Time & Event ( ���� key frame����, event �߻��� full-frame ���� )

typedef struct _RS_SCHEDULE_INFO_T
{
	_RS_SCHEDULE_INFO_T( VOID )
		: evt(RS_RECORD_NONE)
		, recordAudio(0)
		, useAdvance(0)
		, reserve0(0)
		, name(_T(""))
	{
		evt = RS_RECORD_NONE;
	}
	VOID Clear( VOID )
	{
		id = _T("");
		evt = 0;
		recordAudio = 0;
		useAdvance = 0;
		name = _T("");
	}

	CStringW		id;
	UCHAR			evt;			// 0: NONE, 1: TIME(always), 2: EVENT
	UCHAR			recordAudio;	// ����� ����? true or false
	UCHAR			useAdvance;		// Ȯ�� ������ ���?
	UCHAR			reserve0;
	CStringW		name;
} RS_SCHEDULE_INFO_T, *LPRS_SCHEDULE_INFO_T;

/////////////////////// RECORD /////////////////////////
typedef struct _RS_RECORD_INFO_T
{
	_RS_RECORD_INFO_T( VOID )
		: nPreEventTime(0)
		, nPostEventTime(10)
		, bEnableOverwrite(FALSE)
		, bRecordAudio(FALSE) {}

	CStringW				strRecordID;		// Record ID (ī�޶� MAC��巹���� ��������)
	INT						nPreEventTime;		// Pre Event Record �ð�
	INT						nPostEventTime;		// Post Event Record �ð�
	BOOL					bEnableOverwrite;	// Overwrite �������
	BOOL					bRecordAudio;		// ����� ���� �������
	RS_SCHEDULE_INFO_T		schedule[7][24];	// 7�� 24�ð�
} RS_RECORD_INFO_T, *LPRS_RECORD_INFO_T;

typedef struct _RS_RECORD_SPECIAL_DAY_T
{
	_RS_RECORD_SPECIAL_DAY_T( VOID )
		: nMonth(-1)
		, nDay(-1) {}
	CStringW			strRecordID;	// ID
	INT					nMonth;			// ��
	INT					nDay;			// ��
	RS_SCHEDULE_INFO_T	schedule[24];	// 24�ð�
} RS_RECORD_SPECIAL_DAY_T, *LPRS_RECORD_SPECIAL_DAY_T;

typedef struct _RS_RECORD_SPECIAL_INF0_T
{
	_RS_RECORD_SPECIAL_INF0_T( VOID )
		: bEnable(FALSE)
		, bRecordAudio(FALSE)
		, nPreEvent(0)
		, nPostEvent(0) {}
	BOOL											bEnable;		// Special ���ڵ� ���
	BOOL											bRecordAudio;	// ����� ����
	INT												nPreEvent;		// Pre Event Record	
	INT												nPostEvent;		// Post Event Record
	std::map<CStringW, RS_RECORD_SPECIAL_DAY_T>		specialDays;	// Special Day ���� ����Ʈ
} RS_RECORD_SPECIAL_INF0_T, *LPRS_RECORD_SPECIAL_INF0_T;

/////////////////////// RECORD /////////////////////////
#define RS_ONEVENT_ON_MOTION		0x00000001
#define RS_ONEVENT_ON_ALARM1		0x00000002
#define RS_ONEVENT_ON_ALARM2		0x00000004
#define RS_ONEVENT_ON_BOOT			0x00000008
#define RS_ONEVENT_ON_VIDEOLOSS		0x00000010

typedef enum _RS_EVENT_ID 
{
	RS_EVENT_ID_ON_MOTION	= 0,
	RS_EVENT_ID_ON_ALARM1,
	RS_EVENT_ID_ON_ALARM2,
	RS_EVENT_ID_BOOT,
	RS_EVENT_ID_VIDEOLOSS,
	RS_EVENT_ID_MAX,
} RS_EVENT_ID;

typedef enum _RS_SYSTEMEVENT_ID 
{
	// Recording service
	RS_SYSTEM_ID_DISK_FULL = 0,
	RS_SYSTEM_ID_DISK_FAULT,
	// Monitoring service
	RS_SYSTEM_ID_UPDATE,
	RS_SYSTEM_ID_MAX,
} RS_SYSTEMEVENT_ID;

typedef struct _RS_EVENT_FIELD_T
{
	RS_EVENT_ID	Id;
	TCHAR		name[16];
	UINT		Type;
} RS_EVENT_FIELD_T;

typedef struct _RS_EVENT_INFO_T
{
	_RS_EVENT_INFO_T( VOID )
		: bEnable(FALSE)
		, bOldEnable(FALSE)
		, nEventType(0)
		, nRecordDwellTime(-1)
		, nPtzPreset(-1)
		, nManualTrigger(-1) {}

	typedef struct _RS_VIEW_CHANNEL_T 
	{
		_RS_VIEW_CHANNEL_T( VOID )
			: nDwellTime(0)
			, nLiveWindow(0) {}
		INT		nLiveWindow;
		INT		nDwellTime;	// 0: Disabled, 1: Infinite
	} RS_VIEW_CHANNEL_T ;

	typedef struct _RS_VIEW_LAYOUT_T 
	{
		_RS_VIEW_LAYOUT_T( VOID )
			: nDwellTime(0) {}
		INT			nDwellTime;	// 0: Disabled, 1: Infinite
		CStringW	strLayoutID;
	} RS_VIEW_LAYOUT_T;

	typedef struct _RS_SOUND_T 
	{
		_RS_SOUND_T( VOID )
			: bEnable(FALSE) {}
		BOOL		bEnable;
		CStringW	strFileName;
	} RS_SOUND_T;


	UINT				nEventType;
	RS_VIEW_CHANNEL_T	viewChannel;
	RS_VIEW_LAYOUT_T	viewLayout;
	RS_SOUND_T			sound;
	CStringW			RelayoutID;
	//std::set<std::wstring> RelaySet;

	CStringW			strExeAppName;
	VOID				SetEnable( BOOL bEnable ) { this->bEnable = bEnable; }
	BOOL				IsEnabled( VOID ) CONST { return bEnable; }
	VOID				SetOldEnalbe( BOOL bEnable ) { bOldEnable = bEnable; }
	BOOL				IsChanged( VOID ) { return (bEnable!=bOldEnable); }			// ����� bEnable���� bOldEnable���� ����.
	BOOL				IsEnabledRecord( VOID ) { return (nRecordDwellTime!=-1); }
	BOOL				IsEnabledPreset( VOID ) { return (nPtzPreset!=-1); }
	BOOL				IsEnabledTrigger( VOID ) { return (nManualTrigger!= 1); }
	VOID				SetRecordTime( INT nDwellTime ) { nRecordDwellTime = nDwellTime; }
	INT					GetRecordTime( VOID ) CONST { return nRecordDwellTime; }
	VOID				SetPreset( INT nPreset ) { nPtzPreset = nPreset; }
	INT					GetPreset( VOID ) CONST { return nPtzPreset; }
	VOID				SetTrigger( INT nTrigger ) { nManualTrigger = nTrigger; }
	INT					GetTrigger() CONST { return nManualTrigger; }

protected:
	BOOL			bEnable;
	BOOL			bOldEnable;					// ������ ����Ǿ����� Ȯ���ϱ� ����

	INT				nRecordDwellTime;	// -1 : disabled
	INT				nPtzPreset;			// -1 : disabled
	INT				nManualTrigger;		// -1 : disabled
} RS_EVENT_INFO_T;

typedef struct _RS_DEVICE_EVENT_INFO_T
{
	RS_EVENT_INFO_T	events[RS_EVENT_ID_MAX];
} RS_DEVICE_EVENT_INFO_T;

///////////////////////  DEVICE  /////////////////////////
// ONVIF Info
typedef	struct	_RS_ONVIF_INFO_T
{
	CStringW			strServiceUri;
	CStringW			strPtzUri;
	CStringW			strPtzToken;
} RS_ONVIF_INFO_T;
	
// DEVICE info
typedef struct _RS_DEVICE_INFO_T
{
	RS_DEVICE_TYPE		GetDeviceType( VOID ) CONST { return deviceType; }
	UINT				GetKeyID( VOID ) CONST { return nKeyID; }
	// MAC have not delimiter character. (ex: ':', '-')
	CStringW			GetMAC( VOID ) { return strMAC; }
	CStringW			GetProfileName( VOID ) CONST { return strProfileName; }
	CStringW			GetModelType( VOID ) CONST { return strModelType; }
	CStringW			GetModel( VOID ) CONST { return strModel; }
	CStringW			GetID( VOID ) CONST { return strID; }
	CStringW			GetName( VOID ) CONST { return strName; }
	CStringW			GetAddress( VOID ) CONST { return strAddress; }
	CStringW			GetURL( VOID ) CONST { return strURL; }
	UINT				GetRTSPPort( VOID ) CONST { return nRTSPPort; }
	UINT				GetHTTPPort( VOID ) CONST { return nHTTPPort; }
	UINT				GetHTTPSPort( VOID ) CONST { return nHTTPSPort; }
	BOOL				GetSSL( VOID ) CONST { return bSSL; }
	UINT				GetConnectionType( VOID ) CONST { return nConnectType; }
	CStringW			GetUser( VOID ) CONST { return strUser; }
	CStringW			GetPassword( VOID ) CONST { return strPass; }
	CStringW			GetDescription( VOID ) CONST { return strDesc; }
	CStringW			GetFirmwareVersion( VOID ) CONST { return strFirmwareVer; }
	// Gets channel information for ip device. (Zero-Base)
	UINT				GetChannel( VOID ) CONST { return nChannel; };

	CStringW			GetOnvifServiceUri( VOID ) CONST { return Onvif.strServiceUri; }
	CStringW			GetOnvifPtzUri( VOID ) CONST { return Onvif.strPtzUri; }
	CStringW			GetOnvifPtzToken( VOID ) CONST { return Onvif.strPtzToken; }

	// DSKIM
	UINT				GetChannelCount( VOID ) CONST { return nChannelCount; };

	VOID				SetNeedUpdate( VOID ) { if(bCanBeUpdated) bIsUpdated = TRUE; }
	VOID				SetKeyID( CONST UINT key ) { nKeyID = key; }
	VOID				SetDeviceType( RS_DEVICE_TYPE type ) { deviceType = type; }
	VOID				SetMAC( CONST CStringW mac )
	{
		if( strMAC==mac ) return;
		strMAC = mac;
		SetNeedUpdate();
	}
	VOID			SetProfileName( CONST CStringW profileName )
	{
		if( strProfileName==profileName ) return;
		strProfileName = profileName;
		SetNeedUpdate();
	}
	VOID			SetModelType( CONST CStringW modelType ) { strModelType = modelType; }
	VOID			SetModel( CONST CStringW model ) { strModel = model; }
	VOID			SetID( CONST CStringW id) { strID = id; }
	VOID			SetName( CONST CStringW name ) { strName = name; }
	VOID			SetAddress( CONST CStringW address )
	{
		if( strAddress==address ) return;
		strAddress = address;
		SetNeedUpdate();
	}
	VOID			SetURL( CONST CStringW url ) 
	{
		if( strURL==url ) return;
		strURL = url;
		SetNeedUpdate();
	}
	VOID			SetRTSPPort( CONST UINT port ) 
	{
		if( nRTSPPort==port ) return;
		nRTSPPort = port;
		SetNeedUpdate();
	}
	VOID			SetHTTPPort( CONST UINT port ) 
	{
		if( nHTTPPort==port ) return;
		nHTTPPort = port;
		SetNeedUpdate();
	}
	VOID			SetHTTPSPort( CONST UINT port ) 
	{
		if(nHTTPSPort == port) return;
		nHTTPSPort = port;
		SetNeedUpdate();
	}
	VOID			SetSSL( CONST BOOL SSL) 
	{
		if( bSSL==SSL ) return;
		bSSL = SSL;
		SetNeedUpdate();
	}
	VOID			SetConnectionType( CONST UINT connectionType ) 
	{
		if( nConnectType==connectionType ) return;
		nConnectType = connectionType;
		SetNeedUpdate();
	}
	VOID			SetUser( CONST CStringW user ) 
	{
		if( strUser==user ) return;
		strUser = user;
		SetNeedUpdate();
	}
	VOID			SetPassword( CONST CStringW password ) 
	{
		if( strPass==password ) return;
		strPass = password;
		SetNeedUpdate();
	}
	VOID			SetDescription( CONST CStringW description ) { strDesc = description; }
	VOID			SetFirmwareVersion( CONST CStringW firmwareVer) { strFirmwareVer = firmwareVer; }
	VOID			SetGroup( struct _GROUP_INFO* _pGroup ) { pGroup = _pGroup; }
	VOID			SetChannel( INT nCh ) { nChannel = nCh; }

	VOID			SetOnvifServiceUri( CONST CStringW serviceUri )
	{
		if( Onvif.strServiceUri ==serviceUri ) return;
		Onvif.strServiceUri = serviceUri;
		SetNeedUpdate();
	}
	VOID			SetOnvifPtzUri( CONST CStringW ptzUri )
	{
		if( Onvif.strPtzUri==ptzUri ) return;
		Onvif.strPtzUri = ptzUri;
		SetNeedUpdate();
	}
	VOID			SetOnvifPtzToken( CONST CStringW token )
	{
		if( Onvif.strPtzToken==token ) return;
		Onvif.strPtzToken = token;
		SetNeedUpdate();
	}
	VOID			SetChannelCount( CONST UINT channelCount ) 
	{
		nChannelCount	= channelCount;
	}

	struct _GROUP_INFO* GetGroup( VOID ) { return pGroup; }

	VOID			SetRecorderServerId( UINT Id ) { recorderServerID = Id; }
	UINT			GetRecorderServerId( VOID ) CONST { return recorderServerID; }
	BOOL			IsNeedUpdate( VOID ) CONST { return bIsUpdated; }

	VOID			SetMessageText( LPCWSTR lpszMessage ) { strMessageText = lpszMessage; }
	CStringW		GetMessageText( VOID ) CONST { return strMessageText; }

	VOID			SetRecYear( INT nYear ) {	nRecYear = nYear; }
	VOID			SetRecMonth( BYTE *pIndex, size_t nSize ) { bitRecMonth.Assign(pIndex, nSize); }
	VOID			SetRecDay( BYTE* pIndex, size_t nSize ) { bitRecDay.Assign(pIndex, nSize); }
	VOID			SetRecHour(BYTE* pIndex, size_t nSize ) { bitRecHour.Assign(pIndex, nSize); }
	VOID			SetRecMin(BYTE* pIndex, size_t nSize ) { bitRecMinute.Assign(pIndex, nSize); }
	VOID			SetRecDupMin(BYTE* pIndex, size_t nSize ) { bitRecDupMin.Assign(pIndex, nSize); }

	INT				GetRecYear( VOID ) { return nRecYear; }
	CBitArray*		GetRecMonth( VOID ) { return  &bitRecMonth; }
	CBitArray*		GetRecDay( VOID ) { return  &bitRecDay; }
	CBitArray*		GetRecHour( VOID ) { return  &bitRecHour; }
	CBitArray*		GetRecMin( VOID ) { return  &bitRecMinute; }
	CBitArray*		GetRecDupMin( VOID ) { return  &bitRecDupMin; }

	/*
	VOID			SetDeviceStatus(XTREE_CAMERA_STATUS Status) { DeviceStatus = Status; }
	XTREE_CAMERA_STATUS	GetDeviceStatus() CONST { return DeviceStatus; }
	*/
	VOID			SetStreamStatus( DWORD dwStatus ) { dwStreamStatus = dwStatus; }
	DWORD			GetStreamStauts( VOID ) CONST { return dwStreamStatus; }

	VOID			SetRecordStatus(BOOL bOn) { bRecordOn = bOn; }
	BOOL			GetRecordStatus( VOID ) CONST { return bRecordOn; }

	RS_RECORD_INFO_T	GetRecordInfo( VOID ) CONST { return recordInfo; }
	CStringW			GetRecordID( VOID ) CONST { return recordInfo.strRecordID; }
	INT					GetPreRecordTime( VOID ) CONST { return recordInfo.nPreEventTime; }
	INT					GetPostRecordTime( VOID ) CONST { return recordInfo.nPostEventTime;}
	BOOL				IsRecordOverwrite( VOID ) CONST { return recordInfo.bEnableOverwrite; }
	BOOL				IsRecordAudio( VOID ) CONST { return recordInfo.bRecordAudio; }

	VOID			SetRecordInfo( RS_RECORD_INFO_T info) 
	{ 
		recordInfo = info; 
		SetNeedUpdate(); 
	}
	VOID			SetReocrdID( LPCWSTR lpRecordID ) 
	{ 
		recordInfo.strRecordID = lpRecordID; 
		SetNeedUpdate(); 
	}
	VOID			SetPreRecordTime( INT nTime ) 
	{ 
		recordInfo.nPreEventTime = nTime; 
		SetNeedUpdate(); 
	}
	VOID			SetPostRecordTime( INT nTime )
	{
		if( nTime<10 ) nTime = 10;
		recordInfo.nPostEventTime = nTime;
		SetNeedUpdate();
	}
	VOID			SetEnableRecordOverwrite( BOOL bEnable )
	{
		recordInfo.bEnableOverwrite = bEnable;
		SetNeedUpdate();
	}
	VOID			SetEnableRecordAudio( BOOL bEnable )
	{
		recordInfo.bRecordAudio = bEnable;
		SetNeedUpdate();
	}
	VOID			SetScheduleID( INT day, INT time, LPCWSTR id )
	{
		if(id)
		{
			recordInfo.schedule[day][time].id = id;
			SetNeedUpdate();
		}
	}
	VOID			SetScheduleEvent( INT day, INT time, BYTE evt )
	{
		recordInfo.schedule[day][time].evt = evt;
		SetNeedUpdate();
	}
	VOID			SetScheduleRecordAudio( INT day, INT time, BOOL bRecord)
	{
		recordInfo.schedule[day][time].recordAudio = bRecord;
		SetNeedUpdate();
	}
	VOID			SetScheduleName( INT day, INT time, LPCWSTR name )
	{
		recordInfo.schedule[day][time].name = name;
		SetNeedUpdate();
	}
	VOID			SetScheduleReset( INT day, INT time )
	{
		recordInfo.schedule[day][time].Clear();
		SetNeedUpdate();
	}

	LPCWSTR			GetScheduleID( INT day, INT time )
	{
		return recordInfo.schedule[day][time].id;
	}
	BOOL			IsScheduleRecordAudio( INT day, INT time )
	{
		return recordInfo.schedule[day][time].recordAudio;
	}
	BYTE			GetScheduleEvent( INT day, INT time )
	{
		return recordInfo.schedule[day][time].evt;
	}
	LPCWSTR			GetScheduleName( INT day, INT time )
	{
		return recordInfo.schedule[day][time].name;
	}

	VOID			SetSpecialRecordInfo( RS_RECORD_SPECIAL_INF0_T& info )
	{
		specialRecordInfo = info;
		SetNeedUpdate();
	}
	VOID			SetEnableSpecialRecord( BOOL bEnable )
	{
		specialRecordInfo.bEnable = bEnable;
		SetNeedUpdate();
	}
	VOID			SetSpecialPreRecordTime( INT nTime )
	{
		specialRecordInfo.nPreEvent = nTime;
		SetNeedUpdate();
	}
	VOID			SetSpecialPostRecordTime( INT nTime )
	{
		specialRecordInfo.nPostEvent = nTime;
		SetNeedUpdate();
	}
	VOID			GetSpecialRecordDayMap( std::map<CStringW, RS_RECORD_SPECIAL_DAY_T>& spDay )
	{
		spDay = specialRecordInfo.specialDays;
	}
	VOID			AddSpecialRecordDay(CONST CStringW name, RS_RECORD_SPECIAL_DAY_T spDay)
	{
		specialRecordInfo.specialDays[name] = spDay;
		SetNeedUpdate();
	}
	VOID			RemoveSpecialRecordDay( CONST CStringW name)
	{
		specialRecordInfo.specialDays.erase(name);
		SetNeedUpdate();
	}
	RS_RECORD_SPECIAL_INF0_T	GetSpecialRecordInfo( VOID ) CONST { return specialRecordInfo; }
	BOOL						IsSpecialRecord( VOID ) CONST { return specialRecordInfo.bEnable; }
	INT							GetSpecialPreRecordTime( VOID ) CONST { return specialRecordInfo.nPreEvent; }
	INT							GetSpecialPostRecordTime( VOID ) CONST { return specialRecordInfo.nPostEvent; }
	BOOL						GetSpecialRecordDay( CONST CStringW name, RS_RECORD_SPECIAL_DAY_T spDay )
	{
		std::map<CStringW, RS_RECORD_SPECIAL_DAY_T>::iterator pos;
		pos = specialRecordInfo.specialDays.find( name );
		if( pos==specialRecordInfo.specialDays.end() ) return FALSE;
		spDay = pos->second;
		return TRUE;
	}
	RS_RECORD_SPECIAL_DAY_T*	GetSpecialRecordDay( CONST CStringW name)
	{
		std::map<CStringW, RS_RECORD_SPECIAL_DAY_T>::iterator pos;
		pos = specialRecordInfo.specialDays.find( name );
		if( pos==specialRecordInfo.specialDays.end() ) return NULL;
		return &pos->second;
	}

	VOID			SetSupportPtz( BOOL bSupport ) { bSupportPtz = bSupport; }
	BOOL			IsSupportPtz( VOID ) CONST { return bSupportPtz; }
	BOOL			IsSupportMPtz( VOID ) {	return optionMPTZ; }
	BOOL			IsSupportRS485( VOID ) { return (!optionBuiltInPTZ && optionMPTZ && optionPTZ); }
	VOID			SetOptBuiltinPTZ( BOOL bValue ) { optionBuiltInPTZ = bValue; }
	BOOL			GetOptBuiltinPTZ( VOID ) CONST { return optionBuiltInPTZ; }
	VOID			SetOptPTZ( BOOL bValue ) { optionPTZ = bValue; }
	BOOL			GetOptPTZ( VOID ) CONST { return optionPTZ; }
	VOID			SetOptMPTZ( BOOL bValue ) { optionMPTZ = bValue; }
	BOOL			GetOptMPTZ( VOID ) CONST { return optionMPTZ; }
	VOID			SetOptSmartFocus( BOOL bValue ) { optionSmartFocus = bValue; }
	BOOL			GetOptSmartFocus( VOID ) CONST { return optionSmartFocus; }
	VOID			Reset( VOID )
	{
		bSupportPtz			= FALSE;
		optionBuiltInPTZ	= FALSE;
		optionPTZ			= FALSE;
		optionMPTZ			= FALSE;
	}

	VOID			SetErrorConnect( BOOL bError ) { bErrorConnect = bError; }
	BOOL			IsErrorConnect( VOID ) CONST { return bErrorConnect; }
	VOID			SetNoDevice( BOOL bNoDevice ) { bIsNoDevice = bNoDevice; }
	BOOL			IsNoDevice( VOID ) CONST { return bIsNoDevice; }

	VOID			UpdateStream( VOID ) { tLastUpdateStream = time(NULL); }
	time_t			GetLastUpdateStream( VOID ) CONST { return tLastUpdateStream; }

	VOID			SetCustomerModel( CONST CStringW CustomerModel ) { strCustomerModel = CustomerModel; }
	CStringW		GetCUstomerModel( VOID ) CONST { return strCustomerModel; }

	VOID			SetVideoFilter( UINT nFIlter ) { nVideoFilter = nFIlter; }
	UINT			GetVideoFilter( VOID ) CONST { return nVideoFilter; }


	BOOL			IsDependOnRSServer( VOID )
	{
		return ( (deviceType==RS_DEVICE_HITRON_CAMERA) || (deviceType==RS_DEVICE_ONVIF_CAMERA) );
	};

protected:

	RS_DEVICE_TYPE		deviceType;
	INT					nKeyID;			// ID, 32ä���� ��� 0~31, 64ä�� �ΰ�� 0~63
	CStringW			strMAC;			// MAC Address
	CStringW			strProfileName;	// ProfileName (ONVIF Profile, Hitron���� /1/stream1 �� �������� ���� )
	CStringW			strModelType;	// EModel, HModel, SModel, ONVIF, Unknown
	CStringW			strModel;		// ���̸�
	CStringW			strCustomerModel;	// ���̾� ���̸�
	CStringW			strID;			// guid ���
	CStringW			strName;		// �̸�
	CStringW			strAddress;		// IP Addres
	CStringW			strURL;			// RTSP ��ü URL
	UINT				nRTSPPort;		// RTSP Port
	UINT				nHTTPPort;		// HTTP Port
	UINT				nHTTPSPort;		// HTTPS Port
	BOOL				bSSL;			// SSL ���
	UINT				nConnectType;	//1. RTP over UDP	//2. RTP over RTSP (TCP)	//3. RTP over RTSP over HTTP (TCP)	//4. RTP Multicast
	CStringW			strUser;		// Login ID
	CStringW			strPass;		// Login Password
	CStringW			strDesc;		// ŰŸ����
	CStringW			strFirmwareVer;	// ���� ���Ŀ� ������ ���� ����
	RS_ONVIF_INFO_T		Onvif;			// ONVIF ��ġ ����
	UINT				nChannel;		// DVR/NVR ���� ��ġ�� ä�ο� ���� ����

	_GROUP_INFO		*pGroup;

	UINT				recorderServerID;		// Recording ������ �ĺ��� (���� ����1���� 0���� Default ���̴�.)
	BOOL				bIsUpdated;

	// Joseph
	// 2011.02.25
	CStringW			strMessageText;		// Display�� Message 

	// RECORD ��������
	INT					nRecYear;
	CBitArray			bitRecMonth;
	CBitArray			bitRecDay;
	CBitArray			bitRecHour;
	CBitArray			bitRecMinute;
	CBitArray			bitRecDupMin;

	//XTREE_CAMERA_STATUS	DeviceStatus;	// ī�޶� Status
	DWORD			dwStreamStatus;

	BOOL			bRecordOn;

	RS_RECORD_INFO_T			recordInfo;			// ������ ��������
	RS_RECORD_SPECIAL_INF0_T	specialRecordInfo;	// ����ȵ��� ��������

	BOOL			bSupportPtz;

	// Hitron ����options�׸� ����
	// BUILTIN_PTZ (PTZ��Ʈ�� -> /ptz/control.php ���)
	// PTZ (PTZ��Ʈ�� -> /ptz/control.php ���)
	// BUILTIN_PTZ + PTZ (PTZ��Ʈ�� -> /ptz/control.php ���)
	// M_PTZ (PTZ��Ʈ�� -> /mptz/control.php)
	// M_PTZ + PTZ (PTZ��Ʈ�� -> /mptz/control.php, RS485��Ʈ�� -> /ptz/control.php )
	BOOL			optionBuiltInPTZ;
	BOOL			optionPTZ;
	BOOL			optionMPTZ;

	// Hitron Device Only
	BOOL			optionSmartFocus;

	time_t			tLastUpdateStream;		// ���������� ��Ʈ���� �޴� �ð�
	BOOL			bErrorConnect;			// ������ ���� ���� �Ǿ��°�
	BOOL			bIsNoDevice;			// ������ ��ϵ��� ���� ��ġ�ΰ�

	// 2011.11.15	Deinterlaced Filter�� ���� : ��ġ��������!!
	UINT			nVideoFilter;

	// DSKIM
	UINT			nChannelCount;			// channel count

public:
	RS_DEVICE_EVENT_INFO_T	eventInfo;

	BOOL			bCanBeUpdated;
	int				nSeq;					// Sort ID
} RS_DEVICE_INFO_T;

// DEVICE Info SET
typedef	struct	_RS_DEVICE_INFO_SET_T
{
	UINT				validDeviceCount;				// ��� ������ Device ��
	RS_DEVICE_INFO_T	deviceInfo[RS_MAX_DEVICE];		// Device Info array
	_RS_DEVICE_INFO_SET_T( VOID )
	{
		validDeviceCount = 0;
	}
} RS_DEVICE_INFO_SET_T;

// Device status info
typedef	struct	_RS_DEVICE_STATUS_INFO_T
{
	CStringW				strStreamID;			// Stream ID, guid ���
	UINT					nIsConnected;			// ��ġ ���� ����
	UINT					nBitrate;				// kbps
	UINT					nFPS;
	CStringW				strVideoCodec;			// video codec name
												// MJPEG, MPEG4, MPEG4 Simple Profile, MPEG4 Advanced Simple Profile, 
												// H264, H264 Baseline Profile, H264 Main Profile, H264 High Profile, Unknown
	CStringW				strAudioCodec;			// audio codec name
												// PCM8, PCM16, G711U, G711A, G726, AAC, Unknown
	UINT					nAudioBitrate;			// bps
	UINT					nAudioSamplerate;		// kHz
	UINT					nIsRecordingError;		// ���ڵ��� ������ �ִ°�?
	UINT					nIsVideoRecording;		// �����층 ������ ���� Video�� ���ڵ� �ǰ� �־�� �ϴ� �ð��ΰ�?
	UINT					nIsAudioRecording;		// �����층 ������ ���� Audio�� ���ڵ� �ǰ� �־�� �ϴ� �ð��ΰ�?
	UINT					nResolutionWidth;		// Video Width
	UINT					nResolutionHeight;		// Video Height
	RS_ERROR_TYPE			errorCode;				// Error code

	_RS_DEVICE_STATUS_INFO_T( VOID ) 
	{
		strStreamID = _T("");		
		nIsConnected = 0;
		nBitrate = 0;
		nFPS = 0;
		strVideoCodec = _T("");
		strAudioCodec = _T("");
		nAudioBitrate = 0;
		nAudioSamplerate = 0;
		nIsRecordingError = 0;
		nIsVideoRecording = 0;
		nIsAudioRecording = 0;
		nResolutionWidth = 0;
		nResolutionHeight = 0;
		errorCode = RS_ERROR_NO_ERROR;
	}

	_RS_DEVICE_STATUS_INFO_T( const _RS_DEVICE_STATUS_INFO_T &clone )
	{
		strStreamID = clone.strStreamID;			// Stream ID, guid ���
		nIsConnected = clone.nIsConnected;			// ��ġ ���� ����
		nBitrate = clone.nBitrate;				// kbps
		nFPS = clone.nFPS;
		strVideoCodec = clone.strVideoCodec;			// video codec name
		strAudioCodec = clone.strAudioCodec;			// audio codec name
		nAudioBitrate = clone.nAudioBitrate;			// bps
		nAudioSamplerate = clone.nAudioSamplerate;		// kHz
		nIsRecordingError = clone.nIsRecordingError;		// ���ڵ��� ������ �ִ°�?
		nIsVideoRecording = clone.nIsVideoRecording;		// �����층 ������ ���� Video�� ���ڵ� �ǰ� �־�� �ϴ� �ð��ΰ�?
		nIsAudioRecording = clone.nIsAudioRecording;		// �����층 ������ ���� Audio�� ���ڵ� �ǰ� �־�� �ϴ� �ð��ΰ�?
		nResolutionWidth = clone.nResolutionWidth;		// Video Width
		nResolutionHeight = clone.nResolutionHeight;		// Video Height
		errorCode = clone.errorCode;				// Error code
	}

	_RS_DEVICE_STATUS_INFO_T& operator=( const _RS_DEVICE_STATUS_INFO_T &clone )
	{
		strStreamID = clone.strStreamID;			// Stream ID, guid ���
		nIsConnected = clone.nIsConnected;			// ��ġ ���� ����
		nBitrate = clone.nBitrate;				// kbps
		nFPS = clone.nFPS;
		strVideoCodec = clone.strVideoCodec;			// video codec name
		strAudioCodec = clone.strAudioCodec;			// audio codec name
		nAudioBitrate = clone.nAudioBitrate;			// bps
		nAudioSamplerate = clone.nAudioSamplerate;		// kHz
		nIsRecordingError = clone.nIsRecordingError;		// ���ڵ��� ������ �ִ°�?
		nIsVideoRecording = clone.nIsVideoRecording;		// �����층 ������ ���� Video�� ���ڵ� �ǰ� �־�� �ϴ� �ð��ΰ�?
		nIsAudioRecording = clone.nIsAudioRecording;		// �����층 ������ ���� Audio�� ���ڵ� �ǰ� �־�� �ϴ� �ð��ΰ�?
		nResolutionWidth = clone.nResolutionWidth;		// Video Width
		nResolutionHeight = clone.nResolutionHeight;		// Video Height
		errorCode = clone.errorCode;				// Error code
		return (*this);
	}

} RS_DEVICE_STATUS_INFO_T;


// Device status info SET
typedef	struct	_RS_DEVICE_STATUS_SET_T
{
	UINT					validDeviceCount;						// ��� ������ Device ��
	RS_DEVICE_STATUS_INFO_T	deviceStatusInfo[RS_MAX_DEVICE];		// Device status Info array
	_RS_DEVICE_STATUS_SET_T( VOID )
	{
		validDeviceCount = 0;
	}
} RS_DEVICE_STATUS_SET_T;

// Device result status info
typedef	struct	_RS_DEVICE_RESULT_STATUS_T
{
	CStringW				strStreamID;		// Stream ID, guid ���
	RS_ERROR_TYPE			errorCode;			// Error code
} RS_DEVICE_RESULT_STATUS_T;

// Device result status info SET
typedef	struct	_RS_DEVICE_RESULT_STATUS_SET_T
{
	UINT						validDeviceCount;						// ��� ������ Device ��
	RS_DEVICE_RESULT_STATUS_T	deviceStatusInfo[RS_MAX_DEVICE];		// Device status Info array
	_RS_DEVICE_RESULT_STATUS_SET_T( VOID )
	{
		validDeviceCount = 0;
	}
} RS_DEVICE_RESULT_STATUS_SET_T;


///////////////////////  RECORDING  /////////////////////////
// check recording is ready or not
typedef	struct	_RS_RECORDING_STATUS_T
{
	CStringW			strStreamID;
	UINT				isRecording;
} RS_RECORDING_STATUS_T;

typedef	struct	_RS_RECORDING_STATUS_SET_T
{
	UINT					validDeviceCount;
	RS_RECORDING_STATUS_T	recordingStatus[RS_MAX_DEVICE];
	_RS_RECORDING_STATUS_SET_T( VOID )
	{
		validDeviceCount = 0;
	}
} RS_RECORDING_STATUS_SET_T;

// Normal Recording schedule Info
typedef	struct	_RS_NORMAL_SCHEDULE_INFO_T
{
	RS_RECORD_SCHEDULE		nScheduleType;		// Recording Schedule ����
	UINT					nAudio;				// Audio recording ����
	UINT					nSun_bitflag;		// sunday�� �ð��� recording ������ ��Ÿ���� 24bit bitflag (1bit->1�ð�)
	UINT					nMon_bitflag;		// monday�� �ð��� recording ������ ��Ÿ���� 24bit bitflag (1bit->1�ð�)
	UINT					nTue_bitflag;		// thesday�� �ð��� recording ������ ��Ÿ���� 24bit bitflag (1bit->1�ð�)
	UINT					nWed_bitflag;		// wednesday�� �ð��� recording ������ ��Ÿ���� 24bit bitflag (1bit->1�ð�)
	UINT					nThu_bitflag;		// thursday�� �ð��� recording ������ ��Ÿ���� 24bit bitflag (1bit->1�ð�)
	UINT					nFri_bitflag;		// friday�� �ð��� recording ������ ��Ÿ���� 24bit bitflag (1bit->1�ð�)
	UINT					nSat_bitflag;		// saturday�� �ð��� recording ������ ��Ÿ���� 24bit bitflag (1bit->1�ð�)
	_RS_NORMAL_SCHEDULE_INFO_T( VOID ) {}
	_RS_NORMAL_SCHEDULE_INFO_T( const _RS_NORMAL_SCHEDULE_INFO_T &clone )
	{
		nScheduleType = clone.nScheduleType;
		nAudio = clone.nAudio;		
		nSun_bitflag = clone.nSun_bitflag;
		nMon_bitflag = clone.nMon_bitflag;
		nTue_bitflag = clone.nTue_bitflag;
		nWed_bitflag = clone.nWed_bitflag;
		nThu_bitflag = clone.nThu_bitflag;
		nFri_bitflag = clone.nFri_bitflag;
		nSat_bitflag = clone.nSat_bitflag;
	}

	_RS_NORMAL_SCHEDULE_INFO_T operator=( const _RS_NORMAL_SCHEDULE_INFO_T &clone )
	{
		nScheduleType = clone.nScheduleType;
		nAudio = clone.nAudio;		
		nSun_bitflag = clone.nSun_bitflag;
		nMon_bitflag = clone.nMon_bitflag;
		nTue_bitflag = clone.nTue_bitflag;
		nWed_bitflag = clone.nWed_bitflag;
		nThu_bitflag = clone.nThu_bitflag;
		nFri_bitflag = clone.nFri_bitflag;
		nSat_bitflag = clone.nSat_bitflag;
		return (*this);
	}

} RS_NORMAL_SCHEDULE_INFO_T;

// Special Recording - Time Info
typedef	struct	_RS_SPECIAL_TIME_INFO_T
{
	RS_RECORD_SCHEDULE		nScheduleType;		// Recording Schedule ����
	UINT					nHour;				// Recording ���� �ð� (hour), ex) 1�� -> 1
	UINT					nAudio;
	_RS_SPECIAL_TIME_INFO_T( VOID ) {}
	_RS_SPECIAL_TIME_INFO_T( const _RS_SPECIAL_TIME_INFO_T &clone )
	{
		nScheduleType = clone.nScheduleType;
		nHour = clone.nScheduleType;
		nAudio = clone.nScheduleType;
	}
	_RS_SPECIAL_TIME_INFO_T operator=( const _RS_SPECIAL_TIME_INFO_T &clone )
	{
		nScheduleType = clone.nScheduleType;
		nHour = clone.nScheduleType;
		nAudio = clone.nScheduleType;
		return (*this);
	}
} RS_SPECIAL_TIME_INFO_T;

// Special Recording schedule Info
typedef	struct	_RS_SPECIAL_SCHEDULE_INFO_T
{
	UINT						nEnable;				// schedule ��� ���� (1:���, 0:�̻��)
	UINT						nMonth;					// Recording ������ Ư�� month, �ų� ����//  ex) 1�� -> 1
	UINT						nDay;					// Recording ������ Ư�� day, �ų� ����//  ex) 3�� -> 3
	UINT						validSpecialTimeCount;
	RS_SPECIAL_TIME_INFO_T		timeInfo[RS_MAX_HOUR];	// �ð��� schedule ����
	_RS_SPECIAL_SCHEDULE_INFO_T( VOID )
	{
		validSpecialTimeCount = 0;
	}
	_RS_SPECIAL_SCHEDULE_INFO_T( const _RS_SPECIAL_SCHEDULE_INFO_T &clone )
	{
		nEnable = clone.nEnable;
		nMonth = clone.nMonth;
		nDay = clone.nDay;
		validSpecialTimeCount = clone.validSpecialTimeCount;
		for( UINT index=0; index<validSpecialTimeCount; index++ )
		{
			timeInfo[index] = clone.timeInfo[index];
		}
	}
	_RS_SPECIAL_SCHEDULE_INFO_T operator=( const _RS_SPECIAL_SCHEDULE_INFO_T &clone )
	{
		nEnable = clone.nEnable;
		nMonth = clone.nMonth;
		nDay = clone.nDay;
		validSpecialTimeCount = clone.validSpecialTimeCount;
		for( UINT index=0; index<validSpecialTimeCount; index++ )
		{
			timeInfo[index] = clone.timeInfo[index];
		}
		return (*this);
	}
} RS_SPECIAL_SCHEDULE_INFO_T;

// Recording Rule Info
typedef	struct	_RS_RECORD_SCHEDULE_INFO_T
{
	//UINT							nUse;									// schedule ���� ��� ���� (1:���, 0:�̻��)
	CStringW						strStreamID;							// Stream ID, guid ���
	UINT							nPre;									// Pre-recording time (second)
	UINT							nPost;									// Post-recording time (second)
	RS_RECORDING_MODE				nRecordingMode;							// Recording ���� (normal or special)
	UINT							validNormalSchedCount;
	UINT							validSpecialSchedCount;
	RS_NORMAL_SCHEDULE_INFO_T		normalSchedInfo[RS_MAX_SCHEDULE_INFO];
	RS_SPECIAL_SCHEDULE_INFO_T		specialSchedInfo[RS_MAX_SCHEDULE_INFO];
	_RS_RECORD_SCHEDULE_INFO_T( VOID )
	{
		validNormalSchedCount = 0;
		validSpecialSchedCount = 0;
	}
	_RS_RECORD_SCHEDULE_INFO_T( const _RS_RECORD_SCHEDULE_INFO_T &clone )
	{
		strStreamID = clone.strStreamID;
		nPre = clone.nPre;
		nPost = clone.nPost;
		nRecordingMode = clone.nRecordingMode;
		validNormalSchedCount = clone.validNormalSchedCount;
		validSpecialSchedCount = clone.validSpecialSchedCount;
		UINT index;
		for( index=0; index<validNormalSchedCount; index++ )
		{
			normalSchedInfo[index] = clone.normalSchedInfo[index];
		}
		for( index=0; index<validSpecialSchedCount; index++ )
		{
			specialSchedInfo[index] = clone.specialSchedInfo[index];
		}
	}
	_RS_RECORD_SCHEDULE_INFO_T operator=( const _RS_RECORD_SCHEDULE_INFO_T &clone )
	{
		strStreamID = clone.strStreamID;
		nPre = clone.nPre;
		nPost = clone.nPost;
		nRecordingMode = clone.nRecordingMode;
		validNormalSchedCount = clone.validNormalSchedCount;
		validSpecialSchedCount = clone.validSpecialSchedCount;
		UINT index;
		for( index=0; index<validNormalSchedCount; index++ )
		{
			normalSchedInfo[index] = clone.normalSchedInfo[index];
		}
		for( index=0; index<validSpecialSchedCount; index++ )
		{
			specialSchedInfo[index] = clone.specialSchedInfo[index];
		}
		return (*this);
	}
} RS_RECORD_SCHEDULE_INFO_T;

// Recording Schedule Info SET
typedef	struct	_RS_RECORD_SCHEDULE_SET_T
{
	UINT						validScheduleCount;			// ��� ������ schedule ��
	RS_RECORD_SCHEDULE_INFO_T	scheduleInfos[RS_MAX_DEVICE];	// Schedule Info array
	_RS_RECORD_SCHEDULE_SET_T( VOID )
	{
		validScheduleCount = 0;
	}
} RS_RECORD_SCHEDULE_SET_T;


// Recording overwrite Info
typedef	struct	_RS_RECORD_OVERWRITE_INFO_T
{
	UINT						onoff;
} RS_RECORD_OVERWRITE_INFO_T;


// Recording Retention time Info
typedef	struct	_RS_RECORD_RETENTION_INFO_T
{
	UINT						enable;
	UINT						year;			// ex) 1�� -> 1
	UINT						month;			// ex) 1���� -> 1
	UINT						week;			// ex) 1�� -> 1
	UINT						day;			// ex) 7�� -> 7
	_RS_RECORD_RETENTION_INFO_T( VOID )
	{
		enable = 0;
		year = 0;
		month = 0;
		week = 0;
		day = 0;
	}
} RS_RECORD_RETENTION_INFO_T;

// Disk info
typedef	struct	_RS_DISK_INFO_T
{
	//unsigned int			nUse;					// ���� ��� ���� (1:���, 0:�̻��)
	_RS_DISK_INFO_T( VOID )
		: nRecordReserved(0)
		, nRecordUsed(0)
		, nCommitReserved(0) {}
	CStringW					strRootPath;
	CStringW					strDrive;
	CStringW					strVolumeSerial;
	UINT						nDriveType;
	DOUBLE						dbTotal;					// ��ũ �� ũ�� (GByte)
	DOUBLE						dbUsed;						// ��ũ ��뷮 (GByte)
	DOUBLE						dbTotalFree;				// ��ũ �������� (GByte)
	UINT64						nRecordReserved;			// ���ڵ� ���� ������ (Byte)
	UINT64						nRecordUsed;				// ���� ���ڵ� ������ (Byte)
	UINT64						nCommitReserved;			// ������ ���۵� ���ڵ� ������
} RS_DISK_INFO_T;

// Disk info SET
typedef	struct	_RS_DISK_INFO_SET_T
{
	UINT				validDiskCount;				// ��� ������ Disk ��
	RS_DISK_INFO_T		diskInfo[RS_MAX_DISK];		// Disk Info array
	_RS_DISK_INFO_SET_T( VOID )
	{
		validDiskCount = 0;
	}
} RS_DISK_INFO_SET_T;

// Disk Size Info
/*
typedef	struct	_RS_DISK_SIZE_INFO_T
{
	UINT64				sizeOfSpace;				// Reverse space size (Byte)
} RS_DISK_SIZE_INFO_T;
*/

// Disk Policy Info
typedef	struct	_RS_DISK_POLICY_INFO_T
{
//	unsigned int		nUse;						// ���� ��� ���� (1:���, 0:�̻��)
	CStringW				strVolumeSerial;
	UINT					validDeviceCount;			// ��� ������ Device ��
	CStringW				strMAC[RS_MAX_DEVICE];		// MAC Address array
} RS_DISK_POLICY_INFO_T;

// Disk Policy Info SET
typedef	struct	_RS_DISK_POLICY_SET_T
{
	UINT						validDiskCount;					// ��� ������ Disk ��
	RS_DISK_POLICY_INFO_T		diskPolicyInfo[RS_MAX_DISK];	// Info array
} RS_DISK_POLICY_SET_T;


// Disk Response info
typedef	struct	_RS_DISK_RESPONSE_INFO_T
{
	CStringW				strVolumeSerial;
	RS_ERROR_TYPE			errorCode;				// Error code
} RS_DISK_RESPONSE_INFO_T;

// Disk Response info SET
typedef	struct	_RS_DISK_RESPONSE_SET_T
{
	UINT					validDiskCount;				// ��� ������ Disk ��
	RS_DISK_RESPONSE_INFO_T	responseInfo[RS_MAX_DISK];	// Info array
} RS_DISK_RESPONSE_SET_T;


// Response info
typedef	struct	_RS_RESPONSE_INFO_T
{
	CStringW				strStreamID;			// Stream ID, guid ���
	CStringW				strMAC;
	RS_ERROR_TYPE			errorCode;				// Error code
} RS_RESPONSE_INFO_T;

// Response info SET
typedef	struct	_RS_RESPONSE_INFO_SET_T
{
	UINT					validDeviceCount;					// ��� ������ Device ��
	RS_RESPONSE_INFO_T		responseInfo[RS_MAX_DEVICE];		// Device status Info array
} RS_RESPONSE_INFO_SET_T;

// Data Delete Response info
typedef	struct	_RS_DELETE_RESPONSE_INFO_T
{
	CStringW			strMAC;					// MAC Address
	RS_ERROR_TYPE		errorCode;				// Error code
} RS_DELETE_RESPONSE_INFO_T;

// Data Delete Response info SET
typedef	struct	_RS_DELETE_RESPONSE_SET_T
{
	UINT						validDeviceCount;					// ��� ������ Device ��
	RS_DELETE_RESPONSE_INFO_T	responseInfo[RS_MAX_DEVICE];		// Response Info array
} RS_DELETE_RESPONSE_SET_T;


typedef	struct	_RS_RELAY_DEVICE_SET_T
{
	UINT					streamFlag;
	UINT					validDeviceCount;					// ��� ������ Device ��
	RS_DEVICE_INFO_T		deviceInfo[RS_MAX_PLAYBACK_CH];	// Device Info array
} RS_RELAY_DEVICE_SET_T;

// PLAYBACK DEVICE Info SET
typedef	struct	_RS_PLAYBACK_DEVICE_SET_T
{
	UINT					validDeviceCount;					// ��� ������ Device ��
	RS_DEVICE_INFO_T		deviceInfo[RS_MAX_PLAYBACK_CH];	// Device Info array
} RS_PLAYBACK_DEVICE_SET_T;


//BOOL	StartPlayback( RS_PLAYBACK_DEVICE_SET_T *pbDeviceList, RS_PLAYBACK_REQUEST_T *pbRequest, RS_PLAYBACK_INFO_T *pbInfo );
// Calendar Search Request Info
typedef	struct	_RS_SEARCH_REQUEST_T
{
	UINT		year;			// ex) 2012year -> 2012
	UINT		month;			// ex) 1month -> 1
	UINT		day;			// ex) 1day -> 1
	UINT		hour;			// ex) 1hour -> 1
	UINT		minute;			// ex) 1minute -> 1
	UINT		second;			// ex) 1second -> 1
} RS_SEARCH_REQUEST_T;

// Calendar Search Response Info
typedef struct _RS_SEARCH_HOUR_RESPONSE_T
{
	BOOL						bMinutes[RS_MAX_MINUTE];
	_RS_SEARCH_HOUR_RESPONSE_T( VOID )
	{
		for( UINT index=0; index<RS_MAX_MINUTE; index++ )
		{
			bMinutes[index] = FALSE;
		}
	}
} RS_SEARCH_HOUR_RESPONSE_T;

typedef struct _RS_SEARCH_DAY_RESPONSE_T
{
	BOOL						bHours[RS_MAX_HOUR];
	RS_SEARCH_HOUR_RESPONSE_T	hours[RS_MAX_HOUR];
	_RS_SEARCH_DAY_RESPONSE_T( VOID )
	{
		for( UINT index=0; index<RS_MAX_HOUR; index++ )
		{
			bHours[index] = FALSE;
		}
	}
} RS_SEARCH_DAY_RESPONSE_T;

typedef struct _RS_SEARCH_MONTH_RESPONSE_T
{
	BOOL						bDays[RS_MAX_DAY];
	RS_SEARCH_DAY_RESPONSE_T	days[RS_MAX_DAY];
	_RS_SEARCH_MONTH_RESPONSE_T( VOID )
	{
		for( UINT index=0; index<RS_MAX_DAY; index++ )
		{
			bDays[index] = FALSE;
		}
	}
} RS_SEARCH_MONTH_RESPONSE_T;


typedef	struct	_RS_SEARCH_RESPONSE_T
{
	BOOL						bMonths[RS_MAX_MONTH];
	RS_SEARCH_MONTH_RESPONSE_T	months[RS_MAX_MONTH];
	_RS_SEARCH_RESPONSE_T( VOID )
	{
		for( UINT index=0; index<RS_MAX_MONTH; index++ )
		{
			bMonths[index] = FALSE;
		}
	}
} RS_SEARCH_RESPONSE_T;

typedef	struct	_RS_SEARCH_RESPONSE_SET_T
{
	UINT					validDeviceCount;					// ��� ������ Device ��
	RS_SEARCH_RESPONSE_T	responseInfo[1];//RS_MAX_DEVICE
} RS_SEARCH_RESPONSE_SET_T;

///////////////////////  RELAY  /////////////////////////
typedef	struct	_RS_RELAY_REQUEST_T
{
	VOID					*pReceiver;		// IStreamReceiver5 pointer
	RS_RL_FRAME_TYPE		pbFrameType;
} RS_RELAY_REQUEST_T;

typedef	struct	_RS_RELAY_INFO_T
{
	VOID					*pReceiver;
	BOOL					isDisconnected;
	BOOL					isNoDevice;
	CStringW				szID;
	_RS_RELAY_INFO_T( VOID )
	{
		pReceiver = NULL;
		isDisconnected = FALSE;
		isNoDevice = FALSE;
		szID = "";
	}
} RS_RELAY_INFO_T;

///////////////////////  PLAYBACK  /////////////////////////
typedef	struct	_RS_PLAYBACK_REQUEST_T
{
	VOID					*pReceiver;		// IStreamReceiver5 pointer
	UINT					year;			// ex) 2012year -> 2012
	UINT					month;			// ex) 1month -> 1
	UINT					day;			// ex) 1day -> 1
	UINT					hour;			// ex) 1hour -> 1
	UINT					minute;			// ex) 1minute -> 1
	UINT					second;			// ex) 1minute -> 1
	RS_PB_FRAME_TYPE		pbFrameType;
	RS_PLAYBACK_DIRECTION	pbDirection;	// playback direction
	_RS_PLAYBACK_REQUEST_T( VOID )
	{
		pReceiver	= NULL;
		year		= 0;
		month		= 0;
		day			= 0;
		hour		= 0;
		minute		= 0;
		second		= 0;
		pbFrameType = RS_PB_FRAME_NONE;
		pbDirection = RS_PLAYBACK_DIRECTION_T_NONE;
	}
} RS_PLAYBACK_REQUEST_T;

// Playback Info
typedef	struct	_RS_PLAYBACK_INFO_T
{
	VOID					*pReceiver;
	BOOL					Enable;
	UINT					playbackID;				// Playback ID
	RS_PLAYBACK_DIRECTION	pbDirection;			// playback direction
	RS_PLAYBACK_SPEED		pbSpeed;				// playback speed
	BOOL					bOnlyKeyFrame;
} RS_PLAYBACK_INFO_T;

typedef	struct	_RS_PLAYBACK_JUMP_REQUEST_T
{
	UINT					year;			// ex) 2012year -> 2012
	UINT					month;			// ex) 1month -> 1
	UINT					day;			// ex) 1day -> 1
	UINT					hour;			// ex) 1hour -> 1
	UINT					minute;			// ex) 1minute -> 1
	UINT					second;
} RS_PLAYBACK_JUMP_REQUEST_T;

typedef	struct	_RS_PLAYBACK_GOTO_FIRST_RESPONSE_T
{
	UINT					year;			// ex) 2012year -> 2012
	UINT					month;			// ex) 1month -> 1
	UINT					day;			// ex) 1day -> 1
	UINT					hour;			// ex) 1hour -> 1
	UINT					minute;			// ex) 1minute -> 1
	UINT					second;
} RS_PLAYBACK_GOTO_FIRST_RESPONSE_T;

typedef	struct	_RS_PLAYBACK_GOTO_LAST_RESPONSE_T
{
	UINT					year;			// ex) 2012year -> 2012
	UINT					month;			// ex) 1month -> 1
	UINT					day;			// ex) 1day -> 1
	UINT					hour;			// ex) 1hour -> 1
	UINT					minute;			// ex) 1minute -> 1
	UINT					second;
} RS_PLAYBACK_GOTO_LAST_RESPONSE_T;

///////////////////////  EXPORT /////////////////////////
typedef	struct	_RS_EXPORT_REQUEST_T
{
	RS_EP_FRAME_TYPE		type;
	VOID					*pReceiver;
	UINT					bClipViewer;
//	UINT					overlapID;
	UINT					sYear;
	UINT					sMonth;
	UINT					sDay;
	UINT					sHour;
	UINT					sMinute;
	UINT					sSecond;
	UINT					eYear;
	UINT					eMonth;
	UINT					eDay;
	UINT					eHour;
	UINT					eMinute;
	UINT					eSecond;
} RS_EXPORT_REQUEST_T;

typedef	struct	_RS_EXPORT_RESPONSE_T
{
	VOID					*pReceiver;
	UINT					exportID;
	UINT					size;
} RS_EXPORT_RESPONSE_T;

typedef	struct	_RS_EXPORT_INFO_T
{
	UINT					exportID;
} RS_EXPORT_INFO_T;

typedef	struct	_RS_CONNECTION_STOP_NOTIFICATION_T
{
	CStringW					strUUID;
	CStringW					strReason;
} RS_CONNECTION_STOP_NOTIFICATION_T;


typedef	struct	_RS_DRIVE_PATH_NOTIFICATION_T
{
	CStringW					strUUID;
	CStringW					strVolumeSerial;
	CStringW					strRecordingPath;
} RS_DRIVE_PATH_NOTIFICATION_T;

typedef	struct	_RS_STORAGE_FULL_NOTIFICATION_T
{
	CStringW						strUUID;
	RS_DRIVE_PATH_NOTIFICATION_T	driveAndPath[RS_MAX_DISK];
	CStringW						strMac[RS_MAX_DEVICE];
	UINT							validDeviceAndPathCount;
	UINT							validMacCount;
	_RS_STORAGE_FULL_NOTIFICATION_T( VOID )
	{
		validDeviceAndPathCount = 0;
		validMacCount = 0;
	}
} RS_STORAGE_FULL_NOTIFICATION_T;

typedef	struct	_RS_TARGET_FILE_PATH_NOTIFICATION_T
{
	CStringW				strUUID;
	UINT					nFileExist;
	CStringW				strRecordingFilePath;
} RS_TARGET_FILE_PATH_NOTIFICATION_T;

typedef	struct	_RS_OVERWRITE_ERROR_NOTIFICATION_T
{
	CStringW							strUUID;
	RS_TARGET_FILE_PATH_NOTIFICATION_T	targetFilePath[RS_MAX_DEVICE*RS_MAX_DISK];
	UINT								validTargetFilePath;
} RS_OVERWRITE_ERROR_NOTIFICATION_T;

typedef	struct	_RS_CONFIGURATION_CHANGED_NOTIFICATION_T
{
	CStringW							strUUID;
} RS_CONFIGURATION_CHANGED_NOTIFICATION_T;

typedef	struct	_RS_PLAYBACK_ERROR_NOTIFICATION_T
{
	CStringW		strUUID;
	UINT			nPlayBackID;
	CStringW		strMac;
	CStringW		strReason;
	UINT			year;
	UINT			month;
	UINT			day;
	UINT			hour;
	UINT			minute;
	UINT			second;
} RS_PLAYBACK_ERROR_NOTIFICATION_T;

typedef	struct	_RS_DISK_ERROR_NOTIFICATION_T
{
	CStringW		strUUID;
	CStringW		hraFilePath;
} RS_DISK_ERROR_NOTIFICATION_T;

typedef	struct	_RS_KEY_FRAME_MODE_NOTIFICATION_T
{
	CStringW		strUUID;
	CStringW		strMac;
} RS_KEY_FRAME_MODE_NOTIFICATION_T;

typedef	struct	_RS_BUFFER_CLEAN_NOTIFICATION_T
{
	CStringW		strUUID;
} RS_BUFFER_CLEAN_NOTIFICATION_T;

/*
�÷��̹�, ������ ��� �Ʒ� �������� ��Ʈ���� ��
StreamingMessage (4bytes) + data size (4bytes) + data (���ڵ� ���� ���˿� �ִ� ����ü ���Ĵ��)

StreamingMessage�� ���� ���۵Ǵ� ������ ����

NCNetworkLib.h -> NCLiveFrameType �����ϼ���.

SEND_FRAME = 1, FRAME_HEADER + binary
SEND_VIDEO_CODEC = 2, VIDEO_CODEC
SEND_AUDIO_CODEC = 3, AUDIO_CODEC
SEND_XML = 4, xml
*/

typedef enum _RS_HEADER_TYPE
{
	RS_HEADER_UNKNOWN = 0,
	RS_HEADER_FILE,
	RS_HEADER_STREAM,
	RS_HEADER_BLOCK,
	RS_HEADER_FRAME
} RS_HEADER_TYPE;

typedef enum _RS_VIDEO_TYPE
{
	RS_VIDEO_UNKNOWN	= 0,
	RS_VIDEO_MJPEG,
	RS_VIDEO_MPEG4,
	RS_VIDEO_MPEG4SP,
	RS_VIDEO_MPEG4ASP,
	RS_VIDEO_H264,
	RS_VIDEO_H264BP,
	RS_VIDEO_H264MP,
	RS_VIDEO_H264HP
} RS_VIDEO_TYPE;

typedef enum _RS_AUDIO_TYPE
{
	RS_AUDIO_UNKNOWN	= 0,
	RS_AUDIO_PCM8,
	RS_AUDIO_PCM16,
	RS_AUDIO_G711U,
	RS_AUDIO_G711A,
	RS_AUDIO_G726,
	RS_AUDIO_AAC
} RS_AUDIO_TYPE;

#pragma pack(push, _RS_T_)
#pragma pack(1)
typedef struct _RS_VIDEO_CODEC_T
{
	RS_VIDEO_TYPE		type;
	UINT32				channel;
	UINT32				stream;
	UINT32				width;
	UINT32				height;
	UINT32				header1_position; // H.264 SPS header position, MPEG4 VOL header position
	UINT32				header1_size; 
	UINT32				header2_position; // H.264 PPS header position
	UINT32				header2_size;
	UINT32				extra_size;
	//char				extra_data[max_video_extra_size];	// MPEG-4 VOL Header, H.264 SPS/PPS Header data(comma�� ������)
} RS_VIDEO_CODEC_T;

typedef struct _RS_AUDIO_CODEC_T
{
	RS_AUDIO_TYPE		type;
	UINT32				channel;
	UINT32				stream;
	UINT32				bit_rate; // bps
	UINT32				sample_rate; // kHz
	UINT32				extra_size;
	//char				extra_data[max_audio_extra_size];	// AAC Decoding Initial Block
} RS_AUDIO_CODEC_T;

typedef enum _RS_FRAME_TYPE
{
	RS_FRAME_UNKNOWN = 0x00000000,
	RS_FRAME_VIDEO = 0x00000001,
	RS_FRAME_AUDIO = 0x00000002,
	RS_FRAME_META_IMAGE = 0x00000004, // ��Ʈ��-ä�� �����ؼ� ����
	RS_FRAME_META_TEXT = 0x00000008, // ��Ʈ��-ä�� �����ؼ� ����
	RS_FRAME_META_PTZ = 0x00000010,
	RS_FRAME_META_ONBOOT = 0x00000020, // ��Ʈ��-ä�� �ε������� �ߺ� ����, ����������� Ÿ�Ը� ���
	RS_FRAME_META_ALARMIN = 0x00000040, // ��Ʈ��-ä�� �ε������� �ߺ� ����
	RS_FRAME_META_TRIGGER = 0x00000080, // ��Ʈ��-ä�� �ε������� �ߺ� ����
	RS_FRAME_META_MOTION = 0x00000100, // ��Ʈ��-ä�� �����ؼ� ����
	RS_FRAME_META_VLOSS = 0x00000200, // ä���� �����ϰ� ��Ʈ�� �ε������� �ߺ� ����, ����������� Ÿ�Ը� ���
	RS_FRAME_META_NETLOSS = 0x00000400, // ��Ʈ��-ä�� �ε������� �ߺ� ����, ����������� Ÿ�Ը� ���
	RS_FRAME_META_VA_MOTION = 0x00000800,
	RS_FRAME_META_VA_TRACKING = 0x00010000,
	RS_FRAME_META_VA_CROSS = 0x00020000,
	RS_FRAME_META_VA_ABANDON = 0x00040000,
	RS_FRAME_META_VA_REMOVAL = 0x00080000,
	RS_FRAME_META_VA_ENTER = 0x00100000,	
	RS_FRAME_META_VA_ALARM = 0x00200000,
	RS_FRAME_META_VA_ZONE = 0x00400000,
	RS_FRAME_META_VA_FACE = 0x00800000,
	RS_FRAME_META_VA_MINE = 0x01000000,
	RS_FRAME_META_VA_SCNCHG = 0x02000000,
	RS_FRAME_META_VA_UNFOC = 0x04000000
} RS_FRAME_TYPE;

typedef struct _RS_FRAME_HEADER_T
{
	RS_HEADER_TYPE	header_type;
	UINT32				channel; // 0-based
	UINT32				stream; // 0-based
	UINT32				frame_size;							// frame data size
	UINT32				watermark_size;						// watermark size
	RS_FRAME_TYPE		type;								// frame type
	INT32				keyframe;							// video frame only, if mjpeg, this is true
	UINT32				date;								// recording date (YYYYmmdd Format)
	UINT32				time;								// recording time (H(H)MMSSsss Format)
	UINT64				block_header_position;
	UINT64				keyframe_position;					// ���� ��ü���� ��ȿ
	UINT64				next_frame_position;				// current channel&stream's next frame position
	// ���� ��Ͽ����� ��ȿ
} RS_FRAME_HEADER_T;

///////////////////////////////////////////////////////////////////////////////////////////////////////
// Event META Frame
typedef struct _RS_FRAME_ALARMIN_T
{
	UINT32				alarm_number; // 1-based
} RS_FRAME_ALARMIN_T;

typedef struct _RS_FRAME_TRIGGER_T
{
	UINT32				trigger_number; // 1-based
} RS_FRAME_TRIGGER_T;

// ��������� (0,0), ���� �ػ󵵿� ���� ��ǥ��
typedef struct _RS_FRAME_MOTION_T
{
	UINT32			region_number; // 1-based
	UINT32			left, right, top, bottom;
	UINT32			sensitivity; // 1 ~ 100
	UINT32			threshold; // 1 ~ 100
	UINT32			activity; // 1 ~ 100, activity���� threshold���� ũ�� ����� �߻��� ����
} RS_FRAME_MOTION_T;

// ��������� (0,0), ���� �ػ󵵿� ���� ��ǥ��
typedef struct _RS_FRAME_TEXT_T
{
	UINT32 left, top; // position
	UINT32 text_length;
	//char text[0]; // utf-8
} RS_FRAME_TEXT_T;

// ��������� (0,0), ���� �ػ󵵿� ���� ��ǥ��
typedef struct _RS_FRAME_IMAGE_T
{
	UINT32 left, top; // position
	UINT32 image_length;
	//char image[0]; // binary data
} RS_FRAME_IMAGE_T;

///////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma pack(pop, _RS_T_)

typedef struct _EVENT_INFO_T
{
	_EVENT_INFO_T( VOID )
	{
		ZeroMemory(&tAlarmUpdated, sizeof(__timeb64));
		ZeroMemory(&tMotionUpdated, sizeof(__timeb64));
		ZeroMemory(&tTriggerUpdated, sizeof(__timeb64));
		bAlarm = FALSE;
		bMotion = FALSE;
		bTrigger	= FALSE;
	}
	BOOL TimeLag(__timeb64* tSrc, __timeb64* tTime, int milliseconds)
	{
		int diff = ((int)(tTime->time - tSrc->time) * 1000) + (tTime->millitm - tSrc->millitm);

		return diff >= milliseconds;
	}

	// �ð�
	__timeb64	tAlarmUpdated;
	__timeb64	tMotionUpdated;
	__timeb64	tTriggerUpdated;

	// �̺�Ʈ �÷���
	BOOL		bAlarm;
	BOOL		bMotion;
	BOOL		bTrigger;

} EVENT_INFO_T;

typedef union _RS_META_FRAME_T
{
	RS_FRAME_ALARMIN_T		alarm;
	RS_FRAME_TRIGGER_T		trigger;
	RS_FRAME_MOTION_T		motion;
} RS_META_FRAME_T;

typedef struct _RS_META_INFO_T
{
public:
	_RS_META_INFO_T( VOID )
		: pData(NULL)
		,bLog(FALSE)
		,bOn(FALSE) {}
	~_RS_META_INFO_T( VOID ) 
	{ 
		if(pData) 
			delete [] pData; 
	}
	INT					nFrameType;
	std::wstring		sDeviceID;
	__timeb64			tTime;
	CHAR				*pData;
	RS_META_FRAME_T		Frame;
	BOOL				bLog;
	BOOL				bOn;
} RS_META_INFO_T;


typedef struct _RS_STREAM_BUFFER_T : public GRSTREAM_BUFFER
{
	_RS_STREAM_BUFFER_T( VOID ) {}
	_RS_STREAM_BUFFER_T( CONST VOID* pData, size_t nDataSize )
		: Buffer(nDataSize)
		, nDate(0)
		, nTime(0)
		, nDecodeID(0)
		, bNoSkip(FALSE)
		, nCount(0)
		, nDecodeFailCount(0)
	{
		ZeroMemory(this, sizeof(GRSTREAM_BUFFER));
		nFrameType = -1;
		Buffer.Assign(pData, nDataSize);
	}

	CBuffer Buffer;
	struct __timeb64 tReceived;
	UINT	nDate;
	UINT	nTime;
	size_t	nDecodeID;
	BOOL	bNoSkip;
	size_t	nCount;
	int		nDecodeFailCount;
} RS_STREAM_BUFFER_T;

typedef struct _RS_FRAME_SOURCE_T
{
	INT				nId;
	CStringW		strMac;
	INT				nStreamIdx;
	INT				nChannelIdx;
	CStringW		strProfile;
	CStringW		strDeviceName;
} RS_FRAME_SOURCE_T, *LPRS_FRAME_SOURCE_T;

typedef struct _RS_SERVER_INFO_T
{
	_RS_SERVER_INFO_T( VOID )
		: nId(-1)
		, bEnable(FALSE)
		, nPort(RS_DEFAULT_SERVICE_PORT) {}
	UINT			nId;			// 0, 1, 2, 3, ... �������
	CStringW		strServerId;	// guid
	BOOL			bEnable;
	CStringW		strName;
	CStringW		strAddress;
	UINT			nPort;
	CStringW		strUserId;
	CStringW		strUserPassword;
	CStringW		strDesc;
	BOOL			bRunAsHTRecorder;
	ULONG			fileVersion;
	ULONG			dbVersion;
} RS_SERVER_INFO_T;

typedef struct _RS_PLAYBACK_STREAM_INFO_T
{
	AVMEDIA_TYPE	avType;
	INT32			stream;
	SIZE			szVideoSize;
	INT32			audioBitrate;
	INT32			audioSampleRate;
	BOOL			bRun;
	_RS_PLAYBACK_STREAM_INFO_T( VOID )
	{
		avType = AVMEDIA_UNKNOWN;
		stream = 0;
		szVideoSize.cx = 0;
		szVideoSize.cy = 0;
		audioBitrate = 0;
		audioSampleRate = 0;
		bRun = FALSE;
	}
} RS_PLAYBACK_STREAM_INFO_T;

typedef struct _RS_RELAY_STREAM_INFO_T
{
	INT32			stream;
	AVMEDIA_TYPE	videoType;
	SIZE			szVideoSize;
	AVMEDIA_TYPE	audioType;
	INT32			audioBitrate;
	INT32			audioSampleRate;
	BOOL			bRun;
	_RS_RELAY_STREAM_INFO_T( VOID )
	{
		stream = 0;
		videoType = AVMEDIA_UNKNOWN;
		audioType = AVMEDIA_UNKNOWN;
		szVideoSize.cx = 0;
		szVideoSize.cy = 0;
		audioBitrate = 0;
		audioSampleRate = 0;
		bRun = FALSE;
	}
} RS_RELAY_STREAM_INFO_T;

typedef	enum	_RS_EP_MESSAGE_TYPE
{
	RS_EP_MESSAGE_FINISHED = 0,
	RS_EP_MESSAGE_DISCONNECTED,
	RS_EP_MESSAGE_WRITE_FAILED,
	// pMsgParam is percent
	RS_EP_MESSAGE_PROGRESS
} RS_EP_MESSAGE_TYPE;

typedef void (CALLBACK* LPEXPORTMSGHANDLER)( RS_EP_MESSAGE_TYPE eMessage, LPVOID pMsgParam, LPVOID pRegParam );

class MovieMaker;
class AFX_EXT_CLASS ExportStreamReceiver : public IStreamReceiver5
{
public:
	ExportStreamReceiver( VOID );
	virtual ~ExportStreamReceiver( VOID );

	int				Start( CTime tStart, CTime tEnd, CString strFilePath );
	int				Stop( VOID );

	VOID*			GetHandle( VOID );
	VOID			SetHandle( VOID *handle );
	int				GetExportID( VOID ) { return _nExportID; };
	virtual void	OnNotifyMessage(LiveNotifyMsg* pNotify) override;
	virtual void	OnReceive(LPStreamData Data) override;

	VOID RegisterMsgHandler( LPEXPORTMSGHANDLER pHandler, LPVOID pParam )
	{
			_pMsgHandler	= pHandler;
			_pHandlerParam	= pParam;
	};
protected:
	VOID							*_handle;
	CTime							_tStart;
	CTime							_tEnd;
	int								_nExportID;


	LPEXPORTMSGHANDLER				_pMsgHandler;
	LPVOID							_pHandlerParam;
	MovieMaker						*_clsMaker;
};

class AFX_EXT_CLASS IRelayStreamReceiver : public IStreamReceiver5
{
public:
	VOID* GetHandle( VOID );
	VOID SetHandle( VOID *handle );

protected:
	VOID							*_handle;
};

class AFX_EXT_CLASS IPlayBackStreamReceiver : public IStreamReceiver5
{
public:
	VOID* GetHandle( VOID );
	VOID SetHandle( VOID *handle );

protected:
	VOID							*_handle;
};

class AFX_EXT_CLASS IExportStreamReceiver : public IStreamReceiver5
{
public:
	VOID* GetHandle( VOID );
	VOID SetHandle( VOID *handle );

protected:
	VOID							*_handle;
};

class HTRecorder;
class AFX_EXT_CLASS INotificationReceiver	
{
public:
	virtual VOID	SetHTRecorder(HTRecorder * recorder)=0;
	virtual VOID	OnConnectionStop( RS_CONNECTION_STOP_NOTIFICATION_T *notification )=0;
	virtual VOID	OnRecordingStorageFull( RS_STORAGE_FULL_NOTIFICATION_T *notification )=0;
	virtual VOID	OnReservedStorageFull( RS_STORAGE_FULL_NOTIFICATION_T *notification )=0;
	virtual VOID	OnOverwritingError( RS_OVERWRITE_ERROR_NOTIFICATION_T *notification )=0;
	virtual VOID	OnConfigurationChanged( RS_CONFIGURATION_CHANGED_NOTIFICATION_T *notification )=0;
	virtual VOID	OnPlaybackError( RS_PLAYBACK_ERROR_NOTIFICATION_T *notification )=0;
	virtual VOID	OnDiskError( RS_DISK_ERROR_NOTIFICATION_T *notification )=0;
	virtual VOID	OnKeyFrameMode( RS_KEY_FRAME_MODE_NOTIFICATION_T *notification )=0;
	virtual VOID	OnBufferClean( RS_BUFFER_CLEAN_NOTIFICATION_T *notification )=0;
};

class ServiceCore;
class AFX_EXT_CLASS HTRecorder
{
public:
	HTRecorder( INotificationReceiver *notifier=NULL );
	virtual ~HTRecorder( VOID );
	

	BOOL	Connect( WCHAR *UUID, BOOL bRunAsRecorder=FALSE, WCHAR *address=RS_DEFAULT_SERVICE_ADDRESS, WCHAR *userID=RS_DEFAULT_USER, WCHAR *userPW=RS_DEFAULT_PASSWORD, DWORD dwMaxChannel=1, DWORD dwNumberOfThread=1, ULONG fileVersion=RS_FILE_VERSION, ULONG dbVersion=RS_DB_VERSION );
	BOOL	Reconnect(VOID);
	BOOL	Disconnect( VOID );
	BOOL	IsConnected( VOID );
	///////////////////////  LOGIN  /////////////////////////
	BOOL	KeepAliveRequest( VOID );
	
	///////////////////////  DEVICE  /////////////////////////
	BOOL	GetDeviceList( RS_DEVICE_INFO_SET_T *deviceInfoList ); 	//������ ��ϵ� ��ġ ��� ��������
	BOOL	AddDevice( RS_DEVICE_INFO_SET_T *deviceInfoList, RS_DEVICE_RESULT_STATUS_SET_T *deviceResultStatusList ); //��ġ ���
	BOOL	UpdateDevice( RS_DEVICE_INFO_SET_T *deviceInfoList, RS_DEVICE_RESULT_STATUS_SET_T *deviceResultStatusList ); //��ġ ����
	BOOL	RemoveDevice( RS_DEVICE_INFO_SET_T *deviceInfoList, RS_DEVICE_RESULT_STATUS_SET_T *deviceResultStatusList ); //��ġ ����
	BOOL	RemoveDeviceEx( RS_DEVICE_INFO_SET_T *deviceInfoList, RS_DEVICE_RESULT_STATUS_SET_T *deviceResultStatusList ); //��ġ ����
	BOOL	CheckDeviceStatus( RS_DEVICE_INFO_SET_T *deviceInfoList, RS_DEVICE_STATUS_SET_T *deviceStatusList ); //��ġ Status ���� ��û

	
	///////////////////////  RECORDING  /////////////////////////
	BOOL	GetRecordingScheduleList( RS_DEVICE_INFO_SET_T *deviceInfoList, RS_RECORD_SCHEDULE_SET_T *recordShcedList );
	BOOL	UpdateRecordingSchedule( RS_RECORD_SCHEDULE_SET_T *recordSchedList, RS_RESPONSE_INFO_SET_T *responseInfoList );
	BOOL	SetRecordingOverwrite( RS_RECORD_OVERWRITE_INFO_T *overwriteInfo, RS_RESPONSE_INFO_T *responseInfo );
	BOOL	GetRecordingOverwrite( RS_RECORD_OVERWRITE_INFO_T *overwriteInfo );
	BOOL	SetRecordingRetentionTime( RS_RECORD_RETENTION_INFO_T *retentionInfo, RS_RESPONSE_INFO_T *responseInfo );
	BOOL	GetRecordingRetentionTime( RS_RECORD_RETENTION_INFO_T *retentionInfo );
	BOOL	GetDiskInfo( RS_DISK_INFO_SET_T *diskInfoList );
	BOOL	ReserveDiskSpace( RS_DISK_INFO_SET_T *diskInfoList, RS_DISK_RESPONSE_SET_T *diskResponseInfoList );
	BOOL	GetDiskPolicy( RS_DISK_POLICY_SET_T *diskPolicyList );
	BOOL	UpdateDiskPolicy( RS_DISK_INFO_SET_T *diskInfo, RS_DEVICE_INFO_SET_T *deviceInfoList, RS_DISK_RESPONSE_SET_T *diskResponseList );
	BOOL	IsRecording( RS_DEVICE_INFO_SET_T *deviceInfoList, RS_RECORDING_STATUS_SET_T *recordingStatusList );
	BOOL	StartRecordingRequest( RS_DEVICE_INFO_SET_T *deviceInfoList, RS_RESPONSE_INFO_SET_T *responseInfoList );
	BOOL	StopRecordingRequest( RS_DEVICE_INFO_SET_T *deviceInfoList, RS_RESPONSE_INFO_SET_T *responseInfoList );
	BOOL	StartRecordingAll( VOID );
	BOOL	StopRecordingAll( VOID );
	BOOL	StartManualRecording( RS_DEVICE_INFO_SET_T *deviceInfoList, RS_RESPONSE_INFO_SET_T *resposneInfoList );
	BOOL	StopManualRecording( RS_DEVICE_INFO_SET_T *deviceInfoList, RS_RESPONSE_INFO_SET_T *responseInfList );
	BOOL	DeleteRecordingData( RS_DEVICE_INFO_SET_T *deviceInfoList, RS_DELETE_RESPONSE_SET_T *deleteResponseList );

	///////////////// RELAY STREAM
	BOOL	GetRelayInfo( VOID *xmlData, RS_RELAY_INFO_T *rlInfo );
	BOOL	StartRelay( RS_DEVICE_INFO_T *relayDevice, RS_RELAY_REQUEST_T *rlRequest );
	BOOL	UpdateRelay( RS_DEVICE_INFO_T *relayDevice, RS_RELAY_REQUEST_T *rlRequest );
	BOOL	StopRelay( RS_RELAY_INFO_T *rlInfo );

	///////////////// PLAYBACK STREAM
	//CALENDAR SEARCH
	BOOL	GetYearIndex( RS_PLAYBACK_DEVICE_SET_T *pbDeviceList, RS_SEARCH_REQUEST_T *searchRequest, RS_SEARCH_RESPONSE_SET_T *searchRspList );
	BOOL	GetMonthIndex( RS_PLAYBACK_DEVICE_SET_T *pbDeviceList, RS_SEARCH_REQUEST_T *searchRequest, RS_SEARCH_RESPONSE_SET_T *searchRspList );
	BOOL	GetDayIndex( RS_PLAYBACK_DEVICE_SET_T *pbDeviceList, RS_SEARCH_REQUEST_T *searchRequest, RS_SEARCH_RESPONSE_SET_T *searchRspList );
	BOOL	GetHourIndex( RS_PLAYBACK_DEVICE_SET_T *pbDeviceList, RS_SEARCH_REQUEST_T *searchRequest, RS_SEARCH_RESPONSE_SET_T *searchRspList );

	BOOL	GetMonths( RS_PLAYBACK_DEVICE_SET_T *pbDeviceList, RS_SEARCH_REQUEST_T *searchRequest, CAtlArray<CString> *months );
	BOOL	GetDays( RS_PLAYBACK_DEVICE_SET_T *pbDeviceList, RS_SEARCH_REQUEST_T *searchRequest, CAtlArray<CString> *days );
	BOOL	GetHours( RS_PLAYBACK_DEVICE_SET_T *pbDeviceList, RS_SEARCH_REQUEST_T *searchRequest, CAtlArray<CString> *hours );
	BOOL	GetMinutes( RS_PLAYBACK_DEVICE_SET_T *pbDeviceList, RS_SEARCH_REQUEST_T *searchRequest, CAtlArray<CString> *minutes );

	//PLAYBACK
	BOOL	StartPlayback( RS_PLAYBACK_DEVICE_SET_T *pbDeviceList, RS_PLAYBACK_REQUEST_T *pbRequest, RS_PLAYBACK_INFO_T *pbInfo );
	BOOL	GetPlaybackInfo( VOID *xmlData, RS_PLAYBACK_INFO_T *pbInfo );
	BOOL	StopPlayback( RS_PLAYBACK_INFO_T *pbInfo );
	//PLAYBACK CONTROL
	BOOL	ControlPlay( RS_PLAYBACK_INFO_T *pbInfo );
	BOOL	ControlFowardPlay( RS_PLAYBACK_INFO_T *pbInfo );
	BOOL	ControlBackwardPlay( RS_PLAYBACK_INFO_T *pbInfo );
	BOOL	ControlStop( RS_PLAYBACK_INFO_T *pbInfo );
	BOOL	ControlPause( RS_PLAYBACK_INFO_T *pbInfo );
	BOOL	ControlResume( RS_PLAYBACK_INFO_T *pbInfo );
	BOOL	ControlJump( RS_PLAYBACK_JUMP_REQUEST_T *pbRequest, RS_PLAYBACK_INFO_T *pbInfo );

	BOOL	ControlGoToFirst( RS_PLAYBACK_INFO_T *pbInfo, RS_PLAYBACK_GOTO_FIRST_RESPONSE_T *pbResponse );
	BOOL	ControlGoToLast( RS_PLAYBACK_INFO_T *pbInfo, RS_PLAYBACK_GOTO_LAST_RESPONSE_T *pbResponse );
	BOOL	ControlForwardStep( RS_PLAYBACK_INFO_T *pbInfo );
	BOOL	ControlBackwardStep( RS_PLAYBACK_INFO_T *pbInfo );

	///////////////// Export
	BOOL	StartExport( RS_DEVICE_INFO_SET_T *devInfoList, RS_EXPORT_REQUEST_T *expRequest, RS_EXPORT_RESPONSE_T *expResponse );
	BOOL	StopExport( RS_EXPORT_INFO_T *expInfo );
	BOOL	PauseExport( RS_EXPORT_INFO_T *expInfo );
	BOOL	ResumeExport( RS_EXPORT_INFO_T *expInfo );


	///////////////// ServerNotification
	VOID	OnConnectionStop( RS_CONNECTION_STOP_NOTIFICATION_T *notification );
	VOID	OnRecordingStorageFull( RS_STORAGE_FULL_NOTIFICATION_T *notification );
	VOID	OnReservedStorageFull( RS_STORAGE_FULL_NOTIFICATION_T *notification );
	VOID	OnOverwritingError( RS_OVERWRITE_ERROR_NOTIFICATION_T *notification );
	VOID	OnConfigurationChanged( RS_CONFIGURATION_CHANGED_NOTIFICATION_T *notification );
	VOID	OnPlaybackError( RS_PLAYBACK_ERROR_NOTIFICATION_T *notification );
	VOID	OnDiskError( RS_DISK_ERROR_NOTIFICATION_T *notification );
	VOID	OnKeyFrameMode( RS_KEY_FRAME_MODE_NOTIFICATION_T *notification );
	VOID	OnBufferClean( RS_BUFFER_CLEAN_NOTIFICATION_T *notification );
	
private:
	RS_SERVER_INFO_T		_serverInfo;

	ServiceCore				*_service;
	VOID					*_liveSession;

	WCHAR					*_UUID;
	INotificationReceiver	*_notifier;
};

