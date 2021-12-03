
#pragma once

#ifndef GAMEMODULE_H
#define GAMEMODULE_H

#include "igamemodule.h"

class Game;

class GameModule : public IGameModule
{
public:
	GameModule(Game *game_ptr);

	virtual int Init();
	virtual int Start();
	virtual int Update();
	virtual int Stop();
	virtual int Release();

	virtual void Free();

	virtual void StopGame();
	virtual bool GetWorkPath(char* path, unsigned long size)const;

protected:
	Game *m_game;
};

#endif
