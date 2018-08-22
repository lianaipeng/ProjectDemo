/*******************************************************************************
* ��Ȩ���У�	�����в���Ƽ����޹�˾(C) 2013
* �ļ����ƣ�	Thread.h
*
* ����ժҪ�� 	�̶߳������
*
--------------------------------------------------------------------------------
* �汾		�޸�ʱ��		�޸���		�޸�����
* V1.0		2013/07/05		���廪		����
*******************************************************************************/
#ifndef _BroadvTool_Thread_H_
#define _BroadvTool_Thread_H_

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN			// �ų��� winsock �İ���
#include <windows.h>
#include <process.h> 
#pragma warning(disable : 4996)

#else
#include <pthread.h>
#endif

namespace BroadvTool
{

////////////////////////////// Mutex ////////////////////////////////////////////
class Mutex
{
public:
	Mutex(void);
	~Mutex(void);

	void Lock();
	void Unlock();

private:
#ifdef _WIN32
	//HANDLE m_hMutex;
	CRITICAL_SECTION m_hMutex;			// �ٽ����Ȼ���������
#else
	mutable pthread_mutex_t m_hMutex;
#endif

private:
	Mutex(const Mutex &);
	const Mutex& operator=(const Mutex &);
};

#ifndef _WIN32
class CRWLock
{
public:
    CRWLock();
    virtual ~CRWLock();
    void rlock();
    void wlock();
    void unlock();
    bool try_rlock();
    bool try_wlock();

private:
    pthread_rwlock_t m_lock;
};

class CAutoRWLock
{
public:
	CAutoRWLock(CRWLock* pLock, bool bRLock = true);
	virtual ~CAutoRWLock();
private:
	CRWLock* m_pLock;
};
#endif

///////////////////////////// AutoLock /////////////////////////////////////////////
class AutoLock
{
public:
	AutoLock( Mutex & mx) : m_Mutex(mx) { m_Mutex.Lock(); }
	~AutoLock()	{ m_Mutex.Unlock();	}
private:
	Mutex & m_Mutex;
};

///////////////////////////// Thread /////////////////////////////////////////////
class Thread
{
public:
	Thread(void);

	/** don't stop the thread, make sure call Join before */
	virtual ~Thread(void);
	
	/** start a thread, return 0 success , -1 on error */
	int Run();

	/** wait until the thread exit */
	void Join();

	/** wait until the thread exit */
	static void Join(Thread * p);

	/** ms - millisecond */
	static void Sleep(int ms);

	/** thread main loop */
	virtual void Routine() = 0;

private:
#ifdef _WIN32
	HANDLE m_hThread;
#else
	pthread_t m_hThread;
#endif

private:
	Thread(const Thread& );
	Thread& operator=(const Thread& );
};

};	// namespace BroadvTool

#endif	// _BroadvTool_Thread_H_

