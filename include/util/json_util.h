#ifndef _JSON_UTIL_H_
#define  _JSON_UTIL_H_

#include <util/cJSON.h>

typedef int JSON_BOOL;

#define JSON_FALSE 0
#define JSON_TRUE 1

cJSON* json_create_object();
cJSON* json_create_array();
cJSON* json_create_int_array(const int *numbers, int count);
cJSON* json_create_float_array(const float *numbers, int count);
cJSON* json_create_double_array(const double *numbers, int count);
cJSON* json_create_string_array(const char **strings, int count);
void json_add_string_to_object(cJSON *object, const char *key, const char *value);
void json_add_number_to_object(cJSON *object, const char *key, double value);
void json_add_bool_to_object(cJSON *object, const char *key, JSON_BOOL value);
void json_add_object_to_object(cJSON *object, const char *key, cJSON *value);
void json_add_object_to_array(cJSON *arrayObject, cJSON *value);
char* json_print(const cJSON *object);
cJSON* json_parse(const char *value);
void json_delete(cJSON *object);
int json_get_int_from_object(const cJSON *object, const char *key, const int defaultValue);
JSON_BOOL json_get_bool_from_object(const cJSON *object, const char *key, const JSON_BOOL defaultValue);
double json_get_double_from_object(const cJSON *object, const char *key, const double defaultValue);
char* json_get_string_from_object(const cJSON *object, const char *key);
cJSON* json_get_object_from_object(const cJSON *object, const char *key);
int json_get_array_size(const cJSON *array);
int json_get_int_from_array(const cJSON *array, int index, const int defaultValue);
JSON_BOOL json_get_bool_from_array(const cJSON *array, int index, const JSON_BOOL defaultValue);
double json_get_double_from_array(const cJSON *array, int index, const double defaultValue);
char* json_get_string_from_array(const cJSON *array, int index);
cJSON* json_get_object_from_array(const cJSON *array, int index);
int json_is_array(const cJSON * const item);
int json_is_object(const cJSON * const item);
#endif
