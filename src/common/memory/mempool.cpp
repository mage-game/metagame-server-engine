
#include <stdlib.h>
#include <memory.h>
#include <assert.h>
#include "mempool.h"
#include "memdef.h"

MemPool::MemPool(unsigned int alloc_size, unsigned int _increase, const char *class_name)
:m_size(alloc_size), m_size_for_increase(_increase), m_record_index(-1), m_name(class_name)
{

}

MemPool::~MemPool()
{
	for (int i = 0; i < (int)m_malloc_record.size(); ++i)
	{
		::free(m_malloc_record[i]);

		// 这里无法统计释放，不过由于内存池一般都是全局性的，所以，此处的统计也就变得无关紧要了

//#ifdef MEMORY_MONITOR
//		long long increase_size = m_size * m_size_for_increase;
//		memmonitor::AllocStat(-increase_size);
//#endif

	}
}

void * MemPool::Alloc()
{
	void *mem = 0;

	if(m_block_pool.size() == 0)
	{
		Increase();
		if(m_block_pool.size() == 0)
		{
			return 0;
		}
	}

	mem = m_block_pool.back();
	m_block_pool.pop_back();

#ifdef MEMORY_MONITOR
	if (m_record_index == -1)
	{
		m_record_index = memmonitor::RegisterMemMoniter(m_name);
	}
	memmonitor::UseStat(m_record_index, m_size, 1);
#endif

	return mem;
}

void MemPool::Free(void *mem)
{
	m_block_pool.push_back(mem);

#ifdef MEMORY_MONITOR
	assert(m_record_index != -1);
	long long un_size = m_size;
	memmonitor::UseStat(m_record_index, -un_size, -1);
#endif
}

void MemPool::Increase()
{
	size_t increase_size = m_size * m_size_for_increase;
	void *mem = ::malloc(increase_size);
	if (mem == 0) return ;

#ifdef MEMORY_MONITOR
	if (m_record_index == -1)
	{
		m_record_index = memmonitor::RegisterMemMoniter(m_name);
	}
	memmonitor::AllocStat(m_record_index, increase_size, m_size_for_increase);
#endif

	m_malloc_record.push_back(mem);
	char *p = (char *)mem;
	for (unsigned int i = 0; i < m_size_for_increase; ++i, p += m_size)
	{
		m_block_pool.push_back((void *)p);
	}
}
