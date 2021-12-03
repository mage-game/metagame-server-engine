
#pragma once

#ifndef FILEWRITEMODULE_H
#define FILEWRITEMODULE_H

#include "ifilewritemodule.h"
#include "common/platform/thread/threadqueue.h"
#include "common/platform/thread/thread.h"
#include "common/platform/thread/event.h"
#include "common/estring.h"

class FileWriteModule : public IFileWriteModule
{
public:
	FileWriteModule();
	virtual ~FileWriteModule();

	virtual int Init();
	virtual int Start();
	virtual int Update();
	virtual int Stop();
	virtual int Release();
	virtual void Free();

	virtual bool Write(const char *path, const char *data, int length, bool is_append);

protected:
	struct FileWriteItem
	{
		estring path;
		bool is_append;
		const char *data;
		int length;
	};
	typedef _ThreadQueue<FileWriteItem> FileWriteList;
	FileWriteList m_file_write_list;

	Thread	m_write_thread;
	bool	m_run;

	Event	m_write_event;
	static DWORD WriteThread(void *p);
	void WriteThreadHelper();
};

#endif

