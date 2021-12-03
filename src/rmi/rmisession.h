
#ifndef RMISESSION_H
#define RMISESSION_H

#include <map>
#include "common/tlvprotocol.h"
#include "inetworkmodule.h"
#include "irmimodule.h"
#include "rmimain.h"

namespace rmi
{
	class RMIBackObject;
}


namespace rmi
{

class RMISession
{
public:
	enum ReturnResult
	{
		RR_SUC,
		RR_MSGID_NOT_EXIST,
		RR_RESPONSE_ERROR,
	};
	RMISession(IRMIModule *rmi_module, INetworkModule *network, NetID netid, bool be_connected);
	~RMISession();

	NetID GetNetid() { return m_netid; }
	IRMIModule *GetRMIModule() { return m_rmi_module; }
	int GetMessageIndex() { return m_message_index++;}
	bool CallMethod(const void *data, unsigned int length, rmi::RMIBackObject *backobj, unsigned long begin_time, unsigned long timeout);
	bool Return(const void *data, unsigned int length);

	int OnRMIReturn(int message_id, char call_result, TLVUnserializer &out_param);
	bool BeConnected() { return m_be_connected; }
	void InvokeTimeout(unsigned long now);
	void Disconnect();
protected:
	void FreeBackList();

	struct BackItem
	{
		BackItem():back_obj(0), begin_time(0), timeout_time(0){}
		BackItem(rmi::RMIBackObject *bo, unsigned long bt, unsigned long to):back_obj(bo), begin_time(bt), timeout_time(to){}
		rmi::RMIBackObject* back_obj;
		unsigned long begin_time;
		unsigned long timeout_time;
	};
	typedef std::map<int, BackItem>	RMIBackList;
	RMIBackList		m_back_list;

	NetID			m_netid;
	INetworkModule	*m_network;
	IRMIModule		*m_rmi_module;
	int				m_message_index;
	bool			m_be_connected;
};


}




#endif

