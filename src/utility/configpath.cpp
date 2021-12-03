
#include "configpath.h"



ConfigPath::ConfigPath()
{
	m_buffer.reserve(256);
}

ConfigPath::ConfigPath(const char* path)
{
	m_buffer.reserve(256);
	m_buffer.append("/");
	m_buffer.append(path);
}

ConfigPath& ConfigPath::operator/(const char* path)
{
	m_buffer.append("/");
	m_buffer.append(path);
	return *this;
}
ConfigPath& ConfigPath::operator/(const estring& path)
{
	m_buffer.append("/");
	m_buffer.append(path);
	return *this;
}
ConfigPath& ConfigPath::operator/(const ConfigPath& path)
{
	m_buffer.append(path.m_buffer);
	return *this;
}

const char* ConfigPath::operator%(const char* path)
{
	m_buffer.append("@");
	m_buffer.append(path);
	return m_buffer.c_str();
}
const char* ConfigPath::operator%(const estring& path)
{
	m_buffer.append("@");
	m_buffer.append(path);
	return m_buffer.c_str();
}

ConfigPath ConfigPath::operator/(const char* path)const
{
	ConfigPath cp = *this;
	cp.m_buffer.append("/");
	cp.m_buffer.append(path);
	return cp;
}
ConfigPath ConfigPath::operator/(const estring& path)const
{
	ConfigPath cp = *this;
	cp.m_buffer.append("/");
	cp.m_buffer.append(path);
	return cp;
}
ConfigPath ConfigPath::operator/(const ConfigPath& path)const
{
	ConfigPath cp = *this;
	cp.m_buffer.append(path.m_buffer);
	return cp;
}

ConfigPath::operator const char*()const
{
	return m_buffer.c_str();
}

const char* ConfigPath::c_str()const
{
	return m_buffer.c_str();
}

const estring& ConfigPath::e_str()const
{
	return m_buffer;
}
