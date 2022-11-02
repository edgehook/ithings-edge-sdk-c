#if !defined(_WIN32) && !defined(_WIN64)
#include <errno.h>
#include <unistd.h>
#include <sys/time.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <limits.h>
#endif
#include <stdlib.h>
#include <util/thread.h>

/**
 * Start a new thread
 * @param fn the function to run, must be of the correct signature
 * @param parameter pointer to the function parameter, can be NULL
 */
void Thread_start(thread_fn fn, void* parameter){
#if defined(_WIN32) || defined(_WIN64)
	thread_type thread = NULL;
#else
	thread_type thread = 0;
	pthread_attr_t attr;
#endif

#if defined(_WIN32) || defined(_WIN64)
	thread = CreateThread(NULL, 0, fn, parameter, 0, NULL);
    CloseHandle(thread);
#else
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	if (pthread_create(&thread, &attr, fn, parameter) != 0)
		thread = 0;
	pthread_attr_destroy(&attr);
#endif
}


/**
 * Create a new mutex
 * @param rc return code: 0 for success, negative otherwise
 * @return the new mutex
 */
int Thread_create_mutex(mutex_type* mutex){
	int rc = 0;

#if defined(_WIN32) || defined(_WIN64)
	*mutex = CreateMutex(NULL, 0, NULL);
	if(*mutex == NULL)
		rc = GetLastError();
#else
	*mutex = (mutex_type)malloc(sizeof(pthread_mutex_t));
	if (*mutex == NULL) return -1;
	rc = pthread_mutex_init(*mutex, NULL);
#endif

	return rc;
}


/**
 * Lock a mutex which has alrea
 * @return completion code, 0 is success
 */
int Thread_lock_mutex(mutex_type mutex){
	int rc = -1;

	/* don't add entry/exit trace points as the stack log uses mutexes - recursion beckons */
#if defined(_WIN32) || defined(_WIN64)
	/* WaitForSingleObject returns WAIT_OBJECT_0 (0), on success */
	rc = WaitForSingleObject(mutex, INFINITE);
#else
	rc = pthread_mutex_lock(mutex);
#endif

	return rc;
}


/**
 * Unlock a mutex which has already been locked
 * @param mutex the mutex
 * @return completion code, 0 is success
 */
int Thread_unlock_mutex(mutex_type mutex){
	int rc = -1;

	/* don't add entry/exit trace points as the stack log uses mutexes - recursion beckons */
#if defined(_WIN32) || defined(_WIN64)
	/* if ReleaseMutex fails, the return value is 0 */
	if (ReleaseMutex(mutex) == 0)
		rc = GetLastError();
	else
		rc = 0;
#else
		rc = pthread_mutex_unlock(mutex);
#endif

	return rc;
}


/**
 * Destroy a mutex which has already been created
 * @param mutex the mutex
 */
int Thread_destroy_mutex(mutex_type mutex){
	int rc = 0;

#if defined(_WIN32) || defined(_WIN64)
	rc = CloseHandle(mutex);
#else
	rc = pthread_mutex_destroy(mutex);
	free(mutex);
#endif

	return rc;
}


/**
 * Get the thread id of the thread from which this function is called
 * @return thread id, type varying according to OS
 */
thread_id_type Thread_getid(void){
#if defined(_WIN32) || defined(_WIN64)
	return GetCurrentThreadId();
#else
	return pthread_self();
#endif
}


/**
 * Create a new semaphore
 * @param rc return code: 0 for success, negative otherwise
 * @return the new condition variable
 */
int Thread_create_sem(sem_type* sem){
	int rc = -1;

#if defined(_WIN32) || defined(_WIN64)
	*sem = CreateEvent(
	        NULL,               /* default security attributes */
	        FALSE,              /* manual-reset event? */
	        FALSE,              /* initial state is nonsignaled */
	        NULL                /* object name */
	        );
	rc = (*sem == NULL) ? GetLastError() : 0;
#elif defined(OSX)
	*sem = dispatch_semaphore_create(0L);
	rc = (*sem == NULL) ? -1 : 0;
#else
	*sem = malloc(sizeof(sem_t));
	if (*sem)
		rc = sem_init(*sem, 0, 0);
#endif

	return rc;
}


/**
 * Wait for a semaphore to be posted, or timeout.
 * @param sem the semaphore
 * @param timeout the maximum time to wait, in milliseconds
 * @return completion code
 */
int Thread_wait_sem(sem_type sem, int timeout){
/* sem_timedwait is the obvious call to use, but seemed not to work on the Viper,
 * so I've used trywait in a loop instead. Ian Craggs 23/7/2010
 */
	int rc = -1;
#if !defined(_WIN32) && !defined(_WIN64) && !defined(OSX)
	int i = 0;
	useconds_t interval = 1000; /* 1000 microseconds: 1 milliseconds */
	int count = (1000 * timeout) / interval; /* how many intervals in timeout period */
	struct timespec ts;
#endif

#if defined(_WIN32) || defined(_WIN64)
	/* returns 0 (WAIT_OBJECT_0) on success, non-zero (WAIT_TIMEOUT) if timeout occurred */
	rc = WaitForSingleObject(sem, timeout < 0 ? 0 : timeout);
	if (rc == WAIT_TIMEOUT)
		rc = ETIMEDOUT;
#elif defined(OSX)
	/* returns 0 on success, non-zero if timeout occurred */
	rc = (int)dispatch_semaphore_wait(sem, dispatch_time(DISPATCH_TIME_NOW, (int64_t)timeout*1000000L));
	if (rc != 0)
		rc = ETIMEDOUT;
#else
	/* We have to use CLOCK_REALTIME rather than MONOTONIC for sem_timedwait interval.
	 * Does this make it susceptible to system clock changes?
	 * The intervals are small enough, and repeated, that I think it's not an issue.
	 */
	if (clock_gettime(CLOCK_REALTIME, &ts) == 0){
		ts.tv_sec += timeout;
		rc = sem_timedwait(sem, &ts);
	}else{
		while (i++ < count && (rc = sem_trywait(sem)) != 0)	{
			if (rc == -1 && ((rc = errno) != EAGAIN)){
				rc = 0;
				break;
			}
			usleep(interval); /* microseconds - .1 of a second */
		}
	}
#endif

 	return rc;
}


