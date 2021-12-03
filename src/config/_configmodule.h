

#pragma once

#ifndef CONFIGMODULE_H
#define CONFIGMODULE_H

#include "iconfigmodule.h"
#include "common/config/xmlconfig.h"

class ConfigModule : public IConfigModule
{
public:
	ConfigModule(const char* config_path, bool create_when_error);
	~ConfigModule();

	virtual int Init();
	virtual int Start();
	virtual int Update();
	virtual int Stop();
	virtual int Release();

	virtual void Free();

	virtual bool get_item(const char* path, estring* val)const;
	virtual bool set_item(const char* path, const estring& val);
	virtual bool write();

private:
    XMLConfig m_config;
	estring m_config_path;
	bool m_create_when_error;
	bool m_config_change;
};


#endif
