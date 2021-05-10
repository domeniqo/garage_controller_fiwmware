#include <stdio.h>
#include "esp_event.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "iocontrollers.h"
#include "manual_input_event_handler.h"
#include "nvs_flash.h"
#include "protocol_examples_common.h"
#include "temperature.h"

static const char* TAG = "garage_controller";

void app_main(void)
{
    //ESP_ERROR_CHECK(nvs_flash_init());
    //esp_netif_init();
    //ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    //ESP_ERROR_CHECK(example_connect());

    io_controllers_init();
    manual_input_event_handler_init();

    while(1) {
        ESP_LOGI(TAG, "main checking");
        vTaskDelay(500);
    }
    
}
