
#include "cowbuffer.h"
#include <memory.h>

COWBuffer::COWBuffer()
{
	m_data = new Buffer();
}
COWBuffer::COWBuffer(unsigned long size)
{
	m_data = new Buffer();
	m_data->resize(size);
}
COWBuffer::COWBuffer(const char *data, unsigned long size)
{
	m_data = new Buffer();
	m_data->resize(size);
	memcpy(m_data->ptr(), data, size);
}
COWBuffer::COWBuffer(const COWBuffer& val)
{
	m_data = val.m_data;
}
COWBuffer& COWBuffer::operator=(const COWBuffer& val)
{
	m_data = val.m_data;
	return *this;
}
COWBuffer::~COWBuffer()
{

}

COWBuffer COWBuffer::give_up()
{
	COWBuffer buf(*this);
	m_data = 0;
	return buf;
}


char* COWBuffer::ptr_write()
{
	copy();
	return m_data->ptr();
}

const char* COWBuffer::ptr()const
{
	return m_data->ptr();
}

unsigned long COWBuffer::size()const
{
	return m_data->size();
}

void COWBuffer::resize(unsigned long size)
{
	copy();
	m_data->resize(size);
}

void COWBuffer::copy()
{
	if ( m_data.RefCount() > 1 )
	{
		_SmartPtr<Buffer*> ptr = new Buffer(*m_data);
		m_data = ptr;
	}
}
