
#ifndef MEMDEF_H
#define MEMDEF_H

#ifdef _DEBUG

#ifndef MEMORY_MONITOR
#define MEMORY_MONITOR

#endif

#endif

namespace memmonitor
{

#ifdef MEMORY_MONITOR
	int RegisterMemMoniter(const char *name);
	void UseStat(int record_index, long long size, int block);
	void AllocStat(int record_index, long long size, int block);
#endif

}



#endif



