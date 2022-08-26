#ifndef MAPPER_CORE_H
#define MAPPER_CORE_H

#include <device.h>
#include <message.h>
#include <util/log.h>
#include <util/util.h>
#include <device.h>
#include <transport.h>
#include <util/thread_pool.h>
#include <util/event_listener.h>

#define DEVICE_STATUS_ONLINE  	"online"
#define DEVICE_STATUS_OFFLINE 	"offline"

typedef struct {
	int				keep_alive_time; //by ms
	int         	stopped;
	int				connected;
	char* 			mapper_id;		//mapper_id.
	el_manager* 	el_mgr;
	thread_pool* 	th_pool;
	blocked_queue*	report_msg_queue;

	//callback.
	int (*on_connected)(void* context);
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
void mapper_core_setup(int (*on_connected)(void* context),
	int (*life_control)(char* action, devices_spec_meta* devs_spec),
	int (*update_desired_twins)(device_desired_twins_update_msg* update_msg),
	void (*keep_alive)(void));
int mapper_core_init(char* svr_uri, char* usr, char* pwd,
			char* mapper_id, int pool_capacity, int keepalive_time);
int mapper_core_connect();
int register_protocol(char* spec);
devices_spec_meta* fetch_device_metadata(void);
int send_keepalive_msg(devices_status_message* msg);
void send_device_report_msg(device_report_msg* msg);
/*
* submit a task into thread pool and run it.
*/
int submit_task(void (*func)(void* arg), void* arg);
void mapper_core_exit();

#endif
