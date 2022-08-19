#ifndef _COMMON_H_
#define _COMMON_H_

#include <message.h>
#include <device.h>
#include <mapper_core.h>

response_msg* send_request(mapper_core* core, request_msg* req);
int send_response(request_msg* req, char* code, char* payload);
int mcore_register_protocol(mapper_core* core, char* mapper_id, char* spec);
devices_spec_meta* mcore_fetch_device_metadata(mapper_core* core, char* mapper_id);
#endif