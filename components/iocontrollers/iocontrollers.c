#include "iocontrollers.h"

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

esp_err_t output_activate(const Output *output, const uint8_t value) {
    return gpio_set_level(output->pinNumber, value);
}

int input_read(const Input *input) {
    return gpio_get_level(input->pinNumber);
}

void set_output_switch_mode(Output *output, const SwitchMode mode) {
    output->switchMode = mode;
}

void init_io_controller() {
    //set up outputs
    gpio_config_t config;
    config.intr_type = GPIO_INTR_DISABLE;
    config.pin_bit_mask = (1U << RELAY1.pinNumber) | (1U << RELAY2.pinNumber) | (1U << OPTO1.pinNumber) | (1U << OPTO2.pinNumber);
    config.mode = GPIO_MODE_OUTPUT;
    config.pull_up_en = 0;
    config.pull_down_en = 0;

    gpio_config(&config);

    //set up inputs
    config.intr_type = GPIO_INTR_DISABLE;
    config.pin_bit_mask = (1ULL << INPUT1.pinNumber) | (1ULL << INPUT2.pinNumber) | (1ULL << SW1.pinNumber) | (1ULL << SW2.pinNumber);
    config.mode = GPIO_MODE_INPUT;
    config.pull_up_en = 0;
    config.pull_down_en = 0;

    gpio_config(&config);
}