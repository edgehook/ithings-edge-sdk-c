#include "util/json_util.h"

cJSON* json_create_object() {
	return (cJSON*) cJSON_CreateObject();
}

cJSON* json_create_array() {
	return (cJSON*) cJSON_CreateArray();
}

cJSON* json_create_int_array(const int *numbers, int count) {
	return (cJSON*) cJSON_CreateIntArray(numbers, count);
}

cJSON* json_create_float_array(const float *numbers, int count) {
	return (cJSON*) cJSON_CreateFloatArray(numbers, count);
}

cJSON* json_create_double_array(const double *numbers, int count) {
	return (cJSON*) cJSON_CreateDoubleArray(numbers, count);
}

cJSON* json_create_string_array(const char **strings, int count) {
	return (cJSON*) cJSON_CreateStringArray(strings, count);
}

void json_add_string_to_object(cJSON *object, const char *key, const char *value) {
	cJSON *item = cJSON_CreateString(value);
	cJSON_AddItemToObject((cJSON*) object, key, item);
}

void json_add_number_to_object(cJSON *object, const char *key, double value) {
	cJSON *item = cJSON_CreateNumber(value);
	cJSON_AddItemToObject((cJSON*) object, key, item);
}

void json_add_bool_to_object(cJSON *object, const char *key, JSON_BOOL value) {
	cJSON *item = cJSON_CreateBool(value);
	cJSON_AddItemToObject((cJSON*) object, key, item);
}

void json_add_object_to_object(cJSON *object, const char *key, cJSON *value) {
	cJSON_AddItemToObject((cJSON*) object, key, (cJSON*) value);
}

void json_add_object_to_array(cJSON *arrayObject, cJSON *value) {
	cJSON_AddItemToArray((cJSON*) arrayObject, (cJSON*) value);
}

char* json_print(const cJSON *object) {
	return cJSON_Print((const cJSON*) object);
}

cJSON* json_parse(const char *value) {
	return (cJSON*) cJSON_Parse(value);
}

void json_delete(cJSON *object) {
	cJSON_Delete((cJSON*) object);
}

int json_get_int_from_object(const cJSON *object, const char *key, const int defaultValue) {
	cJSON *t_object = (cJSON*) cJSON_GetObjectItem((const cJSON*) object, key);
	return t_object ? t_object->valueint : defaultValue;
}

JSON_BOOL json_get_bool_from_object(const cJSON *object, const char *key, const JSON_BOOL defaultValue) {
	cJSON *t_object = (cJSON*) cJSON_GetObjectItem((const cJSON*) object, key);
	return t_object ? t_object->valueint : defaultValue;
}

double json_get_double_from_object(const cJSON *object, const char *key, const double defaultValue) {
	cJSON *t_object = (cJSON*) cJSON_GetObjectItem((const cJSON*) object, key);
	return t_object ? t_object->valuedouble : defaultValue;
}

char* json_get_string_from_object(const cJSON *object, const char *key) {
	cJSON *t_object = (cJSON*) cJSON_GetObjectItem((const cJSON*) object, key);
	return t_object ? t_object->valuestring : NULL;
}

cJSON* json_get_object_from_object(const cJSON *object, const char *key) {
	return (cJSON*) cJSON_GetObjectItem((const cJSON*) object, key);
}

int json_get_array_size(const cJSON *array) {
	if (array == NULL) {
		return 0;
	}
	return cJSON_GetArraySize((const cJSON*) array);
}

int json_get_int_from_array(const cJSON *array, int index, const int defaultValue) {
	cJSON *t_object = (cJSON*) cJSON_GetArrayItem((const cJSON*) array, index);
	return t_object ? t_object->valueint : defaultValue;
}

JSON_BOOL json_get_bool_from_array(const cJSON *array, int index, const JSON_BOOL defaultValue) {
	cJSON *t_object = (cJSON*) cJSON_GetArrayItem((const cJSON*) array, index);
	return t_object ? t_object->valueint : defaultValue;
}

double json_get_double_from_array(const cJSON *array, int index, const double defaultValue) {
	cJSON *t_object = (cJSON*) cJSON_GetArrayItem((const cJSON*) array, index);
	return t_object ? t_object->valuedouble : defaultValue;
}

char* json_get_string_from_array(const cJSON *array, int index) {
	cJSON *t_object = (cJSON*) cJSON_GetArrayItem((const cJSON*) array, index);
	return t_object ? t_object->valuestring : NULL;
}

cJSON* json_get_object_from_array(const cJSON *array, int index) {
	return (cJSON*) cJSON_GetArrayItem((const cJSON*) array, index);
}

int json_is_array(const cJSON * const item){
	return (int)cJSON_IsArray(item);
}
int json_is_object(const cJSON * const item){
	return (int)cJSON_IsObject(item);
}