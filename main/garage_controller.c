#include <stdio.h>
#include "iocontrollers.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void app_main(void)
{
    init_io_controller();
    output_activate(&RELAY1, 1);
    output_activate(&RELAY2, 1);
    output_activate(&OPTO1, 1);

    while(1) {
        printf("pinumber[%d] = %d\n", INPUT1.pinNumber, input_read(&INPUT1));
        printf("pinumber[%d] = %d\n", INPUT2.pinNumber, input_read(&INPUT2));
        printf("pinumber[%d] = %d\n", SW1.pinNumber, input_read(&SW1));
        printf("pinumber[%d] = %d\n\n", SW2.pinNumber, input_read(&SW2));
        if (input_read(&SW1) == 0) {
            output_activate(&RELAY1, 1);
        } else {
            output_activate(&RELAY1, 0);
        }
        if (input_read(&SW2) == 0) {
            output_activate(&RELAY2, 1);
        } else {
            output_activate(&RELAY2, 0);
        }
        vTaskDelay(1000 / portTICK_RATE_MS);
    }
}
