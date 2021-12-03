
#pragma once

#ifndef TCPHANDLER_H
#define TCPHANDLER_H

#include <memory.h>

#include "def.h"
#include "basicnetworkhandler.h"
#include "common/platform/thread/mutex.h"

class MsgMem;

struct LinkLayer
{
	LinkLayer():len(0){}
	MsgLen			len;
};

struct MessageBlock
{
	MessageBlock():message(0),linklayer(){}
	MsgMem		*message;
	LinkLayer	linklayer;
};

class TcpHandler : public BasicNetworkHandler
{
public:
	TcpHandler(SOCKET socket, int max_package_size);
	~TcpHandler();

	void OnCanRead();
	void OnCanWrite();
	void OnClose();

protected:
	friend class BasicNetwork;
	bool SendPackage(const char *buffer, unsigned int len)
	{
		m_buff_mutex.Lock();

		MsgLen total_len = m_cur_buff + len + sizeof(LinkLayer);
		while (total_len > m_max_buff)
		{
			if (total_len > 16 * 1024 * 1024)		// FOR DEBUG !!
			{
				m_buff_mutex.Unlock();
				return false;
			}

			ReAllocBuff(static_cast<int>(m_max_buff * 1.5));
		}

		char *cur_p = (char *)m_buff + m_cur_buff;
		LinkLayer *link = (LinkLayer*)cur_p;
		link->len = len;
		memcpy(cur_p + sizeof(LinkLayer), buffer, len);
		m_cur_buff = total_len;

		m_buff_mutex.Unlock();

		return true;
	}

private:
	TcpHandler(const TcpHandler&);
	TcpHandler& operator=(const TcpHandler&);

	
	MessageBlock m_cur_read_block;
	MsgLen m_cur_read_byte;
	
	MsgMem		*m_msg_to_write_buff;
	MsgLen		m_max_msg_to_write_buff_len;	// m_msg_buff 的内存长度
	MsgLen		m_cur_msg_to_write_buff_len; // m_msg_buff 中的数据长度

	MsgMem		*m_buff;
	MsgLen		m_max_buff;
	MsgLen		m_cur_buff;
	void		ReAllocBuff(MsgLen len);
	Mutex		m_buff_mutex;

	MsgLen m_cur_write_byte;		// 已写长度

	int m_max_package_size;			// 一个包超过该大小则认为客户端不可靠，果断地断开它

	//unsigned long m_last_recv_time;
};


#endif
