#ifndef __DEV_H__
#define __DEV_H__

#include <device.h>

typedef struct{
	twin_property twin;
	char  path[128];
	char  parms[128];
}demo_property;

typedef struct{
	char name[64];
	char  data_type[16];
	char  event_type[16];
	void* value;
}demo_event;

typedef struct{
	char name[64];
	//list all device_property_spec
	int properties_size;
	demo_property* properties;
	//list all device_event_spec
	int events_size;
	demo_event* events;
}demo_service;

typedef struct {
	char device_id[48];
	char device_os[32];
	char device_catagory[32];
	char device_id_code[64];
	char device_state[32];
	char tags[128];
	int online;
	int size;	/* service count */
	int prop_count;
	int running;
	int stopped;
	int interval;
	int timeout;
	mutex_type  mutex;
	demo_service* services;
}demo_device;

demo_device* create_demo_device(device_spec_meta* dev_spec);
int start_demo_device(demo_device* dev);
int stop_demo_device(demo_device* dev);
void destory_demo_device(demo_device* dev);
#endif