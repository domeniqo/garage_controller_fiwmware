#ifndef MANUAL_INPUT_EVENT_HANDLER_H
#define MANUAL_INPUT_EVENT_HANDLER_H

#include "esp_event.h"

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

typedef struct counter_task_args {
    Input *input;
    int max_count;
    void (* fn_to_execute)(void *);
    void *args;
} counter_task_args;

//initialization of event handler - creating task for checking input interrupts, setting up event handler
void manual_input_event_handler_init();
//one time run task to evaluate whether input given in args is hold for given period of time
void press_down_counter_task(void *arg);
//infinity task to cehck state of inputs (whether interrupt flags were set)
void check_inputs_task(void *arg);
//handler of input events in normal operation mode
void normal_mode_input_event_handler(void* event_handler_arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
//handler of input events in menu operation mode
void menu_mode_input_event_handler(void* event_handler_arg, esp_event_base_t event_base, int32_t event_id, void* event_data);

#endif //MANUAL_INPUT_EVENT_HANDLER_H