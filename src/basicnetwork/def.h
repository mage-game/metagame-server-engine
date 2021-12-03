
#ifndef DEF_H
#define DEF_H

typedef	unsigned int		NetID;
typedef unsigned int		IP;
typedef unsigned short		Port;
typedef int					MsgLen;

class INetworkCallback
{
public:
	virtual ~INetworkCallback(){}
	virtual void OnAccept(Port listen_port, NetID netid, IP ip, Port port)=0;
	virtual void OnRecv(NetID netid, const char *data, unsigned int length)=0;
	virtual void OnDisconnect(NetID netid)=0;
	virtual void OnConnect(bool result, int handle, NetID netid, IP ip, Port port) = 0;
};

struct NetworkConfig
{
	NetworkConfig():job_queue_length(1024 * 16),			// 64K
		max_package_size(1024 * 1024)
	{

	}
	int job_queue_length;
	int max_package_size;
};

#endif

