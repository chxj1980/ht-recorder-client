#pragma once
#include <queue>
#include <list>
#include <atlcoll.h>
using namespace std;

typedef void (__stdcall *WORKERFUNCTION)(LPVOID arg, size_t key);

class ThreadPool
{
public:
	typedef struct _worker_t
	{
		ThreadPool *pThreadPool;
		int nIndex;
		//BOOL bThreadRun;
		LONG bThreadRun;
		HANDLE hEvent;
		HANDLE hThread;
		DWORD dwThreadId;
	} worker_t;

	typedef struct _function_param_t
	{
		LPVOID	arg;
		size_t	nKey;
	} function_param_t;

	typedef struct _function_t
	{
		GUID id;
		WORKERFUNCTION function;
		LPVOID arg;
		size_t key;
		BOOL bIsProcessing;
		// m_bCheckJobKey �� Enable �Ǿ� ���� ��쿡�� �̿�
		// Thread �� Job �� ó���ϴ� ���� �߰� ó�� ��û�� �߻��Ͽ��� ��� ����
		int nReqCallCount;
	} function_t;

	ThreadPool(void);
	~ThreadPool(void);

	BOOL Initialize(LPCSTR lpstrName = NULL, int nNumberOfThread = 0, int nPriority = THREAD_PRIORITY_NORMAL);
	void Destroy();
	void AddWorkerFunction(WORKERFUNCTION worker, LPVOID arg, size_t nKey = 0);
	void ClearAllWorkerFunction();
	int GetJobCount();

	BOOL IsInitialize();

	int GetNumberOfThread() const { return m_nNumberOfThread; }

	// Job Queue �� Unique �� Job �� �߰��� �� �ֵ��� �� ������ ���� (Job Key �̿�)
	void EnableCheckJobKey(BOOL bEnable) { m_bCheckJobKey = bEnable; };
	BOOL IsCheckJobKey() { return m_bCheckJobKey; };

protected:
	static unsigned int WINAPI _WorkerThreadFunction(LPVOID arg);
	BOOL GetWorkerFunction(function_t &func);
	BOOL PopWorkerFunction(LPGUID pGuid);

	BOOL m_bRun;
	int m_nNumberOfThread;
	worker_t *m_pWorker;
	LONG m_lWorker;
	BOOL m_bCheckJobKey;

	CRITICAL_SECTION m_cs;
	list<function_t> m_lstFunctions;
};

class CSafeThreadPool
{
public:
	typedef struct _function_t
	{
		WORKERFUNCTION function;
		LPVOID arg;
		size_t key;
	} function_t;

	typedef struct _worker_t
	{
		CSafeThreadPool *pThreadPool;
		int nIndex;
		//BOOL bThreadRun;
		LONG bThreadRun;
		HANDLE hEvent;
		HANDLE hThread;
		CRITICAL_SECTION cs;
		CAtlList<function_t> qFunctions;
		DWORD dwThreadId;
	} worker_t;

	CSafeThreadPool();
	CSafeThreadPool(size_t nMaxQueueSize, BOOL bAutoCreate);
	CSafeThreadPool(BOOL bAutoCreate);
	CSafeThreadPool(size_t nMaxQueueSize);
	~CSafeThreadPool(void);

	BOOL Initialize(LPCSTR lpstrName = NULL, size_t nNumberOfThread = 0, int nPriority = THREAD_PRIORITY_NORMAL);
	void Destroy();
	BOOL AddWorkerFunction(WORKERFUNCTION worker, LPVOID arg, size_t nID, size_t nKey);
	void ClearAllWorkerFunction();
	void ClearWorkerFunction(size_t nID, size_t nKey);
	BOOL IsInitialize();
	size_t GetNumberOfThread() const { return m_nNumberOfThread; }
	void SetWorkerName(UINT nIndex, LPCSTR lpstrName);
	static int GetDefaultSize();

protected:
	void OnConstructor(size_t nMaxQueueSize, BOOL bAutoCreate);
	static unsigned int WINAPI _WorkerThreadFunction(LPVOID arg);
	BOOL GetWorkerFunction(function_t &func);
	void CreateWorker(int nWorkerIndex);

	BOOL m_bRun;
	size_t m_nNumberOfThread;
	worker_t *m_pWorker;
	LONG m_lWorker;
	size_t m_nMaxQueueSize;
	BOOL m_bAutoCreate;
	CRITICAL_SECTION m_csPool;
	char m_szPoolName[128];
	int m_nPriority;
};
