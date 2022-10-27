#include <string.h>
#include <stdlib.h>

#include <util/event_listener.h>

static event_listener* create_event_listener (char* event_id, int timeout_ms){
	event_listener* el = malloc(sizeof(event_listener));

	if(el == NULL || event_id == NULL) return NULL;

	memset(el, 0, sizeof(event_listener));
	strncpy(el->event_id, event_id, 47);
	Thread_create_sem(&el->notify_sem);
	el->timeout_ms = timeout_ms;

	if(el->notify_sem == NULL){
		free(el);
		return NULL;
	}

	return el;
}

static void send_event_notify(event_listener* el, void* data, int length){
	if(el == NULL) return;

	el->data = data;
	el->length = length;
	Thread_post_sem(el->notify_sem);
}

/*
* wait for event 
* If return NULL, then is timeout, Or will return response
* data.
*/
void* wait_event(event_listener* el){
	if(el == NULL) return NULL;

	Thread_wait_sem(el->notify_sem, el->timeout_ms);

	return el->data;
}

static void destory_event_listener(event_listener* el){
	Thread_destroy_sem(el->notify_sem);
	el->notify_sem = NULL;
	free(el);
}

el_manager* create_el_manager(){
	el_manager* elm = malloc(sizeof(el_manager));

	if(elm == NULL) return NULL;

	elm->el_queue = list_init();
	if(elm->el_queue == NULL){
		free(elm);
		return NULL;
	}

	Thread_create_mutex(&elm->mutex);
	if(!elm->mutex){
		list_destory(elm->el_queue);
		free(elm);
		return NULL;
	}

	return elm;
}

static int compare_event_id(void* content, void* id_data){
	char* event_id = (char*)id_data;
	event_listener* el = (event_listener*)content;

	if(event_id == NULL || el == NULL) return 0;
	return strcmp(el->event_id, event_id) == 0;
}

static event_listener* get_event_listener(el_manager* elm, char* event_id){
	if(elm == NULL || event_id == NULL) return NULL;

	return (event_listener*)list_find_v2(elm->el_queue, event_id, compare_event_id);
}

static int put_event_listener(el_manager* elm, event_listener* el){
	char* event_id;

	if(elm == NULL || el == NULL) return -1;

	event_id = el->event_id;
	if(get_event_listener(elm, event_id)){
		//it has already exists.
		return -5;
	}

	list_push_tail(elm->el_queue, el, sizeof(*el));

	return 0;
}

static void delete_event_listener(el_manager* elm, event_listener* el){
	if(elm == NULL || el == NULL) return;

	list_remove(elm->el_queue, el);
	destory_event_listener(el);
}

//register the event listener.
event_listener* register_event_listener(el_manager* elm, char* event_id, int timeout_ms){
	int ret;
	event_listener* el = NULL;

	Thread_lock_mutex(elm->mutex);
	el = create_event_listener (event_id, timeout_ms);
	ret = put_event_listener(elm, el);
	Thread_unlock_mutex(elm->mutex);
	if(ret) return NULL;

	return el;	
}

//unregister the event listener.
void unregister_event_listener(el_manager* elm, event_listener* el){
	Thread_lock_mutex(elm->mutex);
	delete_event_listener(elm, el);
	Thread_unlock_mutex(elm->mutex);
}

void match_event_and_dispatch(el_manager* elm, char* event_id, void* data, int length){
	event_listener* el= NULL;
	
	if(event_id == NULL || elm == NULL) return;

	el = get_event_listener(elm, event_id);
	if(!el){
		//no such event listener, we return.
		return;
	}

	send_event_notify(el, data, length);
}

void* watch_event(el_manager* elm, char* event_id, int timeout_ms){
	void* d = NULL;
	event_listener* el = register_event_listener(elm, event_id, timeout_ms);

	if(!el){
		errorf("register event Id(%s) failed with\r\n", event_id);
		return NULL;
	}

	d = wait_event(el);
	unregister_event_listener(elm, el);
	return d;
}
void destory_el_manager(el_manager* elm){
	if(elm == NULL) return;

	list_destory(elm->el_queue);
	free(elm);
} 
