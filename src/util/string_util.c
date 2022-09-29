#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#if defined(_WIN32) || defined(_WIN64)
	#include <windows.h>
#else
	#include <sys/time.h>
#endif

#include "util/util.h"

#define EVENT_TIME_LENGTH 		16
#define CLIENT_TIME_LENGTH 		10
#define LONG_LONG_MAX_LENGTH	20
#define SUB_STERING_MAX_LENGTH 	20

char* util_strdup(const char* str){
	if(str == NULL)
		return NULL;

	return strdup(str);
}

double* util_doubledup(double d){
	double* p = (double*)malloc(sizeof(double));	
	if(p) *p = d;
	return p;
}

int* util_intdup(int i){
	int* p = (int*)malloc(sizeof(int));
	if(p) *p = i;
	return p;
}

int util_strlen(char *str) {
	if (str == NULL) {
		return 0;
	}
	int len = 0;
	char *temp_str = str;
	while (*temp_str++ != '\0') {
		len++;
	}
	return len;
}

char* str_in_str(const char *_Str, const char *_SubStr) {
	if (_Str == NULL || _SubStr == NULL) {
		return NULL;
	}

	return strstr(_Str, _SubStr);
}

int string_contain(const char *str, const char *sub_str){
	return str_in_str(str, sub_str) != NULL;
}

size_t ConstStringLength(const char *_Str) {
	return strlen(_Str);
}

int String2Int(const char *value) {
	if (value == NULL) {
		return -1;
	}
	return atoi(value);
}

void* StrMemSet(void *_Dst, int _Val, size_t _Size) {
	memset(_Dst, _Val, _Size);
	return NULL;
}

void free_memory(char **str) {
	if (*str != NULL) {
		free(*str);
		*str = NULL;
	}
}

void StringMalloc(char **str, int length) {
	if (length <= 0) {
		return;
	}
	*str = malloc(length);
	if (*str == NULL) {
		return;
	}
	memset(*str, 0, length);
}

char* combine_strings(int strAmount, char *str1, ...) {
	int length = util_strlen(str1) + 1;
	if (length == 1) {
		return NULL;
	}

	char *result = malloc(length);
	if (result == NULL) {
		return NULL;
	}
	char *temStr;

	strcpy(result, str1);

	va_list args;
	va_start(args, str1);

	while (--strAmount > 0) {
		temStr = va_arg(args, char*);
		if (temStr == NULL) {
			continue;
		}
		length = length + util_strlen(temStr);
		result = realloc(result, length);
		if (result == NULL) {
			return NULL;
		}
		strcat(result, temStr);
	}
	va_end(args);

	return result;
}

/**NOTE: "*dst" will be "malloc" inside this function, and the invocation needs to free it after used.
 * If this function is recalled with the same "**dst", you should free the pointer "*dst" before invoking this function in case of memory leak.
 */
int copy_str_value(char **dst, const char *src, int length) {
	if (length <= 0) {
		return 0;
	}
	*dst = malloc(length + 1);
	if (*dst == NULL) {
		return -1;
	}
	StrMemSet(*dst, 0, length);
	strncat(*dst, src, length);
	return 0;
}

//unsigned  long long getTime() {
//	struct timeval tv;
//	gettimeofday(&tv,NULL);
//	return tv.tv_sec * 1000 + tv.tv_usec / 1000;
//}


//NOTE: the invocation need to free the return char pointer.
//return parameter e.g. 2019053101
char* get_client_timestamp() {
	time_t t;
	struct tm *lt;

	time(&t); //get Unix time stamp
	lt = gmtime(&t); //transform into time struct
	if (lt == NULL) {
		return NULL;
	}
	char *dest_str = malloc(CLIENT_TIME_LENGTH + 1); //length of yyyyMMDDhh + 1
	if (dest_str == NULL) {
		return NULL;
	} else {
		StrMemSet(dest_str, 0, CLIENT_TIME_LENGTH + 1);
		snprintf(dest_str, CLIENT_TIME_LENGTH + 1, "%d%.2d%.2d%.2d", lt->tm_year + 1900, lt->tm_mon + 1, lt->tm_mday, lt->tm_hour);
		return dest_str;
	}
}

/*
 * the max length of substring is SUB_STERING_MAX_LENGTH,
 * */
int GetSubStrIndex(const char *str, const char *substr) {
	if (str == NULL || substr == NULL) {
		return -1;
	}
	int len = (int)strlen(str);
	int subLen = (int)strlen(substr);
	if (len == 0 || substr == 0 || subLen >= SUB_STERING_MAX_LENGTH) {
		return -1;
	}
	int n = 0;
	char tmp[SUB_STERING_MAX_LENGTH] = { "" };

	while (len - n >= subLen) {
		strncpy(tmp, str + n, subLen);
		tmp[subLen] = '\0';
		if (strcmp(substr, tmp) == 0)
			return n;
		n++;
	}

	return -1;

}

char** string_split(const char* in, const char d){
	char** result = 0;
	int count = 0;
	char* last_delim = 0;
	char delim[2] = {0};
	char *s = util_strdup(in);
	char* tmp = s;
	
	delim[0] = d;
	delim[1] = 0;
		
	if (!s)
		return NULL;

	/* Count how many elements will be extracted. */
	while (*tmp) {
		if (d == *tmp) {
			count++;
			last_delim = tmp;
		}

		tmp++;
	}

	/* Add space for trailing token. */
    count += last_delim < (s + strlen(s) - 1);

	/* Add space for terminating null string so caller
	knows where the list of returned strings ends. */
	count++;

	result = malloc(sizeof(char*) * count);
	if (result) {
		int idx  = 0;
		char* token = strtok(s, delim);

		while (token) {
			*(result + idx++) = util_strdup(token);
			token = strtok(0, delim);
		}
		*(result + idx) = 0;
	}

	free(s);
	return result;
}

void free_string_split_result(char** result){
	int count = 0;

	if(result == NULL) return;

	while(result[count]){
		free_memory(&result[count]);
		count++;
	}

	free(result);
}

