#include "driver/adc.h"
#include "driver/gpio.h"
#include "esp_adc_cal.h"
#include "temperature.h"
#include <stdio.h>

#define DEFAULT_VREF    1100        //Use adc2_vref_to_gpio() to obtain a better estimate
#define NO_OF_SAMPLES   64          //Multisampling

static esp_adc_cal_characteristics_t *adc_chars;
static const adc_channel_t channel = ADC_CHANNEL_3;     //GPIO39 if ADC1
static const adc_bits_width_t width = ADC_WIDTH_BIT_12;
static const adc_atten_t atten = ADC_ATTEN_DB_0;
static const adc_unit_t unit = ADC_UNIT_1;

/* 
Coefficients of equation for temperature calculation 

for calculating temperature foolowing equations is used:
voltage v is obtained from adc, r1 = 30kOhm

r = (v*r1)/(3,3-v)
 
t = a3 * r ^ 3 + a2 * r ^ 2 + a1 * r + a0

*/
static double a3 = 3.3016E-11;
static double a2 = -1.7002E-6;
static double a1 = 4.0422E-2;
static double a0 = -2.4153E2;

double temp_sensor_get_temperature() {
    uint32_t adc_reading = 0;
    //Multisampling
    for (int i = 0; i < NO_OF_SAMPLES; i++) {
        adc_reading += adc1_get_raw((adc1_channel_t)channel);
    }
    adc_reading /= NO_OF_SAMPLES;
    //Convert adc_reading to voltage in mV
    uint32_t voltage = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);
    
    //calulation of temperature
    double basicVoltage = ((double)voltage)/1000; //voltage in V
    double resistance = (basicVoltage*RESISTANCE_VALUE)/(3.3-basicVoltage);
    double temperature = a3 * resistance * resistance * resistance + a2 * resistance * resistance + a1 * resistance + a0;
    
    //correction for temperature defined by user
    double correction = (double) TEMP_CORRECTION / 10;
    return temperature + correction;
}

void temp_sensor_turn_on() {
    gpio_set_level(TEMP_SENSOR_ON_PIN, 1);
}

void temp_sensor_turn_off() {
    gpio_set_level(TEMP_SENSOR_ON_PIN, 0);
}

void temp_sensor_init() {
    adc1_config_width(width);
    adc1_config_channel_atten(channel, atten);
    
    //Characterize ADC
    adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_characterize(unit, atten, width, DEFAULT_VREF, adc_chars);

    //set up pin for turning on sensor (see schematic)
    gpio_config_t config;
    config.intr_type = GPIO_INTR_DISABLE;
    config.pin_bit_mask = 1ULL<<TEMP_SENSOR_ON_PIN;
    config.pull_down_en = 0;
    config.pull_up_en = 0;
    config.mode = GPIO_MODE_OUTPUT;

    gpio_config(&config);
}
