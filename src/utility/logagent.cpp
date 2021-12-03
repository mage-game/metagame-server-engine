
#include "ilogmodule.h"
#include "logagent.h"

#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

LogAgent::LogAgent()
:m_logmodule(0), m_catagory(-1), m_new_file_interval(0), m_log_buff(0), m_cur_buff_length(0), m_max_buff_length(0)
{
}

LogAgent::~LogAgent()
{
	if (m_log_buff != 0)
	{
		free(m_log_buff);
	}
}
void LogAgent::printf(int log_level, int log_catagory, const char *fmt, ...)
{
	if ( m_logmodule == 0 ) return;
	va_list argp;
	va_start(argp, fmt);
	m_logmodule->vprintf(log_level, log_catagory, fmt, argp);
	va_end(argp); 
}

void LogAgent::printf(int log_level, const char *fmt, ...)
{
	if ( m_logmodule == 0 ) return;

	va_list argp;
	va_start(argp, fmt);
	m_logmodule->vprintf(log_level, m_catagory, fmt, argp);
	va_end(argp); 
}

void LogAgent::print(int log_level, int log_catagory, const char *str)
{
	if ( m_logmodule == 0 ) return;
	m_logmodule->print(log_level, log_catagory, str);
}

void LogAgent::print(int log_level, const char *str)
{
	if ( m_logmodule == 0 ) return;
	m_logmodule->print(log_level, m_catagory, str);
}

void LogAgent::SetLogModule(ILogModule* module)
{
	m_logmodule = module;
	if (m_logmodule != 0 && m_cata_name != "")
	{
		m_catagory = m_logmodule->RegisterCatagory(m_cata_name.c_str(), m_output.c_str(), m_new_file_interval);
	}
}


void LogAgent::SetCatagory(const char *catagory, const char *output, int new_file_interval)
{
	m_cata_name = catagory;
	m_output = (output == 0) ? "" : output;
	m_new_file_interval = new_file_interval;
	if(m_logmodule != 0 && m_cata_name != "")
	{
		m_catagory = m_logmodule->RegisterCatagory(m_cata_name.c_str(), m_output.c_str(), m_new_file_interval);
	}
}



void LogAgent::buff_printf(const char *fmt, ...)
{
	if (m_max_buff_length - m_cur_buff_length < MAX_LOG_LENGTH)
	{
		resize_buff();
	}

	va_list argp;
	va_start(argp, fmt);
	m_cur_buff_length += ::vsprintf(m_log_buff + m_cur_buff_length, fmt, argp);
	va_end(argp); 
}

void LogAgent::buff_print(const char *str)
{
	if (m_max_buff_length - m_cur_buff_length < MAX_LOG_LENGTH)
	{
		resize_buff();
	}

	m_cur_buff_length += ::sprintf(m_log_buff + m_cur_buff_length, str);
}

void LogAgent::commit_buff(int log_level)
{
	if (m_log_buff != 0)
	{
		if (m_cur_buff_length > MAX_LOG_LENGTH - 256)
		{
			printf(LL_ERROR, "LogAgent::commit_buff ERROR cur_buff_length:%d too large.", m_cur_buff_length);
			m_cur_buff_length = 0;
			return ;
		}
		print(log_level, m_log_buff);
		m_cur_buff_length = 0;
	}
}

void LogAgent::clear_buff()
{
	m_cur_buff_length = 0;
}

void LogAgent::resize_buff()
{
	int max_size = m_max_buff_length + MAX_LOG_LENGTH;
	char *buff = (char*)malloc(max_size);
	if (m_log_buff != 0)
	{
		memcpy(buff, m_log_buff, m_cur_buff_length + 1);
		free(m_log_buff);
	}
	m_log_buff = buff;
	m_max_buff_length = max_size;
}

