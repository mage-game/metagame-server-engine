
#include "rmisession.h"
#include "rmibase.h"

namespace rmi
{

RMISession::RMISession(IRMIModule *rmi_module, INetworkModule *network, NetID netid, bool be_connected)
:m_netid(netid), m_network(network), m_rmi_module(rmi_module), m_message_index(0), m_be_connected(be_connected)
{
}

RMISession::~RMISession()
{
	m_network->Disconnect(m_netid);
	FreeBackList();
}

void RMISession::FreeBackList()
{
	for (RMIBackList::iterator i = m_back_list.begin(); i != m_back_list.end(); ++i)
	{
		i->second.back_obj->__free();
	}
	m_back_list.clear();
}

bool RMISession::CallMethod(const void *data, unsigned int length, rmi::RMIBackObject *backobj, unsigned long begin_time, unsigned long timeout)
{
	bool ret = m_network->Send(m_netid, (const char*)data, length);
	if (ret)
	{
		m_back_list[backobj->__get_id()] = BackItem(backobj, begin_time, timeout);
	}
	return ret;
}

bool RMISession::Return(const void *data, unsigned int length)
{
	return m_network->Send(m_netid, (const char*)data, length);
}

int RMISession::OnRMIReturn(int message_id, char call_result, TLVUnserializer &out_param)
{
	RMIBackList::iterator i = m_back_list.find(message_id);
	if (i == m_back_list.end())
	{
		return RR_MSGID_NOT_EXIST;
	}

	int result = RR_SUC;

	switch (call_result)
	{
	case DispatchOK:
		{
			bool ret = i->second.back_obj->__response(out_param);
			if (!ret)
			{
				result = RR_RESPONSE_ERROR;
			}
		}
		break;
	default:
		i->second.back_obj->__exception(call_result);
	}

	i->second.back_obj->__free();
	m_back_list.erase(i);

	return result;
}

void RMISession::InvokeTimeout(unsigned long now)
{
	for (RMIBackList::iterator i = m_back_list.begin(); i != m_back_list.end();)
	{
		if ((now - i->second.begin_time) > i->second.timeout_time)
		{
			i->second.back_obj->__timeout();
			i->second.back_obj->__free();
			m_back_list.erase(i++);
		}
		else
		{
			++i;
		}
	}
}

void RMISession::Disconnect()
{
	m_netid = -1;
	for (RMIBackList::iterator i = m_back_list.begin(); i != m_back_list.end(); ++i)
	{
		i->second.back_obj->__exception(SessionDisconnect);
		i->second.back_obj->__free();
	}
	m_back_list.clear();
}

}
