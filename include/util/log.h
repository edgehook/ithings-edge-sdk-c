#ifndef _UTIL_LOG_H_
#define _UTIL_LOG_H_

#include "stdarg.h"

typedef void (*PRINTF_LOG_CALLBACK_HANDLER)(int level, char *format, va_list args);

/*
* set log callback.
*/
void set_log_callback(PRINTF_LOG_CALLBACK_HANDLER cb);
//debug
void debugf(char * _format, ...);
//info
void infof(char * _format, ...);
//warning
void warningf(char * _format, ...);
//error.
void errorf(char * _format, ...);

#endif
