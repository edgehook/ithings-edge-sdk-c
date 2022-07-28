#include <stdlib.h>
#include <string.h>

#include <device.h>
#include <util/util.h>
#include <util/json_util.h>

static void decode_device_properties_spec(cJSON* array, device_service_spec* svc){
	int i = 0;
	char* tmp;
	cJSON* obj = NULL;
	device_property_spec* prop = NULL;

	svc->properties_size = json_get_array_size(array);
	if(svc->properties_size <= 0){
		return;
	}

	svc->properties = malloc(sizeof(device_property_spec)*svc->properties_size);
	if(!svc->properties) return;

	memset(svc->properties, 0, sizeof(device_property_spec)*svc->properties_size);

	for(i = 0; i < svc->properties_size; i++){
		obj = json_get_object_from_array(array, i);
		if(!obj){
			continue;
		}

		prop = &svc->properties[i];
		tmp = json_get_string_from_object(obj, "pn");
		if(tmp == NULL) continue;

		strncpy(prop->name, tmp, 63);
		prop->writeable = json_get_bool_from_object(obj, "rw", 0);
		prop->max = json_get_double_from_object(obj, "max", 0.0);
		prop->min = json_get_double_from_object(obj, "min", 0.0);
		tmp = json_get_string_from_object(obj, "un");
		if(tmp) strncpy(prop->unit, tmp, 15);
		tmp = json_get_string_from_object(obj, "dt");
		if(tmp) strncpy(prop->data_type, tmp, 15);
		tmp = json_get_string_from_object(obj, "ac");
		prop->access_config = util_strdup(tmp);
	}
}

static void decode_device_event_spec(cJSON* array, device_service_spec* svc){
	int i = 0;
	char* tmp;
	cJSON* obj = NULL;
	device_event_spec* event = NULL;

	svc->events_size = json_get_array_size(array);
	if(svc->events_size <= 0){
		return;
	}

	svc->events = malloc(sizeof(device_event_spec)*svc->events_size);
	if(!svc->events) return;

	memset(svc->events, 0, sizeof(device_event_spec)*svc->events_size);

	for(i = 0; i < svc->events_size; i++){
		obj = json_get_object_from_array(array, i);
		if(!obj){
			continue;
		}

		event = &svc->events[i];
		tmp = json_get_string_from_object(obj, "en");
		if(tmp == NULL) continue;
		strncpy(event->name, tmp, 63);
		tmp = json_get_string_from_object(obj, "et");
		if(tmp == NULL) continue;
		strncpy(event->event_type, tmp, 15);
		event->max = json_get_double_from_object(obj, "max", 0.0);
		event->min = json_get_double_from_object(obj, "min", 0.0);
		tmp = json_get_string_from_object(obj, "un");
		if(tmp) strncpy(event->unit, tmp, 15);
		tmp = json_get_string_from_object(obj, "dt");
		if(tmp) strncpy(event->data_type, tmp, 15);
		tmp = json_get_string_from_object(obj, "ac");
		event->access_config = util_strdup(tmp);
	}
}

static void decode_device_command_spec(cJSON* array, device_service_spec* svc){
	int i = 0;
	char* tmp;
	cJSON* obj = NULL;
	device_command_spec* cmd = NULL;

	svc->commands_size = json_get_array_size(array);
	if(svc->commands_size <= 0){
		return;
	}
	svc->commands = malloc(sizeof(device_command_spec)*svc->commands_size);
	if(!svc->commands) return;
	memset(svc->events, 0, sizeof(device_command_spec)*svc->commands_size);

	for(i = 0; i < svc->commands_size; i++){
		obj = json_get_object_from_array(array, i);
		if(!obj){
			continue;
		}

		cmd = &svc->commands[i];
		tmp = json_get_string_from_object(obj, "cn");
		if(tmp == NULL) continue;
		strncpy(cmd->name, tmp, 63);
		tmp = json_get_string_from_object(obj, "req_param");
		cmd->parms = util_strdup(tmp);
		tmp = json_get_string_from_object(obj, "ac");
		cmd->access_config = util_strdup(tmp);
	}
}

