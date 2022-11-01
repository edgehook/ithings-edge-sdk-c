#include <stdio.h>

#include <util/log.h>
#include <util/util.h>

typedef void (*PRINTF_LOG_CALLBACK_HANDLER)(int level, char *format, va_list args);
static PRINTF_LOG_CALLBACK_HANDLER log_callback = NULL;

void set_log_callback(PRINTF_LOG_CALLBACK_HANDLER cb) {
	log_callback = cb;
}

static void printf_log(int level, char * _format, va_list args){
	char *fmt = NULL;
	char *prefix = NULL;
	char times[68]={0};
	__time_info info;

	//get the local time.
	get_local_time(&info);
	snprintf(times, 64, "%d:%d:%d.%06d ",info.Hour, info.Minute, info.Second, info.Milliseconds);

	switch(level){
	case 0:
		prefix = "DEBUG:";
		break;
	case 1:
		prefix = "INFO:";
		break;
	case 2:
		prefix = "WARNING:";
		break;
	case 3:
		prefix = "ERROR:";
		break;
	case 4:
		prefix = "U:";
		break;
	}

	fmt = combine_strings(3, prefix, times, _format);

	if(log_callback){
		log_callback(level, fmt, args);
	}else {
		//we use the default vprintf.
		vprintf(fmt, args);
	}

	free_memory(&fmt);
}

void debugf(char * _format, ...){
	if(_format == NULL){
		return; 
	}

	va_list args;
	va_start(args, _format);

	printf_log(0, _format, args);

	va_end(args);
}

void infof(char * _format, ...){
	if(_format == NULL){
		return; 
	}

	va_list args;
	va_start(args, _format);

	printf_log(1, _format, args);

	va_end(args);
}

void warningf(char * _format, ...){
	if(_format == NULL){
		return; 
	}

	va_list args;
	va_start(args, _format);

	printf_log(2, _format, args);

	va_end(args);
}

void errorf(char * _format, ...){
	if(_format == NULL){
		return; 
	}

	va_list args;
	va_start(args, _format);

	printf_log(3, _format, args);

	va_end(args);
}
