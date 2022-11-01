#ifndef _DEVICE_H_
#define _DEVICE_H_

#include "lib_api.h"
#include <stdint.h>

#define DEVICE_STATUS_ONLINE	"online"
#define DEVICE_STATUS_OFFLINE 	"offline"

typedef struct{
	char name[64];
	int writeable;
	double max;
	double min;
	char  unit[16];
	char  data_type[16];
	// List of AccessConfig which describe how to access the device properties,command, and events.
	// AccessConfig must unique by AccessConfig.propertyName.
	// +optional
	//this should be a json string
	// AccessConfig must unique by AccessConfig.propertyName.
	char* access_config;
}device_property_spec;

typedef struct{
	char name[64];
	double max;
	double min;
	char  unit[16];
	char  data_type[16];
	char  event_type[16];
	char* access_config;
}device_event_spec;

typedef struct{
	char name[64];
	char* parms;
	char* access_config;
}device_command_spec;

typedef struct{
	char name[64];
	//list all device_property_spec
	int properties_size;
	device_property_spec* properties;
	//list all device_event_spec
	int events_size;
	device_event_spec* events;
	// list device_command_spec
	int commands_size;
	device_command_spec* commands;
}device_service_spec;

typedef struct{
	char device_id[48];
	char device_os[32];
	char device_catagory[32];
	char device_id_code[64];
	char device_state[32];
	char tags[128];
	char* protocol;
	//list all device_service_spec
	device_service_spec* services;
	int size;
}device_spec_meta;

typedef struct {
	int size;
	device_spec_meta* devices;
}devices_spec_meta;

typedef struct{
	char service[64];
	char property_name[128];
	void* value;
	uint64_t timestamp;
	char err_msg[128];
	int writeable;
	double max;
	double min;
	char  unit[16];
	char  data_type[16];
}twin_property;

typedef struct{
	char device_id[48];
	//the list of the twin_property.
	int size;
	twin_property* desired_twins;
} device_desired_twins_update_msg;

typedef struct {
	char device_id[48];
	char status[64];
	char err_msg[128];
	int  timestamp;
}device_status_msg;

typedef struct {
	int size;
	device_status_msg* devices;
}devices_status_message;

typedef struct {
	char device_id[48];
	int size;
	twin_property* twin_properties;
}report_device_props_msg;

typedef struct {
	int size;
	report_device_props_msg* devices;
}devices_props_report_msg;

typedef struct {
	char device_id[48];
	char service[64];
	char event_name[128];
	char* payload;
	int  timestamp;
	char err_msg[128];
}device_events_report_msg;

typedef struct {
	devices_props_report_msg* props_report_msg;
	device_events_report_msg* events_report_msg;
}device_report_msg;

LIBAPI devices_spec_meta* decode_devices_spec_meta(char* payload);
LIBAPI void destory_devices_spec_meta(devices_spec_meta* meta);
LIBAPI device_desired_twins_update_msg* decode_device_desired_twins_update_msg(char* payload);
LIBAPI void destory_device_desired_twins_update_msg(device_desired_twins_update_msg* msg);
LIBAPI char* encode_devices_status_message(devices_status_message* msg);
LIBAPI char* encode_devices_props_report_msg(devices_props_report_msg* msg);
LIBAPI char* encode_device_events_report_msg(device_events_report_msg* msg);
LIBAPI char* encode_device_report_msg(device_report_msg* msg);
LIBAPI void destory_device_report_msg(device_report_msg* msg);

#endif
