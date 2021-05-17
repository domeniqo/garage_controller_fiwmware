#ifndef IO_CONTROLLERS_H
#define IO_CONTROLLERS_H

#include "driver/gpio.h"
#include "esp_event.h"

#define RELAY_1_PIN CONFIG_RELAY_1_PIN
#define RELAY_2_PIN CONFIG_RELAY_2_PIN
#define OPTO_1_PIN CONFIG_OPTO_1_PIN
#define OPTO_2_PIN CONFIG_OPTO_2_PIN
#define SW_1_PIN CONFIG_SW_1_PIN
#define SW_2_PIN CONFIG_SW_2_PIN
#define INPUT_1_PIN CONFIG_INPUT_1_PIN
#define INPUT_2_PIN CONFIG_INPUT_2_PIN
#define GREEN_LED_1_PIN CONFIG_GREEN_LED_1_PIN
#define GREEN_LED_2_PIN CONFIG_GREEN_LED_2_PIN

ESP_EVENT_DECLARE_BASE(INPUT_BASE);
ESP_EVENT_DECLARE_BASE(OUTPUT_BASE);

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

enum OUTPUT_EVENT_IDS {
    RELAY1_ON,
    RELAY1_OFF,
    RELAY2_ON,
    RELAY2_OFF,
    OPTO1_ON,
    OPTO1_OFF,
    OPTO2_ON,
    OPTO2_OFF,
    GREEN_LED1_ON,
    GREEN_LED1_OFF,
    GREEN_LED2_ON,
    GREEN_LED2_OFF
};

typedef enum SwitchMode {
    SWITCH_MODE_TOGGLE,
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

uint8_t isr_mask;
Output RELAY1;
Output RELAY2;
Output OPTO1;
Output OPTO2;
Input SW1;
Input SW2;
Input INPUT1;
Input INPUT2;
const Output GREEN_LED1;
const Output GREEN_LED2;

//infinity task to cehck state of inputs (whether interrupt flags were set)
void check_inputs_task(void *arg);
/***
 * default isr handler set up isr_mask accordingly
 * 
 * isr_mask structure:
 * 0b0000ABCD
 * A - INPUT1
 * B - INPUT2
 * C - SW1
 * D - SW2
 * 
 * */
void isr_handler(void *value);
//activates output accordignly its settings of switch mode (SIMPLE, TIMER, TOGGLE)
esp_err_t io_controllers_output_activate(const Output *output, const uint8_t value);
//just mapping function to gpio_read
int io_controllers_input_read(const Input *input);
//set up output switch mode
void io_controllers_set_output_switch_mode(Output *output, const SwitchMode mode);
/***
 * Initialization of io pins. Pin numbers can be set in menuconfig.
 * 
 * Default settings are:
 * For INPUT1, INPUT2 - input mode, pullup resistor is active, any edge interrupt
 * For SW1, SW2 - input mode, pullup/pulldown resistor is not active, any edge interrupt
 * For RELAY1, RELAY2, OPTO1, OPTO2 - input/output mode (to be able to toggle), no interrupts
 * For GREEN_LED1, GREEN_LED2 - output mode, no interrupts
 * */
void io_controllers_init();

#endif //IO_CONTROLLERS_H