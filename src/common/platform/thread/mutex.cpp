

#include "mutex.h"


#ifdef __linux__
Mutex::Mutex()
{
	pthread_mutex_init(&m_Lock,NULL);
}
Mutex::~Mutex()
{
	pthread_mutex_destroy(&m_Lock);
}

bool Mutex::Lock()
{
	if ( pthread_mutex_lock(&m_Lock) == 0 ) return true;
	else return false;
}
bool Mutex::TryLock()
{
	if ( pthread_mutex_trylock(&m_Lock) == 0 ) return true;
	else return false;
}
bool Mutex::Unlock()
{
	if ( pthread_mutex_unlock(&m_Lock) == 0 ) return true;
	else return false;
}
#endif

#ifdef WIN32
Mutex::Mutex()
{
	InitializeCriticalSection(&m_CriticalSection);
}
Mutex::~Mutex()
{
	DeleteCriticalSection(&m_CriticalSection);
}

bool Mutex::Lock()
{
	EnterCriticalSection(&m_CriticalSection);
	return true;
}
bool Mutex::TryLock()
{
	if ( TryEnterCriticalSection(&m_CriticalSection) ) return true;
	else return false;
}
bool Mutex::Unlock()
{
	LeaveCriticalSection(&m_CriticalSection);
	return true;
}
#endif

