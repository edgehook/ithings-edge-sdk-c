#include <stdlib.h>
#include <string.h>
#if !defined(_WINDOWS)
#define WINAPI
#else
#include <windows.h>
#endif

#include <util/log.h>
#include <util/util.h>
#include <device.h>
#include <common.h>
#include <transport.h>
#include <mapper_core.h>

static mapper_core mcore;

int register_protocol(char* spec){
	return mcore_register_protocol(&mcore, mcore.mapper_id, spec);
}

devices_spec_meta* fetch_device_metadata(void){
	return mcore_fetch_device_metadata(&mcore, mcore.mapper_id);
}

int send_keepalive_msg(devices_status_message* msg){
	return mcore_send_keepalive_msg(&mcore, mcore.mapper_id, msg);
}

static thread_return_type WINAPI do_process_response(void* context){
	response_msg* resp = NULL;
	mapper_core* core = (mapper_core*)context;

	if(!core || !core->el_mgr) return 0;

	while(1){
		if(core->stopped) return 0;
		resp = get_response_message();
		if(resp == NULL)
			continue;

		match_event_and_dispatch(core->el_mgr, resp->parent_msg_id, resp, sizeof(*resp));
	}
}

static void do_request(void* content){
	int ret = 0;
	mapper_core* core = &mcore;
	request_msg* req = (request_msg*)content;

	if(string_contain(req->operation, ITHINGS_OP_LIFE_CONTROL)){
		ret = mcore_do_life_control(core, req->resource, req->payload);
		if(ret){
			send_response(req, ITHINGS_RSP_INVALID_MSG, "error");
		}else{
			send_response(req, ITHINGS_RSP_SUCCEED, "{}");
		}
	}else if(string_contain(req->operation, ITHINGS_OP_SET_PROPERTY)){
		ret = mcore_do_set_properties(core, req->payload);
		if(ret){
			send_response(req, ITHINGS_RSP_INVALID_MSG, "update desired twins error");
		}else{
			send_response(req, ITHINGS_RSP_SUCCEED, "{}");
		}
	}else{
		send_response(req, ITHINGS_RSP_OPERATION_NOT_FOUND, "operation not found");
	}

	//we should free the request.
	free_request(&req);
}

static thread_return_type WINAPI do_process_requests(void* context){
	request_msg* req = NULL;
	mapper_core* core = (mapper_core*)context;

	if(!core || !core->th_pool) return 0;

	while(1){
		if(core->stopped) return 0;

		req = get_request_message();
		if(req == NULL) continue;

		submit(core->th_pool, do_request, req);
	}
}

static thread_return_type WINAPI do_keep_alive(void* context){
	mapper_core* core = (mapper_core*)context;

	while(1){
		if(core->stopped) return 0;

		if(core->keep_alive && core->connected)
			core->keep_alive();

		util_sleep_v2(core->keep_alive_time);
	}
}
static thread_return_type WINAPI do_device_report(void* context){
	int ret;
	device_report_msg* msg = NULL;
	mapper_core* core = (mapper_core*)context;

	while(1){
		if(core->stopped) return 0;

		msg = (device_report_msg*)blocked_queue_pop(core->report_msg_queue, 1000);
		if(!msg) continue;

		ret = mcore_do_device_report(core, msg);
		if(ret){
			errorf("[Mapper: %s] do_device_report Send Request failed: %d \r\n", core->mapper_id, ret);
		}
		destory_device_report_msg(msg);
	}
}

static void on_lost(void){
	mcore.connected = 0;
}

static void on_connected(void){
	mcore.connected = 1;
	if(mcore.on_connected)
		mcore.on_connected(&mcore);
}

void mapper_core_setup(int (*on_connected)(void* context),
	int (*life_control)(char* action, devices_spec_meta* devs_spec),
	int (*update_desired_twins)(device_desired_twins_update_msg* update_msg),
	void (*keep_alive)(void)){
	mcore.on_connected = on_connected;
	mcore.life_control = life_control;
	mcore.update_desired_twins = update_desired_twins;
	mcore.keep_alive = keep_alive;
}

int mapper_core_init(char* svr_uri, char* usr, char* pwd,
			char* mapper_id, int pool_capacity, int keepalive_time){
	int ret; 

	//transport init
	ret = transport_init(svr_uri, usr, pwd, mapper_id);
	if(ret){
		errorf("transport init failed with (%d) \r\n", ret);
		return ret;
	}

	//maaper core init.
	memset(&mcore, 0, sizeof(mcore));
	mcore.stopped = 0;
	mcore.connected = 0;
	mcore.mapper_id = mapper_id;
	mcore.keep_alive_time = keepalive_time;
	mcore.report_msg_queue = blocked_queue_init();
	if(!mcore.report_msg_queue){
		errorf("create report_msg_queue failed \r\n");
		transport_destory();
		return -1;
	}

	mcore.el_mgr = create_el_manager();
	if(!mcore.el_mgr){
		errorf("create event listener manager failed \r\n");
		transport_destory();
		blocked_queue_destory(mcore.report_msg_queue);
		return -1;
	}
	mcore.th_pool = create_thread_pool(pool_capacity);
	if(mcore.th_pool == NULL){
		errorf("create thread pool failed! \r\n");
		destory_el_manager(mcore.el_mgr);
		transport_destory();
		blocked_queue_destory(mcore.report_msg_queue);
		return -1;
	}

	//start process thread.
	Thread_start(do_process_response, &mcore);
	//start process request thread.
	Thread_start(do_process_requests, &mcore);
	//start device report thread.
	Thread_start(do_device_report, &mcore);
	//start keepalive thread.
	Thread_start(do_keep_alive, &mcore);

	return 0;
}

int mapper_core_connect(){
	return transport_connect(on_connected, on_lost);
}


void send_device_report_msg(device_report_msg* msg){
	if(!msg) return;

	blocked_queue_push(mcore.report_msg_queue, msg, sizeof(*msg));
}

void mapper_core_exit(){
	mcore.stopped = 1;
	destory_thread_pool(mcore.th_pool);
	transport_destory();
	destory_el_manager(mcore.el_mgr);
	blocked_queue_destory(mcore.report_msg_queue);
}
