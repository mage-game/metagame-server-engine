

#include "game.h"
#include "common/platform/system.h"
#include <algorithm>
#include <signal.h>

namespace signalcache
{
	Game *g_game;
	
	void signal_cache(int sig)
	{
		if (g_game != 0)
		{
			g_game->Stop();
			signal(sig, signalcache::signal_cache);
		}
	}
}

Game::Game()
:m_exit(true)
{
	signalcache::g_game = this;
	signal(SIGINT, signalcache::signal_cache);
	//signal(SIGBREAK, signalcache::signal_cache);	// windows下的命令行关闭X
}
Game::~Game()
{
	signalcache::g_game = 0;
	for(ModuleList::iterator it = m_module_list.begin(); it != m_module_list.end(); ++it)
	{
		(*it)->Free();
	}
}


void Game::Loop(const ModuleList& module_list, InterfaceFunc func, ModuleList *succeed_list, bool igrone_exit, int succeed_state, int pending_state)
{
	ModuleList loop_list = module_list;
	while(!loop_list.empty() && (igrone_exit || !m_exit) )
	{
		ModuleList::iterator it = loop_list.begin();
		while(it != loop_list.end())
		{
			(*it)->m_module_state = pending_state;
			switch ( ((**it).*func)() )
			{
			case IModule::Succeed:
				if (succeed_list != NULL)
				{
					succeed_list->push_back(*it);
				}
				(*it)->m_module_state = succeed_state;
				loop_list.erase(it++);
				break;
			case IModule::Pending:
				++it;
				break;
			case IModule::Fail:
				(*it)->m_module_state = IModule::ST_Fail;
				loop_list.erase(it++);
				break;
			}
		}

		if (m_game_config.loop_interval != 0)
		{
			PISleep(m_game_config.loop_interval);
		}
	}
}

/*
	每次Init，Start等过程，最终将保证，最先初始化完成的，最先进入列表。
	那么在释放时，保证以相反的顺序释放和析构即可。
	暂时没有解决循环依赖的安全释放
*/
void Game::Run(const GameConfig& config)
{
	m_game_config = config;
	m_exit = false;
	m_inited_list.clear();
	m_started_list.clear();

	Loop(m_module_list, &IModule::Init, &m_inited_list, false, IModule::ST_Inited, IModule::ST_Initing);		//Init不成功，直接移除
	Loop(m_inited_list, &IModule::Start, &m_started_list, false, IModule::ST_Started, IModule::ST_Starting);	//Start不成功，放入Release列表
	Loop(m_started_list, &IModule::Update, 0, false, IModule::ST_Updated, IModule::ST_Updating);	

	std::reverse(m_started_list.begin(), m_started_list.end());	//先Start的Module必须后Stop		
	Loop(m_started_list, &IModule::Stop, 0, true, IModule::ST_Stopped, IModule::ST_Stopping);
	std::reverse(m_inited_list.begin(), m_inited_list.end());
	Loop(m_inited_list, &IModule::Release, 0, true, IModule::ST_Released, IModule::ST_Releasing);

	
}

void Game::Stop()
{
	m_exit = true;
}

void Game::GetWorkPath(std::string* path)const
{
	*path = m_game_config.work_path;
}

bool Game::RegisterModule(const char* name, IModule* module)
{
	if ( module == 0 )
	{
		return false;
	}
	if ( m_module_map.find(name) != m_module_map.end() )
	{
		return false;
	}
	m_module_map.insert(std::make_pair(name, module));
	module->m_interface_mgr = this;
	m_module_list.push_back(module);
	
	return true;
}

IModule* Game::QueryModule(const char* name)
{
	ModuleMap::iterator it = m_module_map.find(name);
	if ( it == m_module_map.end() )
	{
		return 0;
	}
	else
	{
		return it->second;
	}
}
