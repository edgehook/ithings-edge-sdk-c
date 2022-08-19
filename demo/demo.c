#include <stdlib.h>
#include <util/log.h>
#include <mapper_core.h>

void main(void){
	int ret;
	devices_spec_meta* devs_spec = NULL;
	device_spec_meta* dev_spec = NULL;

	infof(" ===========Start Main ==========\r\n");
	ret = mapper_core_init("tcp://127.0.0.1:1884", NULL, NULL, "modbus");
	if(ret){
		errorf("mapper core init failed %d \r\n", ret);
		return ;
	}

	ret = register_protocol("modbus", NULL);
	if(ret){
		errorf("register mapper failed %d \r\n", ret);
		return;
	}

retry_fetch:
	devs_spec = fetch_device_metadata("modbus");
	if(!devs_spec){
		warningf("fetch device failed\r\n");
		goto retry_fetch;
	}

	infof("devs_spec->size = %d \r\n", devs_spec->size);
	dev_spec = &devs_spec->devices[0];
	infof("device ID %s \r\n", dev_spec->device_id);
	infof("device service count %d \r\n", dev_spec->size);

	mapper_core_exit();
	infof(" ===========End Main ==========\r\n");
}
