
#include "_rmimodule.h"
#include "ilogmodule.h"
#include "iconfigmodule.h"
#include "iclockmodule.h"
#include "inetworkmodule.h"
#include "utility/configpath.h"
#include "rmidef.h"
#include "common/memory/msgmem.h"

#include "rmisession.h"

enum RMICallType
{
	RMI_CALL_TYPE_CALL,
	RMI_CALL_TYPE_RETURN,
};

enum RemoteCallModel
{
	RemoteCallSync,
	RemoteCallAsyn
};


class RMINetworkCallback : public IEngineNetCallback
{
public:
	RMINetworkCallback(RMIModule* rmimodule):m_rmi(rmimodule)
	{

	}
	~RMINetworkCallback(){}

	virtual void OnAccept(Port listen_port, NetID netid, IP ip, Port port)
	{
		if (listen_port != m_rmi->m_listen_port) return;

		m_rmi->m_log.printf(LL_DEBUG, "RMINetworkCallback::ON_ACCEPT %d", netid);

		rmi::RMISession *session = new rmi::RMISession(m_rmi, m_rmi->m_network, netid, true);
		m_rmi->m_session_list_lock.Lock();
		m_rmi->m_session_list[netid] = session;
		m_rmi->m_session_list_lock.Unlock();
	}

	virtual void OnRecv(NetID netid, const char *data, int length)
	{
		RMIModule::SessionList::iterator i = m_rmi->m_session_list.find(netid);		// 此处对m_session_list只读，和有可能修改的地方都在同个线程，所以不用加锁
		if (i != m_rmi->m_session_list.end())
		{
			//m_rmi->m_log.print(LL_DEBUG, "RMINetworkCallback::ON_RECV");

			m_rmi->OnRecvPackage(i->second, data, length);
		}
	}

	virtual void OnDisconnect(NetID netid)
	{
		m_rmi->m_session_list_lock.Lock();
		RMIModule::SessionList::iterator i = m_rmi->m_session_list.find(netid);
		if (i != m_rmi->m_session_list.end())
		{
			m_rmi->m_log.print(LL_WARNING, "RMINetworkCallback::ON_DISCONNECT");
			if (i->second->BeConnected())
			{
				rmi::RMISession *s = i->second;
				m_rmi->m_session_list.erase(i);
				delete s;
			}
			else
			{
				i->second->Disconnect();
			}
		}
		m_rmi->m_session_list_lock.Unlock();
	}

	virtual void OnConnect(bool result, int handle, NetID netid, IP ip, Port port)
	{
	}

private:
	RMIModule *m_rmi;
};

#define DEFAULT_RMI_OUT_BUFF_LENGTH	(1024 * 16)

RMIModule::RMIModule(const char *network_module):m_network(0), m_clock(0), m_listen_port(-1), m_start_asyn_call(false), m_erase_timeout_interval(2000), m_last_erase_time(0)
{
	if (network_module == 0 || (*network_module) == 0)
	{
		network_module = NETWORK_MODULE;
	}
	memset(m_network_module_name, 0, MAX_MODULE_NAME_LEN);
	strcpy(m_network_module_name, network_module);

 	m_network_callback = new RMINetworkCallback(this);

	m_out_param_buff_len = DEFAULT_RMI_OUT_BUFF_LENGTH;
	m_out_param_buff = new char[m_out_param_buff_len];
}

RMIModule::~RMIModule()
{
	delete m_network_callback;

	delete []m_out_param_buff;
}

void RMIModule::Free()
{
	delete this;
}

int RMIModule::Init()
{
	EXPECT_ON_INIT(CONFIG_MODULE)
	EXPECT_ON_INIT(LOG_MODULE)


	IConfigModule* config = dynamic_cast<IConfigModule*>(Interface()->QueryModule(CONFIG_MODULE));
	bool write_log = true;
	
	
	if (config != 0)
	{
		config->SyncValue(ROOT/RMI_MODULE/"WriteLog", &write_log, write_log);
	}

	if(write_log)
	{
		m_log.SetLogModule(dynamic_cast<ILogModule*>(Interface()->QueryModule(LOG_MODULE)));
		m_log.SetCatagory(RMI_MODULE);
	}

	return IModule::Succeed;
}

