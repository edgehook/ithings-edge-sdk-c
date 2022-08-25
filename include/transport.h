#ifndef  _TRANSPORT_H_
#define  _TRANSPORT_H_

#include "MQTTClient.h"
#include <message.h>
#include <util/list.h>
#include <util/blockedqueue.h>

typedef struct {
	char* srv_uri;
	char* client_id;
	char* usr;
	char* pwd;
	char* ca_path;
	char* cert_path;
	char* key_path;
	char* privateKeyPassword;

	int connected;
	MQTTClient client;
	
	//interface function
	void (*on_connlost)(void* context, char* cause);
	void (*msgarrvd)(void* context, char* topic, char* payload, int payload_len);
} mqtt_transport;

typedef void (on_connlost_callback)(void* context, char* cause);
typedef void (msgarrvd_callback)(void* context, char* topic, char* payload, int payload_len);

//mqtt layer.
mqtt_transport* mqtt_transport_config(mqtt_transport* trans, char* svr_uri, char* usr, char* pwd);
void mqtt_set_callback(mqtt_transport* trans, 
	on_connlost_callback* on_connlost,
	msgarrvd_callback* msgarrvd);
int mqtt_transport_connect(mqtt_transport* trans);
int mqtt_transport_subscribe(mqtt_transport* trans, char* topic, int qos);
int mqtt_transport_publish(mqtt_transport* trans, char* topic, int qos, void* payload, int payloadlen);
int mqtt_transport_unsubscribe(mqtt_transport* trans, char* topic);
int mqtt_transport_disconnect(mqtt_transport* trans);
void mqtt_destory_transport(mqtt_transport* trans);

typedef struct {
  mqtt_transport	t;
  char* 				mapper_id;
  blocked_queue*		req_queue;
  blocked_queue*		resp_queue;
  blocked_queue*		tx_queue;
  int				stopped;
  void	(*on_connected)(void);
  void	(*on_lost)(void);
} transport;

int transport_init(char* svr_uri, char* usr, char* pwd, char* mapper_id);
int transport_connect(void (*oc)(void), void (*ol)(void));
void send_async_message(void* message, size_t size);
request_msg* get_request_message(void);
response_msg* get_response_message(void);
int transport_destory();

#endif
