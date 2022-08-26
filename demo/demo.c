#include <stdlib.h>
#include <string.h>

#include <util/log.h>
#include <util/util.h>
#include <mapper_core.h>
#include "dev.h"

devices_status_message msg;
devices_spec_meta* devs_spec = NULL;

typedef struct {
	list* devices;
}devices_manager;

static devices_manager dev_mgr;

int start_demo_device(devices_manager* mgr);


int demo_on_connected(void* context){
	int ret;

retry_register:
	/*
	* If Apphub agent is restarted, then we need to register
	* the protocol again. Or, we can't communicate with apphub
	* agent.
	*/
	ret = register_protocol(NULL);
	if(ret){
		errorf("register mapper failed %d \r\n", ret);
		goto retry_register;
	}

	return 0;
}

static int compare_device_id(void* content, void* id_data){
	char* device_id = (char*)id_data;
	demo_device* dev = (demo_device*)content;

	if(!device_id || !dev) return 0;
	return strcmp(dev->device_id, device_id) == 0;
}

static demo_device* find_device(devices_manager* mgr, char* device_id){
	if(!device_id) return NULL;

	return list_find_v2(mgr->devices, device_id, compare_device_id);	
}

int add_device_from_device_meta(devices_manager* mgr, device_spec_meta* dev_spec){
	demo_device* dev = NULL;

	infof("device ID %s \r\n", dev_spec->device_id);
	infof("device service count %d \r\n", dev_spec->size);

	dev = find_device(mgr, dev_spec->device_id);
	if(dev){
		warningf("device(%s) already exist! \r\n", dev_spec->device_id);
		return -1;
	}

	dev = create_demo_device(dev_spec);
	if(!dev){
		warningf("create device(%s) failed \r\n", dev_spec->device_id);
		return -2;
	}

	list_push_tail(mgr->devices, dev, sizeof(*dev));
	infof("Add device(%s) successfully! \r\n", dev->device_id);

	if(string_contain(dev->device_state, "started")){
		return start_device(mgr, dev->device_id);
	}
	
	return 0;
}

int start_device(devices_manager* mgr, char* dev_id){
	demo_device* dev = find_device(mgr, dev_id);

	if(!dev) return -1;
	infof("Start device(%s) .... \r\n", dev_id);
	return start_demo_device(dev);
}

int stop_device(devices_manager* mgr, char* dev_id){
	demo_device* dev = find_device(mgr, dev_id);

	if(!dev) return -1;
	infof("Stop device(%s) .... \r\n", dev_id);
	return 0;
}


static int demo_life_control(char* action, devices_spec_meta* devs_spec){
	int ret = 0;

	if(!action || !devs_spec) return 1;

	infof("demo_life_control action =%s \r\n", action);
	if(string_contain(action, DEVICE_LFCRTL_CREATE)){
		ret = add_device_from_device_meta(&dev_mgr, devs_spec->devices);
	}else if(string_contain(action, DEVICE_LFCRTL_START)){
		device_spec_meta* dev = devs_spec->devices;
		if(dev){
			ret = start_device(&dev_mgr, dev->device_id);
		}
	}else if(string_contain(action, DEVICE_LFCRTL_STOP)){
		device_spec_meta* dev = devs_spec->devices;
		if(dev){
			ret = stop_device(&dev_mgr, dev->device_id);
		}
	}else if(string_contain(action, DEVICE_LFCRTL_UPDATE)){

	}else if(string_contain(action, DEVICE_LFCRTL_DELETE)){

	}

	destory_devices_spec_meta(devs_spec);
	return ret;
}

static int demo_update_desired_twins(device_desired_twins_update_msg* update_msg){
	infof("demo_update_desired_twins update twins \r\n");
}

static void demo_keep_alive(void){
	int ret, count, i = 0;
	list_node* current = NULL;
	demo_device* dev = NULL;
	device_status_msg* dev_msg;

	infof("demo keepalive \r\n");

	count = dev_mgr.devices->count;
	msg.size = count;
	msg.devices = (device_status_msg*)malloc(count*sizeof(device_status_msg));
	if(!msg.devices){
		errorf("Oops, no more system memory! \r\n");
		return;
	}

	while(list_next_node(dev_mgr.devices, &current) != NULL){
		dev = (demo_device*)current->content;
		if(!dev) continue;

		dev_msg = &msg.devices[i++];

		strncpy(dev_msg->device_id, dev->device_id, 47);
		strncpy(dev_msg->err_msg, "", 1);
	
		if(dev->online){
			strncpy(dev_msg->status, DEVICE_STATUS_ONLINE, 16);
		}else{
			strncpy(dev_msg->status, DEVICE_STATUS_OFFLINE, 16);
		}
	}
	msg.size = i;

	ret = send_keepalive_msg(&msg);
	if(ret){
		errorf("send_keepalive_msg msg failed %d\r\n", ret);
	}

	free(msg.devices);
}

void main(void){
	int i, ret;

	dev_mgr.devices = list_init();

	infof(" ===========Start Main ==========\r\n");
	/*
	* broker address: tcp://127.0.0.1:1884
	* usr: NULL
	* pwd: NULL
	* mapper_id: demo
	* thread_pool_size: 50
	* keepalive time: 5s
	*/
	ret = mapper_core_init("tcp://127.0.0.1:1884", NULL, NULL, "demo", 50, 5000);
	if(ret){
		errorf("mapper core init failed %d \r\n", ret);
		return ;
	}
	mapper_core_setup(demo_on_connected, demo_life_control,
				demo_update_desired_twins, demo_keep_alive);

retry_connect:
	ret = mapper_core_connect();
	if(ret){
		errorf("mapper core init failed %d \r\n", ret);
		goto retry_connect;
	}
	infof("======  mapper connect successfully! =========\r\n");

	util_sleep_v2(1000);
retry_fetch:
	devs_spec = fetch_device_metadata();
	if(!devs_spec){
		warningf("fetch device failed\r\n");
		goto retry_fetch;
	}

	infof("devs_spec->size = %d \r\n", devs_spec->size);
	for(i = 0; i < devs_spec->size; i++){
		demo_device* dev = NULL;
		demo_device* tmp = NULL;
		device_spec_meta* dev_spec = &devs_spec->devices[i];
	
		add_device_from_device_meta(&dev_mgr, dev_spec);
	}
	destory_devices_spec_meta(devs_spec);
	while(1) {
		util_sleep_v2(100000);
		//report property.
	}
	mapper_core_exit();
	list_destory(dev_mgr.devices);
	infof(" ===========End Main ==========\r\n");
}
