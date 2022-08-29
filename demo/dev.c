#include <stdlib.h>
#include <string.h>

#include <device.h>
#include <util/log.h>
#include <util/util.h>
#include <util/json_util.h>
#include <mapper_core.h>
#include "dev.h"

int decode_demo_prop_access_config(char* payload, char* path, char* parms){
	char* tmp = NULL;
	cJSON* object = json_parse(payload);

	if(!object)	return -1;
	tmp = json_get_string_from_object(object, "path");
	if(!tmp) {
		json_delete(object);
		return -1;
	}
	strncpy(path, tmp, 127);

	tmp = json_get_string_from_object(object, "parameter");
	if(tmp){
		strncpy(parms, tmp, 127);
	}

	json_delete(object);
	return 0;
}

static int create_demo_service(demo_service* svc, device_service_spec* svc_spec){
	int i, ret;

	memcpy(svc->name, svc_spec->name, 64);

	//property.
	if(svc_spec->properties_size > 0){
		svc->properties_size = svc_spec->properties_size;
		svc->properties = (demo_property*)malloc(svc->properties_size*sizeof(demo_property));
		if(svc->properties){
			for(i = 0; i < svc->properties_size; i++){
				demo_property* prop = &svc->properties[i];
				device_property_spec* prop_spec = &svc_spec->properties[i];

				memcpy(prop->twin.service, svc->name, 64);
				memcpy(prop->twin.property_name, prop_spec->name, 64);
				prop->twin.writeable = prop_spec->writeable;
				prop->twin.max = prop_spec->max;
				prop->twin.min = prop_spec->min;
				memcpy(prop->twin.unit, prop_spec->unit, 16);
				memcpy(prop->twin.data_type, prop_spec->data_type, 16);
				ret = decode_demo_prop_access_config(prop_spec->access_config,
					prop->path, prop->parms);
				if(ret){
					errorf("decode_demo_prop_access_config failed %d \r\n", ret);
				}
			}
		}
	}

	//event.
	if(svc_spec->events_size > 0){
		svc->events_size = svc_spec->events_size;
		svc->events = (demo_event*)malloc(svc->events_size*sizeof(demo_event));
		if(svc->events){
			for(i = 0; i < svc->events_size; i++){
				demo_event * evt = &svc->events[i];
				device_event_spec* evt_spec = &svc_spec->events[i];

				memcpy(evt->name, evt_spec->name, 64);
				memcpy(evt->data_type, evt_spec->data_type, 16);
				memcpy(evt->event_type, evt_spec->event_type, 64);
			}
		}
	}

	return 0;
}

demo_device* create_demo_device(device_spec_meta* dev_spec){
	int i, ret, size;
	demo_device* dev = NULL;

	if(!dev_spec) return NULL;

	if(dev_spec->size <= 0 || !dev_spec->services) return NULL;

	size = sizeof(demo_device);
	size += dev_spec->size * sizeof(demo_service);

	dev = (demo_device*)malloc(size);
	if(!dev) return NULL;

	memset(dev, 0, size);
	memcpy(dev->device_id, dev_spec->device_id, 48);
	memcpy(dev->device_os, dev_spec->device_os, 32);
	memcpy(dev->device_catagory, dev_spec->device_catagory, 32);
	memcpy(dev->device_id_code, dev_spec->device_id_code, 64);
	memcpy(dev->device_state, dev_spec->device_state, 32);
	memcpy(dev->tags, dev_spec->tags, 32);
	dev->size = dev_spec->size;
	dev->online = 1;
	dev->services = (demo_service*)((char*)dev + sizeof(demo_device));

	for(i = 0; i < dev->size; i++){
		device_service_spec* svc_spec = &dev_spec->services[i];
		demo_service*	svc	= &dev->services[i];

		create_demo_service(svc, svc_spec);
	}

	return dev;
}

void start_colloct_data(void* content){

}
void start_report_data(void* content){

}

int start_demo_device(demo_device* dev){

	submit_task(start_colloct_data, dev);
	submit_task(start_report_data, dev);
	return 0;
}
