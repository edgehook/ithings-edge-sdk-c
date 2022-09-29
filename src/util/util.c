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
#include <util/util.h>

LIBAPI void util_sleep(uint64_t milliseconds){
#if defined(_WIN32) || defined(_WIN64)
	Sleep((DWORD)milliseconds);
#else
	usleep((useconds_t)(milliseconds*1000));
#endif
}

LIBAPI void util_sleep_v2(long milliseconds){
	struct timeval tv;

	if(!milliseconds) return;

	tv.tv_sec = milliseconds / 1000;
	tv.tv_usec = (milliseconds % 1000) * 1000; /* this field is microseconds! */
	(void)select(0, NULL, NULL, NULL, &tv);
}


LIBAPI int64_t get_time(void){
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

LIBAPI long long get_timestamp(void){
#if defined(_WIN32)
	FILETIME ft;
	long long t;

	GetSystemTimeAsFileTime(&ft);
	t = (((long long)ft.dwHighDateTime) << 32) + ft.dwLowDateTime;
	t -= 116444736000000000;
	t = t/10000000;
	return t;
#else
	struct timeval tv;

	gettimeofday(&tv,NULL);

	return tv.tv_sec * 1000 + tv.tv_usec / 1000;
#endif	
}

