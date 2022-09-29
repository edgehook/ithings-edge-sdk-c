#ifndef _BLOCKED_QUEUE_H
#define _BLOCKED_QUEUE_H

#include "lib_api.h"
#include<stdlib.h>
#include<util/list.h>

typedef struct {
	list*		queue;
	sem_type 	bq_sem;
}blocked_queue;

LIBAPI blocked_queue* blocked_queue_init(void);
LIBAPI void blocked_queue_push(blocked_queue* bq, void* content, size_t size);
LIBAPI void* blocked_queue_pop(blocked_queue* bq, int timeout_ms);
LIBAPI void blocked_queue_destory(blocked_queue* bq);
#endif
