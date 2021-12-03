


#include "semaphore.h"
#include <time.h>

#ifdef WIN32

static const long INIFINIT_COUNT = 999999;

Semaphore::Semaphore(int initcount)
{
	m_Semaphore = CreateSemaphore(NULL, initcount, INIFINIT_COUNT, NULL); 
}
Semaphore::~Semaphore()
{
	CloseHandle(m_Semaphore);
}

int Semaphore::TryDown(unsigned long timeout_millsec)
{
	DWORD ret = WaitForSingleObject(m_Semaphore, timeout_millsec);
	if ( ret == WAIT_OBJECT_0 )
	{
		return Succeed;
	}
	else if ( ret == WAIT_TIMEOUT )
	{
		return Timeout;
	}
	else
	{
		return Fail;
	}
}

void Semaphore::Down()
{
	WaitForSingleObject(m_Semaphore, INFINITE);
}
void Semaphore::Up()
{
	ReleaseSemaphore(m_Semaphore, 1, NULL);
}

#endif


#ifdef __linux__
#include <errno.h>

Semaphore::Semaphore(int initcount)
{
	sem_init(&m_Semaphore, 0, initcount);
}
Semaphore::~Semaphore()
{
	sem_destroy(&m_Semaphore);
}

int Semaphore::TryDown(unsigned long timeout_millsec)
{
	timespec t;
	clock_gettime(CLOCK_REALTIME, &t);
	t.tv_sec += timeout_millsec / 1000;
	t.tv_nsec += (timeout_millsec % 1000) * 1000 * 1000;

	const long BILLION = 1000000000;
	if (t.tv_nsec >= BILLION)
	{
		++ t.tv_sec;
		t.tv_nsec %= BILLION;
	}

	if ( 0 == sem_timedwait(&m_Semaphore, &t) )
	{
		return Succeed;
	}
	
	if ( errno == ETIMEDOUT )
	{
		return Timeout;
	}
	else
	{
		return Fail;
	}
	
}

void Semaphore::Down()
{
	sem_wait(&m_Semaphore);
}
void Semaphore::Up()
{
	sem_post(&m_Semaphore);
}


#endif
