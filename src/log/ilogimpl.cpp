
#include "ilogimpl.h"
#include "ilogmodule.h"

#include <string.h>
#include <stdlib.h>

#define  RESERVED_LOG_LENGTH	(2 * MAX_LOG_LENGTH)

ILogImpl::ILogImpl():m_buff(0), m_cur_buff_length(0), m_max_buff_length(RESERVED_LOG_LENGTH), m_max_buff_to_write_length(RESERVED_LOG_LENGTH)
{
	m_buff = (char*)malloc(m_max_buff_length);
	m_buff_to_write = (char *)malloc(m_max_buff_to_write_length);
}

ILogImpl::~ILogImpl()
{
	free(m_buff);
}

char *ILogImpl::PrintBegin()
{
	m_buff_mutex.Lock();
	if (m_max_buff_length - m_cur_buff_length < MAX_LOG_LENGTH)
	{
		Resize();
	}
	return m_buff + m_cur_buff_length;
}

void ILogImpl::PrintEnd(int len)
{
	m_cur_buff_length += len;
	m_buff_mutex.Unlock();
}

void ILogImpl::Resize()
{
	int max_size = m_max_buff_length + RESERVED_LOG_LENGTH;
	char *buff = (char*)malloc(max_size);

	memcpy(buff, m_buff, m_cur_buff_length);
	free(m_buff);

	m_buff = buff;
	m_max_buff_length = max_size;
}

bool ILogImpl::WriteBuff(bool force, unsigned long now)
{
	m_buff_mutex.Lock();

	CheckLogSegment();

	if (m_cur_buff_length == 0)
	{
		m_buff_mutex.Unlock();
		return false;
	}

	if (!force && !CheckWrite(now))
	{
		m_buff_mutex.Unlock();
		return false;
	}

	// ½»»»buff
	char *buff_tmp = m_buff;
	m_buff = m_buff_to_write;
	m_buff_to_write = buff_tmp;

	int length_tmp = m_max_buff_length;
	m_max_buff_length = m_max_buff_to_write_length;
	m_max_buff_to_write_length = length_tmp;

	int cur_length = m_cur_buff_length;
	m_cur_buff_length = 0;
	
	m_buff_mutex.Unlock();

	Write(m_buff_to_write, cur_length);
	return true;
}
