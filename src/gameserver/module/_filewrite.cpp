
#include "common/platform/file64.h"
#include "_filewrite.h"
#include "common/memory/msgmem.h"
#include "common/syspath.h"

#include <string.h>

FileWriteModule::FileWriteModule():m_file_write_list(10240), m_run(false)
{
	#ifdef __unix__
	umask(0);
	#endif
}

FileWriteModule::~FileWriteModule()
{

}

int FileWriteModule::Init()
{
	m_run = true;
	m_write_thread.Run(WriteThread, this);
	return Succeed;
}


int FileWriteModule::Start()
{
	return Succeed;
}


int FileWriteModule::Update()
{
	return Succeed;
}


int FileWriteModule::Stop()
{
	return Succeed;
}


int FileWriteModule::Release()
{
	m_run = false;
	m_write_thread.Join();

	FileWriteItem item;
	while (m_file_write_list.TryPop(&item))
	{
		MsgMem *msg_tmp = (MsgMem *)item.data;
		delete []msg_tmp;
	}
	return Succeed;
}

void FileWriteModule::Free()
{
	delete this;
}

bool FileWriteModule::Write(const char *path, const char *data, int length, bool is_append)
{
	FileWriteItem item;
	
	MsgMem *data_tmp = new MsgMem[length];
	memcpy(data_tmp, data, length);
	item.data = (const char *)data_tmp;
	item.length = length;

	item.path = path;
	item.is_append = is_append;

	if (m_file_write_list.TryPush(item))
	{
		m_write_event.Signal();
		return true;
	}

	delete []data_tmp;

	return false;
}


DWORD FileWriteModule::WriteThread(void *p)
{
	FileWriteModule *pThis = (FileWriteModule*)p;
	pThis->WriteThreadHelper();
	return 0;
}

void FileWriteModule::WriteThreadHelper()
{
	while (m_run)
	{
		FileWriteItem item;
		if (!m_file_write_list.TryPop(&item))
		{
			m_write_event.Wait(100);
			continue;
		}

		estring file_name = item.path;
		bool is_path_avalid = true;
		if (SysPath::Up(&file_name))
		{
			is_path_avalid = SysPath::CreateDir(file_name.c_str());
		}
		if (is_path_avalid)
		{
			int open_mode = O_RDWR | O_CREAT;

			if (!item.is_append)
			{
				open_mode |= O_TRUNC;
			}

			int file = -1;

			#ifdef __unix__
			int flag = DEFFILEMODE;	// 3个组可读可写，不可执行
			#endif

			#ifdef _WIN32
			int flag = S_IWRITE | S_IREAD;
			#endif

			if ( -1 == (file = open(item.path.c_str(), open_mode, flag))
				|| -1 == lseek64(file, 0, SEEK_END)
				|| item.length != write(file, item.data, item.length))
			{
				if (file != -1)
				{
					close(file);
				}
				continue;
			}
			close(file);
		}

		MsgMem *msg_tmp = (MsgMem *)item.data;
		delete []msg_tmp;
	}
}


