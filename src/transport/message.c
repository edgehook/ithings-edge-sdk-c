#include <stdlib.h>
#include <string.h>

#include <message.h>
#include <util/util.h>
#include <util/log.h>
#include <util/json_util.h>

request_msg* build_request(char* mapperID, char* operation, char* resource){
	request_msg* req = NULL;

	if(util_strlen(mapperID) == 0 || util_strlen(operation) == 0){
		errorf("invalid args \r\n");
		return NULL;
	}

	req = (request_msg*)malloc(sizeof(request_msg));
	if(req == NULL){
		errorf("malloc failed, no more system memory! \r\n");
		return NULL;
	}

	memset(req, 0, sizeof(request_msg));
	req->type = MSG_TYPE_REQ;
	strncpy(req->mapper_id, mapperID, sizeof(req->mapper_id)-1);
	strncpy(req->operation, operation, sizeof(req->operation)-1);
	if(util_strlen(resource)){
		strncpy(req->resource, resource, sizeof(req->resource)-1);
	}
	gen_rand_uuid_str(req->msg_id, 0);

	return req;
}

request_msg* build_register_request(char* mapperID){
	return build_request(mapperID, ITHINGS_OP_REGISTER, "mapper");
}

request_msg* build_report_request(char* mapperID, char* resource){
	return build_request(mapperID, ITHINGS_OP_REPORT, resource);
}

void request_set_payload(request_msg* req, char* payload, int payloadlen){
	if(req){
		req->payload = payload;
		req->payloadlen = payloadlen;
	}
}

char* get_request_topic(request_msg* req){
	char* tmp = NULL;
	char* topic = NULL;
	
	if(req == NULL) return NULL;

	topic = combine_strings(5, MAPPER_TOPIC_STARTER, "/request/", req->mapper_id, "/", req->operation);
	if(util_strlen(req->resource)){
		tmp = combine_strings(3, topic, "/",  req->resource);
		free_memory(&topic);
		topic = tmp;
	}

	return topic;	
}

int get_request_payload(request_msg* req, char** payload){
	cJSON* object = json_create_object();

	if(req == NULL || payload == NULL) return 0;
	if(object == NULL) return 0;
	
	json_add_string_to_object(object, "id", req->msg_id);
	if(req->payload)
		json_add_string_to_object(object, "content", req->payload);

	*payload = json_print(object);
	if(*payload == NULL){
		json_delete(object);
		return 0;
	}
	json_delete(object);

	return util_strlen(*payload);
}

/*
* parse the request message.
*/
request_msg* parse_request(char* topic, char* payload){
	int count = 0; 
	char** tmp;
	char* msg_id = NULL;
	char* mapperID = NULL;	
	char* operation = NULL;
	char* resource = NULL;
	char* content = NULL;
	request_msg* req = NULL;
	cJSON* object = json_parse(payload);

	if(topic == NULL || object == NULL) return NULL;

	//split topic.
	tmp = string_split(topic, '/');
	if(tmp == NULL){
		errorf("invalid topic type \r\n");
		json_delete(object);
		return NULL;
	}

	while(tmp[count++]);
	count--;

	if(count < 6 || count > 7){
		errorf("invalid topic format! \r\n");
		free_string_split_result(tmp);
		json_delete(object);
		return NULL;
	}

	mapperID = tmp[4];
	operation = tmp[5];
	if(count == 7) resource = tmp[6];

	msg_id = json_get_string_from_object(object, "id");
	if(msg_id == NULL){
		errorf("parse id failed!\r\n");
		free_string_split_result(tmp);
		json_delete(object);
		return NULL;
	}

	content = json_get_string_from_object(object, "content");
	if(content == NULL){
		errorf("parse content failed!\r\n");
		free_string_split_result(tmp);
		json_delete(object);
		return NULL;
	}

	req = build_request(mapperID, operation, resource);
	if(req == NULL){
		errorf("build_request failed\r\n");
		free_string_split_result(tmp);
		json_delete(object);
		return NULL;
	}

	//fill payload.
	memcpy(req->msg_id, msg_id, 36);
	req->payload = util_strdup(content);

	free_string_split_result(tmp);
	json_delete(object);
	return req;
}

void free_request(request_msg** req){
	if(*req){
		free_memory(&(*req)->payload);
		free(*req);
		*req = NULL;
	}
}

