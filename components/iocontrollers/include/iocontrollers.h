#ifndef IO_CONTROLLERS_H
#define IO_CONTROLLERS_H

#include "driver/gpio.h"

#define RELAY_1_PIN CONFIG_RELAY_1_PIN
#define RELAY_2_PIN CONFIG_RELAY_2_PIN
#define OPTO_1_PIN CONFIG_OPTO_1_PIN
#define OPTO_2_PIN CONFIG_OPTO_2_PIN
#define SW_1_PIN CONFIG_SW_1_PIN
#define SW_2_PIN CONFIG_SW_2_PIN
#define INPUT_1_PIN CONFIG_INPUT_1_PIN
#define INPUT_2_PIN CONFIG_INPUT_2_PIN

typedef enum SwitchMode {
    SWTICH_MODE_TOGGLE,
    SWITCH_MODE_SIMPLE,
    SWITCH_MODE_TIMER
} SwitchMode;

typedef struct Output {
    int pinNumber;
    SwitchMode switchMode;
    int delayMicros;
} Output;

typedef struct Input {
    int pinNumber;
    bool pullUp;
} Input;

Output RELAY1;
Output RELAY2;
Output OPTO1;
Output OPTO2;
Input SW1;
Input SW2;
Input INPUT1;
Input INPUT2;

esp_err_t output_activate(const Output *output, const uint8_t value);

int input_read(const Input *input);

void set_output_switch_mode(Output *output, const SwitchMode mode);

void init_io_controller();

#endif //IO_CONTROLLERS_H