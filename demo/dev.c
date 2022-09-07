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

static int create_demo_service(demo_device* dev, demo_service* svc, device_service_spec* svc_spec){
	int i, ret;

	memcpy(svc->name, svc_spec->name, 64);

	//property.
	if(svc_spec->properties_size > 0){
		svc->properties_size = svc_spec->properties_size;
		dev->prop_count += svc->properties_size;
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

static int decode_demo_dev_protoc_config(char* payload, int* it, int* timeout){
	char* tmp;
	char  a[48] = {0},b[48] = {0};
	cJSON* object = json_parse(payload);

	if(!object) return -1;

	tmp = json_get_string_from_object(object, "it_u");
	if(tmp){
		if(!strcmp(tmp,"h")){
			*it = json_get_int_from_object(object, "it", 0)*3600000;
		}else if(!strcmp(tmp,"m")){
			*it = json_get_int_from_object(object, "it", 0)*60000;
		}else if(!strcmp(tmp,"s")){
			*it = json_get_int_from_object(object, "it", 0)*1000;
		}else{
			if(it)
				*it = json_get_int_from_object(object, "it", 0);
		}
	}else{
		if(it)
			*it = json_get_int_from_object(object, "it", 0);
	}

	tmp = json_get_string_from_object(object, "timeout");
	if(tmp && timeout){
		sscanf(tmp, "%[0-9]%[a-Z]", b, a);
		sscanf(b, "%d", timeout);
		if(!strcmp(a,"h")){
			*timeout = *timeout*3600000;
		}else if(!strcmp(tmp,"m")){
			*timeout = *timeout*60000;
		}else if(!strcmp(tmp,"s")){
			*timeout = *timeout*1000;
		}
	}

	json_delete(object);
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

	decode_demo_dev_protoc_config(dev_spec->protocol, &dev->interval, &dev->timeout);
	dev->interval = dev->interval <= 0 ? 5000: dev->interval;
	Thread_create_mutex(&dev->mutex);
	if(!dev->mutex){
		free(dev);
		return dev;
	}

	//create services.
	dev->size = dev_spec->size;
	dev->online = 1;
	dev->services = (demo_service*)((char*)dev + sizeof(demo_device));

	//setup every service.
	for(i = 0; i < dev->size; i++){
		device_service_spec* svc_spec = &dev_spec->services[i];
		demo_service*	svc	= &dev->services[i];

		create_demo_service(dev, svc, svc_spec);
	}

	return dev;
}

/*
* we do a very simple fetch function.
*/
int do_dummy_fetch(char* path, char* parms){
	infof("path = %s, parms=%s \r\n", path, parms);

	return get_timestamp();
}

void do_fetch_props_and_report(demo_device* dev){
	int i = 0, k = 0;
	int size;
	device_report_msg* report_msg;
	devices_props_report_msg* devices_msg;
	report_device_props_msg*  dev_msg;

	if(dev->prop_count < 0) return;
	if(dev->size <= 0) return;

	report_msg = malloc(sizeof(device_report_msg));
	if(!report_msg) return;

	report_msg->events_report_msg = NULL;
	// create devices report message. 
	devices_msg = malloc(sizeof(devices_props_report_msg));
	if(!devices_msg) {
		free(report_msg);
		return;
	}
	// create device report message.
	size = sizeof(report_device_props_msg);
	size += dev->prop_count*sizeof(twin_property);
	dev_msg = malloc(size);
	if(!dev_msg){
		free(devices_msg);
		free(report_msg);
		return;
	}

	memset(dev_msg, 0, size);
	memcpy(dev_msg->device_id, dev->device_id, 48);
	dev_msg->size = dev->prop_count;
	dev_msg->twin_properties =(twin_property*)((char*)dev_msg + sizeof(report_device_props_msg));

	devices_msg->size = 1;
	devices_msg->devices = dev_msg;
	report_msg->props_report_msg = devices_msg;

	for(i = 0; i < dev->size; i++){
		int j, d;
		demo_service* svc = &dev->services[i];

		for(j = 0; j < svc->properties_size; j++){
			demo_property* prop = &svc->properties[j];
			twin_property* tprop = &dev_msg->twin_properties[k++];

			if(!dev->running) return;

			prop->twin.timestamp = get_timestamp();
			memcpy(tprop, &prop->twin, sizeof(twin_property));
			d = do_dummy_fetch(prop->path, prop->parms);

			if(string_contain(prop->twin.data_type, "int")){
				tprop->value = malloc(sizeof(int));
				if(tprop->value) *((int*)tprop->value) = d%100;
			}else if(string_contain(prop->twin.data_type, "float")){
				tprop->value = malloc(sizeof(float));
				if(tprop->value) *((float*)tprop->value) = (float)d;
			}else if(string_contain(prop->twin.data_type, "boolean")){
				tprop->value = malloc(sizeof(int));
				if(tprop->value) *((int*)tprop->value) = d%2;
			}else {
				tprop->value = malloc(sizeof(char)*64);
				if(tprop->value) sprintf((char*)tprop->value, "%d", d%100);
			}

			if(!tprop->value){
				strncpy(tprop->err_msg, "malloc failed!", 127);
			}else{
				tprop->err_msg[0] = 0;
			}
		}
	}

	//send report message.
	send_device_report_msg(report_msg);
}
void start_colloct_and_report_data(void* content){
	demo_device* dev = (demo_device*)content;

	if(!dev) return;

	while(1){
		do_fetch_props_and_report(dev);
		if(dev->interval <= 1000){
			if(!dev->running) {
				infof("device(%s) fetch and report exit \r\n", dev->device_id);
				dev->stopped = 1;
				return;
			}
			util_sleep_v2(dev->interval);
		}else{
			int i;

			for(i =0; i < dev->interval/1000; i++){
				if(!dev->running) {
					infof("device(%s) fetch and report exit \r\n", dev->device_id);
					dev->stopped = 1;
					return;
				}
				util_sleep_v2(1000);
			}
		}
	}
}

int start_demo_device(demo_device* dev){
	if(dev->size < 0){
		/* no service, no need to start */
		return -1;
	}
	if(dev->running){
		/* has already running.*/
		return 1;
	}
	Thread_lock_mutex(dev->mutex);
	dev->running = 1;
	dev->stopped = 0;
	submit_task(start_colloct_and_report_data, dev);
	Thread_unlock_mutex(dev->mutex);

	return 0;
}

int stop_demo_device(demo_device* dev){
	if(!dev->running) return 0;

	Thread_lock_mutex(dev->mutex);
	dev->running = 0;
retry_wait_stopped:
	if(!dev->stopped){
		util_sleep_v2(100);
		goto retry_wait_stopped;
	}
	Thread_unlock_mutex(dev->mutex);

	return 0;
}

void destory_demo_device(demo_device* dev){
	int ret;

	if(dev->running){
		stop_demo_device(dev);
	}

	Thread_destroy_mutex(dev->mutex);
	//free device.
	if(dev->services){
		int i;

		for(i = 0; i < dev->size; i++){
			demo_service* svc = &dev->services[i];

			if(svc->properties){
				int j = 0;

				for(j=0; j < svc->properties_size; j++){
					demo_property* prop = &svc->properties[j];

					if(prop->twin.value)
						free(prop->twin.value);
				}
				free(svc->properties);
			}
			if(svc->events)
				free(svc->events);
		}
	}
	free(dev);
}
