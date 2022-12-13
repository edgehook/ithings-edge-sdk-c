#include <stdlib.h>
#include <string.h>
#if defined(_WIN32) || defined(_WIN64)
	#include <windows.h>
#else
	#include <unistd.h>
#endif

#include <transport.h>
#include <util/log.h>
#include "util/util.h"

#if defined(_WIN32) || defined(_WIN64)
#include <io.h>

#define access(pathname, mode) _access(pathname, mode)
#endif

void on_connlost(void* context, char* cause);
int on_msgarrvd(void* context, char* topicName, int topicLen, MQTTClient_message* message);

mqtt_transport* mqtt_transport_config(mqtt_transport* trans, char* svr_uri, char* usr, char* pwd){
	int ret;
	char *login_timestamp = get_client_timestamp();

	if(trans == NULL){
		errorf("Oops... trans is NULL in mqtt_transport_config \r\n");
		return NULL;
	}

	memset(trans, 0, sizeof(mqtt_transport));
	trans->srv_uri = util_strdup(svr_uri);
	trans->usr = util_strdup(usr);
	trans->pwd = util_strdup(pwd);
	trans->client_id = combine_strings(3, trans->usr ? trans->usr : "user", "@", login_timestamp);

	infof("uri = %s, id=%s \r\n", trans->srv_uri, trans->client_id);
	ret = MQTTClient_create(&trans->client, trans->srv_uri, trans->client_id, MQTTCLIENT_PERSISTENCE_NONE, NULL);
	if(ret){
		errorf("MQTTClient create with result(%d)\r\n", ret);
		mqtt_destory_transport(trans);
		return NULL;
	}

	ret = MQTTClient_setCallbacks(trans->client, trans, on_connlost, on_msgarrvd, NULL);
	if(ret != MQTTCLIENT_SUCCESS){
		errorf("MQTTAsync_setCallbacks create with result(%d)\r\n", ret);
		mqtt_destory_transport(trans);
		return NULL;
	}
	return trans;
}

void mqtt_set_callback(mqtt_transport* trans, 
	on_connlost_callback* on_connlost,
	msgarrvd_callback* msgarrvd){

	if(trans == NULL){
		return;
	}

	if(on_connlost) {
		trans->on_connlost = on_connlost;
	}

	if(msgarrvd){
		trans->msgarrvd = msgarrvd;
	}
}

int mqtt_transport_connect(mqtt_transport* trans){
	int ret;
	MQTTClient_SSLOptions ssl_opts = MQTTClient_SSLOptions_initializer;
	MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;

	if(trans == NULL){
		errorf("transport object is NULL \r\n");
		return -1;
	}

	conn_opts.cleansession = 1;
	conn_opts.keepAliveInterval = 60;
	conn_opts.reliable = 0;
	conn_opts.connectTimeout = 30;
	conn_opts.retryInterval = 10;
	conn_opts.username = trans->usr;
	conn_opts.password = trans->pwd;

	//enable ssl
	if (trans->ca_path && access(trans->ca_path, 0) == 0) {
		ssl_opts.trustStore = trans->ca_path;
		ssl_opts.enabledCipherSuites = "TLSv1.2";
		ssl_opts.enableServerCertAuth = 1; // TRUE: enable server certificate authentication, FALSE: disable
		// ssl_opts.verify = 0; // 0 for no verifying the hostname, 1 for verifying the hostname

		if (trans->cert_path && trans->key_path &&
			access(trans->cert_path, 0) == 0 && access(trans->key_path, 0) == 0) {
			ssl_opts.keyStore = trans->cert_path;
			ssl_opts.privateKey = trans->key_path;
			ssl_opts.privateKeyPassword = trans->privateKeyPassword;
		}

		conn_opts.ssl = &ssl_opts;
	}

	ret = MQTTClient_connect(trans->client, &conn_opts);
	if( ret != MQTTCLIENT_SUCCESS){
		errorf("connect failed with err %d \r\n", ret);
		return ret;
	}

	trans->connected = 1;
	
	return 0;
}

int mqtt_transport_subscribe(mqtt_transport* trans, char* topic, int qos){
	int ret;

	if(trans == NULL){
		errorf("transport object is NULL \r\n");
		return -1;
	}

	if(topic == NULL || strlen(topic) == 0){
		errorf("topic is NULL or empty! \r\n");
		return -1;
	}
	
	ret = MQTTClient_subscribe(trans->client, topic, qos);
	if(ret != MQTTCLIENT_SUCCESS){
    	errorf("failed to subscribe, return code %d\n", ret);
    	return ret;
    }

	return 0;
}

int mqtt_transport_publish(mqtt_transport* trans, char* topic, int qos, void* payload, int payloadlen){
	int ret;
	MQTTClient_message pubmsg = MQTTClient_message_initializer;
    MQTTClient_deliveryToken token;

	if(trans == NULL){
		errorf("transport object is NULL \r\n");
		return -1;
	}

	//infof("mqtt.c publish payload = %s \r\n", payload);
	pubmsg.payload = payload;
    pubmsg.payloadlen = payloadlen;
    pubmsg.qos = qos;
    pubmsg.retained = 0;

	ret = MQTTClient_publishMessage(trans->client, topic, &pubmsg, &token);
	if(ret != MQTTCLIENT_SUCCESS){
		 errorf("Failed to publish message, return code %d\n", ret);
         return ret;
	}

	ret = MQTTClient_waitForCompletion(trans->client, token, 10000);
	if(ret != MQTTCLIENT_SUCCESS){
		 errorf("wait timeout, return code %d\n", ret);
         return ret;
	}

	return 0;
}

void on_connlost(void* context, char* cause){
	mqtt_transport* trans = (mqtt_transport*)context;

	if(trans == NULL){
		errorf("invalid context...");
		return;
	}

	trans->connected = 0;

	if(trans->on_connlost)
		trans->on_connlost(context, cause);
}

int on_msgarrvd(void* context, char* topicName, int topicLen, MQTTClient_message* message){
	int payload_len = message->payloadlen;
	char* payload = (char *)message->payload;
	mqtt_transport* trans = (mqtt_transport*)context;

	if(trans == NULL){
		errorf("invalid context...");
		return 1;
	}

	//infof("payload length %d, payload = %s \r\n", payload_len, payload);
	if(trans->msgarrvd)
		trans->msgarrvd(context, topicName, payload, payload_len);

	MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);

	return 1;
}

int mqtt_transport_unsubscribe(mqtt_transport* trans, char* topic){
	int ret;

	if(trans == NULL || topic == NULL) {
		return -1;
	}

	ret = MQTTClient_unsubscribe(trans->client, topic);
	if(ret != MQTTCLIENT_SUCCESS){
		errorf("Failed to unsubscribe, return code %d\n", ret);
        return ret;
	}

	return 0;
}

int mqtt_transport_disconnect(mqtt_transport* trans){
	int ret;

	if(trans == NULL) {
		return -1;
	}

	if(!trans->connected){
		return 0;
	}

	ret = MQTTClient_disconnect(trans->client, 10000);
	if (ret != MQTTCLIENT_SUCCESS) {
		errorf("MqttBase: transport_disconnect() error, release result %d\n", ret);
		return ret;
	}

	trans->connected = 0; 

	return 0;
}

void mqtt_destory_transport(mqtt_transport* trans){
	if(trans == NULL) {
		return ;
	}

	if(trans->srv_uri)
		free_memory(&trans->srv_uri);
	if(trans->usr)
		free_memory(&trans->usr);
	if(trans->pwd)
		free_memory(&trans->pwd);

	if(trans->client)
		MQTTClient_destroy(&trans->client);
}
