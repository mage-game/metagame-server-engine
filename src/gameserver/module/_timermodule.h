
#pragma once

#ifndef TIMERMODULE_H
#define TIMERMODULE_H

#include "itimermodule.h"
#include "common/heap.h"

class IClockModule;

class TimerModule : public ITimerModule
{
public:
	TimerModule();
	virtual ~TimerModule();

	virtual int Init();
	virtual int Start();
	virtual int Update();
	virtual int Stop();
	virtual int Release();
	virtual void Free();

	virtual void CreateTimer(unsigned int interval_ms, ITimerCallback *callback_obj);
	virtual void CreateTimerSecond(unsigned int interval_second, ITimerCallback *callback_obj);

protected:
	struct TimerItem 
	{
		TimerItem():trigger_time(0), timer_callback(0) {}
		TimerItem(long long trigger_time, ITimerCallback *back_obj):trigger_time(trigger_time), timer_callback(back_obj){}
		long long trigger_time;
		ITimerCallback *timer_callback;

		bool operator <(const TimerItem &v) const
		{
			return trigger_time < v.trigger_time;
		}
	};

	Heap<TimerItem> m_timer_ms_heap;
	Heap<TimerItem> m_timer_second_heap;

	IClockModule *m_clock;
	time_t m_last_seconed_update_time;
};

#endif
