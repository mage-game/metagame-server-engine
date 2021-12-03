



#include "thread.h"
#include <signal.h>

#ifdef WIN32

Thread::Thread()
:m_Thread(NULL),m_ID(0)
{

}

Thread::~Thread()
{
	Detach();
}



bool Thread::Run(Func func, void *param, unsigned int stack_size)
{
	if ( m_Thread != NULL ) return false;

	ThreadParam *p = new ThreadParam();
	p->func = func;
	p->param = param;

	m_Thread = CreateThread(NULL, stack_size, &Thread::ThreadProc, p, 0, &m_ID);
	if ( m_Thread == NULL ) return false;

	else return true;

}

bool Thread::Join()
{
	if ( WaitForSingleObject(m_Thread, INFINITE ) == WAIT_OBJECT_0 )
	{
		CloseHandle(m_Thread);
		m_Thread = NULL;
		m_ID = 0;
		return true;
	}
	else
	{
		return false;
	}
}

bool Thread::Terminate()
{
	bool ret = (TerminateThread(m_Thread, 0) != 0);
	m_Thread = NULL;
	m_ID = 0;
	return ret;
};

void Thread::Detach()
{
	if ( m_Thread != NULL )
	{
		CloseHandle(m_Thread);
		m_Thread = NULL;
		m_ID = 0;
	}
}

TID Thread::GetThreadID()const
{
	return m_ID;
}

TID Thread::GetCurrentThreadID()
{
	return ::GetCurrentThreadId();
}


DWORD CALLBACK Thread::ThreadProc(void *param)
{
	ThreadParam *p = (ThreadParam*)param;

	DWORD ret = p->func(p->param);

	delete p;
	return ret;
}


#endif




#ifdef __linux__

Thread::Thread()
	:m_PID(0)
{
}
Thread::~Thread()
{
	Detach();
}

bool Thread::Run(Func func, void *param, unsigned int stack_size)
{
	ThreadParam *p = new ThreadParam();
	p->func = func;
	p->param = param;

	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, stack_size);

	if ( 0 != pthread_create(&m_PID, &attr, &Thread::ThreadProc, p) )
	{
		pthread_attr_destroy(&attr);
		return false;
	}
	else 
	{
		pthread_attr_destroy(&attr);
		return true;
	}

}

bool Thread::Join()
{
	if ( m_PID != 0 )
	{
		void *v;
		int ret = pthread_join(m_PID, &v);
		if ( ret == 0 )
		{
			m_PID = 0;
		}

		return ret == 0;
	}
	else
	{
		return false;
	}
}

bool Thread::Terminate()
{
	if ( m_PID != 0 )
	{
		int ret = pthread_kill(m_PID, SIGKILL);
		if ( ret == 0 )
		{
			m_PID = 0;
		}
		return ret == 0;
	}
	else
	{
		return false;
	}
}


void Thread::Detach()
{
	if ( m_PID != 0 )
	{
		pthread_detach(m_PID);
		m_PID = 0;
	}
}


DWORD Thread::ThreadProc(void *param)
{
	ThreadParam *p = (ThreadParam*)param;

	DWORD ret = p->func(p->param);

	delete p;
	return ret;
}

TID Thread::GetThreadID()const
{
	return m_PID;
}

TID Thread::GetCurrentThreadID()
{
	return pthread_self();
}

#endif
