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

void manual_input_event_handler_init();
void press_down_counter_task(void *arg);
void check_inputs_task(void *arg);
void normal_mode_input_event_handler(void* event_handler_arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
void menu_mode_input_event_handler(void* event_handler_arg, esp_event_base_t event_base, int32_t event_id, void* event_data);

#endif //MANUAL_INPUT_EVENT_HANDLER_H