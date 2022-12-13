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
#if defined(_WIN32) || defined(_WIN64)
	Sleep((DWORD)milliseconds);
#else
	struct timeval tv;

	if(!milliseconds) return;

	tv.tv_sec = milliseconds / 1000;
	tv.tv_usec = (milliseconds % 1000) * 1000; /* this field is microseconds! */
	(void)select(0, NULL, NULL, NULL, &tv);
#endif
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

LIBAPI uint64_t get_timestamp(void){
#if defined(_WIN32)
	FILETIME ft;
	long long t;

	GetSystemTimeAsFileTime(&ft);
	t = (((long long)ft.dwHighDateTime) << 32) + ft.dwLowDateTime;
	t -= 116444736000000000;
	t = t/10000;
	return t;
#else
	return time(NULL)*1000;
#endif	
}

LIBAPI void get_local_time(__time_info* info){
#if defined(_WIN32)
	SYSTEMTIME localSysTime;

	GetLocalTime(&localSysTime);
	if(info){
		info->Year= localSysTime.wYear;
		info->Month = localSysTime.wMonth;
		info->Day = localSysTime.wDay;
		info->Hour = localSysTime.wHour;
		info->Minute = localSysTime.wMinute;
		info->Second = localSysTime.wSecond;
		info->Milliseconds = localSysTime.wMilliseconds*1000;
	}
#else
	struct tm *ctm;
	struct timeval tv;

	gettimeofday(&tv, NULL);
	ctm = localtime(&tv.tv_sec);
	if(ctm && info){
		info->Year= ctm->tm_year+1900;
		info->Month = ctm->tm_mon+1;
		info->Day = ctm->tm_mday;
		info->DayOfWeek = ctm->tm_wday;
		info->Hour = ctm->tm_hour;
		info->Minute = ctm->tm_min;
		info->Second = ctm->tm_sec;
		info->Milliseconds = tv.tv_usec;
	}
#endif
}
