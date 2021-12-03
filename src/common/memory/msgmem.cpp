
#include "msgmem.h"
#include "clmempool.h"

namespace msgmempool
{
	class MsgMemPool: public CLMemPool 
	{
	public:
		MsgMemPool():CLMemPool("MsgMem")
		{
			unsigned int length_list[] = {16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 16384, 65536, 262144};
			this->Init(length_list, 12, 64);
		}
		virtual ~MsgMemPool()
		{
		}
	};

	MsgMemPool g_msg_mem_pool;
}


#ifdef _DEBUG
//#include <stdio.h>
namespace msgstat
{
	static int g_msg_malloc_count = 0;
}
#endif

void * MsgMem::operator new[](size_t c)
{
	#ifdef _DEBUG
	++msgstat::g_msg_malloc_count;
	//printf("MsgMem malloc: %d\n", msgstat::g_msg_malloc_count);
	#endif

	return msgmempool::g_msg_mem_pool.Alloc((unsigned int)c);
}

void MsgMem::operator delete[](void *m)
{
	#ifdef _DEBUG
	--msgstat::g_msg_malloc_count;
	//printf("MsgMem free: %d\n", msgstat::g_msg_malloc_count);
	#endif

	msgmempool::g_msg_mem_pool.Free(m);
}


