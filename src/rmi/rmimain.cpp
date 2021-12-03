
#include "rmimain.h"
#include "_rmimodule.h"



extern "C" 
{
	RMI_API void RegisterInterface(IInterfaceMgr* interface_mgr)
	{
		RMIDLL::Register(interface_mgr);
	}
};

RMI_API void RMIDLL::Register(IInterfaceMgr* interface_mgr, const char *network_module)
{
	interface_mgr->RegisterModule(RMI_MODULE, new RMIModule(network_module));
}
