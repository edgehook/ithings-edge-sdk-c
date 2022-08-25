#include <common.h>
#include <transport.h>
#include "util/util.h"
#include "util/json_util.h"
#include <util/event_listener.h>

#define RESPONSE_TIME    (5000UL)

response_msg* send_request(mapper_core* core, request_msg* req){
	response_msg* resp = NULL;
	event_listener* el = NULL;
	
	if(core == NULL || req == NULL) return NULL;

	//register event listener.
	el = register_event_listener(core->el_mgr, req->msg_id, RESPONSE_TIME);
	if(el == NULL){
		errorf("register event listener failed \r\n");
		return NULL;
	}

	//send async message to ithing.
	send_async_message(req, sizeof(request_msg));

	//wait the response.
	resp = wait_event(el);
	unregister_event_listener(core->el_mgr, el);

	return resp;
}

int send_response(request_msg* req, char* code, char* payload){
	response_msg* resp = build_response_by_request(req, code, payload);

	if(!resp) return -1;

	send_async_message(resp, sizeof(response_msg));
	return 0;
}

int mcore_register_protocol(mapper_core* core, char* mapper_id, char* spec){
	char* payload;
	request_msg* req = NULL;
	response_msg* resp = NULL;
	cJSON* object = NULL;
	
	if(!mapper_id) return -1;

	req = build_register_request(mapper_id);
	if(!req) return -2;

	//build json string.
	object = json_create_object();
	if(!object) return -1;

	json_add_string_to_object(object, "type", mapper_id);
	if(spec){
		json_add_string_to_object(object, "spec", spec);
	}

	payload = json_print(object);
	if(!payload){
		json_delete(object);
		free_request(&req);
		return -3;
	}
	json_delete(object);

	//send request and check response.
	request_set_payload(req, payload, util_strlen(payload));
	resp = send_request(core, req);
	if(!resp) return -5;

	if(!string_contain(resp->code, ITHINGS_RSP_SUCCEED)){
		free_response(&resp);
		return -6;
	}

	free_response(&resp);
	return 0;
}

/*
* fetch device metadata from server.
*/
devices_spec_meta* mcore_fetch_device_metadata(mapper_core* core, char* mapper_id){
	request_msg* req = NULL;
	response_msg* resp = NULL;
	devices_spec_meta* devs_spec = NULL;

	req = build_request(mapper_id, ITHINGS_OP_FETCH, ITHINGS_RSC_DEVICE_SPEC_META);
	if(!req) return  NULL;

	resp = send_request(core, req);
	if(!resp) return NULL;

	if(!string_contain(resp->code, ITHINGS_RSP_SUCCEED)){
		free_response(&resp);
		return NULL;
	}

	//decode the device_spec_meta
	devs_spec = decode_devices_spec_meta(resp->payload);
	free_response(&resp);

	return devs_spec;
}

//do real life control.
int mcore_do_life_control(mapper_core* core, char* action, char* payload){
	int ret = 0;
	devices_spec_meta* devs_spec = decode_devices_spec_meta(payload);

	if(!devs_spec) {
		errorf("decode_devices_spec_meta failed \r\n");
		return -5;
	}

	if(core->life_control){
		ret = core->life_control(action, devs_spec);
	}

	return ret;
}

//set property's desried value
int mcore_do_set_properties(mapper_core* core, char* payload){
	int ret = 0;
	device_desired_twins_update_msg* update_msg = NULL;

	update_msg = decode_device_desired_twins_update_msg(payload);
	if(!update_msg){
		errorf("Decode Twin update msg failed \r\n");
		return -5;
	}

	if(core->update_desired_twins){
		ret = core->update_desired_twins(update_msg);
	}

	return ret;
}

int mcore_send_keepalive_msg(mapper_core* core, char* mapper_id, devices_status_message* msg){
	request_msg* req = NULL;
	response_msg* resp = NULL;
	char* payload = encode_devices_status_message(msg);

	if(!payload) return -1;

	req	= build_report_request(mapper_id, ITHINGS_RSC_DEVICE_STATUS);
	if(!req){
		free(payload);
		return -2;
	}
	request_set_payload(req, payload, util_strlen(payload));

	//send request and check response.
	resp = send_request(core, req);
	if(!resp)	return -5;

	if(!string_contain(resp->code, ITHINGS_RSP_SUCCEED)){
		free_response(&resp);
		return -6;
	}

	free_response(&resp);
	return 0;
}

int mcore_do_device_report(mapper_core* core, device_report_msg* msg){
	request_msg* req = NULL;
	response_msg* resp = NULL;
	char* payload = NULL;

	if(msg->props_report_msg){
		payload = encode_devices_props_report_msg(msg->props_report_msg);
		if(!payload) return -1;

		req = build_report_request(core->mapper_id, ITHINGS_RSC_DEVICE_DATA);
		if(!req){
			free(payload);
			return -2;
		}
		request_set_payload(req, payload, util_strlen(payload));
	}else if(msg->events_report_msg){
		payload = encode_device_events_report_msg(msg->events_report_msg);
		if(!payload) return -1;

		if(msg->events_report_msg->payload[0]){
			req = build_report_request(core->mapper_id, ITHINGS_RSC_DEVICE_EVENT);
		}else{
			//recovery
			req = build_report_request(core->mapper_id, ITHINGS_RSC_EVENT_RECOVER);
		}
		if(!req){
			free(payload);
			return -2;
		}
		request_set_payload(req, payload, util_strlen(payload));
	}

	//send request and check response.
	resp = send_request(core, req);
	if(!resp)	return -5;

	if(!string_contain(resp->code, ITHINGS_RSP_SUCCEED)){
		free_response(&resp);
		return -6;
	}

	free_response(&resp);
	return 0;
}