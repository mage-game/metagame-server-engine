
#include "_configmodule.h"

ConfigModule::ConfigModule(const char* config_path, bool create_when_error)
:m_config_path(config_path), m_create_when_error(create_when_error), m_config_change(false)
{

}
ConfigModule::~ConfigModule()
{

}

void ConfigModule::Free()
{
	delete this;
}

int ConfigModule::Init()
{
	if (m_config.read(m_config_path.c_str()))
	{
		return IModule::Succeed;
	}
	else if (m_create_when_error && m_config.write(m_config_path.c_str()))
	{
		return IModule::Succeed;
	}
	else
	{
		return IModule::Fail;
	}
}

int ConfigModule::Start()
{
	m_config.write(m_config_path.c_str());
	return IModule::Succeed;
}
int ConfigModule::Update()
{
	return IModule::Succeed;
}

int ConfigModule::Stop()
{
	return IModule::Succeed;
}

int ConfigModule::Release()
{
	return IModule::Succeed;
}

bool ConfigModule::get_item(const char* path, estring* val)const
{
	return m_config.get_item(path, val);
}

bool ConfigModule::set_item(const char* path, const estring& val)
{
	if (m_config.set_item(path, val))
	{
		m_config_change = true;
		return true;
	}
	return false;
}

bool ConfigModule::write()
{
	if (m_config_change)
	{
		return m_config.write(m_config_path.c_str());
	}
	return true;
}
