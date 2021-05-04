#ifndef TEMPERATURE_H
#define TEMPERATURE_H

#define TEMP_CORRECTION CONFIG_TEMP_CORRECTION
#define RESISTANCE_VALUE CONFIG_RESISTANCE_VALUE
#define TEMP_SENSOR_ON_PIN CONFIG_TEMP_SENSOR_ON_PIN

double temp_sensor_get_temperature();
void temp_sensor_turn_on();
void temp_sensor_turn_off();
void temp_sensor_init();

#endif //TEMPERATURE_H