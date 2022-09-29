#ifndef THREAD_POLL_H
#define THREAD_POLL_H

#include "lib_api.h"
#include <util/thread.h>
#include <util/list.h>
#include <util/blockedqueue.h>

typedef struct {
	void (*func)(void* arg);
	void* arg;
} task;

typedef struct {
	// capacity of the pool
	int 	capacity;
	// running is the number of the currently running threads.
	int		running;
	//we stopped the thread pool.
	int 	stopped;
	//workers is a array that store the available workers.
	list* avaliable_workers;
	//wait singal sem when no avaliable workers
	sem_type 	singal_sem;
	// thread pool mutex.
	mutex_type mutex;
} thread_pool;

typedef struct {
	//job queue. we just store 1 task into
	// the queue.
	blocked_queue*	task_queue;
	thread_pool* 	pool;
	int64_t			recycle_time;
} worker;

LIBAPI thread_pool* create_thread_pool(int capacity);
LIBAPI int submit(thread_pool* pool, void (*func)(void* arg), void* arg);
LIBAPI void destory_thread_pool(thread_pool* pool);
#endif