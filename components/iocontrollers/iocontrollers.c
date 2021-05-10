#include "iocontrollers.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freeRTOS/task.h"

static const char* TAG = "iocontroller";

uint8_t isr_mask = 0;

void isr_handler(void *value) {
    uint8_t mask = (uint8_t)value;
    isr_mask |= mask;
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
    int pin;
    int max_count;
    void (* fn_to_execute)(void *);
} counter_task_args;

void press_down_counter_task(void *arg) {
    counter_task_args *counterTaskArgs = (counter_task_args *) arg;
    int cnt = 0;
    while(gpio_get_level(counterTaskArgs->pin) == 0 && cnt <= counterTaskArgs->max_count) {
        cnt++;
        vTaskDelay(1);
    }
    if(cnt > counterTaskArgs->max_count) {
        ESP_LOGI(TAG, "long press for pin %d: %d", counterTaskArgs->pin, cnt);
    }
    vTaskDelete(NULL);
}

void check_inputs_task(void *arg) {
    while(1) {
        if(isr_mask & 0x01) {
            isr_mask &= 0xfe;
            if(gpio_get_level(2)) {
                ESP_LOGI(TAG, "in1 up");
            } else {
                ESP_LOGI(TAG, "in1 down");
                counter_task_args in1Args;
                in1Args.pin = INPUT1.pinNumber;
                in1Args.max_count = 200;
                xTaskCreate(press_down_counter_task, "counter in1 task", 2048, (void*)(&in1Args), 0, NULL);
            }
        }
        if(isr_mask & 0x02) {
            isr_mask &= 0xfd;
            if(gpio_get_level(4)) {
                ESP_LOGI(TAG, "in2 up");
            } else {
                ESP_LOGI(TAG, "in2 down");
                counter_task_args in2Args;
                in2Args.pin = INPUT2.pinNumber;
                in2Args.max_count = 200;
                xTaskCreate(press_down_counter_task, "counter in2 task", 2048, (void*)(&in2Args), 0, NULL);
            }
        }
        if(isr_mask & 0x04) {
            isr_mask &= 0xfb;
            if(gpio_get_level(35)) {
                ESP_LOGI(TAG, "sw1 up");
            } else {
                ESP_LOGI(TAG, "sw1 down");
                counter_task_args sw1Args;
                sw1Args.pin = SW1.pinNumber;
                sw1Args.max_count = 200;
                xTaskCreate(press_down_counter_task, "counter sw1 task", 2048, (void*)(&sw1Args), 0, NULL);
            }
        }
        if(isr_mask & 0x08) {
            isr_mask &= 0xf7;
            if(gpio_get_level(36)) {
                ESP_LOGI(TAG, "sw2 up");
            } else {
                ESP_LOGI(TAG, "sw2 down");
                counter_task_args sw2Args;
                sw2Args.pin = SW2.pinNumber;
                sw2Args.max_count = 200;
                xTaskCreate(press_down_counter_task, "counter sw2 task", 2048, (void*)(&sw2Args), 0, NULL);
            }
        }
        vTaskDelay(30);
    }
}

esp_err_t io_controllers_output_activate(const Output *output, const uint8_t value) {
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
    config.pin_bit_mask = (1ULL << RELAY1.pinNumber) | (1ULL << RELAY2.pinNumber) | (1ULL << OPTO1.pinNumber) | (1ULL << OPTO2.pinNumber) | (1ULL << GREEN_LED1.pinNumber) | (1ULL << GREEN_LED2.pinNumber);
    config.mode = GPIO_MODE_OUTPUT;
    config.pull_up_en = 0;
    config.pull_down_en = 0;

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
}