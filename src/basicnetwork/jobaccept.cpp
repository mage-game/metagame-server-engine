
#include "jobaccept.h"
#include "tcphandler.h"
#include "basicnetwork.h"

#include "common/memory/mempool.h"

JobAccept::JobAccept(NetID netid, SOCKET sock, Port port, BasicNetwork *network):m_netid(netid), m_accept_sock(sock), m_listen_port(port), m_basicnetwork(network)
{

}

JobAccept::~JobAccept()
{

}

void JobAccept::Invoke(INetworkCallback *callback)
{
	sockaddr_in addr;
	PISocket::PeerName(m_accept_sock, &addr);
	IP ip = ntohl(addr.sin_addr.s_addr);
	Port port = ntohs(addr.sin_port);
	//char *ip_str = inet_ntoa(addr.sin_addr);

	callback->OnAccept(m_listen_port, m_netid, ip, port);
}

namespace jobacceptmempool
{
	MemPool g_jobaccept_mem_pool(sizeof(JobAccept), 64, "JobAccept");
	Mutex g_jobaccept_mem_pool_lock;
}

void *JobAccept::operator new(size_t c)
{
	jobacceptmempool::g_jobaccept_mem_pool_lock.Lock();
	void *mem = jobacceptmempool::g_jobaccept_mem_pool.Alloc();
	jobacceptmempool::g_jobaccept_mem_pool_lock.Unlock();
	return mem;
}

void JobAccept::operator delete(void *m)
{
	jobacceptmempool::g_jobaccept_mem_pool_lock.Lock();
	jobacceptmempool::g_jobaccept_mem_pool.Free(m);
	jobacceptmempool::g_jobaccept_mem_pool_lock.Unlock();
}
