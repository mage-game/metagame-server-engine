
#include <string>
#include <iostream>

#include "iconfigmodule.h"
#include "ilogmodule.h"
#include "utility/configpath.h"
#include "_stdcommand.h"

#include "common/platform/thread/thread.h"
#include "common/platform/thread/threadqueue.h"

namespace stdcommandglobal
{
	typedef _ThreadQueue<std::string> CommandQueue;
	CommandQueue g_command_queue(10);

	Thread	g_std_thread;
	bool	g_std_thread_run = false;

	static DWORD StdCommandThread(void *p)
	{
		//StdCommandModule *pthis = (StdCommandModule*)p;

		while (g_std_thread_run)
		{
			std::string cmd;
			std::getline(std::cin, cmd);

			g_command_queue.TryPush(cmd);
		}
		return 0;
	}
}

StdCommandModule::StdCommandModule()
{

}

StdCommandModule::~StdCommandModule()
{

}


int StdCommandModule::Init()
{
	EXPECT_ON_INIT(CONFIG_MODULE);
	EXPECT_ON_INIT(LOG_MODULE);

	IConfigModule* config = dynamic_cast<IConfigModule*>(Interface()->QueryModule(CONFIG_MODULE));

	bool write_log = true;
	if (config != 0)
	{
		config->SyncValue(ROOT/STD_COMMAND_MODULE/"WriteLog", &write_log, true);
	}

	if (write_log)
	{
		m_log.SetLogModule(dynamic_cast<ILogModule*>(Interface()->QueryModule(LOG_MODULE)));
		m_log.SetCatagory(STD_COMMAND_MODULE);
	}

	return IModule::Succeed;
}

int StdCommandModule::Start()
{
	stdcommandglobal::g_std_thread_run = true;
	if (stdcommandglobal::g_std_thread.Run(stdcommandglobal::StdCommandThread, this))
	{
		return IModule::Succeed;
	}
	else
	{
		return IModule::Fail;
	}
}

int StdCommandModule::Update()
{
	std::string cmd;
	while(stdcommandglobal::g_command_queue.TryPop(&cmd))
	{
		//m_log.printf(LL_DEBUG, "cmd:%s", cmd.c_str());
		for (CallbackList::iterator i = m_callback_list.begin(); i != m_callback_list.end(); ++i)
		{
			(*i)->OnCmd(cmd.c_str());
		}
	}
	return IModule::Pending;
}

int StdCommandModule::Stop()
{
	stdcommandglobal::g_std_thread_run = false;
	stdcommandglobal::g_std_thread.Detach();
	return IModule::Succeed;
}

int StdCommandModule::Release()
{
	return IModule::Succeed;
}


void StdCommandModule::Free()
{
	delete this;
}

void StdCommandModule::RegisterCallback(ICmdCallback *callback)
{
	m_callback_list.push_back(callback);
}



