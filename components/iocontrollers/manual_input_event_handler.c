#include "esp_event.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freeRTOS/task.h"
#include "iocontrollers.h"
#include "manual_input_event_handler.h"

static const char* TAG = "manual input event handler";

void manual_input_event_handler_init() {
    //event loop init and registration
    ESP_ERROR_CHECK(esp_event_handler_instance_register(INPUT_BASE, ESP_EVENT_ANY_ID, normal_mode_input_event_handler, NULL, NULL));
}

void generate_long_press_event(void *arg) {
    enum INPUT_EVENT_IDS eventId = (enum INPUT_EVENT_IDS) arg;
    esp_event_post(INPUT_BASE, eventId, NULL, 0, portMAX_DELAY);
}

void press_down_counter_task(void *arg) {
    //ESP_LOGI(TAG, "counter task execution start");
    counter_task_args *counterTaskArgs = (counter_task_args *) arg;
    int cnt = 0;
    //ESP_LOGI(TAG, "reading state");
    while(io_controllers_input_read(counterTaskArgs->input) == 0 && cnt <= counterTaskArgs->max_count) {
        cnt++;
        vTaskDelay(1);
    }
    if(cnt > counterTaskArgs->max_count) {
        ESP_LOGI(TAG, "long press for pin %d: %d", counterTaskArgs->input->pinNumber, cnt);
        counterTaskArgs->fn_to_execute(counterTaskArgs->args);
    }
    vTaskDelete(NULL);
}

counter_task_args counterArgs;

void normal_mode_input_event_handler(void* event_handler_arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    ESP_LOGI(TAG, "event handling %d", event_id);    
    switch(event_id) {
        case SW_1_PRESSED:
            //ESP_LOGI(TAG, "giving input");
            counterArgs.input = &SW1;
            //ESP_LOGI(TAG, "giving max count");
            counterArgs.max_count = 200;
            //ESP_LOGI(TAG, "giving fn");
            counterArgs.fn_to_execute = generate_long_press_event;
            //ESP_LOGI(TAG, "giving args");
            counterArgs.args = SW_1_LONG_PRESS;
            //ESP_LOGI(TAG, "creating task");
            xTaskCreate(press_down_counter_task, "countdown sw1 task", 2048, (void*)(&counterArgs), 0, NULL);
            break;
        case SW_2_PRESSED:
            counterArgs.input = &SW2;
            counterArgs.max_count = 200;
            counterArgs.fn_to_execute = generate_long_press_event;
            counterArgs.args = SW_2_LONG_PRESS;
            xTaskCreate(press_down_counter_task, "countdown sw2 task", 2048, (void*)(&counterArgs), 0, NULL);
            break;
        case SW_1_RELEASED:
            if(RELAY1.switchMode == SWITCH_MODE_SIMPLE) {
                io_controllers_set_output_switch_mode(&RELAY1, SWITCH_MODE_TOGGLE);
                io_controllers_output_activate(&RELAY1, 1);
                io_controllers_set_output_switch_mode(&RELAY1, SWITCH_MODE_SIMPLE);
            } else {
                io_controllers_output_activate(&RELAY1, 1);
            }
            break;
        case SW_2_RELEASED:
            if(RELAY2.switchMode == SWITCH_MODE_SIMPLE) {
                io_controllers_set_output_switch_mode(&RELAY2, SWITCH_MODE_TOGGLE);
                io_controllers_output_activate(&RELAY2, 1);
                io_controllers_set_output_switch_mode(&RELAY2, SWITCH_MODE_SIMPLE);
            } else {
                io_controllers_output_activate(&RELAY2, 1);
            }
            break;
        case SW_1_LONG_PRESS:
            io_controllers_output_activate(&GREEN_LED1, 1);
            break;
        case SW_2_LONG_PRESS:
            io_controllers_output_activate(&GREEN_LED2, 1);
            break;
        default:
            ESP_LOGE(TAG, "event base: %s event id: %d - no routine in normal_input_event_handler", event_base, event_id);
    }
}
