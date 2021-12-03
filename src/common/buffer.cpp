

#include "buffer.h"
#include <memory.h>
#include <algorithm>

Buffer::Buffer()
	:m_buffer(0), m_size(0)
{
}
Buffer::~Buffer()
{
	release();
}

Buffer::Buffer(const Buffer& val)
	:m_buffer(0), m_size(0)
{
	resize(val.size());
	memcpy(m_buffer, val.m_buffer, m_size);
}
Buffer& Buffer::operator=(const Buffer& val)
{
	if ( this == &val ) return *this;
	resize(val.size());
	memcpy(m_buffer, val.m_buffer, m_size);
	return *this;
}

void Buffer::resize(unsigned long size)
{
	if ( size != m_size )
	{
		
		char* new_buffer = new char[size];
		if ( m_buffer != 0 )
		{
			memcpy(new_buffer, m_buffer, std::min(size, m_size) );
			delete [] m_buffer;
		}	
		m_buffer = new_buffer;
		m_size = size;
	}

	
	
}
void Buffer::grow_to(unsigned long size)
{
	if ( size > m_size )
	{
		resize(size);
	}
}

void Buffer::release()
{
	if ( m_buffer != 0 )
	{
		delete [] m_buffer;
	}
	m_buffer = 0;
	m_size = 0;
}
unsigned long Buffer::size()const
{
	return m_size;
}
	
char* Buffer::ptr()
{
	return m_buffer;
}
const char* Buffer::ptr()const
{
	return m_buffer;
}
