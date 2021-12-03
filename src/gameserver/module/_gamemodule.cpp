



#include "_gamemodule.h"
#include "game.h"
#include <memory.h>
#include <string>

GameModule::GameModule(Game *game_ptr)
:m_game(game_ptr)
{

}

void GameModule::Free()
{
	delete this;
}

int GameModule::Init()
{
	return IModule::Succeed;
}
int GameModule::Start()
{
	return IModule::Succeed;
}
int GameModule::Update()
{
	return IModule::Succeed;
}
int GameModule::Stop()
{
	return IModule::Succeed;
}
int GameModule::Release()
{
	return IModule::Succeed;
}

void GameModule::StopGame()
{
	m_game->Stop();
}

bool GameModule::GetWorkPath(char* path, unsigned long size)const
{
	estring str;
	m_game->GetWorkPath(&str);
	if ( str.size() >= size )
	{
		return false;
	}
	else
	{
		memcpy(path, str.c_str(), str.size()+1);
		return true;
	}
}
