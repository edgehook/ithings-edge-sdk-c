#ifndef _MESSAGE_H_
#define _MESSAGE_H_

/*
* Request message.
*/
typedef struct {
	int  type;
	char mapper_id[64];
	char operation[32];
	char msg_id[48];
	char resource[64];
	char* payload;
	int   payloadlen;
}request_msg;

/*
* response message.
*/
typedef struct {
	int   type;
	char  mapper_id[64];
	char  operation[32];
	char  msg_id[48];
	char  parent_msg_id[48];
	char  code[8];
	char* payload;
	int   payloadlen;
}response_msg;

#define MSG_TYPE_REQ  (0x55)
#define MSG_TYPE_RESP  (0xBB)

#define EDGE_TOPIC_STARTER		"adv/ithings/edge"
#define MAPPER_TOPIC_STARTER	"adv/ithings/mapper"

#define ITHINGS_RSC_MAPPER          	"mapper"
#define ITHINGS_RSC_DEVICE_DATA     	"device_data"
#define ITHINGS_RSC_DEVICE_EVENT    	"device_event"
#define ITHINGS_RSC_EVENT_RECOVER    	"event_recover"
#define ITHINGS_RSC_DEVICE_STATUS    	"device_status"
#define ITHINGS_RSC_DEVICE_SPEC_META  	"device_spec_meta"
#define ITHINGS_RSC_DESIRED_TWINS    	"desired_twins"

#define ITHINGS_OP_REGISTER 	"register"
#define ITHINGS_OP_REPORT		"report"
#define ITHINGS_OP_REPLY		"reply"
#define ITHINGS_OP_FETCH		"fetch"
#define ITHINGS_OP_LIFE_CONTROL	"life_control"
#define ITHINGS_OP_SET_PROPERTY	"set_property"

#define DEVICE_LFCRTL_CREATE		"create"
#define DEVICE_LFCRTL_START  	 	"start"
#define DEVICE_LFCRTL_STOP  		"stop"
#define DEVICE_LFCRTL_UPDATE 		"update"
#define DEVICE_LFCRTL_DELETE 		"delete"

#define ITHINGS_RSP_MAPPER_NOT_FOUND       "4.04"
#define ITHINGS_RSP_OPERATION_NOT_FOUND    "4.05"
#define ITHINGS_RSP_MAPPER_NOT_REGISTER    "4.06"
#define ITHINGS_RSP_MAPPER_REGISTER_FAILED "4.07"
#define ITHINGS_RSP_INVALID_MQTT_MESSAGE   "4.08"
#define ITHINGS_RSP_SUCCEED                "200"
#define ITHINGS_RSP_INVALID_MSG            "201"
#define ITHINGS_RSP_INTERNEL_ERR           "202"
#define ITHINGS_RSP_UNKNOWN_ERR            "205"

request_msg* build_request(char* mapperID, char* operation, char* resource);
request_msg* build_register_request(char* mapperID);
request_msg* build_report_request(char* mapperID, char* resource);
void request_set_payload(request_msg* req, char* payload, int payloadlen);
char* get_request_topic(request_msg* req);
int get_request_payload(request_msg* req, char** payload);
request_msg* parse_request(char* topic, char* payload);
void free_request(request_msg** req);
response_msg* build_response(char* msg_id, char* parent_id, char* code, 
	char* mapperID, 	char* operation, char* content);
response_msg* parse_response(char* topic, char* payload);
response_msg* build_response_by_request(request_msg* req, char* code, char* content);
char* get_response_topic(response_msg* resp);
int get_response_payload(response_msg* resp, char** payload);
void free_response(response_msg** resp);
#endif