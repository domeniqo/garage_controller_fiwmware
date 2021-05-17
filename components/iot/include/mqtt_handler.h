#ifndef MQTT_HANDLER_H
#define MQTT_HANDLER_H

#include "esp_event.h"

void log_error_if_nonzero(const char *message, int error_code);

void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);

void mqtt_init(void);

#endif //MQTT_HANDLER_H