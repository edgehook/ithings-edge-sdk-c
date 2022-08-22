#include <stdint.h>
#include <time.h>
#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#else
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h>
#endif

void util_sleep(uint64_t milliseconds){
#if defined(_WIN32) || defined(_WIN64)
	Sleep((DWORD)milliseconds);
#else
	usleep((useconds_t)(milliseconds*1000));
#endif
}

void util_sleep_v2(long milliseconds){
	struct timeval tv;

	if(!milliseconds) return;

	tv.tv_sec = milliseconds / 1000;
	tv.tv_usec = (milliseconds % 1000) * 1000; /* this field is microseconds! */
	(void)select(0, NULL, NULL, NULL, &tv);
}


int64_t get_time(void){
#if defined(_WIN32)
	FILETIME ft;

	GetSystemTimeAsFileTime(&ft);
	return ((((int64_t) ft.dwHighDateTime) << 8) + ft.dwLowDateTime) / 10000;
#else
	struct timespec ts;

	clock_gettime(CLOCK_REALTIME, &ts);
	return ((int64_t) ts.tv_sec * 1000) + ((int64_t) ts.tv_nsec / 1000000);
#endif
}

