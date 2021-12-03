
#include "network.h"
#include "job.h"
#include "listenhandler.h"
#include "tcphandler.h"
#include <memory.h>

#include "common/memory/memmonitor.h"

class DefaultCallback:public INetworkCallback
{
public:
	virtual ~DefaultCallback(){}
	virtual void OnAccept(Port listen_port, NetID netid, IP ip, Port port){}
	virtual void OnRecv(NetID netid, const char *data, unsigned int length){}
	virtual void OnDisconnect(NetID netid){}
	virtual void OnConnect(bool result, int handle, NetID netid, IP ip, Port port){}
};


Network::Network(NetworkConfig config):m_config(config), m_job_queue(config.job_queue_length), m_basicnetwork(&m_job_queue), 
m_cur_handle_id(0), m_connect_req_queue(256), m_connect_result_queue(256), m_connect_thread_run(false)
, m_update_count(0)
{
	PISocket::Startup();
	m_default_callback = new DefaultCallback();
	m_callback = m_default_callback;
}

Network::~Network()
{
	PISocket::Cleanup();
}

void Network::RegisterCallback(INetworkCallback * callback)
{
	if (callback == 0)
	{
		m_callback = m_default_callback;
	}
	else
	{
		m_callback = callback;
	}
}

void Network::Start()
{
	m_basicnetwork.Start();
	m_connect_thread_run = true;
	for (int i = 0; i < CONNECT_THREAD_NUM; ++i)
	{
		m_connect_thread[i].Run(ConnectThread, this);
	}
}

void Network::Stop()
{
	m_connect_thread_run = false;
	for (int i = 0; i < CONNECT_THREAD_NUM; ++i)
	{
		m_connect_event.Signal();
	}
	for (int i = 0; i < CONNECT_THREAD_NUM; ++i)
	{
		m_connect_thread[i].Join();
	}
	m_basicnetwork.Stop();
	
	Job *job;
	while (m_job_queue.TryPop(&job, 0))
	{
		delete job;
	}
}

void Network::Update()
{
	Job *job;
	while (m_job_queue.TryPop(&job, 0))
	{
		job->Invoke(m_callback);
		delete job;
	}

	ConnectResult cr;
	while (m_connect_result_queue.TryPop(&cr))
	{
		m_callback->OnConnect(cr.result, cr.handle, cr.netid, cr.ip, cr.port);
	}
	
	//if (++ m_update_count >= 500)
	//{
	//	m_update_count = 0;

	//	char buff[4096] = {0};
	//	memmonitor::PrintDyMemInfo(buff);
	//	printf(buff); printf("\n"); 

	//	m_basicnetwork.Print();

	//	fflush(stdout);
	//}
}

bool Network::Listen(Port port, int backlog, NetID *netid_out, const char *ip_bind)
{
	ListenHandler *listenhandler = new ListenHandler(m_config.max_package_size);
	SOCKET sock = listenhandler->Listen(port, backlog, ip_bind);
	if (sock == SOCKET_ERROR)
	{
		return false;
	}

	NetID netid = m_basicnetwork.Add(listenhandler);

	if (netid_out != 0)
	{
		*netid_out = netid;
	}
	
	return true;
}

bool Network::Connect(const char *ip, Port port, NetID *netid_out, unsigned long time_out)
{
	unsigned long ip_n = inet_addr(ip);
	if (ip_n == INADDR_NONE) return false;

	IP ip_host = ntohl(ip_n);
	return Connect(ip_host, port, netid_out, time_out);
}

bool Network::Connect(IP ip, Port port, NetID *netid_out, unsigned long time_out)
{
	SOCKET connect_sock = PISocket::Socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (connect_sock == SOCKET_ERROR)
	{
		PISocket::Close(connect_sock);
		return false;
	}

	//设置为非阻塞
	unsigned long ul = 1;
	if ( SOCKET_ERROR == PISocket::Ioctl(connect_sock, FIONBIO, &ul))
	{
		PISocket::Close(connect_sock);
		return false;
	}

	sockaddr_in server_adr;
	memset(&server_adr, 0, sizeof(sockaddr_in));
	server_adr.sin_family = AF_INET;
	server_adr.sin_addr.s_addr = htonl(ip);
	server_adr.sin_port = htons(port);

	PISocket::Connect(connect_sock, (struct sockaddr *)&server_adr );

	//检测可写，超时则返回
	struct timeval tv ;   
	tv.tv_sec = time_out / 1000;   
	tv.tv_usec = (time_out % 1000) * 1000;

	fd_set fdwrite;
	FD_ZERO(&fdwrite);
	FD_SET(connect_sock, &fdwrite);

	int ret = select((int)connect_sock+1, 0, &fdwrite, 0, &tv);

	if (ret > 0 && FD_ISSET(connect_sock, &fdwrite))
	{
		int error = 0;
		int size = sizeof(int);
		if ( 0 == PISocket::GetSockopt(connect_sock, SOL_SOCKET, SO_ERROR, (char *)&error, &size) && error == 0)
		{
			TcpHandler *tcphandler = new TcpHandler(connect_sock, m_config.max_package_size);
			NetID netid = m_basicnetwork.Add(tcphandler);
			if (netid_out != 0)
			{
				*netid_out = netid;
			}
			return true;
		}
	}

	PISocket::Close(connect_sock);
	return  false;
	
}


bool Network::ConnectAsyn(const char *ip, Port port, int *handle, unsigned long time_out)
{
	unsigned long ip_n = inet_addr(ip);
	if (ip_n == INADDR_NONE) return false;

	IP ip_host = ntohl(ip_n);
	return ConnectAsyn(ip_host, port, handle, time_out);
}
bool Network::ConnectAsyn(IP ip, Port port, int *handle, unsigned long time_out)
{
	int cur_handle = m_cur_handle_id++;
	if (handle != 0)
	{
		*handle = cur_handle;
	}

	ConnectStruct cs;
	cs.handle = cur_handle;
	cs.ip = ip;
	cs.port = port;
	cs.timeout = time_out;

	if (m_connect_req_queue.TryPush(cs))
	{
		m_connect_event.Signal();
		return true;
	}

	return false;
}

void Network::Disconnect(NetID id)
{
	m_basicnetwork.Remove(id);
}

DWORD Network::ConnectThread(void * param)
{
	Network *p_this = (Network*)param;
	p_this->ConnectThreadHelper();
	return 0;
}

void Network::ConnectThreadHelper()
{
	while (m_connect_thread_run)
	{
		ConnectStruct cs;
		if (!m_connect_req_queue.TryPop(&cs))
		{
			m_connect_event.Wait(100);
			continue;
		}

		NetID netid;
		bool ret = Connect(cs.ip, cs.port, &netid, cs.timeout);

		ConnectResult cr;
		cr.handle = cs.handle;
		cr.ip = cs.ip;
		cr.netid = netid;
		cr.port = cs.port;
		cr.result = ret;

		m_connect_result_queue.Push(cr);
	}
}


