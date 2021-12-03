
#include "basicnetwork.h"
#include "common/platform/system.h"
#include "job.h"
#include <stdio.h>


BasicNetwork::BasicNetwork(JobQueue *job_queue):m_is_exit(true),m_job_queue(job_queue)
{
	InitSocket();
}

BasicNetwork::~BasicNetwork()
{
	Clear();
}

void BasicNetwork::Start()
{
	if (m_is_exit)
	{
		m_is_exit = false;
		m_work_thread.Run(ThreadFunc, this, 0);
	}
}

void BasicNetwork::Stop()
{
	if (!m_is_exit)
	{
		m_is_exit = true;
		m_work_thread.Join();
	}
}

NetID BasicNetwork::Add(BasicNetworkHandler *handler)
{
	m_register_table_mutex.Lock();

	RegisterTableItem item;
	item.handler = handler;
	item.write_event = 0;

	NetID netid = m_register_table.Insert(item);
	handler->SetBasicNetwork(this);
	handler->SetNetid(netid);

	AddSocket(handler);

	m_register_table_mutex.Unlock();
	return netid;
}

void BasicNetwork::Remove(NetID netid)
{
	m_dirty_queue_mutex.Lock();
	m_dirty_queue.push_back(netid);
	m_dirty_queue_mutex.Unlock();
}

void BasicNetwork::Clear()
{
	Stop();

	m_register_table_mutex.Lock();
	ReleaseSocket();
	for (RegisterTableIter iter = m_register_table.Beg() ; iter != m_register_table.End(); ++iter)
	{
		iter->handler->OnClose();
		delete iter->handler;
	}
	m_register_table_mutex.Unlock();

	m_dirty_queue_mutex.Lock();
	m_dirty_queue.clear();
	m_dirty_queue_mutex.Unlock();

	while (!m_job_to_push.empty())
	{
		Job *job = m_job_to_push.front();
		m_job_to_push.pop();
		delete job;
	}

	m_is_exit = false;
}

void BasicNetwork::DeleteDirtySocket()
{
	//清理dirty socket
	DirtyQueue temp_queue;
	m_dirty_queue_mutex.Lock();
	m_dirty_queue.swap(temp_queue);
	m_dirty_queue_mutex.Unlock();

	if (temp_queue.size() != 0)
	{
		m_register_table_mutex.Lock();
		for (DirtyQueue::iterator iter = temp_queue.begin(); iter != temp_queue.end(); ++ iter)
		{
			RegisterTableIter item_erase = m_register_table.Find(*iter);
			if (item_erase == m_register_table.End())
			{
				continue;
			}

			//先删除BasicNetwork中的信息，再调用OnClose
			BasicNetworkHandler *handler = item_erase->handler;
			SOCKET sock = handler->GetSocket();
			m_register_table.Erase(*iter);
			handler->OnClose();				// 这里有点问题，这里限制了OnClose里面不能调用BasicNetwork中加锁的接口!*****************************
											// 因为在Linux下pthread_mutex_t是非递归锁

			RemoveSocket(sock);
			delete handler;

		}
		m_register_table_mutex.Unlock();
	}
}

void BasicNetwork::PushJobToInvoke()
{
	while (!m_job_to_push.empty())
	{
		Job *job = m_job_to_push.front();

		if (m_job_queue->TryPush(job))
		{
			m_job_to_push.pop();
		}
		else
		{
			break;
		}
	}
}

void BasicNetwork::PushJob(Job * job)
{
	m_job_to_push.push(job);
}

bool BasicNetwork::UnregisterWrite(NetID netid, int num)
{
	m_register_table_mutex.Lock();

	BasicNetwork::RegisterTableIter iter = m_register_table.Find(netid);
	if (iter == m_register_table.End())
	{
		m_register_table_mutex.Unlock();
		return false;
	}

	iter->write_event -= num;
	if (iter->write_event == 0)
	{
		UnregisterSocketWrite(iter->handler);
	}

	m_register_table_mutex.Unlock();

	return true;
}

//工作线程的主循环函数
DWORD BasicNetwork::ThreadFunc(void *param)
{
	BasicNetwork *pthis = (BasicNetwork*) param;
	return pthis->WorkFunc();
}

