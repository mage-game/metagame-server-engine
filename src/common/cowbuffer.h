#pragma once

#ifndef COWBUFFER_H
#define COWBUFFER_H

/*
	COWBuffer - Copy-On-Write Buffer
	使用“写时拷贝”技术的Buffer类，仅提供简单的接口，当访问可能导致Buffer内容改变的接口时，内部会进行复制，否则多个相同的Buffer将使用同一份内存。
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

	/* 调用此函数后，原对象内容置空，用以尽可能减少多余复制的可能 */
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
