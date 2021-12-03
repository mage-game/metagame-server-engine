
#pragma once

#ifndef BASICNETWORKHANDLER_H
#define BASICNETWORKHANDLER_H

#include "common/platform/socket/pisocket.h"

class BasicNetwork;

class BasicNetworkHandler
{
public:
	enum HandlerType
	{
		HT_TCP,
		HT_LISTEN,
	};
	BasicNetworkHandler(SOCKET socket, char handle_type):m_basic_network(0), m_socket(socket), m_handle_type(handle_type)
	{

	}
	virtual ~BasicNetworkHandler(){}
	virtual void OnCanRead(){};
	virtual void OnCanWrite(){};
	virtual void OnClose(){};

	void SetBasicNetwork(BasicNetwork *network)
	{
		m_basic_network = network;
	}

	char GetType() { return m_handle_type; }

	SOCKET GetSocket() { return m_socket; }
	void SetNetid(NetID netid) { m_netid = netid; }
	NetID GetNetid() { return m_netid; }

protected:
	BasicNetwork *m_basic_network;
	SOCKET m_socket;

	NetID m_netid;
	const char m_handle_type;
};

#endif
