
#ifndef JOBACCEPT_H
#define JOBACCEPT_H


#include "job.h"
#include "common/platform/socket/pisocket.h"
#include "def.h"

class BasicNetwork;

class JobAccept: public Job
{
public:
	JobAccept(NetID netid, SOCKET sock, Port port, BasicNetwork *network);
	virtual ~JobAccept();
	virtual void Invoke(INetworkCallback *callback);

	void *operator new(size_t c);
	void operator delete(void *m);
protected:
	NetID m_netid;
	SOCKET m_accept_sock;
	Port m_listen_port;
	BasicNetwork *m_basicnetwork;
};



#endif

