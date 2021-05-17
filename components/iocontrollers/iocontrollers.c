#include "iocontrollers.h"
#include "esp_event.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freeRTOS/task.h"

static const char* TAG = "iocontroller";

ESP_EVENT_DEFINE_BASE(INPUT_BASE);

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

esp_err_t io_controllers_output_activate(const Output *output, const uint8_t value) {
    switch (output->switchMode) {
        case SWITCH_MODE_SIMPLE: {
            //post event
            return gpio_set_level(output->pinNumber, value);
        }
        case SWITCH_MODE_TOGGLE: {
            int lvl = gpio_get_level(output->pinNumber);
            int newLvl = (lvl ^ 1);
            //post event
            return gpio_set_level(output->pinNumber, newLvl);
        }
        case SWITCH_MODE_TIMER: {
            //post event
            ESP_LOGI(TAG, "timer mode is not implemented");
            break;
        }
    }
    return ESP_OK;
}

int io_controllers_input_read(const Input *input) {
    return gpio_get_level(input->pinNumber);
}

void io_controllers_set_output_switch_mode(Output *output, const SwitchMode mode) {
    output->switchMode = mode;
}

void check_inputs_task(void *arg) {
    while(1) {
        if(isr_mask & 0x01) {
            isr_mask &= 0xfe;
            if(gpio_get_level(INPUT1.pinNumber)) {
                ESP_LOGI(TAG, "in1 up");
                esp_event_post(INPUT_BASE, INPUT_1_RELEASED, NULL, 0, portMAX_DELAY);
            } else {
                ESP_LOGI(TAG, "in1 down");
                esp_event_post(INPUT_BASE, INPUT_1_PRESSED, NULL, 0, portMAX_DELAY);
            }
        }
        if(isr_mask & 0x02) {
            isr_mask &= 0xfd;
            if(gpio_get_level(INPUT2.pinNumber)) {
                ESP_LOGI(TAG, "in2 up");
                esp_event_post(INPUT_BASE, INPUT_2_RELEASED, NULL, 0, portMAX_DELAY);
            } else {
                ESP_LOGI(TAG, "in2 down");
                esp_event_post(INPUT_BASE, INPUT_2_PRESSED, NULL, 0, portMAX_DELAY);
            }
        }
        if(isr_mask & 0x04) {
            isr_mask &= 0xfb;
            if(gpio_get_level(SW1.pinNumber)) {
                ESP_LOGI(TAG, "sw1 up");
                esp_event_post(INPUT_BASE, SW_1_RELEASED, NULL, 0, portMAX_DELAY);
            } else {
                ESP_LOGI(TAG, "sw1 down");
                esp_event_post(INPUT_BASE, SW_1_PRESSED, NULL, 0, portMAX_DELAY);
            }
        }
        if(isr_mask & 0x08) {
            isr_mask &= 0xf7;
            if(gpio_get_level(SW2.pinNumber)) {
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

void io_controllers_init() {
    //set up outputs
    gpio_config_t config;
    config.intr_type = GPIO_INTR_DISABLE;
    config.pin_bit_mask = (1ULL << GREEN_LED1.pinNumber) | (1ULL << GREEN_LED2.pinNumber);
    config.mode = GPIO_MODE_OUTPUT;
    config.pull_up_en = 0;
    config.pull_down_en = 0;
    ESP_ERROR_CHECK(gpio_config(&config));

    //set up inputs/outputs -> input needed when toggle mode is active
    config.pin_bit_mask = (1ULL << RELAY1.pinNumber) | (1ULL << RELAY2.pinNumber) | (1ULL << OPTO1.pinNumber) | (1ULL << OPTO2.pinNumber);
    config.mode = GPIO_MODE_INPUT_OUTPUT;
    ESP_ERROR_CHECK(gpio_config(&config));

    //set up inputs (buttons)
    config.intr_type = GPIO_INTR_ANYEDGE;
    config.pin_bit_mask = (1ULL << SW1.pinNumber) | (1ULL << SW2.pinNumber);
    config.mode = GPIO_MODE_INPUT;
    config.pull_up_en = 0;
    config.pull_down_en = 0;
    ESP_ERROR_CHECK(gpio_config(&config));

    //set up inputs (connector)
    config.pin_bit_mask = (1ULL << INPUT1.pinNumber) | (1ULL << INPUT2.pinNumber);
    config.pull_up_en = true;
    ESP_ERROR_CHECK(gpio_config(&config));

    //isr setup
    ESP_ERROR_CHECK(gpio_install_isr_service(0));
    ESP_ERROR_CHECK(gpio_isr_handler_add(INPUT1.pinNumber, isr_handler, (void *)0x01));
    ESP_ERROR_CHECK(gpio_isr_handler_add(INPUT2.pinNumber, isr_handler, (void *)0x02));
    ESP_ERROR_CHECK(gpio_isr_handler_add(SW1.pinNumber, isr_handler, (void *)0x04));
    //ESP_ERROR_CHECK(gpio_isr_handler_add(SW2.pinNumber, isr_handler, (void *)0x08));
    
    //task for periodically checking inputs states after interrupt was called
    xTaskCreate(check_inputs_task, "check inputs task", 2048, NULL, uxTaskPriorityGet(NULL), NULL);
}