
#ifdef __unix
#include <unistd.h>
#include <sys/time.h>
#elif defined(_WIN32)
#include <windows.h>
#endif

void PISleep(unsigned long timems)
{
	#ifdef __unix 	// usleep( time * 1000 );
		usleep((timems << 10) - (timems << 4) - (timems << 3));
	#elif defined(_WIN32)
		Sleep(timems);
	#endif
}

unsigned long PITime()
{
	#ifdef __unix
		timezone tz={0, 0};
		timeval time;
		gettimeofday(&time,&tz);
		return (time.tv_sec*1000+time.tv_usec/1000);
	#elif defined(_WIN32)
		return GetTickCount();
	#endif
}

