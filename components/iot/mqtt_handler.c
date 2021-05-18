#include "esp_eth.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "freertos/FreeRTOS.h"
#include "freeRTOS/task.h"
#include "iocontrollers.h"
#include <math.h>
#include "mqtt_client.h"
#include "mqtt_handler.h"
#include <stdio.h>
#include "temperature.h"

static const char* temp_id = "6050bdd81d065424b548c632";
static const char* relay1_id = "6048e887d1c2d77d1bd58f16";
static const char* relay2_id = "6048e887d1c2d77d1bd58f17";
static const char* opto1_id = "6048e887d1c2d77d1bd58f18";
static const char* opto2_id = "6048e887d1c2d77d1bd58f19";
static const char* door_magnet_id = "604907d8d1c2d77d1bd58f1f";
static const char* door_magnet2_id = "604907d8d1c2d77d1bd58f1g";
esp_mqtt_client_handle_t client;

static const char* TAG = "mqtt_handler";

void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0) {
        ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
    }
}

void mqtt_subscribed_action_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    char topic[128];
    ESP_LOGI(TAG, "Action event handler");
    esp_mqtt_event_handle_t event = event_data;
    if ((esp_mqtt_event_id_t)event_id != MQTT_EVENT_DATA) {
        ESP_LOGE(TAG, "Optained unknown event id. This hanlder is for handling subsribed actions.");
        return;
    }

    strcpy(topic, relay1_id);
    if (strncmp(strcat(topic, "/directive/powerState"), event->topic, event->topic_len) == 0) {
        if (strncmp(event->data, "ON", event->data_len) == 0) {
            ESP_LOGD(TAG, "Turning ON relay1");
            io_controllers_output_activate(&RELAY1, 1);
        } else if (strncmp(event->data, "OFF", event->data_len) == 0) {
            ESP_LOGD(TAG, "Turning OFF relay1");
            io_controllers_output_activate(&RELAY1, 0);
        } else {
            goto fail;
        }
        return;
    }
    strcpy(topic, relay2_id);
    if (strncmp(strcat(topic, "/directive/powerState"), event->topic, event->topic_len) == 0) {
        if (strncmp(event->data, "ON", event->data_len) == 0) {
            ESP_LOGD(TAG, "Turning ON relay2");
            io_controllers_output_activate(&RELAY2, 1);
        } else if (strncmp(event->data, "OFF", event->data_len) == 0) {
            ESP_LOGD(TAG, "Turning OFF relay2");
            io_controllers_output_activate(&RELAY2, 0);
        } else {
            goto fail;
        }
        return;
    }
    strcpy(topic, opto1_id);
    if (strncmp(strcat(topic, "/directive/powerState"), event->topic, event->topic_len) == 0) {
        if (strncmp(event->data, "ON", event->data_len) == 0) {
            ESP_LOGD(TAG, "Turning ON opto1");
            io_controllers_output_activate(&OPTO1, 1);
        } else if (strncmp(event->data, "OFF", event->data_len) == 0) {
            ESP_LOGD(TAG, "Turning OFF opto1");
            io_controllers_output_activate(&OPTO1, 0);
        } else {
            goto fail;
        }
        return;
    }
    strcpy(topic, opto2_id);
    if (strncmp(strcat(topic, "/directive/powerState"), event->topic, event->topic_len) == 0) {
        if (strncmp(event->data, "ON", event->data_len) == 0) {
            ESP_LOGD(TAG, "Turning ON opto2");
            io_controllers_output_activate(&OPTO2, 1);
        } else if (strncmp(event->data, "OFF", event->data_len) == 0) {
            ESP_LOGD(TAG, "Turning OFF opto2");
            io_controllers_output_activate(&OPTO2, 0);
        } else {
            goto fail;
        }
        return;
    }

    fail:
        ESP_LOGE(TAG, "Unknown topic \'%.*s\' with msg \'%.*s\'", event->topic_len, event->topic, event->data_len, event->data);
}

