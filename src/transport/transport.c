#if !defined(_WINDOWS)
#define WINAPI
#else
#include <windows.h>
#endif

#include <message.h>
#include <transport.h>
#include <util/log.h>
#include <util/util.h>
#include <util/list.h>
#include <util/thread.h>

static transport tp;

int send_message(mqtt_transport* trans, void* content);
int transport_retry_connect();
int transport_publish(mqtt_transport* trans, char* topic, char*payload, int payloadlen);

static thread_return_type WINAPI msg_publish_thread(void* context){
	int result;
	void* content;
	transport* t = (transport*)context;
	blocked_queue*	tx_queue = NULL;
	
	if(t == NULL || t->tx_queue == NULL) return 0;

	tx_queue = t->tx_queue;

	while(1){
		content = blocked_queue_pop(tx_queue, 1000);
		if(content == NULL)
			continue;

		if(t->stopped) return 0;

		result = send_message(&t->t, content);
		if(result){
			warningf("send message with the error %d \r\n", result);
		}
	}

	return 0;
}

static void on_transport_connlost (void* context, char* cause){
	transport* t = (transport*)context;

	/*
	* If the connection is lost, then we will retry to connect the broker until connected
	* the server.
	*/
	warningf("connect lost with the reason (%s), we retry...", cause);
	if(t->on_lost)
		t->on_lost();

	transport_retry_connect();
}

static void transport_msgarrvd(void* context, char* topic, char* payload, int payload_len){
	int count = 0; 
	char** tmp;
	request_msg* req;

	if(topic == NULL || payload == NULL || payload_len <= 0){
		warningf("invalid args, we ignored!");
		return; 
	}

	if(!string_contain(topic, EDGE_TOPIC_STARTER)) return;

	tmp = string_split(topic, '/');
	if(tmp == NULL){
		errorf("invalid topic type \r\n");
		return;
	}

	while(tmp[count++]);
	if(count < 6) {
		errorf("invalid topic format! \r\n");
		free_string_split_result(tmp);
		return;
	}

	//this is a reply.
	if(string_contain(tmp[5], ITHINGS_OP_REPLY)){
		response_msg* resp;

		free_string_split_result(tmp);
		resp = parse_response(topic, payload);
		if(resp == NULL) return;

		//we add the message into thread-safty queue.
		blocked_queue_push(tp.resp_queue, resp, sizeof(response_msg));
		return;
	}

	// this is a request.
	free_string_split_result(tmp);
	req = parse_request(topic, payload);
	if(req == NULL) return;

	//we add the message into thread-safty queue.
	blocked_queue_push(tp.req_queue, req, sizeof(request_msg));
}

int transport_init(char* svr_uri, char* usr, char* pwd, char* mapper_id){
	mqtt_transport* trans;

	if(util_strlen(svr_uri)== 0 || util_strlen(mapper_id) == 0){
		errorf("invalid args or NULL \r\n");
		return -1;
	}
	
	trans = mqtt_transport_config(&tp.t, svr_uri, usr, pwd);
	if(trans == NULL){
		errorf("new mqtt transport failed \r\n");
		return -1;
	}

	tp.mapper_id = util_strdup(mapper_id);
	tp.req_queue = blocked_queue_init();
	tp.resp_queue = blocked_queue_init();
	tp.tx_queue = blocked_queue_init();

	//set transport
	mqtt_set_callback(trans, on_transport_connlost, transport_msgarrvd);

	//start tx thread.
	Thread_start(msg_publish_thread, &tp);
	
	return 0;
}

int transport_retry_connect(){
	int ret;
	char* topic;
	mqtt_transport* trans = &tp.t;
	char* mapper_id = tp.mapper_id; 

retry_connect:
	ret = mqtt_transport_connect(trans);
	if(ret != 0){
		warningf("connect failed with (%d), retry..\r\n", ret);
		util_sleep(3000);
		goto retry_connect;
	}

	infof("transport connect to broker %s successfully!\r\n", trans->srv_uri);

	//subscribe the topic.
	topic = combine_strings(4, EDGE_TOPIC_STARTER, "/mapper/", mapper_id, "/#");
	ret = mqtt_transport_subscribe(trans, topic, 1);
	if(ret != 0){
		errorf("subscribe failed with (%d)..\r\n", ret);
		free_memory(&topic);
		return ret;
	}

	infof("subscribe topic %s successfully!\r\n", topic);
	free_memory(&topic);

	//do on_connected callback.
	if(tp.on_connected)
		tp.on_connected();

	return 0;
}

int transport_connect(void (*oc)(void), void (*ol)(void)){
	tp.on_connected = oc;
	tp.on_lost = ol;

	return transport_retry_connect();
}

int send_message(mqtt_transport* trans, void* content){
	int result = 0;
	char* topic = NULL;
	char* payload = NULL;
	request_msg* req= NULL;
	response_msg* resp = NULL;

	req= (request_msg*)content;
	if(req->type == MSG_TYPE_REQ){
		topic = get_request_topic(req);
		get_request_payload(req, &payload);
		free_request(&req);
	}else{
		resp= (response_msg*)content;
		topic = get_response_topic(resp);
		get_response_payload(resp, &payload);
		free_response(&resp);
	}

	//send the message.
	result = transport_publish(trans, topic, payload, util_strlen(payload));
	free_memory(&topic);
	free_memory(&payload);

	return result;
}

void send_async_message(void* message, size_t size){
	blocked_queue*	tx_queue = NULL;

	if(message == NULL)
		return;

	tx_queue = tp.tx_queue;
	blocked_queue_push(tx_queue, message, size);
}

request_msg* get_request_message(void){
	return blocked_queue_pop(tp.req_queue, 1000);
}

response_msg* get_response_message(void){
	return blocked_queue_pop(tp.resp_queue, 1000);
}

int transport_publish(mqtt_transport* trans, char* topic, char*payload, int payloadlen){
	int ret, retries = 0;

	if(topic == NULL || payload == NULL) return -1;

retry_pub:
	ret = mqtt_transport_publish(trans, topic, 0, payload, payloadlen);
	if(ret){
		if(retries >= 3) return ret;

		retries++;
		warningf("mqtt_transport_publish faild with (%d), we retry publish!\r\n", ret);
		goto retry_pub;
	}

	return 0;
}

int transport_destory(){
	char* topic;
	mqtt_transport* trans = &tp.t;

	topic = combine_strings(4, EDGE_TOPIC_STARTER, "/mapper/", tp.mapper_id, "/#");
	mqtt_transport_unsubscribe(trans, topic);
	free_memory(&topic);

	tp.stopped = 1;
	mqtt_transport_disconnect(trans);
	mqtt_destory_transport(trans);
	free_memory(&tp.mapper_id);
	blocked_queue_destory(tp.resp_queue);
	blocked_queue_destory(tp.req_queue);
	blocked_queue_destory(tp.tx_queue);

	return 0;
}
