
#include "_timermodule.h"
#include "iclockmodule.h"

TimerModule::TimerModule():m_clock(0), m_last_seconed_update_time(0)
{

}

TimerModule::~TimerModule()
{

}

int TimerModule::Init()
{
	DEPEND_ON_INIT(CLOCK_MODULE)
	m_clock = dynamic_cast<IClockModule*>(Interface()->QueryModule(CLOCK_MODULE));
	return Succeed;
}


int TimerModule::Start()
{
	return Succeed;
}


int TimerModule::Update()
{
	long long now = m_clock->GetFrameTime();

	TimerItem timer;
	while (m_timer_ms_heap.Front(&timer))
	{
		if (now  < timer.trigger_time)
		{
			break;
		}
		m_timer_ms_heap.PopFront();
		timer.timer_callback->OnTimer();
		timer.timer_callback->Free();
	}

	long long now_second = (long long)m_clock->Time();
	if (m_last_seconed_update_time != now_second)
	{
		TimerItem second_timeitem;
		while (m_timer_second_heap.Front(&second_timeitem))
		{
			if (now_second  < second_timeitem.trigger_time)
			{
				break;
			}
			m_timer_second_heap.PopFront();
			second_timeitem.timer_callback->OnTimer();
			second_timeitem.timer_callback->Free();
		}
		m_last_seconed_update_time = now_second;
	}

	return Pending;
}


int TimerModule::Stop()
{
	while(m_timer_ms_heap.Size() != 0)
	{
		TimerItem timer = m_timer_ms_heap.Front();
		m_timer_ms_heap.PopFront();
		timer.timer_callback->Free();
	}

	while(m_timer_second_heap.Size() != 0)
	{
		TimerItem timer = m_timer_second_heap.Front();
		m_timer_second_heap.PopFront();
		timer.timer_callback->Free();
	}

	return Succeed;
}


int TimerModule::Release()
{
	return Succeed;
}

void TimerModule::Free()
{
	delete this;
}

void TimerModule::CreateTimer(unsigned int interval_ms, ITimerCallback *callback_obj)
{
	m_timer_ms_heap.Push(TimerItem(m_clock->GetFrameTime() + interval_ms, callback_obj));
}

void TimerModule::CreateTimerSecond(unsigned int interval_second, ITimerCallback *callback_obj)
{
	m_timer_second_heap.Push(TimerItem((unsigned int)m_clock->Time() + interval_second, callback_obj));
}

