
#include "common/platform/file64.h"
#include <stdio.h>
#include <time.h>
#include "filelog.h"
#include "common/syspath.h"
#include "iclockmodule.h"
#include "ilogmodule.h"

FileLog::FileLog(const char* name, int new_file_interval, IClockModule *clock):m_clock(clock), m_file(-1), m_next_new_file_time(0), m_last_write_time(0)
{
	if (name == 0)
	{
		name = "log.txt";
	}
	m_file_name = name;
	
	estring file_name = m_file_name;
	bool is_path_avalid = true;
	if (SysPath::Up(&file_name))
	{
		is_path_avalid = SysPath::CreateDir(file_name.c_str());
	}
	if (is_path_avalid)
	{
		SaveOldLogs();

		#ifdef __unix__
		int flag = DEFFILEMODE;	// 3个组可读可写，不可执行
		#endif

		#ifdef _WIN32
		int flag = S_IWRITE | S_IREAD;
		#endif

		m_file = open(name, O_RDWR | O_CREAT | O_TRUNC, flag);
		//m_file = fopen(name, "w");
	}

	m_new_file_interval = new_file_interval;
	if (new_file_interval != 0)
	{
		if (new_file_interval > 0)
		{
			m_next_new_file_time = time(0) + new_file_interval;
		}
		else
		{
			switch (new_file_interval)
			{
			case ILogModule::NFIS_HALF_HOUR:
				{
					m_new_file_interval = 600;
					int next_interval = m_clock->NextHourInterval(0, 0);
					if (next_interval > m_new_file_interval) 
					{
						next_interval -= (next_interval / m_new_file_interval) * m_new_file_interval;
					}
					m_next_new_file_time = m_clock->Time() + next_interval;
				}
				break;
			case ILogModule::NFIS_HOUR:
				{
					m_new_file_interval = 3600;
					int next_interval = m_clock->NextHourInterval(0, 0);
					m_next_new_file_time = m_clock->Time() + next_interval;
				}
				break;
			case ILogModule::NFIS_HALF_DAY:
				{
					m_new_file_interval = 3600 * 12;
					int next_interval = m_clock->NextDayInterval(0, 0, 0);
					if (next_interval > m_new_file_interval) next_interval -= m_new_file_interval;
					m_next_new_file_time = m_clock->Time() + next_interval;
				}
				break;
			case ILogModule::NFIS_DAY:
				{
					m_new_file_interval = 3600 * 24;
					int next_interval = m_clock->NextDayInterval(0, 0, 0);
					m_next_new_file_time = m_clock->Time() + next_interval;
				}
				break;
			default:
				{
					m_new_file_interval = 0;
					m_next_new_file_time = 0;
				}
			}
		}
	}
}

FileLog::~FileLog()
{
	if(m_file != -1)
	{
		close(m_file);
	}
}

void FileLog::Write(const char* str, int length)
{
	if(m_file == -1) return ;

	write(m_file, str, length);
}

void FileLog::CheckLogSegment()
{
	if (m_new_file_interval != 0)
	{
		time_t now = m_clock->Time();
		if (now >= m_next_new_file_time)
		{
			if(m_file != -1)
			{
				close(m_file);
			}
			SaveOldLogs();

			#ifdef __unix__
			int flag = DEFFILEMODE;	// 3个组可读可写，不可执行
			#endif

			#ifdef _WIN32
			int flag = S_IWRITE | S_IREAD;
			#endif

			m_file = open(m_file_name.c_str(), O_RDWR | O_CREAT | O_TRUNC, flag);
			m_next_new_file_time = now + m_new_file_interval;
		}
	}
}


// 限制最小字节、最短时间写日志策略, 缓解磁盘io过度频繁的问题
#define MIN_WRITE_BYTE_PER_TIME		2048	// 缓存小于该值则不写
#define MIN_INTERVAL_TO_WRITE		1000	// 时间小于该值则不写


bool FileLog::CheckWrite(unsigned long now)
{
	if (m_cur_buff_length < MIN_WRITE_BYTE_PER_TIME && (now - m_last_write_time) < MIN_INTERVAL_TO_WRITE)
	{
		return false;
	}
	m_last_write_time = now;
	return true;
}

bool FileLog::SaveOldLogs()
{
	if (access(m_file_name.c_str(), 0) == -1) return true;

	estring name;
	estring ext;
	if (!SysPath::SplitFileName(m_file_name, &name, &ext))
	{
		name = m_file_name;
		ext = "";
	}

	char old_name[SysPath::PI_MAX_PATH] = {0};
	char now_date[64] = {0};

	time_t now_time = time(0);
	
	struct tm *tmp_tm = ::localtime(&now_time);
	if(NULL == tmp_tm) return false;

	sprintf(now_date, "%04d_%02d_%02d_%02d_%02d", tmp_tm->tm_year + 1900, tmp_tm->tm_mon + 1, tmp_tm->tm_mday,
			tmp_tm->tm_hour, tmp_tm->tm_min);

	sprintf(old_name, "%s_%s%s%s", name.c_str(), now_date, (ext == "" ? "" : "."), ext.c_str());
	return rename(m_file_name.c_str(), old_name) == 0;
}


const int MAX_OLD_LOG = 8;
// 旧文件加上_X的后缀
//bool FileLog::SaveOldLogs()
//{
//	estring name;
//	estring ext;
//	if (!SysPath::SplitFileName(m_file_name, &name, &ext))
//	{
//		name = m_file_name;
//		ext = "";
//	}
//
//	char old_name[SysPath::PI_MAX_PATH] = {0};
//	char new_name[SysPath::PI_MAX_PATH] = {0};
//
//	int i = 0;
//	for (; i < MAX_OLD_LOG; ++i)
//	{
//		sprintf(old_name, "%s_%d.%s", name.c_str(), i, ext.c_str());
//		if (access(old_name, 0) == -1)
//		{
//			break;
//		}
//	}
//
//	if (i == MAX_OLD_LOG)
//	{
//		--i;
//		sprintf(old_name, "%s_%d.%s", name.c_str(), i, ext.c_str());
//		remove(old_name);
//	}
//
//	for (; i > 0; --i)
//	{
//		sprintf(old_name, "%s_%d.%s", name.c_str(), i - 1, ext.c_str());
//		sprintf(new_name, "%s_%d.%s", name.c_str(), i, ext.c_str());
//		rename(old_name, new_name);
//	}
//
//	sprintf(new_name, "%s_%d.%s", name.c_str(), 0, ext.c_str());
//	return rename(m_file_name.c_str(), new_name) == 0;
//}



