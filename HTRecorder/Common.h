// stdafx.h : ���� ��������� ���� ��������� �ʴ�
// ǥ�� �ý��� ���� ���� �� ������Ʈ ���� ���� ������
// ��� �ִ� ���� �����Դϴ�.

#pragma once

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // ���� ������ �ʴ� ������ Windows ������� �����մϴ�.
#endif

#include "targetver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // �Ϻ� CString �����ڴ� ��������� ����˴ϴ�.

#include <afxwin.h>         // MFC �ٽ� �� ǥ�� ���� ����Դϴ�.
#include <afxext.h>         // MFC Ȯ���Դϴ�.

#ifndef _AFX_NO_OLE_SUPPORT
#include <afxole.h>         // MFC OLE Ŭ�����Դϴ�.
#include <afxodlgs.h>       // MFC OLE ��ȭ ���� Ŭ�����Դϴ�.
#include <afxdisp.h>        // MFC �ڵ�ȭ Ŭ�����Դϴ�.
#endif // _AFX_NO_OLE_SUPPORT

#ifndef _AFX_NO_DB_SUPPORT
#include <afxdb.h>                      // MFC ODBC �����ͺ��̽� Ŭ�����Դϴ�.
#endif // _AFX_NO_DB_SUPPORT

#ifndef _AFX_NO_DAO_SUPPORT
#include <afxdao.h>                     // MFC DAO �����ͺ��̽� Ŭ�����Դϴ�.
#endif // _AFX_NO_DAO_SUPPORT

#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>           // Internet Explorer 4 ���� ��Ʈ�ѿ� ���� MFC �����Դϴ�.
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>                     // Windows ���� ��Ʈ�ѿ� ���� MFC �����Դϴ�.
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxsock.h>            // MFC ���� Ȯ���Դϴ�.

#define MAX_LIVE_CHANNEL (64)
#define DEFAULT_NCSERVICE_PORT				11550	// Nautilus ���ڵ� ���� �⺻ Port
#define NAUTILUS_FILE_VERSION				10300
#define NAUTILUS_DB_VERSION					5

#include <process.h>
#include <map>
#include <set>
#include <queue>
#include <XML.h>
#include <LiveSession5.h>
#include <MessageSender.h>
#include <LiveSessionDLL.h>
#include <HTRecorderDLL.h>
#include <atlconv.h>

#define MAKEUINT64(l, h)	(unsigned __int64)( ((unsigned __int64)h << 32) | ((unsigned __int64)l & 0xffffffff) )
#define MAKEHIULONG(x)		(unsigned long)(x >> 32)
#define MAKELOULONG(x)		(unsigned long)(x & 0xffffffff)


#define XML_UTF8_HEAD_WSTR					L"<?xml version=\"1.0\" encoding=\"utf-8\"?>"
#define XML_UTF8_HEAD_STR					"<?xml version=\"1.0\" encoding=\"utf-8\"?>"


/*
// memory leak check
#include <crtdbg.h>
#define DEBUG_CLIENTBLOCK new( _CLIENT_BLOCK, __FILE__, __LINE__)
#define new DEBUG_CLIENTBLOCK
*/