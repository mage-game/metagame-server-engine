
#ifndef BASICNETWORK_H
#define BASICNETWORK_H

#include <vector>
#include <queue>

#include "common/raobjlist.h"
#include "common/platform/thread/mutex.h"
#include "common/platform/socket/pisocket.h"
#include "common/platform/thread/thread.h"

#include "def.h"
#include "basicnetworkhandler.h"
#include "jobqueue.h"


class Job;

#ifdef _WIN32
	#ifdef EPOLL_NETWORK
		#undef EPOLL_NETWORK
	#endif

	#ifndef SELECT_NETWORK
		#define SELECT_NETWORK
	#endif
#endif

#ifdef __unix
	#ifndef SELECT_NETWORK
	#ifndef EPOLL_NETWORK
		#define EPOLL_NETWORK
	#endif
	#endif

#ifdef EPOLL_NETWORK
#include <sys/epoll.h>
#define MAX_EPOLL_SIZE (1024 * 15)
#endif

#endif

class BasicNetwork
{
public:
	BasicNetwork(JobQueue *job_queue);
	~BasicNetwork();

	void Start();
	void Stop();

	NetID Add(BasicNetworkHandler *handler);
	void Remove(NetID netid);
	void Clear();

	void Print();

	bool SendPackage(NetID netid, const char *data, unsigned int len);

protected:
	friend class ListenHandler;
	friend class TcpHandler;
	void PushJob(Job * job);
	bool UnregisterWrite(NetID netid, int num=1);

private:
	struct RegisterTableItem
	{
		BasicNetworkHandler *handler;
		int write_event;

		RegisterTableItem():write_event(0){}
	};
	typedef RAObjList<RegisterTableItem> RegisterTable;
	typedef RAObjList<RegisterTableItem>::Iterator RegisterTableIter;

	typedef std::vector<NetID> DirtyQueue;
	typedef std::queue<Job *> JobTempList;
	typedef std::vector<BasicNetworkHandler*> HandlerList;

	RegisterTable m_register_table;
	Mutex m_register_table_mutex;

	DirtyQueue m_dirty_queue;			//延迟删除项
	Mutex m_dirty_queue_mutex;

	Thread	m_work_thread;				//工作线程
	bool m_is_exit;						//线程退出标志

	JobTempList m_job_to_push;
	JobQueue *m_job_queue;

	void DeleteDirtySocket();
	void PushJobToInvoke();
	static DWORD ThreadFunc(void *param); //线程函数
	DWORD WorkFunc();

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////// 以下为平台网络io模型相关代码 ////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	void InitSocket();
	void ReleaseSocket();
	void AddSocket(BasicNetworkHandler* handler);
	void RemoveSocket(SOCKET sock);
	void RegisterSocketWrite(BasicNetworkHandler *handler);
	void UnregisterSocketWrite(BasicNetworkHandler *handler);
	void PollSocket(HandlerList *readhandler, HandlerList *writehandler);
	
	#ifdef SELECT_NETWORK
	int GetSocketFd(fd_set &fd_read , fd_set &fd_write);
	fd_set	m_fd_read;					//读描述符
	fd_set	m_fd_write;					//写描述符 note：两个描述符与table保持一致，加入时只注册read，且与register_table共用一把锁
	int		m_max_sock;					//表中的sock最大值，linux下用

	fd_set	m_tmp_fd_read;				//在PollSocket中使用的临时读描述符
	fd_set	m_tmp_fd_write;				//在PollSocket中使用的临时写描述符
	#endif

	#ifdef EPOLL_NETWORK
	SOCKET m_epfd;						// epoll fd
	epoll_event m_tmp_event[MAX_EPOLL_SIZE];	// 在PollSocket中使用的临时event数组
	#endif
};


#include "tcphandler.h"
// 每秒几十W次
inline bool BasicNetwork::SendPackage(NetID netid, const char *buffer, unsigned int len)
{
	m_register_table_mutex.Lock();

	RegisterTableIter iter = m_register_table.Find(netid);
	if (iter == m_register_table.End())
	{
		m_register_table_mutex.Unlock();
		return false;
	}

	bool ret = false;
	TcpHandler *tcphandler = (TcpHandler*)(iter->handler);
	if (tcphandler != 0 && tcphandler->GetType() == BasicNetworkHandler::HT_TCP)
	{
		if (!tcphandler->SendPackage(buffer, len))
		{
			m_register_table_mutex.Unlock();
			return false;	
		}
		
		if (iter->write_event == 0)
		{
			RegisterSocketWrite(tcphandler);
		}
		iter->write_event += len + sizeof(LinkLayer);
		ret = true;
	}
	m_register_table_mutex.Unlock();

	return ret;
}

#endif

