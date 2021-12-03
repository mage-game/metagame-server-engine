
#pragma once

#ifndef FILELOG_H
#define FILELOG_H

#include "ilogimpl.h"
#include "common/estring.h"

class IClockModule;

class FileLog : public ILogImpl
{
public:
	FileLog(const char* name, int new_file_interval_min, IClockModule *clock);
	~FileLog();
	virtual void Write(const char* str, int length);
	virtual estring GetTarget()
	{
		return m_file_name;
	}
private:
	virtual void CheckLogSegment();
	virtual bool CheckWrite(unsigned long now);

	IClockModule *m_clock;
	int m_file;
	estring m_file_name;
	int m_new_file_interval;
	time_t m_next_new_file_time;
	bool SaveOldLogs();
	bool LogPathExist();

private:
	unsigned long m_last_write_time;
};

#endif
