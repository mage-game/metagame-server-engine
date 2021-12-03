
#ifndef JOBRECV_H
#define JOBRECV_H

#include "def.h"
#include "job.h"
#include "common/platform/socket/pisocket.h"

class MsgMem;

class JobRecv :public Job
{
public:
	JobRecv(NetID netid, MsgMem *data, MsgLen len);
	virtual ~JobRecv();
	virtual void Invoke(INetworkCallback *callback);

	void *operator new(size_t c);
	void operator delete(void *m);
protected:
	MsgMem *m_data;
	MsgLen m_length;
	NetID m_netid;
};


#endif


