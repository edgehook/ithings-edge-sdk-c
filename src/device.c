#include <stdlib.h>
#include <string.h>

#include <device.h>
#include <util/log.h>
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
		if(!obj) continue;

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

static void decode_device_spec_metadata(device_spec_meta* meta, cJSON* object){
	int i = 0;
	char* tmp = NULL;
	cJSON* array = NULL;

	tmp = json_get_string_from_object(object, "id");
	if(!tmp) return;
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
	if(!array) return;

	meta->size = json_get_array_size(array);
	if(meta->size <= 0) return;

	meta->services = malloc(sizeof(device_service_spec)*meta->size);
	if(!meta->services) return;

	memset(meta->services, 0, sizeof(device_service_spec)*meta->size);
	for(i = 0; i < meta->size; i++){
		cJSON* obj = NULL;
		device_service_spec* svc = &meta->services[i];

		obj = json_get_object_from_array(array, i);
		if(!obj) continue;

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
}

devices_spec_meta* decode_devices_spec_meta(char* payload){
	int i = 0;
	int size;
	char* tmp = NULL;
	cJSON* array = json_parse(payload);
	cJSON* object = NULL;
	devices_spec_meta* metas = NULL;

	if(!array)	return NULL;

	if(json_is_object(array)){
		device_spec_meta* meta;

		//it's object.
		object = array;
		size = sizeof(devices_spec_meta) + sizeof(device_spec_meta);
		metas = (devices_spec_meta*)malloc(size);
		if(!metas){
			errorf("no more system memory, then malloc failed! \r\n");
			json_delete(array);
			return NULL;
		}

		memset(metas, 0, size);
		metas->size = 1;
		metas->devices = (device_spec_meta*)((char*)metas + sizeof(devices_spec_meta));
		meta = &metas->devices[0];

		decode_device_spec_metadata(meta, object);

		json_delete(array);
		return metas;
	}

	//this is array.
	size = sizeof(devices_spec_meta);
	size += sizeof(device_spec_meta)*json_get_array_size(array);
	metas = (devices_spec_meta*)malloc(size);
	if(!metas){
		errorf("no more system memory, then malloc failed! \r\n");
		json_delete(array);
		return NULL;
	}

	memset(metas, 0, size);
	metas->size = json_get_array_size(array);
	metas->devices = (device_spec_meta*)((char*)metas + sizeof(devices_spec_meta));

	for(i = 0; i < json_get_array_size(array); i++){
		device_spec_meta* meta;

		meta = &metas->devices[i];
		object = json_get_object_from_array(array, i);
		if(!object) continue;

		decode_device_spec_metadata(meta, object);
	}

	json_delete(array);
	return metas;
}

void destory_devices_spec_meta(devices_spec_meta* meta){
	if(!meta) return;

	if(meta->devices){
		int size, i,j;

		size = meta->size;
		for(i = 0; i < size; i++){
			device_spec_meta* dev = &meta->devices[i];

			if(!dev->services) continue;

			for(j = 0; j < dev->size; j++){
				device_service_spec* svc = &dev->services[i];

				if(svc->properties) {
					free(svc->properties);
					svc->properties = NULL;
				}

				if(svc->events){
					free(svc->events);
					svc->events = NULL;
				}

				if(svc->commands){
					free(svc->commands);
					svc->commands = NULL;
				}

				free(dev->services);
			}
		}
	}

	free(meta);
}

device_desired_twins_update_msg* decode_device_desired_twins_update_msg(char* payload){
	int i = 0;
	int size, count;
	char* device_id = NULL;
	cJSON* object = json_parse(payload);
	cJSON* array = NULL;
	device_desired_twins_update_msg* update_msg = NULL;

	if(object == NULL) return NULL;

	device_id = json_get_string_from_object(object, "d_id");
	if(!device_id){
		errorf("d_id is not in the json string! \r\n");
		json_delete(object);
		return NULL;
	}
	array = json_get_object_from_object(object, "desired_twins");

	count = json_get_array_size(array);
	size = sizeof(device_desired_twins_update_msg);
	if(count > 0) size += count* sizeof(twin_property);

	update_msg = (device_desired_twins_update_msg*)malloc(size);
	if(!update_msg){
		json_delete(object);
		return NULL;
	}

	memset(update_msg, 0, size);
	strncpy(update_msg->device_id, device_id, 47);
	update_msg->size = count;
	if(count > 0){
		update_msg->desired_twins = (twin_property*)((char*)update_msg + sizeof(device_desired_twins_update_msg));
	}

	if(array){
		for(i = 0 ; i < count; i++){
			char* tmp = NULL;
			cJSON* obj = NULL;
			twin_property* twins = &update_msg->desired_twins[i];

			obj = json_get_object_from_array(array, i);
			if(!obj) continue;

			tmp = json_get_string_from_object(obj, "svc");
			if(!tmp) continue;
			strncpy(twins->service, tmp, 63);

			tmp = json_get_string_from_object(obj, "pn");
			if(!tmp) continue;
			strncpy(twins->property_name, tmp, 127);
			//decode val.
			object = json_get_object_from_object(obj, "val");
			if(object){
				switch((object->type & 0xFF)){
				case cJSON_Number:
					twins->value = util_doubledup(object->valuedouble);
					break;
				case cJSON_Raw:
				case cJSON_String:
					twins->value = util_strdup(object->valuestring);
					break;
				case cJSON_False:
				case cJSON_True:
					twins->value = util_intdup(object->valueint);
					break;
				default:
					twins->value = NULL;
					break;
				}
			}

			twins->timestamp = json_get_int_from_object(obj, "ts", 0);
			tmp = json_get_string_from_object(obj, "err_msg");
			if(tmp){
				strncpy(twins->err_msg, tmp, 127);
			}
		}
	}

	json_delete(object);
	return update_msg;
}

void destory_device_desired_twins_update_msg(device_desired_twins_update_msg* msg){
	int i = 0;
	if(!msg) return;

	if(msg->desired_twins){
		for(i = 0; i < msg->size; i++){
			twin_property* twin = &msg->desired_twins[i];

			if(twin->value)
				free(twin->value);
		}
	}

	free(msg);
}