device_spec_meta* decode_device_spec_meta(char* payload){
	int i = 0;
	char* tmp = NULL;
	cJSON* array = json_parse(payload);
	cJSON* object = NULL;
	device_spec_meta* meta = malloc(sizeof(device_spec_meta));

	if(!array || !meta) {
		if(!array) json_delete(array);
		if(!meta) free(meta);
		return NULL;
	}

	memset(meta, 0, sizeof(device_spec_meta));
	object = json_get_object_from_array(array, 0);
	tmp = json_get_string_from_object(object, "id");
	if(!tmp){
		json_delete(object);
		free(meta);
		return NULL;
	}
	strncpy(meta->device_id, tmp, 47);

	tmp = json_get_string_from_object(object, "os");
	if(tmp){
		strncpy(meta->device_os, tmp, 31);
	}
	tmp = json_get_string_from_object(object, "catagory");
	if(tmp){
		strncpy(meta->device_catagory, tmp, 31);
	}
	tmp = json_get_string_from_object(object, "id_code");
	if(tmp){
		strncpy(meta->device_id_code, tmp, 63);
	}
	tmp = json_get_string_from_object(object, "state");
	if(tmp){
		strncpy(meta->device_state, tmp, 63);
	}
	tmp = json_get_string_from_object(object, "tags");
	if(tmp){
		strncpy(meta->tags, tmp, 127);
	}
	tmp = json_get_string_from_object(object, "proto_conf");
	meta->protocol = util_strdup(tmp);

	array = json_get_object_from_object(object, "svcs");
	if(!array){
		json_delete(object);
		return meta;
	}

	meta->size = json_get_array_size(array);
	if(meta->size <= 0){
		json_delete(object);
		return meta;
	}

	meta->services = malloc(sizeof(device_service_spec)*meta->size);
	if(!meta->services){
		json_delete(object);
		return meta;
	}

	memset(meta->services, 0, sizeof(device_service_spec)*meta->size);
	for(i = 0; i < meta->size; i++){
		cJSON* obj = NULL;
		device_service_spec* svc = &meta->services[i];

		obj = json_get_object_from_array(array, i);
		if(!obj){
			continue;
		}

		tmp = json_get_string_from_object(obj, "name");
		if(tmp == NULL) continue;

		strncpy(svc->name, tmp, 63);

		array = json_get_object_from_object(obj, "props");
		if(array){
			decode_device_properties_spec(array, svc);
		}
		array = json_get_object_from_object(obj, "events");
		if(array){
			decode_device_event_spec(array, svc);
		}
		array = json_get_object_from_object(obj, "cmds");
		if(array){
			decode_device_command_spec(array, svc);
		}
	}
	json_delete(object);
	return meta;
}

device_desired_twins_update_msg* decode_device_desired_twins_update_msg(char* payload){
	int i = 0;
	char* device_id = NULL;
	cJSON* object = json_parse(payload);
	cJSON* array = NULL;
	device_desired_twins_update_msg* update_msg = NULL;

	if(object == NULL) return  NULL;

	update_msg = (device_desired_twins_update_msg*)malloc(sizeof(device_desired_twins_update_msg));
	if(!update_msg){
		json_delete(object);
		return NULL;
	}

	device_id = json_get_string_from_object(object, "d_id");
	strncpy(update_msg->device_id, device_id, 47);

	array = json_get_object_from_object(object, "desired_twins");
	if(!array){
		json_delete(object);
		free(update_msg);
		return  NULL;
	}

	for(i = 0 ; i < json_get_array_size(array); i++){
		char* svc = NULL;
		char* pn = NULL;
		cJSON* obj = NULL;

		obj = json_get_object_from_array(array, i);
		if(!obj){
			continue;
		}

		svc = json_get_string_from_object(obj, "svc");
		pn = json_get_string_from_object(obj, "pn");
		if(svc && pn){
			twin_property* twin = (twin_property*)malloc(sizeof(twin_property));
			if(twin){
				strncpy(twin->service, svc, 63);
				strncpy(twin->property_name, pn, 127);

				//list_push_tail(update_msg->desired_twins, twin, sizeof(twin_property));
			}
		}
	}

	return update_msg;
}