response_msg* build_response(char* msg_id, char* parent_id, char* code, 
	char* mapperID, 	char* operation, char* content){
	response_msg* resp;

	if(util_strlen(msg_id) == 0 || util_strlen(parent_id) == 0 
		|| util_strlen(mapperID) == 0 || util_strlen(operation) == 0 )
		return NULL;

	resp = (response_msg*)malloc(sizeof(response_msg));
	if(resp == NULL){
		errorf("malloc failed, no more system memory! \r\n");
		return NULL;
	}

	memset(resp, 0, sizeof(response_msg));
	resp->type = MSG_TYPE_RESP;
	strncpy(resp->mapper_id, mapperID, sizeof(resp->mapper_id)-1);
	strncpy(resp->operation, operation, sizeof(resp->operation)-1);
	strncpy(resp->msg_id, msg_id, 36);
	strncpy(resp->parent_msg_id, parent_id, 36);
	if(code){
		strncpy(resp->code, code, 7);
	}
	if(content){
		resp->payload = util_strdup(content);
	}

	return resp;
}

response_msg* parse_response(char* topic, char* payload){
	int count = 0; 
	char** tmp;
	char* msg_id = NULL;
	char* parent_msg_id = NULL;
	char* code = NULL;
	char* mapperID = NULL;	
	char* operation = NULL;
	char* content = NULL;
	response_msg* resp;
	cJSON* object = json_parse(payload);
	
	if(topic == NULL || payload == NULL || object == NULL) return NULL;

	tmp = string_split(topic, '/');
	if(tmp == NULL){
		errorf("invalid topic type \r\n");
		json_delete(object);
		return NULL;
	}

	while(tmp[count++]);
	count--;

	if(count != 6) {
		errorf("invalid topic format!\r\n");
		free_string_split_result(tmp);
		json_delete(object);
		return NULL;
	}

	mapperID = tmp[4];
	operation = tmp[5];

	msg_id = json_get_string_from_object(object, "id");
	if(msg_id == NULL){
		errorf("parse id failed!\r\n");
		free_string_split_result(tmp);
		json_delete(object);
		return NULL;
	}
	parent_msg_id = json_get_string_from_object(object, "pid");
	if(parent_msg_id == NULL){
		errorf("parse pid failed!\r\n");
		free_string_split_result(tmp);
		json_delete(object);
		return NULL;
	}
	code = json_get_string_from_object(object, "code");
	if(code == NULL){
		errorf("parse code failed!\r\n");
		free_string_split_result(tmp);
		json_delete(object);
		return NULL;
	}
	content = json_get_string_from_object(object, "content");
	if(content == NULL){
		errorf("parse content failed!\r\n");
		free_string_split_result(tmp);
		json_delete(object);
		return NULL;
	}

	resp = build_response(msg_id, parent_msg_id, code, mapperID, operation, content);
	if(resp == NULL){
		errorf("build_response failed\r\n");
		free_string_split_result(tmp);
		json_delete(object);
		return NULL;
	}

	free_string_split_result(tmp);
	json_delete(object);

	return resp;
}

response_msg* build_response_by_request(request_msg* req, char* code, char* content){
	char msg_id[37] = {0};
	char* parent_id;
	char* mapperID;

	if(req == NULL || util_strlen(code) == 0) return NULL;

	parent_id = req->msg_id;
	mapperID = req->mapper_id;
	gen_rand_uuid_str(msg_id, 0);

	if(util_strlen(mapperID) == 0){
		errorf("invalid mapper id ! \r\n");
		return NULL;
	}

	return build_response(msg_id, parent_id, code, mapperID, ITHINGS_OP_REPLY, content);
}

char* get_response_topic(response_msg* resp){
	if(resp == NULL) return NULL;

	return combine_strings(5, MAPPER_TOPIC_STARTER, 
			"/reply/", resp->mapper_id, "/", resp->operation);	
}

int get_response_payload(response_msg* resp, char** payload){
	cJSON* object = json_create_object();

	if(resp == NULL || payload == NULL) return 0;
	if(object == NULL) return 0;
	
	json_add_string_to_object(object, "id", resp->msg_id);
	json_add_string_to_object(object, "pid", resp->parent_msg_id);
	json_add_string_to_object(object, "code", resp->code);
	if(resp->payload)
		json_add_string_to_object(object, "content", resp->payload);

	*payload = json_print(object);
	if(*payload == NULL){
		json_delete(object);
		return 0;
	}
	json_delete(object);

	return util_strlen(*payload);
}

void free_response(response_msg** resp){
	if(*resp){
		free_memory(&(*resp)->payload);
		free(*resp);
		*resp = NULL;
	}
}
