
#ifndef ILOGIMPL_H
#define ILOGIMPL_H

#include "common/estring.h"
#include "common/platform/thread/mutex.h"

class ILogImpl
{
public:
	ILogImpl();
	virtual ~ILogImpl();
	char *PrintBegin();
	void PrintEnd(int len);

	bool WriteBuff(bool force, unsigned long now);
	virtual estring GetTarget()=0;

protected:
	char *m_buff;
	int m_cur_buff_length;
	int m_max_buff_length;
	Mutex m_buff_mutex;

	char *m_buff_to_write;
	int m_max_buff_to_write_length;

	void Resize();

	virtual void Write(const char* str, int length)=0;
	virtual void CheckLogSegment(){}
	virtual bool CheckWrite(unsigned long now) { return true; }
};




#endif