int RMIModule::Start()
{
	EXPECT_ON_INIT(CONFIG_MODULE)
	DEPEND_ON_START(CLOCK_MODULE)
	DEPEND_ON_START(m_network_module_name)

	IConfigModule* config = dynamic_cast<IConfigModule*>(Interface()->QueryModule(CONFIG_MODULE));

	if (config != 0)
	{
		config->SyncValue(ROOT/RMI_MODULE/"EraseTimeoutInterval", &m_erase_timeout_interval, m_erase_timeout_interval);
	}

	m_network = dynamic_cast<INetworkModule*>(Interface()->QueryModule(m_network_module_name));
	m_network->RegisterCallback(m_network_callback);

	m_clock = dynamic_cast<IClockModule*>(Interface()->QueryModule(CLOCK_MODULE));

	m_log.print(LL_MAINTANCE, "RMI Start.");

	config->write();

	return IModule::Succeed;
}

int RMIModule::Update()
{
	EraseTimeout();
	return IModule::Pending;
}

int RMIModule::Stop()
{
	StartAsyncThread(0);

	// 此处已经停止了异步调用，所以不用加锁
	for (SessionList::iterator i = m_session_list.begin(); i != m_session_list.end(); ++i)
	{
		delete i->second;
	}
	m_session_list.clear();

	m_log.print(LL_MAINTANCE, "RMI Stop.");
	return IModule::Succeed;
}

int RMIModule::Release()
{
	m_asyn_call_queue_lock.Lock();
	while (m_asyn_call_queue.size() != 0)
	{
		AsynCallUnit acu = m_asyn_call_queue.front();
		m_asyn_call_queue.pop();
		delete []acu.in_param_buff;
	}
	m_asyn_call_queue_lock.Unlock();
	FreeObject();
	return IModule::Succeed;
}

bool RMIModule::StartServer(Port listen_port)
{
	NetID netid;
	bool ret = m_network->Listen(listen_port, 64, &netid);
	if (ret)
	{
		m_listen_port = listen_port;
	}
	return true;
}

bool RMIModule::Register(rmi::RMIObject *obj)
{
	std::string module = obj->__get_obj_id();
	ObjectList::iterator i = m_object_list.find(module);
	if (i == m_object_list.end())
	{
		m_object_list[module] = obj;
		return true;
	}
	return false;
}

void RMIModule::FreeObject()
{
	for (ObjectList::iterator i = m_object_list.begin(); i != m_object_list.end(); ++i)
	{
		i->second->__free();
	}
	m_object_list.clear();
}


//bool RMIModule::UnRegister(std::string module)
//{
//	ObjectList::iterator i = m_object_list.find(module);
//	if (i == m_object_list.end())
//	{
//		return false;
//	}
//	delete i->second;
//	m_object_list.erase(i);
//	return true;
//}

rmi::Session RMIModule::CreateSession(const char *ip, Port port)
{
	NetID netid = -1;
	if (!m_network->Connect(ip, port, &netid, 3000))
	{
		rmi::Session s;
		s.handle = 0;
		s.rmi_module = this;
		s.netid = -1;
		return s;
	}
	rmi::RMISession *session = new rmi::RMISession(this, m_network, netid, false);
	
	m_session_list_lock.Lock();
	m_session_list[netid] = session;
	m_session_list_lock.Unlock();

	rmi::Session s;
	s.handle = session;
	s.rmi_module = this;
	s.netid = netid;
	return s;
}

void RMIModule::CloseSession(const rmi::Session &s)
{
	rmi::RMISession *session = (rmi::RMISession *)s.handle;
	if (session == 0)
	{
		return;
	}
	m_session_list_lock.Lock();
	SessionList::iterator i = m_session_list.find(session->GetNetid());
	if (i != m_session_list.end())
	{
		m_session_list.erase(i);
		delete session;
	}
	m_session_list_lock.Unlock();
}

bool RMIModule::Call(const rmi::Session &s, const char *module, const char *method, const TLVSerializer &in_stream, 
					 rmi::RMIBackObject *backobj, bool remotec_syn, unsigned long timeout)
{
	rmi::RMISession *session = (rmi::RMISession *)s.handle;

	if (session == 0)
	{
		return false;
	}

	rmi::RMICall call;
	call.message_id = session->GetMessageIndex();
	call.module = module;
	call.method = method;
	backobj->__set_id(call.message_id);

	unsigned int len = call.GetSerializeLength(in_stream) + 4; // 2字节用于放置RMI_CALL_TYPE_CALL用,2字节用于放
	MsgMem *buff = new MsgMem[len];

	TLVSerializer serializer;
	serializer.Reset(buff, len);
	serializer.Push((char)RMI_CALL_TYPE_CALL);
	char remote_call_module = (remotec_syn ? RemoteCallSync : RemoteCallAsyn);
	serializer.Push(remote_call_module);
	bool ret = call.Serialize(&serializer, in_stream);

	if(ret)
	{
		ret = session->CallMethod(serializer.Ptr(), serializer.Size(), backobj, m_clock->GetFrameTime(), timeout);
	}
	delete []buff;

	return ret;
}

