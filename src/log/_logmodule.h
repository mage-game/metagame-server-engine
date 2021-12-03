
#pragma once

#ifndef LOGMODULE_H
#define LOGMODULE_H

#include "ilogmodule.h"
#include "common/estring.h"
#include "common/platform/thread/thread.h"

/*
	Log 模块，提供根据等级限制及分类信息输出的能力
	使用RegisterCatagory注册分类，指定分类的输出target，"1"表示标准输出，"0"表示nul输出，其他则输出到文件
	不同的Catalog可以输出到相同的target
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
