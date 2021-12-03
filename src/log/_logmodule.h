
#pragma once

#ifndef LOGMODULE_H
#define LOGMODULE_H

#include "ilogmodule.h"
#include "common/estring.h"
#include "common/platform/thread/thread.h"

/*
	Log ģ�飬�ṩ���ݵȼ����Ƽ�������Ϣ���������
	ʹ��RegisterCatagoryע����ָ࣬����������target��"1"��ʾ��׼�����"0"��ʾnul�����������������ļ�
	��ͬ��Catalog�����������ͬ��target
*/


#define  MAX_CATAGORY_NUM		256

class IClockModule;
class ILogImpl;

class LogModule : public ILogModule
{
public:

	struct Catagory
	{
		Catagory():name(""), impl(0){}
		std::string name;
		ILogImpl *impl;
	};

	LogModule();
	virtual int Init();
	virtual int Start();
	virtual int Update();
	virtual int Stop();
	virtual int Release();

	void Free();

	virtual void printf(int log_level, int log_catagory, const char *fmt, ...);
	virtual void vprintf(int log_level, int log_catagory, const char *fmt, va_list argp);
	virtual void print(int log_level, int log_catagory, const char *str);

	virtual void SetLogLevel(int level) { m_log_level = level; }
	virtual int GetLogLevel()const { return m_log_level; }
	virtual int RegisterCatagory(const char *cata, const char *output=0, int new_file_interval=0);

protected:
	
	static inline const char* GetLevelStr(int level);
private:
	IClockModule *m_clock;
	//unsigned int m_now_time;
	const char *m_ack_time;

	int m_log_level;
	Catagory m_catagory_list[MAX_CATAGORY_NUM];
	int m_cur_catagory_num;

	estring m_default_target;

	Thread m_write_thread;

	static  DWORD WriteThread(void *p);
	void WriteThreadHelper();
	bool m_run;
};

#endif
