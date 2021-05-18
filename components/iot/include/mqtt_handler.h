#ifndef MQTT_HANDLER_H
#define MQTT_HANDLER_H

#include "esp_event.h"
#include "mqtt_client.h"

void log_error_if_nonzero(const char *message, int error_code);

void mqtt_subscribed_action_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);

void mqtt_basic_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);

void mqtt_outputs_publish_handler(void* event_handler_arg, esp_event_base_t event_base, int32_t event_id, void* event_data);

void mqtt_inputs_publish_handler(void* event_handler_arg, esp_event_base_t event_base, int32_t event_id, void* event_data);

void report_current_state(esp_mqtt_client_handle_t client);

void mqtt_init(void);

#endif //MQTT_HANDLER_H