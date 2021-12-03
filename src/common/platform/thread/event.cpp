

#include "event.h"


#ifdef __linux__

#include <time.h>
#include <errno.h>

Event::Event()
{
	pthread_cond_init(&m_Cond, NULL);
	pthread_mutex_init(&m_Mutex, NULL);
}

Event::~Event()
{
	pthread_cond_destroy(&m_Cond);
	pthread_mutex_destroy(&m_Mutex);
}

bool Event::Wait()
{
	pthread_mutex_lock(&m_Mutex);
	pthread_cond_wait(&m_Cond, &m_Mutex);
	pthread_mutex_unlock(&m_Mutex);
	return true;
}

int Event::Wait(unsigned long millsecond)
{
	pthread_mutex_lock(&m_Mutex);
	timespec t;
	clock_gettime(CLOCK_REALTIME, &t);
	t.tv_sec += millsecond /1000;
	t.tv_nsec += (millsecond % 1000) * 1000 * 1000;

	const long BILLION = 1000000000;
	if (t.tv_nsec >= BILLION)
	{
		++ t.tv_sec;
		t.tv_nsec %= BILLION;
	}

	int ret = pthread_cond_timedwait(&m_Cond, &m_Mutex, &t);
	pthread_mutex_unlock(&m_Mutex);

	if ( ret == 0 ) return Succeed;

	if ( ret == ETIMEDOUT ) return Timeout;

	return Fail;
}

bool Event::Signal()
{
	pthread_cond_signal(&m_Cond);
	return true;
}

#endif
	
#ifdef WIN32

Event::Event()
{
	m_Event = CreateEvent(NULL, FALSE, FALSE, NULL);
}

Event::~Event()
{
	CloseHandle(m_Event);
}

bool Event::Wait()
{
	WaitForSingleObject(m_Event, INFINITE);
	return true;
}

int Event::Wait(unsigned long millsecond)
{
	int ret = WaitForSingleObject(m_Event, millsecond);

	if (ret == WAIT_OBJECT_0) return Succeed;
	if (ret == WAIT_TIMEOUT) return Timeout;
	
	return Fail;
}

bool Event::Signal()
{
	SetEvent(m_Event);
	return true;
}

#endif