/**
 * Check to see if a semaphore has been posted, without waiting
 * The semaphore will be unchanged, if the return value is false.
 * The semaphore will have been decremented, if the return value is true.
 * @param sem the semaphore
 * @return 0 (false) or 1 (true)
 */
int Thread_check_sem(sem_type sem){
#if defined(_WIN32) || defined(_WIN64)
	/* if the return value is not 0, the semaphore will not have been decremented */
	return WaitForSingleObject(sem, 0) == WAIT_OBJECT_0;
#elif defined(OSX)
	/* if the return value is not 0, the semaphore will not have been decremented */
	return dispatch_semaphore_wait(sem, DISPATCH_TIME_NOW) == 0;
#else
	/* If the call was unsuccessful, the state of the semaphore shall be unchanged,
	 * and the function shall return a value of -1 */
	return sem_trywait(sem) == 0;
#endif
}

/**
 * Post a semaphore
 * @param sem the semaphore
 * @return 0 on success
 */
int Thread_post_sem(sem_type sem){
	int rc = 0;

#if defined(_WIN32) || defined(_WIN64)
	if (SetEvent(sem) == 0)
		rc = GetLastError();
#elif defined(OSX)
	rc = (int)dispatch_semaphore_signal(sem);
#else
	if (sem_post(sem) != 0)
		rc = errno;
#endif

 	return rc;
}

/**
 * Destroy a semaphore which has already been created
 * @param sem the semaphore
 */
int Thread_destroy_sem(sem_type sem){
	int rc = 0;

#if defined(_WIN32) || defined(_WIN64)
	rc = CloseHandle(sem);
#elif defined(OSX)
  dispatch_release(sem);
#else
	rc = sem_destroy(sem);
	free(sem);
#endif
	return rc;
}


#if !defined(_WIN32) && !defined(_WIN64)
/**
 * Create a new condition variable
 * @return the condition variable struct
 */
int Thread_create_cond(cond_type* condvar){
	int rc = 0;
	pthread_condattr_t attr;

	pthread_condattr_init(&attr);

#if 0
    /* in theory, a monotonic clock should be able to be used.  However on at least
     * one system reported, even though setclock() reported success, it didn't work.
     */
	if ((rc = pthread_condattr_setclock(&attr, CLOCK_MONOTONIC)) == 0)
		use_clock_monotonic = 1;
	else
		Log(LOG_ERROR, -1, "Error %d calling pthread_condattr_setclock(CLOCK_MONOTONIC)", rc);
#endif

	*condvar = (cond_type)malloc(sizeof(cond_type_struct));
	if (condvar){
		rc = pthread_cond_init(&(*condvar)->cond, &attr);
		rc = pthread_mutex_init(&(*condvar)->mutex, NULL);
	}

	return rc;
}

/**
 * Signal a condition variable
 * @return completion code
 */
int Thread_signal_cond(cond_type condvar){
	int rc = 0;

	pthread_mutex_lock(&condvar->mutex);
	rc = pthread_cond_signal(&condvar->cond);
	pthread_mutex_unlock(&condvar->mutex);

	return rc;
}

/**
 * Signal a condition variable
 * @return completion code
 */
int Thread_broadcast_cond(cond_type condvar){
	int rc = 0;

	pthread_mutex_lock(&condvar->mutex);
	rc = pthread_cond_broadcast(&condvar->cond);
	pthread_mutex_unlock(&condvar->mutex);

	return rc;
}

/**
 * Wait with a timeout (ms) for condition variable
 * @return 0 for success, ETIMEDOUT otherwise
 */
int Thread_wait_cond(cond_type condvar, int timeout_ms)
{
	int rc = 0;
	struct timespec cond_timeout;
	struct timespec interval;

	interval.tv_sec = timeout_ms / 1000;
	interval.tv_nsec = (timeout_ms % 1000) * 1000000L;

#if defined(__APPLE__) && __MAC_OS_X_VERSION_MIN_REQUIRED < 101200 /* for older versions of MacOS */
	struct timeval cur_time;
    gettimeofday(&cur_time, NULL);
    cond_timeout.tv_sec = cur_time.tv_sec;
    cond_timeout.tv_nsec = cur_time.tv_usec * 1000;
#else
	clock_gettime(CLOCK_REALTIME, &cond_timeout);
#endif

	cond_timeout.tv_sec += interval.tv_sec;
	cond_timeout.tv_nsec += (timeout_ms % 1000) * 1000000L;

	if (cond_timeout.tv_nsec >= 1000000000L)
	{
		cond_timeout.tv_sec++;
		cond_timeout.tv_nsec += (cond_timeout.tv_nsec - 1000000000L);
	}

	pthread_mutex_lock(&condvar->mutex);
	rc = pthread_cond_timedwait(&condvar->cond, &condvar->mutex, &cond_timeout);
	pthread_mutex_unlock(&condvar->mutex);

	return rc;
}

/**
 * Destroy a condition variable
 * @return completion code
 */
int Thread_destroy_cond(cond_type condvar)
{
	int rc = 0;

	rc = pthread_mutex_destroy(&condvar->mutex);
	rc = pthread_cond_destroy(&condvar->cond);
	free(condvar);

	return rc;
}
#endif
