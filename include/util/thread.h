#if !defined(THREAD_H)
#define THREAD_H

#include "lib_api.h"

//define mutex_type
#if defined(_WIN32) || defined(_WIN64)
	#include <windows.h>
	#define mutex_type HANDLE
#else
	#include <pthread.h>
	#define mutex_type pthread_mutex_t*
#endif

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#define thread_type HANDLE
#define thread_id_type DWORD
#define thread_return_type DWORD
#define thread_fn LPTHREAD_START_ROUTINE
#define cond_type HANDLE
#define sem_type HANDLE
#undef ETIMEDOUT
#define ETIMEDOUT WSAETIMEDOUT
#else
#include <pthread.h>

#define thread_type pthread_t
#define thread_id_type pthread_t
#define thread_return_type void*
typedef thread_return_type (*thread_fn)(void*);
typedef struct { pthread_cond_t cond; pthread_mutex_t mutex; } cond_type_struct;
typedef cond_type_struct *cond_type;
#if defined(OSX)
  #include <dispatch/dispatch.h>
  typedef dispatch_semaphore_t sem_type;
#else
  #include <semaphore.h>
  typedef sem_t *sem_type;
#endif

LIBAPI int Thread_create_cond(cond_type* condvar);
LIBAPI int Thread_signal_cond(cond_type);
LIBAPI int Thread_broadcast_cond(cond_type condvar);
LIBAPI int Thread_wait_cond(cond_type condvar, int timeout);
LIBAPI int Thread_destroy_cond(cond_type);
#endif

LIBAPI void Thread_start(thread_fn, void*);

LIBAPI int Thread_create_mutex(mutex_type* mutex);
LIBAPI int Thread_lock_mutex(mutex_type);
LIBAPI int Thread_unlock_mutex(mutex_type);
LIBAPI int Thread_destroy_mutex(mutex_type);

LIBAPI thread_id_type Thread_getid();

LIBAPI int Thread_create_sem(sem_type* sem);
LIBAPI int Thread_wait_sem(sem_type sem, int timeout);
LIBAPI int Thread_check_sem(sem_type sem);
LIBAPI int Thread_post_sem(sem_type sem);
LIBAPI int Thread_destroy_sem(sem_type sem);
#endif
