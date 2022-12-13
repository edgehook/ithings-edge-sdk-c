#ifndef __UTIL_H__
#define __UTIL_H__

#include "lib_api.h"
#include <stdint.h>

typedef struct {
	int Year;
	int Month;
	int DayOfWeek;
	int Day;
    int Hour;
	int Minute;
	int Second;
	int Milliseconds; //us
}__time_info;

LIBAPI void get_local_time(__time_info* info);
LIBAPI void util_sleep(uint64_t milliseconds);
LIBAPI void util_sleep_v2(long milliseconds);

LIBAPI int64_t get_time(void);
LIBAPI uint64_t get_timestamp(void);

//for uuid v4.
LIBAPI void gen_rand_uuid_str(char *uuid_str, int str_format);

LIBAPI char* util_strdup(const char* str);
LIBAPI int* util_intdup(int i);
LIBAPI double* util_doubledup(double d);
LIBAPI void free_memory(char **str);
LIBAPI int util_strlen(char *str);
LIBAPI char* get_client_timestamp();
LIBAPI char* combine_strings(int strAmount, char *str1, ...);
LIBAPI char** string_split(const char* in, const char d);
LIBAPI void free_string_split_result(char** result);
LIBAPI int string_contain(const char *str, const char *sub_str);


#endif
