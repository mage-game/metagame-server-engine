#pragma once

#ifndef COWBUFFER_H
#define COWBUFFER_H

/*
	COWBuffer - Copy-On-Write Buffer
	ʹ�á�дʱ������������Buffer�࣬���ṩ�򵥵Ľӿڣ������ʿ��ܵ���Buffer���ݸı�Ľӿ�ʱ���ڲ�����и��ƣ���������ͬ��Buffer��ʹ��ͬһ���ڴ档
*/


#include "smartptr.h"
#include "buffer.h"


class COWBuffer
{
public:
	COWBuffer();
	explicit COWBuffer(unsigned long size);
	COWBuffer(const char *data, unsigned long size);
	COWBuffer(const COWBuffer& val);
	COWBuffer& operator=(const COWBuffer& val);
	~COWBuffer();

	/* ���ô˺�����ԭ���������ÿգ����Ծ����ܼ��ٶ��ิ�ƵĿ��� */
	COWBuffer give_up();

	char* ptr_write();
	const char* ptr()const;

	unsigned long size()const;
	void resize(unsigned long size);

protected:
	void copy();

	_SmartPtr<Buffer*> m_data;
};

#endif
