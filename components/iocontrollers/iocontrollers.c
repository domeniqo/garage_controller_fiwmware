#include "iocontrollers.h"
#include "esp_event.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freeRTOS/task.h"

ESP_EVENT_DEFINE_BASE(INPUT_BASE);

enum INPUT_EVENT_IDS {
    SW_1_PRESSED,
    SW_1_RELEASED,
    SW_1_LONG_PRESS,
    SW_2_PRESSED,
    SW_2_RELEASED,
    SW_2_LONG_PRESS,
    INPUT_1_PRESSED,
    INPUT_1_RELEASED,
    INPUT_1_LONG_PRESS,
    INPUT_2_PRESSED,
    INPUT_2_RELEASED,
    INPUT_2_LONG_PRESS
};

static const char* TAG = "iocontroller";

uint8_t isr_mask = 0;

void isr_handler(void *value) {
    uint8_t mask = (uint8_t)value;
    isr_mask |= mask;
}

void button_output_activate(void *arg) {
    Output *out = (Output *) arg;
    int lvl = gpio_get_level(out->pinNumber);
    io_controllers_output_activate(out, (lvl ^ 1));
}


void generate_long_press_event(void *arg) {
    enum INPUT_EVENT_IDS eventId = (enum INPUT_EVENT_IDS) arg;
    esp_event_post(INPUT_BASE, eventId, NULL, 0, portMAX_DELAY);
}

Output RELAY1 = {
    .pinNumber = RELAY_1_PIN,
    .switchMode = SWITCH_MODE_SIMPLE,
    .delayMicros = 0
};

Output RELAY2 = {
    .pinNumber = RELAY_2_PIN,
    .switchMode = SWITCH_MODE_SIMPLE,
    .delayMicros = 0
};

Output OPTO1 = {
    .pinNumber = OPTO_1_PIN,
    .switchMode = SWITCH_MODE_SIMPLE,
    .delayMicros = 0
};

Output OPTO2 = {
    .pinNumber = OPTO_2_PIN,
    .switchMode = SWITCH_MODE_SIMPLE,
    .delayMicros = 0
};

Input SW1 = {
    .pinNumber = SW_1_PIN,
    .pullUp = true
};

Input SW2 = {
    .pinNumber = SW_2_PIN,
    .pullUp = true
};

Input INPUT1 = {
    .pinNumber = INPUT_1_PIN,
    .pullUp = true
};

Input INPUT2 = {
    .pinNumber = INPUT_2_PIN,
    .pullUp = true
};

const Output GREEN_LED1 = {
    .pinNumber = GREEN_LED_1_PIN,
    .switchMode = SWITCH_MODE_SIMPLE,
    .delayMicros = 0
};

const Output GREEN_LED2 = {
    .pinNumber = GREEN_LED_2_PIN,
    .switchMode = SWITCH_MODE_SIMPLE,
    .delayMicros = 0
};

typedef struct counter_task_args {
    Input *input;
    int max_count;
    void (* fn_to_execute)(void *);
    void *args;
} counter_task_args;

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

void normal_input_event_handler(void* event_handler_arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
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
            io_controllers_output_activate(&RELAY1, 1);
            break;
        case SW_2_RELEASED:
            io_controllers_output_activate(&RELAY2, 1);
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

esp_err_t io_controllers_output_activate(const Output *output, const uint8_t value) {
    ESP_LOGI(TAG, "turning %d to value: %d", output->pinNumber, value);
    return gpio_set_level(output->pinNumber, value);
}

int io_controllers_input_read(const Input *input) {
    return gpio_get_level(input->pinNumber);
}

void io_controllers_set_output_switch_mode(Output *output, const SwitchMode mode) {
    output->switchMode = mode;
}

void io_controllers_init() {
    //set up outputs
    gpio_config_t config;
    config.intr_type = GPIO_INTR_DISABLE;
    config.pin_bit_mask = (1ULL << GREEN_LED1.pinNumber) | (1ULL << GREEN_LED2.pinNumber);
    config.mode = GPIO_MODE_OUTPUT;
    config.pull_up_en = 0;
    config.pull_down_en = 0;
    gpio_config(&config);

    //set up inputs/outputs -> input needed when toggle mode is active
    config.pin_bit_mask = (1ULL << RELAY1.pinNumber) | (1ULL << RELAY2.pinNumber) | (1ULL << OPTO1.pinNumber) | (1ULL << OPTO2.pinNumber);
    config.mode = GPIO_MODE_INPUT_OUTPUT;
    gpio_config(&config);

    //set up inputs (buttons)
    config.intr_type = GPIO_INTR_ANYEDGE;
    config.pin_bit_mask = (1ULL << SW1.pinNumber) | (1ULL << SW2.pinNumber);
    config.mode = GPIO_MODE_INPUT;
    config.pull_up_en = 0;
    config.pull_down_en = 0;
    gpio_config(&config);

    //set up inputs (connector)
    config.pin_bit_mask = (1ULL << INPUT1.pinNumber) | (1ULL << INPUT2.pinNumber);
    config.pull_up_en = true;
    gpio_config(&config);

    //isr setup
    gpio_install_isr_service(0);
    gpio_isr_handler_add(INPUT1.pinNumber, isr_handler, (void *)0x01);
    gpio_isr_handler_add(INPUT2.pinNumber, isr_handler, (void *)0x02);
    gpio_isr_handler_add(SW1.pinNumber, isr_handler, (void *)0x04);
    gpio_isr_handler_add(SW2.pinNumber, isr_handler, (void *)0x08);

    //task for periodically checking inputs states after interrupt was called
    xTaskCreate(check_inputs_task, "check inputs task", 2048, NULL, uxTaskPriorityGet(NULL), NULL);

    //event loop init and registration
    esp_event_loop_create_default();
    esp_event_handler_instance_register(INPUT_BASE, ESP_EVENT_ANY_ID, normal_input_event_handler, NULL, NULL);
}