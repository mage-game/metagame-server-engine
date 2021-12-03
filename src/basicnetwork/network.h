
#ifndef NETWORK_H
#define NETWORK_H

#include "def.h"
#include "basicnetwork.h"
#include "jobqueue.h"
#include "common/platform/thread/event.h"

class INetworkCallback;

class Network
{
public:
	Network(NetworkConfig config=NetworkConfig());
	~Network();

	void RegisterCallback(INetworkCallback * callback);
	void Start();
	void Stop();

	void Update();

	// Listen 和 Connect都只能在Start之前调用
	bool Listen(Port port, int backlog, NetID *netid_out=0, const char *ip_bind=0);
	bool Connect(IP ip, Port port, NetID *netid_out, unsigned long time_out=3000);
	bool Connect(const char *ip, Port port, NetID *netid_out, unsigned long time_out=3000);
	bool ConnectAsyn(const char *ip, Port port, int *handle, unsigned long time_out=3000);
	bool ConnectAsyn(IP ip, Port port, int *handle, unsigned long time_out=3000);
	void Disconnect(NetID id);

	bool Send(NetID netid, const char *data, unsigned int len);
protected:
	NetworkConfig m_config;
	JobQueue m_job_queue;
	BasicNetwork m_basicnetwork;
	INetworkCallback *m_callback;
	INetworkCallback *m_default_callback;

protected:
	// 异步connect相关
	struct ConnectStruct
	{
		int handle;
		IP ip;
		Port port;
		unsigned long timeout;
	};
	typedef _ThreadQueue<ConnectStruct> ConnectReqQueue;
	struct ConnectResult
	{
		int handle;
		IP ip;
		Port port;
		bool result;
		NetID netid;
	};
	typedef	_ThreadQueue<ConnectResult> ConnectResultQueue;

	int m_cur_handle_id;
	ConnectReqQueue m_connect_req_queue;
	ConnectResultQueue m_connect_result_queue;

	static const int CONNECT_THREAD_NUM = 3;
	Thread m_connect_thread[CONNECT_THREAD_NUM];
	Event m_connect_event;
	bool m_connect_thread_run;

	static DWORD ConnectThread(void * param);
	void ConnectThreadHelper();

	int m_update_count;
	
};

// 每秒几十W次
inline bool Network::Send(NetID netid, const char *data, unsigned int len)
{	
	return m_basicnetwork.SendPackage(netid, data, len);
}

#endif
