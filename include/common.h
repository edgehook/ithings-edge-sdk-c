#ifndef _COMMON_H_
#define _COMMON_H_

#include <message.h>
#include <device.h>
#include <mapper_core.h>

response_msg* send_request(mapper_core* core, request_msg* req);
int send_response(request_msg* req, char* code, char* payload);
int mcore_register_protocol(mapper_core* core, char* mapper_id, char* spec);
devices_spec_meta* mcore_fetch_device_metadata(mapper_core* core, char* mapper_id);
int mcore_do_life_control(mapper_core* core, char* action, char* payload);
int mcore_do_set_properties(mapper_core* core, char* payload);
int mcore_send_keepalive_msg(mapper_core* core, char* mapper_id, devices_status_message* msg);
int mcore_do_device_report(mapper_core* core, device_report_msg* msg);
#endif