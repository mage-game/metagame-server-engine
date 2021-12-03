
#ifndef JOB_H
#define JOB_H

class INetworkCallback;
struct NetworkConfig;

class Job
{
public:
	virtual ~Job(){}
	virtual void Invoke(INetworkCallback *callback)=0;
};

#endif
