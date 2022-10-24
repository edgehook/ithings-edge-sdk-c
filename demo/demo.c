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

int start_device(devices_manager* mgr, char* dev_id);


int demo_on_connected(void* context){
	int ret;

retry_register:
	/*
	* 4. mqtt broker is connected. we register the protocol.
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

static int remove_device(devices_manager* mgr, demo_device* dev){
	return list_remove(mgr->devices, dev);
}

int add_device_from_device_meta(devices_manager* mgr, device_spec_meta* dev_spec){
	demo_device* dev = NULL;

	infof("Add device(%s).... \r\n", dev_spec->device_id);

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

/*
* Start device.
*/
int start_device(devices_manager* mgr, char* dev_id){
	demo_device* dev = find_device(mgr, dev_id);

	if(!dev) return -1;

	return start_demo_device(dev);
}

/*
* Stop device.
*/
int stop_device(devices_manager* mgr, char* dev_id){
	demo_device* dev = find_device(mgr, dev_id);

	if(!dev) return -1;

	return stop_demo_device(dev);
}

/*
* Delete device.
*/
int delete_device(devices_manager* mgr, char* dev_id){
	demo_device* dev = find_device(mgr, dev_id);

	if(!dev) return 0;

	remove_device(mgr, dev);
	destory_demo_device(dev);
	return 0;
}

int update_devices_from_device_meta(devices_manager* mgr, devices_spec_meta* devs_spec){
	int i = 0, ret = 0;
	demo_device* dev = NULL;

	for(i = 0; i < devs_spec->size; i++){
		device_spec_meta* dev_spec = &devs_spec->devices[i];

		dev = find_device(mgr, dev_spec->device_id);
		if(!dev){
			ret = add_device_from_device_meta(mgr, dev_spec);
			if(ret){
				infof("Add device(%s) failed \r\n", dev_spec->device_id);
			}
		}else{
			char* dev_id = dev_spec->device_id;

			delete_device(mgr, dev_id);
			ret = add_device_from_device_meta(mgr, dev_spec);
			if(ret){
				infof("update device(%s) failed \r\n", dev_spec->device_id);
			}else{
				start_device(mgr, dev_id);
			}
		}
	}

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
			infof("Start device(%s) .... \r\n", dev->device_id);
			ret = start_device(&dev_mgr, dev->device_id);
		}
	}else if(string_contain(action, DEVICE_LFCRTL_STOP)){
		device_spec_meta* dev = devs_spec->devices;
		if(dev){
			infof("Stop device(%s) .... \r\n", dev->device_id);
			ret = stop_device(&dev_mgr, dev->device_id);
		}
	}else if(string_contain(action, DEVICE_LFCRTL_UPDATE)){
		ret = update_devices_from_device_meta(&dev_mgr, devs_spec);
	}else if(string_contain(action, DEVICE_LFCRTL_DELETE)){
		device_spec_meta* dev = devs_spec->devices;
		if(dev){
			infof("Delete device(%s) .... \r\n", dev->device_id);
			ret = delete_device(&dev_mgr, dev->device_id);
		}
	}else{
		ret = -5;
	}

	destory_devices_spec_meta(devs_spec);
	return ret;
}

static int demo_update_desired_twins(device_desired_twins_update_msg* update_msg){
	infof("demo_update_desired_twins update twins \r\n");

	return 0;
}

static void demo_keep_alive(void){
	int ret, count, i = 0;
	list_node* current = NULL;
	demo_device* dev = NULL;
	device_status_msg* dev_msg;

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

	//send keepalive message.
	(void)send_keepalive_msg(&msg);

	free(msg.devices);
}

void main(void){
	int i, ret;

	dev_mgr.devices = list_init();

	infof(" ===========Start Main ==========\r\n");
	/*
	* 1. init mapper. 
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
	/* 2. Add setup  callback. */
	mapper_core_setup(demo_on_connected, demo_life_control,
				demo_update_desired_twins, demo_keep_alive);

retry_connect:
	/*3.  retry to connect the mqtt broker. */
	ret = mapper_core_connect();
	if(ret){
		errorf("mapper core init failed %d \r\n", ret);
		goto retry_connect;
	}
	infof("======  mapper connect successfully! =========\r\n");

	util_sleep_v2(1000);
retry_fetch:
	/*5. Fetch device metadat. */
	devs_spec = fetch_device_metadata();
	if(!devs_spec){
		warningf("fetch device failed\r\n");
		goto retry_fetch;
	}

	for(i = 0; i < devs_spec->size; i++){
		demo_device* dev = NULL;
		demo_device* tmp = NULL;
		device_spec_meta* dev_spec = &devs_spec->devices[i];

		/* 6. Add device and start device and relative thread.*/
		add_device_from_device_meta(&dev_mgr, dev_spec);
	}
	destory_devices_spec_meta(devs_spec);

	while(1) {
		util_sleep_v2(100000);
		//DO YOUR TASK.
	}
	mapper_core_exit();
	list_destory(dev_mgr.devices);
	infof(" ===========End Main ==========\r\n");
}