void report_current_state(esp_mqtt_client_handle_t client) {
    char topic[128];
    int state;
    strcpy(topic, relay1_id);
    state = gpio_get_level(RELAY1.pinNumber);
    if (state == 1) {
        esp_mqtt_client_publish(client, strcat(topic, "/report/powerState"), "ON", 0, 1, 1);
    } else {
        esp_mqtt_client_publish(client, strcat(topic, "/report/powerState"), "OFF", 0, 1, 1);
    }
    strcpy(topic, relay2_id);
    state = gpio_get_level(RELAY2.pinNumber);
    if (state == 1) {
        esp_mqtt_client_publish(client, strcat(topic, "/report/powerState"), "ON", 0, 1, 1);
    } else {
        esp_mqtt_client_publish(client, strcat(topic, "/report/powerState"), "OFF", 0, 1, 1);
    }
    strcpy(topic, opto1_id);
    state = gpio_get_level(OPTO1.pinNumber);
    if (state == 1) {
        esp_mqtt_client_publish(client, strcat(topic, "/report/powerState"), "ON", 0, 1, 1);
    } else {
        esp_mqtt_client_publish(client, strcat(topic, "/report/powerState"), "OFF", 0, 1, 1);
    }
    strcpy(topic, opto2_id);
    state = gpio_get_level(OPTO2.pinNumber);
    if (state == 1) {
        esp_mqtt_client_publish(client, strcat(topic, "/report/powerState"), "ON", 0, 1, 1);
    } else {
        esp_mqtt_client_publish(client, strcat(topic, "/report/powerState"), "OFF", 0, 1, 1);
    }
    strcpy(topic, door_magnet_id);
    state = gpio_get_level(INPUT1.pinNumber);
    if (state == 1) {
        esp_mqtt_client_publish(client, strcat(topic, "/report/detectionState"), "false", 0, 1, 1);
    } else {
        esp_mqtt_client_publish(client, strcat(topic, "/report/detectionState"), "true", 0, 1, 1);
    }
    strcpy(topic, door_magnet2_id);
    state = gpio_get_level(INPUT2.pinNumber);
    if (state == 1) {
        esp_mqtt_client_publish(client, strcat(topic, "/report/detectionState"), "false", 0, 1, 1);
    } else {
        esp_mqtt_client_publish(client, strcat(topic, "/report/detectionState"), "true", 0, 1, 1);
    }
}

void mqtt_basic_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        char topic[128];
        //report online status (relay 1 is used as primary id - last will message)
        strcpy(topic, relay1_id);
        msg_id = esp_mqtt_client_publish(client, strcat(topic, "/report/online"), "true", 0, 1, 1);
        ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);

        //subscribe to topics
        strcpy(topic, relay1_id);
        msg_id = esp_mqtt_client_subscribe(client, strcat(topic, "/directive/powerState"), 1);
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
        strcpy(topic, relay2_id);
        msg_id = esp_mqtt_client_subscribe(client, strcat(topic, "/directive/powerState"), 1);
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
        strcpy(topic, opto1_id);
        msg_id = esp_mqtt_client_subscribe(client, strcat(topic, "/directive/powerState"), 1);
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
        strcpy(topic, opto2_id);
        msg_id = esp_mqtt_client_subscribe(client, strcat(topic, "/directive/powerState"), 1);
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

        //report current state
        report_current_state(client);
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        break;
    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
            log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno",  event->error_handle->esp_transport_sock_errno);
            ESP_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));
        }
        break;
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}

void mqtt_outputs_publish_handler(void* event_handler_arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    esp_mqtt_client_handle_t client = event_handler_arg;
    if (event_base != OUTPUT_BASE) {
        ESP_LOGE(TAG, "Bad handler type");
        return;
    }
    char topic[128];
    int msg_id;
    switch (event_id) {
        case RELAY1_ON:
            strcpy(topic, relay1_id);
            msg_id = esp_mqtt_client_publish(client, strcat(topic, "/report/powerState"), "ON", 0, 1, 1);
            ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
            break;
        case RELAY1_OFF:
            strcpy(topic, relay1_id);
            msg_id = esp_mqtt_client_publish(client, strcat(topic, "/report/powerState"), "OFF", 0, 1, 1);
            ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
            break;
        case RELAY2_ON:
            strcpy(topic, relay2_id);
            msg_id = esp_mqtt_client_publish(client, strcat(topic, "/report/powerState"), "ON", 0, 1, 1);
            ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
            break;
        case RELAY2_OFF:
            strcpy(topic, relay2_id);
            msg_id = esp_mqtt_client_publish(client, strcat(topic, "/report/powerState"), "OFF", 0, 1, 1);
            ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
            break;
        case OPTO1_ON:
            strcpy(topic, opto1_id);
            msg_id = esp_mqtt_client_publish(client, strcat(topic, "/report/powerState"), "ON", 0, 1, 1);
            ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
            break;
        case OPTO1_OFF:
            strcpy(topic, opto1_id);
            msg_id = esp_mqtt_client_publish(client, strcat(topic, "/report/powerState"), "OFF", 0, 1, 1);
            ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
            break;
        case OPTO2_ON:
            strcpy(topic, opto2_id);
            msg_id = esp_mqtt_client_publish(client, strcat(topic, "/report/powerState"), "ON", 0, 1, 1);
            ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
            break;
        case OPTO2_OFF:
            strcpy(topic, opto2_id);
            msg_id = esp_mqtt_client_publish(client, strcat(topic, "/report/powerState"), "OFF", 0, 1, 1);
            ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
            break;
    }
}

