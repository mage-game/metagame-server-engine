

#include "module/_clockmodule.h"
#include "module/_gamemodule.h"
#include "module/_stdcommand.h"
#include "module/_timermodule.h"
#include "module/_filewrite.h"

namespace gsclock
{
	void Register(IInterfaceMgr* interface_mgr)
	{
		interface_mgr->RegisterModule(CLOCK_MODULE, new ClockModule());
	}
}


namespace gsgame
{
	void Register(IInterfaceMgr* interface_mgr, Game *game)
	{
		interface_mgr->RegisterModule(GAME_MODULE, new GameModule(game));
	}
}

namespace gsstdcommand
{
	void Register(IInterfaceMgr* interface_mgr)
	{
		interface_mgr->RegisterModule(STD_COMMAND_MODULE, new StdCommandModule());
	}
}

namespace gstimer
{
	void Register(IInterfaceMgr* interface_mgr)
	{
		interface_mgr->RegisterModule(TIMER_MODULE, new TimerModule());
	}
}

namespace gsfilewrite
{
	void Register(IInterfaceMgr *interface_mgr)
	{
		interface_mgr->RegisterModule(FILE_WRITE_MODULE, new FileWriteModule());
	}
}
