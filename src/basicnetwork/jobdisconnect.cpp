
#include "def.h"
#include "jobdisconnect.h"

#include "common/memory/mempool.h"
#include "common/platform/thread/mutex.h"

JobDisconect::JobDisconect(NetID netid):m_netid(netid)
{

}

JobDisconect::~JobDisconect()
{

}

void JobDisconect::Invoke(INetworkCallback *callback)
{
	//»Øµ÷
	callback->OnDisconnect(m_netid);
}

namespace jobdisconnectmempool
{
	MemPool g_jobdisconnect_mem_pool(sizeof(JobDisconect), 64, "JobDisconect");
	Mutex g_jobdisconnect_mem_pool_lock;
}


void *JobDisconect::operator new(size_t c)
{
	jobdisconnectmempool::g_jobdisconnect_mem_pool_lock.Lock();
	void *mem = jobdisconnectmempool::g_jobdisconnect_mem_pool.Alloc();
	jobdisconnectmempool::g_jobdisconnect_mem_pool_lock.Unlock();
	return mem;
}

void JobDisconect::operator delete(void *m)
{
	jobdisconnectmempool::g_jobdisconnect_mem_pool_lock.Lock();
	jobdisconnectmempool::g_jobdisconnect_mem_pool.Free(m);
	jobdisconnectmempool::g_jobdisconnect_mem_pool_lock.Unlock();
}
