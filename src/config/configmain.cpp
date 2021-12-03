
#include "configmain.h"
#include "_configmodule.h"



extern "C" 
{
	EXPORT void RegisterInterface(IInterfaceMgr* interface_mgr, const char* path="neox-s.xml", bool create_file_when_error=true)
	{
		ConfigDLL::Register(interface_mgr, path, create_file_when_error);
	}
};

CONFIG_API void ConfigDLL::Register(IInterfaceMgr* interface_mgr,const char* path, bool create_file_when_error)
{
	interface_mgr->RegisterModule(CONFIG_MODULE, new ConfigModule(path, create_file_when_error));
}
