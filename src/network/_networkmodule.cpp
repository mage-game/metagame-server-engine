

#include "_networkmodule.h"
#include "basicnetwork/network.h"
#include "iconfigmodule.h"
#include "ilogmodule.h"
#include "utility/configpath.h"

class GsNetCallback: public INetworkCallback
{
public:
	GsNetCallback(NetworkModule *network_module):m_network_module(network_module)
	{
	}

	virtual ~GsNetCallback(){}
	virtual void OnAccept(Port listen_port, NetID netid, IP ip, Port port)
	{
		for (NetworkModule::CallbackList::iterator i = m_network_module->m_callback_list.begin(); i != m_network_module->m_callback_list.end(); ++i)
		{
			(*i)->OnAccept(listen_port, netid, ip, port);
		}
	}

	virtual void OnRecv(NetID netid, const char *data, unsigned int length)
	{
		for (NetworkModule::CallbackList::iterator i = m_network_module->m_callback_list.begin(); i != m_network_module->m_callback_list.end(); ++i)
		{
			(*i)->OnRecv(netid, data, length);
		}
	}

	virtual void OnDisconnect(NetID netid)
	{
		for (NetworkModule::CallbackList::iterator i = m_network_module->m_callback_list.begin(); i != m_network_module->m_callback_list.end(); ++i)
		{
			(*i)->OnDisconnect(netid);
		}
	}

	virtual void OnConnect(bool result, int handle, NetID netid, IP ip, Port port)
	{
		for (NetworkModule::CallbackList::iterator i = m_network_module->m_callback_list.begin(); i != m_network_module->m_callback_list.end(); ++i)
		{
			(*i)->OnConnect(result,	handle, netid, ip, port);
		}
	}

private:
	NetworkModule *m_network_module;
};



NetworkModule::NetworkModule(const char *module_name)
{
	m_network_callback = new GsNetCallback(this);

	if (module_name == 0 || (*module_name) == '\0')
	{
		module_name = NETWORK_MODULE;
	}
	memset(m_module_name, 0, MAX_MODULE_NAME_LEN);
	strcpy(m_module_name, module_name);
}

NetworkModule::~NetworkModule()
{
	delete m_network_callback;
}

void NetworkModule::Free()
{
	delete this;
}

int NetworkModule::Init()
{
	EXPECT_ON_INIT(CONFIG_MODULE)

	IConfigModule* config = dynamic_cast<IConfigModule*>(Interface()->QueryModule(CONFIG_MODULE));
	
	if (config != 0)
	{
		config->SyncValue(ROOT/m_module_name/"JobQueueLength", &m_network_config.job_queue_length, 1024 * 100);	// 1024 * 100 * [4B] = 400KB
		config->SyncValue(ROOT/m_module_name/"MaxPackageSize", &m_network_config.max_package_size, 1024 * 1024);	// 1M
	}

	return IModule::Succeed;
}

int NetworkModule::Start()
{
	m_network = new Network(m_network_config);
	m_network->RegisterCallback(m_network_callback);
	m_network->Start();

	return IModule::Succeed;
}

int NetworkModule::Update()
{
	m_network->Update();
	return IModule::Pending;
}

int NetworkModule::Stop()
{
	m_network->Stop();
	m_network->RegisterCallback(0);

	delete m_network;

	return IModule::Succeed;
}

int NetworkModule::Release()
{
	return IModule::Succeed;
}

void NetworkModule::RegisterCallback(IEngineNetCallback * callback)
{
	m_callback_list.push_back(callback);
}

bool NetworkModule::Listen(Port port, int backlog, NetID *netid_out, const char *ip_bind)
{
	return m_network->Listen(port, backlog, netid_out, ip_bind);
}

bool NetworkModule::Connect(IP ip, Port port, NetID *netid_out, unsigned long time_out)
{
	return m_network->Connect(ip, port, netid_out, time_out);
}

bool NetworkModule::Connect(const char *ip, Port port, NetID *netid_out, unsigned long time_out)
{
	return m_network->Connect(ip, port, netid_out, time_out);
}

bool NetworkModule::ConnectAsyn(const char *ip, Port port, int *handle, unsigned long time_out)
{
	return m_network->ConnectAsyn(ip, port, handle, time_out);
}

bool NetworkModule::ConnectAsyn(IP ip, Port port, int *handle, unsigned long time_out)
{
	return m_network->ConnectAsyn(ip, port, handle, time_out);
}

bool NetworkModule::Send(NetID netid, const char *data, unsigned int len)
{
	return m_network->Send(netid, data, len);
}

void NetworkModule::Disconnect(NetID id)
{
	m_network->Disconnect(id);
}


