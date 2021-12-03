
#include <assert.h>
#include <stdlib.h>
#include <memory.h>
#include "memdef.h"
#include "clmempool.h"
#include "mempool.h"
#include "platform/thread/mutex.h"

#ifdef MEMORY_MONITOR
void *AllocFromMem(int record_index, size_t size);
void FreeToMem(int record_index, void *mem);
#endif

CLMemPool::CLMemPool(const char *name):m_length_list(0), m_pool_list(0), m_pool_num(0), m_record_index(-1), m_name(name)
{
}

CLMemPool::~CLMemPool()
{
	for (unsigned int i = 0; i < m_pool_num; ++i)
	{
		if (m_pool_list[i] != 0)
		{
			delete m_pool_list[i];
		}
	}

	::free(m_pool_list);
	::free(m_length_list);

	delete[] m_pool_lock;

	m_pool_list = 0;
	m_length_list = 0;
	m_pool_num = 0;
}

void CLMemPool::Init(unsigned int *length_list, unsigned int n, unsigned int increase)
{
	assert(n < 256);

	m_pool_num = n;
	m_length_list = (unsigned int*)malloc(sizeof(unsigned int) * m_pool_num);
	memcpy(m_length_list, length_list, sizeof(unsigned int) * m_pool_num);

	m_pool_list = (MemPool**)malloc(sizeof(MemPool*) * m_pool_num);
	memset(m_pool_list, 0, sizeof(MemPool*) * m_pool_num);

	for (unsigned int i = 0; i < m_pool_num; ++i)
	{
		// 每个内存池为实际使用内存长度加上sizeof(unsigned int)，前面的sizeof(unsigned int)字节用于存放所申
		// 请内存在该内存池列表中对应的下标
		m_pool_list[i] = new MemPool(m_length_list[i] + sizeof(unsigned int), increase, m_name);
	}

	m_pool_lock = new Mutex[n];
}

void * CLMemPool::Alloc(unsigned int size)
{
	unsigned int i = 0;
	while(i < m_pool_num && m_length_list[i] < size) ++i;
	
	char *mem = 0;
	if (i < m_pool_num)
	{
		m_pool_lock[i].Lock();
		mem = (char *)m_pool_list[i]->Alloc();
		m_pool_lock[i].Unlock();
	}
	else
	{
		// size过大, 直接从物理内存申请,同样多申请sizeof(unsigned int)个字节
		size_t alloc_size = size + sizeof(unsigned int);

		#ifdef MEMORY_MONITOR
		if (m_record_index == -1)
		{
			m_record_index = memmonitor::RegisterMemMoniter(m_name);
		}
		mem = (char *)AllocFromMem(m_record_index, alloc_size);
		#else
		mem = (char *)::malloc(alloc_size);
		#endif

		if (mem == 0) return 0;
	}

	// 前面sizeof(unsigned int)个字节存放下标
	unsigned int *index = (unsigned int *)mem;
	*index = i;

	return (mem + sizeof(unsigned int));
}

void CLMemPool::Free(void *mem) 
{
	// 取出前面sizeof(unsigned int)个字节转换为下标
	char *raw_mem = ((char *)mem) - sizeof(unsigned int);
	unsigned int index = *(unsigned int*)raw_mem;

	if (index == m_pool_num)
	{
		#ifdef MEMORY_MONITOR
		assert(m_record_index != -1);
		FreeToMem(m_record_index, raw_mem);
		#else
		::free(raw_mem);
		#endif
	}
	else
	{
		m_pool_lock[index].Lock();
		m_pool_list[index]->Free(raw_mem);
		m_pool_lock[index].Unlock();
	}
}

#ifdef MEMORY_MONITOR
void *AllocFromMem(int record_index, size_t size)
{
	size_t alloc_size = size + sizeof(unsigned int);
	char *mem = (char*)::malloc(alloc_size);
	memmonitor::AllocStat(record_index, alloc_size, 1);
	memmonitor::UseStat(record_index, alloc_size, 1);

	unsigned int *index = (unsigned int *)mem;
	*index = (unsigned int)alloc_size;

	return (mem + sizeof(unsigned int));
}

void FreeToMem(int record_index, void *mem)
{
	char *raw_mem = ((char *)mem) - sizeof(unsigned int);
	long long alloc_size = (long long)*(unsigned int*)raw_mem;
	memmonitor::AllocStat(record_index, -alloc_size, -1);
	memmonitor::UseStat(record_index, -alloc_size, -1);
	::free(raw_mem);
}
#endif