DWORD BasicNetwork::WorkFunc()
{
	HandlerList vector_can_read;
	HandlerList vector_can_write;

	for (;;)
	{
		if (m_is_exit)
		{
			return 0;
		}

		DeleteDirtySocket();
		PushJobToInvoke();

		m_register_table_mutex.Lock();
		if (m_register_table.Size() == 0)
		{
			m_register_table_mutex.Unlock();
			PISleep(10);
			continue;
		}
		m_register_table_mutex.Unlock();

		PollSocket(&vector_can_read, &vector_can_write);

		//处理事件
		for (HandlerList::iterator iter = vector_can_read.begin(); iter != vector_can_read.end(); ++iter)
		{
			(*iter)->OnCanRead();
		}

		for (HandlerList::iterator iter = vector_can_write.begin(); iter != vector_can_write.end(); ++iter)
		{
			(*iter)->OnCanWrite();
		}
		vector_can_read.clear();
		vector_can_write.clear();
	}
	DeleteDirtySocket();
	return 0;
}

void BasicNetwork::Print()
{
	m_register_table_mutex.Lock();

	long long max_tcphandler_mem = 0, use_tcphandler_mem = 0;
	long long top_tcphandler_mem = 0;

	int greate_1m_num = 0, greate_4m_num = 0, greate_8m_num = 0, greate_16m_num = 0;

	for (BasicNetwork::RegisterTableIter iter = m_register_table.Beg() ; iter != m_register_table.End(); ++iter)
	{
		BasicNetworkHandler *basic_handler = iter->handler;
		if (NULL != basic_handler && BasicNetworkHandler::HT_TCP == basic_handler->GetType())
		{
			TcpHandler *tcp_hander = (TcpHandler*)basic_handler;

			max_tcphandler_mem += (tcp_hander->m_max_msg_to_write_buff_len + tcp_hander->m_max_buff);
			use_tcphandler_mem += (tcp_hander->m_cur_msg_to_write_buff_len + tcp_hander->m_cur_buff);

			if (tcp_hander->m_max_msg_to_write_buff_len + tcp_hander->m_max_buff > top_tcphandler_mem)
			{
				top_tcphandler_mem = tcp_hander->m_max_msg_to_write_buff_len + tcp_hander->m_max_buff;
			}

			if (tcp_hander->m_max_msg_to_write_buff_len >= 16 * 1024 * 1024 || tcp_hander->m_max_buff >= 16 * 1024 * 1024)
			{
				printf("[netid:%d tcp_hander->m_max_msg_to_write_buff_len:%d tcp_hander->m_max_buff:%d] \n", 
					basic_handler->GetNetid(), tcp_hander->m_max_msg_to_write_buff_len, tcp_hander->m_max_buff);
			}

			if (tcp_hander->m_max_msg_to_write_buff_len + tcp_hander->m_max_buff >= 16 * 1024 * 1024)
			{
				++ greate_16m_num;
			}
			else if (tcp_hander->m_max_msg_to_write_buff_len + tcp_hander->m_max_buff >= 8 * 1024 * 1024)
			{
				++ greate_8m_num;
			}
			else if (tcp_hander->m_max_msg_to_write_buff_len + tcp_hander->m_max_buff >= 4 * 1024 * 1024)
			{
				++ greate_4m_num;
			}
			else if (tcp_hander->m_max_msg_to_write_buff_len + tcp_hander->m_max_buff >= 1 * 1024 * 1024)
			{
				++ greate_1m_num;
			}
		}
	}

	printf("m_register_table:%u  max_tcphandler_mem:%lld  use_tcphandler_mem:%lld top_tcphandler_mem:%lld \n", 
		m_register_table.Size(), max_tcphandler_mem, use_tcphandler_mem, top_tcphandler_mem);
	printf("[1M:%d 4M:%d 8M:%d 16M:%d] \n", greate_1m_num, greate_4m_num, greate_8m_num, greate_16m_num);
	fflush(stdout);

	m_register_table_mutex.Unlock();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////// 以下为平台网络io模型相关代码 ////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef SELECT_NETWORK
//由构造函数调用
void BasicNetwork::InitSocket()
{
	ReleaseSocket();
}

//由Clear调用
void BasicNetwork::ReleaseSocket()
{	
	m_max_sock = SOCKET_ERROR;
	FD_ZERO(&m_fd_read);
	FD_ZERO(&m_fd_write);
}

//由Add调用
void BasicNetwork::AddSocket(BasicNetworkHandler* handler)
{
	SOCKET sock_fd = handler->GetSocket();
	FD_SET(sock_fd, &m_fd_read);
	m_max_sock = ((m_max_sock) > (int)(sock_fd)) ? (m_max_sock) : (int)(sock_fd);
}

//由DeleteDirtySocket调用
void BasicNetwork::RemoveSocket(SOCKET sock_remove)
{
	FD_CLR(sock_remove, &m_fd_read);
	FD_CLR(sock_remove, &m_fd_write);
	if (sock_remove == m_max_sock)
	{
		//需要重新找出最大的sock
		m_max_sock = -1;
		for (RegisterTableIter iter = m_register_table.Beg() ; iter != m_register_table.End(); ++iter)
		{
			int sock_iter = (int)iter->handler->GetSocket();
			m_max_sock = ((m_max_sock) > sock_iter ? (m_max_sock) : sock_iter);
		}
	}
}

int BasicNetwork::GetSocketFd(fd_set &fdread , fd_set &fdwrite)
{
	fdread = m_fd_read;
	fdwrite = m_fd_write;
	return m_max_sock;
}

//由SendPackage调用
void BasicNetwork::RegisterSocketWrite(BasicNetworkHandler* handler)
{
	FD_SET(handler->GetSocket(), &m_fd_write);
}

//由UnRegisterWrite调用
void BasicNetwork::UnregisterSocketWrite(BasicNetworkHandler* handler)
{
	FD_CLR(handler->GetSocket(), &m_fd_write);
}

void BasicNetwork::PollSocket(HandlerList *readhandler, HandlerList *writehandler)
{
	struct timeval tv;	//超时参数
	tv.tv_sec = 0;		//秒
	tv.tv_usec = 10000;	//微秒,10毫秒

	m_register_table_mutex.Lock();
	int max_sock =  GetSocketFd(m_tmp_fd_read, m_tmp_fd_write);
	m_register_table_mutex.Unlock();

	int ret = select(max_sock + 1, &m_tmp_fd_read,  &m_tmp_fd_write, 0, &tv);
	if (ret > 0)
	{
		m_register_table_mutex.Lock();
		for (BasicNetwork::RegisterTableIter iter = m_register_table.Beg() ; iter != m_register_table.End(); ++iter)
		{
			SOCKET sock_iter = iter->handler->GetSocket();
			if (FD_ISSET(sock_iter, &m_tmp_fd_read))
			{
				//检查断开在OnCanRead中完成
				readhandler->push_back(iter->handler);
			}

			if (FD_ISSET(sock_iter, &m_tmp_fd_write))
			{
				writehandler->push_back(iter->handler);
			}
		}
		m_register_table_mutex.Unlock();
	}
}
#endif

#ifdef EPOLL_NETWORK
void BasicNetwork::InitSocket()
{
	m_epfd = epoll_create(MAX_EPOLL_SIZE);
}

void BasicNetwork::ReleaseSocket()
{	
	PISocket::Close(m_epfd);
}

void BasicNetwork::AddSocket(BasicNetworkHandler* handler)
{
	SOCKET sock_fd = handler->GetSocket();
	struct epoll_event ev;
	ev.events = EPOLLIN | EPOLLET;
	ev.data.ptr = (void *)handler;
	if (epoll_ctl(m_epfd, EPOLL_CTL_ADD, sock_fd, &ev) == -1)
	{
		// 添加失败
	}
}

void BasicNetwork::RemoveSocket(SOCKET sock_remove)
{
	struct epoll_event ev;
	if (epoll_ctl(m_epfd, EPOLL_CTL_DEL, sock_remove, &ev) == -1)
	{
		// 删除失败
	}
}

void BasicNetwork::RegisterSocketWrite(BasicNetworkHandler* handler)
{
	SOCKET sock = handler->GetSocket();
	struct epoll_event ev;
	ev.events = EPOLLIN | EPOLLOUT | EPOLLET;
	ev.data.ptr = (void *)handler;
	if (epoll_ctl(m_epfd, EPOLL_CTL_MOD, sock, &ev) == -1)
	{
		// 注册写失败
	}
}

void BasicNetwork::UnregisterSocketWrite(BasicNetworkHandler* handler)
{
	SOCKET sock = handler->GetSocket();
	struct epoll_event ev;
	ev.events = EPOLLIN | EPOLLET;
	ev.data.ptr = (void *)handler;
	if (epoll_ctl(m_epfd, EPOLL_CTL_MOD, sock, &ev) == -1)
	{
		// 反注册写失败
	}
}

void BasicNetwork::PollSocket(HandlerList *readhandler, HandlerList *writehandler)
{
	int eret = epoll_wait(m_epfd, m_tmp_event, MAX_EPOLL_SIZE, 10);
	if (eret > 0)
	{
		m_register_table_mutex.Lock();
		for (int i = 0; i < eret; ++i)
		{
			if (m_tmp_event[i].events & EPOLLIN)
			{
				readhandler->push_back((BasicNetworkHandler*)m_tmp_event[i].data.ptr);
			}
			if (m_tmp_event[i].events & EPOLLOUT)
			{
				writehandler->push_back((BasicNetworkHandler*)m_tmp_event[i].data.ptr);
			}
		}
		m_register_table_mutex.Unlock();
	}
}

#endif

