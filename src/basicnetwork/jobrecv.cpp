
#include <stdlib.h>
#include "jobrecv.h"

#include "common/platform/thread/mutex.h"
#include "common/memory/mempool.h"
#include "common/memory/msgmem.h"


JobRecv::JobRecv(NetID netid, MsgMem *data, MsgLen len):m_data(data), m_length(len), m_netid(netid) 
{

}

JobRecv::~JobRecv()
{
	delete []m_data;
}

void JobRecv::Invoke(INetworkCallback *callback)
{
	// »Øµ÷
	callback->OnRecv(m_netid, (const char *)m_data, (unsigned int)m_length);
}

namespace jobrecvmempool
{
	MemPool g_jobrecv_mem_pool(sizeof(JobRecv), 1024, "JobRecv");
	Mutex g_jobrecv_mem_pool_lock;
}


void *JobRecv::operator new(size_t c)
{
	jobrecvmempool::g_jobrecv_mem_pool_lock.Lock();
	void *mem = jobrecvmempool::g_jobrecv_mem_pool.Alloc();
	jobrecvmempool::g_jobrecv_mem_pool_lock.Unlock();
	return mem;
}

void JobRecv::operator delete(void *m)
{
	jobrecvmempool::g_jobrecv_mem_pool_lock.Lock();
	jobrecvmempool::g_jobrecv_mem_pool.Free(m);
	jobrecvmempool::g_jobrecv_mem_pool_lock.Unlock();
}
