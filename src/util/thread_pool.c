#include <stdlib.h>
#include <string.h>
#if !defined(_WINDOWS)
#define WINAPI
#else
#include <windows.h>
#endif

#include <util/util.h>
#include <util/log.h>
#include <util/thread_pool.h>

thread_pool* create_thread_pool(int capacity){
	thread_pool* pool = (thread_pool*)malloc(sizeof(thread_pool));

	if(!pool) return NULL;

	memset(pool, 0, sizeof(thread_pool));
	pool->capacity = capacity;
	pool->avaliable_workers = list_init();
	if(!pool->avaliable_workers) {
		free(pool);
		return NULL;
	}

	Thread_create_mutex(&pool->mutex);
	Thread_create_sem(&pool->singal_sem);
	if(!pool->singal_sem){
		free(pool);
		list_destory(pool->avaliable_workers);
		return NULL;
	}

	return pool;
}

static void pool_add_runing(thread_pool* pool, int delta){
	Thread_lock_mutex(pool->mutex);
	pool->running += delta;
	Thread_unlock_mutex(pool->mutex);
}

static worker* create_worker(thread_pool* pool){
	worker* w = (worker*)malloc(sizeof(worker));

	if(pool == NULL || w == NULL) return NULL;

	w->pool = pool;
	w->task_queue = blocked_queue_init();
	if(!w->task_queue){
		free(w);
		return NULL;
	}

	return w;
}

static void destory_worker(worker* w){
	if(w == NULL) return;

	blocked_queue_destory(w->task_queue);
	free(w);
}

static int revert_worker_avaliable(thread_pool* pool, worker* w){
	w->recycle_time = get_time();
	if(list_find_v2(pool->avaliable_workers, w, NULL) == NULL){
		list_push_tail(pool->avaliable_workers, w, sizeof(*w));
		if(pool->running >= pool->capacity)
			Thread_post_sem(pool->singal_sem);
	}

	return 1;
}

static thread_return_type WINAPI do_work_run(void* context){
	task* tsk = NULL;
	worker* w = (worker*)context;
	thread_pool* pool = w->pool;

	while(1){
		if(pool->stopped) break;

		tsk = (task*)blocked_queue_pop(w->task_queue, 600000);
		if(tsk == NULL){
			infof("no avaliable task, we exit the worker.... \r\n");
			break;
		}

		if(tsk->func){
			tsk->func(tsk->arg);
		}

		free(tsk);
		//put the worker into pool->avaliable_workers.
		revert_worker_avaliable(w->pool, w);
	}

	//remove the woker from avaliable_workers.
	list_remove(w->pool->avaliable_workers, w);
	pool_add_runing(w->pool, -1);
	destory_worker(w);
	if(pool->running >= pool->capacity)
		Thread_post_sem(pool->singal_sem);
	return 0;
}

static void add_task(worker* w, void (*func)(void* arg), void* arg){
	task* tsk = NULL;

	tsk = (task*)malloc(sizeof(task));
	if(!tsk) {
		errorf("no more system memory! \r\n");
		return;
	}

	memset(tsk, 0, sizeof(task));
	tsk->func = func;
	tsk->arg = arg;
	blocked_queue_push(w->task_queue, tsk, sizeof(task));
}

static void worker_run(worker* w){
	pool_add_runing(w->pool, 1);
	Thread_start(do_work_run, w);
}

static int is_closed(thread_pool* pool){
	return pool->stopped == 1;
}

static worker* spawn_worker(thread_pool* pool){
	worker* w = create_worker(pool);

	if(w == NULL) {
		errorf("create worker failed \r\n");
		return NULL;
	}

	worker_run(w);

	return w;
}

static worker* retrieve_worker(thread_pool* pool){
	worker* w = NULL;

	if(pool == NULL) return NULL;

	w = (worker*)list_pop_head(pool->avaliable_workers);
	if(w){
		return w;
	}

	if(pool->capacity <= 0 || pool->capacity > pool->running){
		w = spawn_worker(pool);
	}else{
retry_retrieve:
		if(pool->running == 0){
			w = spawn_worker(pool);
		}else{
			//reachout the capacity, we just only to wait.
			Thread_wait_sem(pool->singal_sem, 1000000);
			w = (worker*)list_pop_head(pool->avaliable_workers);
			if(w == NULL){
				goto retry_retrieve;
			}
		}
	}

	return w;
}

// Submit submits a task to this pool.
int submit(thread_pool* pool, void (*func)(void* arg), void* arg){
	worker* w = NULL;

	if(pool == NULL || func == NULL) return -1;
	if(pool->stopped) return 0;

	w = retrieve_worker(pool);
	if(!w){
		errorf("retrieve worker failed! \r\n");
		return -2;
	}

	add_task(w, func, arg);
	return 0;
}
void destory_thread_pool(thread_pool* pool){
	if(pool == NULL) return;

	pool->stopped = 1;
	while(pool->running);
	Thread_destroy_mutex(pool->mutex);
	list_destory(pool->avaliable_workers);
	Thread_destroy_sem(pool->singal_sem);
	free(pool);
}
