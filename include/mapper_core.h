#ifndef MAPPER_CORE_H
#define MAPPER_CORE_H

#include <device.h>
#include <util/thread_pool.h>
#include <util/event_listener.h>

typedef struct {
	int         	stopped;
	el_manager* 	el_mgr;
	thread_pool* 	th_pool;

	//callback.
	int (*start_up)(void* context);
	int (*life_control)(char* action, char* payload);
	int (*update_desired_twins)(char* device_id, twin_property* desired_twins);
} mapper_core;

void core_init(void);
int mapper_core_init(char* svr_uri, char* usr, char* pwd, char* mapper_id);
int register_protocol(char* mapper_id, char* spec);
device_spec_meta* fetch_device_metadata(char* mapper_id);
void mapper_core_exit();

#endif
