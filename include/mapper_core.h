#ifndef MAPPER_CORE_H
#define MAPPER_CORE_H

#include "lib_api.h"
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

	/*
	* callback when mqtt connection is created.
	* user should call register_protocol in this  function.
	* success return 0, or return !0.
	*/
	int (*on_connected)(void* context);
	/*
	* device life control callback.
	* avaliable action has defined in the message.h and named
	* DEVICE_LFCRTL_*.  If this action is not supported, user should
	* return -5.
	* user should call free() to free the devs_spec by self.
	* success return 0, or return !0.
	* RETURN VALUE:
	* -5 : not support.
	*/
	int (*life_control)(char* action, devices_spec_meta* devs_spec);
	/*
	* set property value callback
	* user should call free() to free the update_msg by self.
	* success return 0, or return !0.
	*/
	int (*update_desired_twins)(device_desired_twins_update_msg* update_msg);
	/*
	* keep alive callback.
	* call it periodically, user should report all device's status in this 
	* function.
	*/
	void (*keep_alive)(void);
} mapper_core;

LIBAPI void core_init(void);
/*
* mapper core call back setup.
* on_connected: callback when mqtt connection is created.
* life_control:  device life control callback.
* update_desired_twins: set property value callback.
* keep_alive: keep alive callback.
*/
LIBAPI void mapper_core_setup(int (*on_connected)(void* context),
	int (*life_control)(char* action, devices_spec_meta* devs_spec),
	int (*update_desired_twins)(device_desired_twins_update_msg* update_msg),
	void (*keep_alive)(void));

/*
* mapper core init.
* user should call the function as mapper core init at firstly.
* this function just only do some initialize action, and not to
* connect the mqtt broker.
* svr_uri: mqtt broker uri
* usr: mqtt broker user name if exists, optional.
* pwd: mqtt broker password if exists. optional.
* mapper_id: mapper id for register, and communicate.
* pool_capacity: thread pool size(max thread we can start).
* keepalive_time: how long time we send the ping to apphub Agent.
*/
LIBAPI int mapper_core_init(char* svr_uri, char* usr, char* pwd,
			char* mapper_id, int pool_capacity, int keepalive_time);
/*
* connect the mqtt broker.
* user should ensure the connect successful.
* After connection is created, if the coonection is lost, 
* the mapper core will retry to connect automatically.
* when connect is created, on_connected call back will
* be called.
*/
LIBAPI int mapper_core_connect();
/*
* register protocol to Apphub agent and Ithings server.
* this function can call many times since if server consider a
* protocol has already regiosted, then return successfully.
* Notice,  the mapper lost connect and after connect successfuly agian,
* user should call this function to register to apphub agent. Or, the 
* apphub agent can't  understand the mapper's message.
*/
LIBAPI int register_protocol(char* spec);
/*
* When finshed register the protocol, user should call this function to get
* all device metadata from server. user should create these device according
* to the devices_spec_meta.
* When finished the creataion, user should start the device by the device's
* state.
*/
LIBAPI devices_spec_meta* fetch_device_metadata(void);
/*
* Send keepalive message to Apphub Agent.
* this message should contain all device's status.
* online/offline.
*/
LIBAPI int send_keepalive_msg(devices_status_message* msg);
/*
* send device report message to ithings server.
* this message will send into a blocked queue. msg should use
* heap memory to store(malloc). After finshed send, the mapper 
* core will free the msg automatically. it 's thread safety.
*/
LIBAPI int send_device_report_msg(device_report_msg* msg);
/*
* submit a task into thread pool and run it.
*/
LIBAPI int submit_task(void (*func)(void* arg), void* arg);

/*
* Do keep alive loop.
*/
LIBAPI void do_keep_alive_loop(void);

/*
* exit mapper core.
*/
LIBAPI void mapper_core_exit();

#endif
