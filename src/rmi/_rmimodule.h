

#pragma once

#ifndef RMIMODULE_H
#define RMIMODULE_H

#include <map>
#include <queue>
#include <vector>
#include <string>

#include "common/platform/thread/mutex.h"
#include "common/platform/thread/thread.h"
#include "common/platform/thread/event.h"

#include "irmimodule.h"
#include "inetworkmodule.h"
#include "utility/logagent.h"
#include "rmibase.h"

class MsgMem;

class RMINetworkCallback;
namespace rmi
{
	class RMISession;
}
class IClockModule;


class RMIModule : public IRMIModule
{
	friend class RMINetworkCallback;
public:
	RMIModule(const char *network_module=0);
	~RMIModule();
	virtual int Init();
	virtual int Start();
	virtual int Update();
	virtual int Stop();
	virtual int Release();
	virtual void Free();

	virtual bool StartServer(Port listen_port);

	virtual bool Register(rmi::RMIObject *obj);
	//virtual bool UnRegister(std::string module);
	
	// CreateSession 和 CloseSession 必须保证在StartAsyncThread(n)和StartAsyncThread(0)之外调用
	// RMIModule 对SessionList不做线程安全处理
	virtual rmi::Session CreateSession(const char *ip, Port port);
	virtual void CloseSession(const rmi::Session &session);

	virtual bool Call(const rmi::Session &session, const char *module, const char *method, const TLVSerializer &in_stream, 
						rmi::RMIBackObject *backobj, bool remotec_syn=true, unsigned long timeout=10000);
	virtual void StartAsyncThread(int threadnum);
protected:
	
private:
	RMIModule(const RMIModule&);
	RMIModule& operator=(const RMIModule&);

	LogAgent m_log;
	INetworkModule *m_network;
	IClockModule *m_clock;
	char m_network_module_name[MAX_MODULE_NAME_LEN];
	Port m_listen_port;
	RMINetworkCallback *m_network_callback;

protected:
	bool OnRecvPackage(rmi::RMISession *session, const void *data, unsigned int length);
	bool RecvRMICallSync(rmi::RMISession *session, TLVUnserializer &s);
	bool RecvRMICallASync(rmi::RMISession *session, TLVUnserializer &s);
	bool RecvRMICallHelper(rmi::RMISession *session, TLVUnserializer &s, char *&out_param_buff, unsigned int &out_param_buff_len);

	bool RecvRMIReturn(rmi::RMISession *session, TLVUnserializer &s);

	void FreeObject();
	// 其他模块在启动时注册后，此处不加锁
	typedef std::map<std::string, rmi::RMIObject*> ObjectList;
	ObjectList		m_object_list;

	typedef std::map<NetID, rmi::RMISession*> SessionList;
	SessionList		m_session_list;
	Mutex			m_session_list_lock;

	char			*m_out_param_buff;	// 用于主线，而各个支线的buff建立在支线线程上
	unsigned int	m_out_param_buff_len;

protected:
	// 启动异步调用线程相关
	struct ThreadUnit 
	{
		bool		run;
		RMIModule	*pThis;
		Thread		thread;
	};
	bool			m_start_asyn_call;
	typedef std::vector<ThreadUnit*>	AsynCallThread;
	AsynCallThread	m_asyn_thread;
	static DWORD AsynThread(void *p);
	void AsynThreadHelper(bool *run);

	struct AsynCallUnit
	{
		NetID		netid;
		rmi::RMISession	*session;
		MsgMem		*in_param_buff;
		unsigned int	in_param_buff_length;
	};
	typedef std::queue<AsynCallUnit>	AsynCallQueue;	// 此处用普通queue然后加Mutex和Event就足够了
	AsynCallQueue	m_asyn_call_queue;
	Mutex			m_asyn_call_queue_lock;
	Event			m_asyn_call_queue_event;

protected:
	unsigned long m_erase_timeout_interval;
	unsigned long m_last_erase_time;
	void EraseTimeout();
};

#endif
