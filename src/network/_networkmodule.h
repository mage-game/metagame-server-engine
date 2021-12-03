

#pragma once

#ifndef NETWORKMODULE_H
#define NETWORKMODULE_H

#include <list>
#include "inetworkmodule.h"
#include "basicnetwork/def.h"
#include "utility/logagent.h"

class Network;
class NetworkModule;
class GsNetCallback;

class NetworkModule : public INetworkModule
{
	friend class GsNetCallback;
public:
	NetworkModule(const char *module_name=0);
	~NetworkModule();
	virtual int Init();
	virtual int Start();
	virtual int Update();
	virtual int Stop();
	virtual int Release();
	virtual void Free();

	virtual void RegisterCallback(IEngineNetCallback * callback);

	virtual bool Listen(Port port, int backlog, NetID *netid_out=0, const char *ip_bind=0);
	virtual bool Connect(IP ip, Port port, NetID *netid_out, unsigned long time_out=3000);
	virtual bool Connect(const char *ip, Port port, NetID *netid_out, unsigned long time_out=3000);
	virtual bool ConnectAsyn(const char *ip, Port port, int *handle, unsigned long time_out=3000);
	virtual bool ConnectAsyn(IP ip, Port port, int *handle, unsigned long time_out=3000);
	virtual bool Send(NetID netid, const char *data, unsigned int len);
	virtual void Disconnect(NetID id);

protected:
	typedef std::list<IEngineNetCallback*> CallbackList;
	CallbackList m_callback_list;

private:
	NetworkModule(const NetworkModule&);
	NetworkModule& operator=(const NetworkModule&);

	NetworkConfig m_network_config;
	Network *m_network;
	GsNetCallback *m_network_callback;

	char m_module_name[MAX_MODULE_NAME_LEN];
};

#endif
