#ifndef EVENT_LISTENER_H
#define EVENT_LISTENER_H

#include "lib_api.h"
#include <util/thread.h>
#include <util/list.h>
#include <util/log.h>

typedef struct{
	char event_id[48];
	sem_type notify_sem;
	int timeout_ms;
	void* data;
	int length;
} event_listener; 

/*
* event listener manager.
*/
typedef struct{
	list*	el_queue;
	mutex_type mutex;
} el_manager;

LIBAPI el_manager* create_el_manager();
LIBAPI void match_event_and_dispatch(el_manager* elm, char* event_id, void* data, int length);
LIBAPI event_listener* register_event_listener(el_manager* elm, char* event_id, int timeout_ms);
LIBAPI void* watch_event(el_manager* elm, char* event_id, int timeout_ms);
LIBAPI void* wait_event(event_listener* el);
LIBAPI void unregister_event_listener(el_manager* elm, event_listener* el);
LIBAPI void destory_el_manager(el_manager* elm);
#endif