
#pragma once

#ifndef STDCOMMAND_H
#define STDCOMMAND_H

#include <vector>

#include "istdcommand.h"
#include "utility/logagent.h"

class StdCommandModule : public IStdCommandModule
{
public:
	StdCommandModule();
	
	virtual int Init();
	virtual int Start();
	virtual int Update();
	virtual int Stop();
	virtual int Release();

	virtual void Free();

	virtual void RegisterCallback(ICmdCallback *callback);

private:
	virtual ~StdCommandModule();
	LogAgent m_log;

	typedef std::vector<ICmdCallback*> CallbackList;
	CallbackList m_callback_list;
};

#endif
