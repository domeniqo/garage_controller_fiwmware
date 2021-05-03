#include <gpio.h>

#define RELAY_1_PIN CONFIG_RELAY_1_PIN
#define RELAY_2_PIN CONFIG_RELAY_2_PIN
#define OPTO_1_PIN CONFIG_OPTO_1_PIN
#define OPTO_2_PIN CONFIG_OPTO_2_PIN
#define SW_1_PIN CONFIG_SW_1_PIN
#define SW_2_PIN CONFIG_SW_2_PIN
#define INPUT_1_PIN CONFIG_INPUT_1_PIN
#define INPUT_2_PIN CONFIG_INPUT_2_PIN

typedef enum switchMode {
    SWTICH_MODE_TOGGLE;
    SWITCH_MODE_SIMPLE;
    SWITCH_MODE_TIMER;
} switchMode;

typedef struct output {
    int pinNumber;
    switchMode switchMode;
    int delayMicros;
} output;

typedef struct input {
    int pinNumber;
    bool pullUp;
} input;

const output RELAY1 = {
    .pinNumber = RELAY_1_PIN,
    .switchMode = SWITCH_MODE_SIMPLE,
    .delayMicros = 0
}

const output RELAY2 = {
    .pinNumber = RELAY_2_PIN,
    .switchMode = SWITCH_MODE_SIMPLE,
    .delayMicros = 0
}

const output OPTO1 = {
    .pinNumber = OPTO_1_PIN,
    .switchMode = SWITCH_MODE_SIMPLE,
    .delayMicros = 0
}

const output OPTO2 = {
    .pinNumber = OPTO_2_PIN,
    .switchMode = SWITCH_MODE_SIMPLE,
    .delayMicros = 0
}

const input SW1 {
    .pinNumber = SW_1_PIN,
    .pullup = true;
}

const input SW2 {
    .pinNumber = SW_2_PIN,
    .pullup = true;
}

const input INPUT1 {
    .pinNumber = INPUT_1_PIN,
    .pullup = true;
}

const input INPUT2 {
    .pinNumber = INPUT_2_PIN,
    .pullup = true;
}

esp_err_t output_activate(output *output, uint8_t value);

int input_read(input *input);

void set_output_switch_mode(output *output, switchMode mode);

void init_io_controller();