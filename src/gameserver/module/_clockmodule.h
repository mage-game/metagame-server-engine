
#pragma once

#ifndef CLOCKMODULE_H
#define CLOCKMODULE_H

#include "iclockmodule.h"
#include "utility/logagent.h"

class ClockModule : public IClockModule
{
public:
	ClockModule();
	virtual ~ClockModule();

	virtual int Init();
	virtual int Start();
	virtual int Update();
	virtual int Stop();
	virtual int Release();

	virtual void Free();

	virtual void SetFPS(unsigned long fps);
	virtual unsigned long GetFPS()const				{ return m_fps; }
	virtual unsigned long GetCurrentFrame()const	{ return m_current_frame; }
	virtual unsigned long GetFrameTime()const		{ return m_current_frame_start_time; }
	virtual time_t Time() const						{ return m_now; }
	virtual unsigned int DayID() const				{ return m_day_id; }
	virtual const tm *LocalTime() const				{ return &m_local_time; }
	virtual const char *AscTime() const				{ return m_time_str; }

	virtual int NextMinuteInterval(int sceond) const;
	virtual int NextHourInterval(int minute, int second) const;
	virtual int NextDayInterval(int hour, int minute, int second) const;
	virtual int NextWeekInterval(int wday, int hour, int minute, int second) const;
	virtual int NextMouthInterval(int date, int hour, int minute, int second) const;
	virtual int LocalTimeInterval(tm *t) const;
private:
	unsigned long m_fps;
	unsigned long m_frame_ms;
	unsigned long m_current_frame;
	unsigned long m_current_frame_start_time;

	time_t m_now;
	unsigned int m_day_id;

	tm m_local_time;
	char m_time_str[64];
};

#endif
