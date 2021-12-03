

#include <time.h>

unsigned int GetDayID()
{
	struct
	{
		unsigned short year;
		unsigned char  mon;
		unsigned char  day;
	} now;

	time_t now_time = time(0);
	struct tm *p_tm = localtime(&now_time);
	now.year = (unsigned short)p_tm->tm_year;
	now.mon = (unsigned char)p_tm->tm_mon;
	now.day = (unsigned char)p_tm->tm_mday;
	return *(unsigned int*)&now;
}

