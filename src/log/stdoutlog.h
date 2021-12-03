
#pragma once

#ifndef STDOUTLOG_H
#define STDOUTLOG_H

#include "ilogimpl.h"

class StdoutLog : public ILogImpl
{
public:
	virtual void Write(const char* str, int length)
	{
		printf("%s", str);
		fflush(stdout);
	}
	virtual estring GetTarget()
	{
		return "1";
	}
};


#endif
