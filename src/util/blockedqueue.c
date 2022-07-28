#include <stdlib.h>
#include <util/blockedqueue.h>
#include <util/thread.h>

blocked_queue* blocked_queue_init(void){
	blocked_queue* bqueue = (blocked_queue*)malloc(sizeof(blocked_queue));

	if(bqueue == NULL) return NULL;

	bqueue->queue = list_init();
	Thread_create_sem(&bqueue->bq_sem);

	return bqueue;
}

void blocked_queue_push(blocked_queue* bq, void* content, size_t size){
	list_push_tail(bq->queue, content, size);
	Thread_post_sem(bq->bq_sem);
}

void* blocked_queue_pop(blocked_queue* bq, int timeout_ms){
	//we will wait until some one push data into the queue.
	Thread_wait_sem(bq->bq_sem, timeout_ms);
	return list_pop_head(bq->queue);
}

void blocked_queue_destory(blocked_queue* bq){
	Thread_destroy_sem(bq->bq_sem);
	list_destory(bq->queue);
	free(bq);
}
