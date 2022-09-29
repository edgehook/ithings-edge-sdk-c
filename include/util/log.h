#ifndef _UTIL_LOG_H_
#define _UTIL_LOG_H_

#include "lib_api.h"
#include "stdarg.h"

typedef void (*PRINTF_LOG_CALLBACK_HANDLER)(int level, char *format, va_list args);

/*
* set log callback.
*/
LIBAPI void set_log_callback(PRINTF_LOG_CALLBACK_HANDLER cb);
//debug
LIBAPI void debugf(char * _format, ...);
//info
LIBAPI void infof(char * _format, ...);
//warning
LIBAPI void warningf(char * _format, ...);
//error.
LIBAPI void errorf(char * _format, ...);

#endif
