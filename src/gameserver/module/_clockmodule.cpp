
#include "_clockmodule.h"
#include "iconfigmodule.h"
#include "ilogmodule.h"
#include "common/platform/system.h"
#include "utility/configpath.h"

#include <string.h>

ClockModule::ClockModule()
	:m_fps(3), m_frame_ms(1000/3), m_current_frame(0), m_current_frame_start_time(0), m_now(0), m_day_id(0)
{
	memset(&m_local_time, 0, sizeof(tm));
	memset(m_time_str, 0, sizeof(m_time_str));
}

ClockModule::~ClockModule()
{

}

void ClockModule::Free()
{
	delete this;
}

int ClockModule::Init()
{
	EXPECT_ON_INIT(CONFIG_MODULE);

	IConfigModule* config = dynamic_cast<IConfigModule*>(Interface()->QueryModule(CONFIG_MODULE));

	if (config != 0)
	{
		unsigned long fps;
		config->SyncValue(ROOT/CLOCK_MODULE/"FPS", &fps, m_fps);

		SetFPS(fps);
	}

	m_current_frame_start_time = PITime();
	m_now = time(0);
	m_day_id = GetDayID();
	struct tm *p_tm = localtime(&m_now);
	strftime(m_time_str, sizeof(m_time_str), "%Y-%m-%d %H:%M:%S", p_tm);
	m_local_time = *p_tm;

	return IModule::Succeed;
}

int ClockModule::Start()
{
	return IModule::Succeed;
}

/*
	当服务器过载时，此处逻辑会导致帧丢失
*/
int ClockModule::Update()
{
	unsigned long new_time = PITime();
	unsigned long diff = new_time - m_current_frame_start_time;
	if (m_frame_ms <= diff)
	{
		m_current_frame_start_time = new_time;
		++m_current_frame;
		time_t now_tmp = time(0);
		if (now_tmp != m_now)
		{
			m_now = now_tmp;
			struct tm *p_tm = localtime(&m_now);
			m_local_time = *p_tm;

			// 时间字符串转化
			//char *time_str = asctime(p_tm);
			//char *time_str = ctime(&m_now);
			//strcpy(m_time_str, time_str);

			strftime(m_time_str, sizeof(m_time_str), "%Y-%m-%d %H:%M:%S", p_tm);
			
			struct
			{
				unsigned short year;
				unsigned char  mon;
				unsigned char  day;
			} now_day;
			now_day.year = (unsigned short)p_tm->tm_year;
			now_day.mon = (unsigned char)p_tm->tm_mon;
			now_day.day = (unsigned char)p_tm->tm_mday;
			m_day_id = *(unsigned int*)&now_day;
		}
	}
	return IModule::Pending;
}

int ClockModule::Stop()
{
	return IModule::Succeed;
}

int ClockModule::Release()
{
	return IModule::Succeed;
}

void ClockModule::SetFPS(unsigned long fps)
{
	m_fps = fps; 
	m_frame_ms = 1000 / m_fps;
}

int ClockModule::NextMinuteInterval(int second) const
{
	int itv_second = second - m_local_time.tm_sec;
	itv_second <= 0 ? itv_second += 60 : 0;
	return itv_second;
}

int ClockModule::NextHourInterval(int minute, int second) const
{
	int itv_second = (minute - m_local_time.tm_min) * 60 + second - m_local_time.tm_sec;
	itv_second <= 0 ? itv_second += 3600 : 0;

	return itv_second;
}

int ClockModule::NextDayInterval(int hour, int minute, int second) const
{
	int itv_second = ((hour - m_local_time.tm_hour) * 60 + minute - m_local_time.tm_min) * 60 + second - m_local_time.tm_sec;
	itv_second <= 0 ? (itv_second += 3600 * 24) : 0;

	return itv_second;
}

int ClockModule::NextWeekInterval(int wday, int hour, int minute, int second) const
{
	int itv_second = (((wday - m_local_time.tm_wday) * 24 + (hour - m_local_time.tm_hour)) * 60 + minute - m_local_time.tm_min) * 60 + second - m_local_time.tm_sec;
	itv_second <= 0 ? (itv_second += 3600 * 24 * 7) : 0;

	return itv_second;
}

int ClockModule::NextMouthInterval(int mday, int hour, int minute, int second) const
{

	int itv_second = (((mday - m_local_time.tm_mday) * 24 + (hour - m_local_time.tm_hour)) * 60 + minute - m_local_time.tm_min) * 60 + second - m_local_time.tm_sec;


	static int _mday[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	int mday_count = _mday[m_local_time.tm_mon];

	if (m_local_time.tm_mon == 1 // 2 月
		&& (((m_local_time.tm_year % 4 == 0) && (m_local_time.tm_year % 100 != 0)) || ((m_local_time.tm_year + 1900) % 400 == 0)))
	{
		mday_count = 29;
	}

	itv_second <= 0 ? (itv_second += 3600 * 24 * mday_count) : 0;
	return itv_second;
}

int ClockModule::LocalTimeInterval(tm *t) const
{
	time_t that_time = mktime(t);
	return int(that_time - m_now);
}