void mqtt_inputs_publish_handler(void* event_handler_arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    esp_mqtt_client_handle_t client = event_handler_arg;
    if (event_base != INPUT_BASE) {
        ESP_LOGE(TAG, "Bad handler type");
        return;
    }
    char topic[128];
    int msg_id;
    switch (event_id) {
        case INPUT_1_PRESSED:
            strcpy(topic, door_magnet_id);
            msg_id = esp_mqtt_client_publish(client, strcat(topic, "/report/detectionState"), "true", 0, 1, 1);
            ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
            break;
        case INPUT_1_RELEASED:
            strcpy(topic, door_magnet_id);
            msg_id = esp_mqtt_client_publish(client, strcat(topic, "/report/detectionState"), "false", 0, 1, 1);
            ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
            break;
        case INPUT_2_PRESSED:
            strcpy(topic, door_magnet2_id);
            msg_id = esp_mqtt_client_publish(client, strcat(topic, "/report/detectionState"), "true", 0, 1, 1);
            ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
            break;
        case INPUT_2_RELEASED:
            strcpy(topic, door_magnet2_id);
            msg_id = esp_mqtt_client_publish(client, strcat(topic, "/report/detectionState"), "false", 0, 1, 1);
            ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
            break;
    }
}

//round to nearest half degree
static double temperature_round(double measured_value) {
    ESP_LOGD(TAG, "Rounding value %.2f", measured_value);
    double mid_result = measured_value * 10;
    int mod = (int)mid_result % 10;
    if(mod < 3) {
        return floor(measured_value);
    } else if (mod < 8) {
        return (floor(measured_value) + 0.5);
    } else {
        return ceil(measured_value);
    }
}

void measure_temperature_task(void *args) {
    char topic[128];
    char text_result[10];
    double result = 0;
    double temperature = 0;
    int counter = 0;
    while(1) {
        counter++;
        //temp_sensor_turn_on();
        temperature = temp_sensor_get_temperature();
        //temp_sensor_turn_off();
        result += temperature;
        ESP_LOGD(TAG, "Temperature is: %.2f", temperature);
        if(counter == 20) {
            result /= 20;
            result = temperature_round(result);
            strcpy(topic, temp_id);
            sprintf(text_result, "%.1f", result);
            ESP_LOGI(TAG, "Average posted temperature is: %s", text_result);
            esp_mqtt_client_publish(client, strcat(topic, "/report/temperature"), text_result, 0, 1, 1);
            counter = 0;
            result = 0;
        }
        vTaskDelay(100);
    }
}

void mqtt_init(void)
{
    char topic[128];
    strcpy(topic, relay1_id);
    esp_mqtt_client_config_t mqtt_cfg = {
        .uri = "mqtt://192.168.1.22:1883",
        .username = "domeniqo",
        .password = "Sasenka1",
        .client_id = relay1_id,
        .lwt_topic = strcat(topic, "/report/online"),
        .lwt_msg = "false",
        .lwt_msg_len = 0,
        .lwt_qos = 2,
        .lwt_retain = 1,
        .keepalive = 10
    };
    client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_basic_event_handler, NULL);
    esp_mqtt_client_register_event(client, MQTT_EVENT_DATA, mqtt_subscribed_action_event_handler, NULL);
    esp_event_handler_instance_register(OUTPUT_BASE, ESP_EVENT_ANY_ID, mqtt_outputs_publish_handler, client, NULL);
    esp_event_handler_instance_register(INPUT_BASE, ESP_EVENT_ANY_ID, mqtt_inputs_publish_handler, client, NULL);
    temp_sensor_init();
    temp_sensor_turn_on();
    xTaskCreate(measure_temperature_task, "periodical temp check", 2048, NULL, uxTaskPriorityGet(NULL), NULL);
    esp_mqtt_client_start(client);
}