bool RMIModule::OnRecvPackage(rmi::RMISession *session, const void *data, unsigned int length)
{
	TLVUnserializer s;
	s.Reset(data, length);

	char calltype = -1;
	if(!s.Pop(&calltype))
	{
		m_log.print(LL_WARNING, "OnRecvPackage pop calltype error.");
		return false;
	}
	
	switch (calltype)
	{
	case RMI_CALL_TYPE_CALL:
		{
			char remote_call_module;
			if (!s.Pop(&remote_call_module))
			{
				m_log.print(LL_WARNING, "OnRecvPackage RMI_CALL_TYPE_CALL pop remote_call_module error.");
				return false;
			}
			remote_call_module = (m_start_asyn_call ? remote_call_module : RemoteCallSync);
			switch (remote_call_module)
			{
			case RemoteCallSync:
				RecvRMICallSync(session, s);
				break;
			case RemoteCallAsyn:
				RecvRMICallASync(session, s);
				break;
			}
		}
		break;
	case RMI_CALL_TYPE_RETURN:
		RecvRMIReturn(session, s);
		break;
	default:
		m_log.printf(LL_WARNING, "Unknow package calltype:%d.", (int)calltype);
		break;
	}

	return true;
}

bool RMIModule::RecvRMICallHelper(rmi::RMISession *session, TLVUnserializer &s, char *&out_param_buff, unsigned int &out_param_buff_len)
{
	rmi::RMICall call;
	TLVUnserializer in_param;
	bool ret = call.UnSerialize(s, &in_param);
	if (!ret)
	{
		m_log.print(LL_WARNING, "UnSerialize RMICall format ERROR.");
		return false;
	}

	ObjectList::iterator i = m_object_list.find(call.module);
	if (i == m_object_list.end())
	{
		rmi::RMIReturn rmiret;
		rmiret.message_id = call.message_id;
		rmiret.call_result = rmi::DispatchObjectNotExist;
		TLVSerializer out_param;
		unsigned int retbufflen = rmiret.GetSerializeLength(out_param) + 2;

		MsgMem *retbuff = new MsgMem[retbufflen];
		TLVSerializer s;
		s.Reset(retbuff, retbufflen);
		s.Push((char)RMI_CALL_TYPE_RETURN);
		bool ret = rmiret.Serialize(&s, out_param);

		ret = session->Return(s.Ptr(), s.Size());

		delete []retbuff;

		return false;
	}

	rmi::RMIObject *obj = i->second;

	for (;;)
	{
		TLVUnserializer in_param_tmp = in_param;

		TLVSerializer out_param;
		out_param.Reset(out_param_buff, out_param_buff_len);

		int result = obj->__dispatch(call.method, in_param_tmp, &out_param);

		switch (result)
		{
		case rmi::DispatchOK:
		case rmi::DispatchMethodNotExist:
		case rmi::DispatchParamError:
			{
				rmi::RMIReturn rmiret;
				rmiret.message_id = call.message_id;
				rmiret.call_result = result;
				unsigned int retbufflen = rmiret.GetSerializeLength(out_param) + 2;

				MsgMem *retbuff = new MsgMem[retbufflen];
				TLVSerializer s;
				s.Reset(retbuff, retbufflen);
				s.Push((char)RMI_CALL_TYPE_RETURN);
				bool ret = rmiret.Serialize(&s, out_param);

				ret = session->Return(s.Ptr(), s.Size());

				delete []retbuff;
			}
			break;
		case rmi::DispatchOutParamBuffTooShort:
			delete []out_param_buff;
			out_param_buff_len *= 2;
			out_param_buff = new char[out_param_buff_len];
			break;
		default:
			break;
		}

		if (result == rmi::DispatchOutParamBuffTooShort)
		{
			continue;
		}
		break;
	}

	return true;
}

bool RMIModule::RecvRMICallSync(rmi::RMISession *session, TLVUnserializer &s)
{
	return RecvRMICallHelper(session, s, m_out_param_buff, m_out_param_buff_len);
}

