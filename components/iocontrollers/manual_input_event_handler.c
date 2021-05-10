#include "esp_event.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freeRTOS/task.h"
#include "iocontrollers.h"
#include "manual_input_event_handler.h"

static const char* TAG = "manual input event handler";

ESP_EVENT_DEFINE_BASE(INPUT_BASE);

void manual_input_event_handler_init() {
    
    //task for periodically checking inputs states after interrupt was called
    xTaskCreate(check_inputs_task, "check inputs task", 2048, NULL, uxTaskPriorityGet(NULL), NULL);

    //event loop init and registration
    esp_event_loop_create_default();
    esp_event_handler_instance_register(INPUT_BASE, ESP_EVENT_ANY_ID, normal_mode_input_event_handler, NULL, NULL);
}

void generate_long_press_event(void *arg) {
    enum INPUT_EVENT_IDS eventId = (enum INPUT_EVENT_IDS) arg;
    esp_event_post(INPUT_BASE, eventId, NULL, 0, portMAX_DELAY);
}

void press_down_counter_task(void *arg) {
    counter_task_args *counterTaskArgs = (counter_task_args *) arg;
    int cnt = 0;
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

void check_inputs_task(void *arg) {
    while(1) {
        if(isr_mask & 0x01) {
            isr_mask &= 0xfe;
            if(gpio_get_level(2)) {
                ESP_LOGI(TAG, "in1 up");
                esp_event_post(INPUT_BASE, INPUT_1_RELEASED, NULL, 0, portMAX_DELAY);
            } else {
                ESP_LOGI(TAG, "in1 down");
                esp_event_post(INPUT_BASE, INPUT_1_PRESSED, NULL, 0, portMAX_DELAY);
            }
        }
        if(isr_mask & 0x02) {
            isr_mask &= 0xfd;
            if(gpio_get_level(4)) {
                ESP_LOGI(TAG, "in2 up");
                esp_event_post(INPUT_BASE, INPUT_2_RELEASED, NULL, 0, portMAX_DELAY);
            } else {
                ESP_LOGI(TAG, "in2 down");
                esp_event_post(INPUT_BASE, INPUT_2_PRESSED, NULL, 0, portMAX_DELAY);
            }
        }
        if(isr_mask & 0x04) {
            isr_mask &= 0xfb;
            if(gpio_get_level(35)) {
                ESP_LOGI(TAG, "sw1 up");
                esp_event_post(INPUT_BASE, SW_1_RELEASED, NULL, 0, portMAX_DELAY);
            } else {
                ESP_LOGI(TAG, "sw1 down");
                esp_event_post(INPUT_BASE, SW_1_PRESSED, NULL, 0, portMAX_DELAY);
            }
        }
        if(isr_mask & 0x08) {
            isr_mask &= 0xf7;
            if(gpio_get_level(36)) {
                ESP_LOGI(TAG, "sw2 up");
                esp_event_post(INPUT_BASE, SW_2_RELEASED, NULL, 0, portMAX_DELAY);
            } else {
                ESP_LOGI(TAG, "sw2 down");
                esp_event_post(INPUT_BASE, SW_2_PRESSED, NULL, 0, portMAX_DELAY);
            }
        }
        vTaskDelay(30);
    }
}

void normal_mode_input_event_handler(void* event_handler_arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    ESP_LOGI(TAG, "event handling %d", event_id);
    counter_task_args counterArgs;
    switch(event_id) {
        case SW_1_PRESSED:
            counterArgs.input = &SW1;
            counterArgs.max_count = 200;
            counterArgs.fn_to_execute = generate_long_press_event;
            counterArgs.args = SW_1_LONG_PRESS;
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
