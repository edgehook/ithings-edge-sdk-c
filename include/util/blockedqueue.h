#ifndef _BLOCKED_QUEUE_H
#define _BLOCKED_QUEUE_H

#include<stdlib.h>
#include<util/list.h>

typedef struct {
	list*		queue;
	sem_type 	bq_sem;
}blocked_queue;

blocked_queue* blocked_queue_init(void);
void blocked_queue_push(blocked_queue* bq, void* content, size_t size);
void* blocked_queue_pop(blocked_queue* bq, int timeout_ms);
void blocked_queue_destory(blocked_queue* bq);
#endif
