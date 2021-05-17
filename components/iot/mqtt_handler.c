#include "esp_eth.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "freertos/FreeRTOS.h"
#include "freeRTOS/task.h"
#include <math.h>
#include "mqtt_client.h"
#include "mqtt_handler.h"
#include <stdio.h>
#include "temperature.h"

static const char* temp_id = "6050bdd81d065424b548c632";
static const char* relay1_id = "6048e887d1c2d77d1bd58f16";
static const char* door_magnet_id = "604907d8d1c2d77d1bd58f1f";
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
    esp_mqtt_client_handle_t client = event->client;
    if ((esp_mqtt_event_id_t)event_id != MQTT_EVENT_DATA) {
        ESP_LOGE(TAG, "Optained unknown event id. This hanlder is for handling subsribed actions.");
        return;
    }

    strcpy(topic, relay1_id);
    if (strncmp(strcpy(topic, "/directive/powerState"), event->topic, event->topic_len) == 0) {
        if (strncmp(event->data, "ON", event->data_len) == 0) {
            ESP_LOGD(TAG, "Turning ON relay1");
        } else if (strncmp(event->data, "OFF", event->data_len) == 0) {
            ESP_LOGD(TAG, "Turning OFF relay1");
        } else {
            goto fail;
        }
        return;
    }

    fail:
        ESP_LOGE(TAG, "Unknown topic \'%.*s\' with msg \'%.*s\'", event->topic_len, event->topic, event->data_len, event->data);
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
        strcpy(topic, relay1_id);
        msg_id = esp_mqtt_client_publish(client, strcat(topic, "/report/online"), "true", 0, 1, 1);
        ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
        strcpy(topic, temp_id);
        msg_id = esp_mqtt_client_publish(client, strcat(topic, "/report/online"), "true", 0, 1, 1);
        ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
        strcpy(topic, door_magnet_id);
        msg_id = esp_mqtt_client_publish(client, strcat(topic, "/report/online"), "true", 0, 1, 1);
        ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);

        strcpy(topic, relay1_id);
        msg_id = esp_mqtt_client_subscribe(client, strcat(topic, "/directive/powerState"), 1);
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
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
        temp_sensor_turn_on();
        temperature = temp_sensor_get_temperature();
        temp_sensor_turn_off();
        result += temperature;
        ESP_LOGD(TAG, "Temperature is: %.2f", temperature);
        if(counter == 5) {
            result /= 5;
            result = temperature_round(result);
            strcpy(topic, temp_id);
            sprintf(text_result, "%.2f", result);
            ESP_LOGI(TAG, "Average posted temperature is: %s", text_result);
            esp_mqtt_client_publish(client, strcat(topic, "/report/temperature"), text_result, 0, 1, 1);
            counter = 0;
            result = 0;
        }
        vTaskDelay(300);
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
    temp_sensor_init();
    xTaskCreate(measure_temperature_task, "periodical temp check", 2048, NULL, uxTaskPriorityGet(NULL), NULL);
    esp_mqtt_client_start(client);
}