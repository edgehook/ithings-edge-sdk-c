#ifndef MAPPER_CORE_H
#define MAPPER_CORE_H

#include <device.h>
#include <util/thread_pool.h>
#include <util/event_listener.h>

typedef struct {
	int				keep_alive_time; //by ms
	int         	stopped;
	el_manager* 	el_mgr;
	thread_pool* 	th_pool;

	//callback.
	int (*start_up)(void* context);
	/*
	* user should call free() to free the devs_spec by self.
	*/
	int (*life_control)(char* action, devices_spec_meta* devs_spec);
	/*
	* user should call free() to free the update_msg by self.
	*/
	int (*update_desired_twins)(device_desired_twins_update_msg* update_msg);
	//callback  periodically
	void (*keep_alive)(void);
} mapper_core;

void core_init(void);
void mapper_core_setup(int keep_alive_time, int (*start_up)(void* context),
	int (*life_control)(char* action, devices_spec_meta* devs_spec),
	int (*update_desired_twins)(device_desired_twins_update_msg* update_msg),
	void (*keep_alive)(void));
int mapper_core_init(char* svr_uri, char* usr, char* pwd, char* mapper_id);
int register_protocol(char* mapper_id, char* spec);
devices_spec_meta* fetch_device_metadata(char* mapper_id);
void mapper_core_exit();

#endif
