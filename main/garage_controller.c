#include <stdio.h>
#include "esp_event.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "iocontrollers.h"
#include "nvs_flash.h"
#include "protocol_examples_common.h"
#include "temperature.h"

void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    esp_netif_init();
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    ESP_ERROR_CHECK(example_connect());

    init_io_controller();
    ESP_ERROR_CHECK(output_activate(&RELAY1, 1));
    ESP_ERROR_CHECK(output_activate(&RELAY2, 1));
    ESP_ERROR_CHECK(output_activate(&OPTO1, 1));

    temp_sensor_init();
    temp_sensor_turn_on();

    while(1) {
        printf("pinumber[%d] = %d\n", INPUT1.pinNumber, input_read(&INPUT1));
        printf("pinumber[%d] = %d\n", INPUT2.pinNumber, input_read(&INPUT2));
        printf("pinumber[%d] = %d\n", SW1.pinNumber, input_read(&SW1));
        printf("pinumber[%d] = %d\n\n", SW2.pinNumber, input_read(&SW2));
        if (input_read(&SW1) == 0) {
            ESP_ERROR_CHECK(output_activate(&RELAY1, 1));
            ESP_ERROR_CHECK(output_activate(&GREEN_LED1, 1));
        } else {
            ESP_ERROR_CHECK(output_activate(&RELAY1, 0));
            ESP_ERROR_CHECK(output_activate(&GREEN_LED1, 0));
        }
        if (input_read(&SW2) == 0) {
            ESP_ERROR_CHECK(output_activate(&RELAY2, 1));
            ESP_ERROR_CHECK(output_activate(&GREEN_LED2, 1));
            
        } else {
            ESP_ERROR_CHECK(output_activate(&RELAY2, 0));
            ESP_ERROR_CHECK(output_activate(&GREEN_LED2, 0));
        }
        printf("Temperature: %fC\n", temp_sensor_get_temperature());
        vTaskDelay(1000 / portTICK_RATE_MS);
    }
}
