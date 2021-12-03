
#include "logmain.h"
#include "_logmodule.h"


extern "C" 
{
	EXPORT void RegisterInterface(IInterfaceMgr* interface_mgr)
	{
		LogDLL::Register(interface_mgr);
	}
};

LOG_API void LogDLL::Register(IInterfaceMgr* interface_mgr)
{
	interface_mgr->RegisterModule(LOG_MODULE, new LogModule());
}
