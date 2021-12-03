
#include <vector>
#include "tcphandler.h"
#include "basicnetwork.h"
#include "jobrecv.h"
#include "jobdisconnect.h"
#include "common/memory/msgmem.h"

#define MIN_MSG_TO_WRITE_BUFF 2048

TcpHandler::TcpHandler(SOCKET socket, int max_package_size)
:BasicNetworkHandler(socket, HT_TCP), m_cur_read_byte(-(MsgLen)sizeof(LinkLayer)),
m_msg_to_write_buff(0), m_max_msg_to_write_buff_len(0), m_cur_msg_to_write_buff_len(0), 
m_buff(0), m_max_buff(0), m_cur_buff(0),
m_cur_write_byte(0),
m_max_package_size(max_package_size)
{
	m_cur_read_block.linklayer.len = 0;

	if (m_socket != SOCKET_ERROR)
	{
		// 设为非阻塞
		unsigned long ul = 1;
		PISocket::Ioctl(m_socket, FIONBIO, (unsigned long*)&ul);
		/*if (no_delay)
		{
			unsigned long enable = 1;
			PISocket::SetSockopt(m_socket, IPPROTO_TCP, TCP_NODELAY, (const char*)&enable, sizeof(unsigned long));
		}*/
	}

	//m_last_recv_time = PITime();

	m_msg_to_write_buff = new MsgMem[MIN_MSG_TO_WRITE_BUFF];
	m_max_msg_to_write_buff_len = MIN_MSG_TO_WRITE_BUFF;
	m_buff = new MsgMem[MIN_MSG_TO_WRITE_BUFF];
	m_max_buff = MIN_MSG_TO_WRITE_BUFF;
}


TcpHandler::~TcpHandler()
{
	OnClose();
}

#define MSG_READ_BUFF_LEN	2048

void TcpHandler::OnCanRead()
{
	if (m_socket == SOCKET_ERROR)
	{
		return;
	}

	char buff[MSG_READ_BUFF_LEN];

	for (;;)
	{
		int ret = PISocket::Recv(m_socket, buff, MSG_READ_BUFF_LEN, 0);
		if (ret <= 0)
		{
			if (ret == SOCKET_ERROR && PISocket::Errno() == EWOULDBLOCK)
			{
				// 读缓冲区空，等待下次可读时间
				return;
			}
			// 出错
			if (m_basic_network != 0)
			{
				m_basic_network->Remove(m_netid);
			}
			return;
		}

		int msg_no_read = ret;
		while (msg_no_read > 0)
		{
			if (m_cur_read_byte < 0)
			{
				int msg_left = -m_cur_read_byte;
				int msg_read = msg_no_read > msg_left ? msg_left : msg_no_read;
				memcpy((char *)&m_cur_read_block.linklayer + m_cur_read_byte + sizeof(LinkLayer), buff + ret - msg_no_read, msg_read);
				m_cur_read_byte += msg_read;
				msg_no_read -= msg_read;
				if (m_cur_read_byte < 0)
				{
					break;
				}

				if (m_cur_read_block.linklayer.len < 0 || m_cur_read_block.linklayer.len > m_max_package_size)
				{
					if (m_basic_network != 0)
					{
						m_basic_network->Remove(m_netid);
					}
					return ;
				}

				// 收完包长，创建缓冲区
				m_cur_read_block.message = new MsgMem[m_cur_read_block.linklayer.len];

				if (msg_no_read == 0) break;	// 消息全部读完
			}


			int msg_left = m_cur_read_block.linklayer.len - m_cur_read_byte;
			int msg_read = msg_no_read > msg_left ? msg_left : msg_no_read;
			memcpy(m_cur_read_block.message + m_cur_read_byte, buff + ret - msg_no_read, msg_read);
			m_cur_read_byte += msg_read;
			msg_no_read -= msg_read;

			if (m_cur_read_byte < m_cur_read_block.linklayer.len)
			{
				break;
			}

			if (m_basic_network != 0)
			{
				JobRecv *jobrecv = new JobRecv(m_netid, m_cur_read_block.message, m_cur_read_block.linklayer.len);
				m_basic_network->PushJob(jobrecv);
			}
			else
			{
				delete []m_cur_read_block.message;
			}

			// note:将内存交出，让上层处理，此处不做内存回收
			m_cur_read_block.linklayer.len = 0;
			m_cur_read_byte = -(MsgLen)sizeof(LinkLayer);
			m_cur_read_block.message = 0;
		}
	}
}

