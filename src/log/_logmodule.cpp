
#include <stdio.h>

#include "iinterfacemgr.h"
#include "_logmodule.h"
#include "stdoutlog.h"
#include "filelog.h"
#include "nullog.h"

#include "iconfigmodule.h"
#include "iclockmodule.h"
#include "utility/configpath.h"
#include "common/platform/system.h"

LogModule::LogModule()
:m_clock(0), /*m_now_time(0),*/ m_ack_time(0), m_log_level((int)LL_INFO),  m_cur_catagory_num(0), m_default_target("1"), m_run(false)
{
}

void LogModule::Free()
{
	delete this;
}

int LogModule::Init()
{
	EXPECT_ON_INIT(CONFIG_MODULE)
	DEPEND_ON_INIT(CLOCK_MODULE)

	m_clock = dynamic_cast<IClockModule*>(Interface()->QueryModule(CLOCK_MODULE));
	//m_now_time = (unsigned int)m_clock->Time();
	m_ack_time = m_clock->AscTime();

	IConfigModule* config = dynamic_cast<IConfigModule*>(Interface()->QueryModule(CONFIG_MODULE));

	if (config != 0)
	{
		int loglevel;
		config->SyncValue(ROOT/LOG_MODULE/"LogLevel", &loglevel, (int)LL_INFO);

		SetLogLevel(loglevel);

		config->SyncValue(ROOT/LOG_MODULE/"LogTarget", &m_default_target, estring("1"));
	}

	m_run = true;
	m_write_thread.Run(WriteThread, this);

	return IModule::Succeed;
}

int LogModule::Start()
{
	return IModule::Succeed;
}

int LogModule::Update()
{
	//m_now_time = (unsigned int)m_clock->Time();
	//return IModule::Pending;
	return IModule::Succeed;
}

int LogModule::Stop()
{
	return IModule::Succeed;
}

int LogModule::Release()
{
	m_run = false;
	m_write_thread.Join();

	for (int i = 0; i < m_cur_catagory_num; ++i)
	{
		if (m_catagory_list[i].impl != 0)
		{
			for (int j = i + 1; j < m_cur_catagory_num; ++j)
			{
				if (m_catagory_list[j].impl != 0 && m_catagory_list[j].impl->GetTarget() == m_catagory_list[i].impl->GetTarget())
				{
					m_catagory_list[j].impl = 0;
				}
			}
			delete m_catagory_list[i].impl;
		}
	}

	return IModule::Succeed;
}

void LogModule::printf(int log_level, int log_catagory, const char *fmt, ...)
{
	va_list argp;
	va_start(argp, fmt);
	vprintf(log_level, log_catagory, fmt, argp);
	va_end(argp); 
}

void LogModule::vprintf(int log_level, int log_catagory, const char *fmt, va_list argp)
{
	if(log_catagory >= MAX_CATAGORY_NUM || log_catagory < 0 || m_catagory_list[log_catagory].impl == 0) return;

	// 有效性检查
	if(LL_SPECIAL != log_level && log_level > m_log_level)
	{
		return;
	}

	char *buffer = m_catagory_list[log_catagory].impl->PrintBegin();

	int cur_length = 0;
	if (LL_SPECIAL != log_level)
	{
		cur_length = ::sprintf(buffer, "[%s] %s (%s): ", m_ack_time, m_catagory_list[log_catagory].name.c_str(), GetLevelStr(log_level));
	}
	cur_length += ::vsprintf(buffer + cur_length, fmt, argp);
	buffer += cur_length++;
	*buffer++ = '\n';
	*buffer = '\0';

	m_catagory_list[log_catagory].impl->PrintEnd(cur_length);
}

void LogModule::print(int log_level, int log_catagory, const char *str)
{
	if(log_catagory >= MAX_CATAGORY_NUM || log_catagory < 0 || m_catagory_list[log_catagory].impl == 0) return;

	// 有效性检查
	if(LL_SPECIAL != log_level && log_level > m_log_level)
	{
		return;
	}

	char *result = m_catagory_list[log_catagory].impl->PrintBegin();

	int cur_length = 0;
	if (LL_SPECIAL != log_level)
	{
		cur_length = ::sprintf(result, "[%s] %s (%s): ", m_ack_time, m_catagory_list[log_catagory].name.c_str(), GetLevelStr(log_level));
	}
	cur_length += ::sprintf(result + cur_length, "%s\n", str);

	m_catagory_list[log_catagory].impl->PrintEnd(cur_length);

}

inline const char* LogModule::GetLevelStr(int level)
{
	static const char *LL_STR[LL_COUNT] = { "Critical", "Error", "Maintance", "Warning", "Info", "Debug", "Special" };

	return (level >= 0 && level < LL_COUNT) ? LL_STR[level] : "Unknown";
}

int LogModule::RegisterCatagory(const char *cata, const char *output, int new_file_interval)
{
	if (m_cur_catagory_num >= MAX_CATAGORY_NUM)
	{
		return -1;
	}

	for (int i = 0; i < m_cur_catagory_num; ++i)	// 查找同个Catagory的
	{
		if(m_catagory_list[i].name == cata)
		{
			return i;
		}
	}

	m_catagory_list[m_cur_catagory_num].name = cata;

	estring target;
	if (output == 0 || estring(output) == "")
	{
		target = m_default_target;
	}
	else
	{
		target = output;
	}

	for (int i = 0; i < m_cur_catagory_num; ++i)
	{
		if (m_catagory_list[i].impl->GetTarget() == target)
		{
			m_catagory_list[m_cur_catagory_num].impl = m_catagory_list[i].impl;
			int channel = m_cur_catagory_num++;
			return channel;
		}
	}

	if (target == "1")	// 标准输出流
	{
		m_catagory_list[m_cur_catagory_num].impl = new StdoutLog();
	}
	else if (target == "0")	// NUL
	{
		m_catagory_list[m_cur_catagory_num].impl = new NulLog();
	}
	else			// 输出到文件
	{
		m_catagory_list[m_cur_catagory_num].impl = new FileLog(target.c_str(), new_file_interval, m_clock);
	}

	int channel = m_cur_catagory_num++;
	return channel;
}

DWORD LogModule::WriteThread(void *p)
{
	LogModule *pthis = (LogModule*)p;
	pthis->WriteThreadHelper();
	return 0;
}

void LogModule::WriteThreadHelper()
{
	while (m_run)
	{
		unsigned long now = PITime();
		for (int i = 0; i < m_cur_catagory_num; ++i)
		{
			m_catagory_list[i].impl->WriteBuff(false, now);
		}
		PISleep(10);
	}
	for (int i = 0; i < m_cur_catagory_num; ++i)
	{
		m_catagory_list[i].impl->WriteBuff(true, 0);
	}
}

