#include "esp_eth.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "ethernet_handler.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "iocontrollers.h"
#include "manual_input_event_handler.h"
#include "mqtt_handler.h"
#include "nvs_flash.h"
#include "temperature.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include <stdio.h>
#include "wifi_handler.h"

static const char* TAG = "garage_controller";

void app_main(void)
{
    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set("esp_netif_handlers", ESP_LOG_VERBOSE);
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    ethernet_init();
    mqtt_app_start();
    //ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_ETH_GOT_IP, &mqtt_got_ip_handler, NULL, NULL));
    //wifi_init_sta();
    //io_controllers_init();
    //manual_input_event_handler_init();

    //mqtt_app_start();
    while(1) {
        ESP_LOGI(TAG, "main checking");
        vTaskDelay(500);
    }
    
}