void TcpHandler::OnCanWrite()
{
	if (m_socket == SOCKET_ERROR)
	{
		return;
	}

	for (;;)
	{
		if (m_cur_msg_to_write_buff_len == 0)
		{
			m_buff_mutex.Lock();

			MsgMem *buff_tmp = m_buff;
			m_buff = m_msg_to_write_buff;
			m_msg_to_write_buff = buff_tmp;

			m_cur_msg_to_write_buff_len = m_cur_buff;
			m_cur_buff = 0;

			MsgLen max_tmp = m_max_buff;
			m_max_buff = m_max_msg_to_write_buff_len;
			m_max_msg_to_write_buff_len = max_tmp;

			m_buff_mutex.Unlock();


			if (m_cur_msg_to_write_buff_len == 0)
			{
				// 没有待写的块
				return;
			}

			if ( m_basic_network != 0 && m_cur_msg_to_write_buff_len > 1)
			{
				m_basic_network->UnregisterWrite(m_netid, m_cur_msg_to_write_buff_len - 1);
			}
			// 得到待写数据
		}

		while(m_cur_write_byte < m_cur_msg_to_write_buff_len)
		{
			int ret = PISocket::Send(m_socket, m_msg_to_write_buff + m_cur_write_byte, m_cur_msg_to_write_buff_len - m_cur_write_byte, 0);
			if (ret <= 0)
			{
				if (ret == SOCKET_ERROR && PISocket::Errno() == EWOULDBLOCK)
				{
					// 写缓冲区满，包内容未发完，等待下次可写时间
					// printf("remote socket buffer full \n"); fflush(stdout);
					return;
				}
				// 出错
				if (m_basic_network != 0)
				{
					m_basic_network->Remove(m_netid);
				}

				return;
			}
			m_cur_write_byte += ret;
		}

		if ( m_basic_network != 0 )
		{
			m_basic_network->UnregisterWrite(m_netid);
		}

		m_cur_msg_to_write_buff_len = 0;
		m_cur_write_byte = 0;

		int use_ratio = m_cur_write_byte / m_max_msg_to_write_buff_len * 100;
		if (use_ratio >= 50)
		{
			printf("use_ratio too much %d m_cur_write_byte:%d m_max_msg_to_write_buff_len:%d \n", use_ratio, m_cur_write_byte, m_max_msg_to_write_buff_len);
			fflush(stdout);
		}

		if (m_max_msg_to_write_buff_len >= 1 * 1024 * 1024)
		{
			printf("m_max_msg_to_write_buff_len >= 1 * 1024 * 1024 [so free :%d] \n", m_max_msg_to_write_buff_len); fflush(stdout);
			
			delete []m_msg_to_write_buff;
			m_max_msg_to_write_buff_len = 0;

			m_msg_to_write_buff = new MsgMem[MIN_MSG_TO_WRITE_BUFF];
			m_max_msg_to_write_buff_len = MIN_MSG_TO_WRITE_BUFF;
		}
	}
}


void TcpHandler::OnClose()
{
	if (m_socket != SOCKET_ERROR)
	{
		// 强制干掉一个socket的两种办法，不过暂时应该不用用到，先放着
		/*
		linger l;
		l.l_onoff = 1;
		l.l_linger = 0;
		int ret = PISocket::SetSockopt(m_socket, SOL_SOCKET, SO_LINGER, (const char *)&l, sizeof(linger));
		*/
		/*
		int b = 0;
		int ret = PISocket::SetSockopt(conn_socket, SOL_SOCKET, SO_DONTLINGER, (const char *)&b, sizeof(bool));
		*/

		PISocket::Shutdown(m_socket, PI_SD_BOTH);
		PISocket::Close(m_socket);
		
		if (m_basic_network != 0)
		{
			JobDisconect *jobdisconnect = new JobDisconect(m_netid);
			m_basic_network->PushJob(jobdisconnect);
		}

		m_socket = SOCKET_ERROR;

	}
	// 从此处开始m_socket已经无效


	if (m_msg_to_write_buff != 0)
	{
		delete []m_msg_to_write_buff;
		m_msg_to_write_buff = 0;

		m_max_msg_to_write_buff_len = 0;
		m_cur_msg_to_write_buff_len = 0;
	}

	if (m_buff != 0)
	{
		delete []m_buff;
		m_buff = 0;
		m_max_buff = 0;
		m_cur_buff = 0;
	}
	
	if (m_cur_read_block.message != 0)
	{
		delete []m_cur_read_block.message;
		m_cur_read_block.message = 0;
	}
	m_cur_read_block.linklayer.len = 0;
}


void TcpHandler::ReAllocBuff(MsgLen len)
{
	MsgMem *new_buff = new MsgMem[len];
	if (m_buff != 0)
	{
		memcpy(new_buff, m_buff, m_cur_buff);
		delete []m_buff;
	}
	m_buff = new_buff;
	m_max_buff = len;

	if (len > 4 * 1024 * 1024)
	{
		printf("netid:%d TcpHandler::ReAllocBuff len:%d \n", m_netid, len); fflush(stdout);
	}
}

