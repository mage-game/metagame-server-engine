
#pragma once

#ifndef NULLOG_H
#define NULLOG_H

#include "ilogimpl.h"

class NulLog : public ILogImpl
{
public:
	virtual void Write(const char* str, int length){};
	virtual estring GetTarget()
	{
		return "0";
	}
};


#endif
