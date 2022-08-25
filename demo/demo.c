#include <stdlib.h>
#include <string.h>

#include <util/log.h>
#include <util/util.h>
#include <mapper_core.h>

device_spec_meta* dev_spec = NULL;
devices_spec_meta* devs_spec = NULL;

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

static int demo_life_control(char* action, devices_spec_meta* devs_spec){
	infof("demo_life_control action =%s \r\n", action);
}

static int demo_update_desired_twins(device_desired_twins_update_msg* update_msg){
	infof("demo_update_desired_twins update twins \r\n");
}

static void demo_keep_alive(void){
	int ret;
	device_status_msg dev;
	devices_status_message msg;

	if(!dev_spec || !devs_spec) return;
	if(devs_spec->size == 0) return;

	infof("demo keepalive \r\n");
	msg.size = 1;
	msg.devices = &dev;
	strncpy(dev.device_id, dev_spec->device_id, 47);
	strncpy(dev.status, DEVICE_STATUS_ONLINE, 16);
	strncpy(dev.err_msg, "", 1);

	ret = send_keepalive_msg(&msg);
	if(ret){
		errorf("send_keepalive_msg msg failed %d\r\n", ret);
	}
}

void main(void){
	int ret;

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
	dev_spec = &devs_spec->devices[0];
	infof("device ID %s \r\n", dev_spec->device_id);
	infof("device service count %d \r\n", dev_spec->size);

	while(1) util_sleep_v2(100000);
	mapper_core_exit();
	infof(" ===========End Main ==========\r\n");
}
