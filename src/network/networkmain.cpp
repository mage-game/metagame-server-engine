
#include "networkmain.h"
#include "_networkmodule.h"



extern "C" 
{
	NETWORK_API void RegisterInterface(IInterfaceMgr* interface_mgr)
	{
		NetworkDLL::Register(interface_mgr);
	}
};

NETWORK_API void NetworkDLL::Register(IInterfaceMgr* interface_mgr, const char *module_name)
{
	if (module_name == 0 || (*module_name) == '\0')
	{
		module_name = NETWORK_MODULE;
	}
	interface_mgr->RegisterModule(module_name, new NetworkModule(module_name));
}
