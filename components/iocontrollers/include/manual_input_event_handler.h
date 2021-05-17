#ifndef MANUAL_INPUT_EVENT_HANDLER_H
#define MANUAL_INPUT_EVENT_HANDLER_H

#include "esp_event.h"

typedef struct counter_task_args {
    Input *input;
    int max_count;
    void (* fn_to_execute)(void *);
    void *args;
} counter_task_args;

void generate_long_press_event(void *arg);
//one time run task to evaluate whether input given in args is hold for given period of time
void press_down_counter_task(void *arg);
//setting default event handler for pysical inputs
void manual_input_event_handler_init();
//handler of input events in normal operation mode
void normal_mode_input_event_handler(void* event_handler_arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
//handler of input events in menu operation mode
void menu_mode_input_event_handler(void* event_handler_arg, esp_event_base_t event_base, int32_t event_id, void* event_data);

#endif //MANUAL_INPUT_EVENT_HANDLER_H