bool RMIModule::RecvRMICallASync(rmi::RMISession *session, TLVUnserializer &s)
{
	AsynCallUnit acu;
	acu.netid = session->GetNetid();
	acu.session = session;
	
	acu.in_param_buff_length = s.Size() - ((const char*)s.CurPtr() - (const char*)s.Ptr());
	acu.in_param_buff = new MsgMem[acu.in_param_buff_length];
	memcpy(acu.in_param_buff, s.CurPtr(), acu.in_param_buff_length);
	
	m_asyn_call_queue_lock.Lock();
	m_asyn_call_queue.push(acu);
	m_asyn_call_queue_lock.Unlock();

	m_asyn_call_queue_event.Signal();
	return true;
}

bool RMIModule::RecvRMIReturn(rmi::RMISession *session, TLVUnserializer &s)
{
	rmi::RMIReturn rmiret;
	TLVUnserializer out_param;
	bool ret = rmiret.UnSerialize(s, &out_param);
	if (!ret)
	{
		m_log.print(LL_WARNING, "UnSerialize RMIReturn format ERROR.");
		return false;
	}

	int reti = session->OnRMIReturn(rmiret.message_id, rmiret.call_result, out_param);
	if (reti != rmi::RMISession::RR_SUC)
	{
		switch (reti)
		{
		case rmi::RMISession::RR_MSGID_NOT_EXIST:
			m_log.print(LL_WARNING, "RecvRMIReturn OnRMIReturn RR_MSGID_NOT_EXIST");
			break;
		case rmi::RMISession::RR_RESPONSE_ERROR:
			m_log.print(LL_WARNING, "RecvRMIReturn OnRMIReturn RR_RESPONSE_ERROR");
			break;
		}
		ret = false;
	}
	return ret;
}

void RMIModule::StartAsyncThread(int threadnum)
{
	if (threadnum > (int)m_asyn_thread.size())
	{
		int old_size = (int)m_asyn_thread.size();
		m_asyn_thread.resize(threadnum);
		for (int i = old_size; i < threadnum; ++i)
		{
			ThreadUnit *tu = new ThreadUnit;
			tu->run = true;
			tu->pThis = this;
			m_asyn_thread[i] = tu;
			tu->thread.Run(AsynThread, tu);
		}
	}
	else if (threadnum < (int)m_asyn_thread.size())
	{
		int old_size = (int)m_asyn_thread.size();
		for (int i = threadnum; i < old_size; ++i)
		{
			m_asyn_thread[i]->run = false;
			m_asyn_thread[i]->thread.Join();
			delete m_asyn_thread[i];
		}
		m_asyn_thread.resize(threadnum);
	}

	m_start_asyn_call = ((threadnum == 0) ? false : true);

}

DWORD RMIModule::AsynThread(void *p)
{
	ThreadUnit *threadUnit = (ThreadUnit*)p;
	threadUnit->pThis->AsynThreadHelper(&threadUnit->run);
	return 0;
}

void RMIModule::AsynThreadHelper(bool *run)
{
	unsigned int out_param_buff_length = DEFAULT_RMI_OUT_BUFF_LENGTH;
	char *out_param_buff = new char[out_param_buff_length];

	while (*run)
	{
		AsynCallUnit acu;
		bool go_ahead = false;
		if (m_asyn_call_queue.size() != 0)
		{
			m_asyn_call_queue_lock.Lock();
			if (m_asyn_call_queue.size() != 0)
			{
				acu = m_asyn_call_queue.front();
				m_asyn_call_queue.pop();
				go_ahead = true;
			}
			m_asyn_call_queue_lock.Unlock();
		}
		if (!go_ahead)
		{
			m_asyn_call_queue_event.Wait(10);
			continue;
		}

		m_session_list_lock.Lock();
		SessionList::const_iterator i = m_session_list.find(acu.netid);	// 此处必须为const_iterator，保证不修改
																		// 原因是主线程对没有修改m_session_list的地方都不加锁
		if (i == m_session_list.end() || i->second != acu.session)
		{
			delete []acu.in_param_buff;
			m_session_list_lock.Unlock();
			continue;
		}
		m_session_list_lock.Unlock();

		TLVUnserializer s;
		s.Reset(acu.in_param_buff, acu.in_param_buff_length);

		RecvRMICallHelper(acu.session, s, out_param_buff, out_param_buff_length);
		delete []acu.in_param_buff;
	}
	delete []out_param_buff;
}

void RMIModule::EraseTimeout()
{
	unsigned long now = m_clock->GetFrameTime();

	if (now - m_last_erase_time > m_erase_timeout_interval)
	{
		// 此处不对m_session_list本身进行修改，和有可能修改的地方都在同个线程，所以不用加锁
		for (SessionList::iterator i = m_session_list.begin(); i != m_session_list.end(); ++i)
		{
			i->second->InvokeTimeout(now);
		}

		m_last_erase_time = now;
	}
}

