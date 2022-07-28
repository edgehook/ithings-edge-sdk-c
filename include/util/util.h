#ifndef __UTIL_H__
#define __UTIL_H__

#include <stdint.h>

void util_sleep(uint64_t milliseconds);
int64_t get_time(void);

//for uuid v4.
void gen_rand_uuid_str(char *uuid_str, int str_format);

char* util_strdup(const char* str);
void free_memory(char **str);
int util_strlen(char *str);
char* get_client_timestamp();
char* combine_strings(int strAmount, char *str1, ...);
char** string_split(const char* in, const char d);
void free_string_split_result(char** result);
int string_contain(const char *str, const char *sub_str);


#endif
