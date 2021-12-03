
#include <assert.h>
#include "memdef.h"
#include "memmonitor.h"

#ifdef MEMORY_MONITOR

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../platform/thread/mutex.h"

#endif

namespace memmonitor
{

#ifdef MEMORY_MONITOR
	
	#define MAX_RECORD_ITEM_COUNT 1024

	struct MEM_RECORD_ITEM
	{
		const char *name;
		long long	user_size;
		int			use_block_count;
		long long	alloc_size;
		int			alloc_block_count;
	};

	MEM_RECORD_ITEM		g_record_list[MAX_RECORD_ITEM_COUNT];
	int					g_cur_record_num;

	long long	g_use_size;
	int			g_use_block_count;
	long long	g_alloc_size;
	int			g_alloc_block_count;

	Mutex		g_monitor_lock;
#endif


#ifdef MEMORY_MONITOR

	int RegisterMemMoniter(const char *name)
	{
		assert(g_cur_record_num < MAX_RECORD_ITEM_COUNT);

		g_monitor_lock.Lock();
		for (int i = 0; i < g_cur_record_num; ++i)
		{
			if (strcmp(name, g_record_list[i].name) == 0)
			{
				g_monitor_lock.Unlock();
				return i;
			}
		}

		int index = g_cur_record_num++;
		g_record_list[index].name = name;

		g_monitor_lock.Unlock();

		return index;
	}

	void UseStat(int record_index, long long size, int block)
	{
		g_monitor_lock.Lock();
		
		g_record_list[record_index].user_size += size;
		g_record_list[record_index].use_block_count += block;

		g_use_size += size;
		g_use_block_count += block;
		g_monitor_lock.Unlock();
	}

	void AllocStat(int record_index, long long size, int block)
	{
		g_monitor_lock.Lock();

		g_record_list[record_index].alloc_size += size;
		g_record_list[record_index].alloc_block_count += block;

		g_alloc_size += size;
		g_alloc_block_count += block;
		g_monitor_lock.Unlock();
	}

#endif


	long long GetMemUse(int *block_num)
	{
		long long use_size = 0;
		*block_num = 0;

	#ifdef MEMORY_MONITOR
		g_monitor_lock.Lock();
		use_size = g_use_size;
		*block_num = g_use_block_count;
		g_monitor_lock.Unlock();
	#endif

		return use_size;
	}

	long long GetMemAlloc(int *block_num)
	{
		long long alloc_size = 0;
		*block_num = 0;

	#ifdef MEMORY_MONITOR
		g_monitor_lock.Lock();
		alloc_size = g_alloc_size;
		*block_num = g_alloc_block_count;
		g_monitor_lock.Unlock();
	#endif

		return alloc_size;
	}

	int PrintDyMemInfo(char *buff)
	{
		int buff_len = 0;
		buff[0] = 0;

		#ifdef MEMORY_MONITOR

		g_monitor_lock.Lock();
		buff_len += sprintf(buff + buff_len, "block:%d/%d size:%u/%u(KB) module:%d ",  g_use_block_count, g_alloc_block_count, (unsigned int)(g_use_size>>10), (unsigned int)(g_alloc_size>>10), g_cur_record_num);
		for (int i = 0; i < g_cur_record_num; ++i)
		{
			buff_len += sprintf(buff + buff_len, "%s[%d/%d %u/%u] ", g_record_list[i].name, g_record_list[i].use_block_count, g_record_list[i].alloc_block_count, (unsigned int)(g_record_list[i].user_size>>10), (unsigned int)(g_record_list[i].alloc_size>>10));
		}
		g_monitor_lock.Unlock();

		#endif

		return buff_len;
	}

}



